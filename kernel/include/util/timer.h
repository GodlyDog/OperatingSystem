#ifndef TIMER_H
#define TIMER_H

#include "util/list.h"

typedef struct timer
{
    void (*function)(uint64_t data);
    uint64_t data;
    uint64_t expires;
    list_link_t link;
} timer_t;

void timer_init(timer_t *timer);

void timer_add(timer_t *timer);

int timer_del(timer_t *timer);

int timer_mod(timer_t *timer, int expires);

int timer_pending(timer_t *timer);

int timer_del_sync(timer_t *timer);

void __timers_fire();

#endif