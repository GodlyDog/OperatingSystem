#include "errno.h"
#include "globals.h"

#include "test/usertest.h"
#include "test/proctest.h"

#include "util/debug.h"
#include "util/printf.h"
#include "util/string.h"

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/kmalloc.h"
#include "vm/vmmap.h"
#include "fs/open.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "mm/mman.h"
#include "fs/vnode.h"
#include "vm/shadow.h"
#include "fs/vfs_syscall.h"

typedef struct mobj_shadow
{
    // the mobj parts of this shadow object
    mobj_t mobj;
    // a reference to the mobj that is the data source for this shadow object
    // This should be a reference to a shadow object of some ancestor process.
    // This is used to traverse the shadow object chain.
    mobj_t *shadowed;
    // a reference to the mobj at the bottom of this shadow object's chain
    // this should NEVER be a shadow object (i.e. it should have some type other
    // than MOBJ_SHADOW)
    mobj_t *bottom_mobj;
} mobj_shadow_t;

#define MOBJ_TO_SO(o) CONTAINER_OF(o, mobj_shadow_t, mobj)


long test_vmmap() {
    vmmap_t *map = curproc->p_vmmap;

    // Make sure we start out cleanly
    KASSERT(vmmap_is_range_empty(map, ADDR_TO_PN(USER_MEM_LOW), ADDR_TO_PN(USER_MEM_HIGH - USER_MEM_LOW)));

    // Go through the address space, make sure we find nothing
    for (size_t i = USER_MEM_LOW; i < ADDR_TO_PN(USER_MEM_HIGH); i += PAGE_SIZE) {
        KASSERT(!vmmap_lookup(map, i));
    }
    
    // You can probably change this.
    size_t num_vmareas = 5;
    // Probably shouldn't change this to anything that's not a power of two.
    size_t num_pages_per_vmarea = 16;

    size_t prev_start = ADDR_TO_PN(USER_MEM_HIGH);
    for (size_t i = 0; i < num_vmareas; i++) {
        ssize_t start = vmmap_find_range(map, num_pages_per_vmarea, VMMAP_DIR_HILO);
        test_assert(start + num_pages_per_vmarea == prev_start, "Incorrect return value from vmmap_find_range");
        
        vmarea_t *vma = kmalloc(sizeof(vmarea_t));
        KASSERT(vma && "Unable to alloc the vmarea");
        memset(vma, 0, sizeof(vmarea_t));

        vma->vma_start = start;
        vma->vma_end = start + num_pages_per_vmarea;
        vmmap_insert(map, vma);

        prev_start = start;
    }

    // Now, our address space should look like:
    // EMPTY EMPTY EMPTY [  ][  ][  ][  ][  ]
    // ^LP
    //                                      ^HP
    //                   ^section_start 
    // HP --> the highest possible userland page number
    // LP --> the lowest possible userland page number 
    // section start --> HP - (num_vmareas * num_pages_per_vmarea) 
    
    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink) {
        list_remove(&vma->vma_plink); 
        kfree(vma);
    }
    ssize_t start = vmmap_find_range(map, 16, VMMAP_DIR_LOHI);
    test_assert(start == ADDR_TO_PN(USER_MEM_LOW), "Range is wonky on the lohi portion");
    ssize_t other_start = vmmap_find_range(map, 16, VMMAP_DIR_HILO);
    test_assert(other_start == ADDR_TO_PN(USER_MEM_HIGH) - 16, "Range is wonky on the hilo portion");
    long fd = do_open("Hello", O_RDONLY | O_CREAT);
    file_t* file = fget(fd);
    size_t off = PAGE_SIZE;
    vmarea_t* area;

    long status;
    for (size_t i = ADDR_TO_PN(USER_MEM_LOW); i < ADDR_TO_PN(USER_MEM_LOW) + 512; i++) {
        status = vmmap_map(curproc->p_vmmap, file->f_vnode, i, 1, PROT_READ, MAP_FIXED, off, VMMAP_DIR_HILO, &area);
        test_assert(status == 0, "Vmmap_map failure");
    }

    for (size_t i = ADDR_TO_PN(USER_MEM_LOW); i < ADDR_TO_PN(USER_MEM_LOW) + 512; i++) {
        area = vmmap_lookup(curproc->p_vmmap, i);
        test_assert(area != NULL, "Could not find an area that should be present");
        area = NULL;
    }

    vmmap_remove(map, ADDR_TO_PN(USER_MEM_LOW), 512);

    for (size_t i = ADDR_TO_PN(USER_MEM_LOW); i < ADDR_TO_PN(USER_MEM_LOW) + 512; i++) {
        area = vmmap_lookup(curproc->p_vmmap, i);
        test_assert(area == NULL, "There should be no areas");
        area = NULL;
    }


    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start, 16, PROT_READ, MAP_FIXED, off, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    test_assert(area->vma_start == (size_t) start, "Start is wrong");
    test_assert(area->vma_end == (size_t) (start + 16), "End is wrong");
    test_assert(area->vma_off == 1, "Offset is wrong");
    test_assert(area->vma_prot == PROT_READ, "Prot is wrong");
    test_assert(area->vma_flags == MAP_FIXED, "Flags are wrong");
    test_assert(area->vma_vmmap == curproc->p_vmmap, "Map is wrong");
    test_assert(area->vma_obj->mo_type == file->f_vnode->vn_mobj.mo_type, "Obj is wrong");
    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink) {
        test_assert(vma == area, "Not the same area as created");
    }
    area = vmmap_lookup(curproc->p_vmmap, start);
    test_assert(area != NULL, "Area should be found");
    area = vmmap_lookup(curproc->p_vmmap, start - 1);
    test_assert(area == NULL, "Area should not be found");
    area = vmmap_lookup(curproc->p_vmmap, start + 16);
    test_assert(area == NULL, "Area should not be found");
    area = vmmap_lookup(curproc->p_vmmap, start + 15);
    test_assert(area != NULL, "Area should be found");
    vmmap_remove(map, start, 16);
    area = vmmap_lookup(curproc->p_vmmap, start);
    test_assert(area == NULL, "Area should not be found");
    area = vmmap_lookup(curproc->p_vmmap, start + 15);
    test_assert(area == NULL, "Area should not be found");
    area = vmmap_lookup(curproc->p_vmmap, start + 8);
    test_assert(area == NULL, "Area should not be found");

    test_assert(list_empty(&map->vmm_list), "List not empty");
    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start, 32, PROT_READ, MAP_FIXED, off, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    vmmap_remove(map, start, 16);
    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink) {
        test_assert(vma->vma_start == (size_t) (start + 16), "Start is wrong");
        test_assert(vma->vma_end == area->vma_end, "End is wrong");
        test_assert(vma->vma_off == 17, "Offset is wrong");
    }
    vmmap_remove(map, start + 16, 16);
    test_assert(list_empty(&map->vmm_list), "List not empty");
    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start, 32, PROT_READ, MAP_FIXED, off, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    vmmap_remove(map, start + 16, 16);
    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink) {
        test_assert(vma->vma_start == area->vma_start, "Start is wrong");
        test_assert(vma->vma_end == (size_t) (start + 16), "End is wrong");
        test_assert(vma->vma_off == area->vma_off, "Offset is wrong");
    }
    test_assert(!vmmap_is_range_empty(map, start, 16), "Range not empty");
    test_assert(!vmmap_is_range_empty(map, start, start + 32), "Range is empty");
    vmmap_remove(map, start, 16);
    test_assert(vmmap_is_range_empty(map, start, 16), "Range not empty");
    test_assert(list_empty(&map->vmm_list), "List not empty");
    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start, 32, PROT_READ, MAP_FIXED, off, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    vmmap_remove(map, start + 8, 16);
    size_t count = 0;
    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink) {
        count += 1;
        if (count == 1) {
            test_assert(vma->vma_start == area->vma_start, "Start is wrong");
            test_assert(vma->vma_end == (size_t) (start + 8), "End is wrong");
            test_assert(vma->vma_off == area->vma_off, "Offset is wrong");
        } else {
            test_assert(vma->vma_start == (size_t) (start + 24), "Start is wrong");
            test_assert(vma->vma_end == (size_t) (start + 32), "End is wrong");
            test_assert(vma->vma_off == area->vma_off + 24, "Offset is wrong");
        }
    }
    test_assert(count == 2, "Not the expected number of vmareas");
    vmmap_remove(map, start, 32);
    test_assert(vmmap_is_range_empty(map, start, 32), "Vmarea not removed");
    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start, 32, PROT_READ, MAP_FIXED, 0, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    const char* buf = "This should be readable";
    count = strlen(buf);
    file->f_vnode->vn_len = PAGE_SIZE * 32;
    status = vmmap_write(map, PN_TO_ADDR(start), buf, count);
    test_assert(status == 0, "Write failed");
    char* receive = "";
    status = vmmap_read(map, PN_TO_ADDR(start), receive, count);
    test_assert(status == 0, "Read failed");
    test_assert(!strncmp(buf, receive, count), "Did not read correctly");
    const char* buf2 = (const char *) kmalloc(PAGE_SIZE * 3 + 1);
    memset((void *) buf2, 'a', PAGE_SIZE * 3);
    status = vmmap_write(map, PN_TO_ADDR(start), buf2, PAGE_SIZE * 3 - 1);
    test_assert(status == 0, "Write failed");
    char* receive2 = (char *) kmalloc(PAGE_SIZE * 4);
    status = vmmap_read(map, PN_TO_ADDR(start), receive2, PAGE_SIZE * 3 - 1);
    test_assert(status == 0, "Read failed");
    test_assert(!strncmp(buf2, receive2, PAGE_SIZE * 3 - 1), "Did not read correctly");
    vmmap_remove(map, ADDR_TO_PN(USER_MEM_LOW), ADDR_TO_PN(USER_MEM_HIGH));
    kfree((void *) buf2);
    kfree((void *) receive2);
    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start, 32, PROT_READ, MAP_FIXED, off, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    vmmap_remove(map, start + 8, 1);
    count = 0;
    list_iterate(&map->vmm_list, vma, vmarea_t, vma_plink) {
        count += 1;
        if (count == 1) {
            test_assert(vma->vma_start == area->vma_start, "Start is wrong");
            test_assert(vma->vma_end == (size_t) (start + 8), "End is wrong");
            test_assert(vma->vma_off == area->vma_off, "Offset is wrong");
        } else {
            test_assert(vma->vma_start == (size_t) (start + 9), "Start is wrong");
            test_assert(vma->vma_end == (size_t) (start + 32), "End is wrong");
            test_assert(vma->vma_off == area->vma_off + 9, "Offset is wrong");
        }
    }

    // shadow testing
    vmmap_remove(map, start, 512);
    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start, 32, PROT_READ, MAP_PRIVATE | MAP_FIXED, 0, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    test_assert(area->vma_obj->mo_type == MOBJ_SHADOW, "Obj type is wrong");
    vmmap_remove(map, ADDR_TO_PN(USER_MEM_LOW), ADDR_TO_PN(USER_MEM_HIGH) - ADDR_TO_PN(USER_MEM_LOW));
    status = vmmap_map(curproc->p_vmmap, file->f_vnode, start+16, 32, PROT_READ, MAP_FIXED, 0, VMMAP_DIR_HILO, &area);
    test_assert(status == 0, "Vmmap_map failure");
    test_assert(!vmmap_is_range_empty(map, start+32, 1), "Range should not be empty");
    vmmap_remove(map, ADDR_TO_PN(USER_MEM_LOW), ADDR_TO_PN(USER_MEM_HIGH) - ADDR_TO_PN(USER_MEM_LOW));
    mobj_t* bottom = &file->f_vnode->vn_mobj;
    mobj_t* shadow1 = shadow_create(bottom);
    mobj_t* shadow2 = shadow_create(shadow1);
    mobj_t* shadow3 = shadow_create(shadow2);
    mobj_t* shadow4 = shadow_create(shadow2);
    mobj_unlock(shadow1);
    mobj_unlock(shadow2);
    mobj_unlock(shadow3);
    mobj_put(&shadow1);
    shadow_collapse(shadow4);
    mobj_shadow_t* shob4 = MOBJ_TO_SO(shadow4);
    test_assert(shob4->shadowed == shadow2, "Removed a shadow object with refcount 2");
    mobj_shadow_t* shob3 = MOBJ_TO_SO(shadow3);
    test_assert(shob3->shadowed == shadow2, "Removed a shadow object with refcount 2");
    mobj_shadow_t* shob2 = MOBJ_TO_SO(shadow2);
    test_assert(shob2->shadowed == bottom, "Failed to remove a shadow object with refcount 1");
    mobj_unlock(shadow4);
    shadow4->mo_ops.destructor(shadow4);
    shadow3->mo_ops.destructor(shadow3);
    shadow2->mo_ops.destructor(shadow2);
    return 0;
}

long vmtest_main(long arg1, void* arg2) {
    test_init(); 
    test_vmmap(); 

    // Write your own tests here!

    test_fini(); 
    return 0; 
}
