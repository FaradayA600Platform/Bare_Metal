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
//#include "fLib.h"
#include "ftlcd210_conf.h"
#include "ftlcd210-OSD.h"

/******************************************************************************
 * Gobal variables
 *****************************************************************************/

extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern LCDMTYPE_T FLcdModule[];
extern unsigned int LCD_PANEL_TYPE;
extern char DEFAULT_PATH[];
extern char FILENAME[];

/******************************************************************************
 *Public functions
 *****************************************************************************/

//choose the 16 grey levels from 32 grey levels
void Set_Graylevel( unsigned char *pFrameBuffer, unsigned int level, unsigned int r, unsigned int g, unsigned int b )
{
   if( level < 16 && r < 32 && g < 32 && b < 32 )
   {
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E00 + level * 4) = r;
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E40 + level * 4) = g;
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E80 + level * 4) = b;
   }
}

void concate_18bit(unsigned int width, unsigned int height, unsigned char *pFrameBuffer, unsigned char endian, unsigned int bpp)
{
   int i;
   char *buf;
   buf = allocateFrameBuffer( width*2, height*2, 16, 1 );
   printf("translation buf=0x%x\n", (unsigned int)(uintptr_t)buf);
   memset(buf, 0, width*height*4);
   memcpy(buf, pFrameBuffer, width*height*(16/8));

   for(i=0; i<width*height*18/(8*4); i++){      //unit is word
      if(i%9 == 0){
         *(unsigned int *)(pFrameBuffer + 4*(i+0)) = (*(unsigned int *)(buf + 4*(i+0))) | ((*(unsigned int *)(buf + 4*(i+1))) << 18);
         *(unsigned int *)(pFrameBuffer + 4*(i+1)) = ((*(unsigned int *)(buf + 4*(i+1))) >> 14) | ((*(unsigned int *)(buf + 4*(i+2))) << 4) | ((*(unsigned int *)(buf + 4*(i+3))) << 22);
         *(unsigned int *)(pFrameBuffer + 4*(i+2)) = ((*(unsigned int *)(buf + 4*(i+3))) >> 10) | ((*(unsigned int *)(buf + 4*(i+4))) << 8) | ((*(unsigned int *)(buf + 4*(i+5))) << 26);
         *(unsigned int *)(pFrameBuffer + 4*(i+3)) = ((*(unsigned int *)(buf + 4*(i+5))) >> 6) | ((*(unsigned int *)(buf + 4*(i+6))) << 12) | ((*(unsigned int *)(buf + 4*(i+7))) << 30); 
         *(unsigned int *)(pFrameBuffer + 4*(i+4)) = ((*(unsigned int *)(buf + 4*(i+7))) >> 2) | ((*(unsigned int *)(buf + 4*(i+8))) << 16);
         *(unsigned int *)(pFrameBuffer + 4*(i+5)) = ((*(unsigned int *)(buf + 4*(i+8))) >> 16) | ((*(unsigned int *)(buf + 4*(i+9))) << 2 ) | ((*(unsigned int *)(buf + 4*(i+10))) << 20);
         *(unsigned int *)(pFrameBuffer + 4*(i+6)) = ((*(unsigned int *)(buf + 4*(i+10))) >> 12) | ((*(unsigned int *)(buf + 4*(i+11))) << 6) |((*(unsigned int *)(buf + 4*(i+12))) << 24);
         *(unsigned int *)(pFrameBuffer + 4*(i+7)) = ((*(unsigned int *)(buf + 4*(i+12))) >> 8) | ((*(unsigned int *)(buf + 4*(i+13))) << 10) | ((*(unsigned int *)(buf + 4*(i+14))) << 28);
         *(unsigned int *)(pFrameBuffer + 4*(i+8)) = ((*(unsigned int *)(buf + 4*(i+14))) >> 4) | ((*(unsigned int *)(buf + 4*(i+15))) << 14);
      }
   }
   free(buf);
}

void fill_color( unsigned char *pFrameBuffer, int bpp, unsigned int color )
{
   char result;
   int i;

   SetBPP( bpp );

   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = color;

   reset_lcd_controller();

   printf("Check next pattern, press 'y'\n");
   do{
      result = getchar();
   }while( (result != 'y') );
}


void fill_background( unsigned char *pFrameBuffer, int bpp )
{
   int i;
   unsigned int bg, color_value;

   do{
      printf("(1) Red, (2) Green, (3) Blue\n\n");
      getchar();
      scanf( "%d", &i );
   }while( i==0 || i>3 );

   if( i == 1 ) color_value = 0x10001000;
   else if( i == 2 ) color_value = 0x00800080;
   else if( i == 3 ) color_value = 0x00020002;

   do{
      printf("level: 0 ~ 15\n\n");
      getchar();
      scanf( "%d", &i );
   }while( i>15 );

   bg = color_value * i;
   fill_color( pFrameBuffer, bpp, bg );
}


void SetVirtualWidth( unsigned int cstn_vir_width )
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) &= 0xff00ffff;  
   *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) |= ( cstn_vir_width / 16 ) << 16; 
}

//currently supports 16bpp, with 320x240 being the suitable size
void Scroll_Test( unsigned char *pFrameBuffer, char *file, int img_width, int img_height, int size, int bpp )
{
   int i;      
   //    #define IMG_WIDTH 320
   //    #define IMG_HEIGHT   240


   //test the scroll function of CSTN
   disable_lcd_controller();
   SetBPP(bpp);
   if( bpp == 16 )
      printf("RGB565 LBLP Test\n");
   else if( bpp == 4 )
      printf("Grey color 4bpp Test\n");
   //    *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) &= 0xff00ffff;
   //    *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) |= ( img_width / 16 ) << 16;   //set the virtual window width( can be actual image widty )
   SetVirtualWidth( img_width );
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, file);
   Load_Image( FILENAME, size, pFrameBuffer, LBLP, bpp );
   enable_lcd_controller();
   for( i=0; i<10000000; i++ );     

   //horizontal scroll
   for( i = 0; i < ( img_width - PANEL_WIDTH ) * bpp/8 / 4; i++ )
   {
      int j;

      for( j=0; j<(32/bpp); j++ ){
         int k;
         *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) &= 0xffffffe0;
         *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) |= j * bpp;
         for( k=0; k<300000; k++ );
      }

      *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) &= 0xffffffe0;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0018) += 4;
      for( j=0; j<300000; j++ );

   }

   //vertical scroll
   for( i = 0; i < ( img_height - PANEL_HEIGHT ) ; i++ )
   {
      int j;

      if( *(unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0018) > (unsigned int )(uintptr_t)pFrameBuffer + (img_width*(img_height - PANEL_HEIGHT))* bpp/8)
         break;

      for( j=0; j<(32/bpp); j++ ){
         *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) &= 0xffffffe0;
         *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) |= j * bpp;
         for( j=0; j<500000; j++ );
      }

      *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) &= 0xffffffe0;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0018) += img_width * bpp/8;
      for( j=0; j<500000; j++ );

   }
   SetVirtualWidth( PANEL_WIDTH );
   SetFrameBase(0, 0, pFrameBuffer);
}


void Draw_color_block( unsigned char *pFrameBuffer, int bpp, unsigned int *color, unsigned int vbar, unsigned int hbar )
{
   int i, j, k, line_no=0; //bar: vertival bar number

   for( i=0; i<PANEL_HEIGHT; i++ )
   {
      k = 1;
      j = 0;
      //      val = 0x0;
      //       bar = 8; //the total bars on the screen

      for( k=1; k<=vbar; k++ ){
         for( ; j<PANEL_WIDTH*bpp/8*k/vbar; j+=4){    //16 means 16 vertical bars, k means ?th bar
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = color[(k-1) + line_no];// * (/*16/bar**/(k-1));
         }
      }

      if( (i != 0) && (i % (PANEL_HEIGHT/hbar/*bar/2*/) == 0) ){
         line_no +=vbar;   //val+=0x44444444;
      }
   }
}

//Test pattern 4, draw a retangle with the same horizontal/vertical width as the length to the edge of the panel
//fill frame buffer values to 0xffffffff, ie. clear the screen
void central_square( unsigned char *pFrameBuffer, int bpp, unsigned int color )
{
   char result;
   int i, j;

   // bpp=16;
   SetBPP( bpp );

   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = color;     

   for( i=0; i<PANEL_HEIGHT; i++ )
      for( j=0; j<PANEL_WIDTH*bpp/8; j+=4){  //4 bits per filling

         if( ( i > PANEL_HEIGHT * 1/3 ) && ( i < PANEL_HEIGHT * 2/3 ) && ( j > PANEL_WIDTH*bpp/8 *1/3 ) && ( j < PANEL_WIDTH*bpp/8 *2/3 ) )
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0x0;
      }

   reset_lcd_controller();

   printf("Check next pattern, press 'y'\n");
   do{
      result = getchar();
   }while( (result != 'y') );
}


//Test pattern 7, 水平漸層4, 8, 16灰階
void horizontal_bar( unsigned char *pFrameBuffer, int bpp, unsigned int color_pattern, int bar )
{
   char result;
   int i, j/*, bpp, bar=16*/;
   volatile unsigned int val=0x0;

   // bpp=16;
   SetBPP( bpp );

   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

   for( i=0; i<PANEL_HEIGHT; i++ )
   {
      for( j=0; j<PANEL_WIDTH*bpp/8; j+=4){
         *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = val;
      }
      if( (i != 0) && (i % (PANEL_HEIGHT/bar) == 0) )
      {  val+=/*0x10821082*/color_pattern * (16/bar);
         //    printf("%d, val = %x, i = %d, panel_height = %d\n", i % (PANEL_HEIGHT/16), val, i, PANEL_HEIGHT);
      }
   }

   // reset_lcd_controller();

   printf("Check next pattern, press 'y'\n");
   do{
      result = getchar();
   }while( (result != 'y') );
}


//Test pattern 8, 垂直漸層4, 8, 16灰階
void vertical_bar( unsigned char *pFrameBuffer, int bpp, unsigned int color_pattern, int bar, int margin )
{
   char result;
   int i, j, k;   //, /*bpp, bar=16,*/ margin=0;
   volatile unsigned int val=0x0;

   // bpp=16;
   SetBPP( bpp );

   /* if( bar == 16 )   margin = 48;
      else if( bar == 8 ) margin = 16;
      else if( bar == 4 ) margin =0;
    */      
   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

   for( i=0; i<PANEL_HEIGHT; i++ )
   {
      k = 1;
      j = 0;
      val = 0x0;

      for( k=1; k<=bar; k++ )
         //subtract 32, because each vertical bar is word-alignment
         for( ; j<(PANEL_WIDTH - margin)*bpp/8/bar*k; j+=4){      //16 means 16 vertical bars, k means ?th bar
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = val + /*0x10821082*/color_pattern * (k-1)*(16/bar);
         }

      //fill the other margin to 'black'
      for( ; j<PANEL_WIDTH*bpp/8/bar*k; j+=4){     //16 means 16 vertical bars, k means ?th bar
         *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0;
      }
   }

   // reset_lcd_controller();

   printf("Check next pattern, press 'y'\n");
   do{
      result = getchar();
   }while( (result != 'y') );
}


void CSTN_On()
{
    int i;

    *(volatile unsigned int *) (LCD_IO_Base + 0x50) |= 0x00000100;
    *(volatile unsigned int *) (LCD_IO_Base + 0x54) = 0x0000000F; //bypass rgb<->ycbcr and other, or some data will lost
    *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) |= ( 1 << 27 );
    *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) |= ( 1 << 6 ); //reverse the sequence of the default value(the STN pattern)
/* *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) = ( 1 << 27 ) | ( 3 << 25 ) | ( ( PANEL_WIDTH / 16 ) << 16 ) | ( PANEL_WIDTH * 3 - 1 );
      *(volatile unsigned int *) (LCD_IO_Base + 0x0C04) = ( 2 * PANEL_HEIGHT + 1 ) | ( 1 << 16 );
      *(volatile unsigned int *) (LCD_IO_Base + 0x0C04) &= 0xffffff7f;  //bit 7 of 0xc04 sets to 0
*/
/*    for( i=0; i<16; i++ )
      {
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E00 + i*4) = i+16;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E40 + i*4) = i+16;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E80 + i*4) = i+16;
   }
*/
    for( i=0; i<3; i++ )
    {
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E00 + i*(0x40) ) = 0x0;
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E04 + i*(0x40) ) = 0x1;
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E08 + i*(0x40) ) = 0x3;
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E0C + i*(0x40) ) = 0x5;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E10 + i*(0x40) ) = 0x7;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E14 + i*(0x40) ) = 0x9;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E18 + i*(0x40) ) = 0xb;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E1C + i*(0x40) ) = 0xd;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E20 + i*(0x40) ) = 0x11;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E24 + i*(0x40) ) = 0x13;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E28 + i*(0x40) ) = 0x14;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E2C + i*(0x40) ) = 0x17;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E30 + i*(0x40) ) = 0x18;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E34 + i*(0x40) ) = 0x1a;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E38 + i*(0x40) ) = 0x1c;
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x0E3C + i*(0x40) ) = 0x1F;
    }
}


void CSTN_Off()
{
    *(volatile unsigned int *) (LCD_IO_Base + 0x0050) &= 0xfffffeff; //GPIO, for disable the power
    *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) &= ~( 1 << 27 );
}


//If want to modify these pattern, fill 0x0C0C the value "5a5a4b4b", then modify the 0x0Fxx the modified value
void fill_CSTN_pattern()
{
/* *(volatile unsigned int *) (LCD_IO_Base + 0x0D00) = 0;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D04) = 0x00008000;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D08) = 0x00005000;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D0C) = 0x00001208;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D10) = 0x00008412;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D14) = 0x00004A24;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D18) = 0x0000219A;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D1C) = 0x0000219B;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D20) = 0x0000A1DA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D24) = 0x00006D9A;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D28) = 0x0000A5BB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D2C) = 0x000039DF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D30) = 0x00003FDB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D34) = 0x0000FBFA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D38) = 0x0000FF9F;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D3C) = 0x00007FFF;

      *(volatile unsigned int *) (LCD_IO_Base + 0x0D00) = ~0x0000FFFF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D04) = ~0x00007FFF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D08) = ~0x00007F7F;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D0C) = ~0x000057FF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D10) = ~0x0000B76F;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D14) = ~0x000007FF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D18) = ~0x0000F54B;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D1C) = ~0x000016DD;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D20) = ~0x00005555;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D24) = ~0x0000924D;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D28) = ~0x00002255;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D2C) = ~0x000002AA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D30) = ~0x00002222;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D34) = ~0x00000448;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D38) = ~0x0000A000;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D3C) = ~0x00000000;
*/
    *(volatile unsigned int *) (LCD_IO_Base + 0x0F00) = ~0xFFFFFFFF;             //0
    *(volatile unsigned int *) (LCD_IO_Base + 0x0F04) = ~0x0000FFEF; //0x0000FFDF;  //1
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F08) = ~0x000077F7;  //0xFFFFFFFE;  //2//0xEFFFFFFD;  //0xFFFFFFBF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F0C) = ~0x0000007E;  //0xFFFFFEFD;  //3//0xFFFFFFFE;  //0xFFBFFFBF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F10) = ~0xFFFFFEFD;  //0xFFFFFEFD;  //4//0xFBFBBBDD;  //0xFBFBBBBB;  //0xFBFBFBFF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F14) = ~0x3DFFFFBE;  //0xFFFFFDDD;  //5//0xF7FBF7FB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F18) = ~0x00006F77;  //0xFFFFFEDE;  //6//0xDFDFDFD5;  //0xDFDFDFDB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F1C) = ~0x00001DEE;  //0xFFFFBBDD;  //7//0xFBEBFB55;  //0xFBEBFBEB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F20) = ~0x00005DDD;  //0x3DFFFDDD;  //8//0xEEEAAAAA;  //0xEEEBEEED;  //0xEEEBEEEF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F24) = ~0x3DFFFDDD;  //0xFFFFFDB6;  //9//0xBDEDAAAA;  //0xBDEDBAAA;  //0xDEF6DEF6;  //0x77777777;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F28) = ~0x0000005D;  //0xFFFFDDB6;  //10//0x757F754A; //0x757F7577;  //0x5DDDDDDD;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F2C) = ~0x00001B6E;  //0x3DFFFED9;  //11//0x5DED5DEA; //0x5DED5DED;  //10
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F30) = ~0x00004F6D;  //0xFFFFB5AD;  //12//0x5D5D4421; //0x5D5D7D5D;
      
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F34) = ~0x00007B6D;  //0xFFFFFAD6;  //13//0xEAAAEEEA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F38) = ~0xFFFFFAD6;  //0x00005AAD;  //14//0xABAAEBAA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F3C) = ~0x00003FFA;  //0x3DFFFE99;  //15//0x55755575;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F40) = ~0x00003FF9; //16
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F44) = ~0x00005BB5; //17
//    *(volatile unsigned int *) (LCD_IO_Base + 0x0D44) = 0x55555551;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F48) = ~0x00001ECD; //18
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F4C) = ~0x0000EB35; //19
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F50) = ~0x00000699;  //20
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F54) = ~0x0000FAAA; //21
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F58) = ~0x0000002A; //22
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F5C) = ~0x00003FEA; //23
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F60) = ~0x00003FE3;  //0x00001249; //24
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F64) = ~0x00000049;  //25
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F68) = ~0xFFFFE912; //26
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F6C) = ~0x0000F912; //27
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F70) = ~0x0000E422; //28
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F74) = ~0x00000842; //29
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F78) = ~0x00000008; //30
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F7C) = ~0x00000000; //31
}


void STN_On()
{
// int i;

/* PANEL_WIDTH = PANEL_HEIGHT = 160;
    Dithering( 0, 2);   //disable dithering
    SetDivNo(2);
*/
    *(volatile unsigned int *) (LCD_IO_Base + 0x50) = 0x00000200;
    *(volatile unsigned int *) (LCD_IO_Base + 0x54) = 0x0000000F; //bypass rgb<->ycbcr and other, or some data will lost
    *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) |= ( 1 << 27 );
    *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) &= 0xffffffbf; //ensure the pattern is for STN
/* *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) = ( 1 << 27 ) | ( 2 << 25 ) | ( 1 << 24 ) | ( ( PANEL_WIDTH / 16 ) << 16 ) | ( PANEL_WIDTH - 1 );
      *(volatile unsigned int *) (LCD_IO_Base + 0x0C04) = ( 2 * PANEL_HEIGHT + 1 ) | ( 1 << 16 );  
      *(volatile unsigned int *) (LCD_IO_Base + 0x0C04) &= 0xffffff7f;  //bit 7 of 0xc04 sets to 0
*/
      //choose the grayscale level
/*    for( i=0; i<16; i++ )
      {
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E00 + i*4) = i;//+16;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E40 + i*4) = i;//+16;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E80 + i*4) = i;//+16;
   }
*/

      *(volatile unsigned int *) (LCD_IO_Base + 0x0E00 ) = 0x0;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E04 ) = 0x1;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E08 ) = 0x3;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E0C ) = 0x5;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E10 ) = 0x7;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E14 ) = 0x9;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E18 ) = 0xb;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E1C ) = 0xd;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E20 ) = 0x11;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E24 ) = 0x13;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E28 ) = 0x14;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E2C ) = 0x17;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E30 ) = 0x18;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E34 ) = 0x1a;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E38 ) = 0x1c;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E3C ) = 0x1F;
}


void STN_Off()
{
    CSTN_Off();
   //*(volatile unsigned int *) (LCD_IO_Base + 0x0C00) &= ~( 1 << 27 );
}
   

void fill_STN_pattern()
{
    *(volatile unsigned int *) (LCD_IO_Base + 0x0F00) = 0xFFFFFFFF;              //0
    *(volatile unsigned int *) (LCD_IO_Base + 0x0F04) = 0x0000FFEF;  //0x0000FFDF;  //1
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F08) = 0x000077F7;   //0xFFFFFFFE;  //2//0xEFFFFFFD;  //0xFFFFFFBF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F0C) = 0x0000007E;   //0xFFFFFEFD;  //3//0xFFFFFFFE;  //0xFFBFFFBF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F10) = 0xFFFFFEFD;   //0xFFFFFEFD;  //4//0xFBFBBBDD;  //0xFBFBBBBB;  //0xFBFBFBFF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F14) = 0x3DFFFFBE;   //0xFFFFFDDD;  //5//0xF7FBF7FB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F18) = 0x00006F77;   //0xFFFFFEDE;  //6//0xDFDFDFD5;  //0xDFDFDFDB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F1C) = 0x00001DEE;   //0xFFFFBBDD;  //7//0xFBEBFB55;  //0xFBEBFBEB;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F20) = 0x00005DDD;   //0x3DFFFDDD;  //8//0xEEEAAAAA;  //0xEEEBEEED;  //0xEEEBEEEF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F24) = 0x3DFFFDDD;   //0xFFFFFDB6;  //9//0xBDEDAAAA;  //0xBDEDBAAA;  //0xDEF6DEF6;  //0x77777777;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F28) = 0x0000005D;   //0xFFFFDDB6;  //10//0x757F754A; //0x757F7577;  //0x5DDDDDDD;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F2C) = 0x00001B6E;   //0x3DFFFED9;  //11//0x5DED5DEA; //0x5DED5DED;  //10
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F30) = 0x00004F6D;   //0xFFFFB5AD;  //12//0x5D5D4421; //0x5D5D7D5D;
      
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F34) = 0x00007B6D;   //0xFFFFFAD6;  //13//0xEAAAEEEA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F38) = 0xFFFFFAD6;   //0x00005AAD;  //14//0xABAAEBAA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F3C) = 0x00003FFA;   //0x3DFFFE99;  //15//0x55755575;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F40) = 0x00003FF9; //16
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F44) = 0x00005BB5; //17
//    *(volatile unsigned int *) (LCD_IO_Base + 0x0D44) = 0x55555551;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F48) = 0x00001ECD; //18
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F4C) = 0x0000EB35; //19
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F50) = 0x00000699;   //20
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F54) = 0x0000FAAA; //21
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F58) = 0x0000002A; //22
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F5C) = 0x00003FEA; //23
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F60) = 0x00003FE3;   //0x00001249; //24
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F64) = 0x00000049;   //25
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F68) = 0xFFFFE912; //26
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F6C) = 0x0000F912; //27
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F70) = 0x0000E422; //28
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F74) = 0x00000842; //29
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F78) = 0x00000008; //30
      *(volatile unsigned int *) (LCD_IO_Base + 0x0F7C) = 0x00000000; //31
      
   
/*    *(volatile unsigned int *) (LCD_IO_Base + 0x0F80) = 0xFFFFE844; //32
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D04) = 0x00007FFF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D08) = 0x00007F7F;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D0C) = 0x000057FF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D10) = 0x0000B76F;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D14) = 0x000007FF;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D18) = 0x0000F54B;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D1C) = 0x000016DD;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D20) = 0x00005555;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D24) = 0x0000924D;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D28) = 0x00002255;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D2C) = 0x000002AA;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D30) = 0x00002222;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D34) = 0x00000448;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D38) = 0x0000A000;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0D3C) = 0x00000000;
*/
}  


void STN_160x80_On()
{
// int i;
   
/* PANEL_WIDTH = PANEL_HEIGHT = 160;
    Dithering( 0, 2);   //disable dithering
    SetDivNo(2);
*/ 
    *(volatile unsigned int *) (LCD_IO_Base + 0x50) = 0x00000600; //GPIO output bit 1 & 2 set to 1
    *(volatile unsigned int *) (LCD_IO_Base + 0x54) = 0x0000000F; //bypass rgb<->ycbcr and other, or some data will lost
    *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) |= ( 1 << 27 );
    *(volatile unsigned int *) (LCD_IO_Base + 0x0C08) &= 0xffffffbf; //ensure the pattern is for STN
/* *(volatile unsigned int *) (LCD_IO_Base + 0x0C00) = ( 1 << 27 ) | ( 0 << 25 ) | ( 1 << 24 ) | ( ( PANEL_WIDTH / 16 ) << 16 ) | ( PANEL_WIDTH - 1 );
      *(volatile unsigned int *) (LCD_IO_Base + 0x0C04) = ( 2 * PANEL_HEIGHT + 1 ) | ( 1 << 16 );  
      *(volatile unsigned int *) (LCD_IO_Base + 0x0C04) &= 0xffffff7f;  //bit 7 of 0xc04 sets to 0
*/    
            //choose the grayscale level
/*    for( i=0; i<16; i++ )
      {
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E00 + i*4) = i;//+16;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E40 + i*4) = i;//+16;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E80 + i*4) = i;//+16;
   }
*/

      *(volatile unsigned int *) (LCD_IO_Base + 0x0E00 ) = 0x0;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E04 ) = 0x1F;
/*    *(volatile unsigned int *) (LCD_IO_Base + 0x0E08 ) = 0x4;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E0C ) = 0x6;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E10 ) = 0x7;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E14 ) = 0x9;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E18 ) = 0xb;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E1C ) = 0xc;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E20 ) = 0xd;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E24 ) = 0xf;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E28 ) = 0x12;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E2C ) = 0x14;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E30 ) = 0x17;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E34 ) = 0x19;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E38 ) = 0x1c;
      *(volatile unsigned int *) (LCD_IO_Base + 0x0E3C ) = 0x1F;
*/
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void Sitronix_stn_pattern( unsigned char *pFrameBuffer, int bpp )
{
   int i, j;
   unsigned char result;


   SetBPP( 1 );

   printf("1 BPP test\n");
   //horizontal two bars
   {
      unsigned int color[16] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
      blocks_4x4( pFrameBuffer, 1, color, 0 );

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );
   }
   //vertical two bars
   {
      unsigned int color[16] = {0x00000000, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffff0000, 0xffffffff, 0xffffffff,
         0x00000000, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffff0000, 0xffffffff, 0xffffffff};
      blocks_4x4( pFrameBuffer, 1, color, 0 );

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );
   }
   //4x4 blocks with 2 colors
   {
      unsigned int color[16] = {0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
         0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000};
      blocks_4x4( pFrameBuffer, 2, color, 0 );

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

   }

   SetBPP( 2 );

   printf("2 BPP test\n");
   //horizontal four bars
   {
      unsigned int color[16] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x55555555, 0x55555555, 0x55555555, 0x55555555,
         0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
      blocks_4x4( pFrameBuffer, 2, color, 0 );

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

   }
   //vertical four bars
   {
      unsigned int color[16] = {0x00000000, 0x55555555, 0xaaaaaaaa, 0xffffffff, 0x00000000, 0x55555555, 0xaaaaaaaa, 0xffffffff,
         0x00000000, 0x55555555, 0xaaaaaaaa, 0xffffffff, 0x00000000, 0x55555555, 0xaaaaaaaa, 0xffffffff};
      blocks_4x4( pFrameBuffer, 2, color, 0 );

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

   }
   //4x4 blocks with 4 colors
   {
      unsigned int color[16] = {0x00000000, 0x00000000, 0x55555555, 0x55555555, 0x00000000, 0x00000000, 0x55555555, 0x55555555,
         0xaaaaaaaa, 0xaaaaaaaa, 0xffffffff, 0xffffffff, 0xaaaaaaaa, 0xaaaaaaaa, 0xffffffff, 0xffffffff};
      blocks_4x4( pFrameBuffer, 2, color, 0 );

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

   }  

   SetBPP( bpp );
   printf("%d BPP test\n", bpp); 
   if( bpp == 24 )   bpp = 32;   //24bpp uses a 32bit address, with upper 8 bits don't care


   //Test pattern 1, fill with horizontal lines with line number 0, 2, 4, 8, ...
   //fill frame buffer values to 0xffffffff, ie. clear the screen
   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

   //currently, the first line is 0x0, the second is 0xffffffff, .... If you want to reverse change the initial i to 1
   // for( i=1; i<PANEL_HEIGHT; i+=2 ) //line --> 0xffffffff, 0x0, 0xffffffff, 0x0, ...
   for( i=0; i<PANEL_HEIGHT; i+=2 ) //line --> 0x0, 0xffffffff, 0x0, 0xffffffff, ...
      for( j=0; j<PANEL_WIDTH*bpp/8; j+=4)
         *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0x0;

   reset_lcd_controller();

   printf("Check next pattern, press 'y'\n");
   do{
      result = getchar();
   }while( (result != 'y') );


   //Test pattern 2, fill with vertical lines with line number 0, 2, 4, 8, ...
   //fill frame buffer values to 0xffffffff, ie. clear the screen
   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

   for( i=0; i<PANEL_HEIGHT; i++ )
      for( j=0; j<PANEL_WIDTH*bpp/8; j+=4)   //4 bits per filling
         if( bpp == 4 ){
            //          *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0x0f0f0f0f;   //the first vertical line is 0xf, the second is 0x0
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0xf0f0f0f0;   //the first vertical line is 0x0, the second is 0xf
         }else if( bpp == 16 ){
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0x0000ffff;
         }else if( bpp == 32 ){  //user inputs 24bit data
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0x0;
            j+=4;
         }

   reset_lcd_controller();

   printf("Check next pattern, press 'y'\n");
   do{
      result = getchar();
   }while( (result != 'y') );


   //Test pattern 3, fill with dots with dot number 0, 2, 4, 8, ...
   //fill frame buffer values to 0xffffffff, ie. clear the screen
   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

   for( i=0; i<PANEL_HEIGHT; i++ )
      for( j=0; j<PANEL_WIDTH*bpp/8; j+=4){  //4 bits per filling
         int val;

         if( bpp == 4 ){
            if( i%2 == 0 ) val = 0x0f0f0f0f;
            else val = 0xf0f0f0f0;

            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = val;
         }else if( bpp == 16 ){
            if( i%2 == 0 ) val = 0x0000ffff;
            else val = 0xffff0000;

            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = val;
         }else if( bpp == 32 ){  //user inputs 24bit data         
            if( i%2 == 0 ) val = 0;
            else val = 4;

            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j + val ) = 0x0;
            j+=4;
         }
      }

   reset_lcd_controller();

   printf("Check next pattern, press 'y'\n");
   do{
      result = getchar();
   }while( (result != 'y') );


   //Test pattern 4, draw a retangle with the same horizontal/vertical width as the length to the edge of the panel
   //fill frame buffer values to 0xffffffff, ie. clear the screen
   central_square( pFrameBuffer, bpp, 0xffffffff );


   //Test pattern 5, draw vertical bars with the width of each not equals to 10 pixels
   //fill frame buffer values to 0xffffffff, ie. clear the screen
   {
      int i, j, k, bpp;
      volatile unsigned int val=0x0;


      bpp=4;
      SetBPP( bpp );

      for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
         *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

      for( i=0; i<PANEL_HEIGHT; i++ )
      {
         k = 1;
         j = 0;
         val = 0x0;  //offset color

         for( k=1; k<=16; k++ )
            //subtract 32, because each vertical bar is word-alignment
            for( ; j<(PANEL_WIDTH-32)*bpp/8/16*k; j+=4){    //16 means 16 vertical bars, k means ?th bar
               if( k%2 == 0 )
                  *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = val + 0x11111111 * (k-1); 
               else
                  *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = val + 0x11111111 * ( 15 - (k-1) ); 
               //k++;
            }
      }

      reset_lcd_controller();

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );
   }


   //Test pattern 6, fill the screen with many pattern 'Q' 
   //fill frame buffer values to 0xffffffff, ie. clear the screen
   //  O * * * * O
   //  *         *
   //  *         *
   //  *         *
   //  *         *
   //  *       * *
   //  O * * * * *
   //              *
   {
#ifdef SIMPLE_OSD
      int i, j;
      unsigned int pattern_Q[] = {  //this pattern has to be fine tuned, each 'Q' is 8x8 pixels, the left and upper line are blank, and the top-right, top-left and left-down are blank, and the upper-left to down-right are three pixels through the down-right pixel of the 'Q'
         ///////////////Q////////////////////
         0x00000000, 0x00000000, 0x00000000, 0x0000003c, 0x00000042, 
         0x00000042, 0x00000042, 0x00000042, 0x00000062, 0x0000007c, 
         0x00000080, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
         0x00000000,};

      unsigned int pattern_Q_Attribute[] =
      {

         //****0******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****1******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****2******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****3******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****4******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****5******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****6******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****7******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****8******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****9******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****10******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****11******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****12******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,

         //****13******************
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,
         0x00000008,   0x00000008,   0x00000008,   0x00000008,   0x00000008,   
      };

      //fill frame buffer values to 0xffffffff, ie. clear the screen
      for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
         *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;


      for(i=0; i< 16/**10 + 16* 26 + 3* 16*/; i++)   // 10 digit & 26 alphabet & 3 symbol
         *(unsigned int *)(uintptr_t)(LCD_IO_Base + 0x8000 +i*4 )  = pattern_Q[i];//OSD_Font_Simple[i];//pattern_Q[i];   // i*4 -> 1 word = 4 bytes

      for(i=0; i< 140  ; i++)
         *(unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 +i*4 )  = pattern_Q_Attribute[i];//pattern_Q_Attribute[i];        


      //    if(mode == 2)   //YCbCr
      {
         OSD_fg_color_Simple( 0x57, 0x88, 0x3B, 0xFF);
         OSD_bg_color_Simple( 0x57, 0x88, 0x3B);
      }
      /*    else
            {
            OSD_fg_color_Simple( 0x07, 0x38, 0xC0, 0xFF);
            OSD_bg_color_Simple( 0x07, 0x38, 0xc0);
            }
       */ 

      OSD_On_Simple(1);  
      OSD_transparent_Simple(3);

      i=0xd, j=0xa;

      OSD_Dim_Simple( i, j);
      OSD_Pos_Simple( 1, 0);
      OSD_Scal_Simple( 0,0 );

      //    reset_lcd_controller();

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

      OSD_On_Simple(0);    
#endif
   }


   //Test pattern 7, 水平漸層4, 8 , 16灰階
   horizontal_bar( pFrameBuffer, bpp, 0x11111111, 4 );
   horizontal_bar( pFrameBuffer, bpp, 0x11111111, 8 );
   horizontal_bar( pFrameBuffer, bpp, 0x11111111, 16 );


   //Test pattern 8, 垂直漸層4, 8, 16灰階
   vertical_bar( pFrameBuffer, bpp, 0x11111111, 4, 0 ); //should be 0
   vertical_bar( pFrameBuffer, bpp, 0x11111111, 8, 32 );
   vertical_bar( pFrameBuffer, bpp, 0x11111111, 16, 32 );


   //Test pattern 10, 4x4 blocks灰階
   {
      unsigned int color[16] = {0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
         0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd, 0xeeeeeeee, 0xffffffff};
      blocks_4x4( pFrameBuffer, bpp, color, 1 );
   }


   /*    //Test pattern 11, 4x8 blocks灰階
         {
         int i, bpp; //, j, k,  bar=4;
   //    volatile unsigned int val=0x0;
   unsigned int color[32] = {0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
   0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd, 0xeeeeeeee, 0xffffffff,
   0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
   0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd, 0xeeeeeeee, 0xffffffff};

   bpp=4;
   SetBPP( bpp );

   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
    *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;     

    Draw_color_block( pFrameBuffer, bpp, color, 4, 8 );
    *//*    for( i=0; i<PANEL_HEIGHT; i++ )
         {
         k = 1;
         j = 0;
   //       val = 0x0;
   //       bar = 8; //the total bars on the screen

   //want to show 32 gray colors (4*8)       
   if( val == 0x11111110 ){
   val = 0;
   //          printf("val = %x\n", val);
   }

   for( k=1; k<=bar; k++ ){
   for( ; j<PANEL_WIDTH*bpp/8*k/bar; j+=4){     //16 means 16 vertical bars, k means ?th bar
       *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = val + 0x11111111 * ((k-1));
       }
       }

       if( (i != 0) && (i % (PANEL_HEIGHT/bar/2) == 0) ){
       val+=0x44444444;
       }
       }
       */
   /*    reset_lcd_controller();

         printf("End of Sitronix pattern test!, press 'y'\n");
         do{
         result = getchar();
         }while( (result != 'y') );
         }
    */
}



void Sitronix_cstn_pattern( unsigned char *pFrameBuffer, int bpp )
{  
   int i;
   char result;
   unsigned int color[16];/* = {0x00000000, 0x10821082, 0x21042104, 0x31863186, 0x42084208, 0x528a528a, 0x630c630c, 0x738e738e,
             0x84108410, 0x94929492, 0xa514a514, 0xb596b596, 0xc618c618, 0xd69ad69a, 0xe71ce71c, 0xffffffff};
           */
   printf("%d bpp test\n", bpp);
   //Test pattern 1, display three colors, Red, Green, Blue
   fill_color( pFrameBuffer, bpp, 0xf000f000 ); //Red
   fill_color( pFrameBuffer, bpp, 0x07800780 ); //Green
   fill_color( pFrameBuffer, bpp, 0x001e001e ); //Blue


   //Test for Grayscale
   horizontal_bar( pFrameBuffer, bpp, 0x10821082, 4 );
   horizontal_bar( pFrameBuffer, bpp, 0x10821082, 8 );
   horizontal_bar( pFrameBuffer, bpp, 0x10821082, 16 );

   vertical_bar( pFrameBuffer, bpp, 0x10821082, 3, 0 );  //make sure if the Voltage of Red, Green, and Blue of the driver IC output is the same -> make the panel's color the same when in each grayscale
   vertical_bar( pFrameBuffer, bpp, 0x10821082, 6, 0 );
   vertical_bar( pFrameBuffer, bpp, 0x10821082, 9, 0 );
   vertical_bar( pFrameBuffer, bpp, 0x10821082, 12, 0 );
   vertical_bar( pFrameBuffer, bpp, 0x10821082, 4, 0 );  //should be 0
   vertical_bar( pFrameBuffer, bpp, 0x10821082, 8, 16 );
   vertical_bar( pFrameBuffer, bpp, 0x10821082, 16, 48 );

   central_square( pFrameBuffer, bpp, 0xf79ef79e );

   for( i=0; i<16; i++ )
      color[i] = ( ( ( 1 << 12 | 1 << 7 | 1 << 1 ) << 16 ) | ( 1 << 12 | 1 << 7 | 1 << 1 ) ) * i;
   blocks_4x4( pFrameBuffer, bpp, color, 1 );


   //Test for color, Red
   horizontal_bar( pFrameBuffer, bpp, 0x10001000, 4 );
   horizontal_bar( pFrameBuffer, bpp, 0x10001000, 8 );
   horizontal_bar( pFrameBuffer, bpp, 0x10001000, 16 );

   vertical_bar( pFrameBuffer, bpp, 0x10001000, 3, 0 );
   vertical_bar( pFrameBuffer, bpp, 0x10001000, 4, 0 );
   vertical_bar( pFrameBuffer, bpp, 0x10001000, 8, 16 );
   vertical_bar( pFrameBuffer, bpp, 0x10001000, 16, 48 );

   central_square( pFrameBuffer, bpp, 0xF000F000 );

   for( i=0; i<16; i++ )
      color[i] = ( ( ( 1 << 12 ) << 16 ) | ( 1 << 12 ) ) * i;
   blocks_4x4( pFrameBuffer, bpp, color, 1 );


   //Test for color, Green
   horizontal_bar( pFrameBuffer, bpp, 0x00800080, 4 );
   horizontal_bar( pFrameBuffer, bpp, 0x00800080, 8 );
   horizontal_bar( pFrameBuffer, bpp, 0x00800080, 16 );

   vertical_bar( pFrameBuffer, bpp, 0x00800080, 4, 0 );
   vertical_bar( pFrameBuffer, bpp, 0x00800080, 8, 16 );
   vertical_bar( pFrameBuffer, bpp, 0x00800080, 16, 48 );

   central_square( pFrameBuffer, bpp, 0x7800780 );

   for( i=0; i<16; i++ )
      color[i] = ( ( ( 1 << 7 ) << 16 ) | ( 1 << 7 ) ) * i;
   blocks_4x4( pFrameBuffer, bpp, color, 1 );


   //Test for color, Blue
   horizontal_bar( pFrameBuffer, bpp, 0x00020002, 4 );
   horizontal_bar( pFrameBuffer, bpp, 0x00020002, 8 );
   horizontal_bar( pFrameBuffer, bpp, 0x00020002, 16 );

   vertical_bar( pFrameBuffer, bpp, 0x00020002, 4, 0 );
   vertical_bar( pFrameBuffer, bpp, 0x00020002, 8, 16 );
   vertical_bar( pFrameBuffer, bpp, 0x00020002, 16, 48 );

   central_square( pFrameBuffer, bpp, 0x1E001E );

   for( i=0; i<16; i++ )
      color[i] = ( ( ( 1 << 1 ) << 16 ) | ( 1 << 1 ) ) * i;
   blocks_4x4( pFrameBuffer, bpp, color, 1 );



   do{
      fill_background( pFrameBuffer, bpp );
      printf("Exit, press 'q', Continue, other than 'q'!n");
      scanf( "%s", &result );
   }while( (result != 'q') );





}

void CSTN_Test( unsigned char *pFrameBuffer )
{
   int i;

   if( LCD_PANEL_TYPE == 9 )
   {
      Sitronix_cstn_pattern( pFrameBuffer, 16 );

      Scroll_Test( pFrameBuffer, "33_565_320x240.bmp.bin", 320, 240, 153600, 16);   
      Scroll_Test( pFrameBuffer, "39_565_320x240.bmp.bin", 320, 240, 153600, 16);
      Scroll_Test( pFrameBuffer, "61_565_320x240.bmp.bin", 320, 240, 153600, 16);
      Scroll_Test( pFrameBuffer, "71_565_320x240.bmp.bin", 320, 240, 153600, 16);
      Scroll_Test( pFrameBuffer, "15_565_320x240.bmp.bin", 320, 240, 153600, 16);

      SetBPP(8);
      printf("RGB332 test\n");
      //Test pattern 1, display five colors, Black, White, Red, Green, Blue
      fill_color( pFrameBuffer, 8, 0 );   //Black
      fill_color( pFrameBuffer, 8, 0xffffffff );   //White
      fill_color( pFrameBuffer, 8, 0xe0e0e0e0 );   //Red
      fill_color( pFrameBuffer, 8, 0x1c1c1c1c );   //Green
      fill_color( pFrameBuffer, 8, 0x02020202 );   //Blue

      printf("RGB 332 LBLP Test\n");
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "11_240x160.bmp_rgb332.bin");
      Load_Image( FILENAME, 38400, pFrameBuffer, LBLP, 8 );
      enable_lcd_controller();
      for( i=0; i<500000; i++ );

      printf("RGB 332 LBLP Test\n");
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "15_240x160.bmp_rgb332.bin");
      Load_Image( FILENAME, 38400, pFrameBuffer, LBLP, 8 );
      enable_lcd_controller();
      for( i=0; i<500000; i++ );

      printf("RGB 332 LBLP Test\n");
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "16_240x160.bmp_rgb332.bin");
      Load_Image( FILENAME, 38400, pFrameBuffer, LBLP, 8 );
      enable_lcd_controller();
      for( i=0; i<500000; i++ );

      printf("RGB 332 LBLP Test\n");
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "20_240x160.bmp_rgb332.bin");
      Load_Image( FILENAME, 38400, pFrameBuffer, LBLP, 8 );
      enable_lcd_controller();
      for( i=0; i<500000; i++ );

      printf("RGB 332 LBLP Test\n");
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "21_240x160.bmp_rgb332.bin");
      Load_Image( FILENAME, 38400, pFrameBuffer, LBLP, 8 );
      enable_lcd_controller();
      for( i=0; i<1000000; i++ );

      //test grey scale color 4bpp  
      /*    disable_lcd_controller();

      //fill the palette value
       *(volatile unsigned int *) (LCD_IO_Base + 0x0A00) = 0x07E0001F;  // Palette 0 = Red,  Palette 1=Green   (LCD palette RAM accessing port)
       *(volatile unsigned int *) (LCD_IO_Base + 0x0A04) = 0x0000F800;  // Palette 2 = Blue, Palette 3=Black, how values in framebuffer remap their color ?
       *(volatile unsigned int *) (LCD_IO_Base + 0x0A08) = 0x07FFFFFF;  // Palette 4 = White,  Palette 5=Green (LCD palette RAM accessing port)
       *(volatile unsigned int *) (LCD_IO_Base + 0x0A0C) = 0xF81F0FFE;  // Palette 6 = Blue, Palette 7=, how values in framebuffer remap their color ?
       *(volatile unsigned int *) (LCD_IO_Base + 0x0B00) = 0x0380000F;  // Palette 0 = Red,  Palette 1=Green   (LCD palette RAM accessing port)
       *(volatile unsigned int *) (LCD_IO_Base + 0x0B04) = 0x00007C00;  // Palette 2 = Blue, Palette 3=Black, how values in framebuffer remap their color ?
       *(volatile unsigned int *) (LCD_IO_Base + 0x0B08) = 0x03887777;  // Palette 4 = White,  Palette 5=Green (LCD palette RAM accessing port)
       *(volatile unsigned int *) (LCD_IO_Base + 0x0B0C) = 0x780F0777;  // Palette 6 = Blue, Palette 7=, how values in framebuffer remap their color ?

       SetBPP(4);
       printf("RGB Grey color 4bpp LBLP Test\n");
       strcpy(FILENAME, DEFAULT_PATH);
      //       strcat(FILENAME, "wmf_grey_240x80.bmp_grey_4bpp.yuv");
      //    strcat(FILENAME, "wmf_240x80.bmp_grey_4bpp.yuv");
      strcat(FILENAME, "15_240x80.bmp_grey_4bpp.yuv");
      Load_Image( FILENAME, 9600, pFrameBuffer, LBLP, 4 );
      enable_lcd_controller();
      for( i=0; i<30000000; i++ );
       */
      /*    //animation test
            disable_lcd_controller();
            SetBPP(16);
            SetYCbCr(422);
            printf("YCbCr422 Animation LBLP Test\n");
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "\\animation\\log_422_240x80_100f.yuv");
            Load_Image( FILENAME, 3840000, pFrameBuffer, LBLP, 16 );
            enable_lcd_controller();

            while(1)
            {
            for( i=0; i<100; i++ )
            {
            int k;
            for( k=0; k<200000; k++ );
            if( i == 0 )   continue;
       *(volatile unsigned int *) (LCD_IO_Base + 0x0018) += 240*80*2;
       }
       *(volatile unsigned int *) (LCD_IO_Base + 0x0018) = pFrameBuffer;
       }
       */

      SetBPP(16);
      printf("RGB565 LBLP Test\n");
      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "11_565_240x160.bmp.bin");
      Load_Image( FILENAME, 76800, pFrameBuffer, LBLP, 16 );
      //       strcat(FILENAME, "new\\15_565_240x160.bmp.bin");
      //    Load_Image( FILENAME, 76800, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      for( i=0; i<10000000; i++ );


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "15_565_240x160.bmp.bin");
      Load_Image( FILENAME, 76800, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      for( i=0; i<10000000; i++ );


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "16_565_240x160.bmp.bin");
      Load_Image( FILENAME, 76800, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      for( i=0; i<10000000; i++ );


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "20_565_240x160.bmp.bin");
      Load_Image( FILENAME, 76800, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      for( i=0; i<10000000; i++ );


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "21_565_240x160.bmp.bin");
      //       strcat(FILENAME, "38_565_240x80.bmp.bin");
      Load_Image( FILENAME, 76800, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      for( i=0; i<30000000; i++ );     

      /*    disable_lcd_controller();
            printf("YCbCr422 LBLP Test\n");
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "lady_422_240x80.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 38400, pFrameBuffer, LBLP );
            enable_lcd_controller();
            for( i=0; i<30000000; i++ );

            SetBPP(24);
            SetYCbCr(0);
            disable_lcd_controller();
            printf("RGB888 LBLP Test\n");
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_888_240x80.bin");
            Load_Image( FILENAME, 76800, pFrameBuffer, LBLP, 24 );
            enable_lcd_controller();   //應該是因為data size比較大的關係，所以造成fifo underrun，而在cstn，這種情況是很嚴重的
            for( i=0; i<30000000; i++ );
       */   }
}


void STN_Test( unsigned char *pFrameBuffer )
{  
   if (LCD_PANEL_TYPE == 10 )
   {     
      Sitronix_stn_pattern( pFrameBuffer, 4 );


      SetBPP(4);

      //    Scroll_Test( pFrameBuffer, "15_160x160.bmp_grey_4bpp.yuv", 160, 160, 12800, 4);
      Scroll_Test( pFrameBuffer, "15_320x240.bmp_grey_4bpp.yuv", 320, 240, 38400, 4);
      Scroll_Test( pFrameBuffer, "16_320x240.bmp_grey_4bpp.yuv", 320, 240, 38400, 4);     

      SetBPP(16);

      Scroll_Test( pFrameBuffer, "33_565_320x240.bmp.bin", 320, 240, 153600, 16);
      Scroll_Test( pFrameBuffer, "39_565_320x240.bmp.bin", 320, 240, 153600, 16);
      Scroll_Test( pFrameBuffer, "61_565_320x240.bmp.bin", 320, 240, 153600, 16);
      Scroll_Test( pFrameBuffer, "71_565_320x240.bmp.bin", 320, 240, 153600, 16);
      Scroll_Test( pFrameBuffer, "15_565_320x240.bmp.bin", 320, 240, 153600, 16);


      printf("RGB565 LBLP Test\n");
      strcpy(FILENAME, DEFAULT_PATH);
      //       strcat(FILENAME, "15_565_160x160.bmp.bin");
      strcat(FILENAME, "33_565_160x160.bmp.bin");
      Load_Image( FILENAME, 51200, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      delay_10ms(70);


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "39_565_160x160.bmp.bin");
      Load_Image( FILENAME, 51200, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      delay_10ms(70);


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "61_565_160x160.bmp.bin");
      Load_Image( FILENAME, 51200, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      delay_10ms(70);


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "71_565_160x160.bmp.bin");
      Load_Image( FILENAME, 51200, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      delay_10ms(70);


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "15_565_160x160.bmp.bin");
      Load_Image( FILENAME, 51200, pFrameBuffer, LBLP, 16 );
      reset_lcd_controller();
      delay_10ms(70);


      /***************************** for PoP test *************************/
      /*    {  unsigned char *fb0, *fb1, *fb2, *fb3;
            int i;
            char endian[] = "    ";

            int bpp = 16;

            SetBPP(bpp);

            fb0 = pFrameBuffer;//allocateFrameBuffer(PANEL_WIDTH, PANEL_HEIGHT, bpp, 1);
            fb1 = allocateFrameBuffer(PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 1);
            fb2 = allocateFrameBuffer(PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 1);
            fb3 = allocateFrameBuffer(PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 1);

            setNumFrameBuffer(1, 1, 1, 1);

      //initialize frame buffer first
      SetFrameBase(0,0, fb0);
      SetFrameBase(1,0, fb1);
      SetFrameBase(2,0, fb2);
      SetFrameBase(3,0, fb3);

      SetVirtualWidth( PANEL_WIDTH/2 );
      PoP_On();

      //action here
      clear_Screen(fb0, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);
      clear_Screen(fb1, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xffffff00);
      clear_Screen(fb2, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0xff00ffff);
      clear_Screen(fb3, PANEL_WIDTH/2, PANEL_HEIGHT/2, bpp, 0x0);


      SetYCbCr(0);


      PoP_On();
      i = LBLP;   //set the current endian to LBLP

      if( bpp == 24 )
      {
      if( ( PANEL_WIDTH == 160 ) && ( PANEL_HEIGHT == 160 ) )
      {
      //             for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
      {
      int k;

      //test LBLP, BBBP, and LBBP
      getEndianString(i, endian);
      printf("Test RGB888 %s in PoP\n", endian);

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "20_888_80x80.bmp.bin");
      Load_Image( FILENAME, 25600, fb0, i, bpp );
      reset_lcd_controller();
      for( k=0; k<300000; k++ );


      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "16_888_80x80.bmp.bin");
      Load_Image( FILENAME, 25600, fb1, i, bpp );
      reset_lcd_controller();
      for( k=0; k<300000; k++ );

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "15_888_80x80.bmp.bin");
      Load_Image( FILENAME, 25600, fb2, i, bpp );
      reset_lcd_controller();
      for( k=0; k<300000; k++ );

      strcpy(FILENAME, DEFAULT_PATH);
      strcat(FILENAME, "11_888_80x80.bmp.bin");
      Load_Image( FILENAME, 25600, fb3, i, bpp );
      reset_lcd_controller();
      for( k=0; k<3000000; k++ );
   }
   }
   }else if( bpp == 16 )
   {  
      if( ( PANEL_WIDTH == 160 ) && ( PANEL_HEIGHT == 160 ) )
      {
         //             for( i = LBLP; i <= LBBP; i++ )     //0=LBLP, 1=BBBP, 2=LBBP
         {
            int k;

            //test LBLP, BBBP, and LBBP
            getEndianString(i, endian);
            printf("Test RGB565 %s in PoP\n", endian);
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "11_565_80x80.bmp.bin");
            Load_Image( FILENAME, 12800, fb0, i, bpp );
            reset_lcd_controller();
            for( k=0; k<3000000; k++ );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_565_80x80.bmp.bin");
            Load_Image( FILENAME, 12800, fb1, i, bpp );
            reset_lcd_controller();
            for( k=0; k<3000000; k++ );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_565_80x80.bmp.bin");
            Load_Image( FILENAME, 12800, fb2, i, bpp );
            reset_lcd_controller();
            for( k=0; k<3000000; k++ );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "20_565_80x80.bmp.bin");
            Load_Image( FILENAME, 12800, fb3, i, bpp );
            reset_lcd_controller();
            for( k=0; k<30000000; k++ );


            SetYCbCr(422);
            //test LBLP, BBBP, and LBBP for YCbCr422
            getEndianString(i, endian);
            printf("Test YCbCr422 %s in PoP\n", endian);
            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "2_422_80x80.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 12800, fb0, i );
            reset_lcd_controller();
            for( k=0; k<3000000; k++ );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "16_422_80x80.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 12800, fb1, i );
            reset_lcd_controller();
            for( k=0; k<3000000; k++ );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "15_422_80x80.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 12800, fb2, i );
            reset_lcd_controller();
            for( k=0; k<3000000; k++ );

            strcpy(FILENAME, DEFAULT_PATH);
            strcat(FILENAME, "18_422_80x80.bmp.yuv");
            Load_Image_YCbCr422( FILENAME, 12800, fb3, i );
            reset_lcd_controller();
            for( k=0; k<30000000; k++ );

            SetYCbCr(0);
         }
      }
   }
   SetVirtualWidth( PANEL_WIDTH );
   PoP_Off();

   free(fb1);  free(fb2);  free(fb3);
   }//end of PoP test
   */ }
}

void OSD_Test_Simple_for_mono( unsigned char *pFrameBuffer, char *str, int bpp)
{
#ifdef SIMPLE_OSD
   int i,j;
   char *msg="                                        ";

   // unsigned char r, g, b;
   SetBPP(bpp);

   strcpy( msg, str);

   for(i=0; i< 16*10 + 16* 26 + 3* 16; i++)   // 10 digit & 26 alphabet & 3 symbol
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x8000 +i*4 )  = OSD_Font_Simple[i];   // i*4 -> 1 word = 4 bytes

   for(i=0; i< 100  ; i++)
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 +i*4 )  = 0;//OSD_Attribute_Simple[i];        

   OSD_puts_Simple(str, 0);   

   OSD_fg_color_Simple( 0xFF, 0x57, 0x88, 0x3B);
   OSD_bg_color_Simple( 0x57, 0x88, 0x3B);

   OSD_On_Simple(1);  
   OSD_transparent_Simple(3);

   // i=0xd, j=0xa;(full screen for STN 160x160)
   i=0xa, j=0x1;  //full screen for STN 160x80

   OSD_Dim_Simple( i, j);
   OSD_Pos_Simple( 20, 30);
   OSD_Scal_Simple( 0,0 );

   // reset_lcd_controller();

   // OSD_On_Simple(0);
#endif
}


void STN_160x80_Test( unsigned char *pFrameBuffer )
{
   if (LCD_PANEL_TYPE == 11 )
   {
      int i,j, bpp=1;
      char result;
      char *str;

      SetBPP(bpp);

      //Test pattern 1, fill with horizontal lines with line number 0, 2, 4, 8, ...
      //fill frame buffer values to 0xffffffff, ie. clear the screen
      for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
         *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

      //currently, the first line is 0x0, the second is 0xffffffff, .... If you want to reverse change the initial i to 1
      // for( i=1; i<PANEL_HEIGHT; i+=2 ) //line --> 0xffffffff, 0x0, 0xffffffff, 0x0, ...
      for( i=0; i<PANEL_HEIGHT; i+=2 ) //line --> 0x0, 0xffffffff, 0x0, 0xffffffff, ...
         for( j=0; j<PANEL_WIDTH*bpp/8; j+=4)
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0;

      reset_lcd_controller();

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );


      //Test pattern 2, fill with vertical lines with line number 0, 2, 4, 8, ...
      //fill frame buffer values to 0xffffffff, ie. clear the screen
      for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
         *(volatile unsigned int *) (pFrameBuffer + i) = 0x55555555;   

      reset_lcd_controller();

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );


      //Test pattern 3, fill with dots with dot number 0, 2, 4, 8, ...
      //currently, the first line is 0x0, the second is 0xffffffff, .... If you want to reverse change the initial i to 1
      for( i=1; i<PANEL_HEIGHT; i+=2 ) //line --> 0xffffffff, 0x0, 0xffffffff, 0x0, ...
         // for( i=0; i<PANEL_HEIGHT; i+=2 ) //line --> 0x0, 0xffffffff, 0x0, 0xffffffff, ...
         for( j=0; j<PANEL_WIDTH*bpp/8; j+=4)
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0x55555555;

      // for( i=1; i<PANEL_HEIGHT; i+=2 ) //line --> 0xffffffff, 0x0, 0xffffffff, 0x0, ...
      for( i=0; i<PANEL_HEIGHT; i+=2 ) //line --> 0x0, 0xffffffff, 0x0, 0xffffffff, ...
         for( j=0; j<PANEL_WIDTH*bpp/8; j+=4)
            *(volatile unsigned int *) (pFrameBuffer + i*PANEL_WIDTH*bpp/8 + j) = 0xaaaaaaaa;

      reset_lcd_controller();

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );


      //Test pattern 4, draw a retangle with the same horizontal/vertical width as the length to the edge of the panel
      //fill frame buffer values to 0xffffffff, ie. clear the screen
      central_square( pFrameBuffer, bpp, 0xffffffff );

      //horizontal two bars with two colors, black and white      
      for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      {
         if( i < PANEL_WIDTH*PANEL_HEIGHT*bpp/8/2 )
            *(volatile unsigned int *) (pFrameBuffer + i) = 0;
         else
            *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;
      }

      reset_lcd_controller();

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );


      //This test is for OSD
      //fill frame buffer values to 0xffffffff, ie. clear the screen
      for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
         *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;

      str = "0123456789";     
      for(i=0;i<100; i++)
      {
         OSD_Test_Simple_for_mono( pFrameBuffer, str, bpp );
         //result = str[0];
         str[(i)%10] = str[(i+1)%10];
         //str[(9)%10] = result;
         for(j=0; j< 1500000; j++);
      }

      printf("Check next pattern, press 'y'\n");
      do{
         result = getchar();
      }while( (result != 'y') );

#ifdef SIMPLE_OSD
      OSD_On_Simple(0);
#endif      
   }
}

//Test pattern 10, 4x4 blocks灰階
void blocks_4x4( unsigned char *pFrameBuffer, int bpp, unsigned int *color, int adjustable )
{
   char result;
   int i;//, bpp;
   //  unsigned int color[16] = {0x00000000, 0x10821082, 0x21042104, 0x31863186, 0x42084208, 0x528a528a, 0x630c630c, 0x738e738e,
   //                        0x84108410, 0x94929492, 0xa514a514, 0xb596b596, 0xc618c618, 0xd69ad69a, 0xe71ce71c, 0xffffffff};

   //  bpp=16;
   SetBPP( bpp );

   for( i=0; i<PANEL_WIDTH*PANEL_HEIGHT*bpp/8; i+=4)
      *(volatile unsigned int *) (pFrameBuffer + i) = 0xffffffff;   

   Draw_color_block( pFrameBuffer, bpp, color, 4, 4 );

   reset_lcd_controller();

   if( adjustable )
   {
      printf("Adjust Grayscale value\n");
      do{
         unsigned int grey[4];

         do{
            printf("level: 0 ~ 15\n\n");
            getchar();
            scanf( "%d", &grey[0] );
         }while( grey[0] > 15 );

         do{
            printf("Grayscale R: 0 ~ 31\n\n");
            getchar();
            scanf( "%d", &grey[1] );
         }while( grey[1] > 31 );

         do{
            printf("Grayscale G: 0 ~ 31\n\n");
            getchar();
            scanf( "%d", &grey[2] );
         }while( grey[2] > 31 );

         do{
            printf("Grayscale B: 0 ~ 31\n\n");
            getchar();
            scanf( "%d", &grey[3] );
         }while( grey[3] > 31 );

         if( grey[0] < 16 && grey[1] < 32 )
         {
            //              if( color[1] >= 0x10001000 
            Set_Graylevel( pFrameBuffer, grey[0], grey[1], grey[2], grey[3] );
         }else
            continue;

         printf("Press 'q' to exit\n");
         //          result = getchar();
         scanf("%s", &result);

      }while( (result != 'q') );

      printf("End of Sitronix pattern test!\n");
   }
}


