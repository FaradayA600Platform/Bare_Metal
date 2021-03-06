
#ifndef __V5_CP15_H
#define __V5_CP15_H




/*
 * Performance Monitor
 */
#define P15_PMNC_ENABLE		(1 << 0)
#define P15_PMNC_PMN_RESET	(1 << 1)	/* reset both count register */
#define P15_PMNC_CCNT_RESET	(1 << 2)	/* reset cycle count register */
#define P15_PMNC_CCNT_DIV	(1 << 3)
#define P15_PMNC_CNT0_INT	(1 << 4)
#define P15_PMNC_CNT1_INT	(1 << 5)
#define P15_PMNC_CCNT_INT	(1 << 6)
#define P15_PMNC_CNT0_OF	(1 << 8)
#define P15_PMNC_CNT1_OF	(1 << 9)
#define P15_PMNC_CCNT_OF	(1 << 10)
#define P15_PMNC_CNT0_MASK	(0xff << 12)
#define P15_PMNC_CNT0(x)	(((x) & 0xff) << 12)
#define P15_PMNC_CNT1_MASK	(0xff << 20)
#define P15_PMNC_CNT1(x)	(((x) & 0xff) << 20)

#define P15_PMNEVENT_ICACHE_MISS	0x0
#define P15_PMNEVENT_STALL_INSTR	0x1
#define P15_PMNEVENT_STALL_DATA_DEP	0x2
#define P15_PMNEVENT_I_MICRO_TLB_MISS	0x3
#define P15_PMNEVENT_D_MICRO_TLB_MISS	0x4
#define P15_PMNEVENT_BRANCH		0x5
#define P15_PMNEVENT_BRANCH_NOT_PREDICT	0x6
#define P15_PMNEVENT_BRANCH_MISPREDICT	0x7
#define P15_PMNEVENT_INSTR_EXEC		0x8
#define P15_PMNEVENT_FOLD_INSTR_EXEC	0x9
#define P15_PMNEVENT_DCACHE_READ	0xa
#define P15_PMNEVENT_DCACHE_READ_MISS	0xb
#define P15_PMNEVENT_DCACHE_WRITE	0xc
#define P15_PMNEVENT_DCACHE_WRITE_MISS	0xd
#define P15_PMNEVENT_DCACHE_LINE_EVICT	0xe
#define P15_PMNEVENT_SW_CHANGE_PC	0xf
#define P15_PMNEVENT_MAIN_TLB_MISS	0x10
#define P15_PMNEVENT_EXT_MEM_REQUEST	0x11
#define P15_PMNEVENT_STALL_LSU_FULL	0x12
#define P15_PMNEVENT_BUFFER_DRAIN	0x13
#define P15_PMNEVENT_WRITE_MERGE	0x14
#define P15_PMNEVENT_LSU_SAFE_MODE	0x15
#define P15_PMNEVENT_CYCLE		0xFF


#endif /* __V5_CP15_H */

