	.file	"kmutex.c"
	.text
.Ltext0:
	.type	equals, @function
equals:
.LFB0:
	.file 1 "include/util/debug.h"
	.loc 1 281 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc 1 282 0
	movq	-8(%rbp), %rax
	cmpq	-16(%rbp), %rax
	sete	%al
	movzbl	%al, %eax
	.loc 1 283 0
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	equals, .-equals
	.type	notequals, @function
notequals:
.LFB1:
	.loc 1 285 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc 1 285 0
	movq	-8(%rbp), %rax
	cmpq	-16(%rbp), %rax
	setne	%al
	movzbl	%al, %eax
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	notequals, .-notequals
	.type	lessthan, @function
lessthan:
.LFB2:
	.loc 1 287 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc 1 287 0
	movq	-8(%rbp), %rax
	cmpq	-16(%rbp), %rax
	setl	%al
	movzbl	%al, %eax
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	lessthan, .-lessthan
	.type	greaterthan, @function
greaterthan:
.LFB3:
	.loc 1 289 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc 1 289 0
	movq	-8(%rbp), %rax
	cmpq	-16(%rbp), %rax
	setg	%al
	movzbl	%al, %eax
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	greaterthan, .-greaterthan
	.type	lessthaneq, @function
lessthaneq:
.LFB4:
	.loc 1 291 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc 1 291 0
	movq	-8(%rbp), %rax
	cmpq	-16(%rbp), %rax
	setle	%al
	movzbl	%al, %eax
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	lessthaneq, .-lessthaneq
	.type	greaterthaneq, @function
greaterthaneq:
.LFB5:
	.loc 1 293 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	.loc 1 293 0
	movq	-8(%rbp), %rax
	cmpq	-16(%rbp), %rax
	setge	%al
	movzbl	%al, %eax
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	greaterthaneq, .-greaterthaneq
	.type	intr_enabled, @function
intr_enabled:
.LFB6:
	.file 2 "include/main/interrupt.h"
	.loc 2 83 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	.loc 2 85 0
#APP
# 85 "include/main/interrupt.h" 1
	pushf; pop %rax; and $0x200, %rax;
# 0 "" 2
#NO_APP
	movq	%rax, -8(%rbp)
	.loc 2 87 0
	movq	-8(%rbp), %rax
	.loc 2 88 0
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	intr_enabled, .-intr_enabled
	.type	intr_enable, @function
intr_enable:
.LFB7:
	.loc 2 90 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 90 0
#APP
# 90 "include/main/interrupt.h" 1
	sti
# 0 "" 2
#NO_APP
	nop
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	intr_enable, .-intr_enable
	.type	intr_disable, @function
intr_disable:
.LFB8:
	.loc 2 92 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 92 0
#APP
# 92 "include/main/interrupt.h" 1
	cli
# 0 "" 2
#NO_APP
	nop
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	intr_disable, .-intr_disable
	.type	intr_wait, @function
intr_wait:
.LFB9:
	.loc 2 99 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 106 0
#APP
# 106 "include/main/interrupt.h" 1
	sti; hlt
# 0 "" 2
	.loc 2 107 0
#NO_APP
	nop
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	intr_wait, .-intr_wait
	.section	.rodata
	.align 8
.LC0:
	.string	"detected deadlock between P%d and P%d (mutexes 0x%p, 0x%p)\n"
.LC1:
	.string	"proc/kmutex.c"
	.text
	.globl	detect_deadlocks
	.type	detect_deadlocks, @function
detect_deadlocks:
.LFB10:
	.file 3 "proc/kmutex.c"
	.loc 3 21 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -40(%rbp)
.LBB10:
	.loc 3 23 0
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	movq	136(%rax), %rax
	subq	$48, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	48(%rax), %rax
	subq	$48, %rax
	movq	%rax, -16(%rbp)
	jmp	.L19
.L23:
.LBB11:
	.loc 3 25 0
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	subq	$120, %rax
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	120(%rax), %rax
	subq	$120, %rax
	movq	%rax, -32(%rbp)
	jmp	.L20
.L22:
	.loc 3 27 0
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	cmpq	%rax, -24(%rbp)
	jne	.L21
	.loc 3 29 0
	movq	-24(%rbp), %rax
	movq	72(%rax), %rax
	movl	(%rax), %edx
	movabsq	$curproc, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	pushq	-40(%rbp)
	pushq	-8(%rbp)
	movl	%edx, %r9d
	movl	%eax, %r8d
	movabsq	$.LC0, %rcx
	movabsq	$__func__.1872, %rdx
	movl	$32, %esi
	movabsq	$.LC1, %rdi
	movl	$0, %eax
	movabsq	$dbg_panic, %r10
	call	*%r10
.LVL0:
.L21:
	.loc 3 25 0 discriminator 2
	movq	-32(%rbp), %rax
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	120(%rax), %rax
	subq	$120, %rax
	movq	%rax, -32(%rbp)
.L20:
	.loc 3 25 0 is_stmt 0 discriminator 1
	movq	-24(%rbp), %rax
	leaq	120(%rax), %rdx
	movq	-8(%rbp), %rax
	cmpq	%rax, %rdx
	jne	.L22
.LBE11:
	.loc 3 23 0 is_stmt 1 discriminator 2
	movq	-16(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	48(%rax), %rax
	subq	$48, %rax
	movq	%rax, -16(%rbp)
.L19:
	.loc 3 23 0 is_stmt 0 discriminator 1
	movq	-8(%rbp), %rax
	leaq	48(%rax), %rdx
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	addq	$136, %rax
	cmpq	%rax, %rdx
	jne	.L23
.LBE10:
	.loc 3 37 0 is_stmt 1
	nop
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	detect_deadlocks, .-detect_deadlocks
	.globl	kmutex_init
	.type	kmutex_init, @function
kmutex_init:
.LFB11:
	.loc 3 43 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.loc 3 44 0
	movq	-8(%rbp), %rax
	movq	$0, 32(%rax)
	.loc 3 45 0
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	movabsq	$sched_queue_init, %rax
	call	*%rax
.LVL1:
	.loc 3 46 0
	movq	-8(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	movabsq	$spinlock_init, %rax
	call	*%rax
.LVL2:
	.loc 3 47 0
	movq	-8(%rbp), %rax
	addq	$48, %rax
	movq	%rax, %rdi
	movabsq	$list_link_init, %rax
	call	*%rax
.LVL3:
	.loc 3 48 0
	nop
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	kmutex_init, .-kmutex_init
	.section	.rodata
.LC2:
	.string	"%s"
.LC3:
	.string	"C%ld P%ld "
.LC4:
	.string	"%s:%d %s(): "
.LC5:
	.string	"locked mutex: %p\n"
.LC6:
	.string	"\033[0m"
	.align 8
.LC7:
	.string	"curthr && \"need thread context to lock mutex\""
.LC8:
	.string	"assertion failed: %s"
	.align 8
.LC9:
	.string	"!kmutex_owns_mutex(mtx) && \"already owner\""
.LC10:
	.string	"kmutex_owns_mutex(mtx)"
	.text
	.globl	kmutex_lock
	.type	kmutex_lock, @function
kmutex_lock:
.LFB12:
	.loc 3 59 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -40(%rbp)
	.loc 3 61 0
	movabsq	$dbg_modes, %rax
	movq	(%rax), %rax
	andl	$16384, %eax
	testq	%rax, %rax
	je	.L26
.LBB12:
	.loc 3 61 0 is_stmt 0 discriminator 1
	movl	$0, %eax
	movabsq	$apic_initialized, %rdx
	call	*%rdx
.LVL4:
	testq	%rax, %rax
	je	.L27
	.loc 3 61 0 discriminator 2
	movl	$224, %edi
	movabsq	$intr_setipl, %rax
	call	*%rax
.LVL5:
	jmp	.L28
.L27:
	.loc 3 61 0 discriminator 3
	movl	$0, %eax
.L28:
	.loc 3 61 0 discriminator 5
	movb	%al, -1(%rbp)
	movabsq	$debug_lock, %rdi
	movabsq	$spinlock_lock, %rax
	call	*%rax
.LVL6:
	movl	$16384, %edi
	movabsq	$dbg_color, %rax
	call	*%rax
.LVL7:
	movq	%rax, %rsi
	movabsq	$.LC2, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rdx
	call	*%rdx
.LVL8:
	movabsq	$curproc, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L29
	.loc 3 61 0 discriminator 6
	movabsq	$curproc, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cltq
	jmp	.L30
.L29:
	.loc 3 61 0 discriminator 7
	movq	$-1, %rax
.L30:
	.loc 3 61 0 discriminator 9
	movabsq	$curcore, %rdx
	movq	(%rdx), %rcx
	movq	%rax, %rdx
	movq	%rcx, %rsi
	movabsq	$.LC3, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rcx
	call	*%rcx
.LVL9:
	movabsq	$__func__.1886, %rcx
	movl	$61, %edx
	movabsq	$.LC1, %rsi
	movabsq	$.LC4, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %r8
	call	*%r8
.LVL10:
	movq	-40(%rbp), %rax
	movq	%rax, %rsi
	movabsq	$.LC5, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rdx
	call	*%rdx
.LVL11:
	movabsq	$.LC6, %rsi
	movabsq	$.LC2, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rdx
	call	*%rdx
.LVL12:
	movabsq	$debug_lock, %rdi
	movabsq	$spinlock_unlock, %rax
	call	*%rax
.LVL13:
	movl	$0, %eax
	movabsq	$apic_initialized, %rdx
	call	*%rdx
.LVL14:
	testq	%rax, %rax
	je	.L26
	.loc 3 61 0 discriminator 10
	movzbl	-1(%rbp), %eax
	movl	%eax, %edi
	movabsq	$intr_setipl, %rax
	call	*%rax
.LVL15:
.L26:
.LBE12:
	.loc 3 62 0 is_stmt 1
	movq	-40(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	movabsq	$spinlock_lock, %rax
	call	*%rax
.LVL16:
	.loc 3 63 0
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	jne	.L31
	.loc 3 63 0 is_stmt 0 discriminator 1
	movabsq	$.LC7, %r8
	movabsq	$.LC8, %rcx
	movabsq	$__func__.1886, %rdx
	movl	$63, %esi
	movabsq	$.LC1, %rdi
	movl	$0, %eax
	movabsq	$dbg_panic, %r9
	call	*%r9
.LVL17:
.L31:
	movq	-40(%rbp), %rax
	movq	%rax, -16(%rbp)
.LBB13:
.LBB14:
	.loc 3 118 0 is_stmt 1
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L32
	movq	-16(%rbp), %rax
	movq	32(%rax), %rdx
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	cmpq	%rax, %rdx
	jne	.L32
	movl	$1, %eax
	jmp	.L33
.L32:
	movl	$0, %eax
.L33:
	cltq
.LBE14:
.LBE13:
	.loc 3 64 0
	testq	%rax, %rax
	je	.L35
	.loc 3 64 0 is_stmt 0 discriminator 3
	movabsq	$.LC9, %r8
	movabsq	$.LC8, %rcx
	movabsq	$__func__.1886, %rdx
	movl	$64, %esi
	movabsq	$.LC1, %rdi
	movl	$0, %eax
	movabsq	$dbg_panic, %r9
	call	*%r9
.LVL18:
.L35:
	.loc 3 66 0 is_stmt 1
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	testq	%rax, %rax
	je	.L36
	.loc 3 68 0
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movabsq	$detect_deadlocks, %rax
	call	*%rax
.LVL19:
	.loc 3 69 0
	movq	-40(%rbp), %rax
	leaq	40(%rax), %rdx
	movq	-40(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movabsq	$sched_sleep_on, %rax
	call	*%rax
.LVL20:
	movq	-40(%rbp), %rax
	movq	%rax, -24(%rbp)
.LBB15:
.LBB16:
	.loc 3 118 0
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L37
	movq	-24(%rbp), %rax
	movq	32(%rax), %rdx
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	cmpq	%rax, %rdx
	jne	.L37
	movl	$1, %eax
	jmp	.L38
.L37:
	movl	$0, %eax
.L38:
	cltq
.LBE16:
.LBE15:
	.loc 3 70 0
	testq	%rax, %rax
	jne	.L41
	.loc 3 70 0 is_stmt 0 discriminator 1
	movabsq	$.LC10, %r8
	movabsq	$.LC8, %rcx
	movabsq	$__func__.1886, %rdx
	movl	$70, %esi
	movabsq	$.LC1, %rdi
	movl	$0, %eax
	movabsq	$dbg_panic, %r9
	call	*%r9
.LVL21:
.L36:
	.loc 3 74 0 is_stmt 1
	movabsq	$curthr, %rax
	movq	(%rax), %rdx
	movq	-40(%rbp), %rax
	movq	%rdx, 32(%rax)
	.loc 3 75 0
	movq	-40(%rbp), %rax
	leaq	48(%rax), %rdx
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	addq	$136, %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movabsq	$list_insert_tail, %rax
	call	*%rax
.LVL22:
	.loc 3 76 0
	movq	-40(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	movabsq	$spinlock_unlock, %rax
	call	*%rax
.LVL23:
.L41:
	.loc 3 78 0
	nop
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	kmutex_lock, .-kmutex_lock
	.section	.rodata
.LC11:
	.string	"unlocked mutex: %p\n"
	.align 8
.LC12:
	.string	"curthr && (curthr == mtx->km_holder) && \"unlocking a mutex we don\\'t own\""
.LC13:
	.string	"!kmutex_owns_mutex(mtx)"
	.text
	.globl	kmutex_unlock
	.type	kmutex_unlock, @function
kmutex_unlock:
.LFB13:
	.loc 3 91 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	.loc 3 92 0
	movabsq	$dbg_modes, %rax
	movq	(%rax), %rax
	andl	$16384, %eax
	testq	%rax, %rax
	je	.L43
.LBB17:
	.loc 3 92 0 is_stmt 0 discriminator 1
	movl	$0, %eax
	movabsq	$apic_initialized, %rdx
	call	*%rdx
.LVL24:
	testq	%rax, %rax
	je	.L44
	.loc 3 92 0 discriminator 2
	movl	$224, %edi
	movabsq	$intr_setipl, %rax
	call	*%rax
.LVL25:
	jmp	.L45
.L44:
	.loc 3 92 0 discriminator 3
	movl	$0, %eax
.L45:
	.loc 3 92 0 discriminator 5
	movb	%al, -1(%rbp)
	movabsq	$debug_lock, %rdi
	movabsq	$spinlock_lock, %rax
	call	*%rax
.LVL26:
	movl	$16384, %edi
	movabsq	$dbg_color, %rax
	call	*%rax
.LVL27:
	movq	%rax, %rsi
	movabsq	$.LC2, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rdx
	call	*%rdx
.LVL28:
	movabsq	$curproc, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L46
	.loc 3 92 0 discriminator 6
	movabsq	$curproc, %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	cltq
	jmp	.L47
.L46:
	.loc 3 92 0 discriminator 7
	movq	$-1, %rax
.L47:
	.loc 3 92 0 discriminator 9
	movabsq	$curcore, %rdx
	movq	(%rdx), %rcx
	movq	%rax, %rdx
	movq	%rcx, %rsi
	movabsq	$.LC3, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rcx
	call	*%rcx
.LVL29:
	movabsq	$__func__.1891, %rcx
	movl	$92, %edx
	movabsq	$.LC1, %rsi
	movabsq	$.LC4, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %r8
	call	*%r8
.LVL30:
	movq	-24(%rbp), %rax
	movq	%rax, %rsi
	movabsq	$.LC11, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rdx
	call	*%rdx
.LVL31:
	movabsq	$.LC6, %rsi
	movabsq	$.LC2, %rdi
	movl	$0, %eax
	movabsq	$dbg_print, %rdx
	call	*%rdx
.LVL32:
	movabsq	$debug_lock, %rdi
	movabsq	$spinlock_unlock, %rax
	call	*%rax
.LVL33:
	movl	$0, %eax
	movabsq	$apic_initialized, %rdx
	call	*%rdx
.LVL34:
	testq	%rax, %rax
	je	.L43
	.loc 3 92 0 discriminator 10
	movzbl	-1(%rbp), %eax
	movl	%eax, %edi
	movabsq	$intr_setipl, %rax
	call	*%rax
.LVL35:
.L43:
.LBE17:
	.loc 3 93 0 is_stmt 1
	movq	-24(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	movabsq	$spinlock_lock, %rax
	call	*%rax
.LVL36:
	.loc 3 94 0
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L48
	.loc 3 94 0 is_stmt 0 discriminator 2
	movq	-24(%rbp), %rax
	movq	32(%rax), %rdx
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	cmpq	%rax, %rdx
	je	.L49
.L48:
	.loc 3 94 0 discriminator 3
	movabsq	$.LC12, %r8
	movabsq	$.LC8, %rcx
	movabsq	$__func__.1891, %rdx
	movl	$95, %esi
	movabsq	$.LC1, %rdi
	movl	$0, %eax
	movabsq	$dbg_panic, %r9
	call	*%r9
.LVL37:
.L49:
	.loc 3 96 0 is_stmt 1
	movq	-24(%rbp), %rax
	leaq	32(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movabsq	$sched_wakeup_on, %rax
	call	*%rax
.LVL38:
	movq	-24(%rbp), %rax
	movq	%rax, -16(%rbp)
.LBB18:
.LBB19:
	.loc 3 118 0
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L50
	movq	-16(%rbp), %rax
	movq	32(%rax), %rdx
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	cmpq	%rax, %rdx
	jne	.L50
	movl	$1, %eax
	jmp	.L51
.L50:
	movl	$0, %eax
.L51:
	cltq
.LBE19:
.LBE18:
	.loc 3 97 0
	testq	%rax, %rax
	je	.L53
	.loc 3 97 0 is_stmt 0 discriminator 1
	movabsq	$.LC13, %r8
	movabsq	$.LC8, %rcx
	movabsq	$__func__.1891, %rdx
	movl	$97, %esi
	movabsq	$.LC1, %rdi
	movl	$0, %eax
	movabsq	$dbg_panic, %r9
	call	*%r9
.LVL39:
.L53:
	.loc 3 98 0 is_stmt 1
	movq	-24(%rbp), %rax
	addq	$48, %rax
	movq	%rax, %rdi
	movabsq	$list_remove, %rax
	call	*%rax
.LVL40:
	.loc 3 99 0
	movq	-24(%rbp), %rax
	movq	32(%rax), %rax
	testq	%rax, %rax
	je	.L54
	.loc 3 100 0
	movq	-24(%rbp), %rax
	leaq	48(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	32(%rax), %rax
	addq	$136, %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movabsq	$list_insert_tail, %rax
	call	*%rax
.LVL41:
.L54:
	.loc 3 101 0
	movq	-24(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	movabsq	$spinlock_unlock, %rax
	call	*%rax
.LVL42:
	.loc 3 102 0
	nop
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	kmutex_unlock, .-kmutex_unlock
	.globl	kmutex_has_waiters
	.type	kmutex_has_waiters, @function
kmutex_has_waiters:
.LFB14:
	.loc 3 108 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.loc 3 109 0
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	movabsq	$sched_queue_empty, %rax
	call	*%rax
.LVL43:
	testq	%rax, %rax
	sete	%al
	movzbl	%al, %eax
	.loc 3 111 0
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE14:
	.size	kmutex_has_waiters, .-kmutex_has_waiters
	.globl	kmutex_owns_mutex
	.type	kmutex_owns_mutex, @function
kmutex_owns_mutex:
.LFB15:
	.loc 3 117 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$8, %rsp
	movq	%rdi, -8(%rbp)
	.loc 3 118 0
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L58
	.loc 3 118 0 is_stmt 0 discriminator 1
	movq	-8(%rbp), %rax
	movq	32(%rax), %rdx
	movabsq	$curthr, %rax
	movq	(%rax), %rax
	cmpq	%rax, %rdx
	jne	.L58
	.loc 3 118 0 discriminator 3
	movl	$1, %eax
	jmp	.L59
.L58:
	.loc 3 118 0 discriminator 4
	movl	$0, %eax
.L59:
	.loc 3 118 0 discriminator 6
	cltq
	.loc 3 119 0 is_stmt 1 discriminator 6
	leave
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE15:
	.size	kmutex_owns_mutex, .-kmutex_owns_mutex
	.section	.rodata
	.align 16
	.type	__func__.1872, @object
	.size	__func__.1872, 17
__func__.1872:
	.string	"detect_deadlocks"
	.align 8
	.type	__func__.1886, @object
	.size	__func__.1886, 12
__func__.1886:
	.string	"kmutex_lock"
	.align 8
	.type	__func__.1891, @object
	.size	__func__.1891, 14
__func__.1891:
	.string	"kmutex_unlock"
	.text
.Letext0:
	.file 4 "include/proc/spinlock.h"
	.file 5 "include/types.h"
	.file 6 "include/kernel.h"
	.file 7 "include/util/list.h"
	.file 8 "include/proc/sched.h"
	.file 9 "include/proc/kmutex.h"
	.file 10 "include/proc/kthread.h"
	.file 11 "include/vm/vmmap.h"
	.file 12 "include/proc/proc.h"
	.file 13 "include/mm/pagetable.h"
	.file 14 "include/proc/context.h"
	.file 15 "include/proc/core.h"
	.file 16 "include/main/smp.h"
	.file 17 "include/globals.h"
	.file 18 "include/main/apic.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0xe00
	.value	0x3
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF828
	.byte	0xc
	.long	.LASF829
	.long	.LASF830
	.quad	.Ltext0
	.quad	.Letext0
	.long	.Ldebug_line0
	.long	.Ldebug_macro0
	.uleb128 0x2
	.long	.LASF701
	.byte	0x1
	.byte	0x4
	.byte	0x3
	.long	0x4a
	.uleb128 0x3
	.long	.LASF703
	.byte	0x4
	.byte	0x5
	.long	0x51
	.byte	0
	.byte	0
	.uleb128 0x4
	.byte	0x1
	.byte	0x6
	.long	.LASF673
	.uleb128 0x5
	.long	0x4a
	.uleb128 0x6
	.long	0x4a
	.uleb128 0x7
	.long	.LASF675
	.byte	0x4
	.byte	0x6
	.long	0x31
	.uleb128 0x4
	.byte	0x1
	.byte	0x6
	.long	.LASF674
	.uleb128 0x7
	.long	.LASF676
	.byte	0x5
	.byte	0xa
	.long	0x78
	.uleb128 0x4
	.byte	0x1
	.byte	0x8
	.long	.LASF677
	.uleb128 0x4
	.byte	0x2
	.byte	0x5
	.long	.LASF678
	.uleb128 0x4
	.byte	0x2
	.byte	0x7
	.long	.LASF679
	.uleb128 0x7
	.long	.LASF680
	.byte	0x5
	.byte	0xd
	.long	0x98
	.uleb128 0x8
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x4
	.byte	0x4
	.byte	0x7
	.long	.LASF681
	.uleb128 0x4
	.byte	0x8
	.byte	0x5
	.long	.LASF682
	.uleb128 0x7
	.long	.LASF683
	.byte	0x5
	.byte	0x11
	.long	0xb8
	.uleb128 0x4
	.byte	0x8
	.byte	0x7
	.long	.LASF684
	.uleb128 0x7
	.long	.LASF685
	.byte	0x5
	.byte	0x13
	.long	0xb8
	.uleb128 0x7
	.long	.LASF686
	.byte	0x5
	.byte	0x14
	.long	0xad
	.uleb128 0x7
	.long	.LASF687
	.byte	0x5
	.byte	0x18
	.long	0x8d
	.uleb128 0x9
	.long	.LASF688
	.byte	0x6
	.byte	0xc
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0xa
	.byte	0x8
	.uleb128 0x9
	.long	.LASF689
	.byte	0x6
	.byte	0xd
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF690
	.byte	0x6
	.byte	0xe
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF691
	.byte	0x6
	.byte	0xf
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF692
	.byte	0x6
	.byte	0x10
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF693
	.byte	0x6
	.byte	0x11
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF694
	.byte	0x6
	.byte	0x12
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF695
	.byte	0x6
	.byte	0x13
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF696
	.byte	0x6
	.byte	0x14
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF697
	.byte	0x6
	.byte	0x15
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF698
	.byte	0x6
	.byte	0x16
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF699
	.byte	0x6
	.byte	0x18
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF700
	.byte	0x6
	.byte	0x19
	.long	0xed
	.byte	0x1
	.byte	0x1
	.uleb128 0x2
	.long	.LASF702
	.byte	0x10
	.byte	0x7
	.byte	0x3c
	.long	0x1b0
	.uleb128 0x3
	.long	.LASF704
	.byte	0x7
	.byte	0x3e
	.long	0x1b0
	.byte	0
	.uleb128 0x3
	.long	.LASF705
	.byte	0x7
	.byte	0x3f
	.long	0x1b0
	.byte	0x8
	.byte	0
	.uleb128 0xb
	.byte	0x8
	.long	0x18b
	.uleb128 0x7
	.long	.LASF706
	.byte	0x7
	.byte	0x40
	.long	0x18b
	.uleb128 0x7
	.long	.LASF707
	.byte	0x7
	.byte	0x40
	.long	0x18b
	.uleb128 0x2
	.long	.LASF708
	.byte	0x20
	.byte	0x8
	.byte	0xf
	.long	0x1fd
	.uleb128 0x3
	.long	.LASF709
	.byte	0x8
	.byte	0x11
	.long	0x1b6
	.byte	0
	.uleb128 0x3
	.long	.LASF710
	.byte	0x8
	.byte	0x12
	.long	0xca
	.byte	0x10
	.uleb128 0x3
	.long	.LASF711
	.byte	0x8
	.byte	0x13
	.long	0x5b
	.byte	0x18
	.byte	0
	.uleb128 0x7
	.long	.LASF712
	.byte	0x8
	.byte	0x14
	.long	0x1cc
	.uleb128 0x2
	.long	.LASF713
	.byte	0x40
	.byte	0x9
	.byte	0xa
	.long	0x245
	.uleb128 0x3
	.long	.LASF714
	.byte	0x9
	.byte	0xc
	.long	0x1fd
	.byte	0
	.uleb128 0x3
	.long	.LASF715
	.byte	0x9
	.byte	0xd
	.long	0x2fa
	.byte	0x20
	.uleb128 0x3
	.long	.LASF716
	.byte	0x9
	.byte	0xe
	.long	0x5b
	.byte	0x28
	.uleb128 0x3
	.long	.LASF717
	.byte	0x9
	.byte	0xf
	.long	0x1c1
	.byte	0x30
	.byte	0
	.uleb128 0x2
	.long	.LASF718
	.byte	0xa8
	.byte	0xa
	.byte	0x21
	.long	0x2fa
	.uleb128 0x3
	.long	.LASF719
	.byte	0xa
	.byte	0x23
	.long	0x4b5
	.byte	0
	.uleb128 0x3
	.long	.LASF720
	.byte	0xa
	.byte	0x24
	.long	0x575
	.byte	0x30
	.uleb128 0x3
	.long	.LASF721
	.byte	0xa
	.byte	0x25
	.long	0xed
	.byte	0x38
	.uleb128 0x3
	.long	.LASF722
	.byte	0xa
	.byte	0x26
	.long	0xa6
	.byte	0x40
	.uleb128 0x3
	.long	.LASF723
	.byte	0xa
	.byte	0x27
	.long	0x419
	.byte	0x48
	.uleb128 0x3
	.long	.LASF724
	.byte	0xa
	.byte	0x29
	.long	0xa6
	.byte	0x50
	.uleb128 0x3
	.long	.LASF725
	.byte	0xa
	.byte	0x2a
	.long	0x509
	.byte	0x58
	.uleb128 0x3
	.long	.LASF726
	.byte	0xa
	.byte	0x2b
	.long	0x56a
	.byte	0x60
	.uleb128 0x3
	.long	.LASF727
	.byte	0xa
	.byte	0x2d
	.long	0x5b
	.byte	0x64
	.uleb128 0x3
	.long	.LASF728
	.byte	0xa
	.byte	0x2f
	.long	0x1c1
	.byte	0x68
	.uleb128 0x3
	.long	.LASF729
	.byte	0xa
	.byte	0x31
	.long	0x1c1
	.byte	0x78
	.uleb128 0x3
	.long	.LASF730
	.byte	0xa
	.byte	0x33
	.long	0x1b6
	.byte	0x88
	.uleb128 0x3
	.long	.LASF731
	.byte	0xa
	.byte	0x34
	.long	0xa6
	.byte	0x98
	.uleb128 0x3
	.long	.LASF732
	.byte	0xa
	.byte	0x36
	.long	0xad
	.byte	0xa0
	.byte	0
	.uleb128 0xb
	.byte	0x8
	.long	0x245
	.uleb128 0x7
	.long	.LASF733
	.byte	0x9
	.byte	0x10
	.long	0x208
	.uleb128 0x2
	.long	.LASF734
	.byte	0x18
	.byte	0xb
	.byte	0xe
	.long	0x330
	.uleb128 0x3
	.long	.LASF735
	.byte	0xb
	.byte	0x10
	.long	0x1b6
	.byte	0
	.uleb128 0x3
	.long	.LASF736
	.byte	0xb
	.byte	0x11
	.long	0x419
	.byte	0x10
	.byte	0
	.uleb128 0xc
	.long	.LASF737
	.value	0x2b0
	.byte	0xc
	.byte	0x1b
	.long	0x419
	.uleb128 0x3
	.long	.LASF738
	.byte	0xc
	.byte	0x1d
	.long	0xd5
	.byte	0
	.uleb128 0x3
	.long	.LASF739
	.byte	0xc
	.byte	0x1e
	.long	0x5ab
	.byte	0x4
	.uleb128 0xd
	.long	.LASF740
	.byte	0xc
	.byte	0x20
	.long	0x1b6
	.value	0x108
	.uleb128 0xd
	.long	.LASF741
	.byte	0xc
	.byte	0x21
	.long	0x1b6
	.value	0x118
	.uleb128 0xd
	.long	.LASF742
	.byte	0xc
	.byte	0x22
	.long	0x5b
	.value	0x128
	.uleb128 0xd
	.long	.LASF743
	.byte	0xc
	.byte	0x23
	.long	0x419
	.value	0x130
	.uleb128 0xd
	.long	.LASF744
	.byte	0xc
	.byte	0x25
	.long	0x1c1
	.value	0x138
	.uleb128 0xd
	.long	.LASF745
	.byte	0xc
	.byte	0x26
	.long	0x1c1
	.value	0x148
	.uleb128 0xd
	.long	.LASF746
	.byte	0xc
	.byte	0x28
	.long	0xa6
	.value	0x158
	.uleb128 0xd
	.long	.LASF747
	.byte	0xc
	.byte	0x29
	.long	0x5a0
	.value	0x160
	.uleb128 0xd
	.long	.LASF748
	.byte	0xc
	.byte	0x2b
	.long	0x4af
	.value	0x168
	.uleb128 0xd
	.long	.LASF749
	.byte	0xc
	.byte	0x32
	.long	0x1fd
	.value	0x170
	.uleb128 0xd
	.long	.LASF750
	.byte	0xc
	.byte	0x35
	.long	0x5bb
	.value	0x190
	.uleb128 0xd
	.long	.LASF751
	.byte	0xc
	.byte	0x36
	.long	0x5dd
	.value	0x290
	.uleb128 0xd
	.long	.LASF752
	.byte	0xc
	.byte	0x39
	.long	0xed
	.value	0x298
	.uleb128 0xd
	.long	.LASF753
	.byte	0xc
	.byte	0x3a
	.long	0xed
	.value	0x2a0
	.uleb128 0xd
	.long	.LASF754
	.byte	0xc
	.byte	0x3b
	.long	0x41f
	.value	0x2a8
	.byte	0
	.uleb128 0xb
	.byte	0x8
	.long	0x330
	.uleb128 0xb
	.byte	0x8
	.long	0x30b
	.uleb128 0xe
	.string	"pt"
	.value	0x1000
	.byte	0xd
	.byte	0x18
	.long	0x43e
	.uleb128 0x3
	.long	.LASF755
	.byte	0xd
	.byte	0x1a
	.long	0x43e
	.byte	0
	.byte	0
	.uleb128 0xf
	.long	0xbf
	.long	0x44f
	.uleb128 0x10
	.long	0xb8
	.value	0x1ff
	.byte	0
	.uleb128 0x7
	.long	.LASF756
	.byte	0xd
	.byte	0x1b
	.long	0x425
	.uleb128 0x2
	.long	.LASF757
	.byte	0x30
	.byte	0xe
	.byte	0xd
	.long	0x4af
	.uleb128 0x3
	.long	.LASF758
	.byte	0xe
	.byte	0xf
	.long	0xbf
	.byte	0
	.uleb128 0x3
	.long	.LASF759
	.byte	0xe
	.byte	0x10
	.long	0xbf
	.byte	0x8
	.uleb128 0x3
	.long	.LASF760
	.byte	0xe
	.byte	0x11
	.long	0xbf
	.byte	0x10
	.uleb128 0x3
	.long	.LASF761
	.byte	0xe
	.byte	0x14
	.long	0x4af
	.byte	0x18
	.uleb128 0x3
	.long	.LASF762
	.byte	0xe
	.byte	0x16
	.long	0xbf
	.byte	0x20
	.uleb128 0x3
	.long	.LASF763
	.byte	0xe
	.byte	0x17
	.long	0xca
	.byte	0x28
	.byte	0
	.uleb128 0xb
	.byte	0x8
	.long	0x44f
	.uleb128 0x7
	.long	.LASF764
	.byte	0xe
	.byte	0x18
	.long	0x45a
	.uleb128 0x2
	.long	.LASF765
	.byte	0x50
	.byte	0xf
	.byte	0x7
	.long	0x509
	.uleb128 0x3
	.long	.LASF766
	.byte	0xf
	.byte	0x9
	.long	0xa6
	.byte	0
	.uleb128 0x3
	.long	.LASF767
	.byte	0xf
	.byte	0xa
	.long	0x4b5
	.byte	0x8
	.uleb128 0x3
	.long	.LASF768
	.byte	0xf
	.byte	0xc
	.long	0x509
	.byte	0x38
	.uleb128 0x3
	.long	.LASF769
	.byte	0xf
	.byte	0xd
	.long	0x50f
	.byte	0x40
	.uleb128 0x3
	.long	.LASF770
	.byte	0xf
	.byte	0xf
	.long	0xbf
	.byte	0x48
	.byte	0
	.uleb128 0xb
	.byte	0x8
	.long	0x1fd
	.uleb128 0xb
	.byte	0x8
	.long	0x5b
	.uleb128 0x7
	.long	.LASF771
	.byte	0xf
	.byte	0x10
	.long	0x4c0
	.uleb128 0xf
	.long	0xbf
	.long	0x52b
	.uleb128 0x11
	.byte	0
	.uleb128 0x9
	.long	.LASF772
	.byte	0x10
	.byte	0xe
	.long	0x520
	.byte	0x1
	.byte	0x1
	.uleb128 0x12
	.byte	0x7
	.byte	0x4
	.long	0x9f
	.byte	0xa
	.byte	0x15
	.long	0x56a
	.uleb128 0x13
	.long	.LASF773
	.byte	0
	.uleb128 0x13
	.long	.LASF774
	.byte	0x1
	.uleb128 0x13
	.long	.LASF775
	.byte	0x2
	.uleb128 0x13
	.long	.LASF776
	.byte	0x3
	.uleb128 0x13
	.long	.LASF777
	.byte	0x4
	.uleb128 0x13
	.long	.LASF778
	.byte	0x5
	.byte	0
	.uleb128 0x7
	.long	.LASF779
	.byte	0xa
	.byte	0x1c
	.long	0x538
	.uleb128 0xb
	.byte	0x8
	.long	0x4a
	.uleb128 0x7
	.long	.LASF780
	.byte	0xa
	.byte	0x37
	.long	0x245
	.uleb128 0x12
	.byte	0x7
	.byte	0x4
	.long	0x9f
	.byte	0xc
	.byte	0x15
	.long	0x5a0
	.uleb128 0x13
	.long	.LASF781
	.byte	0
	.uleb128 0x13
	.long	.LASF782
	.byte	0x1
	.byte	0
	.uleb128 0x7
	.long	.LASF783
	.byte	0xc
	.byte	0x18
	.long	0x586
	.uleb128 0xf
	.long	0x4a
	.long	0x5bb
	.uleb128 0x14
	.long	0xb8
	.byte	0xff
	.byte	0
	.uleb128 0xf
	.long	0x5cb
	.long	0x5cb
	.uleb128 0x14
	.long	0xb8
	.byte	0x1f
	.byte	0
	.uleb128 0xb
	.byte	0x8
	.long	0x5d1
	.uleb128 0x15
	.long	.LASF784
	.byte	0x1
	.uleb128 0x15
	.long	.LASF785
	.byte	0x1
	.uleb128 0xb
	.byte	0x8
	.long	0x5d7
	.uleb128 0x7
	.long	.LASF786
	.byte	0xc
	.byte	0x3d
	.long	0x330
	.uleb128 0x9
	.long	.LASF787
	.byte	0xc
	.byte	0xa4
	.long	0x5e3
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF788
	.byte	0x11
	.byte	0x9
	.long	0x515
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF789
	.byte	0x11
	.byte	0xa
	.long	0x615
	.byte	0x1
	.byte	0x1
	.uleb128 0xb
	.byte	0x8
	.long	0x5e3
	.uleb128 0x9
	.long	.LASF790
	.byte	0x11
	.byte	0xb
	.long	0x628
	.byte	0x1
	.byte	0x1
	.uleb128 0xb
	.byte	0x8
	.long	0x57b
	.uleb128 0x9
	.long	.LASF791
	.byte	0x1
	.byte	0x8a
	.long	0xad
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF792
	.byte	0x1
	.byte	0xb3
	.long	0x5b
	.byte	0x1
	.byte	0x1
	.uleb128 0x16
	.byte	0x1
	.long	.LASF831
	.byte	0x3
	.byte	0x74
	.byte	0x1
	.long	0xa6
	.byte	0x1
	.long	0x666
	.uleb128 0x17
	.string	"mtx"
	.byte	0x3
	.byte	0x74
	.long	0x666
	.byte	0
	.uleb128 0xb
	.byte	0x8
	.long	0x300
	.uleb128 0x18
	.byte	0x1
	.long	.LASF832
	.byte	0x3
	.byte	0x6b
	.byte	0x1
	.long	0xa6
	.quad	.LFB14
	.quad	.LFE14
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0x6ac
	.uleb128 0x19
	.string	"mtx"
	.byte	0x3
	.byte	0x6b
	.long	0x666
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x1a
	.quad	.LVL43
	.long	0xd40
	.byte	0
	.uleb128 0x1b
	.byte	0x1
	.long	.LASF793
	.byte	0x3
	.byte	0x5a
	.byte	0x1
	.quad	.LFB13
	.quad	.LFE13
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0x82e
	.uleb128 0x19
	.string	"mtx"
	.byte	0x3
	.byte	0x5a
	.long	0x666
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x1c
	.long	.LASF795
	.long	0x83e
	.byte	0x1
	.byte	0x9
	.byte	0x3
	.quad	__func__.1891
	.uleb128 0x1d
	.quad	.LBB17
	.quad	.LBE17
	.long	0x7ae
	.uleb128 0x1e
	.long	.LASF796
	.byte	0x3
	.byte	0x5c
	.long	0x6d
	.byte	0x2
	.byte	0x91
	.sleb128 -17
	.uleb128 0x1a
	.quad	.LVL24
	.long	0xd4d
	.uleb128 0x1a
	.quad	.LVL25
	.long	0xd5a
	.uleb128 0x1a
	.quad	.LVL26
	.long	0xd67
	.uleb128 0x1a
	.quad	.LVL27
	.long	0xd74
	.uleb128 0x1a
	.quad	.LVL28
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL29
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL30
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL31
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL32
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL33
	.long	0xd8e
	.uleb128 0x1a
	.quad	.LVL34
	.long	0xd4d
	.uleb128 0x1a
	.quad	.LVL35
	.long	0xd5a
	.byte	0
	.uleb128 0x1f
	.long	0x648
	.quad	.LBB18
	.quad	.LBE18
	.byte	0x3
	.byte	0x61
	.long	0x7d2
	.uleb128 0x20
	.long	0x65a
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x1a
	.quad	.LVL36
	.long	0xd67
	.uleb128 0x1a
	.quad	.LVL37
	.long	0xd9b
	.uleb128 0x1a
	.quad	.LVL38
	.long	0xda8
	.uleb128 0x1a
	.quad	.LVL39
	.long	0xd9b
	.uleb128 0x1a
	.quad	.LVL40
	.long	0xdb5
	.uleb128 0x1a
	.quad	.LVL41
	.long	0xdc2
	.uleb128 0x1a
	.quad	.LVL42
	.long	0xd8e
	.byte	0
	.uleb128 0xf
	.long	0x56
	.long	0x83e
	.uleb128 0x14
	.long	0xb8
	.byte	0xd
	.byte	0
	.uleb128 0x6
	.long	0x82e
	.uleb128 0x1b
	.byte	0x1
	.long	.LASF794
	.byte	0x3
	.byte	0x3a
	.byte	0x1
	.quad	.LFB12
	.quad	.LFE12
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0x9f6
	.uleb128 0x19
	.string	"mtx"
	.byte	0x3
	.byte	0x3a
	.long	0x666
	.byte	0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x1c
	.long	.LASF795
	.long	0xa06
	.byte	0x1
	.byte	0x9
	.byte	0x3
	.quad	__func__.1886
	.uleb128 0x1d
	.quad	.LBB12
	.quad	.LBE12
	.long	0x945
	.uleb128 0x1e
	.long	.LASF796
	.byte	0x3
	.byte	0x3d
	.long	0x6d
	.byte	0x2
	.byte	0x91
	.sleb128 -17
	.uleb128 0x1a
	.quad	.LVL4
	.long	0xd4d
	.uleb128 0x1a
	.quad	.LVL5
	.long	0xd5a
	.uleb128 0x1a
	.quad	.LVL6
	.long	0xd67
	.uleb128 0x1a
	.quad	.LVL7
	.long	0xd74
	.uleb128 0x1a
	.quad	.LVL8
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL9
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL10
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL11
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL12
	.long	0xd81
	.uleb128 0x1a
	.quad	.LVL13
	.long	0xd8e
	.uleb128 0x1a
	.quad	.LVL14
	.long	0xd4d
	.uleb128 0x1a
	.quad	.LVL15
	.long	0xd5a
	.byte	0
	.uleb128 0x1f
	.long	0x648
	.quad	.LBB13
	.quad	.LBE13
	.byte	0x3
	.byte	0x40
	.long	0x969
	.uleb128 0x20
	.long	0x65a
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x1f
	.long	0x648
	.quad	.LBB15
	.quad	.LBE15
	.byte	0x3
	.byte	0x46
	.long	0x98d
	.uleb128 0x20
	.long	0x65a
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.byte	0
	.uleb128 0x1a
	.quad	.LVL16
	.long	0xd67
	.uleb128 0x1a
	.quad	.LVL17
	.long	0xd9b
	.uleb128 0x1a
	.quad	.LVL18
	.long	0xd9b
	.uleb128 0x1a
	.quad	.LVL19
	.long	0xa61
	.uleb128 0x1a
	.quad	.LVL20
	.long	0xdcf
	.uleb128 0x1a
	.quad	.LVL21
	.long	0xd9b
	.uleb128 0x1a
	.quad	.LVL22
	.long	0xdc2
	.uleb128 0x1a
	.quad	.LVL23
	.long	0xd8e
	.byte	0
	.uleb128 0xf
	.long	0x56
	.long	0xa06
	.uleb128 0x14
	.long	0xb8
	.byte	0xb
	.byte	0
	.uleb128 0x6
	.long	0x9f6
	.uleb128 0x1b
	.byte	0x1
	.long	.LASF797
	.byte	0x3
	.byte	0x2a
	.byte	0x1
	.quad	.LFB11
	.quad	.LFE11
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xa61
	.uleb128 0x19
	.string	"mtx"
	.byte	0x3
	.byte	0x2a
	.long	0x666
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x1a
	.quad	.LVL1
	.long	0xddc
	.uleb128 0x1a
	.quad	.LVL2
	.long	0xde9
	.uleb128 0x1a
	.quad	.LVL3
	.long	0xdf6
	.byte	0
	.uleb128 0x1b
	.byte	0x1
	.long	.LASF798
	.byte	0x3
	.byte	0x14
	.byte	0x1
	.quad	.LFB10
	.quad	.LFE10
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xb0d
	.uleb128 0x19
	.string	"mtx"
	.byte	0x3
	.byte	0x14
	.long	0x666
	.byte	0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x1c
	.long	.LASF795
	.long	0xb1d
	.byte	0x1
	.byte	0x9
	.byte	0x3
	.quad	__func__.1872
	.uleb128 0x21
	.quad	.LBB10
	.quad	.LBE10
	.uleb128 0x1e
	.long	.LASF799
	.byte	0x3
	.byte	0x17
	.long	0x666
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x1e
	.long	.LASF800
	.byte	0x3
	.byte	0x17
	.long	0x666
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x21
	.quad	.LBB11
	.quad	.LBE11
	.uleb128 0x1e
	.long	.LASF801
	.byte	0x3
	.byte	0x19
	.long	0x628
	.byte	0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x1e
	.long	.LASF802
	.byte	0x3
	.byte	0x19
	.long	0x628
	.byte	0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x1a
	.quad	.LVL0
	.long	0xd9b
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xf
	.long	0x56
	.long	0xb1d
	.uleb128 0x14
	.long	0xb8
	.byte	0x10
	.byte	0
	.uleb128 0x6
	.long	0xb0d
	.uleb128 0x22
	.long	.LASF803
	.byte	0x2
	.byte	0x62
	.quad	.LFB9
	.quad	.LFE9
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x22
	.long	.LASF804
	.byte	0x2
	.byte	0x5c
	.quad	.LFB8
	.quad	.LFE8
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x22
	.long	.LASF805
	.byte	0x2
	.byte	0x5a
	.quad	.LFB7
	.quad	.LFE7
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x23
	.long	.LASF833
	.byte	0x2
	.byte	0x52
	.long	0xad
	.quad	.LFB6
	.quad	.LFE6
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xba1
	.uleb128 0x1e
	.long	.LASF806
	.byte	0x2
	.byte	0x54
	.long	0xad
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x24
	.long	.LASF807
	.byte	0x1
	.value	0x125
	.byte	0x1
	.long	0xa6
	.quad	.LFB5
	.quad	.LFE5
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xbe0
	.uleb128 0x25
	.string	"l"
	.byte	0x1
	.value	0x125
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x25
	.string	"r"
	.byte	0x1
	.value	0x125
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x24
	.long	.LASF808
	.byte	0x1
	.value	0x123
	.byte	0x1
	.long	0xa6
	.quad	.LFB4
	.quad	.LFE4
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xc1f
	.uleb128 0x25
	.string	"l"
	.byte	0x1
	.value	0x123
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x25
	.string	"r"
	.byte	0x1
	.value	0x123
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x24
	.long	.LASF809
	.byte	0x1
	.value	0x121
	.byte	0x1
	.long	0xa6
	.quad	.LFB3
	.quad	.LFE3
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xc5e
	.uleb128 0x25
	.string	"l"
	.byte	0x1
	.value	0x121
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x25
	.string	"r"
	.byte	0x1
	.value	0x121
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x24
	.long	.LASF810
	.byte	0x1
	.value	0x11f
	.byte	0x1
	.long	0xa6
	.quad	.LFB2
	.quad	.LFE2
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xc9d
	.uleb128 0x25
	.string	"l"
	.byte	0x1
	.value	0x11f
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x25
	.string	"r"
	.byte	0x1
	.value	0x11f
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x24
	.long	.LASF811
	.byte	0x1
	.value	0x11d
	.byte	0x1
	.long	0xa6
	.quad	.LFB1
	.quad	.LFE1
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xcdc
	.uleb128 0x25
	.string	"l"
	.byte	0x1
	.value	0x11d
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x25
	.string	"r"
	.byte	0x1
	.value	0x11d
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x24
	.long	.LASF812
	.byte	0x1
	.value	0x118
	.byte	0x1
	.long	0xa6
	.quad	.LFB0
	.quad	.LFE0
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xd1b
	.uleb128 0x25
	.string	"l"
	.byte	0x1
	.value	0x118
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x25
	.string	"r"
	.byte	0x1
	.value	0x118
	.long	0xa6
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x26
	.long	0x648
	.quad	.LFB15
	.quad	.LFE15
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.long	0xd40
	.uleb128 0x20
	.long	0x65a
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF813
	.long	.LASF813
	.byte	0x8
	.byte	0x78
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF814
	.long	.LASF814
	.byte	0x12
	.byte	0x36
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF815
	.long	.LASF815
	.byte	0x2
	.byte	0x71
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF816
	.long	.LASF816
	.byte	0x4
	.byte	0x1c
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF817
	.long	.LASF817
	.byte	0x1
	.byte	0x9d
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF818
	.long	.LASF818
	.byte	0x1
	.byte	0x99
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF819
	.long	.LASF819
	.byte	0x4
	.byte	0x23
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF820
	.long	.LASF820
	.byte	0x1
	.byte	0xfe
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF821
	.long	.LASF821
	.byte	0x8
	.byte	0x5b
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF822
	.long	.LASF822
	.byte	0x7
	.byte	0x52
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF823
	.long	.LASF823
	.byte	0x7
	.byte	0x50
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF824
	.long	.LASF824
	.byte	0x8
	.byte	0x48
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF825
	.long	.LASF825
	.byte	0x8
	.byte	0x70
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF826
	.long	.LASF826
	.byte	0x4
	.byte	0x11
	.uleb128 0x27
	.byte	0x1
	.byte	0x1
	.long	.LASF827
	.long	.LASF827
	.byte	0x7
	.byte	0x42
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.uleb128 0x2119
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x35
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x13
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x34
	.uleb128 0xc
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x1f
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x20
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x21
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x22
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x23
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x24
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x25
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x26
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x27
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x2007
	.uleb128 0xe
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_macro,"",@progbits
.Ldebug_macro0:
	.value	0x4
	.byte	0x2
	.long	.Ldebug_line0
	.byte	0x7
	.long	.Ldebug_macro1
	.byte	0x3
	.uleb128 0
	.uleb128 0x3
	.byte	0x3
	.uleb128 0x3
	.uleb128 0x9
	.byte	0x3
	.uleb128 0x3
	.uleb128 0x8
	.byte	0x3
	.uleb128 0x3
	.uleb128 0x4
	.byte	0x5
	.uleb128 0x8
	.long	.LASF339
	.byte	0x4
	.byte	0x3
	.uleb128 0x4
	.uleb128 0x7
	.byte	0x3
	.uleb128 0x3
	.uleb128 0x6
	.byte	0x3
	.uleb128 0x3
	.uleb128 0x5
	.byte	0x7
	.long	.Ldebug_macro2
	.byte	0x4
	.byte	0x7
	.long	.Ldebug_macro3
	.byte	0x4
	.byte	0x7
	.long	.Ldebug_macro4
	.byte	0x4
	.byte	0x5
	.uleb128 0x19
	.long	.LASF361
	.byte	0x4
	.byte	0x5
	.uleb128 0x12
	.long	.LASF362
	.byte	0x4
	.byte	0x3
	.uleb128 0x4
	.uleb128 0x11
	.byte	0x3
	.uleb128 0x3
	.uleb128 0x10
	.file 19 "include/boot/config.h"
	.byte	0x3
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x7
	.long	.Ldebug_macro5
	.byte	0x4
	.file 20 "include/mm/page.h"
	.byte	0x3
	.uleb128 0x2
	.uleb128 0x14
	.byte	0x7
	.long	.Ldebug_macro6
	.byte	0x4
	.byte	0x3
	.uleb128 0x3
	.uleb128 0xf
	.byte	0x3
	.uleb128 0x3
	.uleb128 0xe
	.byte	0x3
	.uleb128 0x5
	.uleb128 0xd
	.byte	0x3
	.uleb128 0x4
	.uleb128 0xb
	.byte	0x7
	.long	.Ldebug_macro7
	.byte	0x4
	.byte	0x7
	.long	.Ldebug_macro8
	.byte	0x4
	.byte	0x4
	.byte	0x4
	.byte	0x5
	.uleb128 0xb
	.long	.LASF435
	.byte	0x4
	.byte	0x3
	.uleb128 0x4
	.uleb128 0xa
	.byte	0x4
	.byte	0x3
	.uleb128 0x5
	.uleb128 0xc
	.file 21 "include/config.h"
	.byte	0x3
	.uleb128 0x3
	.uleb128 0x15
	.byte	0x7
	.long	.Ldebug_macro9
	.byte	0x4
	.byte	0x7
	.long	.Ldebug_macro10
	.byte	0x4
	.byte	0x5
	.uleb128 0x7
	.long	.LASF453
	.byte	0x4
	.byte	0x3
	.uleb128 0x5
	.uleb128 0x2
	.byte	0x3
	.uleb128 0x5
	.uleb128 0x1
	.byte	0x3
	.uleb128 0x8
	.uleb128 0x12
	.byte	0x5
	.uleb128 0x10
	.long	.LASF454
	.byte	0x4
	.byte	0x7
	.long	.Ldebug_macro11
	.byte	0x4
	.byte	0x7
	.long	.Ldebug_macro12
	.byte	0x4
	.file 22 "include/errno.h"
	.byte	0x3
	.uleb128 0x6
	.uleb128 0x16
	.byte	0x7
	.long	.Ldebug_macro13
	.byte	0x4
	.byte	0x5
	.uleb128 0x13
	.long	.LASF672
	.byte	0x4
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.0.aae59430552d4c5275dc636173fed1ec,comdat
.Ldebug_macro1:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0
	.long	.LASF0
	.byte	0x5
	.uleb128 0
	.long	.LASF1
	.byte	0x5
	.uleb128 0
	.long	.LASF2
	.byte	0x5
	.uleb128 0
	.long	.LASF3
	.byte	0x5
	.uleb128 0
	.long	.LASF4
	.byte	0x5
	.uleb128 0
	.long	.LASF5
	.byte	0x5
	.uleb128 0
	.long	.LASF6
	.byte	0x5
	.uleb128 0
	.long	.LASF7
	.byte	0x5
	.uleb128 0
	.long	.LASF8
	.byte	0x5
	.uleb128 0
	.long	.LASF9
	.byte	0x5
	.uleb128 0
	.long	.LASF10
	.byte	0x5
	.uleb128 0
	.long	.LASF11
	.byte	0x5
	.uleb128 0
	.long	.LASF12
	.byte	0x5
	.uleb128 0
	.long	.LASF13
	.byte	0x5
	.uleb128 0
	.long	.LASF14
	.byte	0x5
	.uleb128 0
	.long	.LASF15
	.byte	0x5
	.uleb128 0
	.long	.LASF16
	.byte	0x5
	.uleb128 0
	.long	.LASF17
	.byte	0x5
	.uleb128 0
	.long	.LASF18
	.byte	0x5
	.uleb128 0
	.long	.LASF19
	.byte	0x5
	.uleb128 0
	.long	.LASF20
	.byte	0x5
	.uleb128 0
	.long	.LASF21
	.byte	0x5
	.uleb128 0
	.long	.LASF22
	.byte	0x5
	.uleb128 0
	.long	.LASF23
	.byte	0x5
	.uleb128 0
	.long	.LASF24
	.byte	0x5
	.uleb128 0
	.long	.LASF25
	.byte	0x5
	.uleb128 0
	.long	.LASF26
	.byte	0x5
	.uleb128 0
	.long	.LASF27
	.byte	0x5
	.uleb128 0
	.long	.LASF28
	.byte	0x5
	.uleb128 0
	.long	.LASF29
	.byte	0x5
	.uleb128 0
	.long	.LASF30
	.byte	0x5
	.uleb128 0
	.long	.LASF31
	.byte	0x5
	.uleb128 0
	.long	.LASF32
	.byte	0x5
	.uleb128 0
	.long	.LASF33
	.byte	0x5
	.uleb128 0
	.long	.LASF34
	.byte	0x5
	.uleb128 0
	.long	.LASF35
	.byte	0x5
	.uleb128 0
	.long	.LASF36
	.byte	0x5
	.uleb128 0
	.long	.LASF37
	.byte	0x5
	.uleb128 0
	.long	.LASF38
	.byte	0x5
	.uleb128 0
	.long	.LASF39
	.byte	0x5
	.uleb128 0
	.long	.LASF40
	.byte	0x5
	.uleb128 0
	.long	.LASF41
	.byte	0x5
	.uleb128 0
	.long	.LASF42
	.byte	0x5
	.uleb128 0
	.long	.LASF43
	.byte	0x5
	.uleb128 0
	.long	.LASF44
	.byte	0x5
	.uleb128 0
	.long	.LASF45
	.byte	0x5
	.uleb128 0
	.long	.LASF46
	.byte	0x5
	.uleb128 0
	.long	.LASF47
	.byte	0x5
	.uleb128 0
	.long	.LASF48
	.byte	0x5
	.uleb128 0
	.long	.LASF49
	.byte	0x5
	.uleb128 0
	.long	.LASF50
	.byte	0x5
	.uleb128 0
	.long	.LASF51
	.byte	0x5
	.uleb128 0
	.long	.LASF52
	.byte	0x5
	.uleb128 0
	.long	.LASF53
	.byte	0x5
	.uleb128 0
	.long	.LASF54
	.byte	0x5
	.uleb128 0
	.long	.LASF55
	.byte	0x5
	.uleb128 0
	.long	.LASF56
	.byte	0x5
	.uleb128 0
	.long	.LASF57
	.byte	0x5
	.uleb128 0
	.long	.LASF58
	.byte	0x5
	.uleb128 0
	.long	.LASF59
	.byte	0x5
	.uleb128 0
	.long	.LASF60
	.byte	0x5
	.uleb128 0
	.long	.LASF61
	.byte	0x5
	.uleb128 0
	.long	.LASF62
	.byte	0x5
	.uleb128 0
	.long	.LASF63
	.byte	0x5
	.uleb128 0
	.long	.LASF64
	.byte	0x5
	.uleb128 0
	.long	.LASF65
	.byte	0x5
	.uleb128 0
	.long	.LASF66
	.byte	0x5
	.uleb128 0
	.long	.LASF67
	.byte	0x5
	.uleb128 0
	.long	.LASF68
	.byte	0x5
	.uleb128 0
	.long	.LASF69
	.byte	0x5
	.uleb128 0
	.long	.LASF70
	.byte	0x5
	.uleb128 0
	.long	.LASF71
	.byte	0x5
	.uleb128 0
	.long	.LASF72
	.byte	0x5
	.uleb128 0
	.long	.LASF73
	.byte	0x5
	.uleb128 0
	.long	.LASF74
	.byte	0x5
	.uleb128 0
	.long	.LASF75
	.byte	0x5
	.uleb128 0
	.long	.LASF76
	.byte	0x5
	.uleb128 0
	.long	.LASF77
	.byte	0x5
	.uleb128 0
	.long	.LASF78
	.byte	0x5
	.uleb128 0
	.long	.LASF79
	.byte	0x5
	.uleb128 0
	.long	.LASF80
	.byte	0x5
	.uleb128 0
	.long	.LASF81
	.byte	0x5
	.uleb128 0
	.long	.LASF82
	.byte	0x5
	.uleb128 0
	.long	.LASF83
	.byte	0x5
	.uleb128 0
	.long	.LASF84
	.byte	0x5
	.uleb128 0
	.long	.LASF85
	.byte	0x5
	.uleb128 0
	.long	.LASF86
	.byte	0x5
	.uleb128 0
	.long	.LASF87
	.byte	0x5
	.uleb128 0
	.long	.LASF88
	.byte	0x5
	.uleb128 0
	.long	.LASF89
	.byte	0x5
	.uleb128 0
	.long	.LASF90
	.byte	0x5
	.uleb128 0
	.long	.LASF91
	.byte	0x5
	.uleb128 0
	.long	.LASF92
	.byte	0x5
	.uleb128 0
	.long	.LASF93
	.byte	0x5
	.uleb128 0
	.long	.LASF94
	.byte	0x5
	.uleb128 0
	.long	.LASF95
	.byte	0x5
	.uleb128 0
	.long	.LASF96
	.byte	0x5
	.uleb128 0
	.long	.LASF97
	.byte	0x5
	.uleb128 0
	.long	.LASF98
	.byte	0x5
	.uleb128 0
	.long	.LASF99
	.byte	0x5
	.uleb128 0
	.long	.LASF100
	.byte	0x5
	.uleb128 0
	.long	.LASF101
	.byte	0x5
	.uleb128 0
	.long	.LASF102
	.byte	0x5
	.uleb128 0
	.long	.LASF103
	.byte	0x5
	.uleb128 0
	.long	.LASF104
	.byte	0x5
	.uleb128 0
	.long	.LASF105
	.byte	0x5
	.uleb128 0
	.long	.LASF106
	.byte	0x5
	.uleb128 0
	.long	.LASF107
	.byte	0x5
	.uleb128 0
	.long	.LASF108
	.byte	0x5
	.uleb128 0
	.long	.LASF109
	.byte	0x5
	.uleb128 0
	.long	.LASF110
	.byte	0x5
	.uleb128 0
	.long	.LASF111
	.byte	0x5
	.uleb128 0
	.long	.LASF112
	.byte	0x5
	.uleb128 0
	.long	.LASF113
	.byte	0x5
	.uleb128 0
	.long	.LASF114
	.byte	0x5
	.uleb128 0
	.long	.LASF115
	.byte	0x5
	.uleb128 0
	.long	.LASF116
	.byte	0x5
	.uleb128 0
	.long	.LASF117
	.byte	0x5
	.uleb128 0
	.long	.LASF118
	.byte	0x5
	.uleb128 0
	.long	.LASF119
	.byte	0x5
	.uleb128 0
	.long	.LASF120
	.byte	0x5
	.uleb128 0
	.long	.LASF121
	.byte	0x5
	.uleb128 0
	.long	.LASF122
	.byte	0x5
	.uleb128 0
	.long	.LASF123
	.byte	0x5
	.uleb128 0
	.long	.LASF124
	.byte	0x5
	.uleb128 0
	.long	.LASF125
	.byte	0x5
	.uleb128 0
	.long	.LASF126
	.byte	0x5
	.uleb128 0
	.long	.LASF127
	.byte	0x5
	.uleb128 0
	.long	.LASF128
	.byte	0x5
	.uleb128 0
	.long	.LASF129
	.byte	0x5
	.uleb128 0
	.long	.LASF130
	.byte	0x5
	.uleb128 0
	.long	.LASF131
	.byte	0x5
	.uleb128 0
	.long	.LASF132
	.byte	0x5
	.uleb128 0
	.long	.LASF133
	.byte	0x5
	.uleb128 0
	.long	.LASF134
	.byte	0x5
	.uleb128 0
	.long	.LASF135
	.byte	0x5
	.uleb128 0
	.long	.LASF136
	.byte	0x5
	.uleb128 0
	.long	.LASF137
	.byte	0x5
	.uleb128 0
	.long	.LASF138
	.byte	0x5
	.uleb128 0
	.long	.LASF139
	.byte	0x5
	.uleb128 0
	.long	.LASF140
	.byte	0x5
	.uleb128 0
	.long	.LASF141
	.byte	0x5
	.uleb128 0
	.long	.LASF142
	.byte	0x5
	.uleb128 0
	.long	.LASF143
	.byte	0x5
	.uleb128 0
	.long	.LASF144
	.byte	0x5
	.uleb128 0
	.long	.LASF145
	.byte	0x5
	.uleb128 0
	.long	.LASF146
	.byte	0x5
	.uleb128 0
	.long	.LASF147
	.byte	0x5
	.uleb128 0
	.long	.LASF148
	.byte	0x5
	.uleb128 0
	.long	.LASF149
	.byte	0x5
	.uleb128 0
	.long	.LASF150
	.byte	0x5
	.uleb128 0
	.long	.LASF151
	.byte	0x5
	.uleb128 0
	.long	.LASF152
	.byte	0x5
	.uleb128 0
	.long	.LASF153
	.byte	0x5
	.uleb128 0
	.long	.LASF154
	.byte	0x5
	.uleb128 0
	.long	.LASF155
	.byte	0x5
	.uleb128 0
	.long	.LASF156
	.byte	0x5
	.uleb128 0
	.long	.LASF157
	.byte	0x5
	.uleb128 0
	.long	.LASF158
	.byte	0x5
	.uleb128 0
	.long	.LASF159
	.byte	0x5
	.uleb128 0
	.long	.LASF160
	.byte	0x5
	.uleb128 0
	.long	.LASF161
	.byte	0x5
	.uleb128 0
	.long	.LASF162
	.byte	0x5
	.uleb128 0
	.long	.LASF163
	.byte	0x5
	.uleb128 0
	.long	.LASF164
	.byte	0x5
	.uleb128 0
	.long	.LASF165
	.byte	0x5
	.uleb128 0
	.long	.LASF166
	.byte	0x5
	.uleb128 0
	.long	.LASF167
	.byte	0x5
	.uleb128 0
	.long	.LASF168
	.byte	0x5
	.uleb128 0
	.long	.LASF169
	.byte	0x5
	.uleb128 0
	.long	.LASF170
	.byte	0x5
	.uleb128 0
	.long	.LASF171
	.byte	0x5
	.uleb128 0
	.long	.LASF172
	.byte	0x5
	.uleb128 0
	.long	.LASF173
	.byte	0x5
	.uleb128 0
	.long	.LASF174
	.byte	0x5
	.uleb128 0
	.long	.LASF175
	.byte	0x5
	.uleb128 0
	.long	.LASF176
	.byte	0x5
	.uleb128 0
	.long	.LASF177
	.byte	0x5
	.uleb128 0
	.long	.LASF178
	.byte	0x5
	.uleb128 0
	.long	.LASF179
	.byte	0x5
	.uleb128 0
	.long	.LASF180
	.byte	0x5
	.uleb128 0
	.long	.LASF181
	.byte	0x5
	.uleb128 0
	.long	.LASF182
	.byte	0x5
	.uleb128 0
	.long	.LASF183
	.byte	0x5
	.uleb128 0
	.long	.LASF184
	.byte	0x5
	.uleb128 0
	.long	.LASF185
	.byte	0x5
	.uleb128 0
	.long	.LASF186
	.byte	0x5
	.uleb128 0
	.long	.LASF187
	.byte	0x5
	.uleb128 0
	.long	.LASF188
	.byte	0x5
	.uleb128 0
	.long	.LASF189
	.byte	0x5
	.uleb128 0
	.long	.LASF190
	.byte	0x5
	.uleb128 0
	.long	.LASF191
	.byte	0x5
	.uleb128 0
	.long	.LASF192
	.byte	0x5
	.uleb128 0
	.long	.LASF193
	.byte	0x5
	.uleb128 0
	.long	.LASF194
	.byte	0x5
	.uleb128 0
	.long	.LASF195
	.byte	0x5
	.uleb128 0
	.long	.LASF196
	.byte	0x5
	.uleb128 0
	.long	.LASF197
	.byte	0x5
	.uleb128 0
	.long	.LASF198
	.byte	0x5
	.uleb128 0
	.long	.LASF199
	.byte	0x5
	.uleb128 0
	.long	.LASF200
	.byte	0x5
	.uleb128 0
	.long	.LASF201
	.byte	0x5
	.uleb128 0
	.long	.LASF202
	.byte	0x5
	.uleb128 0
	.long	.LASF203
	.byte	0x5
	.uleb128 0
	.long	.LASF204
	.byte	0x5
	.uleb128 0
	.long	.LASF205
	.byte	0x5
	.uleb128 0
	.long	.LASF206
	.byte	0x5
	.uleb128 0
	.long	.LASF207
	.byte	0x5
	.uleb128 0
	.long	.LASF208
	.byte	0x5
	.uleb128 0
	.long	.LASF209
	.byte	0x5
	.uleb128 0
	.long	.LASF210
	.byte	0x5
	.uleb128 0
	.long	.LASF211
	.byte	0x5
	.uleb128 0
	.long	.LASF212
	.byte	0x5
	.uleb128 0
	.long	.LASF213
	.byte	0x5
	.uleb128 0
	.long	.LASF214
	.byte	0x5
	.uleb128 0
	.long	.LASF215
	.byte	0x5
	.uleb128 0
	.long	.LASF216
	.byte	0x5
	.uleb128 0
	.long	.LASF217
	.byte	0x5
	.uleb128 0
	.long	.LASF218
	.byte	0x5
	.uleb128 0
	.long	.LASF219
	.byte	0x5
	.uleb128 0
	.long	.LASF220
	.byte	0x5
	.uleb128 0
	.long	.LASF221
	.byte	0x5
	.uleb128 0
	.long	.LASF222
	.byte	0x5
	.uleb128 0
	.long	.LASF223
	.byte	0x5
	.uleb128 0
	.long	.LASF224
	.byte	0x5
	.uleb128 0
	.long	.LASF225
	.byte	0x5
	.uleb128 0
	.long	.LASF226
	.byte	0x5
	.uleb128 0
	.long	.LASF227
	.byte	0x5
	.uleb128 0
	.long	.LASF228
	.byte	0x5
	.uleb128 0
	.long	.LASF229
	.byte	0x5
	.uleb128 0
	.long	.LASF230
	.byte	0x5
	.uleb128 0
	.long	.LASF231
	.byte	0x5
	.uleb128 0
	.long	.LASF232
	.byte	0x5
	.uleb128 0
	.long	.LASF233
	.byte	0x5
	.uleb128 0
	.long	.LASF234
	.byte	0x5
	.uleb128 0
	.long	.LASF235
	.byte	0x5
	.uleb128 0
	.long	.LASF236
	.byte	0x5
	.uleb128 0
	.long	.LASF237
	.byte	0x5
	.uleb128 0
	.long	.LASF238
	.byte	0x5
	.uleb128 0
	.long	.LASF239
	.byte	0x5
	.uleb128 0
	.long	.LASF240
	.byte	0x5
	.uleb128 0
	.long	.LASF241
	.byte	0x5
	.uleb128 0
	.long	.LASF242
	.byte	0x5
	.uleb128 0
	.long	.LASF243
	.byte	0x5
	.uleb128 0
	.long	.LASF244
	.byte	0x5
	.uleb128 0
	.long	.LASF245
	.byte	0x5
	.uleb128 0
	.long	.LASF246
	.byte	0x5
	.uleb128 0
	.long	.LASF247
	.byte	0x5
	.uleb128 0
	.long	.LASF248
	.byte	0x5
	.uleb128 0
	.long	.LASF249
	.byte	0x5
	.uleb128 0
	.long	.LASF250
	.byte	0x5
	.uleb128 0
	.long	.LASF251
	.byte	0x5
	.uleb128 0
	.long	.LASF252
	.byte	0x5
	.uleb128 0
	.long	.LASF253
	.byte	0x5
	.uleb128 0
	.long	.LASF254
	.byte	0x5
	.uleb128 0
	.long	.LASF255
	.byte	0x5
	.uleb128 0
	.long	.LASF256
	.byte	0x5
	.uleb128 0
	.long	.LASF257
	.byte	0x5
	.uleb128 0
	.long	.LASF258
	.byte	0x5
	.uleb128 0
	.long	.LASF259
	.byte	0x5
	.uleb128 0
	.long	.LASF260
	.byte	0x5
	.uleb128 0
	.long	.LASF261
	.byte	0x5
	.uleb128 0
	.long	.LASF262
	.byte	0x5
	.uleb128 0
	.long	.LASF263
	.byte	0x5
	.uleb128 0
	.long	.LASF264
	.byte	0x5
	.uleb128 0
	.long	.LASF265
	.byte	0x5
	.uleb128 0
	.long	.LASF266
	.byte	0x5
	.uleb128 0
	.long	.LASF267
	.byte	0x5
	.uleb128 0
	.long	.LASF268
	.byte	0x5
	.uleb128 0
	.long	.LASF269
	.byte	0x5
	.uleb128 0
	.long	.LASF270
	.byte	0x5
	.uleb128 0
	.long	.LASF271
	.byte	0x5
	.uleb128 0
	.long	.LASF272
	.byte	0x5
	.uleb128 0
	.long	.LASF273
	.byte	0x5
	.uleb128 0
	.long	.LASF274
	.byte	0x5
	.uleb128 0
	.long	.LASF275
	.byte	0x5
	.uleb128 0
	.long	.LASF276
	.byte	0x5
	.uleb128 0
	.long	.LASF277
	.byte	0x5
	.uleb128 0
	.long	.LASF278
	.byte	0x5
	.uleb128 0
	.long	.LASF279
	.byte	0x5
	.uleb128 0
	.long	.LASF280
	.byte	0x5
	.uleb128 0
	.long	.LASF281
	.byte	0x5
	.uleb128 0
	.long	.LASF282
	.byte	0x5
	.uleb128 0
	.long	.LASF283
	.byte	0x5
	.uleb128 0
	.long	.LASF284
	.byte	0x5
	.uleb128 0
	.long	.LASF285
	.byte	0x5
	.uleb128 0
	.long	.LASF286
	.byte	0x5
	.uleb128 0
	.long	.LASF287
	.byte	0x5
	.uleb128 0
	.long	.LASF288
	.byte	0x5
	.uleb128 0
	.long	.LASF289
	.byte	0x5
	.uleb128 0
	.long	.LASF290
	.byte	0x5
	.uleb128 0
	.long	.LASF291
	.byte	0x5
	.uleb128 0
	.long	.LASF292
	.byte	0x5
	.uleb128 0
	.long	.LASF293
	.byte	0x5
	.uleb128 0
	.long	.LASF294
	.byte	0x5
	.uleb128 0
	.long	.LASF295
	.byte	0x5
	.uleb128 0
	.long	.LASF296
	.byte	0x5
	.uleb128 0
	.long	.LASF297
	.byte	0x5
	.uleb128 0
	.long	.LASF298
	.byte	0x5
	.uleb128 0
	.long	.LASF299
	.byte	0x5
	.uleb128 0
	.long	.LASF300
	.byte	0x5
	.uleb128 0
	.long	.LASF301
	.byte	0x5
	.uleb128 0
	.long	.LASF302
	.byte	0x5
	.uleb128 0
	.long	.LASF303
	.byte	0x5
	.uleb128 0
	.long	.LASF304
	.byte	0x5
	.uleb128 0
	.long	.LASF305
	.byte	0x5
	.uleb128 0
	.long	.LASF306
	.byte	0x5
	.uleb128 0
	.long	.LASF307
	.byte	0x5
	.uleb128 0
	.long	.LASF308
	.byte	0x5
	.uleb128 0
	.long	.LASF309
	.byte	0x5
	.uleb128 0
	.long	.LASF310
	.byte	0x5
	.uleb128 0
	.long	.LASF311
	.byte	0x5
	.uleb128 0
	.long	.LASF312
	.byte	0x5
	.uleb128 0
	.long	.LASF313
	.byte	0x5
	.uleb128 0
	.long	.LASF314
	.byte	0x5
	.uleb128 0
	.long	.LASF315
	.byte	0x5
	.uleb128 0
	.long	.LASF316
	.byte	0x5
	.uleb128 0
	.long	.LASF317
	.byte	0x5
	.uleb128 0
	.long	.LASF318
	.byte	0x5
	.uleb128 0
	.long	.LASF319
	.byte	0x5
	.uleb128 0
	.long	.LASF320
	.byte	0x5
	.uleb128 0
	.long	.LASF321
	.byte	0x5
	.uleb128 0
	.long	.LASF322
	.byte	0x5
	.uleb128 0
	.long	.LASF323
	.byte	0x5
	.uleb128 0
	.long	.LASF324
	.byte	0x5
	.uleb128 0
	.long	.LASF325
	.byte	0x5
	.uleb128 0
	.long	.LASF326
	.byte	0x5
	.uleb128 0
	.long	.LASF327
	.byte	0x5
	.uleb128 0
	.long	.LASF328
	.byte	0x5
	.uleb128 0
	.long	.LASF329
	.byte	0x5
	.uleb128 0
	.long	.LASF330
	.byte	0x5
	.uleb128 0
	.long	.LASF331
	.byte	0x5
	.uleb128 0
	.long	.LASF332
	.byte	0x5
	.uleb128 0
	.long	.LASF333
	.byte	0x5
	.uleb128 0
	.long	.LASF334
	.byte	0x5
	.uleb128 0
	.long	.LASF335
	.byte	0x5
	.uleb128 0
	.long	.LASF336
	.byte	0x5
	.uleb128 0
	.long	.LASF337
	.byte	0x5
	.uleb128 0
	.long	.LASF338
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.types.h.5.ce5cf6e0f5e04c88197477365e766aac,comdat
.Ldebug_macro2:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x5
	.long	.LASF340
	.byte	0x5
	.uleb128 0x7
	.long	.LASF341
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.kernel.h.27.55537b03585a581058170c4d492e9fbd,comdat
.Ldebug_macro3:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x1b
	.long	.LASF342
	.byte	0x5
	.uleb128 0x1c
	.long	.LASF343
	.byte	0x5
	.uleb128 0x1e
	.long	.LASF344
	.byte	0x5
	.uleb128 0x21
	.long	.LASF345
	.byte	0x5
	.uleb128 0x26
	.long	.LASF346
	.byte	0x5
	.uleb128 0x29
	.long	.LASF347
	.byte	0x5
	.uleb128 0x2c
	.long	.LASF348
	.byte	0x5
	.uleb128 0x37
	.long	.LASF349
	.byte	0x5
	.uleb128 0x38
	.long	.LASF350
	.byte	0x5
	.uleb128 0x3a
	.long	.LASF351
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.list.h.51.d60d187a0f884830a7253eca159119da,comdat
.Ldebug_macro4:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x33
	.long	.LASF352
	.byte	0x5
	.uleb128 0x37
	.long	.LASF353
	.byte	0x5
	.uleb128 0x54
	.long	.LASF354
	.byte	0x5
	.uleb128 0x57
	.long	.LASF355
	.byte	0x5
	.uleb128 0x59
	.long	.LASF356
	.byte	0x5
	.uleb128 0x5b
	.long	.LASF357
	.byte	0x5
	.uleb128 0x5e
	.long	.LASF358
	.byte	0x5
	.uleb128 0x60
	.long	.LASF359
	.byte	0x5
	.uleb128 0x66
	.long	.LASF360
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.config.h.3.5098c18d6f25962e3a31ca1ba399ffa3,comdat
.Ldebug_macro5:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x3
	.long	.LASF363
	.byte	0x5
	.uleb128 0x5
	.long	.LASF364
	.byte	0x5
	.uleb128 0x6
	.long	.LASF365
	.byte	0x5
	.uleb128 0x7
	.long	.LASF366
	.byte	0x5
	.uleb128 0xa
	.long	.LASF367
	.byte	0x5
	.uleb128 0xc
	.long	.LASF368
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.page.h.16.ce77fc105ad3d118d91491723596dd47,comdat
.Ldebug_macro6:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x10
	.long	.LASF369
	.byte	0x5
	.uleb128 0x11
	.long	.LASF370
	.byte	0x5
	.uleb128 0x12
	.long	.LASF371
	.byte	0x5
	.uleb128 0x14
	.long	.LASF372
	.byte	0x5
	.uleb128 0x15
	.long	.LASF373
	.byte	0x5
	.uleb128 0x18
	.long	.LASF374
	.byte	0x5
	.uleb128 0x19
	.long	.LASF375
	.byte	0x5
	.uleb128 0x1b
	.long	.LASF376
	.byte	0x5
	.uleb128 0x1c
	.long	.LASF377
	.byte	0x5
	.uleb128 0x1e
	.long	.LASF378
	.byte	0x5
	.uleb128 0x20
	.long	.LASF379
	.byte	0x5
	.uleb128 0x22
	.long	.LASF380
	.byte	0x5
	.uleb128 0x23
	.long	.LASF381
	.byte	0x5
	.uleb128 0x25
	.long	.LASF382
	.byte	0x5
	.uleb128 0x26
	.long	.LASF383
	.byte	0x5
	.uleb128 0x27
	.long	.LASF384
	.byte	0x5
	.uleb128 0x28
	.long	.LASF385
	.byte	0x5
	.uleb128 0x29
	.long	.LASF386
	.byte	0x5
	.uleb128 0x2a
	.long	.LASF387
	.byte	0x5
	.uleb128 0x2b
	.long	.LASF388
	.byte	0x5
	.uleb128 0x2c
	.long	.LASF389
	.byte	0x5
	.uleb128 0x2e
	.long	.LASF390
	.byte	0x5
	.uleb128 0x2f
	.long	.LASF391
	.byte	0x5
	.uleb128 0x30
	.long	.LASF392
	.byte	0x5
	.uleb128 0x31
	.long	.LASF393
	.byte	0x5
	.uleb128 0x32
	.long	.LASF394
	.byte	0x5
	.uleb128 0x33
	.long	.LASF395
	.byte	0x5
	.uleb128 0x34
	.long	.LASF396
	.byte	0x5
	.uleb128 0x35
	.long	.LASF397
	.byte	0x5
	.uleb128 0x37
	.long	.LASF398
	.byte	0x5
	.uleb128 0x38
	.long	.LASF399
	.byte	0x5
	.uleb128 0x39
	.long	.LASF400
	.byte	0x5
	.uleb128 0x3a
	.long	.LASF401
	.byte	0x5
	.uleb128 0x3b
	.long	.LASF402
	.byte	0x5
	.uleb128 0x3d
	.long	.LASF403
	.byte	0x5
	.uleb128 0x40
	.long	.LASF404
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.vmmap.h.7.19545d496afbad2f9659dd15270e0d7a,comdat
.Ldebug_macro7:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x7
	.long	.LASF405
	.byte	0x5
	.uleb128 0x8
	.long	.LASF406
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.pagetable.h.6.22608c150ad8426775e8969d2a213369,comdat
.Ldebug_macro8:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x6
	.long	.LASF407
	.byte	0x5
	.uleb128 0x7
	.long	.LASF408
	.byte	0x5
	.uleb128 0x8
	.long	.LASF409
	.byte	0x5
	.uleb128 0x9
	.long	.LASF410
	.byte	0x5
	.uleb128 0xa
	.long	.LASF411
	.byte	0x5
	.uleb128 0xb
	.long	.LASF412
	.byte	0x5
	.uleb128 0xc
	.long	.LASF413
	.byte	0x5
	.uleb128 0xd
	.long	.LASF414
	.byte	0x5
	.uleb128 0xe
	.long	.LASF415
	.byte	0x5
	.uleb128 0x10
	.long	.LASF416
	.byte	0x5
	.uleb128 0x1d
	.long	.LASF417
	.byte	0x5
	.uleb128 0x1e
	.long	.LASF418
	.byte	0x5
	.uleb128 0x1f
	.long	.LASF419
	.byte	0x5
	.uleb128 0x20
	.long	.LASF420
	.byte	0x5
	.uleb128 0x21
	.long	.LASF421
	.byte	0x5
	.uleb128 0x23
	.long	.LASF416
	.byte	0x5
	.uleb128 0x24
	.long	.LASF422
	.byte	0x5
	.uleb128 0x25
	.long	.LASF423
	.byte	0x5
	.uleb128 0x26
	.long	.LASF424
	.byte	0x5
	.uleb128 0x28
	.long	.LASF425
	.byte	0x5
	.uleb128 0x2c
	.long	.LASF426
	.byte	0x5
	.uleb128 0x2d
	.long	.LASF427
	.byte	0x5
	.uleb128 0x2e
	.long	.LASF428
	.byte	0x5
	.uleb128 0x30
	.long	.LASF429
	.byte	0x5
	.uleb128 0x31
	.long	.LASF430
	.byte	0x5
	.uleb128 0x32
	.long	.LASF431
	.byte	0x5
	.uleb128 0x33
	.long	.LASF432
	.byte	0x5
	.uleb128 0x35
	.long	.LASF433
	.byte	0x5
	.uleb128 0x3f
	.long	.LASF434
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.config.h.14.6e9669dd007d0bcb4308c0e82b88cc6e,comdat
.Ldebug_macro9:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0xe
	.long	.LASF436
	.byte	0x5
	.uleb128 0xf
	.long	.LASF437
	.byte	0x5
	.uleb128 0x10
	.long	.LASF438
	.byte	0x5
	.uleb128 0x1a
	.long	.LASF439
	.byte	0x5
	.uleb128 0x1d
	.long	.LASF440
	.byte	0x5
	.uleb128 0x23
	.long	.LASF441
	.byte	0x5
	.uleb128 0x24
	.long	.LASF442
	.byte	0x5
	.uleb128 0x25
	.long	.LASF443
	.byte	0x5
	.uleb128 0x26
	.long	.LASF444
	.byte	0x5
	.uleb128 0x27
	.long	.LASF445
	.byte	0x5
	.uleb128 0x28
	.long	.LASF446
	.byte	0x5
	.uleb128 0x2b
	.long	.LASF447
	.byte	0x5
	.uleb128 0x31
	.long	.LASF448
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.proc.h.16.4f7df0793aadad62f2750a3ba03cf602,comdat
.Ldebug_macro10:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x10
	.long	.LASF449
	.byte	0x5
	.uleb128 0x11
	.long	.LASF450
	.byte	0x5
	.uleb128 0x9e
	.long	.LASF451
	.byte	0x5
	.uleb128 0x9f
	.long	.LASF452
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.debug.h.29.9c74112c6079abaf2a9ef01af88e6488,comdat
.Ldebug_macro11:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x1d
	.long	.LASF455
	.byte	0x5
	.uleb128 0x1e
	.long	.LASF456
	.byte	0x5
	.uleb128 0x1f
	.long	.LASF457
	.byte	0x5
	.uleb128 0x20
	.long	.LASF458
	.byte	0x5
	.uleb128 0x21
	.long	.LASF459
	.byte	0x5
	.uleb128 0x22
	.long	.LASF460
	.byte	0x5
	.uleb128 0x23
	.long	.LASF461
	.byte	0x5
	.uleb128 0x24
	.long	.LASF462
	.byte	0x5
	.uleb128 0x25
	.long	.LASF463
	.byte	0x5
	.uleb128 0x27
	.long	.LASF464
	.byte	0x5
	.uleb128 0x28
	.long	.LASF465
	.byte	0x5
	.uleb128 0x29
	.long	.LASF466
	.byte	0x5
	.uleb128 0x2a
	.long	.LASF467
	.byte	0x5
	.uleb128 0x2b
	.long	.LASF468
	.byte	0x5
	.uleb128 0x2c
	.long	.LASF469
	.byte	0x5
	.uleb128 0x2d
	.long	.LASF470
	.byte	0x5
	.uleb128 0x2f
	.long	.LASF471
	.byte	0x5
	.uleb128 0x34
	.long	.LASF472
	.byte	0x5
	.uleb128 0x35
	.long	.LASF473
	.byte	0x5
	.uleb128 0x36
	.long	.LASF474
	.byte	0x5
	.uleb128 0x37
	.long	.LASF475
	.byte	0x5
	.uleb128 0x38
	.long	.LASF476
	.byte	0x5
	.uleb128 0x39
	.long	.LASF477
	.byte	0x5
	.uleb128 0x3a
	.long	.LASF478
	.byte	0x5
	.uleb128 0x3b
	.long	.LASF479
	.byte	0x5
	.uleb128 0x3c
	.long	.LASF480
	.byte	0x5
	.uleb128 0x3d
	.long	.LASF481
	.byte	0x5
	.uleb128 0x3e
	.long	.LASF482
	.byte	0x5
	.uleb128 0x3f
	.long	.LASF483
	.byte	0x5
	.uleb128 0x40
	.long	.LASF484
	.byte	0x5
	.uleb128 0x41
	.long	.LASF485
	.byte	0x5
	.uleb128 0x42
	.long	.LASF486
	.byte	0x5
	.uleb128 0x43
	.long	.LASF487
	.byte	0x5
	.uleb128 0x44
	.long	.LASF488
	.byte	0x5
	.uleb128 0x45
	.long	.LASF489
	.byte	0x5
	.uleb128 0x46
	.long	.LASF490
	.byte	0x5
	.uleb128 0x47
	.long	.LASF491
	.byte	0x5
	.uleb128 0x48
	.long	.LASF492
	.byte	0x5
	.uleb128 0x49
	.long	.LASF493
	.byte	0x5
	.uleb128 0x4a
	.long	.LASF494
	.byte	0x5
	.uleb128 0x4b
	.long	.LASF495
	.byte	0x5
	.uleb128 0x4c
	.long	.LASF496
	.byte	0x5
	.uleb128 0x4d
	.long	.LASF497
	.byte	0x5
	.uleb128 0x4e
	.long	.LASF498
	.byte	0x5
	.uleb128 0x4f
	.long	.LASF499
	.byte	0x5
	.uleb128 0x50
	.long	.LASF500
	.byte	0x5
	.uleb128 0x51
	.long	.LASF501
	.byte	0x5
	.uleb128 0x52
	.long	.LASF502
	.byte	0x5
	.uleb128 0x54
	.long	.LASF503
	.byte	0x5
	.uleb128 0x55
	.long	.LASF504
	.byte	0x5
	.uleb128 0x56
	.long	.LASF505
	.byte	0x5
	.uleb128 0x57
	.long	.LASF506
	.byte	0x5
	.uleb128 0x58
	.long	.LASF507
	.byte	0x5
	.uleb128 0x59
	.long	.LASF508
	.byte	0x5
	.uleb128 0x63
	.long	.LASF509
	.byte	0x5
	.uleb128 0x95
	.long	.LASF510
	.byte	0x5
	.uleb128 0xa0
	.long	.LASF511
	.byte	0x5
	.uleb128 0xa3
	.long	.LASF512
	.byte	0x5
	.uleb128 0xb4
	.long	.LASF513
	.byte	0x5
	.uleb128 0xc4
	.long	.LASF514
	.byte	0x5
	.uleb128 0xd1
	.long	.LASF515
	.byte	0x5
	.uleb128 0xe0
	.long	.LASF516
	.byte	0x5
	.uleb128 0xef
	.long	.LASF517
	.byte	0x5
	.uleb128 0x102
	.long	.LASF518
	.byte	0x5
	.uleb128 0x105
	.long	.LASF519
	.byte	0x5
	.uleb128 0x10c
	.long	.LASF520
	.byte	0x5
	.uleb128 0x127
	.long	.LASF521
	.byte	0x5
	.uleb128 0x128
	.long	.LASF522
	.byte	0x5
	.uleb128 0x129
	.long	.LASF523
	.byte	0x5
	.uleb128 0x12a
	.long	.LASF524
	.byte	0x5
	.uleb128 0x12b
	.long	.LASF525
	.byte	0x5
	.uleb128 0x12c
	.long	.LASF526
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.interrupt.h.9.fa9467ed2d6ed6c1217b73543b7dbe5c,comdat
.Ldebug_macro12:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x9
	.long	.LASF527
	.byte	0x5
	.uleb128 0xa
	.long	.LASF528
	.byte	0x5
	.uleb128 0xb
	.long	.LASF529
	.byte	0x5
	.uleb128 0xc
	.long	.LASF530
	.byte	0x5
	.uleb128 0xe
	.long	.LASF531
	.byte	0x5
	.uleb128 0xf
	.long	.LASF532
	.byte	0x5
	.uleb128 0x11
	.long	.LASF533
	.byte	0x5
	.uleb128 0x12
	.long	.LASF534
	.byte	0x5
	.uleb128 0x13
	.long	.LASF535
	.byte	0x5
	.uleb128 0x14
	.long	.LASF536
	.byte	0x5
	.uleb128 0x1a
	.long	.LASF537
	.byte	0x5
	.uleb128 0x1d
	.long	.LASF538
	.byte	0x5
	.uleb128 0x1e
	.long	.LASF539
	.byte	0
	.section	.debug_macro,"G",@progbits,wm4.errno.h.12.8e602d82239fc14b356bf7ba5b1f7039,comdat
.Ldebug_macro13:
	.value	0x4
	.byte	0
	.byte	0x5
	.uleb128 0xc
	.long	.LASF540
	.byte	0x5
	.uleb128 0xd
	.long	.LASF541
	.byte	0x5
	.uleb128 0xe
	.long	.LASF542
	.byte	0x5
	.uleb128 0xf
	.long	.LASF543
	.byte	0x5
	.uleb128 0x10
	.long	.LASF544
	.byte	0x5
	.uleb128 0x11
	.long	.LASF545
	.byte	0x5
	.uleb128 0x12
	.long	.LASF546
	.byte	0x5
	.uleb128 0x13
	.long	.LASF547
	.byte	0x5
	.uleb128 0x14
	.long	.LASF548
	.byte	0x5
	.uleb128 0x15
	.long	.LASF549
	.byte	0x5
	.uleb128 0x16
	.long	.LASF550
	.byte	0x5
	.uleb128 0x17
	.long	.LASF551
	.byte	0x5
	.uleb128 0x18
	.long	.LASF552
	.byte	0x5
	.uleb128 0x19
	.long	.LASF553
	.byte	0x5
	.uleb128 0x1a
	.long	.LASF554
	.byte	0x5
	.uleb128 0x1b
	.long	.LASF555
	.byte	0x5
	.uleb128 0x1c
	.long	.LASF556
	.byte	0x5
	.uleb128 0x1d
	.long	.LASF557
	.byte	0x5
	.uleb128 0x1e
	.long	.LASF558
	.byte	0x5
	.uleb128 0x1f
	.long	.LASF559
	.byte	0x5
	.uleb128 0x20
	.long	.LASF560
	.byte	0x5
	.uleb128 0x21
	.long	.LASF561
	.byte	0x5
	.uleb128 0x22
	.long	.LASF562
	.byte	0x5
	.uleb128 0x23
	.long	.LASF563
	.byte	0x5
	.uleb128 0x24
	.long	.LASF564
	.byte	0x5
	.uleb128 0x25
	.long	.LASF565
	.byte	0x5
	.uleb128 0x26
	.long	.LASF566
	.byte	0x5
	.uleb128 0x27
	.long	.LASF567
	.byte	0x5
	.uleb128 0x28
	.long	.LASF568
	.byte	0x5
	.uleb128 0x29
	.long	.LASF569
	.byte	0x5
	.uleb128 0x2a
	.long	.LASF570
	.byte	0x5
	.uleb128 0x2b
	.long	.LASF571
	.byte	0x5
	.uleb128 0x2c
	.long	.LASF572
	.byte	0x5
	.uleb128 0x2d
	.long	.LASF573
	.byte	0x5
	.uleb128 0x2f
	.long	.LASF574
	.byte	0x5
	.uleb128 0x30
	.long	.LASF575
	.byte	0x5
	.uleb128 0x31
	.long	.LASF576
	.byte	0x5
	.uleb128 0x32
	.long	.LASF577
	.byte	0x5
	.uleb128 0x33
	.long	.LASF578
	.byte	0x5
	.uleb128 0x34
	.long	.LASF579
	.byte	0x5
	.uleb128 0x35
	.long	.LASF580
	.byte	0x5
	.uleb128 0x36
	.long	.LASF581
	.byte	0x5
	.uleb128 0x37
	.long	.LASF582
	.byte	0x5
	.uleb128 0x38
	.long	.LASF583
	.byte	0x5
	.uleb128 0x39
	.long	.LASF584
	.byte	0x5
	.uleb128 0x3a
	.long	.LASF585
	.byte	0x5
	.uleb128 0x3b
	.long	.LASF586
	.byte	0x5
	.uleb128 0x3c
	.long	.LASF587
	.byte	0x5
	.uleb128 0x3d
	.long	.LASF588
	.byte	0x5
	.uleb128 0x3e
	.long	.LASF589
	.byte	0x5
	.uleb128 0x3f
	.long	.LASF590
	.byte	0x5
	.uleb128 0x40
	.long	.LASF591
	.byte	0x5
	.uleb128 0x41
	.long	.LASF592
	.byte	0x5
	.uleb128 0x42
	.long	.LASF593
	.byte	0x5
	.uleb128 0x43
	.long	.LASF594
	.byte	0x5
	.uleb128 0x44
	.long	.LASF595
	.byte	0x5
	.uleb128 0x45
	.long	.LASF596
	.byte	0x5
	.uleb128 0x47
	.long	.LASF597
	.byte	0x5
	.uleb128 0x49
	.long	.LASF598
	.byte	0x5
	.uleb128 0x4a
	.long	.LASF599
	.byte	0x5
	.uleb128 0x4b
	.long	.LASF600
	.byte	0x5
	.uleb128 0x4c
	.long	.LASF601
	.byte	0x5
	.uleb128 0x4d
	.long	.LASF602
	.byte	0x5
	.uleb128 0x4e
	.long	.LASF603
	.byte	0x5
	.uleb128 0x4f
	.long	.LASF604
	.byte	0x5
	.uleb128 0x50
	.long	.LASF605
	.byte	0x5
	.uleb128 0x51
	.long	.LASF606
	.byte	0x5
	.uleb128 0x52
	.long	.LASF607
	.byte	0x5
	.uleb128 0x53
	.long	.LASF608
	.byte	0x5
	.uleb128 0x54
	.long	.LASF609
	.byte	0x5
	.uleb128 0x55
	.long	.LASF610
	.byte	0x5
	.uleb128 0x56
	.long	.LASF611
	.byte	0x5
	.uleb128 0x57
	.long	.LASF612
	.byte	0x5
	.uleb128 0x58
	.long	.LASF613
	.byte	0x5
	.uleb128 0x59
	.long	.LASF614
	.byte	0x5
	.uleb128 0x5a
	.long	.LASF615
	.byte	0x5
	.uleb128 0x5b
	.long	.LASF616
	.byte	0x5
	.uleb128 0x5c
	.long	.LASF617
	.byte	0x5
	.uleb128 0x5d
	.long	.LASF618
	.byte	0x5
	.uleb128 0x5e
	.long	.LASF619
	.byte	0x5
	.uleb128 0x5f
	.long	.LASF620
	.byte	0x5
	.uleb128 0x60
	.long	.LASF621
	.byte	0x5
	.uleb128 0x61
	.long	.LASF622
	.byte	0x5
	.uleb128 0x62
	.long	.LASF623
	.byte	0x5
	.uleb128 0x63
	.long	.LASF624
	.byte	0x5
	.uleb128 0x64
	.long	.LASF625
	.byte	0x5
	.uleb128 0x65
	.long	.LASF626
	.byte	0x5
	.uleb128 0x66
	.long	.LASF627
	.byte	0x5
	.uleb128 0x67
	.long	.LASF628
	.byte	0x5
	.uleb128 0x68
	.long	.LASF629
	.byte	0x5
	.uleb128 0x69
	.long	.LASF630
	.byte	0x5
	.uleb128 0x6a
	.long	.LASF631
	.byte	0x5
	.uleb128 0x6b
	.long	.LASF632
	.byte	0x5
	.uleb128 0x6c
	.long	.LASF633
	.byte	0x5
	.uleb128 0x6d
	.long	.LASF634
	.byte	0x5
	.uleb128 0x6e
	.long	.LASF635
	.byte	0x5
	.uleb128 0x6f
	.long	.LASF636
	.byte	0x5
	.uleb128 0x70
	.long	.LASF637
	.byte	0x5
	.uleb128 0x71
	.long	.LASF638
	.byte	0x5
	.uleb128 0x72
	.long	.LASF639
	.byte	0x5
	.uleb128 0x73
	.long	.LASF640
	.byte	0x5
	.uleb128 0x74
	.long	.LASF641
	.byte	0x5
	.uleb128 0x75
	.long	.LASF642
	.byte	0x5
	.uleb128 0x76
	.long	.LASF643
	.byte	0x5
	.uleb128 0x77
	.long	.LASF644
	.byte	0x5
	.uleb128 0x78
	.long	.LASF645
	.byte	0x5
	.uleb128 0x79
	.long	.LASF646
	.byte	0x5
	.uleb128 0x7a
	.long	.LASF647
	.byte	0x5
	.uleb128 0x7b
	.long	.LASF648
	.byte	0x5
	.uleb128 0x7c
	.long	.LASF649
	.byte	0x5
	.uleb128 0x7d
	.long	.LASF650
	.byte	0x5
	.uleb128 0x7e
	.long	.LASF651
	.byte	0x5
	.uleb128 0x7f
	.long	.LASF652
	.byte	0x5
	.uleb128 0x80
	.long	.LASF653
	.byte	0x5
	.uleb128 0x81
	.long	.LASF654
	.byte	0x5
	.uleb128 0x82
	.long	.LASF655
	.byte	0x5
	.uleb128 0x83
	.long	.LASF656
	.byte	0x5
	.uleb128 0x84
	.long	.LASF657
	.byte	0x5
	.uleb128 0x85
	.long	.LASF658
	.byte	0x5
	.uleb128 0x86
	.long	.LASF659
	.byte	0x5
	.uleb128 0x87
	.long	.LASF660
	.byte	0x5
	.uleb128 0x88
	.long	.LASF661
	.byte	0x5
	.uleb128 0x8a
	.long	.LASF662
	.byte	0x5
	.uleb128 0x8b
	.long	.LASF663
	.byte	0x5
	.uleb128 0x8c
	.long	.LASF664
	.byte	0x5
	.uleb128 0x8d
	.long	.LASF665
	.byte	0x5
	.uleb128 0x8e
	.long	.LASF666
	.byte	0x5
	.uleb128 0x8f
	.long	.LASF667
	.byte	0x5
	.uleb128 0x90
	.long	.LASF668
	.byte	0x5
	.uleb128 0x93
	.long	.LASF669
	.byte	0x5
	.uleb128 0x94
	.long	.LASF670
	.byte	0x5
	.uleb128 0x97
	.long	.LASF671
	.byte	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF634:
	.string	"EOPNOTSUPP 95"
.LASF329:
	.string	"__linux__ 1"
.LASF221:
	.string	"__FLT128_MIN_10_EXP__ (-4931)"
.LASF279:
	.string	"__DEC128_EPSILON__ 1E-33DL"
.LASF499:
	.string	"DBG_VM DBG_MODE(28)"
.LASF47:
	.string	"__UINT32_TYPE__ unsigned int"
.LASF294:
	.string	"__GCC_ATOMIC_WCHAR_T_LOCK_FREE 2"
.LASF104:
	.string	"__UINT32_MAX__ 0xffffffffU"
.LASF686:
	.string	"size_t"
.LASF370:
	.string	"PAGE_SIZE ((uintptr_t)(1UL << PAGE_SHIFT))"
.LASF645:
	.string	"EISCONN 106"
.LASF559:
	.string	"ENOTDIR 20"
.LASF825:
	.string	"sched_queue_init"
.LASF216:
	.string	"__FLT64_HAS_INFINITY__ 1"
.LASF767:
	.string	"kc_ctx"
.LASF505:
	.string	"DBG_VMMAP DBG_MODE(35)"
.LASF89:
	.string	"__SIZE_WIDTH__ 64"
.LASF474:
	.string	"DBG_MM DBG_MODE(1)"
.LASF372:
	.string	"PAGE_ALIGN_DOWN(x) ((void *)(((uintptr_t)(x)&PAGE_MASK)))"
.LASF278:
	.string	"__DEC128_MAX__ 9.999999999999999999999999999999999E6144DL"
.LASF15:
	.string	"__LP64__ 1"
.LASF777:
	.string	"KT_SLEEP_CANCELLABLE"
.LASF487:
	.string	"DBG_ERROR DBG_MODE(14)"
.LASF759:
	.string	"c_rsp"
.LASF411:
	.string	"PT_CACHE_DISABLED 0x010"
.LASF266:
	.string	"__DEC32_SUBNORMAL_MIN__ 0.000001E-95DF"
.LASF210:
	.string	"__FLT64_DECIMAL_DIG__ 17"
.LASF751:
	.string	"p_cwd"
.LASF658:
	.string	"ENAVAIL 119"
.LASF12:
	.string	"__ATOMIC_CONSUME 1"
.LASF359:
	.string	"list_iterate(list,var,type,member) for (type *var = list_head(list, type, member), *__next_ ##var = list_next(var, type, member); &var->member != (list); var = __next_ ##var, __next_ ##var = list_next(var, type, member))"
.LASF181:
	.string	"__DECIMAL_DIG__ 21"
.LASF420:
	.string	"PDE(x) ((((uintptr_t)(x)) >> 21) & INDEX_MASK)"
.LASF566:
	.string	"EFBIG 27"
.LASF369:
	.string	"PAGE_SHIFT 12"
.LASF680:
	.string	"int32_t"
.LASF18:
	.string	"__SIZEOF_LONG_LONG__ 8"
.LASF127:
	.string	"__INT_FAST8_WIDTH__ 8"
.LASF166:
	.string	"__DBL_MAX_10_EXP__ 308"
.LASF818:
	.string	"dbg_print"
.LASF204:
	.string	"__FLT64_MANT_DIG__ 53"
.LASF263:
	.string	"__DEC32_MIN__ 1E-95DF"
.LASF132:
	.string	"__INT_FAST64_MAX__ 0x7fffffffffffffffL"
.LASF28:
	.string	"__ORDER_PDP_ENDIAN__ 3412"
.LASF269:
	.string	"__DEC64_MAX_EXP__ 385"
.LASF761:
	.string	"c_pml4"
.LASF41:
	.string	"__INT8_TYPE__ signed char"
.LASF662:
	.string	"ENOMEDIUM 123"
.LASF196:
	.string	"__FLT32_DECIMAL_DIG__ 9"
.LASF795:
	.string	"__func__"
.LASF32:
	.string	"__SIZE_TYPE__ long unsigned int"
.LASF119:
	.string	"__UINT8_C(c) c"
.LASF500:
	.string	"DBG_TEST DBG_MODE(30)"
.LASF42:
	.string	"__INT16_TYPE__ short int"
.LASF726:
	.string	"kt_state"
.LASF531:
	.string	"INTR_APICTIMER 0xf0"
.LASF366:
	.string	"KERNEL_VMA 0xffff800000000000"
.LASF654:
	.string	"EINPROGRESS 115"
.LASF340:
	.string	"NULL 0"
.LASF286:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 1"
.LASF635:
	.string	"EPFNOSUPPORT 96"
.LASF698:
	.string	"kernel_end_init"
.LASF2:
	.string	"__STDC_HOSTED__ 0"
.LASF701:
	.string	"spinlock"
.LASF404:
	.string	"PAGE_FLAGS(x) ((x) & (~PAGE_MASK))"
.LASF667:
	.string	"EKEYREVOKED 128"
.LASF309:
	.string	"__x86_64 1"
.LASF35:
	.string	"__WINT_TYPE__ unsigned int"
.LASF245:
	.string	"__FLT32X_HAS_QUIET_NAN__ 1"
.LASF198:
	.string	"__FLT32_MIN__ 1.17549435082228750796873653722224568e-38F32"
.LASF195:
	.string	"__FLT32_MAX_10_EXP__ 38"
.LASF101:
	.string	"__INT64_MAX__ 0x7fffffffffffffffL"
.LASF192:
	.string	"__FLT32_MIN_EXP__ (-125)"
.LASF175:
	.string	"__LDBL_MANT_DIG__ 64"
.LASF517:
	.string	"dbg_active(mode) (dbg_modes & (mode))"
.LASF560:
	.string	"EISDIR 21"
.LASF86:
	.string	"__WCHAR_WIDTH__ 32"
.LASF774:
	.string	"KT_ON_CPU"
.LASF207:
	.string	"__FLT64_MIN_10_EXP__ (-307)"
.LASF448:
	.string	"VFS_ROOTFS_TYPE \"ramfs\""
.LASF133:
	.string	"__INT_FAST64_WIDTH__ 64"
.LASF1:
	.string	"__STDC_VERSION__ 199901L"
.LASF373:
	.string	"PAGE_ALIGN_UP(x) ((void *)((((uintptr_t)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)))"
.LASF547:
	.string	"ENOEXEC 8"
.LASF396:
	.string	"PAGE_ALIGNED_1GB(x) ((x) == PAGE_ALIGN_DOWN_1GB(x))"
.LASF829:
	.string	"proc/kmutex.c"
.LASF425:
	.string	"PML4_VADDR_SIZE (PAGE_SIZE * PT_ENTRY_COUNT * PT_ENTRY_COUNT * PT_ENTRY_COUNT * PT_ENTRY_COUNT)"
.LASF567:
	.string	"ENOSPC 28"
.LASF27:
	.string	"__ORDER_BIG_ENDIAN__ 4321"
.LASF808:
	.string	"lessthaneq"
.LASF194:
	.string	"__FLT32_MAX_EXP__ 128"
.LASF277:
	.string	"__DEC128_MIN__ 1E-6143DL"
.LASF650:
	.string	"ECONNREFUSED 111"
.LASF714:
	.string	"km_waitq"
.LASF388:
	.string	"PAGE_ALIGNED_2MB(x) ((x) == PAGE_ALIGN_DOWN_2MB(x))"
.LASF231:
	.string	"__FLT128_HAS_QUIET_NAN__ 1"
.LASF421:
	.string	"PTE(x) ((((uintptr_t)(x)) >> 12) & INDEX_MASK)"
.LASF582:
	.string	"EIDRM 43"
.LASF816:
	.string	"spinlock_lock"
.LASF265:
	.string	"__DEC32_EPSILON__ 1E-6DF"
.LASF452:
	.string	"PID_INIT 1"
.LASF251:
	.string	"__FLT64X_MAX_10_EXP__ 4932"
.LASF383:
	.string	"PAGE_SIZE_2MB ((uintptr_t)(1UL << PAGE_SHIFT_2MB))"
.LASF556:
	.string	"EEXIST 17"
.LASF356:
	.string	"list_next(current,type,member) list_head(&(current)->member, type, member)"
.LASF491:
	.string	"DBG_BRK DBG_MODE(18)"
.LASF6:
	.string	"__VERSION__ \"7.5.0\""
.LASF732:
	.string	"kt_preemption_count"
.LASF36:
	.string	"__INTMAX_TYPE__ long int"
.LASF665:
	.string	"ENOKEY 126"
.LASF377:
	.string	"ADDR_TO_PN(x) (((uintptr_t)(x)) >> PAGE_SHIFT)"
.LASF183:
	.string	"__LDBL_MAX__ 1.18973149535723176502126385303097021e+4932L"
.LASF264:
	.string	"__DEC32_MAX__ 9.999999E96DF"
.LASF134:
	.string	"__UINT_FAST8_MAX__ 0xff"
.LASF629:
	.string	"EMSGSIZE 90"
.LASF766:
	.string	"kc_id"
.LASF244:
	.string	"__FLT32X_HAS_INFINITY__ 1"
.LASF480:
	.string	"DBG_PAGEALLOC DBG_MODE(7)"
.LASF717:
	.string	"km_link"
.LASF518:
	.string	"panic(...) dbg_panic(__FILE__, __LINE__, __func__, __VA_ARGS__)"
.LASF53:
	.string	"__UINT_LEAST8_TYPE__ unsigned char"
.LASF814:
	.string	"apic_initialized"
.LASF313:
	.string	"__ATOMIC_HLE_ACQUIRE 65536"
.LASF824:
	.string	"sched_sleep_on"
.LASF565:
	.string	"ETXTBSY 26"
.LASF682:
	.string	"long int"
.LASF427:
	.string	"IS_2MB_PAGE(n) ((n)&PT_SIZE)"
.LASF238:
	.string	"__FLT32X_DECIMAL_DIG__ 17"
.LASF454:
	.string	"MAX_LAPICS 8"
.LASF315:
	.string	"__GCC_ASM_FLAG_OUTPUTS__ 1"
.LASF275:
	.string	"__DEC128_MIN_EXP__ (-6142)"
.LASF220:
	.string	"__FLT128_MIN_EXP__ (-16381)"
.LASF64:
	.string	"__UINT_FAST64_TYPE__ long unsigned int"
.LASF330:
	.string	"__unix 1"
.LASF424:
	.string	"PDP_VADDR_SIZE (PAGE_SIZE * PT_ENTRY_COUNT * PT_ENTRY_COUNT * PT_ENTRY_COUNT)"
.LASF49:
	.string	"__INT_LEAST8_TYPE__ signed char"
.LASF788:
	.string	"curcore"
.LASF811:
	.string	"notequals"
.LASF31:
	.string	"__SIZEOF_POINTER__ 8"
.LASF380:
	.string	"USE_2MB_PAGES 1"
.LASF257:
	.string	"__FLT64X_HAS_DENORM__ 1"
.LASF540:
	.string	"EPERM 1"
.LASF620:
	.string	"ELIBSCN 81"
.LASF63:
	.string	"__UINT_FAST32_TYPE__ long unsigned int"
.LASF529:
	.string	"INTR_GPF 0x0d"
.LASF484:
	.string	"DBG_PROC DBG_MODE(11)"
.LASF527:
	.string	"INTR_DIVIDE_BY_ZERO 0x00"
.LASF666:
	.string	"EKEYEXPIRED 127"
.LASF817:
	.string	"dbg_color"
.LASF490:
	.string	"DBG_PGTBL DBG_MODE(17)"
.LASF341:
	.string	"packed __attribute__((packed))"
.LASF137:
	.string	"__UINT_FAST64_MAX__ 0xffffffffffffffffUL"
.LASF802:
	.string	"__next_waiter"
.LASF575:
	.string	"ENAMETOOLONG 36"
.LASF596:
	.string	"EBADSLT 57"
.LASF17:
	.string	"__SIZEOF_LONG__ 8"
.LASF21:
	.string	"__SIZEOF_DOUBLE__ 8"
.LASF114:
	.string	"__INT_LEAST32_WIDTH__ 32"
.LASF750:
	.string	"p_files"
.LASF621:
	.string	"ELIBMAX 82"
.LASF599:
	.string	"ENOSTR 60"
.LASF441:
	.string	"MAXPATHLEN 1024"
.LASF116:
	.string	"__INT64_C(c) c ## L"
.LASF214:
	.string	"__FLT64_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F64"
.LASF271:
	.string	"__DEC64_MAX__ 9.999999999999999E384DD"
.LASF462:
	.string	"_CYAN_ \"\\x1b[36;40m\""
.LASF150:
	.string	"__FLT_MIN_10_EXP__ (-37)"
.LASF344:
	.string	"offsetof(type,member) ((uintptr_t)((char *)&((type *)(0))->member - (char *)0))"
.LASF671:
	.string	"ENOTSUP EOPNOTSUPP"
.LASF153:
	.string	"__FLT_DECIMAL_DIG__ 9"
.LASF691:
	.string	"kernel_start_data"
.LASF595:
	.string	"EBADRQC 56"
.LASF674:
	.string	"signed char"
.LASF676:
	.string	"uint8_t"
.LASF385:
	.string	"PAGE_ALIGN_DOWN_2MB(x) (((uintptr_t)(x)) & PAGE_MASK_2MB)"
.LASF200:
	.string	"__FLT32_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F32"
.LASF283:
	.string	"__GNUC_STDC_INLINE__ 1"
.LASF457:
	.string	"_RED_ \"\\x1b[31;40m\""
.LASF160:
	.string	"__FLT_HAS_QUIET_NAN__ 1"
.LASF5:
	.string	"__GNUC_PATCHLEVEL__ 0"
.LASF564:
	.string	"ENOTTY 25"
.LASF293:
	.string	"__GCC_ATOMIC_CHAR32_T_LOCK_FREE 2"
.LASF614:
	.string	"EOVERFLOW 75"
.LASF506:
	.string	"DBG_ELF DBG_MODE(37)"
.LASF88:
	.string	"__PTRDIFF_WIDTH__ 64"
.LASF120:
	.string	"__UINT_LEAST16_MAX__ 0xffff"
.LASF757:
	.string	"context"
.LASF644:
	.string	"ENOBUFS 105"
.LASF636:
	.string	"EAFNOSUPPORT 97"
.LASF568:
	.string	"ESPIPE 29"
.LASF242:
	.string	"__FLT32X_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F32x"
.LASF209:
	.string	"__FLT64_MAX_10_EXP__ 308"
.LASF129:
	.string	"__INT_FAST16_WIDTH__ 64"
.LASF677:
	.string	"unsigned char"
.LASF199:
	.string	"__FLT32_EPSILON__ 1.19209289550781250000000000000000000e-7F32"
.LASF152:
	.string	"__FLT_MAX_10_EXP__ 38"
.LASF765:
	.string	"core"
.LASF276:
	.string	"__DEC128_MAX_EXP__ 6145"
.LASF797:
	.string	"kmutex_init"
.LASF690:
	.string	"kernel_start_text"
.LASF406:
	.string	"VMMAP_DIR_HILO 2"
.LASF248:
	.string	"__FLT64X_MIN_EXP__ (-16381)"
.LASF8:
	.string	"__ATOMIC_SEQ_CST 5"
.LASF496:
	.string	"DBG_THR DBG_MODE(23)"
.LASF414:
	.string	"PT_SIZE 0x080"
.LASF731:
	.string	"kt_recent_core"
.LASF159:
	.string	"__FLT_HAS_INFINITY__ 1"
.LASF587:
	.string	"ELNRNG 48"
.LASF417:
	.string	"INDEX_MASK 0b111111111"
.LASF712:
	.string	"ktqueue_t"
.LASF103:
	.string	"__UINT16_MAX__ 0xffff"
.LASF552:
	.string	"EACCES 13"
.LASF538:
	.string	"IPL_HIGH 0xe0"
.LASF123:
	.string	"__UINT32_C(c) c ## U"
.LASF570:
	.string	"EMLINK 31"
.LASF324:
	.string	"__SSE2_MATH__ 1"
.LASF91:
	.string	"__INTMAX_C(c) c ## L"
.LASF179:
	.string	"__LDBL_MAX_EXP__ 16384"
.LASF19:
	.string	"__SIZEOF_SHORT__ 2"
.LASF471:
	.string	"DBG_MODE(x) (1ULL << (x))"
.LASF613:
	.string	"EBADMSG 74"
.LASF640:
	.string	"ENETUNREACH 101"
.LASF60:
	.string	"__INT_FAST64_TYPE__ long int"
.LASF365:
	.string	"KERNEL_PHYS_END ((uintptr_t)(&kernel_phys_end))"
.LASF285:
	.string	"__STRICT_ANSI__ 1"
.LASF598:
	.string	"EBFONT 59"
.LASF93:
	.string	"__UINTMAX_C(c) c ## UL"
.LASF226:
	.string	"__FLT128_MIN__ 3.36210314311209350626267781732175260e-4932F128"
.LASF323:
	.string	"__SSE_MATH__ 1"
.LASF180:
	.string	"__LDBL_MAX_10_EXP__ 4932"
.LASF763:
	.string	"c_kstacksz"
.LASF381:
	.string	"USE_1GB_PAGES 1"
.LASF673:
	.string	"char"
.LASF692:
	.string	"kernel_start_bss"
.LASF353:
	.string	"LIST_LINK_INITIALIZER(list_link) { .l_next = NULL, .l_prev = NULL }"
.LASF287:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 1"
.LASF738:
	.string	"p_pid"
.LASF756:
	.string	"pml4_t"
.LASF135:
	.string	"__UINT_FAST16_MAX__ 0xffffffffffffffffUL"
.LASF75:
	.string	"__WCHAR_MAX__ 0x7fffffff"
.LASF354:
	.string	"list_item(link,type,member) (type *)((char *)(link)-offsetof(type, member))"
.LASF695:
	.string	"kernel_end_data"
.LASF708:
	.string	"ktqueue"
.LASF661:
	.string	"EDQUOT 122"
.LASF25:
	.string	"__BIGGEST_ALIGNMENT__ 16"
.LASF513:
	.ascii	"dbg(mode,...) do { if (dbg_active(mod"
	.string	"e)) { DEBUG_ENTER dbg_print(\"%s\", dbg_color(mode)); dbg_print(\"C%ld P%ld \", curcore.kc_id, curproc ? curproc->p_pid : -1L); dbg_print(\"%s:%d %s(): \", __FILE__, __LINE__, __func__); dbg_print(__VA_ARGS__); dbg_print(\"%s\", _NORMAL_); DEBUG_EXIT } } while (0)"
.LASF539:
	.string	"IPL_HIGHEST 0xff"
.LASF597:
	.string	"EDEADLOCK EDEADLK"
.LASF416:
	.string	"PT_ENTRY_COUNT (PAGE_SIZE / sizeof(uintptr_t))"
.LASF310:
	.string	"__x86_64__ 1"
.LASF820:
	.string	"dbg_panic"
.LASF721:
	.string	"kt_retval"
.LASF577:
	.string	"ENOSYS 38"
.LASF792:
	.string	"debug_lock"
.LASF102:
	.string	"__UINT8_MAX__ 0xff"
.LASF156:
	.string	"__FLT_EPSILON__ 1.19209289550781250000000000000000000e-7F"
.LASF249:
	.string	"__FLT64X_MIN_10_EXP__ (-4931)"
.LASF589:
	.string	"ENOCSI 50"
.LASF412:
	.string	"PT_ACCESSED 0x020"
.LASF172:
	.string	"__DBL_HAS_DENORM__ 1"
.LASF398:
	.string	"PAGE_SHIFT_512GB 39"
.LASF325:
	.string	"__SEG_FS 1"
.LASF737:
	.string	"proc"
.LASF69:
	.string	"__GXX_ABI_VERSION 1011"
.LASF694:
	.string	"kernel_end_text"
.LASF806:
	.string	"flags"
.LASF109:
	.string	"__INT_LEAST16_MAX__ 0x7fff"
.LASF642:
	.string	"ECONNABORTED 103"
.LASF222:
	.string	"__FLT128_MAX_EXP__ 16384"
.LASF367:
	.string	"PHYS_OFFSET 0xffff880000000000"
.LASF579:
	.string	"ELOOP 40"
.LASF465:
	.string	"_BGREEN_ \"\\x1b[1;32;40m\""
.LASF741:
	.string	"p_children"
.LASF507:
	.string	"DBG_USER DBG_MODE(38)"
.LASF68:
	.string	"__has_include_next(STR) __has_include_next__(STR)"
.LASF780:
	.string	"kthread_t"
.LASF401:
	.string	"PAGE_ALIGN_DOWN_512GB(x) (((uintptr_t)(x)) & PAGE_MASK_512GB)"
.LASF338:
	.string	"__KERNEL__ 1"
.LASF345:
	.string	"NOT_YET_IMPLEMENTED(f) dbg(DBG_PRINT, \"Not yet implemented: %s, file %s, line %d\\n\", f, __FILE__, __LINE__)"
.LASF805:
	.string	"intr_enable"
.LASF483:
	.string	"DBG_FORK DBG_MODE(10)"
.LASF592:
	.string	"EBADR 53"
.LASF57:
	.string	"__INT_FAST8_TYPE__ signed char"
.LASF571:
	.string	"EPIPE 32"
.LASF823:
	.string	"list_insert_tail"
.LASF503:
	.string	"DBG_MEMDEV DBG_MODE(33)"
.LASF789:
	.string	"curproc"
.LASF295:
	.string	"__GCC_ATOMIC_SHORT_LOCK_FREE 2"
.LASF351:
	.string	"QUOTE(...) QUOTE_BY_NAME(__VA_ARGS__)"
.LASF664:
	.string	"ECANCELED 125"
.LASF299:
	.string	"__GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1"
.LASF428:
	.string	"IS_1GB_PAGE IS_2MB_PAGE"
.LASF608:
	.string	"ESRMNT 69"
.LASF332:
	.string	"__ELF__ 1"
.LASF784:
	.string	"file"
.LASF619:
	.string	"ELIBBAD 80"
.LASF76:
	.string	"__WCHAR_MIN__ (-__WCHAR_MAX__ - 1)"
.LASF203:
	.string	"__FLT32_HAS_QUIET_NAN__ 1"
.LASF610:
	.string	"EPROTO 71"
.LASF699:
	.string	"kernel_phys_base"
.LASF188:
	.string	"__LDBL_HAS_INFINITY__ 1"
.LASF364:
	.string	"KERNEL_PHYS_BASE ((uintptr_t)(&kernel_phys_base))"
.LASF748:
	.string	"p_pml4"
.LASF779:
	.string	"kthread_state_t"
.LASF376:
	.string	"PN_TO_ADDR(x) ((void *)(((uintptr_t)(x)) << PAGE_SHIFT))"
.LASF237:
	.string	"__FLT32X_MAX_10_EXP__ 308"
.LASF445:
	.string	"NAME_LEN 28"
.LASF151:
	.string	"__FLT_MAX_EXP__ 128"
.LASF807:
	.string	"greaterthaneq"
.LASF96:
	.string	"__SIG_ATOMIC_MIN__ (-__SIG_ATOMIC_MAX__ - 1)"
.LASF118:
	.string	"__UINT_LEAST8_MAX__ 0xff"
.LASF297:
	.string	"__GCC_ATOMIC_LONG_LOCK_FREE 2"
.LASF778:
	.string	"KT_EXITED"
.LASF651:
	.string	"EHOSTDOWN 112"
.LASF258:
	.string	"__FLT64X_HAS_INFINITY__ 1"
.LASF704:
	.string	"l_next"
.LASF302:
	.string	"__PRAGMA_REDEFINE_EXTNAME 1"
.LASF443:
	.string	"MAX_VFS 8"
.LASF168:
	.string	"__DBL_MAX__ ((double)1.79769313486231570814527423731704357e+308L)"
.LASF469:
	.string	"_BCYAN_ \"\\x1b[1;36;40m\""
.LASF413:
	.string	"PT_DIRTY 0x040"
.LASF550:
	.string	"EAGAIN 11"
.LASF705:
	.string	"l_prev"
.LASF111:
	.string	"__INT_LEAST16_WIDTH__ 16"
.LASF652:
	.string	"EHOSTUNREACH 113"
.LASF145:
	.string	"__DEC_EVAL_METHOD__ 2"
.LASF787:
	.string	"idleproc"
.LASF493:
	.string	"DBG_VFS DBG_MODE(20)"
.LASF573:
	.string	"ERANGE 34"
.LASF273:
	.string	"__DEC64_SUBNORMAL_MIN__ 0.000000000000001E-383DD"
.LASF719:
	.string	"kt_ctx"
.LASF581:
	.string	"ENOMSG 42"
.LASF466:
	.string	"_BYELLOW_ \"\\x1b[1;33;40m\""
.LASF327:
	.string	"__gnu_linux__ 1"
.LASF516:
	.string	"dbginfo(mode,func,data) do { if (dbg_active(mode)) { DEBUG_ENTER dbg_print(\"%s\", dbg_color(mode)); dbg_print(\"C%ld P%ld \", curcore.kc_id, curproc ? curproc->p_pid : -1L); dbg_printinfo(func, data); dbg_print(\"%s\", _NORMAL_); DEBUG_EXIT } } while (0)"
.LASF182:
	.string	"__LDBL_DECIMAL_DIG__ 21"
.LASF52:
	.string	"__INT_LEAST64_TYPE__ long int"
.LASF827:
	.string	"list_link_init"
.LASF363:
	.string	"IDENTITY_MAPPED_RAM_SIZE (1 << 16)"
.LASF693:
	.string	"kernel_end"
.LASF781:
	.string	"PROC_RUNNING"
.LASF106:
	.string	"__INT_LEAST8_MAX__ 0x7f"
.LASF548:
	.string	"EBADF 9"
.LASF524:
	.string	"KASSERT_LESS(l,r) KASSERT_GENERIC(l, r, lessthan, \"<\")"
.LASF508:
	.string	"DBG_DEFAULT DBG_ERROR"
.LASF9:
	.string	"__ATOMIC_ACQUIRE 2"
.LASF749:
	.string	"p_wait"
.LASF762:
	.string	"c_kstack"
.LASF236:
	.string	"__FLT32X_MAX_EXP__ 1024"
.LASF67:
	.string	"__has_include(STR) __has_include__(STR)"
.LASF498:
	.string	"DBG_OSYSCALL DBG_MODE(25)"
.LASF252:
	.string	"__FLT64X_DECIMAL_DIG__ 21"
.LASF117:
	.string	"__INT_LEAST64_WIDTH__ 64"
.LASF422:
	.string	"PT_VADDR_SIZE (PAGE_SIZE * PT_ENTRY_COUNT)"
.LASF511:
	.string	"DEBUG_ENTER uint8_t __ipl = apic_initialized() ? intr_setipl(IPL_HIGH) : IPL_LOW; spinlock_lock(&debug_lock);"
.LASF641:
	.string	"ENETRESET 102"
.LASF98:
	.string	"__INT8_MAX__ 0x7f"
.LASF605:
	.string	"EREMOTE 66"
.LASF301:
	.string	"__GCC_HAVE_DWARF2_CFI_ASM 1"
.LASF684:
	.string	"long unsigned int"
.LASF463:
	.string	"_WHITE_ \"\\x1b[37;40m\""
.LASF438:
	.string	"TICK_MSECS 10"
.LASF3:
	.string	"__GNUC__ 7"
.LASF46:
	.string	"__UINT16_TYPE__ short unsigned int"
.LASF494:
	.string	"DBG_S5FS DBG_MODE(21)"
.LASF58:
	.string	"__INT_FAST16_TYPE__ long int"
.LASF803:
	.string	"intr_wait"
.LASF408:
	.string	"PT_WRITE 0x002"
.LASF623:
	.string	"EILSEQ 84"
.LASF185:
	.string	"__LDBL_EPSILON__ 1.08420217248550443400745280086994171e-19L"
.LASF262:
	.string	"__DEC32_MAX_EXP__ 97"
.LASF144:
	.string	"__FLT_EVAL_METHOD_TS_18661_3__ 0"
.LASF81:
	.string	"__SCHAR_WIDTH__ 8"
.LASF554:
	.string	"ENOTBLK 15"
.LASF374:
	.string	"PAGE_OFFSET(x) (((uintptr_t)(x)) & ~PAGE_MASK)"
.LASF395:
	.string	"PAGE_OFFSET_1GB(x) (((uintptr_t)(x)) & ~PAGE_MASK_1GB)"
.LASF600:
	.string	"ENODATA 61"
.LASF317:
	.string	"__k8__ 1"
.LASF14:
	.string	"_LP64 1"
.LASF431:
	.string	"GDB_PDP_PHYSADDR(pdp,v) (pdp->phys[PDPE(v)] & PAGE_MASK)"
.LASF609:
	.string	"ECOMM 70"
.LASF436:
	.string	"DEFAULT_STACK_SIZE_PAGES 16"
.LASF745:
	.string	"p_child_link"
.LASF504:
	.string	"DBG_ANON DBG_MODE(34)"
.LASF190:
	.string	"__FLT32_MANT_DIG__ 24"
.LASF815:
	.string	"intr_setipl"
.LASF139:
	.string	"__INTPTR_WIDTH__ 64"
.LASF430:
	.string	"GDB_PD_PHYSADDR(pd,v) (pd->phys[PDE(v)] & PAGE_MASK)"
.LASF630:
	.string	"EPROTOTYPE 91"
.LASF740:
	.string	"p_threads"
.LASF696:
	.string	"kernel_end_bss"
.LASF292:
	.string	"__GCC_ATOMIC_CHAR16_T_LOCK_FREE 2"
.LASF722:
	.string	"kt_errno"
.LASF165:
	.string	"__DBL_MAX_EXP__ 1024"
.LASF10:
	.string	"__ATOMIC_RELEASE 3"
.LASF243:
	.string	"__FLT32X_HAS_DENORM__ 1"
.LASF147:
	.string	"__FLT_MANT_DIG__ 24"
.LASF423:
	.string	"PD_VADDR_SIZE (PAGE_SIZE * PT_ENTRY_COUNT * PT_ENTRY_COUNT)"
.LASF472:
	.string	"DBG_ALL (~0ULL)"
.LASF296:
	.string	"__GCC_ATOMIC_INT_LOCK_FREE 2"
.LASF90:
	.string	"__INTMAX_MAX__ 0x7fffffffffffffffL"
.LASF760:
	.string	"c_rbp"
.LASF212:
	.string	"__FLT64_MIN__ 2.22507385850720138309023271733240406e-308F64"
.LASF355:
	.string	"list_head(list,type,member) list_item((list)->l_next, type, member)"
.LASF534:
	.string	"INTR_SPURIOUS 0xfe"
.LASF92:
	.string	"__UINTMAX_MAX__ 0xffffffffffffffffUL"
.LASF648:
	.string	"ETOOMANYREFS 109"
.LASF450:
	.string	"PROC_NAME_LEN 256"
.LASF235:
	.string	"__FLT32X_MIN_10_EXP__ (-307)"
.LASF320:
	.string	"__SSE__ 1"
.LASF94:
	.string	"__INTMAX_WIDTH__ 64"
.LASF637:
	.string	"EADDRINUSE 98"
.LASF289:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 1"
.LASF700:
	.string	"kernel_phys_end"
.LASF191:
	.string	"__FLT32_DIG__ 6"
.LASF461:
	.string	"_MAGENTA_ \"\\x1b[35;40m\""
.LASF551:
	.string	"ENOMEM 12"
.LASF112:
	.string	"__INT_LEAST32_MAX__ 0x7fffffff"
.LASF786:
	.string	"proc_t"
.LASF158:
	.string	"__FLT_HAS_DENORM__ 1"
.LASF616:
	.string	"EBADFD 77"
.LASF569:
	.string	"EROFS 30"
.LASF346:
	.string	"MIN(a,b) ((a) < (b) ? (a) : (b))"
.LASF477:
	.string	"DBG_DISK DBG_MODE(4)"
.LASF670:
	.string	"ENOTRECOVERABLE 131"
.LASF624:
	.string	"ERESTART 85"
.LASF546:
	.string	"E2BIG 7"
.LASF146:
	.string	"__FLT_RADIX__ 2"
.LASF514:
	.ascii	"dbg_force(mode,.."
	.string	".) do { DEBUG_ENTER dbg_print(\"%s\", dbg_color(mode)); dbg_print(\"C%ld P%ld \", curcore.kc_id, curproc ? curproc->p_pid : -1L); dbg_print(\"%s:%d %s(): \", __FILE__, __LINE__, __func__); dbg_print(__VA_ARGS__); dbg_print(\"%s\", _NORMAL_); DEBUG_EXIT } while (0)"
.LASF718:
	.string	"kthread"
.LASF724:
	.string	"kt_cancelled"
.LASF643:
	.string	"ECONNRESET 104"
.LASF189:
	.string	"__LDBL_HAS_QUIET_NAN__ 1"
.LASF186:
	.string	"__LDBL_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951L"
.LASF308:
	.string	"__amd64__ 1"
.LASF85:
	.string	"__LONG_LONG_WIDTH__ 64"
.LASF470:
	.string	"_BWHITE_ \"\\x1b[1;37;40m\""
.LASF468:
	.string	"_BMAGENTA_ \"\\x1b[1;35;40m\""
.LASF128:
	.string	"__INT_FAST16_MAX__ 0x7fffffffffffffffL"
.LASF510:
	.string	"DBG_BUFFER_SIZE (PAGE_SIZE)"
.LASF261:
	.string	"__DEC32_MIN_EXP__ (-94)"
.LASF298:
	.string	"__GCC_ATOMIC_LLONG_LOCK_FREE 2"
.LASF601:
	.string	"ETIME 62"
.LASF713:
	.string	"kmutex"
.LASF208:
	.string	"__FLT64_MAX_EXP__ 1024"
.LASF130:
	.string	"__INT_FAST32_MAX__ 0x7fffffffffffffffL"
.LASF646:
	.string	"ENOTCONN 107"
.LASF37:
	.string	"__UINTMAX_TYPE__ long unsigned int"
.LASF87:
	.string	"__WINT_WIDTH__ 32"
.LASF558:
	.string	"ENODEV 19"
.LASF24:
	.string	"__CHAR_BIT__ 8"
.LASF304:
	.string	"__SIZEOF_WCHAR_T__ 4"
.LASF126:
	.string	"__INT_FAST8_MAX__ 0x7f"
.LASF459:
	.string	"_YELLOW_ \"\\x1b[33;40m\""
.LASF809:
	.string	"greaterthan"
.LASF489:
	.string	"DBG_FREF DBG_MODE(16)"
.LASF831:
	.string	"kmutex_owns_mutex"
.LASF659:
	.string	"EISNAM 120"
.LASF660:
	.string	"EREMOTEIO 121"
.LASF136:
	.string	"__UINT_FAST32_MAX__ 0xffffffffffffffffUL"
.LASF407:
	.string	"PT_PRESENT 0x001"
.LASF720:
	.string	"kt_kstack"
.LASF813:
	.string	"sched_queue_empty"
.LASF460:
	.string	"_BLUE_ \"\\x1b[34;40m\""
.LASF706:
	.string	"list_t"
.LASF29:
	.string	"__BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__"
.LASF768:
	.string	"kc_queue"
.LASF113:
	.string	"__INT32_C(c) c"
.LASF34:
	.string	"__WCHAR_TYPE__ int"
.LASF66:
	.string	"__UINTPTR_TYPE__ long unsigned int"
.LASF800:
	.string	"__next_held"
.LASF464:
	.string	"_BRED_ \"\\x1b[1;31;40m\""
.LASF711:
	.string	"tq_lock"
.LASF521:
	.string	"KASSERTEQ(l,r) KASSERT_GENERIC(l, r, equals, \"==\")"
.LASF594:
	.string	"ENOANO 55"
.LASF256:
	.string	"__FLT64X_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951F64x"
.LASF358:
	.string	"list_tail(list,type,member) list_item((list)->l_prev, type, member)"
.LASF707:
	.string	"list_link_t"
.LASF316:
	.string	"__k8 1"
.LASF55:
	.string	"__UINT_LEAST32_TYPE__ unsigned int"
.LASF681:
	.string	"unsigned int"
.LASF59:
	.string	"__INT_FAST32_TYPE__ long int"
.LASF822:
	.string	"list_remove"
.LASF149:
	.string	"__FLT_MIN_EXP__ (-125)"
.LASF217:
	.string	"__FLT64_HAS_QUIET_NAN__ 1"
.LASF746:
	.string	"p_status"
.LASF536:
	.string	"INTR_SHUTDOWN 0xfd"
.LASF429:
	.string	"GDB_PT_PHYSADDR(pt,v) (pt->phys[PTE(v)] & PAGE_MASK)"
.LASF668:
	.string	"EKEYREJECTED 129"
.LASF675:
	.string	"spinlock_t"
.LASF239:
	.string	"__FLT32X_MAX__ 1.79769313486231570814527423731704357e+308F32x"
.LASF215:
	.string	"__FLT64_HAS_DENORM__ 1"
.LASF148:
	.string	"__FLT_DIG__ 6"
.LASF526:
	.string	"KASSERT_LESSEQ(l,r) KASSERT_GENERIC(l, r, lessthaneq, \"<=\")"
.LASF143:
	.string	"__FLT_EVAL_METHOD__ 0"
.LASF678:
	.string	"short int"
.LASF154:
	.string	"__FLT_MAX__ 3.40282346638528859811704183484516925e+38F"
.LASF70:
	.string	"__SCHAR_MAX__ 0x7f"
.LASF400:
	.string	"PAGE_MASK_512GB (0xffffffffffffffff << PAGE_SHIFT_512GB)"
.LASF227:
	.string	"__FLT128_EPSILON__ 1.92592994438723585305597794258492732e-34F128"
.LASF793:
	.string	"kmutex_unlock"
.LASF84:
	.string	"__LONG_WIDTH__ 64"
.LASF687:
	.string	"pid_t"
.LASF440:
	.string	"PF_HASH_SIZE 17"
.LASF223:
	.string	"__FLT128_MAX_10_EXP__ 4932"
.LASF141:
	.string	"__GCC_IEC_559 2"
.LASF833:
	.string	"intr_enabled"
.LASF378:
	.string	"PAGE_SAME(x,y) (PAGE_ALIGN_DOWN(x) == PAGE_ALIGN_DOWN(y))"
.LASF495:
	.string	"DBG_KB DBG_MODE(22)"
.LASF561:
	.string	"EINVAL 22"
.LASF426:
	.string	"IS_PRESENT(n) ((n)&PT_PRESENT)"
.LASF306:
	.string	"__SIZEOF_PTRDIFF_T__ 8"
.LASF30:
	.string	"__FLOAT_WORD_ORDER__ __ORDER_LITTLE_ENDIAN__"
.LASF45:
	.string	"__UINT8_TYPE__ unsigned char"
.LASF40:
	.string	"__SIG_ATOMIC_TYPE__ int"
.LASF649:
	.string	"ETIMEDOUT 110"
.LASF82:
	.string	"__SHRT_WIDTH__ 16"
.LASF274:
	.string	"__DEC128_MANT_DIG__ 34"
.LASF826:
	.string	"spinlock_init"
.LASF336:
	.string	"__NTERMS__ 3"
.LASF234:
	.string	"__FLT32X_MIN_EXP__ (-1021)"
.LASF702:
	.string	"list"
.LASF736:
	.string	"vmm_proc"
.LASF291:
	.string	"__GCC_ATOMIC_CHAR_LOCK_FREE 2"
.LASF563:
	.string	"EMFILE 24"
.LASF798:
	.string	"detect_deadlocks"
.LASF770:
	.string	"kc_csdpaddr"
.LASF100:
	.string	"__INT32_MAX__ 0x7fffffff"
.LASF689:
	.string	"kernel_start"
.LASF632:
	.string	"EPROTONOSUPPORT 93"
.LASF193:
	.string	"__FLT32_MIN_10_EXP__ (-37)"
.LASF755:
	.string	"phys"
.LASF157:
	.string	"__FLT_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F"
.LASF710:
	.string	"tq_size"
.LASF260:
	.string	"__DEC32_MANT_DIG__ 7"
.LASF419:
	.string	"PDPE(x) ((((uintptr_t)(x)) >> 30) & INDEX_MASK)"
.LASF403:
	.string	"PAGE_CONTROL_FLAGS(x) ((x) & (PT_PRESENT | PT_WRITE | PT_USER | PT_WRITE_THROUGH | PT_CACHE_DISABLED | PT_SIZE | PT_GLOBAL))"
.LASF371:
	.string	"PAGE_MASK (0xffffffffffffffff << PAGE_SHIFT)"
.LASF481:
	.string	"DBG_INTR DBG_MODE(8)"
.LASF528:
	.string	"INTR_INVALID_OPCODE 0x06"
.LASF725:
	.string	"kt_wchan"
.LASF458:
	.string	"_GREEN_ \"\\x1b[32;40m\""
.LASF74:
	.string	"__LONG_LONG_MAX__ 0x7fffffffffffffffLL"
.LASF467:
	.string	"_BBLUE_ \"\\x1b[1;34;40m\""
.LASF447:
	.string	"VFS_ROOTFS_DEV \"disk0\""
.LASF334:
	.string	"__RENAMEDIR__ 1"
.LASF259:
	.string	"__FLT64X_HAS_QUIET_NAN__ 1"
.LASF219:
	.string	"__FLT128_DIG__ 33"
.LASF107:
	.string	"__INT8_C(c) c"
.LASF557:
	.string	"EXDEV 18"
.LASF197:
	.string	"__FLT32_MAX__ 3.40282346638528859811704183484516925e+38F32"
.LASF522:
	.string	"KASSERTNEQ(l,r) KASSERT_GENERIC(l, r, notequals, \"!=\")"
.LASF392:
	.string	"PAGE_SIZE_1GB ((uintptr_t)(1UL << PAGE_SHIFT_1GB))"
.LASF163:
	.string	"__DBL_MIN_EXP__ (-1021)"
.LASF43:
	.string	"__INT32_TYPE__ int"
.LASF742:
	.string	"p_children_lock"
.LASF772:
	.string	"csd_vaddr_table"
.LASF555:
	.string	"EBUSY 16"
.LASF442:
	.string	"MAX_FILES 1024"
.LASF533:
	.string	"INTR_DISK_PRIMARY 0xd0"
.LASF439:
	.string	"KMEM_FRAC(x) (((x) >> 2) + ((x) >> 3))"
.LASF801:
	.string	"waiter"
.LASF187:
	.string	"__LDBL_HAS_DENORM__ 1"
.LASF288:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 1"
.LASF240:
	.string	"__FLT32X_MIN__ 2.22507385850720138309023271733240406e-308F32x"
.LASF456:
	.string	"_BLACK_ \"\\x1b[30;47m\""
.LASF230:
	.string	"__FLT128_HAS_INFINITY__ 1"
.LASF791:
	.string	"dbg_modes"
.LASF655:
	.string	"ESTALE 116"
.LASF44:
	.string	"__INT64_TYPE__ long int"
.LASF26:
	.string	"__ORDER_LITTLE_ENDIAN__ 1234"
.LASF617:
	.string	"EREMCHG 78"
.LASF267:
	.string	"__DEC64_MANT_DIG__ 16"
.LASF246:
	.string	"__FLT64X_MANT_DIG__ 64"
.LASF71:
	.string	"__SHRT_MAX__ 0x7fff"
.LASF758:
	.string	"c_rip"
.LASF434:
	.string	"GDB_CUR_PHYSADDR(v) GDB_PHYSADDR(curproc->p_pml4, (v))"
.LASF77:
	.string	"__WINT_MAX__ 0xffffffffU"
.LASF318:
	.string	"__code_model_large__ 1"
.LASF669:
	.string	"EOWNERDEAD 130"
.LASF253:
	.string	"__FLT64X_MAX__ 1.18973149535723176502126385303097021e+4932F64x"
.LASF65:
	.string	"__INTPTR_TYPE__ long int"
.LASF349:
	.string	"QUOTE_BY_NAME(...) #__VA_ARGS__"
.LASF488:
	.string	"DBG_SYSCALL DBG_MODE(15)"
.LASF743:
	.string	"p_pproc"
.LASF54:
	.string	"__UINT_LEAST16_TYPE__ short unsigned int"
.LASF282:
	.string	"__USER_LABEL_PREFIX__ "
.LASF38:
	.string	"__CHAR16_TYPE__ short unsigned int"
.LASF233:
	.string	"__FLT32X_DIG__ 15"
.LASF776:
	.string	"KT_SLEEP"
.LASF357:
	.string	"list_prev(current,type,member) list_tail(&(current)->member, type, member)"
.LASF138:
	.string	"__INTPTR_MAX__ 0x7fffffffffffffffL"
.LASF13:
	.string	"__FINITE_MATH_ONLY__ 0"
.LASF368:
	.string	"MEMORY_MAP_BASE 0x9000"
.LASF352:
	.string	"LIST_INITIALIZER(list) { .l_next = &(list), .l_prev = &(list) }"
.LASF337:
	.string	"__NDISKS__ 1"
.LASF322:
	.string	"__FXSR__ 1"
.LASF730:
	.string	"kt_mutexes"
.LASF224:
	.string	"__FLT128_DECIMAL_DIG__ 36"
.LASF391:
	.string	"PAGE_MASK_1GB (0xffffffffffffffff << PAGE_SHIFT_1GB)"
.LASF773:
	.string	"KT_NO_STATE"
.LASF303:
	.string	"__SIZEOF_INT128__ 16"
.LASF796:
	.string	"__ipl"
.LASF519:
	.string	"KASSERT(x) do { if (!(x)) panic(\"assertion failed: %s\", #x); } while (0)"
.LASF804:
	.string	"intr_disable"
.LASF453:
	.string	"CORE_SPECIFIC_DATA __attribute__((section(\".csd\"))) = {0}"
.LASF455:
	.string	"_NORMAL_ \"\\x1b[0m\""
.LASF343:
	.string	"noreturn __attribute__((noreturn))"
.LASF115:
	.string	"__INT_LEAST64_MAX__ 0x7fffffffffffffffL"
.LASF348:
	.string	"CONTAINER_OF(obj,type,member) ((type *)((char *)(obj)-offsetof(type, member)))"
.LASF653:
	.string	"EALREADY 114"
.LASF553:
	.string	"EFAULT 14"
.LASF542:
	.string	"ESRCH 3"
.LASF764:
	.string	"context_t"
.LASF733:
	.string	"kmutex_t"
.LASF319:
	.string	"__MMX__ 1"
.LASF342:
	.string	"inline __attribute__((always_inline, used))"
.LASF444:
	.string	"MAX_VNODES 1024"
.LASF399:
	.string	"PAGE_SIZE_512GB ((uintptr_t)(1UL << PAGE_SHIFT_512GB))"
.LASF361:
	.string	"KTQUEUE_INITIALIZER(ktqueue) { .tq_list = LIST_INITIALIZER((ktqueue).tq_list), .tq_lock = SPINLOCK_INITIALIZER(sdf.tq_lock), .tq_size = 0, }"
.LASF729:
	.string	"kt_qlink"
.LASF794:
	.string	"kmutex_lock"
.LASF752:
	.string	"p_brk"
.LASF80:
	.string	"__SIZE_MAX__ 0xffffffffffffffffUL"
.LASF125:
	.string	"__UINT64_C(c) c ## UL"
.LASF832:
	.string	"kmutex_has_waiters"
.LASF433:
	.string	"GDB_PHYSADDR(pml4,v) (GDB_PT_PHYSADDR( GDB_PD_PHYSADDR( GDB_PDP_PHYSADDR(GDB_PML4_PHYSADDR(pml4, (v)) + PHYS_OFFSET, (v)) + PHYS_OFFSET, (v)) + PHYS_OFFSET, (v)) + PHYS_OFFSET)"
.LASF520:
	.ascii	"KA"
	.string	"SSERT_GENERIC(left,right,comparator,comp_str) do { int __left = (int)(left); int __right = (int)(right); if (!comparator(__left, __right)) { panic(\"assertion failed: %s %s %s. Left: %d, Right: %d\\n\", #left, comp_str, #right, __left, __right); } } while (0)"
.LASF591:
	.string	"EBADE 52"
.LASF475:
	.string	"DBG_INIT DBG_MODE(2)"
.LASF638:
	.string	"EADDRNOTAVAIL 99"
.LASF56:
	.string	"__UINT_LEAST64_TYPE__ long unsigned int"
.LASF169:
	.string	"__DBL_MIN__ ((double)2.22507385850720138309023271733240406e-308L)"
.LASF628:
	.string	"EDESTADDRREQ 89"
.LASF254:
	.string	"__FLT64X_MIN__ 3.36210314311209350626267781732175260e-4932F64x"
.LASF486:
	.string	"DBG_PFRAME DBG_MODE(13)"
.LASF393:
	.string	"PAGE_ALIGN_DOWN_1GB(x) (((uintptr_t)(x)) & PAGE_MASK_1GB)"
.LASF586:
	.string	"EL3RST 47"
.LASF51:
	.string	"__INT_LEAST32_TYPE__ int"
.LASF255:
	.string	"__FLT64X_EPSILON__ 1.08420217248550443400745280086994171e-19F64x"
.LASF627:
	.string	"ENOTSOCK 88"
.LASF379:
	.string	"PAGE_NSIZES 8"
.LASF4:
	.string	"__GNUC_MINOR__ 5"
.LASF178:
	.string	"__LDBL_MIN_10_EXP__ (-4931)"
.LASF394:
	.string	"PAGE_ALIGN_UP_1GB(x) (PAGE_ALIGN_DOWN_1GB((x)-1) + PAGE_SIZE_1GB)"
.LASF830:
	.string	"/home/andy/weenix_64/kernel"
.LASF541:
	.string	"ENOENT 2"
.LASF497:
	.string	"DBG_PRINT DBG_MODE(24)"
.LASF241:
	.string	"__FLT32X_EPSILON__ 2.22044604925031308084726333618164062e-16F32x"
.LASF501:
	.string	"DBG_TESTPASS DBG_MODE(31)"
.LASF578:
	.string	"ENOTEMPTY 39"
.LASF174:
	.string	"__DBL_HAS_QUIET_NAN__ 1"
.LASF362:
	.string	"KMUTEX_INITIALIZER(mtx) { .km_waitq = KTQUEUE_INITIALIZER((mtx).km_waitq), .km_holder = NULL, .km_lock = SPINLOCK_INITIALIZER((mtx).km_lock), .km_link = LIST_LINK_INITIALIZER((mtx).km_link), }"
.LASF633:
	.string	"ESOCKTNOSUPPORT 94"
.LASF409:
	.string	"PT_USER 0x004"
.LASF639:
	.string	"ENETDOWN 100"
.LASF321:
	.string	"__SSE2__ 1"
.LASF228:
	.string	"__FLT128_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966F128"
.LASF473:
	.string	"DBG_CORE DBG_MODE(0)"
.LASF281:
	.string	"__REGISTER_PREFIX__ "
.LASF280:
	.string	"__DEC128_SUBNORMAL_MIN__ 0.000000000000000000000000000000001E-6143DL"
.LASF162:
	.string	"__DBL_DIG__ 15"
.LASF754:
	.string	"p_vmmap"
.LASF603:
	.string	"ENONET 64"
.LASF23:
	.string	"__SIZEOF_SIZE_T__ 8"
.LASF213:
	.string	"__FLT64_EPSILON__ 2.22044604925031308084726333618164062e-16F64"
.LASF485:
	.string	"DBG_VNREF DBG_MODE(12)"
.LASF326:
	.string	"__SEG_GS 1"
.LASF333:
	.string	"__DECIMAL_BID_FORMAT__ 1"
.LASF588:
	.string	"EUNATCH 49"
.LASF314:
	.string	"__ATOMIC_HLE_RELEASE 131072"
.LASF61:
	.string	"__UINT_FAST8_TYPE__ unsigned char"
.LASF492:
	.string	"DBG_EXEC DBG_MODE(19)"
.LASF105:
	.string	"__UINT64_MAX__ 0xffffffffffffffffUL"
.LASF626:
	.string	"EUSERS 87"
.LASF311:
	.string	"__SIZEOF_FLOAT80__ 16"
.LASF449:
	.string	"PROC_MAX_COUNT 65536"
.LASF451:
	.string	"PID_IDLE 0"
.LASF782:
	.string	"PROC_DEAD"
.LASF121:
	.string	"__UINT16_C(c) c"
.LASF206:
	.string	"__FLT64_MIN_EXP__ (-1021)"
.LASF683:
	.string	"uint64_t"
.LASF512:
	.string	"DEBUG_EXIT spinlock_unlock(&debug_lock); if (apic_initialized()) intr_setipl(__ipl);"
.LASF284:
	.string	"__NO_INLINE__ 1"
.LASF785:
	.string	"vnode"
.LASF584:
	.string	"EL2NSYNC 45"
.LASF250:
	.string	"__FLT64X_MAX_EXP__ 16384"
.LASF7:
	.string	"__ATOMIC_RELAXED 0"
.LASF790:
	.string	"curthr"
.LASF347:
	.string	"MAX(a,b) ((a) > (b) ? (a) : (b))"
.LASF173:
	.string	"__DBL_HAS_INFINITY__ 1"
.LASF95:
	.string	"__SIG_ATOMIC_MAX__ 0x7fffffff"
.LASF821:
	.string	"sched_wakeup_on"
.LASF290:
	.string	"__GCC_ATOMIC_BOOL_LOCK_FREE 2"
.LASF562:
	.string	"ENFILE 23"
.LASF62:
	.string	"__UINT_FAST16_TYPE__ long unsigned int"
.LASF432:
	.string	"GDB_PML4_PHYSADDR(pml4,v) (pml4->phys[PML4E(v)] & PAGE_MASK)"
.LASF543:
	.string	"EINTR 4"
.LASF593:
	.string	"EXFULL 54"
.LASF20:
	.string	"__SIZEOF_FLOAT__ 4"
.LASF622:
	.string	"ELIBEXEC 83"
.LASF270:
	.string	"__DEC64_MIN__ 1E-383DD"
.LASF606:
	.string	"ENOLINK 67"
.LASF625:
	.string	"ESTRPIPE 86"
.LASF167:
	.string	"__DBL_DECIMAL_DIG__ 17"
.LASF715:
	.string	"km_holder"
.LASF375:
	.string	"PAGE_ALIGNED(x) (!PAGE_OFFSET(x))"
.LASF335:
	.string	"__SMP__ 1"
.LASF631:
	.string	"ENOPROTOOPT 92"
.LASF435:
	.string	"GET_CSD(core,type,name) ((type *)(csd_vaddr_table[(core)] + PAGE_OFFSET(&(name))))"
.LASF99:
	.string	"__INT16_MAX__ 0x7fff"
.LASF389:
	.string	"PAGE_SAME_2MB(x,y) (PAGE_ALIGN_DOWN_2MB(x) == PAGE_ALIGN_DOWN_2MB(y))"
.LASF545:
	.string	"ENXIO 6"
.LASF339:
	.string	"SPINLOCK_INITIALIZER(lock) { .s_locked = 0 }"
.LASF83:
	.string	"__INT_WIDTH__ 32"
.LASF532:
	.string	"INTR_KEYBOARD 0xe0"
.LASF397:
	.string	"PAGE_SAME_1GB(x,y) (PAGE_ALIGN_DOWN_1GB(x) == PAGE_ALIGN_DOWN_1GB(y))"
.LASF218:
	.string	"__FLT128_MANT_DIG__ 113"
.LASF142:
	.string	"__GCC_IEC_559_COMPLEX 2"
.LASF97:
	.string	"__SIG_ATOMIC_WIDTH__ 32"
.LASF604:
	.string	"ENOPKG 65"
.LASF819:
	.string	"spinlock_unlock"
.LASF769:
	.string	"kc_lock"
.LASF405:
	.string	"VMMAP_DIR_LOHI 1"
.LASF410:
	.string	"PT_WRITE_THROUGH 0x008"
.LASF771:
	.string	"core_t"
.LASF300:
	.string	"__GCC_ATOMIC_POINTER_LOCK_FREE 2"
.LASF305:
	.string	"__SIZEOF_WINT_T__ 4"
.LASF709:
	.string	"tq_list"
.LASF602:
	.string	"ENOSR 63"
.LASF0:
	.string	"__STDC__ 1"
.LASF387:
	.string	"PAGE_OFFSET_2MB(x) (((uintptr_t)(x)) & ~PAGE_MASK_2MB)"
.LASF734:
	.string	"vmmap"
.LASF39:
	.string	"__CHAR32_TYPE__ unsigned int"
.LASF201:
	.string	"__FLT32_HAS_DENORM__ 1"
.LASF437:
	.string	"DEFAULT_STACK_SIZE (DEFAULT_STACK_SIZE_PAGES << PAGE_SHIFT)"
.LASF574:
	.string	"EDEADLK 35"
.LASF48:
	.string	"__UINT64_TYPE__ long unsigned int"
.LASF22:
	.string	"__SIZEOF_LONG_DOUBLE__ 16"
.LASF478:
	.string	"DBG_TEMP DBG_MODE(5)"
.LASF753:
	.string	"p_start_brk"
.LASF33:
	.string	"__PTRDIFF_TYPE__ long int"
.LASF225:
	.string	"__FLT128_MAX__ 1.18973149535723176508575932662800702e+4932F128"
.LASF583:
	.string	"ECHRNG 44"
.LASF744:
	.string	"p_list_link"
.LASF211:
	.string	"__FLT64_MAX__ 1.79769313486231570814527423731704357e+308F64"
.LASF523:
	.string	"KASSERT_GREATER(l,r) KASSERT_GENERIC(l, r, greaterthan, \">\")"
.LASF229:
	.string	"__FLT128_HAS_DENORM__ 1"
.LASF476:
	.string	"DBG_SCHED DBG_MODE(3)"
.LASF611:
	.string	"EMULTIHOP 72"
.LASF73:
	.string	"__LONG_MAX__ 0x7fffffffffffffffL"
.LASF16:
	.string	"__SIZEOF_INT__ 4"
.LASF525:
	.string	"KASSERT_GREQ(l,r) KASSERT_GENERIC(l, r, greaterthaneq, \">=\")"
.LASF728:
	.string	"kt_plink"
.LASF350:
	.string	"QUOTE_BY_VALUE(x) QUOTE_BY_NAME(x)"
.LASF384:
	.string	"PAGE_MASK_2MB (0xffffffffffffffff << PAGE_SHIFT_2MB)"
.LASF799:
	.string	"held"
.LASF775:
	.string	"KT_RUNNABLE"
.LASF79:
	.string	"__PTRDIFF_MAX__ 0x7fffffffffffffffL"
.LASF140:
	.string	"__UINTPTR_MAX__ 0xffffffffffffffffUL"
.LASF544:
	.string	"EIO 5"
.LASF657:
	.string	"ENOTNAM 118"
.LASF307:
	.string	"__amd64 1"
.LASF515:
	.string	"dbgq(mode,...) do { if (dbg_active(mode)) { DEBUG_ENTER dbg_print(\"%s\", dbg_color(mode)); dbg_print(\"C%ld P%ld \", curcore.kc_id, curproc ? curproc->p_pid : -1L); dbg_print(__VA_ARGS__); dbg_print(\"%s\", _NORMAL_); DEBUG_EXIT } } while (0)"
.LASF184:
	.string	"__LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L"
.LASF331:
	.string	"__unix__ 1"
.LASF576:
	.string	"ENOLCK 37"
.LASF735:
	.string	"vmm_list"
.LASF716:
	.string	"km_lock"
.LASF530:
	.string	"INTR_PAGE_FAULT 0x0e"
.LASF176:
	.string	"__LDBL_DIG__ 18"
.LASF446:
	.string	"NFILES 32"
.LASF509:
	.ascii	"DBG_TAB {\"error\", DBG_ERROR, _BWHITE_}, {\"temp\", DBG_TEM"
	.ascii	"P, _NORMAL_}, {\"print\", DBG_PRINT, _NORMAL_}, {\"test\", D"
	.ascii	"BG_TEST, _RED_}, {\"testpass\", DBG_TESTPASS, _GREEN_}, {\"t"
	.ascii	"estfail\", DBG_TESTFAIL, _RED_}, {\"proc\", DBG_PROC, _BLUE_"
	.ascii	"}, {\"thr\", DBG_THR, _CYAN_}, {\"sched\", DBG_SCHED, _GREEN"
	.ascii	"_}, {\"init\", DBG_INIT, _NORMAL_}, {\"term\", DBG_TERM, _BM"
	.ascii	"AGENTA_}, {\"disk\", DBG_DISK, _YELLOW_}, {\"memdev\", DBG_M"
	.ascii	"EMDEV, _BBLUE_}, {\"vfs\", DBG_VFS, _WHITE_}, {\"fref\", DBG"
	.ascii	"_FREF, _MAGENTA_}, {\"vnref\", DBG_VNREF, _CYAN_}, {\"s5fs\""
	.ascii	", DBG_S5FS, _BRED_}, {\"pframe\", DBG_PFRAME, _BMAGENTA_}, {"
	.ascii	"\"anon\", DBG_ANON, _WHITE_}, {\"vmmap\", DBG_VMMAP, _BGREEN"
	.ascii	"_}, {\"fork\", DBG_FORK, _BYELLOW_}, {\"brk\", DBG_BRK, _YEL"
	.ascii	"LOW_}, {\"exec\", DBG_EXEC, _BRED_}, {\"elf\", DBG_ELF, _BGR"
	.ascii	"EEN_}, {\"pgtbl\", DBG_PGTBL, _BBLUE_}, {\"osyscall\", DBG_O"
	.ascii	"SYSCALL, _BMAGENTA_}, {\"vm\", DBG_VM, _RED_}, {\"syscall\","
	.ascii	" DBG_SYSCALL, _RED_}, "
	.string	"{\"intr\", DBG_INTR, _BRED_}, {\"kmalloc\", DBG_KMALLOC, _MAGENTA_}, {\"pagealloc\", DBG_PAGEALLOC, _WHITE_}, {\"kb\", DBG_KB, _BLUE_}, {\"core\", DBG_CORE, _GREEN_}, {\"mm\", DBG_MM, _RED_}, {\"user\", DBG_USER, _BYELLOW_}, {\"all\", DBG_ALL, _NORMAL_}, { NULL, 0, NULL }"
.LASF78:
	.string	"__WINT_MIN__ 0U"
.LASF205:
	.string	"__FLT64_DIG__ 15"
.LASF272:
	.string	"__DEC64_EPSILON__ 1E-15DD"
.LASF155:
	.string	"__FLT_MIN__ 1.17549435082228750796873653722224568e-38F"
.LASF108:
	.string	"__INT_LEAST8_WIDTH__ 8"
.LASF50:
	.string	"__INT_LEAST16_TYPE__ short int"
.LASF549:
	.string	"ECHILD 10"
.LASF415:
	.string	"PT_GLOBAL 0x100"
.LASF679:
	.string	"short unsigned int"
.LASF202:
	.string	"__FLT32_HAS_INFINITY__ 1"
.LASF828:
	.string	"GNU C99 7.5.0 -march=x86-64 -m64 -mno-red-zone -mcmodel=large -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-sse4 -mno-sse4a -mno-3dnow -mno-avx -mno-avx2 -g3 -gdwarf-3 -std=c99 -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -fsigned-char"
.LASF580:
	.string	"EWOULDBLOCK EAGAIN"
.LASF177:
	.string	"__LDBL_MIN_EXP__ (-16381)"
.LASF479:
	.string	"DBG_KMALLOC DBG_MODE(6)"
.LASF170:
	.string	"__DBL_EPSILON__ ((double)2.22044604925031308084726333618164062e-16L)"
.LASF312:
	.string	"__SIZEOF_FLOAT128__ 16"
.LASF402:
	.string	"PAGE_ALIGN_UP_512GB(x) (PAGE_ALIGN_DOWN_512GB((x)-1) + PAGE_SIZE_512GB)"
.LASF502:
	.string	"DBG_TESTFAIL DBG_MODE(32)"
.LASF386:
	.string	"PAGE_ALIGN_UP_2MB(x) (PAGE_ALIGN_DOWN_2MB((x)-1) + PAGE_SIZE_2MB)"
.LASF122:
	.string	"__UINT_LEAST32_MAX__ 0xffffffffU"
.LASF727:
	.string	"kt_lock"
.LASF812:
	.string	"equals"
.LASF703:
	.string	"s_locked"
.LASF688:
	.string	"setup_end"
.LASF607:
	.string	"EADV 68"
.LASF232:
	.string	"__FLT32X_MANT_DIG__ 53"
.LASF723:
	.string	"kt_proc"
.LASF663:
	.string	"EMEDIUMTYPE 124"
.LASF382:
	.string	"PAGE_SHIFT_2MB 21"
.LASF572:
	.string	"EDOM 33"
.LASF585:
	.string	"EL3HLT 46"
.LASF110:
	.string	"__INT16_C(c) c"
.LASF537:
	.string	"IPL_LOW 0"
.LASF247:
	.string	"__FLT64X_DIG__ 18"
.LASF685:
	.string	"uintptr_t"
.LASF783:
	.string	"proc_state_t"
.LASF268:
	.string	"__DEC64_MIN_EXP__ (-382)"
.LASF482:
	.string	"DBG_TERM DBG_MODE(9)"
.LASF656:
	.string	"EUCLEAN 117"
.LASF11:
	.string	"__ATOMIC_ACQ_REL 4"
.LASF739:
	.string	"p_name"
.LASF590:
	.string	"EL2HLT 51"
.LASF328:
	.string	"__linux 1"
.LASF164:
	.string	"__DBL_MIN_10_EXP__ (-307)"
.LASF131:
	.string	"__INT_FAST32_WIDTH__ 64"
.LASF171:
	.string	"__DBL_DENORM_MIN__ ((double)4.94065645841246544176568792868221372e-324L)"
.LASF618:
	.string	"ELIBACC 79"
.LASF672:
	.string	"DEBUG_DEADLOCKS 1"
.LASF647:
	.string	"ESHUTDOWN 108"
.LASF535:
	.string	"INTR_APICERR 0xff"
.LASF418:
	.string	"PML4E(x) ((((uintptr_t)(x)) >> 39) & INDEX_MASK)"
.LASF390:
	.string	"PAGE_SHIFT_1GB 30"
.LASF161:
	.string	"__DBL_MANT_DIG__ 53"
.LASF810:
	.string	"lessthan"
.LASF72:
	.string	"__INT_MAX__ 0x7fffffff"
.LASF124:
	.string	"__UINT_LEAST64_MAX__ 0xffffffffffffffffUL"
.LASF615:
	.string	"ENOTUNIQ 76"
.LASF360:
	.string	"list_iterate_reverse(list,var,type,member) for (type *var = list_tail(list, type, member), *__next_ ##var = list_prev(var, type, member); &var->member != (list); var = __next_ ##var, __next_ ##var = list_prev(var, type, member))"
.LASF697:
	.string	"kernel_start_init"
.LASF612:
	.string	"EDOTDOT 73"
.LASF747:
	.string	"p_state"
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
