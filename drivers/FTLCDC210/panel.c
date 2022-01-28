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

/******************************************************************************
 * Define Constants
 *****************************************************************************/


/******************************** not yet *******************************/
LCDMTYPE_T FLcdModule[] = 
{
/*  
  { 
    LCD_TRUE,  "Albert's Para", //A320D
    0x2B135F9C,
    0x101045DF,
    0x00003806,
    0x00013a29,
    640,
    480,
  
    },

  
  
    {
    LCD_TRUE,  "Albert's Parameter", //A320D
    0x1010104c,
    0x070F00EF,
    0x013F37c2,
    0x00043a29,
    320,
    240,
  
    },



  // Sharp LQ084C1DG21 ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "Sharp LQ084C1DG21",

    //LCDEnable
     (CSTNEn   << 14)         //Color STN panel enable
    |(CCIREn   << 13)         //CCIR656 output format
    |(PatGen   << 11)         //Test pattern generator, not implement now
    |(LCDon    <<  1)         //LCD screen on/off control
    |(LCDen        ),         //LCD controller enable control
    
    //PanelPixel
     (LRST     << 15)         //LC_CLK domain reset
    |(HRST     << 14)         //HCLK domain reset
    |(PanelType   << 11)         //TFT panel color depth selection
   |(BGRSW     <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((HBP   -1) << 24)        //Horizontal back porch
   |((HFP   -1) << 16)        //Horizontal front porch
   |((HW -1) <<  8)        //Horizontal Sync. pulse width
   |((PL >> 4) -   1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (VFP    << 24)         //Vertical front porch
   |((VW -1)   << 16)         //Vertical Sync. pulse width
   |(LF         ),         //Lines-per-frame

   //VerticalTiming2
   (VBP  -1     ),         //Vertical back porch

   //Polarity
    (DivNo     <<  8)         //Panel clock divisor = (DivNo + 1)
   |(IDE    <<  3)         //The invert output enable
   |(ICK    <<  2)         //Select the edge of the panel clock
   |(IHS    <<  1)         //The invert horizontal sync bit
   |(IVS        ),         //The invert vertical sync bit

   //SerialPanelPixel
    (AUO052 <<  5)         //AUO052 mode
   |(LSR    <<  4)         //Left shift rotate
   |(ColorSeq  <<  2)         //Color sequence of odd line
   |(DeltaType <<  1)         //Delta type arrangement color filter
   |(SerialMode    ),         //RGB serial output mode
   
   //CCIR656
    (PHASE     <<  2)         //TVE clock phase
   |(P720      <<  1)         //720 pixels per line
   |(NTSC          ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once
   
   //Width
   640,

   //Height
   480
  },
*/  
/*
  // Ampire mono stn 8bit 320x240**************************************
  { // Valid,  Descriptor,
    LCD_TRUE,  "Ampire Mono320x240",

    // Timing0
     ((1       - 1) << 24) //Horizontal back porch
   |((1        - 1) << 16) //Horizontal front porch
   |((1        - 1) << 8 ) //Horizontal Sync. pulse width
   |(((320 >> 4)  - 1) << 2 ),//pixels-per-line = 16(PPL+1)

   //Timing1
    (0         << 24)         //Vertical back porch
   |(0         << 16)         //Vertical front porch
   |((1 - 1)   << 10)         //Vertical Sync. pulse width
   |((240 - 1)     ),         //lines-per-panel = LPP+1

   //Timing2
    (LCD_FALSE << 26)         //Bypass pixel clock divider
   |((40 - 1)  << 16)         //Clock per line
   |(LCD_FALSE << 14)         //invert output enable
   |(LCD_FALSE << 13)         //invert panel clock
   |(LCD_FALSE << 12)         //invert horizontal sync.
   |(LCD_FALSE << 11)         //invert vertical sync
   |((32 - 1)  <<  6)         //ac bias
   |(LCD_FALSE <<  5)         //clock select
   |(8 - 2         ),         //panel clock divdsor

   //Control
    (LCD_FALSE << 16)         //LCD DMA FIFO watermark level
   |(LCD_FALSE << 15)         //LCD DMA FIFO test mode enable
   |(0         << 12)         //LcdVComp, when to generate interrupt
   |(LCD_FALSE << 11)         //LCD power enable
   |(LCD_TRUE  << 10)         //Big-endian pixel ordering
   |(LCD_FALSE <<  9)         //Big-endian Byte ordering
   |(LCD_FALSE <<  8)         //BGR
   |(LCD_FALSE <<  7)         //LcdDual
   |(LCD_TRUE  <<  6)         //LcdMono8
   |(LCD_FALSE <<  5)         //LcdTFT
   |(LCD_TRUE  <<  4)         //LcdBW
   |(0         <<  1)         //LCD bits per pixel: 1bpp
   |(LCD_FALSE     ),         //LCD controller enable

   //Width
   320,

   //Height
   240
  },
  
  // Ampire color stn 8bit 320x240**************************************
  { // Valid,  Descriptor,
    LCD_TRUE,  "Ampire Color320x240",

    // Timing0
     ((16         - 1) << 24) //Horizontal back porch
   |((1        - 1) << 16) //Horizontal front porch
   |((4        - 1) << 8 ) //Horizontal Sync. pulse width
   |(((320 >> 4)  - 1) << 2 ),//pixels-per-line = 16(PPL+1)

   //Timing1
    (14     << 24)         //Vertical back porch
   |(0         << 16)         //Vertical front porch
   |((2 - 1)   << 10)         //Vertical Sync. pulse width
   |((240 - 1)     ),         //lines-per-panel = LPP+1

   //Timing2
    (LCD_FALSE << 26)         //Bypass pixel clock divider
   |((240 - 1) << 16)         //Clock per line
   |(LCD_FALSE << 14)         //invert output enable
   |(LCD_TRUE  << 13)         //invert panel clock
   |(LCD_TRUE  << 12)         //invert horizontal sync.
   |(LCD_TRUE  << 11)         //invert vertical sync
   |((32 - 1)  <<  6)         //ac bias
   |(LCD_FALSE <<  5)         //clock select
   |(2 - 2         ),         //panel clock divdsor

   //Control
    (LCD_FALSE << 16)         //LCD DMA FIFO watermark level
   |(LCD_FALSE << 15)         //LCD DMA FIFO test mode enable
   |(0         << 12)         //LcdVComp, when to generate interrupt
   |(LCD_FALSE << 11)         //LCD power enable
   |(LCD_TRUE  << 10)         //Big-endian pixel ordering
   |(LCD_FALSE <<  9)         //Big-endian Byte ordering
   |(LCD_FALSE <<  8)         //BGR
   |(LCD_FALSE <<  7)         //LcdDual
   |(LCD_FALSE <<  6)         //LcdMono8
   |(LCD_TRUE  <<  5)         //LcdTFT
   |(LCD_FALSE <<  4)         //LcdBW
   |(4         <<  1)         //LCD bits per pixel: 16bpp
   |(LCD_FALSE     ),         //LCD controller enable

   //Width
   320,

   //Height
   240
  },
*/  

  //TYPE0 : AUO A036QN01 with CPLD on, ie. accept the parallel signal*******************************
  { // Valid,  Descriptor
    LCD_TRUE,  "AUO A036QN01",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_FALSE   <<  1)         //LCD screen on/off control
    |(LCD_FALSE       ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(0        << 11)         //TFT panel color depth selection, 6-bit per channel
   |(1      <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((0x2b  -1) << 24)        //Horizontal back porch
   |((0x21  -1) << 16)        //Horizontal front porch
   |((0x15  -1) <<  8)        //Horizontal Sync. pulse width
   |((320 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (0x8    << 24)         //Vertical front porch
   |((7  -1)   << 16)         //Vertical Sync. pulse width
   |(240 -1     ),         //Lines-per-frame
   
   //VerticalTiming2
   (14   -1     ),         //Vertical back porch
   
   //Polarity
    (2         <<  8)         //Panel clock divisor = (DivNo + 1), DivNo >= 3 --> does not FIFO under run
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_FALSE     ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once
   
   //Width
   320,

   //Height
   240,
   
  },
  
    //TYPE1 : AUO A036QN01 without CPLD, ie. accept the serial signal *****************************
  { // Valid,  Descriptor
    LCD_TRUE,  "AUO A036QN01",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_FALSE   <<  1)         //LCD screen on/off control
    |(LCD_FALSE       ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(1        << 11)         //TFT panel color depth selection, 6-bit per channel
   |(0      <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((0x2b  -1) << 24)        //Horizontal back porch
   |((0x21  -1) << 16)        //Horizontal front porch
   |((0x15  -1) <<  8)        //Horizontal Sync. pulse width
   |((320 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (0x8    << 24)         //Vertical front porch
   |((7  -1)   << 16)         //Vertical Sync. pulse width
   |(240 -1        ),         //Lines-per-frame
   
   //VerticalTiming2
   (14   -1     ),         //Vertical back porch
   
   //Polarity
    (0         <<  8)         //Panel clock divisor = (DivNo + 1), DivNo >= 3 --> does not FIFO under run
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_FALSE     ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_TRUE  <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_TRUE      ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

   //Width
   320,

   //Height
   240,
   
  },
  
  //TYPE2 : Sharp Sharp LQ057Q3DC02 ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "Sharp LQ057Q3DC02",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_FALSE   <<  1)         //LCD screen on/off control
    |(LCD_FALSE       ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(0        << 11)         //TFT panel color depth selection, 6-bit per channel
   |(1      <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((0x11  -1) << 24)        //Horizontal back porch
   |((0x11  -1) << 16)        //Horizontal front porch
   |((0x11  -1) <<  8)        //Horizontal Sync. pulse width
   |((320 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (0xF    << 24)         //Vertical front porch
   |((1  -1)   << 16)         //Vertical Sync. pulse width
   |(240  -1       ),         //Lines-per-frame
   
   //VerticalTiming2
   (7/*7*/  -1       ),       //Vertical back porch
   
   //Polarity
    (5         <<  8)         //Panel clock divisor = (DivNo + 1), DivNo >= 3 --> does not FIFO under run
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_FALSE     ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

   //Width
   320,

   //Height
   240,
   
  },

  
  //TYPE3 : Sharp LQ084V1DG21 ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "Sharp LQ084V1DG21",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_FALSE   <<  1)         //LCD screen on/off control
    |(LCD_FALSE       ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(0        << 11)         //TFT panel color depth selection, 0 -> 6-bit per channel
   |(1         <<  4),        //RGB or BGR format selection, 0 -> RGB
    
   //HorizontalTiming
    ((44 -1) << 24)        //Horizontal back porch
   |((20 -1) << 16)        //Horizontal front porch
   |((96 -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)-   1),        //pixels-per-line = 16(PPL+1)

   //VerticalTiming1
    (16     << 24)         //Vertical front porch
   |((17 -1)   << 16)         //Vertical Sync. pulse width
   |(480  -1       ),         //Lines-per-frame
   
   //VerticalTiming2
   (17   -1     ),         //Vertical back porch
   
   //Polarity
    (0         <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_TRUE  <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_TRUE      ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once
   
   //Width
   640,

   //Height
   480,
   
  },
  
  //TYPE4 : PRIME VIEW PD035VX2 ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "PRIME VIEW PD035VX2",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_FALSE   <<  1)         //LCD screen on/off control
    |(LCD_FALSE       ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(0        << 11)         //TFT panel color depth selection, 0 -> 6-bit per channel
   |(1         <<  4),        //RGB or BGR format selection, 0 -> RGB
    
   //HorizontalTiming
    ((0x30  -1) << 24)        //Horizontal back porch
   |((20 -1) << 16)        //Horizontal front porch
   |((96 -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)-   1),        //pixels-per-line = 16(PPL+1)

   //VerticalTiming1
    (16     << 24)         //Vertical front porch
   |((0x0b -1) << 16)         //Vertical Sync. pulse width, john 69->0x0b
   |(480  -1       ),         //Lines-per-frame
   
   //VerticalTiming2
   (0x05    -1     ),         //Vertical back porch, john 0x1E --> 0x05
   
   //Polarity
    (0xf    <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_TRUE  <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_TRUE      ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

   //Width
   640,

   //Height
   480,

  },
  
    //TYPE5 : KOROLA 6.5-inch TFT-LCD TV NTSC ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "KOROLA 6.5'' LCD TV",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_TRUE  << 13)      //CCIR656 enable
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((3  -1) << 24)        //Horizontal back porch
   |((0xD3  -1) << 16)        //Horizontal front porch
   |((4  -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (1         << 24)         //Vertical front porch
   |((18 -1)   << 16)         //Vertical Sync. pulse width
   |(480 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (0x1a    -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled

   //Polarity
    (0         <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_TRUE      ),         //The invert vertical sync bit

   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_TRUE  <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_TRUE      ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once
   
   //Width
   640,

   //Height
   480
  },
 
  
      //TYPE6 : KOROLA 6.5-inch TFT-LCD TV NTSC ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "KOROLA 6.5'' LCD TV",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_TRUE  << 13)      //CCIR656 enable
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((3  -1) << 24)        //Horizontal back porch
   |((0x7f  -1) << 16)        //Horizontal front porch
   |((8  -1) <<  8)        //Horizontal Sync. pulse width
   |((720 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (1         << 24)         //Vertical front porch
   |((0x11  -1)   << 16)         //Vertical Sync. pulse width
   |(480 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (0x1b    -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled

   //Polarity
    (0         <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_TRUE      ),         //The invert vertical sync bit

   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode

   //CCIR656
    (LCD_TRUE  <<  2)         //TVE clock phase
   |(LCD_TRUE  <<  1)         //720 pixels per line
   |(LCD_TRUE      ),         //NTSC/PAL select

   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

   //Width
   720,

   //Height
   480
  },
  

     //TYPE 7 : KOROLA 6.5-inch TFT-LCD TV PAL 640x480 ***************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "KOROLA 6.5'' LCD TV",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_TRUE  << 13)      //CCIR656 enable
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((8  -1) << 24)        //Horizontal back porch
   |((0xd0  -1) << 16)        //Horizontal front porch
   |((8  -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1, VFP + VW + LF + VBP = 625 and VW + VBP = 91
    (0x35         << 24)         //Vertical front porch
   |((0x0b  -1)   << 16)         //Vertical Sync. pulse width
   |(480 -1        ),         //Lines-per-frame
   
   //VerticalTiming2
   (0x51    -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled
   
/*  ((8  -1) << 24)        //Horizontal back porch
   |((0xd0  -1) << 16)        //Horizontal front porch
   |((8  -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (5         << 24)         //Vertical front porch
   |((0x11  -1)   << 16)         //Vertical Sync. pulse width
   |(576 -1        ),         //Lines-per-frame
   
   //VerticalTiming2
   (0x1b -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled
*/
/*  ((8  -1) << 24)        //Horizontal back porch
   |((0xd0  -1) << 16)        //Horizontal front porch
   |((8  -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (5         << 24)         //Vertical front porch
   |((0x11  -1)   << 16)         //Vertical Sync. pulse width
   |(640 -1        ),         //Lines-per-frame
   
   //VerticalTiming2
   (0x1b -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled
*/
   //Polarity
    (0         <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_TRUE      ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_TRUE  <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

   //Width
   640,

   //Height
   480
  },
  
  //TYPE 8 : KOROLA 6.5-inch TFT-LCD TV PAL 720x576 ***************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "KOROLA 6.5'' LCD TV",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_TRUE  << 13)      //CCIR656 enable
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((3  -1) << 24)        //Horizontal back porch
   |((0x85  -1) << 16)        //Horizontal front porch
   |((8  -1) <<  8)        //Horizontal Sync. pulse width
   |((720 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (5         << 24)         //Vertical front porch
   |((0x11  -1)   << 16)         //Vertical Sync. pulse width
   |(576 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (0x1b    -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled

   //Polarity
    (0         <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE  <<  1)         //The invert horizontal sync bit
   |(LCD_TRUE      ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_TRUE  <<  2)         //TVE clock phase
   |(LCD_TRUE  <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once
   
   //Width
   720,

   //Height
   576
  },
  
  
     //TYPE 9 : 240x160 CSTN-LCD  ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "CSTN Panel",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((6  -1) << 24)        //Horizontal back porch
   |((1  -1) << 16)        //Horizontal front porch
   |((2  -1) <<  8)        //Horizontal Sync. pulse width
   |((240 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (0         << 24)         //Vertical front porch
   |((1  -1)   << 16)         //Vertical Sync. pulse width
   |(160 -1        ),         //Lines-per-frame
   
   //VerticalTiming2
   (1    -1     ),            //Vertical back porch
   
   //Polarity
    (0         <<  8)         //Panel clock divisor = (DivNo + 1) //166HZ on 20MHZ
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_FALSE <<  1)         //The invert horizontal sync bit
   |(LCD_FALSE     ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select

   //CSTNPanelControl
    (LCD_TRUE  << 27)         //CSTN function enable bit
   |(3         << 25)         //Bus width of CSTN panel
   |(0         << 24)         //CSTN panel type
   |((240/16)  << 16)         //Virtual window width in framebuffer
   |(240*3  -1     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (1      << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(2*160  +1     ),         //Line numbers that "AC" inverts once

   //Width
   240,

   //Height
   160
  },
  
  
  //TYPE 10 : 160x160 STN-LCD  ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "STN Panel",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((6  -1) << 24)        //Horizontal back porch
   |((1  -1) << 16)        //Horizontal front porch
   |((2  -1) <<  8)        //Horizontal Sync. pulse width
   |((160 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (0         << 24)         //Vertical front porch
   |((1  -1)   << 16)         //Vertical Sync. pulse width
   |(160 -1        ),         //Lines-per-frame
   
   //VerticalTiming2
   (1    -1     ),            //Vertical back porch
   
   //Polarity
    (5         <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_FALSE <<  1)         //The invert horizontal sync bit
   |(LCD_FALSE     ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select

   //CSTNPanelControl
    (LCD_TRUE  << 27)         //CSTN function enable bit
   |(2         << 25)         //Bus width of CSTN panel
   |(1         << 24)         //CSTN panel type
   |((160/16)  << 16)         //Virtual window width in framebuffer
   |(160 -1     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (1         << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(2*160  +1     ),         //Line numbers that "AC" inverts once

   //Width
   160,

   //Height
   160
  },
  
    //TYPE 11 : 160x80 STN-LCD (Mono) *******************************************
  { // Valid,  Descriptor, it is not good for grayscale because the frame rate cannot be fast, almost < 113MHZ
    LCD_TRUE,  "STN Panel",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    
   //HorizontalTiming
    ((6  -1) << 24)        //Horizontal back porch
   |((1  -1) << 16)        //Horizontal front porch
   |((2  -1) <<  8)        //Horizontal Sync. pulse width
   |((160 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (0      << 24)         //Vertical front porch
   |((1  -1)   << 16)         //Vertical Sync. pulse width
   |(80  -1        ),         //Lines-per-frame
   
   //VerticalTiming2
   (1    -1     ),            //Vertical back porch
   
   //Polarity
    (15     <<  8)         //Panel clock divisor = (DivNo + 1), max = 4MHZ, 0xe(27MHZ oscillator) = 113HZ
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_FALSE <<  1)         //The invert horizontal sync bit
   |(LCD_FALSE     ),         //The invert vertical sync bit
   
   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_FALSE <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_FALSE     ),         //NTSC/PAL select

   //CSTNPanelControl
    (LCD_TRUE  << 27)         //CSTN function enable bit
   |(0         << 25)         //Bus width of CSTN panel
   |(1         << 24)         //CSTN panel type
   |((160/16)  << 16)         //Virtual window width in framebuffer
   |(160 -1     ),         //Horizontal dot's resolution of CSTN panel
   
   //CSTNPanelParam1
    (1         << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(2*80   +1     ),         //Line numbers that "AC" inverts once

   //Width
   160,

   //Height
   80
  },
  
   //TYPE12 : 320x240 TPG, ie. accept the serial signal *****************************
   {  // Valid, Descriptor
      LCD_TRUE, "TPG051 serial RGB",
 
      //LCDEnable
      (LCD_FALSE << 14) //Test pattern generator, not implement now
      |(LCD_FALSE <<  1)   //LCD screen on/off control
      |(LCD_FALSE   ),     //LCD controller enable control

      //PanelPixel
      (LCD_FALSE << 15) //LC_CLK domain reset
      |(LCD_FALSE << 14)   //HCLK domain reset
      |(1   << 11)         //TFT panel color depth selection, 6-bit per channel
      |(1   <<  4)         //RGB or BGR format selection
      |(4   <<  0),        //BppFifo: 16bpp
    
      //HorizontalTiming
      ((47 -1) << 24)    //Horizontal back porch
      |((20 -1) << 16)   //Horizontal front porch
      |((4 -1) <<  8)   //Horizontal Sync. pulse width
      |((320 >> 4)   -1),   //pixels-per-line = 16(PL+1)
 
      //VerticalTiming1
      (15  << 24)       //Vertical front porch
      |((1 -1) << 16)    //Vertical Sync. pulse width
      |(240    ),       //Lines-per-frame
 
      //VerticalTiming2
      (14  -1   ),   //Vertical back porch
 
      //Polarity
      (0   <<  8)                   //Panel clock divisor = (DivNo + 1), DivNo >= 3 --> does not FIFO under run
      |(LCD_FALSE <<  3)   //The invert output enable
      |(LCD_FALSE <<  2)   //Select the edge of the panel clock
      |(LCD_TRUE <<  1)   //The invert horizontal sync bit
      |(LCD_TRUE   ),       //The invert vertical sync bit
 
      //SerialPanelPixel
      (LCD_FALSE <<  5)   //AUO052 mode
      |(LCD_FALSE <<  4)   //Left shift rotate
//    |(2 <<  2)        //Color sequence of odd line
      |(1 <<  2)        //Color sequence of odd line; BRG
//    |(LCD_FALSE <<  1)   //Delta type arrangement color filter
      |(LCD_TRUE <<  1)   //Delta type arrangement color filter
      |(LCD_TRUE   ),       //RGB serial output mode
 
      //CCIR656
      (LCD_FALSE <<  2)   //TVE clock phase
      |(LCD_FALSE <<  1)   //720 pixels per line
      |(LCD_FALSE   ),       //NTSC/PAL select
 
      //CSTNPanelControl
       (LCD_FALSE << 27)         //CSTN function enable bit
      |(LCD_FALSE << 25)         //Bus width of CSTN panel
      |(LCD_FALSE << 24)         //CSTN panel type
      |(LCD_FALSE << 16)         //Virtual window width in framebuffer
      |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

      //CSTNPanelParam1
       (LCD_FALSE << 16)         //CSTN horizontal back porch
      |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
      |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

/*    // bypass
      (LCD_TRUE   <<  1)         //bypass RGB2YCbCr
      |(LCD_TRUE),               //bypass YCbCr2RGB
*/
      //Width, //Height
      320, 240
   },
   
      //TYPE13 : 128x160 TD018THEA1 LCM Panel *****************************
   {  // Valid, Descriptor
      LCD_TRUE, "LCM Panel",
 
      //LCDEnable
      (LCD_FALSE << 14) //Test pattern generator, not implement now
      |(LCD_FALSE <<  1)   //LCD screen on/off control
      |(LCD_FALSE   ),     //LCD controller enable control

      //PanelPixel
      (LCD_FALSE << 15) //LC_CLK domain reset
      |(LCD_FALSE << 14)   //HCLK domain reset
      |(0   << 11)         //TFT panel color depth selection, 6-bit per channel
      |(0   <<  4)         //RGB or BGR format selection
      |(4   <<  0),        //BppFifo: 16bpp
    
      //HorizontalTiming
      ((47 -1) << 24)    //Horizontal back porch
      |((20 -1) << 16)   //Horizontal front porch
      |((4 -1) <<  8)   //Horizontal Sync. pulse width
      |((128 >> 4)   -1),   //pixels-per-line = 16(PL+1)
 
      //VerticalTiming1
      (15  << 24)       //Vertical front porch
      |((1 -1) << 16)    //Vertical Sync. pulse width
      |(160    ),       //Lines-per-frame
 
      //VerticalTiming2
      (14  -1   ),   //Vertical back porch
 
      //Polarity
      (0   <<  8)                   //Panel clock divisor = (DivNo + 1), DivNo >= 3 --> does not FIFO under run
      |(LCD_FALSE <<  3)   //The invert output enable
      |(LCD_FALSE <<  2)   //Select the edge of the panel clock
      |(LCD_TRUE <<  1)   //The invert horizontal sync bit
      |(LCD_TRUE   ),       //The invert vertical sync bit
 
      //SerialPanelPixel
      (LCD_FALSE <<  5)   //AUO052 mode
      |(LCD_FALSE <<  4)   //Left shift rotate
//    |(2 <<  2)        //Color sequence of odd line
      |(1 <<  2)        //Color sequence of odd line; BRG
//    |(LCD_FALSE <<  1)   //Delta type arrangement color filter
      |(LCD_TRUE <<  1)   //Delta type arrangement color filter
      |(LCD_TRUE   ),       //RGB serial output mode
 
      //CCIR656
      (LCD_FALSE <<  2)   //TVE clock phase
      |(LCD_FALSE <<  1)   //720 pixels per line
      |(LCD_FALSE   ),       //NTSC/PAL select
 
      //CSTNPanelControl
       (LCD_FALSE << 27)         //CSTN function enable bit
      |(LCD_FALSE << 25)         //Bus width of CSTN panel
      |(LCD_FALSE << 24)         //CSTN panel type
      |(LCD_FALSE << 16)         //Virtual window width in framebuffer
      |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

      //CSTNPanelParam1
       (LCD_FALSE << 16)         //CSTN horizontal back porch
      |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
      |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

/*    // bypass
      (LCD_TRUE   <<  1)         //bypass RGB2YCbCr
      |(LCD_TRUE),               //bypass YCbCr2RGB
*/
      //Width, //Height
      128, 160
   },

      //TYPE14 : 240x320 WXPAT24-MS5#001 LCM Panel *****************************
   {  // Valid, Descriptor
      LCD_TRUE, "LCM Panel",
 
      //LCDEnable
      (LCD_FALSE << 14) //Test pattern generator, not implement now
      |(LCD_FALSE <<  1)   //LCD screen on/off control
      |(LCD_FALSE   ),     //LCD controller enable control

      //PanelPixel
      (LCD_FALSE << 15) //LC_CLK domain reset
      |(LCD_FALSE << 14)   //HCLK domain reset
      |(0   << 11)         //TFT panel color depth selection, 6-bit per channel
      |(0   <<  4)         //RGB or BGR format selection
      |(4   <<  0),        //BppFifo: 16bpp
    
      //HorizontalTiming
      ((47 -1) << 24)    //Horizontal back porch
      |((20 -1) << 16)   //Horizontal front porch
      |((4 -1) <<  8)   //Horizontal Sync. pulse width
      |((240 >> 4)   -1),   //pixels-per-line = 16(PL+1)
 
      //VerticalTiming1
      (15  << 24)       //Vertical front porch
      |((1 -1) << 16)    //Vertical Sync. pulse width
      |(320    ),       //Lines-per-frame
 
      //VerticalTiming2
      (14  -1   ),   //Vertical back porch
 
      //Polarity
      (0   <<  8)                   //Panel clock divisor = (DivNo + 1), DivNo >= 3 --> does not FIFO under run
      |(LCD_FALSE <<  3)   //The invert output enable
      |(LCD_FALSE <<  2)   //Select the edge of the panel clock
      |(LCD_TRUE <<  1)   //The invert horizontal sync bit
      |(LCD_TRUE   ),       //The invert vertical sync bit
 
      //SerialPanelPixel
      (LCD_FALSE <<  5)   //AUO052 mode
      |(LCD_FALSE <<  4)   //Left shift rotate
//    |(2 <<  2)        //Color sequence of odd line
      |(1 <<  2)        //Color sequence of odd line; BRG
//    |(LCD_FALSE <<  1)   //Delta type arrangement color filter
      |(LCD_TRUE <<  1)   //Delta type arrangement color filter
      |(LCD_TRUE   ),       //RGB serial output mode
 
      //CCIR656
      (LCD_FALSE <<  2)   //TVE clock phase
      |(LCD_FALSE <<  1)   //720 pixels per line
      |(LCD_FALSE   ),       //NTSC/PAL select
 
      //CSTNPanelControl
       (LCD_FALSE << 27)         //CSTN function enable bit
      |(LCD_FALSE << 25)         //Bus width of CSTN panel
      |(LCD_FALSE << 24)         //CSTN panel type
      |(LCD_FALSE << 16)         //Virtual window width in framebuffer
      |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

      //CSTNPanelParam1
       (LCD_FALSE << 16)         //CSTN horizontal back porch
      |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
      |(LCD_FALSE     ),         //Line numbers that "AC" inverts once

/*    // bypass
      (LCD_TRUE   <<  1)         //bypass RGB2YCbCr
      |(LCD_TRUE),               //bypass YCbCr2RGB
*/
      //Width, //Height
      240, 320
   },
    //TYPE15 : ViewSonic VA720 ***********************************************
  { // Valid,  Descriptor
    LCD_TRUE,  "ViewSonic VA720 Monitor",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_FALSE  << 13)       //CCIR656 enable
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_FALSE   << 11)         //TFT panel color depth selection
   |(LCD_TRUE <<  4),        //RGB or BGR format selection
    /*
   //HorizontalTiming
    ((88 -1) << 24)        //Horizontal back porch
   |((24 -1) << 16)        //Horizontal front porch
   |((64 -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (1         << 24)         //Vertical front porch
   |((3  -1)   << 16)         //Vertical Sync. pulse width
   |(480 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (15   -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled
*/
/* //HorizontalTiming
    ((120   -1) << 24)        //Horizontal back porch
   |((40 -1) << 16)        //Horizontal front porch
   |((80 -1) <<  8)        //Horizontal Sync. pulse width
   |((800 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (1         << 24)         //Vertical front porch
   |((3  -1)   << 16)         //Vertical Sync. pulse width
   |(600 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (22   -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled
*/
   //HorizontalTiming
    ((88 -1) << 24)        //Horizontal back porch
   |((24 -1) << 16)        //Horizontal front porch
   |((64 -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (1         << 24)         //Vertical front porch
   |((3  -1)   << 16)         //Vertical Sync. pulse width
   |(480 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (15   -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled   //Polarity
   
    (LCD_FALSE <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_FALSE <<  2)         //Select the edge of the panel clock
   |(LCD_FALSE <<  1)         //The invert horizontal sync bit
   |(LCD_FALSE     ),         //The invert vertical sync bit


   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_TRUE  <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_TRUE      ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once
   
   //Width
   640,

   //Height
   480
  },
   //TYPE16 : Acer p234HL ***********************************************
{ // Valid,  Descriptor
    LCD_TRUE,  "Acer p234HL Monitor",

    //LCDEnable
     (LCD_FALSE   << 14)         //Test pattern generator, not implement now
    |(LCD_FALSE  << 13)       //CCIR656 enable
    |(LCD_TRUE <<  1)         //LCD screen on/off control
    |(LCD_TRUE     ),         //LCD controller enable control
    
    //PanelPixel
     (LCD_FALSE   << 15)         //LC_CLK domain reset
    |(LCD_FALSE   << 14)         //HCLK domain reset
    |(LCD_TRUE << 11)         //TFT panel color depth selection
   |(LCD_FALSE <<  4),        //RGB or BGR format selection
    /*
   //HorizontalTiming
    ((88 -1) << 24)        //Horizontal back porch
   |((24 -1) << 16)        //Horizontal front porch
   |((64 -1) <<  8)        //Horizontal Sync. pulse width
   |((640 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (1         << 24)         //Vertical front porch
   |((3  -1)   << 16)         //Vertical Sync. pulse width
   |(480 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (15   -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled
*/
/* //HorizontalTiming
    ((120   -1) << 24)        //Horizontal back porch
   |((40 -1) << 16)        //Horizontal front porch
   |((80 -1) <<  8)        //Horizontal Sync. pulse width
   |((800 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (1         << 24)         //Vertical front porch
   |((3  -1)   << 16)         //Vertical Sync. pulse width
   |(600 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (22   -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled
*/
   //HorizontalTiming
    ((148 -1) << 24)        //Horizontal back porch
   |((88 -1) << 16)        //Horizontal front porch
   |((44 -1) <<  8)        //Horizontal Sync. pulse width
   |((1920 >> 4)   -1),        //pixels-per-line = 16(PL+1)

   //VerticalTiming1
    (4         << 24)         //Vertical front porch
   |((5  -1)   << 16)         //Vertical Sync. pulse width
   |(1080 -1        ),         //Lines-per-frame

   //VerticalTiming2
   (37   -1     ),         //Vertical back porch, for NTSC/PAL, VBP has to add 1 if sharpness disabled   //Polarity
   
    (LCD_FALSE <<  8)         //Panel clock divisor = (DivNo + 1)
   |(LCD_FALSE <<  3)         //The invert output enable
   |(LCD_TRUE <<  2)         //Select the edge of the panel clock
   |(LCD_TRUE <<  1)         //The invert horizontal sync bit
   |(LCD_TRUE     ),         //The invert vertical sync bit


   //SerialPanelPixel
    (LCD_FALSE <<  5)         //AUO052 mode
   |(LCD_FALSE <<  4)         //Left shift rotate
   |(LCD_FALSE <<  2)         //Color sequence of odd line
   |(LCD_FALSE <<  1)         //Delta type arrangement color filter
   |(LCD_FALSE     ),         //RGB serial output mode
   
   //CCIR656
    (LCD_TRUE  <<  2)         //TVE clock phase
   |(LCD_FALSE <<  1)         //720 pixels per line
   |(LCD_TRUE      ),         //NTSC/PAL select
   
   //CSTNPanelControl
    (LCD_FALSE << 27)         //CSTN function enable bit
   |(LCD_FALSE << 25)         //Bus width of CSTN panel
   |(LCD_FALSE << 24)         //CSTN panel type
   |(LCD_FALSE << 16)         //Virtual window width in framebuffer
   |(LCD_FALSE     ),         //Horizontal dot's resolution of CSTN panel

   //CSTNPanelParam1
    (LCD_FALSE << 16)         //CSTN horizontal back porch
   |(LCD_FALSE <<  7)         //Type of the signal, "AC", inverts
   |(LCD_FALSE     ),         //Line numbers that "AC" inverts once
   
   //Width
   1920,

   //Height
   1080
  },
	

   // Null ***********************************************
   //  Valid,  Descriptor, Timing1, Timing2, Timing2, Control, Width, Height
 
    {LCD_FALSE,   "",         0,0,0,0,0,0,0,0,0,       0,        0,     0, },  
  
};

/************************** end not yet ****************************************/
