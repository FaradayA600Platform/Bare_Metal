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
//#include "fa52x_cache.h"
#include "ftlcd210_conf.h"
#include "ftlcd210-OSD.h"
#include "PiP_PoP_lib.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/


extern LCDMTYPE_T FLcdModule[];
extern unsigned int DIVISOR;
extern unsigned int LCD_IRQ;  
extern int FIFO_Under_Run;

/******************************************************************************
 *Public functions
 *****************************************************************************/
void ColorBar_YCbCr_420( unsigned char endian, unsigned char *pFrameBuffer, int width, int height, int int_test)
{
   unsigned char *YPtr, *CbPtr, *CrPtr, r, g, b;
   int  i,j;

   OSD_Off();
   SetYCbCr(420);
   // SetDivNo(DIVISOR); 
   SetBPP(8);  //bpp must be 8 in YCbCr420 mode
   for(i=0; i<4; i++)
      PiP_PoP_Image_Format1( i, 420, endian, 8 );
   SetEndian( endian ); 

   r=0, g=0; b=255;
   rgb2ycbcr( &r, &g, &b ); 

   // *(volatile unsigned int *)(LCD_IO_Base + 0x10) |= 0x20;  //define the size of the frame buffer (at least 1000(binary))and set the LCD frame base address

   YPtr  = pFrameBuffer;   //uses original declaring framebuffer  //allocateFrameBuffer(width, height, 16, 1);
   CbPtr = allocateFrameBuffer(width, height, 8, 1);
   CrPtr = allocateFrameBuffer(width, height, 8, 1);

   setNumFrameBuffer(1, 1, 1, 0);

   //initialize frame buffer first  
   SetFrameBase(0,0, YPtr);
   SetFrameBase(1,0, CbPtr);
   SetFrameBase(2,0, CrPtr);

   switch(endian)
   {
      case BBBP:  printf("Colorbar YCbCr420 BBBP Testing\n");
             break;
      case LBLP:  printf("Colorbar YCbCr420 LBLP Testing\n");
             break;
      case LBBP:   printf("Colorbar YCbCr420 LBBP Testing\n");
              break;                                    
   }

   reset_lcd_controller();
   /************** just for debug ***************/
   r=255, g=0; b=0;
   rgb2ycbcr( &r, &g, &b );
   for(i=0; i<height; i++){
      for(j=0; j<width; j++)     //entries need to be even because Y : Cb : Cr = 4 : 1 : 1 (兵絬琌丁筳Cb,Cr┮璶案计︽)
      {           
         *(unsigned char *) (YPtr + i * width + j ) = 0;//r;

         if( ((i%2) == 0)/* && (j<width/2)*/ )  *(unsigned char *) (CbPtr + (width>>1) * (i>> 1) +  (j>>1) ) = 0;//g;
         //          else if( ((i%2) == 0) && (j>=width/2) )*(unsigned char *) (CbPtr + (width>>1) * (i>> 1) +  (j>>1) ) = 0x20;
         if( (i%2) == 0 )  *(unsigned char *) (CrPtr + (width>>1) * (i>> 1) + (j>>1) ) = 0;//b;
      }
   }

   /************** end just for debug ***********/
   if(endian == BBBP)
   {
      for(i=0; i<height; i++)  //***********  draw pattern
      {
         r=0, g=255; b=0;  //draw red bar
         rgb2ycbcr( &r, &g, &b );
         /*       for(j=0; j<width; j++)  //fill the panel with the same color
                  {
          *(unsigned char *) (YPtr + i * width + j ) = r;
          *(unsigned char *) (CbPtr + i * width + j ) = g; 
          *(unsigned char *) (CrPtr + i * width + j )= b;
          }
          */

         for(j=0; j<(width * 1/3 ); j++)     //entries need to be even because Y : Cb : Cr = 4 : 1 : 1 (兵絬琌丁筳Cb,Cr┮璶案计︽)
         {           
            *(unsigned char *) (YPtr + i * width + (j & 0xFFFFFFFC ) + 3-(j%4) ) = r; 

            if( ((i%2) == 0) && ((j%2) == 0) )  
               *(unsigned char *) (CbPtr + (width>>1) * (i>> 1) +  ((j>>1)  & 0xFFFFFFFC)  + (3 - ((j>>1) %4) ) ) = g; 
            if( ((i%2) == 0) && ((j%2) == 0) )  
               *(unsigned char *) (CrPtr + (width>>1) * (i>> 1) +  ((j>>1)  & 0xFFFFFFFC) + (3 - ((j>>1) %4)) )= b;     
         }                


         r=0, g=0; b=255;
         rgb2ycbcr( &r, &g, &b );
         for( ; j<(width * 2/3 -1); j++)     //entries need to be even because Y : Cb : Cr = 4 : 1 : 1 (兵絬琌丁筳Cb,Cr┮璶案计︽)
         {
            *(unsigned char *) (YPtr + i * width + (j & 0xFFFFFFFC ) + 3-(j%4) ) = r;

            if( ((i%2) == 0) && ((j%2) == 0) )  
               *(unsigned char *) (CbPtr + (width>>1) * (i>> 1) +  ((j>>1)  & 0xFFFFFFFC)  + (3 - ((j>>1) %4) )) = g;
            if( ((i%2) == 0) && ((j%2) == 0) )  
               *(unsigned char *) (CrPtr + (width>>1) * (i>> 1) +  ((j>>1)  & 0xFFFFFFFC) + (3 - ((j>>1) %4))) = b; 
         }     

         r=255, g=0; b=0;
         rgb2ycbcr( &r, &g, &b );  
         for( ; j<width; j++)    //entries need to be even because Y : Cb : Cr = 4 : 1 : 1 (兵絬琌丁筳Cb,Cr┮璶案计︽)
         {
            *(unsigned char *) (YPtr + i * width + (j & 0xFFFFFFFC ) + 3-(j%4) ) = r;

            if( ((i%2) == 0) && ((j%2) == 0) )  
               *(unsigned char *) (CbPtr + (width>>1) * (i>> 1) +  ((j>>1)  & 0xFFFFFFFC)  + (3 - ((j>>1) %4) ) )= g;
            if( ((i%2) == 0) && ((j%2) == 0) )  
               *(unsigned char *) (CrPtr + (width>>1) * (i>> 1) +  ((j>>1)  & 0xFFFFFFFC) + (3 - ((j>>1) %4)) )= b;                    
         }                                   
      } // end of i loop   
   }  //end of if(BBBP)       
   else
   {
      for(i=0; i<height; i++)  //***********  draw pattern
      {
         r=255, g=0; b=0;
         rgb2ycbcr( &r, &g, &b );   
         for(j=0; j<(width * 1/3); j++)      //entries need to be even because Y : Cb : Cr = 4 : 1 : 1 (兵絬琌丁筳Cb,Cr┮璶案计︽)
         {           
            *(unsigned char *) (YPtr + i * width + j ) = r;

            if( (i%2) == 0 )  *(unsigned char *) (CbPtr + (width>>1) * (i>> 1) +  (j>>1) ) = g;
            if( (i%2) == 0 )  *(unsigned char *) (CrPtr + (width>>1) * (i>> 1) + (j>>1) ) = b;     
         }                


         r=0, g=255; b=0;
         rgb2ycbcr( &r, &g, &b );   
         for( ; j<(width * 2/3 -1); j++)     //entries need to be even because Y : Cb : Cr = 4 : 1 : 1 (兵絬琌丁筳Cb,Cr┮璶案计︽)
         {
            *(unsigned char *) (YPtr + i * width + j) = r;
            if( (i%2) == 0 ) *(unsigned char *) (CbPtr + (width>>1) * (i>> 1) + (j>>1) ) = g;
            if( (i%2) == 0 ) *(unsigned char *) (CrPtr + (width>>1) * (i>> 1) + (j>>1) ) = b;
         }     

         r=0, g=0; b=255;
         rgb2ycbcr( &r, &g, &b );  
         for( ; j<width; j++)    //entries need to be even because Y : Cb : Cr = 4 : 1 : 1 (兵絬琌丁筳Cb,Cr┮璶案计︽)
         {
            *(unsigned char *) (YPtr + i * width + j) = r; 
            if( (i%2) == 0 ) *(unsigned char *) (CbPtr + (width>>1) * (i>> 1) + (j>>1) ) = g;
            if( (i%2) == 0 ) *(unsigned char *) (CrPtr + (width>>1) * (i>> 1) + (j>>1) ) = b;                     
         }                                   
      }          
   }   
   /*free(YPtr);*/   
   free(CbPtr);   
   free(CrPtr);
   
   delay_10ms(300);
}



void ColorBar_YCbCr_422( unsigned char endian, unsigned char *pFrameBuffer, int width, int height, int int_test)
{
#if 0
   int i,j;
   unsigned char r,g,b, result;
   int LEVEL=0;

   OSD_Off();
   
   if (int_test) {
      FIFO_Under_Run=0;

      fLib_ConnectInt(LCD_IRQ,  UnderRun_Handler );   //it only can be

      UnmaskIRQ(LCD_IRQ);
      SetIRQmode(LCD_IRQ,LEVEL); 
   }  

   SetBPP(16); //The BPP value is 16 and cannot be changed while YCbCr422 is enabled.
   for(i=0; i<4; i++)
      PiP_PoP_Image_Format1( i, 422, endian, 16 );
      
   SetEndian(endian);
   SetYCbCr(422);
   // SetDivNo(DIVISOR); 

   switch(endian)
   {
      case BBBP:  
         printf("Colorbar YCbCr422 BBBP Testing\n");
         break;
      case LBLP:  
         printf("Colorbar YCbCr422 LBLP Testing\n");
         break;
      case LBBP:  
         printf("Colorbar YCbCr422 LBBP Testing\n");
         break;                                  
   }

   reset_lcd_controller();

   for(i=0; i<height; i++)                           //***********  draw pattern
   {
      r=255, g=0; b=0;                         // Draw red Bar
      rgb2ycbcr( &r, &g, &b );
      for(j=0; j< ( width * 1/3) *2 ; j=j+4) {          
         if( endian == BBBP) {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  //16 bpp = 0.5 pixel per byte
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = g;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = b;  
         }
         else {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = b;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = g;               
         }                           
      }      

      r=0, g=255; b=0;                          // Draw green bar
      rgb2ycbcr( &r, &g, &b );              
      for( ; j< (width * 2/3 )*2 ; j=j+4) {          
         if( endian == BBBP) {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = g;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = b;  
         }
         else {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = b;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = g;          
         }                           
      }

      r=0, g=0; b=255;
      rgb2ycbcr( &r, &g, &b );           //Draw blue bar
      for(  ; j<(width * 3/3) *2 ; j=j+4) {          
         if( endian == BBBP) {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = g;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = b;  
         }
         else {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = b;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = g;                     
         }                 
      }
   }

   //************* Check Buffer Under Run ***************************************

   if( int_test)
   { 
      *(volatile unsigned int *) ( LCD_IO_Base + 0x0C )= 0x00000001; //clear the FIFO under-run interrupt status
      *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000001; //enable FIFO under-run interrupt table

      EnableIRQ();   

      printf("Change divisor, press 'y' or 'Y'");
      do{
         result = getchar();
      }while( (result != 'y') );  

      SetDivNo(1); 


      printf("Check if your screen flash, press 'y' or 'Y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

      if( FIFO_Under_Run ) printf("Generate FIFO Under Run Interrupt Successfully\n");
      else  printf("FIFO Under Run Interrupt Fail\n");    


      if(FIFO_Under_Run) printf("FIFO under run generate interrupt\n");
      else  printf("FIFO under run interrupt fail\n");

      SetDivNo(DIVISOR);      
   }      
   
   delay_10ms(200);
   #endif
}


void ColorBar_YCbCr_422_PiP( unsigned char endian, unsigned char *pFrameBuffer, int width, int height, unsigned int inc)
{
   int i,j;
   unsigned char r,g,b;

   OSD_Off();
   
   SetBPP(16); //The BPP value is 16 and cannot be changed while YCbCr422 is enabled.
   for(i=0; i<4; i++)
      PiP_PoP_Image_Format1( i, 422, endian, 16 );
      
   SetEndian(endian);
   SetYCbCr(422);
   // SetDivNo(DIVISOR); 

   switch(endian)
   {
      case BBBP:  
         printf("PiP Colorbar YCbCr422 BBBP Testing\n");
         break;
      case LBLP:  
         printf("PiP Colorbar YCbCr422 LBLP Testing\n");
         break;
      case LBBP:  
         printf("PiP Colorbar YCbCr422 LBBP Testing\n");
         break;                                  
   }

   reset_lcd_controller();

   for(i=0; i<height; i++)                           //***********  draw pattern
   {
      r=255-inc, g=inc; b=inc;                         // Draw red Bar
      rgb2ycbcr( &r, &g, &b );
      for(j=0; j< ( width * 1/3) *2 ; j=j+4) {          
         if( endian == BBBP) {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  //16 bpp = 0.5 pixel per byte
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = g;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = b;  
         }
         else {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = b;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = g;               
         }                           
      }      

      r=inc, g=255-inc; b=inc;                          // Draw green bar
      rgb2ycbcr( &r, &g, &b );              
      for( ; j< (width * 2/3 )*2 ; j=j+4) {          
         if( endian == BBBP) {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = g;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = b;  
         }
         else {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = b;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = g;          
         }                           
      }

      r=inc, g=inc; b=255-inc;
      rgb2ycbcr( &r, &g, &b );           //Draw blue bar
      for(  ; j<(width * 3/3) *2 ; j=j+4) {          
         if( endian == BBBP) {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = g;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = b;  
         }
         else {
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+3 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j+2 ) = b;                    

            *(unsigned char *) (pFrameBuffer + i * width*2 + j+1 ) = r;  
            *(unsigned char *) (pFrameBuffer + i * width*2 + j ) = g;                     
         }                 
      }
   }
   
   delay_10ms(200);
}



/************************************* have to check the color mapping of the palette ************************************/

/*************** need to check especially ablout palette color ???***************************/
void ColorBar_RGB( unsigned char bpp, unsigned char endian, unsigned char *pFrameBuffer, int width, int height, int int_test)
{
   int i,j;
   char result;
   unsigned int pixel_per_byte;
   int LEV=0;

   //Test normal RGB

   unsigned char pattern[4][5] = {
      {0x00, 0xff, 0x00, 0x0f, 0xf0},  //1bpp {Bar1, Bar2, Bar3, pixel-enidan1, pixel-endian2}
       {0x00, 0x55, 0xaa, 0x05, 0x50},  //2bpp {Bar1, Bar2, Bar3, pixel-enidan1, pixel-endian2}
       {0x00, 0x11, 0x22, 0x01, 0x10},  //4bpp {Bar1, Bar2, Bar3, pixel-enidan1, pixel-endian2}
       {0x00, 0x1,  0x2,  0x99, 0x99}   //8bpp {Bar1, Bar2, Bar3, no-pixel-endian, no-pixel-endian}                             
   };  

   unsigned char bpp_tbl[9] = {99, 0, 1, 99, 2, 99, 99, 99, 3};  //bpp can be 1, 2, 4, 8

   pixel_per_byte = 8 / bpp;
   // Init_LCD( LCD_IO_Base, LcdModule, LCD_PANEL_TYPE, bpp);

   OSD_Off(); 
   *(volatile unsigned int *)(LCD_IO_Base + 0xA00) = 0x07E0F800;
   // Palette 0 = Red,  Palette 1=Green   (LCD palette RAM accessing port)
   *(volatile unsigned int *) (LCD_IO_Base + 0xA04) = 0x0000001F;
   // Palette 2 = Blue, Palette 3=Black, how values in framebuffer remap their color ?
   *(volatile unsigned int *) (LCD_IO_Base + 0xA08) = 0x07E0FFFF;
   // Palette 4 = White, Palette 5=Green  (LCD palette RAM accessing port)
   *(volatile unsigned int *) (LCD_IO_Base + 0xA0C) = 0xF800001F;
   // Palette 6 = Blue, Palette 7=Red, how values in framebuffer remap their color ?

   SetEndian(endian);
   SetYCbCr(0);
   SetBPP(bpp);
   /*if(bpp == 16){
     for(i=0; i<4; i++)
     PiP_PoP_Image_Format1( i, 565, endian, 16 );
     }else if(bpp == 24){
     for(i=0; i<4; i++)
     PiP_PoP_Image_Format1( i, 888, endian, 24 );
     }*/
   //bpp can never > 8

   switch (endian) {
      case BBBP:
         printf("RGB Palette %d BBBP Testing\n", bpp);
         break;
      case LBLP:
         printf("RGB Palette %d LBLP Testing\n", bpp);
         break;
      case LBBP:
         printf("RGB Palette %d LBBP Testing\n", bpp);
         break;                                  
   }

   reset_lcd_controller();
   //       for( j=0; j<(width/pixel_per_byte) * height; j++ )
   //          *(unsigned int *)( pFrameBuffer + j ) = 0xffffffff;

   if (endian == LBLP) {
      //    printf("Little Byte Little Pixel Testing\n");
      for (i = 0; i < height; i++) {      //***********  draw pattern
         /*       for (j = 0;j < (width / pixel_per_byte / 3);j++)
          *(pFrameBuffer + i*(width/pixel_per_byte) + j) = pattern[bpp_tbl[bpp]][0];
          for (j = (width / pixel_per_byte / 3);j < (width/pixel_per_byte) * 2/3; j++)
          *(pFrameBuffer + i*(width/pixel_per_byte) + j) = pattern[bpp_tbl[bpp]][1];
          for (j = (width/pixel_per_byte) * 2/3;j < (width/pixel_per_byte);j++)
          *(pFrameBuffer + i*(width/pixel_per_byte) + j) = pattern[bpp_tbl[bpp]][2];  
          */
         for (j = 0;j < width*1/3;j++){
            if (bpp == 1) {
               if ((j%pixel_per_byte) == 0)
                  *(pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) = pattern[bpp_tbl[bpp]][0];
            }else{
               if( (j% pixel_per_byte ) ==0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][0];
            }
         }

         for(j=width* 1/3; j< width * 2/3; j++){
            if(bpp == 1){
               if( (j% pixel_per_byte ) == 0 ) 
                  *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][1];
            }
            else{
               if( (j% pixel_per_byte ) == 0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][1];
            }
         }

         for(j=width * 2/3 ; j<width; j++){
            if(bpp == 1){
               if( (j% pixel_per_byte )== 0 ) 
               *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) = pattern[bpp_tbl[bpp]][2];
            }
            else{
               if( (j% pixel_per_byte )== 0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) = pattern[bpp_tbl[bpp]][2];
            }
         }
      }    
   }

   else if( endian == BBBP )
   {
      //    printf("Big Byte Big Pixel Testing\n");  
      for(i=0; i<height; i++)  //***********  draw pattern    
      {
         for(j=0; j<( width* 1/3) || (j%pixel_per_byte) != 0 ; j++){             // Byte order formula floor(j/4) *4  + (3-(j%4)) ; 4 -> means 1 words = 4bytes
            if(bpp == 1){
               if( (j%pixel_per_byte) ==0 ) 
                  *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) = pattern[ bpp_tbl[bpp] ][0];
            }
            else{
               if( (j%pixel_per_byte) ==0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) = pattern[ bpp_tbl[bpp] ][0];
            }
         }
         // Test pixel endian
         if(bpp == 1)
            *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) = pattern[ bpp_tbl[bpp] ][3];  // ?
         else
            *( pFrameBuffer + i*(width/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) = pattern[ bpp_tbl[bpp] ][3];  // ?

         for(/*j=width* 1/3*/ ; j< width * 2/3 || (j%pixel_per_byte) != 0 ; j++){
            if(bpp == 1){
               if( (j%pixel_per_byte) == 0 ) 
                  *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4)) = pattern[ bpp_tbl[bpp] ][1];
            }else{
               if( (j%pixel_per_byte) == 0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4)) = pattern[ bpp_tbl[bpp] ][1];
            }
         }
         //        j=419;  // Test pixel endian 
         if(bpp == 1)
            *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) = pattern[ bpp_tbl[bpp] ][4];
         else
            *( pFrameBuffer + i*(width/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) = pattern[ bpp_tbl[bpp] ][4];

         for(/*j=width * 1/3 *2*/  ; j<width; j++){
            if(bpp == 1){
               if( (j%pixel_per_byte)== 0 ) 
                  *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) =pattern[ bpp_tbl[bpp] ][2];
            }else{
               if( (j%pixel_per_byte)== 0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + (unsigned int) floor( (j/pixel_per_byte)/4) *4  +3 - ((j/pixel_per_byte) % 4) ) =pattern[ bpp_tbl[bpp] ][2];
            }
         }
      }             
   }   

   else if( endian == LBBP )
   { 
      //    printf("Little Byte Big Pixel Testing\n");

      for(i=0; i<height; i++)  //***********  draw pattern
      {
         for(j=0; j<( width* 1/3) || (j%pixel_per_byte) != 0 ; j++){
            if(bpp == 1){
               if( (j% pixel_per_byte ) ==0 ) 
                  *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][0];
            }else{
               if( (j% pixel_per_byte ) ==0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][0];
            }
         }

         if(bpp == 1)
            *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][3];
         else
            *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][3];

         for(/*j=width* 1/3*/ ; j< width * 2/3 || (j%pixel_per_byte) != 0 ; j++){
            if(bpp == 1){
               if( (j%pixel_per_byte) == 0 ) 
                  *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) =pattern[ bpp_tbl[bpp] ][1];
            }else{
               if( (j%pixel_per_byte) == 0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) =pattern[ bpp_tbl[bpp] ][1];
            }
         }

         if(bpp == 1)
            *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][4];
         else
            *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][4];

         for(/*j=width * 1/3 *2*/  ; j<width; j++){
            if(bpp == 1) {
               if( (j%pixel_per_byte)== 0 ) 
                  *( pFrameBuffer + i*(ALIGN32(width)/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][2];
            }else{
               if( (j%pixel_per_byte)== 0 ) 
                  *( pFrameBuffer + i*(width/pixel_per_byte) + j/pixel_per_byte) = pattern[ bpp_tbl[bpp] ][2];
            }
         }
      }    
   } 


   //************* Check Buffer Under Run ***************************************
   if( int_test)
   {
      FIFO_Under_Run=0;

      *(volatile unsigned int *) ( LCD_IO_Base + 0x0C )= 0x00000001; //clear the FIFO under-run interrupt status
      *(volatile unsigned int *) ( LCD_IO_Base + 0x08 )= 0x00000001; //enable FIFO under-run interrupt table

      fLib_ConnectInt(LCD_IRQ,  UnderRun_Handler );   //it only can be, routine to check and install requested interrupt.
      //IRQ_LCD is (2+IRQ_OFFSET) and IRQ_OFFSET=32

      UnmaskIRQ(LCD_IRQ);
      SetIRQmode(LCD_IRQ,LEV);  

      EnableIRQ();   

      printf("Change divisor, press 'y' or 'Y'");
      do{
         result = getchar();
      }while( (result != 'y') );  

      SetDivNo(1); 

      printf("Check if your screen flash, press 'y' or 'Y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

      if ( FIFO_Under_Run ) 
         printf("Generate FIFO Under Run Interrupt Successfully\n");
      else  
         printf("FIFO Under Run Interrupt Fail\n");    

      if (FIFO_Under_Run) 
         printf("FIFO under run generate interrupt\n");
      else  
         printf("FIFO under run interrupt fail\n");


      UnmaskIRQ(LCD_IRQ);  //IRQ_LCD   (2+IRQ_OFFSET) IRQ_OFFSET=32
      DisableIRQ();  
      fLib_CloseInt(LCD_IRQ);         
   }  
}


void TV_flicker_ColorBar_RGB( unsigned char bpp,unsigned char *pFrameBuffer, int width, int height)
{
   int i,j,k;
   unsigned int pixel_per_byte;
   unsigned int block;
   unsigned int no;

   unsigned char pattern[3][3] = {
      {0x02, 0xff, 0x04}, // blue, black, white 
       {0x00, 0x01, 0x02}, // red,  green, blue
       {0x01, 0x02, 0x07}, // green, blue, Red                              
   };  

   OSD_Off(); 
   *(volatile unsigned int *)(LCD_IO_Base + 0xA00) = 0x07E0F800;
   // Palette 0 = Red,  Palette 1=Green   (LCD palette RAM accessing port)
   *(volatile unsigned int *) (LCD_IO_Base + 0xA04) = 0x0000001F;
   // Palette 2 = Blue, Palette 3=Black, how values in framebuffer remap their color ?
   *(volatile unsigned int *) (LCD_IO_Base + 0xA08) = 0x07E0FFFF;
   // Palette 4 = White, Palette 5=Green  (LCD palette RAM accessing port)
   *(volatile unsigned int *) (LCD_IO_Base + 0xA0C) = 0xF800001F;
   // Palette 6 = Blue, Palette 7=Red, how values in framebuffer remap their color ?
   
   pixel_per_byte = 8 / bpp;

   SetEndian(LBLP);
   SetYCbCr(0);
   SetBPP(bpp);
    
   reset_lcd_controller();
/* 
   for (i = 0; i < height; i+=30) { 
       for (k=0;k<1;k++) {
          for (j=0;j<width;j++)
              *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[3][0];  
        }                          
           
       for (k=0;k<1;k++) {           
          for (j=0;j<width;j++)
              *(pFrameBuffer + (i+10+k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[3][1];
        }
             
       for (k=0;k<1;k++) {                       
          for (j=0;j<width;j++)
              *(pFrameBuffer + (i+20+k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[3][2];                            
        }
   }

*/
    
    no = 0;
    
    for (block=30;block<height;block+=3) {
       //printf("block=%d\n",block);
    
      for (i = 0; i < height; i+=block) { 
         for (k=0;k<block/3;k++) {
            for (j=0;j<width/3;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][0];      
            for (;j<width*2/3;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][1];  
            for (;j<width;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][2];                
          }  
        
         for (;k<block*2/3;k++) {
            for (j=0;j<width/3;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][1];      
            for (;j<width*2/3;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][2];     
            for (;j<width;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][0];                
          }  
        
         for (;k<block;k++) {
            for (j=0;j<width/3;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][2];         
            for (;j<width*2/3;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][0];     
            for (;j<width;j++)
                *(pFrameBuffer + (i+ k)*(width*pixel_per_byte) + j*pixel_per_byte) = pattern[no][1];                   
          }                                                  
      }
      delay_10ms(200);
    }
    
}

void TV_flicker_YCbCr_422( unsigned char endian, unsigned char *pFrameBuffer, int width, int height, unsigned int inc)
{
   unsigned int i,j,temp,block,r,g,b,step,size;
   unsigned int select[3];
   unsigned char color[3][3];

    //color[0][0]=255, color[0][1]=255;  color[0][2]=255;       // white
    //color[0][0]=255-inc, color[0][1]=inc;  color[0][2]=inc; //red
   //color[2][0]=inc, color[2][1]=inc; color[2][2]=255-inc;  //blue    
   //color[1][0]=inc, color[1][1]=255-inc; color[1][2]=inc; // green      
   //color[2][0]=0, color[2][1]=0; color[2][2]=0;  //black  
      
    color[0][0]=255, color[0][1]=255;  color[0][2]=255;      // white
   rgb2ycbcr( &color[0][0], &color[0][1], &color[0][2] ); 
         
   color[1][0]=inc, color[1][1]=inc; color[1][2]=255-inc;  //blue    
   rgb2ycbcr( &color[1][0], &color[1][1], &color[1][2] );   
      
   color[2][0]=0, color[2][1]=0; color[2][2]=0;  //black                   
   rgb2ycbcr( &color[2][0], &color[2][1], &color[2][2] );    
        
   OSD_Off();
   
   SetBPP(16); //The BPP value is 16 and cannot be changed while YCbCr422 is enabled.
      
   SetEndian(endian);
   SetYCbCr(422);
   // SetDivNo(DIVISOR); 

   switch(endian)
   {
      case BBBP:  
         printf("PiP Colorbar YCbCr422 BBBP Testing\n");
         break;
      case LBLP:  
         printf("PiP Colorbar YCbCr422 LBLP Testing\n");
         break;
      case LBBP:  
         printf("PiP Colorbar YCbCr422 LBBP Testing\n");
         break;                                  
   }

   reset_lcd_controller();
      
    
    for (size=30;size<(height/2);size+=3) 
    {    
       select[0] = 0;
       select[1] = 1;
       select[2] = 2;      
       for (block=size; block<=height; block+=size) 
       {     
         for (i=0; i<block; i++) 
         {
            step = (block-size)*width*2;
               
            r=color[select[0]][0];
            g=color[select[0]][1];
            b=color[select[0]][2];
           for(j=0; j< ( width * 1/3) *2 ; j=j+4) {          
             if( endian == BBBP) {
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+3 ) = r; 
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+2 ) = g;                    

               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+1 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j ) = b;  
             }
             else {
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+3 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+2 ) = b;                    

               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+1 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j ) = g;               
             }                           
           }      

            r=color[select[1]][0];
            g=color[select[1]][1];
            b=color[select[1]][2];            
           for( ; j< (width * 2/3 )*2 ; j=j+4) {          
             if( endian == BBBP) {
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+3 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+2 ) = g;                    

               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+1 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j ) = b;  
             }
             else {
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+3 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+2 ) = b;                    

               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+1 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j ) = g;          
             }                           
           }

            r=color[select[2]][0];
            g=color[select[2]][1];
            b=color[select[2]][2];
           for(  ; j<(width * 3/3) *2 ; j=j+4) {          
             if( endian == BBBP) {
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+3 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+2 ) = g;                    

               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+1 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j ) = b;  
             }
             else {
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+3 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+2 ) = b;                    

               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j+1 ) = r;  
               *(unsigned char *) (pFrameBuffer + step + i * width*2 + j ) = g;                     
             }                 
           }
         }
      
         temp = select[0];
         select[0]=select[1];
         select[1]=select[2];
         select[2]=temp;
      }
      delay_10ms(200);
   }
   
}


