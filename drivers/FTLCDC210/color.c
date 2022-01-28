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
#include "ftlcd210.h"
#include "ftlcd210_conf.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/

extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern LCDMTYPE_T FLcdModule[];
extern unsigned int LCD_PANEL_TYPE;
extern char DEFAULT_PATH[];
extern char FILENAME[];

/******************************************************************************
 *Public functions
 *****************************************************************************/

void Load_Default_Color_Param()
{
   //LCD Image Color Management
    *(volatile unsigned int *) (LCD_IO_Base + 0x400 ) = 0x2000;
    *(volatile unsigned int *) (LCD_IO_Base + 0x404 ) = 0x2000;
    *(volatile unsigned int *) (LCD_IO_Base + 0x408 ) = 0x0;
    *(volatile unsigned int *) (LCD_IO_Base + 0x40C ) = 0x40000;
}


/************************** Color Management Test *****************************/

/*Brightness: -127 ~ 127 (2's complement), SatValue: bit 13 ~ 8*/
void Color_SatBright( unsigned int SatValue, char BrightSign, int Brightness )
{
   if( SatValue > 63 )
      printf("Saturation error\n");
   if( Brightness < -127 || Brightness > 127 )
      printf("Brightness error\n");

   *(volatile unsigned int *) (LCD_IO_Base + 0x400) &= 0xffffc000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x400) |= (SatValue << 8) | (BrightSign << 7) | Brightness;
}

/*SigHuXXX: 0->positive, 1->negative, HuXXXValue: uses 6 bit, and must be the same rotation degree*/
void Color_Hue( unsigned int SigHuCos, int HuCosValue, unsigned int SigHuSin, int HuSinValue )
{
   if( SigHuCos > 1 || SigHuSin > 1 )
      printf("Hue error\n");

   //*(volatile unsigned int *) (LCD_IO_Base + 0x404) &= 0xffff8080;
   *(volatile unsigned int *) (LCD_IO_Base + 0x404) = (SigHuCos << 14) | (HuCosValue << 8) | (SigHuSin << 6) | HuSinValue;
}


/*each parameter is 8 bit*/
void Color_Sharp( unsigned int K1, unsigned int K0, unsigned int ShTh1, unsigned int ShTh0 )
{
   if( K1 > 15 || K0 > 15 )   printf("Sharp parameter K set error\n");
   if( ShTh1 > 255 || ShTh0 > 255 ) printf("Sharp threshold value set error\n");

   *(volatile unsigned int *) (LCD_IO_Base + 0x408) &= 0xff000000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x408) |= (K1 << 20) | (K0 << 16) | (ShTh1 << 8) | ShTh0;
}


/*fill the contrast lookup table with each item value (curve), item ranges from 0~63, value is 32 bits*/
void Color_Contrast( unsigned int slope )
{
   int sign, offset;
   
   if( slope == 0 || slope >= 32)   { printf("Error contrast slope\n"); exit(1); }
   
   if( slope * 128 > 512 ) {
      sign = 1;
      offset = slope * 128 - 512;
   }
   else {
      sign = 0;
      offset = 512 - slope * 128;
   }
      
   *(volatile unsigned int *) (LCD_IO_Base + 0x40C) = (slope << 16) | (sign << 12) | offset;
}


/*fill the Red, Green, and Blue Gamma Correction lookup table*/
void Color_FillGammaTable( unsigned int item, int r, int g, int b )
{
   if( item < 64 ) {
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x600 + item*(0x04)) = r;
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x700 + item*(0x04)) = g;
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x800 + item*(0x04)) = b;
   }
   else
      printf("Gamma lookup table item error\n");
}


void Contrast_Test()
{
   int i;
   //char c;

   Load_Default_Color_Param();
      //test the contrast
   printf("Contrast testing\n");    
      for( i=1; i<32; i+=2 )
   {
      if( i > 5 ) i+=10;

      if( i >= 32 )  break;

      Color_Contrast( i );

        printf("%d....\n",i);
       delay_10ms(500);
        //printf("Enter one key\n");
      //scanf("%c", &c);

      //clear the screen
        //for( j=0; j<PANEL_WIDTH*PANEL_HEIGHT*2; j+=4)
        //   *(volatile unsigned int *) (pFrameBuffer + j) = 0xffffffff;
   }
}

void Saturation_Test()
{
   int i;
   //char c;

   Load_Default_Color_Param();
   //test the saturation
   printf("Saturation testing\n");  
      for( i=0; i<256; i+=64 )
   {
      Color_SatBright( i/4, 0, 0/*-127 + i*/ );

       printf("%d....\n",i);
       delay_10ms(500);
        //printf("Enter one key\n");
      //scanf("%c", &c);

      //clear the screen
        //for( j=0; j<PANEL_WIDTH*PANEL_HEIGHT*2; j+=4)
        //  *(volatile unsigned int *) (pFrameBuffer + j) = 0xffffffff;
   }
}


void Brightness_Test()
{
   int i;
   //char c;

   Load_Default_Color_Param();
   //test the brightness
   printf("Brightness testing\n");  
      for( i=0; i<256; i+=64 )
   {     
      if( i < 128 )
         Color_SatBright( 0x20/*i/4*/, 1, 127 - i );
      else
         Color_SatBright( 0x20, 0, i - 128 );

       printf("%d....\n",i);
       delay_10ms(500);
        //printf("Enter one key\n");
      //scanf("%c", &c);

      //clear the screen
        //for( j=0; j<PANEL_WIDTH*PANEL_HEIGHT*2; j+=4)
        //  *(volatile unsigned int *) (pFrameBuffer + j) = 0xffffffff;
   }
}


void Sharpness_Test()
{
   int i;
   //char c;

   Load_Default_Color_Param();
   //test the sharpness, NTSC/PAL is not support
   printf("Sharpness testing\n");   
// if( ( LCD_PANEL_TYPE < 5 ) || ( LCD_PANEL_TYPE > 8 ) )
   {
      for( i=0; i<24; i+=4 )
      {
         Color_Sharp( i/8 + 2, i/8, i/8 + 2, i/8 );

           printf("%d....\n",i);
            delay_10ms(500);
            //printf("Enter one key\n");
          //scanf("%c", &c);

         //clear the screen
            //for( j=0; j<PANEL_WIDTH*PANEL_HEIGHT*2; j+=4)
            //   *(volatile unsigned int *) (pFrameBuffer + j) = 0xffffffff;
      }
   }
}


void Hue_Test()
{
   int i;
   //char c;

   Load_Default_Color_Param();
   //test the hue
   printf("Hue testing\n");
   for( i=0; i<=360; i+=20 )
   {
      unsigned int SigHuSin, SigHuCos;
      float HuSin;
      float HuCos;
      float ar;
      float inc = 3.141592653589/180;
      
        ar = inc *i;
        HuSin = sin(ar);
        HuCos = cos(ar);
                
      if( HuSin < 0) SigHuSin = 1; else SigHuSin = 0;
      if( HuCos < 0) SigHuCos = 1; else SigHuCos = 0; 

       printf("...%d....%f....%f\n",i,HuSin, HuCos);
        HuSin = HuSin * 32.0;
        HuCos = HuCos * 32.0;                
                        
      Color_Hue( SigHuCos, abs(HuCos), SigHuSin, abs(HuSin) );

       delay_10ms(500);
        //printf("Enter one key\n");
      //scanf("%c", &c);

      //clear the screen
        //for( j=0; j<PANEL_WIDTH*PANEL_HEIGHT*2; j+=4)
        //  *(volatile unsigned int *) (pFrameBuffer + j) = 0xffffffff;
   }
}


/*Loading static image with color management test*/
/************************** with color management test****************************/

void Color_Test( unsigned char *pFrameBuffer )
{
   int i;

   //fill the straight line x-y=0 to the contrast and gamma lookup table
   for( i=0; i<64; i++ )
      {
      int val;
     
      val = 0x03020100 + 0x04040404 * i;
      Color_FillGammaTable( i, val, val, val ); 
   }
/*   
      //initialize frame buffer first     
   pFrameBuffer = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 1); 
*/ setNumFrameBuffer(1, 0, 0, 0);
    SetFrameBase( 0, 0, pFrameBuffer );

   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
      SetBGRSW( BGR );


    // Set to pure red to verify Hue
   SetYCbCr(0);
   SetBPP(24); //bpp must be 8 in YCbCr420 mode  
   SetEndian( LBLP );   
   clear_Screen(pFrameBuffer, PANEL_WIDTH, PANEL_HEIGHT, 24, 0x00FF0000);  
   Contrast_Test();
   Saturation_Test();
      Brightness_Test();
   //test the sharpness, NTSC/PAL is not support
   Sharpness_Test();
   Hue_Test();
   
   printf("Loading RGB565 LBBP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( ( PANEL_WIDTH == 320 ) && ( PANEL_HEIGHT == 240 ) ){
      strcat(FILENAME, "ship565_320x240.bin");
      Load_Image( FILENAME, 153600, pFrameBuffer, LBBP, 16 );
      SetEndian(BBBP);
   }else if( ( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 ) ){
      strcat(FILENAME, "castle64565.bin");
      Load_Image( FILENAME, 614400, pFrameBuffer, LBBP, 16 );
      SetEndian(BBBP);
   }else if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 480 ) ){
      strcat(FILENAME, "41_565_720x480.bmp.bin");
      Load_Image( FILENAME, 691200, pFrameBuffer, LBBP, 16 );
   }else if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 576 ) ){
      strcat(FILENAME, "45_565_720x576.bmp.bin");
      Load_Image( FILENAME, 829440, pFrameBuffer, LBBP, 16 );
   }


   Contrast_Test();
   Saturation_Test();
      Brightness_Test();
   //test the sharpness, NTSC/PAL is not support
   Sharpness_Test();
   Hue_Test();


   //RGB888 Test
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);     
   printf("Loading RGB888 LBLP image\n");
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

   Contrast_Test();
   Saturation_Test();
      Brightness_Test();
   //test the sharpness, NTSC/PAL is not support
   Sharpness_Test();
   Hue_Test();


   //YCbCr420 Test
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 8);      
   printf("Loading YCbCr420 LBLP image\n");
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

   Contrast_Test();
   Saturation_Test();
      Brightness_Test();
   //test the sharpness, NTSC/PAL is not support
   Sharpness_Test();
   Hue_Test();


   //YCbCr422 Test
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);     
   printf("Loading YCbCr422 LBLP image\n");
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

   Contrast_Test();
   Saturation_Test();
      Brightness_Test();
   //test the sharpness, NTSC/PAL is not support
   Sharpness_Test();
   Hue_Test();


   //remember to open this testing if you need
/* Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
   strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "mov3224.bin");
      animation_test( pFrameBuffer, FILENAME,  6144000 );
*/
// animation_test(pFrameBuffer, "D:\\FTLCDC100\\Script\\animation565.bin", 6144000, /*LCD_IRQ*/2 );

// free(pFrameBuffer);
}

