/* hyperbusi_target_specific.h - Source Code for HyperBus CTRL IP Driver */
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

#ifndef __HYPERBUSI_TARGET_SPECIFIC_H__
#define __HYPERBUSI_TARGET_SPECIFIC_H__

/* The hyperbusi_target_specific.h requires modification in order to work in your system. */

//=============================================================================
//  DEFINITIONS
//=============================================================================
#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short int
#endif
#ifndef DWORD
#define DWORD unsigned int
#endif

#define HYPERBUSI_REG_OFFSET(b,o)      *((volatile DWORD*)(b + o))
#define HYPERBUSI_REG_WRITE(b,o,d)     HYPERBUSI_REG_OFFSET((b),(o)) = (d)
#define HYPERBUSI_REG_READ(b,o)        HYPERBUSI_REG_OFFSET((b),(o))

//=============================================================================
//  HYPERBUSI IP version
//=============================================================================
#define HYPERBUSI_IP_VER          (241) // (221) (213) 

#ifndef HYPERBUSI_IP_VER
#define HYPERBUSI_IP_MAJOR_VER     2 //2 //2
#define HYPERBUSI_IP_MINOR_VER     1 //2 //3
#define HYPERBUSI_IP_REVISION_VER  3 //1 //1
#define HYPERBUSI_IP_VER           ((HYPERBUSI_IP_MAJOR_VER*100) + (HYPERBUSI_IP_MINOR_VER*10) + HYPERBUSI_IP_REVISION_VER)
#endif

//=============================================================================
//  Implementation Condition
//=============================================================================
#define HYPERBUSI_IP_WP_SUPPORT     // If WP is not allocated, HYPERBUSI_WPR is not supported. undef this macro.
#define HYPERBUSI_IP_INT_SUPPORT    // If INT is not allocated, HYPERBUSI_IEN and HYPERBUSI_ISR are not supported. undef this macro.

//=============================================================================
// Message
//=============================================================================
#define MSG(arg...)
#define ERR_MSG     

//=============================================================================
// Debug
//=============================================================================
#undef HYPERBUSI_READ_VERIFY

#endif //__HYPERBUSI_TARGET_SPECIFIC_H__

