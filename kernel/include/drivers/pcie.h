#pragma once

#include <util/list.h>

#define PCI_NUM_BUSES 256
#define PCI_NUM_DEVICES_PER_BUS 32
#define PCI_NUM_FUNCTIONS_PER_DEVICE 8
#define PCI_DEVICE_FUNCTION_SIZE 4096
#define PCI_CAPABILITY_PTR_MASK (0b11111100)
#define PCI_MSI_CAPABILITY_ID 0x5

// Intel Vol 3A 10.11.1
//#define MSI_BASE_ADDRESS 0x0FEE0000
#define MSI_ADDRESS_FOR(destination) \
    ((uint32_t)((0x0FEE << 20) | ((destination) << 12) | (0b1100)))
#define MSI_DATA_FOR(vector) ((uint16_t)(0b00000001 << 8) | (vector))

typedef struct pci_capability
{
    uint8_t id;
    uint8_t next_cap;
    uint16_t control;
} packed pci_capability_t;

typedef struct msi_capability
{
    uint8_t id;
    uint8_t next_cap;
    struct
    {
        uint8_t msie : 1; // MSI Enable
        uint8_t mmc : 3;  // Multiple Message Capable
        uint8_t mme : 3;  // Multiple Message Enable
        uint8_t c64 : 1;  // 64 Bit Address Capable
        uint8_t _reserved;
    } control;
    union {
        struct
        {
            uint32_t addr;
            uint16_t data;
        } ad32;
        struct
        {
            uint64_t addr;
            uint16_t data;
        } ad64;
    } address_data;
} packed msi_capability_t;

typedef union pcie_device {
    struct
    {
        char data[PCI_DEVICE_FUNCTION_SIZE];
    } raw;
    struct
    {
        uint16_t vendor_id;
        uint16_t device_id;
        uint16_t command;
        uint16_t status;
        uint8_t revision_id;
        uint8_t prog_if;
        uint8_t subclass;
        uint8_t class;
        uint8_t cache_line_size;
        uint8_t latency_type;
        uint8_t header_type;
        uint8_t bist;
        uint32_t bar[6];
        uint32_t cardbus_cis_pointer;
        uint16_t subsystem_vendor_id;
        uint16_t subsystem_id;
        uint32_t expansion_rom_base_addr;
        uint8_t capabilities_ptr;
        uint8_t _reserved1[7];
        uint8_t interrupt_line;
        uint8_t interrupt_pin;
        uint8_t min_grant;
        uint8_t max_latency;
        pci_capability_t pm_capability;
        uint16_t pmcsr;
        uint8_t bse;
        uint8_t data;
        pci_capability_t msi_capability;
        uint64_t message_address;
        uint16_t message_data;
        uint8_t _reserved2[2];
        pci_capability_t pe_capability;
        uint32_t pcie_device_capabilities;
        uint16_t device_control;
        uint16_t device_status;
        uint32_t pcie_link_capabilities;
        uint16_t link_control;
        uint16_t link_status;
    } standard;
} packed pcie_device_t;

#define PCI_LOOKUP_WILDCARD 0xff

typedef struct pcie_device_wrapper
{
    uint8_t class;
    uint8_t subclass;
    uint8_t interface;
    pcie_device_t *dev;
    list_link_t link;
} pcie_device_wrapper_t;

void pci_init(void);

pcie_device_t *pcie_lookup(uint8_t class, uint8_t subclass, uint8_t interface);
