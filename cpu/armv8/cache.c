/***************************************************************************
* Copyright  Faraday Technology Corp 2015-2016.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:Cache.c                                                             *
* Description: Cache library routine                                       *
* Author: Mark_hs Fu-Tsung Hsu                                             *
****************************************************************************/

#include "arm_acle.h"
#include "cpu.h"
#include "cache.h"


void invalidate_icache_all()
{
    asm volatile("ic ialluis");
    asm volatile("isb sy");
}

void enable_icache(void)
{
    invalidate_icache_all();
    set_sctlr(get_sctlr() | CR_I);
}

void disable_icache(void)
{
    set_sctlr(get_sctlr() & ~CR_I);
}

void flush_dcache_level_setway(unsigned int cache_level)
{
    unsigned int ccsidr;
    unsigned int Line_Size;
    unsigned int log2_line_len;
    unsigned int max_way,max_set;
    unsigned int associativity,log2_associativity;
    int way_offset, way, set, setway;

    set_csselr(cache_level);
fLib_printf("flush_dcache_level_setway cache_level = 0x%x\n", cache_level);
    ccsidr = get_ccsidr();
fLib_printf("flush_dcache_level_setway ccsidr = 0x%x\n", ccsidr);
    Line_Size = ccsidr & 0x7; 	//bit[2:0]
    log2_line_len = Line_Size + 4;

    max_way = ((ccsidr >> 3)  & 0x3ff)  + 1;	//bit[12:2]
    max_set = ((ccsidr >> 13) & 0x7fff) + 1;	//bit[27:13]
fLib_printf("flush_dcache_level_setway max_way = 0x%x\n", max_way);
fLib_printf("flush_dcache_level_setway max_set = 0x%x\n", max_set);

    associativity = (ccsidr >> 3)  & 0x3ff;
    log2_associativity = log_2_n_round_up(associativity);
    way_offset = 32 - log2_associativity;

    for(way = max_way - 1 ; way >= 0 ; way--)
    {
//fLib_printf("flush_dcache_level_setway way = 0x%x\n", way);
        for(set = max_set - 1 ; set >=0 ; set--)
        {
//fLib_printf("flush_dcache_level_setway set = 0x%x\n", set);
            setway = ((cache_level << 1) | (set << log2_line_len) | (way << way_offset));
            asm volatile("dc cisw ,%0" : : "r" (setway) : "cc");
//	    asm volatile("dc cisw x0");
        }
    }
}

void invalidate_dcache_level_setway(unsigned int cache_level)
{
    unsigned int ccsidr;
    unsigned int Line_Size;
    unsigned int log2_line_len;
    unsigned int max_way,max_set;
    unsigned int associativity,log2_associativity;
    unsigned int way_offset, way, set, setway;

    set_csselr(cache_level);
    ccsidr = get_ccsidr();
    Line_Size = ccsidr & 0x7; 	//bit[2:0]
    log2_line_len = Line_Size + 4;

    max_way = ((ccsidr >> 3)  & 0x3ff)  + 1;	//bit[12:2]
    max_set = ((ccsidr >> 13) & 0x7fff) + 1;	//bit[27:13]

    associativity = (ccsidr >> 3)  & 0x3ff;
    log2_associativity = log_2_n_round_up(associativity);
    way_offset = 32 - log2_associativity;

    for(way = max_way - 1 ; way >= 0 ; way--)
    {
        for(set = max_set - 1 ; set >=0 ; set--)
        {
            setway = ((cache_level << 1) | (set << log2_line_len) | (way << way_offset));
            asm volatile("dc isw ,%0" : : "r" (setway) : "cc");
        }
    }
}

void clean_invalidate_dcache()
{
    unsigned int clidr;
    unsigned int loc;
    unsigned int cache_type;
    unsigned int cache_level;

    asm volatile("dsb sy"); 
    
    clidr = get_clidr();
fLib_printf("clean_invalidate_dcache clidr = 0x%x\n", clidr);
    loc = (clidr >>24) & 0x7;   //bit [26:24]
    if(!loc)
        return;

    unsigned int level_start_bit = 0;
    for(cache_level = 0;cache_level<7 ;cache_level++)
    {
        cache_type = (clidr >> level_start_bit) & 0x7;
        if((cache_type == CLIDR_CTYPE_DATA_ONLY) ||
           (cache_type == CLIDR_CTYPE_INSTRUCTION_DATA) ||
           (cache_type == CLIDR_CTYPE_UNIFIED))
	  	flush_dcache_level_setway(cache_level);
        level_start_bit +=3;
    }
}

void invalidate_dcache()
{
    unsigned int clidr;
    unsigned int loc;
    unsigned int cache_type;
    unsigned int cache_level;

    asm volatile("dsb sy"); 
    clidr = get_clidr();

    loc = (clidr >>24) & 0x7;   //bit [26:24]
    if(!loc)
        return;

    unsigned int level_start_bit = 0;
    for(cache_level = 0;cache_level<7 ;cache_level++)
    {
        cache_type = (clidr >> level_start_bit) & 0x7;
        if((cache_type == CLIDR_CTYPE_DATA_ONLY) ||
           (cache_type == CLIDR_CTYPE_INSTRUCTION_DATA) ||
           (cache_type == CLIDR_CTYPE_UNIFIED))
	  	invalidate_dcache_level_setway(cache_level);
        level_start_bit +=3;
    }
}

void flush_dcache_all()
{
	clean_invalidate_dcache();
}
void invalidate_dcache_all()
{
	invalidate_dcache();
}

void enable_dcache(void)
{
    if(!(get_sctlr() & CR_M))
    {
//        invalidate_dcache_all();
          invalidate_tlb_all();
//        mmu_setup();	//handle this in c pattern
    }
    set_sctlr(get_sctlr() | CR_C);
}

void disable_dcache(void)
{
     fLib_printf("\r disable_dcache start\n");  
#if 1
    unsigned int sctlr;
    sctlr = get_sctlr();
     fLib_printf("\r get_sctlr sctlr = 0x%x\n", sctlr);  
    if(!(sctlr & CR_C))
        return;
//    set_sctlr(sctlr & ~(CR_C|CR_M));
    set_sctlr(sctlr & ~(CR_C));
     fLib_printf("\r set_sctlr\n");  
    flush_dcache_all();
      fLib_printf("\r flush_dcache_all\n");     
    invalidate_tlb_all();
     fLib_printf("\r invalidate_tlb_all\n");  
#endif
}
void enable_cache(void)
{
    invalidate_tlb_all();
//    flush_dcache_all();					//CA53 default will invalidate cache
    set_sctlr(get_sctlr() | CR_I | CR_C |CR_M);
}
