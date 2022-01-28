#ifndef __TIMER_H
#define __TIMER_H


typedef enum Timer_IoType
{
	IO_TIMER_RESETALL,
	IO_TIMER_GETTICK,
	IO_TIMER_SETTICK,
	IO_TIMER_SETCLKSRC
}fLib_Timer_IoType;


typedef struct
{
	UINT32 hour;
	UINT32 minute;
	UINT32 second;
}fLib_Time;

typedef struct
{
	UINT32 year;
	UINT32 month;
	UINT32 day;
}fLib_Date;


#define FTTMRCTRL_START         (1 << 1)
#define FTTMRCTRL_UPDATE        (1 << 2)
#define FTTMRCTRL_AUTORELOAD    (1 << 4)
#define FTTMRCTRL_INTEN         (1 << 5)

/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
 
//this routines will export to upper ap or test program
extern BOOL fLib_Timer_Init(UINT32 timer,UINT32 tick,PrHandler handler);
extern INT32 fLib_Timer_IOCtrl(fLib_Timer_IoType IoType,UINT32 timer,UINT32 tick);

#if defined(CONFIG_FTPWMTMR010)
extern INT32 fLib_Timer_Counter(UINT32 timer);
#elif defined(CONFIG_FTTMR010)
extern INT32 fLib_Timer_Counter(UINT32 timer, UINT32 value);
#endif

extern void Timer1_Tick(void);
extern void Timer2_Tick(void);
extern void Timer3_Tick(void);

extern void Timer1_Tick_VP(void);
extern void Timer2_Tick_VP(void);
extern void Timer3_Tick_VP(void);

extern void fLib_Timer_AutoReloadValue(UINT32 timer, UINT32 value);
extern void fLib_Timer_MatchValue1(UINT32 timer, UINT32 value);
extern void fLib_Timer_MatchValue2(UINT32 timer, UINT32 value);
extern void fLib_Timer_Control(UINT32 timer, UINT32 value);
extern void fLib_Timer_CmpValue(UINT32 timer, UINT32 value);

extern INT32 fLib_Timer_Enable(UINT32 timer);
extern INT32 fLib_Timer_Disable(UINT32 timer);
extern INT32 fLib_Timer_Close(UINT32 timer);
extern INT32 fLib_Timer_IntEnable(UINT32 timer);
extern INT32 fLib_Timer_IntClear(UINT32 timer);
extern void Timer_ResetAll(void);

extern int timer_init(unsigned int timer_n,unsigned int ext_clk);
extern void timer_start(unsigned int timer_n);
extern float timer_end(unsigned timer_n);
#endif //__TIMER_H
