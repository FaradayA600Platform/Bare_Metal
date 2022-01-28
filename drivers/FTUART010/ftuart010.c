/*******************************************************************************
* Copyright  Faraday Technology Corp 2020-2021.  All rights reserved.          *
*------------------------------------------------------------------------------*
* Name:ftuart010.c                                                             *
* Description: ftuart010 driver function source file                           *
* Author: Jack Chian                                                           *
*******************************************************************************/

/******************************************************************************
 * Include files
 *****************************************************************************/
#include "ftuart010.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/
#define BACKSP_KEY 0x08
#define RETURN_KEY 0x0D
#define DELETE_KEY 0x7F
#define BELL       0x07

//used for CLI
#define CLI_PORT DebugSerialPort

/******************************************************************************
 * Define gobal variable
 *****************************************************************************/
unsigned int DebugSerialPort = DEFAULT_CONSOLE;

/******************************************************************************
 *Public functions
 *****************************************************************************/

void fLib_SetSerialMode(unsigned int port, unsigned int mode)
{
   unsigned int mdr;

   mdr = inw((unsigned int*)(port + SERIAL_MDR));
   mdr &= ~SERIAL_MDR_MODE_SEL;
   outw(((unsigned int*)(port + SERIAL_MDR)), mdr | mode);
}


void fLib_EnableIRMode(unsigned int port, unsigned int TxEnable, unsigned int RxEnable)
{
   unsigned int acr;

   acr = inw((unsigned int*)(port + SERIAL_ACR));
   acr &= ~(SERIAL_ACR_TXENABLE | SERIAL_ACR_RXENABLE);
   if(TxEnable)
      acr |= SERIAL_ACR_TXENABLE;
   if(RxEnable)
      acr |= SERIAL_ACR_RXENABLE;
   outw(((unsigned int*)(port + SERIAL_ACR)), acr);
}

void fLib_SerialInit (unsigned int port, unsigned int baudrate, unsigned int parity,unsigned int num,unsigned int len)
{
   unsigned int lcr;

   lcr = inw((unsigned int*)(port + SERIAL_LCR)) & ~SERIAL_LCR_DLAB;
   /* Set DLAB=1 */
   outw(((unsigned int*)(port + SERIAL_LCR)),SERIAL_LCR_DLAB);
   /* Set baud rate */
   outw(((unsigned int*)(port + SERIAL_DLM)), ((baudrate & 0xff00) >> 8)); //ycmo090930
   outw(((unsigned int*)(port + SERIAL_DLL)), (baudrate & 0xff));

   //clear orignal parity setting
   lcr &= 0xc0;

   switch (parity)
   {
      case PARITY_NONE: 
         //do nothing
         break;

      case PARITY_ODD:
          lcr|=SERIAL_LCR_ODD;
          break;

      case PARITY_EVEN:
         lcr|=SERIAL_LCR_EVEN;
         break;

      case PARITY_MARK:
         lcr|=(SERIAL_LCR_STICKPARITY|SERIAL_LCR_ODD);
         break;

      case PARITY_SPACE:
         lcr|=(SERIAL_LCR_STICKPARITY|SERIAL_LCR_EVEN);
         break;

      default:
         break;
    }

   if(num==2)
      lcr|=SERIAL_LCR_STOP;

   len-=5;
   lcr|=len;

   outw(((unsigned int*)(port+SERIAL_LCR)),lcr);
}


void fLib_SetSerialLoopback(unsigned int port, unsigned int onoff)
{
   unsigned int temp;

   temp=inw((unsigned int*)(port+SERIAL_MCR));
   if(onoff==1)  
      temp|=SERIAL_MCR_LPBK;
   else
      temp&=~(SERIAL_MCR_LPBK);

   outw(((unsigned int*)(port+SERIAL_MCR)),temp);
}

void fLib_SetSerialFifoCtrl(unsigned int port, unsigned int level_tx, 
    unsigned int level_rx, unsigned int resettx, unsigned int resetrx,
    unsigned int dma)
{
   unsigned char fcr = 0;

   fcr |= SERIAL_FCR_FE;

   switch(level_rx)
   {
      case 1:
         fcr|=0x40;
         break;

      case 2:
         fcr|=0x80;
         break;

      case 3:
         fcr|=0xc0;
         break;

      default:
         //printf("[%s]level_rx ERROR!! level_rx=0x%x\n",__func__,level_rx);
         break;
   }

   switch(level_tx)
   {
      case 1:
         fcr|=0x01<<4;
         break;

      case 2:
         fcr|=0x02<<4;
         break;

      case 3:
         fcr|=0x03<<4;
         break;

      default:
         //printf("[%s]level_tx ERROR!! level_tx=0x%x\n",__func__,level_tx);
         break;
   }

   if(resettx)
      fcr|=SERIAL_FCR_TXFR;

   if(resetrx)
      fcr|=SERIAL_FCR_RXFR;
      
   if(dma)
      fcr|=SERIAL_FCR_DMA_MODE;

   outw(((unsigned int*)(port+SERIAL_FCR)),fcr);
}

void fLib_DisableSerialFifo(unsigned int port)
{
   outw(((unsigned int*)(port+SERIAL_FCR)),0);
}

void fLib_SetSerialInt(unsigned int port, unsigned int IntMask)
{
   outw(((unsigned int*)(port + SERIAL_IER)), IntMask);
}

char fLib_GetSerialChar(unsigned int port)
{
   char Ch;
   unsigned int status;

   do
   {
      status=inw((unsigned int*)(port+SERIAL_LSR));
   } while (!((status & SERIAL_LSR_DR)==SERIAL_LSR_DR));

   Ch = inw((unsigned int*)(port + SERIAL_RBR));
   return (Ch);
}

void fLib_PutSerialChar(unsigned int port, char Ch)
{
   unsigned int status;

   do
   {
      status=inw((unsigned int*)(port+SERIAL_LSR));
   }while (!((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE));

   outw(((unsigned int*)(port + SERIAL_THR)),Ch);
}

void fLib_PutSerialStr(unsigned int port, char *Str)
{
   char *cp;

   for(cp = Str; *cp != 0; cp++)
      fLib_PutSerialChar(port, *cp);
}

void fLib_Modem_waitcall(unsigned int port)
{
   fLib_PutSerialStr(port, "ATS0=2\r");
}

void fLib_Modem_call(unsigned int port, char *tel)
{
   fLib_PutSerialStr(port, "ATDT");
   fLib_PutSerialStr(port,  tel);
   fLib_PutSerialStr(port, "\r");
}

#ifdef CONFIG_FTTIMER
int fLib_Modem_getchar(unsigned int port,int TIMEOUT)
{
   UINT64 start_time, middle_time, dead_time;
   unsigned int status;
   char ch;
   unsigned int n=0;

   start_time = fLib_CurrentT1Tick();
   dead_time = start_time + TIMEOUT;

   do
   {
      if(n>1000)
      {
         middle_time = fLib_CurrentT1Tick();
         if (middle_time > dead_time)
            return 0x100;
      }
      status = inw((unsigned int*)(port + SERIAL_LSR));
      n++;
   }while (!((status & SERIAL_LSR_DR)==SERIAL_LSR_DR));

    ch = inw((unsigned int*)(port + SERIAL_RBR));
    return (ch);
}

char fLib_Modem_putchar(unsigned int port, char Ch)
{
   unsigned long long start_time, middle_time, dead_time;
   unsigned int status;
   unsigned int n=0;

   start_time = fLib_CurrentT1Tick();
   dead_time = start_time + 5;

   do
   {
      if(n>1000)
      {
         middle_time = fLib_CurrentT1Tick();
         if (middle_time > dead_time)
            return 0;
      }
      status = inw((unsigned int*)(port + SERIAL_LSR));
      n++;
   }while (!((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE));

   outw(((unsigned int*)(port + SERIAL_THR)), Ch);

   return 1;
}
#endif //CONFIG_FTTIMER

void fLib_EnableSerialInt(unsigned int port, unsigned int mode)
{
   unsigned int data;

   data = inw((unsigned int*)(port + SERIAL_IER));
   outw(((unsigned int*)(port + SERIAL_IER)), data | mode);
}


void fLib_DisableSerialInt(unsigned int port, unsigned int mode)
{
   unsigned int data;

   data = inw((unsigned int*)(port + SERIAL_IER));
   mode = data & (~mode);  
   outw(((unsigned int*)(port + SERIAL_IER)), mode);
}

unsigned int fLib_SerialIntIdentification(unsigned int port)
{
   return inw((unsigned int*)(port + SERIAL_IIR));
}

void fLib_SetSerialLineBreak(unsigned int port)
{
   unsigned int data;

   data = inw((unsigned int*)(port + SERIAL_LCR));
   outw(((unsigned int*)(port + SERIAL_LCR)), data | SERIAL_LCR_SETBREAK);
}

void fLib_SetSerialLoopBack(unsigned int port,unsigned int onoff)
{
   unsigned int temp;

   temp = inw((unsigned int*)(port+SERIAL_MCR));
   if(onoff == 1)
      temp |= SERIAL_MCR_LPBK;
   else
      temp &= ~(SERIAL_MCR_LPBK);

   outw(((unsigned int*)(port+SERIAL_MCR)),temp);
}

void fLib_SetDMAMode2(unsigned int port,unsigned int onoff)
{
   unsigned int temp;

   temp = inw((unsigned int*)(port+SERIAL_MCR));
   if(onoff == 1)
      temp |= SERIAL_MCR_DMA_MODE2;
   else
      temp &= ~(SERIAL_MCR_DMA_MODE2);

   outw(((unsigned int*)(port+SERIAL_MCR)),temp);
}

void fLib_SetDMAMode(unsigned int port,unsigned int onoff)
{
   unsigned int temp;

   temp = inw((unsigned int*)(port+SERIAL_FCR));
   if(onoff == 1)
      temp |= SERIAL_FCR_DMA_MODE;
   else
      temp &= ~(SERIAL_FCR_DMA_MODE);

   outw(((unsigned int*)(port+SERIAL_FCR)),temp);
}

void fLib_SetRTS(unsigned int port,unsigned int onoff)
{
   unsigned int temp;

   temp = inw((unsigned int*)(port+SERIAL_IER));
   if(onoff == 1)
      temp |= SERIAL_IER_RTS;
   else
      temp &= ~(SERIAL_IER_RTS);

   outw(((unsigned int*)(port+SERIAL_IER)),temp);
}

void fLib_SetCTS(unsigned int port,unsigned int onoff)
{
   unsigned int temp;

   temp = inw((unsigned int*)(port+SERIAL_IER));
   if(onoff == 1)
      temp |= SERIAL_IER_CTS;
   else
      temp &= ~(SERIAL_IER_CTS);

   outw(((unsigned int*)(port+SERIAL_IER)),temp);
}

void fLib_SetDTR(unsigned int port,unsigned int onoff)
{
   unsigned int temp;

   temp = inw((unsigned int*)(port+SERIAL_IER));
   if(onoff == 1)
      temp |= SERIAL_IER_DTR;
   else
      temp &= ~(SERIAL_IER_DTR);

   outw(((unsigned int*)(port+SERIAL_IER)),temp);
}

void fLib_SetDSR(unsigned int port,unsigned int onoff)
{
   unsigned int temp;

   temp = inw((unsigned int*)(port+SERIAL_IER));
   if(onoff == 1)
      temp |= SERIAL_IER_DSR;
   else
      temp &= ~(SERIAL_IER_DSR);

   outw(((unsigned int*)(port+SERIAL_IER)),temp);
}

void fLib_SerialRequestToSend(unsigned int port)
{
   unsigned int data;

   data = inw((unsigned int*)(port + SERIAL_MCR));
   outw(((unsigned int*)(port + SERIAL_MCR)), data | SERIAL_MCR_RTS);
}

void fLib_SerialStopToSend(unsigned int port)
{
   unsigned int data;

   data = inw((unsigned int*)(port + SERIAL_MCR));
   data &= ~(SERIAL_MCR_RTS); 
   outw(((unsigned int*)(port + SERIAL_MCR)), data);
}

void fLib_SerialDataTerminalReady(unsigned int port)
{
   unsigned int data;

   data = inw((unsigned int*)(port + SERIAL_MCR));
   outw(((unsigned int*)(port + SERIAL_MCR)), data | SERIAL_MCR_DTR);
}

void fLib_SerialDataTerminalNotReady(unsigned int port)
{
   unsigned int data;

   data = inw((unsigned int*)(port + SERIAL_MCR));
   data &= ~(SERIAL_MCR_DTR); 
   outw(((unsigned int*)(port + SERIAL_MCR)), data);
}

unsigned int fLib_ReadSerialLineStatus(unsigned int port)
{
   return inw((unsigned int*)(port + SERIAL_LSR));
}

unsigned int fLib_ReadSerialModemStatus(unsigned int port)
{
   return inw((unsigned int*)(port + SERIAL_MSR));
}

unsigned int GetUartStatus(unsigned int port)
{
    unsigned int status;

    status=inw((unsigned int*)(port+SERIAL_LSR));

    return status; 
}

unsigned int IsThrEmpty(unsigned int status)
{
    if((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE)
       return 1;
    else
       return 0;
}

unsigned int IsDataReady(unsigned int status)
{
    if((status & SERIAL_IER_DR)==SERIAL_IER_DR)
       return 1;
    else
       return 0;
}

unsigned int IsOverrunError(unsigned int status)
{
    if((status & SERIAL_LSR_OE)==SERIAL_LSR_OE)
       return 1;
    else
       return 0;
}

void CheckRxStatus(unsigned int port)
{
   unsigned int Status;

   do
   {
      Status = GetUartStatus(port);
   }
   while (!IsDataReady(Status));
}

void CheckTxStatus(unsigned int port)
{
   unsigned int Status;

   do
   {
      Status = GetUartStatus(port);
   }while (!IsThrEmpty(Status));
}

unsigned int fLib_kbhit(unsigned int port)
{
   unsigned int Status;

   Status = GetUartStatus(port);
   if(IsDataReady(Status))
      return 1;
   else
      return 0;
}

char fLib_getch(unsigned int port)
{
   char ch;

   if(fLib_kbhit(port))
      ch=inw((unsigned int*)(port+SERIAL_RBR));
   else
      ch=0;

    return ch;
}  

void fLib_putc_port(char Ch,unsigned int port)
{
   CheckTxStatus(port);
   outw(((unsigned int*)(port+SERIAL_THR)),Ch);

   if (Ch == '\n')
   {
      CheckTxStatus(port);
      outw(((unsigned int*)(port+SERIAL_THR)),'\r');
   }
}

char fLib_getchar()
{
   char Ch;

   CheckRxStatus(CLI_PORT);
   Ch = inw((unsigned int*)(CLI_PORT+SERIAL_RBR));
   return (Ch);
}

void fLib_putchar(char Ch)
{
   if(Ch!='\0')
   {
      CheckTxStatus(CLI_PORT);
      outw(((unsigned int*)(CLI_PORT+SERIAL_THR)),Ch);
   }

   if (Ch == '\n')
   {
      CheckTxStatus(CLI_PORT);
      outw(((unsigned int*)(CLI_PORT+SERIAL_THR)),'\r');
   }
}


void fLib_putc(char Ch)
{
   CheckTxStatus(CLI_PORT);
   outw(((unsigned int*)(CLI_PORT+SERIAL_THR)),Ch);

   if (Ch == '\n')
   {
      CheckTxStatus(CLI_PORT);
      outw(((unsigned int*)(CLI_PORT+SERIAL_THR)),'\r');
   }
}


void fLib_putstr(char *str)
{
   char *cp;

   for(cp = str; *cp != 0; cp++)
      fLib_putchar(*cp);
}

