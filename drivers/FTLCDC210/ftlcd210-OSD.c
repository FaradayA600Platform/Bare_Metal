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
//#include "timer.h"
//#include "fa52x_cache.h"
#include "ftlcd210_conf.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/

#define PAL_NUM   16 

extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern unsigned int LCD_PANEL_TYPE;

/******************************************************************************
 *Public functions
 *****************************************************************************/
//*********** OSD palette color register 0 ~ 15 (offset 0x2040 ~ 0x207C) ***************

void palette_write( unsigned int data)
{
    unsigned int i;
    unsigned int *ptr;

    ptr = (unsigned int *) (LCD_IO_Base + 0x2040);
    
    for( i=0; i< PAL_NUM; i++)
        *ptr++ = data;  //the same data
}


int palette_check( unsigned int data)
{
    unsigned int i, read;
    unsigned int *ptr;

    ptr = (unsigned int *) (LCD_IO_Base + 0x2040);    

    for( i=0; i< PAL_NUM; i++ ) {   //LCD palette color register offset :0x2040 ~ 0x207C
        read = *ptr++;  //acutally read the data
        if ( read != data ) { 
            printf("OSD Palette RW Fail\n");        
            return 0;
        } 
    }       
    return 1;        
}    


void palette_write_seq()   //write palette sequentially by address
{
    unsigned int  i;
    unsigned int *ptr;

    ptr = (unsigned int *) (LCD_IO_Base + 0x2040);    

    for (i=0; i<PAL_NUM; i++) {
        *ptr = (unsigned int)(uintptr_t) ptr;
        ptr++;
    }
}


int palette_check_seq()  //be used after palette_write_seq()
{ 
    unsigned int i, read;
    unsigned int *ptr;

    ptr = (unsigned int *) (LCD_IO_Base + 0x2040);      

    for (i=0; i<PAL_NUM; i++) {
        read = *ptr;  //real read 
        if ( read != (unsigned int)(uintptr_t)ptr ) {
            printf("OSD Palette RW Fail\n");        
            return 0;
        }
        ptr++;
    }
    return 1;          
}    
  
void palette_ram_test( unsigned int base)
{
#ifdef COMPLEX_OSD
    int success=1;
    unsigned int pal[PAL_NUM], i;
    unsigned int *ptr;

    ptr = (unsigned int *) (base + 0x2040);
    for(i=0; i<PAL_NUM; i++)
        pal[i]=  *ptr++; 
  
    palette_write(0x00555555);
    if( palette_check(0x00555555) == 0 ) success =0;

    palette_write(0x00aaaaaa);
    if( palette_check(0x00aaaaaa) == 0 ) success =0;

    palette_write(0x0);
    if( palette_check(0x0) == 0 ) success =0;

    palette_write_seq();

    if( palette_check_seq() == 0) success = 0;

    palette_write(0x0);
    if( palette_check(0x0) == 0 ) success =0;

    if( success ) printf("OSD Palette color register test PASS!!\n");

    ptr = (unsigned int *) (base + 0x2040);
    for(i=0; i<PAL_NUM; i++)
        *ptr++ = pal[i];
#endif
}



/***************************** On Screen Display (OSD) Test *****************************/

void OSD_Pos( unsigned int which, int  HPos, int VPos)
{
   unsigned int temp;
    
   if (which > 0xF) {
      printf("Wrong OSD window id\n");
     return;
   }

   switch (which) {
      case 1:
        temp = *(volatile unsigned int *) (LCD_IO_Base + 0x200C ) & ~0xffff;        
      *(volatile unsigned int *) (LCD_IO_Base + 0x200C ) = temp | (VPos << 8) | HPos;  
      break;
     case 2:
        temp = *(volatile unsigned int *) (LCD_IO_Base + 0x2018 ) & ~0xffff;         
      *(volatile unsigned int *) (LCD_IO_Base + 0x2018 ) = temp | (VPos << 8) | HPos;  
      break;
     case 3:
        temp = *(volatile unsigned int *) (LCD_IO_Base + 0x2024 ) & ~0xffff;         
      *(volatile unsigned int *) (LCD_IO_Base + 0x2024 ) = temp | (VPos << 8) | HPos;
      break;   
     case 4:
        temp = *(volatile unsigned int *) (LCD_IO_Base + 0x2030 ) & ~0xffff;         
      *(volatile unsigned int *) (LCD_IO_Base + 0x2030 ) = temp | (VPos << 8) | HPos;  
      break;
   }
}   

void OSD_Dim( unsigned int which, int HDim, int VDim) //define the total font number of each row and the total row number
{
   unsigned int value;

   if (which > 0xF) {
      printf("Wrong OSD window id\n");
      return;
   } 
 
   switch (which)
   {
   case 1:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x2010);
      value &= ~0x7f;   //clear the horizontal and vertical dimension of the OSD window
      value &= ~(0x7f << 8);
      value |= ((VDim << 8) | HDim );  //set horizontal and vertical dimension
      *(volatile unsigned int *) (LCD_IO_Base + 0x2010) = value;
      break;
   case 2:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x201C);
      value &= ~0x7f;   //clear the horizontal and vertical dimension of the OSD window
      value &= ~(0x7f << 8);
      value |= ((VDim << 8) | HDim );  //set horizontal and vertical dimension
      *(volatile unsigned int *) (LCD_IO_Base + 0x201C) = value;
      break;
   case 3:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x2028);
      value &= ~0x7f;   //clear the horizontal and vertical dimension of the OSD window
      value &= ~(0x7f << 8);
      value |= ((VDim << 8) | HDim );  //set horizontal and vertical dimension
      *(volatile unsigned int *) (LCD_IO_Base + 0x2028) = value;
      break;
   case 4:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x2034);
      value &= ~0x7f;   //clear the horizontal and vertical dimension of the OSD window
      value &= ~(0x7f << 8);
      value |= ((VDim << 8) | HDim );  //set horizontal and vertical dimension
      *(volatile unsigned int *) (LCD_IO_Base + 0x2034) = value;
      break;
   }
} 


#define WIN_HL_NONE     -1
#define WIN_HL_BORDER   0
#define WIN_HL_SHADOW   1

unsigned int OSD_control(unsigned int hightlight,unsigned int bdType, unsigned int bdColor, unsigned int shType, unsigned int shColor )
{
   unsigned int value;
   
   if( hightlight == 0 )   //enable window border hightlight effect for OSD window
      value = (1 << 16) | (0<< 17) | (bdType << 18) | (bdColor << 20) | (shType << 24) | (shColor << 26);
   else if( hightlight == 1 ) //enable window shadow hightlight effect for OSD window
     value = (1 << 16) | (1<< 17) | (bdType << 18) | (bdColor << 20) | (shType << 24) | (shColor << 26);
   else  //disable window hightlight effect for OSD window
     value = (0 << 16) | (bdType << 18) | (bdColor << 20) | (shType << 24) | (shColor << 26);
     
   return value;
}

void OSD_WinControl( unsigned int which, int hightlight, int bdType, int bdColor, int shType, int shColor )
{
   unsigned int value;  //volatile is needed, or it is wrong 

   if (which > 4) {
      printf("Wrong OSD window id\n");
      return;
   }

   switch (which)
   {
   case 1:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x2010);
      value &= 0xc000ffff;
      value |= OSD_control(hightlight,bdType,bdColor,shType,shColor);
      *(volatile unsigned int *) (LCD_IO_Base + 0x2010) = value;
      break;
   case 2:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x201C);
      value &= 0xc000ffff;
      value |= OSD_control(hightlight,bdType,bdColor,shType,shColor);
      *(volatile unsigned int *) (LCD_IO_Base + 0x201C) = value;
      break;
   case 3:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x2028);
      value &= 0xc000ffff;
      value |= OSD_control(hightlight,bdType,bdColor,shType,shColor);
      *(volatile unsigned int *) (LCD_IO_Base + 0x2028) = value;
      break;
   case 4:
      value = *(volatile unsigned int *) (LCD_IO_Base + 0x2034);
      value &= 0xc000ffff;
      value |= OSD_control(hightlight,bdType,bdColor,shType,shColor);
      *(volatile unsigned int *) (LCD_IO_Base + 0x2034) = value;
      break;
   }
}  


#define TRANSP_0  0
#define TRANSP_25 1
#define TRANSP_50 2
#define TRANSP_75 3
#define TRANSP_100   4

#define FONT_HL_NONE 0
#define FONT_HL_BORDER  1
#define FONT_HL_SHADOW  2

/*TranType: 0,1,2,3,4, HighLightType: 0,1,2, BdColorSel & ShColorSel: 0~15*/
void OSD_FontControl( unsigned int TranType, unsigned int HighLightType, unsigned int BdColorSel, unsigned int ShColorSel )
{  
   if((TranType > 4) || (HighLightType > 2) || (BdColorSel > 15) || (ShColorSel > 15))
      printf("OSD font control parameters error\n");
      
   *(volatile unsigned int *) (LCD_IO_Base + 0x2004) &= 0xff0083ff;     
   *(volatile unsigned int *) (LCD_IO_Base + 0x2004) |= ((TranType << 10) | (HighLightType << 13) | (BdColorSel << 16) | (ShColorSel << 20));   //bit 12-10 :set OSD font transparency, 0=0%, 1=25%, 2=50%, 3=75%, 4=100%
}

void OSD_Row_Col_Space( unsigned int RowSpace, unsigned int ColSpace)
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x2008) &= 0xffffe0e0;
   
   if (RowSpace <= 15) {
      *(volatile unsigned int *) (LCD_IO_Base + 0x2008) |= 0x00000001;
      *(volatile unsigned int *) (LCD_IO_Base + 0x2008) |= (RowSpace << 1);
   } 
   else
      printf("Row space set error\n");
      
   if (ColSpace <= 15) {
      *(volatile unsigned int *) (LCD_IO_Base + 0x2008) |= 0x00000100;
      *(volatile unsigned int *) (LCD_IO_Base + 0x2008) |= (ColSpace << 9);
   } 
   else
      printf("Column space set error\n");
}


/*Set the start index (0~511) of OSD font attribute RAM data for window x*/
void OSD_FontAttributeBase( unsigned int which, unsigned short fontIndex )
{
   if (which > 4) {
      printf("Wrong OSD window id\n");
      return;
   }

   switch (which)
   {
      case 1:
         *(volatile unsigned int *)(LCD_IO_Base + 0x2014) = fontIndex;  //indicating the start index of font attribute RAM for OSD window 1
         break;
      case 2:
         *(volatile unsigned int *)(LCD_IO_Base + 0x2020) = fontIndex;
         break;
      case 3:
         *(volatile unsigned int *)(LCD_IO_Base + 0x202C) = fontIndex;
         break;
      case 4:
         *(volatile unsigned int *)(LCD_IO_Base + 0x2038) = fontIndex;
         break;
   }
}

/*0x3FFF means no MCF font*/
#define NO_MCF_FONT 0x200

void OSD_MCFBase( unsigned char *fontbase )
{
   *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x203C) =  (unsigned int) (uintptr_t)fontbase; //indicating the start address of multi-color font attribute RAM
}


/*HZoomIn=0 ->HScal=0,1; HZoomIn=1 ->HScal=0,1,2,3   VZoomIn=0 ->VScal=0,1; VZoomIn=1 ->VScal=0,1,2,3 */
void OSD_Scal( int HZoomIn, int HScal, int VZoomIn, int VScal) //define the horizontal and vertical scaling factor
{
   unsigned int value;

   value = *(volatile unsigned int *) (LCD_IO_Base + 0x2004);
   
   if (HZoomIn == 1) {  //Horizontal zoom in
      value &= ~(0x1f << 5);
      value |= (HScal << 7) | (1 << 5) | (1 << 6);
   } 
   else if (HZoomIn == 0) {   //Horizontal zoom out. ex. zoom in 0.5 = zoom out 2
      value &= ~(0x1f << 5);
      value |= (HScal << 9) | (1 << 5);
   }
   else { //disable horizontal direction zoom in/out mode
      printf("Disable horziontal zoom in/out\n");
      value &= ~(0x1f << 5);
   }

   if ( VZoomIn == 1 ) {   //Vertical zoom in
      value &= 0xffffffe0;
      value |= (VScal << 2) | (1 << 0) | (1 << 1);
   } else if ( VZoomIn == 0) {   //Vertical zoom out. ex. zoom in 0.5 = zoom out 2
      value &= 0xffffffe0;
      value |= (VScal << 4) | (1 << 0);
   }
   else {
      printf("Disable vertical zoom in/out\n");
      value &= 0xffffffe0; 
    }
    
   *(volatile unsigned int *) (LCD_IO_Base + 0x2004) = value;
}

//we can use this function only when there are all single fonts, or all MCF fonts
void OSD_putc( char c, int position, unsigned short foreground, unsigned short background )  //map the OSC_Font[], ie. set font index in the font attribute RAM
{
   if ((foreground > 15) || (background > 15)) {
      printf("OSD font color error!\n");
      return;
   }
   
   
 //   if( *(volatile  unsigned int * )( LCD_IO_Base + 0x203C ) == NO_MCF_FONT || *( unsigned int * )( LCD_IO_Base + 0x203C ) == 0 )
   {
      if( c >= '0' && c <= '9' )
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x20000 + position *4 )  = ( (c -'0') << 8) | (foreground << 4) | background;  
      else if( c>= 'A' && c <= 'Z') 
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x20000 + position *4 )  = ( ( c - 'A' +10) << 8) | (foreground << 4) | background;       
      else if( c == ' ' )
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x20000 + position *4 )  = ( ( 'Z' - 'A' +10 + 1) << 8) | (foreground << 4) | background; 
      else if( c == '=')
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x20000 + position *4 )  = ( ( 'Z' - 'A' +10 + 2) << 8) | (foreground << 4) | background;  
      else if( c == ',')
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x20000 + position *4 )  = ( ( 'Z' - 'A' +10 + 3) << 8) | (foreground << 4) | background;
/* }else{
      printf("MCF base != 0 or 0x3FFF\n");   
*/ }    
}


void OSD_puts( char *str, int position, unsigned short foreground, unsigned short background )
{
   int i;
  
   for(i = 0;i < strlen(str);i++)
      OSD_putc(*(str +i), position + i, foreground, background );
}

/* We use four bits to represent OSD window 1 ~ 4. For example, 0001(binary) -> OSD window 1 is on, while others are off. */
void OSD_On( unsigned int which )  //OSD Enable/disable
{
   if (which > 0xF) {
      printf("Wrong OSD window id\n");
      return;
   }
   *(volatile unsigned int *) LCD_IO_Base |= 0x10; //OSD windows on

   *(volatile unsigned int *)(LCD_IO_Base + 0x2000) &= 0xfffffff0;
   *(volatile unsigned int *)(LCD_IO_Base + 0x2000) |= which;  //enable the sub-window
}

void OSD_Off()  //OSD disable
{
   *(volatile unsigned int *) LCD_IO_Base &= 0xffffffef;
   *(volatile unsigned int *)(LCD_IO_Base + 0x2000) &= 0x0;
}

void OSD_Test_Complex(char *str)
{
   int i,j,x,y, k, ratio;
   char msg[64];
   unsigned char r, g, b;
   char mcf_used = TRUE;      //use MCF font???
   
   strcpy(msg, str);
   ratio = PANEL_WIDTH / 320;

// SetDivNo(Divisor);
// SetBGRSW( BGR );
   
   r=0xff;  g=0;  b=0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2040) = rgb2ycbcr( &r, &g, &b );  // Palette 0 = Red
   r=0;  g=0xff; b=0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2044) = rgb2ycbcr( &r, &g, &b );  // Palette 1 = Green 
   r=0;  g=0;  b=0xff;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2048) = rgb2ycbcr( &r, &g, &b );  // Palette 2 = Blue     
   r=36; g=16; b=16;
   *(volatile unsigned int *) (LCD_IO_Base + 0x204C) = rgb2ycbcr( &r, &g, &b );  // Palette 3 = similar Black  
   r=0xff;  g=0xff; b=0xff;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2050) = rgb2ycbcr( &r, &g, &b );  // Palette 4 = White 
   r=0xaa;  g=0;  b=0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2054) = rgb2ycbcr( &r, &g, &b );  // Palette 5 = Red
   r=0;  g=0xaa; b=0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2058) = rgb2ycbcr( &r, &g, &b );  // Palette 6 = Green
   r=0;  g=0;  b=0xaa;
   *(volatile unsigned int *) (LCD_IO_Base + 0x205C) = rgb2ycbcr( &r, &g, &b );  // Palette 7 = Blue
   r=0x45;  g=0x45;  b=0x45;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2060) = rgb2ycbcr( &r, &g, &b );  // Palette 8 = Black
   r=0x90;  g=0x90; b=0x90;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2064) = rgb2ycbcr( &r, &g, &b );  // Palette 9 = White
   r=0x55;  g=0;  b=0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2068) = rgb2ycbcr( &r, &g, &b );  // Palette 10 = Red
   r=0;  g=0x55;  b=0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x206C) = rgb2ycbcr( &r, &g, &b );  // Palette 11 = Green
   r=0;  g=0;  b=0x55;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2070) = rgb2ycbcr( &r, &g, &b );  // Palette 12 = Blue
   r=0x20;  g=0x50;  b=0x70;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2074) = rgb2ycbcr( &r, &g, &b );  // Palette 13 = Black
   r=0xff;  g=0xf; b=0xf0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2078) = rgb2ycbcr( &r, &g, &b );  // Palette 14 = White
   r=0x11;  g=0;  b=0;
   *(volatile unsigned int *) (LCD_IO_Base + 0x207C) = rgb2ycbcr( &r, &g, &b );  // Palette 15 = Red

   if(mcf_used == TRUE){
      OSD_MCFBase( 0 );
      for(i=0; i< 18* 10* 3 + 18* 26* 3 + 18* 1* 3; i++)
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x10000 + i*4 ) = OSD_MCFont[i];
   }else{
      OSD_MCFBase( (unsigned char *)NO_MCF_FONT );
      for(i=0; i< 18* 10 + 18* 26 + 18* 3  ; i++)   // 10 digit & 26 alphabet & 3 symbol
         *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x10000 + i*4 )  = OSD_Font[i];  // i*4 -> 1 word = 4 bytes
   }

/* //if you need to test MCFont, you can open this
   for(i=0; i<6*18; i++)   *(unsigned int *) (LCD_IO_Base + 0x10000 + i*4 )  = OSD_Font[i];  // i*4 -> 1 word = 4 bytes
   for(i=0; i< 18* 4* 3 + 18* 26* 3 + 18* 1* 3; i++)
      *(unsigned int *) (LCD_IO_Base + 0x10000 + (i+6*18)*4 ) = OSD_MCFont[i]; 
   OSD_MCFBase( 6 ); //indicate that the first 6 fonts is simple font, not MCF
*/

   for(i=0; i< 320  ; i++)
      *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x20000 + i*4 )  = OSD_Attribute[i];      

   OSD_puts(str, 0, 0, 1);    
 
   OSD_On(1);
   OSD_On(0xf);      //enable four OSD window  
   
   //parameter for OSD window 2
   OSD_Pos(2, 0, 0);
   OSD_Dim( 2, 4, 4);
   OSD_FontAttributeBase(2, 50);
   OSD_WinControl( 2, WIN_HL_SHADOW, 3, 2, 3, 11 );   
   OSD_Scal(1, 0, 1, 0 ); 
   OSD_puts("OSD WIN 2", 50, 0, 1);
   
   
   //parameter for OSD window 3
   OSD_Pos(3, 6, 2);
   OSD_Dim( 3, 4, 5);
   OSD_FontAttributeBase(3, 70);
   OSD_WinControl( 3, WIN_HL_BORDER, 3, 3, 3, 12 ); 
   OSD_puts("OSD WIN 3", 70, 0, 1);   
   
   //parameter for OSD window 4
   OSD_Pos(4, 11, 6);
   OSD_Dim( 4, 7, 3);
   OSD_FontAttributeBase(4, 90);
   OSD_WinControl( 4, WIN_HL_SHADOW, 3, 4, 3, 13 ); 
// OSD_puts("OSD WIN 4", 90, 0, 1);   
  
for(i=0; i<300000; i++);  
//OSD_Off();  
//    OSD_On(1);

      /*upscaling*/                  
   i=j=10;

   OSD_Dim( 1, i, j);
   OSD_Row_Col_Space(0,0);
// OSD_WinControl( 1, WIN_HL_BORDER, 1, 0, 1, 2 );
   OSD_FontAttributeBase(1, 0);
   OSD_FontControl( TRANSP_0, FONT_HL_NONE, 0, 1 );

   OSD_Scal(-1, 0, -1, 0 );


   OSD_Pos(1, 0, 0); 
// OSD_puts("OSD WIN 1", 0, 0, 1);
   for( x= 0, y=0 ; x< 16*ratio && y<13*ratio; x++, y++)     //Move from "top left" to "bottom right"
   {
      OSD_Pos(1, x,y);
      for(k=0; k<70000;k++);
   }  
    
   for( x= 15*ratio, y=0 ; x>=0 && y<13*ratio; x--, y++)     //Move from "top right" to "bottom left"
   {
      OSD_Pos(1, x,y);
      for(k=0; k<70000;k++);
   }  

   OSD_Pos(1, 0, 0); 
   
   OSD_Dim( 1, 4, 4);
   //consider single horizontal or vertical
   OSD_Scal(1, 1, -1, 0 );
   delay_10ms(50);

   OSD_Scal(1, 2, -1, 0 );
   delay_10ms(50);

   OSD_Scal(1, 3, -1, 0 );
   delay_10ms(50);
   
   OSD_Scal(-1, 0, 1, 1 );
   delay_10ms(50);

   OSD_Scal(-1, 0, 1, 2 );
   delay_10ms(50);

   OSD_Scal(-1, 0, 1, 3 );
   delay_10ms(50);

   OSD_Scal(1, 0, 1, 0 );
   delay_10ms(50);
 
   OSD_Scal(1, 1, 1, 1 );
   delay_10ms(50);
   
   OSD_Scal(1, 2, 1, 2 );
   delay_10ms(50);
      
   OSD_Scal(1, 3, 1, 3 );
   delay_10ms(50);
  

   OSD_Scal(-1, 0, -1, 0 );
   OSD_WinControl( 1, WIN_HL_BORDER, 3, 5, 0, 3 ); 
   delay_10ms(70);
   

   OSD_WinControl( 1, WIN_HL_SHADOW, 3, 2, 3, 11 );   
   delay_10ms(70);


   OSD_FontControl( TRANSP_50, FONT_HL_BORDER, 0, 1 );
   delay_10ms(70);


   OSD_FontControl( TRANSP_50, FONT_HL_SHADOW, 0, 1 );
   delay_10ms(70);  
     

   OSD_FontAttributeBase(1, 10);
   OSD_Scal(-1, 0, 1, 3);
   delay_10ms(70);
  
   
   OSD_Row_Col_Space(2,2);
   OSD_Scal(-1, 0, -1, 0 );
   delay_10ms(70);
   
   OSD_Row_Col_Space(0,0);
   OSD_Scal(-1, 0, -1, 0);
   for( i=2; i<=10; i++)
      for( j=2; j<=10; j++)   
      {
         OSD_Dim( 1, i, j);
         for(k=0; k<50000;k++);
      }         
     
   OSD_WinControl( 1, WIN_HL_NONE, 3, 5, 0, 3 );
   OSD_Off();   
   //downscaling
   
   OSD_On(1);
// OSD_On(0xf);
   i=j=10;
   OSD_Dim( 1, i, j);
   OSD_Row_Col_Space(0,0);
   OSD_WinControl( 1, WIN_HL_BORDER, 3, 11, 3, 1 ); 
   OSD_FontAttributeBase(1, 10); 
   

   OSD_Scal(0, 0, 0, 0 );
   for( x= 0, y=0 ; x< 16*ratio && y<13*ratio; x++, y++)     //Move from "top left" to "bottom right"
   {
      OSD_Pos(1, x,y);
      for(k=0; k<70000;k++);
   }  
    

   for( x= 15*ratio, y=0 ; x>=0 && y<13*ratio; x--, y++)     //Move from "top right" to "bottom left"
   {
      OSD_Pos(1, x,y);
      for(k=0; k<70000;k++);
   }  
     
    
   OSD_Pos(1, 0, 0);
   OSD_FontControl( TRANSP_100, FONT_HL_SHADOW, 2, 3 );
   delay_10ms(70);
   
   OSD_Row_Col_Space(5,5);
   delay_10ms(50);
   
   OSD_Scal(0, 1, 0, 1 );
   delay_10ms(50);
 
   OSD_WinControl( 1, WIN_HL_SHADOW, 0, 2, 0, 3 ); 
   delay_10ms(50);
   
   OSD_Scal(0, 0, 0, 1);  
   delay_10ms(50);
   
   OSD_Scal(0, 1, 0, 0);  
   delay_10ms(50);

 
   OSD_WinControl( 1, WIN_HL_NONE, 0, 2, 0, 3 );
   OSD_FontControl( TRANSP_100, FONT_HL_NONE, 2, 3 ); 


   OSD_Pos(1, 0, 0); 
   for( i=2; i<=10; i++)
      for( j=2; j<=10; j++)   
      {
         OSD_Dim( 1, i, j);
         for(k=0; k<5000;k++);
      }         
   OSD_Off();
}



/***************************** On Screen Display (OSD) Simple Version Test *****************************/
void OSD_On_Simple( unsigned char on)  //OSD Enable/disable
{
    *(volatile unsigned int *) (LCD_IO_Base) &= 0xffffffef; //OSD windows off
    *(volatile unsigned int *) (LCD_IO_Base) |= on << 4; //OSD windows on
}


void OSD_Pos_Simple( int  HPos, int VPos )
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x2004) = (HPos << 12) | VPos;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2000) |= 1 << 20;  //update OSD parameters
}   


void OSD_Dim_Simple( int HDim, int VDim ) //define the total font number of each row and the total row number
{
    volatile unsigned int value;

    value = *(volatile unsigned int *) (LCD_IO_Base + 0x2000);
    value &= 0x0000000f;   //clear the horizontal and vertical dimension of the OSD window
    value |= ((HDim << 12) | (VDim << 4));   //set horizontal and vertical dimension
    *(volatile unsigned int *) (LCD_IO_Base + 0x2000) = value;
   *(volatile unsigned int *) (LCD_IO_Base + 0x2000) |= 1 << 20;  //update OSD parameters
} 


void OSD_transparent_Simple( int level )
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x200C) &= 0xfffffffc;  //set OSD background transparency = 25%
   *(volatile unsigned int *) (LCD_IO_Base + 0x200C) |= level; //bit 5-4 :set OSD background transparency, 00=25%, 01=50%, 10=75%, 11=100%
}


void OSD_fg_color_Simple( int pal0, int pal1, int pal2, int pal3) //OSD foreground color control
{
    *(volatile unsigned int *) (LCD_IO_Base + 0x2008) = (pal0) | (pal1 <<8) | (pal2 << 16) | (pal3<< 24);   //palette entry 0~3
}


void OSD_bg_color_Simple( int pal1, int pal2, int pal3)
{
    volatile unsigned int value;
  
    value = *(volatile unsigned int *) (LCD_IO_Base + 0x200C);
    value &= 0x000000ff;
    value |= (pal1 <<8) | (pal2 << 16) | (pal3 << 24);
    *(volatile unsigned int *) (LCD_IO_Base + 0x200C) = value;
}


void OSD_Scal_Simple( int HScal, int VScal)  //define the horizontal and vertical up-scaling factor
{
    volatile unsigned int value;

    value = *(volatile unsigned int *) (LCD_IO_Base + 0x2000);
    value &= 0xfffffff0;
    value |= (HScal<<2) | VScal;
    *(volatile unsigned int *) (LCD_IO_Base + 0x2000) = value;
    *(volatile unsigned int *) (LCD_IO_Base + 0x2000) |= 1 << 20; //update OSD parameters
}


void OSD_putc_Simple( char c, int position)  //map the OSC_Font[]
{
    if( c >= '0' && c <= '9' )
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 + position *4 )  = ( (c -'0') << 4);  
    else if( c>= 'A' && c <= 'Z')  
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 + position *4 )  = ( ( c - 'A' +10) << 4); 

    if( c == ' ' )
        *(unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 + position *4 )  = ( ( 'Z' - 'A' +10 + 1) << 4); 

    if( c == '=')
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 + position *4 )  = ( ( 'Z' - 'A' +10 + 2) << 4);  

    if( c == ',')
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 + position *4 )  = ( ( 'Z' - 'A' +10 + 3) << 4);       
}


void OSD_puts_Simple( char *str, int position)
{
    int i;

    for(i=0; i< strlen(str); i++)
        OSD_putc_Simple( *(str +i), position +i );
}


void OSD_Test_Simple( int mode, char *str)
{
    int i,j,x,y;
    char *msg="                                        ";
//  unsigned char r, g, b;
    unsigned int x_row, y_col;
    unsigned int font_row, font_col;

    font_row=12;
    font_col=16;

    strcpy( msg, str);

    for(i=0; i< font_col*10 + font_col* 26 + 3* font_col; i++)   // 10 digit & 26 alphabet & 3 symbol
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0x8000 +i*4 ) = OSD_Font_Simple[i];  // i*4 -> 1 word = 4 bytes

    for(i=0; i< 100  ; i++)
        *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + 0xc000 +i*4 ) = OSD_Attribute_Simple[i];        

    OSD_puts_Simple(str, 0);   
//  Init_LCD( LCD_IO_Base, LcdModule, panel_type, 24);
//  ColorBar_RGB( 4, LBBP, pFrameBuffer, panel_width, panel_height, 0);

// SetBGRSW( BGR );

    if(mode == 2)   //YCbCr
    {
        OSD_fg_color_Simple( 0x57, 0x88, 0x3B, 0xFF);
        OSD_bg_color_Simple( 0x57, 0x88, 0x3B);
//      OSD_puts("YCBCR OSD   ", 0);    
    }
    else
    {
        OSD_fg_color_Simple( 0x07, 0x38, 0xC0, 0xFF);
        OSD_bg_color_Simple( 0x07, 0x38, 0xc0);
//    OSD_puts("RGB OSD ", 0);   
    }

    OSD_On_Simple(1);  
    OSD_transparent_Simple(1);

    x_row=5;
    y_col=5;
    OSD_Dim_Simple( x_row, y_col);
    
    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    OSD_Scal_Simple( 0,0 );
    y=5; 
    for(x=1; x< PANEL_WIDTH-(font_row*x_row+5); x++)  //Move from "top left" to "top right"
    {
        OSD_Pos_Simple( x,y);       
        delay_10ms(1);
    }
    delay_10ms(100);

    OSD_Scal_Simple( 1,1 );
    x= PANEL_WIDTH-(font_row*x_row*2+5);
    for(y=1; y<PANEL_HEIGHT-(font_col*y_col*2+1); y++) //Move From "top right" to "bottom right"
    {
        OSD_Pos_Simple( x,y);        
        delay_10ms(1);
    }   
    delay_10ms(100);    
    
    if (LCD_PANEL_TYPE<=4) { // not TV        
       OSD_Scal_Simple(2,2);    
       y=PANEL_HEIGHT-(font_col*y_col*3+1);
       for(x= PANEL_WIDTH-(font_row*x_row*3+1);x>0; x--) //Move from "bottom right" to "bottom left"   
       {
          OSD_Pos_Simple( x,y);                 
          delay_10ms(1);
       }       
    }
    else {
       OSD_Scal_Simple(0,1);    
       y=PANEL_HEIGHT-(font_col*y_col*2+1);
       for(x= PANEL_WIDTH-(font_row*x_row+5);x>0; x--) //Move from "bottom right" to "bottom left"   
       {
          OSD_Pos_Simple( x,y);                 
          delay_10ms(1);
       }    
    }    
    delay_10ms(100);    

    OSD_Scal_Simple(3,3); 
    x= 1;
    for(y=PANEL_HEIGHT-(font_col*y_col*4+2);y>=0;y--) //Move from "bottom left" to "top left"
    {
        OSD_Pos_Simple( x,y);
        delay_10ms(1);
    }    
    delay_10ms(100);    

    OSD_Scal_Simple( 0, 3); 
    x=1;
    y=1; 
    while ((x<PANEL_WIDTH-(font_row*x_row+2)) && 
           (y<PANEL_HEIGHT-(font_col*y_col*4+2))) //Move from "top left" to "bottom right"
    {
        OSD_Pos_Simple(x,y);
        delay_10ms(1);
        x++;
        y++;
    }    
    delay_10ms(100);    

    OSD_Scal_Simple(3, 0); 
    OSD_transparent_Simple(2); 
    x=PANEL_WIDTH-(font_row*x_row*4+2);
    y=1;     
    while ((x>0) && (y<=PANEL_HEIGHT-(font_col*y_col+2)))   //Move from "top right" to "bottom left"
    {   
        OSD_Pos_Simple(x,y);
        delay_10ms(1);
        x--;
        y++;
    }    
    delay_10ms(100);    

    OSD_Scal_Simple( 0,0 );
    OSD_transparent_Simple( 3); 
    OSD_Pos_Simple( 10, 10); 
    for( i=3; i<=11; i++) {
        for( j=3; j<=11; j++)   
        {
            OSD_Dim_Simple( i, j);
            delay_10ms(1);
        }
    }
    delay_10ms(100);            

   OSD_On_Simple(0); //OSD off

#if 1 //for special check if OSD can still show on screen after continuous turn on/off
   OSD_Pos_Simple( 0x50,0x30);
   for(i=0; i<=3000; i++)
      OSD_On_Simple(i%2);
      
    delay_10ms(300);    
#endif
}

void OSD_Test( char *str)
{
#ifdef SIMPLE_OSD
   OSD_Test_Simple(2, str);
#elif defined(COMPLEX_OSD)
   OSD_Test_Complex(str);
#endif
}

