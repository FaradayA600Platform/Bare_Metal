/*
****************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:trap.s                                                           *
* Description: Interrupt Trap Handler                                            *
* Author: Fred Chien                                                       *
****************************************************************************
*/

	.extern	fLib_IRQHandle
	.extern	fLib_FIQHandle
	.extern SysSWIHandler

	.globl	fLib_IRQTrap
	.globl	fLib_FIQTrap
	.globl	EnableIRQ
	.globl	DisableIRQ
	.globl	EnableFIQ
	.globl	DisableFIQ
	.globl  fLib_TrapSWI
	.globl	_cli_
	.globl	_sti_

	.text
	.align 0
	.code 32


/* Indirect pointers to IRQ handling routines.*/

_pHandleIRQ:	.word fLib_IRQHandle
_pHandleFIQ:	.word fLib_FIQHandle

fLib_IRQTrap:
        /* Try to recover to old version for CA9 Test!!*/
        /*SUB     r14, r14, #4*/
        /*STMFD   sp!, {r0-r12, lr}*/
        /*MRS     v1, SPSR*/
        /*STMFD   sp!, {v1}*/
        /*LDR     a3, _pHandleIRQ*/
        /*LDR     a3, [a3]*/
        /*MOV     lr, pc*/
        /*MOV     pc,a3*/          /* call high-level handler */
        /*LDMFD   sp!, {v1}*/      /* recover SPSR value from stack */
        /*MSREQ   SPSR_c, v1*/     /* restore the SPSR */
        /*LDMFD   sp!, {r0-r12, pc}^*/   /* Restore saved registers */


	/**
	 * We take the cycle hit of modifying the return address here,
	 * to simplify the code needed to return to the interrupted
	 * thread, possibly performing a context switch on the way.
	 */
	SUB	lr, lr, #4
	/**
	 * At this points IRQs are disabled, so we know that the SPSR
	 * register will not be trashed by another interrupt.
	 */
	stmia	sp, {r0, lr}
	mrs	lr, spsr
	str	lr, [sp, #8]
	/**
	 * sp_irq -> +----------+ small address
	 *           | r0 orig  |
	 *           | lr_irq   | (origianl PC)
	 *           | spsr_irq |
	 *           +----------+ large address
	 */
	/* prepare for SVC32 mode */
	mrs	r0, cpsr
	eor	r0, r0, #1
	msr	spsr_cxsf, r0

	mov	r0, sp		/* r0 = sp_irq */
	adr	lr, SVC_Mode
	movs	pc, lr		/* change to SVC mode */

SVC_Mode:
	/* now we are in SVC mode */
	sub	sp, sp, #(18 * 4)
	stmib	sp, {r1 - r12}

	/**
	 * sp_svc -> +----------+ small address
	 *           |          |
	 *           | r1       |
	 *           | ...      |
	 *           | r12      |
	 *           |          |
	 *           |          |
	 *           |          |
	 *           |          |
	 *           |          |
	 *           +----------+ large address
	 */
	ldmia	r0, {r1 - r3}
	/**
	 * r1 = r0 orig
	 * r2 = lr_irq (original pc)
	 * r3 = spsr_irq
	*/
	add	r5, sp, #(13 * 4)
	add	r0, sp, #(18 * 4)

	str	r1, [sp]
	/**
	 * sp_svc -> +----------+ small address
	 *           | r0 orig  |
	 *           | r1       |
	 *           | ...      |
	 *           | r12      |
	 *     r5 -> +----------+
	 *           |          |
	 *           |          |
	 *           |          |
	 *           |          |
	 *           |          |
	 *     r0 -> +----------+ large address
	 */
	mov	r1, lr
	mov	r4, #-1
	/**
	 * r0 = frame pointer (original sp_svc)
	 * r1 = lr_svc (original lr)
	 * r2 = lr_irq (original pc)
	 * r3 = spsr_irq
	 * r4 = 0xffff ffff
	 */
	stmia	r5, {r0 - r4}
	/**
	 * sp_svc -> +----------+ small address
	 *           | r0 orig  |
	 *           | r1       |
	 *           | ...      |
	 *           | r12      |
	 *     r5 -> +----------+
	 *           | r0       | (original sp_svc)
	 *           | lr_svc   | (original lr)
	 *           | lr_irq   | (original pc)
	 *           | spsr_irq |
	 *           | ffffffff |
	 *     r0 -> +----------+ large address
	 */
	mov	lr, r2	/* lr = original PC */

#ifdef CONFIG_FA526
     /* check if [lr] is a FA510 idle instruction */
	LDR a2,=0xee070f38
	LDR a3,[lr]
	CMP a3,a2
	BEQ	IrqUpdateLR

	/* check if [lr] is a FA520 idle instruction */
	LDR a2,=0xee070f90
	LDR a3,[lr]
	CMP a3,a2
	BEQ	IrqUpdateLR

     /* check if [lr] is a FA510 sleep instruction */
	LDR a2,=0xee070f58
	LDR a3,[lr]
	CMP a3,a2
	BEQ	IrqUpdateLR

     /* check if [lr] is a FA520 sleep instruction */
	LDR a2,=0xee070fb0
	LDR a3,[lr]
	CMP a3,a2
	BNE  %1

IrqUpdateLR:
	LDR		lr, [sp, #(15 * 4)]	/* update lr_irq */
	ADD		lr, lr, #4
	STR		lr, [sp, #(15 * 4)]	/* update lr_irq */
#endif
1:
	LDR	a3, _pHandleIRQ		/* Indirect pointer to IRQ Handler */
	LDR	a3, [a3]		/* load pointer to a register */
	MOV     lr, pc			/* arrange for a safe return here */
	/**
	 * The high-level handler dispatches the IRQ to the proper
	 * user defined ISR.
	 */
	MOV pc,a3			/* call high-level handler */
	/**
	 * sp_svc -> +----------+ small address
	 *           | r0 orig  |
	 *           | r1       |
	 *           | ...      |
	 *           | r12      |
	 *     r5 -> +----------+
	 *           | r0       | (original sp_svc)
	 *           | lr_svc   | (original lr)
	 *           | lr_irq   | (original pc)
	 *           | spsr_irq |
	 *           | ffffffff |
	 *     r0 -> +----------+ large address
	 */
	ldr	r0, [sp, #(16 * 4)]	; r0 = spsr_irq
	msr	spsr_cxsf, r0

	ldmfd	sp, {r0 - pc}^

fLib_FIQTrap:
	SUB	lr, lr, #4
	STMFD   sp!, {r0-r12, lr}       /* save registers*/
	MRS     v1, SPSR		/* push SPSR to allow nested interrupts*/
	STMFD   sp!, {v1}
	LDR	a3, _pHandleFIQ		/*; Indirect pointer to FIQ Handler*/
	LDR	a3, [a3]		/*; load pointer to a register*/
	MOV     lr, pc			/*; arrange for a safe return here*/
	MOV pc,a3			/*; call high-level handler */
	LDMFD	sp!, {v1}		/*; recover SPSR value from stack*/
	MSREQ	SPSR_c, v1		/*; restore the SPSR*/
	LDMFD	sp!, {r0-r12, pc}^	/*; Restore saved registers*/

fLib_TrapSWI:
	STMDB	sp!, {r0-r12, lr, pc}
	/*MRS r0, CPSR*/
	/*BIC r0, r0, #(0x80 | 0x40)*/  
        /*MSR CPSR_c, r0*/
	MRS	r1, spsr
	ANDS	r1, r1, #0x20
	SUBEQ	r0, lr, #0x4
	SUBNE	r0, lr, #0x2
	MOVNE	r1, #1
	MOV		r2, sp
	BL		SysSWIHandler
	LDMIA	sp!, {r0-r12, lr}
	ADD		sp, sp, #4
	MOVS	pc, lr

EnableIRQ:
	MRS r0, CPSR
	BIC r0, r0, #0x80
	MSR CPSR_c, r0
	MOV	pc,r14

DisableIRQ:
	MRS r0, CPSR
	ORR r0, r0, #0x80
	MSR CPSR_c, r0
	MOV	pc,r14

EnableFIQ:
	MRS r0, CPSR
	BIC r0, r0, #0x40
	MSR CPSR_c, r0
	MOV	pc,r14

DisableFIQ:
	MRS r0, CPSR
	ORR r0, r0, #0x40
	MSR CPSR_c, r0
	MOV	pc,r14

_cli_:
	MRS r0, CPSR
	ORR r0, r0, #0xC0
	MSR CPSR_c, r0
	MOV	pc,r14

_sti_:
	MRS r0, CPSR
	BIC r0, r0, #0xC0
	MSR CPSR_c, r0
	MOV	pc,r14

	.end

