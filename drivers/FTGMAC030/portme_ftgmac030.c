/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2020-July    BingYao           Example for port to relevant SoC or Board.
 * -------------------------------------------------------------------------
 */
#include "portme_ftgmac030.h"

extern void timer_callback(void * TimerInstance);

//Example of using Xilinx BSP for Timer controller
//XScuTimer TimerInstance;

//Xil_EndianSwap32 implementation
uint32_t ftgmac030_endian_swap32(uint32_t data)
{
	uint16_t LoWord;
	uint16_t HiWord;

	/* get each of the half words from the 32 bit word */

	LoWord = (uint16_t) (data & 0x0000FFFFU);
	HiWord = (uint16_t) ((data & 0xFFFF0000U) >> 16U);

	/* byte swap each of the 16 bit half words */

	LoWord = (((LoWord & 0xFF00U) >> 8U) | ((LoWord & 0x00FFU) << 8U));
	HiWord = (((HiWord & 0xFF00U) >> 8U) | ((HiWord & 0x00FFU) << 8U));

	/* swap the half words before returning the value */

	return ((((uint32_t)LoWord) << (uint32_t)16U) | (uint32_t)HiWord);
}

//Xil_EndianSwap16 implementation
uint16_t ftgmac030_endian_swap16(uint16_t data)
{
	return (uint16_t) (((data & 0xFF00U) >> 8U) | ((data & 0x00FFU) << 8U));
}

void ftgmac030_enable_timer_interrupt(void)
{
	//Example of Xilinx BSP for user reference
	//XScuTimer_EnableInterrupt(&TimerInstance);
}

void ftgmac030_start_timer(void)
{
	//Example of Xilinx BSP for user reference
	//XScuTimer_Start(&TimerInstance);
}

void platform_setup_timer(void)
{
	//User need Timer controller.
	//Example of Xilinx BSP for user reference
#if 0
	int Status = XST_SUCCESS;
	XScuTimer_Config *ConfigPtr;
	int TimerLoadValue = 0;

	ConfigPtr = XScuTimer_LookupConfig(TIMER_DEVICE_ID);
	Status = XScuTimer_CfgInitialize(&TimerInstance, ConfigPtr,
			ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {

		xil_printf("In %s: Scutimer Cfg initialization failed...\r\n",
		__func__);
		return;
	}

	Status = XScuTimer_SelfTest(&TimerInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("In %s: Scutimer Self test failed...\r\n",
		__func__);
		return;

	}

	XScuTimer_EnableAutoReload(&TimerInstance);
	/*
	 * Set for 1 seconds timeout.
	 */
	TimerLoadValue = COUNTS_PER_SECOND;

	XScuTimer_LoadTimer(&TimerInstance, TimerLoadValue);
	return;
#endif
}

void ftgmac030_enable_interrupts()
{

}

void ftgmac030_disable_interrupts()
{

}

void platform_setup_interrupts(uint32_t irq_no, void *handler, void *data)
{

}

uint64_t ftgmac030_get_time(void)
{
	uint64_t t = 0;

	//Implement get ticks per second function.

	return (uint64_t) t;
}

uint64_t ftgmac030_get_ms(void)
{
	//Implement get milliseconds per second function.
	return (ftgmac030_get_time() / COUNTS_PER_MILLI_SECOND);
}

void ftgmac030_timer_udelay(uint32_t usecs)
{

}

void ftgmac030_enable_caches()
{

}

void ftgmac030_disable_caches()
{

}

void ftgmac030_dcache_flushrange(uintptr_t buf, uint32_t len)
{			

}

void ftgmac030_cleanup_platform(void)
{

}

void ftgmac030_init_platform(void *handler, void *data)
{
	platform_setup_interrupts(IRQ_FTGMAC030, handler, data);

	platform_setup_timer();
}
