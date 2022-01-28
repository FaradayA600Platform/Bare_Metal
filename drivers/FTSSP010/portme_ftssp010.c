/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  portme.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2015   	Bing-Yao	 Porting file for different platform        
 * -------------------------------------------------------------------------
 */
#include "portme_ftssp010.h"

#if defined(CONFIG_BSP_LITE)
int ftssp010_setup_interrupt(unsigned int irq, interrupt_handler_t handle_irq)
{
        irq_set_disable(irq);
        irq_set_type(irq, IRQ_TYPE_LEVEL_HIGH);
        irq_install_handler(irq, handle_irq, 0);
        irq_set_enable(irq);
        enable_interrupts();
}

void ftssp010_platform_init(void)
{
	disable_caches(0, NULL);
}

#elif defined(CONFIG_BSP_ZYNQ)
char kbhit(void)
{
	u32 RecievedByte;
	/* Wait until there is data */
	if(XUartPs_IsReceiveData(STDIN_BASEADDRESS)) {
		RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
		return (u8)RecievedByte;
	}
	RecievedByte = 0;
	/* Return the byte received */
	return (u8)RecievedByte;
}

static XScuGic IntcInstance; /* Instance of the Interrupt Controller */
/* This function sets up the interrupt system for the FTPCIE440 controller. */
int ftssp010_setup_interrupt(unsigned int irq, interrupt_handler_t handle_irq)
{
	XScuGic_Config *GicConfig;
	int result;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(GIC_DEVICE_ID);
	if (GicConfig == NULL) {
		return -1;
	}

	/* Initialize the SCU and GIC to enable the desired interrupt
	 * configuration.
	 */
	result = XScuGic_CfgInitialize(&IntcInstance, GicConfig,
		GicConfig->CpuBaseAddress);
	if (result != 0) {
		return -1;
	}

	// Setup interrupt for FTSSP010 master
	// 0x1: active High level sensitive; 0x3: rising-edge sensitive
	XScuGic_SetPriorityTriggerType(&IntcInstance, irq,
		0xA0, 0x3);

	/* Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device. */
	result = XScuGic_Connect(&IntcInstance, irq,
		(Xil_ExceptionHandler) handle_irq, 0);
	if (result != 0) {
		return result;
	}

	/* Enable the interrupt for the FTSSP010 controller device. */
	XScuGic_Enable(&IntcInstance, irq);


	/* Initialize the exception table and register the interrupt controller
	 * handler with the exception table. */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
		(Xil_ExceptionHandler)XScuGic_InterruptHandler, &IntcInstance);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return 0;
}

void ftssp010_platform_init(void)
{
	/* No buffer is used. Each I/O operation is written as soon as possible. */
	setvbuf (stdout, NULL, _IONBF, 0); //
}

#else

#define IRQ_TYPE_LEVEL_HIGH     LEVEL

char kbhit(void) { return fLib_kbhit(); }
int ftssp010_setup_interrupt(unsigned int irq, interrupt_handler_t handle_irq)
{
	irq_set_disable(irq);
	irq_set_type(irq, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler(irq, handle_irq, 0);
	irq_set_enable(irq);
	enable_interrupts();

	return 0;
}
void ftssp010_platform_init(void) {}
#endif
