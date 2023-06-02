/*
 *  FILE: vnode.h
 *  AUTH: mcc
 *  DESC:
 *  DATE: Fri Mar 13 18:54:11 1998
 *  $Id: vnode.h,v 1.2.2.2 2006/06/04 01:02:32 afenn Exp $
 */

#pragma once

#include "drivers/blockdev.h"
#include "drivers/chardev.h"
#include "drivers/dev.h"
#include "mm/mobj.h"
#include "mm/pframe.h"
#include "proc/kmutex.h"
#include "util/list.h"

struct fs;
struct dirent;
struct stat;
struct file;
struct vnode;
struct kmutex;

#define VNODE_LOADING 0
#define VNODE_LOADED 1

typedef struct vnode_ops
{
    /* The following functions map directly to their corresponding
     * system calls. Unless otherwise noted, they return 0 on
     * success, and -errno on failure.
     */

    /* Operations that can be performed on non-directory files: */
    /*
     * read transfers at most count bytes from file into buf. It
     * begins reading from the file at pos bytes into the file. On
     * success, it returns the number of bytes transferred, or 0 if the
     * end of the file has been reached (pos >= file->vn_len).
     */
    ssize_t (*read)(struct vnode *file, size_t pos, void *buf, size_t count);

    /*
     * write transfers count bytes from buf into file. It begins
     * writing at pos bytes into the file. If offset+count extends
     * past the end of the file, the file's length will be increased.
     * If offset is before the end of the file, the existing data is
     * overwritten. On success, it returns the number of bytes
     * transferred.
     */
    ssize_t (*write)(struct vnode *file, size_t pos, const void *buf,
                     size_t count);

    /*
     * Implementations should supply an mobj through the "ret"
     * argument (not by setting vma->vma_obj). If for any reason
     * this cannot be done an appropriate error code should be
     * returned instead.
     */
    long (*mmap)(struct vnode *file, struct mobj **ret);

    /* Operations that can be performed on directory files: */

    /*
     * mknod creates a special specified by name and namelen in the
     * directory pointed to by dir with the specified mode and devid.
     *
     * Upon success, ret must point to the newly created file.
     */
    long (*mknod)(struct vnode *dir, const char *name, size_t namelen, int mode,
                  devid_t devid, struct vnode **ret);

    /*
     * lookup attempts to find the file specified by name and namelen in the
     * directory pointed to by dir.
     *
     * Upon success, ret must point to the child vnode.
     */
    long (*lookup)(struct vnode *dir, const char *name, size_t namelen,
                   struct vnode **out);

    /*
     * Creates a directory entry in dir specified by name and namelen pointing
     * to the inode of target.
     */
    long (*link)(struct vnode *dir, const char *name, size_t namelen,
                 struct vnode *target);

    /*
     * unlink removes the directory entry in dir corresponding to the file
     * specified by name and namelen.
     */
    long (*unlink)(struct vnode *dir, const char *name, size_t namelen);

    /*
     * rename
     */
    long (*rename)(struct vnode *olddir, const char *oldname, size_t oldnamelen,
                   struct vnode *newdir, const char *newname,
                   size_t newnamelen);

    /*
     * mkdir creates a directory specified by name and namelen in the
     * directory pointed to by out.
     *
     * Upon success, out must point to the newly created directory.
     * Upon failure, out must be unchanged.
     */
    long (*mkdir)(struct vnode *dir, const char *name, size_t namelen,
                  struct vnode **out);

    /*
     * rmdir removes the directory specified by name and namelen from dir.
     * The directory to be removed must be empty: the only directory entries
     * must be "." and "..".
     */
    long (*rmdir)(struct vnode *dir, const char *name, size_t namelen);

    /*
     * readdir reads one directory entry from the dir into the struct
     * dirent. On success, it returns the amount that offset should be
     * increased by to obtain the next directory entry with a
     * subsequent call to readdir. If the end of the file as been
     * reached (offset == file->vn_len), no directory entry will be
     * read and 0 will be returned.
     */
    ssize_t (*readdir)(struct vnode *dir, size_t pos, struct dirent *d);

    /* Operations that can be performed on any type of "file" (
     * includes normal file, directory, block/byte device */
    /*
     * stat sets the fields in the given buf, filling it with
     * information about file.
     */
    long (*stat)(struct vnode *vnode, struct stat *buf);

    /*
     * acquire is called on a vnode when a file takes its first
     * reference to the vnode. The file is passed in.
     */
    long (*acquire)(struct vnode *vnode, struct file *file);

    /*
     * release is called on a vnode when the refcount of a file
     * descriptor that has it open comes down to 0. Each call to
     * acquire has exactly one matching call to release with the
     * same file that was passed to acquire.
     */
    long (*release)(struct vnode *vnode, struct file *file);

    long (*get_pframe)(struct vnode *vnode, size_t pagenum, long forwrite,
                       pframe_t **pfp);

    /*
     * Read the page of 'vnode' containing 'offset' into the
     * page-aligned and page-sized buffer pointed to by
     * 'buf'.
     */
    long (*fill_pframe)(struct vnode *vnode, pframe_t *pf);

    /*
     * Write the contents of the page-aligned and page-sized
     * buffer pointed to by 'buf' to the page of 'vnode'
     * containing 'offset'.
     */
    long (*flush_pframe)(struct vnode *vnode, pframe_t *pf);

    /*
    * This will truncate the file to have a length of zero
    * Should only be used on regular files, not directories. 
    */
    void (*truncate_file)(struct vnode *vnode);
} vnode_ops_t;

typedef struct vnode
{
    /*
     * Function pointers to the implementations of file operations (the
     * functions are provided by the filesystem implementation).
     */
    struct vnode_ops *vn_ops;

    /*
     * The filesystem to which this vnode belongs. This is initialized by
     * the VFS subsystem when the vnode is first created and should never
     * change.
     */
    struct fs *vn_fs;

#ifdef __MOUNTING__
    /* This field is used only for implementing mount points (not required) */
    /* This field points the the root of the file system mounted at
     * this vnode. If no file system is mounted at this point this is a
     * self pointer (i.e. vn->vn_mount = vn). See vget for why this is
     * makes things easier for us. */
    struct vnode *vn_mount;
#endif

    /*
     * The object responsible for managing the memory where pages read
     * from this file reside. The VFS subsystem may use this field, but it
     * does not need to create it.
     */
    struct mobj vn_mobj;

    /*
     * A number which uniquely identifies this vnode within its filesystem.
     * (Similar and usually identical to what you might know as the inode
     * number of a file).
     */
    ino_t vn_vno;

    /*
     * File type. See stat.h.
     */
    int vn_mode;

    /*
     * Length of file. Initialized at the fs-implementation-level (in the
     * 'read_vnode' fs_t entry point). Maintained at the filesystem
     * implementation level (within the implementations of relevant vnode
     * entry points).
     */
    size_t vn_len;

    /*
     * A generic pointer which the file system can use to store any extra
     * data it needs.
     */
    void *vn_i;

    /*
     * The device identifier.
     * Only relevant to vnodes representing device files.
     */
    devid_t vn_devid;

    /*
     * The state of the vnode. Can either be loading or loaded. The vnode
     * cannot be used until the vnode is in the loaded state. Potential
     * users should wait on `vn_waitq` if the vnode is being loaded.
     * This field is protected by the 'vn_state_lock'.
     */
    int vn_state;

    /*
     * Protects the `vn_state` field.
     */
    spinlock_t vn_state_lock;

    /*
     * Allows vnode users to wait on the vnode, until the vnode is ready.
     */
    ktqueue_t vn_waitq;

    union {
        chardev_t *chardev;
        blockdev_t *blockdev;
    } vn_dev;

    /* Used (only) by the v{get,ref,put} facilities (vfs/vnode.c): */
    list_link_t vn_link; /* link on system vnode list */
} vnode_t;

void init_special_vnode(vnode_t *vn);

/* Core vnode management routines: */
/*
 *     Obtain a vnode representing the file that filesystem 'fs' identifies
 *     by inode number 'vnum'; returns the vnode_t corresponding to the
 *     given filesystem and vnode number.  If a vnode for the given file
 *     already exists (it already has an entry in the system inode table) then
 *     the reference count of that vnode is incremented and it is returned.
 *     Otherwise a new vnode is created in the system inode table with a
 *     reference count of 1.
 *     This function has no unsuccessful return.
 *
 *     MAY BLOCK.
 */
struct vnode *vget(struct fs *fs, ino_t vnum);

/*
 * Lock a vnode (locks vn_mobj). 
 */
void vlock(vnode_t *vn);

/*
 * Lock two vnodes in order! This prevents the A/B locking problem when locking
 * two directories or two files.
 */
void vlock_in_order(vnode_t *a, vnode_t *b);

/*
 * Acquires a vnode locked (see vget above)
 */
vnode_t *vget_locked(struct fs *fs, ino_t ino);

/**
 * Unlock and put a vnode (see vput)
 */
void vput_locked(struct vnode **vnp);

/**
 * Unlocks a vnode
 */
void vunlock(vnode_t *vn);

/**
 * Unlocks two vnodes (effectively just 2 unlocks)
 */
void vunlock_in_order(vnode_t *a, vnode_t *b);

/*
 * Increments the reference count of the provided vnode
 * (i.e. the refcount of vn_mobj). 
 */
void vref(vnode_t *vn);

/*
 * This function decrements the reference count on this vnode 
 * (i.e. the refcount of vn_mobj).
 *
 * If, as a result of this, refcount reaches zero, the underlying
 * fs's 'delete_vnode' entry point will be called and the vnode will be
 * freed.
 *
 * If the linkcount of the corresponding on inode on the filesystem is zero,
 * then the inode will be freed.
 *
 */
void vput(vnode_t **vnp);

/* Auxilliary: */

/* Unmounting (shutting down the VFS) is the primary reason for the
 * existence of the following three routines (when unmounting an s5 fs,
 * they are used in the order that they are listed here): */
/*
 *     Checks to see if there are any actively-referenced vnodes
 *     belonging to the specified filesystem.
 *     Returns -EBUSY if there is at least one such actively-referenced
 *     vnode, and 0 otherwise.
 *
 */
long vfs_is_in_use(struct fs *fs);

/*
 * Returns the number of vnodes from this filesystem that are in
 * use.
 */
size_t vfs_count_active_vnodes(struct fs *fs);

/* Diagnostic: */
/*
 * Prints the vnodes that are in use. Specifying a fs_t will restrict
 * the vnodes to just that fs. Specifying NULL will print all vnodes
 * in the entire system.
 * 
 * Note that this is currently unimplemented.
 */
void vnode_print(struct fs *fs);
