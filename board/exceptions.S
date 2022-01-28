/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "macro.h"
#include "../include/autoconf.h"
	.extern	fLib_IRQHandle
	.extern	fLib_FIQHandle

_pHandleIRQ:	.word fLib_IRQHandle
_pHandleFIQ:	.word fLib_FIQHandle

/*
 * Enter Exception.
 * This will save the processor state that is ELR/X0~X30
 * to the stack frame.
 */
.macro	exception_entry
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!

	/* Could be running at EL3/EL2/EL1 */
	switch_el x11, 3f, 2f, 1f
3:	mrs	x1, esr_el3
	mrs	x2, elr_el3
	b	0f
2:	mrs	x1, esr_el2
	mrs	x2, elr_el2
	b	0f
1:	mrs	x1, esr_el1
	mrs	x2, elr_el1
0:
	stp	x2, x0, [sp, #-16]!
	mov	x0, sp
.endm

/*
 * Exception vectors.
 */
	.align	11
	.globl	vectors
vectors:
.set VBAR, vectors

	.align	7
	b	_do_bad_sync	/* Current EL Synchronous Thread */

	.align	7
	b	_do_bad_irq	/* Current EL IRQ Thread */

	.align	7
	b	_do_bad_fiq	/* Current EL FIQ Thread */

	.align	7
	b	_do_bad_error	/* Current EL Error Thread */

	.align	7
	b	_do_sync	/* Current EL Synchronous Handler */

	.align	7
	b	_do_irq		/* Current EL IRQ Handler */

	.align	7
	b	_do_fiq		/* Current EL FIQ Handler */

	.align	7
	b	_do_error	/* Current EL Error Handler */


_do_bad_sync:
	exception_entry
//	bl	do_bad_sync
	bl	_do_bad_sync
	b	exception_exit

_do_bad_irq:
	exception_entry
//	bl	do_bad_irq
	bl	_do_bad_irq
	b	exception_exit

_do_bad_fiq:
	exception_entry
//	bl	do_bad_fiq
	bl	_do_bad_fiq
	b	exception_exit

_do_bad_error:
	exception_entry
//	bl	do_bad_error
	bl	_do_bad_error
	b	exception_exit

_do_sync:
	exception_entry
//	bl	do_sync
	bl	_do_sync
	b	exception_exit

_do_irq:
	exception_entry
//	bl	do_irq
//       bl     _pHandleIRQ
       bl     DispatchIRQ
	b	exception_exit

_do_fiq:
	exception_entry
//	bl	do_fiq
	bl	_do_fiq
	b	exception_exit

_do_error:
	exception_entry
//	bl	do_error
	bl	_do_error
	b	exception_exit

exception_exit:
	ldp	x2, x0, [sp],#16
	switch_el x11, 3f, 2f, 1f
3:	msr	elr_el3, x2
	b	0f
2:	msr	elr_el2, x2
	b	0f
1:	msr	elr_el1, x2
0:
	ldp	x1, x2, [sp],#16
	ldp	x3, x4, [sp],#16
	ldp	x5, x6, [sp],#16
	ldp	x7, x8, [sp],#16
	ldp	x9, x10, [sp],#16
	ldp	x11, x12, [sp],#16
	ldp	x13, x14, [sp],#16
	ldp	x15, x16, [sp],#16
	ldp	x17, x18, [sp],#16
	ldp	x19, x20, [sp],#16
	ldp	x21, x22, [sp],#16
	ldp	x23, x24, [sp],#16
	ldp	x25, x26, [sp],#16
	ldp	x27, x28, [sp],#16
	ldp	x29, x30, [sp],#16
	eret

#ifdef CONFIG_FREERTOS

/******************************************************************************
 * Vector table to use when FreeRTOS is running.
 *****************************************************************************/
.set	FREERTOS_VBAR, (VBAR+0x1000)

.globl _freertos_vector_table
.org(FREERTOS_VBAR)
_freertos_vector_table:
	b	FreeRTOS_SWI_Handler

.org (FREERTOS_VBAR + 0x80)
  b _do_irq
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x100)
	b	.

.org (FREERTOS_VBAR + 0x180)
	b	.

.org (FREERTOS_VBAR + 0x200)
	b	FreeRTOS_SWI_Handler

.org (FREERTOS_VBAR + 0x280)
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x300)
	b	.

.org (FREERTOS_VBAR + 0x380)
	b	.

.org (FREERTOS_VBAR + 0x400)
	b	.

.org (FREERTOS_VBAR + 0x480)
	b	.

.org (FREERTOS_VBAR + 0x500)
	b	.

.org (FREERTOS_VBAR + 0x580)
	b	.

.org (FREERTOS_VBAR + 0x600)
	b	.

.org (FREERTOS_VBAR + 0x680)
	b	.

.org (FREERTOS_VBAR + 0x700)
	b	.

.org (FREERTOS_VBAR + 0x780)
	b	.

.org (FREERTOS_VBAR + 0x800)
#endif
