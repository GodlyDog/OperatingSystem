#pragma once

#include "proc/spinlock.h"
#include "util/list.h"

/*===========
 * Structures
 *==========*/

/*
 * Queue structure for kthreads
 * Note that ktqueue functions are private - managing the queue
 *  should be done within sched.c, or using public functions
 */
typedef struct ktqueue
{
    list_t tq_list;
    size_t tq_size;
    spinlock_t tq_lock;
} ktqueue_t;

/*
 * Macro to initialize a ktqueue. See sched_queue_init for how the 
 * queue should be initialized in your code. 
 */
#define KTQUEUE_INITIALIZER(ktqueue)                                \
    {                                                               \
        .tq_list = LIST_INITIALIZER((ktqueue).tq_list),             \
        .tq_lock = SPINLOCK_INITIALIZER(sdf.tq_lock), .tq_size = 0, \
    }

/*
 * kthread declaration to make function signatures happy
 */
struct kthread;

/*==========
 * Functions
 *=========*/

/**
 * Runs a new thread from the run queue.
 *
 * @param queue the queue to place curthr on
 * @param lock a lock to release after curthr is no longer executing
 */
void sched_switch(ktqueue_t *queue, spinlock_t *lock);

/**
 * Helps with context switching.
 */
void core_switch();

/**
 * Yields the CPU to another runnable thread.
 */
void sched_yield();

/**
 * Enables a thread to be selected by the scheduler to run.
 *
 * @param thr the thread to make runnable
 */
void sched_make_runnable(struct kthread *thr);

/**
 * Causes the current thread to enter into an uncancellable sleep on
 * the given queue.
 *
 * @param q the queue to sleep on
 * @param lock optional lock for release in another context
 */
void sched_sleep_on(ktqueue_t *q, spinlock_t *lock);

/**
 * Causes the current thread to enter into a cancellable sleep on the
 * given queue.
 *
 * @param queue the queue to sleep on
 * @param lock optional lock for release in another context
 * @return -EINTR if the thread was cancelled and 0 otherwise
 */
long sched_cancellable_sleep_on(ktqueue_t *queue, spinlock_t *lock);

/**
 * Wakes up a thread from q.
 *
 * @param q queue
 * @param thrp if an address is provided, *thrp is set to the woken up thread
 *
 */
void sched_wakeup_on(ktqueue_t *q, struct kthread **thrp);

/**
 * Wake up all threads running on the queue.
 *
 * @param q the queue to wake up threads from
 */
void sched_broadcast_on(ktqueue_t *q);

/**
 * Cancel the given thread from the queue it sleeps on.
 *
 * @param the thread to cancel sleep from
 */
void sched_cancel(struct kthread *thr);

/**
 * Initializes a queue.
 *
 * @param queue the queue
 */
void sched_queue_init(ktqueue_t *queue);

/**
 * Returns true if the queue is empty.
 *
 * @param queue the queue
 * @return true if the queue is empty
 */
long sched_queue_empty(ktqueue_t *queue);

/**
 * Functions for managing the current thread's preemption status.
 */
void preemption_disable();
void preemption_enable();
void preemption_reset();
long preemption_enabled();