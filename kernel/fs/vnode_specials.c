#include <errno.h>
#include <fs/stat.h>
#include <fs/vfs.h>
#include <fs/vnode.h>
#include <util/debug.h>

static long special_file_stat(vnode_t *file, stat_t *ss);

static ssize_t chardev_file_read(vnode_t *file, size_t pos, void *buf,
                                 size_t count);

static ssize_t chardev_file_write(vnode_t *file, size_t pos, const void *buf,
                                  size_t count);

static long chardev_file_mmap(vnode_t *file, mobj_t **ret);

static long chardev_file_fill_pframe(vnode_t *file, pframe_t *pf);

static long chardev_file_flush_pframe(vnode_t *file, pframe_t *pf);

static vnode_ops_t chardev_spec_vops = {
    .read = chardev_file_read,
    .write = chardev_file_write,
    .mmap = chardev_file_mmap,
    .mknod = NULL,
    .lookup = NULL,
    .link = NULL,
    .unlink = NULL,
    .mkdir = NULL,
    .rmdir = NULL,
    .readdir = NULL,
    .stat = special_file_stat,
    .get_pframe = NULL,
    .fill_pframe = chardev_file_fill_pframe,
    .flush_pframe = chardev_file_flush_pframe,
};

static ssize_t blockdev_file_read(vnode_t *file, size_t pos, void *buf,
                                  size_t count);

static ssize_t blockdev_file_write(vnode_t *file, size_t pos, const void *buf,
                                   size_t count);

static long blockdev_file_mmap(vnode_t *file, mobj_t **ret);

static long blockdev_file_fill_pframe(vnode_t *file, pframe_t *pf);

static long blockdev_file_flush_pframe(vnode_t *file, pframe_t *pf);

static vnode_ops_t blockdev_spec_vops = {
    .read = blockdev_file_read,
    .write = blockdev_file_write,
    .mmap = blockdev_file_mmap,
    .mknod = NULL,
    .lookup = NULL,
    .link = NULL,
    .unlink = NULL,
    .mkdir = NULL,
    .rmdir = NULL,
    .readdir = NULL,
    .stat = special_file_stat,
    .get_pframe = NULL,
    .fill_pframe = blockdev_file_fill_pframe,
    .flush_pframe = blockdev_file_flush_pframe,
};

void init_special_vnode(vnode_t *vn)
{
    if (S_ISCHR(vn->vn_mode))
    {
        vn->vn_ops = &chardev_spec_vops;
        vn->vn_dev.chardev = chardev_lookup(vn->vn_devid);
    }
    else
    {
        KASSERT(S_ISBLK(vn->vn_mode));
        vn->vn_ops = &blockdev_spec_vops;
        vn->vn_dev.blockdev = blockdev_lookup(vn->vn_devid);
    }
}

static long special_file_stat(vnode_t *file, stat_t *ss)
{
    KASSERT(file->vn_fs->fs_root->vn_ops->stat != NULL);
    // call the containing file system's stat routine
    return file->vn_fs->fs_root->vn_ops->stat(file, ss);
}

/*
 * Make a read by deferring to the underlying chardev and its read operation.
 *
 * Returns what the chardev's read returned.
 *
 * Hint: Watch out! chardev_file_read and chardev_file_write are indirectly
 * called in do_read and do_write, respectively, as the read/write ops for
 * chardev-type vnodes. This means that the vnode file should be locked
 * upon entry to this function.
 *
 * However, tty_read and tty_write, the read/write ops for the tty chardev,
 * are potentially blocking. To avoid deadlock, you should unlock the file
 * before calling the chardev's read, and lock it again after. If you fail
 * to do this, a shell reading from /dev/tty0 for instance, will block all
 * access to the /dev/tty0 vnode. This means that if someone runs `ls /dev/`,
 * while a shell is reading from `/dev/tty0`, the `ls` call will hang.
 *
 * Also, if a vnode represents a chardev, you can access the chardev using
 * vnode->vn_dev.chardev.
 *
 */
static ssize_t chardev_file_read(vnode_t *file, size_t pos, void *buf,
                                 size_t count)
{
    vunlock(file);
    ssize_t size = file->vn_dev.chardev->cd_ops->read(file->vn_dev.chardev, pos, buf, count);
    vlock(file);
    return size;
}

/*
 * Make a write by deferring to the underlying chardev and its write operation.
 *
 * Return what the chardev's write returned.
 *
 * See the comments from chardev_file_read above for hints.
 *
 */
static long chardev_file_write(vnode_t *file, size_t pos, const void *buf,
                               size_t count)
{
    vunlock(file);
    ssize_t size = file->vn_dev.chardev->cd_ops->write(file->vn_dev.chardev, pos, buf, count);
    vlock(file);
    return size;
}

/*
 * For this and the following chardev functions, simply defer to the underlying
 * chardev's corresponding operations.
 */
static long chardev_file_mmap(vnode_t *file, mobj_t **ret)
{
    /* VM {{{ */
    return file->vn_dev.chardev->cd_ops->mmap(file, ret);
    /* VM }}} */
    return 0;
}

static long chardev_file_fill_pframe(vnode_t *file, pframe_t *pf)
{
    /* VM {{{ */
    return file->vn_dev.chardev->cd_ops->fill_pframe(file, pf);
    /* VM }}} */
    return 0;
}

static long chardev_file_flush_pframe(vnode_t *file, pframe_t *pf)
{
    /* VM {{{ */
    return file->vn_dev.chardev->cd_ops->flush_pframe(file, pf);
    /* VM }}} */
    return 0;
}

static ssize_t blockdev_file_read(vnode_t *file, size_t pos, void *buf,
                                  size_t count)
{
    return -ENOTSUP;
}

static long blockdev_file_write(vnode_t *file, size_t pos, const void *buf,
                                size_t count)
{
    return -ENOTSUP;
}

static long blockdev_file_mmap(vnode_t *file, mobj_t **ret) { return -ENOTSUP; }

static long blockdev_file_fill_pframe(vnode_t *file, pframe_t *pf)
{
    return -ENOTSUP;
}

static long blockdev_file_flush_pframe(vnode_t *file, pframe_t *pf)
{
    return -ENOTSUP;
}
