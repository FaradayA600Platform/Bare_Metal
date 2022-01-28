#include "cpu.h"
#include "cp15_v5.h"


/*
 * cpu id register
 */
/*static*/ __inline int
cp15_read_processor_id(void)
{
	int	value;
	
	__asm volatile(
		"mrc	p15, 0, %[value], c0, c0, 5" :[value]"=r"(value):
	);

	return value & 0xf;
}


/* move to cpu.h
__inline void
cp15_isb(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c7, c5, 4" : :[zero]"r"(zero)
	);
}

__inline void
cp15_dsb(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c7, c10, 4" : :[zero]"r"(zero)
	);
}

__inline void
cp15_dmb(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c7, c10, 5" : :[zero]"r"(zero)
	);
}
*/

__inline void
cp15_idle(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c7, c0, 4" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t": :[zero]"r"(zero)
	);
}


__inline void
cp15_invalidate_icache_dcache_all(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c7, c7, 0" : :[zero]"r"(zero)
	);
}

__inline void
cp15_clean_dcache_all(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c7, c10, 0" : :[zero]"r"(zero)
	);
}

__inline void
cp15_clean_invalidate_dcache_all(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c7, c14, 0" : :[zero]"r"(zero)
	);
}


__inline void
cp15_set_tlb_base(void *addr)
{
	__asm volatile(
		"mcr	p15, 0, %[addr], c2, c0, 0" ::[addr]"r"(addr)
	);
	
}

__inline void
cp15_set_domain_access_control(unsigned char domain, unsigned char access)
{
	unsigned int	tmp;

	domain &= 0xf;	/* there are totally 16 domains */
	access &= 0x3;	/* 2 bits in each domain */

	__asm volatile(
		"mrc	p15, 0, %[tmp], c3, c0, 0" :[tmp]"=r"(tmp) :
	);
	
	domain *= 2;

	tmp &= ~(0x3 << domain);
	tmp |= access << domain;

	__asm volatile(
		"mcr	p15, 0, %[tmp], c3, c0, 0" ::[tmp]"r"(tmp)
	);
}

__inline void
cp15_enter_smp_mode(void)
{
	unsigned int tmp;

	cp15_clean_invalidate_dcache_all();

	/* turn on SMP bit */
	__asm volatile(
		"mrc	p15, 0, %0, c1, c0, 1" "\n\t"
		"orr	%0, %0, #0x20" "\n\t"
		"mcr	p15, 0, %0, c1, c0, 1" "\n\t" : :"r"(tmp)
	);
	
}

#if 0
/* ARM11 MPCore only */
static __inline void
cp15_enable_extended_page_table(void)
{
	unsigned int reg;

	reg = cp15_read_c1();	/* get control reg. */
	cp15_write_c1(reg | P15_C1_XP);
}

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

static __inline unsigned int
cp15_read_pmon_ctrl(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c15, c12, 0
	}

	return value;
}

static __inline void
cp15_write_pmon_ctrl(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c15, c12, 0
	}
}

static __inline unsigned int
cp15_read_pmon_ccnt(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c15, c12, 1
	}

	return value;
}

static __inline void
cp15_write_pmon_ccnt(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c15, c12, 1
	}
}

static __inline unsigned int
cp15_read_pmon_cnt0(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c15, c12, 2
	}

	return value;
}

static __inline void
cp15_write_pmon_cnt0(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c15, c12, 2
	}
}

static __inline unsigned int
cp15_read_pmon_cnt1(void)
{
	unsigned int	value;

	__asm {
		mrc	p15, 0, value, c15, c12, 3
	}

	return value;
}

static __inline void
cp15_write_pmon_cnt1(unsigned int value)
{
	__asm {
		mcr	p15, 0, value, c15, c12, 3
	}
}
#endif

/*
#endif 

#endif 
*/
