/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:intc.c                                                               *
* Description: 
	1: Interrupt routine                                       
	2: Adding 64 bit IRQ/FIQ by Richard Lin 08/11
* Author: Fred Chien                                                       *
****************************************************************************/

#include <stdarg.h> // for va_list va_start()
#include "SoFlexible.h"
#include "ftintc030.h"

#ifdef CONFIG_EXTERNAL_GIC
#include "gic.h"
#endif

#define IPMODULE INTC
#define IPNAME   FTINTC030

extern fLib_INT IRQVector[PLATFORM_IRQ_TOTALCOUNT];

extern PrHandler fLib_IRQHandle ;
extern PrHandler fLib_FIQHandle ;

extern UINT32 PeriphBase;

static void INTC_SetSPIPriority(UINT32 SPIID, UINT32 priority)
{
	volatile UINT32 reg;
	UINT32 spi_reg_id = SPIID / 4;
	UINT32 spi_id = SPIID % 4;
	reg = *(UINT32 *)(FTINTC030_BASE + FTINTC030_SPI_PRIORITY + spi_reg_id * 4);
	reg |= priority << spi_id * 8;
	*(UINT32 *)(FTINTC030_BASE + FTINTC030_SPI_PRIORITY + spi_reg_id * 4) = reg;
}

static void INTC_PriorityMask(UINT32 InterruptLevel)
{
	*(UINT32*) (FTINTC030_BASE + FTINTC030_PRIORITY_MASK) = InterruptLevel;
}

static void INTC_BinaryPoint(UINT32 BinaryPoint)
{
	*(UINT32*) (FTINTC030_BASE + FTINTC030_BINARY_POINT) = BinaryPoint;
}

static void INTC_EnableErrorMask(void)
{
	volatile UINT32 reg;
	reg = *(UINT32*) (FTINTC030_BASE + FTINTC030_INTCTL);
	reg |= 1 << 2;
	*(UINT32*) (FTINTC030_BASE + FTINTC030_INTCTL) = reg;
}

static void INTC_SetIntCtrl(UINT32 InterruptCtrl)
{
	*(UINT32*) (FTINTC030_BASE + FTINTC030_INTCTL) = InterruptCtrl;
}

void INTC_SetCPUMatch(UINT32 CPUNum, UINT32 CPUID)
{	
	*(UINT32*) (FTINTC030_BASE + FTINTC030_CPU0MATCH + sizeof(UINT32) * CPUNum) = CPUID;	
}

/* Mode and Level routine */
void SetIRQmode(UINT32 irq, UINT32 mode)
{	
	volatile UINT32 reg, base;
	UINT32 spi_reg_id = irq / 32;
	UINT32 spi_id     = irq % 32;
	
	if (irq < 64)
		base = FTINTC030_BASE + spi_reg_id * 0x20;	   // 0x00: SPI[31:0] register base
	else
		base = FTINTC030_BASE + spi_reg_id * 0x20 + 0x20;  // 0x60: SPI[95:64] register base
		
	reg = *(UINT32*)(base + FTINTC030_SPI_MODE);
	
	if (mode == EDGE) //1: Edge-trigger mode
		reg |= (1 << spi_id); 
	else		
		reg &= ~(1 << spi_id);
		
	*(UINT32*)(base + FTINTC030_SPI_MODE) = reg;
}	

void SetIRQlevel(UINT32 irq, UINT32 low)
{	
	volatile UINT32 reg, base;
	UINT32 spi_reg_id = irq / 32;
	UINT32 spi_id     = irq % 32;
	
	if (irq < 64)
		base = FTINTC030_BASE + spi_reg_id * 0x20;	   // 0x00: SPI[31:0] register base
	else
		base = FTINTC030_BASE + spi_reg_id * 0x20 + 0x20;  // 0x60: SPI[95:64] register base
		
	reg = *(UINT32*)(base + FTINTC030_SPI_LEVEL);
	
	if (low) // spi_level_reg, 1: Active-low or falling-edge trigger
		reg |= (1 << spi_id); 
	else		
		reg &= ~(1 << spi_id);
		
	*(UINT32*)(base + FTINTC030_SPI_LEVEL) = reg;	
}	
/*
void fLib_SetIntTrig(UINT32 IsFiq, UINT32 irq, int IntMode, int IntLevel)
{
	SetIRQmode (irq, IntMode);
	SetIRQlevel(irq, IntLevel); 
}
*/
void fLib_SetIRQmode(UINT32 irq, UINT32 mode)
{
	SetIRQmode (irq, mode);
}

/* Clear interrupt routine */
void ClearIRQStatus(UINT32 irq)
{
	UINT32 base;
	UINT32 spi_reg_id = irq / 32;
	UINT32 spi_id     = irq % 32;
	
	if (irq < 64)
		base = FTINTC030_BASE + spi_reg_id * 0x20; 	   // 0x00: SPI[31:0] register base
	else
		base = FTINTC030_BASE + spi_reg_id * 0x20 + 0x20;  // 0x60: SPI[95:64] register base
		
	*(UINT32*)(base + FTINTC030_SPI_CLEAR) = (1 << spi_id); 
}

void fLib_ClearIRQ(UINT32 irq)
{
	ClearIRQStatus(irq);			
}

/* Enable and Disable interrupt routine */
// Turn off the interrupt, i.e. Disable Interrupt
void MaskIRQ(UINT32 irq)
{    
	 fLib_DisableIRQ(irq);   
}

// Turn on the interrupt, i.e. Enable Interrupt
void UnmaskIRQ(UINT32 irq)
{    
	 fLib_EnableIRQ(irq);     
}

void fLib_MaskInt(UINT32 IsFiq, UINT32 irq)
{
	 fLib_DisableIRQ(irq);     
}

void fLib_UnmaskInt(UINT32 IsFiq, UINT32 irq)
{
	 fLib_EnableIRQ(irq);      
}

void fLib_EnableIRQ(UINT32 irq)
{
	volatile UINT32 reg, base;
	UINT32 spi_reg_id = irq / 32;
	UINT32 spi_id     = irq % 32;
	
	if (irq < 64)
		base = FTINTC030_BASE + spi_reg_id * 0x20;		   // 0x00: SPI[31:0] register base
	else
		base = FTINTC030_BASE + spi_reg_id * 0x20 + 0x20;  // 0x60: SPI[95:64] register base
		
	reg = *(UINT32*)(base + FTINTC030_SPI_ENABLE);
	reg |= (1 << spi_id);  
	*(UINT32*)(base + FTINTC030_SPI_ENABLE) = reg;
}

void fLib_DisableIRQ(UINT32 irq)
{	 
	volatile UINT32 reg, base;
	UINT32 spi_reg_id = irq / 32;
	UINT32 spi_id     = irq % 32;
	
	if (irq < 64)
		base = FTINTC030_BASE + spi_reg_id * 0x20;		   // 0x00: SPI[31:0] register base
	else
		base = FTINTC030_BASE + spi_reg_id * 0x20 + 0x20;  // 0x60: SPI[95:64] register base
		
	reg = *(UINT32*)(base + FTINTC030_SPI_ENABLE);
	reg &= ~(1 << spi_id);
	*(UINT32*)(base + FTINTC030_SPI_ENABLE) = reg;
}

void fLib_DisableFIQ(UINT32 fiq)
{
}

static void intc030_dist_init(UINT32 nSPI)
{    
	UINT32 irq, prior, target, base;	
	
	/* Do not handle SGI and PPI */
	for (irq = 0; irq < nSPI; irq += 32)
	{		
		if (irq < 64)
			base = FTINTC030_BASE + irq;						
		else
		{
			base = FTINTC030_BASE + irq + 0x20;           // 0x60: SPI[95:64] register base
			*(UINT32*)(base + FTINTC030_DEBOUNCE) = 0xFFFFFFFF;
		}		
		*(UINT32*)(base + FTINTC030_SPI_ENABLE) = 0x00000000; // Disable all Interrupt
		*(UINT32*)(base + FTINTC030_SPI_MODE)   = 0x00000000; // Level-trigger mode
		*(UINT32*)(base + FTINTC030_SPI_LEVEL)  = 0x00000000; // High Active
		*(UINT32*)(base + FTINTC030_SPI_CLEAR)  = 0xFFFFFFFF;
	}						
	*(UINT32*)(FTINTC030_BASE + FTINTC030_DEBOUNCE0)  = 0xFFFFFFFF; // SPI[31:0] de-bounce register
	*(UINT32*)(FTINTC030_BASE + FTINTC030_DEBOUNCE1)  = 0xFFFFFFFF; // SPI[63:32] de-bounce register
		
	// Set SPI Priority and SPI Target to assert the interrupt	
	prior  = FTINTC030_BASE + FTINTC030_SPI_PRIORITY;
	target = FTINTC030_BASE + FTINTC030_SPI_TARGET;

        for (irq = 0; irq < nSPI; irq += 4)
        {
                *(UINT32*)(prior  + irq) = 0x00000000;	// all SPI are the highest priority
        } 
#ifdef CONFIG_SPI_TRARGET_INVERSE	
	for (irq = 0; irq < nSPI; irq += 32)
        {
		*(UINT32*)(target) = 0xFFFFFFFF; 	// assume enable CPU0 only
                target += 4; 
        }
#else
	for (irq = 0; irq < nSPI; irq += 4)
	{
		*(UINT32*)(target + irq) = 0x01010101;	// enable CPU0 only
	}	
#endif
}

static void intc030_cpu_init(void)
{
#ifdef CONFIG_PLATFORM_A380 || CONFIG_PLATFORM_GM8220
        // Set CPU Match Register to access the bank register
        // Assume only one CPU core for gerneral case
        INTC_SetCPUMatch(0, FTINTC030_SPI_CPU0_ID);
#endif

	// Set Interrupt Control Register 
    	INTC_SetIntCtrl(4);		// mask error response and enable priority arbitration

    	// Set Binary Point Registers
    	INTC_BinaryPoint(7);		// No pre-emption
    
    	// Set Priority Mask Register to mask the interrupt priority 	
    	INTC_PriorityMask(0xF);		// All priority are allowed
}

/*
 * Interrupt service routine dispatcher.
 *
 * This is the IRQ handler called when an IRQ trap is taken.
 */
void DispatchIRQ(UINT32 flags)
{
    int irq, spi;
    fLib_INT *action;
#ifdef CONFIG_EXTERNAL_GIC
    int gic_irq_id;
#endif    

    spi   = *(UINT32*)(FTINTC030_BASE + FTINTC030_ACK);
    irq    = spi - 32;
    if (irq > PLATFORM_IRQ_TOTALCOUNT){
        fLib_printf("ftintc030: irq=0x%x, no such irq number\n", irq);	
#if 0/* Shantai test. May be an issue of INTC030, or should change ISR flow. 2013/11/20 */
	if(*(UINT32*)(FTINTC030_BASE + FTINTC030_SPI_SRC0) == BIT29)
	{
		/* the irq=29 is usb interrupt number of A380 */
//		fLib_printf("IRQ 29\n");
		action = &IRQVector[29];
		if (action->Handler != NULL) 
		{
			action->Handler(29, action->private_arg);
			ClearIRQStatus(29); // it have to do for edge trigger interrupt
		}
//		*(UINT32*)(FTINTC030_BASE + FTINTC030_SPI_ENABLE) &= (~BIT29);
//		*(UINT32*)(FTINTC030_BASE + FTINTC030_SPI_CLEAR) = BIT29;
	}
#endif
    }else{
        action = &IRQVector[irq];

        if (action->Handler != NULL) 
        {
            action->Handler(irq, action->private_arg);
            ClearIRQStatus(irq); // it have to do for edge trigger interrupt
        }else{
            fLib_printf("ftintc030: No handler installed: %d\n", irq);		// can't call printf in int handler
#ifdef CONFIG_EXTERNAL_GIC
            gic_irq_id = GIC_CPU_REG32(GIC_CPU_INTACK);
            fLib_printf("gic_irq_id = %d\n", gic_irq_id);		
            GIC_CPU_REG32(GIC_CPU_EOI)=gic_irq_id;
#endif    
        }
    }
    *(UINT32*)(FTINTC030_BASE + FTINTC030_EOI) = spi;
}

/* 
 * Main routines
 * Called once on start-up to initialise data 
 * structures & interrupt routines 
 */
void fLib_Int_Init(void)
{
     	UINT32 idx, *tvec;	

        intc030_cpu_init(); 
        intc030_dist_init(PLATFORM_IRQ_TOTALCOUNT); 
	
	/* Initialise and mask all interrupts without interrupt handlers */
    	for (idx = 0; idx < PLATFORM_IRQ_TOTALCOUNT; idx++)
    	{	    	
	     fLib_CloseIRQ(idx);
    	}	
	
  	tvec  = ((UINT32 *) IrqV) + 8;
  	*tvec = (UINT32) fLib_IRQTrap;		

	fLib_IRQHandle = DispatchIRQ;
	
	EnableIRQ();
#ifndef CONFIG_EXTERNAL_GIC
    *(UINT32*)(PeriphBase+0x100) = 0;
#endif

#ifdef CONFIG_EXTERNAL_GIC
    pin_mux_enable(EXT_INT,0);
    gic_init();
#endif
}


