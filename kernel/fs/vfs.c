#include "errno.h"
#include "globals.h"
#include "kernel.h"
#include "util/string.h"
#include <fs/s5fs/s5fs.h>
#include <fs/vnode.h>

#include "fs/file.h"
#include "fs/ramfs/ramfs.h"

#include "mm/kmalloc.h"
#include "mm/slab.h"
#include "util/debug.h"

#ifdef __S5FS__
#include "fs/s5fs/s5fs.h"
#endif

#ifdef __MOUNTING__
/* The fs listed here are only the non-root file systems */
list_t mounted_fs_list;

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * The purpose of this function is to set up the pointers between the file
 * system struct and the vnode of the mount point. Remember to watch your
 * reference counts. (The exception here is when the vnode's vn_mount field
 * points to the mounted file system's root we do not increment the reference
 * count on the file system's root vnode. The file system is already keeping
 * a reference to the vnode which will not go away until the file system is
 * unmounted. If we kept a second such reference it would conflict with the
 * behavior of vfs_is_in_use(), make sure you understand why.)
 *
 * Once everything is set up add the file system to the list of mounted file
 * systems.
 *
 * Remember proper error handling.
 *
 * This function is not meant to mount the root file system.
 */
int vfs_mount(struct vnode *mtpt, fs_t *fs)
{
    NOT_YET_IMPLEMENTED("MOUNTING: vfs_mount");
    return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * The purpose of this function is to undo the setup done in vfs_mount(). Also
 * you should call the underlying file system's umount() function. Make sure
 * to keep track of reference counts. You should also kfree the fs struct at
 * the end of this method.
 *
 * Remember proper error handling. You might want to make sure that you do not
 * try to call this function on the root file system (this function is not meant
 * to unmount the root file system).
 */
int vfs_umount(fs_t *fs)
{
    NOT_YET_IMPLEMENTED("MOUNTING: vfs_unmount");
    return -EINVAL;
}
#endif /* __MOUNTING__ */

fs_t vfs_root_fs = {
    .fs_dev = VFS_ROOTFS_DEV,
    .fs_type = VFS_ROOTFS_TYPE,
    .vnode_list = LIST_INITIALIZER(vfs_root_fs.vnode_list),
    .vnode_list_mutex = KMUTEX_INITIALIZER(vfs_root_fs.vnode_list_mutex),
    .fs_vnode_allocator = NULL,
    .fs_i = NULL,
    .fs_ops = NULL,
    .fs_root = NULL,
};

/*
 * Call mountfunc on vfs_root_fs and set curproc->p_cwd (reference count!)
 */
void vfs_init()
{
    long err = mountfunc(&vfs_root_fs);
    if (err)
    {
        panic(
            "Failed to mount root fs of type \"%s\" on device "
            "\"%s\" with errno of %ld\n",
            vfs_root_fs.fs_type, vfs_root_fs.fs_dev, -err);
    }

    vlock(vfs_root_fs.fs_root);
    vref(curproc->p_cwd = vfs_root_fs.fs_root);
    vunlock(vfs_root_fs.fs_root);

#ifdef __MOUNTING__
    list_init(&mounted_fs_list);
    fs->fs_mtpt = vfs_root_fs.fs_root;
#endif
}

/*
 * Wrapper around the sync call() to vfs_root_fs using fs_ops
 */
void do_sync()
{
    vfs_root_fs.fs_ops->sync(&vfs_root_fs);
#ifdef __MOUNTING__
    // if implementing mounting, just sync() all the mounted FS's as well
#endif
}

/*
 *
 */
long vfs_shutdown()
{
    dbg(DBG_VFS, "shutting down vfs\n");
    long ret = 0;

#ifdef __MOUNTING__
    list_iterate(&mounted_fs_list, mtfs, fs_t, fs_link)
    {
        ret = vfs_umount(mtfs);
        KASSERT(!ret);
    }
#endif

    if (vfs_is_in_use(&vfs_root_fs))
    {
        panic("vfs_shutdown: found active vnodes in root filesystem");
    }

    if (vfs_root_fs.fs_ops->umount)
    {
        ret = vfs_root_fs.fs_ops->umount(&vfs_root_fs);
    }
    else
    {
        // vlock(vfs_root_fs.fs_root);
        vput(&vfs_root_fs.fs_root);
    }

    if (vfs_count_active_vnodes(&vfs_root_fs))
    {
        panic(
            "vfs_shutdown: vnodes still in use after unmounting root "
            "filesystem");
    }
    return ret;
}

long mountfunc(fs_t *fs)
{
    static const struct
    {
        char *fstype;

        long (*mountfunc)(fs_t *);
    } types[] = {
#ifdef __S5FS__
        {"s5fs", s5fs_mount},
#endif
        {"ramfs", ramfs_mount},
    };

    for (unsigned int i = 0; i < sizeof(types) / sizeof(types[0]); i++)
    {
        if (strcmp(fs->fs_type, types[i].fstype) == 0)
        {
            return types[i].mountfunc(fs);
        }
    }

    return -EINVAL;
}

/*
 * A filesystem is in use if the total number of vnode refcounts for that
 * filesystem > 1. The singular refcount in a fs NOT in use comes from fs_root.
 *
 * Error cases vfs_is_in_use is responsible for generating:
 *  - EBUSY: if the filesystem is in use
 */
long vfs_is_in_use(fs_t *fs)
{
    long ret = 0;
    // kmutex_lock(&fs->vnode_list_mutex);
    list_iterate(&fs->vnode_list, vn, vnode_t, vn_link)
    {
        vlock(vn);
        size_t expected_refcount = vn->vn_fs->fs_root == vn ? 1 : 0;
        size_t refcount = vn->vn_mobj.mo_refcount;
        vunlock(vn);
        if (refcount != expected_refcount)
        {
            dbg(DBG_VFS,
                "vnode %d still in use with %d references and %lu mobj "
                "references (expected %lu)\n",
                vn->vn_vno, vn->vn_mobj.mo_refcount, refcount,
                expected_refcount);
            ret = -EBUSY;
            // break;
        }
    }
    // kmutex_unlock(&fs->vnode_list_mutex);
    return ret;
}

/*
 * Return the size of fs->vnode_list
 */
size_t vfs_count_active_vnodes(fs_t *fs)
{
    size_t count = 0;
    kmutex_lock(&fs->vnode_list_mutex);
    list_iterate(&fs->vnode_list, vn, vnode_t, vn_link) { count++; }
    kmutex_unlock(&fs->vnode_list_mutex);
    return count;
}
