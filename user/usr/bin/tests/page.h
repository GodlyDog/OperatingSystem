#pragma once

#ifdef __KERNEL__
#include "types.h"
#else
#include "sys/types.h"
#endif

/* This header file contains the functions for allocating
 * and freeing page-aligned chunks of data which are a
 * multiple of a page in size. These are the lowest level
 * memory allocation functions. In general code should
 * use the slab allocator functions in mm/slab.h unless
 * they require page-aligned buffers. */

#define PAGE_SHIFT 12
#define PAGE_SIZE ((uintptr_t)(1UL << PAGE_SHIFT))
#define PAGE_MASK (0xffffffffffffffff << PAGE_SHIFT)

#define PAGE_ALIGN_DOWN(x) ((void *)(((uintptr_t)(x)&PAGE_MASK)))
#define PAGE_ALIGN_UP(x) \
    ((void *)((((uintptr_t)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)))

#define PAGE_OFFSET(x) (((uintptr_t)(x)) & ~PAGE_MASK)
#define PAGE_ALIGNED(x) (!PAGE_OFFSET(x))

#define PN_TO_ADDR(x) ((void *)(((uintptr_t)(x)) << PAGE_SHIFT))
#define ADDR_TO_PN(x) (((uintptr_t)(x)) >> PAGE_SHIFT)

#define PAGE_SAME(x, y) (PAGE_ALIGN_DOWN(x) == PAGE_ALIGN_DOWN(y))

#define PAGE_NSIZES 8

#define USE_2MB_PAGES 1
#define USE_1GB_PAGES 1

#define PAGE_SHIFT_2MB 21
#define PAGE_SIZE_2MB ((uintptr_t)(1UL << PAGE_SHIFT_2MB))
#define PAGE_MASK_2MB (0xffffffffffffffff << PAGE_SHIFT_2MB)
#define PAGE_ALIGN_DOWN_2MB(x) (((uintptr_t)(x)) & PAGE_MASK_2MB)
#define PAGE_ALIGN_UP_2MB(x) (PAGE_ALIGN_DOWN_2MB((x)-1) + PAGE_SIZE_2MB)
#define PAGE_OFFSET_2MB(x) (((uintptr_t)(x)) & ~PAGE_MASK_2MB)
#define PAGE_ALIGNED_2MB(x) ((x) == PAGE_ALIGN_DOWN_2MB(x))
#define PAGE_SAME_2MB(x, y) (PAGE_ALIGN_DOWN_2MB(x) == PAGE_ALIGN_DOWN_2MB(y))

#define PAGE_SHIFT_1GB 30
#define PAGE_MASK_1GB (0xffffffffffffffff << PAGE_SHIFT_1GB)
#define PAGE_SIZE_1GB ((uintptr_t)(1UL << PAGE_SHIFT_1GB))
#define PAGE_ALIGN_DOWN_1GB(x) (((uintptr_t)(x)) & PAGE_MASK_1GB)
#define PAGE_ALIGN_UP_1GB(x) (PAGE_ALIGN_DOWN_1GB((x)-1) + PAGE_SIZE_1GB)
#define PAGE_OFFSET_1GB(x) (((uintptr_t)(x)) & ~PAGE_MASK_1GB)
#define PAGE_ALIGNED_1GB(x) ((x) == PAGE_ALIGN_DOWN_1GB(x))
#define PAGE_SAME_1GB(x, y) (PAGE_ALIGN_DOWN_1GB(x) == PAGE_ALIGN_DOWN_1GB(y))

#define PAGE_SHIFT_512GB 39
#define PAGE_SIZE_512GB ((uintptr_t)(1UL << PAGE_SHIFT_512GB))
#define PAGE_MASK_512GB (0xffffffffffffffff << PAGE_SHIFT_512GB)
#define PAGE_ALIGN_DOWN_512GB(x) (((uintptr_t)(x)) & PAGE_MASK_512GB)
#define PAGE_ALIGN_UP_512GB(x) (PAGE_ALIGN_DOWN_512GB((x)-1) + PAGE_SIZE_512GB)

#define PAGE_CONTROL_FLAGS(x)                                    \
    ((x) & (PT_PRESENT | PT_WRITE | PT_USER | PT_WRITE_THROUGH | \
            PT_CACHE_DISABLED | PT_SIZE | PT_GLOBAL))
#define PAGE_FLAGS(x) ((x) & (~PAGE_MASK))

typedef enum page_size
{
    ps_4kb,
    ps_2mb,
    ps_1gb,
    ps_512gb,
} page_size_t;

typedef struct page_status
{
    page_size_t size;
    int mapped;
} page_status_t;

/* Performs all initialization necessary for the
 * page allocation system. This should be called
 * only once at boot time before any other functions
 * in this header are called. */
void page_init();

void *physmap_start();

void *physmap_end();

/* These functions allocate and free one page-aligned,
 * page-sized block of memory. Values passed to
 * page_free MUST have been returned by page_alloc
 * at some previous point. There should be only one
 * call to page_free for each value returned by
 * page_alloc. If the system is out of memory page_alloc
 * will return NULL. */
void *page_alloc(void);

void *page_alloc_bounded(void *max_paddr);

void page_free(void *addr);

/* These functions allocate and free a page-aligned
 * block of memory which are npages pages in length.
 * A call to page_alloc_n will allocate a block, to free
 * that block a call should be made to page_free_n with
 * npages set to the same as it was when the block was
 * allocated */
void *page_alloc_n(size_t npages);

void *page_alloc_n_bounded(size_t npages, void *max_paddr);

void page_free_n(void *start, size_t npages);

void page_add_range(void *start, void *end);

void page_mark_reserved(void *paddr);

void page_init_finish();

/* Returns the number of free pages remaining in the
 * system. Note that calls to page_alloc_n(npages) may
 * fail even if page_free_count() >= npages. */
size_t page_free_count();
