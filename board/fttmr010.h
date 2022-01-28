/*
 * Timer
 */
#ifndef __LIBGLOSS_ARM_FTTMR_H
#define __LIBGLOSS_ARM_FTTMR_H

#ifndef	__ASSEMBLER__
#include <stdint.h>

typedef struct {
	volatile uint32_t	timer1_counter;	/* 0x00 */
	volatile uint32_t	timer1_load;	/* 0x04 */
	volatile uint32_t	timer1_match1;	/* 0x08 */
	volatile uint32_t	timer1_match2;	/* 0x0C */
	volatile uint32_t	timer2_counter;	/* 0x10 */
	volatile uint32_t	timer2_load;	/* 0x14 */
	volatile uint32_t	timer2_match1;	/* 0x18 */
	volatile uint32_t	timer2_match2;	/* 0x1C */
	volatile uint32_t	timer3_counter;	/* 0x20 */
	volatile uint32_t	timer3_load;	/* 0x24 */
	volatile uint32_t	timer3_match1;	/* 0x28 */
	volatile uint32_t	timer3_match2;	/* 0x2C */
	volatile uint32_t	cr;		/* 0x30 */
	volatile uint32_t	interrupt_state;/* 0x34 */
	volatile uint32_t	interrupt_mask;	/* 0x38 */
} fttmr_t;

#endif	/* __ASSEMBLER__ */

	/* for Timer Control Register */
#define	FTTMR_TM1_ENABLE	(1 << 0)
#define	FTTMR_TM1_CLOCK		(1 << 1)
#define	FTTMR_TM1_OFENABLE	(1 << 2)
#define	FTTMR_TM2_ENABLE	(1 << 3)
#define	FTTMR_TM2_CLOCK		(1 << 4)
#define	FTTMR_TM2_OFENABLE	(1 << 5)
#define	FTTMR_TM3_ENABLE	(1 << 6)
#define	FTTMR_TM3_CLOCK		(1 << 7)
#define	FTTMR_TM3_OFENABLE	(1 << 8)
#define	FTTMR_TM1_UPDOWN	(1 << 9)
#define	FTTMR_TM2_UPDOWN	(1 << 10)
#define	FTTMR_TM3_UPDOWN	(1 << 11)

	/* for Timer Interrupt State & Mask Registers */
#define	FTTMR_TM1_MATCH1	(1 << 0)
#define	FTTMR_TM1_MATCH2	(1 << 1)
#define	FTTMR_TM1_OVERFLOW	(1 << 2)
#define	FTTMR_TM2_MATCH1	(1 << 3)
#define	FTTMR_TM2_MATCH2	(1 << 4)
#define	FTTMR_TM2_OVERFLOW	(1 << 5)
#define	FTTMR_TM3_MATCH1	(1 << 6)
#define	FTTMR_TM3_MATCH2	(1 << 7)
#define	FTTMR_TM3_OVERFLOW	(1 << 8)


#endif	/* __LIBGLOSS_ARM_FTTMR_H */
