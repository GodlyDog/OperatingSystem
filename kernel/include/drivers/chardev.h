#pragma once

#include "drivers/dev.h"
#include "util/list.h"

struct vnode;
struct pframe;

struct chardev_ops;
struct mobj;

typedef struct chardev
{
    devid_t cd_id;
    struct chardev_ops *cd_ops;
    list_link_t cd_link;
} chardev_t;

typedef struct chardev_ops
{
    ssize_t (*read)(chardev_t *dev, size_t pos, void *buf, size_t count);

    ssize_t (*write)(chardev_t *dev, size_t pos, const void *buf, size_t count);

    long (*mmap)(struct vnode *file, struct mobj **ret);

    long (*fill_pframe)(struct vnode *file, struct pframe *pf);

    long (*flush_pframe)(struct vnode *file, struct pframe *pf);
} chardev_ops_t;

/**
 * Initializes the byte device subsystem.
 */
void chardev_init(void);

/**
 * Registers the given byte device.
 *
 * @param dev the byte device to register
 */
long chardev_register(chardev_t *dev);

/**
 * Finds a byte device with a given device id.
 *
 * @param id the device id of the byte device to find
 * @return the byte device with the given id if it exists, or NULL if
 * it cannot be found
 */
chardev_t *chardev_lookup(devid_t id);
