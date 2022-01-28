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

/******************************************************************************
 * Define Constants
 *****************************************************************************/
extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern LCDMTYPE_T FLcdModule[];
extern unsigned int LCD_PANEL_TYPE;
extern char DEFAULT_PATH[];
extern char FILENAME[];

/***************************** PoP Test ***********************************/
void PoP_On()
{
   *(volatile unsigned int *) LCD_IO_Base &= 0xfffffffe;
   //    *(unsigned int *) LCD_IO_Base &= 0xffffff7f; //PoP off
   *(volatile unsigned int *) LCD_IO_Base |= 0x00000080; //PoP on
   *(volatile unsigned int *) LCD_IO_Base |= 0x1;
}

void PoP_Off()
{
   *(volatile unsigned int *) LCD_IO_Base &= 0xffffff7f;
}

//Image scaling down can be used in not only PoP but also any other situation
void PoP_ImageScalDown( unsigned int im0, unsigned int im1, unsigned int im2, unsigned int im3 )
{
   //#ifdef OLD_CCIR656
#if 1
   if( im0 > 2 || im1 > 2 || im2 > 2 || im3 > 2 )
      printf("PoP windows enable error\n");

   *(volatile unsigned int *) (LCD_IO_Base + 0x0014) &= 0xffff00ff;
   *(volatile unsigned int *) (LCD_IO_Base + 0x0014) |= (im0 << 8) | (im1 << 10) | (im2 << 12) | (im3 << 14);
#endif
}

//void animation_test( char *pFrameBuffer, char *file, int size , int LCD_IRQ);
void PoP_Test( unsigned char *pFrameBuffer, unsigned short bpp )
{
   unsigned char *fb0, *fb1, *fb2, *fb3;
   int i, k;
   char endian[] = "    ";

   OSD_Off();

   fb0 = pFrameBuffer;
   fb1 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 8, 4);
   fb2 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 8, 4);
   fb3 = allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, 8, 4);

   setNumFrameBuffer(1, 1, 1, 1);

   //initialize frame buffer first
   SetFrameBase(0,0, fb0);
   SetFrameBase(1,0, fb1);
   SetFrameBase(2,0, fb2);
   SetFrameBase(3,0, fb3);

   //if you want to enable animation test, open this
   /*  printf("Please input the picture directory, [type '!' to use default path, default path %s]:\n", DEFAULT_PATH);
       scanf("%s", FILENAME);
       if( FILENAME[0] != '!' )  strcpy( DEFAULT_PATH, FILENAME);
    */

   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
   PoP_On();
   // SetDivNo(DIVISOR);   //FIFO under run still occurs

   for(k=0; k<4; k++)
      PiP_PoP_Image_Format1( k, 565, LBLP, bpp );

   //action here
   if(LCD_PANEL_TYPE < 5 || LCD_PANEL_TYPE > 8)
      clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
   else
      clear_Screen(fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
   clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
   clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
   clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);

   PoP_ImageScalDown(0, 0, 0, 0);   

   printf("Test YCbCr422 in PoP\n");
   for(k=0; k<4; k++)
      PiP_PoP_Image_Format1( k, 422, BBBP, 16 );
   if(LCD_PANEL_TYPE < 5 || LCD_PANEL_TYPE > 8)
      ColorBar_YCbCr_422( BBBP, fb0, PANEL_WIDTH, PANEL_HEIGHT, 0);
   else
      ColorBar_YCbCr_422( BBBP, fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   /*
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "16_320x240.yuv");
      Load_Image( FILENAME, 307200, fb0, LBLP, bpp );
    */
    
   ColorBar_YCbCr_422( BBBP, fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( BBBP, fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( BBBP, fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);

   for(k=0; k<4; k++)
      PiP_PoP_Image_Format1( k, 422, LBBP, 16 );
   if(LCD_PANEL_TYPE < 5 || LCD_PANEL_TYPE > 8)
      ColorBar_YCbCr_422( LBBP, fb0, PANEL_WIDTH, PANEL_HEIGHT, 0);
   else
      ColorBar_YCbCr_422( LBBP, fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( LBBP, fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( LBBP, fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( LBBP, fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);

   for(k=0; k<4; k++)
      PiP_PoP_Image_Format1( k, 422, LBLP, 16 );
   if(LCD_PANEL_TYPE < 5 || LCD_PANEL_TYPE > 8)
      ColorBar_YCbCr_422( LBLP, fb0, PANEL_WIDTH, PANEL_HEIGHT, 0);
   else
      ColorBar_YCbCr_422( LBLP, fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( LBLP, fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( LBLP, fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);
   ColorBar_YCbCr_422( LBLP, fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, 0);

   // if you want to test it, remember to open this test
   /*  strcpy(FILENAME, DEFAULT_PATH);
       strcat(FILENAME, "mov3224.bin");
       animation_test( fb0, FILENAME,  1536000 );
       animation_test( fb1, FILENAME,  1536000 );
       animation_test( fb2, FILENAME,  1536000 );
       animation_test( fb3, FILENAME,  1536000 );
    */

   //for recovery the setting
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
   //#ifdef OLD_CCIR656
   PoP_ImageScalDown(0, 0, 0, 0);   
   //#else
   // printf("im0 in PoP will show with errors because new type of CCIR656 does not support image scaling down\n");
   //#endif

   PoP_On();

   //OSD Test
   OSD_Test("BIG OSD OK");    //have to define COMPLEX_OSD
      
   i = LBLP;   //set the current endian to LBLP
   //  SetBGRSW( BGR );
         
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
            printf("Test RGB888 %s in PoP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "41_888_720x576.bmp.bin");
            Load_Image( FILENAME, 1658880, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_360x288.bmp.bin");
            Load_Image( FILENAME, 414720, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "12_888_360x288.bmp.bin");
            Load_Image( FILENAME, 414720, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "13_888_360x288.bmp.bin");
            Load_Image( FILENAME, 414720, fb3, i, bpp );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }else if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 480 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB888 %s in PoP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "41_888_720x480.bmp.bin");
            Load_Image( FILENAME, 1382400, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_888_360x240.bmp.bin");
            Load_Image( FILENAME, 345600, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "12_888_360x240.bmp.bin");
            Load_Image( FILENAME, 345600, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "14_888_360x240.bmp.bin");
            Load_Image( FILENAME, 345600, fb3, i, bpp );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }else if( ( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 ) )
      {
         for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB888 %s in PoP\n", endian);

               PoP_ImageScalDown(1, 0, 1, 1);   
               
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "17_888_640x480.bmp.bin");
            Load_Image( FILENAME, 1228800, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "lady888_320x240.bin");
            Load_Image( FILENAME, 307200, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_888_640x480.bin");
            Load_Image( FILENAME, 1228800, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_888_640x480.bin");
            Load_Image( FILENAME, 1228800, fb3, i, bpp );

            //if( i == LBBP ) continue;

            delay_10ms(100);

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
               PiP_PoP_Image_Format1( k, 888, i, bpp );

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test RGB888 %s in PoP\n", endian);

            /*              strcpy(FILENAME, DEFAULT_PATH);
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
   else if( bpp == 16 ) 
   {
      if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 576 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB565 %s in PoP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "43_565_720x576.bmp.bin");
            Load_Image( FILENAME, 829440, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "13_565_360x288.bmp.bin");
            Load_Image( FILENAME, 207360, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "14_565_360x288.bmp.bin");
            Load_Image( FILENAME, 207360, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_565_360x288.bmp.bin");
            Load_Image( FILENAME, 207360, fb3, i, bpp );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }else if( ( PANEL_WIDTH == 720 ) && ( PANEL_HEIGHT == 480 ) )
      {
         //          for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB565 %s in PoP\n", endian);

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "41_565_720x480.bmp.bin");
            Load_Image( FILENAME, 691200, fb0, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "13_565_360x240.bmp.bin");
            Load_Image( FILENAME, 172800, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_565_360x240.bmp.bin");
            Load_Image( FILENAME, 172800, fb2, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "14_565_360x240.bmp.bin");
            Load_Image( FILENAME, 172800, fb3, i, bpp );

            //              if( i == LBBP )  continue;

            clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
         }
      }else if( ( PANEL_WIDTH == 640 ) && ( PANEL_HEIGHT == 480 ) )
      {
#ifdef SIMPLE_POP_TEST
         SetFrameBase(0,0,fb0);
         SetFrameBase(1,0,fb0);
         SetFrameBase(2,0,fb0);
         SetFrameBase(3,0,fb0);

         strcpy(FILENAME, DEFAULT_PATH);
         strcat(FILENAME, "20_565_640x480.bin");
         Load_Image( FILENAME, 614400, fb0, i, bpp );

         *(volatile  unsigned int *) (LCD_IO_Base + 0x0000 )|= (0x1 << 7);
         *(volatile  unsigned int *) (LCD_IO_Base + 0x00014) = (0x5500 << 0);
         enable_lcd_controller();
#endif
         for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
             printf("...%d/%d....\n",i,LBBP);
                     
            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 565, i, bpp );

            //test LBLP, BBBP, and LBBP 
            getEndianString(i, endian);
            printf("Test RGB565 %s in PoP\n", endian);

               PoP_ImageScalDown(1, 1, 2, 0);                     
   
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_565_640x480.bin");
            Load_Image( FILENAME, 614400, fb0, i, bpp ); //for CCIR656, only image scaling down 1/2 x 1 is supported

            //strcpy(FILENAME, DEFAULT_PATH);
            //strcat(FILENAME, "16_565_640x480.bin");
            //Load_Image( FILENAME, 614400, fb0, i, bpp );
            
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_565_640x480.bin");
            Load_Image( FILENAME, 614400, fb1, i, bpp );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_565_640x480.bin");
            Load_Image( FILENAME, 614400, fb2, i, bpp );
            
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_565_320x240.bin");
            Load_Image( FILENAME, 153600, fb3, i, bpp );

            delay_10ms(500);
                        
            //clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            //clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            //clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            //clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);           

            /******************************* This is for YCbCr422 test ****************************************************/

            for(k=0; k<4; k++)
               PiP_PoP_Image_Format1( k, 422, i, bpp );
            printf("Test YCbCr422 %s in PoP\n", endian);

               PoP_ImageScalDown(1, 0, 0, 2);   
               
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, fb0, i );   

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, fb1, i );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_422_320x240.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 153600, fb2, i );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_422_640x480.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 614400, fb3, i );   
            
            //strcpy(FILENAME, DEFAULT_PATH);
            //strcat(FILENAME, "16_422_320x240.bmp.yuv");
            //Load_Image_YCbCr422( FILENAME, 153600, fb3, i );

            delay_10ms(500);

            /******************************* This is for different input type ******************************************/
            //              if( i == LBBP )  continue;

            //clear_Screen(fb0, PANEL_WIDTH, PANEL_HEIGHT, bpp, 0x0);
            //clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
            //clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
            //clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
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
   PoP_ImageScalDown(0, 0, 0, 0);   //all four window are scaling down to 1/4
   // SetDivNo(DIVISOR);
   /*free(fb0);*/ 
   free(fb1);  
   free(fb2);  
   free(fb3);
}
