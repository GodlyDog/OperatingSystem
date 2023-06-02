#include "util/time.h"
#include "drivers/cmos.h"
#include "main/apic.h"
#include "proc/sched.h"
#include "util/printf.h"
#include "util/timer.h"
#include <drivers/screen.h>

#define TIME_APIC_TICK_FREQUENCY 16
// this is pretty wrong...
#define MICROSECONDS_PER_APIC_TICK (16 * 1000 / TIME_APIC_TICK_FREQUENCY)

volatile uint64_t jiffies;
uint64_t timer_tickcount CORE_SPECIFIC_DATA;
uint64_t kernel_preempted_count CORE_SPECIFIC_DATA;
uint64_t user_preempted_count CORE_SPECIFIC_DATA;
uint64_t not_preempted_count CORE_SPECIFIC_DATA;
uint64_t idle_count CORE_SPECIFIC_DATA;

// (freq / 16) interrupts per millisecond
static long timer_tick_handler(regs_t *regs)
{
    timer_tickcount++;

#ifdef __VGABUF__
    if (timer_tickcount % 128 == 0)
        screen_flush();
#endif

    if (curcore.kc_id == 0)
    {
        jiffies = timer_tickcount;
        __timers_fire();
    }

#ifdef __KPREEMPT__ // if (preemption_enabled()) {
    (regs->r_cs & 0x3) ? user_preempted_count++ : kernel_preempted_count++;
    apic_eoi();
    if (regs->r_cs & 0x3 && curthr->kt_cancelled)
        kthread_exit((void *)-1);
    sched_yield();
    return 1;

#endif
#ifndef __KPREEMPT__ //} else {
    curthr ? not_preempted_count++ : idle_count++;
    return 0;
#endif //}

    return 0;
}

void time_init()
{
    timer_tickcount = 0;
    intr_register(INTR_APICTIMER, timer_tick_handler);
    apic_enable_periodic_timer(TIME_APIC_TICK_FREQUENCY);
}

void time_spin(uint64_t ms)
{
    uint64_t ticks_to_wait = ms * TIME_APIC_TICK_FREQUENCY / 16;
    uint64_t target = timer_tickcount + ticks_to_wait;
    dbg(DBG_SCHED, "spinning for %lu ms (%lu APIC ticks)\n", ms, ticks_to_wait);
    while (timer_tickcount < target)
        ;
}

void time_sleep(uint64_t ms)
{
    // TODO make curthr runnable and place on runqueue
    time_spin(ms);
}

inline time_t core_uptime()
{
    return (MICROSECONDS_PER_APIC_TICK * timer_tickcount) / 1000;
}

static int mdays[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

time_t do_time()
{
    rtc_time_t tm = rtc_get_time();
    // dbg(DBG_SCHED, "rtc_time (Y-M-D:hh:mm:ss): %d-%d-%d:%d:%d:%d\n", tm.year,
    // tm.month, tm.day, tm.hour, tm.minute, tm.second);

    int yday = mdays[tm.month - 1] + tm.day - 1;
    if (tm.month >= 3 && (((tm.year % 4 == 0) && (tm.year % 100 != 0)) ||
                          (tm.year % 400 == 0)))
    {
        yday += 1;
    }
    tm.year -= 1900;

    /* oof */
    time_t unix_time =
        tm.second + tm.minute * 60 + tm.hour * 3600 + yday * 86400 +
        (tm.year - 70) * 31536000 + ((tm.year - 69) / 4) * 86400 -
        ((tm.year - 1) / 100) * 86400 + ((tm.year + 299) / 400) * 86400;

    return unix_time;
}

static size_t human_readable_format(char *buf, size_t size, uint64_t ticks)
{
    uint64_t milliseconds = core_uptime();
    uint64_t minutes = milliseconds / (60 * 1000);
    milliseconds -= minutes * 60 * 1000;
    uint64_t seconds = milliseconds / 1000;
    milliseconds -= seconds * 1000;
    return (size_t)snprintf(buf, size, "%lu min, %lu sec, %lu ms", minutes,
                            seconds, milliseconds);
}

static size_t percentage(char *buf, size_t size, uint64_t numerator,
                         uint64_t denominator)
{
    // 2 decimal points, no floats
    uint64_t new_numerator = numerator * 10000;
    if (new_numerator < numerator)
    {
        return (size_t)snprintf(buf, size, "N/A");
    }
    uint64_t result = denominator ? new_numerator / denominator : 0;
    return (size_t)snprintf(buf, size, "%lu.%02lu%%", result / 100,
                            result % 100);
}

size_t time_stats(char *buf, size_t len)
{
    size_t off = 0;
    off += snprintf(buf + off, len - off, "core uptime:\t");
    off += human_readable_format(buf + off, len - off, timer_tickcount);
    off += snprintf(buf + off, len - off, "\nidle time:\t");
    off += human_readable_format(buf + off, len - off, idle_count);
    off += snprintf(buf + off, len - off, "\t");
    off += percentage(buf + off, len - off, idle_count, timer_tickcount);

    KASSERT(not_preempted_count + user_preempted_count +
                kernel_preempted_count + idle_count - timer_tickcount <=
            2);

    off += snprintf(buf + off, len - off, "\n\ntotal tick count       = %lu",
                    timer_tickcount);
    off += snprintf(buf + off, len - off, "\nidle count             = %lu",
                    idle_count);
    off += snprintf(buf + off, len - off, "\t");
    off += percentage(buf + off, len - off, idle_count, timer_tickcount);
    off += snprintf(buf + off, len - off, "\nkernel preempted count = %lu",
                    kernel_preempted_count);
    off += snprintf(buf + off, len - off, "\t");
    off += percentage(buf + off, len - off, kernel_preempted_count,
                      timer_tickcount);
    off += snprintf(buf + off, len - off, "\nuser preempted count   = %lu",
                    user_preempted_count);
    off += snprintf(buf + off, len - off, "\t");
    off +=
        percentage(buf + off, len - off, user_preempted_count, timer_tickcount);
    off += snprintf(buf + off, len - off, "\nnot preempted count    = %lu",
                    not_preempted_count);
    off += snprintf(buf + off, len - off, "\t");
    off +=
        percentage(buf + off, len - off, not_preempted_count, timer_tickcount);

    return off;
}

static void do_wakeup(uint64_t arg)
{
    kthread_t *thr = (kthread_t *)arg;

    spinlock_lock(&thr->kt_lock);
    if (thr->kt_wchan)
    {
        sched_broadcast_on(thr->kt_wchan);
    }
    spinlock_unlock(&thr->kt_lock);
}

long do_usleep(useconds_t usec)
{
    ktqueue_t waitq;
    sched_queue_init(&waitq);

    timer_t timer;
    timer_init(&timer);
    timer.function = do_wakeup;
    timer.data = (uint64_t)curthr;
    timer.expires = jiffies + (usec / MICROSECONDS_PER_APIC_TICK);

    spinlock_lock(&curthr->kt_lock);
    timer_add(&timer);
    long ret = sched_cancellable_sleep_on(&waitq, &curthr->kt_lock);
    timer_del(&timer);
    spinlock_lock(&curthr->kt_lock);
    spinlock_unlock(&curthr->kt_lock);
    return ret;
}