#include "fs/s5fs/s5fs_subr.h"
#include "drivers/blockdev.h"
#include "errno.h"
#include "fs/s5fs/s5fs.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "kernel.h"
#include "mm/pframe.h"
#include "proc/kmutex.h"
#include "util/debug.h"
#include "util/string.h"
#include <fs/s5fs/s5fs.h>

static void s5_free_block(s5fs_t *s5fs, blocknum_t block);

static long s5_alloc_block(s5fs_t *s5fs);

static inline void s5_lock_super(s5fs_t *s5fs)
{
    kmutex_lock(&s5fs->s5f_mutex);
}

static inline void s5_unlock_super(s5fs_t *s5fs)
{
    kmutex_unlock(&s5fs->s5f_mutex);
}

/* Helper function to obtain inode info from disk given an inode number.
 *
 *  s5fs     - The file system (it will usually be obvious what to pass for this
 *             parameter)
 *  ino      - Inode number to fetch
 *  forwrite - Set if you intend to write any fields in the s5_inode_t, clear
 *             if you only intend to read
 *  pfp      - Return parameter for a page frame that will contain the disk
 *             block of the desired inode
 *  inodep   - Return parameter for the s5_inode_t corresponding to the desired
 *             inode
 */
static inline void s5_get_inode(s5fs_t *s5fs, ino_t ino, long forwrite,
                                pframe_t **pfp, s5_inode_t **inodep)
{
    s5_get_disk_block(s5fs, S5_INODE_BLOCK(ino), forwrite, pfp);
    *inodep = (s5_inode_t *)(*pfp)->pf_addr + S5_INODE_OFFSET(ino);
    KASSERT((*inodep)->s5_number == ino);
}

/* Release an inode by releasing the page frame of the disk block containing the
 * inode. See comments above s5_release_disk_block to see why we don't write
 * anything back yet.
 *
 *  pfp    - The page frame containing the inode
 *  inodep - The inode to be released
 *
 * On return, pfp and inodep both point to NULL.
 */
static inline void s5_release_inode(pframe_t **pfp, s5_inode_t **inodep)
{
    KASSERT((s5_inode_t *)(*pfp)->pf_addr +
                S5_INODE_OFFSET((*inodep)->s5_number) ==
            *inodep);
    *inodep = NULL;
    s5_release_disk_block(pfp);
}

/* Helper function to obtain a specific block of a file.
 *
 * sn       - The s5_node representing the file in question
 * blocknum - The offset of the desired block relative to the beginning of the
 *            file, i.e. index 8000 is block 1 of the file, even though it may
 *            not be block 1 of the disk
 * forwrite - Set if you intend to write to the block, clear if you only intend
 *            to read
 * pfp      - Return parameter for a page frame containing the block data
 */
static inline long s5_get_file_block(s5_node_t *sn, size_t blocknum,
                                     long forwrite, pframe_t **pfp)
{
    return sn->vnode.vn_mobj.mo_ops.get_pframe(&sn->vnode.vn_mobj, blocknum,
                                               forwrite, pfp);
}

/* Release the page frame associated with a file block. See comments above
 * s5_release_disk_block to see why we don't write anything back yet.
 *
 * On return, pfp points to NULL.
 */
static inline void s5_release_file_block(pframe_t **pfp)
{
    pframe_release(pfp);
}

/* Given a file and a file block number, return the disk block number of the
 * desired file block.
 *
 *  sn            - The s5_node representing the file
 *  file_blocknum - The offset of the desired block relative to the beginning of
 *                  the file
 *  alloc         - If set, allocate the block / indirect block as necessary
 *                  If clear, don't allocate sparse blocks
 *
 * Return a disk block number on success, or:
 *  - 0: The block is sparse, and alloc is clear, OR
 *       The indirect block would contain the block, but the indirect block is
 *       sparse, and alloc is clear
 *  - EINVAL: The specified block number is greater than or equal to 
 *            S5_MAX_FILE_BLOCKS
 *  - Propagate errors from s5_alloc_block.
 *
 * Hints:
 *  - Use the file inode's s5_direct_blocks and s5_indirect_block to perform the
 *    translation.
 *  - Use s5_alloc_block to allocate blocks.
 *  - Be sure to mark the inode as dirty when appropriate, i.e. when you are
 *    making changes to the actual s5_inode_t struct. Hint: Does allocating a
 *    direct block dirty the inode? What about allocating the indirect block?
 *    Finally, what about allocating a block pointed to by the indirect block?
 *  - Cases to consider:
 *    1) file_blocknum < S_NDIRECT_BLOCKS
 *    2) Indirect block is not allocated but alloc is set. Be careful not to
 *       leak a block in an error case!
 *    3) Indirect block is allocated. The desired block may be sparse, and you
 *       may have to allocate it.
 *    4) The indirect block has not been allocated and alloc is clear.
 */
long s5_file_block_to_disk_block(s5_node_t *sn, size_t file_blocknum,
                                 int alloc)
{
    if (file_blocknum >= S5_MAX_FILE_BLOCKS) {
        return -EINVAL;
    }
    s5_inode_t inode = sn->inode;
    if (file_blocknum < S5_NDIRECT_BLOCKS) { // checking direct blocks
        if (alloc && !sn->inode.s5_direct_blocks[file_blocknum]) {
            long allocated = s5_alloc_block(FS_TO_S5FS(sn->vnode.vn_fs));
            if (allocated < 0) {
                return allocated;
            }
            sn->inode.s5_direct_blocks[file_blocknum] = allocated;
            sn->dirtied_inode = 1;
        }
        return sn->inode.s5_direct_blocks[file_blocknum];
    }
    if (!sn->inode.s5_indirect_block) { // no indirect block
        if (!alloc) {
            return 0;
        }
        long allocated = s5_alloc_block(FS_TO_S5FS(sn->vnode.vn_fs));
        if (allocated < 0) {
            return allocated;
        }
        sn->inode.s5_indirect_block = allocated;
        pframe_t* pframe;
        s5_get_disk_block(FS_TO_S5FS(sn->vnode.vn_fs), sn->inode.s5_indirect_block, 1, &pframe);
        long block = s5_alloc_block(FS_TO_S5FS(sn->vnode.vn_fs));
        if (block < 0) {
            s5_release_disk_block(&pframe);
            sn->inode.s5_indirect_block = 0;
            s5_free_block(FS_TO_S5FS(sn->vnode.vn_fs), allocated);
            return block;
        }
        ((uint32_t *) pframe->pf_addr)[file_blocknum - S5_NDIRECT_BLOCKS] = block;
        s5_release_disk_block(&pframe);
        sn->dirtied_inode = 1;
    }
    pframe_t* pframe; // indirect block already allocated
    s5_get_disk_block(FS_TO_S5FS(sn->vnode.vn_fs), sn->inode.s5_indirect_block, alloc, &pframe);
    if (!((uint32_t *) pframe->pf_addr)[file_blocknum - S5_NDIRECT_BLOCKS] && alloc) {
        long block = s5_alloc_block(FS_TO_S5FS(sn->vnode.vn_fs));
        if (block < 0) {
            s5_release_disk_block(&pframe);
            return block;
        }
        ((uint32_t *) pframe->pf_addr)[file_blocknum - S5_NDIRECT_BLOCKS] = block;
    }
    long retval = ((uint32_t *) pframe->pf_addr)[file_blocknum - S5_NDIRECT_BLOCKS];
    s5_release_disk_block(&pframe);
    return retval;
}

/* Read from a file.
 *
 *  sn  - The s5_node representing the file to read from
 *  pos - The position to start reading from
 *  buf - The buffer to read into
 *  len - The number of bytes to read
 *
 * Return the number of bytes read, or:
 *  - Propagate errors from s5_get_file_block (do not return a partial
 *    read). As in, if s5_get_file_block returns an error, 
 *    the call to s5_read_file should fail.
 *
 * Hints:
 *  - Do not directly call s5_file_block_to_disk_block. To obtain pframes with
 *    the desired blocks, use s5_get_file_block and s5_release_file_block.
 *  - Be sure to handle all edge cases regarding pos and len relative to the
 *    length of the actual file. (If pos is greater than or equal to the length
 *    of the file, then s5_read_file should return 0). 
 */
ssize_t s5_read_file(s5_node_t *sn, size_t pos, char *buf, size_t len)
{
    int offset = S5_INODE_OFFSET(sn->inode.s5_number);
    blocknum_t blocknum = S5_DATA_BLOCK(pos);
    size_t bytes_read = 0;
    pframe_t* pframe;
    if (pos >= sn->vnode.vn_len) {
        return 0;
    }
    len = MIN(len, sn->vnode.vn_len - pos);
    while (bytes_read < len) {
        blocknum = S5_DATA_BLOCK(pos);
        size_t to_read = MIN(S5_BLOCK_SIZE - S5_DATA_OFFSET(pos), len - bytes_read);
        if (to_read + pos > sn->vnode.vn_len) {
            return bytes_read;
        }
        long status = s5_get_file_block(sn, blocknum, 0, &pframe);
        if (status < 0) {
            return status;
        }
        memcpy(buf + bytes_read, (char *) pframe->pf_addr + S5_DATA_OFFSET(pos), to_read);
        bytes_read += to_read;
        pos = pos + to_read;
        s5_release_file_block(&pframe);
    }
    return bytes_read;
}

/* Write to a file.
 *
 *  sn  - The s5_node representing the file to write to
 *  pos - The position to start writing to
 *  buf - The buffer to write from
 *  len - The number of bytes to write
 *
 * Return the number of bytes written, or:
 *  - EFBIG: pos was beyond S5_MAX_FILE_SIZE
 *  - Propagate errors from s5_get_file_block (that is, do not return a partial
 *    write)
 *
 * Hints:
 *  - You should return -EFBIG only if the provided pos was invalid. Otherwise,
 *    it is okay to make a partial write up to the maximum file size.
 *  - Use s5_get_file_block and s5_release_file_block to obtain pframes with
 *    the desired blocks.
 *  - Because s5_get_file_block calls s5fs_get_pframe, which checks the length
 *    of the vnode, you may have to update the vnode's length before you call
 *    s5_get_file_block. In this case, you should also update the inode's
 *    s5_size and mark the inode dirty.
 *  - If, midway through writing, you run into an error with s5_get_file_block,
 *    it is okay to merely undo your most recent changes while leaving behind
 *    writes you've already made to other blocks, before returning the error.
 *    That is, it is okay to make a partial write that the caller does not know
 *    about, as long as the file's length is consistent with what you've
 *    actually written so far.
 *  - You should maintain the vn_len of the vnode and the s5_un.s5_size field of the 
 *    inode to be the same. 
 */
ssize_t s5_write_file(s5_node_t *sn, size_t pos, const char *buf, size_t len)
{
    if (pos >= S5_MAX_FILE_SIZE) {
        return -EFBIG;
    }
    int offset = S5_INODE_OFFSET(sn->inode.s5_number);
    blocknum_t blocknum = S5_DATA_BLOCK(pos);
    size_t bytes_written = 0;
    pframe_t* pframe;
    if (pos + len > S5_MAX_FILE_SIZE) {
        len = S5_MAX_FILE_SIZE - pos;
    }
    while (bytes_written < len) {
        blocknum = S5_DATA_BLOCK(pos);
        size_t to_write = MIN(S5_BLOCK_SIZE - S5_DATA_OFFSET(pos), len - bytes_written);
        uint32_t previous_size = sn->vnode.vn_len;
        if (to_write + pos > sn->vnode.vn_len) {
            sn->vnode.vn_len = to_write + pos;
            sn->inode.s5_un.s5_size = to_write + pos;
            sn->dirtied_inode = 1;
        }
        long status = s5_get_file_block(sn, blocknum, 1, &pframe);
        if (status < 0) {
            sn->vnode.vn_len = previous_size;
            sn->inode.s5_un.s5_size = previous_size;
            return status;
        }
        memcpy((char *) pframe->pf_addr + S5_DATA_OFFSET(pos), buf + bytes_written, to_write);
        bytes_written += to_write;
        pos = pos + to_write;
        sn->dirtied_inode = 1;
        s5_release_file_block(&pframe);
    }
    return bytes_written;
}

/* Allocate one block from the filesystem.
 *
 * Return the block number of the newly allocated block, or:
 *  - ENOSPC: There are no more free blocks
 *
 * Hints:
 *  - Protect access to the super block using s5_lock_super and s5_unlock super.
 *  - Recall that the free block list is a linked list of blocks containing disk
 *    block numbers of free blocks. Each node contains S5_NBLKS_PER_FNODE block
 *    numbers, where the last entry is a pointer to the next node in the linked
 *    list, or -1 if there are no more free blocks remaining. The super block's
 *    s5s_free_blocks is the first node of this linked list.
 *  - The super block's s5s_nfree member is the number of blocks that are free
 *    within s5s_free_blocks. You could use it as an index into the
 *    s5s_free_blocks array. Be sure to update the field appropriately.
 *  - When s5s_free_blocks runs out (i.e. s5s_nfree == 0), refill it by
 *    collapsing the next node of the free list into the super block. Exactly
 *    when you do this is up to you.
 *  - You should initialize the block's contents to 0. Specifically, 
 *    when you use s5_alloc_block to allocate an indirect block,
 *    as your implementation of s5_file_block_to_disk_block probably expects
 *    sparse blocks to be represented by a 0.
 *  - You may find it helpful to take a look at the implementation of
 *    s5_free_block below.
 *  - You may assume/assert that any pframe calls succeed.
 */
static long s5_alloc_block(s5fs_t *s5fs)
{
    s5_lock_super(s5fs);
    s5_super_t *super = &s5fs->s5f_super;
    if (super->s5s_nfree == 0 && super->s5s_free_blocks[S5_NBLKS_PER_FNODE - 1] == (uint32_t) -1) {
        s5_unlock_super(s5fs);
        return -ENOSPC;
    }
    pframe_t *pf;
    if (super->s5s_nfree == 0) {
        s5_get_disk_block(s5fs, super->s5s_free_blocks[S5_NBLKS_PER_FNODE - 1], 1, &pf);
        long retval = super->s5s_free_blocks[S5_NBLKS_PER_FNODE - 1];
        super->s5s_nfree = S5_NBLKS_PER_FNODE - 1;
        memcpy(super->s5s_free_blocks, pf->pf_addr, sizeof(super->s5s_free_blocks));
        memset(pf->pf_addr, 0, PAGE_SIZE);
        s5_release_disk_block(&pf);
        s5_unlock_super(s5fs);
        return retval;
    } else {
        s5_get_disk_block(s5fs, super->s5s_free_blocks[super->s5s_nfree-1], 1, &pf);
        super->s5s_nfree -= 1;
    }
    memset(pf->pf_addr, 0, PAGE_SIZE);
    s5_release_disk_block(&pf);
    s5_unlock_super(s5fs);
    return super->s5s_free_blocks[super->s5s_nfree];
}

/*
 * The exact opposite of s5_alloc_block: add blockno to the free list of the
 * filesystem. This should never fail. You may assert that any pframe calls
 * succeed.
 *
 * Don't forget to protect access to the super block, update s5s_nfree, and
 * expand the linked list correctly if the super block can no longer hold any
 * more free blocks in its s5s_free_blocks array according to s5s_nfree.
 */
static void s5_free_block(s5fs_t *s5fs, blocknum_t blockno)
{
    s5_lock_super(s5fs);
    s5_super_t *s = &s5fs->s5f_super;
    dbg(DBG_S5FS, "freeing disk block %d\n", blockno);
    KASSERT(blockno);
    KASSERT(s->s5s_nfree < S5_NBLKS_PER_FNODE);

    pframe_t *pf;
    s5_get_disk_block(s5fs, blockno, 1, &pf);

    if (s->s5s_nfree == S5_NBLKS_PER_FNODE - 1)
    {
        memcpy(pf->pf_addr, s->s5s_free_blocks, sizeof(s->s5s_free_blocks));

        s->s5s_nfree = 0;
        s->s5s_free_blocks[S5_NBLKS_PER_FNODE - 1] = blockno;
    }
    else
    {
        s->s5s_free_blocks[s->s5s_nfree++] = blockno;
        pf->pf_dirty = 0;
    }
    s5_release_disk_block(&pf);
    s5_unlock_super(s5fs);
}

/*
 * Allocate one inode from the filesystem. You will need to use the super block
 * s5s_free_inode member. You must initialize the on-disk contents of the
 * allocated inode according to the arguments type and devid.
 *
 * Recall that the free inode list is a linked list. Each free inode contains a
 * link to the next free inode. The super block s5s_free_inode must always point
 * to the next free inode, or contain -1 to indicate no more inodes are
 * available.
 *
 * Don't forget to protect access to the super block and update s5s_free_inode.
 *
 * You should use s5_get_inode and s5_release_inode.
 *
 * On success, return the newly allocated inode number.
 * On failure, return -ENOSPC.
 */
long s5_alloc_inode(s5fs_t *s5fs, uint16_t type, devid_t devid)
{
    KASSERT((S5_TYPE_DATA == type) || (S5_TYPE_DIR == type) ||
            (S5_TYPE_CHR == type) || (S5_TYPE_BLK == type));

    s5_lock_super(s5fs);
    uint32_t new_ino = s5fs->s5f_super.s5s_free_inode;
    if (new_ino == (uint32_t)-1)
    {
        s5_unlock_super(s5fs);
        return -ENOSPC;
    }

    pframe_t *pf;
    s5_inode_t *inode;
    s5_get_inode(s5fs, new_ino, 1, &pf, &inode);

    s5fs->s5f_super.s5s_free_inode = inode->s5_un.s5_next_free;
    KASSERT(inode->s5_un.s5_next_free != inode->s5_number);

    inode->s5_un.s5_size = 0;
    inode->s5_type = type;
    inode->s5_linkcount = 0;
    memset(inode->s5_direct_blocks, 0, sizeof(inode->s5_direct_blocks));
    inode->s5_indirect_block =
        (S5_TYPE_CHR == type || S5_TYPE_BLK == type) ? devid : 0;

    s5_release_inode(&pf, &inode);
    s5_unlock_super(s5fs);

    dbg(DBG_S5FS, "allocated inode %d\n", new_ino);
    return new_ino;
}

/*
 * Free the inode by:
 *  1) adding the inode to the free inode linked list (opposite of
 * s5_alloc_inode), and 2) freeing all blocks being used by the inode.
 *
 * The suggested order of operations to avoid deadlock, is:
 *  1) lock the super block
 *  2) get the inode to be freed
 *  3) update the free inode linked list
 *  4) copy the blocks to be freed from the inode onto the stack
 *  5) release the inode
 *  6) unlock the super block
 *  7) free all direct blocks
 *  8) get the indirect block
 *  9) copy the indirect block array onto the stack
 *  10) release the indirect block
 *  11) free the indirect blocks
 *  12) free the indirect block itself
 */
void s5_free_inode(s5fs_t *s5fs, ino_t ino)
{
    pframe_t *pf;
    s5_inode_t *inode;
    s5_lock_super(s5fs);
    s5_get_inode(s5fs, ino, 1, &pf, &inode);

    uint32_t direct_blocks_to_free[S5_NDIRECT_BLOCKS];
    uint32_t indirect_block_to_free;
    if (inode->s5_type == S5_TYPE_DATA || inode->s5_type == S5_TYPE_DIR)
    {
        indirect_block_to_free = inode->s5_indirect_block;
        memcpy(direct_blocks_to_free, inode->s5_direct_blocks,
               sizeof(direct_blocks_to_free));
    }
    else
    {
        KASSERT(inode->s5_type == S5_TYPE_BLK || inode->s5_type == S5_TYPE_CHR);
        indirect_block_to_free = 0;
        memset(direct_blocks_to_free, 0, sizeof(direct_blocks_to_free));
    }

    inode->s5_un.s5_next_free = s5fs->s5f_super.s5s_free_inode;
    inode->s5_type = S5_TYPE_FREE;
    s5fs->s5f_super.s5s_free_inode = inode->s5_number;

    s5_release_inode(&pf, &inode);
    s5_unlock_super(s5fs);

    for (unsigned i = 0; i < S5_NDIRECT_BLOCKS; i++)
    {
        if (direct_blocks_to_free[i])
        {
            s5_free_block(s5fs, direct_blocks_to_free[i]);
        }
    }
    if (indirect_block_to_free)
    {
        uint32_t indirect_blocks_to_free[S5_NIDIRECT_BLOCKS];

        s5_get_disk_block(s5fs, indirect_block_to_free, 0, &pf);
        KASSERT(S5_BLOCK_SIZE == PAGE_SIZE);
        memcpy(indirect_blocks_to_free, pf->pf_addr, S5_BLOCK_SIZE);
        s5_release_disk_block(&pf);

        for (unsigned i = 0; i < S5_NIDIRECT_BLOCKS; i++)
        {
            if (indirect_blocks_to_free[i])
            {
                s5_free_block(s5fs, indirect_blocks_to_free[i]);
            }
        }
        s5_free_block(s5fs, indirect_block_to_free);
    }
    dbg(DBG_S5FS, "freed inode %d\n", ino);
}

/* Return the inode number corresponding to the directory entry specified by
 * name and namelen within a given directory.
 *
 *  sn      - The directory to search in
 *  name    - The name to search for
 *  namelen - Length of name
 *  filepos - If non-NULL, use filepos to return the starting position of the
 *            directory entry
 *
 * Return the desired inode number, or:
 *  - ENOENT: Could not find a directory entry with the specified name
 *
 * Hints:
 *  - Use s5_read_file in increments of sizeof(s5_dirent_t) to read successive
 *    directory entries and compare them against name and namelen.
 *  - To avoid reading beyond the end of the directory, check if the return 
 *    value of s5_read_file is 0
 *  - You could optimize this function by using s5_get_file_block (rather than
 *    s5_read_file) to ensure you do not read beyond the length of the file,
 *    but doing so is optional.
 */
long s5_find_dirent(s5_node_t *sn, const char *name, size_t namelen,
                    size_t *filepos)
{
    KASSERT(S_ISDIR(sn->vnode.vn_mode) && "should be handled at the VFS level");
    KASSERT(S5_BLOCK_SIZE == PAGE_SIZE && "be wary, thee");
    s5_dirent_t result;
    size_t read = 0;
    while (s5_read_file(sn, read, (char *) &result, sizeof(s5_dirent_t)) >= 0 && read < sn->inode.s5_un.s5_size) {
        if (name_match(result.s5d_name, name, namelen)) {
            if (filepos) {
                *filepos = read;
            }
            return result.s5d_inode;
        }
        read += sizeof(s5_dirent_t);
    }
    return -ENOENT;
}

/* Remove the directory entry specified by name and namelen from the directory
 * sn.
 *
 *  child - The found directory entry must correspond to the caller-provided
 *          child
 *
 * No return value. This function should never fail. You should assert that
 * anything which could be incorrect is correct, and any function calls which
 * could fail succeed.
 *
 * Hints:
 *  - Assert that the directory exists.
 *  - Assert that the found directory entry corresponds to child.
 *  - Ensure that the remaining directory entries in the file are contiguous. To
 *    do this, you should:
 *    - Overwrite the removed entry with the last directory entry.
 *    - Truncate the length of the directory by sizeof(s5_dirent_t).
 *  - Make sure you are only using s5_dirent_t, and not dirent_t structs.
 *  - Decrement the child's linkcount, because you have removed the directory's
 *    link to the child.
 *  - Mark the inodes as dirtied.
 *  - Use s5_find_dirent to find the position of the entry being removed. 
 */
void s5_remove_dirent(s5_node_t *sn, const char *name, size_t namelen,
                      s5_node_t *child)
{
    vnode_t *dir = &sn->vnode;
    s5_inode_t *inode = &sn->inode;
    size_t write_position;
    long ino = s5_find_dirent(sn, name, namelen, &write_position);
    KASSERT(ino >= 0);
    KASSERT(child->inode.s5_number == ino);
    s5_dirent_t end;
    s5_read_file(sn, sn->vnode.vn_len - sizeof(s5_dirent_t), (char *) &end, sizeof(s5_dirent_t));
    sn->vnode.vn_len -= sizeof(s5_dirent_t);
    sn->inode.s5_un.s5_size -= sizeof(s5_dirent_t);
    if (!name_match(end.s5d_name, name, namelen)) {
        s5_write_file(sn, write_position, (char *) &end, sizeof(s5_dirent_t));
    }
    child->inode.s5_linkcount -= 1;
    KASSERT(child->inode.s5_linkcount >= 0);
    child->dirtied_inode = 1;
    sn->dirtied_inode = 1;
}

/* Replace a directory entry.
 *
 *  sn      - The directory to search within
 *  name    - The name of the old directory entry
 *  namelen - Length of the old directory entry name
 *  old     - The s5_node corresponding to the old directory entry
 *  new     - The s5_node corresponding to the new directory entry
 *
 * No return value. Similar to s5_remove_dirent, this function should never
 * fail. You should assert that everything behaves correctly.
 *
 * Hints:
 *  - Assert that the directory exists, that the directory entry exists, and
 *    that it corresponds to the old s5_node.
 *  - When forming the new directory entry, use the same name and namelen from
 *    before, but use the inode number from the new s5_node.
 *  - Update linkcounts and dirty inodes appropriately.
 *
 * s5_replace_dirent is NOT necessary to implement. It's only useful if 
 * you're planning on implementing the renaming of directories (which you shouldn't 
 * attempt until after the rest of S5FS is done).
 */
void s5_replace_dirent(s5_node_t *sn, const char *name, size_t namelen,
                       s5_node_t *old, s5_node_t *new)
{
    vnode_t *dir = &sn->vnode;
    s5_inode_t *inode = &sn->inode;
    NOT_YET_IMPLEMENTED("RENAMEDIR: s5_replace_dirent()");
}

/* Create a directory entry.
 *
 *  dir     - The directory within which to create a new entry
 *  name    - The name of the new entry
 *  namelen - Length of the new entry name
 *  child   - The s5_node holding the inode which the new entry should represent
 *
 * Return 0 on success, or:
 *  - EEXIST: The directory entry already exists
 *  - Propagate errors from s5_write_file
 *
 * Hints:
 *  - Update linkcounts and mark inodes dirty appropriately.
 *  - You may wish to assert at the end of s5_link that the directory entry
 *    exists and that its inode is, as expected, the inode of child.
 */
long s5_link(s5_node_t *dir, const char *name, size_t namelen,
             s5_node_t *child)
{
    KASSERT(kmutex_owns_mutex(&dir->vnode.vn_mobj.mo_mutex));
    long status = s5_find_dirent(dir, name, namelen, NULL);
    if (status >= 0) {
        return -EEXIST;
    }
    s5_dirent_t dirent;
    memset(dirent.s5d_name, 0, S5_NAME_LEN);
    for (size_t i = 0; i < namelen; i++) {
        dirent.s5d_name[i] = name[i];
    }
    dirent.s5d_name[namelen] = '\0';
    dirent.s5d_inode = child->inode.s5_number;
    child->dirtied_inode = 1;
    status = s5_write_file(dir, dir->vnode.vn_len, (char *) &dirent, sizeof(s5_dirent_t));
    if (status < 0) {
        return status;
    }
    child->inode.s5_linkcount += 1;
    KASSERT(child->inode.s5_linkcount > 0);
    return 0;
}

/* Return the number of file blocks allocated for sn. This means any
 * file blocks that are not sparse, direct or indirect. If the indirect
 * block itself is allocated, that must also count. This function should not
 * fail.
 *
 * Hint:
 *  - You may wish to assert that the special character / block files do not
 *    have any blocks allocated to them. Remember, the s5_indirect_block for
 *    these special files is actually the device id.
 */
long s5_inode_blocks(s5_node_t *sn)
{   
    long count = 0;
    for (int d = 0; d < S5_NDIRECT_BLOCKS; d++) {
        if (sn->inode.s5_direct_blocks[d]) {
            count += 1;
        }
    }
    if (sn->inode.s5_indirect_block) {
        count += 1;
        pframe_t* pframe;
        s5_get_disk_block(FS_TO_S5FS(sn->vnode.vn_fs), sn->inode.s5_indirect_block, 0, &pframe);
        for (long unsigned int i = 0; i < S5_NIDIRECT_BLOCKS; i++) {
            if (((long *) pframe->pf_addr)[i]) { // QUESTION: should this be cast differently?
                count += 1;
            }
        }
        s5_release_disk_block(&pframe);
    }
    return count;
}

/**
 * Given a s5_node_t, frees the associated direct blocks and 
 * the indirect blocks if they exist. 
 * 
 * Should only be called from the truncate_file routine. 
 */
void s5_remove_blocks(s5_node_t *sn)
{
    // Free the blocks used by the node
    // First, free the the direct blocks
    s5fs_t* s5fs = VNODE_TO_S5FS(&sn->vnode);
    s5_inode_t* s5_inode = &sn->inode; 
    for (unsigned i = 0; i < S5_NDIRECT_BLOCKS; i++) 
    {
        if (s5_inode->s5_direct_blocks[i])
        {
            s5_free_block(s5fs, s5_inode->s5_direct_blocks[i]);
        }
    }

    memset(s5_inode->s5_direct_blocks, 0, sizeof(s5_inode->s5_direct_blocks));

    // Get the indirect blocks and free them, if they exist
    if (s5_inode->s5_indirect_block)
    {
        pframe_t *pf;
        s5_get_disk_block(s5fs, s5_inode->s5_indirect_block, 0, &pf);
        uint32_t *blocknum_ptr = pf->pf_addr;

        for (unsigned i = 0; i < S5_NIDIRECT_BLOCKS; i++)
        {
            if (blocknum_ptr[i])
            {
                s5_free_block(s5fs, blocknum_ptr[i]);
            }
        }

        s5_release_disk_block(&pf);
        // Free the indirect block itself
        s5_free_block(s5fs, s5_inode->s5_indirect_block);
        s5_inode->s5_indirect_block = 0;
    }
}
