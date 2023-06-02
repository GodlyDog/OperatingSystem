#include "util/timer.h"
#include "proc/spinlock.h"
#include "util/time.h"

static timer_t *timer_running = NULL;
static uint64_t timer_next_expiry = -1;
static list_t timers_primary = LIST_INITIALIZER(timers_primary);
static list_t timers_secondary = LIST_INITIALIZER(timers_secondary);
static spinlock_t timers_spinlock = SPINLOCK_INITIALIZER(timers_spinlock);
static int timers_firing = 0;

void timer_init(timer_t *timer)
{
    timer->expires = -1;
    list_link_init(&timer->link);
}

void timer_add(timer_t *timer) { timer_mod(timer, timer->expires); }

int __timer_del(timer_t *timer)
{
    int ret = 0;
    if (list_link_is_linked(&timer->link))
    {
        list_remove(&timer->link);
        ret = 1;
    }
    return ret;
}

int timer_del(timer_t *timer)
{
    spinlock_lock(&timers_spinlock);
    int ret = __timer_del(timer);
    spinlock_unlock(&timers_spinlock);

    return ret;
}

void __timer_add(timer_t *timer)
{
    KASSERT(!list_link_is_linked(&timer->link));
    list_t *list = timers_firing ? &timers_secondary : &timers_primary;
    list_insert_head(list, &timer->link);
}

int timer_mod(timer_t *timer, int expires)
{
    spinlock_lock(&timers_spinlock);

    timer->expires = expires;
    int ret = __timer_del(timer);
    __timer_add(timer);
    timer_next_expiry = MIN(timer_next_expiry, timer->expires);

    spinlock_unlock(&timers_spinlock);
    return ret;
}

int timer_pending(timer_t *timer)
{
    spinlock_lock(&timers_spinlock);
    int ret = list_link_is_linked(&timer->link);
    spinlock_unlock(&timers_spinlock);
    return ret;
}

int timer_del_sync(timer_t *timer)
{
    /* Not great performance wise... */
    spinlock_lock(&timers_spinlock);
    while (timer_running == timer)
    {
        spinlock_unlock(&timers_spinlock);
        sched_yield();
        spinlock_lock(&timers_spinlock);
    }

    int ret = __timer_del(timer);
    spinlock_unlock(&timers_spinlock);

    return ret;
}

/* Note: using a linked-list rather than some priority is terribly inefficient
 * Also this implementation is just bad. Sorry.
 */
int ready = 0;
void __timers_fire()
{
    if (curthr && !preemption_enabled())
    {
        return;
    }

    spinlock_lock(&timers_spinlock);
    timers_firing = 1;

    //dbg(DBG_PRINT, "next expiry: %d\n", timer_next_expiry);
    if (jiffies < timer_next_expiry)
    {
        timers_firing = 0;
        spinlock_unlock(&timers_spinlock);
        return;
    }

    uint64_t min_expiry = 0;

    list_iterate(&timers_primary, timer, timer_t, link)
    {
        if (jiffies >= timer->expires)
        {
            list_remove(&timer->link);
            timer_running = timer;
            spinlock_unlock(&timers_spinlock);
            timer->function(timer->data);
            spinlock_lock(&timers_spinlock);
            timer_running = NULL;
        }
        else
        {
            min_expiry = MIN(min_expiry, timer->expires);
        }
    }

    /* migrate from the backup list to the primary list */
    list_iterate(&timers_secondary, timer, timer_t, link)
    {
        min_expiry = MIN(min_expiry, timer->expires);
        list_remove(&timer->link);
        list_insert_head(&timers_primary, &timer->link);
    }

    timer_next_expiry = min_expiry;
    timers_firing = 0;
    spinlock_unlock(&timers_spinlock);
}
