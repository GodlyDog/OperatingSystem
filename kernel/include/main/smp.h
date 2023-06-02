#include "boot/config.h"
#include "mm/page.h"
#include "proc/core.h"

// For any given piece of global data, there are 4 cases we must protect
// against: (SMP.1) our core's other threads, (mutex or mask interrupts) (SMP.2)
// our core's interrupt handlers, and (mask interrupts) (SMP.3) other cores'
// threads, (mutex or spinlock) (SMP.4) other cores' interrupt handlers
// (spinlock) mask interrupts + spinlock covers all 4 cases!

#define GET_CSD(core, type, name) \
    ((type *)(csd_vaddr_table[(core)] + PAGE_OFFSET(&(name))))

extern uintptr_t csd_vaddr_table[];

void map_in_core_specific_data(pml4_t *pml4);

void smp_init();

void core_init();

long is_core_specific_data(void *addr);
