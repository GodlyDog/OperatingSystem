#include "drivers/chardev.h"
#include "drivers/memdevs.h"
#include "drivers/tty/tty.h"
#include "kernel.h"
#include "util/debug.h"

static list_t chardevs = LIST_INITIALIZER(chardevs);

void chardev_init()
{
    tty_init();
    memdevs_init();
}

long chardev_register(chardev_t *dev)
{
    if (!dev || (NULL_DEVID == dev->cd_id) || !(dev->cd_ops))
    {
        return -1;
    }
    list_iterate(&chardevs, cd, chardev_t, cd_link)
    {
        if (dev->cd_id == cd->cd_id)
        {
            return -1;
        }
    }
    list_insert_tail(&chardevs, &dev->cd_link);
    return 0;
}

chardev_t *chardev_lookup(devid_t id)
{
    list_iterate(&chardevs, cd, chardev_t, cd_link)
    {
        KASSERT(NULL_DEVID != cd->cd_id);
        if (id == cd->cd_id)
        {
            return cd;
        }
    }
    return NULL;
}
