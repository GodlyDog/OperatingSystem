#pragma once

#include "types.h"

#include "mm/pagetable.h"

/*
 * The function pointer to be implemented by functions which are entry
 * points for new threads.
 */
typedef void *(*context_func_t)(long, void *);

typedef struct context
{
    uintptr_t c_rip; /* instruction pointer (RIP) */
    uintptr_t c_rsp; /* stack pointer (RSP) */
    uintptr_t c_rbp; /* frame pointer (RBP) */

    pml4_t
        *c_pml4; /* pointer to the top level page table (PML4) for this proc. 
                It's the 'root' of the page table where virtual address -> physical address
                lookup starts. */

    uintptr_t c_kstack;
    size_t c_kstacksz;
} context_t;

/**
 * Initialize the given context such that when it begins execution it
 * will execute func(arg1,arg2). A kernel stack and page directory 
 * exclusive to this context must also be provided.
 *
 * @param c the context to initialize
 * @param func the function which will begin executing when this
 * context is first made active
 * @param arg1 the first argument to func
 * @param arg2 the second argument to func
 * @param kstack a pointer to the kernel stack this context will use
 * @param kstacksz the size of the kernel stack
 * @param pdptr the pagetable this context will use
 */
void context_setup(context_t *c, context_func_t func, long arg1, void *arg2,
                   void *kstack, size_t kstacksz, pml4_t *pml4);

void context_setup_raw(context_t *c, void (*func)(), void *kstack,
                       size_t kstacksz, pml4_t *pml4);
/**
 * Makes the given context the one currently running on the CPU. Use
 * this mainly for the initial context.
 *
 * @param c the context to make active
 */
void context_make_active(context_t *c);

/**
 * Save the current state of the machine into the old context, and begin
 * executing the new context. Used primarily by the scheduler.
 *
 * @param oldc the context to switch from
 * @param newc the context to switch to
 */
void context_switch(context_t *oldc, context_t *newc);
