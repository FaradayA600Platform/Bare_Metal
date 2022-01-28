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
#include "colorbar.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/


extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern LCDMTYPE_T FLcdModule[];
extern unsigned int LCD_PANEL_TYPE;
extern char DEFAULT_PATH[];
extern char FILENAME[];

/******************************* PiP Test ********************************/
/*pipNum: 0 -> PiP_Off(), 1 -> single PiP window, 2 -> double PiP window*/
void PiP_On( unsigned int pipNum )
{
   if (pipNum > 3)
      printf("PiP window enable error\n");

   *(volatile unsigned int *) LCD_IO_Base &= 0xfffffffe;
#ifndef QDS_RELEASE
   *(volatile unsigned int *) LCD_IO_Base &= 0xfffff3ff; //PiP off, bit10 & bit11
   *(volatile unsigned int *) LCD_IO_Base |= (pipNum << 10);   //PiP on
#else
   *(volatile unsigned int *) LCD_IO_Base &= 0xfffff9ff; //PiP off, bit9 & bit10
   *(volatile unsigned int *) LCD_IO_Base |= (pipNum << 9); //PiP on
#endif
   *(volatile unsigned int *) LCD_IO_Base |= 0x1;
}


void PiP_Off()
{
   *(unsigned int *) LCD_IO_Base &= 0xfffffffe;
#ifndef QDS_RELEASE
   *(unsigned int *) LCD_IO_Base &= 0xfffff3ff;
#else
   *(unsigned int *) LCD_IO_Base &= 0xfffff9ff; //PiP off, bit9 & bit10
#endif
   *(unsigned int *) LCD_IO_Base |= 0x1;
}

void PiP_Color_Key( unsigned int enable, unsigned int key)
{
   *(unsigned int *) (LCD_IO_Base + 0x320) &= 0xff000000;
   *(unsigned int *) (LCD_IO_Base + 0x320) |= key | (enable << 24);
}

/*make sure that blend1 + blend2 < 16*/
void PiP_Blending( unsigned int on, int blend1, int blend2 )
{
   if (on) {
#ifndef QDS_RELEASE
      *(volatile unsigned int *) LCD_IO_Base &= 0xfffffcff;
#else
      *(volatile unsigned int *) LCD_IO_Base &= 0xfffffeff;
#endif
      *(volatile unsigned int *) LCD_IO_Base |= (on << 8);

#ifdef ARGB5888_USED
      *(volatile unsigned int *) (LCD_IO_Base + 0x300) &= 0xff00e0e0;
#elif defined(ARGB8888_USED)
      *(volatile unsigned int *) (LCD_IO_Base + 0x300) &= 0xff000000;
#endif
      *(volatile unsigned int *) (LCD_IO_Base + 0x300) |= blend1 | (blend2 << 8) | (0x8 << 16);

      printf("PiP blending on\n");
   } else {
#ifndef QDS_RELEASE
      *(volatile unsigned int *) LCD_IO_Base &= 0xfffffcff;
#else
      *(volatile unsigned int *) LCD_IO_Base &= 0xfffffeff;
#endif
      printf("PiP blending off\n");
   }
}

//HPos needs to be odd if YCbCr422 is used
void PiP_Pos( unsigned int which, int  HPos, int VPos)
{
   if (which > 3)
      printf("PiP position error\n");

   switch (which)
   {
      case 1:
         *(volatile unsigned int *) (LCD_IO_Base + 0x304) &= 0xf800f800;
         *(volatile unsigned int *) (LCD_IO_Base + 0x304) |= VPos | (HPos << 16);
         break;
      case 2:
         *(volatile unsigned int *) (LCD_IO_Base + 0x30C) &= 0xf800f800;
         *(volatile unsigned int *) (LCD_IO_Base + 0x30C) |= VPos | (HPos << 16);
         break;
      case 3:
         *(volatile unsigned int *) (LCD_IO_Base + 0x32C) &= 0xf800f800;
         *(volatile unsigned int *) (LCD_IO_Base + 0x32C) |= VPos | (HPos << 16);
         break;
   }
   *(volatile unsigned int *) (LCD_IO_Base + 0x304 ) |= 1 << 28;     //update PiP parameters
}   


void PiP_Dim( unsigned int which, int HDim, int VDim) //define the total font number of each row and the total row number
{

   if (which > 3)
      printf("PiP dimension error\n");

   switch (which)
   {
      case 1:
         *(volatile unsigned int *) (LCD_IO_Base + 0x308) &= 0xf800f800;
         *(volatile unsigned int *) (LCD_IO_Base + 0x308) |= VDim | (HDim << 16);
         break;
      case 2:
         *(volatile unsigned int *) (LCD_IO_Base + 0x310) &= 0xf800f800;
         *(volatile unsigned int *) (LCD_IO_Base + 0x310) |= VDim | (HDim << 16);
         break;
      case 3:
         *(volatile unsigned int *) (LCD_IO_Base + 0x330) &= 0xf800f800;
         *(volatile unsigned int *) (LCD_IO_Base + 0x330) |= VDim | (HDim << 16);
         break;
   }
   *(volatile unsigned int *) (LCD_IO_Base + 0x304 ) |= 1 << 28;     //update PiP parameters
} 

void Pip_Priority()
{
   int i,j,k,m;
   
   PiP_On(1);  
   for (i=0;i<4;i++) {
      for (j=0;j<4;j++) {
         if (i!=j) {
            PiP_Priority_Sequence( i, j, 0, 0);
            delay_10ms(50);   
            //printf("printf press key (%d,%d)\n",i,j);
             //scanf("%c",&c);                        
          }
      }
   }
      
      
   PiP_On(2);  
   for (i=0;i<4;i++) {
      for (j=0;j<4;j++) {
         for (k=0;k<4;k++) {
            if ((i!=j) && (i!=k) && (j!=k)) {
                PiP_Priority_Sequence( i, j, k,0);
               delay_10ms(50);   
               //printf("printf press key (%d,%d,%d)\n",i,j,k);
                //scanf("%c",&c);                        
             }
         }
      }
   }
            
   PiP_On(3);     
   for (i=0;i<4;i++) {
      for (j=0;j<4;j++) {
         for (k=0;k<4;k++) {
            for (m=0;m<4;m++) {
               if ((i!=j) && (i!=k) && (i!=m) && (j!=k) && (j!=m) && (k!=m)) {                        
                  PiP_Priority_Sequence( i, j, k, m);
                  delay_10ms(50);   
                  //printf("printf press key (%d, %d,%d,%d)\n",i,j,k,m);                     
                   //scanf("%c",&c);                        
                }
            }
         }
      }  
   }
}  
   
void Color_Key_Test( unsigned char *fb0, unsigned char *fb1, unsigned char *fb2, unsigned char *fb3)
{
#ifndef QDS_RELEASE

   int i;
   unsigned int *wfb0 = (unsigned int *)fb0;
   unsigned int *wfb1 = (unsigned int *)fb1;
   unsigned int *wfb2 = (unsigned int *)fb2;
   unsigned int *wfb3 = (unsigned int *)fb3;       
   
   printf("Test color key in PiP\n");
   //    PiP_Blending(1, 0x7, 0x7);
   
   *(volatile unsigned int *) LCD_IO_Base &= 0xfffffff3;  // no YCbCr
   
   //for TV, should use 16 BPP, 24 BPP will overrun   
   //for (i=0; i<4; i++)
   // PiP_PoP_Image_Format1( i, 888, LBLP, 24 );
   for(i=0; i<4; i++)
      PiP_PoP_Image_Format1( i, 422, LBLP, 16 );
            
   for (i=0; i<PANEL_WIDTH*PANEL_HEIGHT; i++)
      *wfb0++ = 0x777777;

   for (i=0; i<(PANEL_WIDTH/2)*(PANEL_HEIGHT/2); i++)
      *wfb1++ = 0xff0000; //0xaaaaaa;  //0x0aaa50;
      
   for (i=0; i<(PANEL_WIDTH/2)*(PANEL_HEIGHT/2); i++)
      *wfb2++ = 0x00ff00; //0x4488aa;  //0xddddddd;

   for (i=0; i<(PANEL_WIDTH/2)*(PANEL_HEIGHT/2); i++)
      *wfb3++ = 0x0000ff; //0xff1133; //0xaa1155
      
   delay_10ms(100);
   *(volatile unsigned int *)(LCD_IO_Base + 0x320) &= 0xfe000000;
   *(volatile unsigned int *)(LCD_IO_Base + 0x320) |= 0x01ff0000;  //0x1aaaaaa; //0x1ff0000;

   *(volatile unsigned int *)(LCD_IO_Base + 0x324) &= 0xfe000000;
   *(volatile unsigned int *)(LCD_IO_Base + 0x324) |= 0x01dddddd;
   
   *(volatile unsigned int *)(LCD_IO_Base + 0x328) &= 0xfe000000;
   *(volatile unsigned int *)(LCD_IO_Base + 0x328) |= 0x010000ff;
      
   delay_10ms(300);
   // PiP_Blending(0, 0, 0);
   
    Pip_Priority();    
#endif
}


void ARGB8888_Test( unsigned char *fb0, unsigned char *fb1, unsigned char *fb2, unsigned char *fb3)
{
#ifndef QDS_RELEASE
   int i=0;
   volatile unsigned int backup[2];

   printf("ARGB8888 Test\n");
   backup[0] = *(unsigned int *)(LCD_IO_Base);
   backup[1] = *(unsigned int *)(LCD_IO_Base + 0x318);

   *(volatile unsigned int *)(LCD_IO_Base) &= 0xfffffcff;
   *(volatile unsigned int *)(LCD_IO_Base) |= 0x200;     //turn on the ARGB888 mode

   //All the window use ARGB8888
   for(i=0; i<4; i++)
      PiP_PoP_Image_Format1( i, 888, LBLP, ARGB8888 );
      
   clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, 24, 0x1f718293);     
   clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);
   clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x97608010);
   clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x37415089);
   
   //background uses RGB888
   PiP_PoP_Image_Format1( 0, 888, LBLP, 24 );

   strcpy(FILENAME, DEFAULT_PATH);
   printf("Background is RGB888\n");
   
      
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "17_888_320x240.bmp.bin");
      Load_Image( FILENAME, 307200, fb0, LBLP, 24 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "16_888_160x120.bin");
      Load_Image( FILENAME, 76800, fb1, LBLP, 24 );
      for(i=0; i<76800;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
      strcat(FILENAME, "17_888_640x480.bmp.bin");
      Load_Image( FILENAME, 1228800, fb0, LBLP, 24 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "lady888_320x240.bin");
      Load_Image( FILENAME, 307200, fb1, LBLP, 24 );
      for(i=0; i<307200;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
      strcat(FILENAME, "20_888_720x480.bmp.bin");
      Load_Image( FILENAME, 1382400, fb0, LBLP, 24 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "11_888_360x240.bmp.bin");
      Load_Image( FILENAME, 345600, fb1, LBLP, 24 );
      for(i=0; i<345600;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
      strcat(FILENAME, "62_888_720x576.bmp.bin");
      Load_Image( FILENAME, 1658880, fb0, LBLP, 24 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "11_888_360x288.bmp.bin");
      Load_Image( FILENAME, 414720, fb1, LBLP, 24 );
      for(i=0; i<414720;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }

   //background uses RGB565
   PiP_PoP_Image_Format1( 0, 565, LBLP, 16 );

   strcpy(FILENAME, DEFAULT_PATH);
   printf("Background is RGB565\n");
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "12_565_320x240.bmp.bin");
      Load_Image( FILENAME, 153600, fb0, LBLP, 16 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 16, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "15_888_160x120.bmp.bin");
      Load_Image( FILENAME, 76800, fb1, LBLP, 24 );
      for(i=0; i<76800;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
      strcat(FILENAME, "12_565_640x480.bmp.bin");
      Load_Image( FILENAME, 614400, fb0, LBLP, 16 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "73_888_320x240.bmp.bin");
      Load_Image( FILENAME, 307200, fb1, LBLP, 24 );
      for(i=0; i<307200;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
      strcat(FILENAME, "12_565_720x480.bmp.bin");
      Load_Image( FILENAME, 691200, fb0, LBLP, 16 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "15_888_360x240.bmp.bin");
      Load_Image( FILENAME, 345600, fb1, LBLP, 24 );
      for(i=0; i<345600;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
      strcat(FILENAME, "46_565_720x576.bmp.bin");
      Load_Image( FILENAME, 829440, fb0, LBLP, 16 );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "14_888_360x288.bmp.bin");
      Load_Image( FILENAME, 414720, fb1, LBLP, 24 );
      for(i=0; i<414720;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }

   //background uses YCbCr422
   PiP_PoP_Image_Format1( 0, 422, LBLP, 16 );

   strcpy(FILENAME, DEFAULT_PATH);
   printf("Background is YCbCr422\n");
   if( (PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240) ){     //320x240
      strcat(FILENAME, "16_422_320x240.bmp.yuv");
      Load_Image_YCbCr422( FILENAME, 153600, fb0, LBLP );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 16, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "6_888_160x120.bmp.bin");
      Load_Image( FILENAME, 76800, fb1, LBLP, 24 );
      for(i=0; i<76800;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 640) && (PANEL_HEIGHT == 480) ){  //640x480
      strcat(FILENAME, "16_422_640x480.bmp.yuv");
      Load_Image_YCbCr422( FILENAME, 614400, fb0, LBLP );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "61_888_320x240.bmp.bin");
      Load_Image( FILENAME, 307200, fb1, LBLP, 24 );
      for(i=0; i<307200;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 480) ){  //720x480
      strcat(FILENAME, "6_422_720x480.bmp.yuv");
      Load_Image_YCbCr422( FILENAME, 691200, fb0, LBLP );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "11_888_360x240.bmp.bin");
      Load_Image( FILENAME, 345600, fb1, LBLP, 24 );
      for(i=0; i<345600;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }else if( (PANEL_WIDTH == 720) && (PANEL_HEIGHT == 576) ){  //720x576
      strcat(FILENAME, "16_422_720x576.bmp.yuv");
      Load_Image_YCbCr422( FILENAME, 829440, fb0, LBLP );

      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x551080a0);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "13_888_360x288.bmp.bin");
      Load_Image( FILENAME, 414720, fb1, LBLP, 24 );
      for(i=0; i<414720;i+=4)
#ifdef ARGB5888_USED
         *(unsigned int *)(fb1 + i) |= 0x8000000;
#elif defined(ARGB8888_USED)
      *(unsigned int *)(fb1 + i) |= 0x55000000;
#endif
   }


   delay_10ms(70);
   clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, 24, 0x1f123123);

   *(volatile unsigned int *)(LCD_IO_Base) = backup[0];     //restore the original blending setting
   *(volatile unsigned int *)(LCD_IO_Base + 0x318) = backup[1];     //restore the original blending setting
#endif
}

void YCbCr420_in_PiP_Test(unsigned char *fb0, unsigned char *fb1, unsigned char *fb2, unsigned char *fb3)
{
#ifdef ENABLE_YCBCR420_IN_PIP
   unsigned int i;
   //YCbCr420 in CCIR656 test, YCbCr420 will use fifo 0~2, the other format will use fifo 3.

   if((PANEL_WIDTH == 320) && (PANEL_HEIGHT == 240)) {
   
       // YCbCr420 only support 1 PiP
      PiP_On(1);
      PiP_PoP_Image_Format1( 0, 420, LBLP, 8 ); 
      PiP_PoP_Image_Format1( 1, 420, LBLP, 8 );
      PiP_PoP_Image_Format1( 2, 420, LBLP, 8 );    
      //PiP_PoP_Image_Format1( 3, 565, LBLP, 16 ); 
      //PiP_PoP_Image_Format1( 3, 422, LBLP, 16 );       
      PiP_PoP_Image_Format1( 3, 888, LBLP, 24 );   
               
      //im0 uses YCbCr420, im1 uses other format
      *(volatile unsigned int *)(LCD_IO_Base + 0x31C) &= 0xffffffcf;
      *(volatile unsigned int *)(LCD_IO_Base + 0x31C) |= 1 << 5;  //im0 will be YCbCr420  

       printf("Test pure color\n");              
       clear_Screen(fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x4c4c4c4c);
       clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x54545454);
       clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0xffffffff);
      clear_Screen(fb3, PANEL_WIDTH, PANEL_HEIGHT, 24, 0x00FF3C9D); //0x80808080);       
/*      
      *(volatile unsigned int *)(LCD_IO_Base) = 0;
            
       clear_Screen(fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x4c4c4c4c);
       clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x54545454);
       clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0xffffffff);
      clear_Screen(fb3, PANEL_WIDTH, PANEL_HEIGHT, 24, 0x00FFc8cb); 
      
      *(volatile unsigned int *)(LCD_IO_Base+0x04) = 0x10000;     
      
      *(volatile unsigned int *)(LCD_IO_Base) = 0x0c;    
      
      *(volatile unsigned int *)(LCD_IO_Base+0x04) &= 0xFFFFFFF8;
      *(volatile unsigned int *)(LCD_IO_Base+0x04) |= 0x03;              
      
      *(volatile unsigned int *)(LCD_IO_Base+0x318) = 0x5003;  
      *(volatile unsigned int *)(LCD_IO_Base+0x31c) = 1<<5; 
      *(volatile unsigned int *)(LCD_IO_Base+0x314) = 0x0b;             
                  
      PiP_Pos(1, 10, 10);  //HPos needs to be odd if YCbCr422 is used
      PiP_Dim(1, 32, 32);

      *(volatile unsigned int *)(LCD_IO_Base+0x04) |= 0x08;
      
      *(volatile unsigned int *)(LCD_IO_Base) |= 0x400;  
      
      *(volatile unsigned int *)(LCD_IO_Base) |= 0x02;   
         
      *(volatile unsigned int *)(LCD_IO_Base) |= 0x01;
                  
      delay_10ms(300000);
*/              
             
       for (i=2;i<=140;i+=2) {
          printf("%d x %d\n",i,i);
         PiP_Pos(1, PANEL_WIDTH/8 +1, PANEL_HEIGHT/8);   //HPos needs to be odd if YCbCr422 is used
         PiP_Dim(1, i, i);

         delay_10ms(30);   
       }

      PiP_Pos(1, PANEL_WIDTH/8 +1 /*41*/, PANEL_HEIGHT/8 /*50*/); //HPos needs to be odd if YCbCr422 is used
      PiP_Dim(1, PANEL_WIDTH/2, PANEL_HEIGHT/2);
      
      printf("load YCbCr420 in forground\n");
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "20_420_160x120.bmp.yuv");
      Load_Image_YCbCr420(FILENAME, PANEL_WIDTH/2, PANEL_HEIGHT/2, 28800, fb0, LBLP);  
      delay_10ms(300);

       // should change code to mask change to YCBCr422
      //printf("im1 uses YCbCr422\n");
      //strcpy(FILENAME, DEFAULT_PATH);
      //strcat(FILENAME, "11_422_320x240.bmp.yuv");
      //Load_Image_YCbCr422(FILENAME, 153600, fb3, LBLP );
   
      printf("im1 uses RGB888\n");
      strcpy(FILENAME, DEFAULT_PATH);  
      strcat(FILENAME, "78_888_320x240.bmp.bin");
      Load_Image( FILENAME, 307200, fb3, LBLP, 24 );     
   
      delay_10ms(300);  
      *(volatile unsigned int *)(LCD_IO_Base + 0x31C) =0;   
   
      PiP_Off();  
      
      PiP_Pos(1, PANEL_WIDTH/8 +1 /*41*/, PANEL_HEIGHT/8 /*50*/); //HPos needs to be odd if YCbCr422 is used
      PiP_Dim(1, PANEL_WIDTH/2, PANEL_HEIGHT/2);

      PiP_Pos(2, PANEL_WIDTH/4 +1/*111*/, PANEL_HEIGHT/4 /*100*/);
      PiP_Dim(2, PANEL_WIDTH/2, PANEL_HEIGHT/2);

      PiP_Pos(3, (PANEL_WIDTH/8)*3+1/*211*/, (PANEL_HEIGHT/8)*3 + 1/*200*/); 
      PiP_Dim(3, PANEL_WIDTH/2, PANEL_HEIGHT/2);      
            
      return;
   }

   if((PANEL_WIDTH != 640) || (PANEL_HEIGHT != 480)) {
      printf("Currently SW supports only 640x480\n");
      return;
   }
   
    // YCbCr420 only support 1 PiP
   PiP_On(1);
   PiP_PoP_Image_Format1( 0, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 1, 420, LBLP, 8 );
   PiP_PoP_Image_Format1( 2, 420, LBLP, 8 );    
   //PiP_PoP_Image_Format1( 3, 565, LBLP, 16 ); 
   //PiP_PoP_Image_Format1( 3, 422, LBLP, 16 );       
   PiP_PoP_Image_Format1( 3, 888, LBLP, 24 );   
               
   //im0 uses YCbCr420, im1 uses other format
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) &= 0xffffffcf;
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) |= 1 << 5;  //im0 will be YCbCr420  

    printf("Test pure color\n");              
    clear_Screen(fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x4c4c4c4c);
    clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0x54545454);
    clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 24, 0xffffffff);
   clear_Screen(fb3, PANEL_WIDTH, PANEL_HEIGHT, 24, 0x00FF3C9D); //0x80808080);       
       
    for (i=2;i<=140;i+=2) {
       printf("%d x %d\n",i,i);
      PiP_Pos(1, PANEL_WIDTH/8 +1, PANEL_HEIGHT/8);   //HPos needs to be odd if YCbCr422 is used
      PiP_Dim(1, i, i);

      delay_10ms(30);   
    }

   PiP_Pos(1, PANEL_WIDTH/8 +1 /*41*/, PANEL_HEIGHT/8 /*50*/); //HPos needs to be odd if YCbCr422 is used
   PiP_Dim(1, PANEL_WIDTH/2, PANEL_HEIGHT/2);
      
   printf("load YCbCr420 in forground\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "20_420_320x240.bmp.yuv");
   Load_Image_YCbCr420(FILENAME, PANEL_WIDTH/2, PANEL_HEIGHT/2, 115200, fb0, LBLP); 
   delay_10ms(300);

    // should change code to mask change to YCBCr422
   //printf("im1 uses YCbCr422\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "11_422_320x240.bmp.yuv");
   //Load_Image_YCbCr422(FILENAME, 153600, fb3, LBLP );
   
   printf("im1 uses RGB888\n");
   strcpy(FILENAME, DEFAULT_PATH);  
   strcat(FILENAME, "78_888_640x480.bmp.bin");
   Load_Image( FILENAME, 1228800, fb3, LBLP, 24 ); 
   delay_10ms(300);     
   
   //////////////////////////////////////////////////////////////////// 
   
    // YCbCr420 only support 1 PiP
   PiP_On(1);
   PiP_PoP_Image_Format1( 0, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 1, 420, LBLP, 8 );
   PiP_PoP_Image_Format1( 2, 420, LBLP, 8 );    
   PiP_PoP_Image_Format1( 3, 565, LBLP, 16 );   
   //PiP_PoP_Image_Format1( 3, 422, LBLP, 16 );       
   //PiP_PoP_Image_Format1( 3, 888, LBLP, 24 ); 
               
   //im0 uses YCbCr420, im1 uses other format
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) &= 0xffffffcf;
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) |= 1 << 4;  //im0 will be YCbCr420  

   printf("load YCbCr420 in back\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "20_420_640x480.bmp.yuv");
   Load_Image_YCbCr420(FILENAME, PANEL_WIDTH, PANEL_HEIGHT, 460800, fb0, LBLP);  
   delay_10ms(300);

    // should change code to mask change to YCBCr422
   //printf("im1 uses YCbCr422\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "11_422_320x240.bmp.yuv");
   //Load_Image_YCbCr422(FILENAME, 153600, fb3, LBLP );
   
   printf("load RGB565 in fore\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "lady565_320x240.bin");  
   Load_Image( FILENAME, 153600, fb3, LBLP, 16 );  //when im0 is YCbCr420, im1 is put in the framebase 3
               
   //printf("im1 uses RGB888\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "78_888_320x240.bmp.bin");
   //Load_Image( FILENAME, 307200, fb3, LBLP, 24 );   
   delay_10ms(300);     
   
   ////////////////////////////////////////////////////////////////////    
   PiP_PoP_Image_Format1( 0, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 1, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 2, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 3, 565, LBLP, 16 );   
                     
   //im0 uses YCbCr420, im1 uses other format
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) &= 0xffffffcf;
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) |= 1 << 5;  //im0 will be YCbCr420

   printf("load YCbCr420 in fore\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "20_420_320x240.bmp.yuv");
   Load_Image_YCbCr420(FILENAME, PANEL_WIDTH/2, PANEL_HEIGHT/2, 115200, fb0, LBLP);
   delay_10ms(300);
      
   printf("load RGB565 in back\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "82_565_640x480.bmp.bin");
   Load_Image( FILENAME, 614400, fb3, LBLP, 16 );  
   
   delay_10ms(300);  
   
   /////////////////////////////////////////////////////////////////////////////
   PiP_PoP_Image_Format1( 0, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 1, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 2, 420, LBLP, 8 ); 
   PiP_PoP_Image_Format1( 3, 888, LBLP, 24 );      
   
   //im0 uses other format, im1 uses YCbCr420
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) &= 0xffffffcf;
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) |= 1 << 5;     //im1 will be YCbCr420

   printf("load RGB888 in back\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "49_888_640x480.bmp.bin");
   Load_Image( FILENAME, 1228800, fb3, LBLP, 24 );
   delay_10ms(300);
      
   printf("load YCbCr420 in fore\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "16_420_320x240.bmp.yuv");
   Load_Image_YCbCr420( FILENAME, PANEL_WIDTH/2, PANEL_HEIGHT/2, 115200, fb0, LBLP );
   
   //printf("im2 uses YCbCr422\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "20_422_640x480.bmp.yuv");
   //Load_Image_YCbCr422( FILENAME, 614400, fb3, LBLP );
   
   //printf("im3 uses RGB565\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "76_565_640x480.bmp.bin");
   //Load_Image( FILENAME, 614400, fb3, LBLP, 16 );   //when im0 is YCbCr420, im1 is put in the framebase 3
   
   delay_10ms(300);  
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) =0;   
   
   PiP_Off();  
   
/*
   SetFrameBase(0,0, fb0);
   SetFrameBase(1,0, fb1);
   SetFrameBase(2,0, fb2);
   *(volatile unsigned int *)(LCD_IO_Base + 0x31C) &= 0xffffffcf;
   PiP_Pos(1, 41, 50);  //HPos needs to be odd if YCbCr422 is used
   PiP_On(2);
*/ 
#endif

}

void PiP_Test( unsigned char *pFrameBuffer, unsigned short bpp )
{
   int i,k;
   char endian[32];
   unsigned char *fb0, *fb1, *fb2, *fb3;
   
   fb0 = pFrameBuffer;
   fb1 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 32, 1);
   fb2 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 32, 1);
   fb3 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 32, 1);
   //printf("fb0=0x%x fb1=0x%x, fb2=0x%x, fb3=0x%x\n", (unsigned int)fb0, (unsigned int)fb1, (unsigned int)fb2, (unsigned int)fb3);

   setNumFrameBuffer(1, 1, 1, 0);

   //initialize frame buffer first
   SetFrameBase(0, 0, fb0);
   SetFrameBase(1, 0, fb1);
   SetFrameBase(2, 0, fb2);
   SetFrameBase(3, 0, fb3);  

   //    SetDivNo(60);
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);

   PiP_Blending(0, 0, 0);     /* blending off */

   PiP_Pos(1, PANEL_WIDTH/8 +1 /*41*/, PANEL_HEIGHT/8 /*50*/); //HPos needs to be odd if YCbCr422 is used
   PiP_Dim(1, PANEL_WIDTH/2, PANEL_HEIGHT/2);

   PiP_Pos(2, PANEL_WIDTH/4 +1/*111*/, PANEL_HEIGHT/4 /*100*/);
   PiP_Dim(2, PANEL_WIDTH/2, PANEL_HEIGHT/2);

   PiP_Pos(3, (PANEL_WIDTH/8)*3+1/*211*/, (PANEL_HEIGHT/8)*3 + 1/*200*/); 
   PiP_Dim(3, PANEL_WIDTH/2, PANEL_HEIGHT/2);   

   for (k = 0;k < 4;k++)
      PiP_PoP_Image_Format1( k, 565, LBLP, bpp );

   PiP_Priority_Sequence( 3, 2, 1, 0);
   //for wendy's bit file
   //*(volatile unsigned int *)(LCD_IO_Base + 0x314) = 8;
   PiP_On(3);
         
   //action here
   clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0xfffffff);
   clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xfffff00);
   clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xf00ffff);
   clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xfff00ff); 

    // reset frame buffer
   SetFrameBase(0, 0, fb0);
   SetFrameBase(1, 0, fb1);
   SetFrameBase(2, 0, fb2);
   SetFrameBase(3, 0, fb3);
   
   PiP_On(3);
   Color_Key_Test(fb0, fb1, fb2, fb3);
   
   delay_10ms(300);
      
   printf("Test YCbCr420 in PiP\n");
   YCbCr420_in_PiP_Test(fb0, fb1, fb2, fb3);
      
    // reset frame buffer
   SetFrameBase(0, 0, fb0);
   SetFrameBase(1, 0, fb1);
   SetFrameBase(2, 0, fb2);
   SetFrameBase(3, 0, fb3);
         
   delay_10ms(300);
    ///////////////////////////////////////////////////////
   PiP_On(3);       
   PiP_Priority_Sequence(3, 1, 2, 0);
   printf("Test YCbCr422 in PiP\n");
   for(k=0; k<4; k++)
      PiP_PoP_Image_Format1( k, 422, BBBP, bpp );
   ColorBar_YCbCr_422_PiP( BBBP, fb0, PANEL_WIDTH, PANEL_HEIGHT, 0);       
   ColorBar_YCbCr_422_PiP( BBBP, fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 64);  
   ColorBar_YCbCr_422_PiP( BBBP, fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 96);
   ColorBar_YCbCr_422_PiP( BBBP, fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, 192); 
   
   for(k=0; k<4; k++)
      PiP_PoP_Image_Format1( k, 422, LBBP, bpp );;    
   ColorBar_YCbCr_422_PiP( LBBP, fb0, PANEL_WIDTH, PANEL_HEIGHT, 0);
   ColorBar_YCbCr_422_PiP( LBBP, fb1, PANEL_WIDTH/2, PANEL_WIDTH/2, 32);   
   ColorBar_YCbCr_422_PiP( LBBP, fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 120);
   ColorBar_YCbCr_422_PiP( LBBP, fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, 192);
   
   for(k=0; k<4; k++)
      PiP_PoP_Image_Format1( k, 422, LBLP, bpp );     
   ColorBar_YCbCr_422_PiP( LBLP, fb0, PANEL_WIDTH, PANEL_HEIGHT, 0);
   ColorBar_YCbCr_422_PiP( LBLP, fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 44);  
   ColorBar_YCbCr_422_PiP( LBLP, fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 156); 
   ColorBar_YCbCr_422_PiP( LBBP, fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, 192);
   
   //Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp );
   //PiP_On(3);
   /* strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "mov3224.bin");
      animation_test( fb0, FILENAME,  1536000 );
      animation_test( fb1, FILENAME,  1536000 );
      animation_test( fb2, FILENAME,  1536000 );
    */

#ifdef ARGB5888_USED
   PiP_Blending(0, 0xb, 0xb);
#elif defined(ARGB8888_USED)
   PiP_Blending(0, 0x10, 0xb);
#endif
   i = LBLP;   //set the current endian to LBLP
   
   // SetBGRSW( BGR );
   //PiP_Dim(1, PANEL_WIDTH/4, PANEL_HEIGHT/4);
   //PiP_Dim(2, PANEL_WIDTH/4, PANEL_HEIGHT/4);
   //ARGB8888_Test(fb0, fb1, fb2, fb3);
   //PiP_Dim(1, PANEL_WIDTH/2, PANEL_HEIGHT/2);
   //PiP_Dim(2, PANEL_WIDTH/2, PANEL_HEIGHT/2);
   
   
   printf("bpp= %d....................................\n", bpp);
   if( bpp == 24 )
   {
      if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 576 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB888 %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "44_888_720x576.bmp.bin");
            Load_Image( FILENAME, 1658880, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_360x288.bmp.bin");
            Load_Image( FILENAME, 414720, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "12_888_360x288.bmp.bin");
            Load_Image( FILENAME, 414720, fb2, i, bpp );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
         }
      }else if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 480 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB888 %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_888_720x480.bmp.bin");
            Load_Image( FILENAME, 1382400, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_360x240.bmp.bin");
            Load_Image( FILENAME, 345600, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "12_888_360x240.bmp.bin");
            Load_Image( FILENAME, 345600, fb2, i, bpp );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
         }
      }else if( ( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 ) )
      {
         for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
             printf("...%d/%d....\n",i,LBBP);         
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB888 %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_640x480.bin");
            Load_Image( FILENAME, 1228800, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "lady888_320x240.bin");
            Load_Image( FILENAME, 307200, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_888_320x240.bin");
            Load_Image( FILENAME, 307200, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "82_888_320x240.bmp.bin");
            Load_Image( FILENAME, 307200, fb3, i, bpp );

           }
      }
      else if( ( PANEL_WIDTH == 320 ) && ( PANEL_HEIGHT == 240 ) ) {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test RGB888 %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_320x240.bin");
            Load_Image( FILENAME, 307200, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "lady888_160x120.bin");
            Load_Image( FILENAME, 76800, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_888_160x120.bin");
            Load_Image( FILENAME, 76800, fb2, i, bpp );

            //              if( i == LBBP )  continue;

            /*              clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
                  clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
                  clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
             */          }
      }
   }else if ( bpp == 16 )
   {
      //OSD Test
      OSD_Test("BIG OSD OK");    //have to define COMPLEX_OSD

      if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 576 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB565 %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "45_565_720x576.bmp.bin");
            Load_Image( FILENAME, 829440, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "13_565_360x288.bmp.bin");
            Load_Image( FILENAME, 207360, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "14_565_360x288.bmp.bin");
            Load_Image( FILENAME, 207360, fb2, i, bpp );


            PiP_PoP_Image_Format1( 0, 565, i, 16 );
            PiP_PoP_Image_Format1( 1, 888, i, 24 );
            PiP_PoP_Image_Format1( 2, 422, i, 16 );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test different input type %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "72_565_720x576.bmp.bin");
            Load_Image( FILENAME, 829440, fb0, i, 16 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_888_360x288.bmp.bin");
            Load_Image( FILENAME, 414720, fb1, i, 24 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_422_360x288.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 207360, fb2, i );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
         }
      }else if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 480 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB565 %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_565_720x480.bmp.bin");
            Load_Image( FILENAME, 691200, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "13_565_360x240.bmp.bin");
            Load_Image( FILENAME, 172800, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_565_360x240.bmp.bin");
            Load_Image( FILENAME, 172800, fb2, i, bpp );


            PiP_PoP_Image_Format1( 0, 565, i, 16 );
            PiP_PoP_Image_Format1( 1, 888, i, 24 );
            PiP_PoP_Image_Format1( 2, 422, i, 16 );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test different input type %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "78_565_720x480.bmp.bin");
            Load_Image( FILENAME, 691200, fb0, i, 16 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_360x240.bmp.bin");
            Load_Image( FILENAME, 345600, fb1, i, 24 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_422_360x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 172800, fb2, i );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
         }
      }else if( ( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 ) )
      {
#ifdef SIMPLE_PIP_TEST
         /* SetFrameBase(0,0,pFrameBuffer);
            pFrameBuffer = (unsigned char *)0x91000000;
            SetFrameBase(1,0,pFrameBuffer);
            SetFrameBase(2,0,pFrameBuffer);
          */
         strcpy(FILENAME, DEFAULT_PATH);
         strcat(FILENAME, "11_565_640x480.bin");
         Load_Image( FILENAME, 614400, fb0, i, bpp );
   printf("press key\n");
   scanf("%c",&c);         

         strcpy(FILENAME, DEFAULT_PATH);
         strcat(FILENAME, "11_565_320x240.bin");
         Load_Image( FILENAME, 153600, fb1, i, bpp );
   printf("press key\n");
   scanf("%c",&c);
   
         strcpy(FILENAME, DEFAULT_PATH);
         strcat(FILENAME, "lady565_320x240.bin");
         Load_Image( FILENAME, 153600, fb2, i, bpp );
   printf("press key\n");
   scanf("%c",&c);
   
         *( unsigned int *) (LCD_IO_Base + 0x0000 )|= (0x2 << 9);
         *( unsigned int *) (LCD_IO_Base + 0x0300 )= (0x0 << 0);
         *( unsigned int *) (LCD_IO_Base + 0x0304 )= (51 << 0)|(50 << 16);
         *( unsigned int *) (LCD_IO_Base + 0x0308 )= (PANEL_WIDTH/2 << 16)|(PANEL_HEIGHT/2 << 0);  //originally horizontal vertical inverse
         *( unsigned int *) (LCD_IO_Base + 0x030c )= (100 << 0)|(101 << 16);
         *( unsigned int *) (LCD_IO_Base + 0x0310 )= (PANEL_WIDTH/2 << 16)|(PANEL_HEIGHT/2 << 0);  //originally horizontal vertical inverse
         enable_lcd_controller();   
#endif
         for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
             printf("...%d/%d....\n",i,LBBP);
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf( "Test RGB565 %s in PiP\n", endian );

               PiP_Priority_Sequence( 2, 1, 0, 3);
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_565_640x480.bin");
            Load_Image( FILENAME, 614400, fb0, i, bpp );          
                delay_10ms(100);
   
               PiP_Priority_Sequence( 1, 2, 0, 3);    
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_565_320x240.bin");
            Load_Image( FILENAME, 153600, fb1, i, bpp );          
                delay_10ms(100);
   
               PiP_Priority_Sequence( 0, 1, 2, 3); 
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "lady565_320x240.bin");
            Load_Image( FILENAME, 153600, fb2, i, bpp );             
                delay_10ms(100);
   
               PiP_Priority_Sequence( 3, 0, 1, 2);    
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "oldman_565_320x240.bmp.bin");
            Load_Image( FILENAME, 153600, fb3, i, bpp );          
                delay_10ms(100);
   
               ////////////////////////////////////////////////////////             
            PiP_PoP_Image_Format1( 0, 565, i, 16 );
            PiP_PoP_Image_Format1( 1, 888, i, 24 );
            PiP_PoP_Image_Format1( 2, 422, i, 16 );
            PiP_PoP_Image_Format1( 3, 422, i, 24 );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test different input type %s in PiP\n", endian);

               PiP_Priority_Sequence( 2, 1, 0, 3);
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "68_565_640x480.bmp.bin");
            Load_Image( FILENAME, 614400, fb0, i, 16 );        
                delay_10ms(100);
   
               PiP_Priority_Sequence( 1, 2, 0, 3);    
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "82_888_320x240.bmp.bin");
            Load_Image( FILENAME, 320*240*4, fb1, i, 24 );           
                delay_10ms(100);
   
               PiP_Priority_Sequence( 0, 1, 2, 3);    
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "45_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, fb2, i );            
                delay_10ms(100);
   
               PiP_Priority_Sequence( 3, 0, 1, 2); 
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "46_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, fb3, i );            
                delay_10ms(100);
               
            //              if( i == LBBP )  continue;

            //clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            //clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            //clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
         }
      }else if( ( PANEL_WIDTH == 320 ) && ( PANEL_HEIGHT == 240 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            PiP_PoP_Image_Format1( 0, 565, i, 16 );
            PiP_PoP_Image_Format1( 1, 888, i, 24 );
            PiP_PoP_Image_Format1( 2, 422, i, 16 );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test different input type %s in PiP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_565_320x240.bin");
            Load_Image( FILENAME, 153600, fb0, i, 16 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "18_888_160x120.bmp.bin");
            Load_Image( FILENAME, 76800, fb1, i, 24 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_422_160x120.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 38400, fb2, i );
            
                strcpy(FILENAME, DEFAULT_PATH);
                strcat(FILENAME, "16_422_160x120.bmp.yuv");
                Load_Image_YCbCr422( FILENAME, 38400, fb3, i );            
                delay_10ms(100);          

            //              if( i == LBBP )  continue;

            /*              clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
                  clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
                  clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
             */          }
      }
   }
   delay_10ms(100);
    Pip_Priority(); 
   
   //PiP_On(2);
   PiP_Blending(1, 0, 0);
   delay_10ms(50);

   PiP_Blending(1, 0, 0xa);
   delay_10ms(50);

   PiP_Blending(1, 0, 0xf);
   delay_10ms(50);

   PiP_Blending(1, 0xf, 0xf);
   delay_10ms(50);

   PiP_Blending(0, 0, 0);

   Cursor_Test();

   // SetBGRSW( RGB );
   // free(fb0);
   free(fb1);
   free(fb2);
   free(fb3);
   
    PiP_Off(); 
}
