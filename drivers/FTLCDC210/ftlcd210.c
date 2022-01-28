/*
 * Faraday FTLCDC210 LCD controller non os test driver c file
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
/******************************************************************************
 * Include files
 *****************************************************************************/


#include "ftlcd210.h"
#include "SoFlexible.h"
//#include "fLib.h"
#include <stdio.h>
//#include "timer.h"
//#include "fa52x_cache.h"
#include "ftlcd210_conf.h"
#include "ftlcd210-OSD.h"

#include "PiP_PoP_lib.h"
#include "virtual_screen.h"
#include "scalar.h"
#include "colorbar.h"
#include "portme_ftlcdc210.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/
#define TICK_HZ 100

/******************************************************************************
 * Gobal variables
 *****************************************************************************/
unsigned int LCD_IRQ;         //iqr = 20 in A320D (cpe120 architecture) in interrupt.h
unsigned int PANEL_WIDTH, PANEL_HEIGHT;
unsigned int LCD_PANEL_TYPE;
unsigned int DIVISOR;
char DEFAULT_PATH[50]= "D:\\picture\\";
char FILENAME[80];

unsigned int lcm_bus_width = 16;                //LCM bus interface, could be 8, 9, 16, 18
unsigned int lcm_panel_bodr = LCM_PANEL_BODR_16;      //LCM panel bodr when bus is 16 bit

unsigned int TV_panel_type;
int FIFO_Under_Run = 0;


void VBI_Test(unsigned char *pFrameBuffer);
void Virtual_Screen_YUV_Test(unsigned char *pBuffer, unsigned char bpp);

extern LCDMTYPE_T FLcdModule[];

/******************************************************************************
 *Public functions
 *****************************************************************************/

void delay_10ms(unsigned int num)
{/*
   unsigned int StartTime, EndTime,i;
   
   fLib_Timer_Init(1, TICK_HZ, Timer1_Tick);

   StartTime = fLib_CurrentT1Tick();

   while(1){
      EndTime = fLib_CurrentT1Tick();
        //printf("StartTime=%d, EndTime=%d\n", StartTime, EndTime);
      if(EndTime - StartTime > num)
         break;
      else {
         for (i=0;i<20;i++) ;   // NOP delay 
      }
   }
   fLib_Timer_Close(1);
   */
}

void getEndianString( unsigned char i, char *str )
{
    if (i==0)
       strcpy(str, "LBLP");
    else if (i==1)   
       strcpy(str, "BBBP");
    else
       strcpy(str, "LBBP");
}

void disable_lcd_controller()
{
    if( ( LCD_PANEL_TYPE >= 9 ) && ( LCD_PANEL_TYPE <= 11 ) )  //for STN/CSTN panel
        *(volatile unsigned int *) (LCD_IO_Base + 0x0050) &= 0xfffffeff;   //GPIO, for disable the power

    *(volatile unsigned int *) (LCD_IO_Base) &= 0xfffffffe;
}

void enable_lcd_controller()
{
    *(volatile unsigned int *) (LCD_IO_Base) |= 1;
    if( ( LCD_PANEL_TYPE >= 9 ) && ( LCD_PANEL_TYPE <= 11 ) )  //for STN/CSTN panel
        *(volatile unsigned int *) (LCD_IO_Base + 0x0050) |= 0x00000100;   //GPIO, for enable the power
}

void reset_lcd_controller()
{
    unsigned int temp,i;
    
   //disable_lcd_controller();
   temp = *(volatile unsigned int *) (LCD_IO_Base);
   *(volatile unsigned int *) (LCD_IO_Base) = 0x00;   
   for (i=0;i<10;i++)
      ;
   //delay_10ms(1);  
   *(volatile unsigned int *) (LCD_IO_Base) = temp;   
   //enable_lcd_controller();
}

void Init_LCD( volatile unsigned int LCD_Base, volatile LCDMTYPE_T  *pLCDC, unsigned int lcd_type, unsigned char bpp)
{     
   //*(volatile unsigned int *) (LCD_Base + 0x0000 )= pLCDC[lcd_type].LCDEnable;
   * (volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0004 )= pLCDC[lcd_type].PanelPixel;
   * (volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0100 )= pLCDC[lcd_type].HorizontalTiming;
   * (volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0104 )= pLCDC[lcd_type].VerticalTiming1;
   * (volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0108 )= pLCDC[lcd_type].VerticalTiming2;
   * (volatile unsigned int *)(uintptr_t)(LCD_Base + 0x010C )= pLCDC[lcd_type].Polarity;
   * (volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0200 )= pLCDC[lcd_type].SerialPanelPixel;
#ifdef OLD_CCIR656
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0204 )= pLCDC[lcd_type].CCIR656;
#endif
   printf(">> PanelWidth: %d; PanelHeight: %d\n", 
          pLCDC[lcd_type].Width,
          pLCDC[lcd_type].Height);
   //spade: not used?
   //*(volatile unsigned int *) (LCD_Base + 0x0C00 )= pLCDC[lcd_type].CSTNPanelControl;
   //*(volatile unsigned int *) (LCD_Base + 0x0C04 )= pLCDC[lcd_type].CSTNPanelParam1;
   PANEL_WIDTH = pLCDC[lcd_type].Width;   //set the panel width
   PANEL_HEIGHT = pLCDC[lcd_type].Height; //set the panel height
   DIVISOR = pLCDC[lcd_type].Polarity >> 8;

   //offset 0x50 represents ignoring the contrast lookup table and Red, Green, Blue gamma lookup table, but LF has to -1, VBP has to +1
   //*(volatile unsigned int *) (LCD_Base + 0x0050 ) = 0x7;

   //offset 0x0 bit 12-11, set 1 represents color bar generator
   //*(volatile unsigned int *) (LCD_Base) &= 0xffffe7ff;
   //*(volatile unsigned int *) (LCD_Base) |= 0x00000800;

   //LCD Image Color Management
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x400 ) = 0x2000;
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x404 ) = 0x2000;
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x408 ) = 0x0;
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x40C ) = 0x40000;

   //FrameBuffer Parameter
   //*(volatile unsigned int *) (LCD_Base + 0x0014 ) &= 0x000000FF;

   //Ignore some transformation
   //*(volatile unsigned int *) (LCD_Base + 0x0050 ) &= 0x0;

   //reset the scalar control register
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x1110 ) = 0x0;

   printf("In Init_LCD function, LCD_IO_Base = %x\n", LCD_Base);
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0C ) |= 0xf; //interrupt status clear
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x08 ) &= ~0xf;
//   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x08 ) |= 0xf; //enable interrupt
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x08 ) |= 0xB; //enable interrupt
   *(volatile unsigned int *)(uintptr_t)LCD_Base &= ~0x3;
   *(volatile unsigned int *)(uintptr_t)LCD_Base |= 0x3;  //Turn on LCD screen and LCD controller   
//   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x04 ) &= 0xfffff9f7;
//   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x04 ) |= 0x00000608;   // enable panel, start of vertical front porch, HBUSTREQM is raised when DMA FIFO has eight or more empty locations

   //*(volatile unsigned int *) (LCD_Base + 0x1c ) = 0x00013a21; // Turn on LCD ie.enable LCD controller, TFT panel, big endian byte, big endian pixel,screen,
   SetBPP( bpp );                                              // start of vertical front porch, HBUSTREQM is raised when DMA FIFO has eight or more empty locations
   //set SetDivNo in each LCD panel parameter, not set it here
   //SetDivNo(DIVISOR); //????
   //LF = LF -1 for all input format. we have do this in panel.h
   //*(unsigned int *) (LCD_IO_Base + 0x0104) -= 1;
   //suppose we will use dithering for all the test, except AUO serial panel 3.6", CSTN, need modify further
   //*(volatile unsigned int *)(LCD_Base + 0x0) |= (1 << 18);  /* 2nd out put */
   
   if (TV_panel_type !=0) {
      unsigned int temp;
      
      temp = *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x100 );
      temp = temp & 0xff00ffff;
      temp = temp | 0x00490000;
      *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x100 ) = temp;      
   
      temp = *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x104 );
      temp = temp & 0x00ffffff;
      temp = temp | 0x1D000000;
      *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x104 ) = temp;   
         
      temp = *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x10C );
      temp = temp & 0xffff00ff;
      *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x10C ) = temp;   
      
      *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x230 ) = 0x03;
      
      temp = *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x00 );
      temp = temp | 0x0040000;
      *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x00 ) = temp;    
      
      
       *(volatile unsigned int *)(uintptr_t)0x90100080 = 0x3e0;  //0x200;  // change priority                      
   }
}


void clear_Screen( unsigned char *pFrameBuffer, unsigned short panel_width, unsigned short panel_height, unsigned short bpp, unsigned int color )
{
    int i, j, tmp_bpp=bpp;

    if( bpp == 24 )  tmp_bpp = 32;  //24bit also uses 4 bytes

    for( i=0; i<panel_height; i++ ) //bpp should be 16
        for( j=0; j<panel_width*(tmp_bpp/8); j+=4)
            *(volatile unsigned int *)(uintptr_t)(pFrameBuffer + i*panel_width*(tmp_bpp/8) + j) = color;
}


/********************* Dithering *************************/
/* type 00: 888 to 565, 01: 888 to 555, 02: 888 to 444 */
void Dithering( int enable, unsigned int type )
{
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) &= ~(0x3 << 12);
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base) &= ~(1 << 6);    //disable dithering

   if (enable != 0) {
      *(volatile unsigned int *)(uintptr_t)LCD_IO_Base |= 0x40;
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) |= (type << 12);
   }
}

//use 24bit picture in 16bit panel will find certain color is different(should pick better picture)
void Dithering_Test( unsigned char *fb )  
{
   unsigned int i, j;
   unsigned int *ptr;

   Init_LCD(LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);    
   //SetDivNo(DIVISOR);
   printf("Start dithering testing\n");
         
   for (j = 0;j < 3;j++) {
         
        ptr = (unsigned int *)(uintptr_t)fb;
       for (i = 0;i < PANEL_WIDTH * PANEL_HEIGHT;i++)
         *ptr++ = 0xffffffff;
      
      strcpy(FILENAME, DEFAULT_PATH);
      
      Dithering(1, j);
            
      if ((PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240)) {
         strcat(FILENAME, "17_888_320x240.bmp.bin");
         Load_Image( FILENAME, 307200, fb, LBBP, 24 );
      } 
      else if ((PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480)) {
         strcat(FILENAME, "17_888_640x480.bmp.bin");
         Load_Image( FILENAME, 1228800, fb, LBBP, 24 );
      }     
       delay_10ms(500);    
   }
   //SetDivNo(DIVISOR);
   Dithering( 0, 0 );
}


/************************ Test GPI/GPO *************************/
// /*not sure, errorous*/

void GPIO_Test()
{
    unsigned char success=1;

    *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C) =  0x00FF0000;
    if(  *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C)  != 0x000000FF )   // bit 7-0 : LCDGPI, bit 15-8 : LCDGPO, actually assign 0x000000FF
        success =0;

    *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C) =  0x00AA0000;
    if(  *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C)  != 0x000000AA )
        success =0; 

    *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C) =  0x00550000;
    if(  *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C)  != 0x00000055 )
        success =0;    

    *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C) =  0x00000000;
    if(  *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base+ 0x4C)  != 0x00000000 )
        success =0;

    if( success ) printf("GPIO PASS\n");
    else printf("GPIO Fail\n");          
}    
/****************** end not sure *****************/



/************************ Test interrupts ********************************/

unsigned char VerticalInt = 0;
unsigned char BaseUpdateInt = 0;

void LCD_Handler()
{
   unsigned int status;

   status = *(volatile unsigned char *)(uintptr_t)(LCD_IO_Base + 0x10);
   *(volatile unsigned int *)(uintptr_t)( LCD_IO_Base + 0x08 )= 0x00000000;   //disable all LCD interrupt

   *(volatile unsigned char *)(uintptr_t)(LCD_IO_Base + 0x0C)  = 0x04;   //clear the interrupt status of vertical duration comparison
   if( status & 0x4) //vertical duration comparison, set when one of the four vertical duration is reached
      VerticalInt = 1; 
   //if (status & 0x2)
   
   //MaskIRQ(LCD_IRQ);
   //DisableIRQ();
}


void Vertical_Int_Test()
{
}

/*{
   fLib_ConnectInt(LCD_IRQ, LCD_Handler); //interrupt -> VerticalInt = 1, cpe120-> irq = 20, ftlcdc200 uses irq=29
         //It enables ARM's IRQ in the Boot.s     
   //EnableIRQ(); 

   VerticalInt = 0;
   *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000004;//vertical duration comparison interrupt table is enabled

   UnmaskIRQ(LCD_IRQ);
   SetIRQmode(LCD_IRQ, LEVEL);

   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) &= 0xfffff9ff;
   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) |= 0x00000000;  //generate interrupt @ start of vertical sync

   EnableIRQ();
   //Because the fresh rate is too fast, so the VerticalInt is replaced soon
   do {
      if( VerticalInt == 1 ) {
         printf("Interrupt testing: start of vertical sync  OK\n");
      }
      break;
   } while (1);

   if (*(volatile unsigned char *)(LCD_IO_Base + 0x10))
      printf("Interrupt status bit clear fail\n");
   else
      printf("Interrupt status bit clear OK\n");

   VerticalInt = 0;
   *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000004;

   //UnmaskIRQ(LCD_IRQ);
   //SetIRQmode(LCD_IRQ,LEVEL);

   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) &= 0xfffff9ff;  
   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) |= 0x00000200;  //generate interrupt @ start of vertical back porch
   UnmaskIRQ(LCD_IRQ);

   //EnableIRQ(); 

   do {
      if( VerticalInt == 1 ) {
         printf("Interrupt testing: start of vertical porch  OK\n");
      }
      break;
   } while (1);

   if(*(volatile unsigned char *)(LCD_IO_Base + 0x10))
      printf("Interrupt status bit clear fail\n");
   else
      printf("Interrupt status bit clear OK\n");

   VerticalInt = 0;
   *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000004;

   //UnmaskIRQ(LCD_IRQ);
   //SetIRQmode(LCD_IRQ,LEVEL);

   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) &= 0xfffff9ff;  
   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) |= 0x00000400;  //generate interrupt @ start of vertical active image
   UnmaskIRQ(LCD_IRQ);
   //EnableIRQ();

   do {
      if( VerticalInt == 1 ) {
         printf("Interrupt testing: start of vertical active image  OK\n");
      }
      break;
   } while (1);
   if (*(volatile unsigned char *) (LCD_IO_Base + 0x10))
      printf("Interrupt status bit clear fail\n");
   else
      printf("Interrupt status bit clear OK\n");

   VerticalInt = 0;
   *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000004;

   //UnmaskIRQ(LCD_IRQ);
   //SetIRQmode(LCD_IRQ,LEVEL);

   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) &= 0xfffff9ff;  
   *(volatile unsigned int *) ( LCD_IO_Base + 0x04 ) |= 0x00000600;  //generate interrupt @ start of vertical front porch
   UnmaskIRQ(LCD_IRQ);

   //EnableIRQ();

   do {
      if( VerticalInt == 1 ) {
         printf("Interrupt testing: start of vertical front porch OK\n");
      }
      break;
   } while (1);
   if (*(volatile unsigned char *) (LCD_IO_Base + 0x10) )
      printf("Interrupt status bit clear fail\n");
   else
      printf("Interrupt status bit clear OK\n");

   fLib_CloseInt(LCD_IRQ);
   *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000000; //disable all interrupt
}*/


void Base_Update_Int_Test()
{
   volatile unsigned int BaseUpdateInt = 0;

   /*fLib_ConnectInt(LCD_IRQ, LCD_Handler);
   *(volatile unsigned int *)(uintptr_t)( LCD_IO_Base + 0x08 ) = 0x00000000;   //disable all LCD interrupt
   *(volatile unsigned int *)(uintptr_t)( LCD_IO_Base + 0x0C ) = 0x02;   //clear the frame buffer base addr. updated interrupt status
   *(volatile unsigned int *)(uintptr_t)( LCD_IO_Base + 0x08 ) = 0x00000002;   //IntNxtBaseEn
   UnmaskIRQ(LCD_IRQ);
*/
   SetFrameBase( 0, 0, (unsigned char*)0x1000000 ); //assume that 0x1000000 is the framebuffer

   do {
         if( BaseUpdateInt == 1 )
            printf("Interrupt testing: Next frame base updated OK\n");
         break;
      } while (1);

   SetFrameBase( 0, 0, (uintptr_t)0 );
   //MaskIRQ(LCD_IRQ);
   //fLib_CloseInt(LCD_IRQ);
}

void SetDivNo( unsigned char num  )  //LCD panel clock divisor control. divisor is equal to  (num + 1)
{
   if( num >= 64 )
      printf("wrong divisor \n");
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x10C) &= ~(unsigned int)(0x7f << 8);
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x10C) |= num << 8;
}


void UnderRun_Handler()
{
   //DisableIRQ();
   //MaskIRQ(LCD_IRQ); 

   if ((*(volatile unsigned char *)(uintptr_t)(LCD_IO_Base + 0x10)) & 0x1)  //FIFO under-run, set when FIFO being read is not ready
   {
      *(volatile unsigned char *)(uintptr_t)(LCD_IO_Base + 0x0C) = 0x01; //clear the FIFO under-run interrupt status
      //*(volatile unsigned char *) (LCD_IO_Base + 0x08) = 0x01;  //enable FIFO UnderRun Interrupt
         FIFO_Under_Run = 1;
   }
}


/*In raw RGB mode*/
void FIFO_UnderRun_test( unsigned char *pFrameBuffer )
{
// unsigned char *pFrameBuffer;
   int result;
   int  i, j;
   int  bpp=16;

   setNumFrameBuffer(1, 0, 0, 0);
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);  // 16 bpp  
   SetDivNo(DIVISOR);

   if (SetFrameBase( 0, 0, pFrameBuffer))
      printf("Set framebase error\n");

   for (i = 0; i < PANEL_HEIGHT;i++)
   {
      for (j = 0;j < (PANEL_WIDTH / 3) * 2;j += 2)
         *(unsigned int *)(uintptr_t)(pFrameBuffer + i * PANEL_WIDTH *2 + j) = 0x001f001f; //red bar. In raw RGB mode
      for (j=(PANEL_WIDTH/3) *2 ; j< (PANEL_WIDTH*2/3) * 2; j=j+2)
         *(unsigned int *)(uintptr_t)(pFrameBuffer + i * PANEL_WIDTH *2 + j) = 0x07E007E0;  // green bar
      for (j= (PANEL_WIDTH*2/3) *2; j<PANEL_WIDTH * 2; j=j+2)
         *(unsigned int *)(uintptr_t)(pFrameBuffer + i * PANEL_WIDTH *2 + j) = 0xF100F100; // blue bar
   }

   //fLib_ConnectInt(LCD_IRQ,  UnderRun_Handler );   //it only can be
   *(volatile unsigned int *)(uintptr_t)( LCD_IO_Base + 0x0C )= 0x00000001; //clear the FIFO under-run status
   *(volatile unsigned int *)(uintptr_t)( LCD_IO_Base + 0x08 )= 0x00000001; //enable FIFO under-run interrupt table

   //UnmaskIRQ(LCD_IRQ);
   //SetIRQmode(LCD_IRQ,LEVEL);  

   //EnableIRQ();

   SetDivNo(0); //FIFO does not under run if SetDivNo(>=3)

   printf("Check if your screen flash, press 'y' or 'Y'\n");
   do {
      result = ftldcd210_get_char();
   } while( result != 'y');

   if (FIFO_Under_Run)
      printf("FIFO underrun interrupt is generated\n");
   else
      printf("FIFO under run interrupt fail\n");

   //recover to the original settings
   *(volatile unsigned char *)(uintptr_t)(LCD_IO_Base + 0x0C) = 0x01;
   SetDivNo(DIVISOR);
   //DisableIRQ();
}

#if 0 
/******** Just for testing. Now(2006/3/15) it's error.**********/

void Load_Image_YCbCr422_from_YCbCr420( char *file, int size, unsigned char *pFrameBuffer, unsigned char endian )
{
   int i;
   FILE *fptr;
   unsigned char *buffer, *YPtr, *CbPtr, *CrPtr;

// OSD_Off();
   SetYCbCr(422);
// SetDivNo(DIVISOR); 
   SetBPP(16); //bpp must be 8 in YCbCr420 mode  
   SetEndian( endian );
  
   YPtr  = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 16, 1);;   //uses original declaring framebuffer  //allocateFrameBuffer(width, height, 16);
   CbPtr = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 16, 1);
   CrPtr = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 16, 1);

   setNumFrameBuffer(1, 0, 0, 0);

   //initialize frame buffer first
   SetFrameBase(0,0, pFrameBuffer);

   //There should be some endian problem
   if( (fptr = fopen( file, "rb") ) == 0 ) 
      printf("No picture to show\n");  // the file is in little-endian on Intel x86

   buffer = allocateFrameBuffer( PANEL_WIDTH, PANEL_HEIGHT, 16, 1 );
   if( fread(buffer, 1, size, fptr) == 0 )
      printf("File open error \n");
   else
      fclose(fptr);
   //Read data from 0x11000000 and write to FB(0x12000000)
    //0x11000000  ->   0x12000000   76800 bytes
    //0x11012c00  ->   0x12080000   19200 bytes
    //0x11017700  ->   0x120A0000   19200 bytes

    for(i=0;i</*19200*/PANEL_WIDTH * PANEL_HEIGHT / 4;i++){
      *( (unsigned int *)(uintptr_t)(YPtr) + i ) = *( (unsigned int *)(uintptr_t)(buffer) + i );
    }
    for(i=0;i</*4800*/PANEL_WIDTH * PANEL_HEIGHT / (4*4);i++){
      *( (unsigned int *)(CbPtr) + i*2 ) = *( (unsigned int *)(uintptr_t)(buffer + /*19200*4*/PANEL_WIDTH * PANEL_HEIGHT) + i );
      
      if( i+1 == PANEL_WIDTH * PANEL_HEIGHT / (4*4) ) 
         *( (unsigned int *)(uintptr_t)(CbPtr) + i*2+1 ) = *( (unsigned int *)(uintptr_t)(buffer + /*19200*4*/PANEL_WIDTH * PANEL_HEIGHT) + i );
      else 
         *( (unsigned int *)(uintptr_t)(CbPtr) + i*2+1 ) = (*( (unsigned int *)(uintptr_t)(buffer + /*19200*4*/PANEL_WIDTH * PANEL_HEIGHT) + i ) + *( (unsigned int *)(buffer + /*19200*4*/PANEL_WIDTH * PANEL_HEIGHT) + i+1 ) ) / 2 ;

      *( (unsigned int *)(uintptr_t)(CrPtr) + i*2 ) = *( (unsigned int *)(uintptr_t)(buffer + /*19200*4 + 4800*4*/PANEL_WIDTH * PANEL_HEIGHT + PANEL_WIDTH * PANEL_HEIGHT / 4) + i );

      if( i+1 == PANEL_WIDTH * PANEL_HEIGHT / (4*4) ) 
         *( (unsigned int *)(uintptr_t)(CrPtr) + i*2+1 ) = *( (unsigned int *)(uintptr_t)(buffer + /*19200*4 + 4800*4*/PANEL_WIDTH * PANEL_HEIGHT + PANEL_WIDTH * PANEL_HEIGHT / 4) + i );
      else 
         *( (unsigned int *)(uintptr_t)(CrPtr) + i*2+1 ) = (*( (unsigned int *)(uintptr_t)(buffer + /*19200*4 + 4800*4*/PANEL_WIDTH * PANEL_HEIGHT + PANEL_WIDTH * PANEL_HEIGHT / 4) + i ) + *( (unsigned int *)(buffer + /*19200*4 + 4800*4*/PANEL_WIDTH * PANEL_HEIGHT + PANEL_WIDTH * PANEL_HEIGHT / 4) + i+1 ) ) / 2;
    }
    
    for(i=0;i<PANEL_WIDTH * PANEL_HEIGHT / 2 + PANEL_WIDTH * PANEL_HEIGHT / 4; i=i+4 )
    {
      *(unsigned char *)(uintptr_t)( pFrameBuffer + i ) = *( (unsigned int *)(uintptr_t)(YPtr) + (int)(i/2) );
      *(unsigned char *)(uintptr_t)( pFrameBuffer + i+1 ) = *( (unsigned int *)(uintptr_t)(CbPtr) + (int)(i/4) );
      *(unsigned char *)(uintptr_t)( pFrameBuffer + i+2 ) = *( (unsigned int *)(uintptr_t)(YPtr) + (int)(i/2)+1 );
      *(unsigned char *)(uintptr_t)( pFrameBuffer + i+3 ) = *( (unsigned int *)(uintptr_t)(CrPtr) + (int)(i/4) );
   }

    free(buffer);
    free(YPtr);
    free(CbPtr);
    free(CrPtr);
}



/*********** It(YCbCr420 & 422) needs to add endian testing. **********/

void Load_Image_YCbCr422( char *file, int size, unsigned char *pFrameBuffer, unsigned char endian )
{
   int i;
   FILE *fptr;

    printf("Load Y422..%s\n", file);
    
// OSD_Off();
   SetYCbCr(422);
// SetDivNo(DIVISOR); 
   SetBPP(16); //bpp must be 16 in YCbCr422 mode  
   SetEndian(endian);
  
   //There should be some endian problem if no BBBP handling
   if( (fptr = fopen( (const char *__restrict)file, "rb") ) == 0 ) 
      printf("No picture to show\n");  // the file is in little-endian on Intel x86
   
   if( (endian == LBLP) || (endian == LBBP) ) {
      if( fread(pFrameBuffer, 4, size/4, fptr) == 0 )
         printf("File open error \n");
   }
   else if( endian == BBBP)
   {
      unsigned char *buffer, *ptr1;

      ptr1 = buffer = malloc(size); 
      
        if (buffer == 0 ) {
           printf("memory allocate fail\n");
           fclose(fptr);           
           return;
      }
      
      if( fread( buffer, 4, size/4, fptr) == 0 ) {
         printf("File open error \n");
         free(buffer);        
           fclose(fptr);   
           return;      
       }

//    for(i=0; i<size; i++)
//    {
//       if( ( i%4 == 0 ) || ( i%4 == 1 ) )
//          *( unsigned char * )( pFrameBuffer + i+2 ) = *buffer++;
//       if( ( i%4 == 2 ) || ( i%4 == 3 ) )
//          *( unsigned char * )( pFrameBuffer + i-2 ) = *buffer++;  
//    }    
       
      for(i=0; i<size/4; i++)
      {           
         *( pFrameBuffer + 2) = *buffer++;
         *( pFrameBuffer + 3) = *buffer++;
         *( pFrameBuffer + 0) = *buffer++;
         *( pFrameBuffer + 1) = *buffer++;         
          pFrameBuffer+=4;             
      }     
      buffer=ptr1;
      free(buffer);      
   }
      fclose(fptr);
    printf("Done\n");      
}


void Load_Image_YCbCr420( char *file, unsigned int width, unsigned int height, unsigned int size, unsigned char *pFrameBuffer, unsigned char endian )
{
   int i;
   FILE *fptr;
   unsigned char *buffer,*buf2;  //data maybe override in PiP mode when buffer is released, and other buffer is used.
    
    printf("Load Y420...%s\n", file);
    
// OSD_Off();
   SetYCbCr(420);
// SetDivNo(DIVISOR); 
   SetBPP(8);  //bpp must be 8 in YCbCr420 mode  
   SetEndian( endian );

   setNumFrameBuffer(1, 1, 1, 0);

   //There should be some endian problem if no BBBP handling
   if( (fptr = fopen( file, "rb") ) == 0 ) 
      printf("No picture to show\n");  // the file is in little-endian on Intel x86

   buf2 = buffer = pFrameBuffer;  //allocateFrameBuffer( width, height, 8, 1 );
   
   //printf("buffer=0x%x\n", (unsigned int)buffer);
   if ((endian == LBLP) || (endian == LBBP)) {
      if( fread(buffer, 4, size/4, fptr) == 0 )
         printf("File open error \n");
    }
   else if( endian == BBBP)
   {
      unsigned char *buf, *ptr1;

      ptr1 = buf = malloc(size); 
      
        if (buf == 0 ) {
           printf("memory allocate fail\n");
           fclose(fptr);           
           return;
      }
      
      if( fread( buf, 4, size/4, fptr) == 0 ) {
         printf("File open error \n");
         free(buf);        
           fclose(fptr);   
           return;      
       }


//    for(i=0; i<size; i++)
//    {
//       if( i%4 == 0 )
//          *( unsigned char * )( buffer + i+3 ) = *buf++;
//       if( i%4 == 1 )
//          *( unsigned char * )( buffer + i+1 ) = *buf++;  
//       if( i%4 == 2 )
//          *( unsigned char * )( buffer + i-1 ) = *buf++;
//       if( i%4 == 3 )
//          *( unsigned char * )( buffer + i-3 ) = *buf++;  
//          
//    }
      
      for(i=0; i<size/4; i++)
      {
         *(buffer+3) = *buf++;
         *(buffer+2) = *buf++;   
         *(buffer+1) = *buf++;
         *(buffer+0) = *buf++;   
         buffer+=4;
      }
            
       free(ptr1);   
   }

    buffer = buf2;
             
   SetFrameBase(0,0, buffer);
   SetFrameBase(1,0, buffer + width * height);
   SetFrameBase(2,0, buffer + width * height + width * height / 4);
    
   fclose(fptr);
    //free(buffer);
    //SetFrameBase(0,0, pFrameBuffer);
    printf("Done\n");    
}
#endif
void Load_Image( char *file, int size, unsigned char *pFrameBuffer, unsigned char endian, unsigned int bpp)
{
   FILE *fptr;
   int  i;

   SetYCbCr(0);
   SetBPP(bpp);   //bpp must be 8 in YCbCr420 mode  
   SetEndian( endian );

    printf("Load I...%s\n", file);
#if 0  //Mark
   if( (fptr = fopen( file, "rb") ) == 0 ) { 
      printf("No picture to show\n"); 
      return;
   } // the file is in little-endian on Intel x86

   if( endian == LBLP || endian == LBBP || ((endian == BBBP) && (bpp == 24)) ) {
      if( fread(pFrameBuffer, 4, size/4, fptr) == 0 )
         printf("File open error case 1\n");
   }     
   else if( (endian == BBBP) && (bpp == 16) )   //used to fix BBBP in 16bpp (not in 24bpp) because the file opens in LBLP   
   {
      unsigned char *buffer, *ptr1;

      ptr1 = buffer = malloc(size); 

        if (buffer == 0 ) {
           printf("memory allocate fail\n");
           fclose(fptr);           
           return;
      }
      
      if( fread( buffer, 4, size/4, fptr) == 0 ) {
         printf("File open error case 2\n");
         free(buffer);        
           fclose(fptr);   
           return;      
       }

/*
      for(i=0; i<size; i++)
      {
         if( ( i%4 == 0 ) || ( i%4 == 1 ) )
            *( unsigned char * )( pFrameBuffer + i+2 ) = *buffer++;
         if( ( i%4 == 2 ) || ( i%4 == 3 ) )
            *( unsigned char * )( pFrameBuffer + i-2 ) = *buffer++;
      }
*/    
      for(i=0; i<size/4; i++)
      {           
         *( pFrameBuffer + 2) = *buffer++;  
         *( pFrameBuffer + 3) = *buffer++;  
         *( pFrameBuffer + 0) = *buffer++;  
         *( pFrameBuffer + 1) = *buffer++;         
          pFrameBuffer+=4;             
      }
   
        buffer = ptr1;  
      free(buffer);
   }
   fclose(fptr);
  #endif 
    printf("Done\n");   
}

/************************** Frame Buffer Parameters ************************/
/*imX value can be 0,1,2,3*/
void setNumFrameBuffer(unsigned int im0, unsigned int im1, unsigned int im2, unsigned im3)
{
#if 0 //feature is no longer supported
   //if imX == 0, then give one frame buffer to each window
   if( im0 > 0 )  im0--; 
   if( im1 > 0 )  im1--; 
   if( im2 > 0 )  im2--; 
   if( im3 > 0 )  im3--;

   if( im0 > 2 || im1 > 2 || im2 > 2 || im3 > 2 )
      printf("Number of frame buffer error\n");
      
   *(volatile unsigned int *) (LCD_IO_Base + 0x0014) &= 0xffffff00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x0014) |= im0 | (im1 << 2) | (im2 << 4) | (im3 << 6);
#endif
}
 
int SetFrameBase( unsigned short image, unsigned short frame, unsigned char  *pFrameBuffer )
{  
   //spade: seems no frame=1,2
   if( image > 4 || frame > 2 ){ //value of image = 0,1,2,3 and value of frame = 0,1,2
      printf("Frame base set error\n");
      return -1;
   }

   if (image == 4) //VBI base addr
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x5C) = (unsigned int) (uintptr_t)pFrameBuffer;
    else 
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x18 + (image*3+frame)*0x4) = (unsigned int) (uintptr_t)pFrameBuffer;

   return 0;
} 


unsigned char *allocateFrameBuffer(unsigned int width,unsigned int high,unsigned short bpp,unsigned int frame_no )
{

   unsigned int *pFrameBuffer;

    printf("allocate width = %d, high=%d, bpp=%d, frame_no=%d\n", width,high,bpp,frame_no);
    if (bpp == 8)
        pFrameBuffer = malloc( width*high * frame_no ); 
    else if (bpp == 24)
        pFrameBuffer = malloc( width*high * 4 * frame_no );
    else
      pFrameBuffer = malloc( width*high * (bpp/8) * frame_no ); 

    if (pFrameBuffer == 0 ) 
       printf("memory allocate fail\n");
    
    return ((unsigned char *)(uintptr_t)pFrameBuffer);
/*

   unsigned char *pFrameBuffer;

    //if( bpp == 8 )
    //    pFrameBuffer = malloc( (PANEL_WIDTH* PANEL_HEIGHT * 3/2 * frame_no ) + 0x3f);    // Maximum xxbpp frame_no Frame for motion piture, 0x3f is used as a margin for alignment
    //else if(bpp == 24)
    //    pFrameBuffer = malloc( (PANEL_WIDTH* PANEL_HEIGHT * 4 * frame_no ) + 0x3f);
    //else
    //   pFrameBuffer = malloc( (PANEL_WIDTH* PANEL_HEIGHT * (bpp/8) * frame_no ) + 0x3f);    // Maximum xxbpp frame_no Frame for motion piture, 0x3f is used as a margin for alignment

    if( bpp == 8 )
        pFrameBuffer = malloc( (width* high * 3/2 * frame_no ) + 0x3f);    // Maximum xxbpp frame_no Frame for motion piture, 0x3f is used as a margin for alignment
    else if(bpp == 24)
        pFrameBuffer = malloc( (width* high * 4 * frame_no ) + 0x3f);
    else
      pFrameBuffer = malloc( (width* high * (bpp/8) * frame_no ) + 0x3f);    // Maximum xxbpp frame_no Frame for motion piture, 0x3f is used as a margin for alignment


    if(pFrameBuffer == 0 ) printf("memory allocate fail\n");
    
    pFrameBuffer += 0x3F;  //should not be done here, or free() may fail owing to wrong pointer
    pFrameBuffer = (unsigned char *) ((unsigned int ) pFrameBuffer & 0xffffffc0) ;  //alignment
    return pFrameBuffer;            
*/    
}


/******************************** Color Mode Test ******************************************/

void SetBGRSW( unsigned char choice )
{
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) &= 0xffffffef;

   if( choice == BGR )
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) |= 0x00000010;

   reset_lcd_controller();
}

void SetBPP( unsigned char bpp)
{
    unsigned int temp;
    
   temp = *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04);

   switch( bpp)
   {
      case 1:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) = (temp & 0xFFFFFFF8) | 0x00000000;
         break;
      case 2:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) = (temp & 0xFFFFFFF8) | 0x00000001;
         break;
      case 4:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) = (temp & 0xFFFFFFF8) | 0x00000002;
         break;
      case 8:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) = (temp & 0xFFFFFFF8) | 0x00000003;
         break;
      case 16:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) = (temp & 0xFFFFFFF8) | 0x00000004;
         break;  
      case 24:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) = (temp & 0xFFFFFFF8) | 0x00000005;
         break;
   }
   reset_lcd_controller();
}          

#if 0
//set when bpp = 16
void SetRGBType( unsigned int rgbtype )
{
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) &= 0xfffffe7f;

   switch( rgbtype )
   {
      case 565:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) |= 0x00000000;
         break;
      case 555:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) |= 0x00000080;
         break;
      case 444:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) |= 0x00000100;
         break;
   }
   reset_lcd_controller();
}
#endif

void SetEndian( unsigned char endian )
{
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) &= 0xFFFFFF9F;

   switch( endian )
   {
      case LBLP:
         break;
      case BBBP:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) |= 0x00000020;
         break;
      case LBBP:
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x04) |= 0x00000040;
         break;
   }
//   reset_lcd_controller();

}


void SetYCbCr( int type )
{
   unsigned int temp;
   
   temp = *(volatile unsigned int *)(uintptr_t)LCD_IO_Base;
   
   if( type == 422)
      *(volatile unsigned int *)(uintptr_t)LCD_IO_Base = (temp & 0xfffffff3) | 0x00000008;         
   else if( type == 420)
     *(volatile unsigned int *)(uintptr_t)LCD_IO_Base = (temp & 0xfffffff3) | 0x0000000C;
   else
   *(volatile unsigned int *)(uintptr_t)LCD_IO_Base = temp & 0xFFFFFFF3;  //RGB   

   reset_lcd_controller();
}

#if 0
unsigned int rgb2ycbcr( unsigned char *r, unsigned char *g, unsigned char *b)
{
   unsigned char r_tmp, g_tmp, b_tmp;

   r_tmp = *r;
   g_tmp = *g;
   b_tmp = *b;

   /* SOFTFPA */
   *r= (unsigned char) (0.299   * r_tmp + 0.587  * g_tmp + 0.114  * b_tmp);        //Y =　0.299 R + 0.587　G　 + 0.114 B
   *g= (unsigned char) (-0.1687 * r_tmp - 0.3313 * g_tmp + 0.5    * b_tmp   +128);  //Cb = - 0.1687R - 0.3313G  + 0.5　B + 128
   *b= (unsigned char) (0.5     * r_tmp - 0.4187 * g_tmp - 0.0813 * b_tmp  + 128); //Cr = 0.5 R - 0.4187G - 0.0813 B + 128

   return ((*r << 16) | (*g << 8) | (*b << 0));
}


//Draw_Square can be used to test the boundary of the panel
//thickness should be even, unit = pixel(s)
void Draw_Square( unsigned char *pFrameBuffer, unsigned char bpp, unsigned int thickness )
{
   int i, j, pixel;

   if (bpp == 24)
      bpp = 32;   //the upper 8 bits are don't care
   SetBPP( bpp );

    printf ("hi = %d\n", PANEL_HEIGHT);
    
   for (i=0; i<PANEL_HEIGHT; i++) {     
      for (j=0; j< PANEL_WIDTH * (bpp/8) ; j++) {
          pixel = (uintptr_t)(pFrameBuffer + i * PANEL_WIDTH*(bpp/8) + j);
      
         if( (i < thickness) || (i >= ( PANEL_HEIGHT - thickness) ) ) 
            *(unsigned int *)(uintptr_t)(pixel) = 0xffffffff;       
         else{
            if( (j < thickness*(bpp/8)) || (j >= (PANEL_WIDTH*(bpp/8) - thickness*(bpp/8) ) ) )
               *(unsigned int *)(uintptr_t)(pixel) = 0xffffffff;
            else
               *(unsigned int *)(uintptr_t)(pixel) = 0x0;
         }
      }
   }
      
}

//****************************************** For MBU Test
#if 0
#define SQUARE 50
#define SQUARE_COLOR    RGB16(0x00, 0x00, 0x00);
#define BK_COLOR        RGB16(0x00, 0x4D, 0x7C);
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define RGB16(r,g,b) (((b >> 3) << 0) | ((g >> 2) << 5) | ((r >> 3) << 11))

void draw_square(unsigned short *frame_base, int x, int y, int lcd_w, int lcd_h)
{
   int i, j;
   int width, height;
   int lcd_size = lcd_w * lcd_h;

   width = MIN(SQUARE, (lcd_w-x));
   height = MIN(SQUARE, (lcd_h-y));    

   //printf("Draw square width=%d, height=%d at (%d, %d)\n", width, height, x, y);

   // white background
   for (i=0; i<lcd_size; i++)
      frame_base[i] = BK_COLOR;

   for (i=y; i<y+height; i++) {
      for (j=x; j<x+width; j++)
         frame_base[i*lcd_w+j] = SQUARE_COLOR;
   }
}
#endif
//**************************************************************

void Patgen_Test()
{
   int i, bpp=16;

   //Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
   //Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);
   //Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
   
   SetYCbCr(422);

   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
   for(i=0; i<4; i++)
      PiP_PoP_Image_Format1( i, 565, LBLP, bpp );

   PiP_Priority_Sequence( 0, 2, 1, 3);
   PiP_On(2);
   PiP_Blending(0, 0, 0);

   PiP_Pos(1, 41, 50);  //HPos needs to be odd if YCbCr422 is used
   PiP_Dim(1, PANEL_WIDTH/2, PANEL_HEIGHT/2);

   PiP_Pos(2, 111, 100);
   PiP_Dim(2, PANEL_WIDTH/2, PANEL_HEIGHT/2);

   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
   PoP_On();

   SetBPP(16);
}

void patgen_interlace()
{
   int i, j, k=0;
   int width = 1440;
   int height = 960;
   int Bpp = 2;
   unsigned char *pFrameBuffer;
   //unsigned char *pFrameBuffer1;
   unsigned char *buffer;
   FILE *fptr;
   char infile[80]= "D:\\FTLCDC200\\FA5A320NOOS_FTLCDC200\\picture\\79_565_1440x960.bmp.bin";
   char outfile[80]= "D:\\FTLCDC200\\FA5A320NOOS_FTLCDC200\\picture\\79_565_1440x960.bmp.bin.i";
   
   
   pFrameBuffer = malloc(width* 1 * Bpp );
   //pFrameBuffer1 = malloc(width* height * 3 / 2);
   buffer = malloc(width* height * Bpp);
   if( (fptr = fopen(infile, "rb") ) == 0 ){ 
      printf("No picture to show\n"); return;} 
   if( fread(buffer, 1, width* height * Bpp, fptr) == 0 )
      printf("File open error \n");
   if( (fptr = fopen( outfile, "wb") ) == 0 ){ 
      printf("No picture to show\n"); return;} 
   /*for(i=0; i<height; i++){
     for(j=0; j<width * Bpp/2; j++)
     pFrameBuffer[i  * height + j] = buffer[i * height + j];//even
     }*/
   for(i=0; i<height; i++){
      if(i%2 == 0){
         k++;
         for(j=0; j<width * Bpp; j++)
            pFrameBuffer[j] = buffer[i * width * Bpp + j];//even
         if( fwrite(pFrameBuffer, 1, width* 1 * Bpp, fptr) == 0 )
            printf("File open error \n");
      }
   }
   fclose( (FILE *)infile);

   for(i=0; i<height; i++){
      if(i%2 != 0){
         for(j=0; j<width * Bpp; j++)
            pFrameBuffer[j] = buffer[i * width * Bpp  + j];//even
         if( fwrite(pFrameBuffer, 1, width* 1 * Bpp, fptr) == 0 )
            printf("File open error \n");
      }
   }
   //if( fwrite(pFrameBuffer1, 1, width* height * Bpp / 2, fptr) == 0 )
   // printf("File open error \n");
   free(pFrameBuffer);
   //free(pFrameBuffer1);
   free(buffer);
}

void TV_Initialize(unsigned int TV_type) 
{
#ifndef OLD_CCIR656
   if (TV_type==5) {          
      *(volatile unsigned int *) ( LCD_IO_Base ) |= 0x00002000;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x204 ) = 0 | (1 << 1) | (0 << 2) | (0 << 3);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x208 ) = 1716 | (525 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x20C ) = 4 | (266 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x218 ) = 240 | (240 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x21C ) = 268 | (80 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x220 ) = 80;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x224 ) = 1280;            
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 22 | (23 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 0;
   }
   else if (TV_type==6) {              
      *(volatile unsigned int *) ( LCD_IO_Base ) |= 0x00002000;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x204 ) = 0 | (1 << 1) | (0 << 2) | (0 << 3);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x208 ) = 1716 | (525 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x20C ) = 4 | (266 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x218 ) = 240 | (240 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x21C ) = 268 | (0 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x220 ) = 0;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x224 ) = 1440;           
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 22 | (23 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 0;
   }
   else if (TV_type==7) {           
      *(volatile unsigned int *) ( LCD_IO_Base ) |= 0x00002000;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x204 ) = 0 | (1 << 1) | (0 << 2) | (0 << 3);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x208 ) = 1728 | (625 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x20C ) = 1 | (313 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x218 ) = 240 | (240 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x21C ) = 280 | (80 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x220 ) = 80;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x224 ) = 1280;           
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 22 | (25 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 2 | (24 << 12);
   }
   else if (TV_type==8) {              
      *(volatile unsigned int *) ( LCD_IO_Base ) |= 0x00002000;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x204 ) = 0 | (1 << 1) | (0 << 2) | (0 << 3);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x208 ) = 1728 | (625 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x20C ) = 1 | (313 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x218 ) = 288 | (288 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x21C ) = 280 | (0 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x220 ) = 0;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x224 ) = 1440;           
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 22 | (25 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 2;
   }
#endif   
}
#endif

static void FTLCDC210_set_gamma(volatile unsigned int base, unsigned int i, unsigned int val)
{
   unsigned int reg;
   unsigned int offset = i & ~0x3;
   unsigned int shift = (i % 4) * 8;
   unsigned int mask = 0xff << shift;

   val = val > 0xff ? 0xff : val;
   val <<= shift;

   reg = readl(base + offset);
   reg &= ~mask;
   reg |= val;
   writel(reg, base + offset);
}


static void FTLCDC210_set_gamma_red(volatile unsigned int base, unsigned int i, unsigned int val)
{
   FTLCDC210_set_gamma(base + 0x600, i, val);
}

static void FTLCDC210_set_gamma_green(volatile unsigned int base, unsigned int i, unsigned int val)
{
   FTLCDC210_set_gamma(base + 0x700, i, val);
}

static void FTLCDC210_set_gamma_blue(volatile unsigned int base, unsigned int i, unsigned int val)
{
   FTLCDC210_set_gamma(base + 0x800, i, val);
}

int FTLCDC210_Test_Main()
{
   int i;
   unsigned int val;
   unsigned int result_panel;
   unsigned char *pFrameBuffer;
   unsigned int *ptr;
//   unsigned int temp;
   //unsigned char *pFrameBuffer1;
   //unsigned char *pFrameBuffer2;
   

   //enable_fa52x_cache();
   //patgen_interlace();
   
   do {
      printf("\nWhat kind of Panel?\n");
#if 0   	  
      printf("(0) Parallel TFT LCD - AUO A036QN01, with CPLD on A320D\n");
      printf("(1) Serial   TFT LCD - AUO A036QN01, bypass CPLD on A320D\n");
      printf("(2) Parallel TFT LCD - Sharp LQ057Q3DC02 on A320D\n");
      printf("(3) Parallel TFT LCD - Sharp LQ084V1DG21 on A320D\n");
      printf("(4) Parallel TFT LCD - PRIME VIEW PD035VX2 on A320D\n");
      printf("(5) CCIR656  TFT-LCD - KOROLA 6.5'' NTSC 640x480 on A320D\n");
      printf("(6) CCIR656  TFT-LCD - KOROLA 6.5'' NTSC 720x480 on A320D\n");
      printf("(7) CCIR656  TFT-LCD - KOROLA 6.5'' PAL 640x480 on A320D\n");
      printf("(8) CCIR656  TFT-LCD - KOROLA 6.5'' PAL 720x576 on A320D\n");
      printf("(9) VGA      TFT-LCD - ViewSonic VA720 Monitor on A320D\n");
#endif	  
      printf("(10) HD      TFT-LCD - Acer p236HL 1920x1080p Monitor\n");
      scanf("%d", &result_panel);
      if (result_panel <= 10)
         break;
   } while (1);
   LCD_IRQ = 11;

   switch( result_panel )
   {
#if 0
      case 0:  //LCD TFT - AUO A036QN01 with CPLD on, ie. accept the parallel signal
         LCD_PANEL_TYPE = 0;  
         printf("320x240 AUO A036QN01 TFT, with CPLD (Parallel) on A320D\n");
         break;
      case 1:  //LCD TFT - AUO A036QN01 with CPLD off, ie. accept the serial signal
         LCD_PANEL_TYPE = 1;  
         printf("320x240 AUO A036QN01 TFT, bypass CPLD (Serial) on A320D\n");
         break;
      case 2:     //LCD TFT - Sharp LQ057Q3DC02
         LCD_PANEL_TYPE = 2;
         printf("320x240 Sharp LQ057Q3DC02 TFT on A320D\n");
         break;
      case 3:     //LCD TFT - Sharp LQ084V1DG21
         LCD_PANEL_TYPE = 3;  
         printf("640x480 Sharp LQ084V1DG21 TFT on A320D\n");
         break;
      case 4:     //LCD TFT - Sharp LQ084V1DG21
         LCD_PANEL_TYPE = 4;  
         printf("640x480 PRIME VIEW PD035VX2 TFT on A320D\n");
            break;			
         case 5: 
            LCD_PANEL_TYPE = 5;  
         printf("640x480 KOROLA 6.5'' TFT-LCD TV (CCIR656) NTSC on A320D\n");          
            TV_Initialize(LCD_PANEL_TYPE);
         break;
		 
         case 6:
            LCD_PANEL_TYPE = 6;
         printf("720x480 KOROLA 6.5'' TFT-LCD TV (CCIR656) NTSC on A320D\n");             
            TV_Initialize(LCD_PANEL_TYPE);
             break;
      case 7:
            LCD_PANEL_TYPE = 7;
            printf("640x480 KOROLA 6.5'' TFT-LCD TV (CCIR656) PAL on A320D\n");           
            TV_Initialize(LCD_PANEL_TYPE);
            break;
       case 8:
            LCD_PANEL_TYPE = 8;
         printf("720x576 KOROLA 6.5'' TFT-LCD TV (CCIR656) PAL on A320D\n");           
            break;
      case 9:     //LCD TFT - Sharp LQ057Q3DC02
             LCD_PANEL_TYPE = 15;
            printf("720x576 ViewSonic 17'' VA720 LCD Monitor on A320D\n");
            break;
#endif			
      case 10:     //LCD TFT - Sharp LQ057Q3DC02
             LCD_PANEL_TYPE = 16;
            printf("Acer 1920x1080 p236HL LCD Monitor \n");
            break;			
   }        
#if 0 
    // if LCD panel, ask for TV out at the same time
    TV_panel_type = 0;
    if (LCD_PANEL_TYPE<=4) { 
       unsigned int c;
       
      printf("Would you like to enable TV output (1/0) ?\n");
      scanf("%d", &c);
      printf("\r\n");
      if (c==1) {
         do {     
           printf("\nWhat kind of TV Panel?\n");
           printf("(0) Nothing\n");
           printf("(5) CCIR656  TFT-LCD - KOROLA 6.5'' NTSC 640x480 on A320D\n");
           printf("(6) CCIR656  TFT-LCD - KOROLA 6.5'' NTSC 720x480 on A320D\n");
           printf("(7) CCIR656  TFT-LCD - KOROLA 6.5'' PAL 640x480 on A320D\n");
           printf("(8) CCIR656  TFT-LCD - KOROLA 6.5'' PAL 720x576 on A320D\n");
           printf("(9) VGA      TFT-LCD - ViewSonic VA720 Monitor on A320D\n");
           scanf("%d", &result_panel);
           if (((result_panel <= 9) && (result_panel >=5)) || (result_panel ==0))
             break;
         } while (1);             
          if ((result_panel <= 9) && (result_panel >=5)) {
             TV_panel_type = result_panel;
             TV_Initialize(TV_panel_type);             
          }
       }
    }

   printf("Please input the picture directory, [type '!' to use default path %s]:\n", DEFAULT_PATH);
   scanf("%s", FILENAME);
   if (( FILENAME[0] != '!' ) || (strlen(FILENAME)==0))  
      strcpy( DEFAULT_PATH, FILENAME);



/********************** end not yet ************************/

/* //Generic LCD Controller Registers Access Test
   RegisterFile_Test( LCD_IO_Base); //do not test interrupt & GPIO related register
   //palette_ram_test can just be used on COMPLEX OSD VERSION!
   palette_ram_test( LCD_IO_Base);  
*/
   //offset 0x54: bit3->gamma bypass, bit2->color management bypass, bit1->YCbCr to RGB bypass, bit0-> RGB to YCbCr bypass
   //*(unsigned int *)(LCD_IO_Base + 0x54) = 0xf;
   //initialize the contrast lookup table and the Red, Green, Blue gamma lookup table
   //fill the straight line x-y=0 to the contrast and gamma lookup table

   for (i = 0;i < 64;i++) {
      val = 0x03020100 + 0x04040404 * i;
      Color_FillGammaTable( i, val, val, val );
   }
 

   for (i = 0; i < 256; i++) {
      FTLCDC210_set_gamma_red(LCD_IO_Base, i, i);
      FTLCDC210_set_gamma_green(LCD_IO_Base, i, i);
      FTLCDC210_set_gamma_blue(LCD_IO_Base, i, i);
   }
#endif     
   //Auxiliary (GPI/GPO/Polarity Control) Test
   //GPIO_Test();
    printf("put the faraday.raw into 0xc0000000 and press 'esc'\n");
    while(0x1b !=fLib_getchar());

   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
//   pFrameBuffer = allocateFrameBuffer(PANEL_WIDTH*2, PANEL_HEIGHT*2, 8, 3 );
   pFrameBuffer = (unsigned char *)(0xC0000000);//allocateFrameBuffer( PANEL_WIDTH*2, PANEL_HEIGHT*2, 16, 100 );

   printf("Framebuffer1 allocated, address = %x \n", (unsigned int)(uintptr_t)pFrameBuffer);
   //setNumFrameBuffer(1,0,0,0);
   
   //Interrupt test
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);   
   SetEndian(LBLP);
   SetFrameBase(0,0,pFrameBuffer);
	printf("------------------------------------------------------------------\n");   
	printf("check the monitor show the image or not\n");
	printf("if yes : pass\n");   
	printf("if no : fail\n");      
	printf("Press 'q' to quit the test\n");
	printf("------------------------------------------------------------------\n");      
	while(0x71 !=fLib_getchar() );   
#if 0

   
/* //Interrupt Test (Vertical Duration Comparison/Next Frame Base Address Updated/FIFO Under-run), need be put after Init_LCD() and SetFrameBase()
   Vertical_Int_Test();
   Base_Update_Int_Test();
   FIFO_UnderRun_test( pFrameBuffer ); 
*/ /* no bus err test */
   
   //fill frame buffer values to 0xffffffff, ie. clear the screen
   ptr = (unsigned int *) pFrameBuffer;
   for (i = 0;i < PANEL_WIDTH*PANEL_HEIGHT;i++)
      *ptr++ = 0xffffffff;   

   //SetFrameBase(0,0,pFrameBuffer);
   
/*************** End just for debug ************/
   do{
      printf("\nWhat kind of test?\n");
      printf("(0) VBI Test\n");
      printf("(1) Draw Square\n");
      printf("(2) Dithering Test\n");
      printf("(3) Draw Color Bar RGB Test and OSD Test\n");
      printf("(4) Draw Color Bar YUV Test and OSD Test\n");
      printf("(5) Draw YUV Image Test\n");
      printf("(6) Draw RGB Image Test\n");
      printf("(7) Color Management Test\n");
      printf("(8) Cursor_Test\n");
      printf("(9) Scalar Test\n");
      printf("(10) PiP Test\n");
      printf("(11) PoP Test\n");
      printf("(12) Virtual Screen Test\n");
      printf("(13) PoP and Virtual Screen Test\n");
      printf("(14) 8bpp 800*600 color bar for TFT/TV output\n");
      printf("(15) TFT+TV:TV scalaring down 800*600->640*480\n");
      printf("(16) TV flicker\n");
                  
      scanf("%d", &result_panel);
      if(result_panel > 16)
         break;
      switch(result_panel) {
      case 0:  
         SetFrameBase(0,0,pFrameBuffer);
         //Patgen_Test();  //still need to do...      
         VBI_Test( pFrameBuffer);
         //CSTN_Test( pFrameBuffer );  //bus width = 8
         //STN_Test( pFrameBuffer );   //bus width = 4
         //STN_160x80_Test( pFrameBuffer );  //bus width = 1
         break;
      case 1:
         SetFrameBase(0,0,pFrameBuffer);
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
         Draw_Square( pFrameBuffer, 16, 2 );
         delay_10ms(100);
         break;
      case 2:
         SetFrameBase(0,0,pFrameBuffer);
         Dithering_Test( pFrameBuffer );
         break;
      case 3:
         SetFrameBase(0,0,pFrameBuffer);
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);
         //Test RGB Palette's Red, Green, and Blue
         ColorBar_RGB( 1, LBLP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         OSD_Test( "RGB LBLP 1BBP");  
            
         //ColorBar_RGB( 1, BBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB BBP 1BPP"); 
         
         //ColorBar_RGB( 1, LBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB LBBP 1BPP"); 
         
         //ColorBar_RGB( 2, LBLP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB LBLP 2BPP"); 
         
         ColorBar_RGB( 2, BBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
            OSD_Test("RGB BBP 2BPP"); 
               
         //ColorBar_RGB( 2, LBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB LBBP 2BPP"); 
         
         //ColorBar_RGB( 4, LBLP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB LBLP 4BPP"); 
         
          //ColorBar_RGB( 4, BBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB BBP 4BPP"); 
         
          ColorBar_RGB( 4, LBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
          OSD_Test("RGB LBBP 4BPP"); 
          
          ColorBar_RGB( 8, LBLP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
          OSD_Test("RGB LBLP 8BPP"); 
          
          //ColorBar_RGB( 8, BBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB BBP 8BPP"); 
         
         //ColorBar_RGB( 8, LBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT,0);
         //OSD_Test("RGB LBBP 8BPP"); 
         break;
 #if 0        
      case 4:
         SetFrameBase(0,0,pFrameBuffer);
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 8);         
         //Test YCbCr420's Red, Green, and Blue 
            ColorBar_YCbCr_420( LBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 0);
         //      OSD_Test("YCBCR 420 LBBP");
            ColorBar_YCbCr_420( BBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 0);
         //      OSD_Test("YCBCR 420 BBBP");
            ColorBar_YCbCr_420( LBLP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 0);
         //      OSD_Test("YCBCR 420 LBLP");

         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);   
         //Test YCbCr422's Red, Green, and Blue
            ColorBar_YCbCr_422( LBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 0);     
         //   OSD_Test("YCBCR 422 LBBP");      
         ColorBar_YCbCr_422( BBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 0);
         // OSD_Test("YCBCR 422 BBBP");      
         ColorBar_YCbCr_422( LBLP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 0);     
         //   OSD_Test("YCBCR 422 LBLP");  
         break;

      case 5:
         //YCbCr422 Test
         //fixed---> YCbCr420 does not work for LCD TV Panel???看起來好像是memory allocation的問題，memory中data那一塊空間不足
         //YCbCr420 Test
         
         SetFrameBase(0,0,pFrameBuffer);
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 8);      
         printf("YCbCr420 LBLP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
            strcat(FILENAME, "16_420_320x240.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 115200, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "20_420_640x480.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 460800, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "16_420_720x480.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 518400, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "41_420_720x576.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 622080, pFrameBuffer, LBLP );
         }
         delay_10ms(300);        
         reset_lcd_controller();

         printf("YCbCr420 BBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "11_420_320x240.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 115200, pFrameBuffer, BBBP );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "15_420_640x480.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 460800, pFrameBuffer, BBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "14_420_720x480.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 518400, pFrameBuffer, BBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "43_420_720x576.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 622080, pFrameBuffer, BBBP );
         }
         delay_10ms(300);        
         reset_lcd_controller();

         printf("YCbCr420 LBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "20_420_320x240.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 115200, pFrameBuffer, LBBP );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "20_420_640x480.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 460800, pFrameBuffer, LBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "17_420_720x480.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 518400, pFrameBuffer, LBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "45_420_720x576.bmp.yuv");
            Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 622080, pFrameBuffer, LBBP );
         }
         delay_10ms(300);        
         reset_lcd_controller();

         //YCbCr422 Test
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);     
         printf("YCbCr422 LBLP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "16_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "16_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "6_422_720x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 691200, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "16_422_720x576.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 829440, pFrameBuffer, LBLP );
         }
         delay_10ms(300);        
         reset_lcd_controller();

         printf("YCbCr422 BBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "11_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, pFrameBuffer, BBBP );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "11_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, pFrameBuffer, BBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "33_422_720x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 691200, pFrameBuffer, BBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "11_422_720x576.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 829440, pFrameBuffer, BBBP );
         }
         delay_10ms(300);        
         reset_lcd_controller();

         printf("YCbCr422 LBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
            strcat(FILENAME, "16_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, pFrameBuffer, LBBP );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "16_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, pFrameBuffer, LBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "45_422_720x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 691200, pFrameBuffer, LBBP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "16_422_720x576.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 829440, pFrameBuffer, LBBP );
         }
         delay_10ms(300);        
         reset_lcd_controller();
         break;
  #endif       
      case 6:
         //RGB565 Test
         SetFrameBase(0,0,pFrameBuffer);
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);     
         printf("RGB565 LBLP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "12_565_320x240.bmp.bin");
            Load_Image( FILENAME, 153600, pFrameBuffer, LBLP, 16 );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "12_565_640x480.bmp.bin");
            Load_Image( FILENAME, 614400, pFrameBuffer, LBLP, 16 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "12_565_720x480.bmp.bin");
            Load_Image( FILENAME, 691200, pFrameBuffer, LBLP, 16 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "46_565_720x576.bmp.bin");
            Load_Image( FILENAME, 829440, pFrameBuffer, LBLP, 16 );
         }
         reset_lcd_controller();

         printf("RGB565 BBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "20_565_320x240.bin");
            Load_Image( FILENAME, 153600, pFrameBuffer, BBBP, 16 );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "20_565_640x480.bin");
            Load_Image( FILENAME, 614400, pFrameBuffer, BBBP, 16 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "20_565_720x480.bmp.bin");
            Load_Image( FILENAME, 691200, pFrameBuffer, BBBP, 16 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "49_565_720x576.bmp.bin");
            Load_Image( FILENAME, 829440, pFrameBuffer, BBBP, 16 );
         }
         reset_lcd_controller();
         delay_10ms(300);

         printf("RGB565 LBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "12_565_320x240.bmp.bin");
            Load_Image( FILENAME, 153600, pFrameBuffer, LBBP, 16 );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "12_565_640x480.bmp.bin");
            Load_Image( FILENAME, 614400, pFrameBuffer, LBBP, 16 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "16_565_720x480.bmp.bin");
            Load_Image( FILENAME, 691200, pFrameBuffer, LBBP, 16 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "50_565_720x576.bmp.bin");
            Load_Image( FILENAME, 829440, pFrameBuffer, LBBP, 16 );
         }
         reset_lcd_controller();


         //RGB888 Test
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);     
         printf("RGB888 LBLP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "17_888_320x240.bmp.bin");
            Load_Image( FILENAME, 307200, pFrameBuffer, LBLP, 24 );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "17_888_640x480.bmp.bin");
            Load_Image( FILENAME, 1228800, pFrameBuffer, LBLP, 24 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "20_888_720x480.bmp.bin");
            Load_Image( FILENAME, 1382400, pFrameBuffer, LBLP, 24 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "62_888_720x576.bmp.bin");
            Load_Image( FILENAME, 1658880, pFrameBuffer, LBLP, 24 );
         }
         reset_lcd_controller();

         printf("RGB888 BBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "14_888_320x240.bmp.bin");
            Load_Image( FILENAME, 307200, pFrameBuffer, BBBP, 24 );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "14_888_640x480.bmp.bin");
            Load_Image( FILENAME, 1228800, pFrameBuffer, BBBP, 24 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "16_888_720x480.bmp.bin");
            Load_Image( FILENAME, 1382400, pFrameBuffer, BBBP, 24 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "63_888_720x576.bmp.bin");
            Load_Image( FILENAME, 1658880, pFrameBuffer, BBBP, 24 );
         }
         reset_lcd_controller();

         printf("RGB888 LBBP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "17_888_320x240.bmp.bin");
            Load_Image( FILENAME, 307200, pFrameBuffer, LBBP, 24 );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            strcat(FILENAME, "62_888_640x480.bmp.bin");
            Load_Image( FILENAME, 1228800, pFrameBuffer, LBBP, 24 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "62_888_720x480.bmp.bin");
            Load_Image( FILENAME, 1382400, pFrameBuffer, LBBP, 24 );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "68_888_720x576.bmp.bin");
            Load_Image( FILENAME, 1658880, pFrameBuffer, LBBP, 24 );
         }
         //Color_Test( pFrameBuffer );
         reset_lcd_controller();
         break;

      case 7:
          //Color Management Test (Contrast/Brightness/Sharpness/Saturation/Hue/Gamma)    
         //OSD Test
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
         OSD_Test("BIG OSD OK");             //have to define COMPLEX_OSD
         SetFrameBase(0,0,pFrameBuffer);
         Color_Test( pFrameBuffer );
         break;

      case 8:
         //Hardware Cursor Test
         SetFrameBase(0,0,pFrameBuffer);
         Cursor_Test();
         break;

      case 9:
         //Scalar Test
         SetFrameBase(0,0,pFrameBuffer);
         if( LCD_PANEL_TYPE == 6 ) {   //new version of CCIR656 does not support scalar currently
            TV_Scalar_Test(pFrameBuffer,16);
            TV_Scalar_Test(pFrameBuffer,24);//speed up AHB 
         }
         else if ( LCD_PANEL_TYPE <= 4) {
             if (( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 576 )) {         
               Scalar_Test_720_576(pFrameBuffer,16 );
              Scalar_Test_720_576(pFrameBuffer,24);
            }
            else if (( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 480 )) {
               Scalar_Test_720_480(pFrameBuffer,16 );
              Scalar_Test_720_480(pFrameBuffer,24);                   
            }
            else if (( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 )) {
               Scalar_Test_640_480(pFrameBuffer,16 );
              Scalar_Test_640_480(pFrameBuffer,24);            
            }
            else if (( PANEL_WIDTH == 320 ) && ( PANEL_HEIGHT == 240 )) {
               Scalar_Test_320_240(pFrameBuffer,16 );
              Scalar_Test_320_240(pFrameBuffer,24);            
            }
            else if (( PANEL_WIDTH == 240 ) && ( PANEL_HEIGHT == 80 )) {
               Scalar_Test_240_80(pFrameBuffer,16 );
              Scalar_Test_240_80(pFrameBuffer,24);          
            }            
         }
         else
            printf("Not Support Scalar Test for this panel");
         break;
      case 10:
         //PiP Test
         SetFrameBase(0,0,pFrameBuffer);
         PiP_Test( pFrameBuffer, 16 );
         PiP_Test( pFrameBuffer, 24 );
         break;

      case 11:
         //PoP Test
         SetFrameBase(0,0,pFrameBuffer);
         PoP_Test( pFrameBuffer, 16 );
         PoP_Test( pFrameBuffer, 24 );
         break;
      case 12:
         //Virtual Screen Test-Prime View
         SetFrameBase(0,0,pFrameBuffer);
         Virtual_Screen_YUV_Test(pFrameBuffer,16);       
         Virtual_Screen_Test(pFrameBuffer, 16);
         Virtual_Screen_Test(pFrameBuffer, 24);       
         break;
      case 13:
         SetFrameBase(0,0,pFrameBuffer);
         PoP_Virtual_Screen_Test(pFrameBuffer, 16);
         PoP_Virtual_Screen_Test(pFrameBuffer, 24);
         break;
      case 14:
         SetFrameBase(0,0,pFrameBuffer);
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);

         Scalar_Bypass2(0);         //bypass the 2nd stage scalar
         Scalar_Interpolation( 0, 0 );
         reset_lcd_controller();
         //bypass the 1st and 2nd stage scalar
         Scalar_Resolution(800, 600, 640, 480);
         Scalar_DownRatio(0);
         //for TV scalar start
         //*(unsigned int *) (LCD_IO_Base + 0x0204) = 0x2;
         //*(unsigned int *) (LCD_IO_Base + 0x0014) = 0x100;//0x100->1/2x1/2 x200->1/2x1
         //for TV end
         Scalar_On();
   
         //Test RGB Palette's Red, Green, and Blue
         ColorBar_RGB( 8, LBLP, pFrameBuffer, 800, 600, 0);
         break;
      case 15:
          Scalar_down_TV();
          break;
      case 16:
         SetFrameBase(0,0,pFrameBuffer);
         
         Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);     
         printf("YCbCr422 LBLP Test\n");
         strcpy(FILENAME, DEFAULT_PATH);
         if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
               strcat(FILENAME, "16_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
            //strcat(FILENAME, "flick_640x480.yuv");  
            strcat(FILENAME, "flick_640x480_1.yuv");              
            //strcat(FILENAME, "16_422_640x480.bmp.yuv");            
            Load_Image_YCbCr422( FILENAME, 614400, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
            strcat(FILENAME, "6_422_720x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 691200, pFrameBuffer, LBLP );
         }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
            strcat(FILENAME, "16_422_720x576.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 829440, pFrameBuffer, LBLP );
         }
         delay_10ms(300);
               
//            temp = *(volatile unsigned int *)LCD_IO_Base;
            *(volatile unsigned int *) LCD_IO_Base |= 0x00080000;  // enable de-flicker function 
         
//            temp = *(volatile unsigned int *)(LCD_IO_Base+0x408);
            *(volatile unsigned int *) (LCD_IO_Base+0x408) |= 0x00200000;  // sharpness 
                                 
         //Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);
         ////TV_flicker_ColorBar_RGB( 8, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT);
          //TV_flicker_YCbCr_422( LBBP, pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 8);         
         break;          
      //case 15:
      // patgen_interlace();
      // break;
      }
   } while (1);
   free(pFrameBuffer);
#endif   
   return 0;
}
