#include "errno.h"
#include "globals.h"
#include "kernel.h"
#include "types.h"

#include "mm/mm.h"
#include "mm/pframe.h"

#include "util/debug.h"
#include "util/string.h"

#include "vm/pagefault.h"

typedef enum
{
    UNMAPPED,
    PAGE_4KB,
    PAGE_2MB,
    PAGE_1GB
} vaddr_map_status;

static pml4_t *global_kernel_only_pml4;

void pt_set(pml4_t *pml4)
{
    KASSERT((void *)pml4 >= physmap_start());
    uintptr_t phys_addr = pt_virt_to_phys((uintptr_t)pml4);
    __asm__ volatile("movq %0, %%cr3" ::"r"(phys_addr)
                     : "memory");
}

/*
 * Don't use this for proc_create. You want each new proc to have a copy
 * of the current page table (see pt_create).
 * 
 * Returns a pointer to the current pagetable (a virtual address).
 */
inline pml4_t *pt_get()
{
    uintptr_t pml4;
    __asm__ volatile("movq %%cr3, %0"
                     : "=r"(pml4));
    return (pml4_t *)(pml4 + PHYS_OFFSET);
}

vaddr_map_status _vaddr_status(pml4_t *pml4, uintptr_t vaddr)
{
    uint64_t idx;
    pml4_t *table = pml4;

    idx = PML4E(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return UNMAPPED;
    }
    table = (pdp_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PDP (1GB pages)
    idx = PDPE(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return UNMAPPED;
    }
    if (IS_1GB_PAGE(table->phys[idx]))
    {
        return PAGE_1GB;
    }
    table = (pd_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PD (2MB pages)
    idx = PDE(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return UNMAPPED;
    }
    if (IS_2MB_PAGE(table->phys[idx]))
    {
        return PAGE_2MB;
    }
    table = (pt_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PT (4KB pages)
    idx = PTE(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return UNMAPPED;
    }
    return PAGE_4KB;
}

uintptr_t pt_virt_to_phys_helper(pml4_t *table, uintptr_t vaddr)
{
    if (vaddr >= (uintptr_t)physmap_start() &&
        vaddr < (uintptr_t)physmap_end())
    {
        return vaddr - PHYS_OFFSET;
    }

    uint64_t idx;

    // PML4
    idx = PML4E(vaddr);
    KASSERT(IS_PRESENT(table->phys[idx]));
    table = (pdp_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PDP (1GB pages)
    idx = PDPE(vaddr);
    KASSERT(IS_PRESENT(table->phys[idx]));
    if (USE_1GB_PAGES && IS_1GB_PAGE(table->phys[idx]))
    {
        return PAGE_ALIGN_DOWN_1GB(table->phys[idx]) + PAGE_OFFSET_1GB(vaddr);
    }
    table = (pd_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PD (2MB pages)
    idx = PDE(vaddr);
    KASSERT(IS_PRESENT(table->phys[idx]));
    if (USE_2MB_PAGES && IS_2MB_PAGE(table->phys[idx]))
    {
        return PAGE_ALIGN_DOWN_2MB(table->phys[idx]) + PAGE_OFFSET_2MB(vaddr);
    }
    table = (pt_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PT (4KB pages)
    idx = PTE(vaddr);

    KASSERT(IS_PRESENT(table->phys[idx]));

    return (uintptr_t)PAGE_ALIGN_DOWN(table->phys[idx]) + PAGE_OFFSET(vaddr);
}

uintptr_t pt_virt_to_phys(uintptr_t vaddr)
{
    if (vaddr >= (uintptr_t)physmap_start() &&
        vaddr < (uintptr_t)physmap_end())
    {
        // if the address is within the PHYS_MAP region, then subtract the
        // PHYS_OFFSET to get the physical address. There is a one-to-one mapping
        // between virtual and physical addresses in this region.
        return vaddr - PHYS_OFFSET;
    }
    return pt_virt_to_phys_helper(pt_get(), vaddr);
}

void _fill_pt(pt_t *pt, uintptr_t paddr, uintptr_t vaddr, uintptr_t vmax)
{
    for (uintptr_t idx = PTE(vaddr); idx < PT_ENTRY_COUNT && vaddr < vmax;
         idx++, paddr += PAGE_SIZE, vaddr += PAGE_SIZE)
    {
        pt->phys[idx] = (uintptr_t)paddr | PT_PRESENT | PT_WRITE;
    }
}

long _fill_pd(pd_t *pd, uintptr_t paddr, uintptr_t vaddr, uintptr_t vmax,
              uintptr_t max_paddr)
{
    for (uintptr_t idx = PDE(vaddr); idx < PT_ENTRY_COUNT && vaddr < vmax;
         idx++, paddr += PT_VADDR_SIZE, vaddr += PT_VADDR_SIZE)
    {
        KASSERT(!IS_PRESENT(pd->phys[idx]));
#if USE_2MB_PAGES
        if (vmax - vaddr >= PT_VADDR_SIZE)
        {
            pd->phys[idx] = paddr | PT_PRESENT | PT_WRITE | PT_SIZE;
            continue;
        }
#endif

        uintptr_t pt = (uintptr_t)page_alloc_bounded((void *)max_paddr);
        if (!pt)
        {
            return 1;
        }
        pt -= PHYS_OFFSET;

        memset((void *)pt, 0, PAGE_SIZE);
        pd->phys[idx] = pt | PT_PRESENT | PT_WRITE;
        _fill_pt((pt_t *)pt, paddr, vaddr, vmax);
    }
    return 0;
}

long _fill_pdp(pdp_t *pdp, uintptr_t paddr, uintptr_t vaddr, uintptr_t vmax,
               uintptr_t max_paddr)
{
    for (uintptr_t idx = PDPE(vaddr); idx < PT_ENTRY_COUNT && vaddr < vmax;
         idx++, paddr += PD_VADDR_SIZE, vaddr += PD_VADDR_SIZE)
    {
        KASSERT(!IS_PRESENT(pdp->phys[idx]));
#if USE_1GB_PAGES
        if (vmax - vaddr >= PD_VADDR_SIZE)
        {
            pdp->phys[idx] = paddr | PT_PRESENT | PT_WRITE | PT_SIZE;
            continue;
        }
#endif

        uintptr_t pd = (uintptr_t)page_alloc_bounded((void *)max_paddr);
        if (!pd)
        {
            return 1;
        }
        pd -= PHYS_OFFSET;

        memset((void *)pd, 0, PAGE_SIZE);
        pdp->phys[idx] = pd | PT_PRESENT | PT_WRITE;
        if (_fill_pd((pd_t *)pd, paddr, vaddr, vmax, max_paddr))
        {
            return 1;
        }
    }
    return 0;
}

long _fill_pml4(pml4_t *pml4, uintptr_t paddr, uintptr_t vaddr, uintptr_t vmax,
                uintptr_t max_paddr)
{
    for (uintptr_t idx = PML4E(vaddr); idx < PT_ENTRY_COUNT && vaddr < vmax;
         idx++, paddr += PDP_VADDR_SIZE, vaddr += PDP_VADDR_SIZE)
    {
        KASSERT(!IS_PRESENT(pml4->phys[idx]));

        uintptr_t pdp = (uintptr_t)page_alloc_bounded((void *)max_paddr);
        if (!pdp)
        {
            return 1;
        }
        pdp -= PHYS_OFFSET;

        memset((void *)pdp, 0, PAGE_SIZE);
        pml4->phys[idx] = pdp | PT_PRESENT | PT_WRITE;
        if (_fill_pdp((pdp_t *)pdp, paddr, vaddr, vmax, max_paddr))
        {
            return 1;
        }
    }
    return 0;
}

long pt_map(pml4_t *pml4, uintptr_t paddr, uintptr_t vaddr, uint32_t pdflags,
            uint32_t ptflags)
{
    return pt_map_range(pml4, paddr, vaddr, vaddr + PAGE_SIZE, pdflags,
                        ptflags);
}

long pt_map_range(pml4_t *pml4, uintptr_t paddr, uintptr_t vaddr,
                  uintptr_t vmax, uint32_t pdflags, uint32_t ptflags)
{
    dbg(DBG_PGTBL, "[0x%p, 0x%p) mapped to 0x%p; pml4: 0x%p\n", (void *)vaddr,
        (void *)vmax, (void *)paddr, pml4);
    KASSERT(PAGE_ALIGNED(paddr) && PAGE_ALIGNED(vaddr) && PAGE_ALIGNED(vmax));
    KASSERT(vmax > vaddr && (ptflags & PAGE_MASK) == 0 &&
            (pdflags & PAGE_MASK) == 0);
    KASSERT((pdflags & PT_USER) == (ptflags & PT_USER));
    KASSERT(!(pdflags & PT_SIZE) && !(ptflags & PT_SIZE));

    while (vaddr < vmax)
    {
        uint64_t size = vmax - vaddr;

        uint64_t idx = PML4E(vaddr);
        pml4_t *table = pml4;

        if (!IS_PRESENT(table->phys[idx]))
        {
            uintptr_t page = (uintptr_t)page_alloc();
            if (!page)
            {
                return -ENOMEM;
            }
            memset((void *)page, 0, PAGE_SIZE);
            KASSERT(pt_virt_to_phys(page) == page - PHYS_OFFSET);
            KASSERT(*(uintptr_t *)page == 0);
            table->phys[idx] = (page - PHYS_OFFSET) | pdflags;
        }
        else
        {
            // can't split up if control flags don't match, so liberally include
            // all of them
            table->phys[idx] |= pdflags;
        }
        table = (pdp_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

        // PDP (1GB pages)
        idx = PDPE(vaddr);
        if (!IS_PRESENT(table->phys[idx]))
        {
#if USE_1GB_PAGES
            if (PAGE_ALIGNED_1GB(vaddr) && size > PAGE_SIZE_1GB)
            {
                table->phys[idx] = (uintptr_t)paddr | ptflags | PT_SIZE;
                paddr += PAGE_SIZE_1GB;
                vaddr += PAGE_SIZE_1GB;
                continue;
            }
#endif
            uintptr_t page = (uintptr_t)page_alloc();
            if (!page)
            {
                return -ENOMEM;
            }
            memset((void *)page, 0, PAGE_SIZE);
            table->phys[idx] = (page - PHYS_OFFSET) | pdflags;
        }
        else if (IS_1GB_PAGE(table->phys[idx]))
        {
            if (PAGE_SAME_1GB(table->phys[idx], paddr) &&
                PAGE_OFFSET_1GB(paddr) == PAGE_OFFSET_1GB(vaddr) &&
                PAGE_CONTROL_FLAGS(table->phys[idx]) - PT_SIZE == pdflags)
            {
                vaddr = PAGE_ALIGN_UP_1GB(vaddr + 1);
                continue;
            }
            pd_t *pd = page_alloc();
            if (!pd)
            {
                return -ENOMEM;
            }
            for (unsigned i = 0; i < PT_ENTRY_COUNT; i++)
            {
                pd->phys[i] =
                    table->phys[idx] +
                    i * PAGE_SIZE_2MB; // keeps all flags, including PT_SIZE
            }
            table->phys[idx] =
                ((uintptr_t)pd - PHYS_OFFSET) |
                pdflags; // overwrite flags as well for particular entry
        }
        else
        {
            table->phys[idx] |= pdflags;
        }
        table = (pd_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

        // PD (2MB pages)
        idx = PDE(vaddr);
        if (!IS_PRESENT(table->phys[idx]))
        {
#if USE_2MB_PAGES
            if (PAGE_ALIGNED_2MB(vaddr) && size > PAGE_SIZE_2MB)
            {
                table->phys[idx] = (uintptr_t)paddr | ptflags | PT_SIZE;
                paddr += PAGE_SIZE_2MB;
                vaddr += PAGE_SIZE_2MB;
                continue;
            }
#endif
            uintptr_t page = (uintptr_t)page_alloc();
            if (!page)
            {
                return -ENOMEM;
            }
            memset((void *)page, 0, PAGE_SIZE);
            table->phys[idx] = (page - PHYS_OFFSET) | pdflags;
        }
        else if (IS_2MB_PAGE(table->phys[idx]))
        {
            if (PAGE_SAME_2MB(table->phys[idx], paddr) &&
                PAGE_OFFSET_2MB(paddr) == PAGE_OFFSET_2MB(vaddr) &&
                PAGE_CONTROL_FLAGS(table->phys[idx]) - PT_SIZE == ptflags)
            {
                vaddr = PAGE_ALIGN_UP_2MB(vaddr + 1);
                continue;
            }
            pt_t *pt = page_alloc();
            if (!pt)
            {
                return -ENOMEM;
            }
            for (unsigned i = 0; i < PT_ENTRY_COUNT; i++)
            {
                pt->phys[i] = table->phys[idx] + i * PAGE_SIZE -
                              PT_SIZE; // remove PT_SIZE flag
            }
            table->phys[idx] =
                ((uintptr_t)pt - PHYS_OFFSET) | pdflags; // overwrite flags
        }
        else
        {
            table->phys[idx] |= pdflags;
        }
        table = (pt_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

        // PT (4KB pages)

        idx = PTE(vaddr);
        table->phys[idx] = (uintptr_t)paddr | ptflags;

        KASSERT(IS_PRESENT(table->phys[idx]));

        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }

    return 0;
}

static long _pt_fault_handler(regs_t *regs)
{
    uintptr_t vaddr;
    /* Get the address where the fault occurred */
    __asm__ volatile("movq %%cr2, %0"
                     : "=r"(vaddr));
    uintptr_t cause = regs->r_err;

    /* Check if pagefault was in user space (otherwise, BAD!) */
    if (cause & FAULT_USER)
    {
        handle_pagefault(vaddr, cause);
    }
    else
    {
        dump_registers(regs);
        panic("\nKernel page fault at vaddr 0x%p\n", (void *)vaddr);
    }
    return 0;
}

void pt_init()
{
    static long inited = 0;
    if (!inited)
    {
        inited = 1;
        // allocate a page to set up the new page table structure
        // important caveat: we have not mapped in the physmap region, which
        // is where the addresses from page_alloc come, so we use the actual
        // physical addrses of the page, which we request to be in the
        // first 4MB of RAM, as they are identity-mapped by the boot-time
        // page tables
        uintptr_t max_paddr = (1UL << 22);
        pml4_t *pml4 = page_alloc_bounded((void *)max_paddr);
        if (!pml4)
            panic("ran out of memory in pt_init");
        pml4 = (pml4_t *)((uintptr_t)pml4 - PHYS_OFFSET);
        KASSERT((uintptr_t)pml4 < max_paddr);
        memset(pml4, 0, PAGE_SIZE);

        // map the kernel in to it's expected virtual memory address
        if (_fill_pml4(pml4, KERNEL_PHYS_BASE, KERNEL_VMA + KERNEL_PHYS_BASE,
                       KERNEL_VMA + KERNEL_PHYS_END, max_paddr))
            panic("ran out of memory in pt_init");

        // map in physmap
        if (_fill_pml4(pml4, 0, (uintptr_t)physmap_start(),
                       (uintptr_t)physmap_end(), max_paddr))
            panic("ran out of memory in pt_init");

        page_init_finish();

        // use the kernel memory address synonym instead of the physical address
        // identity map for pml4 make the MMU use the new pml4
        pt_set((pml4_t *)((uintptr_t)pml4 + PHYS_OFFSET));
        global_kernel_only_pml4 = (pml4_t *)((uintptr_t)pml4 + PHYS_OFFSET);
        // pt_unmap_range(global_kernel_only_pml4, USER_MEM_LOW, USER_MEM_HIGH);
        intr_register(INTR_PAGE_FAULT, _pt_fault_handler);
    }
    pt_set(global_kernel_only_pml4);
}

pt_t *clone_pt(pt_t *pt)
{
    pt_t *clone = page_alloc();
    dbg(DBG_PRINT, "cloning pt at 0x%p to 0x%p\n", pt, clone);
    if (clone)
    {
        memcpy(clone, pt, PAGE_SIZE);
    }
    return clone;
}

pd_t *clone_pd(pd_t *pd)
{
    pd_t *clone = page_alloc();
    dbg(DBG_PRINT, "cloning pd at 0x%p to 0x%p\n", pd, clone);
    if (!clone)
    {
        return NULL;
    }
    memset(clone, 0, PAGE_SIZE); // in case the clone fails, need to know what
                                 // we have allocated
    for (unsigned i = 0; i < PT_ENTRY_COUNT; i++)
    {
        // dbg(DBG_PRINT, "checking pd i = %u\n", i);
        if (pd->phys[i])
        {
            if (IS_2MB_PAGE(pd->phys[i]))
            {
                clone->phys[i] = pd->phys[i];
                continue;
            }
            pt_t *cloned_pt =
                clone_pt((pt_t *)((pd->phys[i] & PAGE_MASK) + PHYS_OFFSET));
            if (!cloned_pt)
            {
                return NULL;
            }
            clone->phys[i] = (((uintptr_t)cloned_pt) - PHYS_OFFSET) |
                             PAGE_FLAGS(pd->phys[i]);
        }
        else
        {
            clone->phys[i] = 0;
        }
    }
    return clone;
}

pdp_t *clone_pdp(pdp_t *pdp)
{
    pdp_t *clone = page_alloc();
    dbg(DBG_PRINT, "cloning pdp at 0x%p to 0x%p\n", pdp, clone);
    if (!clone)
    {
        return NULL;
    }
    memset(clone, 0, PAGE_SIZE); // in case the clone fails, need to know what
                                 // we have allocated
    for (unsigned i = 0; i < PT_ENTRY_COUNT; i++)
    {
        // dbg(DBG_PRINT, "checking pdp i = %u\n", i);
        if (pdp->phys[i])
        {
            if (IS_1GB_PAGE(pdp->phys[i]))
            {
                clone->phys[i] = pdp->phys[i];
                continue;
            }
            pd_t *cloned_pd =
                clone_pd((pd_t *)((pdp->phys[i] & PAGE_MASK) + PHYS_OFFSET));
            if (!cloned_pd)
            {
                return NULL;
            }
            clone->phys[i] = (((uintptr_t)cloned_pd) - PHYS_OFFSET) |
                             PAGE_FLAGS(pdp->phys[i]);
        }
        else
        {
            clone->phys[i] = 0;
        }
    }
    return clone;
}

pml4_t *clone_pml4(pml4_t *pml4, long include_user_mappings)
{
    pml4_t *clone = page_alloc();
    dbg(DBG_PRINT, "cloning pml4 at 0x%p to 0x%p\n", pml4, clone);
    if (!clone)
    {
        return NULL;
    }
    memset(clone, 0, PAGE_SIZE); // in case the clone fails, need to know what
                                 // we have allocated
    for (uintptr_t i = include_user_mappings ? 0 : PT_ENTRY_COUNT / 2;
         i < PT_ENTRY_COUNT; i++)
    {
        // dbg(DBG_PRINT, "checking pml4 i = %u\n", i);
        if (pml4->phys[i])
        {
            pdp_t *cloned_pdp =
                clone_pdp((pdp_t *)((pml4->phys[i] & PAGE_MASK) + PHYS_OFFSET));
            if (!cloned_pdp)
            {
                pt_destroy(clone);
                return NULL;
            }
            clone->phys[i] = (((uintptr_t)cloned_pdp) - PHYS_OFFSET) |
                             PAGE_FLAGS(pml4->phys[i]);
        }
        else
        {
            clone->phys[i] = 0;
        }
    }
    return clone;
}

pml4_t *pt_create() { return clone_pml4(pt_get(), 0); }

void pt_destroy_helper(pt_t *pt, long depth)
{
    // 4 = pml4, 3 = pdp, 2 = pd, 1 = pt
    if (depth != 1)
    {
        for (uintptr_t i = 0; i < PT_ENTRY_COUNT; i++)
        {
            if (!pt->phys[i] || (PT_SIZE & pt->phys[i]))
            {
                continue;
            }
            KASSERT(IS_PRESENT(pt->phys[i]) && (pt->phys[i] & PAGE_MASK));
            pt_destroy_helper((pt_t *)((pt->phys[i] & PAGE_MASK) + PHYS_OFFSET),
                              depth - 1);
            pt->phys[i] = 0;
        }
    }
    page_free(pt);
}

void pt_destroy(pml4_t *pml4) { pt_destroy_helper(pml4, 4); }

void pt_unmap(pml4_t *pml4, uintptr_t vaddr)
{
    pt_unmap_range(pml4, vaddr, vaddr + PAGE_SIZE);
}

void pt_unmap_range(pml4_t *pml4, uintptr_t vaddr, uintptr_t vmax)
{
    // TODO reclaim pages on-the-fly?

    dbg(DBG_PGTBL, "virt[0x%p, 0x%p); pml4: 0x%p\n", (void *)vaddr,
        (void *)vmax, pml4);
    KASSERT(PAGE_ALIGNED(vaddr) && PAGE_ALIGNED(vmax) && vmax > vaddr);

    uintptr_t vaddr_start = vaddr;

    while (vaddr < vmax)
    {
        uint64_t size = vmax - vaddr;

        uint64_t idx = PML4E(vaddr);
        pml4_t *table = pml4;

        if (!IS_PRESENT(table->phys[idx]))
        {
            vaddr = PAGE_ALIGN_UP_512GB(vaddr + 1);
            continue;
        }
        table = (pdp_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

        // PDP (1GB pages)
        idx = PDPE(vaddr);
        if (!IS_PRESENT(table->phys[idx]))
        {
            vaddr = PAGE_ALIGN_UP_1GB(vaddr + 1);
            ;
            continue;
        }
        if (IS_1GB_PAGE(table->phys[idx]))
        {
            if (PAGE_ALIGNED_1GB(vaddr) && size >= PAGE_SIZE_1GB)
            {
                table->phys[idx] = 0;
                vaddr += PAGE_SIZE_1GB;
            }
            else
            {
                pd_t *pd = page_alloc();
                if (!pd)
                {
                    panic(
                        "Ran out of memory during pt_unmap_range; recovery "
                        "from this situation has not yet been implemented!");
                }
                uint64_t unmap_start = PDE(vaddr);
                uint64_t unmap_end =
                    PAGE_SAME_1GB(vaddr, vmax) ? PDE(vmax) : 512;
                for (unsigned i = 0; i < unmap_start; i++)
                {
                    pd->phys[i] = table->phys[idx] +
                                  i * PAGE_SIZE_2MB; // keeps all flags,
                                                     // including PT_SIZE
                }
                memset(&pd->phys[unmap_start], 0,
                       sizeof(uint64_t) * (unmap_end - unmap_start));
                vaddr += (unmap_end - unmap_start) * PAGE_SIZE_2MB;
                for (uintptr_t i = unmap_end; unmap_end < PT_ENTRY_COUNT; i++)
                {
                    pd->phys[i] = table->phys[idx] +
                                  i * PAGE_SIZE_2MB; // keeps all flags,
                                                     // including PT_SIZE
                }
                table->phys[idx] = ((uintptr_t)pd - PHYS_OFFSET) |
                                   PAGE_CONTROL_FLAGS(table->phys[idx]);
            }
            continue;
        }
        table = (pd_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

        // PD (2MB pages)
        idx = PDE(vaddr);
        if (!IS_PRESENT(table->phys[idx]))
        {
            vaddr = PAGE_ALIGN_UP_2MB(vaddr + 1);
            continue;
        }
        if (IS_2MB_PAGE(table->phys[idx]))
        {
            if (PAGE_ALIGNED_2MB(vaddr) && size >= PAGE_SIZE_2MB)
            {
                table->phys[idx] = 0;
                vaddr += PAGE_SIZE_2MB;
            }
            else
            {
                pt_t *pt = page_alloc();
                if (!pt)
                {
                    panic(
                        "Ran out of memory during pt_unmap_range; recovery "
                        "from this situation has not yet been implemented!");
                }
                uint64_t unmap_start = PTE(vaddr);
                uint64_t unmap_end =
                    PAGE_SAME_2MB(vaddr, vmax) ? PTE(vmax) : 512;
                for (unsigned i = 0; i < unmap_start; i++)
                {
                    pt->phys[i] = table->phys[idx] + i * PAGE_SIZE -
                                  PT_SIZE; // remove PT_SIZE flag
                }
                memset(&pt->phys[unmap_start], 0,
                       sizeof(uint64_t) * (unmap_end - unmap_start));
                vaddr += (unmap_end - unmap_start) * PAGE_SIZE;
                for (uintptr_t i = unmap_end; unmap_end < PT_ENTRY_COUNT; i++)
                {
                    pt->phys[i] = table->phys[idx] + i * PAGE_SIZE -
                                  PT_SIZE; // remove PT_SIZE flag
                }
                table->phys[idx] =
                    ((uintptr_t)pt - PHYS_OFFSET) |
                    (PAGE_CONTROL_FLAGS(table->phys[idx]) - PT_SIZE);
            }
            continue;
        }
        table = (pt_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

        // PT (4KB pages)
        idx = PTE(vaddr);
        if (!IS_PRESENT(table->phys[idx]))
        {
            vaddr += PAGE_SIZE;
            continue;
        }
        table->phys[idx] = 0;

        vaddr += PAGE_SIZE;
    }
    KASSERT(_vaddr_status(pml4, vaddr_start) == UNMAPPED);
}

static char *entry_strings[] = {
    "4KB",
    "2MB",
    "1GB",
    "512GB",
};

inline long _vaddr_status_detailed(pml4_t *pml4, uintptr_t vaddr)
{
    uintptr_t idx;
    pml4_t *table = pml4;

    idx = PML4E(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return -4;
    }
    table = (pdp_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PDP (1GB pages)
    idx = PDPE(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return -3;
    }
    if (IS_1GB_PAGE(table->phys[idx]))
    {
        return 3;
    }
    table = (pd_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PD (2MB pages)
    idx = PDE(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return -2;
    }
    if (IS_2MB_PAGE(table->phys[idx]))
    {
        return 2;
    }
    table = (pt_t *)((table->phys[idx] & PAGE_MASK) + PHYS_OFFSET);

    // PT (4KB pages)
    idx = PTE(vaddr);
    if (!IS_PRESENT(table->phys[idx]))
    {
        return -1;
    }
    return 1;
}

void check_invalid_mappings(pml4_t *pml4, vmmap_t *vmmap, char *prompt)
{
    // checks that anything that is mapped in pml4 actually should be according
    // to vmmap

    uintptr_t vaddr = USER_MEM_LOW;
    while (vaddr < USER_MEM_HIGH)
    {
        long state = _vaddr_status_detailed(pml4, vaddr);
        if (state > 0)
        {
            uintptr_t paddr = pt_virt_to_phys_helper(pml4, vaddr);

            vmarea_t *vma = vmmap_lookup(vmmap, ADDR_TO_PN(vaddr));
            if (!vma)
            {
                dbg(DBG_PGTBL,
                    "[+] %s: pml4 0x%p, 0x%p (paddr: 0x%p) cannot be found in "
                    "vmmap!\n",
                    prompt, pml4, (void *)vaddr, (void *)paddr);
                pt_unmap(pml4, vaddr);
            }
            else
            {
                pframe_t *pf = NULL;
                uintptr_t pagenum =
                    vma->vma_off + (ADDR_TO_PN(vaddr) - vma->vma_start);

                mobj_lock(vma->vma_obj);
                long ret = mobj_get_pframe(vma->vma_obj, pagenum, 0, &pf);
                mobj_unlock(vma->vma_obj);
                if (ret)
                {
                    dbg(DBG_PGTBL,
                        "[+] %s: pml4 0x%p, the page frame for virtual address "
                        "0x%p (mapping to 0x%p) could not be found!\n",
                        prompt, pml4, (void *)vaddr, (void *)paddr);
                    pt_unmap(pml4, vaddr);
                }
                else
                {
                    uintptr_t pf_paddr =
                        pt_virt_to_phys_helper(pml4, (uintptr_t)pf->pf_addr);
                    if (pf_paddr != paddr)
                    {
                        dbg(DBG_PGTBL,
                            "[+] %s: pml4 0x%p, 0x%p (paddr: 0x%p) supposed to "
                            "be 0x%p (obj: 0x%p, %lu)\n",
                            prompt, pml4, (void *)vaddr, (void *)paddr,
                            (void *)pf_paddr, vma->vma_obj, pf->pf_pagenum);
                        pt_unmap(pml4, vaddr);
                    }
                }
            }
        }
        switch (state)
        {
        case 1:
        case -1:
            vaddr = (uintptr_t)PAGE_ALIGN_UP(vaddr + 1);
            break;
        case -2:
            vaddr = (uintptr_t)PAGE_ALIGN_UP_2MB(vaddr + 1);
            break;
        case -3:
            vaddr = (uintptr_t)PAGE_ALIGN_UP_1GB(vaddr + 1);
            break;
        case -4:
            vaddr = (uintptr_t)PAGE_ALIGN_UP_512GB(vaddr + 1);
            break;
        case 2:
        case 3:
        default:
            panic("should not get here!");
        }
    }
}
