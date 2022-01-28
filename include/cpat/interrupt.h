#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include <stdint.h>


#ifndef REG32
#define REG32(addr) *(volatile uint32_t *)(addr)
#endif

#define TRIG_MODE_LEVEL 0
#define TRIG_MODE_EDGE  1

#define INT_LEVEL_LOW       1
#define INT_LEVEL_HIGH      0
#define INT_EDGE_RISING     1
#define INT_EDGE_FALLING    0

/* TODO: FIX CA9 cpu id */
#define INTC030_CPU_ID_CA9					0x600
#define INTC030_CPU_ID_FA626				0x300
#define INTC030_CPU_ID_EXT					0x600


#define INTC_NUM_SPI_IRQ		48//478	//shared peripheral interrupts


/*****************************************************************************************
 *          FTCINTC030
 *****************************************************************************************/
#if INTCTRL == 1 //
#define INTC_NUM_PPI_IRQ		0	//private peripheral interrupts
#define INTC_NUM_SGI_IRQ		0	//softwar generation interrupts

#define INTC030_PPI_ENABLE		0
/*	the interval of register offset between current 32irq set and next 32irq set is 0x20.
*	ex. address of irq[0:31] source register is 0x0
*	                    irq[32:63] is 0x20
*/
#define INTC030_SET_INTERVAL				0x20
#define INTC030_SET_INTERVAL_GAP			0x20
													
/* Device Registers */
/*** SPI ***/
#define INTC030_SPI_SOURCE_OFFSET			0x0
#define INTC030_SPI_ENABLE_OFFSET			0x4
#define INTC030_SPI_CLEAR_OFFSET			0x8
#define INTC030_SPI_TRIGGER_MODE_OFFSET		0xC
#define INTC030_SPI_TRIGGER_LEVEL_OFFSET	0x10
#define INTC030_SPI_PENDING_OFFSET			0x14
#define INTC030_SPI_ACTIVE_OFFSET			0x600
/*** PPI_SGI ***/
#define INTC030_PPI_SOURCE_OFFSET			0x640
#define INTC030_PPI_ENABLE_OFFSET			0x644
#define INTC030_PPI_CLEAR_OFFSET			0x648
#define INTC030_PPI_TRIGGER_MODE_OFFSET		0x64C
#define INTC030_PPI_TRIGGER_LEVEL_OFFSET	0x650
#define INTC030_PPI_PENDING_OFFSET			0x654
#define INTC030_PPI_SGI_ACTIVE_OFFSET		0x65C

#define INTC030_ACKNOWLEDGE_OFFSET          0x68c
#define INTC030_SPI_TARGET_OFFSET			0x420
#define INTC030_PRIORITY_MASK				0x684
#define INTC030_BINARY_POINT				0x688
#define INTC030_EOI_OFFSET			        0x690
#define INTC030_INT_CONTROL					0x69C
#define INTC030_MATCH_CPU0                  0x6a0



/* interrupt type */
enum {
	INTC030_INT_SPI = 0,
	INTC030_INT_PPI,
	INTC030_INT_SGI,
	INTC030_INT_LEGACY
};

enum {
	INTC030_INTCTRL_PRIORITY = 0,
	INTC030_INTCTRL_LEGACY,
	INTC030_INTCTRL_BACKWARD,
};
#endif //INTCTRL == FTCINTC030





typedef void (ISR_FUNC)(uint32_t);

extern void enable_irq (uint32_t id, uint32_t mode, uint32_t level, ISR_FUNC handler);
extern void disable_irq(uint32_t id);
extern uint32_t irq_ack(uint32_t irq);
extern uint32_t get_periph_base(void);

#endif
