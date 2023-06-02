#include "globals.h"
#include "main/apic.h"

void spinlock_init(spinlock_t *lock) { lock->s_locked = 0; }

inline void spinlock_lock(spinlock_t *lock)
{
// __sync_bool_compare_and_swap is a GCC intrinsic for atomic compare-and-swap
// If lock->locked is 0, then it is set to 1 and __sync_bool_compare_and_swap
// returns true Otherwise, lock->locked is left at 1 and
// __sync_bool_compare_and_swap returns false
#ifdef __SMP__
    preemption_disable();
    KASSERT(lock->s_locked <= MAX_LAPICS && "using invalid spinlock");
    KASSERT(lock->s_locked != curcore.kc_id + 1 && "double-locking spinlock");
    while (
        !__sync_bool_compare_and_swap(&lock->s_locked, 0, curcore.kc_id + 1))
    {
        // As an optimization, do a read-only loop and pause
        // See
        // https://stackoverflow.com/questions/12894078/what-is-the-purpose-of-the-pause-instruction-in-x86
        while (lock->s_locked)
        {
            __asm__("pause;");
        }
    }
#endif
}

inline void spinlock_unlock(spinlock_t *lock)
{
#ifdef __SMP__
    __sync_synchronize(); // Put a memory barrier before setting the locked
                          // flag
    lock->s_locked = 0;
    preemption_enable();
#endif
}

inline long spinlock_ownslock(spinlock_t *lock)
{
#ifdef __SMP__
    return lock->s_locked == curcore.kc_id + 1;
#endif
    return 1;
}
