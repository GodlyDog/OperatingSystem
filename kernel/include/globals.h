#pragma once

#include "main/smp.h"
#include "proc/kthread.h"
#include "proc/proc.h"

#define CORE_SPECIFIC_DATA __attribute__((section(".csd"))) = {0}

extern core_t curcore;
extern proc_t *curproc;
extern kthread_t *curthr;
