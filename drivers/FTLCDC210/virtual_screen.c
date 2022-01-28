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
 * Gobal variables
 *****************************************************************************/

extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern LCDMTYPE_T FLcdModule[];
extern unsigned int LCD_PANEL_TYPE;
extern char FILENAME[];
extern char DEFAULT_PATH[];

/******************************************************************************
 *Public functions
 *****************************************************************************/

void Virtual_Screen_Off()
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x1500) =0; 
   *(volatile unsigned int *) (LCD_IO_Base + 0x1504) =0; 
   *(volatile unsigned int *) (LCD_IO_Base + 0x1508) =0; 
   *(volatile unsigned int *) (LCD_IO_Base + 0x150c) =0;       
}


void move_buffer(unsigned char *fb0,unsigned int offset,unsigned int bbp, unsigned int line)
{
   unsigned char i;  
     
/*     
   for (i = 0; i<32 ; i++) {  
      *(volatile unsigned int *) (LCD_IO_Base + 0x1500) = (1<<20) | (i<<12) | line;    
     delay_10ms(5);
   }
   for (i = 32; i>0 ; i--) {  
      *(volatile unsigned int *) (LCD_IO_Base + 0x1500) = (1<<20) | ((i-1)<<12) | line;    
     delay_10ms(5);
   }
*/   
   // unknown usage   
   //*(volatile unsigned int *) (LCD_IO_Base + 0x1500) = (1<<20) | ((31)<<12) | line; 
   //delay_10ms(300);
                   
     
   for (i = 0; i < offset; i++) {  //right
      SetFrameBase(0, 0, (unsigned char *)(fb0 + i*bbp));
     delay_10ms(5);
   }
   for (i = offset; i> 0 ; i--) {  //left
      SetFrameBase(0, 0, (unsigned char *)(fb0 + (i-1)*bbp));
     delay_10ms(5);
   }   
   for (i = 0; i < offset; i++) {  //down
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*i ));
     delay_10ms(5);
   }
   for (i = 0; i < offset; i++) {  //right
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*(offset-1) + i*bbp ));
     delay_10ms(5);
   }
   for (i = offset; i> 0 ; i--) { //up
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*(i-1) + (offset-1)*bbp));
     delay_10ms(5);
   } 
   for (i = 0; i < offset ; i++) {  //down
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*i + (offset-1)*bbp));
     delay_10ms(5);
   } 
   for (i = offset; i> 0 ; i--) {  //left
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*(offset-1) + (i-1)*bbp));
     delay_10ms(5);
   } 
   for (i = offset; i > 0 ; i--) {  //up
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*(i-1)));
     delay_10ms(5);
   }
   
   // dialog
   for (i = 0; i < offset ; i++) {  
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*i + i*bbp ));
     delay_10ms(5);
   }      
   for (i = offset; i >0 ; i--) {  
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*bbp*i + i*bbp ));
     delay_10ms(5);
   } 
         
   delay_10ms(300);         
   //printf("Check Virtual Screen Test OK!, press 'y'\n");
   //do {
   //   result = getchar();
   //}while( (result != 'y') );
}

void move_buffer_420(unsigned char *fb0,unsigned int offset,unsigned int bbp, unsigned int line,unsigned int high)
{
   short i;
   unsigned char *fb1 = fb0 + line * high;
   unsigned char *fb2 = fb0 + line * high + line * high/4;     
      
   for (i = 0; i <= offset; i+=16) {  //right
      SetFrameBase(0, 0, (unsigned char *)(fb0 + i));
      SetFrameBase(1, 0, (unsigned char *)(fb1 + i*2/bbp));
      SetFrameBase(2, 0, (unsigned char *)(fb2 + i*2/bbp));            
     delay_10ms(50);
   }
   for (i = offset; i>=0 ; i-=16) {  //left
      SetFrameBase(0, 0, (unsigned char *)(fb0 + i));
      SetFrameBase(1, 0, (unsigned char *)(fb1 + i*2/bbp));
      SetFrameBase(2, 0, (unsigned char *)(fb2 + i*2/bbp));            
     delay_10ms(50);
   }   
   for (i = 0; i <= offset; i+=16) {  //down
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*i ));
     SetFrameBase(1, 0, (unsigned char *)(fb1 + line*i/bbp ));
     SetFrameBase(2, 0, (unsigned char *)(fb2 + line*i/bbp ));      
     delay_10ms(50);
   }
   for (i = 0; i <= offset; i+=16) {  //right
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*offset + i ));
     SetFrameBase(1, 0, (unsigned char *)(fb1 + line*offset/bbp + i*2/bbp ));
     SetFrameBase(2, 0, (unsigned char *)(fb2 + line*offset/bbp + i*2/bbp ));      
     delay_10ms(50);
   }
   for (i = offset; i>= 0 ; i-=16) { //up
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*i + offset));
      SetFrameBase(1, 0, (unsigned char *)(fb1 + line*i/bbp + offset*2/bbp));
      SetFrameBase(2, 0, (unsigned char *)(fb2 + line*i/bbp + offset*2/bbp));      
     delay_10ms(50);
   } 
   for (i = 0; i <= offset ; i+=16) {  //down
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*i + offset));
      SetFrameBase(1, 0, (unsigned char *)(fb1 + line*i/bbp + offset*2/bbp));
      SetFrameBase(2, 0, (unsigned char *)(fb2 + line*i/bbp + offset*2/bbp));            
     delay_10ms(50);
   } 
   for (i = offset; i>= 0 ; i-=16) {  //left
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*offset + i));
      SetFrameBase(1, 0, (unsigned char *)(fb1 + line*offset/bbp + i*2/bbp));
      SetFrameBase(2, 0, (unsigned char *)(fb2 + line*offset/bbp + i*2/bbp));            
     delay_10ms(50);
   } 
   for (i = offset; i >= 0 ; i-=16) {  //up
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*i));
     SetFrameBase(1, 0, (unsigned char *)(fb1 + line*i/bbp));
     SetFrameBase(2, 0, (unsigned char *)(fb2 + line*i/bbp));       
     delay_10ms(50);
   }
   
   // dialog
   for (i = 0; i <= offset ; i+=16) {  
      SetFrameBase(0, 0, (unsigned char *)(fb0 + line*i + i ));
      SetFrameBase(1, 0, (unsigned char *)(fb1 + line*i/bbp + i*2/bbp ));
      SetFrameBase(2, 0, (unsigned char *)(fb2 + line*i/bbp + i*2/bbp ));            
     delay_10ms(50);
   }      
   for (i = offset; i >=0 ; i-=16) {  
     SetFrameBase(0, 0, (unsigned char *)(fb0 + line*i + i));
     SetFrameBase(1, 0, (unsigned char *)(fb1 + line*i/bbp + i*2/bbp ));
     SetFrameBase(2, 0, (unsigned char *)(fb2 + line*i/bbp + i*2/bbp ));        
     delay_10ms(50);
   } 
         
   delay_10ms(300);       
}
  
void Virtual_Screen_Test(unsigned char *pBuffer, unsigned char bpp)
{
   unsigned char *fb0;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
   strcpy(FILENAME, DEFAULT_PATH);
   
   fb0 = pBuffer;
   
   if(bpp == 16){     
      // enable virtual screen before loading
      *(volatile unsigned int *) (LCD_IO_Base + 0x1500) = (1<<20) | (720);   
      strcat(FILENAME, "10_565_720x576.bmp.bin");
     Load_Image( FILENAME, 829440, pBuffer, LBLP, bpp );
      move_buffer(pBuffer,150,2,720);    
   }
   else if (bpp == 24){
      // enable virtual screen before loading   
      *(volatile unsigned int *) (LCD_IO_Base + 0x1500) = (1<<20) | (720);   
     strcat(FILENAME, "38_888_720x576.bmp.bin");
     Load_Image( FILENAME, 1658880, pBuffer, LBLP, bpp );
      move_buffer(pBuffer,150,4,720);    
   }
   SetFrameBase(0, 0, (unsigned char *)(fb0));      
   Virtual_Screen_Off();
}

void Virtual_Screen_YUV_Test(unsigned char *pBuffer, unsigned char bpp)
{
   unsigned char *fb0;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
   strcpy(FILENAME, DEFAULT_PATH);
   
   fb0 = pBuffer;
   
   ///////////////////////////////////////////////////////////////
   //YCbCr420
   *(volatile unsigned int *) (LCD_IO_Base + 0x1500) = (1<<20) | (720);  
   *(volatile unsigned int *) (LCD_IO_Base + 0x1504) = (1<<20) | (720); 
   *(volatile unsigned int *) (LCD_IO_Base + 0x1508) = (1<<20) | (720);        
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "7_420_720x576.bmp.yuv");
   Load_Image_YCbCr420( FILENAME, 720, 576, 622080, pBuffer, LBLP );
   delay_10ms(300);
   move_buffer_420(pBuffer,96,4,720,576);
   SetFrameBase(0, 0, (unsigned char *)(fb0));    
   Virtual_Screen_Off();   

   //////////////////////////////////////////////////////////////
   // YCbCr422
   *(volatile unsigned int *) (LCD_IO_Base + 0x1500) = (1<<20) | (720);   
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "45_422_720x480.bmp.yuv");
   Load_Image_YCbCr422(FILENAME, 691200, pBuffer, LBLP);
   move_buffer(pBuffer,150,2,720);
   
   SetFrameBase(0, 0, (unsigned char *)(fb0));    
   Virtual_Screen_Off();            
}

void PoP_Virtual_Screen_Test(unsigned char *pFrameBuffer, unsigned char bpp) // spade added
{
   unsigned char *fb0, *fb1, *fb2, *fb3;
   unsigned char *bfb0, *bfb1, *bfb2, *bfb3;
   int i, k;
   unsigned char endian[18]="";

   OSD_Off();

   bfb0 = fb0 = pFrameBuffer;
   bfb1 = fb1 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 24, 1);
   bfb2 = fb2 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 24, 1);
   bfb3 = fb3 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 24, 1);

   //initialize frame buffer first
   SetFrameBase(0,0, fb0);
   SetFrameBase(1,0, fb1);
   SetFrameBase(2,0, fb2);
   SetFrameBase(3,0, fb3);

   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);

   PoP_ImageScalDown(0, 0, 0, 0);   
   PoP_On();

   i = LBLP;   //set the current endian to LBLP
   //  SetBGRSW( BGR );

   if( bpp == 24 ) {
      if ( ( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 ) ) {
         for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {     
              for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB888 %s in PoP\n", endian);

            *(volatile unsigned int *)(LCD_IO_Base + 0x1500) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x1504) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x1508) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x150C) = (1 << 20) | (640);
            
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "17_888_640x480.bmp.bin");
            Load_Image( FILENAME, 1228800, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "lady888_640x480.bmp.bin");
            Load_Image( FILENAME, 1228800, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_888_640x480.bin");
            Load_Image( FILENAME, 1228800, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_888_640x480.bin");
            Load_Image( FILENAME, 1228800, fb3, i, bpp );

              delay_10ms(300);            
            
              for (k = 0; k < 150 ; k++) {  
                 SetFrameBase(0, 0, (unsigned char *)(fb0 + 640*k*4 + k*4 ));
                 SetFrameBase(1, 0, (unsigned char *)(fb1 + 640*k*4 + k*4 ));
                 SetFrameBase(2, 0, (unsigned char *)(fb2 + 640*k*4 + k*4 ));
                 SetFrameBase(3, 0, (unsigned char *)(fb3 + 640*k*4 + k*4 ));                                     
                 delay_10ms(5);
              }      
              for (k = 150; k >0 ;k--) {  
                SetFrameBase(0, 0, (unsigned char *)(fb0 + 640*(k-1)*4 + (k-1)*4 ));
                SetFrameBase(1, 0, (unsigned char *)(fb1 + 640*(k-1)*4 + (k-1)*4 ));
                SetFrameBase(2, 0, (unsigned char *)(fb2 + 640*(k-1)*4 + (k-1)*4 ));
                 SetFrameBase(3, 0, (unsigned char *)(fb3 + 640*(k-1)*4 + (k-1)*4 ));  
                 delay_10ms(5);
              }
            delay_10ms(300);
            
            SetFrameBase(0, 0, (unsigned char *)(fb0));
            SetFrameBase(1, 0, (unsigned char *)(fb1));
            SetFrameBase(2, 0, (unsigned char *)(fb2));
            SetFrameBase(3, 0, (unsigned char *)(fb3));
              Virtual_Screen_Off(); 
                
            //delay_10ms(300);                  

              //              if( i == LBBP )   continue;
            
            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }
      else if( ( PANEL_WIDTH == 320 ) && ( PANEL_HEIGHT == 240 ) ) {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test RGB888 %s in PoP\n", endian);

            /*      
                    strcpy(FILENAME, DEFAULT_PATH);
                  strcat(FILENAME, "20_888_160x120.bin");
                  Load_Image( FILENAME, 76800, fb0, i, bpp );
            */              strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_320x240.bin");
            Load_Image( FILENAME, 307200, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "lady888_160x120.bin");
            Load_Image( FILENAME, 76800, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_888_160x120.bin");
            Load_Image( FILENAME, 76800, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_888_160x120.bin");
            Load_Image( FILENAME, 76800, fb3, i, bpp );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }
   }
   else if( bpp == 16 ) {
      //OSD Test
      //OSD_Test("BIG OSD OK");     //have to define COMPLEX_OSD

      if( ( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 ) )
      {
#ifdef SIMPLE_POP_TEST
         SetFrameBase(0,0,fb0);
         SetFrameBase(1,0,fb0);
         SetFrameBase(2,0,fb0);
         SetFrameBase(3,0,fb0);
         
         strcpy(FILENAME, DEFAULT_PATH);
         strcat(FILENAME, "20_565_640x480.bin");
         Load_Image( FILENAME, 614400, fb0, i, bpp );

         *(volatile unsigned int *) (LCD_IO_Base + 0x0000 )|= (0x1 << 7);
         *(volatile unsigned int *) (LCD_IO_Base + 0x00014) = (0x5500 << 0);
         enable_lcd_controller();
#endif
         for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {        
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            /******** RGB565 test ************/
            getEndianString(i, endian);
            printf("Test RGB565 %s in PoP\n", endian);

            *(volatile unsigned int *)(LCD_IO_Base + 0x1500) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x1504) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x1508) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x150C) = (1 << 20) | (640);
            
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_565_640x480.bin");
            Load_Image( FILENAME, 614400, fb0, i, bpp ); 

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_565_640x480.bin");
            Load_Image( FILENAME, 614400, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "63_565_640x480.bmp.bin");
            Load_Image( FILENAME, 614400, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "76_565_640x480.bmp.bin");
            Load_Image( FILENAME, 614400, fb3, i, bpp );

                delay_10ms(300);   

                for (k = 0; k < 150 ; k++) {  
                   SetFrameBase(0, 0, (unsigned char *)(fb0 + 640*2*k + k*2 ));
                   SetFrameBase(1, 0, (unsigned char *)(fb1 + 640*4*k + k*4 ));
                   SetFrameBase(2, 0, (unsigned char *)(fb2 + 640*6*k + k*6 ));
                   //SetFrameBase(3, 0, (unsigned char *)(fb3 + 640*2*k + k*2 )); 
                   SetFrameBase(3, 0, (unsigned char *)(fb3 + k*4 ));                                    
                   delay_10ms(5);
                }      
                for (k = 150; k >0 ;k--) {  
                  SetFrameBase(0, 0, (unsigned char *)(fb0 + 640*2*(k-1) + (k-1)*2 ));
                  SetFrameBase(1, 0, (unsigned char *)(fb1 + 640*4*(k-1) + (k-1)*4 ));
                  SetFrameBase(2, 0, (unsigned char *)(fb2 + 640*6*(k-1) + (k-1)*6 ));
                   //SetFrameBase(3, 0, (unsigned char *)(fb3 + 640*2*(k-1) + (k-1)*2 )); 
                   SetFrameBase(3, 0, (unsigned char *)(fb3 + k*4 ));                             
                   delay_10ms(5);
                }

            delay_10ms(300);
            
            SetFrameBase(0, 0, (unsigned char *)(fb0));
            SetFrameBase(1, 0, (unsigned char *)(fb1));
            SetFrameBase(2, 0, (unsigned char *)(fb2));
            SetFrameBase(3, 0, (unsigned char *)(fb3));
                Virtual_Screen_Off();  
                
            //delay_10ms(300);                                       
                
            /********* This is for YCbCr422 test *********/
               Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
               
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 422, i, bpp );
            printf("Test YCbCr422 %s in PoP\n", endian);

            *(volatile unsigned int *)(LCD_IO_Base + 0x1500) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x1504) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x1508) = (1 << 20) | (640);
            *(volatile unsigned int *)(LCD_IO_Base + 0x150C) = (1 << 20) | (640);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, fb0, i );   

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, fb1, i );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, fb2, i );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "34_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, fb3, i );
                
            delay_10ms(300);           
      
            for (k = 0; k < 200; k++) {
               SetFrameBase(0, 0, (unsigned char *)(fb0 + 640*k*2) );
               SetFrameBase(1, 0, (unsigned char *)(fb1 + (k*2)) );
               SetFrameBase(2, 0, (unsigned char *)(fb2 + 640*k*2) );
               SetFrameBase(3, 0, (unsigned char *)(fb3 + (k*2)) );
               delay_10ms(5);
            }
            for (k = 200; k >0; k--) {
               SetFrameBase(0, 0, (unsigned char *)(fb0 + 640*(k-1)*2) );
               SetFrameBase(1, 0, (unsigned char *)(fb1 + ((k-1)*2)) );
               SetFrameBase(2, 0, (unsigned char *)(fb2 + 640*(k-1)*2) );
               SetFrameBase(3, 0, (unsigned char *)(fb3 + ((k-1)*2)) );
               delay_10ms(5);
            }           
            delay_10ms(300);
            
            SetFrameBase(0, 0, (unsigned char *)(fb0));
            SetFrameBase(1, 0, (unsigned char *)(fb1));
            SetFrameBase(2, 0, (unsigned char *)(fb2));
            SetFrameBase(3, 0, (unsigned char *)(fb3));
                Virtual_Screen_Off();           

            //delay_10ms(300);
            
            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }else if( ( PANEL_WIDTH == 320 ) && ( PANEL_HEIGHT == 240 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test RGB565 %s in PoP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_565_320x240.bin");
            Load_Image( FILENAME, 153600, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_565_160x120.bin");
            Load_Image( FILENAME, 38400, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_565_160x120.bin");
            Load_Image( FILENAME, 38400, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_565_160x120.bin");
            Load_Image( FILENAME, 38400, fb3, i, bpp );

            /******************************* This is for different input type ******************************************/

            PiP_PoP_Image_Format1( 0, 565, i, 16 );
            PiP_PoP_Image_Format1( 1, 888, i, 24 );
            PiP_PoP_Image_Format1( 2, 422, i, 16 );
            PiP_PoP_Image_Format1( 3, 565, i, 16 );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test different input type %s in PoP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "17_565_320x240.bmp.bin");
            Load_Image( FILENAME, 153600, fb0, i, 16 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "18_888_160x120.bmp.bin");
            Load_Image( FILENAME, 76800, fb1, i, 24 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_422_160x120.bmp.yuv");
            Load_Image( FILENAME, 38400, fb2, i, 16 );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "1_565_160x120.bmp.bin");
            Load_Image( FILENAME, 38400, fb3, i, 16 );

            /******************************* This is for different input type ******************************************/
            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }
   }

   Cursor_Test();

   PoP_Off();
   // SetBGRSW( RGB );
   PoP_ImageScalDown(0, 0, 0, 0);   
   // SetDivNo(DIVISOR);
   /*free(fb0);*/ 
   free(fb1);  
   free(fb2);  
   free(fb3);
}

