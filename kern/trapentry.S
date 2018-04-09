/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
	TRAPHANDLER_NOEC(TRAP_0, 0)
	TRAPHANDLER_NOEC(TRAP_1, 1)
	TRAPHANDLER_NOEC(TRAP_2, 2)     //??
	TRAPHANDLER_NOEC(TRAP_3, 3)
	TRAPHANDLER_NOEC(TRAP_4, 4)
	TRAPHANDLER_NOEC(TRAP_5, 5)
	TRAPHANDLER_NOEC(TRAP_6, 6)
	TRAPHANDLER_NOEC(TRAP_7, 7)
	TRAPHANDLER(TRAP_8, 8)
	TRAPHANDLER_NOEC(TRAP_9, 9)
	TRAPHANDLER(TRAP_10, 10)
	TRAPHANDLER(TRAP_11, 11)
	TRAPHANDLER(TRAP_12, 12)
	TRAPHANDLER(TRAP_13, 13)
	TRAPHANDLER(TRAP_14, 14)
	TRAPHANDLER(TRAP_15, 15)    // 
	TRAPHANDLER_NOEC(TRAP_16, 16)
	TRAPHANDLER(TRAP_17, 17)
	TRAPHANDLER_NOEC(TRAP_18, 18)
	TRAPHANDLER_NOEC(TRAP_19, 19)
/*
	TRAPHANDLER(TRAP_20, 20)    //
	TRAPHANDLER(TRAP_21, 21)
	TRAPHANDLER(TRAP_22, 22)
	TRAPHANDLER(TRAP_23, 23)
	TRAPHANDLER(TRAP_24, 24)
	TRAPHANDLER(TRAP_25, 25)
	TRAPHANDLER(TRAP_26, 26)
	TRAPHANDLER(TRAP_27, 27)
	TRAPHANDLER(TRAP_28, 28)
	TRAPHANDLER(TRAP_29, 29)
	TRAPHANDLER(TRAP_30, 30)
	TRAPHANDLER(TRAP_31, 31)
*/



.globl sysenter_handler;
.type sysenter_handler, @function;
.align 2;
sysenter_handler:
/*
 * Lab 3: Your code here for system call handling
 */
	pushl %esi
	pushl %ebp
	pushl $0
	pushl %edi
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %eax
	call syscall
	add $24, %esp
	popl %ecx
	popl %edx
	sysexit


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushl %ds
	pushl %es
	pushal
	pushl $GD_KD
	pushl $GD_KD
	popl %ds
	popl %es
	pushl %esp
	call trap
	popl %esp
	popal
	popl %es
	popl %ds
	addl $0x8, %esp
	iret

