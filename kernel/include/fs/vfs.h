#pragma once

#include "types.h"

#include "fs/open.h"
#include "proc/kmutex.h"
#include "util/list.h"

struct vnode;
struct file;
struct vfs;
struct fs;
struct slab_allocator;

/* name_match: fname should be null-terminated, name is namelen long */
#define name_match(fname, name, namelen) \
    (strlen(fname) == namelen && !strncmp((fname), (name), (namelen)))

typedef struct fs_ops
{
    /*
     * Initialize vn_ops, vn_mode, vn_devid and vn_len.
     * If the filesystem wishes, it may initialize and use vn_i.
     */
    void (*read_vnode)(struct fs *fs, struct vnode *vn);

    /*
     * Called when the vnode's reference count drops to 0.
     * Perform any necessary cleanup for the corresponding inode.
     */
    void (*delete_vnode)(struct fs *fs, struct vnode *vn);

    /*
     * Optional. Default behavior is to vput() fs_root.
     * Unmount the filesystem, performing any desired sanity checks
     * and/or necessary cleanup.
     * Return 0 on success; negative number on error.
     */
    long (*umount)(struct fs *fs);

    void (*sync)(struct fs *fs);
} fs_ops_t;

#ifndef STR_MAX
#define STR_MAX 32
#endif

/* similar to Linux's super_block. */
typedef struct fs
{
    /*
     * The string name of the device from which this file system should
     * be mounted. This may be used by the mount function of some file
     * systems which need to know which device they are mounting.
     */
    char fs_dev[STR_MAX];
    /*
     * The type of file system this structure represents (given as a
     * well-defined string). This is used by the generic VFS mount
     * function to decide which filesystem-specific mount function to
     * call.  Valid values are hard-coded in vfs.c.
     */
    char fs_type[STR_MAX];

#ifdef __MOUNTING__
    /*
     * If mounting is implemented then this should point to the vnode
     * of the file that this file system is mounted on. For the root file
     * system this will just point to the root of that file system.
     */
    struct vnode *fs_mtpt;

    /*
     * An identifier for the mounted file system. This should be enlisted
     * by the the kernel to keep track of all mounted file systems.
     */
    list_link_t fs_link;
#endif

    /*
     * The following members are initialized by the filesystem
     * implementation's mount routine:
     */

    /*
     * The struct of operations that define which filesystem-specific
     * functions to call to perform filesystem manipulation.
     */
    fs_ops_t *fs_ops;

    /*
     * The root vnode for this filesystem (not to be confused with
     * either / (the root of VFS) or the vnode where the filesystem is
     * mounted, which is on a different file system.
     */
    struct vnode *fs_root;

    /* Filesystem-specific data. */
    void *fs_i;

    struct slab_allocator *fs_vnode_allocator;
    list_t vnode_list;
    kmutex_t vnode_list_mutex;
    kmutex_t vnode_rename_mutex;

} fs_t;

/* - this is the vnode on which we will mount the vfsroot fs.
 */
extern fs_t vfs_root_fs;

void do_sync();

/* VFS {{{ */
/*
 * - called by the init process at system shutdown
 * - at this point, init process is the only process running
 *     => so, there should be no "live" vnodes
 *
 * unmount the root filesystem (and first unmount any filesystems mounted
 * on the root filesystem in the proper order (bottom up)).
 *
 */

/* VFS }}} */
/* VFS Shutdown: */
/*
 *    Called by the init process at system shutdown.
 *     
 *    At this point, the init process is the only process running
 *     => so, there should be no "live" vnodes
 */
long vfs_shutdown();

/* Pathname resolution: */
/* (the corresponding definitions live in namev.c) */
long namev_lookup(struct vnode *dir, const char *name, size_t namelen,
                  struct vnode **out);

long namev_dir(struct vnode *base, const char *path, struct vnode **res_vnode,
               const char **name, size_t *namelen);

long namev_open(struct vnode *base, const char *path, int oflags, int mode,
                devid_t devid, struct vnode **res_vnode);

long namev_resolve(struct vnode *base, const char *path,
                   struct vnode **res_vnode);

long namev_get_child(struct vnode *dir, char *name, size_t namelen,
                     struct vnode **out);

long namev_get_parent(struct vnode *dir, struct vnode **out);

long namev_is_descendant(struct vnode *a, struct vnode *b);

#ifdef __GETCWD__
long lookup_name(struct vnode *dir, struct vnode *entry, char *buf,
                 size_t size);
long lookup_dirpath(struct vnode *dir, char *buf, size_t size);
#endif /* __GETCWD__ */

long mountfunc(fs_t *fs);
