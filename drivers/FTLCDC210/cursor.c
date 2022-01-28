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
#include "SoFlexible.h"
#include "ftlcd210.h"
//#include "fLib.h"
//#include "fLib.h"
//#include "flcd210.h"
#include "ftlcd210_conf.h"
#include "ftlcd210-OSD.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/

extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;

/************************* Hardware Cursor Test ***************************/

void Cursor_On()
{
    *(volatile unsigned int *) (LCD_IO_Base) |= 0x00001000;
}

void Cursor_Off()
{
    *(volatile unsigned int *) (LCD_IO_Base) &= 0xffffefff;
}

void Cursor_Position(unsigned int hpos, unsigned int vpos)
{
   if(hpos >= 2048)  hpos=0;
   if(vpos >= 2048)  vpos=0;

   *(volatile unsigned int *) (LCD_IO_Base + 0x1200) &= 0xf000f000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1200) |= vpos | (hpos << 16);
   *(volatile unsigned int *) (LCD_IO_Base + 0x1200) |= 1 << 28;     //update Cursor parameters
   //spade
   //*(volatile unsigned int *) (LCD_IO_Base + 0x1200) |= 1 << 29;      /* 32 * 32 cursor */
}

void Cursor_Palette(unsigned int color1, unsigned int color2, unsigned int color3)
{
   unsigned char r, g, b;
   unsigned int color;

   *(volatile unsigned int *) (LCD_IO_Base + 0x1204) = color1;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1208) = color2;
   *(volatile unsigned int *) (LCD_IO_Base + 0x120C) = color3;

   r = 0xff;
   g = 0xff;
   b = 0x00;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1210) = color;

   r = 0xff;
   g = 0x00;
   b = 0xff;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1214) = color;

   r = 0x00;
   g = 0xff;
   b = 0xff;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1218) = color;

   r = 0xff;
   g = 0xff;
   b = 0xff;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x121C) = color;

   r = 0x00;
   g = 0x00;
   b = 0x00;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1220) = color;

   r = 0x88;
   g = 0x88;
   b = 0x88;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1224) = color;

   r = 0x88;
   g = 0x00;
   b = 0x88;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1228) = color;

   r = 0x00;
   g = 0x88;
   b = 0x88;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x122C) = color;

   r = 0x88;
   g = 0x88;
   b = 0x00;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1230) = color;

   r = 0x11;
   g = 0x22;
   b = 0x33;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1234) = color;

   r = 0x66;
   g = 0x55;
   b = 0x77;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x1238) = color;

   r = 0x11;
   g = 0x99;
   b = 0xaa;
   color = rgb2ycbcr( &r, &g, &b );
   *(volatile unsigned int *) (LCD_IO_Base + 0x123C) = color;
}

void Cursor_Test()
{
#ifndef QDS_RELEASE
   int i=0;
   unsigned char r, g, b;
   unsigned int color[3];

   printf("Cursor Test\n");

   //for(i=0; i<16; i++)
   //*(volatile unsigned int *) (LCD_IO_Base + 0x1300 + i*4) = Cursor_Font[i];
   
   for (i = 0;i < 0x800;i+=4) 
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x1600 + i) = (i & 0xff) | (((i+1) & 0xff)<<8) | (((i+2) & 0xff)<<16) | (((i+3) & 0xff)<<24);
/*
   for (i = 0;i < 0x200;i+=4) 
      *(volatile unsigned int *) (LCD_IO_Base + 0x1600 + i) = 0x11111111;

   for (;i < 0x400;i+=4) 
      *(volatile unsigned int *) (LCD_IO_Base + 0x1600 + i) = 0x22222222;

   for (;i < 0x600;i+=4) 
      *(volatile unsigned int *) (LCD_IO_Base + 0x1600 + i) = 0x33333333;

   for (;i < 0x800;i+=4) 
      *(volatile unsigned int *) (LCD_IO_Base + 0x1600 + i) = 0x44444444;
*/

   //for (i = 0;i < 0x800;i+=4) 
   //   *(volatile unsigned int *) (LCD_IO_Base + 0x1600 + i) = 0x0;
 
   for (i=0;i< 0x800; i+=32) {
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x1600 + i) = 0x11111111;
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x1600 + i + 28) = 0x33333333;            
   }    
   
    r=0xff, g=0, b=0;
    color[0] = rgb2ycbcr( &r, &g, &b );

    r=0, g=0xff, b=0;
    color[1] = rgb2ycbcr( &r, &g, &b );

    r=0, g=0, b=0xff;
    color[2] = rgb2ycbcr( &r, &g, &b );

    Cursor_Palette(color[1], color[2], color[0]);
    Cursor_On();

#if 0
   for (i = 0;i < PANEL_HEIGHT;i++) {
      Cursor_Position(i, i);
      delay_10ms(1);
   }

   Cursor_Palette(color[1], color[0], color[2]);
   for (i = 0;i < PANEL_HEIGHT;i++) {
      Cursor_Position(PANEL_HEIGHT-i, PANEL_HEIGHT-i);
      delay_10ms(1);
   }

   Cursor_Palette(color[2], color[0], color[1]);
   for (i = 0;i < PANEL_HEIGHT;i++) {
      Cursor_Position(PANEL_HEIGHT-i, i);
      delay_10ms(1);
   }

   Cursor_Palette(color[0], color[1], color[2]);
   for (i = 0;i < PANEL_HEIGHT;i++) {
      Cursor_Position(i, PANEL_HEIGHT-i);
      delay_10ms(1);
   }
#endif

   for (i = 0;i < PANEL_WIDTH - 64;i++) {
      Cursor_Position(i, 0);
      delay_10ms(1);
   }
   for (i = 0;i < PANEL_HEIGHT - 64;i++) {
      Cursor_Position(PANEL_WIDTH - 64, i);
      delay_10ms(1);
   }
   for (i = 0;i < PANEL_WIDTH - 64;i++) {
      Cursor_Position(PANEL_WIDTH - 64 - i, PANEL_HEIGHT - 64);
      delay_10ms(1);
   }
   for (i = 0;i < PANEL_HEIGHT - 64;i++) {
      Cursor_Position(0, PANEL_HEIGHT - 64 - i);
      delay_10ms(1);
   }

   Cursor_Off();
#endif
}



