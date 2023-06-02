#include "vm/mmap.h"
#include "errno.h"
#include "fs/file.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "globals.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/tlb.h"
#include "util/debug.h"

/*
 * This function implements the mmap(2) syscall: Add a mapping to the current
 * process's address space. Supports the following flags: MAP_SHARED,
 * MAP_PRIVATE, MAP_FIXED, and MAP_ANON.
 *
 *  ret - If provided, on success, *ret must point to the start of the mapped area
 *
 * Return 0 on success, or:
 *  - EACCES: 
 *     - A file descriptor refers to a non-regular file.  
 *     - a file mapping was requested, but fd is not open for reading. 
 *     - MAP_SHARED was requested and PROT_WRITE is set, but fd is
 *       not open in read/write (O_RDWR) mode.
 *     - PROT_WRITE is set, but the file has FMODE_APPEND specified.
 *  - EBADF:
 *     - fd is not a valid file descriptor and MAP_ANON was
 *       not set
 *  - EINVAL:
 *     - addr is not page aligned and MAP_FIXED is specified 
 *     - off is not page aligned 
 *     - len is <= 0 or off < 0 
 *     - flags do not contain MAP_PRIVATE or MAP_SHARED
 *  - ENODEV:
 *     - The underlying filesystem of the specified file does not
 *       support memory mapping or in other words, the file's vnode's mmap
 *       operation doesn't exist
 *  - Propagate errors from vmmap_map()
 * 
 *  See the man page for mmap(2) errors section for more details
 * 
 * Hints:
 *  1) A lot of error checking.
 *  2) Call vmmap_map() to create the mapping.
 *     a) Use VMMAP_DIR_HILO as default, which will make other stencil code in
 *        Weenix happy.
 *  3) Call tlb_flush_range() on the newly-mapped region. This is because the
 *     newly-mapped region could have been used by someone else, and you don't
 *     want to get stale mappings.
 *  4) Don't forget to set ret if it was provided.
 * 
 *  If you are mapping less than a page, make sure that you are still allocating 
 *  a full page. 
 */
long do_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off,
             void **ret)
{
    dbg(DBG_TEST, "\nSTARTING DO_MMAP\n");
    // all EINVAL cases
    if ((int) len <= 0 || (int) off < 0) {
        dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
        return -EINVAL;
    }
    if (!(flags & MAP_PRIVATE) && !(flags & MAP_SHARED)) {
        dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
        return -EINVAL;
    }
    if (!PAGE_ALIGNED(off)) {
        dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
        return -EINVAL;
    }
    if (!PAGE_ALIGNED(addr) && (flags & MAP_FIXED)) {
        dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
        return -EINVAL;
    }
    if (((uintptr_t) addr < USER_MEM_LOW) && (flags & MAP_FIXED)) {
        return -EINVAL;
    }
    file_t* file = NULL;
    if ((int) fd >= 0) {
        file = curproc->p_files[fd];
        if (!file) {
            return -EBADF;
        }
    } else if (!(flags & MAP_ANON)) {
        return -EBADF;
    }

    // the EBADF case
    if (!file && !(flags & MAP_ANON) && (flags & MAP_FIXED)) {
        dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
        return -EBADF;
    }

    // all ENODEV cases
    if (file) {
        if (!file->f_vnode->vn_ops) {
            dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
            return -ENODEV;
        }
        if (!file->f_vnode->vn_ops->mmap) {
            dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
            return -ENODEV;
        }
        if (!(file->f_mode & FMODE_READ)) {
            dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
            return -EACCES;
        }
        if ((file->f_mode & FMODE_APPEND) && (prot & PROT_WRITE)) {
            dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
            return -EACCES;
        }
        if (!(file->f_mode & FMODE_READ || file->f_mode & FMODE_WRITE) && (flags & MAP_SHARED) && (prot & PROT_WRITE)) {
            dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
            return -EACCES;
        }
        if (!(file->f_mode & FMODE_WRITE) && (prot & PROT_WRITE) && (flags & MAP_SHARED)) {
            dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
            return -EACCES;
        }
    }
    size_t lopage = ADDR_TO_PN(PAGE_ALIGN_DOWN(addr));
    size_t npages = ADDR_TO_PN(PAGE_ALIGN_UP((uintptr_t) addr + len)) - lopage;
    vmarea_t* new_vma;
    vnode_t* node = NULL;
    if (file) {
        node = file->f_vnode;
    }
    long status = vmmap_map(curproc->p_vmmap, node, lopage, npages, prot, flags, off, VMMAP_DIR_HILO, &new_vma);
    if (status < 0) {
        dbg(DBG_TEST, "\nDO_MMAP FAILED\n");
        return status;
    }
    dbg(DBG_TEST, "\nFLUSHING RANGE\n");
    tlb_flush_range((uintptr_t) PN_TO_ADDR(new_vma->vma_start), (new_vma->vma_end - new_vma->vma_start) * PAGE_SIZE);
    dbg(DBG_TEST, "\nFINSIHED FLUSHING RANGE\n");
    if (ret) {
        *ret = PN_TO_ADDR(new_vma->vma_start);
    }
    dbg(DBG_TEST, "\nFINISHED DO_MMAP\n");
    return 0;
}

/*
 * This function implements the munmap(2) syscall.
 *
 * Return 0 on success, or:
 *  - EINVAL:
 *     - addr is not aligned on a page boundary
 *     - the region to unmap is out of range of the user address space
 *     - len is 0
 *  - Propagate errors from vmmap_remove()
 * 
 *  See the man page for munmap(2) errors section for more details
 *
 * Hints:
 *  - Similar to do_mmap():
 *  1) Perform error checking.
 *  2) Call vmmap_remove().
 */
long do_munmap(void *addr, size_t len)
{
    if (!PAGE_ALIGNED(addr)) {
        return -EINVAL;
    }
    if (len <= 0) {
        return -EINVAL;
    }
    if ((uintptr_t) addr < USER_MEM_LOW) {
        return -EINVAL;
    }
    if ((uintptr_t) addr > USER_MEM_HIGH) {
        return -EINVAL;
    }
    if (len > USER_MEM_HIGH) {
        return -EINVAL;
    }
    if ((uintptr_t) addr + len > USER_MEM_HIGH) {
        return -EINVAL;
    }
    size_t lopage = ADDR_TO_PN(addr);
    size_t endpage = ADDR_TO_PN(PAGE_ALIGN_UP((uintptr_t) addr + len));
    KASSERT(lopage != endpage); // Your math is bad
    long status = vmmap_remove(curproc->p_vmmap, lopage, endpage - lopage);
    return status;
}