#include "types.h"

#include "boot/config.h"

#include "main/acpi.h"
#include "main/apic.h"
#include "main/cpuid.h"
#include "main/interrupt.h"
#include "main/io.h"

#define APIC_SIGNATURE (*(uint32_t *)"APIC")

#define TYPE_LAPIC 0
#define TYPE_IOAPIC 1

/* For disabling interrupts on the 8259 PIC, it needs to be
 * disabled to use the APIC
 */
#define PIC_COMPLETE_MASK 0xff

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC1_VECTOR 0x20

#define PIC2 0xa0
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC2_VECTOR 0x28

#define ICW1_ICW4 0x01      /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

/* For enabling interrupts from the APIC rather than the
 * Master PIC, use the Interrupt Mode Configuration Register (IMCR)
 */

#define SELECT_REGISTER 0x22
#define IMCR_REGISTER 0x70
#define ENABLE_APIC 0x23
#define ENABLE_APIC_PORT 0x01

/* For Local APICS */
#define IA32_APIC_BASE_MSR 0x1b
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define LOCAL_APIC_SPURIOUS_REGISTER 0xf0
#define LOCAL_APIC_ENABLE_INTERRUPT 0x100

#define LOCAL_APIC_ID 0x20
#define LOCAL_APIC_VERSION 0x30
#define LOCAL_APIC_TASKPRIOR 0x80
#define LOCAL_APIC_EOI 0xb0
#define LOCAL_APIC_LDR 0xd0
#define LOCAL_APIC_DFR 0xe0
#define LOCAL_APIC_SPURIOUS 0xf0
#define LOCAL_APIC_ESR 0x280
#define LOCAL_APIC_ICRL 0x300
#define LOCAL_APIC_ICRH 0x310
#define LOCAL_APIC_LVT_TMR 0x320
#define LOCAL_APIC_LVT_PERF 0x340
#define LOCAL_APIC_LVT_LINT0 0x350
#define LOCAL_APIC_LVT_LINT1 0x360
#define LOCAL_APIC_LVT_ERR 0x370
#define LOCAL_APIC_TMRINITCNT 0x380
#define LOCAL_APIC_TMRCURRCNT 0x390
#define LOCAL_APIC_TMRDIV 0x3e0
#define LOCAL_APIC_LAST 0x38f
#define LOCAL_APIC_DISABLE 0x10000
#define LOCAL_APIC_SW_ENABLE 0x100
#define LOCAL_APIC_CPUFOCUS 0x200
#define LOCAL_APIC_NMI (4 << 8)
#define LOCAL_APIC_TMR_PERIODIC 0x20000
#define LOCAL_APIC_TMR_BASEDIV (1 << 20)

#define APIC_ADDR (apic->at_addr + PHYS_OFFSET)
#define APIC_REG(x) (*(uint32_t *)(APIC_ADDR + (x)))
#define LAPICID APIC_REG(LOCAL_APIC_ID)
#define LAPICVER APIC_REG(LOCAL_APIC_VERSION)
#define LAPICTPR APIC_REG(LOCAL_APIC_TASKPRIOR)
#define LAPICSPUR APIC_REG(LOCAL_APIC_SPURIOUS)
#define LAPICEOI APIC_REG(LOCAL_APIC_EOI)
#define LAPICDFR APIC_REG(LOCAL_APIC_DFR)
#define LAPICLDR APIC_REG(LOCAL_APIC_LDR)
#define LAPICLVTTMR APIC_REG(LOCAL_APIC_LVT_TMR)
#define LAPICLVTPERF APIC_REG(LOCAL_APIC_LVT_PERF)
#define LAPICLVTLINT0 APIC_REG(LOCAL_APIC_LVT_LINT0)
#define LAPICLVTLINT1 APIC_REG(LOCAL_APIC_LVT_LINT1)
#define LAPICLVTERR APIC_REG(LOCAL_APIC_LVT_ERR)
#define LAPICTIC APIC_REG(LOCAL_APIC_TMRINITCNT)
#define LAPICTCC APIC_REG(LOCAL_APIC_TMRCURRCNT)
#define LAPICTMRDIV APIC_REG(LOCAL_APIC_TMRDIV)
#define LAPICICRH APIC_REG(LOCAL_APIC_ICRH)
#define LAPICICRL APIC_REG(LOCAL_APIC_ICRL)
#define LAPICESR APIC_REG(LOCAL_APIC_ESR)

/* IO APIC */
#define IOAPIC_IOWIN 0x10

/* Some configuration for the IO APIC */
#define IOAPIC_ID 0x00
#define IOAPIC_VER 0x01
#define IOAPIC_ARB 0x02
#define IOAPIC_REDTBL 0x03

#define IOAPIC_ADDR (ioapic->at_addr + PHYS_OFFSET)
#define IOAPIC (*(uint32_t *)IOAPIC_ADDR)
#define IOAPICWIN (*(uint32_t *)(IOAPIC_ADDR + IOAPIC_IOWIN))

/* Helpful Macros for IO APIC programming */
#define BIT_SET(data, bit)                  \
    do                                      \
    {                                       \
        (data) = ((data) | (0x1 << (bit))); \
    } while (0);
#define BIT_UNSET(data, bit)                 \
    do                                       \
    {                                        \
        (data) = ((data) & ~(0x1 << (bit))); \
    } while (0);

#define IRQ_TO_OFFSET(irq, part) ((uint8_t)((0x10 + (irq * 2) + part)))

typedef struct apic_table
{
    struct acpi_header at_header;
    uint32_t at_addr;
    uint32_t at_flags;
} packed apic_table_t;

typedef struct lapic_table
{
    uint8_t at_type;
    uint8_t at_size;
    uint8_t at_procid;
    uint8_t at_apicid;
    uint32_t at_flags;
} packed lapic_table_t;

typedef struct ioapic_table
{
    uint8_t at_type;
    uint8_t at_size;
    uint8_t at_apicid;
    uint8_t at_reserved;
    uint32_t at_addr;
    uint32_t at_inti;
} packed ioapic_table_t;

static apic_table_t *apic = NULL;
static ioapic_table_t *ioapic = NULL;

// Use MAX_LAPICS + 1 entries so we can guarantee the last entry is null
static lapic_table_t *lapics[MAX_LAPICS + 1] = {NULL};
static long max_apicid;

static long initialized = 0;

// Returns the maximum APIC ID
inline long apic_max_id() { return max_apicid; }

/* [APIC  ID------------------------] */
inline static long __lapic_getid(void) { return (LAPICID >> 24) & 0xff; }

// Returns the APIC ID of the current processor/core
inline long apic_current_id() { return __lapic_getid(); }

inline static uint32_t __lapic_getver(void) { return LAPICVER & 0xff; }

inline static void __lapic_setspur(uint8_t intr)
{
    uint32_t data = LAPICSPUR | LOCAL_APIC_SW_ENABLE;
    *((uint8_t *)&data) = intr;
    LAPICSPUR = data;
}

/* [LOGICID-------------------------] */
inline static void __lapic_setlogicalid(uint8_t id)
{
    LAPICLDR = ((uint32_t)id) << 24;
}

inline static uint32_t ioapic_read(uint8_t reg_offset)
{
    /* Tell IOREGSEL where we want to read from */
    IOAPIC = reg_offset;
    return IOAPICWIN;
}

inline static void ioapic_write(uint8_t reg_offset, uint32_t value)
{
    /* Tell IOREGSEL where to write to */
    IOAPIC = reg_offset;
    /* Write the value to IOWIN */
    IOAPICWIN = value;
}

inline static uint32_t __ioapic_getid(void)
{
    return (ioapic_read(IOAPIC_ID) >> 24) & 0x0f;
}

inline static uint32_t __ioapic_getver(void)
{
    return ioapic_read(IOAPIC_VER) & 0xff;
}

inline static uint32_t __ioapic_getmaxredir(void)
{
    return (ioapic_read(IOAPIC_VER) >> 16) & 0xff;
}

inline static void __ioapic_setredir(uint32_t irq, uint8_t intr)
{
    /* Read in the redirect table lower register first */
    uint32_t data = ioapic_read(IRQ_TO_OFFSET(irq, 0));
    /* Set the interrupt vector */
    ((uint8_t *)&data)[0] = intr;
    /* Set bit 8, unset bits 9,10 to set interrupt delivery mode to lowest
     * priority */
    BIT_SET(data, 8);
    BIT_UNSET(data, 9);
    BIT_UNSET(data, 10);
    /* Set bit 11 to set the destination mode to a logical destination */
    BIT_SET(data, 11);
    /* Unset bit 13 to set the pin polarity to Active High */
    BIT_UNSET(data, 13);
    /* Unset bit 15 to set the trigger mode to Edge */
    BIT_UNSET(data, 15);
    /* Write this value to the apic */
    ioapic_write(IRQ_TO_OFFSET(irq, 0), data);
    /* Now deal with the higher order register */
    data = ioapic_read(IRQ_TO_OFFSET(irq, 1));
    ((uint8_t *)&data)[3] = 0xff;
    ioapic_write(IRQ_TO_OFFSET(irq, 1), data);
}

inline static void __ioapic_setmask(uint32_t irq, int mask)
{
    uint32_t data = ioapic_read(IRQ_TO_OFFSET(irq, 0));
    if (mask)
    {
        BIT_SET(data, 16);
    }
    else
    {
        BIT_UNSET(data, 16);
    }
    ioapic_write(IRQ_TO_OFFSET(irq, 0), data);
}

static uint32_t apic_exists(void)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(CPUID_GETFEATURES, &eax, &ebx, &ecx, &edx);
    return edx & CPUID_FEAT_EDX_APIC;
}

static void apic_set_base(uint32_t apic)
{
    uint32_t edx = 0;
    uint32_t eax = (apic & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;
    edx = 0;
    cpuid_set_msr(IA32_APIC_BASE_MSR, eax, edx);
}

static uint32_t apic_get_base(void)
{
    uint32_t eax, edx;
    cpuid_get_msr(IA32_APIC_BASE_MSR, &eax, &edx);
    return (eax & 0xfffff000);
}

static long __apic_err()
{
    dbg(DBG_PRINT, "[+] APIC Error: 0x%d", LAPICESR);
    __asm__("cli; hlt");
    return 0;
}

void apic_enable()
{
    // [MODE---------------------------]
    //     L
    LAPICDFR = 0xffffffff;

    KASSERT(apic_current_id() < 8);
    __lapic_setlogicalid((uint8_t)(1 << apic_current_id()));
    LAPICLVTTMR = LOCAL_APIC_DISABLE;
    LAPICLVTPERF = LOCAL_APIC_NMI;
    LAPICLVTLINT0 = LOCAL_APIC_DISABLE;
    LAPICLVTLINT1 = LOCAL_APIC_DISABLE;
    LAPICLVTERR = INTR_APICERR;
    LAPICTPR = 0;
    apic_set_base(apic_get_base());
    apic_setspur(INTR_SPURIOUS);
    intr_register(INTR_APICERR, __apic_err);
}

void apic_disable_periodic_timer()
{
    LAPICLVTTMR = LOCAL_APIC_DISABLE;
    LAPICLVTPERF = LOCAL_APIC_NMI;
    LAPICLVTLINT0 = LOCAL_APIC_DISABLE;
    LAPICLVTLINT1 = LOCAL_APIC_DISABLE;
    LAPICTPR = 0;
}

/* get_cpu_bus_frequency - Uses PIT to determine APIC frequency in Hz (ticks per
 * second). NOTE: NOT SMP FRIENDLY! Note: For more info, visit the osdev wiki
 * page on the Programmable Interval Timer. */
static uint32_t get_cpu_bus_frequency()
{
    static uint32_t freq = 0;
    if (!freq)
    {
        /* Division rate: 0b1011 corresponds to division by 1, which does
         * nothing. */
        LAPICTMRDIV = 0b1011;

        /* 0x61 controls the PC speaker.
         * Clearing bit 1 prevents any sound.
         * Setting bit 0 connects the speaker to the output of PIT channel 2. */
        outb(0x61, (uint8_t)((inb(0x61) & 0xfd) | 1));

        /* Control reg:
         * 0x1011 = Channel 2, lobyte/hibyte access
         * 0x0010 = Mode 1 (hardware one-shot) */
        outb(0x43, 0xb2);

        /* Not sure why there's an inb, but the two outb send the reload value:
         * 0x2e9b = 11931, aka 1/100th of the PIT oscillator rate, aka 10 ms. */
        outb(0x42, 0x9b);
        inb(0x60);
        outb(0x42, 0x2e);

        /* Reset the one-shot counter by clearing and resetting bit 0. */
        uint32_t tmp = (uint32_t)(inb(0x61) & 0xfe);
        outb(0x61, (uint8_t)tmp);
        outb(0x61, (uint8_t)(tmp | 1));
        /* Reset APIC's initial countdown value. */
        LAPICTIC = 0xffffffff;
        /* PC speaker sets bit 5 when it hits 0. */
        while (!(inb(0x61) & 0x20))
            ;
        /* Stop the APIC timer */
        LAPICLVTTMR = LOCAL_APIC_DISABLE;
        /* Subtract current count from the initial count to get total ticks per
         * second. */
        freq = (LAPICTIC - LAPICTCC) * 100;
        dbgq(DBG_CORE, "CPU Bus Freq: %u ticks per second\n", freq);
    }
    return freq;
}

/* apic_enable_periodic_timer - Starts the periodic timer (continuously send
 * interrupts) at a given frequency. For more information, refer to: Intel
 * System Programming Guide, Vol 3A Part 1, 10.5.4. */
void apic_enable_periodic_timer(uint32_t freq)
{
    // TODO: Check this math! Don't assume it's correct...

    uint32_t ticks_per_second = get_cpu_bus_frequency();
    /* Demand at least the desired precision. */
    if (ticks_per_second < freq)
    {
        panic(
            "apic timer is not precise enough for desired frequency\n");
    }

    /* TODO: Pretty sure this can be more precise using the initial count
     * properly. */

    /* Round the bus frequency down to the nearest multiple of the desired
     * frequency. If bus/freq is large, the remainder will get amortized to a
     * degree that should be acceptable for Weenix. */
    uint32_t rem = ticks_per_second % freq;
    if (rem > (freq / 2))
        ticks_per_second += (freq - rem);
    else
        ticks_per_second -= rem;
    // TODO: Provide a warning when there is a lot of drift, e.g. more than
    // 1/10th inaccuracy per interval

    /* Divide configuration. */
    uint32_t div = 0b0111; /* Starts at division by 1. */
    uint32_t tmp = ticks_per_second;
    for (int i = 1; i < 7; i++)
    { /* Max division is 2^7. */
        /* Don't cut the freq in half if it would ruin divisibility. */
        if ((tmp >> 1) % freq != 0)
            break;
        if ((tmp >> 1) < freq)
            break;
        /* Cut freq in half. */
        tmp >>= 1;
        /* Increment the order of division (1, 2, 4, ...). */
        div++;
    }

    uint32_t tmpdiv = div;

    /* Clear bit 3, which probably artificially overflowed. */
    div &= 0b0111;

    /* APIC DIV register skips bit 2, so if set, move it to bit 3. */
    if (div & 0b0100)
    {
        div &= 0b0011; /* Clear bit 2. */
        div |= 0b1011; /* Set bit 3. */
    }

    /* Set up three registers to configure timer:
     * 1) Initial count: count down from this value, send interrupt upon hitting
     * 0. */
    LAPICTIC = tmp / freq;
    /* 3) Divide config: calculated above to cut bus clock. */
    LAPICTMRDIV = div;
    /* 2) LVT timer: use a periodic timer and raise the provided interrupt
     * vector. */
    LAPICLVTTMR = LOCAL_APIC_TMR_PERIODIC | INTR_APICTIMER;
}

static void apic_disable_8259()
{
    dbgq(DBG_CORE, "--- DISABLE 8259 PIC ---\n");
    /* disable 8259 PICs by initializing them and masking all interrupts */
    /* the first step is initialize them normally */
    outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, PIC1_VECTOR);
    io_wait();
    outb(PIC2_DATA, PIC2_VECTOR);
    io_wait();
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    /* Now mask all interrupts */
    dbgq(DBG_CORE, "Masking all interrupts on the i8259 PIC\n");
    outb(PIC1_DATA, PIC_COMPLETE_MASK);
    outb(PIC2_DATA, PIC_COMPLETE_MASK);
}

static void map_apic_addr(uintptr_t paddr)
{
    page_mark_reserved((void *)paddr);
    pt_map(pt_get(), paddr, paddr + PHYS_OFFSET, PT_WRITE | PT_PRESENT,
           PT_WRITE | PT_PRESENT);
}

void apic_init()
{
    uint8_t *ptr = acpi_table(APIC_SIGNATURE, 0);
    apic = (apic_table_t *)ptr;
    KASSERT(NULL != apic && "APIC table not found in ACPI.");

    apic_disable_8259();

    dbgq(DBG_CORE, "--- APIC INIT ---\n");
    dbgq(DBG_CORE, "local apic paddr:     0x%x\n", apic->at_addr);
    dbgq(DBG_CORE, "PC-AT compatible:    %i\n", apic->at_flags & 0x1);
    KASSERT(PAGE_ALIGNED((void *)(uintptr_t)apic->at_addr));

    KASSERT(apic->at_addr < 0xffffffff);

    map_apic_addr(apic->at_addr);

    /* Get the tables for the local APIC and IO APICS */
    uint8_t off = sizeof(*apic);
    while (off < apic->at_header.ah_size)
    {
        uint8_t type = *(ptr + off);
        uint8_t size = *(ptr + off + 1);
        lapic_table_t *lapic = NULL;
        if (TYPE_LAPIC == type)
        {
            KASSERT(apic_exists() && "Local APIC does not exist");
            KASSERT(sizeof(lapic_table_t) == size);
            lapic = (lapic_table_t *)(ptr + off);
            KASSERT(lapic->at_apicid < MAX_LAPICS &&
                    "Weenix only supports MAX_LAPICS local APICs");
            lapics[lapic->at_apicid] = lapic;

            page_mark_reserved(PAGE_ALIGN_DOWN((uintptr_t)lapic - PHYS_OFFSET));
            max_apicid = lapic->at_apicid;

            dbgq(DBG_CORE, "LAPIC:\n");
            dbgq(DBG_CORE, "   id:         0x%.2x\n",
                 (uint32_t)lapic->at_apicid);
            dbgq(DBG_CORE, "   processor:  0x%.3x\n",
                 (uint32_t)lapic->at_procid);
            dbgq(DBG_CORE, "   enabled:    %i\n", apic->at_flags & 0x1);
        }
        else if (TYPE_IOAPIC == type)
        {
            KASSERT(apic_exists() && "IO APIC does not exist");
            KASSERT(sizeof(ioapic_table_t) == size);
            KASSERT(NULL == ioapic && "Weenix only supports a single IO APIC");
            ioapic = (ioapic_table_t *)(ptr + off);
            page_mark_reserved(
                PAGE_ALIGN_DOWN((uintptr_t)ioapic - PHYS_OFFSET));
            map_apic_addr(ioapic->at_addr);

            dbgq(DBG_CORE, "IOAPIC:\n");
            dbgq(DBG_CORE, "   id:         0x%.2x\n",
                 (uint32_t)ioapic->at_apicid);
            dbgq(DBG_CORE, "   base paddr:  0x%.8x\n", ioapic->at_addr);
            dbgq(DBG_CORE, "   inti addr:   0x%.8x\n", ioapic->at_inti);
            KASSERT(PAGE_ALIGNED((void *)(uintptr_t)ioapic->at_addr));
        }
        else
        {
            dbgq(DBG_CORE, "Unknown APIC type:  0x%x\n", (uint32_t)type);
        }
        off += size;
    }
    KASSERT(NULL != lapics[apic_current_id()] &&
            "Could not find a local APIC device");
    KASSERT(NULL != ioapic && "Could not find an IO APIC");

    initialized = 1;
}

inline long apic_initialized() { return initialized; }

inline uint8_t apic_getipl() { return (uint8_t)LAPICTPR; }

inline void apic_setipl(uint8_t ipl) { LAPICTPR = ipl; }

inline void apic_setspur(uint8_t intr)
{
    dbg(DBG_CORE, "mapping spurious interrupts to %u\n", intr);
    __lapic_setspur(intr);
}

inline void apic_eoi() { LAPICEOI = 0x0; }

void apic_setredir(uint32_t irq, uint8_t intr)
{
    dbg(DBG_CORE, "redirecting irq %u to interrupt %u\n", irq, intr);
    __ioapic_setredir(irq, intr);
    __ioapic_setmask(irq, 0);
}

void apic_start_processor(uint8_t processor, uint8_t execution_page)
{
    // [+] TODO FIX MAGIC NUMBERS
    KASSERT(processor < 8);
    uint32_t icr_low = 0;
    icr_low |= 0;
    icr_low |= DESTINATION_MODE_INIT << 8;
    BIT_UNSET(icr_low, 11); // physical destination

    BIT_SET(icr_low, 14);
    BIT_UNSET(icr_low, 15);

    dbg(DBG_CORE, "Sending IPI: ICR_LOW = 0x%.8x, ICR_HIGH = 0x%.8x\n", icr_low,
        processor << 24);
    LAPICICRH = processor << 24;
    LAPICICRL = icr_low;

    apic_wait_ipi();

    icr_low = 0;
    icr_low |= execution_page;
    icr_low |= DESTINATION_MODE_SIPI << 8;
    BIT_UNSET(icr_low, 11); // physical destination

    BIT_SET(icr_low, 14);
    BIT_UNSET(icr_low, 15);
    dbg(DBG_CORE, "Sending IPI: ICR_LOW = 0x%.8x, ICR_HIGH = 0x%.8x\n", icr_low,
        processor << 24);

    LAPICICRH = processor << 24;
    LAPICICRL = icr_low;

    apic_wait_ipi();
}

void apic_send_ipi(uint8_t target, ipi_destination_mode mode, uint8_t vector)
{
    // See https://wiki.osdev.org/APIC#Interrupt_Command_Register for a
    // description of how this works. This function only supports targeting a
    // single APIC, instead of using the special destination modes. Since we
    // already parse the APIC table, it's more reliable to interrupt a specific
    // processor.
    KASSERT(target < 8);

    uint32_t icr_low = 0;
    icr_low |= vector;    // bits 0-7 are the vector number
    icr_low |= mode << 8; // bits 8-10 are the destination mode
    BIT_SET(icr_low, 11); // logical destination

    BIT_SET(icr_low, 14);

    dbgq(DBG_CORE, "Sending IPI: ICR_LOW = 0x%.8x, ICR_HIGH = 0x%.8x\n",
         icr_low, (1U << target) << 24);

    // Bits 24-27 of ICR_HIGH are the target logical APIC ID. Setting ICR_LOW
    // sends the interrupt, so we have to set this first
    LAPICICRH = (1U << target) << 24;
    // send the IPI
    LAPICICRL = icr_low;
}

void apic_broadcast_ipi(ipi_destination_mode mode, uint8_t vector,
                        long include_self)
{
    uint32_t icr_low = 0;
    icr_low |= vector;
    icr_low |= mode << 8;
    BIT_SET(icr_low, 11);
    BIT_SET(icr_low, 14);

    if (!include_self)
        BIT_SET(icr_low, 18);
    BIT_SET(icr_low, 19);

    LAPICICRH = 0;
    LAPICICRL = icr_low;
}

/**
 * Wait for the last IPI sent to be acknowledged by the other processor.
 *
 * Note: this is separate from apic_send_ipi because there are circumstances
 * where we don't want to wait.
 */
void apic_wait_ipi()
{
    // Bit 12 of ICR_LOW is the delivery status flag.
    while (LAPICICRL & (1 << 12))
        ;
}
