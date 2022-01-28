#include "spec_asm.h"
/*#include "cpu.h"*/
#include "autoconf.h"

	.extern 	__start 	 

	   
	.global 	SelfBoot
	.global		TopOfStack	 	
	.global		BaseOfStack     
	.global		TopOfHeap	 	
	.global		BaseOfHeap      
	.global		TopOfFreeMemory	
	.global		BaseOfFreeMemory   	 
	.global  	ReadMode
	.global		do_semi
	.global		jmp_code
	.global		PeriphBase





 .text 
 .align 0
 .code 32 


SelfBoot:
	BL fLib_Start 
	LDR	PC, fLib_UndefVector
	LDR	PC, fLib_SWIVector
	LDR	PC, fLib_PreAbtVector
	LDR	PC, fLib_DataAbtVector
	LDR	PC, fLib_ReservdVector
	LDR	PC, fLib_IRQVector
	LDR	PC, fLib_FIQVector	
	

fLib_Reset:			.word	fLib_Start	
fLib_UndefVector:	.word	fLib_TrapUndef
fLib_SWIVector:		.word	fLib_TrapSWI		
fLib_PreAbtVector:	.word	fLib_PrefetchAbort	
fLib_DataAbtVector:	.word	fLib_DataAbort
fLib_ReservdVector:	.word	fLib_ResetStop
fLib_IRQVector:		.word	fLib_DummyIRQ		
fLib_FIQVector:		.word	fLib_DummyFIQ 		
fLib_LastVector:  	


fLib_TrapUndef:
	B   fLib_TrapUndef 
	
fLib_PrefetchAbort:
	B   fLib_PrefetchAbort
	
fLib_DataAbort:
	B   fLib_DataAbort
	
fLib_DummyIRQ:
	STMFD	sp!, {r4}	
	LDMFD	sp!, {r4}
	SUBS	pc, r14, #4		@ return from IRQ
	

fLib_DummyFIQ:
	STMFD	sp!, {r4}	
	LDMFD	sp!, {r4}
	SUBS	pc, r14, #4		@ return from IRQ
		
	
fLib_ResetStop:	
	NOP
	NOP
	NOP



StartOfCode:		.word __code_start
StartOfBSS:		.word __bss_start			
EndOfBSS:	  	.word __bss_end__


fLib_Start:

    
 
fLib_Start_Normal: 


   
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0
  

#ifndef CONFIG_SKIP_RELOCATE
relocate:							/* relocate myself to RAM	    */
	adr	r0, SelfBoot					/* r0 <- current position of code   */
	ldr	r1, StartOfCode				/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	beq     not_relocate

	ldr	r2, StartOfCode
	ldr	r3, StartOfBSS
	sub	r2, r3, r2					/* r2 <- size of bootcode2            */
	add	r2, r0, r2					/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}			/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}			/* copy to   target address [r1]    */
	cmp	r0, r2						/* until source end addreee [r2]    */
	ble	copy_loop
#endif	/* CONFIG_SKIP_RELOCATE */
not_relocate:
    @support code relocation & PIC
	

/* ycmo: jump to new location relocated */
	ldr pc, _NewLoc
_NewLoc:
	.word NewLoc
NewLoc:

	/*
	 * Reset co-processor
	 */
	

//ycmo: from u-boot


#ifdef CONFIG_CPU_FA626TE
    /*
     * flush D cache before disabling it
     */
    mov r0, #0
    mcr p15, 0, r0, c7, c6, 0   /* invalidate D Cache */
    mcr p15, 0, r0, c8, c7, 0   /* flush v4 TLB */
    mcr p15, 0, r0, c7, c5, 0   /* invalidate I Cache */

    /*
     * disable MMU and D cache
     * enable I cache if CONFIG_SYS_ICACHE_OFF is not defined
     */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x00000300 /* clear bits 9:8 (---- --RS) */
    bic r0, r0, #0x00000087 /* clear bits 7, 2:0 (B--- -CAM) */
    bic r0, r0, #0x00003000 /* clear bits 13:12 (--VI ----) */
    orr r0, r0, #0x00000002 /* set bit 1 (A) Align */
#ifdef CONFIG_HIGH_VECTOR
    orr r0, r0, #0x00002000 /* set bit 13 (--V- ----) */
#endif
#ifndef CONFIG_SYS_ICACHE_OFF
    orr r0, r0, #0x00001000 /* set bit 12 (I) I-Cache */
#endif
    mcr p15, 0, r0, c1, c0, 0

#endif //CONFIG_CPU_FA626TE

#ifdef CONFIG_CPU_CA9
    /*
     * Invalidate L1 I/D
     */
    mov r0, #0          @ set up for MCR
    mcr p15, 0, r0, c8, c7, 0   @ invalidate TLBs
    mcr p15, 0, r0, c7, c5, 0   @ invalidate icache
    mcr p15, 0, r0, c7, c5, 6   @ invalidate BP array
    mcr     p15, 0, r0, c7, c10, 4  @ DSB
    mcr     p15, 0, r0, c7, c5, 4   @ ISB

    /*
     * disable MMU stuff and caches
     */
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #0x00000800 @ set bit 11 (Z---) BTB
    bic r0, r0, #0x00000007 @ clear bits 2:0 (-CAM)
    bic r0, r0, #0x00003000 /* clear bits 13:12 (--VI ----) */
    orr r0, r0, #0x00000002 @ set bit 1 (--A-) Align
#ifdef CONFIG_HIGH_VECTOR
	orr r0, r0, #0x00002000	/* set bit 13 High Vector */
#endif
#ifndef CONFIG_SYS_ICACHE_OFF
    orr r0, r0, #0x00001000 /* set bit 12 (I) I-Cache */
#endif
    mcr p15, 0, r0, c1, c0, 0

    mrc p15, 4, r0, c15, c0, 0  // Read periph base address
    LDR r1, =PeriphBase
    STR r0,[r1]     
#endif //CONFIG_CPU_CA9




    @Turn off IRQ/FIQ bit of CPU
	MRS		r0, CPSR
	ORR		r0,r0,#(I_BIT|F_BIT)
	MSR		CPSR_c,r0

	LDR     r1, =TopOfStack
	LDR     r0, [r1]               @load top of stack	
	BL	    InitStacks
#ifdef CONFIG_START_W_MMU
	BL		MMU_Enable
	//BL CPU_AllCache_Enable
#endif

#ifdef CONFIG_VBAR_REMAP_TEXT_BASE 
//no need to copy vector, just change vector base

    ldr     r0, =SelfBoot
    mcr     p15, 0, r0, c12, c0, 0  @Set VBAR

#else //not CONFIG_VBAR_REMAP_TEXT_BASE

#ifdef CONFIG_CPU_CA9
    mov r0, #0
    mcr     p15, 0, r0, c12, c0, 0  @Reset VBAR 
#endif

	adr		r12, SelfBoot		@ r12 = current load addr
	CMP		r12, #0
	BEQ		vector_cpy_end

	@	Copy vectors  to RAM at zero.
	MOV	    r4, #0

#ifdef CONFIG_REDIRECT_VECTOR
	LDR 	r4, =RedirectVectorBase      @redirect vector
	LDR		r4, [r4]
#endif //CONFIG_REDIRECT_VECTOR

#ifdef CONFIG_HIGH_VECTOR
	LDR 	r4, =HighVectorBase      @High vector
	LDR		r4, [r4]
#endif //CONFIG_HIGH_VECTOR

	LDR	    r5, =SelfBoot	        @Copy from here..
	LDR	    r6, =fLib_LastVector	@ ..to here
	
LC3:
	LDR	    r7, [r5], #4		      @ get word and store to RAM	 
	STR	    r7, [r4], #4
	CMP	    r5, r6			          @ Repeat until all done
	BCC	    LC3
vector_cpy_end:

#endif //CONFIG_VBAR_REMAP_TEXT_BASE

/*
	LDR     r1, =TopOfStack
	LDR     r0, [r1]               @load top of stack	
	BL	    InitStacks
	*/
	
	@ For reload issue, read SDRAM setting and rewrite again
//	BL		InitBSSMemory
	//ycmo: newlib crtx:__start do the same thing


	BL	    Board_Init 	           @ Everything required to init HW

//ycmo for gcc 4
#ifdef GCC4
	B _mainCRTStartup
#else
	B __start  
#endif



/*
;===========================================================================
;	InitStacks
;
; 'Routine' called from boot-up to set all stacks.
; Although we have no stack yet, r14 contains return address. r0 contains
; top of memory (do not corrupt!) and all stacks are relative to this value.
*/

InitStacks:
	MOV		r6, r14			@ !!! Protect the return address !!!

	@Any modes which are only used to cause a reset can all work in
	@ the same stack space, since only one can occur at any one time.
	MRS		r4, cpsr
	ORR		r4, r4, #I_BIT		@ No IRQs & blank mode
	ORR		r4, r4, #F_BIT		@ Or FIQs
	BIC		r4, r4, #ModeMask

	@ Setup Undefined & Abort stack.
	ORR		r5, r4, #Undef32Mode
	MOV		r2, #User32Mode
	MSR		cpsr_c, r5
	MSR		spsr_c, r2
	BIC		r7, r0, #3		@ Must be word-aligned
	MOV		sp, r7			  @ Initial Undefined stack pointer...
	
	SUB		r7, r7, #UndefStackSize
	BIC		r7, r7, #3		@ Must be word-aligned
	
	ORR		r5, r4, #Abort32Mode
	MSR		cpsr_c, r5
	MSR		spsr_c, r2	
	MOV		sp, r7			@ Initial Abort stack pointer...
	
	SUB		r7, r7, #AbortStackSize
	BIC		r7, r7, #3		@ Must be word-aligned 
	
	@ Setup FIQ stacks.  
 
	ORR		r5, r4, #F_BIT 
	ORR		r5, r4, #FIQ32Mode
	MSR		cpsr_c, r5
	MSR		spsr_c, r2
	MOV		sp, r7			@ Initial FIQ stack pointer...
	
	SUB		r7, r7, #FIQStackSize
	BIC		r7, r7, #3		@ Must be word-aligned 

	@ Setup IRQ stacks.
	
	ORR		r5, r4, #IRQ32Mode
	MSR		cpsr_c, r5
	MSR		spsr_c, r2
	MOV		sp, r7			@ Initial IRQ stack pointer...
	
	SUB		r7, r7, #IRQStackSize
	BIC		r7, r7, #3		@ Must be word-aligned

	@ Setup Supervisor stacks.
	ORR		r5, r4, #SVC32Mode
	MSR		cpsr_c, r5
	MSR		spsr_c, r2
	MOV		sp, r7			@ Initial SVC stack pointer...
	SUB		r7, r7, #SVCStackSize
	BIC		r7, r7, #3		@ Must be word-aligned	
	MOV		r10, r7			  @ Bottom of SVC stack
 
	@ Switch into System mode (v4 architecture only)
	ORR		r5, r4, #Sys32Mode
	MSR		cpsr_c, r5
	MOV 	sp, r7 
	SUB		r7, r7, #UserStackSize
	BIC		r7, r7, #3	 @ Must be word-aligned
	MOV		r10, r7			 @ Bottom of stacks


	@Save base of Stack
    LDR     r0,=BaseOfStack
    STR     r10,[r0]          

	@ Save the top of heap = TopOfRAM - all stacks
	SUB     r7,r7,#4
	MOV		r1, r7			@ Return top of heap in r1
	LDR     r0, =TopOfHeap
	STR     r1,[r0]          @save top of heap
	
	
	SUB     r1,r1,#HeapSize
	LDR     r0,=BaseOfHeap
	STR     r1,[r0]                 @save base of heap 

	BIC     r1,r1,#3
	LDR     r0,=TopOfFreeMemory  
	STR     r1,[r0]                 @Save Top of free memory
        
	ORR		r5, r4, #SVC32Mode	@ Switch back to SVC mode
	MSR		cpsr_c, r5 

	MOV		r11, #0			@ No previous frame, so fp=0
	@ Just return back to boot up code
 	MOV	 	pc,r6



/*	
;===========================================================================
;	InitBSSMemory
;
; Routine called from boot-up to initialise all memory used by 'C' and any
; predefined assembler data areas. Use of indirect links to real linker
; variables allows any memory map to be used & hides linker specifics.
; This is a real subroutine, called after stack initialisation.
*/
InitBSSMemory:
	STMFD	sp!, {r4-r8, r14}
	
	LDR	   	r4, =StartOfBSS  	@ Top of zero init memory
	LDR		r4, [r4]
	LDR	    r6, =EndOfBSS		@ 'C' BSS end here
	LDR		r6, [r6]
	MOV	    r5, #0 
init_bss1:
	CMP	    r4, r6				@ Check if done..
	STRCC	r5, [r4], #4		@ if not, clear another word
	BCC	    init_bss1
	
	LDR     r0,=BaseOfFreeMemory @save base of Free memory
	STR     r4 ,[r0]
         
	@restore registers and return
	LDMFD	sp!, {r4-r8, pc} 
	MOV     pc, lr



/*		some utility function */
ReadMode:
	STMFD   sp!, {r4-r12, lr}    @save registers	
	MRS	  	r0, CPSR
	LDMFD	sp!, {r4-r12, pc}^	@Restore saved registers

WriteMode:
	STMFD   sp!, {r4-r12, lr}    @ save registers	
	MSR		CPSR_c, r0	
	LDMFD	sp!, {r4-r12, pc}^	 @ Restore saved registers

do_semi:
	mov 	r12, r14
	swi 	0x123456
	MOV		pc,r12

jmp_code:
	mov		r14, r0
	mov		pc, r14


 .data 

 

/*TopOfStack: .word __stack */
/*TopOfStack: .word CONFIG_STACK_ADDR*/
TopOfStack: .word  CONFIG_STACK_ADDR
#ifdef CONFIG_REDIRECT_VECTOR
RedirectVectorBase: .word  CONFIG_REDIRECT_VECTOR_BASE
#endif
HighVectorBase: .word  0xffff0000
BaseOfStack: .word 0
TopOfHeap: .word 0
BaseOfHeap: .word 0
TopOfFreeMemory: .word 0
BaseOfFreeMemory: .word 0
PeriphBase: .word 0
			  	
	.end

