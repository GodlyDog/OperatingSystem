#include "errno.h"
#include "fs/fcntl.h"
#include "fs/file.h"
#include "fs/vfs.h"
#include "fs/vfs_syscall.h"
#include "fs/vnode.h"
#include "globals.h"
#include "util/debug.h"
#include <fs/vnode.h>

// NOTE: IF DOING MULTI-THREADED PROCS, NEED TO SYNCHRONIZE ACCESS TO FILE
// DESCRIPTORS, AND, MORE GENERALLY SPEAKING, p_files, IN PARTICULAR IN THIS
// FUNCTION AND ITS CALLERS.
/*
 * Go through curproc->p_files and find the first null entry.
 * If one exists, set fd to that index and return 0.
 *
 * Error cases get_empty_fd is responsible for generating:
 *  - EMFILE: no empty file descriptor
 */
long get_empty_fd(int *fd)
{
    for (*fd = 0; *fd < NFILES; (*fd)++)
    {
        if (!curproc->p_files[*fd])
        {
            return 0;
        }
    }
    *fd = -1;
    return -EMFILE;
}

/*
 * Open the file at the provided path with the specified flags.
 *
 * Returns the file descriptor on success, or error cases:
 *  - EINVAL: Invalid oflags
 *  - EISDIR: Trying to open a directory with write access
 *  - ENXIO: Blockdev or chardev vnode does not have an actual underlying device
 *  - ENOMEM: Not enough kernel memory (if fcreate() fails)
 *
 * Hints:
 * 1) Use get_empty_fd() to get an available fd.
 * 2) Use namev_open() with oflags, mode S_IFREG, and devid 0.
 * 3) Check for EISDIR and ENXIO errors.
 * 4) Convert oflags (O_RDONLY, O_WRONLY, O_RDWR, O_APPEND) into corresponding
 *    file access flags (FMODE_READ, FMODE_WRITE, FMODE_APPEND). 
 * 5) Use fcreate() to create and initialize the corresponding file descriptor
 *    with the vnode from 2) and the mode from 4).
 *
 * When checking oflags, you only need to check that the read and write
 * permissions are consistent. However, because O_RDONLY is 0 and O_RDWR is 2,
 * there's no way to tell if both were specified. So, you really only need
 * to check if O_WRONLY and O_RDWR were specified. 
 * 
 * If O_TRUNC specified and the vnode represents a regular file, make sure to call the
 * the vnode's truncate routine (to reduce the size of the file to 0).
 *
 * If a vnode represents a chardev or blockdev, then the appropriate field of
 * the vnode->vn_dev union will point to the device. Otherwise, the union will be NULL.
 */
long do_open(const char *filename, int oflags)
{
    if ((oflags & O_WRONLY) && (oflags & O_RDONLY)) {
        return -EINVAL;
    }
    if ((oflags & O_WRONLY) && (oflags & O_RDWR)) {
        return -EINVAL;
    }
    int fd;
    long fd_status = get_empty_fd(&fd);
    if (fd_status < 0) {
        return fd_status;
    }
    vnode_t* res_vnode;
    long open_status = namev_open(curproc->p_cwd, filename, oflags, S_IFREG, 0, &res_vnode);
    if (open_status < 0) {
        return open_status;
    }
    if (S_ISBLK(res_vnode->vn_mode)) {
        if (!res_vnode->vn_dev.blockdev) {
            vput(&res_vnode);
            return -ENXIO;
        }
    }
    if (S_ISCHR(res_vnode->vn_mode)) {
        if (!res_vnode->vn_dev.chardev) {
            vput(&res_vnode);
            return -ENXIO;
        }
    }
    if (S_ISDIR(res_vnode->vn_mode) && ((oflags & O_WRONLY) || (oflags & O_RDWR))) {
        vput(&res_vnode);
        return -EISDIR; 
    }
    unsigned int mode = 0;
    if (oflags & O_RDWR) {
        mode = mode | FMODE_WRITE | FMODE_READ;
    } else if (oflags & O_WRONLY) {
        mode = mode | FMODE_WRITE;
    } else {
        mode = mode | FMODE_READ;
    }
    
    if (oflags & O_APPEND) {
        mode = mode | FMODE_APPEND;
    }
    if ((oflags & O_TRUNC) && !S_ISDIR(res_vnode->vn_mode)) {
        res_vnode->vn_ops->truncate_file(res_vnode);
    }
    file_t* created = fcreate(fd, res_vnode, mode);
    vput(&res_vnode);
    if (!created) {
        return -ENOMEM;
    }
    return fd;
}
