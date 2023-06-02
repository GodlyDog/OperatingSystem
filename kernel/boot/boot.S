.file "boot.S"

#define ASM_FILE 1
#include "multiboot.h"
#include "boot/config.h"
#undef ASM_FILE
#define AOUT_KLUDGE MULTIBOOT_AOUT_KLUDGE
#define PHYSADDR(x) (x - 0xffff800000000000)

.global entry, _start, initial_page_table

.code32
.set ARCH, 0
.set CHECKSUM, -(MULTIBOOT2_HEADER_MAGIC + ARCH + (multiboot_header_end - multiboot_header))

/* This header tells GRUB we can be run */
.section .multiboot
.align 8
multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC
    .long ARCH
    .long multiboot_header_end - multiboot_header
    .long CHECKSUM


.align 8
address_tag_start:
    .short MULTIBOOT_HEADER_TAG_ADDRESS
    .short MULTIBOOT_HEADER_TAG_OPTIONAL
    .long address_tag_end - address_tag_start
    .long PHYSADDR(multiboot_header) /*  header_addr = beginning of MB header */
    .long PHYSADDR(k_start) /*  load_addr = beginning of .text */
    .long PHYSADDR(_edata)  /*  load_end_addr = end of .data */
    .long PHYSADDR(_end)  /*  bss_end_addr = end of .bss */
address_tag_end:

.align 8
entry_address_tag_start:
    .short MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
    .short MULTIBOOT_HEADER_TAG_OPTIONAL
    .long entry_address_tag_end - entry_address_tag_start
    .long PHYSADDR(_start) /*  entry_addr */
entry_address_tag_end:

#if 0
.align 8
framebuffer_tag_start:
    .short 5
    .short 0
    .long frame_buffer_tag_end - framebuffer_tag_start
    .long 0 // 1280
    .long 0 // 720
    .long 0 // 32
frame_buffer_tag_end:
#endif

.align 8
.short MULTIBOOT_HEADER_TAG_END
    .short 0
    .long 8
multiboot_header_end:


_start:
    // disable interrupts during boot
    cli

    // Take the multiboot information and store it somewhere.
    movl    $PHYSADDR(sys_stack_bottom), %esp

    // reset the stack flags
    pushl $0
    popf

    // set base pointer
    movl    %esp, %ebp

    // pushl   %eax
    pushl   $0x0
    pushl   %ebx /* Stash the meminfo for later */

    // Set up the gdt
    lgdt PHYSADDR(GDTPointer)

    // set cr3 = start of PML4
    mov $PHYSADDR(pml4), %eax
    mov %eax, %cr3

    // enable PAE
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4

    // enter long mode
    mov $0xC0000080, %ecx
    rdmsr
    or $0x101, %eax
    wrmsr

    // Enable paging
    movl    %cr0, %eax
    or     $0x80000000, %eax
    movl    %eax, %cr0

    // jump into 64 bit code
    ljmp $0x08, $PHYSADDR(_trampoline)

.code64

// for some god-knows why reason, GDB wont set up breakpoints correctly without this trampoline
// even though Weenix still runs if you ljmp directly into _start64 -_-
_trampoline:
    // paging is at this point enabled, so no more need more PHYSADDR() wrappers
    movabsq $_start64, %rax
    jmp *%rax

_start64:
    // move the stack pointer to himem so that it is valid once we delete the low map
    movq $KERNEL_VMA, %rax
    addq %rax, %rsp
    addq %rax, %rbp
    
    popq %rbx
    movq %rbx, %r11

    // set up sregs
    movq $0x0, %rax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    mov %r11, %rdi
    // now we jump into the C entrypoint.
    call entry
    cli
    hlt // when its done, we are done
// [+] TODO we dont actually set the stack pointer anywhere here???

.align 16
GDT64:
    GDTNull:
        .quad 0
    GDTKernelCode:
        // base = 0x0, limit = 0x0
        // flags: present, ring 0, executable, readable, 64bit
        .word 0, 0
        .byte 0, 0x9a, 0x20, 0
    GDTEnd:
    GDTPointer:
        .word GDTEnd - GDT64 - 1 // size of gdt - 1
        .long PHYSADDR(GDT64) // pointer to gdt

.code32
.data
sys_stack: // set up 1KB stack
    .align 4
    .skip 0x1000
sys_stack_bottom:

.align 0x1000
initial_page_table: // maps first 1GB of RAM to both 0x0000000000000000 and 0xffff800000000000
pml4:
    .quad PHYSADDR(pdpt) + 3 // 0x0000000000000000
    .fill 255,8,0
    .quad PHYSADDR(pdpt) + 3 // 0xffff800000000000
    .fill 255,8,0
pdpt:
    .quad 0x0000000000000083 // 0
    .fill 511,8,0



