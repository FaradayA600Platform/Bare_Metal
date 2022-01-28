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
#include "ftlcd210_conf.h"
#include "PiP_PoP_lib.h"

/************************PiP & PoP Test ***********************************/
void PiP_PoP_Image_Format1( unsigned int win, unsigned int in_type, unsigned int endian, unsigned int bpp )
{
    unsigned int ycbcr422_enable=0, type=0, bpps=0;

    if( win > 3 )
        printf("Window set up error!\n");

    if(in_type == 565)
        type=0;
    else if(in_type == 555)
        type=1;
    else if(in_type == 444)
        type=2;
    else if(in_type == 422)
        ycbcr422_enable=1;
    else{
        type=0;
        ycbcr422_enable=0;
    }

    switch(bpp)
    {
        case 1: bpps=0; break;
        case 2: bpps=1; break;
        case 4: bpps=2; break;
        case 8: bpps=3; break;
        case 16: bpps=4; break;
        case 24: bpps=5; break;
        case ARGB8888: bpps=6; break; //ARGB8888
    }

    *(volatile unsigned int *) (LCD_IO_Base + 0x0318) &= ~( (7 << 4*win) | (1 << 4*win + 3) | (3 << (16 + 2*win)) | (3 << (24 + 2*win)) );
    *(volatile unsigned int *) (LCD_IO_Base + 0x0318) |= (bpps << 4*win) | (ycbcr422_enable << 4*win + 3) | (endian << (16 + 2*win)) | (type << (24 + 2*win));
    reset_lcd_controller();
}

//0: image is progress, 1: image is interlaced, can only be used in CCIR656
void PiP_PoP_Image_Format2( unsigned int im0, unsigned int im1, unsigned int im2, unsigned int im3 )
{
    if( im0 > 2 || im1 > 2 || im2 > 2 || im3 > 2 )
      printf("PiP or PoP image format2 error\n");

   *(volatile unsigned int *) (LCD_IO_Base + 0x031C) &= 0xfffffff0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x031C) |= im0 | (im1 << 1) | (im2 << 2) | (im3 << 3);
}

//00: low, 01: middle, 10: high priority
void PiP_Priority_Sequence( unsigned int im0, unsigned int im1, unsigned int im2, unsigned int im3)
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x314) &= 0xffffff00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x314) |= im0 | (im1 << 2) | (im2 << 4) | (im3 << 6);
}

