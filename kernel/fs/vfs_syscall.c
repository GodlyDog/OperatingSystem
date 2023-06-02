#include "fs/vfs_syscall.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "fs/file.h"
#include "fs/lseek.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "globals.h"
#include "kernel.h"
#include "util/debug.h"
#include "util/string.h"
#include <limits.h>

/*
 * Read len bytes into buf from the fd's file using the file's vnode operation
 * read.
 *
 * Return the number of bytes read on success, or:
 *  - EBADF: fd is invalid or is not open for reading
 *  - EISDIR: fd refers to a directory
 *  - Propagate errors from the vnode operation read
 *
 * Hints:
 *  - Be sure to update the file's position appropriately.
 *  - Lock/unlock the file's vnode when calling its read operation.
 */
ssize_t do_read(int fd, void *buf, size_t len)
{
    if (fd >= NFILES || fd < 0) {
        return -EBADF;
    }
    file_t* file = curproc->p_files[fd];
    if (!file) {
        return -EBADF;
    }
    vnode_t *node = file->f_vnode;
    vlock(node);
    if (S_ISDIR(node->vn_mode)) {
        vunlock(node);
        return -EISDIR;
    }
    if (!(FMODE_READ & file->f_mode)) {
        vunlock(node);
        return -EBADF;
    }
    ssize_t num_read = node->vn_ops->read(node, file->f_pos, buf, len);
    vunlock(node);
    if (num_read < 0) {
        return num_read;
    }
    file->f_pos = file->f_pos + num_read;
    return num_read;
}

/*
 * Write len bytes from buf into the fd's file using the file's vnode operation
 * write.
 *
 * Return the number of bytes written on success, or:
 *  - EBADF: fd is invalid or is not open for writing
 *  - Propagate errors from the vnode operation write
 *
 * Hints:
 *  - Check out `man 2 write` for details about how to handle the FMODE_APPEND
 *    flag.
 *  - Be sure to update the file's position appropriately.
 *  - Lock/unlock the file's vnode when calling its write operation.
 */
ssize_t do_write(int fd, const void *buf, size_t len)
{
    if (fd >= NFILES || fd < 0) {
        return -EBADF;
    }
    file_t* file = curproc->p_files[fd];
    if (!file) {
        return -EBADF;
    }
    if (!(FMODE_WRITE & file->f_mode)) {
        return -EBADF;
    }
    vnode_t *node = file->f_vnode;
    vlock(node);
    if (FMODE_APPEND & file->f_mode) {
        file->f_pos = node->vn_len;
    }
    ssize_t num_written = node->vn_ops->write(node, file->f_pos, buf, len);
    if (num_written < 0) {
        vunlock(node);
        return num_written;
    }
    file->f_pos = file->f_pos + num_written;
    vunlock(node);
    return num_written;
}

/*
 * Close the file descriptor fd.
 *
 * Return 0 on success, or:
 *  - EBADF: fd is invalid or not open
 * 
 * Hints: 
 * Check `proc.h` to see if there are any helpful fields in the 
 * proc_t struct for checking if the file associated with the fd is open. 
 * Consider what happens when we open a file and what counts as closing it
 */
long do_close(int fd)
{
    if (fd >= NFILES || fd < 0) {
        return -EBADF;
    }
    if (!curproc->p_files[fd]) {
        return -EBADF;
    }
    fput(&curproc->p_files[fd]);
    curproc->p_files[fd] = NULL;
    return 0;
}

/*
 * Duplicate the file descriptor fd.
 *
 * Return the new file descriptor on success, or:
 *  - EBADF: fd is invalid or not open
 *  - Propagate errors from get_empty_fd()
 *
 * Hint: Use get_empty_fd() to obtain an available file descriptor.
 */
long do_dup(int fd)
{
    if (fd < 0 || fd >= NFILES) {
        return -EBADF;
    }
    int new_fd;
    long status = get_empty_fd(&new_fd);
    if (status < 0) {
        return status;
    }
    file_t* file = fget(fd);
    if (!file) {
        return -EBADF;
    }
    if (!file->f_mode) {
        fput(&file);
        return -EBADF;
    }
    curproc->p_files[new_fd] = file;
    return new_fd;
}

/*
 * Duplicate the file descriptor ofd using the new file descriptor nfd. If nfd
 * was previously open, close it.
 *
 * Return nfd on success, or:
 *  - EBADF: ofd is invalid or not open, or nfd is invalid
 *
 * Hint: You don't need to do anything if ofd and nfd are the same.
 * (If supporting MTP, this action must be atomic)
 */
long do_dup2(int ofd, int nfd)
{
    if (ofd < 0 || ofd >= NFILES || nfd < 0 || nfd >= NFILES) {
        return -EBADF;
    }
    file_t* file = fget(ofd);
    if (!file) {
        return -EBADF;
    }
    if (!file->f_mode) {
        fput(&file);
        return -EBADF;
    }
    if (curproc->p_files[nfd]) {
        long close_status = do_close(nfd);
        if (close_status < 0) {
            fput(&file);
            return close_status;
        }
    }
    curproc->p_files[nfd] = file;
    return nfd;
}

/*
 * Create a file specified by mode and devid at the location specified by path.
 *
 * Return 0 on success, or:
 *  - EINVAL: Mode is not S_IFCHR, S_IFBLK, or S_IFREG
 *  - Propagate errors from namev_open()
 *
 * Hints:
 *  - Create the file by calling namev_open() with the O_CREAT flag.
 *  - Be careful about refcounts after calling namev_open(). The newly created 
 *    vnode should have no references when do_mknod returns. The underlying 
 *    filesystem is responsible for maintaining references to the inode, which 
 *    will prevent it from being destroyed, even if the corresponding vnode is 
 *    cleaned up.
 *  - You don't need to handle EEXIST (this would be handled within namev_open, 
 *    but doing so would likely cause problems elsewhere)
 */
long do_mknod(const char *path, int mode, devid_t devid)
{
    if (!(mode & S_IFCHR) && !(mode & S_IFBLK) && !(mode & S_IFREG)) {
        return -EINVAL;
    }
    vnode_t* res;
    long status = namev_open(curproc->p_cwd, path, O_CREAT, mode, devid, &res);
    if (status < 0) {
        return status;
    }
    vput(&res);
    return 0;
}

/*
 * Create a directory at the location specified by path.
 *
 * Return 0 on success, or:
 *  - ENAMETOOLONG: The last component of path is too long
 *  - ENOTDIR: The parent of the directory to be created is not a directory
 *  - EEXIST: A file located at path already exists
 *  - Propagate errors from namev_dir(), namev_lookup(), and the vnode
 *    operation mkdir
 *
 * Hints:
 * 1) Use namev_dir() to find the parent of the directory to be created.
 * 2) Use namev_lookup() to check that the directory does not already exist.
 * 3) Use the vnode operation mkdir to create the directory.
 *  - Compare against NAME_LEN to determine if the basename is too long.
 *    Check out ramfs_mkdir() to confirm that the basename will be null-
 *    terminated.
 *  - Be careful about locking and refcounts after calling namev_dir() and
 *    namev_lookup().
 */
long do_mkdir(const char *path)
{
    vnode_t* res_vnode;
    const char* name;
    size_t namelen = 0;
    long status = namev_dir(curproc->p_cwd, path, &res_vnode, &name, &namelen);
    if (namelen > NAME_LEN) {
        vput(&res_vnode);
        return -ENAMETOOLONG;
    }
    if (status < 0) {
        return status;
    }
    vnode_t* existing;
    vlock(res_vnode);
    status = namev_lookup(res_vnode, name, namelen, &existing);
    if (status >= 0) {
        vput(&existing);
        vput_locked(&res_vnode);
        return -EEXIST;
    }
    if (status != -ENOENT) {
        if (res_vnode) {
            vput_locked(&res_vnode);
        }
        return status;
    }
    vnode_t* created;
    status = res_vnode->vn_ops->mkdir(res_vnode, name, namelen, &created);
    vput_locked(&res_vnode);
    if (status < 0) {
        return status;
    }
    vput(&created);
    return 0;
}

/*
 * Delete a directory at path.
 *
 * Return 0 on success, or:
 *  - EINVAL: Attempting to rmdir with "." as the final component
 *  - ENOTEMPTY: Attempting to rmdir with ".." as the final component
 *  - ENOTDIR: The parent of the directory to be removed is not a directory
 *  - ENAMETOOLONG: the last component of path is too long
 *  - Propagate errors from namev_dir() and the vnode operation rmdir
 *
 * Hints:
 *  - Use namev_dir() to find the parent of the directory to be removed.
 *  - Be careful about refcounts from calling namev_dir().
 *  - Use the parent directory's rmdir operation to remove the directory.
 *  - Lock/unlock the vnode when calling its rmdir operation.
 */
long do_rmdir(const char *path)
{
    vnode_t* res;
    const char* name;
    size_t len;
    long status = namev_dir(curproc->p_cwd, path, &res, &name, &len);
    if (status < 0) {
        return status;
    }
    if (path[strlen(path) - 1] == '.') {
        vput(&res);
        if (path[strlen(path) - 2] == '.') {
            return -ENOTEMPTY;
        }
        return -EINVAL;
    }
    vlock(res);
    if (!S_ISDIR(res->vn_mode)) {
        vput_locked(&res);
        return -ENOTDIR;
    }
    status = res->vn_ops->rmdir(res, name, len);
    vput_locked(&res);
    return status;
}

/*
 * Remove the link between path and the file it refers to.
 *
 * Return 0 on success, or:
 *  - ENOTDIR: the parent of the file to be unlinked is not a directory
 *  - ENAMETOOLONG: the last component of path is too long
 *  - Propagate errors from namev_dir() and the vnode operation unlink
 *
 * Hints:
 *  - Use namev_dir() and be careful about refcounts.
 *  - Lock/unlock the parent directory when calling its unlink operation.
 */
long do_unlink(const char *path)
{
    vnode_t* res;
    const char* name;
    size_t len;
    long status = namev_dir(curproc->p_cwd, path, &res, &name, &len);
    if (status < 0) {
        return status;
    }
    if (!S_ISDIR(res->vn_mode)) {
        vput(&res);
        return -ENOTDIR;
    }
    vnode_t* found;
    vlock(res);
    status = namev_lookup(res, name, len, &found);
    if (status < 0) {
        vput_locked(&res);
        return status;
    }
    if (S_ISDIR(found->vn_mode)) {
        vput_locked(&res);
        vput(&found);
        return -EPERM;
    }
    status = res->vn_ops->unlink(res, name, len);
    vput(&found);
    vput_locked(&res);
    return status;
}

/*
 * Create a hard link newpath that refers to the same file as oldpath.
 *
 * Return 0 on success, or:
 *  - EPERM: oldpath refers to a directory
 *  - ENAMETOOLONG: The last component of newpath is too long
 *  - ENOTDIR: The parent of the file to be linked is not a directory
 *
 * Hints:
 * 1) Use namev_resolve() on oldpath to get the target vnode.
 * 2) Use namev_dir() on newpath to get the directory vnode.
 * 3) Use vlock_in_order() to lock the directory and target vnodes.
 * 4) Use the directory vnode's link operation to create a link to the target.
 * 5) Use vunlock_in_order() to unlock the vnodes.
 * 6) Make sure to clean up references added from calling namev_resolve() and
 *    namev_dir().
 */
long do_link(const char *oldpath, const char *newpath)
{
    vnode_t* old_res;
    long status = namev_resolve(curproc->p_cwd, oldpath, &old_res);
    if (status < 0) {
        return status;
    }
    if (S_ISDIR(old_res->vn_mode)) {
        vput(&old_res);
        return -EPERM;
    }
    vnode_t* directory_res;
    const char* new_name;
    size_t new_len;
    status = namev_dir(curproc->p_cwd, newpath, &directory_res, &new_name, &new_len);
    if (status < 0) {
        vput(&old_res);
        return status;
    }
    if (strlen(new_name) > NAME_LEN) {
        vput(&old_res);
        vput(&directory_res);
        return -ENAMETOOLONG;
    }
    if (!S_ISDIR(directory_res->vn_mode)) {
        vput(&old_res);
        vput(&directory_res);
        return -ENOTDIR;
    }
    vlock_in_order(old_res, directory_res);
    status = directory_res->vn_ops->link(directory_res, new_name, new_len, old_res);
    vunlock_in_order(old_res, directory_res);
    vput(&old_res);
    vput(&directory_res);
    return status;
}

/* Rename a file or directory.
 *
 * Return 0 on success, or:
 *  - ENOTDIR: the parent of either path is not a directory
 *  - ENAMETOOLONG: the last component of either path is too long
 *  - Propagate errors from namev_dir() and the vnode operation rename
 *
 * You DO NOT need to support renaming of directories.
 * Steps:
 * 1. namev_dir oldpath --> olddir vnode
 * 2. namev_dir newpath --> newdir vnode
 * 4. Lock the olddir and newdir in ancestor-first order (see `vlock_in_order`)
 * 5. Use the `rename` vnode operation
 * 6. Unlock the olddir and newdir
 * 8. vput the olddir and newdir vnodes
 *
 * Alternatively, you can allow do_rename() to rename directories if
 * __RENAMEDIR__ is set in Config.mk. As with all extra credit
 * projects this is harder and you will get no extra credit (but you
 * will get our admiration). Please make sure the normal version works first.
 * Steps:
 * 1. namev_dir oldpath --> olddir vnode
 * 2. namev_dir newpath --> newdir vnode
 * 3. Lock the global filesystem `vnode_rename_mutex`
 * 4. Lock the olddir and newdir in ancestor-first order (see `vlock_in_order`)
 * 5. Use the `rename` vnode operation
 * 6. Unlock the olddir and newdir
 * 7. Unlock the global filesystem `vnode_rename_mutex`
 * 8. vput the olddir and newdir vnodes
 *
 * P.S. This scheme /probably/ works, but we're not 100% sure.
 */
long do_rename(const char *oldpath, const char *newpath)
{
    vnode_t* old_res;
    const char* name;
    size_t len;
    long status = namev_dir(curproc->p_cwd, oldpath, &old_res, &name, &len);
    if (status < 0) {
        return status;
    }
    if (!S_ISDIR(old_res->vn_mode)) {
        vput(&old_res);
        return -ENOTDIR;
    }
    vnode_t* new_res;
    const char* new_name;
    size_t new_len;
    status = namev_dir(curproc->p_cwd, newpath, &new_res, &new_name, &new_len);
    if (status < 0) {
        vput(&old_res);
        return status;
    }
    if (!S_ISDIR(new_res->vn_mode)) {
        vput(&old_res);
        vput(&new_res);
        return -ENOTDIR;
    }
    vlock_in_order(old_res, new_res);
    status = old_res->vn_ops->rename(old_res, name, len, new_res, new_name, new_len);
    vunlock_in_order(old_res, new_res);
    vput(&old_res);
    vput(&new_res);
    return status;
}

/* Set the current working directory to the directory represented by path.
 *
 * Returns 0 on success, or:
 *  - ENOTDIR: path does not refer to a directory
 *  - Propagate errors from namev_resolve()
 *
 * Hints:
 *  - Use namev_resolve() to get the vnode corresponding to path.
 *  - Pay attention to refcounts!
 *  - Remember that p_cwd should not be locked upon return from this function.
 *  - (If doing MTP, must protect access to p_cwd)
 */
long do_chdir(const char *path)
{
    vnode_t* res;
    long status = namev_resolve(curproc->p_cwd, path, &res);
    if (status < 0) {
        return status;
    }
    if (!S_ISDIR(res->vn_mode)) {
        vput(&res);
        return -ENOTDIR;
    }
    vput(&curproc->p_cwd);
    curproc->p_cwd = res;
    return 0;
}

/*
 * Read a directory entry from the file specified by fd into dirp.
 *
 * Return sizeof(dirent_t) on success, or:
 *  - EBADF: fd is invalid or is not open
 *  - ENOTDIR: fd does not refer to a directory
 *  - Propagate errors from the vnode operation readdir
 *
 * Hints:
 *  - Use the vnode operation readdir.
 *  - Be sure to update file position according to readdir's return value.
 *  - On success (readdir return value is strictly positive), return
 *    sizeof(dirent_t).
 */
ssize_t do_getdent(int fd, struct dirent *dirp)
{
    if (fd < 0 || fd >= NFILES) {
        return -EBADF;
    }
    file_t* file = fget(fd);
    if (!file) {
        return -EBADF;
    }
    vnode_t* vnode = file->f_vnode;
    if (!S_ISDIR(vnode->vn_mode)) {
        fput(&file);
        return -ENOTDIR;
    }
    vlock(vnode);
    size_t read = vnode->vn_ops->readdir(vnode, file->f_pos, dirp);
    vunlock(vnode);
    if (read == 0) {
        fput(&file);
        return read;
    }
    file->f_pos = file->f_pos + read;
    fput(&file);
    return sizeof(dirent_t);
}

/*
 * Set the position of the file represented by fd according to offset and
 * whence.
 *
 * Return the new file position, or:
 *  - EBADF: fd is invalid or is not open
 *  - EINVAL: whence is not one of SEEK_SET, SEEK_CUR, or SEEK_END;
 *            or, the resulting file offset would be negative
 *
 * Hints:
 *  - See `man 2 lseek` for details about whence.
 *  - Be sure to protect the vnode if you have to access its vn_len.
 */
off_t do_lseek(int fd, off_t offset, int whence)
{
    if (fd < 0 || fd >= NFILES) {
        return -EBADF;
    }
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        return -EINVAL;
    }
    file_t* file = curproc->p_files[fd];
    // QUESTION: how to check if file is open?
    if (!file) {
        return -EBADF;
    }
    if (whence == SEEK_CUR) {
        if ((int) file->f_pos + offset < 0) {
            return -EINVAL;
        }
        file->f_pos = file->f_pos + offset;
    }
    if (whence == SEEK_SET) {
        if (offset < 0) {
            return -EINVAL;
        }
        file->f_pos = offset;
    }
    if (whence == SEEK_END) {
        vnode_t* node = file->f_vnode;
        vlock(node);
        if ((int) node->vn_len + offset < 0) {
            vunlock(node);
            return -EINVAL;
        }
        file->f_pos = node->vn_len + offset;
        vunlock(node);
    }
    return file->f_pos;
}

/* Use buf to return the status of the file represented by path.
 *
 * Return 0 on success, or:
 *  - Propagate errors from namev_resolve() and the vnode operation stat.
 */
long do_stat(const char *path, stat_t *buf)
{
    vnode_t* res_vnode;
    long open_status = namev_resolve(curproc->p_cwd, path, &res_vnode);
    if (open_status < 0) {
        return open_status;
    }
    long stat_status = res_vnode->vn_ops->stat(res_vnode, buf);
    vput(&res_vnode);
    if (stat_status < 0) {
        return stat_status;
    }
    return 0;
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int do_mount(const char *source, const char *target, const char *type)
{
    NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
    return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not
 * worry about freeing the fs_t struct here, that is done in vfs_umount. All
 * this function does is figure out which file system to pass to vfs_umount and
 * do good error checking.
 */
int do_umount(const char *target)
{
    NOT_YET_IMPLEMENTED("MOUNTING: do_unmount");
    return -EINVAL;
}
#endif
