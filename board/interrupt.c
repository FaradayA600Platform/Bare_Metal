#include "SoFlexible.h"
#include <stdarg.h> // for va_list va_start()

fLib_INT IRQVector[PLATFORM_IRQ_TOTALCOUNT];
#if defined(CONFIG_GIC)||defined(CONFIG_FTINTC030)||defined(CONFIG_GIC400)
fLib_INT IRQVector_priv[32];
#endif

/* define IRQ trap and handler routine */
PrVoid fLib_NewIRQTrap = (PrVoid) 0;	
PrHandler fLib_IRQHandle = (PrHandler) 0;


/* define FIQ trap and handler routine */
PrHandler fLib_FIQHandle = (PrHandler) 0;

UINT32 fLib_WriteIntVector(UINT32 *vect, PrHandler newHandler)
{
    UINT32 temp;
    UINT32 *tvec;


    temp=*vect;	
    tvec=vect+8;//32/4=8
    if((temp & 0xFFFFF000)==0xE59FF000)
    {	      
        *tvec=(UINT32)newHandler;
        return TRUE;
    }
    return FALSE;	
}	

int fLib_ConnectInt(fLib_INT * IntVector,UINT32 IntNum, PrHandler Handler, void *args)
{
    fLib_INT *interrupt;


    if (IntNum >= PLATFORM_IRQ_TOTALCOUNT)
        return FALSE;

    if (!Handler)
        return FALSE;

    interrupt = &IntVector[IntNum];

    if (interrupt->Handler)
        return FALSE;//Interrupt handler has assigned

    interrupt->Handler = Handler;   
    interrupt->IntNum=IntNum;
    interrupt->private_arg= args;

    return TRUE;
}

/*
 * Routine to check and install requested interrupt. Just sets up logical
 * structures, doesn't alter interrupts at all.
 * 081202: Richard Lin: Adding appending args examples ~~
 	usage 1: fLib_ConnectInt(IRQ_IAmIRQ, i, handler, (void *)private_point)
 	usage 2: fLib_ConnectInt(IRQ_IAmIRQ, i, handler, NULL)
 	usage 3: fLib_ConnectInt(IRQ_IAmIRQ, i, handler)
 */
int fLib_ConnectIRQ(UINT32 IntNum, PrHandler Handler, ...)
{
    fLib_INT *Irq;

    va_list arg;
    void *string;

    va_start(arg, Handler);
    string = va_arg(arg, char *);
    va_end(arg);

    return fLib_ConnectInt(IRQVector, IntNum, Handler, string);
}

#if defined(CONFIG_GIC)||defined(CONFIG_FTINTC030)
//for PPI & SGI
int fLib_ConnectIRQ_priv(UINT32 IntNum, PrHandler Handler, ...)
{
    fLib_INT *Irq;

    va_list arg;
    void *string;

    va_start(arg, Handler);
    string = va_arg(arg, char *);
    va_end(arg);

    return fLib_ConnectInt(IRQVector_priv, IntNum, Handler, string);
}
#endif

/* Call this routine before trying to change the routine attached to an IRQ */
INT32 fLib_CloseIRQ(UINT32 IntNum)
{   
    if (IntNum >= PLATFORM_IRQ_TOTALCOUNT)
    {
        printf("Can't close Interrupt %d\n", IntNum);
        return FALSE;
    }

    // Disable the interrupt & then remove the handler
    fLib_DisableIRQ(IntNum);
    IRQVector[IntNum].Handler = (PrHandler) 0;
    IRQVector[IntNum].IntNum =  0;

    return TRUE;

}


/* Initialise the IRQ environment without installing interrupt handlers. */
void fLib_ResetIRQ()
{
    int i;     

    /* Mask out all interrupt sources. */
    for (i = 0; i < PLATFORM_IRQ_TOTALCOUNT; i++)
    {	
	    fLib_DisableIRQ(i);
        IRQVector[i].IntNum = 0;
        IRQVector[i].Handler = (PrHandler) 0;      
    }	

}

#if defined(CONFIG_FTINTC010)||defined(CONFIG_FTINTC020)
/*
 * FIQ API : ftintc010 & ftintc020 only 
 */

fLib_INT FIQVector[PLATFORM_FIQ_TOTALCOUNT];


int fLib_ConnectFIQ(UINT32 IntNum, PrHandler Handler, ...)
{
    fLib_INT *Fiq;

    va_list arg;
    void *string;

    va_start(arg, Handler);
    string = va_arg(arg, char *);
    va_end(arg);

    return fLib_ConnectInt(FIQVector, IntNum, Handler, string);
}

/* Initialise the FIQ environment without installing interrupt handlers. */
void fLib_ResetFIQ()
{
    int i;     

   /* Mask out all interrupt sources. */
    for (i = 0; i < PLATFORM_FIQ_TOTALCOUNT; i++)
    {	
		fLib_DisableFIQ(i);
        FIQVector[i].IntNum = 0;
        FIQVector[i].Handler = (PrHandler) 0;      
    }		 	
}

/* Call this routine before trying to change the routine attached to an IRQ */
INT32 fLib_CloseFIQ(UINT32 IntNum)
{   
    if (IntNum >= PLATFORM_FIQ_TOTALCOUNT)
    {
        printf("Can't close Interrupt %d\n", IntNum);
        return FALSE;
    }

    // Disable the interrupt & then remove the handler
    fLib_DisableFIQ(IntNum);
    FIQVector[IntNum].Handler = (PrHandler) 0;
    FIQVector[IntNum].IntNum =  0; 	   	

    return TRUE;

}


#endif //defined(CONFIG_FTINTC010)||defined(CONFIG_FTINTC020)
