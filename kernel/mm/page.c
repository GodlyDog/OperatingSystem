// SMP.1 + SMP.3
// spinlocks + mask interrupts
#include "kernel.h"
#include "types.h"
#include <boot/multiboot_macros.h>

#include "boot/config.h"

#include "mm/mm.h"
#include "mm/page.h"

#include "util/debug.h"
#include "util/gdb.h"
#include "util/string.h"

#include "multiboot.h"

// BTREE === Binary Tree (not an actual B-Tree)

// Algorithmic optimization ideas
// have a "min free idx" pointer for each order (have a "count free" at each
// order) delay cascading availability bits up the tree until needed; would
// prevent state "thrashing"
//      can do this with a cascaded_order flag that equals the highest order
//      which we have cascaed up to. For a given allocation, if the required
//      order is > cascaded_order, then we cascade up to the required order

// get ready for bit manipulation heaven :)

typedef uintptr_t btree_word;

#define BTREE_ROW_START_INDEX(order) \
    (((uintptr_t)1 << (max_order - (order))) - 1)
#define BTREE_ROW_END_INDEX(order) ((BTREE_ROW_START_INDEX(order) << 1) | 1)
#define BTREE_INDEX_TO_ADDR(idx, order) \
    (((1 << (order)) * ((idx)-BTREE_ROW_START_INDEX(order))) << PAGE_SHIFT)
#define BTREE_ADDR_TO_INDEX(addr, order) \
    (BTREE_ROW_START_INDEX(order) +      \
     ((((uintptr_t)(addr)) >> PAGE_SHIFT) / (1 << (order))))

#define BTREE_LEAF_START_INDEX BTREE_ROW_START_INDEX(0)
#define BTREE_ADDR_TO_LEAF_INDEX(addr) BTREE_ADDR_TO_INDEX(addr, 0)
#define BTREE_LEAF_INDEX_TO_ADDR(idx) BTREE_INDEX_TO_ADDR(idx, 0)

#define BTREE_NUM_BITS (sizeof(btree_word) << 3)
#define BTREE_WORD_POS(idx) ((idx) / BTREE_NUM_BITS)
#define BTREE_BIT_POS(idx) ((idx) & (BTREE_NUM_BITS - 1))
#define BTREE_AVAILABILITY_MASK(idx) \
    ((uintptr_t)1 << (BTREE_NUM_BITS - 1 - BTREE_BIT_POS(idx)))

// we really don't want branching here (predictor would be quite bad and
// branches are slowwwww)
#define BTREE_SIBLING(idx) ((idx)-1 + (((idx)&1) << 1))
// uintptr_t btree_sibling(uintptr_t idx) {
//     // in a 0-indexed binary tree, a sibling of an odd node is its right
//     neighbor --> add 1
//     // and the sibling of an even node is its left neighbor --> subtract 1
//     // so we need: (idx % 2) ? (idx + 1) : (idx - 1);
//     uintptr_t odd_addend = idx & 1; // 1 if odd, 0 if even
//     uintptr_t even_addend = (uintptr_t) -1 + odd_addend; // 0 if odd, -1 if
//     even return idx + odd_addend + even_addend; return idx + (idx & 1) +
//     ((uintptr_t) -1 + (idx & 1)); return idx - 1 + ((idx & 1) << 1);
//     // now it looks like: always subtract 1, add 2 if odd. which works :)
// }

// get the left sibling (odd) of a pair; idx may already be the left sibling or
// may be the right sibling (even) subtract 1 from idx if it's even --> subtract
// 1 from LSB and add it back in
#define BTREE_LEFT_SIBLING(idx) ((idx) + (((idx)&1) - 1))

#define BTREE_PARENT(idx) (((idx)-1) >> 1)
#define BTREE_LEFT_CHILD(idx) (((idx) << 1) + 1)
#define BTREE_RIGHT_CHILD(idx) (((idx) + 1) << 1)
#define BTREE_IS_LEFT_CHILD(idx) ((idx)&1)
#define BTREE_IS_RIGHT_CHILD(idx) (!BTREE_IS_LEFT_CHILD(idx))

#define BTREE_IS_AVAILABLE(idx) \
    (btree[BTREE_WORD_POS(idx)] & BTREE_AVAILABILITY_MASK(idx))
#define BTREE_MARK_AVAILABLE(idx) \
    (btree[BTREE_WORD_POS(idx)] |= BTREE_AVAILABILITY_MASK(idx))
#define BTREE_MARK_UNAVAILABLE(idx) \
    (btree[BTREE_WORD_POS(idx)] &= ~BTREE_AVAILABILITY_MASK(idx))

// potential optimization: use these when clearing pairs. something about the
// following is apparently buggy though (causes fault) #define
// BTREE_SIBLING_AVAILABILITY_MASK(idx)    (BTREE_AVAILABILITY_MASK(idx) |
// BTREE_IS_AVAILABLE(BTREE_SIBLING(idx))) #define
// BTREE_MARK_SIBLINGS_AVAILABLE(idx)      (btree[BTREE_WORD_POS(idx)] |=
// BTREE_SIBLING_AVAILABILITY_MASK(idx)) #define
// BTREE_MARK_SIBLINGS_UNAVAILABLE(idx)    (btree[BTREE_WORD_POS(idx)] &=
// ~BTREE_SIBLING_AVAILABILITY_MASK(idx))

GDB_DEFINE_HOOK(page_alloc, void *addr, size_t npages)

GDB_DEFINE_HOOK(page_free, void *addr, size_t npages)

static spinlock_t page_spinlock;

static size_t page_freecount;

// if you rename these variables, update them in the macros above
static size_t
    max_pages;           // max number of pages as determined by RAM, NOT max_order
static size_t max_order; // max depth of binary tree

static btree_word *btree;
static uintptr_t *min_available_idx_by_order;
static size_t *count_available_by_order;

static char *type_strings[] = {"ERROR: type = 0", "Available", "Reserved",
                               "ACPI Reclaimable", "ACPI NVS", "GRUB Bad Ram"};
static size_t type_count = sizeof(type_strings) / sizeof(type_strings[0]);

inline void *physmap_start() { return (void *)PHYS_OFFSET; }

inline void *physmap_end()
{
    return (void *)(PHYS_OFFSET + (max_pages << PAGE_SHIFT));
}

#undef DEBUG_PHYSICAL_PAGING

static inline void _btree_expensive_sanity_check()
{
#ifdef DEBUG_PHYSICAL_PAGING
    size_t available = 0;
    for (unsigned order = 0; order <= max_order; order++)
    {
        long checked_first = 0;
        unsigned order_count = 0;
        uintptr_t max = BTREE_ROW_END_INDEX(order);

        for (uintptr_t idx = BTREE_ROW_START_INDEX(order); idx < max; idx++)
        {
            if (BTREE_IS_AVAILABLE(idx))
            {
                if (!checked_first)
                {
                    KASSERT(min_available_idx_by_order[order] == idx);
                    checked_first = 1;
                }
                available += (1 << order);
                order_count++;
                KASSERT(BTREE_INDEX_TO_ADDR(idx + 1, order) <= physmap_end());
            }
        }
        if (!checked_first)
        {
            KASSERT(min_available_idx_by_order[order] == max);
        }
        KASSERT(count_available_by_order[order] == order_count);
    }
    KASSERT(available == page_freecount);
#endif
}

void page_init()
{
    spinlock_init(&page_spinlock);
    uintptr_t ram = 0;
    uintptr_t memory_available_for_use = 0;

    // detect amount of RAM and memory available for use immediately after
    // kernel before any reserved region

    KASSERT(PAGE_ALIGNED(mb_tag) && (uintptr_t)mb_tag == KERNEL_PHYS_END);

    for (struct multiboot_tag *tag = mb_tag + 1;
         tag->type != MULTIBOOT_TAG_TYPE_END; tag += TAG_SIZE(tag->size))
    {
        if (tag->type != MULTIBOOT_TAG_TYPE_MMAP)
        {
            continue;
        }
        struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)tag;
        dbg(DBG_PAGEALLOC, "Physical memory map (%d entries):\n",
            mmap->size / mmap->entry_size);
        for (unsigned i = 0; i < mmap->size / mmap->entry_size; i++)
        {
            struct multiboot_mmap_entry *entry = &mmap->entries[i];
            dbgq(DBG_MM, "\t[0x%p-0x%p) (%llu bytes): %s\n",
                 (void *)entry->addr, (void *)(entry->addr + entry->len),
                 entry->len,
                 entry->type < type_count ? type_strings[entry->type]
                                          : "Unknown");
            if (entry->type != MULTIBOOT_MEMORY_AVAILABLE)
            {
                continue;
            }

            if (entry->addr < KERNEL_PHYS_END &&
                entry->addr + entry->len > KERNEL_PHYS_END)
            {
                memory_available_for_use =
                    entry->addr + entry->len - KERNEL_PHYS_END;
            }

            if (entry->addr + entry->len > ram)
            {
                ram = entry->addr + entry->len;
            }
        }
    }

    // check we have enough space available following the kernel to map in all
    // of RAM detected
    max_pages = ram >> PAGE_SHIFT;
    max_order = 0;
    size_t npages = max_pages;
    while (npages)
    {
        max_order++;
        npages >>= 1;
    }

    // we may have too much RAM than we can map in with the single memory holy
    // after the kernel keep shrinking the maximum order until we find a size
    // that fits (this can obviously be done more intelligently, but this also
    // works)
    size_t btree_size;
    size_t metadata_size;
    while (max_order)
    {
        // we need 2^(max_order+1) pages, and one byte maps 8 pages, so we need
        // 2^(max_order-2) bytes for the binary tree
        btree_size = 1UL << (max_order - 2);
        metadata_size = sizeof(uintptr_t) * (max_order + 1) +
                        sizeof(size_t) * (max_order + 1);

        if (memory_available_for_use >= btree_size + metadata_size)
        {
            break;
        }
        if (max_pages ==
            (ram >> PAGE_SHIFT))
        { // only print first time we shrink
            dbg(DBG_PAGEALLOC,
                "Warning! Need 0x%p B of memory to map in 0x%p B of RAM, but "
                "only have 0x%p available!",
                (void *)(btree_size + metadata_size), (void *)ram,
                (void *)memory_available_for_use);
        }
        max_order--;
        max_pages = 1UL << max_order;
    }
    if (max_pages !=
        (ram >> PAGE_SHIFT))
    { // only print if we shrank available RAM
        dbg(DBG_PAGEALLOC, "Supporting only up to 0x%p B of RAM!",
            (void *)(max_pages << PAGE_SHIFT));
    }

    btree = (btree_word
                 *)(KERNEL_PHYS_END +
                    PAGE_SIZE); // 1 page padding for the multiboot information
    memset(btree, 0, btree_size);

    min_available_idx_by_order = (uintptr_t *)((uintptr_t)btree + btree_size);
    for (unsigned order = 0; order <= max_order; order++)
    {
        min_available_idx_by_order[order] = BTREE_ROW_END_INDEX(order);
    }

    count_available_by_order =
        min_available_idx_by_order + sizeof(uintptr_t) * (max_order + 1);
    memset(count_available_by_order, 0, sizeof(size_t) * (max_order + 1));

    page_freecount = 0;

    uintptr_t reserved_ram_start = KERNEL_PHYS_BASE;
    uintptr_t reserved_ram_end =
        KERNEL_PHYS_END + PAGE_SIZE + btree_size + metadata_size;

    for (struct multiboot_tag *tag = mb_tag + 1;
         tag->type != MULTIBOOT_TAG_TYPE_END; tag += TAG_SIZE(tag->size))
    {
        if (tag->type != MULTIBOOT_TAG_TYPE_MMAP)
        {
            continue;
        }
        struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)tag;
        for (unsigned i = 0; i < mmap->size / mmap->entry_size; i++)
        {
            struct multiboot_mmap_entry *entry = &mmap->entries[i];
            if (entry->type != MULTIBOOT_MEMORY_AVAILABLE)
            {
                continue;
            }
            uintptr_t addr = entry->addr;
            uintptr_t len = entry->len;

            if (addr >= reserved_ram_start && addr < reserved_ram_end)
            {
                if (len <= reserved_ram_end - addr)
                {
                    continue;
                }
                len -= reserved_ram_end - addr;
                addr = reserved_ram_end;
            }
            if (addr < reserved_ram_start && addr + len > reserved_ram_start)
            {
                len = reserved_ram_start - addr;
            }

            // TODO [+] see why removing this crashes SMP
            if (addr < reserved_ram_start)
            {
                continue;
            }

            page_add_range((void *)addr, (void *)(addr + len));
        }
    }

    page_mark_reserved(0); // don't allocate the first page of memory

    size_t bytes = page_freecount << PAGE_SHIFT;
    size_t gigabytes = (bytes >> 30);
    bytes -= (gigabytes << 30);
    size_t megabytes = (bytes >> 20);
    bytes -= (megabytes << 20);
    size_t kilobytes = (bytes >> 10);
    bytes -= (kilobytes << 10);
    KASSERT(bytes == 0);

    dbg(DBG_PAGEALLOC,
        "Amount of physical memory available for use: %lu GB, %lu MB, and %lu "
        "KB; [0x%p, 0x%p)\n",
        gigabytes, megabytes, kilobytes, physmap_start(), physmap_end());
    _btree_expensive_sanity_check();
}

void page_init_finish()
{
    btree = (btree_word *)((uintptr_t)btree + PHYS_OFFSET);
    min_available_idx_by_order =
        (uintptr_t *)((uintptr_t)min_available_idx_by_order + PHYS_OFFSET);
    count_available_by_order =
        (uintptr_t *)((uintptr_t)count_available_by_order + PHYS_OFFSET);
}

static void _btree_update_metadata_after_removal(size_t order, size_t idx)
{
    // [+] TODO Intel-specific optimizations, see BSF, BSR, REPE CMPS/SCAS
    if (count_available_by_order[order])
    {
        if (idx == min_available_idx_by_order[order])
        {
            uintptr_t word_idx = BTREE_WORD_POS(idx);
            if (btree[word_idx] &&
                word_idx == BTREE_WORD_POS(BTREE_ROW_START_INDEX(order)))
            {
                // mask off bits to the left of BTREE_BIT_POS(idx); i.e.
                // consider only positions > than BTREE_BIT_POS(idx) in
                // btree[word_idx] when idx is the old index of the first
                // available node for the given order. This is to avoid setting
                // min available for an order x to be an index that actually
                // belongs to order (x + 1) (in the row above).
                btree_word copy =
                    btree[word_idx] &
                    ((1UL << (BTREE_NUM_BITS - BTREE_BIT_POS(idx))) - 1);
                unsigned bit_idx = BTREE_NUM_BITS;
                while (copy != 0 && bit_idx > BTREE_BIT_POS(idx))
                {
                    bit_idx--;
                    copy = copy >> 1;
                }
                if (BTREE_IS_AVAILABLE(word_idx * BTREE_NUM_BITS + bit_idx))
                {
                    min_available_idx_by_order[order] =
                        word_idx * BTREE_NUM_BITS + bit_idx;
                    return;
                }
                word_idx++;
            }
            while (!btree[word_idx])
                word_idx++;
            btree_word copy = btree[word_idx];
            unsigned bit_idx = BTREE_NUM_BITS;
            while (copy != 0)
            {
                bit_idx--;
                copy = copy >> 1;
            }
            uintptr_t min_available = word_idx * BTREE_NUM_BITS + bit_idx;
            if (min_available > BTREE_ROW_END_INDEX(order))
            {
                min_available = BTREE_ROW_END_INDEX(order);
            }
            min_available_idx_by_order[order] = min_available;
        }
    }
    else
    {
        min_available_idx_by_order[order] = BTREE_ROW_END_INDEX(order);
    }
}

static void _btree_mark_available(uintptr_t idx, size_t order)
{
    KASSERT(!BTREE_IS_AVAILABLE(idx));
    BTREE_MARK_AVAILABLE(idx);

    uintptr_t start = BTREE_INDEX_TO_ADDR(idx, order);
    uintptr_t end = BTREE_INDEX_TO_ADDR(idx + 1, order);
    dbg(DBG_MM, "marking available (0x%p, 0x%p)\n", (void *)start, (void *)end);
    KASSERT(!(0xb1000 >= start && 0xb1000 < end));

    count_available_by_order[order]++;
    if (idx < min_available_idx_by_order[order])
    {
        min_available_idx_by_order[order] = idx;
    }

    while (idx > 0 && BTREE_IS_AVAILABLE(BTREE_SIBLING(idx)))
    {
        BTREE_MARK_UNAVAILABLE(idx);
        BTREE_MARK_UNAVAILABLE(BTREE_SIBLING(idx));

        count_available_by_order[order] -= 2;
        _btree_update_metadata_after_removal(order, BTREE_LEFT_SIBLING(idx));

        idx = BTREE_PARENT(idx);
        order++;
        BTREE_MARK_AVAILABLE(idx);
        count_available_by_order[order]++;
        if (idx < min_available_idx_by_order[order])
        {
            min_available_idx_by_order[order] = idx;
        }
    }
}

static void _btree_mark_range_available(uintptr_t leaf_idx, size_t npages)
{
    // coult be optimized further so that we don't need to keep traversing fromm
    // leaf to max order. can instead jump to parent's (right) sibling when
    // we are a right child, and by jumping to left child while npages > what
    // would be allocated but for now, this works and is fast enough it seems...
    // TODO potential optimization
    while (npages)
    {
        uintptr_t idx = leaf_idx;
        size_t order = 0;
        while (BTREE_IS_LEFT_CHILD(idx) && (2UL << order) <= npages)
        {
            idx = BTREE_PARENT(idx);
            order++;
        }
        _btree_mark_available(idx, order);
        npages -= 1 << order;
        leaf_idx += 1 << order;
    }
}

void page_add_range(void *start, void *end)
{
    dbg(DBG_MM, "Page system adding range [0x%p, 0x%p)\n", start, end);
    KASSERT(end > start);
    if (start == 0)
    {
        start = PAGE_ALIGN_UP(1);
        if (end <= start)
        {
            return;
        }
    }
    start = PAGE_ALIGN_UP(start);
    end = PAGE_ALIGN_DOWN(end);
    size_t npages = ((uintptr_t)end - (uintptr_t)start) >> PAGE_SHIFT;
    _btree_mark_range_available(BTREE_ADDR_TO_LEAF_INDEX(start), npages);
    page_freecount += npages;
    _btree_expensive_sanity_check();
}

void *page_alloc() { return page_alloc_n(1); }

void *page_alloc_bounded(void *max_paddr)
{
    return page_alloc_n_bounded(1, max_paddr);
}

void page_free(void *addr) { page_free_n(addr, 1); }

static void *_btree_alloc(size_t npages, uintptr_t idx, size_t smallest_order,
                          size_t actual_order)
{
    while (actual_order != smallest_order)
    {
        BTREE_MARK_UNAVAILABLE(idx);
        count_available_by_order[actual_order]--;
        _btree_update_metadata_after_removal(actual_order, idx);

        idx = BTREE_LEFT_CHILD(idx);
        BTREE_MARK_AVAILABLE(idx);
        BTREE_MARK_AVAILABLE(BTREE_SIBLING(idx));
        actual_order--;

        count_available_by_order[actual_order] += 2;
        if (idx < min_available_idx_by_order[actual_order])
        {
            min_available_idx_by_order[actual_order] = idx;
        }
        _btree_expensive_sanity_check();
    }

    // actually allocate the 2^smallest_order pages by marking them unavailable
    BTREE_MARK_UNAVAILABLE(idx);
    count_available_by_order[actual_order]--;
    _btree_update_metadata_after_removal(actual_order, idx);

    uintptr_t allocated_idx = idx;
    size_t allocated_order = actual_order;
    while (allocated_order-- > 0)
        allocated_idx = BTREE_LEFT_CHILD(allocated_idx);

    KASSERT(BTREE_LEAF_INDEX_TO_ADDR(allocated_idx));

    // we allocated some 2^smallest_order of pages; it is possible they asked
    // for fewer than 2^smallest_order pages; make sure we mark as available the
    // remaining (2^smallest_order - npages) pages.
    _btree_mark_range_available(allocated_idx + npages,
                                (1 << smallest_order) - npages);

    //    while (over_allocated > 0 && (1 << reclaimed_order) < over_allocated
    //    && next_leaf_to_reclaim < max_reclaim_idx) {
    //        BTREE_MARK_AVAILABLE(idx);
    //        count_available_by_order[reclaimed_order]++;
    //        if (idx < min_available_idx_by_order[reclaimed_order]) {
    //            min_available_idx_by_order[reclaimed_order] = idx;
    //        }
    //        over_allocated -= (1 << reclaimed_order);
    //        next_leaf_to_reclaim += (2 << reclaimed_order);
    //        idx = BTREE_SIBLING(BTREE_PARENT(idx));
    //        reclaimed_order++;
    //    }

    page_freecount -= npages;

    uintptr_t addr = BTREE_LEAF_INDEX_TO_ADDR(allocated_idx);
    dbgq(DBG_MM, "page_alloc_n(%lu): [0x%p, 0x%p)\t\t%lu pages remain\n",
         npages, (void *)(PHYS_OFFSET + addr),
         (void *)(PHYS_OFFSET + addr + (npages << PAGE_SHIFT)), page_freecount);
    _btree_expensive_sanity_check();
    return (void *)(addr + PHYS_OFFSET);
}

void *page_alloc_n(size_t npages)
{
    return page_alloc_n_bounded(npages, (void *)~0UL);
}

// this is really only used for setting up initial page tables
// this memory will be immediately overriden, so no need to poison the memory
void *page_alloc_n_bounded(size_t npages, void *max_paddr)
{
    spinlock_lock(&page_spinlock);
    KASSERT(npages > 0 && npages <= (1UL << max_order));
    if (npages > page_freecount)
    {
        return 0;
    }
    // a note on max_pages: so long as we never mark a page that is beyond our
    // RAM as available, we will never allocate it. So put all those checks at
    // the free and map functions

    // find the smallest order that will fit npages
    uintptr_t max_page_number =
        ((uintptr_t)max_paddr >> PAGE_SHIFT) - npages + 1;

    // [+] TODO intel-specific optimization possible here?
    size_t smallest_order = 0;
    while ((1UL << smallest_order) < npages)
        smallest_order++;

    for (size_t actual_order = smallest_order; actual_order <= max_order;
         actual_order++)
    {
        if (!count_available_by_order[actual_order])
        {
            continue;
        }
        uintptr_t idx = min_available_idx_by_order[actual_order];
        KASSERT(idx >= BTREE_ROW_START_INDEX(actual_order) &&
                idx < BTREE_ROW_END_INDEX(actual_order));
        if ((idx - BTREE_ROW_START_INDEX(actual_order)) * (1 << actual_order) <
            max_page_number)
        {
            KASSERT((idx - BTREE_ROW_START_INDEX(actual_order)) *
                        (1 << actual_order) <
                    max_pages);

            void *ret = _btree_alloc(npages, idx, smallest_order, actual_order);
            KASSERT(((uintptr_t)ret + (npages << PAGE_SHIFT)) <=
                    (uintptr_t)physmap_end());
            spinlock_unlock(&page_spinlock);
            return ret;
        }
    }
    spinlock_unlock(&page_spinlock);
    return 0;
}

void page_free_n(void *addr, size_t npages)
{
    spinlock_lock(&page_spinlock);
    dbgq(DBG_MM, "page_free_n(%lu): [0x%p, 0x%p)\t\t%lu pages remain\n", npages,
         addr, (void *)((uintptr_t)addr + (npages << PAGE_SHIFT)),
         page_freecount);
    GDB_CALL_HOOK(page_free, addr, npages);
    KASSERT(npages > 0 && npages <= (1UL << max_order) && PAGE_ALIGNED(addr));
    uintptr_t idx = BTREE_ADDR_TO_LEAF_INDEX((uintptr_t)addr - PHYS_OFFSET);
    KASSERT(idx + npages - BTREE_LEAF_START_INDEX <= max_pages);
    _btree_mark_range_available(idx, npages);
    page_freecount += npages;
    _btree_expensive_sanity_check();
    spinlock_unlock(&page_spinlock);
}

void page_mark_reserved(void *paddr)
{
    if ((uintptr_t)paddr > (max_pages << PAGE_SHIFT))
        return;

    spinlock_lock(&page_spinlock);
    dbgq(DBG_MM, "page_mark_reserved(0x%p): [0x%p, 0x%p)\n",
         (void *)((uintptr_t)paddr + PHYS_OFFSET),
         (void *)((uintptr_t)paddr + PHYS_OFFSET),
         (void *)((uintptr_t)paddr + PHYS_OFFSET + PAGE_SIZE));

    KASSERT(PAGE_ALIGNED(paddr));
    uintptr_t idx = BTREE_ADDR_TO_LEAF_INDEX(paddr);
    size_t order = 0;
    while (idx && !BTREE_IS_AVAILABLE(idx))
    {
        idx = BTREE_PARENT(idx);
        order++;
    }
    if (!BTREE_IS_AVAILABLE(idx))
    {
        spinlock_unlock(&page_spinlock);
        return; // can sometimes be a part of reserved RAM anyway
    }

    BTREE_MARK_UNAVAILABLE(idx);
    count_available_by_order[order]--;
    _btree_update_metadata_after_removal(order, idx);

    uintptr_t unavailable_leaf_idx = BTREE_ADDR_TO_LEAF_INDEX(paddr);
    uintptr_t still_available_leaf_idx_start =
        BTREE_ADDR_TO_LEAF_INDEX(BTREE_INDEX_TO_ADDR(idx, order));
    uintptr_t still_available_leaf_idx_end =
        BTREE_ADDR_TO_LEAF_INDEX(BTREE_INDEX_TO_ADDR(idx + 1, order));

    _btree_mark_range_available(
        still_available_leaf_idx_start,
        unavailable_leaf_idx - still_available_leaf_idx_start);
    _btree_mark_range_available(
        unavailable_leaf_idx + 1,
        still_available_leaf_idx_end - unavailable_leaf_idx - 1);

    page_freecount--;

    _btree_expensive_sanity_check();
    spinlock_unlock(&page_spinlock);
}

size_t page_free_count() { return page_freecount; }
