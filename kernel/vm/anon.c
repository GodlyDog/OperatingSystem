#include "mm/mobj.h"
#include "mm/page.h"
#include "mm/pframe.h"
#include "mm/slab.h"

#include "util/debug.h"
#include "util/string.h"

/* for debugging/verification purposes */
int anon_count = 0; 

static slab_allocator_t *anon_allocator;

static long anon_fill_pframe(mobj_t *o, pframe_t *pf);

static long anon_flush_pframe(mobj_t *o, pframe_t *pf);

static void anon_destructor(mobj_t *o);

static mobj_ops_t anon_mobj_ops = {.get_pframe = NULL,
                                   .fill_pframe = anon_fill_pframe,
                                   .flush_pframe = anon_flush_pframe,
                                   .destructor = anon_destructor};

/*
 * Initialize anon_allocator using the slab allocator.
 */
void anon_init()
{
    anon_allocator = slab_allocator_create("anon", sizeof(mobj_t));
    KASSERT(anon_allocator);
}

/*
 * The mobj should be locked upon successful return. Use mobj_init and
 * mobj_lock.
 */
mobj_t *anon_create()
{
    mobj_t* anon = slab_obj_alloc(anon_allocator);
    mobj_init(anon, MOBJ_ANON, &anon_mobj_ops);
    mobj_lock(anon);
    return anon;
}

/* 
 * This function is not complicated -- think about what the pframe should look
 * like for an anonymous object 
 */
static long anon_fill_pframe(mobj_t *o, pframe_t *pf)
{
    memset(pf->pf_addr, 0, PAGE_SIZE);
    return 0;
}

static long anon_flush_pframe(mobj_t *o, pframe_t *pf) { return 0; }

/*
 * Release all resources associated with an anonymous object.
 *
 * Hints:
 *  1) Call mobj_default_destructor() to free pframes
 *  2) Free the mobj
 */
static void anon_destructor(mobj_t *o)
{
    mobj_default_destructor(o);
    slab_obj_free(anon_allocator, o);
}
