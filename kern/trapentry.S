/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


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
TRAPHANDLER_NOEC(h_divzero, 0x0)
TRAPHANDLER_NOEC(h_dbgexcept, 0x1)
TRAPHANDLER_NOEC(h_nmiinterrupt, 0x2)
TRAPHANDLER_NOEC(h_brkpoint, 0x3)
TRAPHANDLER_NOEC(h_overflow, 0x4)
TRAPHANDLER_NOEC(h_bdcheck, 0x5)
TRAPHANDLER_NOEC(h_invalidop, 0x6)
TRAPHANDLER_NOEC(h_dnavail, 0x7)
TRAPHANDLER(h_dbfault, 0x8)
TRAPHANDLER_NOEC(h_coprosegovr, 0x9)
TRAPHANDLER(h_invalidtss, 0xA)
TRAPHANDLER(h_segnotpre, 0xB)
TRAPHANDLER(h_stkexception, 0xC)
TRAPHANDLER(h_gpexception, 0xD)
TRAPHANDLER(h_pagefault, 0xE)
TRAPHANDLER_NOEC(h_fpuerr, 0x10)
TRAPHANDLER(h_alchkexception, 0x11)
TRAPHANDLER_NOEC(h_machinechk, 0x12)
TRAPHANDLER_NOEC(h_simdfp, 0x13)

TRAPHANDLER_NOEC(h_irq0, IRQ_OFFSET+0);
TRAPHANDLER_NOEC(h_irq1, IRQ_OFFSET+1);
TRAPHANDLER_NOEC(h_irq2, IRQ_OFFSET+2);
TRAPHANDLER_NOEC(h_irq3, IRQ_OFFSET+3);
TRAPHANDLER_NOEC(h_irq4, IRQ_OFFSET+4);
TRAPHANDLER_NOEC(h_irq5, IRQ_OFFSET+5);
TRAPHANDLER_NOEC(h_irq6, IRQ_OFFSET+6);
TRAPHANDLER_NOEC(h_irq7, IRQ_OFFSET+7);
TRAPHANDLER_NOEC(h_irq8, IRQ_OFFSET+8);
TRAPHANDLER_NOEC(h_irq9, IRQ_OFFSET+9);
TRAPHANDLER_NOEC(h_irq10, IRQ_OFFSET+10);
TRAPHANDLER_NOEC(h_irq11, IRQ_OFFSET+11);
TRAPHANDLER_NOEC(h_irq12, IRQ_OFFSET+12);
TRAPHANDLER_NOEC(h_irq13, IRQ_OFFSET+13);
TRAPHANDLER_NOEC(h_irq14, IRQ_OFFSET+14);
TRAPHANDLER_NOEC(h_irq15, IRQ_OFFSET+15);

TRAPHANDLER_NOEC(h_syscall, 0x30);

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
    pushw $0
    pushw %ds
    pushw $0
    pushw %es
    pushal
    movw $(GD_KD), %ax
    movw %ax, %ds
    movw %ax, %es
    pushl %esp
    call trap
