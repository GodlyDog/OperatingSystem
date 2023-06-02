#pragma once

#include "proc/sched.h"
#include "proc/spinlock.h"

/*===========
 * Structures
 *==========*/

typedef struct kmutex
{
    ktqueue_t km_waitq;        /* wait queue */
    struct kthread *km_holder; /* current holder */
    spinlock_t km_lock;
    list_link_t km_link;
} kmutex_t;

#define KMUTEX_INITIALIZER(mtx)                                             \
    {                                                                       \
        .km_waitq = KTQUEUE_INITIALIZER((mtx).km_waitq), .km_holder = NULL, \
        .km_lock = SPINLOCK_INITIALIZER((mtx).km_lock),                     \
        .km_link = LIST_LINK_INITIALIZER((mtx).km_link),                    \
    }

/*==========
 * Functions
 *=========*/

/**
 * Initializes a mutex.
 *
 * @param mtx the mutex
 */
void kmutex_init(kmutex_t *mtx);

/**
 * Locks the specified mutex.
 *
 * Note: This function may block.
 *
 * Note: These locks are not re-entrant
 *
 * @param mtx the mutex to lock
 */
void kmutex_lock(kmutex_t *mtx);

/**
 * Unlocks the specified mutex.
 *
 * @param mtx the mutex to unlock
 */
void kmutex_unlock(kmutex_t *mtx);

/**
 * Indicates if a mutex has waiters.
 */
long kmutex_has_waiters(kmutex_t *mtx);

/**
 * Indicates if curthr owns a mutex.
 */
long kmutex_owns_mutex(kmutex_t *mtx);
