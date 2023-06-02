#pragma once

#include "types.h"

#include "util/list.h"

#define VMMAP_DIR_LOHI 1
#define VMMAP_DIR_HILO 2

struct mobj;
struct proc;
struct vnode;

typedef struct vmmap
{
    list_t vmm_list;       /* list of virtual memory areas */
    struct proc *vmm_proc; /* the process that corresponds to this vmmap */
} vmmap_t;

/* Make sure you understand why mapping boundaries are in terms of frame
 * numbers (page numbers) and not addresses */
typedef struct vmarea
{
    size_t vma_start; /* [starting vfn, */
    size_t vma_end;   /*  ending vfn) */
    size_t vma_off;   /* offset from beginning of vma_obj in pages */
                      /* the reason this field is necessary is that 
                         when files are mmap'ed, it doesn't have 
                         to start from location 0. You could, for instance, 
                         map pages 10-15 of a file, and vma_off would be 10. */

    int vma_prot;  /* permissions (protections) on mapping, see mman.h */
    int vma_flags; /* either MAP_SHARED or MAP_PRIVATE. It can also specify 
                      MAP_ANON and MAP_FIXED */

    struct vmmap *vma_vmmap; /* address space that this area belongs to */
    struct mobj *vma_obj;    /* the memory object that corresponds to this address region */
    list_link_t vma_plink;   /* link on process vmmap maps list */
} vmarea_t;

void vmmap_init(void);

vmmap_t *vmmap_create(void);

void vmmap_destroy(vmmap_t **mapp);

void vmmap_collapse(vmmap_t *map);

vmarea_t *vmmap_lookup(vmmap_t *map, size_t vfn);

long vmmap_map(vmmap_t *map, struct vnode *file, size_t lopage, size_t npages,
               int prot, int flags, off_t off, int dir, vmarea_t **new_vma);

long vmmap_remove(vmmap_t *map, size_t lopage, size_t npages);

long vmmap_is_range_empty(vmmap_t *map, size_t startvfn, size_t npages);

void vmmap_insert(vmmap_t *map, vmarea_t *new_vma); 

ssize_t vmmap_find_range(vmmap_t *map, size_t npages, int dir);

long vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count);

long vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count);

vmmap_t *vmmap_clone(vmmap_t *map);

size_t vmmap_mapping_info_helper(const void *map, char *buf, size_t size,
                                 char *prompt);

size_t vmmap_mapping_info(const void *map, char *buf, size_t size);