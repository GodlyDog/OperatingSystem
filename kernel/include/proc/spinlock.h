#pragma once

typedef struct spinlock
{
    volatile char s_locked;
} spinlock_t;

#define SPINLOCK_INITIALIZER(lock) \
    {                              \
        .s_locked = 0              \
    }

/**
 * Initializes the fields of the specified spinlock_t
 * @param lock the spinlock to initialize
 */
void spinlock_init(spinlock_t *lock);

/**
 * Locks the specified spinlock.
 *
 * Note: this function may spin on the current core.
 *
 * Note: these locks are not re-entrant
 *
 * @param lock the spinlock to lock
 */
void spinlock_lock(spinlock_t *lock);

/**
 * Unlocks the specified spinlock.
 *
 * @param lock the spinlock to unlock
 */
void spinlock_unlock(spinlock_t *lock);

long spinlock_ownslock(spinlock_t *lock);
