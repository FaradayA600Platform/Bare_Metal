/*******************************************************************************
* Copyright  Faraday Technology Corp 2020-2021.  All rights reserved.          *
*------------------------------------------------------------------------------*
* Name:ftuart010_test.c                                                        *
* Description: ftuart010 test function source file                             *
* Author: Jack Chian                                                           *
*******************************************************************************/

/******************************************************************************
 * Include files
 *****************************************************************************/

#include "autoconf.h"

#if defined(CONFIG_BSP_LITE)

#include "portme_ftuart010.h"

extern char fLib_getch(unsigned int port);
extern void fLib_putc_port(char Ch,unsigned int port);

/* UART */
unsigned IP_pa_base[IP_COUNT] = {
     UART0_REG_BASE,
     UART1_REG_BASE
};


#elif defined(CONFIG_BSP_A380)

//#include "fLib.h"
#include "SoFlexible.h"
#include "stdio.h"
#include "serial.h"
#include "string.h"
#include "ftdmac020.h"
//#include "../FTSCU010/pmu.h"

#elif defined(CONFIG_BSP_NONE) || defined(CONFIG_PLATFORM_A600)

#include "portme_ftuart010.h"

#endif

/******************************************************************************
 * Define Constants
 *****************************************************************************/
#define MAX_FIFO_TRIGGER_LEVEL_INDEX 4 
#define MAX_BAUDRATE_INDEX 9
#define MAX_PARITY_INDEX 5 
#define MAX_STOP_INDEX 2
#define MAX_WORD_LENGTH_INDEX 4
 
#define DEFAULT_RX_TRIGGER_LEVEL 0
#define DEFAULT_TX_TRIGGER_LEVEL 0

const unsigned int ScanBaudRate[MAX_BAUDRATE_INDEX]={12,24,48,96,144,192,384,576,1152};

/******************************************************************************
 * Define gobal variable
 *****************************************************************************/

/******************************************************************************
 *Public functions
 *****************************************************************************/
static unsigned int BaudRateSelect(unsigned int chose)
{
   unsigned int baudrate;

   if (chose ==0)
   {
      printf("Setting Baudrate(12,24,48,96,144,192,384,576,1152)=");
      scanf("%d",&chose);
      printf("\n");
   }

   switch(chose)
   {
      case 12:
         baudrate=BAUD_1200;
         break;
      case 24:
         baudrate=BAUD_2400;
         break;
      case 48:
         baudrate=BAUD_4800;
         break; 
      case 96:
         baudrate=BAUD_9600;
         break;
      case 144:
         baudrate=BAUD_14400;
         break;
      case 192:
         baudrate=BAUD_19200;
         break;
      case 384:
         baudrate=BAUD_38400;
         break;
      case 576:
         baudrate=BAUD_57600;
         break;
      case 1152:
         baudrate=BAUD_115200;
         break;
      default:
         baudrate=BAUD_38400;
         break;
    }
   return baudrate;
}

static void printUartPortSetting(unsigned int port,
                        unsigned int rx_fifo_trigger_level,
                        unsigned int tx_fifo_trigger_level,
                        unsigned int baudrate, unsigned int parity,
                        unsigned int stop,unsigned int len,
                        unsigned int dma)
{
   unsigned int u32baudrate;

   u32baudrate=BaudRateSelect(ScanBaudRate[baudrate]);

   printf("RX_Level=%d TX_Level=%d ",rx_fifo_trigger_level,tx_fifo_trigger_level);

   switch (u32baudrate)
   {
      case BAUD_1200:
         printf("baudrate=1200 ");
         break;
      case BAUD_2400:
         printf("baudrate=2400 ");
         break;
      case BAUD_4800:
         printf("baudrate=4800 ");
         break; 
      case BAUD_9600:
         printf("baudrate=9600 ");
         break;
      case BAUD_14400:
         printf("baudrate=14400 ");
         break;
      case BAUD_19200:
         printf("baudrate=19200 ");
         break;
      case BAUD_38400:
         printf("baudrate=38400 ");
         break;
      case BAUD_57600:
         printf("baudrate=57600 ");
         break;
      case BAUD_115200:
         printf("baudrate=115200 ");
         break;
      default:
         printf("baudrate=38400 ");
         break;      
   }

   printf("parity=%d stop=%d len=%d dma=%d\n",parity,stop,len,dma);

}

static void UartRestPort(unsigned int port,
                        unsigned int rx_fifo_trigger_level,
                        unsigned int tx_fifo_trigger_level,
                        unsigned int baudrate, unsigned int parity,
                        unsigned int stop,unsigned int len,
                        unsigned int dma)
{
   unsigned int u32baudrate;

   u32baudrate=BaudRateSelect(ScanBaudRate[baudrate]);

   fLib_DisableSerialFifo(port);
   fLib_SerialInit(port, u32baudrate, parity, stop,len);
   fLib_SetSerialFifoCtrl(port,tx_fifo_trigger_level,rx_fifo_trigger_level,1,1,dma);  
}

static void uart_simple_test(void)
{
   int i;
   int port;
   char ch;

   printf ("Please choose test uart port:\n");

   for (i=0;i<IP_COUNT;i++)
     printf ("[%d] UART%d\n",i,i);

   scanf ("%d", &port);
   printf("\n");

   if ((port<0) || (port>IP_COUNT))
   {
      printf("Select wrong uart port!!\n");
      return;
   }
   else
   {
      printf("Switch to uart Port:[%d]\n",port);
      ch=fLib_GetSerialChar(IP_pa_base[port]);

      UartRestPort(IP_pa_base[port],DEFAULT_RX_TRIGGER_LEVEL,DEFAULT_TX_TRIGGER_LEVEL,8,PARITY_NONE,1,8,0);   

      fLib_PutSerialStr(IP_pa_base[port],"Now, you can key any key to test!!\n\r");
      fLib_PutSerialStr(IP_pa_base[port],"Press Esc or ctrl-c to exit:\n\r");

      while (1)
      {
         ch=fLib_GetSerialChar(IP_pa_base[port]);
         fLib_PutSerialChar(IP_pa_base[port],ch);

         if (ch=='\r')//'\r'
            fLib_PutSerialChar(IP_pa_base[port],'\n');
         if (ch==27){//esc
            fLib_PutSerialChar(IP_pa_base[port],'T');
            break;
         }
         if (ch==3) //ctrl-c
            break;
      }

      fLib_PutSerialStr(IP_pa_base[port],"\n\rTest finish!! Switch back uart0 Port!!\n\r");

      UartRestPort(IP_pa_base[0],DEFAULT_RX_TRIGGER_LEVEL,DEFAULT_TX_TRIGGER_LEVEL,8,PARITY_NONE,1,8,0); 

      printf("\n\rSwitch back UART0!!\n");
      ch=fLib_GetSerialChar(IP_pa_base[0]);
   }
}

static void uart_internal_loopback_test(void)
{
   unsigned int FIFO_trigger_level;
   unsigned int baudrate;
   unsigned int pariyt_bit;
   unsigned int stop_bit;
   unsigned int word_length;
   unsigned int port;
   unsigned char ch1,ch2;
   int i;

   printf ("Please choose test uart port:\n");

   for (i=1;i<IP_COUNT;i++)
      printf ("[%d] UART%d\n",i,i);

   scanf ("%d", &port);
   printf("\n");

   if ((port<=0) || (port>IP_COUNT))
   {
      printf("Select wrong uart port!!\n");
      return;
   }
   else
   {

      printf("UART%d Internal Loopback Test:\n",port);

      //Set UART port loopback mode
      fLib_SetSerialLoopBack(IP_pa_base[port],1);

      for(FIFO_trigger_level=0;FIFO_trigger_level<MAX_FIFO_TRIGGER_LEVEL_INDEX;FIFO_trigger_level++)//FIFO TRIGGER LEVEL
      {
         for(baudrate=0;baudrate<MAX_BAUDRATE_INDEX;baudrate++)//Baudrate
         {
            for(pariyt_bit=0;pariyt_bit<MAX_PARITY_INDEX;pariyt_bit++)//Parity bit
            {
               for(stop_bit=0;stop_bit<MAX_STOP_INDEX;stop_bit++)//stop bit & word length
               {
                  for(word_length=0;word_length<MAX_WORD_LENGTH_INDEX;word_length++)//stop bit & word length
                  {
                     printUartPortSetting(port,FIFO_trigger_level,FIFO_trigger_level
                     ,baudrate, pariyt_bit,stop_bit,(word_length+5),0);

                     //Internal
                     UartRestPort(IP_pa_base[port],FIFO_trigger_level,FIFO_trigger_level
                     ,baudrate, pariyt_bit,stop_bit,(word_length+5),0);

                     //send data to THR and compare with RBR
                     ch1=0x15; 
                     fLib_PutSerialChar(IP_pa_base[port], ch1);

                     ch2=fLib_GetSerialChar(IP_pa_base[port]); 
                     if(ch1 != ch2)
                     {
                        printf("Fail!\n");
                        return;
                     }
                     else
                     {
                        printf("Pass!\n");
                     }
                  }
               }
            }
         }
      }

      //Set UART port Normal mode
      fLib_SetSerialLoopBack(IP_pa_base[port],0);

      printf("UART%d internel loopback test is finished!!\n\n",port);
   }
}

static void uart_external_loopback_test(void)
{   
   unsigned int FIFO_trigger_level;
   unsigned int baudrate;
   unsigned int pariyt_bit;
   unsigned int stop_bit;
   unsigned int word_length;
   unsigned char ch1,ch2;

   unsigned int port1;
   unsigned int port2;
   unsigned int result = 0;
   unsigned char ch;
   int i;

   printf ("Please choose Send data uart port:\n");

   for (i=0;i<IP_COUNT;i++)
      printf ("[%d] UART%d\n",i,i);

   scanf ("%d", &port1);
   printf("\n");

   if ((port1<0) || (port1>IP_COUNT))
   {
      printf("Select wrong uart port!!\n");
      return;
   }

   printf ("\nPlease choose Receive data uart port:\n");

   for (i=0;i<IP_COUNT;i++)
   {
      if (i != port1)
         printf ("[%d] UART%d\n",i,i);
   }

   scanf ("%d", &port2);
   printf("\n");

   if ((port2<0) || (port2>IP_COUNT))
   {
      printf("Select wrong uart port!!\n");
      return;
   }

   printf("UART%d & UART%d External Loopback test start!!\n",port1,port2);

   //delay for connect cross over cable
   printf("\nWait for connect Cross over cable!!\n");
   Do_Delay(3000000);
   printf("Start to test!!\n");
   
   result = 1;
      
   for(FIFO_trigger_level=0;FIFO_trigger_level<MAX_FIFO_TRIGGER_LEVEL_INDEX;FIFO_trigger_level++)//FIFO TRIGGER LEVEL
   {
      for(baudrate=0;baudrate<MAX_BAUDRATE_INDEX;baudrate++)//Baudrate
      {
         for(pariyt_bit=0;pariyt_bit<MAX_PARITY_INDEX;pariyt_bit++)//Parity bit
         {
            for(stop_bit=0;stop_bit<MAX_STOP_INDEX;stop_bit++)//stop bit & word length
            {
               for(word_length=0;word_length<MAX_WORD_LENGTH_INDEX;word_length++)//stop bit & word length
               {
                  //External
                  UartRestPort(IP_pa_base[port1],FIFO_trigger_level,FIFO_trigger_level
                  ,baudrate, pariyt_bit,stop_bit,(word_length+5),0);
                  
                  UartRestPort(IP_pa_base[port2],FIFO_trigger_level,FIFO_trigger_level
                  ,baudrate, pariyt_bit,stop_bit,(word_length+5),0);
                        
                  //send data to THR and compare with RBR
                  ch1=0x15; 
                  fLib_PutSerialChar(IP_pa_base[port1], ch1);

                  ch2=fLib_GetSerialChar(IP_pa_base[port2]); 
                  if(ch1 != ch2)
                  {
                     result = 0;
                     break;
                  }
               }
            }
         }
      }
   }

   UartRestPort(IP_pa_base[0],DEFAULT_RX_TRIGGER_LEVEL,DEFAULT_TX_TRIGGER_LEVEL,8,PARITY_NONE,1,8,0);    
   ch=fLib_GetSerialChar(IP_pa_base[0]);

   if (result)
   {
      printf("test Pass!!\n");
   }
   else
   {
      printf("test Fail!!\n");
   }

   printf("UART%d & UART%d Externel loopback test is finished!!\n\n",port1,port2);
}


static void uart_HW_flow_control_dsr_dtr_test(void)
{
   unsigned int status;
   char ch;

   //DSR/DTR mode(DSR/DTR ON), detect Overrun ERROR
   printf("DSR/DTR is ON!! UART can not Send/Receive Any Data!!\n");
   fLib_SetDTR(IP_pa_base[0],1);
   fLib_SetDSR(IP_pa_base[0],1);

   Do_Delay(100000);

   fLib_SetDTR(IP_pa_base[0],0);
   fLib_SetDSR(IP_pa_base[0],0);

   printf("DSR/DTR is OFF!! UART can Send/Receive Data!!\n");

   //read data from FIFO
   do
   {
     status = GetUartStatus(IP_pa_base[0]);
     if (IsDataReady(status))
        ch=fLib_GetSerialChar(IP_pa_base[0]);   
   } while (IsDataReady(status));

   printf("UART0 Hardware Flow Control test(DSR/DTR) is finished!!\n\n");
}

static void uart_HW_flow_control_cts_rts_test(void)
{
   unsigned int status;
   int i;
   char ch;

   //Normal mode(cts/rts OFF), detect Overrun ERROR
   printf("CTS/RTS is OFF!! Ready to receive data!! Start to detect Overrun error!!\n");
   
   i=0;
   do
   {
     status = GetUartStatus(IP_pa_base[0]);
     Do_Delay(1);
     i++;
     if (i>100000)
        break;
   } while (!IsOverrunError(status));

   if (i<=100000)
   {
      printf("Got Overrun Error!!\n");
      printf("Test PASS!!\n");
   }
   else
   {
      printf("Timeout!!\n");
      printf("Test FAIL!!\n");
   }

   //read data from FIFO
   do
   {
      status = GetUartStatus(IP_pa_base[0]);
      if (IsDataReady(status))
         ch=fLib_GetSerialChar(IP_pa_base[0]);
   } while (IsDataReady(status));

   //CTS/RTS mode(CTS/RTS ON), detect Overrun ERROR
   printf("CTS/RTS is ON!! Ready to receive data!! Start to detect Overrun error!!\n");
   fLib_SetRTS(IP_pa_base[0],1);
   fLib_SetCTS(IP_pa_base[0],1);
   i=0;
   do
   {
      status = GetUartStatus(IP_pa_base[0]);  
      Do_Delay(10);
      i++;
      if (i>100000)
         break;
   } while (!IsOverrunError(status));

   if (i<=100000)
   {
      printf("Got Overrun Error!!\n");
      printf("Test FAIL!!\n");
   }  
   else
   {
      printf("Timeout!!\n");
      printf("Test PASS!!\n");
   }

   fLib_SetRTS(IP_pa_base[0],0);
   fLib_SetCTS(IP_pa_base[0],0);
   printf("CTS/RTS is OFF!!\n");

   //read data from FIFO
   do
   {
      status = GetUartStatus(IP_pa_base[0]);
      if (IsDataReady(status))
         ch=fLib_GetSerialChar(IP_pa_base[0]);
   } while (IsDataReady(status));

   printf("UART0 Hardware Flow Control test(CTS/RTS) is finished!!\n\n");
}


#ifdef CONFIG_FTDMAC030
void uart_dma_tx_test(unsigned int port)
{
   int length;
   int i;
   unsigned char dma_buf[256];
   unsigned int temp = 0;

   unsigned int dma_channel0_control_reg = 0;
   unsigned int dma_channel0_config_reg = 0;
   unsigned int dma_channel0_src_addr_reg = 0;
   unsigned int dma_channel0_dst_addr_reg = 0;
   unsigned int dma_channel0_lld_reg = 0;
   unsigned int dma_channel0_trans_size_reg = 0;

   for (i=0;i<(256/4);i+=4)
   {
      dma_buf[i] = 0x39;
      dma_buf[i+1] = 0x35;
      dma_buf[i+2] = 0x30;
      dma_buf[i+3] = 0x33;
   }
   length = 16;

   UartRestPort(IP_pa_base[0],DEFAULT_RX_TRIGGER_LEVEL,DEFAULT_TX_TRIGGER_LEVEL,8,PARITY_NONE,1,8,0);

   //Setting DMAC030 to start DMA
   dma_channel0_control_reg = 0x00080000;
//   dma_channel0_config_reg =  0x00002E00;// channel 7
   dma_channel0_config_reg =  0x00002000;// channel 7
   dma_channel0_src_addr_reg = &dma_buf[0];
   dma_channel0_dst_addr_reg = IP_pa_base[0];
   dma_channel0_lld_reg = 0x0;
   dma_channel0_trans_size_reg = length;

   //write register to FTDMAC030
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x100)),dma_channel0_control_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x104)),dma_channel0_config_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x108)),dma_channel0_src_addr_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x10C)),dma_channel0_dst_addr_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x110)),dma_channel0_lld_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x114)),dma_channel0_trans_size_reg);

   //enable DMA Channel0
   dma_channel0_control_reg = 0x00090000;
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x100)),dma_channel0_control_reg);

   //Polling check DMA is finished
   temp = inw((unsigned int*)(FTDMAC030_REG_BASE+0x114));
   while (temp != 0)
   {
      Do_Delay(100);
      temp = inw((unsigned int*)(FTDMAC030_REG_BASE+0x114));
   }

   //disable DMA Channel0
   dma_channel0_control_reg = 0x00080000;
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x100)),dma_channel0_control_reg);

   printf("\nThe Correct Pattern is =>\n");

   for (i=0;i<length;i++)
   {
      printf("%c",dma_buf[i]);
   }
   printf("\n");
}

void uart_dma_rx_test(unsigned int port)
{
   int length;
   int i;
   unsigned char dma_buf[256];
   unsigned int temp = 0;

   unsigned int dma_channel1_control_reg = 0;
   unsigned int dma_channel1_config_reg = 0;
   unsigned int dma_channel1_src_addr_reg = 0;
   unsigned int dma_channel1_dst_addr_reg = 0;
   unsigned int dma_channel1_lld_reg = 0;
   unsigned int dma_channel1_trans_size_reg = 0;

   for (i=0;i<256;i++)
      dma_buf[i] = 0;

   length = 8;

   UartRestPort(IP_pa_base[0],DEFAULT_RX_TRIGGER_LEVEL,DEFAULT_TX_TRIGGER_LEVEL,8,PARITY_NONE,1,8,0);    

   printf("Please type %d characters for test TX DMA!!\n",length);

   //Setting DMAC030 to start DMA
   dma_channel1_control_reg = 0x00200000;
//   dma_channel1_config_reg =  0x000000B0;// channel 6
   dma_channel1_config_reg =  0x00000088;// channel 6
   dma_channel1_src_addr_reg = IP_pa_base[0];
   dma_channel1_dst_addr_reg = &dma_buf[0];
   dma_channel1_lld_reg = 0x0;
   dma_channel1_trans_size_reg = length;
   
   //write register to FTDMAC030
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x120)),dma_channel1_control_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x124)),dma_channel1_config_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x128)),dma_channel1_src_addr_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x12C)),dma_channel1_dst_addr_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x130)),dma_channel1_lld_reg);
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x134)),dma_channel1_trans_size_reg);

   //enable DMA Channel1
   dma_channel1_control_reg = 0x00210000;
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x120)),dma_channel1_control_reg);

   //Polling check DMA is finished
   temp = inw((unsigned int*)(FTDMAC030_REG_BASE+0x134));
   while (temp != 0)
   {
      Do_Delay(100);
      temp = inw((unsigned int*)(FTDMAC030_REG_BASE+0x134));
   }

   //disable DMA Channel1
   dma_channel1_control_reg = 0x00200000;
   outw(((unsigned int*)(FTDMAC030_REG_BASE+0x120)),dma_channel1_control_reg);

   //print dma buff
   for (i=0;i<256;i++)
   {
      if (dma_buf[i])
         printf("%c",dma_buf[i]);
   }
    printf("\n\n");
}


static void uart_dma_main(void)
{
   printf("\nUART0 DMA TX Test!!\n");
   uart_dma_tx_test(IP_pa_base[0]); 

   printf("\nUART0 DMA RX Test!!\n");
   uart_dma_rx_test(IP_pa_base[0]);

   printf("UART0 DMA test is finished!!\n\n");
}
#endif

void FTUART010_Test_Main (void)
{
   int item;

   while (1)
   {
      printf ("Please choose a test item:\n");
      printf ("[0] UART Simple Typing Test\n");
      printf ("[1] UART Internal loopback test\n");
//      printf ("[2] UART External loopback test (Plug in the Crossover connector)\n");
//      printf ("[3] UART Hardware Flow Control test (DSR/DTR)\n");
//      printf ("[4] UART Hardware Flow Control test (CTS/RTS)\n");
#ifdef CONFIG_FTDMAC030
      printf ("[5] UART DMA Test\n");
#endif
      printf ("[9] Exit UART Test\n");

      scanf ("%d", &item);
      printf("\n");

      switch (item)
      {
         case 0:
            uart_simple_test();
            break;
         case 1:
            uart_internal_loopback_test();
            break;
         case 2:
            uart_external_loopback_test();
            break;
         case 3:
            uart_HW_flow_control_dsr_dtr_test();
            break;
         case 4:
            uart_HW_flow_control_cts_rts_test();
            break;
#ifdef CONFIG_FTDMAC030
         case 5:
            uart_dma_main ();
            break;
#endif
         case 9:
            return;
            break;
         default:
            printf ( "Non-existed item, Return to main menu\n");
            break;
       }
    }
}

