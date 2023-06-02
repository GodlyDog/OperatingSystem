#include "globals.h"

#include "mm/pframe.h"
#include "mm/slab.h"

#include "util/debug.h"
#include "util/string.h"

static slab_allocator_t *pframe_allocator;

void pframe_init()
{
    pframe_allocator = slab_allocator_create("pframe", sizeof(pframe_t));
    KASSERT(pframe_allocator);
}

/*
 * Create a pframe and initialize its members appropriately.
 */
pframe_t *pframe_create()
{
    pframe_t *pf = slab_obj_alloc(pframe_allocator);
    if (!pf)
    {
        return NULL;
    }
    memset(pf, 0, sizeof(pframe_t));
    kmutex_init(&pf->pf_mutex);
    list_link_init(&pf->pf_link);
    return pf;
}

/*
 * Free the pframe (don't forget to unlock the mutex) and set *pfp = NULL
 *
 * The pframe must be locked, its contents not in memory (pf->pf_addr == NULL),
 * have a pincount of 0, and not be linked into a memory object's list.
 */
void pframe_free(pframe_t **pfp)
{
    KASSERT(kmutex_owns_mutex(&(*pfp)->pf_mutex));
    KASSERT(!(*pfp)->pf_addr);
    KASSERT(!(*pfp)->pf_dirty);
    KASSERT(!list_link_is_linked(&(*pfp)->pf_link));
    kmutex_unlock(&(*pfp)->pf_mutex);
    slab_obj_free(pframe_allocator, *pfp);
    *pfp = NULL;
}

/*
 * Unlock the pframe and set *pfp = NULL
 */
void pframe_release(pframe_t **pfp)
{
    pframe_t *pf = *pfp;
    KASSERT(kmutex_owns_mutex(&pf->pf_mutex));
    *pfp = NULL;
    kmutex_unlock(&pf->pf_mutex);
}
