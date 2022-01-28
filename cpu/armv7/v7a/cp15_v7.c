#include "cpu.h"


/*
 * cpu id register
 */
__inline int
cp15_read_processor_id(void)
{
	int	value;

	__asm volatile(
		"mrc	p15, 0, %[value], c0, c0, 5" :[value]"=r"(value):
	);

	return value & 0x3;
}

/*
 * system control register (SCTLR) c1
 */
#define	P15_C1_MMU	(1 << 0)	/* MMU enable */
#define	P15_C1_ALIGN	(1 << 1)	/* alignment check enable */
#define	P15_C1_DCACHE	(1 << 2)	/* data cache enable */
#define	P15_C1_BIGEND	(1 << 7)	/* big endian */
#define P15_C1_BRNPRED	(1 << 11)	/* branch prediction enable */
#define	P15_C1_ICACHE	(1 << 12)	/* instruction cache enable */
#define	P15_C1_HIVEC	(1 << 13)	/* high exception vector */



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

//ycmo add : wfi ?
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
*/


void invalidate_icache_dcache_all(void)
{
    invalidate_dcache_all();
    invalidate_icache_all();

}


/*
 * ------------------------------------------------------------
 * CP15 MMU related
 * ------------------------------------------------------------
 */
__inline void
cp15_set_tlb_base(void *addr)
{

	// Cortex-A9 supports two translation tables
        // Configure translation table base (TTB) control register cp15,c2
        // to a value of all zeros, indicates we are using TTB register 0.
#if 0
	__asm volatile(
		"mcr	p15, 0, %[zero], c2, c0, 2" "\n\t"
		"mcr	p15, 0, %[addr], c2, c0, 0" 	// write the address of our page table base to TTB register 0
        : :[zero]"r"(zero), [addr]"r"(addr)
	);
#endif 
	__asm volatile(
		"mcr	p15, 0, %[addr], c2, c0, 0" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" 
        : :[addr]"r"(addr)
	);

}
__inline void
cp15_set_domain_access_control(unsigned char domain, unsigned char access)
{
	unsigned int	tmp;

#if 0
	tmp = 0x55555555;	/* Set all Domains to Client */
	__asm volatile(
		"mcr	p15, 0, %[tmp], c3, c0, 0" ::[tmp]"r"(tmp)
	);

#else
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
#endif
}

#if 0

/*
 * ------------------------------------------------------------
 * CP15 SMP related
 * ------------------------------------------------------------
 */
static __inline void
cp15_enter_smp_mode(void)
{
	unsigned int tmp;

	cp15_clean_invalidate_dcache_all();

	/* turn on SMP bit (bit 6) and FW bit in Auxiliary Control Register */
	__asm {
		mrc	p15, 0, tmp, c1, c0, 1
		orr	tmp, tmp, #0x41
		mcr	p15, 0, tmp, c1, c0, 1
	}
}

static __inline unsigned int
cp15_is_add_swp_instr(void)
{
	unsigned int	value;
	/* Read ID_ISAR0 */
	__asm {
		mrc	p15, 0, value, c0, c2, 0
	}

	return (value & 0x1);
}
#else

__inline unsigned int
cp15_read_ACTLR(void)
{
	unsigned int	value;

	__asm volatile(
		"mrc	p15, 0, %[value], c1, c0, 1" :[value]"=r"(value):
	);
	
	return value;
}

__inline void
cp15_set_SMP(bool enabled)
{
	unsigned int	tmp;

	tmp = cp15_read_ACTLR();
	if(enabled)
		tmp |= 0x00000040/*BIT6*/;
	else
		tmp &= (~0x00000040/*BIT6*/);
	cp15_write_ACTLR(tmp);
}

#endif
