#pragma once

#include "errno.h"
#include "stddef.h"
#include "sys/types.h"
#include "weenix/syscall.h"

#define TRAP_INTR_STRING QUOTE(INTR_SYSCALL)

/* ssize_t will be 32 bits or 64 bits wide as appropriate.
   args are passed via %(r/e)ax and %(r/e)dx, so they need
   to be the size of a register.  */

static inline ssize_t trap(ssize_t num, ssize_t arg)
{
    ssize_t ret;
    __asm__ volatile("int $" TRAP_INTR_STRING
                     : "=a"(ret)
                     : "a"(num), "d"(arg));

    /* Copy in errno */
    __asm__ volatile("int $" TRAP_INTR_STRING
                     : "=a"(errno)
                     : "a"(SYS_errno));
    return ret;
}
