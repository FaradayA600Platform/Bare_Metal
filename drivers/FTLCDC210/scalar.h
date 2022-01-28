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
#ifndef __SCALAR_H
#define __SCALAR_H

void Scalar_On( void );
void Scalar_Off( void );
void Scalar_Resolution( unsigned int Hor_in, unsigned int Ver_in, unsigned int Hor_out, unsigned int Ver_out );	//set the resolution of input and output image
//define the upscaling thresholds
void Scalar_UPThreshold( unsigned int Hor_high_th, unsigned int Hor_low_th, unsigned int Ver_high_th, unsigned int Ver_low_th );
//define the upscaling user-defined coefficients ( Hor_high_th, Ver_high_th )
//void Scalar_2Coeff( unsigned int Hor_user_coeff, unsigned int Ver_user_coeff);
//0: nearly bilinear mode, 1: threshold nearly bilinear mode, 2: most neighborhood mode, 3: reserved 
void Scalar_Interpolation( unsigned int Hor_inter_mode, unsigned int Ver_inter_mode );
//if enabled, the center area will be zoomed in
void Scalar_PartialDisplay( int on );
//bypass the 2nd stage automatic scalar (from (1/2) * (1/2) to 2 * 2 )
void Scalar_Bypass2( int on );
//the scaling ratio of 1st stage scalar. 000:bypass 1st stage scalar, 001:(1/2) * (1/2),......, 111:(1/128) *(1/128)
void Scalar_DownRatio( unsigned int ratio );
unsigned char *allocateLargeBuffer( unsigned int );
void Scalar_Test_720_576(unsigned char *pFrameBuffer, unsigned char bpp );
void Scalar_Test_720_480(unsigned char *pFrameBuffer, unsigned char bpp );
void Scalar_Test_640_480(unsigned char *pFrameBuffer, unsigned char bpp );
void Scalar_Test_320_240(unsigned char *pFrameBuffer, unsigned char bpp );
void Scalar_Test_240_80(unsigned char *pFrameBuffer, unsigned char bpp );
void TV_Scalar_Test(unsigned char *pFrameBuffer, unsigned char bpp );
void Scalar_down_TV(void);

#endif