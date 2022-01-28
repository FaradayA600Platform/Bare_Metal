/* hyperbusi.c - Source Code for HyperBus CTRL IP Driver */
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

#include "hyperbusi.h"

//=============================================================================
//  REGISTER FUNCTIONS
//=============================================================================
#if (HYPERBUSI_IP_VER >= 220)
//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_CSR(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the Controller Status Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_CSR(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    
    *reg_value = HYPERBUSI_REG_READ(base_addr, REG_CSR_ADDR);
    MSG("CSR: 0x%08X\n",*reg_value);
    return ret;
}
#endif

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_IEN(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the Interrupt Enable Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_IEN(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;

    HYPERBUSI_REG_WRITE(base_addr, REG_IEN_ADDR, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
    reg_value = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    MSG("IEN: 0x%08X\n",reg_value);
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_IEN_INTP(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the Interrupt Enable Register for Interrupt Polarity.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit0 is valid).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_IEN_INTP(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    reg &= CLR_INT_POLARITY;
    reg |= SET_INT_POLARITY(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, REG_IEN_ADDR, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    MSG("IEN polarity: %d\n",
        GET_INT_POLARITY(reg));
#endif

    return ret;
}


//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_IEN_RPCINTE(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the Interrupt Enable Register for Interrupt Polarity.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit0 is valid).
//  @attention Nothing
// @date Nov.11/2014
// @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_IEN_RPCINTE(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    reg &= CLR_INT_EN;
    reg |= SET_INT_EN(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, REG_IEN_ADDR, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    MSG("IEN rpcinte: %d\n",
         GET_INT_EN(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_IEN(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the Interrupt Enable Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
// @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_IEN(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;

    *reg_value = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    MSG("IEN: 0x%08X\n", *reg_value);
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_IEN_INTP(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the Interrupt Enable Register for HyperBus Memory Interrupt Enable.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit0 is valid).
//  @attention Nothing
//  @date Nov.11/2014
// @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_IEN_INTP(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    *reg_value = GET_INT_POLARITY(reg);
    MSG("IEN polarity: %d\n",
        GET_INT_POLARITY(reg));    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_IEN_RPCINTE(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the Interrupt Enable Register for HyperBus Memory Interrupt Enable.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit0 is valid).
//  @attention Nothing
//  @date Nov.11/2014
// @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_IEN_RPCINTE(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_IEN_ADDR);
    *reg_value = GET_INT_EN(reg);
    MSG("IEN rpcinte: %d\n",
         GET_INT_EN(reg));
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_ISR(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the Interrupt Status Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_ISR(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;

    *reg_value = HYPERBUSI_REG_READ(base_addr, REG_ISR_ADDR);
    MSG("ISR: 0x%08X\n", *reg_value);
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MBR (DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Base Address Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (32bit).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MBR(DWORD base_addr, DWORD reg_value, BYTE cs)
{
  HYPERBUSI_STATUS ret = NO_ERROR;
  DWORD mbrRegAddr = REG_MBR0_ADDR;

  if (cs==CS1_SEL) {
    mbrRegAddr = REG_MBR1_ADDR;
  }

  if (cs > CS1_SEL) {
    return ERROR;
  }

  HYPERBUSI_REG_WRITE(base_addr, mbrRegAddr, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
  reg_value = HYPERBUSI_REG_READ(base_addr, mbrRegAddr);
  MSG("MBR%1d: 0x%08X\n", cs, reg_value);
#endif

  return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MBR (DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Base Address Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MBR(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
  HYPERBUSI_STATUS ret = NO_ERROR;
  DWORD mbrRegAddr = REG_MBR0_ADDR;

  if (cs==CS1_SEL) {
    mbrRegAddr = REG_MBR1_ADDR;
  }

  if (cs > CS1_SEL) {
    return ERROR;
  }

  *reg_value = HYPERBUSI_REG_READ(base_addr, mbrRegAddr);
  MSG("MBR%1d: 0x%08X\n", cs, *reg_value);
  return ret;
}


//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (32bit).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    HYPERBUSI_REG_WRITE(base_addr, regAddr, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
    reg_value = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d: 0x%08X\n", cs, reg_value);
#endif

	//fLib_printf("\nMCR%1d: 0x%08X\n", cs, reg_value);
	
	//HYPERBUSI_REG_WRITE(base_addr, REG_MCR1_ADDR, reg_value);
	//reg_value = HYPERBUSI_REG_READ(base_addr, REG_MCR1_ADDR);
	//fLib_printf("\nMCR%1d: 0x%08X\n", cs, reg_value);
    
	return ret;
}

#if (HYPERBUSI_IP_VER >= 230)
//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR_MAXEN(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register for Maximum Length Enable.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR_MAXEN(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);		
    reg &= CLR_MAXEN;
    reg |= SET_MAXEN(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, regAddr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d MAXEN %d\n", cs, GET_MAXEN(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR_MAXLEN(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register for Maximum Length.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[8:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR_MAXLEN(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    reg &= CLR_MAXLEN(reg);
    reg |= SET_MAXLEN(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, regAddr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d MAXLEN %d\n", cs, GET_MAXLEN(reg));
#endif

    return ret;
}
#endif

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR_CRMO(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register for Continuous Read Merging Option.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR_CRMO(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }

    if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    reg &= CLR_TC_OPTION;
    reg |= SET_TC_OPTION(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, regAddr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d TC Option %d\n", cs, GET_TC_OPTION(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR_ACS(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register for Asymmetry Cache System Support.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR_ACS(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }

    if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    reg &= CLR_ACACHE;
    reg |= SET_ACACHE(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, regAddr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d Asymmetry Cache Support %d\n", cs, GET_ACACHE(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR_CRT(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register for Configuration Register Target.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR_CRT(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_CRT;
    reg |= SET_CRT(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

	fLib_printf("\n\n\r hss %d\n\r", ret);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MCR%1d CRT %d\n", cs, GET_CRT(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR_DEVTYPE(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register for Device Type.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR_DEVTYPE(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    reg &= CLR_DEVTYPE;
    reg |= SET_DEVTYPE(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, regAddr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d DEVTYPE %d\n", cs, GET_DEVTYPE(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MCR_WRAPSIZE(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Configuration Register for Wrapped Burst Size.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[1:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MCR_WRAPSIZE(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }

    if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    reg &= CLR_WRAP_SIZE;
    reg |= SET_WRAP_SIZE(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, regAddr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d Wrap Size %d\n", cs, GET_WRAP_SIZE(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    *reg_value = HYPERBUSI_REG_READ(base_addr, regAddr);
    MSG("MCR%1d: 0x%08X\n", cs, *reg_value);
    return ret;
}

#if (HYPERBUSI_IP_VER >= 230)
//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR_MAXEN(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register for Maximum Length Enable.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR_MAXEN(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    *reg_value = GET_MAXEN(reg);
    MSG("MCR%1d MAXEN %d\n", cs, *reg_value);
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR_MAXLEN(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register for Maximum Length.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[8:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR_MAXLEN(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    *reg_value = GET_MAXLEN(reg);
    MSG("MCR%1d MAXLEN %d\n", cs, *reg_value);
    return ret;
}
#endif

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR_CRMO(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register for Continuous Read Merging Option.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR_CRMO(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
 HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }

    if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    *reg_value = GET_TC_OPTION(reg);
    MSG("MCR%1d TC Option %d\n", cs, GET_TC_OPTION(reg));    
    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR_ACS(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register for Asymmetry Cache System Support.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR_ACS(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }

    if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    *reg_value = GET_ACACHE(reg);
    MSG("MCR%1d Asymmetry Cache Support %d\n", cs, GET_ACACHE(reg));    

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR_CRT(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register for Configuration Register Target..
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR_CRT(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_CRT(reg);
    MSG("MCR%1d CRT %d\n", cs, GET_CRT(reg));    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR_DEVTYPE(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register for Device Type.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit0 is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR_DEVTYPE(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    *reg_value = GET_DEVTYPE(reg);
    MSG("MCR%1d DEVTYPE %d\n", cs, *reg_value);
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MCR_WRAPSIZE(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Configuration Register for Wrapped Burst Size.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[1:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MCR_WRAPSIZE(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD regAddr = REG_MCR0_ADDR;

    if (cs == CS1_SEL) {
        regAddr = REG_MCR1_ADDR;
    }

    if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, regAddr);
    *reg_value = GET_WRAP_SIZE(reg);
    MSG("MCR%1d Wrap Size %d\n", cs, GET_WRAP_SIZE(reg));    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (32bit).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    HYPERBUSI_REG_WRITE(base_addr, addr, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
    reg_value = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d: 0x%08X\n", cs, reg_value);
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSHI(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register for Read Chip Select High.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSHI(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_RCSHI(reg);
    reg |= SET_RCSHI(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d RCSHI %d\n", cs, GET_RCSHI(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSHI(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register for Write Chip Select High.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSHI(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_WCSHI(reg);
    reg |= SET_WCSHI(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d WCSHI %d\n", cs, GET_WCSHI(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSS(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register for Read Chip Select Setup.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSS(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_RCSS(reg);
    reg |= SET_RCSS(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d RCSS %d\n", cs, GET_RCSS(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSS(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register for Write Chip Select Setup.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSS(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_WCSS(reg);
    reg |= SET_WCSS(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d WCSS %d\n", cs, GET_WCSS(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSH(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register for Read Chip Select Hold.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR_RCSH(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_RCSH(reg);
    reg |= SET_RCSH(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d RCSH %d\n", cs, GET_RCSH(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSH(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register for Write Chip Select Hold.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR_WCSH(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_WCSH(reg);
    reg |= SET_WCSH(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d WCSH %d\n", cs, GET_WCSH(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_MTR_LTCY(DWORD base_addr, DWORD reg_value, BYTE cs)
//! @brief This function writes the Memory Timing Register for Latency Cycle (HyperRAM only).
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select.
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_MTR_LTCY(DWORD base_addr, DWORD reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    reg &= CLR_LTCY(reg);
    reg |= SET_LTCY(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, addr, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d LTCY %d\n", cs, GET_LTCY(reg));
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSHI(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register for Read Chip Select High.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSHI(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_RCSHI(reg);
    MSG("MTR%1d RCSHI %d\n", cs, GET_RCSHI(reg));    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
        return ERROR;
    }

    *reg_value = HYPERBUSI_REG_READ(base_addr, addr);
    MSG("MTR%1d: 0x%08X\n", cs, *reg_value);
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSHI(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register for Write Chip Select High.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSHI(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_WCSHI(reg);
    MSG("MTR%1d WCSHI %d\n", cs, GET_WCSHI(reg));    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSS(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register for Read Chip Select Setup.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSS(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_RCSS(reg);
    MSG("MTR%1d RCSS %d\n", cs, GET_RCSS(reg));
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSS(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register for Write Chip Select Setup.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSS(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_WCSS(reg);
    MSG("MTR%1d WCSS %d\n", cs, GET_WCSS(reg));
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSH(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register for Read Chip Select Hold.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value.
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR_RCSH(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_RCSH(reg);
    MSG("MTR%1d RCSH %d\n", cs, GET_RCSH(reg));    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSH(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register for Write Chip Select Hold.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR_WCSH(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_WCSH(reg);
    MSG("MTR%1d WCSH %d\n", cs, GET_WCSH(reg));    
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_MTR_LTCY(DWORD base_addr, DWORD* reg_value, BYTE cs)
//! @brief This function reads the Memory Timing Register for Latency Cycle (HyperRAM only).
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[3:0] is valid).
//! @param  cs                  : Set the chip select. 
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_MTR_LTCY(DWORD base_addr, DWORD* reg_value, BYTE cs)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;
    DWORD addr = REG_MTR0_ADDR;

    if (cs == CS1_SEL) {
        addr = REG_MTR1_ADDR;
    }
    else if (cs > CS1_SEL) {
       return ERROR;
    }

    reg = HYPERBUSI_REG_READ(base_addr, addr);
    *reg_value = GET_LTCY(reg);
    MSG("MTR%1d LTCY %d\n", cs, GET_LTCY(reg));
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_GPOR(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the General Purpose Output Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_GPOR(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;

    HYPERBUSI_REG_WRITE(base_addr, REG_GPOR_ADDR, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
    reg_value = HYPERBUSI_REG_READ(base_addr, REG_GPOR_ADDR);
    MSG("GPOR: 0x%08X\n", reg_value);
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_GPOR(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the General Purpose Output Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_GPOR(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;

    *reg_value = HYPERBUSI_REG_READ(base_addr, REG_GPOR_ADDR);
    MSG("GPOR: 0x%08X\n", *reg_value);
    return ret;
}
#ifdef HYPERBUSI_IP_WP_SUPPORT
//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_WPR(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the Write Protection Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_WPR(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
//    DWORD reg = 0;

    HYPERBUSI_REG_WRITE(base_addr, REG_WPR_ADDR, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
    reg_value = HYPERBUSI_REG_READ(base_addr, REG_WPR_ADDR);
    MSG("WPR: 0x%08X\n", reg_value);
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_WPR(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the Write Protection Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_WPR(DWORD base_addr, DWORD* reg_value)
{
 HYPERBUSI_STATUS ret = NO_ERROR;
    
    *reg_value = HYPERBUSI_REG_READ(base_addr, REG_WPR_ADDR);
    MSG("WPR: 0x%08X\n", *reg_value);
    return ret;
}
#endif
//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_TEST(DWORD base_addr, DWORD reg_value)
//! @brief This function writes Test Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value            : Set the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_TEST(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;

    HYPERBUSI_REG_WRITE(base_addr, REG_LBR_ADDR, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
    reg_value = HYPERBUSI_REG_READ(base_addr, REG_LBR_ADDR);
    MSG("TEST: 0x%08X\n", reg_value);
#endif

    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_TEST(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads Test Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//  @attention Nothing
//  @date Nov.11/2014
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_TEST(DWORD base_addr, DWORD* reg_value)
{

    HYPERBUSI_STATUS ret = NO_ERROR;
//    DWORD lbrReg = 0;

    *reg_value = HYPERBUSI_REG_READ(base_addr, REG_LBR_ADDR);
    MSG("TEST: 0x%08X\n", *reg_value);
    return ret;
}

#if (HYPERBUSI_IP_VER >= 240)

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_TAR(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the Transaction Allocation Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value           : Set the register value (32bit).
//  @attention Nothing
//  @date Jan.06/2016
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_TAR(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;

    HYPERBUSI_REG_WRITE(base_addr, REG_TAR_ADDR, reg_value);

#ifdef HYPERBUSI_READ_VERIFY
    reg_value = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    MSG("TAR: 0x%08X\n", reg_value);
#endif
    return ret;
}


//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_TAR_RTA(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the Transaction Allocation Register for Read Transaction Allocation.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value           : Set the register value (bit[0:1] is valid).
//  @attention Nothing
//  @date Jan.06/2016
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_TAR_RTA(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    reg &= CLR_RTA(reg);
    reg |= SET_RTA(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, REG_TAR_ADDR, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    MSG("TAR: RTA %d\n", GET_RTA(reg));
#endif
	
		return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS setHYPERBUSI_TAR_WTA(DWORD base_addr, DWORD reg_value)
//! @brief This function writes the Transaction Allocation Register for Write Transaction Allocation.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param reg_value           : Set the register value (bit[0:1] is valid).
//  @attention Nothing
//  @date Jan.06/2016
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS setHYPERBUSI_TAR_WTA(DWORD base_addr, DWORD reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    reg &= CLR_WTA(reg);
    reg |= SET_WTA(reg_value);
    HYPERBUSI_REG_WRITE(base_addr, REG_TAR_ADDR, reg);

#ifdef HYPERBUSI_READ_VERIFY
    reg = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    MSG("TAR: WTA %d\n", GET_WTA(reg));
#endif
	
		return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_TAR(DWORD base_addr, DWORD* reg_value)
//! @brief This function reads the Transaction Allocation Register.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (32bit).
//  @attention Nothing
//  @date Jan.06/2016
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_TAR(DWORD base_addr, DWORD* reg_value)
{

    HYPERBUSI_STATUS ret = NO_ERROR;

    *reg_value = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    MSG("TAR: 0x%08X\n", *reg_value);
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_TAR_RTA(DWORD base_addr, DWORD reg_value)
//! @brief This function reads the Transaction Allocation Register for Read Transaction Allocation.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[0:1] is valid).
//  @attention Nothing
//  @date Jan.06/2016
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_TAR_RTA(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    *reg_value = GET_RTA(reg);
    MSG("TAR: RTA %d\n", *reg_value);
    return ret;
}

//--------------------------------------------------------------------------
//! @fn HYPERBUSI_STATUS getHYPERBUSI_TAR_WTA(DWORD base_addr, DWORD reg_value)
//! @brief This function reads the Transaction Allocation Register for Write Transaction Allocation.
//! @return HYPERBUSI_STATUS    : NO_ERROR or ERROR(ref. hyperbusi.h)
//! @param base_addr            : System base address of HYPERBUSI to be manipulated.
//! @param *reg_value           : Get the register value (bit[0:1] is valid).
//  @attention Nothing
//  @date Jan.06/2016
//  @author H.Saito
//--------------------------------------------------------------------------
HYPERBUSI_STATUS getHYPERBUSI_TAR_WTA(DWORD base_addr, DWORD* reg_value)
{
    HYPERBUSI_STATUS ret = NO_ERROR;
    DWORD reg = 0;

    reg = HYPERBUSI_REG_READ(base_addr, REG_TAR_ADDR);
    *reg_value = GET_WTA(reg);
    MSG("TAR: WTA %d\n", *reg_value);
    return ret;
}

#endif // #if (HYPERBUSI_IP_VER >= 240)

