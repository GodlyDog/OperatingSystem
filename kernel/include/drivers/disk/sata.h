#pragma once

#define SATA_BLOCK_SIZE 4096

#include <drivers/blockdev.h>
#include <drivers/disk/ahci.h>

void sata_init();

typedef struct ata_disk
{
    hba_port_t *port;
    blockdev_t bdev;
} ata_disk_t;
