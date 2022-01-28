/*
 * Faraday FTLCDC210 LCD controller non os test driver header file
 *
 * (C) Copyright 2022 Faraday Technology
 * Jack Chain <jack_ch@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __FTLCD210_H
#define __FTLCD210_H


/******************************************************************************
 * Include files
 *****************************************************************************/

#include "portme_ftlcdc210.h"

/******************************************************************************
 * Gobal variables
 *****************************************************************************/

extern unsigned int Cursor_Font[];
extern unsigned int OSD_Font_Simple[];
extern unsigned int OSD_MCFont[];
extern unsigned int OSD_Font[];

extern unsigned int OSD_Attribute[];
extern unsigned int OSD_Attribute_Simple[];
 
/******************************************************************************
 * Define Constants
 *****************************************************************************/ 
   
#define LCD_TRUE  1
#define LCD_FALSE   0

enum { RGB, BGR };
enum { LBLP, BBBP, LBBP };

typedef struct LcdMTypeTag
{
   int Valid;
   unsigned char Descriptor[30];
   unsigned int LCDEnable;
   unsigned int PanelPixel;
   unsigned int HorizontalTiming;
   unsigned int VerticalTiming1;
   unsigned int VerticalTiming2;
   unsigned int Polarity;
   unsigned int SerialPanelPixel;
   unsigned int CCIR656;
   unsigned int CSTNPanelControl;
   unsigned int CSTNPanelParam1;
   int Width;
   int Height;
} LCDMTYPE_T;

/******************************************************************************
 * Define Constants
 *****************************************************************************/ 
   
/***************************** LCM *****************************/
//==========================================================================
// LCM command definition
//==========================================================================
#define LCM_CHIPID              (0x00)
#define LCM_MODE_SEL1           (0x01)
#define LCM_MODE_SEL2           (0x02)
#define LCM_VCO_MODE            (0x06)
#define LCM_EB                  (0x3F)
#define LCM_DC                  (0x4A)

#define LCM_SRAM_Position_X     (0x10)
#define LCM_SRAM_Position_Y     (0x11)
#define LCM_SRAM_Control        (0x12)
#define LCM_SRAM_Win_Star_X     (0x13)  // Window Left-UP X point
#define LCM_SRAM_Win_Star_Y     (0x14)  // Window Left-UP Y point
#define LCM_SRAM_Win_End_X      (0x15)  // Window Right-Down X point
#define LCM_SRAM_Win_End_Y      (0x16)  // Window Right-Down Y point

// LCM Timing Control Register
#define LCM_TIME_Tpwh_r(x)      (x << 16)
#define LCM_TIME_Tas(x)         (x << 12)   // Tas Width
#define LCM_TIME_Tah(x)         (x << 8)    // Tah Width
#define LCM_TIME_Tpwl(x)        (x << 4)    // Tpwl Width
#define LCM_TIME_Tpwh_w(x)      (x << 0)    // Tpwh Width

// LCM Byte Select Register
#define LCM_DMA_ENABLE          (0x1 << 2)
#define LCM_INTERFACE_6800      (0x1 << 1)
#define LCM_INTERFACE_8080      (0x0 << 1)
#define LCM_BYTE_SEL_8bit       (0x1 << 0)
#define LCM_BYTE_SEL_32bit      (0x0 << 0)

#define LCM_INTERFACE_MASK      (0x1 << 1)
#define LCM_BYTE_SEL_MASK       (0x1)

#define LCM_WAIT_TIME_OUT       0x1FFF

#define LCM_PANEL_8             (0 << 2)
#define LCM_PANEL_9             (2 << 2)
#define LCM_PANEL_16            (1 << 2)
#define LCM_PANEL_18            (3 << 2)

#define LCM_PANEL_BODR_2_16     0
#define LCM_PANEL_BODR_16_2     1
#define LCM_PANEL_BODR_16       2

#define LCDC_LCM_BODR_2_16      2
#define LCDC_LCM_BODR_16_2      1
#define LCDC_LCM_BODR_16        0

#define LCM_BUS_IF_8            0
#define LCM_BUS_IF_9            1
#define LCM_BUS_IF_16           2
#define LCM_BUS_IF_18           3

#define LCM_PANEL_IF_MONO       0
#define LCM_PANEL_IF_16         1
#define LCM_PANEL_IF_18         2

#define BUS8_TO_RGB565(r,g,b) ((r&0x1f) << 3 | (g&0x38) >> 3 | ((g&0x7) << 13) | (b&0x1f) << 8)
#define BUS8_TO_RGB565_MASTER(r,g,b) ((r&0x1f) << (3+8) | (g&0x38) /*>> 3*/ << 5 | ((g&0x7) << (13-8)) | (b&0x1f) << (8-8))   //because master send data1->data0->data3->data2
#define BUS9_TO_RGB565(r,g,b) ((r&0x3f) << 3 | (g&0x38) >> 3 | (g&0x7) << 22 | (b&0x3f) << 16)
#define BUS16_TO_RGB565(r,g,b) ((b&0x1f) << 0 | (g&0x3f) << 5 | (r&0x1f) << 11)
#define BUS16_TO_RGB565_2_16(r,g,b) ((r&0x30) >> 4 | (r&0xf) << 28 | (g&0x3f) << 22 | (b&0x3f) << 16)
#define BUS16_TO_RGB565_16_2(r,g,b) ((b&0x3) << 30 | (b&0x3c) >> 2 | (g&0x3f) << 4 | (r&0x3f) << 10)
#define BUS18_TO_RGB565 BUS16_TO_RGB565

//bit 0 & bit 9 are not used by WANXIN panel with bus=16-bits, but lcd200 sends data[15:0]. If wire jump is not used, CPU mode in 16-bits also can be used.
#define WANXIN_16BIT_WIRE_JUMPER          //If DMA mode with bus=16-bit is used, define this and then we map data[15:0] in LCDC200 to data[17:10] & data[8:1] in WANXIN MS4#001 according to wire jumper. CPU mode can work whether wire jumper is done or not. Define this according to wire jumper.
#define RGB565(r,g,b) BUS16_TO_RGB565(r,g,b) //translate from RGB565 to LCM interface, used in LCM_CPU_MODE

#if 1
#define LCM_MASTER_MODE    //LCD controller will fetch data automatically
#else
#define LCM_CPU_MODE    //AP. has to manually send the data to LCM panel
#endif

#if 1
#define WXPAT24_MS4_001
#else
#define WXPAT24_MS5_001
#endif

/************************* END LCM *****************************/



/************ Function prototype *********************/
void VBI_Test( unsigned char *pFrameBuffer);

void getEndianString( unsigned char i, char *str );
void clear_Screen( unsigned char *pFrameBuffer, unsigned short panel_width, unsigned short panel_height, unsigned short bpp, unsigned int color );
void Dithering( int enable, unsigned int type );
void Dithering_Test( unsigned char *fb );
void palette_write( unsigned int data);
int  palette_check( unsigned int data);
void palette_write_seq( void );   //write palette sequentially by address
int  palette_check_seq( void );  //be used after palette_write_seq()
void palette_ram_test( unsigned int base);
void Init_LCD( volatile unsigned int LCD_Base, volatile LCDMTYPE_T  *pLCDC, unsigned int lcd_type, unsigned char bpp);
int  RegisterFile_Test( unsigned int base);  //do not test interrupt & GPIO related register
int  Register_Test( int address, unsigned int compare );
void GPIO_Test( void );
void LCD_Handler( void );
void FrameBaseUpdate( void);
void Vertical_Int_Test(void);
void Base_Update_Int_Test( void );
void SetDivNo( unsigned char num  );  //LCD panel clock divisor control. divisor is equal to  (num + 1)
void UnderRun_Handler( void );
void FIFO_UnderRun_test( unsigned char *pFrameBuffer );
/* We use four bits to represent OSD window 1 ~ 4. For example, 0001 -> OSD window 1 is on, while others are off. */
void OSD_On( unsigned int which );  //OSD Enable/disable
void OSD_Off( void );  //OSD disable
void OSD_Pos( unsigned int which, int  HPos, int VPos);
void OSD_Dim( unsigned int which, int HDim, int VDim);   //define the total font number of each row and the total row number
void OSD_WinControl( unsigned int which, int hightlight, int bdType, int bdColor, int shType, int shColor );
void OSD_FontControl( unsigned int TranType, unsigned int HighLightType, unsigned int BdColorSel, unsigned int ShColorSel );
void OSD_Row_Col_Space( unsigned int RowSpace, unsigned int ColSpace);
/*Set the start index (0~511) of OSD font attribute RAM data for window x*/
void OSD_FontAttributeBase( unsigned int which, unsigned short fontIndex );
void OSD_MCFBase( unsigned char *fontbase );
/*HZoomIn=0 ->HScal=0,1; HZoomIn=1 ->HScal=0,1,2,3   VZoomIn=0 ->VScal=0,1; VZoomIn=1 ->VScal=0,1,2,3 */
void OSD_Scal( int HZoomIn, int HScal, int VZoomIn, int VScal);   //define the horizontal and vertical scaling factor
void OSD_putc( char c, int position, unsigned short foreground, unsigned short background ); //map the OSC_Font[], ie. set font index in the font attribute RAM
void OSD_puts( char *str, int position, unsigned short foreground, unsigned short background );
void OSD_Test( char *str);

/*Brightness: -127 ~ 127 (2's complement), SatValue: bit 13 ~ 8*/
void Color_SatBright( unsigned int SatValue, char BrightSign, int Brightness );
/*SigHuXXX: 0->positive, 1->negative, HuXXXValue: uses 6 bit, and must be the same rotation degree*/
void Color_Hue( unsigned int SigHuCos, int HuCosValue, unsigned int SigHuSin, int HuSinValue );
/*each parameter is 8 bit*/
void Color_Sharp( unsigned int K1, unsigned int K0, unsigned int ShTh1, unsigned int ShTh0 );
/*fill the contrast lookup table with each item value (curve), item ranges from 0~63, value is 32 bits*/
void Color_Contrast( unsigned int slope );
/*fill the Red, Green, and Blue Gamma Correction lookup table*/
void Color_FillGammaTable( unsigned int item, int r, int g, int b );
void Load_Image_YCbCr422( char *file, int size, unsigned char *pFrameBuffer, unsigned char endian );
void Load_Image_YCbCr420( char *file, unsigned int width, unsigned int height, unsigned int size, unsigned char *pFrameBuffer, unsigned char endian );
void Load_Image( char *file, int size, unsigned char *pFrameBuffer, unsigned char endian, unsigned int bpp);
void Color_Test( unsigned char *pFrameBuffer );
void animation( void );
void animation_test( unsigned char *pFrameBuffer, char *file, int size );
/*imX value can be 1,2,3*/
void setNumFrameBuffer(unsigned int im0, unsigned int im1, unsigned int im2, unsigned im3); 
int  SetFrameBase( unsigned short image, unsigned short frame, unsigned char  *pFrameBuffer );
unsigned char *allocateFrameBuffer( unsigned int PANEL_WIDTH, unsigned int PANEL_HEIGHT, unsigned short bpp, unsigned int frame_no );
void PoP_On( void );
void PoP_Off( void );
void PoP_ImageScalDown( unsigned int im0, unsigned int im1, unsigned int im2, unsigned int im3 );
void PoP_Test( unsigned char *pFrameBuffer, unsigned short bpp );
/*pipNum: 0 -> PiP_Off(), 1 -> single PiP window, 2 -> double PiP window*/
void PiP_On( unsigned int pipNum );
void PiP_Off( void );
/*make sure that blend1 + blend2 < 16*/
void PiP_Blending( unsigned int on, int blend1, int blend2 );
void PiP_Pos( unsigned int which, int  HPos, int VPos);
void PiP_Dim( unsigned int which, int HDim, int VDim);   //define the total font number of each row and the total row number
void PiP_Test( unsigned char *pFrameBuffer, unsigned short bpp );
void SetBGRSW( unsigned char choice );
void SetBPP( unsigned char bpp);
void SetRGBType( unsigned int rgbtype );
void SetEndian( unsigned char endian );
void SetYCbCr( int type );
unsigned int rgb2ycbcr( unsigned char *r, unsigned char *g, unsigned char *b);
void ColorBar_YCbCr_420( unsigned char endian, unsigned char *base, int width, int height, int int_test);
void ColorBar_YCbCr_422( unsigned char endian, unsigned char *pFrameBuffer, int width, int height, int int_test);
void ColorBar_RGB( unsigned char bpp, unsigned char endian, unsigned char *pFrameBuffer, int width, int height, int int_test);
void Draw_Square( unsigned char *pFrameBuffer, unsigned char bpp, unsigned int thickness );
void disable_lcd_controller( void );
void enable_lcd_controller( void );
void CSTN_Test( unsigned char *pFrameBuffer );
void STN_Test( unsigned char *pFrameBuffer );
void Virtual_Screen_Test(unsigned char *pBuffer, unsigned char bpp);

void reset_lcd_controller(void);
void blocks_4x4( unsigned char *pFrameBuffer, int bpp, unsigned int *color, int adjustable );
void central_square( unsigned char *pFrameBuffer, int bpp, unsigned int color );
void horizontal_bar( unsigned char *pFrameBuffer, int bpp, unsigned int color_pattern, int bar );
void vertical_bar( unsigned char *pFrameBuffer, int bpp, unsigned int color_pattern, int bar, int margin );
void fill_color( unsigned char *pFrameBuffer, int bpp, unsigned int color );
void fill_background( unsigned char *pFrameBuffer, int bpp );
void Cursor_Test(void);
void Scroll_Test( unsigned char *pFrameBuffer, char *file, int img_width, int img_height, int size, int bpp );
void delay_10ms(unsigned int num);
void Draw_color_block( unsigned char *pFrameBuffer, int bpp, unsigned int *color, unsigned int vbar, unsigned int hbar );

#endif

