// SMP.1 + SMP.3
// spinlock + mask interrupts
#include "proc/kmutex.h"
#include "globals.h"
#include "main/interrupt.h"
#include <errno.h>

/*
 * IMPORTANT: Mutexes can _NEVER_ be locked or unlocked from an
 * interrupt context. Mutexes are _ONLY_ lock or unlocked from a
 * thread context.
 */

/*
 * Checks for the specific deadlock case where:
 *  curthr wants mtx, but the owner of mtx is waiting on a mutex that curthr is
 * holding
 */
#define DEBUG_DEADLOCKS 1
void detect_deadlocks(kmutex_t *mtx)
{
#if DEBUG_DEADLOCKS
    list_iterate(&curthr->kt_mutexes, held, kmutex_t, km_link)
    {
        list_iterate(&held->km_waitq.tq_list, waiter, kthread_t, kt_qlink)
        {
            if (waiter == mtx->km_holder)
            {
                panic(
                    "detected deadlock between P%d and P%d (mutexes 0x%p, "
                    "0x%p)\n",
                    curproc->p_pid, waiter->kt_proc->p_pid, held, mtx);
            }
        }
    }
#endif
}

/*
 * Initializes the members of mtx
 */
void kmutex_init(kmutex_t *mtx)
{
    /* PROCS {{{ */
    mtx->km_holder = NULL;
    sched_queue_init(&mtx->km_waitq);
    spinlock_init(&mtx->km_lock);
    list_link_init(&mtx->km_link);
    /* PROCS }}} */
}

/*
 * Obtains a mutex, potentially blocking.
 *
 * Hints:
 * Remember to protect access to the mutex via its spinlock.
 * You are strongly advised to maintain the kt_mutexes member of curthr and call
 * detect_deadlocks() to help debugging.
 */
void kmutex_lock(kmutex_t *mtx)
{
    /* PROCS {{{ */

    dbg(DBG_ERROR, "locked mutex: %p\n", mtx);
    spinlock_lock(&mtx->km_lock);
    KASSERT(curthr && "need thread context to lock mutex");
    KASSERT(!kmutex_owns_mutex(mtx) && "already owner");

    if (mtx->km_holder)
    {
        detect_deadlocks(mtx);
        sched_sleep_on(&mtx->km_waitq, &mtx->km_lock);
        KASSERT(kmutex_owns_mutex(mtx));
    }
    else
    {
        mtx->km_holder = curthr;
        list_insert_tail(&curthr->kt_mutexes, &mtx->km_link);
        spinlock_unlock(&mtx->km_lock);
    }
    /* PROCS }}} */
}

/*
 * Releases a mutex.
 *
 * Hints:
 * Remember to protect access to the mutex via the spinlock.
 * Again, you are strongly advised to maintain kt_mutexes.
 * Use sched_wakeup_on() to hand off the mutex - think carefully about how
 *  these two functions interact to ensure that the mutex's km_holder is
 * properly set before the new owner is runnable.
 */
void kmutex_unlock(kmutex_t *mtx)
{
    /* PROCS {{{ */
    dbg(DBG_ERROR, "unlocked mutex: %p\n", mtx);
    spinlock_lock(&mtx->km_lock);
    KASSERT(curthr && (curthr == mtx->km_holder) &&
            "unlocking a mutex we don\'t own");
    sched_wakeup_on(&mtx->km_waitq, &mtx->km_holder);
    KASSERT(!kmutex_owns_mutex(mtx));
    list_remove(&mtx->km_link);
    if (mtx->km_holder)
        list_insert_tail(&mtx->km_holder->kt_mutexes, &mtx->km_link);
    spinlock_unlock(&mtx->km_lock);
    /* PROCS }}} */
}

/*
 * Checks if mtx's wait queue is empty.
 */
long kmutex_has_waiters(kmutex_t *mtx)
{
    return !sched_queue_empty(&mtx->km_waitq);
    ;
}

/*
 * Checks if the current thread owns mtx.
 */
inline long kmutex_owns_mutex(kmutex_t *mtx)
{
    return curthr && mtx->km_holder == curthr;
}