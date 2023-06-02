#include <drivers/blockdev.h>
#include <drivers/disk/ahci.h>
#include <drivers/disk/sata.h>
#include <drivers/pcie.h>
#include <errno.h>
#include <mm/kmalloc.h>
#include <mm/page.h>
#include <util/debug.h>
#include <util/string.h>

#define ENABLE_NATIVE_COMMAND_QUEUING 1

#define bdev_to_ata_disk(bd) (CONTAINER_OF((bd), ata_disk_t, bdev))
#define SATA_SECTORS_PER_BLOCK (SATA_BLOCK_SIZE / ATA_SECTOR_SIZE)

#define SATA_PCI_CLASS 0x1      /* 0x1 = mass storage device */
#define SATA_PCI_SUBCLASS 0x6   /* 0x6 = sata */
#define SATA_AHCI_INTERFACE 0x1 /* 0x1 = ahci */

static hba_t *hba; /* host bus adapter */

/* If NCQ, this is an outstanding tag bitmap.
 * If standard, this is an outstanding command slot bitmap. */
static uint32_t outstanding_requests[AHCI_MAX_NUM_PORTS] = {0};

/* Each command slot on each port has a waitqueue for a thread waiting on a
 * command to finish execution. */
static ktqueue_t outstanding_request_queues[AHCI_MAX_NUM_PORTS]
                                           [AHCI_COMMAND_HEADERS_PER_LIST];

/* Each port has a waitqueue for a thread waiting on a new command slot to open
 * up. */
static ktqueue_t command_slot_queues[AHCI_MAX_NUM_PORTS];

/* SMP: Protect access to ports. */
static spinlock_t port_locks[AHCI_MAX_NUM_PORTS];

long sata_read_block(blockdev_t *bdev, char *buf, blocknum_t block,
                     size_t block_count);
long sata_write_block(blockdev_t *bdev, const char *buf, blocknum_t block,
                      size_t block_count);

/* sata_disk_ops - Block device operations for SATA devices. */
static blockdev_ops_t sata_disk_ops = {
    .read_block = sata_read_block,
    .write_block = sata_write_block,
};

/* find_cmdslot - Checks various bitmaps to find the lowest index command slot
 * that is free for a given port. */
inline long find_cmdslot(hba_port_t *port)
{
    /* From 1.3.1: Free command slot will have corresponding bit clear in both
     * px_sact and px_ci. To be safe, also check against our local copy of
     * outstanding requests, in case a recently completed command is clear in
     * the port's actual descriptor, but has not been processed by Weenix yet.
     */
    return __builtin_ctz(~(port->px_sact | port->px_ci |
                           outstanding_requests[PORT_INDEX(hba, port)]));
}

/* ensure_mapped - Wrapper for pt_map_range(). */
void ensure_mapped(void *addr, size_t size)
{
    pt_map_range(pt_get(), (uintptr_t)PAGE_ALIGN_DOWN(addr) - PHYS_OFFSET,
                 (uintptr_t)PAGE_ALIGN_DOWN(addr),
                 (uintptr_t)PAGE_ALIGN_UP((uintptr_t)addr + size),
                 PT_WRITE | PT_PRESENT, PT_WRITE | PT_PRESENT);
}

kmutex_t because_qemu_doesnt_emulate_ahci_ncq_correctly;

/* ahci_do_operation - Sends a command to the HBA to initiate a disk operation.
 */
long ahci_do_operation(hba_port_t *port, ssize_t lba, uint16_t count, void *buf,
                       int write)
{
    kmutex_lock(&because_qemu_doesnt_emulate_ahci_ncq_correctly);
    KASSERT(count && buf);
    KASSERT(lba >= 0 && lba < 1L << 23);

    /* Obtain the port and the physical system memory in question. */
    size_t port_index = PORT_INDEX(hba, port);

    uint8_t ipl = intr_setipl(IPL_HIGH);
    spinlock_lock(port_locks + port_index);

    uint64_t physbuf = pt_virt_to_phys((uintptr_t)buf);

    /* Get an available command slot. */
    long command_slot;
    while ((command_slot = find_cmdslot(port)) == -1)
    {
        sched_sleep_on(command_slot_queues + port_index,
                       port_locks + port_index);
        /* Spinlock is important: find_cmdslot() does not actually reserve the
         * command slot. */
        spinlock_lock(port_locks + port_index);
    }

    /* Get corresponding command_header in the port's command_list. */
    command_list_t *command_list =
        (command_list_t *)(port->px_clb + PHYS_OFFSET);
    command_header_t *command_header =
        command_list->command_headers + command_slot;
    memset(command_header, 0, sizeof(command_header_t));

    /* Command setup: Header. */
    command_header->cfl = sizeof(h2d_register_fis_t) / sizeof(uint32_t);
    command_header->write = (uint8_t)write;
    command_header->prdtl = (uint16_t)(
        ALIGN_UP_POW_2(count, AHCI_SECTORS_PER_PRDT) / AHCI_SECTORS_PER_PRDT);
    KASSERT(command_header->prdtl);

    /* Command setup: Table. */
    command_table_t *command_table =
        (command_table_t *)(command_header->ctba + PHYS_OFFSET);
    memset(command_table, 0, sizeof(command_table_t));

    /* Command setup: Physical region descriptor table. */
    prd_t *prdt = command_table->prdt;
    /* Note that this loop is only called when the size of the data transfer is
     * REALLY big. */
    for (unsigned i = 0; i < command_header->prdtl - 1U; i++)
    {
        prdt->dbc = AHCI_MAX_PRDT_SIZE - 1;
        prdt->dba = physbuf; /* Data from physical buffer. */
        prdt->i = 1;         /* Set interrupt on completion. */
        physbuf +=
            AHCI_MAX_PRDT_SIZE; /* Advance physical buffer for next prd. */
        prdt++;
    }
    prdt->dbc = (uint32_t)(count % AHCI_SECTORS_PER_PRDT) * ATA_SECTOR_SIZE - 1;
    prdt->dba = (uint64_t)physbuf;

    /* Set up the particular h2d_register_fis command (the only one we use). */
    h2d_register_fis_t *command_fis = &command_table->cfis.h2d_register_fis;
    command_fis->fis_type = fis_type_h2d_register;
    command_fis->c = 1;
    command_fis->device = ATA_DEVICE_LBA_MODE;
    command_fis->lba = (uint32_t)lba;
    command_fis->lba_exp = (uint32_t)(lba >> 24);

    /* NCQ: Allows the hardware to queue commands in its *own* order,
     * independent of software delivery. */
#if ENABLE_NATIVE_COMMAND_QUEUING
    if (hba->ghc.cap.sncq)
    {
        /* For NCQ, sector count is stored in features. */
        command_fis->features = (uint8_t)count;
        command_fis->features_exp = (uint8_t)(count >> 8);

        /* For NCQ, bits 7:3 of sector_count field specify NCQ tag. */
        command_fis->sector_count = (uint16_t)(command_slot << 3);

        /* Choose the appropriate NCQ read/write command. */
        command_fis->command = (uint8_t)(write ? ATA_WRITE_FPDMA_QUEUED_COMMAND
                                               : ATA_READ_FPDMA_QUEUED_COMMAND);
    }
    else
    {
        command_fis->sector_count = count;

        command_fis->command = (uint8_t)(write ? ATA_WRITE_DMA_EXT_COMMAND
                                               : ATA_READ_DMA_EXT_COMMAND);
    }
#else
    /* For regular commands, simply set the command type and the sector count.
     */
    command_fis->sector_count = count;
    command_fis->command =
        (uint8_t)(write ? ATA_WRITE_DMA_EXT_COMMAND : ATA_READ_DMA_EXT_COMMAND);
#endif

    dbg(DBG_DISK, "initiating request on slot %ld to %s sectors [%lu, %lu)\n",
        command_slot, write ? "write" : "read", lba, lba + count);

    /* Locally mark that we sent out a command on the given command slot of the
     * given port. */
    outstanding_requests[port_index] |= (1 << command_slot);

    /* Explicitly notify the port that a command is available for execution. */
    port->px_sact |= (1 << command_slot);
    port->px_ci |= (1 << command_slot);

    /* Sleep until the command has been serviced. */
    spinlock_lock(&curthr->kt_lock);
    KASSERT(!curthr->kt_retval);

    dbg(DBG_DISK,
        "initiating request on slot %ld to %s sectors [%lu, %lu)...sleeping\n",
        command_slot, write ? "write" : "read", lba, lba + count);
    sched_sleep_on(outstanding_request_queues[port_index] + command_slot,
                   port_locks + port_index);
    intr_setipl(ipl);
    dbg(DBG_DISK, "completed request on slot %ld to %s sectors [%lu, %lu)\n",
        command_slot, write ? "write" : "read", lba, lba + count);
    kmutex_unlock(&because_qemu_doesnt_emulate_ahci_ncq_correctly);

    long ret = (long)curthr->kt_retval;
    spinlock_unlock(&curthr->kt_lock);

    return ret;
}

/* start_cmd - Start a port's DMA engines. See 10.3 of 1.3.1. */
static inline void start_cmd(hba_port_t *port)
{
    while (port->px_cmd.cr)
        ;                 /* Wait for command list DMA to stop running. */
    port->px_cmd.fre = 1; /* Enable posting received FIS. */
    port->px_cmd.st = 1;  /* Enable processing the command list. */
}

/* stop_cmd - Stop a port's DMA engines. See 10.3 of 1.3.1. */
static inline void stop_cmd(hba_port_t *port)
{
    port->px_cmd.st = 0; /* Stop processing the command list. */
    while (port->px_cmd.cr)
        ;                 /* Wait for command list DMA to stop running. */
    port->px_cmd.fre = 0; /* Stop posting received FIS. */
    while (port->px_cmd.fr)
        ; /* Wait for FIS receive DMA to stop running. */
}

/* ahci_initialize_port */
static void ahci_initialize_port(hba_port_t *port, unsigned int port_number,
                                 uintptr_t ahci_base)
{
    dbg(DBG_DISK, "Initializing AHCI Port %d\n", port_number);

    /* Pretty sure this is unnecessary. */
    // port->px_serr = port->px_serr;

    /* Make sure the port is not doing any DMA. */
    stop_cmd(port);

    /* Pretty sure this is unnecessary. */
    // port->px_serr = (unsigned) -1;

    /* Determine and set the command list and received FIS base addresses in the
     * port's descriptor. */
    command_list_t *command_list =
        (command_list_t *)AHCI_COMMAND_LIST_ARRAY_BASE(ahci_base) + port_number;
    received_fis_t *received_fis =
        (received_fis_t *)AHCI_RECEIVED_FIS_ARRAY_BASE(ahci_base) + port_number;

    port->px_clb = (uint64_t)command_list - PHYS_OFFSET;
    port->px_fb = (uint64_t)received_fis - PHYS_OFFSET;
    port->px_ie =
        px_interrupt_enable_all_enabled; /* FLAG: Weenix does not need to enable
                                          * all interrupts. Aside from dhrs and
                                          * sdbs, I think we could either
                                          * disable others,
                                          *  or tell the handler to panic if
                                          * other interrupts are encountered. */
    port->px_is =
        px_interrupt_status_clear; /* RWC: Read / Write '1' to Clear. */

    /* Determine and set the command tables.
     * For each header, set its corresponding table and set up its queue. */
    command_table_t *port_command_table_array_base =
        (command_table_t *)AHCI_COMMAND_TABLE_ARRAY_BASE(ahci_base) +
        port_number * AHCI_COMMAND_HEADERS_PER_LIST;
    for (unsigned i = 0; i < AHCI_COMMAND_HEADERS_PER_LIST; i++)
    {
        command_list->command_headers[i].ctba =
            (uint64_t)(port_command_table_array_base + i) - PHYS_OFFSET;
        sched_queue_init(outstanding_request_queues[port_number] + i);
    }

    /* Start the queue to wait for an open command slot. */
    sched_queue_init(command_slot_queues + port_number);

    spinlock_init(port_locks + port_number);

    /* For SATA disks, allocate, setup, and register the disk / block device. */
    if (port->px_sig == SATA_SIG_ATA)
    {
        dbg(DBG_DISK, "\tAdding SATA Disk Drive at Port %d\n", port_number);
        ata_disk_t *disk = kmalloc(sizeof(ata_disk_t));
        disk->port = port;
        disk->bdev.bd_id = MKDEVID(DISK_MAJOR, port_number);
        disk->bdev.bd_ops = &sata_disk_ops;
        list_link_init(&disk->bdev.bd_link);
        long ret = blockdev_register(&disk->bdev);
        KASSERT(!ret);
    }
    else
    {
        /* FLAG: Should we just check sig first and save some work on unknown
         * devices? */
        dbg(DBG_DISK, "\tunknown device signature: 0x%x\n", port->px_sig);
    }

    /* Start the port's DMA engines and allow it to start servicing commands. */
    start_cmd(port);

    /* RWC: Write back to clear errors one more time. FLAG: WHY?! */
    // port->px_serr = port->px_serr;
}

/* ahci_initialize_hba - Called at initialization to set up hba-related fields.
 */
void ahci_initialize_hba()
{
    kmutex_init(&because_qemu_doesnt_emulate_ahci_ncq_correctly);

    /* Get the HBA controller for the SATA device. */
    pcie_device_t *dev =
        pcie_lookup(SATA_PCI_CLASS, SATA_PCI_SUBCLASS, SATA_AHCI_INTERFACE);

    /* Set bit 2 to enable memory and I/O requests.
     * This actually doesn't seem to be necessary...
     * See: 2.1.2, AHCI SATA 1.3.1. */
    // dev->standard.command |= 0x4;

    /* Traverse the pcie_device_t's capabilities to look for an MSI capability.
     */
    KASSERT(dev->standard.capabilities_ptr & PCI_CAPABILITY_PTR_MASK);
    pci_capability_t *cap =
        (pci_capability_t *)((uintptr_t)dev + (dev->standard.capabilities_ptr &
                                               PCI_CAPABILITY_PTR_MASK));
    while (cap->id != PCI_MSI_CAPABILITY_ID)
    {
        KASSERT(cap->next_cap && "couldn't find msi control for ahci device");
        cap = (pci_capability_t *)((uintptr_t)dev +
                                   (cap->next_cap & PCI_CAPABILITY_PTR_MASK));
    }
    msi_capability_t *msi_cap = (msi_capability_t *)cap;

    /* Set MSI Enable to turn on MSI. */
    msi_cap->control.msie = 1;

    /* For more info on MSI, consult Intel 3A 10.11.1, and also 2.3 of the 1.3.1
     * spec. */

    /* Set up MSI for processor 1, with interrupt vector INTR_DISK_PRIMARY.
     * TODO: Check MSI setup details to determine if MSI can be handled more
     * efficiently in SMP.
     */
    if (msi_cap->control.c64)
    {
        msi_cap->address_data.ad64.addr = MSI_ADDRESS_FOR(1);
        msi_cap->address_data.ad64.data = MSI_DATA_FOR(INTR_DISK_PRIMARY);
    }
    else
    {
        msi_cap->address_data.ad32.addr = MSI_ADDRESS_FOR(1);
        msi_cap->address_data.ad32.data = MSI_DATA_FOR(INTR_DISK_PRIMARY);
    }

    KASSERT(dev && "Could not find AHCI Controller");
    dbg(DBG_DISK, "Found AHCI Controller\n");

    /* bar = base address register. The last bar points to base memory for the
     * host bus adapter. */
    hba = (hba_t *)(PHYS_OFFSET + dev->standard.bar[5]);

    /* Create a page table mapping for the hba. */
    ensure_mapped(hba, sizeof(hba_t));

    /* This seems to do nothing, because interrupt_line is never set, and MSIE
     * is set. */
    // intr_map(dev->standard.interrupt_line, INTR_DISK_PRIMARY);

    /* Allocate space for what will become the command lists and received FISs
     * for each port. */
    uintptr_t ahci_base = (uintptr_t)page_alloc_n(AHCI_SIZE_PAGES);
    memset((void *)ahci_base, 0, AHCI_SIZE_PAGES * PAGE_SIZE);

    KASSERT(ahci_base);
    /* Set AHCI Enable bit.
     * Actually this bit appears to be read-only (see 3.1.2 AE and 3.1.1 SAM).
     * I do get a "mis-aligned write" complaint when I try to manually set it.
     */
    KASSERT(hba->ghc.ghc.ae);

    /* Temporarily clear Interrupt Enable bit before setting up ports. */
    hba->ghc.ghc.ie = 0;

    dbg(DBG_DISK, "ahci ncq supported: %s\n",
        hba->ghc.cap.sncq ? "true" : "false");

    /* Initialize each of the available ports. */
    uint32_t ports_implemented = hba->ghc.pi;
    KASSERT(ports_implemented);
    while (ports_implemented)
    {
        unsigned port_number = __builtin_ctz(ports_implemented);
        ports_implemented &= ~(1 << port_number);
        ahci_initialize_port(hba->ports + port_number, port_number, ahci_base);
    }

    /* Clear any outstanding interrupts from any ports. */
    hba->ghc.is = (uint32_t)-1;

    /* Restore Interrupt Enable bit. */
    hba->ghc.ghc.ie = 1;
}

/* ahci_interrupt_handler - Service an interrupt that was raised by the HBA.
 */
static long ahci_interrupt_handler(regs_t *regs)
{
    /* Check interrupt status bitmap for ports to service. */
    while (hba->ghc.is)
    {
        /* Get a port from the global interrupt status bitmap. */
        unsigned port_index = __builtin_ctz(hba->ghc.is);

        /* Get the port descriptor from the HBA's ports array. */
        hba_port_t *port = hba->ports + port_index;
        spinlock_lock(port_locks + port_index);

        /* Beware: If a register is marked "RWC" in the spec, you must clear it
         * by writing 1. This is rather understated in the specification. */

        /* Clear the cause of the interrupt.
         * See 5.6.2 and 5.6.4 in the 1.3.1 spec for confirmation of the FIS and
         * corresponding interrupt that are used depending on the type of
         * command.
         */

#if ENABLE_NATIVE_COMMAND_QUEUING
        if (hba->ghc.cap.sncq)
        {
            KASSERT(port->px_is.bits.sdbs);
            port->px_is.bits.sdbs = 1;
        }
        else
        {
            KASSERT(port->px_is.bits.dhrs);
            port->px_is.bits.dhrs = 1;
        }
#else
        KASSERT(port->px_is.bits.dhrs);
        port->px_is.bits.dhrs = 1;
#endif

        /* Clear the port's bit on the global interrupt status bitmap, to
         * indicate we have handled it. */
        /* Note: Changed from ~ to regular, because this register is RWC. */
        hba->ghc.is &= (1 << port_index);

        /* Get the list of commands still outstanding. */
#if ENABLE_NATIVE_COMMAND_QUEUING
        /* If NCQ, use SACT register. */
        uint32_t active = hba->ghc.cap.sncq ? port->px_sact : port->px_ci;
#else
        /* If not NCQ, use CI register. */
        uint32_t active = port->px_ci;
#endif

        /* Compare the active commands against those we actually sent out to get
         * completed commands. */
        uint32_t completed = outstanding_requests[port_index] &
                             ~(outstanding_requests[port_index] & active);
        /* Handle each completed command: */
        while (completed)
        {
            uint32_t slot = __builtin_ctz(completed);

            /* Wake up the thread that was waiting on that command. */
            kthread_t *thr;
            sched_wakeup_on(&outstanding_request_queues[port_index][slot],
                            &thr);

            /* Mark the command as available. */
            completed &= ~(1 << slot);
            outstanding_requests[port_index] &= ~(1 << slot);

            /* TODO: Wake up threads that were waiting for a command slot to
             * free up on the port. */
        }

        spinlock_unlock(port_locks + port_index);
    }
    return 0;
}

void sata_init()
{
    intr_register(INTR_DISK_PRIMARY, ahci_interrupt_handler);
    ahci_initialize_hba();
}

/**
 * Read the given number of blocks from a block device starting at
 * a given block number into a buffer.
 * 
 * To do this, you will need to call ahci_do_operation(). SATA devices 
 * conduct operations in terms of sectors, rather than blocks, thus 
 * you will need to convert the arguments passed in to be in terms of 
 * sectors. 
 *
 * @param  bdev        block device to read from
 * @param  buf         buffer to write to
 * @param  block       block number to start reading at
 * @param  block_count the number of blocks to read
 * @return             0 on success and <0 on error
 */
long sata_read_block(blockdev_t *bdev, char *buf, blocknum_t block,
                     size_t block_count)
{
    long status = ahci_do_operation(bdev_to_ata_disk(bdev)->port, block * SATA_SECTORS_PER_BLOCK, block_count * SATA_SECTORS_PER_BLOCK, (void*) buf, 0);
    return status;
}

/**
 * Writes a a given number of blocks from a buffer to a block device
 * starting at a given block. This function should be very similar to what 
 * is done in sata_read, save for the write argument that is passed to 
 * ahci_do_operation(). 
 *
 * @param  bdev        block device to write to
 * @param  buf         buffer to read from
 * @param  block       block number to start writing at
 * @param  block_count the number of blocks to write
 * @return             0 on success and <0 on error
 */
long sata_write_block(blockdev_t *bdev, const char *buf, blocknum_t block,
                      size_t block_count)
{
    long status = ahci_do_operation(bdev_to_ata_disk(bdev)->port, block * SATA_SECTORS_PER_BLOCK, block_count * SATA_SECTORS_PER_BLOCK, (void*) buf, 1);
    return status;
}
