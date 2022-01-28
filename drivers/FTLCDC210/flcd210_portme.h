#ifndef __FLCD210_PORTME_H
#define __FLCD210_PORTME_H

//#define LCD_IRQ LCD_FTLCDC210_IRQ 

void MaskIRQ(UINT32 irq);
void UnmaskIRQ(UINT32 irq);
void EnableIRQ();
void DisableIRQ();
void SetIRQmode(UINT32 irq,UINT32 mode);
void fLib_CloseInt(UINT32 irq);

#endif 	//__FLCD210_PORTME_H