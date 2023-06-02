#pragma once

#include "mm/page.h"
#include "vm/vmmap.h"

#define PT_PRESENT 0x001
#define PT_WRITE 0x002
#define PT_USER 0x004
#define PT_WRITE_THROUGH 0x008
#define PT_CACHE_DISABLED 0x010
#define PT_ACCESSED 0x020
#define PT_DIRTY 0x040
#define PT_SIZE 0x080
#define PT_GLOBAL 0x100

#define PT_ENTRY_COUNT (PAGE_SIZE / sizeof(uintptr_t))

typedef struct page
{
    uint8_t data[PAGE_SIZE];
} page_t;

// Generalized structure for all directory like entries
typedef struct pt
{
    uintptr_t phys[PT_ENTRY_COUNT];
} pt_t, pd_t, pdp_t, pml4_t;

#define INDEX_MASK 0b111111111
#define PML4E(x) ((((uintptr_t)(x)) >> 39) & INDEX_MASK)
#define PDPE(x) ((((uintptr_t)(x)) >> 30) & INDEX_MASK)
#define PDE(x) ((((uintptr_t)(x)) >> 21) & INDEX_MASK)
#define PTE(x) ((((uintptr_t)(x)) >> 12) & INDEX_MASK)

#define PT_ENTRY_COUNT (PAGE_SIZE / sizeof(uintptr_t))
#define PT_VADDR_SIZE (PAGE_SIZE * PT_ENTRY_COUNT)
#define PD_VADDR_SIZE (PAGE_SIZE * PT_ENTRY_COUNT * PT_ENTRY_COUNT)
#define PDP_VADDR_SIZE \
    (PAGE_SIZE * PT_ENTRY_COUNT * PT_ENTRY_COUNT * PT_ENTRY_COUNT)
#define PML4_VADDR_SIZE                                             \
    (PAGE_SIZE * PT_ENTRY_COUNT * PT_ENTRY_COUNT * PT_ENTRY_COUNT * \
     PT_ENTRY_COUNT)

#define IS_PRESENT(n) ((n)&PT_PRESENT)
#define IS_2MB_PAGE(n) ((n)&PT_SIZE)
#define IS_1GB_PAGE IS_2MB_PAGE

#define GDB_PT_PHYSADDR(pt, v) (pt->phys[PTE(v)] & PAGE_MASK)
#define GDB_PD_PHYSADDR(pd, v) (pd->phys[PDE(v)] & PAGE_MASK)
#define GDB_PDP_PHYSADDR(pdp, v) (pdp->phys[PDPE(v)] & PAGE_MASK)
#define GDB_PML4_PHYSADDR(pml4, v) (pml4->phys[PML4E(v)] & PAGE_MASK)

#define GDB_PHYSADDR(pml4, v)                                             \
    (GDB_PT_PHYSADDR(                                                     \
         GDB_PD_PHYSADDR(                                                 \
             GDB_PDP_PHYSADDR(GDB_PML4_PHYSADDR(pml4, (v)) + PHYS_OFFSET, \
                              (v)) +                                      \
                 PHYS_OFFSET,                                             \
             (v)) +                                                       \
             PHYS_OFFSET,                                                 \
         (v)) +                                                           \
     PHYS_OFFSET)
#define GDB_CUR_PHYSADDR(v) GDB_PHYSADDR(curproc->p_pml4, (v))

uintptr_t pt_virt_to_phys_helper(pml4_t *pml4, uintptr_t vaddr);

uintptr_t pt_virt_to_phys(uintptr_t vaddr);

void pt_init(void);

/* Currently unused. */
void pt_template_init(void);

pml4_t *pt_get();

void pt_set(pml4_t *pml4);

pml4_t *clone_pml4(pml4_t *pml4, long include_user_mappings);

pml4_t *pt_create();

void pt_destroy(pml4_t *pml4);

long pt_map(pml4_t *pml4, uintptr_t paddr, uintptr_t vaddr, uint32_t pdflags,
            uint32_t ptflags);

long pt_map_range(pml4_t *pml4, uintptr_t paddr, uintptr_t vaddr,
                  uintptr_t vmax, uint32_t pdflags, uint32_t ptflags);

void pt_unmap(pml4_t *pml4, uintptr_t vaddr);

void pt_unmap_range(pml4_t *pml4, uintptr_t vaddr, uintptr_t vmax);

void check_invalid_mappings(pml4_t *pml4, vmmap_t *vmmap, char *prompt);
