/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:gic.c                                                               *
* Description: 
****************************************************************************/

#include <stdarg.h> // for va_list va_start()
#include "SoFlexible.h"
#include "interrupt.h"
#include "gic400.h"



extern fLib_INT IRQVector[PLATFORM_IRQ_TOTALCOUNT];
extern fLib_INT IRQVector_priv[32];
  
/* define IRQ trap and handler routine */
extern PrHandler fLib_IRQHandle;

UINT32 PeriphBase;
//static unsigned int PeriphBase;
/////////////////////////////////////////////////////
//
//	Only for detail function call subroutine
//
/////////////////////////////////////////////////////

uint32_t get_periph_base()
{
#if 0
      unsigned long long CBAR_val;
      asm volatile("mrs %0,S3_1_C15_C3_0" : "=r"( CBAR_val) :: "cc");
      CBAR_val &=  0x7FFFFC0000;
 	
      return (uint32_t)CBAR_val;		//periph_base is 32 bit,so it can directly use uint32_t to express the value
#else
       //return 0x18700000; //for VDK base
	   return 0x28600000; // for A600

#endif
}

static void gic_dist_init(unsigned int gic_irqs)
{
    unsigned int i;

    /* EnableGrp0 | EnableGrp1 */
    *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + GIC_DIST_CTRL) = 0;
    
    /*
     * Set all global interrupts to be level triggered, active high.
     */
    for(i = 32;i < gic_irqs; i+= 16)
    {
        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_CONFIG + i * 4 /16))= 0x55555555;
    }
    
    for(i = 32; i < gic_irqs; i += 4){
        /*
         * Set all global interrupts target to CPU 0 only.
         */
//        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_TARGET + i * 4 /4))= 0x01010101;
        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_TARGET + i * 4 /4))= 0xffffffff;
        /*
         * Set priority on all global interrupts.
         */
        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_PRI + i * 4 / 4))= 0x0a0a0a0a;
    }

    /*
     * Disable all interrupts.  Leave the PPI and SGIs alone
     * as these enables are banked registers.
     */
    for(i = 32; i < gic_irqs; i += 32)
    {
        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_ENABLE_CLEAR + i * 4 / 32))= 0xFFFFFFFF;
    }

    *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + GIC_DIST_CTRL)= 1;
}

static void gic_cpu_init(void)
{
    int i;

    /*
     * Deal with the banked PPI and SGI interrupts - disable all
     * PPI interrupts, ensure all SGI interrupts are enabled.
     */
    *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + GIC_DIST_ENABLE_CLEAR)= 0xffff0000;
    *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + GIC_DIST_ENABLE_SET)= 0x0000ffff;

    /*
     * Set priority on PPI and SGI interrupts
     */
    for (i = 0; i < 32; i += 4)
    {
        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_PRI + i * 4 / 4))= 0x0a0a0a0a;
    }
    
    *(volatile unsigned int*)(PeriphBase + GIC_CPU_OFFSET + GIC_CPU_PRIMASK)= 0xf0;
    *(volatile unsigned int*)(PeriphBase + GIC_CPU_OFFSET + GIC_CPU_CTRL)= 0x1;
}

/**
 * VINTC0 at A369 connect to IRQ 90 of GIC at ZYNQ board
 */
void gic_init(void)
{
    unsigned int gic_irqs;

    PeriphBase = get_periph_base();
    gic_irqs= (*(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + GIC_DIST_CTR)) & 0x1f;
    gic_irqs = (gic_irqs + 1) * 32;
    if (gic_irqs > 1020)
         gic_irqs = 1020;
    //printf("GIC: provides %d interrupts, %d external interrupt lines\n", gic_irqs, (gic_irqs - 32));

    gic_dist_init(gic_irqs);
    gic_cpu_init();
}

static void gicDisableIRQ(UINT32 irq_n);
/*
 * Turn the interrupt source off. 
 * old function name alias: 
 *     MaskIRQ 
 *     fLib_MaskInt
 *     fLib_DisableInt  */
void fLib_DisableIRQ(UINT32 spi_n)
{
    gicDisableIRQ(spi_n + 32);
}
void fLib_DisableIRQ_priv(UINT32 irq_n)
{
    gicDisableIRQ(irq_n);
}
static void gicDisableIRQ(UINT32 irq_n)
{
//    outll(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_ENABLE_CLEAR + (irq_n / 32) * 4 ), (UINT32)1 << irq_n % 32);
    *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_ENABLE_CLEAR + (irq_n / 32) * 4 ))= 1 << irq_n % 32;
}

static void gicEnableIRQ(UINT32 irq_n);
/*  Turn the interrupt source on. 
 *  old function name alias: 
 *      UnmaskIRQ 
 *      fLib_EnableInt
 *      fLib_UnmaskInt */
void fLib_EnableIRQ(UINT32 spi_n)
{
    gicEnableIRQ(spi_n + 32);
}
void fLib_EnableIRQ_priv(UINT32 irq_n)
{
    gicEnableIRQ(irq_n);
}
static void gicEnableIRQ(UINT32 irq_n)
{
     u32 offset; 
     u32 var;
     u32 gicd_base;

     gicd_base = PeriphBase + GIC_DIST_OFFSET;

/*
    // set Interrupt Configuration Registers (ICDICFRn)
    offset    = 0xC00 + ((irq_n>> 4) * 4);
//    HAL_READ_UINT32(gicd_base + offset, var);
    var = *(volatile unsigned int*)(gicd_base + offset);
    var   &= ~(0x3ul << ((irq_n & 0xF) * 2 ));
    var   |= mode << ((irq_n & 0xF) * 2 + 1);
//    HAL_WRITE_UINT32(gicd_base + offset, var);
    *(volatile unsigned int*)(gicd_base + offset) = var;
*/
/*
    // Interrupt Processor Targets Registers (ICDIPTRn)n
    offset    = 0x800 + ((irq_n >> 2) * 4);
//    HAL_READ_UINT32(gicd_base + offset, var);
    var = *(volatile unsigned int*)(gicd_base + offset);
    var &= ~(0xfful << ((irq_n & 0x3) * 8));
    var |= 1 << ((irq_n & 0x3) * 8);  // only CPU0 target  ->it need to add other cpu
    //REG32(gicd_base + offset) = var;
//    HAL_WRITE_UINT32(gicd_base + offset, var);
    *(volatile unsigned int*)(gicd_base + offset) = var;
*/    
    // enable Interrupt Set-Enable Registers (ICDISERn)
    offset    = 0x100 + ((irq_n >> 5) * 4);
    var = *(volatile unsigned int*)(gicd_base + offset);
    var |= 1 << (irq_n & 0x1F); // assume only one irq enable
//    HAL_WRITEOR_UINT32(gicd_base + offset, var);
    *(volatile unsigned int*)(gicd_base + offset) = var;
    
}

//porting from arch/arm/common/gic.c
//gic_set_type(struct irq_data *d, unsigned int type)
void fLib_SetIRQmode(u32 spi_n,u32 mode)
{	

//    void __iomem *base = gic_dist_base(d);
    u32 int_n = spi_n + 32;
    u32 enablemask = 1 << (int_n % 32);
    u32 enableoff = (int_n / 32) * 4;
    u32 confmask = 0x2 << ((int_n % 16) * 2);
    u32 confoff = (int_n / 16) * 4;
    bool enabled = false;
    u32 val;
    u32 gicd_base;
          
    val= *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_CONFIG + confoff));
    //val= inw(PeriphBase + GIC_DIST_OFFSET + GIC_DIST_OFFSET + (GIC_DIST_CONFIG + confoff));
    
    if (mode == LEVEL)
        val &= ~confmask;
    else if (mode == EDGE)
        val |= confmask;

    /*
     * As recommended by the spec, disable the interrupt before changing
     * the configuration
     */
    if((*(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_ENABLE_SET + enableoff))) & enablemask){
        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_ENABLE_CLEAR + enableoff)) = enablemask;
        enabled = true;        
    }
    *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_CONFIG + confoff)) = val;

    if (enabled)
        *(volatile unsigned int*)(PeriphBase + GIC_DIST_OFFSET + (GIC_DIST_ENABLE_SET + enableoff)) = enablemask;
    return;
}	


void SetIRQlevel(UINT32 IRQ,UINT32 low)
{
    fLib_printf("ERROR: GIC can't set level! \n");
//    while (1);
}	


void fLib_ClearIRQ(UINT32 IRQ)
{
    return;
}


/*
 * Interrupt service routine dispatcher.
 *
 * This is the IRQ handler called when an IRQ trap is taken.
 */
void DispatchIRQ(UINT32 flags)
{
    unsigned int spi_n , int_n;
    fLib_INT *action;

    int_n = *(volatile unsigned int*)(PeriphBase + GIC_CPU_OFFSET + (GIC_CPU_INTACK));
//fLib_printf("%s int_n = 0x%x\n", __func__, int_n);
    if (int_n >= 32){
        spi_n = int_n - 32;

        // Get the vector and process the interrupt.
        action = &IRQVector[spi_n];

        if (action->Handler == NULL)
            fLib_printf("No SPI handler installed: %d\n", spi_n);		// can't call printf in int handler
        else 
            action->Handler(spi_n, action->private_arg);          

    }else{ //int_n < 32
        action = &IRQVector_priv[int_n];

        if (action->Handler == NULL)
            fLib_printf("No PPI or SGI handler installed: %d\n", int_n);		// can't call printf in int handler
        else 
            action->Handler(int_n, action->private_arg);          

    }
    *(volatile unsigned int*)(PeriphBase + GIC_CPU_OFFSET + (GIC_CPU_EOI)) = int_n;
}                               /* DispatchIRQ */

/* Called once on start-up to initialise data structures & interrupt routines */
void fLib_Int_Init()
{
    PeriphBase = get_periph_base();
    fLib_ResetIRQ();
//    fLib_WriteIntVector((UINT32 *)IrqV, (PrVoid)fLib_IRQTrap);
    fLib_IRQHandle = DispatchIRQ;

    gic_init();
}

