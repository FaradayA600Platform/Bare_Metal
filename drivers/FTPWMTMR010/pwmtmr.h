#ifndef __PWMTMR_H
#define __PWMTMR_H

#define MAX_TIMER               8

#define FTTMRCTRL_START         (1 << 1)
#define FTTMRCTRL_UPDATE        (1 << 2)
#define FTTMRCTRL_AUTORELOAD    (1 << 4)
#define FTTMRCTRL_INTEN         (1 << 5)


extern INT32 fLib_Timer_Counter(UINT32 timer);

extern void Timer1_Tick(void);
extern void Timer2_Tick(void);
extern void Timer3_Tick(void);
extern void Timer4_Tick(void);
extern void Timer5_Tick(void);
extern void Timer6_Tick(void);
extern void Timer7_Tick(void);
extern void Timer8_Tick(void);

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
#endif //__PWMTMR_H
