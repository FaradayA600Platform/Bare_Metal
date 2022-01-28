/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SoFlexible.h"

#include "types.h"
#include "io.h"

#include "FTIIC010/I2C_test.h"
#include "FTIIC010/I2C.h"			//library


#define IPMODULE 		IIC
#define IPNAME 			FTIIC010
*/
extern FTIIC010_Test_Main();

// Constant macro definition for the status of I2C state machine
#define I2C_SLAVE_INIT    	0
#define I2C_SAM_MATCH     	1
#define I2C_SLAVE_DATA		2

#define I2C_BUFFER_SIZE     16

void FTIIC010_QC_Main()
{
	FTIIC010_Test_Main();

#if 0
	char buf[128];
	char buf1[20];
	I2C_Dev dev[4];
	int chose;
	int i;
	unsigned int val;
	int deviceNum;

#if defined(CONFIG_PLATFORM_A369)
	printf(" i2c CONFIG_PLATFORM_A369\n");
	pin_mux_enable(TVEncode_DEV,0);
	// --------------------------------------------------------------------
	//	解決 tv encode 會拉住 I2C 的問題
	// --------------------------------------------------------------------
	// 1. select clock source	==> external clock
	val = readl(0x92000228);
	val = (val & 0xfffffffc) | 0x2;
	writel(val, 0x92000228);
	// 2. enable lcd
	writel(3, 0x94A00000);

	I2C_Init(&dev[0], IP_PA_BASE(0), I2C_GSR_Value, I2C_TSR_Value, 50);
	I2C_Init(&dev[1], IP_PA_BASE(1), I2C_GSR_Value, I2C_TSR_Value, 50);

#elif defined(CONFIG_PLATFORM_A380)	
	#define SCU_BASE	0x90A00000	
	#define PMUR_MFS0	0x1120	
	#define PMUR_MFS1	0x1124	
	#define PMUR_MFS2	0x1128	
	#define PMUR_MFS3	0x112c	
	
	unsigned long scu_pmur_mfs0, scu_pmur_mfs1, scu_pmur_mfs2, scu_pmur_mfs3; 
	printf(" i2c CONFIG_PLATFORM_A380\n");
	// Enable GPIO pin mux	
	scu_pmur_mfs0 = readl(SCU_BASE + PMUR_MFS0);				

	scu_pmur_mfs1 = readl(SCU_BASE + PMUR_MFS1);			

	scu_pmur_mfs2 = readl(SCU_BASE + PMUR_MFS2);		
	scu_pmur_mfs0 &= (~(BIT18|BIT19|BIT20|BIT21));	
	writel(scu_pmur_mfs0, SCU_BASE + PMUR_MFS0);
	
	scu_pmur_mfs3 = readl(SCU_BASE + PMUR_MFS3);		

	printf("scu_pmur_mfs0: 0x%x, scu_pmur_mfs1: 0x%x, scu_pmur_mfs2: 0x%x, scu_pmur_mfs3: 0x%x\n",
			scu_pmur_mfs0, scu_pmur_mfs1, scu_pmur_mfs2, scu_pmur_mfs3);
	
	I2C_Init(&dev[0], IP_PA_BASE(0), I2C_GSR_Value, I2C_TSR_Value, 50);		/// 50);		
	I2C_Init(&dev[1], IP_PA_BASE(1), I2C_GSR_Value, I2C_TSR_Value, 50);		/// 50);
	
#endif
	/*
	需等 spec.c 加入後才能這樣使用
	for (i=0; i<IP_COUNT; ++i)
	{
		I2C_Init(&dev[i], IP_PA_BASE(i), I2C_GSR_Value, I2C_TSR_Value, 50);		/// 50);
	}
	*/
	for (i=0; i<IP_COUNT; ++i)
	{
		printf("scan i2c %d\n",i);
		deviceNum=do_scan_dev(&dev[i],1);
		printf("There is(are) %d device(s)\n",deviceNum);
	}
#endif
	fLib_printf("End the FTIIC010_QC test\n");

}



