#pragma once

#define IDENTITY_MAPPED_RAM_SIZE (1 << 16)

#define KERNEL_PHYS_BASE ((uintptr_t)(&kernel_phys_base))
#define KERNEL_PHYS_END ((uintptr_t)(&kernel_phys_end))
#define KERNEL_VMA 0xffff800000000000

// https://www.usenix.org/sites/default/files/conference/protected-files/sec14_slides_kemerlis.pdf
#define PHYS_OFFSET 0xffff880000000000

#define MEMORY_MAP_BASE 0x9000
