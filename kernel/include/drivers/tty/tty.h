#pragma once

#include "drivers/chardev.h"
#include "ldisc.h"
#include "vterminal.h"

#define TTY_MAJOR 2
#define cd_to_tty(cd) \
    CONTAINER_OF((cd), tty_t, tty_cdev) 

typedef struct tty
{
    vterminal_t tty_vterminal; // the virtual terminal, where the characters will be displayed
    ldisc_t tty_ldisc; // the line discipline for the tty 
    chardev_t tty_cdev; // the super struct for the tty 
    kmutex_t tty_read_mutex;
    kmutex_t tty_write_mutex;
    spinlock_t tty_lock;
} tty_t;

void tty_init(void);

