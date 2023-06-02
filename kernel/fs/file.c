#include "fs/file.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "kernel.h"
#include "mm/slab.h"
#include "util/debug.h"
#include "util/string.h"

static slab_allocator_t *file_allocator;

void file_init(void)
{
    file_allocator = slab_allocator_create("file", sizeof(file_t));
}

void fref(file_t *f)
{
    KASSERT(f->f_mode <= FMODE_MAX_VALUE && f->f_vnode);

    f->f_refcount++;

    if (f->f_vnode)
    {
        dbg(DBG_FREF, "fref: 0x%p, 0x%p ino %u, up to %lu\n", f,
            f->f_vnode->vn_fs, f->f_vnode->vn_vno, f->f_refcount);
    }
    else
    {
        dbg(DBG_FREF, "fref: 0x%p up to %lu\n", f, f->f_refcount);
    }
}

/*
 * Create a file, initialize its members, vref the vnode, call acquire() on the
 * vnode if the function pointer is non-NULL, and set the file descriptor in
 * curproc->p_files.
 *
 * On successful return, the vnode's refcount should be incremented by one,
 * the file's refcount should be 1, and curproc->p_files[fd] should point to
 * the file being returned.
 */
file_t *fcreate(int fd, vnode_t *vnode, unsigned int mode)
{
    KASSERT(!curproc->p_files[fd]);
    file_t *file = slab_obj_alloc(file_allocator);
    if (!file)
        return NULL;
    memset(file, 0, sizeof(file_t));
    file->f_mode = mode;

    vref(file->f_vnode = vnode);
    if (vnode->vn_ops->acquire)
        vnode->vn_ops->acquire(vnode, file);

    curproc->p_files[fd] = file;
    fref(file);
    return file;
}

/*
 * Perform bounds checking on the fd, use curproc->p_files to get the file,
 * fref it if it exists, and return.
 */
file_t *fget(int fd)
{
    if (fd < 0 || fd >= NFILES)
        return NULL;
    file_t *file = curproc->p_files[fd];
    if (file)
        fref(file);
    return file;
}

/*
 * Decrement the refcount, and set *filep to NULL.
 *
 * If the refcount drops to 0, call release on the vnode if the function pointer
 * is non-null, vput() file's vnode, and free the file memory.
 *
 * Regardless of the ending refcount, *filep == NULL on return.
 */
void fput(file_t **filep)
{
    file_t *file = *filep;
    *filep = NULL;

    KASSERT(file && file->f_mode <= FMODE_MAX_VALUE);
    KASSERT(file->f_refcount > 0);
    if (file->f_refcount != 1)
        KASSERT(file->f_vnode);

    file->f_refcount--;

    if (file->f_vnode)
    {
        dbg(DBG_FREF, "fput: 0x%p, 0x%p ino %u, down to %lu\n", file,
            file->f_vnode->vn_fs, file->f_vnode->vn_vno, file->f_refcount);
    }
    else
    {
        dbg(DBG_FREF, "fput: 0x%p down to %lu\n", file, file->f_refcount);
    }

    if (!file->f_refcount)
    {
        if (file->f_vnode)
        {
            vlock(file->f_vnode);
            if (file->f_vnode->vn_ops->release)
                file->f_vnode->vn_ops->release(file->f_vnode, file);
            vput_locked(&file->f_vnode);
        }
        slab_obj_free(file_allocator, file);
    }
}
