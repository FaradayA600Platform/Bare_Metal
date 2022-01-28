#ifndef __PMU_H
#define __PMU_H

/*
 * Performance Monitor
 * 
 * You can access PMU registers through CP15 or APB
 */

/* Performance Monitor Control Register (PMCR) */ 
#define PMU_ENABLE		(1 << 0)
#define PMU_PMN_RESET		(1 << 1)	/* reset event count register */
#define PMU_CCNT_RESET		(1 << 2)	/* reset cycle count register */
#define PMU_CCNT_DIV		(1 << 3)
#define PMU_EXPORT		(1 << 4)
#define PMU_DISABLE_CCNT	(1 << 5)
#define PMU_EVENT_NUM(reg)	(((reg) >> 11) & 0x1f)	/* event count number N */
#define PMU_IDCODE(reg)		(((reg) >> 16) & 0xff)

#define PMN_ENABLE(x)		(1 << (x))	/* Enable which event count */
#define CCNT_ENABLE		(1 << 31)	/* Enable cycle count*/


/* Event number */
#define PMU_EVENT_SW_INCR		0x0
#define PMU_EVENT_L1I_CACHE_MISS	0x1
#define PMU_EVENT_L1I_MICRO_TLB_MISS	0x2
#define PMU_EVENT_L1D_CACHE_MISS	0x3
#define PMU_EVENT_L1D_CACHE_ACCESS	0x4
#define PMU_EVENT_L1D_MICRO_TLB_MISS	0x5
#define PMU_EVENT_DATA_READ		0x6
#define PMU_EVENT_DATA_WRITE		0x7
#define PMU_EVENT_EXC_TAKEN		0x9
#define PMU_EVENT_EXC_RETURN		0xa
#define PMU_EVENT_CID_WRITE		0xb
#define PMU_EVENT_PC_WRITE		0xc
#define PMU_EVENT_BR_IMMED		0xd
#define PMN_EVENT_UNALIGNED_LDST	0xf
#define PMN_EVENT_BR_MIS		0x10
#define PMN_EVENT_CPU_CYCLES		0x11
#define PMN_EVENT_BR_PRED		0x12
/* Cortex-A9 specific events number */
#define PMN_EVENT_JAVA_BYTECODE		0x40
#define PMN_EVENT_SW_JAVA_BYTECODE	0x41
#define PMN_EVENT_JAZELLE_BACK_BR	0x42
#define PMN_EVENT_COHER_LINEFILL_MISS	0x50	
#define PMN_EVENT_COHER_LINEFILL_HIT	0x51
#define PMN_EVENT_L1I_STALL_CYCLE	0x60
#define PMN_EVENT_L1D_STALL_CYCLE	0x61
#define PMN_EVENT_MAIN_TLB_MISS_CYCLE	0x62
#define PMN_EVENT_STREX_PASS		0x63
#define PMN_EVENT_STREX_FAIL		0x64
#define PMN_EVENT_DATA_EVIT		0x65
#define PMN_EVENT_ISS_NODIS		0x66
#define PMN_EVENT_ISS_EMPTY		0x67
#define PMN_EVENT_INST_COME_OUT		0x68
#define PMN_EVENT_LINEFILL_NUM		0x69
#define PMN_EVENT_PREF_LINEFILL_NUM	0x6A
#define PMN_EVENT_PREF_LINEFILL_HIT_NUM	0x6B
#define PMN_EVENT_PRED_FUNC		0x6E
#define PMN_EVENT_MAIN_EXEC_INST	0x70
#define PMN_EVENT_SEND_EXEC_INST	0x71
#define PMN_EVENT_LDST_INST		0x72
#define PMN_EVENT_FP_INST		0x73
#define PMN_EVENT_NEON_INST		0x74
#define PMN_EVENT_CPU_STALL_PLD		0x80
#define PMN_EVENT_CPU_STALL_WRITE	0x81
#define PMN_EVENT_CPU_STALL_I_MTLB_MISS	0x82
#define PMN_EVENT_CPU_STALL_D_MTLB_MISS	0x83
#define PMN_EVENT_CPU_STALL_ITLB_MISS	0x84
#define PMN_EVENT_CPU_STALL_DTLB_MISS	0x85
#define PMN_EVENT_CPU_STALL_DMB		0x86
#define PMN_EVENT_INT_CLK_EN		0x8A
#define PMN_EVENT_DATA_CLK_EN		0x8B
#define PMN_EVENT_NEON_SIMD_EN		0x8C
#define PMN_EVENT_ITLB_ALLOC		0x8D
#define PMN_EVENT_DTLB_ALLOC		0x8E
#define PMN_EVENT_ISB_INST		0x90
#define PMN_EVENT_DSB_INST		0x91
#define PMN_EVENT_DMB_INST		0x92
#define PMN_EVENT_EXT_IRQ		0x93
#define PMN_EVENT_PLE_CACHE_REQ_COMP	0xA0
#define PMN_EVENT_PLE_CACHE_REQ_SKIP	0xA1
#define PMN_EVENT_PLE_FIFO_FLUSH	0xA2
#define PMN_EVENT_PLE_REQ_COMP		0xA3
#define PMN_EVENT_PLE_FIFO_OVERFLOW	0xA4
#define PMN_EVENT_PLE_REQ_PROG		0xA5


/*
 * Through APB, using the offset when PADDRDBG[12]=1 
 */
#define PMU_PMXEVCNTR		0x000	/* Event Count Register, 0x0~0x14 */
#define PMU_PMCCNTR		0x07C	/* Cycle Count Register */
#define PMU_PMXEVTYPER		0x400	/* Event Type Selection Register, 0x400~0x414 */
#define PMU_PMCNTENSET		0xC00	/* Count Enable Set Register */
#define PMU_PMCNTENCLR		0xC20	/* Count Enable Clear Register */
#define PMU_PMINTENSET		0xC40	/* Interrupt Enable Set Register */
#define PMU_PMINTENCLR		0xC60	/* Interrupt Enable Clear Register */
#define PMU_PMOVSR		0xC80	/* Overflow Flag Status Register */
#define PMU_PMSWINC		0xCA0	/* Software Increment Register */
#define PMU_PMCR		0xE04	/* Performance Monitor Control Register */
#define PMU_PMUSERENR		0xE08	/* User Enable Register */
#define PMU_PMSELR		0xE08	/* Event Counter Selection Register */
#define PMU_MANAGMENT		0xF00	/* PMU Management Register */

/* Performance Monitor Control Register (PMCR) */
static __inline unsigned int
PMU_read_control_reg(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c9, c12, 0
	}

	return value;
}

static __inline void
PMU_write_control_reg(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c9, c12, 0
	}
}

/* Cycle Count Register (PMCCNTR) */
static __inline unsigned int
PMU_read_cycle_counter(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c9, c13, 0
	}

	return value;
}


/* 
 * write Count Enable Set Register (PMCNTENSET)
 *
 * To enable cycle count register(PMCCNTR) or event counter(PMNx)
 * value = bit 0~N-1: event counter; bit31: cycle count
 */
static __inline void
PMU_set_counter_enable(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c9, c12, 1
	}
}
static __inline unsigned int
PMU_read_counter_enable(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c9, c12, 1
	}

	return value;
}

/* 
 * Event Counter Selection Register (PMSELR) 
 *
 * select an event counter PMNx, or the cycle count CCNT
 * sel = bit 0~4; 0x1f: cycle count
 */
static __inline void
PMU_select_which_counter(unsigned int sel)
{
	__asm {
		mcr	p15, 0, sel, c9, c12, 5
	}
}

/* 
 * Event Type Selection Register (PMXEVTYPER) 
 *
 * choost the event to count
 */
static __inline void
PMU_select_event_type(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c9, c13, 1
	}
}

/* Event Count Register (PMXEVCNTR) */
static __inline unsigned int
PMU_read_event_counter(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c9, c13, 2
	}

	return value;
}

static __inline void
PMU_write_event_counter(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c9, c13, 2
	}
}

#endif /* __PMU_H */
