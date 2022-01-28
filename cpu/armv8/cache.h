/***************************************************************************
* Copyright  Faraday Technology Corp 2015-2020.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:cache.h                                                             *
* Description: library API of L1/L2 cache                                  *
* Author: Mark_hs Fu-Tsung Hsu                                             *
****************************************************************************/
#ifndef __CACHE_H__
#define __CACHE_H__

void invalidate_icache_all(void);
void enable_icache(void);
void disable_icache(void);
void flush_dcache_level_setway(unsigned int cache_level);
void invalidate_dcache_level_setway(unsigned int cache_level);
void clean_invalidate_dcache(void);
void invalidate_dcache(void);
void flush_dcache_all(void);
void invalidate_dcache_all(void);
void enable_dcache(void);
void disable_dcache(void);
void enable_cache(void);



static inline void set_L2CTRL_EL1(unsigned int val )
{
    asm volatile("msr S3_1_C11_C0_2 , %0": : "r" (val) : "cc");
}

static inline unsigned int get_L2CTRL_EL1()
{
    unsigned val;
    asm volatile("mrs %0,S3_1_C11_C0_2" : "=r"(val) :: "cc");
    return val;
}

static inline void set_csselr(unsigned int level )
{
    unsigned csselr = level << 1 ;
    asm volatile("msr csselr_el1 , %0": : "r" (csselr) : "cc");
    asm volatile("isb");
}

static inline unsigned int get_ccsidr()
{
    unsigned ccsidr;
    asm volatile("mrs %0,ccsidr_el1" : "=r"(ccsidr) :: "cc");
    return ccsidr;
}

static inline unsigned int current_el(void)
{
	unsigned int el;
	asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
	return el >> 2;
}

static inline int get_sctlr()
{
	unsigned int el, val;

	el = current_el();
	if (el == 1)
		asm volatile("mrs %0, sctlr_el1" : "=r" (val) : : "cc");
	else if (el == 2)
		asm volatile("mrs %0, sctlr_el2" : "=r" (val) : : "cc");
	else
		asm volatile("mrs %0, sctlr_el3" : "=r" (val) : : "cc");

	return val;
}

static inline void set_sctlr(unsigned int val)
{
	unsigned int el;

	el = current_el();
	if (el == 1)
		asm volatile("msr sctlr_el1, %0" : : "r" (val) : "cc");
	else if (el == 2)
		asm volatile("msr sctlr_el2, %0" : : "r" (val) : "cc");
	else
		asm volatile("msr sctlr_el3, %0" : : "r" (val) : "cc");

	asm volatile("isb");
}

static inline int get_clidr()
{
	unsigned int val;

        asm volatile("mrs %0, clidr_el1" : "=r" (val) : : "cc");

	return val;
}

//performance control register
static inline unsigned int get_PMCR_EL0()
{
    unsigned val;
    asm("mrs %0,PMCR_EL0" : "=r"(val) :: "cc");
    return val;
}
static inline void set_PMCR_EL0(unsigned long long val)
{
    asm("msr PMCR_EL0 , %0" :: "r"(val) : "cc");   
}

//Event counter selection register
static inline unsigned int get_PMSELR_EL0()
{
    unsigned val;
    asm("mrs %0,PMSELR_EL0" : "=r"(val) :: "cc");
    return val;
}
static inline void set_PMSELR_EL0(unsigned long long val)
{
    asm("msr PMSELR_EL0 , %0" :: "r"(val) : "cc");   
}

//Event counter register
static inline unsigned int get_PMXEVCNTR_EL0()
{
    unsigned int val;	
    asm("mrs %0,PMXEVCNTR_EL0" :"=r"(val):: "cc");  
    return val; 
}
static inline void set_PMXEVTYPER_EL0(unsigned long long val)
{
    asm("msr PMXEVTYPER_EL0 , %0" :: "r"(val) : "cc");  
}

//interrupt enable set register
static inline void set_PMINTENSET_EL1(unsigned long long val)
{
    asm("msr PMINTENSET_EL1, %0" :: "r"(val) : "cc");   
}

//interrupt enable clear register
static inline void set_PMINTENCLR_EL1(unsigned long long val)
{
    asm("msr PMINTENCLR_EL1 , %0" :: "r"(val) : "cc");   
}

//overflow flag status register
static inline void set_PMOVSCLR_EL0(unsigned long long val)
{
    asm("msr PMOVSCLR_EL0 , %0" :: "r"(val) : "cc");   
}

//count enable set register
static inline void set_PMCNTENSET_EL0(unsigned long long val)
{
    asm("msr PMCNTENSET_EL0 , %0" :: "r"(val) : "cc");   
}

//performance monitor event count register
static inline void set_PMEVCNTR1_EL0(unsigned long long val)
{
    asm("msr PMEVCNTR1_EL0 , %0" :: "r"(val) : "cc");   
}



#endif