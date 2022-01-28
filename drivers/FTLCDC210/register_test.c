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
#include "ftlcd210_conf.h"

/******************************************************************************
 *Public functions
 *****************************************************************************/

int Register_Test( int address, unsigned int compare )
{
    unsigned int write;

    write = 0x55555555 & compare;
    *(volatile unsigned int *)(uintptr_t)address = write;
    if( * ( volatile unsigned int *)(uintptr_t)address != write ) 
    {
        printf("address = %x, write data = %x, read data = %x\n", address,  write, * ( volatile unsigned int *)(uintptr_t) address);
        return 0;
    }

    write = 0xaaaaaaaa & compare;
    *(volatile unsigned int *)(uintptr_t)address = write;
    if( * ( volatile unsigned int *)(uintptr_t)address != write ) 
    {
        printf("address = %x, write data = %x, read data = %x\n", address,  write, * ( volatile unsigned int *)(uintptr_t)address);
        return 0;
    }

    write = 0xFFFFFFFF & compare;
    *(volatile unsigned int *)(uintptr_t)address = write;
    if( * ( volatile unsigned int *)(uintptr_t)address != write ) 
    {
        printf("address = %x, write data = %x, read data = %x\n", address,  write, * ( volatile unsigned int *)(uintptr_t)address);
        return 0;
    }

    write = 0x00000000 & compare;
    *(volatile unsigned int *)(uintptr_t)address = write;
    if( * ( volatile unsigned int *)(uintptr_t)address != write ) 
    {
        printf("address = %x, write data = %x, read data = %x\n", address,  write, * ( volatile unsigned int *)(uintptr_t)address);
        return 0;
    }

	return 1;
}



/***************** Generic LCD Controller Registers Access Test *************************************************/

int RegisterFile_Test( unsigned int base)	//do not test interrupt & GPIO related register
{
    int success = 1;

    /*LCD global parameter*/
    if( Register_Test( base, 0x00003FFF) == 0 ) success =0;	//LCD function enable

    if( Register_Test( base + 0x4 , 0x0001FFFF) == 0 )  success =0;	//LCD panel pixel parameters
    if( Register_Test( base + 0x14, 0x0000FFFF) == 0 )  success =0;	//Frame buffer parameter

    if( Register_Test( base + 0x18, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image0 frame0 base address
    if( Register_Test( base + 0x1C, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image0 frame1 base address
    if( Register_Test( base + 0x20, 0xFFFFFFFC) == 0 )  success =0;  //LCD panel image0 frame2 base address

    if( Register_Test( base + 0x24, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image1 frame0 base address  
    if( Register_Test( base + 0x28, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image1 frame1 base address
    if( Register_Test( base + 0x2c, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image1 frame2 base address

    if( Register_Test( base + 0x30, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image2 frame0 base address
    if( Register_Test( base + 0x34, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image2 frame1 base address
    if( Register_Test( base + 0x38, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image2 frame2 base address

    if( Register_Test( base + 0x3C, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image3 frame0 base address
    if( Register_Test( base + 0x40, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image3 frame1 base address
    if( Register_Test( base + 0x44, 0xFFFFFFFC) == 0 )  success =0;	//LCD panel image3 frame2 base address

    if( Register_Test( base + 0x48, 0x00FFFFFF) == 0 )  success =0;	//PatGen pattern bar distance
    if( Register_Test( base + 0x4C, 0xFFFFFFFF) == 0 )  success =0;	//FIFO threshold control

    /*LCD timing and polarity parameter*/
    if( Register_Test( base + 0x100, 0xFFFFFFFF) == 0 )  success =0;	//LCD horizontal timing control
    if( Register_Test( base + 0x104, 0xFF3F0FFF) == 0 )  success =0;	//LCD vertical timing control
    if( Register_Test( base + 0x108, 0x000000FF) == 0 )  success =0;	//LCD vertical timing control
    if( Register_Test( base + 0x10C, 0x00003F0F) == 0 )  success =0;	//LCD polarity control
 
/*LCD output format parameters, should not test here*/
/*	if( Register_Test( base + 0x200, 0xFFFFFF3F) == 0 )  success =0;	//LCD serial panel pixel parameters
	if( Register_Test( base + 0x204, 0x00000007) == 0 )  success =0;	//LCD CCIR656 parameters
*/
    /*LCD image parameters*/
    if( Register_Test( base + 0x300, 0x0000001F) == 0 )  success =0;	//LCD PiP parameter (alpha blending) value: 0 or 1
    if( Register_Test( base + 0x304, 0x07FF07FF) == 0 )  success =0;	//PiP sub-picture1 position
    if( Register_Test( base + 0x308, 0x07FF07FF) == 0 )  success =0;	//PiP sub-picture1 dimension

    if( Register_Test( base + 0x30C, 0x07FF07FF) == 0 )  success =0;	//PiP sub-picture2 position
    if( Register_Test( base + 0x310, 0x07FF07FF) == 0 )  success =0;	//PiP sub-picture2 dimension

    /*LCD image color management*/
    if( Register_Test( base + 0x400, 0x00003FFF) == 0 )  success =0;	//LCD color management parameter0
    if( Register_Test( base + 0x404, 0x00007F7F) == 0 )  success =0;	//LCD color management parameter1
    if( Register_Test( base + 0x408, 0x00FFFFFF) == 0 )  success =0;	//LCD color management parameter2

    /*CSTN panel parameters*/
    if( Register_Test( base + 0xC00, 0x000003FF) == 0 )  success =0;	//CSTN control register
    if( Register_Test( base + 0xC04, 0x01FFFF3F) == 0 )  success =0;	//CSTN R-channel data control register
    if( Register_Test( base + 0xC08, 0x01FFFF3F) == 0 )  success =0;	//CSTN G-channel data control register
    if( Register_Test( base + 0xC0C, 0x01FFFF3F) == 0 )  success =0;	//CSTN B-channel data control register

    /*Scalar control registers*/
    if( Register_Test( base + 0x1100, 0x00000FFF) == 0 )  success =0;	//Horizontal resolution register of scalar input
    if( Register_Test( base + 0x1104, 0x00000FFF) == 0 )  success =0;	//Vertical resolution register of scalar input

    if( Register_Test( base + 0x1108, 0x00003FFF) == 0 )  success =0;	//Horizontal resolution register of scalar output
    if( Register_Test( base + 0x110C, 0x00003FFF) == 0 )  success =0;	//Vertical resolution register of scalar output

    /*Miscellaneous control registers*/
    if( Register_Test( base + 0x1110, 0x000001FF) == 0 )  success =0;
    if( Register_Test( base + 0x1114, 0x000001FF) == 0 )  success =0;	//Horizontal high threshold register
    if( Register_Test( base + 0x1118, 0x000001FF) == 0 )  success =0;	//Horizontal low threshold register

    if( Register_Test( base + 0x111C, 0x000001FF) == 0 )  success =0;	//Vertical high threshold register
    if( Register_Test( base + 0x1120, 0x000001FF) == 0 )  success =0;	//Vertical low threshold register

/*	if( Register_Test( base + 0x1124, 0x000001FF) == 0 )  success =0;	//User define upscaling horizontal coefficient
	if( Register_Test( base + 0x1128, 0x000001FF) == 0 )  success =0;	//User define upscaling vertical coefficient
*/
    /*OSD control registers*/
#ifdef COMPLEX_OSD
    if( Register_Test( base + 0x2000, 0x0000000F) == 0 )  success =0;	//OSD windows On/Off switch register
    if( Register_Test( base + 0x2004, 0x00FF7FFF) == 0 )  success =0;	//OSD font control register
    if( Register_Test( base + 0x2008, 0x00001F1F) == 0 )  success =0;	//OSD font control register1

    if( Register_Test( base + 0x200C, 0x0000FFFF) == 0 )  success =0;	//OSD window1 control register0
    if( Register_Test( base + 0x2010, 0x3FFF3F3F) == 0 )  success =0;	//OSD window1 control register1
    if( Register_Test( base + 0x2014, 0x000001FF) == 0 )  success =0;	//OSD window1 control register2

    if( Register_Test( base + 0x2018, 0x0000FFFF) == 0 )  success =0;	//OSD window2 control register0
    if( Register_Test( base + 0x201C, 0x3FFF3F3F) == 0 )  success =0;	//OSD window2 control register1
    if( Register_Test( base + 0x2020, 0x000001FF) == 0 )  success =0;	//OSD window2 control register2

    if( Register_Test( base + 0x2024, 0x0000FFFF) == 0 )  success =0;	//OSD window3 control register0
    if( Register_Test( base + 0x2028, 0x3FFF3F3F) == 0 )  success =0;	//OSD window3 control register1
    if( Register_Test( base + 0x202C, 0x000001FF) == 0 )  success =0;	//OSD window3 control register2

    if( Register_Test( base + 0x2030, 0x0000FFFF) == 0 )  success =0;	//OSD window4 control register0  
    if( Register_Test( base + 0x2034, 0x3FFF3F3F) == 0 )  success =0;	//OSD window4 control register1  
    if( Register_Test( base + 0x2038, 0x000001FF) == 0 )  success =0;	//OSD window4 control register2

    if( Register_Test( base + 0x203C, 0x00003FFF) == 0 )  success =0;	//OSD multi-color font base address
#endif

#ifdef SIMPLE_OSD
    if( Register_Test( base + 0x2000, 0x0003F1FF) == 0 )  success =0;	//OSD scaling and dimension control register
    if( Register_Test( base + 0x2004, 0x00FFF7FF) == 0 )  success =0;	//OSD position control register
    if( Register_Test( base + 0x2008, 0xFFFFFFFF) == 0 )  success =0;	//OSD foreground color control register
    if( Register_Test( base + 0x200C, 0xFFFFFF03) == 0 )  success =0;	//OSD background color control register
#endif

    if( success) printf("Register Testing PASS\n");
    return 0;
}

