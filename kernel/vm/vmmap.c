#include "globals.h"
#include "kernel.h"
#include <errno.h>

#include "vm/anon.h"
#include "vm/shadow.h"

#include "util/debug.h"
#include "util/printf.h"
#include "util/string.h"

#include "fs/file.h"
#include "fs/vfs_syscall.h"
#include "fs/vnode.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/slab.h"
#include "mm/tlb.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void vmmap_init(void)
{
    vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
    vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
    KASSERT(vmmap_allocator && vmarea_allocator);
}

/*
 * Allocate and initialize a new vmarea using vmarea_allocator.
 */
vmarea_t *vmarea_alloc(void)
{
    vmarea_t* area = slab_obj_alloc(vmarea_allocator);
    if (!area) {
        return NULL;
    }
    area->vma_start = 0;
    area->vma_end = 0;
    area->vma_off = 0;
    area->vma_prot = 0;
    area->vma_flags = 0;
    area->vma_vmmap = NULL;
    area->vma_obj = NULL;
    list_link_init(&area->vma_plink);
    return area;
}

/*
 * Free the vmarea by removing it from any lists it may be on, putting its
 * vma_obj if it exists, and freeing the vmarea_t.
 */
void vmarea_free(vmarea_t *vma)
{
    if (list_link_is_linked(&vma->vma_plink)) {
        list_remove(&vma->vma_plink);
    }
    if (vma->vma_obj) {
        KASSERT(vma->vma_obj->mo_refcount);
        mobj_put(&vma->vma_obj);
    }
    slab_obj_free(vmarea_allocator, vma);
}

/*
 * Create and initialize a new vmmap. Initialize all the fields of vmmap_t.
 */
vmmap_t *vmmap_create(void)
{
    vmmap_t* vmmap = slab_obj_alloc(vmmap_allocator);
    if (!vmmap) {
        return NULL;
    }
    list_init(&vmmap->vmm_list);
    vmmap->vmm_proc = curproc;
    return vmmap;
}

/*
 * Destroy the map pointed to by mapp and set *mapp = NULL.
 * Remember to free each vma in the maps list.
 */
void vmmap_destroy(vmmap_t **mapp)
{
    list_iterate(&(*mapp)->vmm_list, area, vmarea_t, vma_plink) {
        list_remove(&area->vma_plink);
        vmarea_free(area);
    }
    slab_obj_free(vmmap_allocator, *mapp);
    *mapp = NULL;
}

/*
 * Add a vmarea to an address space. Assumes (i.e. asserts to some extent) the
 * vmarea is valid. Iterate through the list of vmareas, and add it 
 * accordingly. 
 */
void vmmap_insert(vmmap_t *map, vmarea_t *new_vma)
{
    list_iterate(&map->vmm_list, area, vmarea_t, vma_plink) {
        if (area->vma_end >= new_vma->vma_start) {
            list_insert_before(&area->vma_plink, &new_vma->vma_plink);
            return;
        }
    }
    list_insert_tail(&map->vmm_list, &new_vma->vma_plink);
}

/*
 * Find a contiguous range of free virtual pages of length npages in the given
 * address space. Returns starting page number for the range, without altering the map.
 * Return -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is
 *    - VMMAP_DIR_HILO: a gap as high in the address space as possible, starting 
 *                      from USER_MEM_HIGH.  
 *    - VMMAP_DIR_LOHI: a gap as low in the address space as possible, starting 
 *                      from USER_MEM_LOW. 
 * 
 * Make sure you are converting between page numbers and addresses correctly! 
 */
ssize_t vmmap_find_range(vmmap_t *map, size_t npages, int dir)
{
    KASSERT(dir == VMMAP_DIR_HILO || dir == VMMAP_DIR_LOHI);
    size_t user_start_page = ADDR_TO_PN(USER_MEM_LOW);
    size_t user_end_page = ADDR_TO_PN(USER_MEM_HIGH);
    if (dir == VMMAP_DIR_LOHI) {
        size_t count = 0;
        size_t start = 0;
        for (size_t i = user_start_page; i < user_end_page; i++) {
            vmarea_t* lookup = vmmap_lookup(map, i);
            if (!lookup) {
                if (count == 0) {
                    start = i;
                }
                count += 1;
            } else {
                count = 0;
                start = 0;
            }
            if (count == npages) {
                KASSERT(start >= user_start_page);
                return start;
            }
        }
    }
    if (dir == VMMAP_DIR_HILO) {
        size_t count = 0;
        for (size_t i = user_end_page - 1; i >= user_start_page; i--) {
            vmarea_t* lookup = vmmap_lookup(map, i);
            if (!lookup) {
                count += 1;
            } else {
                count = 0;
            }
            if (count == npages) {
                KASSERT(i >= user_start_page);
                return i;
            }
        }
    }
    return -1;
}

/*
 * Return the vm_area that vfn (a page number) lies in. Scan the address space looking
 * for a vma whose range covers vfn. If the page is unmapped, return NULL.
 */
vmarea_t *vmmap_lookup(vmmap_t *map, size_t vfn)
{
    list_iterate(&map->vmm_list, area, vmarea_t, vma_plink) {
        if (area->vma_start <= vfn && area->vma_end > vfn) {
            return area;
        }
    }
    return NULL;
}

/*
 * For each vmarea in the map, if it is a shadow object, call shadow_collapse.
 */
void vmmap_collapse(vmmap_t *map)
{
    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink)
    {
        if (vma->vma_obj->mo_type == MOBJ_SHADOW)
        {
            mobj_lock(vma->vma_obj);
            shadow_collapse(vma->vma_obj);
            mobj_unlock(vma->vma_obj);
        }
    }
}

/*
 * This is where the magic of fork's copy-on-write gets set up. 
 * 
 * Upon successful return, the new vmmap should be a clone of map with all 
 * shadow objects properly set up.
 *
 * For each vmarea, clone its members. 
 *  1) vmarea is share-mapped, you don't need to do anything special. 
 *  2) vmarea is not share-mapped, time for shadow objects: 
 *     a) Create two shadow objects, one for map and one for the new vmmap you
 *        are constructing, both of which shadow the current vma_obj the vmarea
 *        being cloned. 
 *     b) After creating the shadow objects, put the original vma_obj
 *     c) and insert the shadow objects into their respective vma's.
 *
 * Be sure to clean up in any error case, manage the reference counts correctly,
 * and to lock/unlock properly.
 */
vmmap_t *vmmap_clone(vmmap_t *map)
{
    vmmap_t* new_map = vmmap_create();
    if (!new_map) {
        return NULL;
    }
    list_iterate(&map->vmm_list, area, vmarea_t, vma_plink) {
        int old_refcount = area->vma_obj->mo_refcount;
        vmarea_t* new_area = vmarea_alloc();
        if (!new_area) {
            vmmap_destroy(&new_map);
            return NULL;
        }
        new_area->vma_end = area->vma_end;
        new_area->vma_start = area->vma_start;
        new_area->vma_off = area->vma_off;
        new_area->vma_flags = area->vma_flags;
        new_area->vma_prot = area->vma_prot;
        if (!(area->vma_flags & MAP_SHARED)) {
            mobj_lock(area->vma_obj);
            mobj_t* new_shadow = shadow_create(area->vma_obj);
            mobj_unlock(area->vma_obj);
            mobj_unlock(new_shadow);
            if (!new_shadow) {
                vmmap_destroy(&new_map);
                return NULL;
            }
            mobj_t* old_shadow = shadow_create(area->vma_obj);
            mobj_unlock(old_shadow);
            if (!old_shadow) {
                vmmap_destroy(&new_map);
                return NULL;
            }
            new_area->vma_obj = new_shadow;
            KASSERT(area->vma_obj->mo_refcount);
            mobj_put(&area->vma_obj);
            area->vma_obj = old_shadow;
        } else {
            new_area->vma_obj = area->vma_obj;
            mobj_ref(area->vma_obj);
        }
        vmmap_insert(new_map, new_area);
    }
    return new_map;
}

/*
 *
 * Insert a mapping into the map starting at lopage for npages pages.
 * 
 *  file    - If provided, the vnode of the file to be mapped in
 *  lopage  - If provided, the desired start range of the mapping
 *  prot    - See mman.h for possible values
 *  flags   - See do_mmap()'s comments for possible values
 *  off     - Offset in the file to start mapping at, in bytes
 *  dir     - VMMAP_DIR_LOHI or VMMAP_DIR_HILO
 *  new_vma - If provided, on success, must point to the new vmarea_t
 * 
 *  Return 0 on success, or:
 *  - ENOMEM: On vmarea_alloc, annon_create, shadow_create or 
 *    vmmap_find_range failure 
 *  - Propagate errors from file->vn_ops->mmap and vmmap_remove
 * 
 * Hints:
 *  - You can assume/assert that all input is valid. It may help to write
 *    this function and do_mmap() somewhat in tandem.
 *  - If file is NULL, create an anon object.
 *  - If file is non-NULL, use the vnode's mmap operation to get the mobj.
 *    Do not assume it is file->vn_obj (mostly relevant for special devices).
 *  - If lopage is 0, use vmmap_find_range() to get a valid range
 *  - If lopage is nonzero and MAP_FIXED is specified and 
 *    the given range overlaps with any preexisting mappings, 
 *    remove the preexisting mappings.
 *  - If MAP_PRIVATE is specified, set up a shadow object. Be careful with
 *    refcounts!
 *  - Be careful: off is in bytes (albeit should be page-aligned), but
 *    vma->vma_off is in pages.
 *  - Be careful with the order of operations. Hold off on any irreversible
 *    work until there is no more chance of failure.
 */
long vmmap_map(vmmap_t *map, vnode_t *file, size_t lopage, size_t npages,
               int prot, int flags, off_t off, int dir, vmarea_t **new_vma)
{
    // set up the new vmarea_t
    dbg(DBG_TEST, "\nENTERING VMMAP_MAP\n");
    size_t start = lopage;
    if (!lopage) {
        start = vmmap_find_range(map, npages, dir);
        if (!start) {
            return -ENOMEM;
        }
    }
    vmarea_t* new_area = vmarea_alloc();
    if (!new_area) {
        return -ENOMEM;
    }
    // set the fields of the new area
    new_area->vma_end = start + npages;
    new_area->vma_off = ADDR_TO_PN((char *) PN_TO_ADDR(start) + off) - start;
    new_area->vma_flags = flags;
    new_area->vma_prot = prot;
    new_area->vma_vmmap = map;
    new_area->vma_start = start;

    // get the mobj
    mobj_t* mobj = NULL;
    if (!file) {
        mobj = anon_create();
        mobj_unlock(mobj);
    } else {
        long status = file->vn_ops->mmap(file, &mobj);
        if (status < 0) {
            vmarea_free(new_area);
            dbg(DBG_TEST, "\nERRORING VMMAP_MAP\n");
            return status;
        }
    }
    if (!mobj) {
        vmarea_free(new_area);
        dbg(DBG_TEST, "\nERRORING VMMAP_MAP\n");
        return -ENOMEM;
    }

    new_area->vma_obj = mobj;
    
    // set up shadow object if needed
    mobj_t* shadow = NULL;
    if (flags & MAP_PRIVATE) {
        shadow = shadow_create(mobj);
        new_area->vma_obj = shadow;
        KASSERT(mobj->mo_refcount);
        mobj_put(&mobj);
        if (!shadow) {
            vmarea_free(new_area);
            dbg(DBG_TEST, "\nERRORING VMMAP_MAP\n");
            return -ENOMEM;
        }
        mobj_unlock(shadow);
    }

    // remove mappings in the specified range if MAP_FIXED is set
    if ((flags & MAP_FIXED) && (lopage)) {
        long status = vmmap_remove(map, lopage, npages);
        if (status < 0) {
            vmarea_free(new_area);
            if (shadow) {
                KASSERT(shadow->mo_refcount);
                mobj_put(&shadow);
            } else {
                KASSERT(mobj->mo_refcount);
                mobj_put(&mobj);
            }
            dbg(DBG_TEST, "\nFINISHING VMMAP_MAP\n");
            return status;
        }
    }
    vmmap_insert(map, new_area);
    if (new_vma) {
        *new_vma = new_area;
    }
    dbg(DBG_TEST, "\nFINISHING VMMAP_MAP\n");
    return 0;
}

/*
 * Iterate over the mapping's vmm_list and make sure that the specified range
 * is completely empty. You will have to handle the following cases:
 *
 * Key:     [             ] = existing vmarea_t
 *              *******     = region to be unmapped
 *
 * Case 1:  [   *******   ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. Be sure to increment the refcount of
 * the object associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 * 
 * Return 0 on success, or:
 *  - ENOMEM: Failed to allocate a new vmarea when splitting a vmarea (case 1).
 * 
 * Hints:
 *  - Whenever you shorten/remove any mappings, be sure to call pt_unmap_range()
 *    tlb_flush_range() to clean your pagetables and TLB.
 */
long vmmap_remove(vmmap_t *map, size_t lopage, size_t npages)
{
    KASSERT(npages > 0);
    size_t endpage = lopage + npages;
    uintptr_t vaddr = (uintptr_t) PN_TO_ADDR(lopage);
    uintptr_t vmax = (uintptr_t) PN_TO_ADDR(endpage);
    list_iterate(&map->vmm_list, area, vmarea_t, vma_plink) {
        if (area->vma_start >= lopage && area->vma_end > endpage && endpage > area->vma_start) {
            size_t old_start = area->vma_start;
            area->vma_start = endpage;
            area->vma_off += area->vma_start - old_start;
            KASSERT(vmax > vaddr);
            KASSERT(PAGE_ALIGNED(vaddr) && PAGE_ALIGNED(vmax));
            pt_unmap_range(map->vmm_proc->p_pml4, vaddr, vmax);
            tlb_flush_range(vaddr, vmax - vaddr);
        } else if (area->vma_start < lopage && area->vma_end > endpage) {
            vmarea_t* new_area = vmarea_alloc();
            if (!new_area) {
                return -ENOMEM;
            }
            new_area->vma_start = endpage;
            new_area->vma_end = area->vma_end;
            new_area->vma_obj = area->vma_obj;
            mobj_ref(new_area->vma_obj);
            new_area->vma_flags = area->vma_flags;
            new_area->vma_off = area->vma_off + (new_area->vma_start - area->vma_start);
            new_area->vma_prot = area->vma_prot;
            new_area->vma_vmmap = map;
            area->vma_end = lopage;
            vmmap_insert(map, new_area);
            KASSERT(vmax > vaddr);
            KASSERT(PAGE_ALIGNED(vaddr) && PAGE_ALIGNED(vmax));
            pt_unmap_range(map->vmm_proc->p_pml4, vaddr, vmax);
            tlb_flush_range(vaddr, vmax - vaddr);
        } else if (area->vma_start < lopage && area->vma_end > lopage && area->vma_end <= endpage) {
            size_t old_end = area->vma_end;
            area->vma_end = lopage;
            KASSERT(vmax > vaddr);
            KASSERT(PAGE_ALIGNED(vaddr) && PAGE_ALIGNED(vmax));
            pt_unmap_range(map->vmm_proc->p_pml4, vaddr, vmax);
            tlb_flush_range(vaddr, vmax - vaddr);
        } else if (area->vma_start >= lopage && area->vma_end <= endpage) {
            list_remove(&area->vma_plink);
            vmarea_free(area);
            KASSERT(vmax > vaddr);
            KASSERT(PAGE_ALIGNED(vaddr) && PAGE_ALIGNED(vmax));
            pt_unmap_range(map->vmm_proc->p_pml4, vaddr, vmax);
            tlb_flush_range(vaddr, vmax - vaddr);
        }
    }
    return 0;
}

/*
 * Returns 1 if the given address space has no mappings for the given range,
 * 0 otherwise.
 */
long vmmap_is_range_empty(vmmap_t *map, size_t startvfn, size_t npages)
{
    size_t endvfn = startvfn + npages;
    list_iterate(&map->vmm_list, area, vmarea_t, vma_plink) {
        if (area->vma_start < endvfn && area->vma_start >= startvfn) {
            return 0;
        }
        if (area->vma_end <= endvfn && area->vma_end > startvfn) {
            return 0;
        }
        if (area->vma_start <= startvfn && area->vma_end >= endvfn) {
            return 0;
        }
    }
    return 1;
}

/*
 * Read into 'buf' from the virtual address space of 'map'. Start at 'vaddr'
 * for size 'count'. 'vaddr' is not necessarily page-aligned. count is in bytes.
 * 
 * Hints:
 *  1) Find the vmareas that correspond to the region to read from.
 *  2) Find the pframes within those vmareas corresponding to the virtual 
 *     addresses you want to read.
 *  3) Read from those page frames and copy it into `buf`.
 *  4) You will not need to check the permissisons of the area.
 *  5) You may assume/assert that all areas exist.
 * 
 * Return 0 on success, -errno on error (propagate from the routines called).
 * This routine will be used within copy_from_user(). 
 */
long vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
    size_t start_page = ADDR_TO_PN(vaddr);
    size_t starting_offset = PAGE_OFFSET(vaddr);
    size_t end_page = ADDR_TO_PN((char *) vaddr + count);
    size_t bytes_read = 0;
    const char* position = vaddr;
    for (size_t i = start_page; i <= end_page; i++) {
        vmarea_t* area = vmmap_lookup(map, i);
        KASSERT(area != NULL);
        size_t to_read = MIN(PAGE_SIZE - PAGE_OFFSET(position), count - bytes_read);
        pframe_t* pframe;
        mobj_lock(area->vma_obj);
        long status = mobj_get_pframe(area->vma_obj, i - area->vma_start + area->vma_off, 0, &pframe);
        mobj_unlock(area->vma_obj);
        if (status < 0) {
            return status;
        }
        memcpy((char *) buf + bytes_read, (char *) pframe->pf_addr + PAGE_OFFSET(position), to_read);
        bytes_read += to_read;
        position = position + to_read;
        pframe_release(&pframe);
        if (bytes_read == count) {
            return 0;
        }
    }
    KASSERT(bytes_read == count);
    return 0;
}

/*
 * Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'.
 * 
 * Hints:
 *  1) Find the vmareas to write to.
 *  2) Find the correct pframes within those areas that contain the virtual addresses
 *     that you want to write data to.
 *  3) Write to the pframes, copying data from buf.
 *  4) You do not need check permissions of the areas you use.
 *  5) Assume/assert that all areas exist.
 *  6) Remember to dirty the pages that you write to. 
 * 
 * Returns 0 on success, -errno on error (propagate from the routines called).
 * This routine will be used within copy_to_user(). 
 */
long vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
    size_t start_page = ADDR_TO_PN(vaddr);
    size_t starting_offset = PAGE_OFFSET(vaddr);
    size_t end_page = ADDR_TO_PN((char *) vaddr + count);
    size_t bytes_written = 0;
    const char* position = vaddr;
    for (size_t i = start_page; i <= end_page; i++) {
        vmarea_t* area = vmmap_lookup(map, i);
        KASSERT(area != NULL);
        size_t to_write = MIN(PAGE_SIZE - PAGE_OFFSET(position), count - bytes_written);
        pframe_t* pframe;
        mobj_lock(area->vma_obj);
        long status = mobj_get_pframe(area->vma_obj, i - area->vma_start + area->vma_off, 1, &pframe);
        mobj_unlock(area->vma_obj);
        if (status < 0) {
            return status;
        }
        memcpy((char *) pframe->pf_addr + PAGE_OFFSET(position), (char *) buf + bytes_written, to_write);
        bytes_written += to_write;
        position = position + to_write;
        pframe->pf_dirty = 1;
        pframe_release(&pframe);
        if (bytes_written == count) {
            return 0;
        }
    }
    KASSERT(bytes_written == count);
    return 0;
}

size_t vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
    return vmmap_mapping_info_helper(vmmap, buf, osize, "");
}

size_t vmmap_mapping_info_helper(const void *vmmap, char *buf, size_t osize,
                                 char *prompt)
{
    KASSERT(0 < osize);
    KASSERT(NULL != buf);
    KASSERT(NULL != vmmap);

    vmmap_t *map = (vmmap_t *)vmmap;
    ssize_t size = (ssize_t)osize;

    int len =
        snprintf(buf, (size_t)size, "%s%37s %5s %7s %18s %11s %23s\n", prompt,
                 "VADDR RANGE", "PROT", "FLAGS", "MOBJ", "OFFSET", "VFN RANGE");

    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink)
    {
        size -= len;
        buf += len;
        if (0 >= size)
        {
            goto end;
        }

        len =
            snprintf(buf, (size_t)size,
                     "%s0x%p-0x%p  %c%c%c  %7s 0x%p %#.9lx %#.9lx-%#.9lx\n",
                     prompt, (void *)(vma->vma_start << PAGE_SHIFT),
                     (void *)(vma->vma_end << PAGE_SHIFT),
                     (vma->vma_prot & PROT_READ ? 'r' : '-'),
                     (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                     (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                     (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                     vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
    }

end:
    if (size <= 0)
    {
        size = osize;
        buf[osize - 1] = '\0';
    }
    return osize - size;
}
