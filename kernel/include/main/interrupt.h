#pragma once

#include "kernel.h"
#include "types.h"
#include "util/debug.h"

// intr_disk_priamry/seconday so that they are different task priority classes
#define INTR_DIVIDE_BY_ZERO 0x00
#define INTR_INVALID_OPCODE 0x06
#define INTR_GPF 0x0d
#define INTR_PAGE_FAULT 0x0e

#define INTR_APICTIMER 0xf0
#define INTR_KEYBOARD 0xe0

#define INTR_DISK_PRIMARY 0xd0
#define INTR_SPURIOUS 0xfe
#define INTR_APICERR 0xff
#define INTR_SHUTDOWN 0xfd

/* NOTE: INTR_SYSCALL is not defined here, but is in syscall.h (it must be
 * in a userland-accessible header) */

// Intel Volume 3-A, 10.8.3.1 (10-29)
#define IPL_LOW 0
// we want to keep timer interrupts happening all the time to keep track of time
// :)
#define IPL_HIGH 0xe0
#define IPL_HIGHEST 0xff

typedef struct regs
{
    // all the regs
    uint64_t r_r15;
    uint64_t r_r14;
    uint64_t r_r13;
    uint64_t r_r12;
    uint64_t r_rbp;
    uint64_t r_rbx;
    uint64_t r_r11;
    uint64_t r_r10;
    uint64_t r_r9;
    uint64_t r_r8;
    uint64_t r_rax;
    uint64_t r_rcx;
    uint64_t r_rdx;
    uint64_t r_rsi;
    uint64_t r_rdi;

    // interrupt number
    uint64_t r_intr;

    // pushed by processor
    uint64_t r_err;
    uint64_t r_rip;
    uint64_t r_cs;
    uint64_t r_rflags;
    uint64_t r_rsp;
    uint64_t r_ss;
} packed regs_t;

void intr_init();

/* The function pointer which should be implemented by functions
 * which will handle interrupts. These handlers should be registered
 * with the interrupt subsystem via the intr_register function.
 * The regs structure contains the state of the registers saved when
 * the interrupt occured. Return whether or not the handler has itself
 * acknowledged the interrupt with a call to apic_eoi(). */
typedef long (*intr_handler_t)(regs_t *regs);

/* Registers an interrupt handler for the given interrupt handler.
 * If another handler had been previously registered for this interrupt
 * number it is returned, otherwise this function returns NULL. It
 * is good practice to assert that this function returns NULL unless
 * it is known that this will not be the case. */
intr_handler_t intr_register(uint8_t intr, intr_handler_t handler);

int32_t intr_map(uint16_t irq, uint8_t intr);

static inline uint64_t intr_enabled()
{
    uint64_t flags;
    __asm__ volatile("pushf; pop %0; and $0x200, %0;"
                     : "=r"(flags)::);
    return flags;
}

static inline void intr_enable() { __asm__ volatile("sti"); }

static inline void intr_disable() { __asm__ volatile("cli"); }

/* Atomically enables interrupts using the sti
 * instruction and puts the processor into a halted
 * state, this function returns once an interrupt
 * occurs. */
static inline void intr_wait()
{
    /* the sti instruction enables interrupts, however
     * interrupts are not checked for until the next
     * instruction is executed, this means that the following
     * code will not be succeptible to a bug where an
     * interrupt occurs between the sti and hlt commands
     * and does not wake us up from the hlt. */
    __asm__ volatile("sti; hlt");
}

/* Sets the interrupt priority level for hardware interrupts.
 * At initialization time devices should detect their individual
 * IPLs and save them for use with this function. IPL_LOW allows
 * all hardware interrupts. IPL_HIGH blocks all hardware interrupts */
uint8_t intr_setipl(uint8_t ipl);

/* Retreives the current interrupt priority level. */
uint8_t intr_getipl();

void dump_registers(regs_t *regs);
