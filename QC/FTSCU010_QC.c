
#include "SoFlexible.h"

#include "FTSCU010/pmu.h"

void FTSCU010_QC_Main(){

	int loop_tmp;
	u32 pmu_input;


	printf("main begin\n");

	//RichardLin A369
	fLib_Int_Init();

	for(;;){
		printf("\n\n======================================= IP_PA_BASE(0)=0x%0x\n", IP_PA_BASE(0));
		printf("PMU Testing in SoFlexible\n");
		printf("1: CPU Idle Test\n");
		printf("2: Turbo test\n");
		printf("3: FCS Test: Used Direct Entering FCS Mode (A369)by GMAC PLL\n");
		printf("6: Sleep: Wake Up Form Wake Up Buttom\n");
		printf("10: I2C change voltage\n");
		printf("11: exit\n");
		printf("Enter a Number for Testing ---> ");
//		scanf("%d", &pmu_input);
		pmu_input = get_dex();
		printf("Your input is ---> [ %d ] \n", pmu_input);
		switch(pmu_input){
			case 1:
				GPIO_InputTest_CPUIdel(0xff000000);
				printf("+++ begin 1: CPU Idle Test\n");
				printf("please toggle any pin of GPIO pin\n");
				CPU_Idle(); // in *asm.s
				printf("--- finish 1: CPU Idle Test\n");
				break;
			case 2:
				printf("+++ begin 2: Turbo test\n");
				TURBO_test_func();
				printf("--- finish 2: Turbo test\n");
				break;
			case 3:
				printf("+++ begin 3: FCS Test: Used Direct Entering FCS Mode by GMAC PLL\n");
				QC_FCS_byGMACPLL();
				printf("--- finish 3: FCS Test: Used Direct Entering FCS Mode by GMAC PLL\n");
				break;
			case 6:
				printf("+++ begin 6: Sleep: Wake Up Form Wake Up Buttom\n");
				PMU_sleep_test(2);
				printf("--- finish 6: Sleep: Wake Up Form Wake Up Buttom\n");
				break;
			case 10:
				printf("+++ begin 10: I2C change voltage\n");
				FTSCU010_Test_Main_I2CChangeVoltage();
				printf("--- finish 10: I2C change voltage\n");
				break;
			case 11:
				return;
		}

	}
}

