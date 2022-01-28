#ifndef __SERIAL_H 
#define __SERIAL_H 

//#include "FIE31XX.h"
#include "types.h"

/* ================================================================================ */
/* ================                    FT_UART0                    ================ */
/* ================================================================================ */
#define __I     volatile const      /*!< Defines 'read only' permissions            */
                                                                                   
#define __O     volatile            /*!< Defines 'write only' permissions           */
#define __IO    volatile            /*!< Defines 'read / write' permissions         */

/**
  * @brief UART0 (FT_UART0)
  */
typedef struct {                    /*!< FT_UART0 Structure                                                 */
    union {
        __IO UINT32  DLL;           /*!< Offset:0x00 UARTn Baud Rate Divisor Latch Least Significant
                                       Byte                                                                 */
        __O  UINT32  THR;           /*!< Offset:0x00 UARTn Transmit Holding Register                        */
        __I  UINT32  RBR;           /*!< Offset:0x00 UARTn Receiver Buffer Register                         */
    };
  
    union {
        __IO UINT32  DLM;           /*!< Offset:0x04 UARTn Baud Rate Divisor Latch Most Significant Byte    */
        __IO UINT32  IER;           /*!< Offset:0x04 UARTn Interrupt Enable Register                        */
    };
  
    union {
        __IO UINT32  PSR;           /*!< Offset:0x08 UARTn Prescalerl Register                              */
        __O  UINT32  FCR;           /*!< Offset:0x08 UARTn FIFO Control Register                            */
        __I  UINT32  IIR;           /*!< Offset:0x08 UARTn Interrupt Identification Register                */
    };

    __IO UINT32  LCR;               /*!< Offset:0x0C UARTn Line Control Register                            */
    __IO UINT32  MCR;               /*!< Offset:0x10 UARTn Modem Control Register                           */
  
    union {
        __O  UINT32  TST;           /*!< Offset:0x14 UARTn Testing Register                                 */
        __I  UINT32  LSR;           /*!< Offset:0x14 UARTn Line Status Register                             */
    };

    __I  UINT32  MSR;               /*!< Offset:0x18 UARTn Modem Status Register                            */
    __IO UINT32  SPR;               /*!< Offset:0x1C UARTn Scratch Pad Register                             */
    __IO UINT32  MDR;               /*!< Offset:0x20 UARTn Mode Definition Register                         */
    __IO UINT32  ACR;               /*!< Offset:0x24 UARTn Auxiliary Control Register                       */
    __IO UINT32  TXLENL;            /*!< Offset:0x28 UARTn Transmitter Frame Length Low                     */
    __IO UINT32  TXLENH;            /*!< Offset:0x2C UARTn Transmitter Frame Length High                    */
    __IO UINT32  MRXLENL;           /*!< Offset:0x30 UARTn Maximum Receiver Frame Length Low                */
    __IO UINT32  MRXLENH;           /*!< Offset:0x34 UARTn Maximum Receiver Frame Length High               */
    __IO UINT32  PLR;               /*!< Offset:0x38 UARTn FIR Preamble Length Register                     */

    union {
        __I  UINT32  FMIIR_DMA;     /*!< Offset:0x3C UARTn FIR Mode Interrupt Identification Register
                                      in the DMA mode                                                       */
        __I  UINT32  FMIIR_PIO;     /*!< Offset:0x3C UARTn FIR Mode Interrupt Identification Register
                                      in the PIO mode                                                       */
    };
} FT_UART0_Type;


typedef enum {
    DRVUART_PORT0=0,
    DRVUART_PORT1=1,
    DRVUART_PORT2=2,   
    DRVUART_PORT3=3,
    DRVUART_PORT4=4,
    DRVUART_PORT5=5,
    DRVUART_PORT6=6,
    DRVUART_PORT7=7,
    DRVUART_PORT8=8,
    DRVUART_PORT9=9,
    DRVUART_MAX_UART = 10,
} DRVUART_PORT;

typedef struct uart_dma_req_mode{   
	int mode;
	int irda_ntxrdy;
	int irda_nrxrdy;
} fLib_UART_DMAREQ;

#if defined(__PLATFORM_FIE3200F__) || defined(__PLATFORM_MODEM__)
extern FT_UART0_Type * UART_PORT[4];
#else
extern FT_UART0_Type * UART_PORT[DRVUART_MAX_UART];
#endif

#define FT_UART0    ((FT_UART0_Type *) UART_FTUART010_0_PA_BASE)
#define FT_UART1    ((FT_UART0_Type *) UART_FTUART010_1_PA_BASE)
#define FT_UART2    ((FT_UART0_Type *) UART_FTUART010_2_PA_BASE)
#define FT_UART3    ((FT_UART0_Type *) UART_FTUART010_3_PA_BASE)
#define FT_UART4    ((FT_UART0_Type *) UART_FTUART010_4_PA_BASE)
#define FT_UART5    ((FT_UART0_Type *) UART_FTUART010_5_PA_BASE)
#define FT_UART6    ((FT_UART0_Type *) UART_FTUART010_6_PA_BASE)
#define FT_UART7    ((FT_UART0_Type *) UART_FTUART010_7_PA_BASE)
#define FT_UART8    ((FT_UART0_Type *) UART_FTUART010_8_PA_BASE)
#define FT_UART9    ((FT_UART0_Type *) UART_FTUART010_9_PA_BASE)

/* IER Register */
#define SERIAL_IER_DR                   BIT0        /* Data ready Enable */
#define SERIAL_IER_TE                   BIT1        /* THR Empty Enable */
#define SERIAL_IER_RLS                  BIT2        /* Receive Line Status Enable */
#define SERIAL_IER_MS                   BIT3        /* Modem Staus Enable */
#define SERIAL_IER_RTSEN                BIT4        /* RTS Flow Control Enable */
#define SERIAL_IER_CTSEN                BIT5        /* CTS Flow Control Enable */

/* IIR Register */
#define SERIAL_IIR_NONE                 0x1         /* No interrupt pending */
#define SERIAL_IIR_RLS                  0x6         /* Receive Line Status */
#define SERIAL_IIR_DR                   0x4         /* Receive Data Ready */
#define SERIAL_IIR_TIMEOUT              0xc         /* Receive Time Out */
#define SERIAL_IIR_TE                   0x2         /* THR Empty */
#define SERIAL_IIR_MODEM                0x0         /* Modem Status */

/* FCR Register */
#define SERIAL_FCR_FE                   (BIT0)      /* FIFO Enable */
#define SERIAL_FCR_RXFR                 (BIT1)      /* Rx FIFO Reset */
#define SERIAL_FCR_TXFR                 (BIT2)      /* Tx FIFO Reset */
#define SERIAL_FCR_TXF_TRG_LVL_0        (0x00)      /* Tx FIFO Trigger Level */
#define SERIAL_FCR_TXF_TRG_LVL_1        (BIT4)      /* Tx FIFO Trigger Level */
#define SERIAL_FCR_TXF_TRG_LVL_2        (BIT5)      /* Tx FIFO Trigger Level */
#define SERIAL_FCR_TXF_TRG_LVL_3        (BIT5|BIT4) /* Tx FIFO Trigger Level */
#define SERIAL_FCR_RXF_TRG_LVL_0        (0x00)      /* Rx FIFO Trigger Level */
#define SERIAL_FCR_RXF_TRG_LVL_1        (BIT6)      /* Rx FIFO Trigger Level */
#define SERIAL_FCR_RXF_TRG_LVL_2        (BIT7)      /* Rx FIFO Trigger Level */
#define SERIAL_FCR_RXF_TRG_LVL_3        (BIT7|BIT6) /* Rx FIFO Trigger Level */

/* LCR Register */
#define SERIAL_LCR_LEN5                 0x0
#define SERIAL_LCR_LEN6                 0x1
#define SERIAL_LCR_LEN7                 0x2
#define SERIAL_LCR_LEN8                 0x3

#define SERIAL_LCR_STOP                 0x4
#define SERIAL_LCR_EVEN                 0x18        /* Even Parity */
#define SERIAL_LCR_ODD                  0x8         /* Odd Parity */
#define SERIAL_LCR_PE                   0x8         /* Parity Enable */
#define SERIAL_LCR_SETBREAK             0x40        /* Set Break condition */
#define SERIAL_LCR_STICKPARITY          0x20        /* Stick Parity Enable */
#define SERIAL_LCR_DLAB                 0x80        /* Divisor Latch Access Bit */

/* LSR Register */
#define SERIAL_LSR_DR                   BIT0        /* Data Ready */
#define SERIAL_LSR_OE                   BIT1        /* Overrun Error */
#define SERIAL_LSR_PE                   BIT2        /* Parity Error */
#define SERIAL_LSR_FE                   BIT3        /* Framing Error */
#define SERIAL_LSR_BI                   BIT4        /* Break Interrupt */
#define SERIAL_LSR_THRE                 BIT5        /* THR Empty */
#define SERIAL_LSR_TE                   BIT6        /* Transmitte Empty */
#define SERIAL_LSR_DE                   BIT7        /* FIFO Data Error */

/* MCR Register */
#define SERIAL_MCR_DTR                  BIT0        /* Data Terminal Ready */
#define SERIAL_MCR_RTS                  BIT1        /* Request to Send */
#define SERIAL_MCR_OUT1                 BIT2        /* output1 */
#define SERIAL_MCR_OUT2                 BIT3        /* output2 or global interrupt enable */
#define SERIAL_MCR_LPBK                 BIT4        /* loopback mode */
#define SERIAL_MCR_DMA2                 BIT5        /* DMA Mode 2 */

/* MSR Register */
#define SERIAL_MSR_DELTACTS             BIT0        /* Delta CTS */
#define SERIAL_MSR_DELTADSR             BIT1        /* Delta DSR */
#define SERIAL_MSR_TERI                 BIT2        /* Trailing Edge RI */
#define SERIAL_MSR_DELTACD              BIT3        /* Delta CD */
#define SERIAL_MSR_CTS                  BIT4        /* Clear To Send */
#define SERIAL_MSR_DSR                  BIT5        /* Data Set Ready */
#define SERIAL_MSR_RI                   BIT6        /* Ring Indicator */
#define SERIAL_MSR_DCD                  BIT7        /* Data Carrier Detect */

/* MDR register */
#define SERIAL_MDR_MODE_SEL             0x03
#define SERIAL_MDR_UART                 0x0
#define SERIAL_MDR_SIR                  0x1
#define SERIAL_MDR_FIR                  0x2

/* ACR register */
#define SERIAL_ACR_TXENABLE             BIT0
#define SERIAL_ACR_RXENABLE             BIT1
#define SERIAL_ACR_SET_EOT              BIT3

#define BAUD_921600                     (921600)
#define BAUD_115200                     (115200)
#define BAUD_57600                      (57600) 
#define BAUD_38400                      (38400)
#define BAUD_19200                      (19200)
#define BAUD_14400                      (14400)
#define BAUD_9600                       (9600)
#define BAUD_4800                       (4800)
#define BAUD_2400                       (2400)
#define BAUD_1200                       (1200)

#define DEBUG_CONSOLE                   DRVUART_PORT0
#define DEFAULT_CONSOLE_BAUD            BAUD_115200

#ifndef PARITY_NONE
#define PARITY_NONE                     0
#endif

#ifndef PARITY_ODD
#define PARITY_ODD                      1
#endif

#ifndef PARITY_EVEN
#define PARITY_EVEN                     2
#endif

#ifndef PARITY_MARK
#define PARITY_MARK                     3
#endif

#ifndef PARITY_SPACE
#define PARITY_SPACE                    4
#endif

#ifndef ON
#define ON                              1
#endif

#ifndef OFF
#define OFF                             0
#endif

#define BACKSP_KEY                      0x08
#define RETURN_KEY                      0x0D
#define DELETE_KEY                      0x7F
#define BELL                            0x07

/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
extern INT32 fLib_GetSerialDMAREQ(UINT32 id, fLib_UART_DMAREQ *req);
extern void fLib_EnableSerialDMAMode2(DRVUART_PORT port_no);
extern void fLib_SetSerialMode(DRVUART_PORT port_no, UINT32 mode);
extern void fLib_EnableIRMode(DRVUART_PORT port_no, UINT32 TxEnable, UINT32 RxEnable);
extern void fLib_SerialInit (DRVUART_PORT port_no, UINT32 baudrate, UINT32 parity,UINT32 num,UINT32 len);
extern void fLib_SetSerialLoopback(DRVUART_PORT port_no, UINT32 onoff);
extern void fLib_SetSerialFifoCtrl(DRVUART_PORT port_no, UINT32 level_tx, UINT32 level_rx, UINT32 resettx, UINT32 resetrx);  //V1.20//ADA10022002
extern void fLib_SetSerialFifoRst(DRVUART_PORT port_no, UINT32 resettx, UINT32 resetrx);
extern void fLib_DisableSerialFifo(DRVUART_PORT port_no);
extern void fLib_SetSerialInt(DRVUART_PORT port_no, UINT32 IntMask);
extern char fLib_GetSerialChar(DRVUART_PORT port_no);
extern void fLib_PutSerialChar(DRVUART_PORT port_no, char Ch);
extern void fLib_PutSerialStr(DRVUART_PORT port_no, char *Str);
extern void fLib_Modem_waitcall(DRVUART_PORT port_no);
extern void fLib_Modem_call(DRVUART_PORT port_no, char *tel);
extern void fLib_EnableSerialInt(DRVUART_PORT port_no, UINT32 mode);
extern void fLib_DisableSerialInt(DRVUART_PORT port_no, UINT32 mode);
extern UINT32 fLib_ReadSerialIER(DRVUART_PORT port_no);
extern UINT32 fLib_SerialIntIdentification(DRVUART_PORT port_no);
extern void fLib_SetSerialLineBreak(DRVUART_PORT port_no);
extern void fLib_SerialRequestToSend(DRVUART_PORT port_no);
extern void fLib_SerialStopToSend(DRVUART_PORT port_no);
extern void fLib_SerialDataTerminalReady(DRVUART_PORT port_no);
extern void fLib_SerialDataTerminalNotReady(DRVUART_PORT port_no);
extern UINT32 fLib_ReadSerialLineStatus(DRVUART_PORT port_no);
extern UINT32 fLib_ReadSerialModemStatus(DRVUART_PORT port_no);
extern UINT32 GetUartStatus(DRVUART_PORT port_no);
extern UINT32 IsThrEmpty(UINT32 status);
extern UINT32 IsDataReady(UINT32 status);
extern void CheckRxStatus(DRVUART_PORT port_no);
extern void CheckTxStatus(DRVUART_PORT port_no);
extern UINT32 fLib_kbhit(DRVUART_PORT port_no);
extern char fLib_getch(DRVUART_PORT port_no);
extern char fLib_getchar(DRVUART_PORT port_no);
extern char fLib_getchar_timeout(DRVUART_PORT port_no, unsigned long timeout);
extern void fLib_putchar(DRVUART_PORT port_no, char Ch);
extern void fLib_putc(DRVUART_PORT port_no, char Ch);
extern void fLib_putstr(DRVUART_PORT port_no, char *str);
extern void fLib_printf(const char *f, ...);	/* variable arguments */
extern int fLib_gets(DRVUART_PORT port_no, char *buf);
extern void fLib_DebugPrintChar(DRVUART_PORT port_no, char ch);
extern void fLib_DebugPrintString(DRVUART_PORT port_no, char *str);
extern char fLib_DebugGetChar(DRVUART_PORT port_no);
extern UINT32 fLib_DebugGetUserCommand(DRVUART_PORT port_no, UINT8 * buffer, UINT32 Len);
#endif // __SERIAL_H 
