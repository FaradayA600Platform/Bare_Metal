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
//#include "fLib.h"
//#include "timer.h"
//#include "fa52x_cache.h"
#include "ftlcd210_conf.h"
//#include "flcd210-OSD.h"
//#include "PiP_PoP_lib.h"
//#include "virtual_screen.h"
//#include "scalar.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/


extern LCDMTYPE_T FLcdModule[];

extern unsigned int LCD_IRQ;  //iqr = 20 in A320D (cpe120 architecture) in interrupt.h
extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern unsigned int LCD_PANEL_TYPE;

/******************************************************************************
 *Public functions
 *****************************************************************************/

/**************************** Motion Picture Test *****************************/

int animation_frame;
void animation()
{  
   *(volatile unsigned char *) (LCD_IO_Base + 0x000C)  = 0x02;
   animation_frame++; 
   //MaskIRQ(LCD_IRQ);
   //DisableIRQ();        
}


void animation_test( unsigned char *pFrameBuffer, char *file, int size )
{
// unsigned char *pFrameBuffer;
   int  current_frame;
   //char *InputFile[10];
   FILE *fptr;

   setNumFrameBuffer(1, 0, 0, 0);
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE , 16);  // 16 bpp  
   //InputFile[0]="D:\\FTLCDc100\\Script\\animation565.bin";
  
   fptr = fopen( file, "rb");
   
   if(( fread(pFrameBuffer, size , 1 , fptr )) == 0)
      printf("File open error!!\n");
 
    
   //fLib_ConnectInt(LCD_IRQ,  animation );  //it only can be
   //EnableIRQ();                       //It enables ARM's IRQ in the Boot.s 
   
   *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000007;   //IntNxtBaseEn : next frame base addr. updted interrupt table
   //UnmaskIRQ(LCD_IRQ);
   //SetIRQmode(LCD_IRQ,LEVEL);     
      
   do{
      if ( current_frame != animation_frame )
      {        
         current_frame = animation_frame;
         //UnmaskIRQ(LCD_IRQ);
         //SetIRQmode(LCD_IRQ,LEVEL);
         SetFrameBase( 0, 0, pFrameBuffer + PANEL_WIDTH * PANEL_HEIGHT *2 * (animation_frame % 10) ); // animation_frame % 10, the size of frame buffer can contains 10 frames
      }    
   }while(!feof(fptr));
   
   fclose(fptr);    
// free( pFrameBuffer);
   //fLib_CloseInt(LCD_IRQ);
   SetFrameBase( 0, 0, pFrameBuffer) ;
}  
