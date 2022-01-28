#include "spec_asm_64.h"
#include "macro.h"
/*#include "cpu.h"*/
#include "autoconf.h"

	.global		TopOfStack	 	
	.global		BaseOfStack     
	.global		jmp_code
	
.text

.globl _start
_start:
                mov     x0,  xzr
                mov     x1,  xzr
                mov     x2,  xzr
                mov     x3,  xzr
                mov     x4,  xzr
                mov     x5,  xzr
                mov     x6,  xzr
                mov     x7,  xzr
                mov     x8,  xzr
                mov     x9,  xzr
                mov     x10, xzr
                mov     x11, xzr
                mov     x12, xzr
                mov     x13, xzr
                mov     x14, xzr
                mov     x15, xzr
                mov     x16, xzr
                mov     x17, xzr
                mov     x18, xzr
                mov     x19, xzr
                mov     x20, xzr
                mov     x21, xzr
                mov     x22, xzr
                mov     x23, xzr
                mov     x24, xzr
                mov     x25, xzr
                mov     x26, xzr
                mov     x27, xzr
                mov     x28, xzr
                mov     x29, xzr
                mov     x30, xzr
                mov     sp,  x0
reset:

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */

//	mrs     x0, mpidr_el1
/*
	and     x0, x0, #0xFFFF           // x0 == CPU number
*/
//	and     x0, x0, #0xFF           // x0 == CPU number
//	cbnz     x0, el1_secondary
	
	/*
	 * Could be EL3/EL2/EL1, Initial State:
	 * Little Endian, MMU Disabled, i/dCache Disabled
	 */
	adr	x0, vectors
	switch_el x1, 3f, 2f, 1f
3:	msr	vbar_el3, x0
	mrs	x0, scr_el3
	mov x1, #0xf
	orr	x0, x0, x1			/* SCR_EL3.NS|IRQ|FIQ|EA|SMD|RW */
//	orr	x0, x0, #0xf			/* SCR_EL3.NS|IRQ|FIQ|EA|SMD|RW */
//	orr	x0, x0, #0x80			/* SCR_EL3.NS|IRQ|FIQ|EA|SMD|RW */	
//	orr	x0, x0, #0x400			/* SCR_EL3.NS|IRQ|FIQ|EA|SMD|RW */		
//	orr	x0, x0, #(SCR_EL3_RW | SCR_EL3_IRQ | SCR_EL3_FIQ | SCR_EL3_NS)
	msr	scr_el3, x0
	msr	cptr_el3, xzr			/* Enable FP/SIMD */
#ifdef COUNTER_FREQUENCY
	ldr	x0, =COUNTER_FREQUENCY
	msr	cntfrq_el0, x0			/* Initialize CNTFRQ */
#endif
	b	0f
2:	msr	vbar_el2, x0
	mov	x0, #0x33ff
	msr	cptr_el2, x0			/* Enable FP/SIMD */
	b	0f
1:	msr	vbar_el1, x0
	mov	x0, #3 << 20
	msr	cpacr_el1, x0			/* Enable FP/SIMD */
0:

	/* ---------------------------------------------
	 * Enable IRQ/FIQ mode
	 * ---------------------------------------------
	 */
//	mrs	x0, cpsr
//	bic x0, x0, #0xc0
//	msr cpsr_c, x0
	
    
//	mov w3, #(SCR_EL3_RW  | SCR_EL3_SMD | SCR_EL3_IRQ | SCR_EL3_FIQ | SCR_EL3_NS)
//	msr SCR_EL3, x3
/*
drop_el1:
	adr x1, el1_entry_aarch64
	msr ELR_EL3, x1
    	mov x1, #(AARCH64_SPSR_EL1h  | AARCH64_SPSR_F  | AARCH64_SPSR_I  | AARCH64_SPSR_A)
    	msr SPSR_EL3, x1

//    	eret		//the purpose of this instruction is for drop_el1 to exit exception

el1_entry_aarch64:
*/
	msr 	DAIFclr, #3

// 	Initialize the stack pointer
	LDR     x1, =TopOfStack
	LDR     x1, [x1]	
	mrs     x2, mpidr_el1
	mov 	x4, x2
	and 	x4, x4, #0xFF00 //	0814 
	lsr	x4, x4, #0x8	//	0814 X4 == CLUSTER number	
	mov 	x3, #4        	//	0814
	mul	x4, x4, x3	//	0814
	and     x2, x2, #0xFF   // x2 == CPU number
	add	x2, x2, x4	//	0814 x2:id
	mov     x3, #1024
	mul     x3, x2, x3
	sub     x1, x1, x3
	mov     sp, x1

clear_bss:
	ldr	x0, =__bss_start		/* find start of bss segment        */
	ldr	x1, =_end		/* stop here                        */
	mov	x2, #0x0			/* clear                            */
clbss_l:
	str     w2, [x0]				/* clear loop...                    */
	add     x0, x0, #4
	cmp     x0, x1
	blo	clbss_l

//    ldr     x4,=BaseOfStack
//    str    x1,[x4]      

	BL	    Board_Init 	           // Everything required to init HW

#ifdef CONFIG_START_W_MMU
	BL		MMU_Enable
	//BL CPU_AllCache_Enable
#endif

	B	    main 	           // Start nonos main function

jmp_code:
	mov		x30, x0
	br           x30		
	
invalidateUDCaches_end:
    dmb ish
    ret
	
// ------------------------------------------------------------
// EL1 - secondary CPU init code
//
// This code is run on CPUs 1, 2, 3 etc....
// ------------------------------------------------------------

    .global el1_secondary

el1_secondary:

loop_wfi:
    wfi         // Go to sleep
    b loop_wfi

.global invalidate_tlb_all
invalidate_tlb_all:
	switch_el x9, 3f, 2f, 1f
3:	tlbi	alle3
	dsb	sy
	isb
	b	0f
2:	tlbi	alle2
	dsb	sy
	isb
	b	0f
1:	tlbi	vmalle1
	dsb	sy
	isb
0:
	ret
     
.data
TopOfStack: .word  CONFIG_STACK_ADDR
BaseOfStack: .word 0
//MEM_ADDR			:	.word  0x80800000
.end
