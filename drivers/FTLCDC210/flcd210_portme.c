#include <stdio.h>
#include "SoFlexible.h"
void MaskIRQ(UINT32 irq)
{    
	 fLib_DisableIRQ(irq);   
}

void UnmaskIRQ(UINT32 irq)
{    
	 fLib_EnableIRQ(irq);   
}

void EnableIRQ()
{    
	 fLib_EnableIRQ(LCD_FTLCDC210_IRQ);   
}

void DisableIRQ()
{    
	 fLib_DisableIRQ(LCD_FTLCDC210_IRQ);   
}

void SetIRQmode(UINT32 irq,UINT32 mode)
{
	fLib_SetIRQmode(irq, mode);
}

void fLib_CloseInt(UINT32 irq)
{
	fLib_CloseIRQ(irq);
}