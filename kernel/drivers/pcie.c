#include "drivers/pcie.h"
#include <drivers/pcie.h>
#include <main/acpi.h>
#include <mm/kmalloc.h>
#include <mm/pagetable.h>
#include <util/debug.h>

#define MCFG_SIGNATURE (*(uint32_t *)"MCFG")
static uintptr_t pcie_base_addr;

typedef struct pcie_table
{
    pcie_device_t devices[PCI_NUM_BUSES][PCI_NUM_DEVICES_PER_BUS]
                         [PCI_NUM_FUNCTIONS_PER_DEVICE];
} pcie_table_t;

static pcie_table_t *pcie_table;

#define PCIE_DEV(bus, device, func) \
    (&pcie_table->devices[(bus)][(device)][(func)])
static list_t pcie_wrapper_list;

void pci_init(void)
{
    // TODO document; needs -machine type=q35 flag in qemu!
    void *table = acpi_table(MCFG_SIGNATURE, 0);
    KASSERT(table);
    pcie_base_addr = *(uintptr_t *)((uintptr_t)table + 44) + PHYS_OFFSET;
    pcie_table = (pcie_table_t *)pcie_base_addr;
    pt_map_range(pt_get(), pcie_base_addr - PHYS_OFFSET, pcie_base_addr,
                 pcie_base_addr + PAGE_SIZE_1GB, PT_WRITE | PT_PRESENT,
                 PT_WRITE | PT_PRESENT);

    list_init(&pcie_wrapper_list);
    for (unsigned bus = 0; bus < PCI_NUM_BUSES; bus++)
    {
        for (unsigned device = 0; device < PCI_NUM_DEVICES_PER_BUS; device++)
        {
            unsigned int max_functions =
                (PCIE_DEV(bus, device, 0)->standard.header_type & 0x80)
                    ? PCI_NUM_DEVICES_PER_BUS
                    : 1;
            for (unsigned function = 0; function < max_functions; function++)
            {
                pcie_device_t *dev = PCIE_DEV(bus, device, function);
                if (!dev->standard.vendor_id ||
                    dev->standard.vendor_id == (uint16_t)-1)
                    continue;
                pcie_device_wrapper_t *wrapper =
                    kmalloc(sizeof(pcie_device_wrapper_t));
                wrapper->dev = dev;
                wrapper->class = dev->standard.class;
                wrapper->subclass = dev->standard.subclass;
                wrapper->interface = dev->standard.prog_if;
                list_link_init(&wrapper->link);
                list_insert_tail(&pcie_wrapper_list, &wrapper->link);
            }
        }
    }
}

pcie_device_t *pcie_lookup(uint8_t class, uint8_t subclass, uint8_t interface)
{
    list_iterate(&pcie_wrapper_list, wrapper, pcie_device_wrapper_t, link)
    {
        /* verify the class subclass and interface are correct */
        if (((class == PCI_LOOKUP_WILDCARD) || (wrapper->class == class)) &&
            ((subclass == PCI_LOOKUP_WILDCARD) ||
             (wrapper->subclass == subclass)) &&
            ((interface == PCI_LOOKUP_WILDCARD) ||
             (wrapper->interface == interface)))
        {
            return wrapper->dev;
        }
    }
    return NULL;
}
