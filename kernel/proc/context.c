
#include "proc/context.h"
#include "proc/kthread.h"
#include <main/cpuid.h>

#include "main/apic.h"
#include "main/gdt.h"

typedef struct context_initial_func_args
{
    context_func_t func;
    long arg1;
    void *arg2;
} packed context_initial_func_args_t;

static void __context_thread_initial_func(context_initial_func_args_t args)
{
    preemption_reset();
    apic_setipl(IPL_LOW);
    intr_enable();

    void *result = (args.func)(args.arg1, args.arg2);
    kthread_exit(result);

    panic("\nReturned from kthread_exit.\n");
}

void context_setup_raw(context_t *c, void (*func)(), void *kstack,
                       size_t kstacksz, pml4_t *pml4)
{
    KASSERT(NULL != pml4);
    KASSERT(PAGE_ALIGNED(kstack));
    c->c_kstack = (uintptr_t)kstack;
    c->c_kstacksz = kstacksz;
    c->c_pml4 = pml4;
    c->c_rsp = (uintptr_t)kstack + kstacksz;
    c->c_rsp -= sizeof(uintptr_t);
    *((uintptr_t *)c->c_rsp) = 0;
    c->c_rbp = c->c_rsp;
    c->c_rip = (uintptr_t)func;
}

/*
 * Initializes a context_t struct with the given parameters. arg1 and arg2 will
 * appear as arguments to the function passed in when this context is first
 * used.
 */
void context_setup(context_t *c, context_func_t func, long arg1, void *arg2,
                   void *kstack, size_t kstacksz, pml4_t *pml4)
{
    KASSERT(NULL != pml4);
    KASSERT(PAGE_ALIGNED(kstack));

    c->c_kstack = (uintptr_t)kstack;
    c->c_kstacksz = kstacksz;
    c->c_pml4 = pml4;

    /* put the arguments for __context_thread_initial_func onto the
     * stack */
    c->c_rsp = (uintptr_t)kstack + kstacksz;
    c->c_rsp -= sizeof(arg2);
    *(void **)c->c_rsp = arg2;
    c->c_rsp -= sizeof(arg1);
    *(long *)c->c_rsp = arg1;
    c->c_rsp -= sizeof(context_func_t);
    *(context_func_t *)c->c_rsp = func;
    // Take space for the function return address (unused)
    c->c_rsp -= sizeof(uintptr_t);

    c->c_rbp = c->c_rsp;
    c->c_rip = (uintptr_t)__context_thread_initial_func;
}

/*
 * WARNING!! POTENTIAL EDITOR BEWARE!!
 * IF YOU REMOVE THE PT_SET CALLS BELOW,
 * YOU ***MUST*** DEAL WITH SMP TLB SHOOTDOWN
 *
 * IN OTHER WORDS, THINK *VERY* CAREFULLY BEFORE
 * REMOVING THE CALLS TO PT_SET BELOW
 */

void context_make_active(context_t *c)
{
    // gdt_set_kernel_stack((void *)((uintptr_t)c->c_kstack + c->c_kstacksz));
    pt_set(c->c_pml4);

    /* Switch stacks and run the thread */
    __asm__ volatile(
        "movq %0,%%rbp\n\t" /* update rbp */
        "movq %1,%%rsp\n\t" /* update rsp */
        "push %2\n\t"       /* save rip   */
        "ret"               /* jump to new rip */
        ::"m"(c->c_rbp),
        "m"(c->c_rsp), "m"(c->c_rip));
}

void context_switch(context_t *oldc, context_t *newc)
{
    gdt_set_kernel_stack(
        (void *)((uintptr_t)newc->c_kstack + newc->c_kstacksz));

    // sanity check that core-specific data is being managed (paged in)
    // correctly
    KASSERT(oldc->c_pml4 == pt_get());
    uintptr_t curthr_paddr =
        pt_virt_to_phys_helper(oldc->c_pml4, (uintptr_t)&curthr);
    uintptr_t new_curthr_paddr =
        pt_virt_to_phys_helper(newc->c_pml4, (uintptr_t)&curthr);

    kthread_t *prev_curthr = curthr;
    pt_set(newc->c_pml4);
    KASSERT(pt_get() == newc->c_pml4);

    KASSERT(curthr_paddr == new_curthr_paddr);
    KASSERT(prev_curthr == curthr);

    /*
     * Save the current value of the stack pointer and the frame pointer into
     * the old context. Set the instruction pointer to the return address
     * (whoever called us).
     */
    __asm__ volatile(
        "pushfq;"             /* save RFLAGS on the stack */
        "pushq  %%rbp     \n" /* save base pointer */
        "pushq  %%rbx     \n" /* save other callee-saved registers */
        "pushq  %%r12     \n"
        "pushq  %%r13     \n"
        "pushq  %%r14     \n"
        "pushq  %%r15     \n"
        "movq   %%rsp, %0 \n" /* save RSP into oldc */
        "movq   %2, %%rsp \n" /* restore RSP from newc */
        "pushq %%rax\n\t"
        "movabs  $1f, %%rax   \n\t" /* save RIP into oldc (saves the label '1'
                                       below) */
        "mov %%rax, %1\n\t"
        "popq %%rax\n\t"
        "pushq  %3        \n\t" /* restore RIP */
        "ret              \n\t"
        "1:\t"                  /* this is where oldc starts executing later */
        "popq   %%r15     \n\t" /* restore callee-saved registers */
        "popq   %%r14     \n\t"
        "popq   %%r13     \n\t"
        "popq   %%r12     \n\t"
        "popq   %%rbx     \n\t"
        "popq   %%rbp     \n\t" /* restore base pointer */
        "popfq"                 /* restore RFLAGS */
        : "=m"(oldc->c_rsp), "=m"(oldc->c_rip)
        : "m"(newc->c_rsp), "m"(newc->c_rip));
}
