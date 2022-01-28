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
#ifndef __PIP_POP_LIB_H
#define __PIP_POP_LIB_H

#define ARGB8888  29

void PiP_PoP_Image_Format1( unsigned int win, unsigned int in_type, unsigned int endian, unsigned int bpp );
void PiP_PoP_Image_Format2( unsigned int im0, unsigned int im1, unsigned int im2, unsigned int im3 );

void PiP_Priority_Sequence( unsigned int im0, unsigned int im1, unsigned int im2, unsigned int im3);

#endif