#ifndef CPU_H_
#define CPU_H_


//#ifndef ASM

#ifdef CONFIG_ARMV5TE
static inline void __enable_irq(){ 
	__asm volatile(
		"MRS r0, CPSR" "\n\t"
		"BIC r0, r0, #0x80" "\n\t"
		"MSR CPSR_c, r0" 
	);
}
static inline void __isb()          {asm volatile ("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0));}
static inline void __dsb()          {asm volatile ("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0));}
static inline void __dmb()          {asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0));}
//#define cpu_dsb cp15_dsb 
//#define cpu_isb cp15_isb 
#define CPU_Idle                        cp15_idle


#define clean_dcache_all                cp15_clean_dcache_all
#define flush_dcache_all                cp15_clean_invalidate_dcache_all
//no implement "invalidate", use clean_invalidate instead of it
#define invalidate_dcache_all           cp15_clean_invalidate_dcache_all
#define invalidate_icache_dcache_all    cp15_invalidate_icache_dcache_all

//static inline void EnableIRQ	{asm volatile("MRS %0, CPSR" "\n\t" "BIC %0, %0, #0x80" "\n\t"	"MSR CPSR_c, %0" ::"r"(0));}

#endif

#ifdef CONFIG_ARMV7A
static inline void __enable_irq()               { __asm volatile ("cpsie i"); }
static inline void __disable_irq()              { __asm volatile ("cpsid i"); }

static inline void __enable_fault_irq()         { __asm volatile ("cpsie f"); }
static inline void __disable_fault_irq()        { __asm volatile ("cpsid f"); }

static inline void __nop()                      { __asm volatile ("nop"); }
static inline void __wfi()                      { __asm volatile ("wfi"); }
static inline void __wfe()                      { __asm volatile ("wfe"); }
static inline void __sev()                      { __asm volatile ("sev"); }
static inline void __isb()                      { __asm volatile ("isb"); }
static inline void __dsb()                      { __asm volatile ("dsb"); }
static inline void __dmb()                      { __asm volatile ("dmb"); }
static inline void __clrex()                    { __asm volatile ("clrex"); }
//#define cpu_dsb cp15_dsb 
//#define cpu_isb cp15_isb 
#define CPU_Idle __wfi

#define clean_dcache_all                flush_dcache_all

#endif

#include "types.h"

//no implement "clean", use clean_invalidate instead of it

//ycmo: copy from bootcode2
typedef struct _mmap
{
	unsigned int    va;
	unsigned int    pa;

	unsigned int    sz   : 16;  // MB
	unsigned int    cb   : 2;   // Cacheable(BIT1) | Bufferable(BIT0)
	unsigned int    rsvd : 14;
} mmap_t;


extern void CPU_Idle(void);
extern void CPU_ChangeETTBBase(int ptb_addr);
extern void CPU_AllCache_Enable();
extern void CPU_AllCache_Disable();
extern UINT32 CPUReadVendorID(void);
extern UINT32 CPUReadArchID(void);
extern UINT32 CPUReadPartNum(void);
extern UINT32 CPUReadVersion(void);
extern UINT32 CPUGetICacheLineLength(void);
extern UINT32 CPUGetDCacheLineLength(void);
extern void fLib_DisableDCache();
/* ycmo 130320: 
 * Per u-boot and fa626 data sheet :  flush = clean + invalidate
 * if dcache on, use invaliate may cause problem in on stack memory
 * for safe, please use flush instead of invalidate */
extern void flush_dcache_all(void); 

extern void CPU_Idle();

extern void CPUDelay(float sec);
//#endif



#endif /*CPU_H_*/
