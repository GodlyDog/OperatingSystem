#include "util/debug.h"
#include <util/string.h>

#include "main/gdt.h"

#include "api/binfmt.h"
#include "api/exec.h"
#include "api/syscall.h"

/* Enters userland from the kernel. Call this for a process that has up to now
 * been a kernel-only process. Takes the registers to start userland execution
 * with. Does not return. Note that the regs passed in should be on the current
 * stack of execution.
 */

void userland_entry(const regs_t regs)
{
    KASSERT(preemption_enabled());

    dbg(DBG_ELF, ">>>>>>>>>>>>>>> pid: %d\n", curproc->p_pid);

    intr_disable();
    dbg(DBG_ELF, ">>>>>>>>>>>>>>>> intr_disable()\n");
    intr_setipl(IPL_LOW);
    dbg(DBG_ELF, ">>>>>>>>>>>>>>>> intr_setipl()\n");

    __asm__ __volatile__(
        "movq %%rax, %%rsp\n\t" /* Move stack pointer up to regs */
        "popq %%r15\n\t"        /* Pop all general purpose registers (except rsp, */
        "popq %%r14\n\t"        /* which gets popped by iretq) */
        "popq %%r13\n\t"
        "popq %%r12\n\t"
        "popq %%rbp\n\t"
        "popq %%rbx\n\t"
        "popq %%r11\n\t"
        "popq %%r10\n\t"
        "popq %%r9\n\t"
        "popq %%r8\n\t"
        "popq %%rax\n\t"
        "popq %%rcx\n\t"
        "popq %%rdx\n\t"
        "popq %%rsi\n\t"
        "popq %%rdi\n\t"
        "add $16, %%rsp\n\t" /*
                              * Move stack pointer up to the location of the
                              * arguments automatically pushed by the processor
                              * on an interrupt
                              */
        "iretq\n"
        /* We're now in userland! */
        :            /* No outputs */
        : "a"(&regs) /* Forces regs to be in the 'a' register (%rax). */
    );
}

long do_execve(const char *filename, char *const *argv, char *const *envp,
               struct regs *regs)
{
    uint64_t rip, rsp;
    long ret = binfmt_load(filename, argv, envp, &rip, &rsp);
    if (ret < 0)
    {
        return ret;
    }
    /* Make sure we "return" into the start of the newly loaded binary */
    dbg(DBG_EXEC, "Executing binary with rip 0x%p, rsp 0x%p\n", (void *)rip,
        (void *)rsp);
    regs->r_rip = rip;
    regs->r_rsp = rsp;
    return 0;
}

/*
 * The kernel version of execve needs to construct a set of saved user registers
 * and fake a return from an interrupt to get to userland.  The 64-bit version
 * behaves mostly the same as the 32-bit version, but there are a few
 * differences. Besides different general purpose registers, there is no longer
 * a need for two esp/rsp fields since popa is not valid assembly in 64-bit. The
 * only non-null segment registers are now cs and ss, but they are set the same
 * as in 32-bit, although the segment descriptors they point to are slightly
 * different.
 */
void kernel_execve(const char *filename, char *const *argv, char *const *envp)
{
    uint64_t rip, rsp;
    long ret = binfmt_load(filename, argv, envp, &rip, &rsp);
    dbg(DBG_EXEC, "ret = %ld\n", ret);

    KASSERT(0 == ret); /* Should never fail to load the first binary */

    dbg(DBG_EXEC, "Entering userland with rip 0x%p, rsp 0x%p\n", (void *)rip,
        (void *)rsp);
    /* To enter userland, we build a set of saved registers to "trick" the
     * processor into thinking we were in userland before. Yes, it's horrible.
     * c.f. http://wiki.osdev.org/index.php?title=Getting_to_Ring_3&oldid=8195
     */
    regs_t regs;
    memset(&regs, 0, sizeof(regs_t));

    /* Userland gdt entries (0x3 for ring 3) */
    regs.r_cs = GDT_USER_TEXT | 0x3;
    regs.r_ss = GDT_USER_DATA | 0x3;

    /* Userland instruction pointer and stack pointer */
    regs.r_rip = rip;
    regs.r_rsp = rsp;

    regs.r_rflags = 0x202; // see 32-bit version
    userland_entry(regs);
}