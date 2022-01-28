#ifndef TIMERTEST_H_
#define TIMERTEST_H_


#define NUMOFTIMER 3
#define TIMEOUT_COUNTER  0xfffffffe		//for count-down or count-up??
#define UPPER_COUNT 0x5
#define UP 1
#define DOWN 0
#define INTERNAL 1
#define EXTERNAL 2
#define PCLK 0
#define EXTCLK 1

extern void Timer1_Tick(void);
extern void Timer2_Tick(void);
extern void Timer3_Tick(void);
extern void Timer4_Tick(void);
extern UINT32 T1_Tick, T2_Tick, T3_Tick, T4_Tick;

#endif /*TIMERTEST_H_*/
