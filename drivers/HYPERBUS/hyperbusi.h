/* hyperbusi.h - Source Code for HyperBus CTRL IP Driver */
/**************************************************************************
* Copyright (C)2014 Spansion LLC. All Rights Reserved .
*
* This software is owned and published by:
* Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software constitutes driver source code for use in programming Spansion's
* Flash memory components. This software is licensed by Spansion to be adapted only
* for use in systems utilizing Spansion's Flash memories. Spansion is not be
* responsible for misuse or illegal use of this software for devices not
* supported herein.  Spansion is providing this source code "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the source code herein.
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED
* USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY,
* FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  SPANSION WILL
* HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR
* OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE,
* INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS,
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Copyright notice must be included with
* this software, whether used in part or whole, at all times.
*/
#ifndef __HYPERBUSI_H__
#define __HYPERBUSI_H__

#include "hyperbusi_target_specific.h"
#include "types.h"

#define HYPERBUSI_DRV_VERSION   "16.1.1"   //  Year.Quarter.Minor

//=============================================================================
//  DEFINITIONS
//=============================================================================
#define CS0_SEL                  0
#define CS1_SEL                  1

//
#define MAX_CMD_LEN	128
#define	CMDLEN			50
#define	MAXARGS			20
#define BIT0                            0x00000001
#define BIT1                            0x00000002
//

// hyperbus interface register operation status
typedef enum {
   NO_ERROR = 0,
   ERROR = -1,
} HYPERBUSI_STATUS;

#if (HYPERBUSI_IP_VER >= 220)
#define REG_CSR_ADDR    0x00000000
#endif
#define REG_IEN_ADDR    0x00000004
#define REG_ISR_ADDR    0x00000008
#define REG_ICR_ADDR    0x0000000C
#define REG_MBR0_ADDR   0x00000010
#define REG_MBR1_ADDR   0x00000014
#define REG_MBR2_ADDR   0x00000018
#define REG_MBR3_ADDR   0x0000001C
#define REG_MCR0_ADDR   0x00000020
#define REG_MCR1_ADDR   0x00000024
#define REG_MCR2_ADDR   0x00000028
#define REG_MCR3_ADDR   0x0000002C
#define REG_MTR0_ADDR   0x00000030
#define REG_MTR1_ADDR   0x00000034
#define REG_GPOR_ADDR   0x00000040
#define REG_WPR_ADDR    0x00000044
#define REG_LBR_ADDR    0x00000048
#if (HYPERBUSI_IP_VER >= 240)
#define REG_TAR_ADDR    0x0000004C
#endif

#if (HYPERBUSI_IP_VER >= 220)
#define WRSTOERR_BIT    0x04000000
#define WTRSERR_BIT     0x02000000
#define WDECERR_BIT     0x01000000
#define WACT_BIT        0x00010000
#define RDSSTALL_BIT    0x00000800
#define RRSTOERR_BIT    0x00000400
#define RTRSERR_BIT     0x00000200
#define RDECERR_BIT     0x00000100
#define RACT_BIT        0x00000001
#else
#define WTQ_BIT         0x01FF0000
#define RTQ_BIT         0x0000001F
#endif

#define INTP_BIT        0x80000000
#define RPCINTE_BIT     0x00000001
#define RPCINTS_BIT     0x00000001
#define A_BIT           0xFF000000
#if (HYPERBUSI_IP_VER >= 230)
#define MAXEN_BIT       0x80000000
#define MAXLEN_BIT      0x07FC0000
#endif
#define TCMO_BIT        0x00020000
#define ACS_BIT         0x00010000
#define CRT_BIT         0x00000020
#define DEVTYPE_BIT     0x00000010
#define WRAPSIZE_BIT    0x00000003
#define RCSHI_BIT       0xF0000000
#define WCSHI_BIT       0x0F000000
#define RCSS_BIT        0x00F00000
#define WCSS_BIT        0x000F0000
#define RCSH_BIT        0x0000F000
#define WCSH_BIT        0x00000F00
#define LTCY_BIT        0x0000000F
#define GPO_BIT         0x00000003
#define WM_BIT          0x00000003
#define WP_BIT          0x00000001
#define LOOPBACK_BIT    0x00000001

#if (HYPERBUSI_IP_VER >= 240)
#define RTA_BIT         0x00000030
#define WTA_BIT         0x00000003
#endif

//#define BIT0            0x00000001
//#define BIT1            (BIT0 << 1)

#if (HYPERBUSI_IP_VER >= 230)
#define MAXEN_DISABLED           (0)
#define MAXEN_ENABLED            (1)

#define SET_MAXEN(value)         ((value & BIT0) << 31)
#define CLR_MAXEN                (~(BIT0 << 31))
#define GET_MAXEN(value)         ((value >> 31) & BIT0)

#define SET_MAXLEN(value)        ((value << 18) & MAXLEN_BIT)
#define CLR_MAXLEN(value)        (value & ~MAXLEN_BIT)
#define GET_MAXLEN(value)        ((value & MAXLEN_BIT) >> 18)
#endif

#if (HYPERBUSI_IP_VER >= 220)
#define CSR_DEFAULT              (0)

#define GET_WRSTOERR(value)      ((value >> 26) & BIT0)
#define GET_WTRSERR(value)       ((value >> 25) & BIT0)
#define GET_WDECERR(value)       ((value >> 24) & BIT0)
#define GET_WACT(value)          ((value >> 16) & BIT0)
#define GET_RDSSTALL(value)      ((value >> 11) & BIT0)
#define GET_RRSTOERR(value)      ((value >> 10) & BIT0)
#define GET_RTRSERR(value)       ((value >>  9) & BIT0)
#define GET_RDECERR(value)       ((value >>  8) & BIT0)
#define GET_RACT(value)          (value & BIT0)
#endif

#define GET_WP(value)            (value & BIT0)
#define SET_INT_EN(value)        (value & BIT0)
#define CLR_INT_EN               (~BIT0)
#define GET_INT_EN(value)        (value & BIT0)

#define SET_INT_POLARITY(value)  ((value << 31) & INTP_BIT)
#define CLR_INT_POLARITY         (~INTP_BIT)
#define GET_INT_POLARITY(value)  ((value >> 31) & BIT0)

#define SET_TC_OPTION(value)     ((value & BIT0) << 17)
#define CLR_TC_OPTION            (~(BIT0 << 17))
#define GET_TC_OPTION(value)     ((value >> 17) & BIT0)

#define SET_WRAP_SIZE(value)     (value & (BIT0|BIT1))
#define CLR_WRAP_SIZE            (~(BIT0|BIT1))
#define GET_WRAP_SIZE(value)     (value & (BIT0|BIT1))

#define SET_ACACHE(value)        ((value & BIT0) << 16)
#define CLR_ACACHE               (~(BIT0 << 16))
#define GET_ACACHE(value)        ((value >> 16) & BIT0)

#define SET_8BIT_CTRL(value)     ((value & BIT0) <<  4)
#define CLR_8BIT_CTRL            (~(BIT0 <<  4))
#define GET_8BIT_CTRL(value)     ((value >>  4) & BIT0)

#define SET_DEVTYPE(value)       ((value & BIT0) <<  4)
#define CLR_DEVTYPE              (~(BIT0 <<  4))
#define GET_DEVTYPE(value)       ((value >>  4) & BIT0)

#define SET_CRT(value)           ((value & BIT0) <<  5)
#define CLR_CRT                  (~(BIT0 <<  5))
#define GET_CRT(value)           ((value >>  5) & BIT0)

#define SET_GPO(value, bit)      ((value & BIT0) << (bit & 31))
#define CLR_GPO(bit)             (~(BIT0 << (bit & 31)))
#define GET_GPO(value, bit)      ((value >> (bit & 31)) & BIT0)

#define SET_RCSHI(value)         ((value << 28) & RCSHI_BIT)
#define SET_WCSHI(value)         ((value << 24) & WCSHI_BIT)
#define SET_RCSS(value)          ((value << 20) & RCSS_BIT)
#define SET_WCSS(value)          ((value << 16) & WCSS_BIT)
#define SET_RCSH(value)          ((value << 12) & RCSH_BIT)
#define SET_WCSH(value)          ((value <<  8) & WCSH_BIT)
#define SET_LTCY(value)          ((value <<  0) & LTCY_BIT)

#define CLR_RCSHI(value)         (value & ~RCSHI_BIT)
#define CLR_WCSHI(value)         (value & ~WCSHI_BIT)
#define CLR_RCSS(value)          (value & ~RCSS_BIT)
#define CLR_WCSS(value)          (value & ~WCSS_BIT)
#define CLR_RCSH(value)          (value & ~RCSH_BIT)
#define CLR_WCSH(value)          (value & ~WCSH_BIT)
#define CLR_LTCY(value)          (value & ~LTCY_BIT)

#define GET_RCSHI(value)         ((value & RCSHI_BIT) >> 28)
#define GET_WCSHI(value)         ((value & WCSHI_BIT) >> 24)
#define GET_RCSS(value)          ((value & RCSS_BIT)  >> 20)
#define GET_WCSS(value)          ((value & WCSS_BIT)  >> 16)
#define GET_RCSH(value)          ((value & RCSH_BIT)  >> 12)
#define GET_WCSH(value)          ((value & WCSH_BIT)  >>  8)
#define GET_LTCY(value)          ((value & LTCY_BIT)  >>  0)

#if (HYPERBUSI_IP_VER >= 240)
#define SET_RTA(value)         ((value << 4) & RTA_BIT)
#define SET_WTA(value)         ((value << 0) & WTA_BIT)

#define CLR_RTA(value)         (value & ~RTA_BIT)
#define CLR_WTA(value)         (value & ~WTA_BIT)

#define GET_RTA(value)         ((value & RTA_BIT) >> 4)
#define GET_WTA(value)         ((value & WTA_BIT) >> 0)
#endif


//=============================================================================
//  PROTOTYPES
//=============================================================================
/* Controller Status Register */
#if (HYPERBUSI_IP_VER >= 220)
HYPERBUSI_STATUS getHYPERBUSI_CSR(DWORD base_addr, DWORD* reg_value);
#endif
/* Interrupt Enable Register */
#ifdef HYPERBUSI_IP_INT_SUPPORT
HYPERBUSI_STATUS setHYPERBUSI_IEN(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS setHYPERBUSI_IEN_INTP(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS setHYPERBUSI_IEN_RPCINTE(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS getHYPERBUSI_IEN(DWORD base_addr, DWORD* reg_value);
HYPERBUSI_STATUS getHYPERBUSI_IEN_INTP(DWORD base_addr, DWORD* reg_value);
HYPERBUSI_STATUS getHYPERBUSI_IEN_RPCINTE(DWORD base_addr, DWORD* reg_value);
/* Interrupt Status Register */
HYPERBUSI_STATUS getHYPERBUSI_ISR(DWORD base_addr, DWORD* reg_value);
#endif
/* Memory Base Address Register */
HYPERBUSI_STATUS setHYPERBUSI_MBR(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MBR(DWORD base_addr, DWORD* reg_value, BYTE cs);
/* Memory Configuration Register */
HYPERBUSI_STATUS setHYPERBUSI_MCR(DWORD base_addr, DWORD reg_value, BYTE cs);
#if (HYPERBUSI_IP_VER >= 230)
HYPERBUSI_STATUS setHYPERBUSI_MCR_MAXEN(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MCR_MAXLEN(DWORD base_addr, DWORD reg_value, BYTE cs);
#endif
HYPERBUSI_STATUS setHYPERBUSI_MCR_CRMO(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MCR_ACS(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MCR_CRT(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MCR_DEVTYPE(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MCR_WRAPSIZE(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MCR(DWORD base_addr, DWORD* reg_value, BYTE cs);
#if (HYPERBUSI_IP_VER >= 230)
HYPERBUSI_STATUS getHYPERBUSI_MCR_MAXEN(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MCR_MAXLEN(DWORD base_addr, DWORD* reg_value, BYTE cs);
#endif
HYPERBUSI_STATUS getHYPERBUSI_MCR_CRMO(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MCR_ACS(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MCR_CRT(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MCR_DEVTYPE(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MCR_WRAPSIZE(DWORD base_addr, DWORD* reg_value, BYTE cs);
/* Memory Timing Register */
HYPERBUSI_STATUS setHYPERBUSI_MTR(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSHI(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSHI(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSS(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSS(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSH(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSH(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS setHYPERBUSI_MTR_LTCY(DWORD base_addr, DWORD reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSHI(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSHI(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSS(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSS(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSH(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSH(DWORD base_addr, DWORD* reg_value, BYTE cs);
HYPERBUSI_STATUS getHYPERBUSI_MTR_LTCY(DWORD base_addr, DWORD* reg_value, BYTE cs);
/* General Purpose Output Register */
HYPERBUSI_STATUS setHYPERBUSI_GPOR(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS getHYPERBUSI_GPOR(DWORD base_addr, DWORD* reg_value);
/* Write Protection Register */
#ifdef HYPERBUSI_IP_WP_SUPPORT
HYPERBUSI_STATUS setHYPERBUSI_WPR(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS getHYPERBUSI_WPR(DWORD base_addr, DWORD* reg_value);
#endif
/* Test Register */
HYPERBUSI_STATUS setHYPERBUSI_TEST(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS getHYPERBUSI_TEST(DWORD base_addr, DWORD* reg_value);

/* Transaction Allocation Register */
#if (HYPERBUSI_IP_VER >= 240)
HYPERBUSI_STATUS setHYPERBUSI_TAR(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS setHYPERBUSI_TAR_RTA(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS setHYPERBUSI_TAR_WTA(DWORD base_addr, DWORD reg_value);
HYPERBUSI_STATUS getHYPERBUSI_TAR(DWORD base_addr, DWORD* reg_value);
HYPERBUSI_STATUS getHYPERBUSI_TAR_RTA(DWORD base_addr, DWORD* reg_value);
HYPERBUSI_STATUS getHYPERBUSI_TAR_WTA(DWORD base_addr, DWORD* reg_value);
#endif

//
void HYPERBUS_Test_Main(void);
//
#endif //__HYPERBUSI_H__
