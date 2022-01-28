#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SoFlexible.h"
//#include "platform.h"
//#include "common.h"
//#include "types.h"
//#include "spec_macro.h"
//#include "spec.h"
//#include "io.h"

#include "I2C_test.h"
#include "I2C.h"			//library
#include "portme_i2c.h"

#define IPMODULE            IIC
#define IPNAME              FTIIC010

// Constant macro definition for the status of I2C state machine
#define I2C_HS_CODE         4
#define I2C_SLAVE_INIT      0
#define I2C_SAM_MATCH       1
#define I2C_SLAVE_TX_DATA   2
#define I2C_SLAVE_RX_DATA   3
//#define I2C_BUFFER_SIZE     256

#define NUMOFTEST           1

unsigned short gBurnIn;
unsigned short gHS=0;
unsigned short gBurst=0;
unsigned short gMasterINT=0;
char gCMD_buf[MAX_ARRAY_SIZE];
static unsigned long long gTick = 0;
unsigned short g10BITMODE = 0;
// 0 for disable 1 for 10-bit address mode
unsigned short g2IDX = 0;

void timer_tick_plus(void) {
	writel(0x7, 0x98700034);
	gTick++;
	writel(0x2, 0x98200008);
}

/*unsigned long int atonum(char *val_str)
{
	unsigned long int address;

	if (val_str[0] =='0' && val_str[1] == 'x')
		sscanf(val_str, "0x%x\n", &address);
	else
		address = atoi(val_str);

	return address;
}

int substring(char **ptr, char *string, char *pattern)
{
	int i;

	ptr[0]=(char *)strtok(string, pattern);
	for (i=0;ptr[i]!=NULL;++i)
	{
		ptr[i+1]=strtok(NULL,pattern);
	}
	return i;
}*/

// FTIIC010 as slave device, the current state of I2C_Slave_ISR()
static volatile int gI2CState;

// FTIIC010 as slave device with internal storage
static volatile int gIndex = 0;		//index of required to access from I2C_DataBuf
unsigned int I2C_DataBuf[I2C_BUFFER_SIZE];

// for debug purpose, save FTIIC010 SR history
unsigned int I2C_Status[I2C_BUFFER_SIZE * 2];
static volatile int gStatusIdx = 0;

I2C_Dev gI2CDev[4];
char errMessages[128];

void I2C_Slave_ISR(I2C_Dev *dev)
{
	volatile uint32_t SRstatus,CRstatus;
	uint32_t dummy;

	SRstatus = fLib_FTIIC010_ReadReg(dev, I2C_SR);
	fLib_FTIIC010_WriteReg(dev, I2C_SR, SRstatus); // w1c
	CRstatus = fLib_FTIIC010_ReadReg(dev, I2C_CR);

	// for debugging purpose
	if (gStatusIdx >= (I2C_BUFFER_SIZE * 2)) {
		gStatusIdx = 0;
	}
	I2C_Status[gStatusIdx++] = SRstatus;
/*
prints("slave state = 0x%x\n", gI2CState);
prints("slave status = 0x%x\n", SRstatus);
prints("----------------\n");
*/
	// debug end

	if (SRstatus & I2C_SR_AL) {
		fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_TB_EN|I2C_CR_SLAVE_ENABLE);
		// save error messages
		sprintf(errMessages, "I2C(0x%X): Arbitration lost, switch to slave mode\n", dev->io_base);
		return;
	}

	if (!gHS && (gI2CState==I2C_HS_CODE))
		gI2CState=I2C_SLAVE_INIT;

	if ((SRstatus&0x7FFFFF)==0x400924)
	{
		// this sloves the problem that HSS and ADDR come at the same time.
		// in burst HS mode, because there is no TB_en between HS code and
		// ADDR and the burst mode sends both HSS and ADDR by controller,
		// the controller can send HSS and ADDR almost at the same time.
		gI2CState = I2C_SLAVE_INIT;
		SRstatus = 0x924;
	}

	if ((gI2CState==I2C_HS_CODE)&&((SRstatus&I2C_SR_HSS)==I2C_SR_HSS))
		gI2CState=I2C_SLAVE_INIT;
	else if(SRstatus & I2C_SR_SAM)
	{
		// the I2C controller receives slave address that matches 
		// the address in the Slave Register (SAR) when the I2C controller is in the slave mode
		if (!(SRstatus & I2C_SR_RW)) // rx 
		{
			gI2CState = I2C_SAM_MATCH;
			dummy = fLib_FTIIC010_ReadReg(dev, I2C_DR);
		}
		else // slave write(tx)
		{
			gI2CState = I2C_SLAVE_TX_DATA;
			if (gIndex > I2C_BUFFER_SIZE)
			{
				gIndex = 0;
			}

			fLib_FTIIC010_WriteReg(dev, I2C_DR, I2C_DataBuf[gIndex++]);
		}
	
		fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_TB_EN|CRstatus);
	}
	else if ((gI2CState==I2C_SAM_MATCH) && (SRstatus&I2C_SR_TD))
	{
		// device address
		gIndex = fLib_FTIIC010_ReadReg(dev, I2C_DR);
		gI2CState = I2C_SLAVE_RX_DATA;
		fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_TB_EN|CRstatus);
	}
	else if (((gI2CState == I2C_SLAVE_RX_DATA) && (SRstatus & I2C_SR_TD)) ||\
		 ((gI2CState == I2C_SLAVE_RX_DATA) && (SRstatus & I2C_SR_STOP)))
	{
		if (SRstatus & I2C_SR_STOP)
		{
			gI2CState = I2C_SLAVE_INIT;
		}
		else
		{
			if (gIndex > I2C_BUFFER_SIZE)
				gIndex = 0;

			I2C_DataBuf[gIndex++] = fLib_FTIIC010_ReadReg(dev, I2C_DR);
			fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_TB_EN|CRstatus);
		}
	}
	else if ((gI2CState == I2C_SLAVE_TX_DATA) && (SRstatus & I2C_SR_TD))
	{
		if ((SRstatus & I2C_SR_STOP) || (SRstatus & I2C_SR_NACK))
		{
			gI2CState = I2C_SLAVE_INIT;
		}
		else
		{
			if (gIndex > I2C_BUFFER_SIZE)
			{
				gIndex = 0;
			}

			fLib_FTIIC010_WriteReg(dev, I2C_DR, I2C_DataBuf[gIndex++]);
			fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_TB_EN|CRstatus);
		}
	}
	else if ((SRstatus & I2C_SR_START) || (SRstatus & I2C_SR_STOP));
	else
	{
		// unknown, just ignore
		for (;;) ;
		//I2C_Init(dev, dev->io_base, I2C_GSR_Value, I2C_TSR_Value, 50);
		//fLib_FTIIC010_WriteReg(dev, I1C_SAR, SLAVE_DEVICE_ADDR);
		//fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_SLAVE_ENABLE);//|I2C_CR_GC_EN);
		//gI2CState = I2C_SLAVE_INIT;		// I2C_STOP is set ==> go to init state
		//gIndex = 0;
	}
}

int do_scan_dev(I2C_Dev *dev,unsigned int time)
{
	int slave_address;
	unsigned char tmp;
	unsigned int i;
	int deviceNum = 0;

	for(i = 0; i < time; i++){
		if (g10BITMODE) // 10-bit mode
		{
			for (slave_address=0; slave_address<=0x3ff; slave_address+=1)
			{
				if (I2C_MasterBurstRX(dev, &tmp, slave_address, 0, 1, 1) != -1)
				{
					prints("0x%3x ", slave_address);
					deviceNum++;
				}

				if (I2C_MasterBurstTX(dev, tmp, slave_address, 0, 1) != -1)
				{
					prints("0x%3x ", slave_address);
					deviceNum++;
				}
			}
		}
		else
		{
			prints("scan %dth / %d \n",i,time);
			for (slave_address=0; slave_address<0x7f; slave_address+=1)
			{
				// prints("addr[0x%2x]: ", slave_address);
				if (I2C_MasterReceive(dev, &tmp, slave_address, 0, 0) != -1)
				{
					prints("address 0x%x maybe have a device\n", slave_address);
					deviceNum++;
				}
			}
		}
	}
	return deviceNum;
}

#define TEST_SIZE			8	// some eeprom support only up to 8-byte/16-byte page write
//#define TEST_SIZE			16
void do_test_dev(I2C_Dev *dev, int slave_address)
{
	unsigned char buf[TEST_SIZE];
	unsigned char rd_buf[TEST_SIZE];
	int i;
	int rd_num;

	for (i=0; i<TEST_SIZE; ++i)
	{
		buf[i] = i;
	}
	if (I2C_MasterTransfer(dev, buf, slave_address, 0, TEST_SIZE) != TEST_SIZE)
	{
		prints("test write fail\n");
		return;
	}
	for (i=0; i<1000000; ++i)		// do_delay (why test will fail without this delay??)
	{
	}
	memset(rd_buf, 0, TEST_SIZE);
	if ((rd_num = I2C_MasterReceive(dev, rd_buf, slave_address, 0, TEST_SIZE)) != TEST_SIZE)
	{
		prints("test read fail: %d\n", rd_num);
		return;
	}
	if (memcmp(buf, rd_buf, TEST_SIZE) != 0)
	{
		prints("compare fail\n");
		return;
	}

	prints("test success\n\n");
}

// --------------------------------------------------------------------
//	return
//		0 ==> time out
//		1 ==> success
// --------------------------------------------------------------------
void Do_MultiMaster_Test()
{
	unsigned char addr0 = 0xa8, addr1 = 0x5c;

	// gI2CDev[1] and gI2CDev[0] both as Master
	// gI2CDev[0] set slave address as 0x5c
	// gI2CDev[1] write to slave address 0x5c
	// gI2CDev[0] write to slave address 0xa8
	prints("I2C(0x%X) as Master and select slave 0x%x\n", gI2CDev[1].io_base, addr1);
	prints("I2C(0x%X) as Master and select slave 0x%x\n", gI2CDev[0].io_base, addr0);

	if (!I2C_CheckStatus(&gI2CDev[1], 0, I2C_SR_BB, READ_TIME_OUT))
	{
		prints("I2C(0x%X) bus busy\n", gI2CDev[1].io_base);
		return;
	}
	fLib_FTIIC010_WriteReg(&gI2CDev[1], I2C_DR, addr1);
	fLib_FTIIC010_WriteReg(&gI2CDev[1], I2C_CR, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START);

	if (!I2C_CheckStatus(&gI2CDev[0], 0, I2C_SR_BB, READ_TIME_OUT))
	{
		prints("I2C(0x%X) bus busy\n", gI2CDev[0].io_base);
		return;
	}
	fLib_FTIIC010_WriteReg(&gI2CDev[0], I2C_DR, addr0);
	fLib_FTIIC010_WriteReg(&gI2CDev[0], I2C_CR, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START);

	// wait status
	// slave addresss for write
	while (!I2C_CheckStatus(&gI2CDev[1], I2C_SR_TD, I2C_SR_TD|I2C_SR_RW|I2C_SR_NACK|I2C_SR_AL, WRITE_TIME_OUT)) {
		prints("I2C(0x%X) select slave 0x%x timeout, select again...\n", gI2CDev[1].io_base, addr1);
		fLib_FTIIC010_WriteReg(&gI2CDev[1], I2C_DR, addr1);
		fLib_FTIIC010_WriteReg(&gI2CDev[1], I2C_CR, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START);
		prints("%s", errMessages);
		memset(errMessages, 0, sizeof(errMessages));
	}

	prints("%s", errMessages);
	memset(errMessages, 0, sizeof(errMessages));
	prints("I2C(0x%X) select slave 0x%x successed\n", gI2CDev[1].io_base, addr1);

	if ( Do_Write_Data(&gI2CDev[1], (unsigned char)0, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP|I2C_CR_NACK) == 0 ) {
		prints("I2C(0x%X) select slave 0x%x Failed\n", gI2CDev[1].io_base, addr1);
	}

	prints("I2C(0x%X): Do test slave 0x%X ...\n", gI2CDev[1].io_base, addr1);
	do_test_dev(&gI2CDev[1], addr1);
}

void Do_Arbitration_Lost_Test()
{
/****************************************************************/
/*************  Arbitration Lost Test for YAMAHA  ***************/
/****************************************************************/
/* Date: 2020/07/03						*/
/* customer: YAMAHA						*/
/* scenario:							*/
/*	4 I2Cs:							*/
/*		M1: addr=0xa8700000 IRQ=12			*/
/*		S1: addr=0xa8800000 IRQ=13 SA=0x70		*/
/*		M2: addr=0xa8900000 IRQ=14			*/
/*		S2: addr=0xa8a00000 IRQ=15 SA=0x71		*/
/* The goal of this test is we want to let two masters 		*/
/* write addr at the same time and let M1 arbitration 		*/
/* lost. While M1 is in AL, the M2 access should go 		*/
/* finely. In this test, we use interrupt mode.			*/
/* the programming sequence is:					*/
/*	1. setup for master1 writing slave addr2 (0x71), 	*/
/*	   but NOT really write					*/
/*	2. setup for master2 writing slave addr1 (0x70), 	*/
/*	   but NOT really write					*/
/*	3. set both masters' cr to transfer addr 		*/
/*	   almost simotanuously in order to get 		*/
/*	   arbitration lost					*/
/*	4. wait status						*/
/*		M1 gets AL and timeout, because M1 		*/
/*		sends 0x70 to bus.				*/
/*		M2 wins, because M2 sends 0x71 to bus.		*/
/*	5. M2 writes slave index, while M1 is in AL		*/
/*	6. M2 writes data and sets STOP bit, while M1 is AL	*/
/*		In this test, the STOPI_EN has to be set,	*/
/*		so that M1 which is in AL can get STOP 		*/
/*		interrupt and get out of AL.			*/
/*		After M1 gets STOPI, and ISR clears the 	*/
/*		STOP bit of SR, M1 can access normally 		*/
/*		again.						*/
/*	After this test, you can use normal read 		*/
/*	(ex. read 0x70 0x3 1) to read S2, and you will 		*/
/*	get 0x5. And everything go back to normal as well.	*/
/*								*/
/* Note:							*/
/*	please set cdr=0xA100ffff to make sure the I2C slow 	*/
/*	enough to have AL. And enable STOPI_EN. When M1 gets 	*/
/*	STOP, the ISR clears the SR and then M1 can transfer 	*/
/*	again.							*/
/****************************************************************/

	unsigned long cr1, cr2;

	I2C_RequestIRQ(&gI2CDev[0], IRQ, IP_IRQ(0));
	I2C_RequestIRQ(&gI2CDev[2], IRQ, IP_IRQ(2));
	gI2CDev[0].slaveState = I2C_SLAVE_INIT;
	gI2CDev[1].slaveState = I2C_SLAVE_INIT;
	gI2CDev[2].slaveState = I2C_SLAVE_INIT;
	gI2CDev[3].slaveState = I2C_SLAVE_INIT;
	gI2CDev[0].ind=0;
	gI2CDev[1].ind=0;
	gI2CDev[2].ind=0;
	gI2CDev[3].ind=0;

	// setup for master1 writing slave addr2 (0x71)
	cr1 = fLib_FTIIC010_ReadReg(&gI2CDev[0], I2C_CR);
	cr1 = (cr1|I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START)&0xfffbffff; // clear arb_off
	gI2CDev[0].status = 0;
	fLib_FTIIC010_WriteReg(&gI2CDev[0], I2C_DR, 0x71<<1);

	// setup for master2 writing slave addr1 (0x70)
	cr2 = fLib_FTIIC010_ReadReg(&gI2CDev[2], I2C_CR);
	cr2 = (cr2|I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START)&0xfffbffff; // clear arb_off
	gI2CDev[2].status = 0;
	fLib_FTIIC010_WriteReg(&gI2CDev[2], I2C_DR, 0x70<<1);

	// set both masters' cr to transfer addr almost simotanuously
	// in order to get arbitration lost
        fLib_FTIIC010_WriteReg(&gI2CDev[0], I2C_CR, cr1);
	fLib_FTIIC010_WriteReg(&gI2CDev[2], I2C_CR, cr2);

        // wait status
	I2C_CheckStatus(&gI2CDev[0], I2C_SR_TD, I2C_SR_TD|I2C_SR_NACK|I2C_SR_AL, WRITE_TIME_OUT);
	I2C_CheckStatus(&gI2CDev[2], I2C_SR_TD, I2C_SR_TD|I2C_SR_NACK|I2C_SR_AL, WRITE_TIME_OUT);

	// master2 writing slave index, while master1 is AL
	cr2 = I2C_CR_ENABLE|I2C_CR_TB_EN;
	Do_Write_Data(&gI2CDev[2], 0x3, cr2);
	
	// master2 writing data, while master1 is AL
	cr2 = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP;
	Do_Write_Data(&gI2CDev[2], 0x5, cr2);

	prints("M2 writes S2(sa=0x70): index=0x3 data=0x5\n");
}

void do_command(I2C_Dev *dev, unsigned char *dBuf)
{
	int i;
	int slave_address;
	int slave_idx;
	int threshold;
	int num;
	int argc;
	char *argv[40];
#ifdef A369_VOLTAGE_VAL
	unsigned char vol_setting;
#endif

	for (i=0;i<MAX_ARRAY_SIZE;i++)
		dBuf[i] = '\0';

	while(1)
	{
		if (gMasterINT) // interrupt mode
		{
			I2C_RequestIRQ(&gI2CDev[0], IRQ, IP_IRQ(0));		// for i2c channel 0
			//I2C_RequestIRQ(&gI2CDev[1], IRQ, IP_IRQ(1));		// for i2c channel 1
			//I2C_RequestIRQ(&gI2CDev[2], IRQ, IP_IRQ(2));		// for i2c channel 2
			//I2C_RequestIRQ(&gI2CDev[3], IRQ, IP_IRQ(3));		// for i2c channel 3
		}
		else // polling mode
		{
			irq_set_disable(IP_IRQ(0));			// for i2c channel 0
			//irq_set_disable(IP_IRQ(1));			// for i2c channel 1
			//irq_set_disable(IP_IRQ(2));			// for i2c channel 2
			//irq_set_disable(IP_IRQ(3));			// for i2c channel 3
			dev->irq = -1;
		}

		if (gHS)
			gI2CState = I2C_HS_CODE; // init state
		else
			gI2CState = I2C_SLAVE_INIT; 
			
		if (gBurst) {
			if (!gBurnIn)
			{
				prints("command:\n");
				prints("    read [slave_address] [slave_index] [num] [threshold]\n");
				prints("    write [slave_address] [slave_index] [data0] [data1] ... [data31]\n");
				prints("    exit\n");
				prints("please input command(LOWERcase please): ");
				for (i=0;i<MAX_ARRAY_SIZE;i++)
					gCMD_buf[i]='\0';

				gets(gCMD_buf);
				prints("\n");
			}

			argv[0]=NULL;
			argc = substring(argv, gCMD_buf, " \r\n\t");

			if (argc==5 && strcmp(argv[0], "read")==0)
			{
				slave_address = atonum(argv[1]);
				slave_idx = atonum(argv[2]);
				threshold = atonum(argv[4]);
				num = atonum(argv[3]);

				if (I2C_MasterBurstRX(dev, dBuf, slave_address, slave_idx, num, threshold)<0)
				{
					prints("read failure\n");
				}
				else
				{
					// prints("read %x %x %x\n", slave_address, slave_idx, num);
					if (!gBurnIn)
					{
						for (i=0; i<num; ++i)
							prints("[0x%x] = 0x%x\n", slave_idx+i, dBuf[i]);
					}
				}
			}
			else if (argc>=4 && strcmp(argv[0], "write")==0)
			{
				slave_address = atonum(argv[1]);
				slave_idx = atonum(argv[2]);
				for (i=0; i<argc-3; ++i)
				{
					dBuf[i] = atonum(argv[i+3]);
				}

				if (I2C_MasterBurstTX(dev, dBuf, slave_address, slave_idx, argc-3)<0)
				{
					prints("write failure\n");
				}
			}
			else if (argc==1 && strcmp(argv[0], "exit")==0)
			{
				return;
			}
		}
		else {
			if (!gBurnIn)
			{
				prints("command:\n");
				prints("    read [slave_address] [slave_index] {num}\n");
				prints("    write [slave_address] [slave_index] [data0] [data1] [data2] [data3]\n");
//		prints("    test [slave_address of eeprom]\n");
	#ifdef A369_VOLTAGE_VAL
				prints("    voltageval {0~63}\n");
	#endif
				prints("    scan  \n");
//		prints("    combined_format [slave_address] [slave_index] [num]\n");
//		prints("    multimaster  \n");
				prints("    exit\n");
				prints("please input command(LOWERcase please): ");
				for (i=0;i<MAX_ARRAY_SIZE;i++)
					gCMD_buf[i]='\0';

				gets(gCMD_buf);
				prints("\n");
			}

			argv[0]=NULL;
			argc = substring(argv, gCMD_buf, " \r\n\t");
			if (argc>=3 && strcmp(argv[0], "read")==0)
			{
				slave_address = atonum(argv[1]);
				slave_idx = atonum(argv[2]);
				if (argc>3)
					num = atonum(argv[3]);
				else
					num = 1;

				if (I2C_MasterReceive(dev, dBuf, slave_address, slave_idx, num)<0)
				{
					prints("read failure\n");
				}
				else
				{
					//prints("read %x %x %x\n", slave_address, slave_idx, num);
					if (!gBurnIn)
					{
						for (i=0; i<num; ++i)
							prints("[%x] = %x\n", slave_idx+i, dBuf[i]);
					}
				}
			}
			else if (argc>=4 && strcmp(argv[0], "write")==0)
			{
				slave_address = atonum(argv[1]);
				slave_idx = atonum(argv[2]);
				for (i=0; i<argc-3; ++i)
				{
					dBuf[i] = atonum(argv[i+3]);
				}
				if (I2C_MasterTransfer(dev, dBuf, slave_address, slave_idx, argc-3)<0)
				{
					prints("write failure\n");
				}
			}
			else if (argc==2 && strcmp(argv[0], "test")==0)
			{
				slave_address = atonum(argv[1]);
				do_test_dev(dev, slave_address);
			}
#ifdef A369_VOLTAGE_VAL
			else if (argc==2 && strcmp(argv[0], "voltageval")==0)
			{
				slave_address = atonum(argv[1]);
				slave_idx= slave_address*125+ 7500;
				prints("voltageval  B vol_mul=%d vol=%d/10000  \n", slave_address, slave_idx);

				vol_setting= (unsigned char)slave_address| 0x80;
				//vol_setting= (unsigned char)slave_address;
				//vol_setting= vol_setting| 0x80;
				prints("voltageval  B VOl_MEL %d 0x%0x\n", vol_setting, vol_setting);
				//I2C_Write (dev, 0x00000096, 0x00000000 , (unsigned char )vol_setting);
				//prints("voltageval  B 0\n");
				I2C_Write (dev, 0x00000096, 0x00000001 , (unsigned char )vol_setting);
				prints("voltageval  B 1\n");
				I2C_Write (dev, 0x00000096, 0x00000002 , 0x00000000);
				prints("voltageval  B 2\n");
				I2C_Write (dev, 0x00000096, 0x00000003 , 0x00000080 );
				prints("voltageval  B 3\n");

				#if 0
				I2C_Write (dev, 0x00000096, 0x00000001 , (unsigned char )vol_setting);
				prints("voltageval  B 2.1\n");
				I2C_Write (dev, 0x00000096, 0x00000002 , 0x00000000);
				prints("voltageval  B 2.2\n");
				I2C_Write (dev, 0x00000096, 0x00000003 , 0x00000080 );
				prints("voltageval  B 2.3\n");
				#endif
				prints("voltageval E\n");
				
			}
#endif
			else if (argc==1 && strcmp(argv[0], "exit")==0)
			{
				return;
			}
			else if (strcmp(argv[0], "scan")==0)
			{
				if (argc==1)
				{
					if (g10BITMODE)
					{
						for (slave_address=0x0; slave_address<=0x3ff; slave_address++)
						{
							prints("addr[0x%3x]: ", slave_address);
                					fLib_FTIIC010_WriteReg(&gI2CDev[1], I2C_SAR, (slave_address&TENBITADDR_MASK)|ADDR10EN);
							i=do_scan_dev(dev, 1);
							if (i==2) // one for RX, the other for TX
								prints("has a valid address\n", slave_address);
							else
								prints("has %d matches\n", i);
						}
					}
					else
						do_scan_dev(dev, 1);
				}
				if (argc==2)
				{
					do_scan_dev(dev, strtol(argv[1], NULL ,16));
				}
			}
			else if (strcmp(argv[0], "multimaster")==0)
			{
				Do_MultiMaster_Test();
			}
			else if (strcmp(argv[0], "arbitration")==0)
			{
				Do_Arbitration_Lost_Test();
			}
			else if (argc==4 && strcmp(argv[0], "combined_format")==0)
			{
				slave_address = atonum(argv[1]);
				slave_idx = atonum(argv[2]);
				num = atonum(argv[3]);
				if (I2C_CombinedFormatTest(dev, dBuf, slave_address, slave_idx, num)<0)
				{
					prints("combined_format test failed\n");
				}
				else
				{
					prints("combined_format test slave 0x%x:\n", slave_address);
					prints("read 0x%x 0x%x:\n", slave_idx, num);
					for (i=0; i<num; ++i)
					{
						prints("[%x] = 0x%x\n", slave_idx+i, dBuf[i]);
					}
				}
			}
		}

		if (gBurnIn)
			return;
	}
}

void FTIIC010_Slave_Init(I2C_Dev *dev, int nIRQ)
{
	unsigned long int cr, ar;

	if (g10BITMODE)
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (SLAVE_DEVICE_ADDR&TENBITADDR_MASK)|ADDR10EN);
	else
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, SLAVE_DEVICE_ADDR&ADDR_MASK);

	cr=I2C_CR_SLAVE_ENABLE;
	if (gHS)
		cr=cr|I2C_CR_HSI_EN|I2C_CR_SBI_EN;

	fLib_FTIIC010_WriteReg(dev, I2C_CR, cr);

	irq_set_type(nIRQ, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler(nIRQ, I2C_Slave_ISR, &dev[0]);
	irq_set_enable(nIRQ);
}

void BurnInTest(int sAddr, unsigned char *dBuf)
{
	int i, j, k, b, h;
	unsigned char dBufForBurnIn[MAX_ARRAY_SIZE]; // to collect all dBufs
	unsigned long int ind = 0; // index for dBufForBurnIn

	prints("Burn-In: gMasterINT = %d; gHS = %d; gBurst = %d\n", gMasterINT, gHS, gBurst);
	for (j=0;j<NUMOFTEST;j++)
	{
		for (i=0;i<MAX_ARRAY_SIZE;i++)
			gCMD_buf[i]='\0';

		//Do write
		for (i=0;i<0x100;i+=8)
		{
			sprintf(gCMD_buf, "write 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\0",
			        sAddr, i, i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
			prints("gCMD_buf = %s\n", gCMD_buf);
			do_command(&gI2CDev[0], dBuf);
		}

		prints("Burn-In: write finishes...\n");

		for (b=1;b<=32;b++) // test each number of burst ex. burst1, burst2... burst32
		{
			prints("start round[%d] read-compare test...\n", b);
			//Do read
			if (gBurst)
			{
				// Because of 32-byte buffer, we have to split 256 bytes to multiple 32-byte accesses.
				ind = 0;
				for (i=0;i<256;i=i+b)
				{
					for (k=0;k<MAX_ARRAY_SIZE;k++)
						gCMD_buf[k]='\0';

					if (b<3)
						sprintf(gCMD_buf, "read 0x%x 0x%x %d %d\0", sAddr, ind, b, b);
					else
						sprintf(gCMD_buf, "read 0x%x 0x%x %d %d\0", sAddr, ind, b, 3);

					do_command(&gI2CDev[0], dBuf);
					for (h=0;h<b;h++)
					{
						dBufForBurnIn[ind]=dBuf[h];
						ind++;
					}
				}
			}
			else
			{
				for (i=0;i<MAX_ARRAY_SIZE;i++)
					gCMD_buf[i]='\0';

				sprintf(gCMD_buf, "read 0x%x 0x0 256\0", sAddr);
				do_command(&gI2CDev[0], dBufForBurnIn);
			}

			//Do compare
			for (i=0;i<256;i++)
			{
				if (dBufForBurnIn[i]!=i)
				{
					prints("Burn-In failed!!!\n");
					prints("index0x%X = 0x%x NOT 0x%x\n", i, dBufForBurnIn[i], i);
					while(1);
				}
			}
		}
	}
}

void FTIIC010_Test_Main()
{
	char buf[128];
	unsigned char dBuf[MAX_ARRAY_SIZE];
	int chose;
	int sAddr;
	int i, j, k, b;
	unsigned int val;
	unsigned short burnInMode;
	unsigned long long t = 0;

	int argc;
	char *argv[40];

#if defined(CONFIG_A369)
	// --------------------------------------------------------------------
	//	解決 tv encode 會拉住 I2C 的問題
	// --------------------------------------------------------------------
	// 1. select clock source	==> external clock
	val = readl(0x92000228);
	val = (val & 0xfffffffc) | 0x2;
	writel(val, 0x92000228);
	// 2. enable lcd
	writel(3, 0x94A00000);

	/**********************************************************************************
	A369 pinmux setting: FMPSR1(Bit Assignment of Multi-function Pin Setting Register 1)
	A369: FMPSR1 0x23C: FG_I2C1[17:16] = b'10: I2C1
	A369: FMPSR1 0x23C: FG_SSP1[9:8] = b'01: I2C1
	***********************************************************************************/
	/*
	val = readl(0x9200023c);
	val = (val & 0xfffcfcff) | 0x20100;
	writel(val, 0x9200023c);
	*/
#elif defined(CONFIG_A360)
	unsigned int SYSC_R = SCU_FTSCU010_PA_BASE;
	unsigned int SYSR_I2C1_MASK_0x18 = 0x000f0000;
	unsigned int tmp;

	tmp = readl(SYSC_R+0x18) & (~SYSR_I2C1_MASK_0x18);
	tmp |= 0x000a0000;           // enable I2C 1 
	writel(tmp, SYSC_R+0x18);
#elif defined(CONFIG_PLATFORM_A380)
	#define SCU_BASE        0x90A00000
	#define PMUR_MFS0       0x1120
	#define PMUR_MFS1       0x1124
	#define PMUR_MFS2       0x1128
	#define PMUR_MFS3       0x112c

	unsigned long scu_pmur_mfs0, scu_pmur_mfs1, scu_pmur_mfs2, scu_pmur_mfs3;

	// Enable I2C pin mux
	scu_pmur_mfs0 = readl(SCU_BASE + PMUR_MFS0);
	scu_pmur_mfs0 &= (~(BIT18|BIT19|BIT20|BIT21));
	writel(scu_pmur_mfs0, SCU_BASE + PMUR_MFS0);

	scu_pmur_mfs1 = readl(SCU_BASE + PMUR_MFS1);

	scu_pmur_mfs2 = readl(SCU_BASE + PMUR_MFS2);

	scu_pmur_mfs3 = readl(SCU_BASE + PMUR_MFS3);

	printf("scu_pmur_mfs0: 0x%x, scu_pmur_mfs1: 0x%x, scu_pmur_mfs2: 0x%x, scu_pmur_mfs3: 0x%x\n",
	        scu_pmur_mfs0, scu_pmur_mfs1, scu_pmur_mfs2, scu_pmur_mfs3);
	        
#elif defined(CONFIG_PLATFORM_A500) || defined(CONFIG_PLATFORM_A600) 
        
		// 2020-1110, A500 code, A600 can't work now. Just mark as note.
		//#define SCU_BASE        0x2D000000     
        //writel(0x1, SCU_BASE + 0x8418);
        //writel(0x1, SCU_BASE + 0x841c);
        //writel(0x1, SCU_BASE + 0x8420);
        //writel(0x1, SCU_BASE + 0x8424);
		// 2020-1110, A500 code, A600 can't work now. Just mark as note.
 
        //I2C_Init(&gI2CDev[0], IP_PA_BASE(0), I2C_GSR_Value, I2C_TSR_Value, 50);             /// 50);                
        //I2C_Init(&gI2CDev[1], IP_PA_BASE(1), I2C_GSR_Value, I2C_TSR_Value, 50);             /// 50);

#else
	#error "unknown platform"
/*	
	enable_interrupts(); // clear the cpu interrupt mask(I bit) to let CPU able to catch irq
	writel(0x0, 0x98700030); // tiemr CR
	writel(0x0, 0x98700038); // tiemr mask
	irq_set_type(1, IRQ_TYPE_EDGE_RISING);
	irq_install_handler(1, timer_tick_plus, 0);
	irq_set_enable(1);
	writel(0x1000000, 0x98700000); // tiemr counter
	writel(0x1000000, 0x98700004); // timer load
	writel(0x5, 0x98700030); //timer enable
	//prints("gTick = %llu\n", gTick);
*/
#endif

	//prints("ii2 i1\n\r");
	fLib_FTIIC010_WriteReg(&gI2CDev[0], I2C_CR, I2C_CR_I2C_RST);
#if IP_COUNT > 1
	//fLib_FTIIC010_WriteReg(&gI2CDev[1], I2C_CR, I2C_CR_I2C_RST);
	//fLib_FTIIC010_WriteReg(&gI2CDev[2], I2C_CR, I2C_CR_I2C_RST);
	//fLib_FTIIC010_WriteReg(&gI2CDev[3], I2C_CR, I2C_CR_I2C_RST);
#endif

	/*** i2c init ***/
	// Note: init must be after reset otherwise the cr would be reset to 0.
	// I2C_Init has to apply to both master and slave
	// the last parameter indicates the speed of I2C bus(50kHz)
	//prints("ii2 i2\n\r");
	I2C_Init(&gI2CDev[0], IP_PA_BASE(0), I2C_GSR_Value, I2C_TSR_Value, LOW_SPEED, HIGH_SPEED); 
#if IP_COUNT > 1
	//I2C_Init(&gI2CDev[1], IP_PA_BASE(1), I2C_GSR_Value, I2C_TSR_Value, LOW_SPEED, HIGH_SPEED);
	//I2C_Init(&gI2CDev[2], IP_PA_BASE(2), I2C_GSR_Value, I2C_TSR_Value, LOW_SPEED, HIGH_SPEED);
	//I2C_Init(&gI2CDev[3], IP_PA_BASE(3), I2C_GSR_Value, I2C_TSR_Value, LOW_SPEED, HIGH_SPEED);
#endif

	// slave must be interrupt mode, while master can becaome interrupt mode by this function, otherwise it is polling mode.
	//I2C_RequestIRQ(&gI2CDev[0], IRQ, IP_IRQ(0)); // set master as interrupt mode
	//I2C_RequestIRQ(&gI2CDev[1], IRQ, IP_IRQ(1));

	//prints("ii2 i3\n\r");
//#if IP_COUNT > 1
	//FTIIC010_Slave_Init(&gI2CDev[1], IP_IRQ(2));
//#endif
	enable_interrupts(); // clear the cpu interrupt mask(I bit) to let CPU able to catch irq
	prints("ii2 i4\n\r");
#if 0
	while(1)
	{
		prints("-----------------------------------\n");
		prints("choose test mode:\n");
		prints("    0. normal test\n");
		prints("    1. burn-in test\n");
		prints("please enter 0 or 1: ");
		for (i=0;i<MAX_ARRAY_SIZE;i++)
			gCMD_buf[i]='\0';

		gets(gCMD_buf);
		prints("\n");
		argv[0]=NULL;
		argc=substring(argv, gCMD_buf, " \r\n\t");
		gBurnIn=atonum(argv[0]);
		if (argv[0][1]=='\0')
		{
			if ((argv[0][0]=='0')||(argv[0][0]=='1'))
				break;
		}
	}

	if (gBurnIn)
	{
		while(1)
		{
			prints("-----------------------------------\n");
			prints("choose slave to burn-in:\n");
			prints("    0. 0x2f(I2C as slave)\n");
			prints("    1. 0x54(EEPROM)\n");
			prints("please enter 0 or 1: ");
			for (i=0;i<MAX_ARRAY_SIZE;i++)
				gCMD_buf[i]='\0';

			gets(gCMD_buf);
			prints("\n");
			argv[0]=NULL;
			argc=substring(argv, gCMD_buf, " \r\n\t");
			sAddr=atonum(argv[0]);
			if (argv[0][1]=='\0')
			{
				if ((argv[0][0]=='0')||(argv[0][0]=='1'))
					break;
			}
		}

		if (sAddr==0) sAddr=0x2f; else sAddr=0x54;

		while(1)
		{
			prints("-----------------------------------\n");
			prints("choose mode:\n");
			prints("    0. nonHS nonBurst polling test\n");
			prints("    1. nonHS Burst polling test\n");
			prints("    2. HS nonBurst polling test\n");
			prints("    3. HS Burst polling test\n");
			prints("    4. nonHS nonBurst interrupt test\n");
			prints("    5. nonHS Burst interrupt test\n");
			prints("    6. HS nonBurst interrupt test\n");
			prints("    7. HS Burst interrupt test\n");
			prints("    8. ALL tests\n");
			prints("please enter 0, 1, 2, 3, 4, 5, 6, 7 or 8: ");
			for (i=0;i<MAX_ARRAY_SIZE;i++)
				gCMD_buf[i]='\0';

			gets(gCMD_buf);
			prints("\n");
			argv[0]=NULL;
			argc=substring(argv, gCMD_buf, " \r\n\t");
			burnInMode=atonum(argv[0]);
			if (argv[0][1]=='\0')
			{
				if ((argv[0][0]=='0')||(argv[0][0]=='1')||(argv[0][0]=='2')||(argv[0][0]=='3')||
				    (argv[0][0]=='4')||(argv[0][0]=='5')||(argv[0][0]=='6')||(argv[0][0]=='7')||
				    (argv[0][0]=='8'))
					break;
			}
		}

		if ((sAddr==0x54)&&(burnInMode==2||burnInMode==3||burnInMode==6||burnInMode==7))
		{
			prints("EEPROM does NOT support HS mode\n");
			return;
		}

		if (burnInMode==0) {gMasterINT = 0; gHS = 0; gBurst = 0; BurnInTest(sAddr, dBuf);}
		else if (burnInMode==1) {gMasterINT = 0; gHS = 0; gBurst = 1; BurnInTest(sAddr, dBuf);}
		else if (burnInMode==2) {gMasterINT = 0; gHS = 1; gBurst = 0; BurnInTest(sAddr, dBuf);}
		else if (burnInMode==3) {gMasterINT = 0; gHS = 1; gBurst = 1; BurnInTest(sAddr, dBuf);}
		
		else if (burnInMode==4) {gMasterINT = 1; gHS = 0; gBurst = 0; BurnInTest(sAddr, dBuf);}
		else if (burnInMode==5) {gMasterINT = 1; gHS = 0; gBurst = 1; BurnInTest(sAddr, dBuf);}
		else if (burnInMode==6) {gMasterINT = 1; gHS = 1; gBurst = 0; BurnInTest(sAddr, dBuf);}
		else if (burnInMode==7) {gMasterINT = 1; gHS = 1; gBurst = 1; BurnInTest(sAddr, dBuf);}
		else if (burnInMode==8)
		{
			gMasterINT = 0; gHS = 0; gBurst = 0; BurnInTest(sAddr, dBuf);
			gMasterINT = 0; gHS = 0; gBurst = 1; BurnInTest(sAddr, dBuf);
			if (sAddr!=0x54) // EEPROM does not support HS mode
			{
				gMasterINT = 0; gHS = 1; gBurst = 0; BurnInTest(sAddr, dBuf);
				gMasterINT = 0; gHS = 1; gBurst = 1; BurnInTest(sAddr, dBuf);
			}

			gMasterINT = 1; gHS = 0; gBurst = 0; BurnInTest(sAddr, dBuf);
			gMasterINT = 1; gHS = 0; gBurst = 1; BurnInTest(sAddr, dBuf);
			if (sAddr!=0x54)
			{
				gMasterINT = 1; gHS = 1; gBurst = 0; BurnInTest(sAddr, dBuf);
				gMasterINT = 1; gHS = 1; gBurst = 1; BurnInTest(sAddr, dBuf);
			}
		}

		prints("Burn-in test finished...\n");
	}
	else // normal test
	{
		while(1)
		{
			prints("-----------------------------------\n");
			prints("choose interrput mode or polling for master:\n");
			prints("    0. polling mode\n");
			prints("    1. interrupt mode\n");
			prints("please enter 0 or 1: ");
			for (i=0;i<MAX_ARRAY_SIZE;i++)
				gCMD_buf[i]='\0';

			gets(gCMD_buf);
			prints("\n");
			argv[0]=NULL;
			argc=substring(argv, gCMD_buf, " \r\n\t");
			gMasterINT=atonum(argv[0]);
			if (argv[0][1]=='\0')
			{
				if ((argv[0][0]=='0')||(argv[0][0]=='1'))
					break;
			}
		}

		while(1)
		{
			prints("-----------------------------------\n");
			prints("choose access mode:\n");
			prints("    0. command mode\n");
			prints("    1. burst mode\n");
			prints("please enter 0 or 1: ");
			for (i=0;i<MAX_ARRAY_SIZE;i++)
				gCMD_buf[i]='\0';

			gets(gCMD_buf);
			prints("\n");
			argv[0]=NULL;
			argc=substring(argv, gCMD_buf, " \r\n\t");
			gBurst=atonum(argv[0]);
			if (argv[0][1]=='\0')
			{
				if ((argv[0][0]=='0')||(argv[0][0]=='1'))
					break;
			}
		}

		while(1)
		{
			prints("-----------------------------------\n");
			prints("speed mode:\n");
			prints("    0. normal speed\n");
// 			prints("    1. high speed\n");
//			prints("please enter 0 or 1: ");
			prints("please enter 0 : ");
			for (i=0;i<MAX_ARRAY_SIZE;i++)
				gCMD_buf[i]='\0';

			gets(gCMD_buf);
			prints("\n");
			argv[0]=NULL;
			argc=substring(argv, gCMD_buf, " \r\n\t");
			gHS=atonum(argv[0]);
			if (argv[0][1]=='\0')
			{
				if ((argv[0][0]=='0')||(argv[0][0]=='1'))
					break;
			}
		}

		do_command(&gI2CDev[0], dBuf);
	}
#else
		while(1)
		{
			prints("-----------------------------------\n");
			prints("speed mode:\n");
			prints("    0. normal speed\n");
// 			prints("    1. high speed\n");
//			prints("please enter 0 or 1: ");
			prints("please enter 0 : ");
			for (i=0;i<MAX_ARRAY_SIZE;i++)
				gCMD_buf[i]='\0';

			gets(gCMD_buf);
			prints("\n");
			argv[0]=NULL;
			argc=substring(argv, gCMD_buf, " \r\n\t");
			gHS=atonum(argv[0]);
			if (argv[0][1]=='\0')
			{
				if ((argv[0][0]=='0')||(argv[0][0]=='1'))
					break;
			}
		}
		do_command(&gI2CDev[0], dBuf);		// for i2c channel 0
		//do_command(&gI2CDev[1], dBuf);		// for i2c channel 1
		//do_command(&gI2CDev[2], dBuf);		// for i2c channel 2
		//do_command(&gI2CDev[3], dBuf);		// for i2c channel 3
#endif
	/* 2/7 We want to debug by using ICE, so I2C0 is master(skip select).
	for (;;)
	{
		for (i = 0; i < IP_COUNT; ++i)
		{
			prints("%d: I2C%d\n", i + 1, i);
		}
		prints("0: exit\n");
		prints("choose master:");
		gets(buf);
		prints("\r");
		chose = atoi(buf);
		if (chose > 0 && chose <= IP_COUNT)
		{
			do_command(&gI2CDev[chose - 1]);
		}
		else if (chose == 0)
		{
			return;
		}
	}
	*/
}
