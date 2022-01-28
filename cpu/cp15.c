
/*
 * id code register
 */
__inline unsigned int
cp15_read_id_code(void)
{
	unsigned int	value;


	__asm volatile(
		"mrc	p15, 0, %[value], c0, c0, 0" :[value] "=r" (value) :
	);

	return value;
}


/*
 * control register c1
 */
#define	P15_C1_MMU	(1 << 0)	/* MMU enable */
#define	P15_C1_ALIGN	(1 << 1)	/* alignment check enable */
#define	P15_C1_DCACHE	(1 << 2)	/* data cache enable */
#define	P15_C1_WB	(1 << 3)	/* write buffer enable */
#define	P15_C1_BIGEND	(1 << 7)	/* big endian */
#define	P15_C1_SYS	(1 << 8)	/* system protection bit */
#define	P15_C1_ROM	(1 << 9)	/* rom protection bit */
#define	P15_C1_ICACHE	(1 << 12)	/* instruction cache enable */
#define	P15_C1_HIVEC	(1 << 13)	/* high exception vector */
#define	P15_C1_XP	(1 << 23)	/* extended page table */

__inline unsigned int
cp15_read_c1(void)
{
	unsigned int	value;

	__asm volatile(
		"mrc	p15, 0, %[value], c1, c0, 0" :[value]"=r"(value):
	);
	
	return value;
}

#if 0 //move to cp15s.S
__inline void
cp15_write_c1(unsigned int value)
{
    unsigned int zero = 0;
	__asm volatile(
		"mcr	p15, 0, %[value], c1, c0, 0" /*"\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"mcr	p15, 0, %[zero], c7, c5, 4"*/ 
        ::[value]"r"(value), [zero]"r"(zero)
	);
	
}
#endif

__inline void
cp15_enable_icache(void)
{
	unsigned int	reg;

	reg = cp15_read_c1();		/* get control reg. */
	cp15_write_c1(reg | P15_C1_ICACHE);
}

__inline void
cp15_disable_icache(void)
{
	unsigned int	tmp;

	tmp = cp15_read_c1();
	cp15_write_c1(tmp & ~P15_C1_ICACHE);
}

__inline void
cp15_enable_dcache(void)
{
	unsigned int	tmp;

	tmp = cp15_read_c1();
	cp15_write_c1(tmp | P15_C1_DCACHE);
}

__inline void
cp15_disable_dcache(void)
{
	unsigned int	tmp;

	tmp = cp15_read_c1();
	cp15_write_c1(tmp & ~P15_C1_DCACHE);
}


/*
 * tlb operations
 */
__inline void
cp15_invalidate_utlb_all(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"mcr	p15, 0, %[zero], c8, c7, 0" : :[zero]"r"(zero)
	);
}

//ycmo add : 
__inline void
cp15_lock_down_icache_enable(void)
{
	unsigned int	value = 0x80000000;

	__asm volatile(
		"mcr 	p15, 0, %[value], c9, c0, 1"::[value]"r"(value)
	);
}

//ycmo add : 
__inline void
cp15_lock_down_icache_disable(void)
{
	unsigned int	value = 0x0;

	__asm volatile(
		"mcr 	p15, 0, %[value], c9, c0, 1"::[value]"r"(value)
	);
}

//ycmo add : 
__inline void
cp15_lock_down_icache_feed_icache(void)
{


	__asm volatile(
		"mov		r1, lr      @r1=line address of certain line" "\n\t"
		"mov		r5, #0x40" "\n\t"
		"mov		r6, #0" "\n"
		"1:" "\n\t"
		"MCR 	p15, 0, r1, c7, c13, 1	@Force line fetch from external memory" "\n\t"
		"ADD 	r1, r1, #16	@Add 4-word offset to address" "\n\t"
		"add		r6, r6, #1" "\n\t"
		"cmp		r6, r5" "\n\t"
		"bne		1b" "\n\t"
		: : : "r1","r5","r6"
	);
}
