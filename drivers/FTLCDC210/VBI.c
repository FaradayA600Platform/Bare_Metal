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
//#include "timer.h"
#include "ftlcd210_conf.h"

/******************************************************************************
 * Gobal variables
 *****************************************************************************/
extern LCDMTYPE_T FLcdModule[];

extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern unsigned int LCD_PANEL_TYPE;
extern char DEFAULT_PATH[];
extern char FILENAME[];

/******************************************************************************
 * Define Constants
 *****************************************************************************/

#define VBI_CNT 10

/******************************************************************************
 * Gobal variables
 *****************************************************************************/
void VBI_Init_LCD(volatile unsigned int LCD_Base, volatile LCDMTYPE_T  *pLCDC, unsigned int lcd_type, unsigned char bpp)
{
   Init_LCD( LCD_Base, pLCDC, lcd_type, bpp);
   *(volatile unsigned int *)(uintptr_t)(LCD_Base + 0x0000 ) |= 0x8000;
}

void TV_VBI_Initialize(unsigned int TV_type) 
{
   if (TV_type==5) {          
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 11 | (12 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 0;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x228 ) = 10 | (10 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x22C ) = 1 | (1 << 12);
   }
   else if (TV_type==6) {              
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 11 | (12 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 0;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x228 ) = 10 | (10 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x22C ) = 1 | (1 << 12);
   }
   else if (TV_type==7) {           
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 11 | (14 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 2 | (24 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x228 ) = 10 | (10 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x22C ) = 1 | (1 << 12);
   }
   else if (TV_type==8) {              
      *(volatile unsigned int *) ( LCD_IO_Base + 0x210 ) = 11 | (14 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x214 ) = 2;
      *(volatile unsigned int *) ( LCD_IO_Base + 0x228 ) = 10 | (10 << 12);
      *(volatile unsigned int *) ( LCD_IO_Base + 0x22C ) = 1 | (1 << 12);
   }
}

void VBI_Field_Int_Test()
{  
   volatile unsigned int tmp;
   unsigned int i;
   unsigned int status[VBI_CNT];
   unsigned int count;

   for (i=0;i<VBI_CNT;i++)
      status[i]=0;
   VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
   *(volatile unsigned int *)( LCD_IO_Base + 0x08 ) |= 0x6;    //enable VBI field and frame interrupt mask
   *(volatile unsigned int *)( LCD_IO_Base + 0x04 ) &= ~0x600; //start of vertical sync
   SetDivNo(0xf);

   count=0;
   while (count<VBI_CNT) {
      tmp = *(volatile unsigned int *)( LCD_IO_Base + 0x10) & 0x6;

     if (tmp == 0x6) {
       *(volatile unsigned int *)( LCD_IO_Base + 0x0C ) |= 0x6;    //clear VBI field and frame interrupt status
         status[count++] = tmp;      
     }
     else if (tmp == 0x04) {
       *(volatile unsigned int *)( LCD_IO_Base + 0x0C ) |= 0x04; //clear frame interrupt status
         status[count++] = tmp;      
     }       
     else if (tmp == 0x10) {
       *(volatile unsigned int *)( LCD_IO_Base + 0x0C ) |= 0x10; //clear VBI field interrupt status
        status[count++] = tmp;     
      }
   }
   
   for (i=0; i<VBI_CNT; i++)
      printf("0x%x ", status[i]);
}

void Set_Frame_VBI(unsigned char *fb,unsigned char *vbi)
{
   *(unsigned int *)(LCD_IO_Base) |= 3 << 15;   //enable VBI and Address sync enable
   //SetFrameBase(0,0, fb);
   SetFrameBase(4,0, vbi);
   *(unsigned int *)(LCD_IO_Base + 0x4) |= 1 << 16;   //address update
   delay_10ms(500);    
}
   
void Set_YCBCR420_Frame_VBI(unsigned char *fb,unsigned char *vbi)
{
   *(unsigned int *)(LCD_IO_Base) |= 3 << 15;   //enable VBI and Address sync enable
   //SetFrameBase(0,0, fb);
   //SetFrameBase(1,0, fb + PANEL_WIDTH * PANEL_HEIGHT);
   //SetFrameBase(2,0, fb + PANEL_WIDTH * PANEL_HEIGHT + PANEL_WIDTH * PANEL_HEIGHT / 4);   
   SetFrameBase(4,0, vbi);
   *(unsigned int *)(LCD_IO_Base + 0x4) |= 1 << 16;   //address update
   delay_10ms(500);    
}
   
void VBI_RGB565(unsigned char *vbi_y[],unsigned char *vbi[],unsigned char *fb0)
{  
   printf("RGB565 VBI Test\n");
   
   VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
   SetFrameBase(0,0, fb0);
      
   printf("Loading RGB565 LBLP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "39_565_320x240.bmp.bin");
     Load_Image( FILENAME, 153600, fb0, LBLP, 16 );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
      strcat(FILENAME, "39_565_640x480.bmp.bin");
     Load_Image( FILENAME, 614400, fb0, LBLP, 16 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
      strcat(FILENAME, "39_565_720x480.bmp.bin");
     Load_Image( FILENAME, 691200, fb0, LBLP, 16 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
      strcat(FILENAME, "39_565_720x576.bmp.bin");
     Load_Image( FILENAME, 829440, fb0, LBLP, 16 );
   }
   Set_Frame_VBI(fb0,vbi[1]);

   //RGB565 Test
   //VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);
   printf("Loading RGB565 LBLP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "12_565_320x240.bmp.bin");
     Load_Image( FILENAME, 153600, fb0, LBLP, 16 );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
      strcat(FILENAME, "12_565_640x480.bmp.bin");
     Load_Image( FILENAME, 614400, fb0, LBLP, 16 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
      strcat(FILENAME, "12_565_720x480.bmp.bin");
     Load_Image( FILENAME, 691200, fb0, LBLP, 16 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
      strcat(FILENAME, "45_565_720x576.bmp.bin");
     Load_Image( FILENAME, 829440, fb0, LBLP, 16 );
   }
   Set_Frame_VBI(fb0,vbi[0]);    
}

void VBI_RGB888(unsigned char *vbi_y[],unsigned char *vbi[],unsigned char *fb0)
{
   printf("RGB888 VBI Test\n");
   
   VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);
   SetFrameBase(0,0, fb0);
            
   printf("Loading RGB888 LBLP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "17_888_320x240.bmp.bin");
     Load_Image( FILENAME, 307200, fb0, LBLP, 24 );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
      strcat(FILENAME, "17_888_640x480.bmp.bin");
     Load_Image( FILENAME, 1228800, fb0, LBLP, 24 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
      strcat(FILENAME, "20_888_720x480.bmp.bin");
     Load_Image( FILENAME, 1382400, fb0, LBLP, 24 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
      strcat(FILENAME, "62_888_720x576.bmp.bin");
     Load_Image( FILENAME, 1658880, fb0, LBLP, 24 );
   } 
   Set_Frame_VBI(fb0,vbi[1]);    
         
   //RGB888 Test
   //VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 24);     
   printf("Loading RGB888 LBLP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "68_888_320x240.bmp.bin");
     Load_Image( FILENAME, 307200, fb0, LBLP, 24 );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
      strcat(FILENAME, "68_888_640x480.bmp.bin");
     Load_Image( FILENAME, 1228800, fb0, LBLP, 24 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
     strcat(FILENAME, "68_888_720x480.bmp.bin");
     Load_Image( FILENAME, 1382400, fb0, LBLP, 24 );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
     strcat(FILENAME, "68_888_720x576.bmp.bin");
     Load_Image( FILENAME, 1658880, fb0, LBLP, 24 );
   }
   Set_Frame_VBI(fb0,vbi[0]);   
}


void VBI_YCbCr422(unsigned char *vbi_y[],unsigned char *vbi[],unsigned char *fb0)
{   
   printf("YCbCr422 VBI Test\n");
   
   VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16); 
   SetFrameBase(0,0, fb0);
         
   printf("Loading YCbCr422 LBLP image\n");      
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
     strcat(FILENAME, "16_422_320x240.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 153600, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
     strcat(FILENAME, "16_422_640x480.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 614400, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
     strcat(FILENAME, "6_422_720x480.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 691200, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
     strcat(FILENAME, "16_422_720x576.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 829440, fb0, LBLP );
   } 
   Set_Frame_VBI(fb0,vbi[1]);                  
   
   //YCbCr422 Test
   //VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 16);     
   printf("Loading YCbCr422 LBLP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
     strcat(FILENAME, "50_422_320x240.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 153600, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
     strcat(FILENAME, "50_422_640x480.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 614400, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
     strcat(FILENAME, "50_422_720x480.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 691200, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
     strcat(FILENAME, "50_422_720x576.bmp.yuv");
     Load_Image_YCbCr422( FILENAME, 829440, fb0, LBLP );
   }
   Set_Frame_VBI(fb0,vbi[0]);     
}


void VBI_YCbCr420(unsigned char *vbi_y[],unsigned char *vbi[],unsigned char *fb0)
{    
   printf("YCbCr420 VBI Test\n");
        
   VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 8);
   SetFrameBase(0,0, fb0);
   SetFrameBase(1,0, fb0 + PANEL_WIDTH * PANEL_HEIGHT);
   SetFrameBase(2,0, fb0 + PANEL_WIDTH * PANEL_HEIGHT + PANEL_WIDTH * PANEL_HEIGHT / 4); 
               
   printf("Loading YCbCr420 LBLP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
     strcat(FILENAME, "16_420_320x240.bmp.yuv");
     Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 115200, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
     strcat(FILENAME, "20_420_640x480.bmp.yuv");
     Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 460800, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
     strcat(FILENAME, "16_420_720x480.bmp.yuv");
     Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 518400, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
     strcat(FILENAME, "41_420_720x576.bmp.yuv");
     Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 622080, fb0, LBLP );
   }
   Set_YCBCR420_Frame_VBI(fb0,vbi[1]);   
   

   //YCbCr420 Test
   //VBI_Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, 8);      
   printf("Loading YCbCr420 LBLP image\n");
   strcpy(FILENAME, DEFAULT_PATH);
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "68_420_320x240.bmp.yuv");
      Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 115200, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){   //640x480
      strcat(FILENAME, "68_420_640x480.bmp.yuv");
      Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 460800, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){   //720x480
      strcat(FILENAME, "68_420_720x480.bmp.yuv");
      Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 518400, fb0, LBLP );
   }
   else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){   //720x576
      strcat(FILENAME, "68_420_720x576.bmp.yuv");
      Load_Image_YCbCr420( FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 622080, fb0, LBLP );
   }
   Set_YCBCR420_Frame_VBI(fb0,vbi[0]); 
}

void VBI_Test(unsigned char *pFrameBuffer)
{
   FILE *vbi_y_fptr[2];
   unsigned char *vbi_y[2], *vbi[2];
   int vbi_y_len[2];
   unsigned char *fb0;
   unsigned int i,j;

   TV_VBI_Initialize(LCD_PANEL_TYPE);  
   
   fb0 =  pFrameBuffer; //allocateFrameBuffer( PANEL_WIDTH, PANEL_HEIGHT, 24, 1 );
   for (j=0;j<2;j++) {
      vbi_y_len[j] = 1440; 
      vbi_y[j] = malloc(1440);
      vbi[j] = malloc(1440 * 2);
      memset(vbi[j], 0, 1440 * 2);   
      printf("vbi[%d]=0x%x\n", j,(unsigned int)(uintptr_t)(vbi[j]));
   }
         
   //fill vbi0,1
   for (j=0;j<2;j++) {
      strcpy(FILENAME, DEFAULT_PATH);
      if (j==0)
         strcat(FILENAME, "vbi_hd1_50_51");
      else
         strcat(FILENAME, "vbi_hd1_52_53");      
      if( (vbi_y_fptr[j] = fopen( FILENAME, "rb") ) == 0 ) {   // the file is in little-endian on Intel x86
         printf("No picture to show\n"); 
         goto error;
      } 

      if (fread(vbi_y[j], 1, vbi_y_len[j], vbi_y_fptr[j]) == 0 )
         printf("File open error \n");
      else
        fclose(vbi_y_fptr[j]);         

      for (i=0; i<vbi_y_len[j]; i++) 
      *(unsigned char * )( vbi[j] + i*2 + 1 ) = *(vbi_y[j]+i); //Sequence is Cb, Y, Cr, Y, Cb, Y, Cr, Y, ...
   }
   
   // set default VBI buffer
   SetFrameBase(4,0, vbi[0]);   
   
   //VBI_Field_Int_Test();

   while (1) {
   //RGB565 Test
   VBI_RGB565(vbi_y,vbi,fb0);

   //RGB888 Test
   VBI_RGB888(vbi_y,vbi,fb0);
   
   //YCbCr420 Test
   VBI_YCbCr420(vbi_y,vbi,fb0);
   
   //YCbCr422 Test
   VBI_YCbCr422(vbi_y,vbi,fb0);
   }
   
error:
   free(vbi_y[0]);
   free(vbi[0]);
   free(vbi_y[1]);
   free(vbi[1]);
}
