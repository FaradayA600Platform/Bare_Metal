/**
 * @brief   ARM ACLE and utility 
 *
 * @author  brliu@faraday-tech.com
 */

#ifndef __ARM_ACLE
#define __ARM_ACLE

#include "cp15.h"

#if _ARM_ARCH == 7
// cpsie/cpsid only support V6 and above
static inline void __enable_irq()               { __asm volatile ("cpsie i"); }
static inline void __disable_irq()              { __asm volatile ("cpsid i"); }

static inline void __enable_fiq()         { __asm volatile ("cpsie f"); }
static inline void __disable_fiq()        { __asm volatile ("cpsid f"); }

static inline void __nop()                      { __asm volatile ("nop"); }
#define __wfi()   __asm__ __volatile__ ("wfi" : : : "memory")
#define __wfe()   __asm__ __volatile__ ("wfe" : : : "memory")
#define __sev()   __asm__ __volatile__ ("sev" : : : "memory")
#define __isb() __asm__ __volatile__ ("isb" : : : "memory")
#define __dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#define __dmb() __asm__ __volatile__ ("dmb" : : : "memory")
static inline void __clrex()                    { __asm volatile ("clrex"); }


#endif // _ARM_ARCH == 7

#if _ARM_ARCH == 5
#define __enable_irq()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ local_irq_enable\n"	\
"	bic	%0, %0, #128\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})


#define __disable_irq()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ local_irq_disable\n"	\
"	orr	%0, %0, #128\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})


#define __enable_fiq()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ stf\n"		\
"	bic	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})


#define __disable_fiq()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ clf\n"		\
"	orr	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

static inline void __wfi()          {WRITE_CP32(0, LPWFI);}
static inline void __isb()	        {asm volatile ("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0));}
static inline void __dsb()          {asm volatile ("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0));}
static inline void __dmb()	        {asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0));}
#endif  // _ARM_ARCH == 5
#ifndef __ASSEMBLY__
#if 0
#if __LINUX_ARM_ARCH__ >= 4
#define vectors_high()	(cr_alignment & CR_V)
#else
#define vectors_high()	(0)
#endif

extern unsigned long cr_no_alignment;	/* defined in entry-armv.S */
extern unsigned long cr_alignment;	/* defined in entry-armv.S */
#endif

static inline uint32_t log_2_n_round_up(uint32_t n)
{
    int32_t log2n = -1;
    uint32_t temp = n;

    while (temp) {
        log2n++;
        temp >>= 1;
    }

    if (n & (n - 1))
        return log2n + 1; /* not power of 2 - round up */
    else
        return log2n; /* power of 2 */
}

static inline unsigned int get_cr(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR"
	  : : "r" (val) : "cc");
	__isb();
}


static inline void ttbSet(uint32_t ttb)
{
    ttb &= 0xffffc000;
	/* Copy the page table address to cp15 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (ttb) : "memory");
	/* Set the access control to all-supervisor */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (~0));
}
static inline void 
dcache_clean_inval_range(uint32_t start,
					uint32_t stop, uint32_t line_len)
{
	uint32_t mva;

	/* Align start to cache line boundary */
	start &= ~(line_len - 1);
	for (mva = start; mva < stop; mva = mva + line_len) {
		/* DCCIMVAC - Clean & Invalidate data cache by MVA to PoC */
		asm volatile ("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
	}
    __dsb();
}
#ifndef CONFIG_SMP
extern void adjust_cr(unsigned long mask, unsigned long set);
#endif

#define CPACC_FULL(n)		(3 << (n * 2))
#define CPACC_SVC(n)		(1 << (n * 2))
#define CPACC_DISABLE(n)	(0 << (n * 2))

static inline unsigned int get_copro_access(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c0, 2 @ get copro access"
	  : "=r" (val) : : "cc");
	return val;
}

static inline void set_copro_access(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 2 @ set copro access"
	  : : "r" (val) : "cc");
	__isb();
}

#endif
static inline void
L1Dcache_InvalidateLine(uint32_t addr)
{
    WRITE_CP32(0, CSSELR);
    WRITE_CP32((addr & (~0x1F)), DCIMVAC);
    __dsb();
}

static inline void
L1Dcache_FlushLine(uint32_t addr)
{
    WRITE_CP32(0, CSSELR);
    WRITE_CP32((addr & (~0x1F)), DCCIMVA);
    __dsb();
}
#endif // __ARM_ACLE
