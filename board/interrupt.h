
#ifndef INTC_H_
#define INTC_H_

#include "types.h"

#if defined(CONFIG_VBAR_REMAP_TEXT_BASE)
    #define IrqV                              (CONFIG_TEXT_BASE+0x18)
    #define FiqV                              (CONFIG_TEXT_BASE+0x1C)
#elif defined(CONFIG_HIGH_VECTOR) 
    #define IrqV                              (0xffff0000+0x18)
    #define FiqV                              (0xffff0000+0x1C)
#elif defined(CONFIG_REDIRECT_VECTOR_BASE)
    #define IrqV                              (CONFIG_REDIRECT_VECTOR_BASE+0x18)
    #define FiqV                              (CONFIG_REDIRECT_VECTOR_BASE+0x1C)
#else //default
    //vector at 0x0
    #define IrqV                            0x18
    #define FiqV                            0x1C
#endif



//Adding 64 bit IRQ/FIQ by Richard Lin 08/11
#define IRQ_IAmIRQ					0
#define IRQ_IAmFIQ					1
 
/*  Interrupt bit positions  */

#define LEVEL                           0
#define EDGE                            1
                         
#define H_ACTIVE                        0
#define L_ACTIVE                        1



/* A function with no argument returning pointer to a void function */

typedef void (*PrVoid) (void);
typedef void (*PrHandler) (UINT32, void *);  /* As PrVoid with one parameter */


typedef struct {
	PrHandler Handler;          /* Routine for specific interrupt */
	UINT32 IntNum; 
	void *private_arg;                //
}fLib_INT;



/*  -------------------------------------------------------------------------------
 *   define in intc.c
 *  -------------------------------------------------------------------------------
 */
  
 
extern UINT32 fLib_ReadIntVector(UINT32 *vect, PrVoid *oldHandler);
extern UINT32 fLib_WriteIntVector(UINT32 *vect, PrHandler newHandler);

extern void   fLib_Int_Init(void);


/* 
 * IRQ API
 */
extern int    fLib_ConnectIRQ( UINT32 IntNum,PrHandler Handler, ...);
extern void   fLib_EnableIRQ(UINT32 IRQ);
extern void   fLib_DisableIRQ(UINT32 IRQ);
extern void   fLib_ClearIRQ(UINT32 IRQ);
extern void   fLib_SetIRQmode(UINT32 IRQ,UINT32 mode);

/* 
 * IRQ_priv API : for SGI, PPI in gic/intc030
 */
extern void fLib_DisableIRQ_priv(UINT32 irq_n);
extern void fLib_EnableIRQ_priv(UINT32 irq_n);
extern int fLib_ConnectIRQ_priv(UINT32 IntNum, PrHandler Handler, ...);

/* 
 * IRQ Legacy API
 */
extern INT32  fLib_CloseIRQ(UINT32 IntNum);
extern void   SetIRQmode(UINT32 IRQ,UINT32 edge);
extern void   SetIRQlevel(UINT32 IRQ,UINT32 low);
extern void   ClearIRQStatus(UINT32 IRQ);

/* 
 * FIQ API
 */
extern void   fLib_EnableFIQ(UINT32 FIQ);
extern void   fLib_DisableFIQ(UINT32 FIQ);
extern void   fLib_ClearFIQ(UINT32 FIQ);


 
extern void fLib_FIQTrap(void);
extern void fLib_IRQTrap(void);


extern PrHandler fLib_FIQHandle ;
extern PrHandler fLib_IRQHandle ;

extern void   DispatchFIQ(UINT32 flags);
extern void   DispatchIRQ(UINT32 flags);


extern fLib_INT  FIQVector[];
extern fLib_INT  IRQVector[PLATFORM_IRQ_TOTALCOUNT];
#if defined(CONFIG_GIC)||defined(CONFIG_FTINTC030)||defined(CONFIG_GIC400)
extern fLib_INT IRQVector_priv[32];
#endif




#endif
                     

