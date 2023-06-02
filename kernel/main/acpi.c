#include "main/acpi.h"

#include "boot/config.h"
#include "mm/page.h"
#include "types.h"
#include "util/debug.h"
#include "util/string.h"

#define XSDT_SIGNATURE (*(uint32_t *)"XSDT")
#define RSDT_SIGNATURE (*(uint32_t *)"RSDT")
#define FACP_SIGNATURE (*(uint32_t *)"FACP")
#define DSDT_SIGNATURE (*(uint32_t *)"DSDT")

#define RSDP_ALIGN 16

#define EBDA_MIN_PADDR 0x80000
#define EBDA_MAX_PADDR 0xa0000
#define EBDA_PTR_LOC_PADDR 0x040e

#define EBDA_MIN (PHYS_OFFSET + EBDA_MIN_PADDR)
#define EBDA_MAX (PHYS_OFFSET + EBDA_MAX_PADDR)
#define EBDA_PTR_LOC (PHYS_OFFSET + EBDA_PTR_LOC_PADDR)

static const uint8_t rsdp_sig[8] = {'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};

typedef struct rsdp
{
    uint8_t rp_sign[8];
    uint8_t rp_checksum;
    uint8_t rp_oemid[6];
    uint8_t rp_rev;
    uint32_t rp_addr;
} packed rsdp_t;

typedef struct rsdp_20
{
    rsdp_t rsdp;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} packed rsdp_20_t;

typedef struct rsd_table
{
    acpi_header_t rt_header;
    uint64_t rt_other[];
} packed rsd_table_t;

static uint8_t __acpi_checksum(const uint8_t *buf, long size)
{
    uint8_t sum = 0;
    for (long i = 0; i < size; i++)
        sum += buf[i];
    return sum;
}

static rsdp_20_t *__rsdp_search_range(uintptr_t start, uintptr_t end)
{
    uintptr_t rsdp_candidate = start;
    while (rsdp_candidate <= end - sizeof(struct rsdp))
    {
        if (memcmp((void *)rsdp_candidate, rsdp_sig, sizeof(rsdp_sig)) == 0 &&
            __acpi_checksum((uint8_t *)rsdp_candidate, sizeof(rsdp_20_t)) ==
                0)
        {
            return (rsdp_20_t *)rsdp_candidate;
        }
        rsdp_candidate += RSDP_ALIGN;
    }
    return NULL;
}

static void *__rsdp_search()
{
    // detect the location of the EBDA from the BIOS data section
    uintptr_t ebda =
        ((uintptr_t) * (uint16_t *)EBDA_PTR_LOC << 4) + PHYS_OFFSET;
    rsdp_20_t *rsdp = 0;
    if (ebda >= EBDA_MIN && ebda <= EBDA_MAX && ebda % RSDP_ALIGN == 0)
    {
        // check only if it's valid
        rsdp = __rsdp_search_range(ebda, EBDA_MAX);
    }
    if (!rsdp)
    {
        // darmanio: unsure where these magic constants came from...
        rsdp =
            __rsdp_search_range(PHYS_OFFSET + 0xe0000, PHYS_OFFSET + 0x100000);
    }
    return rsdp;
}

static rsdp_20_t *rsd_ptr = NULL;
static rsd_table_t *rsd_table = NULL;

static rsd_table_t *_acpi_load_table(uintptr_t paddr)
{
    page_mark_reserved(PAGE_ALIGN_DOWN(paddr));
    return (rsd_table_t *)(PHYS_OFFSET + paddr);
}

void acpi_init()
{
    if (rsd_ptr == NULL)
    {
        rsd_ptr = __rsdp_search();
        KASSERT(rsd_ptr && "Could not find the ACPI Root Descriptor Table.");

        rsd_table = _acpi_load_table(rsd_ptr->xsdt_addr);
        KASSERT(XSDT_SIGNATURE == rsd_table->rt_header.ah_sign);
        if (__acpi_checksum((void *)rsd_table, rsd_table->rt_header.ah_size))
        {
            panic("Weenix only supports ACPI 2.0 or higher");
        }

        dbgq(DBG_CORE, "--- ACPI INIT ---\n");
        dbgq(DBG_CORE, "rsdp addr:  %p\n", rsd_ptr);
        dbgq(DBG_CORE, "rsdt addr:  %p\n", rsd_table);
        dbgq(DBG_CORE, "rev:        %i\n", (int)rsd_ptr->rsdp.rp_rev);

        rsd_ptr->rsdp.rp_oemid[5] = 0;
        dbgq(DBG_CORE, "oem:        %s\n", (char *)rsd_ptr->rsdp.rp_oemid);

        // search for all tables listed in the RSDT and checksum them
        dbgq(DBG_CORE, "ents:\t");
        size_t headers =
            (rsd_table->rt_header.ah_size - sizeof(rsd_table->rt_header)) /
            sizeof(rsd_table->rt_other[0]);

        for (size_t i = 0; i < headers; ++i)
        {
            acpi_header_t *header =
                &_acpi_load_table(rsd_table->rt_other[i])->rt_header;
            rsd_table->rt_other[i] = (uintptr_t)header;

            dbgq(DBG_CORE, "%.4s ", (char *)&header->ah_sign);
            KASSERT(0 == __acpi_checksum((void *)header, header->ah_size));
        }
        dbgq(DBG_CORE, "\n");
    }
}

void *acpi_table(uint32_t signature, int index)
{
    KASSERT(index >= 0);

    size_t headers =
        (rsd_table->rt_header.ah_size - sizeof(rsd_table->rt_header)) /
        sizeof(rsd_table->rt_other[0]);

    for (size_t i = 0; i < headers; ++i)
    {
        acpi_header_t *header = (acpi_header_t *)rsd_table->rt_other[i];
        if (header->ah_sign == signature && 0 == index--)
        {
            return header;
        }
    }
    return NULL;
}
