/*
 * Faraday FTLCDC210 LCD controller non os test OSD functions header file
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
#ifndef __FTLCD210_CONF_H
#define __FTLCD210_CONF_H

/*********************** configuration *****************/
//FTLCDC200 uses CPE120 architecture, ie. shared AHB bus architecture
//#define LCD_PANEL_TYPE  0x1
#define LCD_IO_Base       0x2a900000   //FLCDC210
//#define LCD_IO_Base     0x90e00000   //FLCDC200
#define INTC_BASE       0x98800000  //fLib\include\chipset.h
//#define LCD_IRQ       20       //A320D (cpe120 architecture) in interrupt.h

#if 1
#define SIMPLE_OSD                  //two versions of OSD for ftlcdc200, SIMPLE_OSD and COMPLEX_OSD
#else
#define COMPLEX_OSD
#endif

#define lcdc_read(reg)     (*(volatile unsigned int *)(LCD_IO_Base + reg))
#define lcdc_write(val, reg)  (*(volatile unsigned int *)(LCD_IO_Base + reg) = val)

//#define VBI

//#define QDS_RELEASE

//#define OLD_CCIR656

//if you define QDS_RELEASE, disable this test
#define ENABLE_YCBCR420_IN_PIP
//not to un-define this because we do not know when all data are sent
#define LCM_CONTROLLER_SEND_ONCE

#if 0
#define ARGB5888_USED
#else
#define ARGB8888_USED
#endif

//8185 (new CCIR656) release needs to be word-alignment, QDS (old CCIR656) release needs not to be word-alignment, 7021 (old CCIR656) release needs not to be word-alignment
#define ALIGN32(i)   (i+31 - (i+31)%32)
//#define ALIGN32(i) i

/******************* end configuration *****************/

#endif
