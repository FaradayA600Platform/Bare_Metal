/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:gpio.c                                                              *
* Description: GPIO library routine                                        *
* Author: Fred Chien                                                       *
****************************************************************************/

//#include 	"fLib.h"
#include "SoFlexible.h"
#include "GPIO.h"




UINT32 fLib_Gpio_ReadData(UINT32 io_base)
{
	return inw(io_base + GPIO_DIN_OFFSET);
}                         


void fLib_Gpio_WriteData(UINT32 io_base, UINT32 data)
{
    outw(io_base + GPIO_DOUT_OFFSET, data);
}


// ====================================================================
//	input:
//		dir: 	GPIO_DIR_OUTPUT ==> output
//				GPIO_DIR_INPUT ==> input
// ====================================================================
void fLib_SetGpioDir(UINT32 io_base, UINT32 pin, UINT32 dir)
{
	UINT32 tmp;
	
 	tmp = inw(io_base + GPIO_PINOUT_OFFSET);

 	if(dir==GPIO_DIR_OUTPUT)
 	{
 		tmp |= (1<<pin); 
 	}
 	else
 	{
		tmp &= ~(1<<pin);
	}

    outw(io_base + GPIO_PINOUT_OFFSET, tmp); 
}


void fLib_SetGpioBypass(UINT32 io_base, UINT32 pin)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_PIN_BYPASS);
	tmp |= (1<<pin);
  	outw(io_base + GPIO_PIN_BYPASS, tmp);
}            


void fLib_Gpio_SetData(UINT32 io_base, UINT32 data)
{
    outw(io_base + GPIO_DATASET, data);
}   


void fLib_Gpio_ClearData(UINT32 io_base, UINT32 data)
{
    outw(io_base + GPIO_DATACLR, data);	
}


void  fLib_Gpio_PullEnable(UINT32 io_base, UINT32 pin)
{
	
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_PULLENABLE);
	tmp |= (1<<pin);
  	outw(io_base + GPIO_PULLENABLE, tmp);
}   

void  fLib_Gpio_PullDisable(UINT32 io_base, UINT32 pin)
{
	
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_PULLENABLE);
	tmp &= ~(1<<pin);
  	outw(io_base + GPIO_PULLENABLE, tmp);
}  
   
void  fLib_Gpio_PullHigh(UINT32 io_base, UINT32 pin)
{
	
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_PULLType);
	tmp |= (1<<pin);
  	outw(io_base + GPIO_PULLType, tmp);
}   
   
void  fLib_Gpio_PullLow(UINT32 io_base, UINT32 pin)
{	
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_PULLType);
	tmp &= ~(1<<pin);
  	outw(io_base + GPIO_PULLType, tmp);
}  
   
void  fLib_SetGpioIntEnable(UINT32 io_base, UINT32 pin)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_INT_ENABLE);
	tmp |= (1<<pin);
  	outw(io_base + GPIO_INT_ENABLE, tmp);
}                        


void  fLib_SetGpioIntDisable(UINT32 io_base, UINT32 pin)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_INT_ENABLE);
	tmp &= ~(1<<pin);
  	outw(io_base + GPIO_INT_ENABLE, tmp);
}  

BOOL  fLib_Gpio_IsRawInt(UINT32 io_base, UINT32 pin)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_INT_RAWSTATE);

  	if((tmp & (1<<pin)) != 0)
  		return TRUE;
  	else
  		return FALSE;
}   

void  fLib_SetGpioIntMask(UINT32 io_base, UINT32 pin)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_INT_MASK);
	tmp |= 1<<pin;
  	outw(io_base + GPIO_INT_MASK, tmp);
}                        


void  fLib_SetGpioIntUnMask(UINT32 io_base, UINT32 pin)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_INT_MASK);
	tmp &= ~(1<<pin);
  	outw(io_base + GPIO_INT_MASK, tmp);
}


UINT32 fLib_GetGpioIntMaskStatus(UINT32 io_base)
{
 	return inw(io_base + GPIO_INT_MASKSTATE);	
}


void fLib_ClearGpioInt(UINT32 io_base, UINT32 data)
{
  	outw(io_base + GPIO_INT_CLEAR, data);
}


void fLib_SetGpioTrigger(UINT32 io_base, UINT32 pin, UINT32 trigger)
{
    UINT32 tmp, tmp1;

 	tmp = inw(io_base + GPIO_INT_TRIGGER);

  	if (trigger)
  	{
  		tmp |= (1<<pin);
  		outw(io_base + GPIO_INT_TRIGGER, tmp);
  	}
  	else
  	{
  	    tmp1 |= (1<<pin);
  	    tmp &= ~tmp1;
  		outw(io_base + GPIO_INT_TRIGGER, tmp);  	    
  	}
}


void fLib_SetGpioEdgeMode(UINT32 io_base, UINT32 pin, UINT32 both)
{
    UINT32 tmp,tmp1;

 	tmp = inw(io_base + GPIO_INT_BOTH);

  	if (both)
  	{
  		tmp |= (1<<pin);
  		outw(io_base + GPIO_INT_BOTH, tmp);
  	}
  	else
  	{
  	    tmp1 |= (1<<pin);
  		tmp &= ~tmp1;
  		outw(io_base + GPIO_INT_BOTH, tmp);
  	}
  	
}


void fLib_SetGpioActiveMode(UINT32 io_base, UINT32 pin, UINT32 Active)
{
    UINT32 tmp, tmp1;

 	tmp = inw(io_base + GPIO_INT_RISENEG);

  	if (Active)
  	{
  		tmp |= (1<<pin);
  		outw(io_base + GPIO_INT_RISENEG, tmp);
  	}
  	else
  	{
  	    tmp1 |= (1<<pin);
  	    tmp &= ~tmp1;
  		outw(io_base + GPIO_INT_RISENEG, tmp);  	    
  	}
}


void fLib_EnableGpioInt(UINT32 io_base, UINT32 pin, UINT32 trigger, UINT32 active)
{
 	fLib_SetGpioIntEnable(io_base, pin);
 	fLib_SetGpioIntUnMask(io_base, pin);
 	fLib_SetGpioTrigger(io_base, pin, trigger);	
 	fLib_SetGpioActiveMode(io_base, pin, active);		
}

void fLib_DisableGpioInt(UINT32 io_base, UINT32 pin)
{
 	fLib_SetGpioIntDisable(io_base, pin);
 	fLib_SetGpioIntMask(io_base, pin); 	
}

void fLib_EnableGpioBounce(UINT32 io_base, UINT32 pin, UINT32 clkdiv)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_INT_BOUNCEENABLE);
 	tmp |= (1<<pin);
	outw(io_base + GPIO_INT_BOUNCEENABLE, tmp);
	
 	outw(io_base + GPIO_INT_PRESCALE, clkdiv); 	
}

void fLib_DisableGpioBounce(UINT32 io_base, UINT32 pin)
{
	UINT32 tmp;

 	tmp = inw(io_base + GPIO_INT_BOUNCEENABLE);
 	tmp &= ~(1<<pin);
 	outw(io_base + GPIO_INT_BOUNCEENABLE, tmp);
}
