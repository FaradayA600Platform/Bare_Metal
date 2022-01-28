/*******************************************************************************
* Copyright  Faraday Technology Corp 2020-2021.  All rights reserved.          *
*------------------------------------------------------------------------------*
* Name:ftuart010.h                                                             *
* Description: ftuart010 driver function header file                           *
* Author: Jack Chian                                                           *
*******************************************************************************/
#ifndef __FTUART010_H 
#define __FTUART010_H 

#include "autoconf.h"

/******************************************************************************
 * Include files
 *****************************************************************************/
#if defined(CONFIG_BSP_LITE)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "platform_AXI.h"

#define printf prints

#define DEFAULT_CONSOLE UART0_REG_BASE

#define PARITY_NONE     0
#define PARITY_ODD      1
#define PARITY_EVEN     2
#define PARITY_MARK     3
#define PARITY_SPACE    4

#elif defined(CONFIG_BSP_A380) || defined(CONFIG_PLATFORM_A600)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "SoFlexible.h"
#include "system.h"

#elif defined(CONFIG_BSP_NONE)

#include "portme_ftuart010.h"

#endif
/******************************************************************************
 * Define Constants
 *****************************************************************************/
#define SERIAL_THR                     0x00   /* Transmitter Holding Register(Write).*/
#define SERIAL_RBR                     0x00   /* Receive Buffer register (Read).*/
#define SERIAL_IER                     0x04   /* Interrupt Enable register.*/
#define SERIAL_IIR                     0x08   /* Interrupt Identification register(Read).*/
#define SERIAL_FCR                     0x08   /* FIFO control register(Write).*/
#define SERIAL_LCR                     0x0C   /* Line Control register.*/
#define SERIAL_MCR                     0x10   /* Modem Control Register.*/
#define SERIAL_LSR                     0x14   /* Line status register(Read) .*/
#define SERIAL_MSR                     0x18   /* Modem Status register (Read).*/
#define SERIAL_SPR                     0x1C   /* Scratch pad register */
#define SERIAL_DLL                     0x00   /* Divisor Register LSB */
#define SERIAL_DLM                     0x04   /* Divisor Register MSB */
#define SERIAL_PSR                     0x08   /* Prescale Divison Factor */

#define SERIAL_MDR                     0x20
#define SERIAL_ACR                     0x24
#define SERIAL_TXLENL                  0x28
#define SERIAL_TXLENH                  0x2C
#define SERIAL_MRXLENL                 0x30
#define SERIAL_MRXLENH                 0x34
#define SERIAL_PLR                     0x38
#define SERIAL_FMIIR_PIO               0x3C

/* IER Register */
#define SERIAL_IER_DR                  0x01   /* Data ready Enable */
#define SERIAL_IER_TE                  0x02   /* THR Empty Enable */
#define SERIAL_IER_RLS                 0x04   /* Receive Line Status Enable */
#define SERIAL_IER_MS                  0x08   /* Modem Staus Enable */
#define SERIAL_IER_RTS                 0x10   /* RTS flow control enabled */
#define SERIAL_IER_CTS                 0x20   /* CTS flow control enabled */
#define SERIAL_IER_DTR                 0x40   /* DTR flow control enabled */
#define SERIAL_IER_DSR                 0x80   /* DSR flow control enabled */

/* IIR Register */
#define SERIAL_IIR_NONE                0x01   /* No interrupt pending */
#define SERIAL_IIR_RLS                 0x06   /* Receive Line Status */
#define SERIAL_IIR_DR                  0x04   /* Receive Data Ready */
#define SERIAL_IIR_TIMEOUT             0x0c   /* Receive Time Out */
#define SERIAL_IIR_TE                  0x02   /* THR Empty */
#define SERIAL_IIR_MODEM               0x00   /* Modem Status */

/* FCR Register */
#define SERIAL_FCR_FE                  0x01   /* FIFO Enable */
#define SERIAL_FCR_RXFR                0x02   /* Rx FIFO Reset */
#define SERIAL_FCR_TXFR                0x04   /* Tx FIFO Reset */
#define SERIAL_FCR_DMA_MODE            0x08   /* DMA mode */

/* LCR Register */
#define SERIAL_LCR_LEN5                0x00
#define SERIAL_LCR_LEN6                0x01
#define SERIAL_LCR_LEN7                0x02
#define SERIAL_LCR_LEN8                0x03

#define SERIAL_LCR_STOP                0x04
#define SERIAL_LCR_EVEN                0x18   /* Even Parity */
#define SERIAL_LCR_ODD                 0x08   /* Odd Parity */
#define SERIAL_LCR_PE                  0x08   /* Parity Enable */
#define SERIAL_LCR_SETBREAK            0x40   /* Set Break condition */
#define SERIAL_LCR_STICKPARITY         0x20   /* Stick Parity Enable */
#define SERIAL_LCR_DLAB                0x80   /* Divisor Latch Access Bit */

/* LSR Register */
#define SERIAL_LSR_DR                  0x01   /* Data Ready */
#define SERIAL_LSR_OE                  0x02   /* Overrun Error */
#define SERIAL_LSR_PE                  0x04   /* Parity Error */
#define SERIAL_LSR_FE                  0x08   /* Framing Error */
#define SERIAL_LSR_BI                  0x10   /* Break Interrupt */
#define SERIAL_LSR_THRE                0x20   /* THR Empty */
#define SERIAL_LSR_TE                  0x40   /* Transmitte Empty */
#define SERIAL_LSR_DE                  0x80   /* FIFO Data Error */

/* MCR Register */
#define SERIAL_MCR_DTR                 0x01   /* Data Terminal Ready */
#define SERIAL_MCR_RTS                 0x02   /* Request to Send */
#define SERIAL_MCR_OUT1                0x04   /* output   1 */
#define SERIAL_MCR_OUT2                0x08   /* output2 or global interrupt enable */
#define SERIAL_MCR_LPBK                0x10   /* loopback mode */
#define SERIAL_MCR_DMA_MODE2           0x20   /* DMA mode 2 */


/* MSR Register */
#define SERIAL_MSR_DELTACTS            0x01   /* Delta CTS */
#define SERIAL_MSR_DELTADSR            0x02   /* Delta DSR */
#define SERIAL_MSR_TERI                0x04   /* Trailing Edge RI */
#define SERIAL_MSR_DELTACD             0x08   /* Delta CD */
#define SERIAL_MSR_CTS                 0x10   /* Clear To Send */
#define SERIAL_MSR_DSR                 0x20   /* Data Set Ready */
#define SERIAL_MSR_RI                  0x40   /* Ring Indicator */
#define SERIAL_MSR_DCD                 0x80   /* Data Carrier Detect */


/* MDR register */
#define SERIAL_MDR_MODE_SEL            0x03
#define SERIAL_MDR_UART                0x00
#define SERIAL_MDR_SIR                 0x01
#define SERIAL_MDR_FIR                 0x02

/* ACR register */
#define SERIAL_ACR_TXENABLE            0x01
#define SERIAL_ACR_RXENABLE            0x02
#define SERIAL_ACR_SET_EOT             0x04


#define BAUD_115200                   (UART_CLOCK / 1843200)
#define BAUD_57600                    (UART_CLOCK / 921600)
#define BAUD_38400                    (UART_CLOCK / 614400)
#define BAUD_19200                    (UART_CLOCK / 307200)
#define BAUD_14400                    (UART_CLOCK / 230400)
#define BAUD_9600                     (UART_CLOCK / 153600)
#define BAUD_4800                     (UART_CLOCK / 76800)
#define BAUD_2400                     (UART_CLOCK / 38400)
#define BAUD_1200                     (UART_CLOCK / 19200)

/******************************************************************************
 *Public functions
 *****************************************************************************/

extern unsigned int DebugSerialPort;
extern unsigned int SystemSerialPort;

extern void fLib_SerialInit(unsigned int port, unsigned int baudrate, unsigned int parity,unsigned int num,unsigned int len);
extern void fLib_SetSerialFifoCtrl(unsigned int port, unsigned int level_tx, unsigned int level_rx,unsigned int resettx, unsigned int resetrx, unsigned int dma); //V1.20//ADA10022002
extern void fLib_DisableSerialFifo(unsigned int port);
extern void fLib_SetSerialInt(unsigned int port, unsigned int IntMask);

extern char fLib_GetSerialChar(unsigned int port);
extern void fLib_PutSerialChar(unsigned int port, char Ch);
extern void fLib_PutSerialStr(unsigned int port, char *Str);

extern void fLib_EnableSerialInt(unsigned int port, unsigned int mode);
extern void fLib_DisableSerialInt(unsigned int port, unsigned int mode);

extern void fLib_SerialRequestToSend(unsigned int port);
extern void fLib_SerialStopToSend(unsigned int port);
extern void fLib_SerialDataTerminalReady(unsigned int port);
extern void fLib_SerialDataTerminalNotReady(unsigned int port);

extern void fLib_SetSerialLineBreak(unsigned int port);
extern void fLib_SetSerialLoopBack(unsigned int port,unsigned int onoff);
extern void fLib_SetRTS(unsigned int port,unsigned int onoff);
extern void fLib_SetCTS(unsigned int port,unsigned int onoff);
extern void fLib_SetDTR(unsigned int port,unsigned int onoff);
extern void fLib_SetDSR(unsigned int port,unsigned int onoff);
extern unsigned int fLib_SerialIntIdentification(unsigned int port);

extern unsigned int fLib_ReadSerialLineStatus(unsigned int port);
extern unsigned int fLib_ReadSerialModemStatus(unsigned int port);

extern void fLib_SetSerialMode(unsigned int port, unsigned int mode);
extern void fLib_EnableIRMode(unsigned int port, unsigned int TxEnable, unsigned int RxEnable);

extern void fLib_Modem_call(unsigned int port, char *tel);
extern void fLib_Modem_waitcall(unsigned int port);
extern int  fLib_Modem_getchar(unsigned int port,int TIMEOUT);
extern char fLib_Modem_putchar(unsigned int port, char Ch);

extern char fLib_getch(unsigned int port);

extern void fLib_putc_port(char Ch,unsigned int port);
extern void fLib_printf_port(char Ch,unsigned int port);

extern unsigned int GetUartStatus(unsigned int port);
extern unsigned int IsThrEmpty(unsigned int status);
extern unsigned int IsDataReady(unsigned int status);
extern unsigned int IsOverrunError(unsigned int status);

#endif // __FTUART010_H 
