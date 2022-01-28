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
#ifndef __FTLCD210_OSD_H
#define __FTLCD210_OSD_H

void palette_write( unsigned int data);
int palette_check( unsigned int data);
void palette_write_seq(void);
int palette_check_seq(void);
void palette_ram_test( unsigned int base);
void OSD_Pos( unsigned int which, int  HPos, int VPos);
void OSD_Dim( unsigned int which, int HDim, int VDim);
void OSD_WinControl( unsigned int which, int hightlight, int bdType, int bdColor, int shType, int shColor );
void OSD_FontControl( unsigned int TranType, unsigned int HighLightType, unsigned int BdColorSel, unsigned int ShColorSel );
void OSD_Row_Col_Space( unsigned int RowSpace, unsigned int ColSpace);
void OSD_FontAttributeBase( unsigned int which, unsigned short fontIndex );
void OSD_MCFBase( unsigned char *fontbase );
void OSD_Scal( int HZoomIn, int HScal, int VZoomIn, int VScal);
void OSD_putc( char c, int position, unsigned short foreground, unsigned short background );
void OSD_puts( char *str, int position, unsigned short foreground, unsigned short background );
void OSD_On( unsigned int which );
void OSD_Off(void);
void OSD_Test_Complex(char *str);
void OSD_On_Simple( unsigned char on);
void OSD_Pos_Simple( int  HPos, int VPos );
void OSD_Dim_Simple( int HDim, int VDim );
void OSD_transparent_Simple( int level );
void OSD_fg_color_Simple( int pal0, int pal1, int pal2, int pal3);
void OSD_bg_color_Simple( int pal1, int pal2, int pal3);
void OSD_Scal_Simple( int HScal, int VScal);
void OSD_putc_Simple( char c, int position);
void OSD_puts_Simple( char *str, int position);
void OSD_Test_Simple( int mode, char *str);
void OSD_Test( char *str);

#endif
