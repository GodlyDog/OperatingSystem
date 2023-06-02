#pragma once

#include "types.h"
#include "util/debug.h"

extern uint64_t timer_tickcount;
extern uint64_t kernel_preempted_count;
extern uint64_t user_preempted_count;
extern uint64_t not_preempted_count;
extern uint64_t idle_count;
extern volatile uint64_t jiffies;

void time_init();

void time_spin(time_t ms);

void time_sleep(time_t ms);

long do_usleep(useconds_t usec);

time_t core_uptime();

time_t do_time();

size_t time_stats(char *buf, size_t len);
