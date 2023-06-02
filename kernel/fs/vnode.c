#include "fs/vnode.h"
#include "errno.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "kernel.h"
#include "mm/slab.h"
#include "util/debug.h"
#include "util/string.h"
#include <fs/vnode_specials.h>

#define MOBJ_TO_VNODE(o) CONTAINER_OF((o), vnode_t, vn_mobj)

static long vnode_get_pframe(mobj_t *o, uint64_t pagenum, long forwrite,
                             pframe_t **pfp);
static long vnode_fill_pframe(mobj_t *o, pframe_t *pf);
static long vnode_flush_pframe(mobj_t *o, pframe_t *pf);
static void vnode_destructor(mobj_t *o);

static mobj_ops_t vnode_mobj_ops = {.get_pframe = vnode_get_pframe,
                                    .fill_pframe = vnode_fill_pframe,
                                    .flush_pframe = vnode_flush_pframe,
                                    .destructor = vnode_destructor};

/**
 * locks the vnodes in the order of their inode number,
 * in the case that they are the same vnode, then only one vnode is locked.
 *
 * this scheme prevents the A->B/B->A locking problem, but it only
 * works only if the `vlock_in_order` function is used in all cases where 2
 * nodes must be locked.
 */
void vlock_in_order(vnode_t *a, vnode_t *b)
{
    /* these vnode's must be on the same filesystem */
    KASSERT(a->vn_fs == b->vn_fs);

    if (a->vn_vno == b->vn_vno)
    {
        vlock(a);
        return;
    }

    /* */
    if (S_ISDIR(a->vn_mode) && S_ISDIR(b->vn_mode))
    {
        if (namev_is_descendant(a, b))
        {
            vlock(b);
            vlock(a);
            return;
        }
        else if (namev_is_descendant(b, a))
        {
            vlock(a);
            vlock(b);
            return;
        }
    }
    else if (S_ISDIR(a->vn_mode))
    {
        vlock(a);
        vlock(b);
    }
    else if (S_ISDIR(b->vn_mode))
    {
        vlock(b);
        vlock(a);
    }
    else if (a->vn_vno < b->vn_vno)
    {
        vlock(a);
        vlock(b);
    }
    else
    {
        vlock(b);
        vlock(a);
    }
}

void vunlock_in_order(vnode_t *a, vnode_t *b)
{
    if (a->vn_vno == b->vn_vno)
    {
        vunlock(a);
        return;
    }

    vunlock(a);
    vunlock(b);
}

void await_vnode_loaded(vnode_t *vnode)
{
    /* blocks until the vnode's vn_state is loaded */
    spinlock_lock(&vnode->vn_state_lock);
    while (vnode->vn_state != VNODE_LOADED)
    {
        sched_sleep_on(&vnode->vn_waitq, &vnode->vn_state_lock);
        spinlock_lock(&vnode->vn_state_lock);
    }
    KASSERT(vnode->vn_state == VNODE_LOADED);
    spinlock_unlock(&vnode->vn_state_lock);
}

void notify_vnode_loaded(vnode_t *vn)
{
    /* set the state to loaded and release all waiters */
    spinlock_lock(&vn->vn_state_lock);
    vn->vn_state = VNODE_LOADED;
    spinlock_unlock(&vn->vn_state_lock);
    sched_broadcast_on(&vn->vn_waitq);
}

void vnode_init(vnode_t *vn, fs_t *fs, ino_t ino, int state)
{
    vn->vn_state = VNODE_LOADING;
    vn->vn_fs = fs;
    vn->vn_vno = ino;
    spinlock_init(&vn->vn_state_lock);
    sched_queue_init(&vn->vn_waitq);
    mobj_init(&vn->vn_mobj, MOBJ_VNODE, &vnode_mobj_ops);
    KASSERT(vn->vn_mobj.mo_refcount);
}

vnode_t *__vget(fs_t *fs, ino_t ino, int get_locked)
{
find:
    kmutex_lock(&fs->vnode_list_mutex);
    list_iterate(&fs->vnode_list, vn, vnode_t, vn_link)
    {
        if (vn->vn_vno == ino)
        {
            if (atomic_inc_not_zero(&vn->vn_mobj.mo_refcount))
            {
                /* reference acquired, we can release the per-FS list */
                kmutex_unlock(&fs->vnode_list_mutex);
                await_vnode_loaded(vn);
                if (get_locked)
                {
                    vlock(vn);
                }
                return vn;
            }
            else
            {
                /* count must be 0, wait and try again later */
                kmutex_unlock(&fs->vnode_list_mutex);
                sched_yield();
                goto find;
            }
        }
    }

    /* vnode does not exist, must allocate one */
    dbg(DBG_VFS, "creating vnode %d\n", ino);
    vnode_t *vn = slab_obj_alloc(fs->fs_vnode_allocator);
    KASSERT(vn);
    memset(vn, 0, sizeof(vnode_t));

    /* initialize the vnode state */
    vnode_init(vn, fs, ino, VNODE_LOADING);

    /* add the vnode to the per-FS list, lock the vnode, and release the list
     * (unblocking other `vget` calls) */
    list_insert_tail(&fs->vnode_list, &vn->vn_link);
    vlock(vn);
    kmutex_unlock(&fs->vnode_list_mutex);

    /* load the vnode */
    vn->vn_fs->fs_ops->read_vnode(vn->vn_fs, vn);
    if (S_ISCHR(vn->vn_mode) || S_ISBLK(vn->vn_mode))
    {
        init_special_vnode(vn);
    }

    /* notify potential waiters that the vnode is ready for use and return */
    notify_vnode_loaded(vn);
    if (!get_locked)
    {
        vunlock(vn);
    }
    return vn;
}

inline vnode_t *vget(fs_t *fs, ino_t ino) { return __vget(fs, ino, 0); }

inline vnode_t *vget_locked(fs_t *fs, ino_t ino) { return __vget(fs, ino, 1); }

inline void vref(vnode_t *vn) { mobj_ref(&vn->vn_mobj); }

inline void vlock(vnode_t *vn) { mobj_lock(&vn->vn_mobj); }

inline void vunlock(vnode_t *vn) { mobj_unlock(&vn->vn_mobj); }

inline void vput(struct vnode **vnp)
{
    vnode_t *vn = *vnp;
    *vnp = NULL;
    mobj_t *mobj = &vn->vn_mobj;
    mobj_put(&mobj);
}

inline void vput_locked(struct vnode **vnp)
{
    vunlock(*vnp);
    vput(vnp);
}

static long vnode_get_pframe(mobj_t *o, uint64_t pagenum, long forwrite,
                             pframe_t **pfp)
{
    vnode_t *vnode = MOBJ_TO_VNODE(o);
    KASSERT(vnode->vn_ops->get_pframe);
    return vnode->vn_ops->get_pframe(vnode, pagenum, forwrite, pfp);
}

static long vnode_fill_pframe(mobj_t *o, pframe_t *pf)
{
    vnode_t *vnode = MOBJ_TO_VNODE(o);
    KASSERT(vnode->vn_ops->fill_pframe);
    return vnode->vn_ops->fill_pframe(vnode, pf);
}

static long vnode_flush_pframe(mobj_t *o, pframe_t *pf)
{
    vnode_t *vnode = MOBJ_TO_VNODE(o);
    KASSERT(vnode->vn_ops->flush_pframe);
    return vnode->vn_ops->flush_pframe(vnode, pf);
}

static void vnode_destructor(mobj_t *o)
{
    vnode_t *vn = MOBJ_TO_VNODE(o);
    dbg(DBG_VFS, "destroying vnode %d\n", vn->vn_vno);

    /* lock, flush, and delete the vnode */
    KASSERT(!o->mo_refcount);
    vlock(vn);
    KASSERT(!o->mo_refcount);
    KASSERT(!kmutex_has_waiters(&o->mo_mutex));
    mobj_flush(o);
    if (vn->vn_fs->fs_ops->delete_vnode)
    {
        vn->vn_fs->fs_ops->delete_vnode(vn->vn_fs, vn);
    }
    KASSERT(!kmutex_has_waiters(&o->mo_mutex));
    vunlock(vn);

    /* remove the vnode from the list and free it*/
    kmutex_lock(&vn->vn_fs->vnode_list_mutex);
    KASSERT(list_link_is_linked(&vn->vn_link));
    list_remove(&vn->vn_link);
    kmutex_unlock(&vn->vn_fs->vnode_list_mutex);
    slab_obj_free(vn->vn_fs->fs_vnode_allocator, vn);
}
