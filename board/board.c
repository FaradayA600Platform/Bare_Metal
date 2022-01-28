/***************************************************************************
* Copyright  Faraday Technology Corp 2008-2012.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:board.c                                                             *
* Description: SoFlexible board specfic routine                            *
* Author: Fred Chien                                                       *
****************************************************************************/

#include "SoFlexible.h"
#include "serial.h"
#include <stdio.h>
#include "cpat/cp15.h"
//#include "apb_dma.h"


/*============================================================================
 *
 *  routine:     Board_Init()
 *
 *  parameters:  void
 *
 *  description: this routine does platform specific system initialization.
 *
 *  calls:       none
 *
 *  returns:     void
 *
 *============================================================================*/

extern  __rt_lib_init(int,int);
extern  _fp_init(void);

extern int BaseOfFreeMemory;
extern int TopOfFreeMemory;

__attribute__((weak)) int platform_init(void){
    return 0;
}
//void Board_Init(void) __attribute__ ((constructor)); 
UINT32 Esc_Key=0;                
void Board_Init(void)
{
	
	fLib_SerialInit(DEFAULT_CONSOLE,DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);
  	//BSS not init, init the global variable before use!!  	

  	fLib_Int_Init(); 

  	//For arm compilation tools
#ifndef CONFIG_ARMV8
  	__rt_lib_init(BaseOfFreeMemory, TopOfFreeMemory);
#endif
//#ifdef CONFIG_PLATFORM_A369
//	pin_mux_enable(IP_UART3, 0);
//#endif
#ifdef CONFIG_EXTERNAL_GIC
	pin_mux_enable(IP_EXTAHB, 0);
#endif
	
#ifdef CONFIG_JIFFIES
    init_jiffies();
#endif
    platform_init();
}
 


void Do_Delay(UINT32 num)
{
 UINT32 i;
 	for(i=num;i>0;i--); 
}


UINT32 inw(UINT32 *port)
{
	return *port;
}

void outw(UINT32 *port,UINT32 data)
{
 	*port=data;
}

UINT16 inhw(UINT16 *port)
{
 	return *port;
}

void outhw(UINT16 *port,UINT16 data)
{
 	*port=data;
}

UINT8 inb(UINT8 *port)
{
	return *port;
}

void outb(UINT8 *port,UINT8 data)
{
 	*port=data;
}


//Numeric LED routines

/* Initialise the LED(s) to given value - usually 0 */
void fLib_SetLEDs(UINT32 value)
{
  //  volatile UINT32 *LedBank = (UINT32 *) LED_BASE;

  //  *LedBank = value;
}


/*
#if defined(CONFIG_PLATFORM_A369)
	#include "platform_a369.c"
#elif defined(CONFIG_A360)
	#include "platform_a360.c"
#elif defined(CONFIG_FH36)
	#include "platform_fh36.c"
#else
	#include "platform.c" //general
#endif
*/
