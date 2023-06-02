#define CR0_PG 0x80000000
#define CR0_PE 0x00000001

#define CR4_PAE 0x00000020
#define CR4_PGE 0x00000080

#define PHYSADDR(x) (x - smp_initialization_start)

# NOTE: THIS CODE REQUIRES THAT IT BE PLACED STARTING AT PHYSICAL ADDRESS 0x0

.file "smp_trampoline.S"
.global smp_initialization_start, smp_initialization_end

smp_initialization_start:

/* When we initialize a processor, it starts in 16-bit real mode */
.code16
.align 0x1000
smp_processor_init:
    cli

    // enable PAE
    mov $(CR4_PAE | CR4_PGE), %eax
    mov %eax, %cr4

    mov $PHYSADDR(pml4), %eax
    mov %eax, %cr3

    // enter long mode
    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr

    lgdt PHYSADDR(GDTPointer)

    // Enable paging AND protection simultaneously
    movl    %cr0, %eax
    or     $(CR0_PG | CR0_PE), %eax
    movl    %eax, %cr0

    ljmp $0x8, $PHYSADDR(smp_trampoline)

.code64
smp_trampoline:
    movabsq $(0xffff880000000000 + PHYSADDR(stack_pointer)), %rsp
    xor %rbp, %rbp
    movabsq $smp_processor_entry, %rax
    call *%rax


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

.align 0x1000
pml4: // maps first 1GB of RAM to both 0x0000000000000000 and 0xffff800000000000
    .quad PHYSADDR(pdpt) + 3
    .fill 255,8,0
    .quad PHYSADDR(pdpt) + 3
    .fill 15,8,0
    .quad PHYSADDR(pdpt) + 3
    .fill 239,8,0
pdpt:
    .quad 0x0000000000000083
    .fill 511,8,0

.skip 0x1000
stack_pointer:

smp_initialization_end:
