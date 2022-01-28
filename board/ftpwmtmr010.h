/*
 * Timer
 */
#ifndef __LIBGLOSS_ARM_FTPWMTMR_H
#define __LIBGLOSS_ARM_FTPWMTMR_H

#ifndef	__ASSEMBLER__
#include <stdint.h>

typedef struct {
	volatile uint32_t	interrupt_state;/* 0x00 */
	volatile uint32_t	reserve0;       /* 0x04 */
	volatile uint32_t	reserve1;       /* 0x08 */
	volatile uint32_t	reserve2;       /* 0x0c */
	volatile uint32_t	timer1_cr;	    /* 0x10 */
	volatile uint32_t	timer1_load;	/* 0x14 */
	volatile uint32_t	timer1_compare;	/* 0x18 */
	volatile uint32_t	timer1_counter;	/* 0x1C */
	volatile uint32_t	timer2_cr;	    /* 0x20 */
	volatile uint32_t	timer2_load;	/* 0x24 */
	volatile uint32_t	timer2_compare;	/* 0x28 */
	volatile uint32_t	timer2_counter;	/* 0x2C */
	volatile uint32_t	timer3_cr;	    /* 0x30 */
	volatile uint32_t	timer3_load;	/* 0x34 */
	volatile uint32_t	timer3_compare;	/* 0x38 */
	volatile uint32_t	timer3_counter;	/* 0x3C */
	volatile uint32_t	timer4_cr;	    /* 0x40 */
	volatile uint32_t	timer4_load;	/* 0x44 */
	volatile uint32_t	timer4_compare;	/* 0x48 */
	volatile uint32_t	timer4_counter;	/* 0x4C */

#if 0
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
#endif
} ftpwmtmr_t;

#endif	/* __ASSEMBLER__ */

	/* for Timer Control Register */
#define	FTPWMTMR_CLKSRC	    (1 << 0)
#define	FTPWMTMR_START  	(1 << 1)
#define	FTPWMTMR_UPDATE	    (1 << 2)
#define	FTPWMTMR_OUTINV	    (1 << 3)
#define	FTPWMTMR_AUTOLOAD   (1 << 4)
#define	FTPWMTMR_INTEN	    (1 << 5)
#define	FTPWMTMR_INTMODE    (1 << 6)
#define	FTPWMTMR_DMAEN      (1 << 7)
#define	FTPWMTMR_PWMEN      (1 << 8)

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
