#include "kernel.h"
#include "util/debug.h"
#include <drivers/disk/sata.h>

#include "drivers/blockdev.h"

#include "mm/pframe.h"

static long blockdev_fill_pframe(mobj_t *mobj, pframe_t *pf);

static long blockdev_flush_pframe(mobj_t *mobj, pframe_t *pf);

static mobj_ops_t blockdev_mobj_ops = {.get_pframe = NULL,
                                       .fill_pframe = blockdev_fill_pframe,
                                       .flush_pframe = blockdev_flush_pframe,
                                       .destructor = NULL};

static list_t blockdevs = LIST_INITIALIZER(blockdevs);

void blockdev_init() { sata_init(); }

long blockdev_register(blockdev_t *dev)
{
    if (!dev || dev->bd_id == NULL_DEVID || !dev->bd_ops)
    {
        return -1;
    }

    list_iterate(&blockdevs, bd, blockdev_t, bd_link)
    {
        if (dev->bd_id == bd->bd_id)
        {
            return -1;
        }
    }

    mobj_init(&dev->bd_mobj, MOBJ_BLOCKDEV, &blockdev_mobj_ops);

    list_insert_tail(&blockdevs, &dev->bd_link);
    return 0;
}

blockdev_t *blockdev_lookup(devid_t id)
{
    list_iterate(&blockdevs, bd, blockdev_t, bd_link)
    {
        if (id == bd->bd_id)
        {
            return bd;
        }
    }
    return NULL;
}

static long blockdev_fill_pframe(mobj_t *mobj, pframe_t *pf)
{
    KASSERT(mobj && pf);
    KASSERT(pf->pf_pagenum <= (1UL << (8 * sizeof(blocknum_t))));
    blockdev_t *bd = CONTAINER_OF(mobj, blockdev_t, bd_mobj);
    return bd->bd_ops->read_block(bd, pf->pf_addr, (blocknum_t)pf->pf_pagenum,
                                  1);
}

static long blockdev_flush_pframe(mobj_t *mobj, pframe_t *pf)
{
    KASSERT(mobj && pf);
    KASSERT(pf->pf_pagenum <= (1UL << (8 * sizeof(blocknum_t))));
    dbg(DBG_S5FS, "writing disk block %lu\n", pf->pf_pagenum);
    blockdev_t *bd = CONTAINER_OF(mobj, blockdev_t, bd_mobj);
    return bd->bd_ops->write_block(bd, pf->pf_addr, (blocknum_t)pf->pf_pagenum,
                                   1);
}
