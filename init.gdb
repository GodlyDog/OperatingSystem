define restore_regs
    set $tmp_regs=$arg0
	set $rip=$tmp_regs->r_rip
	set $rbp=$tmp_regs->r_rbp
	set $rsp=$tmp_regs->r_rsp
	set $rax=$tmp_regs->r_rax
	set $rbx=$tmp_regs->r_rbx
	set $rcx=$tmp_regs->r_rcx
	set $rdx=$tmp_regs->r_rdx
	set $rsi=$tmp_regs->r_rsi
	set $rdi=$tmp_regs->r_rdi
	set $r8=$tmp_regs->r_r8
	set $r9=$tmp_regs->r_r9
	set $r10=$tmp_regs->r_r10
	set $r11=$tmp_regs->r_r11
	set $r12=$tmp_regs->r_r12
	set $r13=$tmp_regs->r_r13
	set $r14=$tmp_regs->r_r14
	set $r15=$tmp_regs->r_r15
	set $rflags=$tmp_regs->r_rflags
end

define restore_context
    set $tmp_rip=$arg0->c_rip
    set $tmp_rbp=$arg0->c_rbp
    set $tmp_rsp=$arg0->c_rsp
    frame 0
    set $rip=$tmp_rip
    set $rbp=$tmp_rbp
    set $rsp=$tmp_rsp
end

handle SIGSEGV nostop noprint nopass

break dbg_panic_halt
break entry

source ./python/weenix/debug_userland.py

continue
