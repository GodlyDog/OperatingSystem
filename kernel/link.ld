OUTPUT_FORMAT(elf64-x86-64)

KERNEL_LMA = 0x00100000;
KERNEL_VMA = 0xffff800000000000;

ENTRY(_start)
STARTUP(entry/entry.o)

SECTIONS {
	. = KERNEL_VMA + KERNEL_LMA;

	k_start = .;

	.text : AT(ADDR(.text) - KERNEL_VMA) {
		_code = .;
		*(.multiboot)
		*(.text)
		. = ALIGN(0x1000);
	}

	csd_start = .;
	.csd : AT(ADDR(.csd) - KERNEL_VMA) {
		*(.csd)
		. = ALIGN(0x1000);
	}
	csd_end = .;

	.init :  AT(ADDR(.init) - KERNEL_VMA) {
		kernel_start_init = .;
		*(.init)
		. = ALIGN(0x1000);
		kernel_end_init = .;
	}


	.rodata : AT(ADDR(.rodata) - KERNEL_VMA) {
		_rodata = .;
		*(.rodata)
		. = ALIGN(0x1000);
	}

	.data : AT(ADDR(.data) - KERNEL_VMA) {
		_data = .;
		*(.data)
		. = ALIGN(0x1000);
	}

	_edata = .;

	.bss : AT(ADDR(.bss) - KERNEL_VMA) {
		_bss = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(0x1000);
	}

	_end = .;

	/DISCARD/ : {
		*(.comment)
		*(note.*)
	}
	kernel_phys_off = k_start - KERNEL_LMA;
	kernel_phys_base = k_start - kernel_phys_off;
	kernel_phys_end = _end - kernel_phys_off;
	kernel_page_tables = ((_end - k_start) / 0x80000) + 1; /* XXX might be 0x200000 */
	kernel_text_sectors = ((_end - k_start) / 512) + 1;
}