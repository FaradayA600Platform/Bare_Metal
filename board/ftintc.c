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
#include "ftintc.h"

#ifdef CONFIG_EXTERNAL_GIC
#include "gic.h"
#endif


#define IPMODULE INTC

#if defined(CONFIG_FTINTC010)
	#define IPNAME   FTINTC010
#elif defined(CONFIG_FTINTC020)
	#define IPNAME   FTINTC020
#endif
unsigned int io_base = IP_PA_BASE(0);	



extern fLib_INT IRQVector[PLATFORM_IRQ_TOTALCOUNT];
extern fLib_INT FIQVector[PLATFORM_FIQ_TOTALCOUNT];
  

/* define IRQ trap and handler routine */
extern PrHandler fLib_IRQHandle;


/* define FIQ trap and handler routine */
extern PrHandler fLib_FIQHandle;

#if PLATFORM_IRQ_TOTALCOUNT > 64
#  error Unable to handle more than 64 IRQ levels.
#endif

#if PLATFORM_FIQ_TOTALCOUNT > 64
#  error Unable to handle more than 64 FIQ levels.
#endif


/////////////////////////////////////////////////////
//
//	Only for detail function call subroutine
//
/////////////////////////////////////////////////////



/*
 * Turn the interrupt source off. 
 * old function name alias: 
 *     MaskIRQ 
 *     fLib_MaskInt
 *     fLib_DisableInt  */
void fLib_DisableIRQ(UINT32 IRQ)
{    
 volatile UINT32 *IRQBase;

    //RichardLin XXX MoreSoFlexible
    IRQBase = (UINT32 *)io_base;     

	if(IRQ<32)
    IRQBase[(IRQ_MASK / sizeof(UINT32))] &= ~(1 << IRQ);
	else
		IRQBase[(EIRQ_MASK / sizeof(UINT32))] &= ~(1 << (IRQ-32));
}

/*  Turn the interrupt source on. 
 *  old function name alias: 
 *      UnmaskIRQ 
 *      fLib_EnableInt
 *      fLib_UnmaskInt */
void fLib_EnableIRQ(UINT32 IRQ)
{    
 volatile UINT32 *IRQBase;

    
    IRQBase = (UINT32 *)io_base;       

	if(IRQ<32)
    IRQBase[(IRQ_MASK / sizeof(UINT32))] |= (1 << IRQ);
	else
		IRQBase[(EIRQ_MASK / sizeof(UINT32))] |= (1 << (IRQ-32));
    EnableIRQ();           

}


void fLib_SetIRQmode(UINT32 IRQ,UINT32 mode)
{	
    volatile UINT32 *IRQBase;


    IRQBase = (UINT32 *)io_base;   

    if(mode==EDGE){
        if(IRQ<32)
            IRQBase[(IRQ_MODE / sizeof(UINT32))] |= (1 << IRQ);
        else
            IRQBase[(EIRQ_MODE / sizeof(UINT32))] |= (1 << (IRQ-32));
    }
    if(mode==LEVEL){
        if(IRQ<32)
            IRQBase[(IRQ_MODE / sizeof(UINT32))] &= ~(1 << IRQ);	
        else
            IRQBase[(EIRQ_MODE / sizeof(UINT32))] &= ~(1 << (IRQ-32));
    }
}	

//only for test, platform dependent
void SetIRQlevel(UINT32 IRQ,UINT32 low)
{
    volatile UINT32 *IRQBase;

    IRQBase = (UINT32 *)io_base;

    if(low) { 
        if(IRQ<32)
            IRQBase[(IRQ_LEVEL / sizeof(UINT32))] |= (1 << IRQ);
        else
            IRQBase[(EIRQ_LEVEL / sizeof(UINT32))] |= (1 << (IRQ-32));
    } else {
        if(IRQ<32)
            IRQBase[(IRQ_LEVEL / sizeof(UINT32))] &= ~(1 << IRQ);
        else
            IRQBase[(EIRQ_LEVEL / sizeof(UINT32))] &= ~(1 << (IRQ-32));
    }
}	


 
/*  old function name alias: fLib_ClearInt, ClearIRQStatus */
void fLib_ClearIRQ(UINT32 IRQ)
{
    volatile UINT32 *IRQBase;

    IRQBase = (UINT32 *)io_base; 

    if(IRQ<32) {
        IRQBase[(IRQ_CLEAR / sizeof(UINT32))] |= (1 << IRQ);
    } else {
        IRQBase[(EIRQ_CLEAR / sizeof(UINT32))] |= (1 << (IRQ-32));
    }
}


/*
 * Interrupt service routine dispatcher.
 *
 * This is the IRQ handler called when an IRQ trap is taken.
 */
void DispatchIRQ(UINT32 flags)
{
 int IRQ = 0;
 fLib_INT *action;

#ifdef CONFIG_EXTERNAL_GIC
	int gic_irq_id;
	#ifdef CONFIG_EXT_GIC_IRQ_N
	/* IRQn may be different in some daughter boards. */
	int gic_is = GIC_DIST_REG32(ICDISPRn + (CONFIG_EXT_GIC_IRQ_N/32));
	if (!(gic_is & (1 << (CONFIG_EXT_GIC_IRQ_N%32))))
		return;
	#else
	/* Do not handle SGI and PPI */
	/* VINTC connect to IRQ 90 at GIC */
	int gic_is = GIC_DIST_REG32(ICDISPRn + 8);

	if (!(gic_is & (1 << 26)))
		return;
	#endif/* CONFIG_EXT_GIC_IRQ_N */
#endif/* CONFIG_EXTERNAL_GIC */

    flags=inw(io_base+IRQ_STATUS);

    // Test to see if there is a flag set, if not then don't do anything
    while (flags != 0)
    {
                
        if ((flags & 0xffff) == 0)
        {
            IRQ += 16;
            flags >>= 16;
        }

        if ((flags & 0xff) == 0)
        {
            IRQ += 8;
            flags >>= 8;
        }

        if ((flags & 0xf) == 0)
        {
            IRQ += 4;
            flags >>= 4;
        }

        if ((flags & 0x3) == 0)
        {
            IRQ += 2;
            flags >>= 2;
        }

        if ((flags & 0x1) == 0)
        {
            IRQ += 1;
            flags >>= 1;
        }

        // Get the vector and process the interrupt.
        action = &IRQVector[IRQ];

        if (action->Handler == NULL)
        {
        	fLib_printf("No handler installed: %d\n", IRQ);		// can't call printf in int handler
        }
        else 
        {
        	action->Handler(IRQ, action->private_arg);          
        }

        flags &= ~1;
    }//end while

	IRQ =32;
	flags=inw(io_base+ EIRQ_STATUS);
    
    // Test to see if there is a flag set, if not then don't do anything
    while (flags != 0)
    {
                
        if ((flags & 0xffff) == 0)
        {
            IRQ += 16;
            flags >>= 16;
        }

        if ((flags & 0xff) == 0)
        {
            IRQ += 8;
            flags >>= 8;
        }

        if ((flags & 0xf) == 0)
        {
            IRQ += 4;
            flags >>= 4;
        }

        if ((flags & 0x3) == 0)
        {
            IRQ += 2;
            flags >>= 2;
        }

        if ((flags & 0x1) == 0)
        {
            IRQ += 1;
            flags >>= 1;
        }

        // Get the vector and process the interrupt.
        action = &IRQVector[IRQ];

        if (action->Handler == NULL){                   
        	fLib_printf("No handler installed: %d\n", IRQ);		// can't call printf in int handler
#ifdef CONFIG_EXTERNAL_GIC
            gic_irq_id = GIC_CPU_REG32(GIC_CPU_INTACK);
            fLib_printf("gic_irq_id = %d\n", gic_irq_id);
            GIC_CPU_REG32(GIC_CPU_EOI)=gic_irq_id;
#endif    
        }else 
        	action->Handler(IRQ, action->private_arg);          

        flags &= ~1;
    }//end while
}                               /* DispatchIRQ */



/////////////////////////////////////////////////////
//
//	Only for detail function call subroutine
//
/////////////////////////////////////////////////////

/*  Turn the interrupt source off. 
 * old function name alias: MaskFIQ */
void fLib_DisableFIQ(UINT32 FIQ)
{    
 volatile UINT32 *FIQBase;

    //RichardLin XXX MoreSoFlexible
    FIQBase = (UINT32 *)io_base;    

	if(FIQ<32)
		FIQBase[(FIQ_MASK / sizeof(UINT32))] &= ~(1 << FIQ);
	else
		FIQBase[(EFIQ_MASK / sizeof(UINT32))] &= ~(1 << (FIQ-32));
}

/*  Turn the interrupt source on 
 *  old function name alias: UnmaskFIQ */
void fLib_EnableFIQ(UINT32 FIQ)
{
 volatile UINT32 *FIQBase;

    
    FIQBase = (UINT32 *)io_base;      

	if(FIQ<32)
		FIQBase[(FIQ_MASK / sizeof(UINT32))] |= (1 << FIQ);
	else
		FIQBase[(EFIQ_MASK / sizeof(UINT32))] |= (1 << (FIQ-32));
}


void SetFIQmode(UINT32 FIQ,UINT32 edge)
{
	
 volatile UINT32 *FIQBase;

    
    FIQBase = (UINT32 *)io_base;   

	if(edge){
		if(FIQ<32)
		    	FIQBase[(FIQ_MODE / sizeof(UINT32))] |= (1 << FIQ);
	    else
			FIQBase[(EFIQ_MODE / sizeof(UINT32))] |= (1 << (FIQ-32));
    }
    else{
		if(FIQ<32)
    			FIQBase[(FIQ_MODE / sizeof(UINT32))] &= ~(1 << FIQ);
		else
			FIQBase[(EFIQ_MODE / sizeof(UINT32))] &= ~(1 << (FIQ-32));
    }
	
}	

void SetFIQlevel(UINT32 FIQ,UINT32 low)
{
	
 volatile UINT32 *FIQBase;

    
    FIQBase = (UINT32 *)io_base;   
    
	if(low){ 
		if(FIQ<32)
    			FIQBase[(FIQ_LEVEL / sizeof(UINT32))] |= (1 << FIQ);
		else
			FIQBase[(EFIQ_LEVEL / sizeof(UINT32))] |= (1 << (FIQ-32));
    }
    else{
		if(FIQ<32)
		    	FIQBase[(FIQ_LEVEL / sizeof(UINT32))] &= ~(1 << FIQ);
		else
			FIQBase[(EFIQ_LEVEL / sizeof(UINT32))] &= ~(1 << (FIQ-32));
    }
	
}	

void fLib_ClearFIQ(UINT32 FIQ)
{
 volatile UINT32 *FIQBase;

    
 	FIQBase = (UINT32 *)io_base;

	if(FIQ<32){	
 		FIQBase[(FIQ_CLEAR / sizeof(UINT32))] |= (1 << FIQ);
	}
	else{
 		FIQBase[(EFIQ_CLEAR / sizeof(UINT32))] |= (1 << (FIQ-32));
	}
}


/*
 * Interrupt service routine dispatcher.
 *
 * This is the FIQ handler called when an FIQ trap is taken.
 */
void DispatchFIQ(UINT32 flags)
{
    int FIQ = 0;
    fLib_INT *action;
    

    flags=inw(io_base+FIQ_STATUS);
    // Test to see if there is a flag set, if not then don't do anything
    while (flags != 0)
    {
                
        if ((flags & 0xffff) == 0)
        {
            FIQ += 16;
            flags >>= 16;
        }

        if ((flags & 0xff) == 0)
        {
            FIQ += 8;
            flags >>= 8;
        }

        if ((flags & 0xf) == 0)
        {
            FIQ += 4;
            flags >>= 4;
        }

        if ((flags & 0x3) == 0)
        {
            FIQ += 2;
            flags >>= 2;
        }

        if ((flags & 0x1) == 0)
        {
            FIQ += 1;
            flags >>= 1;
        }

        // Get the vector and process the interrupt.
        action = &FIQVector[FIQ];

        if (action->Handler == NULL)                   
			fLib_printf("No handler installed: %d\n", FIQ);		// can't call printf in int handler
        else 
            action->Handler(FIQ, action->private_arg);
          
         flags =0;
        //flags &= ~1;
    }//end while

	FIQ= 32;
	 flags=inw(io_base+ EFIQ_STATUS);
	 
    // Test to see if there is a flag set, if not then don't do anything
    while (flags != 0)
    {
                
        if ((flags & 0xffff) == 0)
        {
            FIQ += 16;
            flags >>= 16;
        }

        if ((flags & 0xff) == 0)
        {
            FIQ += 8;
            flags >>= 8;
        }

        if ((flags & 0xf) == 0)
        {
            FIQ += 4;
            flags >>= 4;
        }

        if ((flags & 0x3) == 0)
        {
            FIQ += 2;
            flags >>= 2;
        }

        if ((flags & 0x1) == 0)
        {
            FIQ += 1;
            flags >>= 1;
        }

        // Get the vector and process the interrupt.
        action = &FIQVector[FIQ];

        if (action->Handler == NULL)                   
			fLib_printf("No handler installed: %d\n", FIQ);		// can't call printf in int handler
        else 
            action->Handler(FIQ, action->private_arg);
          
         flags =0;
        //flags &= ~1;
    }//end while
}                               /* DispatchFIQ */




/* Called once on start-up to initialise data structures & interrupt routines */
void fLib_Int_Init()
{
    writel( 0x00000000, IP_PA_BASE(0) + IRQ_MASK );
    writel( 0x00000000, IP_PA_BASE(0) + EIRQ_MASK );
    writel( 0x00000000, IP_PA_BASE(0) + FIQ_MASK );
    writel( 0xffffffff, IP_PA_BASE(0) + IRQ_CLEAR );
    writel( 0xffffffff, IP_PA_BASE(0) + EIRQ_CLEAR );
    writel( 0xffffffff, IP_PA_BASE(0) + FIQ_CLEAR );
    writel( PLATFORM_IRQ_TRIGGER_MODE, IP_PA_BASE(0) + IRQ_MODE );
    writel( PLATFORM_IRQ_TRIGGER_MODE >>32, IP_PA_BASE(0) + EIRQ_MODE );
    writel( PLATFORM_FIQ_TRIGGER_MODE, IP_PA_BASE(0) + FIQ_MODE );
    writel( PLATFORM_FIQ_TRIGGER_MODE >>32, IP_PA_BASE(0) + EFIQ_MODE );

    /* FTINTC010: bit 0=active high or rising edge, 1=active low or falling edge. */
    writel( ~PLATFORM_IRQ_TRIGGER_LEVEL, IP_PA_BASE(0) + IRQ_LEVEL );
    writel( (~PLATFORM_IRQ_TRIGGER_LEVEL)>>32, IP_PA_BASE(0) + EIRQ_LEVEL );
    writel( ~PLATFORM_FIQ_TRIGGER_LEVEL, IP_PA_BASE(0) + FIQ_LEVEL );
    writel( (~PLATFORM_FIQ_TRIGGER_LEVEL)>>32, IP_PA_BASE(0) + EFIQ_LEVEL );

    fLib_ResetIRQ();
    fLib_ResetFIQ();

    fLib_WriteIntVector((UINT32 *)IrqV, (PrVoid)fLib_IRQTrap);
    fLib_IRQHandle = DispatchIRQ;

    fLib_WriteIntVector((UINT32 *)FiqV, (PrVoid)fLib_IRQTrap);
    fLib_FIQHandle = DispatchFIQ;
//    fLib_OldIRQTrap = NewIRQ(DispatchIRQ, fLib_NewIRQTrap);
//    fLib_OldFIQTrap = NewFIQ(DispatchFIQ, fLib_NewFIQTrap);

#ifdef CONFIG_EXTERNAL_GIC
    pin_mux_enable(EXT_INT,0);
    gic_init();
#endif
}
