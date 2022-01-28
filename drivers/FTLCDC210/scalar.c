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

/******************************************************************************
 * Define Constants
 *****************************************************************************/


extern unsigned int PANEL_WIDTH, PANEL_HEIGHT;
extern LCDMTYPE_T FLcdModule[];
extern unsigned int LCD_PANEL_TYPE;
extern char FILENAME[];
extern char DEFAULT_PATH[];

/******************************************************************************
 *Public functions
 *****************************************************************************/

/*************************** Scalar Test **************************************/
void Scalar_On()
{
    unsigned int temp;
    
    temp = *(volatile unsigned int *)(LCD_IO_Base + 0x04);
    temp = temp | (0x3 << 9);
    *(volatile unsigned int *)(LCD_IO_Base + 0x04) = temp;
    
    temp = *(volatile unsigned int *)(LCD_IO_Base + 0x10);
    while ((temp & 0x04)==0)
       temp = *(volatile unsigned int *)(LCD_IO_Base + 0x10);
       
    *(volatile unsigned int *)(LCD_IO_Base + 0x0c) = 0x04;    
        
   *(volatile unsigned int *) LCD_IO_Base &= 0xffffffdf; //scalar off
   *(volatile unsigned int *) LCD_IO_Base |= 0x00000020; //scalar on
}


void Scalar_Off()
{
    unsigned int temp;
    
    temp = *(volatile unsigned int *)(LCD_IO_Base + 0x04);
    temp = temp | (0x3 << 9);
    *(volatile unsigned int *)(LCD_IO_Base + 0x04) = temp;
    
    temp = *(volatile unsigned int *)(LCD_IO_Base + 0x10);
    while ((temp & 0x04)==0)
       temp = *(volatile unsigned int *)(LCD_IO_Base + 0x10);
       
    *(volatile unsigned int *)(LCD_IO_Base + 0x0c) = 0x04; 
    
   *(volatile unsigned int *) LCD_IO_Base &= 0xffffffdf; 
   *(volatile unsigned int *) LCD_IO_Base |= 0x00000000; //scalar off
}


/*Hor_in & Ver_in: 1~2047, Hor_out & Ver_out: 1~4096*/
void Resolution( unsigned int Hor_in, unsigned int Ver_in, unsigned int Hor_out, unsigned int Ver_out) //set the resolution of input and output image
{
   if( Hor_in > 2047 || Ver_in > 2047 || Hor_out > 4096 || Ver_out > 4096 )
   {
      printf("Scalar resolution error\n");
      return;
   }

   *(volatile unsigned int *) (LCD_IO_Base + 0x1100) &= 0xfffff000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1100) |= ( Hor_in - 1 );    //Horizontal input resolution needs to substract by one
   
   *(volatile unsigned int *) (LCD_IO_Base + 0x1104) &= 0xfffff000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1104) |= ( Ver_in - 1 );    //Vertical input resolution needs to substract by one

   *(volatile unsigned int *) (LCD_IO_Base + 0x1108) &= 0xffffc000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1108) |= Hor_out;

   *(volatile unsigned int *) (LCD_IO_Base + 0x110C) &= 0xffffc000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x110C) |= Ver_out;  
}


void Scalar_Resolution( unsigned int Hor_in, unsigned int Ver_in, unsigned int Hor_out, unsigned int Ver_out) //set the resolution of input and output image
{
   unsigned int Cpu_wr_ver_num_r, Cpu_wr_hor_num_r;
   
   if( Hor_in > 2047 || Ver_in > 2047 || Hor_out > 4096 || Ver_out > 4096 )
   {
      printf("Scalar resolution error\n");
      return;
   }
   
   //fill in the coefficients of the 2nd-stage scalar, partial display is always scaling up
   if (Ver_in <= Ver_out)  //up-scaling
      Cpu_wr_ver_num_r = Ver_in * 256 / Ver_out; 
   else //( Ver_in > Ver_out )   //down-scaling
      Cpu_wr_ver_num_r = (Ver_in % Ver_out) * 256 / Ver_in;   
  
   if (Hor_in <= Hor_out)  //up-scaling
      Cpu_wr_hor_num_r = Hor_in *256 / Hor_out; 
   else //if( Hor_in > Hor_out ) //down-scaling
      Cpu_wr_hor_num_r = (Hor_in % Hor_out) * 256 / Hor_out;
   
    if (Cpu_wr_ver_num_r >= 0xFF)
       Cpu_wr_ver_num_r = 0xFF;
    if (Cpu_wr_hor_num_r >= 0xFF)
       Cpu_wr_hor_num_r = 0xFF;         
   printf("Resolution...%x...%x..\n",Cpu_wr_ver_num_r,Cpu_wr_hor_num_r);
   *(volatile unsigned int *) (LCD_IO_Base + 0x112C) &= 0xffff0000;
   *(volatile unsigned int *) (LCD_IO_Base + 0x112C) |= ( Cpu_wr_ver_num_r ) | ( Cpu_wr_hor_num_r << 8 );   //User define 2nd-stage horizontal coefficient  
}


//define the upscaling thresholds, all threshold 0~511
void Scalar_UPThreshold( unsigned int Hor_high_th, unsigned int Hor_low_th, unsigned int Ver_high_th, unsigned int Ver_low_th )
{
   if( Hor_high_th > 511 || Hor_low_th > 511 || Ver_high_th > 511 || Ver_low_th > 511 )
   {
      printf("Scalar upscaling threshold error\n");
      exit(-1);
   }

   *(volatile unsigned int *) (LCD_IO_Base + 0x1114) &= 0xfffffe00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1114) |= Hor_high_th; //The real value equal to Hor_high_th/256
      
   *(volatile unsigned int *) (LCD_IO_Base + 0x1118) &= 0xfffffe00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1118) |= Hor_low_th;  //The real value equal to Hor_low_th/256

   *(volatile unsigned int *) (LCD_IO_Base + 0x111C) &= 0xfffffe00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x111C) |= Ver_high_th; //The real value equal to Ver_high_th/256

   *(volatile unsigned int *) (LCD_IO_Base + 0x1120) &= 0xfffffe00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1120) |= Ver_low_th;  //The real value equal to Ver_low_th/256

}

/*
//define the 2-stage scalar horizontal and vertical coefficients
void Scalar_2Coeff( unsigned int Cpu_wr_ver_num_r, unsigned int Cpu_wr_hor_num_r)
{
   if( Hor_user_coeff > 511 || Ver_user_coeff > 511 )
      printf("Scalar upscaling user-defined coefficient error\n");

   *(volatile unsigned int *) (LCD_IO_Base + 0x1124) &= 0xfffffe00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1124) |= Hor_user_coeff; //User define upscaling horizontal coefficient
      
   *(volatile unsigned int *) (LCD_IO_Base + 0x1128) &= 0xfffffe00;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1128) |= Ver_user_coeff; //User define upscaling vertical coefficient 
}*/

//2nd-stage related. 0: nearly bilinear mode, 1: threshold nearly bilinear mode, 2: most neighborhood mode, 3: reserved 
void Scalar_Interpolation( unsigned int Hor_inter_mode, unsigned int Ver_inter_mode )
{
    unsigned int temp,i;
    
   if( Hor_inter_mode > 3 || Ver_inter_mode > 3 )
   {
      printf("Scalar interpolation error\n");
      exit(-1);
   }
    
   temp = *(volatile unsigned int *) (LCD_IO_Base);
   *(volatile unsigned int *) (LCD_IO_Base) = 0x00;
         
   for (i=0;i<10;i++)
      ;     
   *(volatile unsigned int *) (LCD_IO_Base + 0x1110) &= 0xffffffe1;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1110) |= (Hor_inter_mode << 3) | (Ver_inter_mode << 1);   
   
   *(volatile unsigned int *) (LCD_IO_Base) = temp;   
}  


//if enabled, the center area will be zoomed in
void Scalar_PartialDisplay( int on )
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x1110) &= 0xffffffdf;
   if( on != 0)
   {
      *(volatile unsigned int *) (LCD_IO_Base + 0x1110) |= (1 << 5); 
   }
}  


//bypass the 2nd stage automatic scalar (from (1/2) * (1/2) to 2 * 2 )
void Scalar_Bypass2( int on )
{
   *(volatile unsigned int *) (LCD_IO_Base + 0x1110) &= 0xfffffffe;
   if( on != 0)
      *(volatile unsigned int *) (LCD_IO_Base + 0x1110) |= 0x00000001;  
}


//the scaling ratio of 1st stage scalar. 000:bypass 1st stage scalar, 001:(1/2) * (1/2),......, 111:(1/128) *(1/128)
void Scalar_DownRatio( unsigned int ratio )
{
   if( ratio > 7 )   printf("Scaling down ratio error\n");
   
   *(volatile unsigned int *) (LCD_IO_Base + 0x1110) &= 0xfffffe3f;
   *(volatile unsigned int *) (LCD_IO_Base + 0x1110) |= (ratio << 6);   
}  


//use the non-managed memory space, need especially noticed!!!!!!!!!!!!!
unsigned char *allocateLargeBuffer(unsigned int offset_addr )  
{ 
   return ((unsigned char *)(uintptr_t)(0x2000000 + offset_addr)); 
}


void TV_Scalar_Test(unsigned char *pFrameBuffer, unsigned char bpp)
{
   unsigned char *pBuffer;
   int i;
   pBuffer =  pFrameBuffer; //allocateLargeBuffer(0);
   for(i=0; i<PANEL_WIDTH*2*PANEL_HEIGHT*2 *4; i+=4)
      *(volatile unsigned int *) (pBuffer + i) = 0xffffffff;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);
      
   *(volatile unsigned int *) (LCD_IO_Base + 0x0204) = 0x2;
   *(volatile unsigned int *) (LCD_IO_Base + 0x0014) = 0x100;//0x100->1/2x1/2 x200->1/2x1
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){//speed up AHB bus to 50MHz
      strcat(FILENAME, "80_888_1440x960.bmp.bin");
      Load_Image( FILENAME, 1382400, pBuffer, LBLP, 24 );//1382400
   }else if(bpp == 16){
      //*(volatile unsigned int *) (LCD_IO_Base + 0x204) |= 0x4;
      strcat(FILENAME, "82_565_1440x960.bmp.bin");
      Load_Image( FILENAME, 2764800, pBuffer, LBLP, 16 );
      //Load_Image( FILENAME, 2764800, pBuffer, LBLP, 16 );//tv interlace scalar
   }
   delay_10ms(500);
   SetFrameBase(0,0,pBuffer);    //remember to restore framebuffer to pFrameBuffer if going on Scalar_Test()

}

void demo_delay()
{   
   Scalar_On();
    reset_lcd_controller();      
   delay_10ms(300);
   Scalar_Off();
}
   
void Scalar_Test_720_576(unsigned char *pFrameBuffer, unsigned char bpp)
{  
   int i;
   unsigned char *pBuffer;
   
   pBuffer = pFrameBuffer; 
   setNumFrameBuffer(1,0,0,0);
   SetFrameBase(0,0,pBuffer);
   printf("pBuffer allocated in Scalar_Test, address = 0x%x \n", (unsigned int)(uintptr_t)pBuffer);
   
   for( i=0; i<PANEL_WIDTH*2*PANEL_HEIGHT*2 *4; i+=4)
      *(volatile unsigned int *)(uintptr_t)(pBuffer + i) = 0xffffffff;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);

    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00;
   
   Resolution(720, 576, 720, 576);     
   Scalar_Resolution(720, 576, 720, 576);
   Scalar_DownRatio(0);    
   Scalar_Bypass2(1);         //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );

   //bypass the 1st and 2nd stage scalar
   printf("1st stage scalar RGB%d LBLP 720x576 -> 720x576 Test\n", (bpp==24)?888:565);

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "77_888_720x576.bmp.bin");
      Load_Image( FILENAME, 1658880, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "77_565_720x576.bmp.bin");
      Load_Image( FILENAME, 829440, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //down scaling from 1440x1152 -> 720x576
   printf("1st stage scalar RGB%d LBLP 1440x1152 -> 720x576 Test\n", (bpp==24)?888:565);
   
   Resolution(1440, 1152, 720, 576);   
   Scalar_Resolution(1440, 1152, 720, 576);
   Scalar_DownRatio(1);
   Scalar_Bypass2(1);         //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "78_888_1440x1152.bmp.bin");
      Load_Image( FILENAME, 6635520, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "78_565_1440x1152.bmp.bin");
      Load_Image( FILENAME, 3317760, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //bypass the 1st stage scalar and enable the 2nd stage scalar
   printf("2nd stage scalar RGB%d LBLP 720x576 -> 720x576 Test\n", (bpp==24)?888:565);
   
   Resolution(720, 576, 720, 576);  
   Scalar_Resolution(720, 576, 720, 576);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );
   
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "79_888_720x576.bmp.bin");
      Load_Image( FILENAME, 1658880, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "79_565_720x576.bmp.bin");
      Load_Image( FILENAME, 829440, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //2nd stage scalar, up scaling from 640x480 -> 720x576
   printf("2nd stage scalar RGB%d LBLP 640x480 -> 720x576 Test\n", (bpp==24)?888:565);
   
   Resolution(640, 480, 720, 576);  
   Scalar_Resolution(640, 480, 720, 576);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );
      
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "80_888_640x480.bmp.bin");
      Load_Image( FILENAME, 1228800, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "80_565_640x480.bmp.bin");
      Load_Image( FILENAME, 614400, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   printf("YCbCr420 with 2nd stage scaling 640x480 -> 720x576 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "71_420_640x480.bmp.yuv");
   Load_Image_YCbCr420( FILENAME, 640, 480, 460800, pBuffer, LBLP );
    demo_delay(); 
         
   printf("YCbCr422 with 2nd stage scaling 640x480 -> 720x576 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "49_422_640x480.bmp.yuv");
   Load_Image_YCbCr422( FILENAME, 614400, pBuffer, LBLP );
    demo_delay(); 

   Scalar_DownRatio(0);

#if 0
   //bypass the 1st stage scalar and do partial display
   printf("Scalar partial display RGB%d LBLP 720x576 -> 720x576 Test\n", (bpp==24)?888:565);

   Resolution(720, 576, 720, 576);
   Scalar_Resolution(720, 576, 720, 576);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 
   Scalar_PartialDisplay(1);

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "81_888_720x576.bmp.bin");

      (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))++;   //VBP+1, to fix RGB888 partial the upper line problem
      Load_Image( FILENAME, 1658880, pBuffer, LBLP, 24 );
      (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))--;   //VBP-1, reload the original VBP value
   }else if(bpp == 16){
      strcat(FILENAME, "81_565_720x576.bmp.bin");
      Load_Image( FILENAME, 829440, pBuffer, LBLP, 16 );
   }

    demo_delay(); 
   Scalar_PartialDisplay(0);
#endif

    printf("Do Threshold neerly bilinear mode\n");
   Scalar_Interpolation( 1, 1 );
    demo_delay(); 

    printf("Do Most neighberhood mode\n");
   Scalar_Interpolation( 2, 2 );
    demo_delay(); 

   *(volatile unsigned int *) (LCD_IO_Base) = 0x00;   
    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00; 
}


void Scalar_Test_720_480(unsigned char *pFrameBuffer, unsigned char bpp)
{  
   int i;
   unsigned char *pBuffer;
   
   pBuffer = pFrameBuffer; 
   setNumFrameBuffer(1,0,0,0);
   SetFrameBase(0,0,pBuffer);
   printf("pBuffer allocated in Scalar_Test, address = 0x%x \n", (unsigned int)(uintptr_t)pBuffer);
   
   for( i=0; i<PANEL_WIDTH*2*PANEL_HEIGHT*2 *4; i+=4)
      *(volatile unsigned int *)(uintptr_t)(pBuffer + i) = 0xffffffff;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);

    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00;

   //bypass the 1st and 2nd stage scalar
   printf("1st stage scalar RGB%d LBLP 720x480 -> 720x480 Test\n", (bpp==24)?888:565);

   Resolution(720, 480, 720, 480);
   Scalar_Resolution(720, 480, 720, 480);
   //Scalar_Resolution(720, 576, 720, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(1);         //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );
      
   //for TV scalar start
   //*(volatile unsigned int *) (LCD_IO_Base + 0x0204) = 0x2;
   //*(volatile unsigned int *) (LCD_IO_Base + 0x0014) = 0x100;//0x100->1/2x1/2 x200->1/2x1
   //for TV end
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "82_888_720x480.bmp.bin");
      Load_Image( FILENAME, 1382400, pBuffer, LBLP, 24 );//1382400
   }else if(bpp == 16){
      //*(volatile unsigned int *) (LCD_IO_Base + 0x204) |= 0x4;
      strcat(FILENAME, "82_565_720x480.bmp.bin");
      Load_Image( FILENAME, 691200, pBuffer, LBLP, 16 );
      //Load_Image( FILENAME, 2764800, pBuffer, LBLP, 16 );//tv interlace scalar
   }
    demo_delay(); 

   //down scaling from 1440x960 -> 720x480
   printf("1st stage scalar RGB%d LBLP 1440x960 -> 720x480 Test\n", (bpp==24)?888:565);
   
   Resolution(1440, 960, 720, 480); 
   Scalar_Resolution(1440, 960, 720, 480);
   Scalar_DownRatio(1);
   Scalar_Bypass2(1);         //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "81_888_1440x960.bmp.bin");
      Load_Image( FILENAME, 5529600, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "81_565_1440x960.bmp.bin");
      Load_Image( FILENAME, 2764800, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //bypass the 1st stage scalar and enable the 2nd stage scalar
   printf("2nd stage scalar RGB%d LBLP 720x480 -> 720x480 Test\n", (bpp==24)?888:565);

   Resolution(720, 480, 720, 480);
   Scalar_Resolution(720, 480, 720, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "80_888_720x480.bmp.bin");
      Load_Image( FILENAME, 1382400, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "80_565_720x480.bmp.bin");
      Load_Image( FILENAME, 691200, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //2nd stage scalar, up scaling from 500x400 -> 720x480
   printf("2nd stage scalar RGB%d LBLP 500x400 -> 720x480 Test\n", (bpp==24)?888:565);
   
   Resolution(500, 400, 720, 480);  
   Scalar_Resolution(500, 400, 720, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );
   
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "79_888_500x400.bmp.bin");
      Load_Image( FILENAME, 800000, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "79_565_500x400.bmp.bin");
      Load_Image( FILENAME, 400000, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //1st & 2nd stage scalar, down scaling from 720x576 -> 360x288 -> 720x480
   printf("1st & 2nd stage scalar RGB%d LBLP 720x576 -> 360x288 -> 720x480 Test\n", (bpp==24)?888:565);
   
   Resolution(720, 576, 640, 480);  
   Scalar_Resolution(360, 288, 640, 480);
   Scalar_DownRatio(1);    //if you disable 1st stage scaler, only 2nd stage scaler is used, scaling from 720x576 -> 640x480
   Scalar_Bypass2(0);         //enable the 2nd stage scalar

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_720x576.bmp.bin");
      Load_Image( FILENAME, 1658880, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_720x576.bmp.bin");
      Load_Image( FILENAME, 829440, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   printf("YCbCr420 with 1st & 2nd stage scaling 720x576 -> 360x288 -> 720x480 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "14_420_720x576.bmp.yuv");
   Load_Image_YCbCr420( FILENAME, 720, 576, 622080, pBuffer, LBLP );
    demo_delay(); 
         
   printf("YCbCr422 with 1st & 2nd stage scaling 720x576 -> 360x288 -> 720x480 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "21_422_720x576.bmp.yuv");
   Load_Image_YCbCr422( FILENAME, 829440, pBuffer, LBLP );
    demo_delay(); 

   Scalar_DownRatio(0);
#if 0
   //bypass the 1st stage scalar and do partial display
   printf("Scalar partial display RGB%d LBLP 720x480 -> 720x480 Test\n", (bpp==24)?888:565);

   Resolution(720, 480, 720, 480);
   Scalar_Resolution(720, 480, 720, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 
   Scalar_PartialDisplay(1);

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "78_888_720x480.bmp.bin");

      (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))++;   //VBP+1, to fix RGB888 partial the upper line problem
      Load_Image( FILENAME, 1382400, pBuffer, LBLP, 24 );
      (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))--;   //VBP-1, reload the original VBP value
   }else if(bpp == 16){
      strcat(FILENAME, "78_565_720x480.bmp.bin");
      Load_Image( FILENAME, 691200, pBuffer, LBLP, 16 );
   }

    demo_delay(); 
   Scalar_PartialDisplay(0);
#endif

    printf("Do Threshold neerly bilinear mode\n");
   Scalar_Interpolation( 1, 1 );
    demo_delay(); 

    printf("Do Most neighberhood mode\n");
   Scalar_Interpolation( 2, 2 );
    demo_delay(); 

   *(volatile unsigned int *) (LCD_IO_Base) = 0x00;   
    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00;    
}

void Scalar_Test_640_480(unsigned char *pFrameBuffer, unsigned char bpp)
{  
   int i;
   unsigned char *pBuffer;
   
   pBuffer = pFrameBuffer; 
   setNumFrameBuffer(1,0,0,0);
   SetFrameBase(0,0,pBuffer);
   printf("pBuffer allocated in Scalar_Test, address = 0x%x \n", (unsigned int)(uintptr_t)pBuffer);
   
   for( i=0; i<PANEL_WIDTH*2*PANEL_HEIGHT*2 *4; i+=4)
      *(volatile unsigned int *) (pBuffer + i) = 0xffffffff;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);

    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00;
      
#ifdef SIMPLE_SCALARUP_TEST
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "16_888_320x240.bin");
      Load_Image( FILENAME, 307200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "16_565_320x240.bin");
      Load_Image( FILENAME, 153600, pBuffer, LBLP, 16 );
   }

   *(volatile  unsigned int *) (LCD_IO_Base + 0x0000 )|= (0x1 << 5);
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1100 )= 319;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1104 )= 239;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1108 )= 640;   //originally 639
   *(volatile  unsigned int *) (LCD_IO_Base + 0x110c )= 480;   //originally 479
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1110 )= 0;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x112c )= 0x8080;
   enable_lcd_controller();
#endif                  
   //bypass the 1st and 2nd stage scalar
   printf("1st stage scalar RGB%d LBLP 640x480 -> 640x480 Test\n", (bpp==24)?888:565);

   Resolution(640, 480, 640, 480);
   Scalar_Resolution(640, 480, 640, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(1);         //bypass the 2nd stage scalar       
   Scalar_Interpolation( 0, 0 );       
   //for TV scalar start
   // *(volatile unsigned int *) (LCD_IO_Base + 0x0204) = 0x2;
   // *(volatile unsigned int *) (LCD_IO_Base + 0x0014) = 0x100;//0x100->1/2x1/2 x200->1/2x1
   //for TV end               
         
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_640x480.bmp.bin");
      Load_Image( FILENAME, 4915200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_640x480.bmp.bin");
      Load_Image( FILENAME, 614400, pBuffer, LBLP, 16 );
   }
    demo_delay();    

   //down scaling from 1280x960 -> 640x480
    printf("1st stage scalar RGB%d LBLP 1280x960 -> 640x480 Test\n", (bpp==24)?888:565);
   
   Resolution(1280, 960, 640, 480); 
   Scalar_Resolution(1280, 960, 640, 480);
   Scalar_DownRatio(1);
   Scalar_Bypass2(1);         //bypass the 2nd stage scalar       
   Scalar_Interpolation( 0, 0 );          
         
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){          
      strcat(FILENAME, "76_888_1280x960.bmp.bin");
      Load_Image( FILENAME, 4915200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "76_565_1280x960.bmp.bin");
      Load_Image( FILENAME, 2457600, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //printf("YCbCr420 with 1st stage scaling down Test\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "4color_420_1280x960.bmp.yuv");
   //Load_Image_YCbCr420( FILENAME, 1280, 960, 1843200, pBuffer, LBLP );
    //demo_delay();

   printf("YCbCr420 with 1st stage scaling down Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "77_420_1280x960.yuv");
   Load_Image_YCbCr420( FILENAME, 1280, 960, 1843200, pBuffer, LBLP );
    demo_delay(); 

   //printf("YCbCr422 with 1st stage scaling down Test\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "4color_422_1280x960.bmp.yuv");
   //Load_Image_YCbCr422( FILENAME, 2457600, pBuffer, LBLP );
    //demo_delay();

   printf("YCbCr422 with 1st stage scaling down Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "78_422_1280x960.bmp.yuv");
   Load_Image_YCbCr422( FILENAME, 2457600, pBuffer, LBLP );
    demo_delay(); 

   //bypass the 1st stage scalar and enable the 2nd stage scalar
   printf("2nd stage scalar RGB%d LBLP 640x480 -> 640x480 Test\n", (bpp==24)?888:565);

   Resolution(640, 480, 640, 480);
   Scalar_Resolution(640, 480, 640, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );                   

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_640x480.bmp.bin");
      Load_Image( FILENAME, 1228800, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_640x480.bmp.bin");
      Load_Image( FILENAME, 614400, pBuffer, LBLP, 16 );
   }
    demo_delay();     

   //2nd stage scalar, up scaling from 320x240 -> 640x480
   printf("2nd stage scalar RGB%d LBLP 320x240 -> 640x480 Test\n", (bpp==24)?888:565);
   
   Resolution(320, 240, 640, 480);  
   Scalar_Resolution(320, 240, 640, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );                   

   strcpy(FILENAME, DEFAULT_PATH);        
   if(bpp == 24){
      strcat(FILENAME, "16_888_320x240.bin");
      Load_Image( FILENAME, 307200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "16_565_320x240.bin");
      Load_Image( FILENAME, 153600, pBuffer, LBLP, 16 );
   }
    demo_delay(); 
    //reset_lcd_controller();

   printf("YCbCr420 with 2nd stage scaling 320x240 -> 640x480 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "46_420_320x240.bmp.yuv");
   Load_Image_YCbCr420( FILENAME, 320, 240, 115200, pBuffer, LBLP );
    demo_delay(); 
    //reset_lcd_controller();    

   printf("YCbCr422 with 2nd stage scaling 320x240 -> 640x480 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "38_422_320x240.bmp.yuv");
   Load_Image_YCbCr422( FILENAME, 153600, pBuffer, LBLP );
    demo_delay(); 
    //reset_lcd_controller();    

   //2nd stage scalar, up scaling from 500x400 -> 640x480
   printf("2nd stage scalar RGB%d LBLP 500x400 -> 640x480 Test\n", (bpp==24)?888:565);

   Resolution(500, 400, 640, 480);  
   Scalar_Resolution(500, 400, 640, 480);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar 
            
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "79_888_500x400.bmp.bin");
      Load_Image( FILENAME, 800000, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "79_565_500x400.bmp.bin");
      Load_Image( FILENAME, 400000, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //2nd stage scalar, down scaling from 1280x960 -> 640x480
   printf("2nd stage scalar RGB%d LBLP 1280x960 -> 640x480 Test\n", (bpp==24)?888:565);
   
   Resolution(1280, 960, 640, 480); 
   Scalar_Resolution(1280, 960, 640, 480);
   Scalar_DownRatio(0);    //if you disable 1st stage scaler, only 2nd stage scaler is used, scaling from 720x576 -> 640x480
   Scalar_Bypass2(0);         //enable the 2nd stage scalar
            
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){          
      strcat(FILENAME, "76_888_1280x960.bmp.bin");
      Load_Image( FILENAME, 4915200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "76_565_1280x960.bmp.bin");
      Load_Image( FILENAME, 2457600, pBuffer, LBLP, 16 );
   }
    demo_delay(); 
                     
   //printf("YCbCr420 with 2nd stage scaling down Test\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "4color_420_1280x960.bmp.yuv");
   //Load_Image_YCbCr420( FILENAME, 1280, 960, 1843200, pBuffer, LBLP );
    //demo_delay();

   printf("YCbCr420 with 2nd stage scaling down Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "81_420_1280x960.yuv");
   Load_Image_YCbCr420( FILENAME, 1280, 960, 1843200, pBuffer, LBLP );
    demo_delay(); 
         
   //printf("YCbCr422 with 2nd stage scaling down Test\n");
   //strcpy(FILENAME, DEFAULT_PATH);
   //strcat(FILENAME, "4color_422_1280x960.bmp.yuv");
   //Load_Image_YCbCr422( FILENAME, 2457600, pBuffer, LBLP );
    //demo_delay();

   printf("YCbCr422 with 2nd stage scaling down Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "82_422_1280x960.bmp.yuv");
   Load_Image_YCbCr422( FILENAME, 2457600, pBuffer, LBLP );
    demo_delay(); 

   //1st & 2nd stage scalar, down scaling from 720x576 -> 360x288 -> 640x480
   printf("RGB%d LBLP with 1st & 2nd stage scaling 720x576 -> 360x288 -> 640x480 Test\n", (bpp==24)?888:565);
   
   Resolution(720, 576, 640, 480);  
   Scalar_Resolution(360, 288, 640, 480);
   Scalar_DownRatio(1);    //if you disable 1st stage scaler, only 2nd stage scaler is used, scaling from 720x576 -> 640x480
   Scalar_Bypass2(0);         //enable the 2nd stage scalar
            
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_720x576.bmp.bin");
      Load_Image( FILENAME, 1658880, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_720x576.bmp.bin");
      Load_Image( FILENAME, 829440, pBuffer, LBLP, 16 );
   }
    demo_delay();    
   
   printf("YCbCr420 with 1st & 2nd stage scaling 720x576 -> 360x288 -> 640x480 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "39_420_720x576.bmp.yuv");
   Load_Image_YCbCr420( FILENAME, 720, 576, 622080, pBuffer, LBLP );
    demo_delay(); 
   
   printf("YCbCr422 with 1st & 2nd stage scaling 720x576 -> 360x288 -> 640x480 Test\n");
   strcpy(FILENAME, DEFAULT_PATH);
   strcat(FILENAME, "46_422_720x576.bmp.yuv");
   Load_Image_YCbCr422( FILENAME, 829440, pBuffer, LBLP );
    demo_delay();  

    printf("Do Threshold neerly bilinear mode\n");
   Scalar_Interpolation( 1, 1 );
    demo_delay();

    printf("Do Most neighberhood mode\n");
   Scalar_Interpolation( 2, 2 ); 
    demo_delay();

   *(volatile unsigned int *) (LCD_IO_Base) = 0x00;   
    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00;       
}

void Scalar_Test_320_240(unsigned char *pFrameBuffer, unsigned char bpp)
{  
   int i;
   unsigned char *pBuffer;
   
   pBuffer = pFrameBuffer; //allocateLargeBuffer(0);
   setNumFrameBuffer(1,0,0,0);
   SetFrameBase(0,0,pBuffer);
   printf("pBuffer allocated in Scalar_Test, address = 0x%x \n", (unsigned int)(uintptr_t)pBuffer);
   
   for( i=0; i<PANEL_WIDTH*2*PANEL_HEIGHT*2 *4; i+=4)
      *(volatile unsigned int *) (pBuffer + i) = 0xffffffff;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);

    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00;

   //bypass the 1st and 2nd stage scalar
   printf("1st stage scalar RGB%d LBLP 320x240 -> 320x240 Test\n", (bpp==24)?888:565);
   
   Resolution(320, 240, 320, 240);  
   Scalar_Resolution(320, 240, 320, 240);
   //Scalar_Resolution(352, 288, 320, 240);//for test
   Scalar_DownRatio(0);
   Scalar_Bypass2(1);   //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 );
   
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_320x240.bmp.bin");
      Load_Image( FILENAME, 307200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_320x240.bmp.bin");
      Load_Image( FILENAME, 153600, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //down scaling from 640x480 -> 320x240
   printf("1st stage scalar RGB%d LBLP 640x480 -> 320x240 Test\n", (bpp==24)?888:565);
   
   Resolution(640, 480, 320, 240);  
   Scalar_Resolution(640, 480, 320, 240);
   Scalar_DownRatio(1);
   Scalar_Bypass2(1);   //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "16_888_640x480.bin");
      Load_Image( FILENAME, 1228800, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "16_565_640x480.bin");
      Load_Image( FILENAME, 614400, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //bypass 1st stage scalar and enable the 2nd stage scalar
   //bypass the 1st and 2nd stage scalar
   printf("2nd stage scalar RGB%d LBLP 320x240 -> 320x240 Test\n", (bpp==24)?888:565);
   
   Resolution(320, 240, 320, 240);  
   Scalar_Resolution(320, 240, 320, 240);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_320x240.bmp.bin");
      Load_Image( FILENAME, 307200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_320x240.bmp.bin");
      Load_Image( FILENAME, 153600, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //2nd stage scalar, up scaling from 160x120 -> 320x240
   printf("2nd stage scalar RGB%d LBLP 160x120 -> 320x240 Test\n", (bpp==24)?888:565);
   
   Resolution(160, 120, 320, 240);  
   Scalar_Resolution(160, 120, 320, 240);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 
   
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "16_888_160x120.bin");
      Load_Image( FILENAME, 76800, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "16_565_160x120.bin");
      Load_Image( FILENAME, 38400, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //2nd stage scalar, down scaling from 640x480 -> 320x240
   printf("2nd stage scalar RGB%d LBLP 640x480 -> 320x240 Test\n", (bpp==24)?888:565);
// printf("2nd stage scalar RGB%d LBLP 500x400 -> 320x240 Test\n", (bpp==24)?888:565);

   Resolution(640, 480, 320, 240);
   Scalar_Resolution(640, 480, 320, 240);
// Scalar_Resolution(500, 400, 320, 240);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);         //enable the 2nd stage scalar

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "16_888_640x480.bin");
      Load_Image( FILENAME, 1228800, pBuffer, LBLP, 24 );
//    strcat(FILENAME, "80_888_500x400.bmp.bin");
//    Load_Image( FILENAME, 800000, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "16_565_640x480.bin");
      Load_Image( FILENAME, 614400, pBuffer, LBLP, 16 );
//    strcat(FILENAME, "80_565_500x400.bmp.bin");
//    Load_Image( FILENAME, 400000, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //1st & 2nd stage scalar, down scaling from 500x400 -> 250x200 -> 320x240
   printf("1st & 2nd stage scalar RGB%d LBLP 500x400 -> 250x200 -> 320x240 Test\n", (bpp==24)?888:565);

   Resolution(500, 400, 320, 240);
   Scalar_Resolution(250, 200, 320, 240);
   Scalar_DownRatio(1);    //if you disable 1st stage scaler, only 2nd stage scaler is used, scaling from 500x400 -> 320x240
   Scalar_Bypass2(0);         //enable the 2nd stage scalar

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "80_888_500x400.bmp.bin");
      Load_Image( FILENAME, 800000, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "80_565_500x400.bmp.bin");
      Load_Image( FILENAME, 400000, pBuffer, LBLP, 16 );
   }
    demo_delay(); 
    
   Scalar_DownRatio(0);

#if 0
   //partial dispay from 256x192 -> 320x240
   printf("Scalar partial display RGB%d LBLP 256x192 -> 320x240 Test\n", (bpp==24)?888:565);

   Resolution(256, 192, 320, 240);
   Scalar_Resolution(256, 192, 320, 240);
   Scalar_DownRatio(0);
   Scalar_PartialDisplay(1);

// (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))++;   //VBP+1, to fix RGB888 partial the upper line problem
// Load_Image( "D:\\Star_cell\\picture\\15_888_320x240.bmp.bin", 307200, pBuffer, LBLP, 24 );
// (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))--;   //VBP-1, reload the original VBP value
   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_256x192.bin");
      Load_Image( FILENAME, 196608, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_256x192.bin");
      Load_Image( FILENAME, 98304, pBuffer, LBLP, 16 );
   }
    demo_delay(); 
   Scalar_PartialDisplay(0);
#endif

    printf("Do Threshold neerly bilinear mode\n");
   Scalar_Interpolation( 1, 1 );
    demo_delay(); 

    printf("Do Most neighberhood mode\n");
   Scalar_Interpolation( 2, 2 );
    demo_delay(); 

   *(volatile unsigned int *) (LCD_IO_Base) = 0x00;   
    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00; 
}

void Scalar_Test_240_80(unsigned char *pFrameBuffer, unsigned char bpp)
{  
   int i;
   unsigned char *pBuffer;
   
   pBuffer = pFrameBuffer; //allocateLargeBuffer(0);
   setNumFrameBuffer(1,0,0,0);
   SetFrameBase(0,0,pBuffer);
   printf("pBuffer allocated in Scalar_Test, address = 0x%x \n", (unsigned int)(uintptr_t)pBuffer);
   
   for( i=0; i<PANEL_WIDTH*2*PANEL_HEIGHT*2 *4; i+=4)
      *(volatile unsigned int *) (pBuffer + i) = 0xffffffff;
   
   Init_LCD( LCD_IO_Base, FLcdModule, LCD_PANEL_TYPE, bpp);

    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00;

   //bypass the 1st and 2nd stage scalar
   printf("1st stage scalar RGB%d LBLP 240x80 -> 240x80 Test\n", (bpp==24)?888:565);

   Resolution(240, 80, 240, 80);
   Scalar_Resolution(240, 80, 240, 80);
   Scalar_DownRatio(0);
   Scalar_Bypass2(1);   //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_240x80.bin");
      Load_Image( FILENAME, 76800, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_240x80.bin");
      Load_Image( FILENAME, 38400, pBuffer, LBLP, 16 );
   }
    demo_delay(); 
   
   //down scaling from 480x160 -> 240x80
   printf("1st stage scalar RGB%d LBLP 480x160 -> 240x80 Test\n", (bpp==24)?888:565);
   
   Resolution(480, 160, 240, 80);   
   Scalar_Resolution(480, 160, 240, 80);
   Scalar_DownRatio(1);
   Scalar_Bypass2(1);   //bypass the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "1_888_480x160.bmp.bin");
      Load_Image( FILENAME, 307200, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "1_565_480x160.bmp.bin");
      Load_Image( FILENAME, 153600, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //bypass the 1st stage scalar and enable the 2nd stage scalar

   printf("2st stage scalar RGB%d LBLP 240x80 -> 240x80 Test\n", (bpp==24)?888:565);

   Resolution(240, 80, 240, 80);
   Scalar_Resolution(240, 80, 240, 80);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "lady_888_240x80.bmp.bin");
      Load_Image( FILENAME, 76800, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_240x80.bin");
      Load_Image( FILENAME, 38400, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

   //2nd stage scalar, up/down scaling from 160x120 -> 240x80
   printf("2nd stage scalar RGB%d LBLP 160x120 -> 240x80 Test\n", (bpp==24)?888:565);
   
   Resolution(160, 120, 240, 80);   
   Scalar_Resolution(160, 120, 240, 80);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "20_888_160x120.bin");
      Load_Image( FILENAME, 76800, pBuffer, LBLP, 24 );
   }else if(bpp == 16){
      strcat(FILENAME, "10_565_160x120.bmp.bin");
      Load_Image( FILENAME, 38400, pBuffer, LBLP, 16 );
   }
    demo_delay(); 

#if 0
   //bypass the 1st stage scalar and do partial display
   printf("Scalar partial display RGB%d LBLP 240x80 -> 240x80 Test\n", (bpp==24)?888:565);

   Resolution(240, 80, 240, 80);
   Scalar_Resolution(240, 80, 240, 80);
   Scalar_DownRatio(0);
   Scalar_Bypass2(0);   //enable the 2nd stage scalar
   Scalar_Interpolation( 0, 0 ); 
   Scalar_PartialDisplay(1);

   strcpy(FILENAME, DEFAULT_PATH);
   if(bpp == 24){
      strcat(FILENAME, "15_888_240x80.bin");
      (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))++;   //VBP+1, to fix RGB888 partial the upper line problem
      Load_Image( FILENAME, 76800, pBuffer, LBLP, 24 );
      (*(volatile unsigned int *) (LCD_IO_Base + 0x0108))--;   //VBP-1, reload the original VBP value
   }else if(bpp == 16){
      strcat(FILENAME, "15_565_240x80.bin");
      Load_Image( FILENAME, 38400, pBuffer, LBLP, 16 );
   }

    demo_delay(); 
   Scalar_PartialDisplay(0);
#endif

    printf("Do Threshold neerly bilinear mode\n");
   Scalar_Interpolation( 1, 1 );
    demo_delay(); 

    printf("Do Most neighberhood mode\n");
   Scalar_Interpolation( 2, 2 );
    demo_delay(); 

   *(volatile unsigned int *) (LCD_IO_Base) = 0x00;   
    for (i=0x1100;i<=0x112c;i+=4)
       *(volatile unsigned int *)(uintptr_t)(LCD_IO_Base + i) = 0x00; 
}


void Scalar_down_TV()
{
   printf("Scalar down: use hardware color bar\n");
   printf("TFT: 800*600\n");
   printf("TV : 800*600 --> 640*480\n");
   printf("Need special FPGA on VIRTEX-4:only work on this function\n");
      
   *(volatile  unsigned int *) (LCD_IO_Base + 0x0000 ) = 0;
   
   *(volatile  unsigned int *) (LCD_IO_Base + 0x04 ) = 0x61C;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x100 ) = 0x9EFE9E31;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x104 ) = 0x280A0257;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x108 ) = 0x4;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x10C ) = 0x0;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x110 ) = 0x1011D3;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x208 ) = 0x20D6B4;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x20C ) = 0x10A004;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x210 ) = 0x17016;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x218 ) = 0xF00F0;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x21C ) = 0x5010C;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x220 ) = 0x50;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x224 ) = 0x500;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x230 ) = 0x4;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1100 ) = 0x31F;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1104 ) = 0x257;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1108 ) = 0x280;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x110C ) = 0x1E0;
   *(volatile  unsigned int *) (LCD_IO_Base + 0x1110 ) = 0x0;                                                      
   
   *(volatile  unsigned int *) (LCD_IO_Base + 0x0000 ) = 0x46023;
   delay_10ms(500); 
   Scalar_Off();
}

