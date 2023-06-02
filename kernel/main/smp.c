#include "globals.h"
#include "types.h"
#include <main/gdt.h>

#include "main/apic.h"
#include "main/inits.h"

#include "mm/tlb.h"

#include "util/string.h"
#include "util/time.h"

static long smp_processor_count;
static spinlock_t smp_startup_spinlock =
    SPINLOCK_INITIALIZER(smp_startup_spinlock);

extern uintptr_t smp_initialization_start;
extern uintptr_t smp_initialization_end;
#define smp_initialization_start ((uintptr_t)(&smp_initialization_start))
#define smp_initialization_end ((uintptr_t)(&smp_initialization_end))
#define smp_initialization_size \
    (smp_initialization_end - smp_initialization_start)

static void smp_start_processor(uint8_t apic_id);
static long smp_stop_processor(regs_t *regs);

extern void *csd_start;
extern void *csd_end;
#define CSD_START ((uintptr_t)&csd_start)
#define CSD_END ((uintptr_t)&csd_end)
#define CSD_PAGES (uintptr_t)((CSD_END - CSD_START) >> PAGE_SHIFT)

core_t curcore CORE_SPECIFIC_DATA;
uintptr_t csd_vaddr_table[MAX_LAPICS] = {NULL};

void map_in_core_specific_data(pml4_t *pml4)
{
    pt_map_range(pml4, curcore.kc_csdpaddr, CSD_START, CSD_END,
                 PT_PRESENT | PT_WRITE, PT_PRESENT | PT_WRITE);
    uintptr_t mapped_paddr = pt_virt_to_phys_helper(pml4, (uintptr_t)&curcore);
    uintptr_t expected_paddr =
        (uintptr_t)GET_CSD(curcore.kc_id, core_t, curcore) - PHYS_OFFSET;
    uintptr_t expected_paddr2 =
        pt_virt_to_phys_helper(pt_get(), (uintptr_t)&curcore);
    KASSERT(mapped_paddr == expected_paddr);
    KASSERT(expected_paddr == expected_paddr2);
}

long is_core_specific_data(void *addr)
{
    return (uintptr_t)addr >= CSD_START && (uintptr_t)addr < CSD_END;
}

void core_init()
{
    // order of operations are pretty important here
    pt_init();
    pt_set(pt_create());

    uintptr_t csd_paddr = (uintptr_t)page_alloc_n(CSD_PAGES);
    if (!csd_paddr)
        panic("not enough memory for core-specific data!");
    csd_vaddr_table[apic_current_id()] =
        csd_paddr; // still in PHYSMAP region; still a VMA
    csd_paddr -= PHYS_OFFSET;

    dbg(DBG_CORE, "mapping in core specific data to 0x%p\n", (void *)csd_paddr);
    pt_map_range(pt_get(), csd_paddr, CSD_START, CSD_END, PT_PRESENT | PT_WRITE,
                 PT_PRESENT | PT_WRITE);
    tlb_flush_all();

    memset((void *)CSD_START, 0, CSD_END - CSD_START);

    curcore.kc_id = apic_current_id();
    curcore.kc_queue = NULL;
    curcore.kc_lock = NULL;
    curcore.kc_csdpaddr = csd_paddr;

    intr_init();
    gdt_init();

    apic_enable();
    time_init();
    sched_init();

    void *stack = page_alloc();
    KASSERT(stack != NULL);

    context_setup_raw(&curcore.kc_ctx, core_switch, stack, PAGE_SIZE, pt_get());
}

void __attribute__((used)) smp_processor_entry()
{
    core_init();
    spinlock_lock(&smp_startup_spinlock);
    dbg_force(DBG_CORE, "started C%ld!\n", curcore.kc_id);
    smp_processor_count++;

    KASSERT(!intr_enabled());
    preemption_disable();
    proc_idleproc_init();
    curcore.kc_lock = &smp_startup_spinlock;
    context_make_active(&curcore.kc_ctx);
}

/*
 * Prepare for SMP by copying the real-mode trampoline code into the
 * first 1mb of memory.
 */
void smp_init()
{
    NOT_YET_IMPLEMENTED("SMP: smp_init");
}

// Intel Vol. 3A 10-11, 10.4.7.3
static void smp_start_processor(uint8_t apic_id)
{
    // TODO: not necessarily true that apic_id == processor_id
    dbg_force(DBG_CORE, "Booting C%d\n", apic_id);

    memcpy((void *)PHYS_OFFSET, (void *)smp_initialization_start,
           smp_initialization_size);

    // First, send a INIT IPI

    long prev_count = smp_processor_count;
    apic_start_processor(apic_id, 0);

    while (smp_processor_count == prev_count)
        ;

    // by the time we get the spinlock, we are guaranteed that the core we are
    // starting now has its own proper context and is no longer using the temp
    // stack in the first 4 MB of RAM
    spinlock_lock(&smp_startup_spinlock);
    spinlock_unlock(&smp_startup_spinlock);
}

static long smp_stop_processor(regs_t *regs)
{
    char buf[2048];
    time_stats(buf, sizeof(buf));

    dbg_force(DBG_CORE, "\n%s\nhalted cleanly!\n\n", buf);

    __asm__ volatile("cli; hlt;");

    return 0;
}
