/*
 * This is a special filesystem designed to be a test filesystem before s5fs has
 * been written.  It is an in-memory filesystem that supports almost all of the
 * vnode operations.  It has the following restrictions:
 *
 *    o File sizes are limited to a single page (4096 bytes) in order
 *      to keep the code simple.
 *
 *    o There is no support for fill_pframe, etc. 
 *
 *    o There is a maximum directory size limit
 *
 *    o There is a maximum number of files/directories limit
 */

#include "fs/ramfs/ramfs.h"
#include "errno.h"
#include "fs/dirent.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "globals.h"
#include "kernel.h"
#include "mm/kmalloc.h"
#include "mm/slab.h"
#include "util/debug.h"
#include "util/string.h"

/*
 * Filesystem operations
 */
static void ramfs_read_vnode(fs_t *fs, vnode_t *vn);

static void ramfs_delete_vnode(fs_t *fs, vnode_t *vn);

static long ramfs_umount(fs_t *fs);

static fs_ops_t ramfs_ops = {.read_vnode = ramfs_read_vnode,
                             .delete_vnode = ramfs_delete_vnode,
                             .umount = ramfs_umount};

/*
 * vnode operations
 */
static ssize_t ramfs_read(vnode_t *file, size_t offset, void *buf,
                          size_t count);

static ssize_t ramfs_write(vnode_t *file, size_t offset, const void *buf,
                           size_t count);

/* getpage */
static ssize_t ramfs_create(vnode_t *dir, const char *name, size_t name_len,
                            vnode_t **result);

static ssize_t ramfs_mknod(struct vnode *dir, const char *name, size_t name_len,
                           int mode, devid_t devid, struct vnode **out);

static ssize_t ramfs_lookup(vnode_t *dir, const char *name, size_t namelen,
                            vnode_t **out);

static long ramfs_link(vnode_t *dir, const char *name, size_t namelen,
                       vnode_t *child);

static ssize_t ramfs_unlink(vnode_t *dir, const char *name, size_t name_len);

static ssize_t ramfs_rename(vnode_t *olddir, const char *oldname,
                            size_t oldnamelen, vnode_t *newdir,
                            const char *newname, size_t newnamelen);

static ssize_t ramfs_mkdir(vnode_t *dir, const char *name, size_t name_len,
                           struct vnode **out);

static ssize_t ramfs_rmdir(vnode_t *dir, const char *name, size_t name_len);

static ssize_t ramfs_readdir(vnode_t *dir, size_t offset, struct dirent *d);

static ssize_t ramfs_stat(vnode_t *file, stat_t *buf);

static void ramfs_truncate_file(vnode_t *file);

static vnode_ops_t ramfs_dir_vops = {.read = NULL,
                                     .write = NULL,
                                     .mmap = NULL,
                                     .mknod = ramfs_mknod,
                                     .lookup = ramfs_lookup,
                                     .link = ramfs_link,
                                     .unlink = ramfs_unlink,
                                     .rename = ramfs_rename,
                                     .mkdir = ramfs_mkdir,
                                     .rmdir = ramfs_rmdir,
                                     .readdir = ramfs_readdir,
                                     .stat = ramfs_stat,
                                     .acquire = NULL,
                                     .release = NULL,
                                     .get_pframe = NULL,
                                     .fill_pframe = NULL,
                                     .flush_pframe = NULL,
                                     .truncate_file = NULL};

static vnode_ops_t ramfs_file_vops = {.read = ramfs_read,
                                      .write = ramfs_write,
                                      .mmap = NULL,
                                      .mknod = NULL,
                                      .lookup = NULL,
                                      .link = NULL,
                                      .unlink = NULL,
                                      .mkdir = NULL,
                                      .rmdir = NULL,
                                      .stat = ramfs_stat,
                                      .acquire = NULL,
                                      .release = NULL,
                                      .get_pframe = NULL,
                                      .fill_pframe = NULL,
                                      .flush_pframe = NULL,
                                      .truncate_file = ramfs_truncate_file};

/*
 * The ramfs 'inode' structure
 */
typedef struct ramfs_inode
{
    size_t rf_size;       /* Total file size */
    ino_t rf_ino;         /* Inode number */
    char *rf_mem;         /* Memory for this file (1 page) */
    ssize_t rf_mode;      /* Type of file */
    ssize_t rf_linkcount; /* Number of links to this file */
} ramfs_inode_t;

#define RAMFS_TYPE_DATA 0
#define RAMFS_TYPE_DIR 1
#define RAMFS_TYPE_CHR 2
#define RAMFS_TYPE_BLK 3

#define VNODE_TO_RAMFSINODE(vn) ((ramfs_inode_t *)(vn)->vn_i)
#define VNODE_TO_RAMFS(vn) ((ramfs_t *)(vn)->vn_fs->fs_i)
#define VNODE_TO_DIRENT(vn) ((ramfs_dirent_t *)VNODE_TO_RAMFSINODE(vn)->rf_mem)

/*
 * ramfs filesystem structure
 */
#define RAMFS_MAX_FILES 64

typedef struct ramfs
{
    ramfs_inode_t *rfs_inodes[RAMFS_MAX_FILES]; /* Array of all files */
} ramfs_t;

/*
 * For directories, we simply store an array of (ino, name) pairs in the
 * memory portion of the inode.
 */
typedef struct ramfs_dirent
{
    ssize_t rd_ino;         /* Inode number of this entry */
    char rd_name[NAME_LEN]; /* Name of this entry */
} ramfs_dirent_t;

#define RAMFS_MAX_DIRENT ((size_t)(PAGE_SIZE / sizeof(ramfs_dirent_t)))

/* Helper functions */
static ssize_t ramfs_alloc_inode(fs_t *fs, ssize_t type, devid_t devid)
{
    ramfs_t *rfs = (ramfs_t *)fs->fs_i;
    KASSERT((RAMFS_TYPE_DATA == type) || (RAMFS_TYPE_DIR == type) ||
            (RAMFS_TYPE_CHR == type) || (RAMFS_TYPE_BLK == type));
    /* Find a free inode */
    ssize_t i;
    for (i = 0; i < RAMFS_MAX_FILES; i++)
    {
        if (NULL == rfs->rfs_inodes[i])
        {
            ramfs_inode_t *inode;
            if (NULL == (inode = kmalloc(sizeof(ramfs_inode_t))))
            {
                return -ENOSPC;
            }

            if (RAMFS_TYPE_CHR == type || RAMFS_TYPE_BLK == type)
            {
                /* Don't need any space in memory, so put devid in here */
                inode->rf_mem = (char *)(uint64_t)devid;
            }
            else
            {
                /* We allocate space for the file's contents immediately */
                if (NULL == (inode->rf_mem = page_alloc()))
                {
                    kfree(inode);
                    return -ENOSPC;
                }
                memset(inode->rf_mem, 0, PAGE_SIZE);
            }
            inode->rf_size = 0;
            inode->rf_ino = i;
            inode->rf_mode = type;
            inode->rf_linkcount = 1;

            /* Install in table and return */
            rfs->rfs_inodes[i] = inode;
            return i;
        }
    }
    return -ENOSPC;
}

/*
 * Function implementations
 */

long ramfs_mount(struct fs *fs)
{
    /* Allocate filesystem */
    ramfs_t *rfs = kmalloc(sizeof(ramfs_t));
    if (NULL == rfs)
    {
        return -ENOMEM;
    }

    memset(rfs->rfs_inodes, 0, sizeof(rfs->rfs_inodes));

    fs->fs_i = rfs;
    fs->fs_ops = &ramfs_ops;

    /* Set up root inode */
    ssize_t root_ino;
    if (0 > (root_ino = ramfs_alloc_inode(fs, RAMFS_TYPE_DIR, 0)))
    {
        return root_ino;
    }

    slab_allocator_t *allocator =
        slab_allocator_create("ramfs_node", sizeof(vnode_t));
    fs->fs_vnode_allocator = allocator;
    KASSERT(allocator);

    KASSERT(0 == root_ino);
    ramfs_inode_t *root = rfs->rfs_inodes[root_ino];

    /* Set up '.' and '..' in the root directory */
    ramfs_dirent_t *rootdent = (ramfs_dirent_t *)root->rf_mem;
    rootdent->rd_ino = 0;
    strcpy(rootdent->rd_name, ".");
    rootdent++;
    rootdent->rd_ino = 0;
    strcpy(rootdent->rd_name, "..");

    /* Increase root inode size accordingly */
    root->rf_size = 2 * sizeof(ramfs_dirent_t);

    /* Put the root in the inode table */
    rfs->rfs_inodes[0] = root;

    /* And vget the root vnode */
    fs->fs_root = vget(fs, 0);

    return 0;
}

static void ramfs_read_vnode(fs_t *fs, vnode_t *vn)
{
    ramfs_t *rfs = VNODE_TO_RAMFS(vn);
    ramfs_inode_t *inode = rfs->rfs_inodes[vn->vn_vno];
    KASSERT(inode && inode->rf_ino == vn->vn_vno);

    inode->rf_linkcount++;

    vn->vn_i = inode;
    vn->vn_len = inode->rf_size;

    switch (inode->rf_mode)
    {
    case RAMFS_TYPE_DATA:
        vn->vn_mode = S_IFREG;
        vn->vn_ops = &ramfs_file_vops;
        break;
    case RAMFS_TYPE_DIR:
        vn->vn_mode = S_IFDIR;
        vn->vn_ops = &ramfs_dir_vops;
        break;
    case RAMFS_TYPE_CHR:
        vn->vn_mode = S_IFCHR;
        vn->vn_ops = NULL;
        vn->vn_devid = (devid_t)(uint64_t)(inode->rf_mem);
        break;
    case RAMFS_TYPE_BLK:
        vn->vn_mode = S_IFBLK;
        vn->vn_ops = NULL;
        vn->vn_devid = (devid_t)(uint64_t)(inode->rf_mem);
        break;
    default:
        panic("inode %ld has unknown/invalid type %ld!!\n",
              (ssize_t)vn->vn_vno, (ssize_t)inode->rf_mode);
    }
}

static void ramfs_delete_vnode(fs_t *fs, vnode_t *vn)
{
    ramfs_inode_t *inode = VNODE_TO_RAMFSINODE(vn);
    ramfs_t *rfs = VNODE_TO_RAMFS(vn);

    if (0 == --inode->rf_linkcount)
    {
        KASSERT(rfs->rfs_inodes[vn->vn_vno] == inode);

        rfs->rfs_inodes[vn->vn_vno] = NULL;
        if (inode->rf_mode == RAMFS_TYPE_DATA ||
            inode->rf_mode == RAMFS_TYPE_DIR)
        {
            page_free(inode->rf_mem);
        }
        /* otherwise, inode->rf_mem is a devid */

        kfree(inode);
    }
}

static ssize_t ramfs_umount(fs_t *fs)
{
    /* We don't need to do any flushing or anything as everything is in memory.
     * Just free all of our allocated memory */
    ramfs_t *rfs = (ramfs_t *)fs->fs_i;

    vput(&fs->fs_root);

    /* Free all the inodes */
    ssize_t i;
    for (i = 0; i < RAMFS_MAX_FILES; i++)
    {
        if (NULL != rfs->rfs_inodes[i])
        {
            if (NULL != rfs->rfs_inodes[i]->rf_mem &&
                (rfs->rfs_inodes[i]->rf_mode == RAMFS_TYPE_DATA ||
                 rfs->rfs_inodes[i]->rf_mode == RAMFS_TYPE_DIR))
            {
                page_free(rfs->rfs_inodes[i]->rf_mem);
            }
            kfree(rfs->rfs_inodes[i]);
        }
    }

    return 0;
}

static ssize_t ramfs_create(vnode_t *dir, const char *name, size_t name_len,
                            vnode_t **result)
{
    vnode_t *vn;
    size_t i;
    ramfs_dirent_t *entry;

    /* Look for space in the directory */
    entry = VNODE_TO_DIRENT(dir);
    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (!entry->rd_name[0])
        {
            break;
        }
    }

    if (i == RAMFS_MAX_DIRENT)
    {
        return -ENOSPC;
    }

    /* Allocate an inode */
    ssize_t ino;
    if (0 > (ino = ramfs_alloc_inode(dir->vn_fs, RAMFS_TYPE_DATA, 0)))
    {
        return ino;
    }

    /* Get a vnode, set entry in directory */
    vn = vget(dir->vn_fs, (ino_t)ino);

    entry->rd_ino = vn->vn_vno;
    strncpy(entry->rd_name, name, MIN(name_len, NAME_LEN - 1));
    entry->rd_name[MIN(name_len, NAME_LEN - 1)] = '\0';

    VNODE_TO_RAMFSINODE(dir)->rf_size += sizeof(ramfs_dirent_t);

    *result = vn;

    return 0;
}

static ssize_t ramfs_mknod(struct vnode *dir, const char *name, size_t name_len,
                           int mode, devid_t devid, struct vnode **out)
{
    size_t i;
    ramfs_dirent_t *entry;

    /* Look for space in the directory */
    entry = VNODE_TO_DIRENT(dir);
    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (!entry->rd_name[0])
        {
            break;
        }
    }

    if (i == RAMFS_MAX_DIRENT)
    {
        return -ENOSPC;
    }

    ssize_t ino;
    if (S_ISCHR(mode))
    {
        ino = ramfs_alloc_inode(dir->vn_fs, RAMFS_TYPE_CHR, devid);
    }
    else if (S_ISBLK(mode))
    {
        ino = ramfs_alloc_inode(dir->vn_fs, RAMFS_TYPE_BLK, devid);
    }
    else if (S_ISREG(mode))
    {
        ino = ramfs_alloc_inode(dir->vn_fs, RAMFS_TYPE_DATA, devid);
    }
    else
    {
        panic("Invalid mode!\n");
    }

    if (ino < 0)
    {
        return ino;
    }

    /* Set entry in directory */
    entry->rd_ino = ino;
    strncpy(entry->rd_name, name, MIN(name_len, NAME_LEN - 1));
    entry->rd_name[MIN(name_len, NAME_LEN - 1)] = '\0';

    VNODE_TO_RAMFSINODE(dir)->rf_size += sizeof(ramfs_dirent_t);

    vnode_t *child = vget(dir->vn_fs, ino);

    dbg(DBG_VFS, "creating ino(%ld), vno(%d) with path: %s\n", ino,
        child->vn_vno, entry->rd_name);

    KASSERT(child);
    *out = child;
    return 0;
}

static ssize_t ramfs_lookup(vnode_t *dir, const char *name, size_t namelen,
                            vnode_t **out)
{
    size_t i;
    ramfs_inode_t *inode = VNODE_TO_RAMFSINODE(dir);
    ramfs_dirent_t *entry = (ramfs_dirent_t *)inode->rf_mem;

    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (name_match(entry->rd_name, name, namelen))
        {
            if (dir->vn_vno != entry->rd_ino)
            {
                fs_t *fs = (dir)->vn_fs;
                *out = vget(fs, entry->rd_ino);
            }
            else
            {
                vref(dir);
                *out = dir;
            }
            return 0;
        }
    }

    return -ENOENT;
}

static ssize_t ramfs_find_dirent(vnode_t *dir, const char *name,
                                 size_t namelen)
{
    size_t i;
    ramfs_inode_t *inode = VNODE_TO_RAMFSINODE(dir);
    ramfs_dirent_t *entry = (ramfs_dirent_t *)inode->rf_mem;

    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (name_match(entry->rd_name, name, namelen))
        {
            return entry->rd_ino;
        }
    }

    return -ENOENT;
}

static ssize_t ramfs_append_dirent(vnode_t *dir, const char *name,
                                   size_t namelen, vnode_t *child)
{
    vnode_t *vn;
    size_t i;
    ramfs_dirent_t *entry;

    KASSERT(child->vn_fs == dir->vn_fs);

    /* Look for space in the directory */
    entry = VNODE_TO_DIRENT(dir);
    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (name_match(entry->rd_name, name, namelen))
        {
            return -EEXIST;
        }

        if (!entry->rd_name[0])
        {
            break;
        }
    }

    if (i == RAMFS_MAX_DIRENT)
    {
        return -ENOSPC;
    }

    /* Set entry in parent */
    entry->rd_ino = child->vn_vno;
    strncpy(entry->rd_name, name, MIN(namelen, NAME_LEN - 1));
    entry->rd_name[MIN(namelen, NAME_LEN - 1)] = '\0';

    VNODE_TO_RAMFSINODE(dir)->rf_size += sizeof(ramfs_dirent_t);

    /* Increase linkcount */
    VNODE_TO_RAMFSINODE(child)->rf_linkcount++;

    return 0;
}

static ssize_t ramfs_delete_dirent(vnode_t *dir, const char *name,
                                   size_t namelen, vnode_t *child)
{
    int found = 0;
    size_t i;
    ramfs_dirent_t *entry = VNODE_TO_DIRENT(dir);
    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (name_match(entry->rd_name, name, namelen))
        {
            found = 1;
            entry->rd_name[0] = '\0';
            break;
        }
    }

    if (!found)
    {
        return -EEXIST;
    }

    VNODE_TO_RAMFSINODE(dir)->rf_size -= sizeof(ramfs_dirent_t);
    VNODE_TO_RAMFSINODE(child)->rf_linkcount--;

    return 0;
}

static long ramfs_link(vnode_t *dir, const char *name, size_t namelen,
                       vnode_t *child)
{
    return ramfs_append_dirent(dir, name, namelen, child);
}

static ssize_t ramfs_unlink(vnode_t *dir, const char *name, size_t namelen)
{
    ssize_t ret;
    size_t i;
    ramfs_dirent_t *entry;

    vnode_t *vn = dir;

    long ino = ramfs_find_dirent(dir, name, namelen);
    if (ino < 0)
    {
        return ino;
    }

    vnode_t *child = vget_locked(dir->vn_fs, (ino_t)ino);
    KASSERT(!S_ISDIR(child->vn_mode) && "handled at VFS level");

    ret = ramfs_delete_dirent(dir, name, namelen, child);
    KASSERT(ret == 0);

    vput_locked(&child);

    return 0;
}

static ssize_t ramfs_rename(vnode_t *olddir, const char *oldname,
                            size_t oldnamelen, vnode_t *newdir,
                            const char *newname, size_t newnamelen)
{
    long ino = ramfs_find_dirent(olddir, oldname, oldnamelen);
    if (ino < 0)
    {
        return ino;
    }

    vnode_t *oldvn = vget_locked(olddir->vn_fs, (ino_t)ino);
    if (S_ISDIR(oldvn->vn_mode))
    {
        vput_locked(&oldvn);
        return -EPERM;
    }
    if (S_ISDIR(oldvn->vn_mode))
    {
        vput_locked(&oldvn);
        return -EISDIR;
    }

    /* Determine if an entry corresponding to `newname` already exists */
    ino = ramfs_find_dirent(newdir, newname, newnamelen);
    if (ino != -ENOENT)
    {
        if (ino < 0)
        {
            return ino;
        }
        return -EEXIST;
    }

    ssize_t ret = ramfs_append_dirent(newdir, newname, newnamelen, oldvn);
    if (ret < 0)
    {
        vput_locked(&oldvn);
        return ret;
    }

    ret = ramfs_delete_dirent(olddir, oldname, oldnamelen, oldvn);
    vput_locked(&oldvn);

    return ret;
}

static ssize_t ramfs_mkdir(vnode_t *dir, const char *name, size_t name_len,
                           struct vnode **out)
{
    vnode_t *vn;
    size_t i;
    ramfs_dirent_t *entry;

    /* Look for space in the directory */
    entry = VNODE_TO_DIRENT(dir);
    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (!entry->rd_name[0])
        {
            break;
        }
    }

    if (i == RAMFS_MAX_DIRENT)
    {
        return -ENOSPC;
    }

    /* Allocate an inode */
    ssize_t ino;
    if (0 > (ino = ramfs_alloc_inode(dir->vn_fs, RAMFS_TYPE_DIR, 0)))
    {
        return ino;
    }

    /* Set entry in parent */
    entry->rd_ino = ino;
    strncpy(entry->rd_name, name, MIN(name_len, NAME_LEN - 1));
    entry->rd_name[MIN(name_len, NAME_LEN - 1)] = '\0';

    VNODE_TO_RAMFSINODE(dir)->rf_size += sizeof(ramfs_dirent_t);

    /* Set up '.' and '..' in the directory */
    entry = (ramfs_dirent_t *)VNODE_TO_RAMFS(dir)->rfs_inodes[ino]->rf_mem;
    entry->rd_ino = ino;
    strcpy(entry->rd_name, ".");
    entry++;
    entry->rd_ino = dir->vn_vno;
    strcpy(entry->rd_name, "..");

    /* Increase inode size accordingly */
    VNODE_TO_RAMFS(dir)->rfs_inodes[ino]->rf_size = 2 * sizeof(ramfs_dirent_t);

    /* This probably can't fail... (unless OOM :/) */
    *out = vget(dir->vn_fs, ino);

    return 0;
}

static ssize_t ramfs_rmdir(vnode_t *dir, const char *name, size_t name_len)
{
    ssize_t ret;
    size_t i;
    ramfs_dirent_t *entry;

    KASSERT(!name_match(".", name, name_len) &&
            !name_match("..", name, name_len));

    long ino = ramfs_find_dirent(dir, name, name_len);
    if (ino < 0)
    {
        return ino;
    }

    vnode_t *child = vget_locked(dir->vn_fs, (ino_t)ino);
    if (!S_ISDIR(child->vn_mode))
    {
        vput_locked(&child);
        return -ENOTDIR;
    }

    /* We have to make sure that this directory is empty */
    entry = VNODE_TO_DIRENT(child);
    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (!strcmp(entry->rd_name, ".") || !strcmp(entry->rd_name, ".."))
        {
            continue;
        }

        if (entry->rd_name[0])
        {
            vput_locked(&child);
            return -ENOTEMPTY;
        }
    }

    /* Finally, remove the entry from the parent directory */
    entry = VNODE_TO_DIRENT(dir);
    for (i = 0; i < RAMFS_MAX_DIRENT; i++, entry++)
    {
        if (name_match(entry->rd_name, name, name_len))
        {
            entry->rd_name[0] = '\0';
            break;
        }
    }
    VNODE_TO_RAMFSINODE(dir)->rf_size -= sizeof(ramfs_dirent_t);

    VNODE_TO_RAMFSINODE(child)->rf_linkcount--;
    vput_locked(&child);

    return 0;
}

static ssize_t ramfs_read(vnode_t *file, size_t offset, void *buf,
                          size_t count)
{
    ssize_t ret;
    ramfs_inode_t *inode = VNODE_TO_RAMFSINODE(file);

    KASSERT(!S_ISDIR(file->vn_mode));

    if (offset > inode->rf_size)
    {
        ret = 0;
    }
    else if (offset + count > inode->rf_size)
    {
        ret = inode->rf_size - offset;
    }
    else
    {
        ret = count;
    }

    memcpy(buf, inode->rf_mem + offset, ret);
    return ret;
}

static ssize_t ramfs_write(vnode_t *file, size_t offset, const void *buf,
                           size_t count)
{
    ssize_t ret;
    ramfs_inode_t *inode = VNODE_TO_RAMFSINODE(file);

    KASSERT(!S_ISDIR(file->vn_mode));

    ret = MIN((size_t)count, (size_t)PAGE_SIZE - offset);
    memcpy(inode->rf_mem + offset, buf, ret);

    KASSERT(file->vn_len == inode->rf_size);
    file->vn_len = MAX(file->vn_len, offset + ret);
    inode->rf_size = file->vn_len;

    return ret;
}

static ssize_t ramfs_readdir(vnode_t *dir, size_t offset, struct dirent *d)
{
    ssize_t ret = 0;
    ramfs_dirent_t *dir_entry, *targ_entry;

    KASSERT(S_ISDIR(dir->vn_mode));
    KASSERT(0 == offset % sizeof(ramfs_dirent_t));

    dir_entry = VNODE_TO_DIRENT(dir);
    dir_entry = (ramfs_dirent_t *)(((char *)dir_entry) + offset);
    targ_entry = dir_entry;

    while ((offset < (size_t)(RAMFS_MAX_DIRENT * sizeof(ramfs_dirent_t))) &&
           (!targ_entry->rd_name[0]))
    {
        ++targ_entry;
        offset += sizeof(ramfs_dirent_t);
    }

    if (offset >= (size_t)(RAMFS_MAX_DIRENT * sizeof(ramfs_dirent_t)))
    {
        return 0;
    }

    ret = sizeof(ramfs_dirent_t) +
          (targ_entry - dir_entry) * sizeof(ramfs_dirent_t);

    d->d_ino = targ_entry->rd_ino;
    d->d_off = 0; /* unused */
    strncpy(d->d_name, targ_entry->rd_name, NAME_LEN - 1);
    d->d_name[NAME_LEN - 1] = '\0';
    return ret;
}

static ssize_t ramfs_stat(vnode_t *file, stat_t *buf)
{
    ramfs_inode_t *i = VNODE_TO_RAMFSINODE(file);
    memset(buf, 0, sizeof(stat_t));
    buf->st_mode = file->vn_mode;
    buf->st_ino = (ssize_t)file->vn_vno;
    buf->st_dev = 0;
    if (file->vn_mode == S_IFCHR || file->vn_mode == S_IFBLK)
    {
        buf->st_rdev = (ssize_t)i->rf_mem;
    }
    buf->st_nlink = i->rf_linkcount - 1;
    buf->st_size = (ssize_t)i->rf_size;
    buf->st_blksize = (ssize_t)PAGE_SIZE;
    buf->st_blocks = 1;

    return 0;
}

static void ramfs_truncate_file(vnode_t *file)
{
    KASSERT(S_ISREG(file->vn_mode) && "This routine should only be called for regular files");
    ramfs_inode_t *i = VNODE_TO_RAMFSINODE(file);
    i->rf_size = 0;
    file->vn_len = 0;
    memset(i->rf_mem, 0, PAGE_SIZE);
}