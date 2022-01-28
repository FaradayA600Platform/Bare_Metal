/* lld.c - Source Code for Cypress's Low Level Driver */
/**************************************************************************
* Copyright (C)2011 Cypress LLC. All Rights Reserved . 
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
//#include "lld_target_specific.h" /* For building project, it needs enabled */

#include "types.h" 
#include "S29GLxxxS.h"
#define LLD_BUFFER_SIZE (256*LLD_BUF_SIZE_MULTIPLIER)
#define LLD_SUSP_RESUME_DELAY  40


/* private functions */

/* Public Functions  */
/******************************************************************************
* 
* lld_GetVersion - Get LLD Version Number in string format.
*
*  PARAMETERS:  LLD_CHAR[] versionStr
*               Pointer to an empty char array. The size of the array has to be at 
*               least 12 in order to avoid buffer overflow.
*  RETURNS: version number in string returned in versionStr  
*
* ERRNO: 
*/
void lld_GetVersion( LLD_CHAR versionStr[])
{
  LLD_CHAR*  pVer = (LLD_CHAR*)LLD_VERSION;    
  
  if (versionStr)
  {
    while (*pVer) *versionStr++ = *pVer++;
    *versionStr = 0;   
  }
}

/******************************************************************************
* 
* lld_InitCmd - Initialize LLD
*
*
* RETURNS: void
*
* ERRNO: 
*/
void lld_InitCmd
(
FLASHDATA * base_addr     /* device base address in system */
)
{
  (void) base_addr;
}

/******************************************************************************
* 
* lld_ResetCmd - Writes a Software Reset command to the flash device
*
*
* RETURNS: void
*
* ERRNO: 
*/

void lld_ResetCmd
(
FLASHDATA * base_addr   /* device base address in system */
)
{       
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
  lld_InitCmd(base_addr);
}

/******************************************************************************
* 
* lld_SectorEraseCmd - Writes a Sector Erase Command to Flash Device
*
* This function only issues the Sector Erase Command sequence.
* Erase status polling is not implemented in this function.
*
* RETURNS: void
*
*/
 
void lld_SectorEraseCmd
(
FLASHDATA * base_addr,                   /* device base address in system */
ADDRESS offset                           /* address offset from base address */
)
{ 
 
  /* Issue unlock sequence command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_ERASE_SETUP_CMD);
  
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Sector Erase Command to Offset */
  FLASH_WR(base_addr, offset, NOR_SECTOR_ERASE_CMD);

}
/******************************************************************************
* 
* lld_ChipEraseCmd - Writes a Chip Erase Command to Flash Device
*
* This function only issues the Chip Erase Command sequence.
* Erase status polling is not implemented in this function.
*
* RETURNS: void
* 
*/
void lld_ChipEraseCmd
(
FLASHDATA * base_addr    /* device base address in system */
)
{       
  /* Issue inlock sequence command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_ERASE_SETUP_CMD);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Chip Erase Command to Base Address */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_CHIP_ERASE_CMD);
}
/******************************************************************************
* 
* lld_ProgramCmd - Writes a Program Command to Flash Device
*
* This function only issues the Program Command sequence.
* Program status polling is not implemented in this function.
*
* RETURNS: void
*
*/
void lld_ProgramCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset,                  /* address offset from base address */
FLASHDATA *pgm_data_ptr          /* variable containing data to program */
)
{       
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Program Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_PROGRAM_CMD);
  /* Write Data */
  FLASH_WR(base_addr, offset, *pgm_data_ptr);
}
/******************************************************************************
* 
* lld_WriteToBufferCmd - Writes "Write to Buffer Pgm" Command sequence to Flash
*
* RETURNS: void
*
*/
void lld_WriteToBufferCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset                       /* address offset from base address */
)
{  
  /* Issue unlock command sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Write Buffer Load Command */
  FLASH_WR(base_addr, offset, NOR_WRITE_BUFFER_LOAD_CMD);

}
/******************************************************************************
* 
* lld_ProgramBufferToFlashCmd - Writes "Pgm Buffer To Flash" Cmd sequence to Flash
*
* RETURNS: void
* 
*/ 
void lld_ProgramBufferToFlashCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset                       /* address offset from base address */
)
{       
  /* Transfer Buffer to Flash Command */
  FLASH_WR(base_addr, offset, NOR_WRITE_BUFFER_PGM_CONFIRM_CMD);
}
/******************************************************************************
* 
* lld_WriteBufferAbortResetCmd - Writes "Write To Buffer Abort" Reset to Flash
*
* RETURNS: void
*
*/
void lld_WriteBufferAbortResetCmd
(
FLASHDATA * base_addr        /* device base address in system */
)
{       
  
  /* Issue Write Buffer Abort Reset Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write to Buffer Abort Reset Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_WRITE_BUFFER_ABORT_RESET_CMD);   
}

/******************************************************************************
* 
* lld_CfiEntryCmd - Writes CFI Entry Command Sequence to Flash
*
* RETURNS: void
*
*/
void lld_CfiEntryCmd
(
FLASHDATA * base_addr,    /* device base address in system */
ADDRESS offset            /* sector address offset for ASO(Address Space Overlay) */
)
{        
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_CFI_UNLOCK_ADDR1, NOR_CFI_QUERY_CMD);
}
/******************************************************************************
* 
* lld_CfiExitCmd - Writes Cfi Exit Command Sequence to Flash
*
* This function resets the device out of CFI Query mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
*
* RETURNS: void
*
*/

void lld_CfiExitCmd
(
FLASHDATA * base_addr   /* device base address in system */
)
{       

  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD); 

}

/******************************************************************************
* 
* lld_ReadCfiWord - Read CFI word operation. 
*
* RETURNS: word read
*
*/
FLASHDATA lld_ReadCfiWord
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA data;

  lld_CfiEntryCmd(base_addr, offset);
  data  = FLASH_RD(base_addr, offset);
  lld_CfiExitCmd(base_addr);

  return(data);
}


/******************************************************************************
* 
* lld_StatusRegReadCmd - Status register read command
*
* This function sends the status register read command before actually reading it.
*
* RETURNS: void
*
*/

void lld_StatusRegReadCmd
(
FLASHDATA * base_addr    /* device base address in system */
)
{         
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_STATUS_REG_READ_CMD); 
}

/******************************************************************************
* 
* lld_StatusRegClearCmd - Status register clear command
*
* This function clear the status register. It will not clear the device operation 
* bits such as program suspend and erase suspend bits.
*
* RETURNS: void
*
*/

void lld_StatusRegClearCmd
(
FLASHDATA * base_addr   /* device base address in system */
)
{         
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_STATUS_REG_CLEAR_CMD); 
}



/******************************************************************************
* 
* lld_AutoselectEntryCmd - Writes Autoselect Command Sequence to Flash
*
* This function issues the Autoselect Command Sequence to device.
*
* RETURNS: void
* 
*/
void lld_AutoselectEntryCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset          /* address offset from base address */
)
{ 
  /* Issue Autoselect Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_AUTOSELECT_CMD); 
}

/******************************************************************************
* 
* lld_AutoselectExitCmd - Writes Autoselect Exit Command Sequence to Flash
*
* This function resets the device out of Autoselect mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
*
* RETURNS: void
* 
*/
void lld_AutoselectExitCmd
(
FLASHDATA * base_addr     /* device base address in system */
)
{       
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
}


/******************************************************************************
* 
* lld_ProgramSuspendCmd - Writes Suspend Command to Flash
*
* RETURNS: void
* 
*/
void lld_ProgramSuspendCmd
(
FLASHDATA * base_addr           /* device base address in system */
)
{         
  /* Write Suspend Command */
  FLASH_WR(base_addr, 0, NOR_PROGRAM_SUSPEND_CMD);
}

/******************************************************************************
* 
* lld_EraseSuspendCmd - Writes Suspend Command to Flash
*
* RETURNS: void
*
*/
void lld_EraseSuspendCmd
(
FLASHDATA * base_addr      /* device base address in system */
)
{     
  
  
  /* Write Suspend Command */
  FLASH_WR(base_addr, 0, NOR_ERASE_SUSPEND_CMD);
}

/******************************************************************************
* 
* lld_EraseResumeCmd - Writes Resume Command to Flash
*
* RETURNS: void
*
*/
void lld_EraseResumeCmd
(
FLASHDATA * base_addr       /* device base address in system */
)
{       

  /* Write Resume Command */
  FLASH_WR(base_addr, 0, NOR_ERASE_RESUME_CMD);

}

/******************************************************************************
* 
* lld_ProgramResumeCmd - Writes Resume Command to Flash
*
* RETURNS: void
*
*/
void lld_ProgramResumeCmd
(
FLASHDATA * base_addr       /* device base address in system */
)
{       
  /* Write Resume Command */
  FLASH_WR(base_addr, 0, NOR_PROGRAM_RESUME_CMD);
}



/******************************************************************************
* 
* lld_EraseSuspendOp - Performs Erase Suspend Operation
*
* Function pergorm erase suspend operation.
* Function issues erase suspend commands and will poll for completion.
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_EraseSuspendOp
(
FLASHDATA * base_addr    /* device base address is system */
)
{       
  FLASHDATA    status_reg;
 
  lld_EraseSuspendCmd( base_addr );   /* issue erase suspend command */  
  status_reg = lld_Poll(base_addr, 0 );    /* wait for device done */
  
  if( (status_reg & DEV_ERASE_SUSP_MASK) == DEV_ERASE_SUSP_MASK )
    return( DEV_ERASE_SUSPEND  );        /* Erase suspend  */
      
  return( DEV_ERASE_SUSPEND_ERROR );       /* Erase suspend error */

}

/******************************************************************************
* 
* lld_ProgramSuspendOp - Performs Program Suspend Operation
*
* Function pergorm program suspend operation.
* Function issues program suspend commands and will poll for completion.
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_ProgramSuspendOp
(
FLASHDATA * base_addr    /* Device base address is system */
)
{       
  FLASHDATA    status_reg;

  lld_ProgramSuspendCmd( base_addr );   /* Issue program suspend command */  
  status_reg = lld_Poll( base_addr, 0 );
  
  if( (status_reg & DEV_PROGRAM_SUSP_MASK) == DEV_PROGRAM_SUSP_MASK )
    return( DEV_PROGRAM_SUSPEND );      /* Program suspend */
      
  return( DEV_PROGRAM_SUSPEND_ERROR  );     /* Program suspend error */ 
}

/******************************************************************************
* 
* lld_GetDeviceId - Get device ID operation
*
* RETURNS: three byte ID in a single int
*
*/
unsigned int lld_GetDeviceId
(
FLASHDATA * base_addr   /* device base address in system */
)
{
  unsigned int id;


  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_AUTOSELECT_CMD);

#ifndef LLD_CONFIGURATION_X8X16_AS_X8
#ifndef LLD_CONFIGURATION_X8_AS_X8
  id  = (unsigned int)(FLASH_RD(base_addr, 0x0001) & 0x000000FF) << 16;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x000E) & 0x000000FF) <<  8;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x000F) & 0x000000FF)      ;
#else
  id  = (unsigned int)(FLASH_RD(base_addr, 0x0002) & 0x000000FF) << 16;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x001C) & 0x000000FF) <<  8;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x001E) & 0x000000FF)      ;
#endif
#else
  id  = (unsigned int)(FLASH_RD(base_addr, 0x0002) & 0x000000FF) << 16;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x001C) & 0x000000FF) <<  8;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x001E) & 0x000000FF)      ;
#endif
  fLib_printf("HyperFlash ID: %x\n", id);
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
  
  return(id);
}








/******************************************************************************
* 
* lld_Poll - Polls flash device for embedded operation completion
*
*  Function polls the Flash device to determine when an embedded
*  operation has finished - bit 7 is 1.  
*
* RETURNS: value of status register
*
*/
FLASHDATA lld_Poll
(
FLASHDATA * base_addr,          /* device base address in system */
ADDRESS offset                 /* address offset from base address */
)
{       
  unsigned long polling_counter = 0xFFFFFFFF;
  volatile FLASHDATA status_reg;

  do
  {
    polling_counter--;
    lld_StatusRegReadCmd( base_addr );    /* Issue status register read command */
    status_reg = FLASH_RD(base_addr, offset);       /* read the status register */
    if( (status_reg & DEV_RDY_MASK) == DEV_RDY_MASK  )  /* Are all devices done bit 7 is 1 */
      break;

  }while(polling_counter);
  
  return( status_reg );          /* retrun the status reg. */
}


/******************************************************************************
*    
* lld_StatusClear - Clears the flash status
*
*
* RETURNS: void
*
*/
void lld_StatusClear
(
FLASHDATA *  base_addr      /* device base address in system */
)
{
  lld_StatusRegClearCmd(base_addr );
}

/******************************************************************************
*    
* lld_StatusGetReg - Gets the flash status register bits 
*
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA lld_StatusGetReg
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
)
{
  FLASHDATA status_reg = 0xFFFF;
  lld_StatusRegReadCmd( base_addr );    /* Issue status register read command */
  status_reg = FLASH_RD( base_addr, offset );     /* read the status register */
  return status_reg;
}





/******************************************************************************
* 
* lld_SecSiSectorExitCmd - Writes SecSi Sector Exit Command Sequence to Flash
*
* This function issues the Secsi Sector Exit Command Sequence to device.
* Use this function to Exit the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO: 
*/

void lld_SecSiSectorExitCmd
(
FLASHDATA * base_addr               /* device base address in system */
)
{       
  /* Issue SecSi Sector Exit Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* First Secsi Sector Reset Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  /* Second Secsi Sector Reset Command */
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}
/******************************************************************************
* 
* lld_SecSiSectorEntryCmd - Writes SecSi Sector Entry Command Sequence to Flash
*
* This function issues the Secsi Sector Entry Command Sequence to device.
* Use this function to Enable the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO: 
*/
void lld_SecSiSectorEntryCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset        /* sector offset for ASO(Address Space Overlay) */
)
{       
  
  /* Issue SecSi Sector Entry Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_SECSI_SECTOR_ENTRY_CMD); 
}


/******************************************************************************
* 
* lld_BlankCheckCmd - Blank Check command
*
* This function checks the sector is blank.
*
* RETURNS: void
*
* ERRNO: 
*/
void lld_BlankCheckCmd
(
FLASHDATA * base_addr,   /* device base address in system */
ADDRESS offset           /* sector address offset from base address */
)
{         
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_BLANK_CHECK_CMD);  
}

/******************************************************************************
* 
* lld_BlankCheckOp - Performs a Blank Check Operation
*
* Function check blank at <base_addr> + <offset>.
* Function issues all required commands and will pool for completion
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_BlankCheckOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{       
    FLASHDATA    status_reg;

  lld_BlankCheckCmd( base_addr, offset );  

  status_reg = lld_Poll(base_addr, offset );
  
  if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
    return( DEV_ERASE_ERROR );    /* sector not blank */
  else
     return( DEV_NOT_BUSY );      /* sector are blank */  

}


/******************************************************************************
* 
* lld_ReadOp - Read memory array operation
*
* RETURNS: data read
*
*/
FLASHDATA lld_ReadOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA data;
  
  data = FLASH_RD(base_addr, offset);

  return(data);
}
#ifdef USER_SPECIFIC_CMD_3 //added NOR Page Read
/******************************************************************************
* 
* lld_PageReadOp - Read memory array operation
*
* RETURNS: NA
*
*/
void lld_PageReadOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset,        /* address offset from base address */
FLASHDATA * read_buf,  /* read data */
FLASHDATA cnt        /* read count */
)
{
  FLASH_PAGE_RD(base_addr, offset, read_buf, cnt);
}
#endif

/******************************************************************************
* 
* lld_WriteBufferProgramOp - Performs a Write Buffer Programming Operation.
*
* Function programs a write-buffer overlay of addresses to data 
* passed via <data_buf>.
* Function issues all required commands and polls for completion.
*
* There are 4 main sections to the function:
*  Set-up and write command sequence
*  Determine number of locations to program and load buffer
*  Start operation with "Program Buffer to Flash" command
*  Poll for completion
*
* REQUIREMENTS:
*  Data in the buffer MUST be properly aligned with the Flash bus width.
*  No parameter checking is performed. 
*  The <word_count> variable must be properly initialized.  
*  Valid <byte_cnt> values: 
*   min = 1 byte (only valid when used with a single x8 Flash)
*   max = write buffer size in bytes * number of devices in parallel
      (e.g. 32-byte buffer per device, 2 x16 devices = 64 bytes)
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_WriteBufferProgramOp
(
FLASHDATA *   base_addr,  /* device base address in system     */
ADDRESS   offset,     /* address offset from base address  */
WORDCOUNT word_count, /* number of words to program        */
FLASHDATA *data_buf   /* buffer containing data to program */
)
{
  ADDRESS   last_loaded_addr;
  ADDRESS   current_offset;
  ADDRESS   end_offset;
  FLASHDATA wcount;
  FLASHDATA status_reg;

  /* Initialize variables */
  current_offset   = offset;
  end_offset       = offset + word_count - 1;
  last_loaded_addr = offset;

  /* don't try with a count of zero */
  if (!word_count) 
  {
    return(DEV_NOT_BUSY);
  }

  /* Issue Load Write Buffer Command Sequence */
  lld_WriteToBufferCmd(base_addr, offset);

  /* Write # of locations to program */
  wcount = (FLASHDATA)word_count - 1;
  wcount *= LLD_DEV_MULTIPLIER;     /* For interleaving devices */

  /* In the datasheets of some latest Cypress devices, such as GLP, GLS, etc, the 
  command sequence of "write to buffer" command states the address of word count is 
  "Sector Address". Notice that to make LLD backward compatibility, the actual word 
  count address implemented is "Sector Address + LLD_UNLOCK_ADDR2", since the lower 
  address bits (a0-a15) are "don't care" bits and will be ignored anyway.
  */
    FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR2 , wcount);
  
  /* Load Data into Buffer */
  while(current_offset <= end_offset)
  {
    /* Store last loaded address & data value (for polling) */
    last_loaded_addr = current_offset;

    /* Write Data */
    FLASH_WR(base_addr, current_offset, *data_buf++);
    current_offset++;
  }

  /* Issue Program Buffer to Flash command */
  lld_ProgramBufferToFlashCmd(base_addr, offset);

  status_reg = lld_Poll(base_addr, last_loaded_addr);

  if( status_reg & DEV_SEC_LOCK_MASK )
    return( DEV_SECTOR_LOCK );    /* sector locked */

  if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
    return( DEV_PROGRAM_ERROR );    /* program error */

  return( DEV_NOT_BUSY );         /* program complete */
}


/******************************************************************************
* 
* lld_ProgramOp - Performs a standard Programming Operation.
*
* Function programs a single location to the specified data.
* Function issues all required commands and polls for completion.
*
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_ProgramOp
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset,         /* address offset from base address */
FLASHDATA write_data    /* variable containing data to program */
)
{   
 DEVSTATUS status;
 status = lld_WriteBufferProgramOp(base_addr, offset, 1, &write_data );
 return(status);
}


/******************************************************************************
* 
* lld_SectorEraseOp - Performs a Sector Erase Operation
*
* Function erases the sector containing <base_addr> + <offset>.
* Function issues all required commands and polls for completion.
*
*
* RETURNS: DEVSTATUS
*
* ERRNO: 
*/

DEVSTATUS lld_SectorEraseOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA         status_reg;

  lld_SectorEraseCmd(base_addr, offset);
  status_reg = lld_Poll(base_addr, offset );

  if( status_reg & DEV_SEC_LOCK_MASK )
    return( DEV_SECTOR_LOCK );    /* sector locked */

  if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
    return( DEV_ERASE_ERROR );    /* erase error */
      
  return( DEV_NOT_BUSY );         /* erease complete */
}



/******************************************************************************
* 
* lld_ChipEraseOp - Performs a Chip Erase Operation
*
* Function erases entire device located at <base_addr>.
* Function issues all required commands and polls for completion.
*
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_ChipEraseOp
(
FLASHDATA * base_addr   /* device base address in system */
)
{    
  FLASHDATA status_reg;

  lld_ChipEraseCmd(base_addr);
  status_reg = lld_Poll(base_addr, 0 );

  if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
    return( DEV_ERASE_ERROR );    /* erase error */
      
  return( DEV_NOT_BUSY );         /* erease complete */ 
}


/******************************************************************************
* 
* lld_memcpy   This function attempts to mimic the standard memcpy
*              function for flash.  It segments the source data
*              into page size chunks for use by Write Buffer Programming.
*
* RETURNS: DEVSTATUS
*
*/
DEVSTATUS lld_memcpy
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset,           /* address offset from base address */
WORDCOUNT word_cnt,       /* number of words to program */
FLASHDATA *data_buf       /* buffer containing data to program */
)
{
  ADDRESS mask = LLD_BUFFER_SIZE - 1;
  WORDCOUNT intwc = word_cnt;
  DEVSTATUS status = DEV_NOT_BUSY;

  if (offset & mask)
  {
    /* program only as much as necessary, so pick the lower of the two numbers */
    if (word_cnt < (LLD_BUFFER_SIZE - (offset & mask)) ) 
      intwc = word_cnt; 
    else
      intwc = LLD_BUFFER_SIZE - (offset & mask);

    /* program the first few to get write buffer aligned */
    status = lld_WriteBufferProgramOp(base_addr, offset, intwc, data_buf);
    if (status != DEV_NOT_BUSY) 
    {
     return(status);
    }

    offset   += intwc; /* adjust pointers and counter */
    word_cnt -= intwc;
    data_buf += intwc;
    if (word_cnt == 0)
    {
     return(status);
    }
  }

  while(word_cnt >= LLD_BUFFER_SIZE) /* while big chunks to do */
  {
    status = lld_WriteBufferProgramOp(base_addr, offset, LLD_BUFFER_SIZE, data_buf);
    if (status != DEV_NOT_BUSY)
    {
      return(status);
    }

    offset   += LLD_BUFFER_SIZE; /* adjust pointers and counter */
    word_cnt -= LLD_BUFFER_SIZE;
    data_buf += LLD_BUFFER_SIZE;
  }
  if (word_cnt == 0)
  {
    return(status);
  }

  status = lld_WriteBufferProgramOp(base_addr, offset, word_cnt, data_buf);
  return(status);
}

/******************************************************************************
* 
* DelayMilliseconds - Performs a delay.  If you have a better way,
*                     edit the macro DELAY_MS in lld_target_specific.h
*
* RETURNS: void
*
*/
void DelayMilliseconds(int milliseconds)
{
  int i;

  for (i = 0; i < milliseconds; i++)
    DelayMicroseconds(1000);
 
}

/******************************************************************************
* 
* DelayMicroseconds - Performs a delay.  If you have a better way,
*                     
* RETURNS: void
*
*/
#define DELAY_1us 150

void DelayMicroseconds(int microseconds)
{
  int volatile i, j;

  for (j = 0; j < microseconds; j++)
    for(i = 0; i < DELAY_1us; i++) i = i;
    
}

/************************************************************
*************************************************************
* Following are sector protection driver routines     *
*************************************************************
*************************************************************/

/******************************************************************************
* 
* lld_LockRegEntryCmd - Lock register entry command.
*
* RETURNS: n/a
*
*/
void lld_LockRegEntryCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_LOCK_REG_ENTRY);
}

/******************************************************************************
* 
* lld_LockRegBitsProgramCmd - Lock register program command.
*
* RETURNS: n/a
*
*/
void lld_LockRegBitsProgramCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
FLASHDATA value       /* value to program to lock reg. */
)
{

  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, 0x0, value);

}

/******************************************************************************
* 
* lld_LockRegBitsReadCmd - Lock register read command.
* Note: Need to issue lld_LockRegEntryCmd() before use this routine.
*
* RETURNS:  
* DQ0 Customer SecSi Sector Protection Bit  0 = set
* DQ1 Persistent Protection Mode Lock Bit   0 = set
* DQ2 Password Protection Mode Lock Bit     0 = set
*
*/
FLASHDATA lld_LockRegBitsReadCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{

  return(FLASH_RD(base_addr, 0x0));

}

/******************************************************************************
* 
* lld_LockRegExitCmd - Exit lock register read/write mode command.
*
* RETURNS: n/a
*
*/
void lld_LockRegExitCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{

  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);

}



/******************************************************************************
* 
* lld_PasswordProtectionEntryCmd - Write Password Protection Entry command sequence
*
* RETURNS: n/a
*
*/
void lld_PasswordProtectionEntryCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_PSWD_PROT_CMD_ENTRY);
}

/******************************************************************************
* 
* lld_PasswordProtectionProgramCmd - Write Password Protection Program command.
* Note: Need to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PasswordProtectionProgramCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
ADDRESS   offset,
FLASHDATA pwd
)
{
  FLASH_WR(base_addr, offset, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, pwd);
}

/******************************************************************************
* 
* lld_PasswordProtectionReadCmd - Issue read password command
* Note: Need to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PasswordProtectionReadCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
FLASHDATA *pwd0,      /* Password 0 */
FLASHDATA *pwd1,      /* Password 1 */
FLASHDATA *pwd2,      /* Password 2 */
FLASHDATA *pwd3       /* Password 3 */
)
{
  *pwd0 = FLASH_RD(base_addr, 0);
  *pwd1 = FLASH_RD(base_addr, 1);
  *pwd2 = FLASH_RD(base_addr, 2);
  *pwd3 = FLASH_RD(base_addr, 3);
}

/******************************************************************************
* 
* lld_PasswordProtectionUnlockCmd - Issue unlock password command.
* Note: Need to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PasswordProtectionUnlockCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
FLASHDATA pwd0,       /* Password 0 */
FLASHDATA pwd1,       /* Password 1 */
FLASHDATA pwd2,       /* Password 2 */
FLASHDATA pwd3        /* Password 3 */
)
{
  FLASH_WR(base_addr, 0, WSXXX_PSWD_UNLOCK_1);
  FLASH_WR(base_addr, 0, WSXXX_PSWD_UNLOCK_2);
  FLASH_WR(base_addr, 0, pwd0);
  FLASH_WR(base_addr, 1, pwd1);
  FLASH_WR(base_addr, 2, pwd2);
  FLASH_WR(base_addr, 3, pwd3);
  FLASH_WR(base_addr, 0, WSXXX_PSWD_UNLOCK_3);

}

/******************************************************************************
* 
* lld_PasswordProtectionExitCmd - Issue exit password protection mode command.
*
* RETURNS: n/a
*
*/
void lld_PasswordProtectionExitCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}

/******************************************************************************
* 
* lld_PpbEntryCmd - Non-Volatile Sector Protection Entry Command.
* Ppb entry command will disable the reads and writes for the bank selectd.
* RETURNS: n/a
*
*/
void lld_PpbEntryCmd
(
FLASHDATA *   base_addr  /* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_PPB_ENTRY);
}

/******************************************************************************
* 
* lld_PpbProgramCmd - Program Non-Volatile Sector Protection Command.
* Note: Need to issue lld_PpbEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PpbProgramCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
ADDRESS   offset      /* Sector offset address */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, 0);
}

/******************************************************************************
* 
* lld_PpbAllEraseCmd - Erase Non-Volatile Protection for All  Sectors Command.
* Note: Need to issue lld_PpbEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PpbAllEraseCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_ERASE_SETUP_CMD);
  FLASH_WR(base_addr, 0, WSXXX_PPB_ERASE_CONFIRM);
}

/******************************************************************************
* 
* lld_PpbStatusReadCmd - Read Non-Volatile Sector Status Command.
* Note: Need to issue lld_PpbEntryCmd() before issue this routine.
* Sector status 0 is locked and 1 is unlocked.
* RETURNS: 
*
*/
FLASHDATA lld_PpbStatusReadCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
ADDRESS   offset      /* sector offset address */
)
{
  return(FLASH_RD(base_addr, offset));
}

/******************************************************************************
* 
* lld_PpbExitCmd - Exit the Non-Volatile Sector Status mode.
* After the exit command the device goes back to memory array mode.
* RETURNS: n/a
*
*/
void lld_PpbExitCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}

/******************************************************************************
* 
* lld_PpbLockBitEntryCmd - Issue Persistent Protection Bit Lock Bit Entry Command.
* The Ppb Lock Bit is a global bit for all sectors. 
* RETURNS: n/a
*
*/
void lld_PpbLockBitEntryCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_PPB_LOCK_ENTRY);
}

/******************************************************************************
* 
* lld_PpbLockBitSetCmd - Issue set Persistent Protection Bit Lock Bit command.
* Once bit is set there is no command to unset it only hardware reset and power up 
* will clear the bit.
* RETURNS: n/a
*
*/
void lld_PpbLockBitSetCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, 0, 0);
}

/******************************************************************************
* 
* lld_PpbLockBitReadCmd - Read the Ppb Lock Bit value.
* Note: Need to issue lld_PpbLockBitEntryCmd() before read.
* RETURNS: 
*
*/
FLASHDATA lld_PpbLockBitReadCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  return(FLASH_RD(base_addr, 0));
}

/******************************************************************************
* 
* lld_PpbLockBitExitCmd - Exit Ppb Lock Bit mode command.
*
* RETURNS: n/a
*
*/
void lld_PpbLockBitExitCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}


/******************************************************************************
* 
* lld_DybEntryCmd - Dynamic (Volatile) Sector Protection Entry Command.
*
* RETURNS: n/a
*
*/
void lld_DybEntryCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_DYB_ENTRY);
}

/******************************************************************************
* 
* lld_DybSetCmd - Dynamic (Volatile) Sector Protection Set Command.
* Note: Need to issue lld_DybEntryCmd() before issue this command.
* RETURNS: n/a
*
*/
void lld_DybSetCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
ADDRESS   offset      /* sector offset address */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, 0x00000000);
}

/******************************************************************************
* 
* lld_DybClrCmd - Dynamic (Volatile) Sector Protection Clear Command.
* Note: Need to issue lld_DybEntryCmd() before issue this command.
* RETURNS: n/a
*
*/
void lld_DybClrCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
ADDRESS   offset      /* sector offset address */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, WSXXX_DYB_CLEAR);
}

/******************************************************************************
* 
* lld_DybReadCmd - Dynamic (Volatile) Sector Protection Read Command.
* Note: Need to issue lld_DybEntryCmd() before issue this command.
* RETURNS: 
*
*/
FLASHDATA lld_DybReadCmd
(
FLASHDATA *   base_addr,  /* device base address in system */
ADDRESS   offset      /* sector offset address */
)
{
  return(FLASH_RD(base_addr, offset));
}

/******************************************************************************
* 
* lld_DybExitCmd - Exit Dynamic (Volatile) Sector Protection Mode Command.
*
* RETURNS: n/a
*
*/
void lld_DybExitCmd
(
FLASHDATA *   base_addr   /* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}

/******************************************************************************
* 
* lld_PpbLockBitReadOp - Operation to read global lock bit.
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA  lld_PpbLockBitReadOp ( FLASHDATA *base_addr)
{
  FLASHDATA retval;

  (lld_PpbLockBitEntryCmd)(base_addr);
  retval = (lld_PpbLockBitReadCmd)(base_addr);
  lld_PpbLockBitExitCmd(base_addr);

  return retval;
}

/******************************************************************************
* 
* lld_PpbLockBitSetOp - Operation to set the global lock bit.
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_PpbLockBitSetOp ( FLASHDATA *   base_addr)
{ 
  DEVSTATUS dev_status = DEV_STATUS_UNKNOWN;
  FLASHDATA    status_reg;


  lld_PpbLockBitEntryCmd(base_addr);
  lld_PpbLockBitSetCmd(base_addr);
  /* poll for completion */
  status_reg = lld_Poll(base_addr, 0);
  if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
    dev_status =  DEV_PROGRAM_ERROR;    /* program error */
  else
    dev_status = DEV_NOT_BUSY;

  /* if not done, then we have an error */
  if (dev_status != DEV_NOT_BUSY)
  {
    /* Write Software RESET command */
    FLASH_WR(base_addr, 0, NOR_RESET_CMD);
    lld_PpbLockBitExitCmd(base_addr);
    return (-1);  /* return error */ 
  }

  lld_PpbLockBitExitCmd(base_addr);
  return 0; /* successfull */
}

/******************************************************************************
* 
* lld_PpbAllEraseOp - Operation to clear protection for all sections.
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_PpbAllEraseOp ( FLASHDATA *   base_addr)
{
  DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
  FLASHDATA    status_reg;

  if (lld_PpbLockBitReadOp(base_addr) == PPB_PROTECTED)    /* if it is already locked */
  {
    return(-1);                            /* return an error cuz Lock Bit is locked */
  }

  lld_PpbEntryCmd(base_addr);
  lld_PpbAllEraseCmd(base_addr);
  /* poll for completion */
  status_reg = lld_Poll(base_addr, 0);
  if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
    dev_status =  DEV_ERASE_ERROR;    /* program error */
  else
    dev_status = DEV_NOT_BUSY;

  /* if not done, then we have an error */
  if (dev_status != DEV_NOT_BUSY)
  {
    /* Write Software RESET command */
    FLASH_WR(base_addr, 0, NOR_RESET_CMD);
    lld_PpbExitCmd(base_addr);
    return (-1); /* return error */
  }

  lld_PpbExitCmd(base_addr);    /* exit Ppb protection mode command */
  return 0; /* successful */
}

/******************************************************************************
* 
* lld_PpbProgramOp - Operation set the Persistent Protection for a sector. 
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_PpbProgramOp ( FLASHDATA *base_addr, ADDRESS offset)
{
  DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
  FLASHDATA    status_reg;
  
  if (lld_PpbLockBitReadOp(base_addr) == PPB_PROTECTED)      /* if it is already locked */
  {
    return(-1);                              /* return an error cuz Lock Bit is locked */
  }


  lld_PpbEntryCmd(base_addr);
  lld_PpbProgramCmd(base_addr, offset);

  /* poll for completion */
  status_reg = lld_Poll(base_addr, 0);
  if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
    dev_status =  DEV_PROGRAM_ERROR;    /* program error */
  else
    dev_status = DEV_NOT_BUSY;

  /* if not done, then we have an error */
  if (dev_status != DEV_NOT_BUSY)
  {
    /* Write Software RESET command */
    FLASH_WR(base_addr, 0, NOR_RESET_CMD);
    lld_PpbExitCmd(base_addr);
    return (-1); /* return error */
  }

  lld_PpbExitCmd(base_addr);
  return 0; /* successful */
}

/******************************************************************************
* 
* lld_PpbStatusReadOp - Operation to read the Persistent Protection status register.
*
* RETURNS: FLASHDATA
*lld_read
*/

FLASHDATA lld_PpbStatusReadOp ( FLASHDATA *base_addr, ADDRESS offset)
{
  FLASHDATA  status;
  
  lld_PpbEntryCmd(base_addr);
  status = (FLASH_RD(base_addr, offset));
  lld_PpbExitCmd(base_addr);
  
  return status;

}

/******************************************************************************
* 
* lld_LockRegBitsReadOp - Operation to read the lock status register.
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA lld_LockRegBitsReadOp ( FLASHDATA *base_addr)
{ 
  FLASHDATA value;

  lld_LockRegEntryCmd(base_addr);
  value = lld_LockRegBitsReadCmd(base_addr);
  lld_LockRegExitCmd(base_addr);

  return(value);
}


/******************************************************************************
* 
* lld_LockRegBitsProgramOp - Operation to program the lock register.
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_LockRegBitsProgramOp ( FLASHDATA *base_addr, FLASHDATA value)
{
  DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
  FLASHDATA    status_reg;

  lld_LockRegEntryCmd(base_addr);
  lld_LockRegBitsProgramCmd(base_addr,value);

  /* poll for completion */
    status_reg = lld_Poll(base_addr, 0);
    if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
      dev_status =  DEV_PROGRAM_ERROR;    /* program error */
  else
    dev_status = DEV_NOT_BUSY;

  /* if not done, then we have an error */
  if (dev_status != DEV_NOT_BUSY)
  {
     /* Write Software RESET command */
     FLASH_WR(base_addr, 0, NOR_RESET_CMD);
     lld_LockRegExitCmd(base_addr);
     return (-1); /* return error */
  }

  lld_LockRegExitCmd(base_addr);
  return 0; /* successful */
}







