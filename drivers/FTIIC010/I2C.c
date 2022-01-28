/***************************************************************************
* Copyright  Faraday Technology Corp 2014.  All rights reserved.           *
*--------------------------------------------------------------------------*
* Name:I2C.c                                                               *
* Description: I2C C Library routine                                       *
* Author: Fred Chien                                                       *
* Author: Alex Chang                                                       *
****************************************************************************
* Revise History                                                           *
* 2003/05/29 Add fLib_I2C_GetCR function                                   *
* 2014/03/26 Alex Chang, Modify codes to sync with SW programming guide    *
****************************************************************************/

#define Debug_IIC_Scan_001 1
//#define DEBUG

//#include "types.h"			//library
//#include "common.h"
//#include "platform.h"
//#include "tool/bits.h"
//#include "tool/utility.h"

//#include "spec.h"
//#include "intc.h"			//library
//#include "board.h"			//library
//#include "io.h"
#include "I2C.h"
#include "SoFlexible.h"
#include "portme_i2c.h"

extern I2C_Dev gI2CDev[];
extern unsigned short gHS;
extern unsigned short gBurst;
extern unsigned short g10BITMODE;
extern unsigned short g2IDX;
unsigned char gData_buf[MAX_ARRAY_SIZE];

uint32_t fLib_FTIIC010_ReadReg(I2C_Dev *dev, uint32_t reg_offset)
{
	return readl(dev->io_base + reg_offset);
}

void fLib_FTIIC010_WriteReg(I2C_Dev *dev, uint32_t reg_offset, uint32_t value)
{
	writel(value, dev->io_base + reg_offset);
}

// --------------------------------------------------------------------
//	return
//		0 ==> time out
//		1 ==> success
// --------------------------------------------------------------------
int I2C_CheckStatus(I2C_Dev *dev, unsigned long wait_status, unsigned long wait_status_mask, unsigned int time_out)
{
	for (; time_out>0; --time_out)
	{
		if (dev->irq == -1)		// polling
		{
			dev->status = fLib_FTIIC010_ReadReg(dev, I2C_SR);
			fLib_FTIIC010_WriteReg(dev, I2C_SR, dev->status); // w1c
		}
		else // interrupt
		{
			/*
			These two consecutive lines have potential problem.
			Think about this scenario:
			if an interrupt happens between them, then when pc comes back,
			the program clears dev->status and you never get the waited status!
			So, it is uncorrect to clear the status in this point. The right
			point should be the moment we get the waited status.
			*/
			//status = dev->status;
			//dev->status = 0;
		}

		if ((dev->status & wait_status_mask) == wait_status)
			break;
	}
	
	// without debugging, we always turn it off, because it generates mess when "scan".
	//if (time_out==0)
//		printf("time out");
//	while(1);
	
	return (time_out > 0);
}

// --------------------------------------------------------------------
//	return
//		0 ==> time out
//		1 ==> success
// --------------------------------------------------------------------
int Do_Write_RWAddr(I2C_Dev *dev, unsigned char addr, unsigned long control)
{
	unsigned long cr = fLib_FTIIC010_ReadReg(dev, I2C_CR);
	int ret = 0;

	printf("addr = %x\n",addr);
//	while(1);
	dev->status = 0; // this initial code should be before setting CR,
	                 // otherwise this intial code might ruin the result.
	fLib_FTIIC010_WriteReg(dev, I2C_DR, addr);
	fLib_FTIIC010_WriteReg(dev, I2C_CR, control);


//	printf("@");
	// wait status
	if (gHS &&( addr == 0x8)) // HS code
	{
			ret = I2C_CheckStatus(dev, I2C_SR_TD|I2C_SR_NACK, I2C_SR_TD|I2C_SR_NACK|I2C_SR_AL, WRITE_TIME_OUT);
//			printf("AA ret =%d\n",ret);
//			while(1);
			return ret;
	}
	else
	{
//		printf("normal speed\n");
		if (cr & I2C_CR_ARB_OFF)
		{
			ret = I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD|I2C_SR_NACK, WRITE_TIME_OUT);
//			printf("BB ret =%d\n",ret);
//			printf("aa\n");
//			while(1);
			return ret;
		}
		else
		{
			ret =  I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD|I2C_SR_NACK|I2C_SR_AL, WRITE_TIME_OUT);
//			printf("CC ret =%d\n",ret);
//			printf("bb\n");			
//			while(1);
			return ret;
		}
	}
}

// --------------------------------------------------------------------
//	return
//		0 ==> time out
//		1 ==> success
// --------------------------------------------------------------------
int Do_Write_Data(I2C_Dev *dev, unsigned char data, unsigned long control)
{
	dev->status = 0; // this initial code should be before setting CR,
	                 // otherwise this intial code might ruin the result.
	fLib_FTIIC010_WriteReg(dev, I2C_DR, data);
	fLib_FTIIC010_WriteReg(dev, I2C_CR, control);

	// wait status
	return I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD|I2C_SR_RW|I2C_SR_NACK|I2C_SR_AL, WRITE_TIME_OUT);
}

// --------------------------------------------------------------------
//	return
//		-1 ==> time out
//		0~255 ==> data received
// --------------------------------------------------------------------
int Do_Read(I2C_Dev *dev, unsigned long control)
{
	dev->status = 0; // this initial code should be before setting CR,
	                 // otherwise this intial code might ruin the result.
	fLib_FTIIC010_WriteReg(dev, I2C_CR, control);

	if ( I2C_CheckStatus(dev, I2C_SR_TD|I2C_SR_RW, I2C_SR_TD|I2C_SR_RW|I2C_SR_NACK|I2C_SR_AL, READ_TIME_OUT) == 0 )
	{
		return -1;
	}

	return fLib_FTIIC010_ReadReg(dev, I2C_DR);
}

// --------------------------------------------------------------------
//	return
//		-1 ==> time out
//		0~255 ==> data received
// --------------------------------------------------------------------
int Do_Read_Last(I2C_Dev *dev, unsigned long control)
{
	dev->status = 0; // this initial code should be before setting CR,
	                 // otherwise this intial code might ruin the result.
	fLib_FTIIC010_WriteReg(dev, I2C_CR, control);

	if ( I2C_CheckStatus(dev, I2C_SR_TD|I2C_SR_RW|I2C_SR_NACK, I2C_SR_TD|I2C_SR_RW|I2C_SR_NACK|I2C_SR_AL, READ_TIME_OUT) == 0 )
	{
		return -1;
	}

	return fLib_FTIIC010_ReadReg(dev, I2C_DR);
}

// --------------------------------------------------------------------
//	SCLout: 要設成的 SCL rate (單位: KHz), 0 ==> 使用 default clock rate
//			CSCLout means I2C bus clock value , *1024
// --------------------------------------------------------------------
#define divRoundUp(n,s)	((n+s-1)/(s))
int I2C_SetSCLout(I2C_Dev *dev, unsigned int ls, unsigned int hs)
{
	int  i, gsr;
	unsigned long int count, countH, cdr;

/* I comment these code, because the max frequency become to be 3.4MHz	
	if ( i_SCLout == 0 )
	{
		i_SCLout = I2C_Default_FREQ;		// default. clock is 45KHz
	}

	if (i_SCLout > I2C_MAX_FREQ) 
	{
		i_SCLout = I2C_MAX_FREQ;			// max. clock is 400KHz
	}
*/

	gsr = ((fLib_FTIIC010_ReadReg(dev, I2C_TGSR) >> 10) & 0x7);
	// prints("(((TIMER_CLOCK<%u>/ls<%u>)-gsr<%u>)/2)-2\n", TIMER_CLOCK, ls, gsr);
	count = 0xF7; // (((TIMER_CLOCK/ls)-gsr)/2)-2;
	countH = 0x8; // (((TIMER_CLOCK/hs)-gsr)/2)-2;
	if (!gHS&&(count>=BIT10)) 
	{
		prints(" APB_CLK=%d, ls=%d\n",TIMER_CLOCK, ls);
		prints("Pclk is to fast to form i2c clock, fail \n");
		return -1;
	}

	//cdr = (countH<<20)|count;
	cdr = 0xA10000F7;
		
	fLib_FTIIC010_WriteReg(dev, I2C_CDR, cdr);

	return 0;
}

// --------------------------------------------------------------------
//	return
//		-1 ==> error
//		other ==> number of bytes readed
// --------------------------------------------------------------------
int I2C_MasterReceive(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num)
{
	unsigned int i;
	int readed_value;
	unsigned char waddr = ((unsigned char)address<<1) & 0xFE;
	unsigned char raddr = waddr | 0x1;
	unsigned long int cr;

	if (!I2C_CheckStatus(dev, 0, I2C_SR_BB, READ_TIME_OUT))
	{
		prints("I2C bus busy\n");
		return -1;
	}

	if (gHS)
	{
		printf("write addr = 0x8\n");
	// --------------------------------------------------------------------
	// 	0. write 0b'00001xxx to boardcast HS access(0x8=0b00001000)
	// --------------------------------------------------------------------
		if ( Do_Write_RWAddr(dev, (unsigned char)0x8, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START)==0 )
		{
			return -1;
		}
	}

//	printf("waddr = %d\n",waddr);

	// --------------------------------------------------------------------
	// 	1. write slave address to select device and set CR
	// --------------------------------------------------------------------
	cr = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START;
//	printf("cr = %lx\n",cr);
	if (gHS)
		cr = cr|I2C_CR_HS_MODE; //|I2C_CR_ARB_OFF;

	if ( Do_Write_RWAddr(dev, (unsigned char)waddr, cr) == 0 )
	{
//		printf("z0\n");
		#if Debug_IIC_Scan_001
//		prints("Debug_IIC_Scan_001 I2C_MasterReceive() 001: waddr = 0x%X\n", waddr);
		#endif 

		fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_TB_EN|I2C_CR_STOP);	
		return -1;
	}

	if (num == 0)
	{
		if ( Do_Write_Data(dev, (unsigned char)(index&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP|I2C_CR_NACK) == 0 )
		{
			#if Debug_IIC_Scan_001
			prints("Debug_IIC_Scan_001 I2C_MasterReceive() 002\n");
			#endif 
			readed_value = -1;
		}
		else
		{
			readed_value = 0;
		}
		return readed_value;
	}

	printf("dev->address_mode = %lx\n",dev->address_mode);

	if (dev->address_mode >= 1)
	{
		// --------------------------------------------------------------------
		//	2a. write index to device (要從那個 index 開始讀)
		// --------------------------------------------------------------------
		cr = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP;
		if (gHS)
			cr = cr|I2C_CR_HS_MODE|I2C_CR_ARB_OFF;

		if ( Do_Write_Data(dev, (unsigned char)(index&0xff), cr) == 0 )
		{
			#if Debug_IIC_Scan_001
			prints("Debug_IIC_Scan_001 I2C_MasterReceive() 003\n ");
			#endif 
			return -1;
		}
	}


	if (dev->address_mode >= 2)
	{
		// --------------------------------------------------------------------
		//	2b. write index to device (要從那個 index 開始讀)
		// --------------------------------------------------------------------
		cr = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP;
		if (gHS)
			cr = cr|I2C_CR_HS_MODE;

		if ( Do_Write_Data(dev, (unsigned char)((index>>8)&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN) == 0 )
		{
			#if Debug_IIC_Scan_001
			prints("Debug_IIC_Scan_001 I2C_MasterReceive() 004\n");
			#endif 
			return -1;
		}
	}

	// --------------------------------------------------------------------
	// 	3. write slave address(RW bit high) to start master receive 
	// --------------------------------------------------------------------
	cr = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START;
	if (gHS)
		cr = cr|I2C_CR_HS_MODE|I2C_CR_ARB_OFF;

	if ( Do_Write_RWAddr(dev, (unsigned char)raddr, cr) == 0 )
	{
		#if Debug_IIC_Scan_001
		prints("Debug_IIC_Scan_001 I2C_MasterReceive() 005\n ");
		#endif 
		return -1;
	}

	// --------------------------------------------------------------------
	// 	4. more data to read
	// --------------------------------------------------------------------
	for (i = 0; i < num - 1; i++)
	{	
		cr = I2C_CR_ENABLE|I2C_CR_TB_EN;
		if (gHS)
			cr = cr|I2C_CR_HS_MODE|I2C_CR_ARB_OFF;

		readed_value = Do_Read(dev, cr);
		if (readed_value == -1)
		{
			#if Debug_IIC_Scan_001
			prints("Debug_IIC_Scan_001 I2C_MasterReceive() 006\n ");
			#endif 
			return -1;
		}

		buf[i] = (unsigned char)readed_value;
		//prints("r buf[%d] = %u\n", i, buf[i]);
	}

	// --------------------------------------------------------------------
	// 	5. read last data 
	// --------------------------------------------------------------------
	cr = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP|I2C_CR_NACK;
	if (gHS)
		cr = cr|I2C_CR_HS_MODE|I2C_CR_ARB_OFF;

	readed_value = Do_Read_Last(dev, cr);
	if (readed_value == -1)
	{
		#if Debug_IIC_Scan_001
		prints("Debug_IIC_Scan_001 I2C_MasterReceive() 007\n ");
		#endif 
		return -1;
	}

	fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_DISABLE);

	buf[i] = (unsigned char)readed_value;
	// prints("r buf[last] = %u\n", buf[i]);
	return num;
}

int I2C_MasterBurstRX(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num, unsigned long threshold)
{
	int i;
	unsigned char dr;
	u32 cr=0;
	u32 bstmr = 0, threshold_cnt = 0, total_cnt = 0;

	if (g10BITMODE&&g2IDX)
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX2_OFF)|(index<<MEM_IDX_OFF)|(address&TENBITADDR_MASK)|ADDR10EN|M2BIDX_EN);	
	else if (g10BITMODE&&!g2IDX)
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX_OFF)|(address&TENBITADDR_MASK)|ADDR10EN);
	else if (!g10BITMODE&&g2IDX)
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX2_OFF)|(index<<MEM_IDX_OFF)|(address&ADDR_MASK)|M2BIDX_EN);
	else
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX_OFF)|(address&ADDR_MASK));

	fLib_FTIIC010_WriteReg(dev, I2C_BSTMR, (num<<BSTTDC)|threshold);
	bstmr = fLib_FTIIC010_ReadReg(dev, I2C_BSTMR);
	threshold_cnt = bstmr&0x3f;
	total_cnt = (bstmr>>BSTTDC)&0x3f;
	if (dev->irq != -1) // interrupt mode
	{
		for (i=0; i<MAX_ARRAY_SIZE; i++ )
			gData_buf[i] = 0;
	}

	cr = FLOW_CTRL_READ;
	if (gHS)
		cr = cr|I2C_CR_HS_MODE;

	if (dev->irq != -1)
		cr = cr|I2C_CR_BSTTHODI_EN|I2C_CR_TDI_EN;

	dev->status = 0; // this initial code should be before setting CR,
	                 // otherwise this intial code might ruin the result.
	fLib_FTIIC010_WriteReg(dev, I2C_CR, cr);

	if (dev->irq == -1) // polling mode
	{ 
		if (!I2C_CheckStatus(dev, I2C_SR_BSTTHOD, I2C_SR_BSTTHOD, BURST_READ_TIME_OUT)) // waiting for theshold
			return -1;

		if (!(dev->status&I2C_SR_TD))
		{
			for (i=0; i<threshold_cnt; i++)
			{
				dr = (unsigned char)fLib_FTIIC010_ReadReg(dev, I2C_DR);
				buf[i] = dr;
				//prints("dr[%u] = %u\n", i, dr);
			}

			if (!I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD, BURST_READ_TIME_OUT)) // waiting for TD
				return -1;

			for (i=threshold_cnt; i<total_cnt; i++)
			{
				dr = (unsigned char)fLib_FTIIC010_ReadReg(dev, I2C_DR);
				buf[i] = dr;
				//prints("dr[%u] = %u\n", i, dr);
			}

			return num;
		}
		else // TD comes with THOD
		{
			for (i=0; i<total_cnt; i++)
			{
				dr = (unsigned char)fLib_FTIIC010_ReadReg(dev, I2C_DR);
				buf[i] = dr;
				// prints("dr[0x%x] = %x\n", i, dr);
			}

			return num;
		}
	}
	else // interrupt mode
	{
		if (!I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD, BURST_READ_TIME_OUT)) // waiting for TD
			return -1;

		for (i=0;i<total_cnt;i++)
			buf[i] = gData_buf[i];

		return num;
	}
/* working version with out considering of theshold
	if (!I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD, BURST_READ_TIME_OUT)) // waiting for TD
	{
		#if Debug_IIC_Scan_001
		prints("Burst read: TD is NOT got until time out\n");
		#endif 
		return -1;
	}

	for (i=0; i<num; i++)
	{
		dr = (unsigned char)fLib_FTIIC010_ReadReg(dev, I2C_DR);
		buf[i] = dr;
	}
*/
}

// --------------------------------------------------------------------
//	return
//		-1 ==> failure
//		other ==> byte readed
// --------------------------------------------------------------------
int I2C_Read(I2C_Dev *dev, unsigned int address, unsigned int index)
{
	unsigned char val;

	if (I2C_MasterReceive(dev, &val, address, index, 1)==-1)
	{
//		prints("read failure: %x %x\n", address, index);
		return -1;
	}
	return val;
}

// --------------------------------------------------------------------
//	return
//		-1 ==> error
//		other ==> number of bytes writed
// --------------------------------------------------------------------
int I2C_MasterTransfer(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num)
{
	unsigned int i;
	unsigned char waddr = ((unsigned char)address << 1) & 0xFE;
	unsigned long int cr;

	if (!I2C_CheckStatus(dev, 0, I2C_SR_BB, READ_TIME_OUT))
	{
		prints("I2C bus busy\n");
		return -1;
	}

	if (gHS)
	{
	// --------------------------------------------------------------------
	// 	0. write 0b'00001xxx to boardcast HS access(0x8=0b00001000)
	// --------------------------------------------------------------------
	if ( Do_Write_RWAddr(dev, (unsigned char)0x8, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START)==0 ) //|I2C_CR_ARB_OFF)==0 )
	{
		return -1;
	}
	}

	// --------------------------------------------------------------------
	// 	1. write slave address to select device and set CR
	// --------------------------------------------------------------------
	cr = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START;
	if (gHS)
		cr = cr|I2C_CR_HS_MODE; //|I2C_CR_ARB_OFF;

	if ( Do_Write_RWAddr(dev, (unsigned char)waddr, cr)==0 )
	{
		return -1;
	}

	if (dev->address_mode >= 1) // 8-bit mode
	{
		// --------------------------------------------------------------------
		//	2a. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		cr = I2C_CR_ENABLE|I2C_CR_TB_EN;
		if (gHS)
			cr = cr|I2C_CR_HS_MODE; //|I2C_CR_ARB_OFF;

		if (Do_Write_Data(dev, (unsigned char)(index&0xff), cr)==0)
			return -1;
	}

	if (dev->address_mode >= 2) // 16-bit mode
	{
		// --------------------------------------------------------------------
		//	2b. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		cr = I2C_CR_ENABLE|I2C_CR_TB_EN;
		if (gHS)
			cr = cr|I2C_CR_HS_MODE;

		if ( Do_Write_Data(dev, (unsigned char)((index>>8)&0xff), cr)==0 )
			return -1;
	}	

	// --------------------------------------------------------------------
	// 	3. more than one data to write
	// --------------------------------------------------------------------
	for (i=0; i < num-1; i++)
	{
		//prints("w buf[%d] = %u\n", i, buf[i]);
		cr = I2C_CR_ENABLE|I2C_CR_TB_EN;
		if (gHS)
			cr = cr|I2C_CR_HS_MODE; //|I2C_CR_ARB_OFF;

		if ( Do_Write_Data(dev, buf[i], cr) == 0)
				return -1;
	}

	// --------------------------------------------------------------------
	// 	4. write last data
	// --------------------------------------------------------------------
	cr = I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP;
	if (gHS)
		cr = cr|I2C_CR_HS_MODE; //|I2C_CR_ARB_OFF;

	// prints("w buf[last] = %u\n", buf[i]);
	if (Do_Write_Data(dev, buf[i], cr)==0)
		return -1;

	fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_DISABLE);
	return num;
}

int I2C_MasterBurstTX(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num)
{
	int i;
	u32 cr;

	if (g10BITMODE&&g2IDX)
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX2_OFF)|(index<<MEM_IDX_OFF)|(address&TENBITADDR_MASK)|ADDR10EN|M2BIDX_EN);
	else if (g10BITMODE&&!g2IDX)
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX_OFF)|(address&TENBITADDR_MASK)|ADDR10EN);
	else if (!g10BITMODE&&g2IDX)
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX2_OFF)|(index<<MEM_IDX_OFF)|(address&ADDR_MASK)|M2BIDX_EN);
	else
		fLib_FTIIC010_WriteReg(dev, I2C_SAR, (index<<MEM_IDX_OFF)|(address&ADDR_MASK));

	fLib_FTIIC010_WriteReg(dev, I2C_BSTMR, (num<<BSTTDC)|num);
	for (i=0;i<num;i++)
	{
		fLib_FTIIC010_WriteReg(dev, I2C_DR, buf[i]);
		//prints("buf[%u] = %u\n", i, buf[i]);
	}

	cr = FLOW_CTRL_WRITE;
	if (gHS)
		cr = cr|I2C_CR_HS_MODE;

	if (dev->irq != -1)
		cr = cr|I2C_CR_TDI_EN;

	dev->status = 0; // this initial code should be before setting CR,
	                 // otherwise this intial code might ruin the result.
	fLib_FTIIC010_WriteReg(dev, I2C_CR, cr);

	if (!I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD, BURST_WRITE_TIME_OUT))
	{
		#if Debug_IIC_Scan_001
		prints("Burst read: TD is NOT got until time out\n");
		#endif 
		return -1;
	}


/*
	if (dev->irq == -1) // polling mode
	{
		if (!I2C_CheckStatus(dev, I2C_SR_TD, I2C_SR_TD, BURST_WRITE_TIME_OUT))
		{
			#if Debug_IIC_Scan_001
			prints("Burst read: TD is NOT got until time out\n");
			#endif 
			return -1;
		}
	}
	else // interrupt mode
	{
		while (!(dev->status&I2C_SR_TD));
	}
*/

	return num;
}

// --------------------------------------------------------------------
//	return
//		-1 ==> error
//		other ==> number of bytes writed
// --------------------------------------------------------------------
int I2C_CombinedFormatTest_EEPROM(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num)
{
	unsigned int i;
	unsigned char waddr = (unsigned char)address & 0xFE;
	int readed_value;
	unsigned char raddr = waddr | 0x1;

	if (!I2C_CheckStatus(dev, 0, I2C_SR_BB, READ_TIME_OUT))
	{
		prints("I2C bus busy\n");
		return -1;
	}

	// --------------------------------------------------------------------
	// 	Combined format: write direction
	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// 	1. write slave address to select device
	// --------------------------------------------------------------------
	if ( Do_Write_RWAddr(dev, (unsigned char)waddr, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START)==0 )
	{
		return -1;
	}

	if (dev->address_mode >= 1)
	{
		// --------------------------------------------------------------------
		//	2a. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		if ( Do_Write_Data(dev, (unsigned char)(index&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN)==0 )
		{
			return -1;
		}
	}

	if (dev->address_mode >= 2)
	{
		// --------------------------------------------------------------------
		//	2b. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		if ( Do_Write_Data(dev, (unsigned char)((index>>8)&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN)==0 )
		{
			return -1;
		}
	}

	// --------------------------------------------------------------------
	// 	Combined format: read direction
	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// 	3. write slave address(RW bit high) to start master receive 
	// --------------------------------------------------------------------
	if ( Do_Write_RWAddr(dev, (unsigned char)raddr, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START) == 0 )
	{
		#if Debug_IIC_Scan_001
		prints("Debug_IIC_Scan_001 I2C_CombinedFormatTest_EEPROM() 001\n ");
		#endif 
		return -1;
	}

	// --------------------------------------------------------------------
	// 	4. more data to read
	// --------------------------------------------------------------------
	for (i = 0; i < num - 1; i++)
	{
		readed_value = Do_Read(dev, I2C_CR_ENABLE|I2C_CR_TB_EN);
		if (readed_value == -1)
		{
			#if Debug_IIC_Scan_001
			prints("Debug_IIC_Scan_001 I2C_CombinedFormatTest_EEPROM() 002\n ");
			#endif 
			return -1;
		}
		buf[i] = (unsigned char)readed_value;
	}

	// --------------------------------------------------------------------
	// 	5. read last data 
	// --------------------------------------------------------------------
	readed_value = Do_Read_Last(dev, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP|I2C_CR_NACK);
	if (readed_value == -1)
	{
		#if Debug_IIC_Scan_001
		prints("Debug_IIC_Scan_001 I2C_CombinedFormatTest_EEPROM() 003\n ");
		#endif 
		return -1;
	}

	buf[i] = (unsigned char)readed_value;

	fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_DISABLE);
	return 0;
}

// --------------------------------------------------------------------
//	return
//		-1 ==> error
//		other ==> number of bytes writed
// --------------------------------------------------------------------
int I2C_CombinedFormatTest(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num)
{
	unsigned int i;
	unsigned char waddr = (unsigned char)address & 0xFE;
	int readed_value;
	unsigned char raddr = waddr | 0x1;

	if (!I2C_CheckStatus(dev, 0, I2C_SR_BB, READ_TIME_OUT))
	{
		prints("I2C bus busy\n");
		return -1;
	}

	// --------------------------------------------------------------------
	// 	Combined format: write direction
	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// 	1. write slave address to select device
	// --------------------------------------------------------------------
	if ( Do_Write_RWAddr(dev, (unsigned char)waddr, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START)==0 )
	{
		return -1;
	}

	if (dev->address_mode >= 1)
	{
		// --------------------------------------------------------------------
		//	2a. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		if ( Do_Write_Data(dev, (unsigned char)(index&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN)==0 )
		{
			return -1;
		}
	}

	if (dev->address_mode >= 2)
	{
		// --------------------------------------------------------------------
		//	2b. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		if ( Do_Write_Data(dev, (unsigned char)((index>>8)&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN)==0 )
		{
			return -1;
		}
	}

	// --------------------------------------------------------------------
	// 	Combined format: change to read direction
	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// 	3. write slave address(RW bit high) to start master receive 
	// --------------------------------------------------------------------
	if ( Do_Write_RWAddr(dev, (unsigned char)raddr, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START) == 0 )
	{
		#if Debug_IIC_Scan_001
		prints("Debug_IIC_Scan_001 I2C_CombinedFormatTest() 001\n ");
		#endif 
		return -1;
	}

	// --------------------------------------------------------------------
	// 	4. more data to read
	// --------------------------------------------------------------------
	for (i = 0; i < num - 1; i++)
	{
		readed_value = Do_Read(dev, I2C_CR_ENABLE|I2C_CR_TB_EN);
		if (readed_value == -1)
		{
			#if Debug_IIC_Scan_001
			prints("Debug_IIC_Scan_001 I2C_MasterReceive() 002\n ");
			#endif 
			return -1;
		}
		buf[i] = (unsigned char)readed_value;
	}

	// --------------------------------------------------------------------
	// 	5. read last data 
	// --------------------------------------------------------------------
	readed_value = Do_Read_Last(dev, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_NACK);
	if (readed_value == -1)
	{
		#if Debug_IIC_Scan_001
		prints("Debug_IIC_Scan_001 I2C_CombinedFormatTest() 003\n ");
		#endif 
		return -1;
	}

	buf[i] = (unsigned char)readed_value;

	// --------------------------------------------------------------------
	// 	Combined format: change to write direction
	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// 	6. write slave address to select device
	// --------------------------------------------------------------------
	if ( Do_Write_RWAddr(dev, (unsigned char)waddr, I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_START)==0 )
	{
		return -1;
	}

	index += num;
	if (dev->address_mode >= 1)
	{
		// --------------------------------------------------------------------
		//	7a. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		if ( Do_Write_Data(dev, (unsigned char)(index&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN)==0 )
		{
			return -1;
		}
	}

	if (dev->address_mode >= 2)
	{
		// --------------------------------------------------------------------
		//	7b. write index to device (要從那個 index 開始寫)
		// --------------------------------------------------------------------
		if ( Do_Write_Data(dev, (unsigned char)((index>>8)&0xff), I2C_CR_ENABLE|I2C_CR_TB_EN)==0 )
		{
			return -1;
		}
	}

	// --------------------------------------------------------------------
	// 	8. more data to write
	// --------------------------------------------------------------------
	for (i=0; i < num-1; i++)
	{
		if ( Do_Write_Data(dev, buf[i], I2C_CR_ENABLE|I2C_CR_TB_EN) == 0)
		{
			return -1;
		}
	}

	// --------------------------------------------------------------------
	// 	9. write last data
	// --------------------------------------------------------------------
	if (Do_Write_Data(dev, buf[i], I2C_CR_ENABLE|I2C_CR_TB_EN|I2C_CR_STOP)==0)
	{
		return -1;
	}

	fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_DISABLE);
	return 0;
}

// --------------------------------------------------------------------
//	return
//		-1 ==> failure
//		1 ==> success
// --------------------------------------------------------------------
int I2C_Write (I2C_Dev *dev, unsigned int address, unsigned int index, unsigned char value)
{
	if (I2C_MasterTransfer(dev, &value, address, index, 1)!=1)
	{
		prints("I2C write: %x %x %x failure\n", address, index, value);
		return -1;
	}
	return 1;
}

void I2C_SetAddressMode(I2C_Dev *dev, int mode)
{
	dev->address_mode = mode;
}

void I2C_Init(I2C_Dev *dev, unsigned long io_base, uint32_t gsr, uint32_t tsr, uint32_t ls, uint32_t hs)
{
	u32 ar;
	dev->io_base = io_base;
	printf("dev->iobase = %lx\n",dev->io_base);
	dev->irq = -1;
	dev->address_mode = ADDR_MODE_8BIT;

	fLib_FTIIC010_WriteReg(dev, I2C_CR, I2C_CR_I2C_RST);

	//setting PCLK clock  cycles
	fLib_FTIIC010_WriteReg(dev, I2C_TGSR, gsr|tsr);

	I2C_SetSCLout(dev, ls, hs);
}

void I2C_Master_ISR(I2C_Dev *dev)
{
	u32 bstmr, threshold_cnt, total_cnt, i, dr;
	unsigned long status_tmp=0;

	status_tmp = fLib_FTIIC010_ReadReg(dev, I2C_SR);
	if ((status_tmp&I2C_SR_RW)==1) // RX
	{
		bstmr = fLib_FTIIC010_ReadReg(dev, I2C_BSTMR);
		threshold_cnt = bstmr&0x3f;
		total_cnt = (bstmr>>BSTTDC)&0x3f;
		if ((status_tmp&I2C_SR_BSTTHOD)&&!(status_tmp&I2C_SR_TD))
		{
			for (i=0; i<threshold_cnt; i++)
			{
				dr = (unsigned char)fLib_FTIIC010_ReadReg(dev, I2C_DR);
				gData_buf[i] = dr;
				//prints("dr[%u] = %u\n", i, dr);
			}
		}
		else if ((status_tmp&I2C_SR_TD)&&!(status_tmp&I2C_SR_BSTTHOD))
		{
			for (i=threshold_cnt; i<total_cnt; i++)
			{
				dr = (unsigned char)fLib_FTIIC010_ReadReg(dev, I2C_DR);
				gData_buf[i] = dr;
				//prints("dr[%u] = %u\n", i, dr);
			}
		}
		else if ((status_tmp&I2C_SR_BSTTHOD)&&(status_tmp&I2C_SR_TD))
		{
			for (i=0; i<total_cnt; i++)
			{
				dr = (unsigned char)fLib_FTIIC010_ReadReg(dev, I2C_DR);
				gData_buf[i] = dr;
				//prints("dr[%u] = %u\n", i, dr);
			}
		}
		else
			prints("I2C_Master_ISR: ERROR!!!\n");
	}

	fLib_FTIIC010_WriteReg(dev, I2C_SR, status_tmp); // w1c
	dev->status = status_tmp;
}

void I2C_RequestIRQ(I2C_Dev *dev, int isFiq, int nIRQ)		/// int intMode, int intLevel)
{
	dev->isFiq = isFiq;
	dev->irq = nIRQ;

	irq_set_type(nIRQ, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler(nIRQ, I2C_Master_ISR, &dev[0]);
	irq_set_enable(nIRQ);
}

void I2C_FreeIRQ(I2C_Dev *dev)
{
/*
	fLib_CloseInt(dev->isFiq, dev->irq);
	dev->irq = -1;
*/
}
