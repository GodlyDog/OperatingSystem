#pragma once

#include <types.h>

/* Documents referenced:
 * ATA Command Set 4:
 * http://www.t13.org/Documents/UploadedDocuments/docs2016/di529r14-ATAATAPI_Command_Set_-_4.pdf
 * AHCI SATA 1.3.1:
 * https://www.intel.com/content/www/us/en/io/serial-ata/serial-ata-ahci-spec-rev1-3-1.html
 * Serial ATA Revision 2.6:
 * http://read.pudn.com/downloads157/doc/project/697017/SerialATA_Revision_2_6_Gold.pdf
 */

/* Macros for working with physical region descriptors. */
#define AHCI_PRDT_DBC_WIDTH 22
#define AHCI_MAX_PRDT_SIZE (1 << AHCI_PRDT_DBC_WIDTH)
#define ATA_SECTOR_SIZE 512
#define AHCI_SECTORS_PER_PRDT (AHCI_MAX_PRDT_SIZE / ATA_SECTOR_SIZE)
#define AHCI_MAX_SECTORS_PER_COMMAND \
    (1 << 16) /* FLAG: Where does this come from? */
#define ACHI_NUM_PRDTS_PER_COMMAND_TABLE \
    (AHCI_MAX_SECTORS_PER_COMMAND / AHCI_SECTORS_PER_PRDT)

#define AHCI_MAX_NUM_PORTS 32
#define AHCI_COMMAND_HEADERS_PER_LIST 32

#define AHCI_COMMAND_LIST_ARRAY_BASE(ahci_base) (ahci_base)
#define AHCI_COMMAND_LIST_ARRAY_SIZE \
    (AHCI_MAX_NUM_PORTS * sizeof(command_list_t))

#define AHCI_RECEIVED_FIS_ARRAY_BASE(ahci_base) \
    ((ahci_base) + AHCI_COMMAND_LIST_ARRAY_SIZE)
#define AHCI_RECEIVED_FIS_ARRAY_SIZE \
    (AHCI_MAX_NUM_PORTS * sizeof(received_fis_t))

#define AHCI_COMMAND_TABLE_ARRAY_BASE(ahci_base) \
    (AHCI_RECEIVED_FIS_ARRAY_BASE(ahci_base) + AHCI_RECEIVED_FIS_ARRAY_SIZE)
#define AHCI_COMMAND_TABLE_ARRAY_SIZE                     \
    (AHCI_MAX_NUM_PORTS * AHCI_COMMAND_HEADERS_PER_LIST * \
     sizeof(command_table_t))

#define AHCI_SIZE                                                  \
    (AHCI_COMMAND_LIST_ARRAY_SIZE + AHCI_RECEIVED_FIS_ARRAY_SIZE + \
     AHCI_COMMAND_TABLE_ARRAY_SIZE)
#define AHCI_SIZE_PAGES ((uintptr_t)PAGE_ALIGN_UP(AHCI_SIZE) / PAGE_SIZE)

#define ALIGN_DOWN_POW_2(x, align) ((x) & -(align))
#define ALIGN_UP_POW_2(x, align) (ALIGN_DOWN_POW_2((x)-1, align) + (align))

/*=============================
 * Frame Information Structures
 *============================*/

/* fis_type_t - FIS types are recognized by an ID.
 * For more info, see section 10.3 (FIS Types) of Serial ATA Revision 2.6. */
typedef enum fis_type
{
    fis_type_h2d_register = 0x27
} packed fis_type_t;

/* Command codes used when forming the host-to-device FIS (see: ATA Command Set
 * 4). The first two are standard commands. The second two are for NCQ commands.
 */
#define ATA_READ_DMA_EXT_COMMAND 0x25
#define ATA_WRITE_DMA_EXT_COMMAND 0x35
#define ATA_READ_FPDMA_QUEUED_COMMAND 0x60
#define ATA_WRITE_FPDMA_QUEUED_COMMAND 0x61

/* 8-bit device setting for host-to-device FIS.
 * Bit 6 is specified as either obsolete or "shall be set to one" for all
 * commands used in Weenix. So, we can safely just default to this value for all
 * commands. More info in sections 7.20, 7.21, 7.55, and 7.57 of ATA Command
 * Set 4. */
#define ATA_DEVICE_LBA_MODE 0x40

/* h2d_register_fis - Register Host to Device FIS.
 * This is the only FIS used in Weenix.
 */
typedef struct h2d_register_fis
{
    uint8_t fis_type; /* Must be set to fis_type_h2d_register. */
    uint8_t : 7;
    uint8_t c : 1;   /* When set, indicates that this is an FIS for a command.
                      * This is always the case in Weenix. */
    uint8_t command; /* See command codes further up. */
    uint8_t
        features;      /* For regular read/write, no use.
                        * For NCQ commands, features and features_exp form the lower
                        * and upper 8 bits of sector count, respectively. */
    uint32_t lba : 24; /* lba and lba_exp form the lower and upper 24 bits of
                          the first logical block address, respectively. */
    uint8_t device;    /* Device register.
                        * For Weenix's purposes, this should always be set to
                        * ATA_DEVICE_LBA_MODE. */
    uint32_t lba_exp : 24;
    uint8_t features_exp;
    uint16_t sector_count; /* For regular read/write, specifies number of
                            * sectors to read/write.
                            * For NCQ commands, bits 7:3 specify NCQ tag. */
    uint16_t : 16;
    uint32_t : 32;
} packed h2d_register_fis_t;

/*========================
 * Command List Structures
 *=======================*/

/* command_fis_t - Represents a software-constructed FIS stored in a
 * command_table_t. */
typedef union command_fis {
    h2d_register_fis_t h2d_register_fis;
    /* Must occupy 64 bytes in its corresponding command_table_t.
     * Recall that unions conform to the size of the largest member. */
    struct
    {
        uint8_t size[64];
    };
} packed command_fis_t;

/* received_fis_t - Per-port structure that contains information on received
 * FISes. More info in section 4.2.1 of the 1.3.1 spec. */
typedef struct received_fis
{
    uint8_t _omit[256]; /* Weenix does not make use of any received FIS from the
                           device. */
} packed received_fis_t;

/* prd_t - Physical Region Descriptor.
 * Represents an entry in the PRD table in a command table
 * (command_table_t->prdt). Points to a chunk of system memory for the device to
 * use according to whatever command it is executing.
 */
typedef struct prd
{
    uint64_t dba; /* Data Base Address. */
    uint32_t : 32;
    uint32_t
        dbc : 22; /* Data Byte Count: Indicates length of data block in bytes,
                   * but starts counting from 0. Ex: Length 1 is 0x0. Length 2
                   * is 0x1. Length 3 is 0x10. And so on... Must be even. Due to
                   * counting from 0, this means least-significant bit MUST
                   * be 1. Max length is 4MB (all bits set). */
    uint16_t : 9;
    uint8_t i : 1; /* Interrupt on Completion: When set, then upon processing
                    * all PRDs in the current FIS, the port will try to generate
                    * an interrupt by setting PxIS.DPS.
                    *
                    * Whether or not this actually behaves as expected, or ever
                    * is even used, is unclear.
                    */
} packed prd_t;

/* command_table_t - Structure detailing a command and associated data / memory.
 * More info in section 4.2.3 of SATA AHCI 1.3.1.
 */
typedef struct command_table
{
    command_fis_t
        cfis; /* Command FIS: The actual software constructed command. */
    uint8_t _omit[64];
    prd_t prdt[ACHI_NUM_PRDTS_PER_COMMAND_TABLE]; /* Physical Region Descriptor
                                                   * Table: A list of,
                                                   * theoretically, up to 2^16
                                                   * entries of PRDs.
                                                   * Number of actual usable
                                                   * entries is indicated by
                                                   * command_header_t->prdtl. */
} packed command_table_t;

/* command_header_t - Structure detailing command details. Stored in a
 * command_list_t. More info in section 4.2.2 of the SATA AHCI 1.3.1 spec. */
typedef struct command_header
{
    uint8_t cfl : 5; /* Command FIS length in DW (4 bytes). Max value is 0x10
                        (16). */
    uint8_t : 1;
    uint8_t write : 1; /* Write: Set indicates write, clear indicates read. */
    uint16_t : 9;
    uint16_t prdtl; /* Physical Region Descriptor Table Length: Number of PRD
                       entries. */
    uint32_t : 32;
    uint64_t ctba; /* Command Table Descriptor Base Address: Pointer to the
                      command table. */
    uint64_t : 64;
    uint64_t : 64;
} packed command_header_t;

/* command_list_t - Per-port command list.
 * More info in section 4.2.2 of the SATA AHCI 1.3.1 spec.
 * See also: Figure 5: Port System Memory Structures. */
typedef struct command_list
{
    command_header_t command_headers[AHCI_COMMAND_HEADERS_PER_LIST];
} packed command_list_t;

/*=================
 * Host Bus Adapter
 *================*/

/* px_interrupt_status - Per-port bitmap indicating that a corresponding
 * interrupt has occurred on the port. Observe that this is a union, making
 * initialization a little easier. */
typedef union px_interrupt_status {
    struct
    {
        uint8_t dhrs : 1; /* Interrupt requested by a device-to-host FIS.
                           * Used by normal read/write commands, see 5.6.2
                           * in 1.3.1. */
        uint8_t : 2;
        uint8_t
            sdbs : 1; /* Interrupt requested by a set device bits FIS.
                       * Used by NCQ read/write commands, see 5.6.4 in 1.3.1. */
        uint8_t : 1;
        uint8_t dps : 1; /* Interrupt set upon completing an FIS that requested
                          * an interrupt upon completion.
                          * Currently doesn't seem to be working... */
        uint32_t : 26;
    } bits;
    uint32_t value;
} packed px_interrupt_status_t;

/* Observe that, to clear interrupt status, must set to 1. */
static px_interrupt_status_t px_interrupt_status_clear = {.value =
                                                              (uint32_t)-1};

/* Port x Interrupt Enable - Bitwise register controlling generation of various
 * interrupts. */
typedef union px_interrupt_enable {
    uint32_t value;
} packed px_interrupt_enable_t;

/* Weenix uses this to initialize all ports to enable all interrupts by default.
 */
static px_interrupt_enable_t px_interrupt_enable_all_enabled = {
    .value = (uint32_t)-1};

/* hba_ghc_t - Generic Host Control: Information and control registers
 * pertaining to the entire HBA. More info in section 3.1 of 1.3.1.
 */
typedef struct hba_ghc
{
    struct
    {
        uint32_t : 30;
        uint8_t sncq : 1; /* Supports Native Command Queueing. */
        uint8_t : 1;
    } packed cap;
    struct
    {
        uint8_t : 1;
        uint8_t ie : 1; /* Interrupt Enable: Enables/disables interrupts from
                           HBA. */
        uint32_t : 29;
        uint8_t ae : 1; /* AHCI Enable: Indicates software adheres to AHCI
                           specification. */
    } packed ghc;
    uint32_t is; /* Interrupt Status: If bit x is set, then port x has a pending
                    interrupt. */
    uint32_t pi; /* Ports Implemented: If bit x is set, then port x is available
                    for use. */
    uint32_t _omit[7];
} packed hba_ghc_t;

/* Signature for SATA devices. Compare this against hba_port_t->px_sig to
 * determine if a SATA device is sitting behind a given port. */
#define SATA_SIG_ATA 0x00000101

/* hba_port - A per-port structure storing port information.
 *  Each port represents a device that the HBA is communicating with (e.g. a
 * SATA device!). Details not relevant to Weenix have been omitted. More info in
 * section 3.3 of the SATA AHCI 1.3.1 spec.
 */
typedef struct hba_port
{
    uint64_t px_clb;             /* 1K-byte aligned base physical address of this port's
                      * command list. This is a pointer to a command_list_t. */
    uint64_t px_fb;              /* Base physical address for received FISes.
                      * Weenix never uses received FIS, but we allocate and set
                      * up memory to make the HBA happy. */
    px_interrupt_status_t px_is; /* Interrupt Status. */
    px_interrupt_enable_t px_ie; /* Interrupt Enable. */
    struct
    {
        uint8_t st : 1; /* Start: Allows the HBA to process the command list. */
        uint8_t : 3;
        uint8_t fre : 1; /* FIS Receive Enable: Allows HBA to post received
                            FISes in px_fb. */
        uint16_t : 9;
        uint8_t fr : 1; /* FIS Receive Running: Read-only indicating if FIS
                           Receive DMA is running. */
        uint8_t cr : 1; /* Command List Running: Read-only indicating if command
                           list DMA is running. */
        uint16_t : 16;
    } packed px_cmd; /* Port Command and Status. */
    uint64_t : 64;
    uint32_t px_sig; /* Signature: Contains attached device's signature.
                      * SATA devices should have signature SATA_SIG_ATA, defined
                      * above. */
    uint64_t : 64;
    uint32_t px_serr; /* SATA Error: Unclear how Weenix is actually making use
                         of this register. */
    uint32_t px_sact; /* SATA Active: Used for NCQ.
                       * Each bit corresponds to TAG and command slot of an NCQ
                       * command. Must be set by software before issuing a NCQ
                       * for a command slot.
                       */
    uint32_t px_ci;   /* Commands Issued: Software sets bit x if a command x is
                       * ready to be sent.   Each bit corresponds to a command slot.
                       * HBA clears bit upon completing a command.
                       */
    uint32_t _omit[17];
} packed hba_port_t;

/* Host Bus Adapter - Control block for the device that actually interfaces
 * between the OS and the SATA disk device. For more info, see section 3 of
 * the 1.3.1 spec.
 */
typedef struct hba
{
    hba_ghc_t ghc; /* Generic Host Control. */
    uint32_t _omit[53];
    hba_port_t ports[32]; /* Static array of port descriptors. */
} packed hba_t;

#define PORT_INDEX(hba, port) ((port) - (hba)->ports)
