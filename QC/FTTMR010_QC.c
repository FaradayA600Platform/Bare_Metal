#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SoFlexible.h"
#include "../board/boardtest/timertest.h"
#include "../board/timer.h"

extern void Timer_Test_Main();
extern void Setup(int timer, int test, PrHandler handler);

#define	timer1 1
#define	timer2 2
#define	timer3 3
#define overflow_test	1
extern UINT32 TIMER_TEST_RESULT;

void FTTMR010_QC_Main()
{
	//Timer_Test_Main(0); // the same as ctd code
	TIMER_TEST_RESULT = TRUE;
	printf("Start timer QC test (overflow test)\n");
	Setup(timer1, overflow_test, (PrHandler)Timer1_Tick);
	Setup(timer2, overflow_test, (PrHandler)Timer2_Tick);
	Setup(timer3, overflow_test, (PrHandler)Timer3_Tick);
	printf("End the FTTMR010_QC test \n");
	return;
}
