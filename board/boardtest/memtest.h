/***************************************************************************
* Copyright  Faraday Technology Corp 2008-2012.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:memtest.h                                                             *
* Description: SoFlexible board specfic routine                            *
* Author: Fred Chien                                                       *
****************************************************************************/
#define FILL_CONTENT_ADDRESS (0x00000001)
#define FILL_CONTENT_TYPE1   (0xffff0000)
#define FILL_CONTENT_TYPE2   (0x0000ffff)
#define FILL_CONTENT_TYPE3   (0xffffffff)
#define FILL_CONTENT_TYPE4   (0x5a5a5a5a)
#define FILL_CONTENT_TYPE5   (0xa5a5a5a5)

#define DATA_PATTERN0	(0x5a5a5a5a)
#define DATA_PATTERN1	(0xa5a5a5a5)
//#define DATA_PATTERN1	(0xa535535a)	// ==> ok
#define DATA_PATTERN2	(0xffffffff)
#define DATA_PATTERN3	(0x55555555)
#define DATA_PATTERN4	(0xf3f33f3f)	//==> fail
//#define DATA_PATTERN4	(0xf1f11f1f)	==> fail
//#define DATA_PATTERN4	(0x10100101)	// ==> ok
//#define DATA_PATTERN4	(0x00100000)
#define DATA_PATTERN5	(0xaabbccdd)	// ==> fail

#define EXIT_TUBE_FAIL   0x66
#define EXIT_TUBE_PASS   0x88
#define EXIT_TUBE_FINISH 0x99
