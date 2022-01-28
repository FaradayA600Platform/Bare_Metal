#include "SoFlexible.h"
#include "serial.h"

#define SYSTEM_CONTROLLER_PA_BASE

void FTLCDC210_QC_Main(void)
{
	printf("==Start the FTLCDC210_QC test==\n");
       int i =0;
       int val;
       
       //PINMUX
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x844c, 0x81);
       
//       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8124, 0x09);
//       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8130, 0x000403FF);

       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8338, 0x60D60340);
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x833c, 0x00000BFF);
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8338, 0x60D60341);

       //AXI_CLK of LCDC
//       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8118, 0x00000005);
//       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8128, 0x00000300);

       //CA53 Clock Counter Register 5 1920x1080@60Hz / 1280x720@60Hz
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8114, 0x30C43000);


       //FTLCDC210 Initialize
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE, 0x00000000);

       for(i = 0; i <0x100; i=i+4)
       {
            val = ((i + 3) << 0x18) | ((i + 2) << 0x10) | ((i +1) << 0x8) | i;
            vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x600, i);
            vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x700, i);
            vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x800, i);
       }

       //LCD Polarity Control Parameter
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x10c, 0x00000007);
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x100, 0x93572B77);
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x104, 0x04040437);
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x108, 0x00000024);

       //LCD Panel Pixel Parameter
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x4, 0x00000804);

       //LCD Panel Image0 Frame0 Base Address
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE + 0x18, 0x90000000);

       //Enable LCDC
       vLib_LeWrite32(LCD_FTLCDC210_PA_BASE, 0x00010003);

	printf("End the FTLCDC210_QC test\n");
}
