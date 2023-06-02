#pragma once

#include "main/interrupt.h"
#include "types.h"

typedef enum
{
    DESTINATION_MODE_FIXED = 0,
    DESTINATION_MODE_LOWEST_PRIORITY = 1,
    DESTINATION_MODE_SMI = 2,
    DESTINATION_MODE_NMI = 4,
    DESTINATION_MODE_INIT = 5,
    DESTINATION_MODE_SIPI = 6
} ipi_destination_mode;

#define MAX_LAPICS 8

/* Initializes the APIC using data from the ACPI tables.
 * ACPI handlers must be initialized before calling this
 * function. */
void apic_init();

/* Returns the APIC ID of the current processor */
long apic_current_id();

/* Returns the largest known APIC ID */
long apic_max_id();

/* Maps the given IRQ to the given interrupt number. */
void apic_setredir(uint32_t irq, uint8_t intr);

void apic_enable();

// timer interrupts arrive at a rate of (freq / 16) interrupts per millisecond
// (with an )
/* Starts the APIC timer */
void apic_enable_periodic_timer(uint32_t freq);

/* Stops the APIC timer */
void apic_disable_periodic_timer();

/* Sets the interrupt to raise when a spurious
 * interrupt occurs. */
void apic_setspur(uint8_t intr);

/* Sets the interrupt priority level. This function should
 * be accessed via wrappers in the interrupt subsystem. */
void apic_setipl(uint8_t ipl);

/* Gets the interrupt priority level. This function should
 * be accessed via wrappers in the interrupt subsystem. */
uint8_t apic_getipl();

long apic_initialized();

/* Writes to the APIC's memory mapped end-of-interrupt
 * register to indicate that the handling of an interrupt
 * originating from the APIC has been finished. This function
 * should only be called from the interrupt subsystem. */
void apic_eoi();

void apic_start_processor(uint8_t target, uint8_t execution_page);

void apic_send_ipi(uint8_t target, ipi_destination_mode destination_mode,
                   uint8_t vector);

void apic_broadcast_ipi(ipi_destination_mode mode, uint8_t vector,
                        long include_self);

/**
 * Wait for the last IPI sent to be acknowledged by the target processor.
 */
void apic_wait_ipi();