/* lld.h - Source Code for Cypress's Low Level Driver */
/**************************************************************************
* Copyright (C)2011 Cypress LLC. All Rights Reserved. 
*
* This software is owned and published by: 
* Cypress LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Cypress").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software constitutes driver source code for use in programming Cypress's 
* Flash memory components. This software is licensed by Cypress to be adapted only 
* for use in systems utilizing Cypress's Flash memories. Cypress is not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein.  Cypress is providing this source code "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the source code herein.  
*
* CYPRESS MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE, 
* REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED 
* USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY, 
* FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  CYPRESS WILL 
* HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR 
* OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE, 
* INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL, 
* SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS, 
* EVEN IF CYPRESS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Copyright notice must be included with 
* this software, whether used in part or whole, at all times.  
*/

#ifndef __INC_H_lldh
#define __INC_H_lldh


#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

 
#define LLD_VERSION   "15.3.1"   //  Year.Quarter.Minor
 

/* LLD System Specific Typedefs */
typedef unsigned char  BYTE;         /* 8 bits wide */
typedef unsigned char  LLD_CHAR;      /* 8 bit wids */
typedef unsigned short LLD_UINT16;  /* 16 bits wide */
typedef unsigned long  LLD_UINT32;  /* 32 bits wide */
typedef unsigned long long  LLD_UINT64;  /* 64 bits wide */
typedef LLD_UINT32     ADDRESS;     /* Used for system level addressing */
typedef unsigned int   WORDCOUNT;   /* used for multi-byte operations */

/* boolean macros */
#ifndef TRUE  // LLD_KEEP
 #define TRUE  (1)
#endif      // LLD_KEEP
#ifndef FALSE // LLD_KEEP
 #define FALSE (0)
#endif      // LLD_KEEP

#define SA_OFFSET_MASK  0xFFFFF000   /* mask off the offset */

/* LLD Command Definition */
#define NOR_CFI_QUERY_CMD                ((0x98)*LLD_DEV_MULTIPLIER)
#define NOR_CHIP_ERASE_CMD               ((0x10)*LLD_DEV_MULTIPLIER)
#define NOR_ERASE_SETUP_CMD              ((0x80)*LLD_DEV_MULTIPLIER)
#define NOR_RESET_CMD                    ((0xF0)*LLD_DEV_MULTIPLIER)
#define NOR_SECSI_SECTOR_ENTRY_CMD       ((0x88)*LLD_DEV_MULTIPLIER)
#define NOR_SECTOR_ERASE_CMD             ((0x30)*LLD_DEV_MULTIPLIER)
#define NOR_WRITE_BUFFER_LOAD_CMD        ((0x25)*LLD_DEV_MULTIPLIER)
#define NOR_WRITE_BUFFER_PGM_CONFIRM_CMD ((0x29)*LLD_DEV_MULTIPLIER) 
#define NOR_SET_CONFIG_CMD           ((0xD0)*LLD_DEV_MULTIPLIER)
#define NOR_BIT_FIELD_CMD        ((0xBF)*LLD_DEV_MULTIPLIER)

#define NOR_ERASE_SUSPEND_CMD      ((0xB0)*LLD_DEV_MULTIPLIER)
#define NOR_ERASE_RESUME_CMD       ((0x30)*LLD_DEV_MULTIPLIER)
#define NOR_PROGRAM_SUSPEND_CMD      ((0x51)*LLD_DEV_MULTIPLIER)
#define NOR_PROGRAM_RESUME_CMD       ((0x50)*LLD_DEV_MULTIPLIER)
#define NOR_STATUS_REG_READ_CMD      ((0x70)*LLD_DEV_MULTIPLIER)
#define NOR_STATUS_REG_CLEAR_CMD     ((0x71)*LLD_DEV_MULTIPLIER)
#define NOR_BLANK_CHECK_CMD        ((0x33)*LLD_DEV_MULTIPLIER)

/* Command code definition */
#define NOR_AUTOSELECT_CMD               ((0x90)*LLD_DEV_MULTIPLIER)
#define NOR_PROGRAM_CMD                  ((0xA0)*LLD_DEV_MULTIPLIER)
#define NOR_SECSI_SECTOR_EXIT_SETUP_CMD  ((0x90)*LLD_DEV_MULTIPLIER)
#define NOR_SECSI_SECTOR_EXIT_CMD        ((0x00)*LLD_DEV_MULTIPLIER)
#define NOR_UNLOCK_BYPASS_ENTRY_CMD      ((0x20)*LLD_DEV_MULTIPLIER)
#define NOR_UNLOCK_BYPASS_PROGRAM_CMD    ((0xA0)*LLD_DEV_MULTIPLIER)
#define NOR_UNLOCK_BYPASS_RESET_CMD1     ((0x90)*LLD_DEV_MULTIPLIER)
#define NOR_UNLOCK_BYPASS_RESET_CMD2     ((0x00)*LLD_DEV_MULTIPLIER)
#define NOR_UNLOCK_DATA1                 ((0xAA)*LLD_DEV_MULTIPLIER)
#define NOR_UNLOCK_DATA2                 ((0x55)*LLD_DEV_MULTIPLIER)
#define NOR_SUSPEND_CMD                  ((0xB0)*LLD_DEV_MULTIPLIER)
#define NOR_RESUME_CMD                   ((0x30)*LLD_DEV_MULTIPLIER)
#define NOR_READ_CONFIG_CMD          ((0xC6)*LLD_DEV_MULTIPLIER)
#define NOR_WRITE_BUFFER_ABORT_RESET_CMD ((0xF0)*LLD_DEV_MULTIPLIER)

/* Sector protection command definition */
#define PPB_PROTECTED                           (0*LLD_DEV_MULTIPLIER)
#define PPB_UNPROTECTED                         (1*LLD_DEV_MULTIPLIER)

#define WSXXX_LOCK_REG_ENTRY      (0x40*LLD_DEV_MULTIPLIER)
#define WSXXX_LOCK_REG2_ENTRY     (0x41*LLD_DEV_MULTIPLIER)  // for GL-R
#define WSXXX_PSWD_PROT_CMD_ENTRY (0x60*LLD_DEV_MULTIPLIER)
#define WSXXX_PSWD_UNLOCK_1       (0x25*LLD_DEV_MULTIPLIER)
#define WSXXX_PSWD_UNLOCK_2       (0x03*LLD_DEV_MULTIPLIER)
#define WSXXX_PSWD_UNLOCK_3       (0x29*LLD_DEV_MULTIPLIER)
#define WSXXX_PPB_ENTRY           (0xC0*LLD_DEV_MULTIPLIER)
#define WSXXX_PPB_ERASE_CONFIRM   (0x30*LLD_DEV_MULTIPLIER)
#define WSXXX_PPB_LOCK_ENTRY      (0x50*LLD_DEV_MULTIPLIER)
#define WSXXX_DYB_ENTRY           (0xE0*LLD_DEV_MULTIPLIER)
#define WSXXX_DYB_CLEAR           (0x01*LLD_DEV_MULTIPLIER)

#define NOR_LOCK_REG_ENTRY        (0x40*LLD_DEV_MULTIPLIER)
#define NOR_SECTOR_LOCK_CMD     (0x60*LLD_DEV_MULTIPLIER)
#define NOR_LOAD_SECTOR_ADR     (0x61*LLD_DEV_MULTIPLIER)
#define NOR_SECTOR_UNLOCK_ADR6    (0x40*LLD_DEV_MULTIPLIER)
#define NOR_SECTOR_LOCK_ADR6    ((~NOR_SECTOR_UNLOCK_ADR6)*LLD_DEV_MULTIPLIER)



/* polling routine options */
typedef enum
{
LLD_P_POLL_NONE = 0,      /* pull program status */
LLD_P_POLL_PGM,           /* pull program status */
LLD_P_POLL_WRT_BUF_PGM,     /* Poll write buffer   */
LLD_P_POLL_SEC_ERS,         /* Poll sector erase   */
LLD_P_POLL_CHIP_ERS,      /* Poll chip erase     */
LLD_P_POLL_RESUME,
LLD_P_POLL_BLANK          /* Poll device sector blank check */
}POLLING_TYPE;

/* polling return status */
typedef enum {
 DEV_STATUS_UNKNOWN = 0,
 DEV_NOT_BUSY,
 DEV_BUSY,
 DEV_EXCEEDED_TIME_LIMITS,
 DEV_SUSPEND,
 DEV_WRITE_BUFFER_ABORT,
 DEV_STATUS_GET_PROBLEM,
 DEV_VERIFY_ERROR,
 DEV_BYTES_PER_OP_WRONG,
 DEV_ERASE_ERROR,       
 DEV_PROGRAM_ERROR,       
 DEV_SECTOR_LOCK,
 DEV_PROGRAM_SUSPEND,     /* Device is in program suspend mode */
 DEV_PROGRAM_SUSPEND_ERROR,   /* Device program suspend error */
 DEV_ERASE_SUSPEND,       /* Device is in erase suspend mode */
 DEV_ERASE_SUSPEND_ERROR,   /* Device erase suspend error */
 DEV_BUSY_IN_OTHER_BANK,     /* Busy operation in other bank */
 DEV_CONTINUITY_CHECK_PATTERN_ERROR, /*Continuity Check error, detected continuity pattern as unexpected */
 DEV_CONTINUITY_CHECK_NO_PATTERN_ERROR, /* Continuity Check error, no detected continuity pattern as unexpected */
 DEV_CONTINUITY_CHECK_PATTERN_DETECTED /* Continuity Check successfully and pattern detected */
} DEVSTATUS;

#include "lld_target_specific.h"

typedef enum 
{
  FLSTATE_NOT_BUSY = 0,
  FLSTATE_ERASE = 1,
  FLSTATE_WRITEBUFFER = 2
} FLSTATE;

#define FLRESUME 0
#define FLSUSPEND 1

#define DEV_RDY_MASK      (0x80*LLD_DEV_MULTIPLIER) /* Device Ready Bit */
#define DEV_ERASE_SUSP_MASK   (0x40*LLD_DEV_MULTIPLIER) /* Erase Suspend Bit */
#define DEV_ERASE_MASK      (0x20*LLD_DEV_MULTIPLIER) /* Erase Status Bit */
#define DEV_PROGRAM_MASK    (0x10*LLD_DEV_MULTIPLIER) /* Program Status Bit */
#define DEV_RFU_MASK      (0x08*LLD_DEV_MULTIPLIER) /* Reserved */
#define DEV_PROGRAM_SUSP_MASK (0x04*LLD_DEV_MULTIPLIER) /* Program Suspend Bit */
#define DEV_SEC_LOCK_MASK   (0x02*LLD_DEV_MULTIPLIER) /* Sector lock Bit */
#define DEV_BANK_MASK     (0x01*LLD_DEV_MULTIPLIER) /* Operation in current bank */



/*****************************************************
* Define Flash read/write macro to be used by LLD    *
*****************************************************/
#define FLASH_OFFSET(b,o)       (*(( (volatile FLASHDATA*)(b) ) + (o)))

  #ifdef TRACE
    #define FLASH_WR(b,o,d)         FlashWrite( b,o,d )
    #define FLASH_RD(b,o)           FlashRead(b,o)
  #else
    #ifdef EXTEND_ADDR
      #define FLASH_WR(b,o,d)     FlashWrite_Extend(b,o,d)
      #define FLASH_RD(b,o)       FlashRead_Extend(b,o) 
    #else
      #ifdef USER_SPECIFIC_CMD
        #define FLASH_WR(b,o,d) FlashWriteUserCmd((ADDRESS) ((volatile FLASHDATA *)(b) + (ADDRESS)o),d)
        #define FLASH_RD(b,o) FlashReadUserCmd((ADDRESS) ((volatile FLASHDATA *)(b) + (ADDRESS)o))
      #else
        #ifdef USER_SPECIFIC_CMD_2 // S26KSxxxS_S26KLxxxS LLD verification IO
          #define FLASH_WR(b,o,d)         ApiDrvLLDWriteWord(o,d )
          #define FLASH_RD(b,o)           ApiDrvLLDReadWord(o)
          #define FLASH_PAGE_RD(b,o,buf,cnt) ApiDrvReadArray((volatile FLASHDATA *)buf, (ADDRESS) ((volatile FLASHDATA *)(b) + (ADDRESS)o), cnt)
        #else
          #ifdef USER_SPECIFIC_CMD_3 //added NOR Page Read
            #define FLASH_WR(b,o,d) FlashWriteUserCmd((ADDRESS) ((volatile FLASHDATA *)(b) + (ADDRESS)o),d)
            #define FLASH_RD(b,o) FlashReadUserCmd((ADDRESS) ((volatile FLASHDATA *)(b) + (ADDRESS)o))
            #define FLASH_PAGE_RD(b,o,buf,cnt) FlashPageReadUserCmd((FLASHDATA *)(b), (ADDRESS)o, (FLASHDATA *)(buf),  (FLASHDATA) cnt)
          #else
            #define FLASH_WR(b,o,d) FLASH_OFFSET((b),(o)) = (d)
            #define FLASH_RD(b,o)   FLASH_OFFSET((b),(o))
          #endif //#ifdef USER_SPECIFIC_CMD_3
        #endif //#ifdef USER_SPECIFIC_CMD_2
      #endif //#ifdef USER_SPECIFIC_CMD
    #endif // #ifdef EXTEND_ADDR
  #endif // #ifdef TRACE


#ifdef  LLD_CONFIGURATION_X16_AS_X16           // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x00000001
#define LLD_DB_READ_MASK   0x0000FFFF
#define LLD_DEV_READ_MASK  0x0000FFFF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000002
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef LLD_UINT16 FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 1

#elif defined  LLD_CONFIGURATION_X32_AS_X32           
#define LLD_DEV_MULTIPLIER 0x00000001
#define LLD_DB_READ_MASK   0xFFFFFFFF 
#define LLD_DEV_READ_MASK  0xFFFFFFFF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000004
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef LLD_UINT32 FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 1

#elif defined LLD_CONFIGURATION_X8X16_AS_X8    // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x00000001
#define LLD_DB_READ_MASK   0x000000FF
#define LLD_DEV_READ_MASK  0x000000FF
#define LLD_UNLOCK_ADDR1   0x00000AAA
#define LLD_UNLOCK_ADDR2   0x00000555
#define LLD_BYTES_PER_OP   0x00000001
#define LLD_CFI_UNLOCK_ADDR1 0x000000AA
typedef BYTE FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 2

#elif defined LLD_CONFIGURATION_X8X16_AS_X16   // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x00000001
#define LLD_DB_READ_MASK   0x0000FFFF
#define LLD_DEV_READ_MASK  0x0000FFFF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000002
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef LLD_UINT16 FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 1

#elif defined LLD_CONFIGURATION_X16_AS_X32     // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x00010001
#define LLD_DB_READ_MASK   0xFFFFFFFF
#define LLD_DEV_READ_MASK  0x0000FFFF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000004
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef LLD_UINT32 FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 1

#elif defined LLD_CONFIGURATION_X8X16_AS_X32   // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x00010001
#define LLD_DB_READ_MASK   0xFFFFFFFF
#define LLD_DEV_READ_MASK  0x0000FFFF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000004
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef LLD_UINT32 FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 1

#elif defined LLD_CONFIGURATION_X8_AS_X8       // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x00000001
#define LLD_DB_READ_MASK   0x000000FF
#define LLD_DEV_READ_MASK  0x000000FF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000001
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef BYTE FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 1

#elif defined LLD_CONFIGURATION_X8_AS_X16     // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x00000101
#define LLD_DB_READ_MASK   0x0000FFFF
#define LLD_DEV_READ_MASK  0x0000FFFF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000002
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef LLD_UINT16 FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 2

#elif defined LLD_CONFIGURATION_X8_AS_X32     // LLD_KEEP
#define LLD_DEV_MULTIPLIER 0x01010101
#define LLD_DB_READ_MASK   0xFFFFFFFF
#define LLD_DEV_READ_MASK  0xFFFFFFFF
#define LLD_UNLOCK_ADDR1   0x00000555
#define LLD_UNLOCK_ADDR2   0x000002AA
#define LLD_BYTES_PER_OP   0x00000004
#define LLD_CFI_UNLOCK_ADDR1 0x00000055
typedef LLD_UINT32 FLASHDATA;
#define LLD_BUF_SIZE_MULTIPLIER 2
#endif     // LLD_KEEP              

/* public function prototypes */

/* Operation Functions */
extern FLASHDATA lld_ReadOp
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset          /* address offset from base address */
);

extern void lld_PageReadOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset,        /* address offset from base address */
FLASHDATA * read_buf,  /* read data */
FLASHDATA cnt        /* read count */
);

extern DEVSTATUS lld_WriteBufferProgramOp
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset,         /* address offset from base address */
WORDCOUNT word_cnt,       /* number of words to program */
FLASHDATA *data_buf       /* buffer containing data to program */
);


extern DEVSTATUS lld_ProgramOp
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset,         /* address offset from base address */
FLASHDATA write_data      /* variable containing data to program */
);


extern DEVSTATUS lld_SectorEraseOp
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset          /* address offset from base address */
);


extern DEVSTATUS lld_ChipEraseOp
(
FLASHDATA * base_addr     /* device base address is system */
);


extern void lld_GetVersion( LLD_CHAR versionStr[]);

extern void lld_InitCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_ResetCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_SectorEraseCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset          /* address offset from base address */
);

extern void lld_ChipEraseCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_ProgramCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset,         /* address offset from base address */
FLASHDATA *pgm_data_ptr     /* variable containing data to program */
);

extern void lld_SecSiSectorExitCmd
(
FLASHDATA * base_addr     /* device base address in system */
);
extern void lld_SecSiSectorEntryCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset          /* address offset from base address */
);

extern void lld_WriteToBufferCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset          /* address offset from base address */
);

extern void lld_ProgramBufferToFlashCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset          /* address offset from base address */
);
extern void lld_WriteBufferAbortResetCmd
(
FLASHDATA * base_addr     /* device base address in system */
);



FLASHDATA lld_ReadCfiWord
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset          /* address offset from base address */
);
extern void lld_CfiExitCmd
(
FLASHDATA * base_addr     /* device base address in system */
);
extern void lld_CfiEntryCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS     offset        /* address offset from base address */
);

void lld_StatusClear
(
FLASHDATA *  base_addr      /* device base address in system */
);

FLASHDATA lld_StatusGetReg
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
);


DEVSTATUS lld_memcpy
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset,         /* address offset from base address */
WORDCOUNT words_cnt,      /* number of words to program */
FLASHDATA *data_buf       /* buffer containing data to program */
);

extern void lld_StatusRegClearCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_StatusRegReadCmd
(
FLASHDATA * base_addr     /* device base address in system */
);



extern void lld_AutoselectEntryCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset          /* address offset from base address */
);

extern void lld_AutoselectExitCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_ProgramSuspendCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_EraseSuspendCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_EraseResumeCmd
(
FLASHDATA * base_addr     /* device base address in system */
);

extern void lld_ProgramResumeCmd
(
FLASHDATA * base_addr     /* device base address in system */
);
extern DEVSTATUS lld_EraseSuspendOp
(
FLASHDATA * base_addr     /* device base address is system */
);
extern DEVSTATUS lld_ProgramSuspendOp
(
FLASHDATA * base_addr     /* Device base address is system */
);

extern FLASHDATA lld_Poll
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset          /* address offset from base address */
);

extern unsigned int lld_GetDeviceId
(
FLASHDATA * base_addr     /* device base address is system */
);

extern void lld_BlankCheckCmd
(
FLASHDATA * base_addr,    /* device base address in system */
ADDRESS offset        /* address offset from base address */
);

extern DEVSTATUS lld_BlankCheckOp
(
FLASHDATA * base_addr,    /* device base address in system */
ADDRESS offset        /* address offset from base address */
);

  

/* WARNING - Make sure the macro DELAY_1us (lld.c)           */
/* is defined appropriately for your system !!                     */

extern void DelayMilliseconds
 (
 int milliseconds
 );

extern void DelayMicroseconds
 (
 int microseconds
 );

#ifdef TRACE
extern void FlashWrite(FLASHDATA * addr, ADDRESS offset, FLASHDATA data);
extern FLASHDATA FlashRead(FLASHDATA * addr, ADDRESS offset);
#endif

#ifdef EXTEND_ADDR
extern void FlashWrite_Extend(FLASHDATA *base_addr, ADDRESS offset, FLASHDATA data);
extern FLASHDATA FlashRead_Extend(FLASHDATA *base_addr, ADDRESS offset);
#endif

#ifdef USER_SPECIFIC_CMD
extern void FlashWriteUserCmd(ADDRESS address, FLASHDATA data);
extern FLASHDATA FlashReadUserCmd(ADDRESS address);
#endif

#ifdef USER_SPECIFIC_CMD_3 //for NOR Page Read
extern void FlashWriteUserCmd(ADDRESS address, FLASHDATA data);
extern FLASHDATA FlashReadUserCmd(ADDRESS address);
extern void FlashPageReadUserCmd(FLASHDATA * base_address, ADDRESS offset, FLASHDATA * buf, FLASHDATA cnt);
#endif

/**********************************************************
* Sector protection functions prototype.
**********************************************************/

extern void lld_LockRegEntryCmd     
( 
 FLASHDATA *   base_addr    /* device base address in system */ 
);
extern void lld_LockRegBitsProgramCmd 
( 
 FLASHDATA *   base_addr,   /* device base address in system */
 FLASHDATA value
);
extern FLASHDATA lld_LockRegBitsReadCmd 
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern void lld_LockRegExitCmd      
( 
 FLASHDATA *   base_addr    /* device base address in system */
);


extern void lld_PasswordProtectionEntryCmd
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern void lld_PasswordProtectionProgramCmd
( 
 FLASHDATA * base_addr,     /* device base address in system */
 ADDRESS offset,        /* address offset from base address */
 FLASHDATA pwd 
);
extern void lld_PasswordProtectionReadCmd
( 
 FLASHDATA *    base_addr,    /* device base address in system */
 FLASHDATA *pwd0,       /* Password 0 */
 FLASHDATA *pwd1,       /* Password 1 */
 FLASHDATA *pwd2,       /* Password 2 */
 FLASHDATA *pwd3        /* Password 3 */
);
extern void lld_PasswordProtectionUnlockCmd
( 
 FLASHDATA *  base_addr,    /* device base address in system */
 FLASHDATA pwd0,        /* Password 0 */
 FLASHDATA pwd1,        /* Password 1 */
 FLASHDATA pwd2,        /* Password 2 */
 FLASHDATA pwd3         /* Password 3 */
);
extern void lld_PasswordProtectionExitCmd
( 
 FLASHDATA *  base_addr     /* device base address in system */
);

extern void lld_PpbEntryCmd       
( 
 FLASHDATA *   base_addr   /* device base address in system */
);

extern void lld_PpbProgramCmd     
( 
 FLASHDATA *   base_addr,   /* device base address in system */
 ADDRESS   offset       /* address offset from base address */
);
extern void lld_PpbAllEraseCmd      
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern FLASHDATA lld_PpbStatusReadCmd 
( 
 FLASHDATA *   base_addr,   /* device base address in system */
 ADDRESS   offset       /* address offset from base address */
);
extern void lld_PpbExitCmd        
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern void lld_PpbLockBitEntryCmd    
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern void lld_PpbLockBitSetCmd    
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern FLASHDATA lld_PpbLockBitReadCmd  
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern void lld_PpbLockBitExitCmd   
(
 FLASHDATA *   base_addr    /* device base address in system */
);
extern void lld_DybEntryCmd       
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern void lld_DybSetCmd       
( 
 FLASHDATA *   base_addr,   /* device base address in system */
 ADDRESS   offset       /* address offset from base address */
);
extern void lld_DybClrCmd       
( 
 FLASHDATA *   base_addr,   /* device base address in system */
 ADDRESS   offset       /* address offset from base address */
);
extern FLASHDATA lld_DybReadCmd     
( FLASHDATA *   base_addr,    /* device base address in system */
 ADDRESS   offset       /* address offset from base address */
);
extern void lld_DybExitCmd        
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern FLASHDATA  lld_PpbLockBitReadOp  
( 
 FLASHDATA *  base_addr   /* device base address in system */
);
extern int lld_PpbAllEraseOp      
( 
 FLASHDATA *   base_addr    /* device base address in system */
);
extern FLASHDATA lld_PpbStatusReadOp  
( 
 FLASHDATA *  base_addr,    /* device base address in system */
 ADDRESS offset
);
extern int lld_PpbProgramOp       
( 
 FLASHDATA *  base_addr,    /* device base address in system */
 ADDRESS offset         /* address offset from base address */
);
extern int lld_PpbLockBitSetOp      
( 
 FLASHDATA *   base_addr    /* device base address in system */
);

extern FLASHDATA lld_LockRegBitsReadOp  
( 
 FLASHDATA *  base_addr   /* device base address in system */
);

extern int lld_LockRegBitsProgramOp   
( 
 FLASHDATA *  base_addr,    /* device base address in system */
 FLASHDATA value
);
































#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_H_lldh  */


