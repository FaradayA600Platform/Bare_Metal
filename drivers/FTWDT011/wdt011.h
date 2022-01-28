#ifndef __WATCH_DOG_011_H
#define __WATCH_DOG_011_H

#include "types.h"

 /* registers */

#define IWDT_OFFSET     0x20

#define IWDT_KR         0x00
#define IWDT_CURR       0x00
#define IWDT_PR         0x04
#define IWDT_RLR        0x08
#define IWDT_SR         0x0C
#define IWDT_INTSR      0x10
#define IWDT_CR         0x14
#define IWDT_INTRLEN    0x18
#define IWDT_UDF        0x1C

#define IWDT_INTSTS    0x100
#define IWDT_FEATURES  0x104
#define IWDT_REVISION  0x10C

 /* status registers */
#define WdPVU           0x01
#define WdRVU           0x02

 /* control registers */

#define WdEnable        0x01
#define WdIntrEn        0x02  // system interrupt enable bit
#define WdRstEn         0x04  // system reset enable bit
#define WdExtEn         0x08  // external signal enable bit
#define WdClockSrc      0x10  // clock source
#define WdUDFIntrEn     0x40  // system interrupt enable bit for underflow
#define WdUDFEn         0x80  // underflow function enable bit

#define WdClkInt        0x00
#define WdClkExt        0x01

 /* prescaler registers */

typedef enum {
    DIVIDE_4,
    DIVIDE_8,
    DIVIDE_16,
    DIVIDE_32,
    DIVIDE_64,
    DIVIDE_128,
    DIVIDE_256,
    DIVIDE_MAX
} DIVISOR;

 /* interrupt status register (0x100) */
#define UDFShiftBits      16
#define OVFShiftBits      24

#define WdIntrMask        0xFF
#define WdUDFIntrMask     0xFF0000
#define WdOVFIntrMask     0xFF000000

 /* KEY Definition */
#define KICKDOG_KEY   0xAAAA
#define REPROG_KEY    0x5555
#define START_KEY     0xCCCC

typedef enum {
	DRVIWDT_0,
#ifndef CONFIG_PLATFORM_A600
	DRVIWDT_1,
	DRVIWDT_2,
	DRVIWDT_3,
	DRVIWDT_4,
	DRVIWDT_5,
	DRVIWDT_6,
	DRVIWDT_7,
#endif	
	DRVIWDT_MAX
} DRVIWDT;

/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

extern void fLib_WatchDog_Enable(DRVIWDT);
extern void fLib_WatchDog_KickDog(DRVIWDT);
extern void fLib_WatchDog_ReProg(DRVIWDT);
extern void fLib_WatchDog_SetPrescaler(DRVIWDT, UINT8);
extern void fLib_WatchDog_SetAutoReLoad(DRVIWDT,UINT32);
extern void fLib_WatchDog_SetSignalLength(DRVIWDT, UINT8);
extern void fLib_WatchDog_IntClockSrcEnable(void);
extern void fLib_WatchDog_ExtClockSrcEnable(void);
extern void fLib_WatchDog_ExtSignalEnable(DRVIWDT);
extern void fLib_WatchDog_ExtSignalDisable(DRVIWDT);
extern void fLib_WatchDog_SysIntEnable(DRVIWDT);
extern void fLib_WatchDog_SysIntDisable(DRVIWDT);
extern void fLib_WatchDog_UnderflowIntEnable(DRVIWDT);
extern void fLib_WatchDog_UnderflowIntDisable(DRVIWDT);
extern void fLib_WatchDog_SysResetEnable(DRVIWDT);
extern void fLib_WatchDog_SysResetDisable(DRVIWDT);

extern void fLib_WatchDog_UnderflowEnable(DRVIWDT);
extern void fLib_WatchDog_UnderflowDisable(DRVIWDT);
extern void fLib_WatchDog_SetUnderflowValue(DRVIWDT,UINT32);
extern BOOL fLib_WatchDog_IsCounterZero(DRVIWDT);
extern void fLib_WatchDog_ClearOverflowStatus(DRVIWDT);
extern void fLib_WatchDog_ClearUnderflowStatus(DRVIWDT);

extern UINT32 fLib_WatchDog_GetCurrentCounter(DRVIWDT);
extern UINT32 fLib_WatchDog_GetGlobalIntrStatus(void);
extern void fLib_WatchDog_ClearGlobalIntrStatus(UINT32);

#endif // __WATCH_DOG_H
