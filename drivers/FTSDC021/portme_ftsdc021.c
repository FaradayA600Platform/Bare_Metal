#include "portme_ftsdc021.h"

/************************NonOS Lite************************/
#if defined(CONFIG_BSP_LITE)

void disable_dcache() { disable_caches(0, NULL); }

/* Interrupt trigger type: Level High*/
void ftsdc021_setup_interrupt(uint32_t irq, intr_handler_t intr_handler)
{
	irq_set_disable(irq);
	irq_set_type(irq, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler(irq, intr_handler, 0);
	irq_set_enable(irq);
	enable_interrupts();
}

/*
 * Return clock in msec
 */
clock_t clock(void)
{
	return get_timer(0);
}

void reset_dll(void)
{
	clock_t t0;

	*((volatile uint32_t *)0xa8500008) |= 0x4010;
	//reset wrapper
	*((volatile uint32_t *)0xa8500000) |= 0x4000;
	t0 = clock(); //need some delay
	*((volatile uint32_t *)0xa8500000) &= 0xffffbfff;
	//reset DLL
	*((volatile uint32_t *)0xa8500000) |= 0x10;
	t0 = clock(); //need some delay
	*((volatile uint32_t *)0xa8500000) &= 0xffffffef;
	//wait reset ok
	while ((*((volatile uint32_t *)0xa8500004) & 0x8) != 0x8) {
		if ((clock() - t0 > 1000)) {
			printf(" ERR##: Tuning wait for DLL reset timeout.\r\n");
			break;
		}
	}
}

void wait_dll_lock(void)
{
	clock_t t0;

	// Check DLL if ready
	t0 = clock();
	while ((*((volatile uint32_t *)0xa8500004) & 0x4) == 0x4) {
		if ((clock() - t0 > 1000)) {
			printf(" ERR##: Tuning wait for DLL timeout.\r\n");
			break;
		}
	}
}

/************************  ZYNQ  ************************/
#elif defined(CONFIG_BSP_ZYNQ)

void disable_dcache() { Xil_DCacheDisable(); }

/*
 * Return clock in msec
 */
clock_t clock(void)
{
	XTime time_now;

	XTime_GetTime(&time_now);

	return time_now / CLOCKS_PER_MSEC;
}

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
/* This function sets up the interrupt system for the FTSDC021 controller. */
void ftsdc021_setup_interrupt(uint32_t irq, intr_handler_t handle_irq)
{
	XScuGic_Config *GicConfig;
	int result;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(GIC_DEVICE_ID);
	if (GicConfig == NULL) {
		return;
	}

	/* Initialize the SCU and GIC to enable the desired interrupt
	 * configuration.
	 */
	result = XScuGic_CfgInitialize(&IntcInstance, GicConfig,
		GicConfig->CpuBaseAddress);
	if (result != 0) {
		return;
	}

	// Setup interrupt for FTSDC021
	// 0x1: active High level sensitive; 0x3: rising-edge sensitive
	XScuGic_SetPriorityTriggerType(&IntcInstance, irq,
		0xA0, 0x3);

	/* Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device. */
	result = XScuGic_Connect(&IntcInstance, irq,
		(Xil_ExceptionHandler) handle_irq, 0);
	if (result != 0) {
		return;
	}

	/* Enable the interrupt for the FTSDC021 controller device. */
	XScuGic_Enable(&IntcInstance, irq);


	/* Initialize the exception table and register the interrupt controller
	 * handler with the exception table. */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
		(Xil_ExceptionHandler)XScuGic_InterruptHandler, &IntcInstance);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return;
}

void reset_dll(void) {}
void wait_dll_lock(void) {}

/************************  A600  ************************/
#elif defined(CONFIG_PLATFORM_A600)

#define IRQ_TYPE_LEVEL_HIGH     LEVEL

#define kbhit fLib_getchar

clock_t clock(void) {	return jiffies(); }
void reset_dll(void) {}
void wait_dll_lock(void) {}
/* Interrupt trigger type: Level High*/
void ftsdc021_setup_interrupt(uint32_t irq, intr_handler_t intr_handler)
{
	irq_set_disable(irq);
	irq_set_type(irq, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler(irq, intr_handler, 0);
	irq_set_enable(irq);
	enable_interrupts();
}
/************************Without BSP************************/
#else
char kbhit(void) { return getchar(); }
void reset_dll(void) {}
void wait_dll_lock(void) {}
void disable_dcache() {}
/* Interrupt trigger type: Level High*/
void ftsdc021_setup_interrupt(uint32_t irq, intr_handler_t intr_handler) {}

#ifdef FTSDC021_EDMA
void edma_init(void) {};
/*
 * Burst size is 128 * 4 Bytes. Because sector size is 512 bytes.
 */
int ftsdc021_Start_DMA(uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size,
                       uint32_t SrcWidth, uint32_t DstWidth, uint32_t SrcSize,
                       uint32_t SrcCtrl, uint32_t DstCtrl)
{
	return 0;
}

int ftsdc021_wait_DMA_done(uint32_t wait_t) { return 0; }
#endif
#endif
