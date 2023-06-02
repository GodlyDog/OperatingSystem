#ifndef __DYNAMIC__

.globl __libc_static_entry

__libc_static_entry:
	add $8, %rsp       /* Make sure when we overwrite dummy return address
	                      with the correct one, so args will be in the right
	                      place when we call main */
    movq (%rsp), %rdi  /* Copy arguments from stack into registers */
    movq 8(%rsp), %rsi
    movq 16(%rsp), %rdx
    movq 24(%rsp), %rcx
	call main
	mov %rax, %rdi
	call exit

#endif
