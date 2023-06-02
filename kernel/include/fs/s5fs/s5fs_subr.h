/*
 *   FILE: s5fs_subr.h
 * AUTHOR: afenn
 *  DESCR: S5 low-level subroutines
 */

#pragma once

#include "types.h"

struct s5fs;
struct s5_node;

long s5_alloc_inode(struct s5fs *s5fs, uint16_t type, devid_t devid);

void s5_free_inode(struct s5fs *s5fs, ino_t ino);

ssize_t s5_read_file(struct s5_node *vn, size_t pos, char *buf, size_t len);

ssize_t s5_write_file(struct s5_node *vn, size_t pos, const char *buf,
                      size_t len);

long s5_link(struct s5_node *dir, const char *name, size_t namelen,
             struct s5_node *child);

long s5_find_dirent(struct s5_node *dir, const char *name, size_t namelen,
                    size_t *filepos);

void s5_remove_dirent(struct s5_node *dir, const char *name, size_t namelen,
                      struct s5_node *ent);

void s5_replace_dirent(struct s5_node *sn, const char *name, size_t namelen,
                       struct s5_node *old, struct s5_node *new);

long s5_file_block_to_disk_block(struct s5_node *sn, size_t file_blocknum,
                                 int alloc);

long s5_inode_blocks(struct s5_node *vnode);

void s5_remove_blocks(struct s5_node *vnode);

/* Converts a vnode_t* to the s5fs_t* (s5fs file system) struct */
#define VNODE_TO_S5FS(vn) ((s5fs_t *)((vn)->vn_fs->fs_i))

/* Converts an s5fs_t* to its memory object (the memory object of the block device) */
#define S5FS_TO_VMOBJ(s5fs) (&(s5fs)->s5f_bdev->bd_mobj)