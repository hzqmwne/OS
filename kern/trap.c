#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>
#include <kern/time.h>

static struct Taskstate ts;

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};

extern void TRAP_0();
extern void TRAP_1();
extern void TRAP_2();
extern void TRAP_3();
extern void TRAP_4();
extern void TRAP_5();
extern void TRAP_6();
extern void TRAP_7();
extern void TRAP_8();
extern void TRAP_9();
extern void TRAP_10();
extern void TRAP_11();
extern void TRAP_12();
extern void TRAP_13();
extern void TRAP_14();
extern void TRAP_15();    // 
extern void TRAP_16();
extern void TRAP_17();
extern void TRAP_18();
extern void TRAP_19();

extern void IRQ_0();
extern void IRQ_1();
extern void IRQ_2();
extern void IRQ_3();
extern void IRQ_4();
extern void IRQ_5();
extern void IRQ_6();
extern void IRQ_7();
extern void IRQ_8();
extern void IRQ_9();
extern void IRQ_10();
extern void IRQ_11();
extern void IRQ_12();
extern void IRQ_13();
extern void IRQ_14();
extern void IRQ_15();

extern void TRAP_48();

extern void sysenter_handler();

static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
		return "Hardware Interrupt";
	return "(unknown trap)";
}


void
trap_init(void)
{
	extern struct Segdesc gdt[];

	// LAB 3: Your code here.
	uint32_t cs = GD_KT;    // also PROT_MODE_CSEG in boot.S; GD_KT is in memlayout.h
	SETGATE(idt[0], 0, cs, TRAP_0, 0);
	SETGATE(idt[1], 0, cs, TRAP_1, 0);
	SETGATE(idt[2], 0, cs, TRAP_2, 0);
	SETGATE(idt[3], 0, cs, TRAP_3, 3);
	SETGATE(idt[4], 0, cs, TRAP_4, 0);
	SETGATE(idt[5], 0, cs, TRAP_5, 0);
	SETGATE(idt[6], 0, cs, TRAP_6, 0);
	SETGATE(idt[7], 0, cs, TRAP_7, 0);
	SETGATE(idt[8], 0, cs, TRAP_8, 0);
	SETGATE(idt[9], 0, cs, TRAP_9, 0);
	SETGATE(idt[10], 0, cs, TRAP_10, 0);
	SETGATE(idt[11], 0, cs, TRAP_11, 0);
	SETGATE(idt[12], 0, cs, TRAP_12, 0);
	SETGATE(idt[13], 0, cs, TRAP_13, 0);
	SETGATE(idt[14], 0, cs, TRAP_14, 0);
	SETGATE(idt[15], 0, cs, TRAP_15, 0);
	SETGATE(idt[16], 0, cs, TRAP_16, 0);
	SETGATE(idt[17], 0, cs, TRAP_17, 0);
	SETGATE(idt[18], 0, cs, TRAP_18, 0);
	SETGATE(idt[19], 0, cs, TRAP_19, 0);

	SETGATE(idt[IRQ_OFFSET+0], 0, cs, IRQ_0, 0);    // IRQ_OFFSET is 32
	SETGATE(idt[IRQ_OFFSET+1], 0, cs, IRQ_1, 0);
	SETGATE(idt[IRQ_OFFSET+2], 0, cs, IRQ_2, 0);
	SETGATE(idt[IRQ_OFFSET+3], 0, cs, IRQ_3, 0);
	SETGATE(idt[IRQ_OFFSET+4], 0, cs, IRQ_4, 0);
	SETGATE(idt[IRQ_OFFSET+5], 0, cs, IRQ_5, 0);
	SETGATE(idt[IRQ_OFFSET+6], 0, cs, IRQ_6, 0);
	SETGATE(idt[IRQ_OFFSET+7], 0, cs, IRQ_7, 0);
	SETGATE(idt[IRQ_OFFSET+8], 0, cs, IRQ_8, 0);
	SETGATE(idt[IRQ_OFFSET+9], 0, cs, IRQ_9, 0);
	SETGATE(idt[IRQ_OFFSET+10], 0, cs, IRQ_10, 0);
	SETGATE(idt[IRQ_OFFSET+11], 0, cs, IRQ_11, 0);
	SETGATE(idt[IRQ_OFFSET+12], 0, cs, IRQ_12, 0);
	SETGATE(idt[IRQ_OFFSET+13], 0, cs, IRQ_13, 0);
	SETGATE(idt[IRQ_OFFSET+14], 0, cs, IRQ_14, 0);
	SETGATE(idt[IRQ_OFFSET+15], 0, cs, IRQ_15, 0);

	SETGATE(idt[48], 0, cs, TRAP_48, 3);

	// Per-CPU setup 
	trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void
trap_init_percpu(void)
{
	// The example code here sets up the Task State Segment (TSS) and
	// the TSS descriptor for CPU 0. But it is incorrect if we are
	// running on other CPUs because each CPU has its own kernel stack.
	// Fix the code so that it works for all CPUs.
	//
	// Hints:
	//   - The macro "thiscpu" always refers to the current CPU's
	//     struct Cpu;
	//   - The ID of the current CPU is given by cpunum() or
	//     thiscpu->cpu_id;
	//   - Use "thiscpu->cpu_ts" as the TSS for the current CPU,
	//     rather than the global "ts" variable;
	//   - Use gdt[(GD_TSS0 >> 3) + i] for CPU i's TSS descriptor;
	//   - You mapped the per-CPU kernel stacks in mem_init_mp()
	//
	// ltr sets a 'busy' flag in the TSS selector, so if you
	// accidentally load the same TSS on more than one CPU, you'll
	// get a triple fault.  If you set up an individual CPU's TSS
	// wrong, you may not get a fault until you try to return from
	// user space on that CPU.
	//
	// LAB 4: Your code here:

	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	struct Cpu *this_cpu = thiscpu;
	int i = this_cpu->cpu_id;
	this_cpu->cpu_ts.ts_esp0 = KSTACKTOP - i*(KSTKSIZE+KSTKGAP);
	this_cpu->cpu_ts.ts_ss0 = GD_KD;

	// Initialize the TSS slot of the gdt.
	gdt[(GD_TSS0 >> 3) + i] = SEG16(STS_T32A, (uint32_t) (&this_cpu->cpu_ts),
					sizeof(struct Taskstate), 0);
	gdt[(GD_TSS0 >> 3) + i].sd_s = 0;

	// Load the TSS selector (like other segment selectors, the
	// bottom three bits are special; we leave them 0)
	ltr(GD_TSS0 + (i << 3));

	// Load the IDT
	lidt(&idt_pd);

	uint32_t cs = GD_KT;    // also PROT_MODE_CSEG in boot.S; GD_KT is in memlayout.h
	wrmsr(0x174, cs, 0);    /* SYSENTER_CS_MSR */
	wrmsr(0x175, KSTACKTOP-i*(KSTKSIZE+KSTKGAP), 0);    /* SYSENTER_ESP_MSR */
	wrmsr(0x176, (uint32_t)sysenter_handler, 0);    /* SYSENTER_EIP_MSR */
}

void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p from CPU %d\n", tf, cpunum());
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	// If this trap was a page fault that just happened
	// (so %cr2 is meaningful), print the faulting linear address.
	if (tf == last_tf && tf->tf_trapno == T_PGFLT)
		cprintf("  cr2  0x%08x\n", rcr2());
	cprintf("  err  0x%08x", tf->tf_err);
	// For page faults, print decoded fault error code:
	// U/K=fault occurred in user/kernel mode
	// W/R=a write/read caused the fault
	// PR=a protection violation caused the fault (NP=page not present).
	if (tf->tf_trapno == T_PGFLT)
		cprintf(" [%s, %s, %s]\n",
			tf->tf_err & 4 ? "user" : "kernel",
			tf->tf_err & 2 ? "write" : "read",
			tf->tf_err & 1 ? "protection" : "not-present");
	else
		cprintf("\n");
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	if ((tf->tf_cs & 3) != 0) {
		cprintf("  esp  0x%08x\n", tf->tf_esp);
		cprintf("  ss   0x----%04x\n", tf->tf_ss);
	}
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void
trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here.
	if(tf->tf_trapno <= 19 || tf->tf_trapno == T_SYSCALL) {
		switch(tf->tf_trapno) {
			case T_DEBUG: case T_BRKPT: {    // 1 3
				monitor(tf);
				return;
				break;
			}
			case T_PGFLT: {    // 14
				page_fault_handler(tf);
				return;
				break;
			}
			case T_SYSCALL: {    // 48, 0x30
				uint32_t syscallno = tf->tf_regs.reg_eax;
				uint32_t a1 = tf->tf_regs.reg_ebx;
				uint32_t a2 = tf->tf_regs.reg_ecx;
				uint32_t a3 = tf->tf_regs.reg_edx;
				uint32_t a4 = tf->tf_regs.reg_esi;
				uint32_t a5 = tf->tf_regs.reg_edi;
				int32_t r = syscall_dispatch(syscallno, a1, a2, a3, a4, a5);
				tf->tf_regs.reg_eax = r;
				return;
				break;
			}
			default: {
				break;
			}
		}
	}

	// Handle spurious interrupts
	// The hardware sometimes raises these because of noise on the
	// IRQ line or other reasons. We don't care.
	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
		cprintf("Spurious interrupt on irq 7\n");
		print_trapframe(tf);
		return;
	}

	// Handle clock interrupts. Don't forget to acknowledge the
	// interrupt using lapic_eoi() before calling the scheduler!
	// LAB 4: Your code here.
	if (tf->tf_trapno == IRQ_OFFSET + IRQ_TIMER) {
		lapic_eoi();
		sched_yield();
		return;
	}
 
	// Add time tick increment to clock interrupts.
	// Be careful! In multiprocessors, clock interrupts are
	// triggered on every CPU.
	// LAB 6: Your code here.


	// Unexpected trap: The user process or the kernel has a bug.
	print_trapframe(tf);
	if (tf->tf_cs == GD_KT)
		panic("unhandled trap in kernel");
	else {
		env_destroy(curenv);
		return;
	}
}

void
trap(struct Trapframe *tf)
{
	// The environment may have set DF and some versions
	// of GCC rely on DF being clear
	asm volatile("cld" ::: "cc");

	// Halt the CPU if some other CPU has called panic()
	extern char *panicstr;
	if (panicstr)
		asm volatile("hlt");

	// Check that interrupts are disabled.  If this assertion
	// fails, DO NOT be tempted to fix it by inserting a "cli" in
	// the interrupt path.
	assert(!(read_eflags() & FL_IF));

	if ((tf->tf_cs & 3) == 3) {
		// Trapped from user mode.
		// Acquire the big kernel lock before doing any
		// serious kernel work.
		// LAB 4: Your code here.
		lock_kernel();

		assert(curenv);

		// Garbage collect if current enviroment is a zombie
		if (curenv->env_status == ENV_DYING) {
			env_free(curenv);
			curenv = NULL;
			sched_yield();
		}

		// Copy trap frame (which is currently on the stack)
		// into 'curenv->env_tf', so that running the environment
		// will restart at the trap point.
		curenv->env_tf = *tf;
		// The trapframe on the stack should be ignored from here on.
		tf = &curenv->env_tf;
	}

	// Record that tf is the last real trapframe so
	// print_trapframe can print some additional information.
	last_tf = tf;

	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);

	// If we made it to this point, then no other environment was
	// scheduled, so we should return to the current environment
	// if doing so makes sense.
	if (curenv && curenv->env_status == ENV_RUNNING)
		env_run(curenv);
	else
		sched_yield();
}


void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.

	// LAB 3: Your code here.
	if((tf->tf_cs & 3) == 0) {    // see 3.4.2 Segment Selectors, Intel manual
		print_trapframe(tf);
		panic("page fault happens in kernel mode");
	}

	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Call the environment's page fault upcall, if one exists.  Set up a
	// page fault stack frame on the user exception stack (below
	// UXSTACKTOP), then branch to curenv->env_pgfault_upcall.
	//
	// The page fault upcall might cause another page fault, in which case
	// we branch to the page fault upcall recursively, pushing another
	// page fault stack frame on top of the user exception stack.
	//
	// The trap handler needs one word of scratch space at the top of the
	// trap-time stack in order to return.  In the non-recursive case, we
	// don't have to worry about this because the top of the regular user
	// stack is free.  In the recursive case, this means we have to leave
	// an extra word between the current top of the exception stack and
	// the new stack frame because the exception stack _is_ the trap-time
	// stack.
	//
	// If there's no page fault upcall, the environment didn't allocate a
	// page for its exception stack or can't write to it, or the exception
	// stack overflows, then destroy the environment that caused the fault.
	// Note that the grade script assumes you will first check for the page
	// fault upcall and print the "user fault va" message below if there is
	// none.  The remaining three checks can be combined into a single test.
	//
	// Hints:
	//   user_mem_assert() and env_run() are useful here.
	//   To change what the user environment runs, modify 'curenv->env_tf'
	//   (the 'tf' variable points at 'curenv->env_tf').

	// LAB 4: Your code here.
	struct Env *cur_env = curenv;
	if(cur_env->env_pgfault_upcall != NULL) {
		user_mem_assert(curenv, (void *)(UXSTACKTOP-1), 1, PTE_U|PTE_W|PTE_P);
		uint32_t old_tf_esp = cur_env->env_tf.tf_esp;
		uint32_t new_tf_esp = UXSTACKTOP;
		if(old_tf_esp >= UXSTACKTOP-PGSIZE && old_tf_esp < UXSTACKTOP) {
			new_tf_esp = old_tf_esp - 4;
			*(int *)new_tf_esp = 0;
		}
		new_tf_esp -= sizeof(struct UTrapframe);
		struct UTrapframe *utf = (struct UTrapframe *)(new_tf_esp);
		utf->utf_fault_va = fault_va;
		utf->utf_err = cur_env->env_tf.tf_err;
		utf->utf_regs = cur_env->env_tf.tf_regs;
		utf->utf_eip = cur_env->env_tf.tf_eip;
		utf->utf_eflags = cur_env->env_tf.tf_eflags;
		utf->utf_esp = old_tf_esp;

		cur_env->env_tf.tf_eip = (uintptr_t)cur_env->env_pgfault_upcall;
		cur_env->env_tf.tf_esp = new_tf_esp;
		return;
	}

	// Destroy the environment that caused the fault.
	cprintf("[%08x] user fault va %08x ip %08x\n",
		curenv->env_id, fault_va, tf->tf_eip);
	print_trapframe(tf);
	env_destroy(curenv);
}

