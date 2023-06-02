#include "errno.h"

#include "mm/mobj.h"
#include "mm/pframe.h"

#include "util/debug.h"
#include <util/string.h>

/*
 * Initialize o according to type and ops. If ops do not specify a
 * get_pframe function, set it to the default, mobj_default_get_pframe.
 * Do the same with the destructor function pointer.
 *
 * Upon return, the refcount of the mobj should be 1.
 */
void mobj_init(mobj_t *o, long type, mobj_ops_t *ops)
{
    o->mo_type = type;

    memcpy(&o->mo_ops, ops, sizeof(mobj_ops_t));
    if (!o->mo_ops.get_pframe)
    {
        o->mo_ops.get_pframe = mobj_default_get_pframe;
        KASSERT(o->mo_ops.fill_pframe);
        KASSERT(o->mo_ops.flush_pframe);
    }
    if (!o->mo_ops.destructor)
    {
        o->mo_ops.destructor = mobj_default_destructor;
    }

    kmutex_init(&o->mo_mutex);

    o->mo_refcount = ATOMIC_INIT(1);
    list_init(&o->mo_pframes);
}

/*
 * Lock the mobj's mutex
 */
inline void mobj_lock(mobj_t *o) { kmutex_lock(&o->mo_mutex); }

/*
 * Unlock the mobj's mutex
 */
inline void mobj_unlock(mobj_t *o) { kmutex_unlock(&o->mo_mutex); }

/*
 * Increment refcount
 */
void mobj_ref(mobj_t *o)
{
    atomic_inc(&o->mo_refcount);
}

void mobj_put_locked(mobj_t **op)
{
    mobj_unlock(*op);
    mobj_put(op);
}

/*
 * Decrement refcount, and set *op = NULL.
 * If the refcount drop to 0, call the destructor, otherwise unlock the mobj.
 */
void mobj_put(mobj_t **op)
{
    mobj_t *o = *op;
    KASSERT(o->mo_refcount);
    *op = NULL;

    dbg(DBG_ERROR, "count: %d\n", o->mo_refcount);
    if (atomic_dec_and_test(&o->mo_refcount))
    {
        dbg(DBG_ERROR, "count: %d\n", o->mo_refcount);

        KASSERT(!kmutex_owns_mutex(&o->mo_mutex));
        o->mo_ops.destructor(o);
    }
    else
    {
        dbg(DBG_ERROR, "count: %d\n", o->mo_refcount);
    }
}

/*
 * Find a pframe that already exists in the memory object's mo_pframes list.
 * If a pframe is found, it must be locked upon return from this function using
 * pf_mutex.
 */
void mobj_find_pframe(mobj_t *o, uint64_t pagenum, pframe_t **pfp)
{
    KASSERT(kmutex_owns_mutex(&o->mo_mutex));
    list_iterate(&o->mo_pframes, pf, pframe_t, pf_link)
    {
        if (pf->pf_pagenum == pagenum)
        {
            kmutex_lock(&pf->pf_mutex);
            *pfp = pf;
            return;
        }
    }
    *pfp = NULL;
}

/*
 * Wrapper around the memory object's get_pframe function
 * Assert a sane state of the world surrounding the call to get_pframe
 */
long mobj_get_pframe(mobj_t *o, uint64_t pagenum, long forwrite,
                     pframe_t **pfp)
{
    KASSERT(kmutex_owns_mutex(&o->mo_mutex));
    *pfp = NULL;
    long ret = o->mo_ops.get_pframe(o, pagenum, forwrite, pfp);
    KASSERT((!*pfp && ret) || kmutex_owns_mutex(&(*pfp)->pf_mutex));
    return ret;
}

/*
 * Create and initialize a pframe and add it to the mobj's mo_pframes list.
 * Upon successful return, the pframe's pf_mutex is locked.
 */
static void mobj_create_pframe(mobj_t *o, uint64_t pagenum, pframe_t **pfp)
{
    KASSERT(kmutex_owns_mutex(&o->mo_mutex));
    pframe_t *pf = pframe_create();
    if (pf)
    {
        kmutex_lock(&pf->pf_mutex);

        pf->pf_pagenum = pagenum;
        list_insert_tail(&o->mo_pframes, &pf->pf_link);
    }
    KASSERT(!pf || kmutex_owns_mutex(&pf->pf_mutex));
    *pfp = pf;
}

/*
 * The default get pframe that is at the center of the mobj/pframe subsystem.
 * This is the routine that is used when the memory object does not have a 
 * get_pframe function associated with it (or called in the case of shadow objects
 * when the forwrite flag is set).
 *
 * First, check if an pframe already exists in the mobj, creating one as
 * necessary. Then, ensure that the pframe's contents are loaded: i.e. that
 * pf->pf_addr is non-null. You will want to use page_alloc() and fill_pframe
 * function pointer of the mobj. Finally, if forwrite is true, mark the pframe
 * as dirtied. The resulting pframe should be set in *pfp.
 *
 * Note that upon failure, *pfp MUST be null. As always, make sure you cleanup
 * properly in all error cases (especially if fill_prame fails)
 *
 * Upon successful return, *pfp refers to the found pframe and MUST be locked.
 *
 * Error cases mobj_default_get_pframe is responsible for generating:
 *  - ENOMEM: either cannot create the pframe or cannot allocate memory for
 *            the pframe's contents
 */
long mobj_default_get_pframe(mobj_t *o, uint64_t pagenum, long forwrite,
                             pframe_t **pfp)
{
    KASSERT(kmutex_owns_mutex(&o->mo_mutex));
    *pfp = NULL;
    pframe_t *pf = NULL;
    mobj_find_pframe(o, pagenum, &pf);
    if (!pf)
    {
        mobj_create_pframe(o, pagenum, &pf);
    }
    if (!pf)
    {
        return -ENOMEM;
    }
    KASSERT(kmutex_owns_mutex(&pf->pf_mutex));
    if (!pf->pf_addr)
    {
        KASSERT(!pf->pf_dirty &&
                "dirtied page doesn't have a physical address");
        pf->pf_addr = page_alloc();
        if (!pf->pf_addr)
        {
            return -ENOMEM;
        }

        dbg(DBG_PFRAME, "filling pframe 0x%p (mobj 0x%p page %lu)\n", pf, o,
            pf->pf_pagenum);
        KASSERT(o->mo_ops.fill_pframe);
        long ret = o->mo_ops.fill_pframe(o, pf);
        if (ret)
        {
            page_free(pf->pf_addr);
            pf->pf_addr = NULL;
            kmutex_unlock(&pf->pf_mutex);
            return ret;
        }
    }
    pf->pf_dirty |= forwrite;
    *pfp = pf;
    return 0;
}

/*
 * If the pframe is dirty, call the mobj's flush_pframe; if flush_pframe returns
 * successfully, clear pf_dirty flag and return 0. Otherwise, return what
 * flush_pframe returned.
 *
 * Both o and pf must be locked when calling this function
 */
long mobj_flush_pframe(mobj_t *o, pframe_t *pf)
{
    KASSERT(kmutex_owns_mutex(&o->mo_mutex));
    KASSERT(kmutex_owns_mutex(&pf->pf_mutex));
    KASSERT(pf->pf_addr && "cannot flush a frame not in memory!");
    dbg(DBG_PFRAME, "pf 0x%p, mobj 0x%p, page %lu\n", pf, o, pf->pf_pagenum);
    if (pf->pf_dirty)
    {
        KASSERT(o->mo_ops.flush_pframe);
        long ret = o->mo_ops.flush_pframe(o, pf);
        if (ret)
            return ret;
        pf->pf_dirty = 0;
    }
    KASSERT(!pf->pf_dirty);
    return 0;
}

/*
 * Iterate through the pframes of the mobj and try to flush each one.
 * If any of them fail, let that reflect in the return value.
 *
 * The mobj o must be locked when calling this function
 */
long mobj_flush(mobj_t *o)
{
    long ret = 0;
    KASSERT(kmutex_owns_mutex(&o->mo_mutex));
    list_iterate(&o->mo_pframes, pf, pframe_t, pf_link)
    {
        kmutex_lock(&pf->pf_mutex); // get the pframe (lock it)
        if (pf->pf_addr)
        {
            ret |= mobj_flush_pframe(o, pf);
        }
        pframe_release(&pf);
    }
    return ret;
}

/*
 * Attempt to flush the pframe. If the flush succeeds, then free the pframe's
 * contents (pf->pf_addr) using page_free, remove the pframe from the mobj's
 * list and call pframe_free.
 *
 * Upon successful return, *pfp MUST be null. If the function returns an error
 * code, *pfp must be unchanged.
 */
long mobj_free_pframe(mobj_t *o, pframe_t **pfp)
{
    pframe_t *pf = *pfp;

    if (pf->pf_addr)
    {
        long ret = mobj_flush_pframe(o, pf);
        if (ret)
            return ret;

        // [+] TODO REMOVE THIS SECTION WHEN FLUSH DOES IT (I.E. WHEN WE HAVE
        // SUPPORT FOR FREEING PFRAME'S IN USE BY UNMAPPING THEM FROM PAGE
        // TABLES THAT USE THEM)
        if (pf->pf_addr)
        {
            page_free(pf->pf_addr);
            pf->pf_addr = NULL;
        }
    }
    *pfp = NULL;
    list_remove(&pf->pf_link);
    pframe_free(&pf);
    return 0;
}

/*
 * Simply flush the memory object
 */
void mobj_default_destructor(mobj_t *o)
{
    mobj_lock(o);
    KASSERT(kmutex_owns_mutex(&o->mo_mutex));

    long ret = 0;
    list_iterate(&o->mo_pframes, pf, pframe_t, pf_link)
    {
        kmutex_lock(&pf->pf_mutex); // get the pframe (lock it)
        ret |= mobj_free_pframe(o, &pf);
    }

    if (ret)
    {
        dbg(DBG_MM,
            "WARNING: flushing pframes in mobj destructor failed for one or "
            "more frames\n"
            "This means the memory for the pframe will be leaked!");
    }

    KASSERT(!kmutex_has_waiters(&o->mo_mutex));
    mobj_unlock(o);
}
