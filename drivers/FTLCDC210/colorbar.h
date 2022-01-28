/*
 * Faraday FTLCDC210 LCD controller non os test header file
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
#ifndef __COLORBAR_H
#define __COLORBAR_H

void ColorBar_YCbCr_422_PiP( unsigned char endian, unsigned char *pFrameBuffer, int width, int height, unsigned int inc);

void TV_flicker_ColorBar_RGB( unsigned char bpp,unsigned char *pFrameBuffer, int width, int height);

void TV_flicker_YCbCr_422( unsigned char endian, unsigned char *pFrameBuffer, int width, int height, unsigned int inc);

#endif