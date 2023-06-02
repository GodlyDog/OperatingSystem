#include "main/gdt.h"
#include "globals.h"

#include "util/debug.h"
#include "util/printf.h"
#include "util/string.h"

typedef struct gdt_entry
{
    uint16_t ge_limitlo;
    uint16_t ge_baselo;
    uint8_t ge_basemid;
    uint8_t ge_access;
    uint8_t ge_flags;
    uint8_t ge_basehi;
} packed gdt_entry_t;

static gdt_entry_t gdt[GDT_COUNT] CORE_SPECIFIC_DATA;

typedef struct tss_entry
{
    uint32_t ts_reserved1;
    uint64_t ts_rsp0;
    uint64_t ts_rsp1;
    uint64_t ts_rsp2;
    uint64_t ts_reserved2;
    uint64_t ts_ist1;
    uint64_t ts_ist2;
    uint64_t ts_ist3;
    uint64_t ts_ist4;
    uint64_t ts_ist5;
    uint64_t ts_ist6;
    uint64_t ts_ist7;
    uint64_t ts_reserved3;
    uint16_t ts_iopb;
    uint16_t ts_reserved4;
} packed tss_entry_t;

typedef struct gdt_location
{
    uint16_t gl_size;
    uint64_t gl_offset;
} packed gdt_location_t;

static gdt_location_t gdtl = {.gl_size = GDT_COUNT * sizeof(gdt_entry_t),
                              .gl_offset = (uint64_t)&gdt};

static tss_entry_t tss CORE_SPECIFIC_DATA;

void gdt_init(void)
{
    memset(gdt, 0, sizeof(gdt));
    gdt_set_entry(GDT_KERNEL_TEXT, 0x0, 0xFFFFF, 0, 1, 0, 1);
    gdt_set_entry(GDT_KERNEL_DATA, 0x0, 0xFFFFF, 0, 0, 0, 1);
    gdt_set_entry(GDT_USER_TEXT, 0x0, 0xFFFFF, 3, 1, 0, 1);
    gdt_set_entry(GDT_USER_DATA, 0x0, 0xFFFFF, 3, 0, 0, 1);

    uintptr_t tss_pointer = (uintptr_t)&tss;
    gdt_set_entry(GDT_TSS, (uint32_t)tss_pointer, sizeof(tss), 0, 1, 0, 0);
    gdt[GDT_TSS / 8].ge_access &= ~(0b10000);
    gdt[GDT_TSS / 8].ge_access |= 0b1;
    gdt[GDT_TSS / 8].ge_flags &= ~(0b10000000);

    uint64_t tss_higher_half = ((uint64_t)tss_pointer) >> 32;
    memcpy(&gdt[GDT_TSS / 8 + 1], &tss_higher_half, 8);

    memset(&tss, 0, sizeof(tss));
    tss.ts_iopb = sizeof(tss);

    gdt_location_t *data = &gdtl;
    int segment = GDT_TSS;

    dbg(DBG_CORE, "Installing GDT and TR\n");
    __asm__ volatile("lgdt (%0); ltr %1" ::"p"(data), "m"(segment));
}

void gdt_set_kernel_stack(void *addr) { tss.ts_rsp0 = (uint64_t)addr; }

void gdt_set_entry(uint32_t segment, uint32_t base, uint32_t limit,
                   uint8_t ring, int exec, int dir, int rw)
{
    KASSERT(segment < GDT_COUNT * 8 && 0 == segment % 8);
    KASSERT(ring <= 3);
    KASSERT(limit <= 0xFFFFF);

    int index = segment / 8;
    gdt[index].ge_limitlo = (uint16_t)limit;
    gdt[index].ge_baselo = (uint16_t)base;
    gdt[index].ge_basemid = (uint8_t)(base >> 16);
    gdt[index].ge_basehi = (uint8_t)(base >> 24);

    // For x86-64, set the L bit to indicate a 64-bit descriptor and clear Sz
    // Having both L and Sz set is reserved for future use
    gdt[index].ge_flags = (uint8_t)(0b10100000 | (limit >> 16));

    gdt[index].ge_access = 0b10000000;
    gdt[index].ge_access |= (ring << 5);
    gdt[index].ge_access |= 0b10000;
    if (exec)
    {
        gdt[index].ge_access |= 0b1000;
    }
    if (dir)
    {
        gdt[index].ge_access |= 0b100;
    }
    if (rw)
    {
        gdt[index].ge_access |= 0b10;
    }
}

void gdt_clear(uint32_t segment)
{
    KASSERT(segment < GDT_COUNT * 8 && 0 == segment % 8);
    memset(&gdt[segment / 8], 0, sizeof(gdt[segment / 8]));
}

size_t gdt_tss_info(const void *arg, char *buf, size_t osize)
{
    size_t size = osize;

    KASSERT(NULL == arg);

    iprintf(&buf, &size, "TSS:\n");
    iprintf(&buf, &size, "kstack: 0x%p\n", (void *)tss.ts_rsp0);

    return size;
}
