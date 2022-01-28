#ifndef __GPIO_H
#define __GPIO_H

#include	"board.h"        
#include	"types.h"
 
#define GPIO_DOUT_OFFSET            0x0
#define GPIO_DIN_OFFSET             0x4
#define GPIO_PINOUT_OFFSET          0x8
#define GPIO_PIN_BYPASS             0xC
#define GPIO_DATASET                0x10
#define GPIO_DATACLR                0x14
#define GPIO_PULLENABLE             0x18
#define GPIO_PULLType               0x1C
#define GPIO_INT_ENABLE             0x20
#define GPIO_INT_RAWSTATE           0x24
#define GPIO_INT_MASKSTATE          0x28
#define GPIO_INT_MASK               0x2C
#define GPIO_INT_CLEAR              0x30
#define GPIO_INT_TRIGGER            0x34
#define GPIO_INT_BOTH               0x38
#define GPIO_INT_RISENEG            0x3C
#define GPIO_INT_BOUNCEENABLE       0x40
#define GPIO_INT_PRESCALE           0x44	


#define GPIO_NUM               		32
#define GPIO_EDGE             		0
#define GPIO_LEVEL               	1	
#define SINGLE                    	0
#define BOTH                      	1


#define GPIO_DIR_OUTPUT			1
#define GPIO_DIR_INPUT			0


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

extern UINT32 fLib_Gpio_ReadData(UINT32 io_base);
extern void fLib_Gpio_WriteData(UINT32 io_base, UINT32 data);
extern void fLib_Gpio_SetData(UINT32 io_base, UINT32 data);
extern void fLib_Gpio_ClearData(UINT32 io_base, UINT32 data);

extern void  fLib_Gpio_PullEnable(UINT32 io_base, UINT32 pin);  
extern void  fLib_Gpio_PullDisable(UINT32 io_base, UINT32 pin);  
extern void  fLib_Gpio_PullHigh(UINT32 io_base, UINT32 pin);  
extern void  fLib_Gpio_PullLow(UINT32 io_base, UINT32 pin);
extern BOOL  fLib_Gpio_IsRawInt(UINT32 io_base, UINT32 pin);

extern void fLib_EnableGpioInt(UINT32 io_base, UINT32 pin, UINT32 trigger, UINT32 active);
extern void fLib_DisableGpioInt(UINT32 io_base, UINT32 pin);
extern void fLib_EnableGpioBounce(UINT32 io_base, UINT32 pin, UINT32 clkdiv);
extern void fLib_DisableGpioBounce(UINT32 io_base, UINT32 pin);

extern void fLib_SetGpioDir(UINT32 io_base, UINT32 pin, UINT32 dir);
extern void fLib_SetGpioBypass(UINT32 io_base, UINT32 pin);
extern void fLib_SetGpioEdgeMode(UINT32 io_base, UINT32 pin, UINT32 both);

extern UINT32 fLib_GetGpioIntMaskStatus(UINT32 io_base);
extern void fLib_ClearGpioInt(UINT32 io_base, UINT32 data);

extern void  fLib_SetGpioIntMask(UINT32 io_base, UINT32 pin);
extern void  fLib_SetGpioIntUnMask(UINT32 io_base, UINT32 pin);
extern void  fLib_SetGpioIntEnable(UINT32 io_base, UINT32 pin);
extern void  fLib_SetGpioIntDisable(UINT32 io_base, UINT32 pin);
extern void fLib_SetGpioTrigger(UINT32 io_base, UINT32 pin, UINT32 trigger);
extern void fLib_SetGpioActiveMode(UINT32 io_base, UINT32 pin, UINT32 Active);


#endif /* __GPIO_H */
