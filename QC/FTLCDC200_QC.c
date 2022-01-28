#include "SoFlexible.h"

#include "FTLCDC200/flcd.h"
#include "FTLCDC200/scalar.h"
#include "FTLCDC200/display.h"
#include "FTLCDC200/lcd_config.h"
#include "FTLCDC200/basic_test.h"

#define IPMODULE 		LCDC
#define IPNAME 			FTLCDC200

extern int enable_pip;
extern int enable_pop;

// --------------------------------------------------------------------

extern int RegisterFile_Test(struct faradayfb_info *fbi);
extern void Color_Test(struct faradayfb_info *fbi, int mode);

extern struct faradayfb_info lcd_fbi;

int FTLCDC200_QC_Main()
{
	//unsigned char *pFrameBuffer;
    struct faradayfb_info *fbi = &lcd_fbi;
    int irq[IP_IRQ_NUM],i;
    int val;


#ifdef IP_IRQ_4
/*
	irq[0] = IP_IRQ0(0);		// 22;
	irq[1] = IP_IRQ1(0);		// 25;
	irq[2] = IP_IRQ2(0);		// 24;
	irq[3] = IP_IRQ3(0);		// 23;
*/
 
	for(i=0;i<IP_IRQ_COUNT;i++){
		irq[i] = IP_irq[0][i];	
	}

#else
	irq[ClrBusErr_IRQ] = IP_IRQ0(0); 		// 27
	irq[ClrVstatus_IRQ] = IP_IRQ0(0);		// 27;
	irq[ClrNxtBase_IRQ] = IP_IRQ0(0);		// 27;
	irq[ClrFIFOUdn_IRQ] = IP_IRQ0(0);		// 27;
#endif

	printf("\rPlease set EVB v1.1: SW12 off, SW16 off, SW17 off \n\n");
	RegisterFile_Test(fbi);

	val=Init_LCD(fbi, LCDC_FTLCDC200_PA_BASE, irq, -1, MODE_RGB565);		/// MODE_RGB24);		///MODE_YCbCr422);		// MODE_RGB565
	if (val<0){
		return 0;
	}

	Vertical_Int_Test(fbi);
	Dithering_Test(fbi);

	Do_Basic_Test(fbi, MODE_YCbCr420);
	Do_Basic_Test(fbi, MODE_YCbCr422);
	Do_Basic_Test(fbi, MODE_RGB24);
	Do_Basic_Test(fbi, MODE_RGB565);
	Do_Basic_Test(fbi, MODE_PALETTE_8);
//	Do_Basic_Test(fbi, MODE_PALETTE_4);
//	Do_Basic_Test(fbi, MODE_PALETTE_2);

	printf("going to run scalar\n");
	
	return 0;
}
