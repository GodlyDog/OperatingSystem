/*
 *  File: bs_x86_64.S
 *  Date: 26 Mar 2019
 *  Acct: Sandy Harvie (charvie)
 *  Desc: Bootstrap routine for x86-64 ELF dynamic linker
 */

#include "asm.h"

.globl _bootstrap

_bootstrap:
        movq     8(%rsp), %rdi           # Pass argc
        movq     16(%rsp), %rsi          # Pass argv
        movq     24(%rsp), %rdx          # Pass envp
        movq     32(%rsp), %rcx          # Pass auxv
        call     _ldloadrtld             # Dynamic linking!

        movq     8(%rsp), %rdi           # Pass argc
        movq     16(%rsp), %rsi          # Pass argv
        movq     24(%rsp), %rdx          # Pass envp
        movq     32(%rsp), %rcx          # Pass auxv
        call     *%rax                   # Call main

        movq     %rax, %rdi              # Pass return value of main
        call     _ldcleanup              # Clean up and exit

ENTRY(_ld_bind)
        pushfq                           # Save RFLAGS
        pushq    %rax                    # Save %rax
        pushq    %rcx                    # Save %rcx
        pushq    %rdx                    # Save %rdx
        pushq    %rdi                    # Save %rdi
        pushq    %rsi                    # Save %rsi
        pushq    %r8                     # Save %r8
        pushq    %r9                     # Save %r9
        pushq    %r10                    # Save %r10
        pushq    %r11                    # Save %r11

        movq     80(%rsp), %rdi          # Copy module argument
        movq     88(%rsp), %rsi          # Copy reloff argument
        call     _rtresolve@PLT          # Transfer control to binder
        movq     %rax, 88(%rsp)          # Store target over reloff argument

        popq     %r11                    # Restore %r11
        popq     %r10                    # Restore %r10
        popq     %r9                     # Restore %r9
        popq     %r8                     # Restore %r8
        popq     %rsi                    # Restore %rsi
        popq     %rdi                    # Restore %rdi
        popq     %rdx                    # Restore %rdx
        popq     %rcx                    # Restore %rcx
        popq     %rax                    # Restore %rax
        popfq                            # Restore RFLAGS

        leaq     8(%rsp), %rsp           # Discard module without changing RFLAGSs
        ret                              # Return to target address
END(_ld_bind)
