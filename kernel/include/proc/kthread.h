#pragma once

#include <proc/context.h>
#include <proc/sched.h>
#include <proc/spinlock.h>
#include <util/list.h>

/*=====================
 * Types and structures
 *====================*/

/*
 * Alias for an entry point function of a new thread.
 */
typedef context_func_t kthread_func_t;

/*
 * Thread states.
 */
typedef enum
{
    KT_NO_STATE,          /* Illegal state */
    KT_ON_CPU,            /* Currently running */
    KT_RUNNABLE,          /* On the run queue */
    KT_SLEEP,             /* Blocked indefinitely */
    KT_SLEEP_CANCELLABLE, /* Blocked, but can be interrupted */
    KT_EXITED             /* Exited, waiting to be joined */
} kthread_state_t;

/*
 * Thread descriptor.
 */
typedef struct kthread
{
    context_t kt_ctx;     /* Thread context */
    char *kt_kstack;      /* Kernel stack */
    void *kt_retval;      /* Return value */
    long kt_errno;        /* Errno of most recent syscall */
    struct proc *kt_proc; /* Corresponding process */

    long kt_cancelled;   /* Set if the thread has been cancelled */
    ktqueue_t *kt_wchan; /* If blocking, the queue this thread is blocked on  NULL*/
    kthread_state_t kt_state;

    spinlock_t kt_lock;

    list_link_t kt_plink; /* Link on the process's thread list, p_threads  processor link */
    list_link_t kt_qlink; /* Link on some ktqueue if the thread is not running  NULL */

    list_t kt_mutexes;   /* List of owned mutexes, for use in debugging */
    long kt_recent_core; /* For SMP */

    uint64_t kt_preemption_count;
} kthread_t;

/*==========
 * Functions
 *=========*/

/**
 * Initializes the kthread subsystem at system startup.
 */
void kthread_init(void);

/**
 * Allocates and initializes a kernel thread.
 *
 * @param proc the process in which the thread will run
 * @param func the function that will be called when the newly created
 * thread starts executing
 * @param arg1 the first argument to func
 * @param arg2 the second argument to func
 * @return the newly created thread
 *
 */
kthread_t *kthread_create(struct proc *proc, kthread_func_t func, long arg1,
                          void *arg2);

/**
 * Creates a clone of the specified thread
 *
 * @param thr the thread to clone
 * @return a clone of thr
 */
kthread_t *kthread_clone(kthread_t *thr);

/**
 * Frees resources associated with a thread.
 *
 * @param thr the thread to free
 */
void kthread_destroy(kthread_t *thr);

/**
 * Cancels a thread.
 *
 * @param kthr the thread to be cancelled
 * @param retval the return value for the thread
 */
void kthread_cancel(kthread_t *kthr, void *retval);

/**
 * Exits the current thread.
 *
 * @param retval the return value for the thread
 */
void kthread_exit(void *retval);
