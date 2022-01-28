/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:types.h                                                            *
* Description: Faraday code types define                                 *
* Author: Fred Chien                                                       *
****************************************************************************/
#ifndef types_H_
#define types_H_
#include <autoconf.h>
#include <stdint.h>
#define CR									0x0D
#define LF									0x0A
#define BS									0x08
#define ESC 								27

/* These defines are in a common coding practices header file */

#ifndef	OK
#define OK		0                   /* Can never work out should this be 1 or 0? */
#endif

#ifndef	FALSE
#define FALSE	0
#endif
#ifndef	TRUE
#define TRUE	1
#endif
#ifndef	NULL
#define	NULL	0
#endif

#ifndef	bool
#define bool unsigned char
#endif
#ifndef	BOOLEAN
#define BOOLEAN unsigned char
#endif

#ifndef	false
#define false 0
#endif
#ifndef	true
#define true 1
#endif

#ifndef ON
#define ON		1
#endif

#ifndef OFF
#define	OFF		0
#endif


#ifndef ENABLE
#define ENABLE	1
#endif

#ifndef DISABLE
#define	DISABLE	0
#endif

#ifndef PARITY_NONE
#define PARITY_NONE		0
#endif

#ifndef PARITY_ODD
#define PARITY_ODD		1
#endif

#ifndef PARITY_EVEN
#define PARITY_EVEN		2
#endif

#ifndef PARITY_MARK
#define PARITY_MARK		3
#endif

#ifndef PARITY_SPACE
#define PARITY_SPACE	4
#endif

/* type define */
typedef unsigned long long 	UINT64;
typedef long long 			INT64;
typedef	unsigned int		UINT32;
typedef	int					INT32;
typedef	unsigned short		UINT16;
typedef	short				INT16;
typedef unsigned char		UINT8;
typedef char				INT8;
typedef unsigned char		BOOL;



typedef unsigned char           u_char;
typedef unsigned short          u_short;
typedef unsigned int            u_int;
typedef unsigned long           u_long;

/* sysv */
typedef unsigned char           unchar;
typedef unsigned short          ushort;
typedef unsigned int            uint;
typedef unsigned long           ulong;


typedef unsigned char           u8_t;
typedef unsigned short          u16_t;
typedef unsigned long           u32_t;

typedef unsigned char           __u8;
typedef unsigned short          __u16;
typedef unsigned int            __u32;
typedef unsigned long long      __u64;

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;

//typedef long long           int64_t;
typedef int64_t             s64;

typedef INT8          INT8S;
typedef UINT8         INT8U;
typedef INT16         INT16S;
typedef UINT16        INT16U;
typedef INT32         INT32S;
typedef UINT32        INT32U;

#define vLib_LeWrite8(x,y)   *(volatile INT8U *)((INT8U * )x)=(y)
#define vLib_LeWrite32(x,y)   *(volatile INT32U *)((INT8U * )x)=(y)  //bessel:add  (INT8U * )
#define u32lib_leread32(x)      *((volatile INT32U *)((INT8U * )x))  //bessel:add  (INT8U * )
#define u32Lib_LeRead32(x)      *((volatile INT32U *)((INT8U * )x)) //bessel:add  (INT8U * )

#if 0
#define STDBG(format, args...) printf("[%s:%d] "format, __FILE__, __LINE__, ##args)
#else
#define STDBG(args...)
#endif

#ifdef CONFIG_USE_FLIB_PRINTF
#define printf	fLib_printf
#define gets	fLib_scanf
#define atoi	fLib_atoi
#endif
#endif

