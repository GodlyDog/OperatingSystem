#pragma once

#include "proc/context.h"
#include "proc/sched.h"
#include "proc/spinlock.h"

typedef struct core
{
    long kc_id;
    context_t kc_ctx;

    ktqueue_t *kc_queue;
    spinlock_t *kc_lock;

    uintptr_t kc_csdpaddr;
} core_t;
