/***************************************************************************
* Copyright  Faraday Technology Corp 2006-2008.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:cpu.c                                                       *
* Description: generic cpu helper routines for all kinds of cpu                           			   *
* Author: Fred Chien                                                       *
****************************************************************************/
//#include "types.h"
//#include "sizes.h"
#include "SoFlexible.h"
#include "cpu.h"

#define MMU_BIT             0
#define ALIGN_BIT			1
#define DCACHE_BIT			2
#define WBUF_BIT			3
#define BIGEN_BIT			7
#define SYSPROT_BIT			8
#define ROMPROT_BIT			9
#define BTB_BIT				11
#define ICACHE_BIT			12
#define VECTOR_BIT			13 //high vector 
#define RAO_BIT				14 //Read allocation 
#define L4_BIT				15 //Load PC change T bit
#define STMWA_BIT			16 //stm write allocation
#define NHE_BIT				21 //no hit under miss

extern UINT32 PeriphBase;

#ifdef CONFIG_MMU
int	cache_state=0;//global cache state(0:off 1:on)
static int	mmu_state=0;  //global mmu state(0:off 1:on)
//#define PTB_LOCATION        0x03000000 //page table base


void MMU_Enable()
{
	unsigned temp,ctrl;
	mmu_init();

	/*	temp = PTB_LOCATION;
		__cpu_fn(CPU_NAME,_CPUSetTLBBaseAddr)(temp);*/
	cp15_set_tlb_base((void *)PTB_LOCATION);


	/*	temp = 0x0;//domain 0
		ctrl=3;//manager, no permission check
		__cpu_fn(CPU_NAME,_CPUSetDomainAccessCtrl)(temp,ctrl); */
	cp15_set_domain_access_control(0x0, 0x3);

	//invalidate tlb
	//	__cpu_fn(CPU_NAME,_CPUInvalidateTLBAll)();  
	cp15_invalidate_utlb_all();

	// enable MMU 
	//	temp=__cpu_fn(CPU_NAME,_CPUReadCFG)();
	temp = cp15_read_c1();
	temp = temp | (1 << MMU_BIT);
	//	__cpu_fn(CPU_NAME,_CPUWriteCFG)(temp);
	cp15_write_c1(temp);

	//invalidate tlb
	//	__cpu_fn(CPU_NAME,_CPUInvalidateTLBAll)();
	cp15_invalidate_utlb_all();
	mmu_state = 1; //ycmo: useless if mmu turned on in start.s, because it will be overwrite by C Runtime, work around in CPU_AllCache_Enable

}


void EnableCacheMMU()
{
	unsigned temp,ctrl;
	// set TTB, locate at 16K boundary
	// enable data/instruction cache 
	/*	temp = PTB_LOCATION;
		__cpu_fn(CPU_NAME,_CPUSetTLBBaseAddr)(temp);*/
	cp15_set_tlb_base((void *)PTB_LOCATION);


	// set domain register
	/*	temp = 0x0;//domain 0
		ctrl=3;//manager, no permission check
		__cpu_fn(CPU_NAME,_CPUSetDomainAccessCtrl)(temp,ctrl); */
	cp15_set_domain_access_control(0x0, 0x3);

	//invalidate i/d cache 
	//     __cpu_fn(CPU_NAME,_CPUInvalidateDCacheICacheAll)();

	invalidate_icache_dcache_all();

	//invalidate tlb
	//	__cpu_fn(CPU_NAME,_CPUInvalidateTLBAll)();  
	cp15_invalidate_utlb_all();

	// enable MMU and I/D cache
	//	temp=__cpu_fn(CPU_NAME,_CPUReadCFG)();
	temp = cp15_read_c1();
//	temp = temp | (1 << MMU_BIT);
	temp = temp | (1 << ICACHE_BIT) | (1 << BTB_BIT) | (1 << DCACHE_BIT )|(1 << MMU_BIT);
	//	__cpu_fn(CPU_NAME,_CPUWriteCFG)(temp);
	cp15_write_c1(temp);
	//    __cpu_fn(CPU_NAME,_CPUPrefetchFlush)();
	__isb();



	//invalidate tlb
	//	__cpu_fn(CPU_NAME,_CPUInvalidateTLBAll)();  
//	cp15_invalidate_utlb_all();

}



#if 0 //move to platform.c
mmap_t board_mmtbl[] =
{
    /*        VA,          PA,      SZ,   CB, RSVD */
    { 0x00000000,  0x00000000,     240,    3,    0 },   /* Cached + Buffered */
    { 0x0F000000,  0x0F000000,    3856,    0,    0 },   /* Uncached + Unbuffered */
    { 0 },
};
#endif


extern mmap_t board_mmtbl[]; //define in Board/xxx/platform.c

void mmu_init(void)
{
    unsigned int i, j, pa, *p;

    /* Translation Table must be aligned on 16K boundary */
//    unsigned int ttb = 0x4000;
    unsigned int ttb = PTB_LOCATION;


    // 1. build Translation Table (Section translation)
    memset((void *)ttb, 0, 4096 * 4);
    for (i = 0; board_mmtbl[i].sz; ++i)
    {
        p = (unsigned int *)(ttb + ((board_mmtbl[i].va >> 20) << 2));
        for (j = 0; j < board_mmtbl[i].sz; ++j)
        {
            int ap = 3; // Access permission
            pa = board_mmtbl[i].pa + (j << 20);
            p[j] = (pa & 0xFFF00000) | (ap << 10) | (board_mmtbl[i].cb << 2) | 0x12;
        }
    }
/*
///////////////for test//////////////
    p = (unsigned int *)(ttb + ((PeriphBase >> 20) << 2));
    for (j = 0; j < 1; ++j)
    {
        int ap = 3; // Access permission
        pa = PeriphBase + (j << 20);
        p[j] = (pa & 0xFFF00000) | (ap << 10) | (0 << 2) | 0x12;
    }
///////////////test end//////////////
*/
}


void CPU_AllCache_Enable()
{       
#ifdef CONFIG_START_W_MMU //ycmo work around to avoid C runtime overwrite data/bss section
	mmu_state=1;
#endif
	if (cache_state) 
		return;

	if(mmu_state==0) 
		mmu_init();
	EnableCacheMMU();
	//printf("\rMMU/Cache enabled\n");
	mmu_state=1;
	cache_state=1;
}
void CPU_AllCache_Disable()
{
    UINT32 temp=0;
    UINT32 temp_v=0;
    
	/* clean and invalidate all cache */

	if (cache_state) 
        // ycmo: clean dcache to sync cache and memory
        clean_dcache_all();

	__dsb(); 
    	
    
	//	temp=__cpu_fn(CPU_NAME,_CPUReadCFG)();
	temp = cp15_read_c1();
#ifdef CONFIG_START_W_MMU //ycmo : let MMU always on
    temp_v = (1 << ICACHE_BIT) | (1 << DCACHE_BIT );
#else
    temp_v = (1 << ICACHE_BIT) | (1 << DCACHE_BIT )| (1 << MMU_BIT);
#endif
    temp_v = ~temp_v;
    temp = temp & temp_v;
	//	__cpu_fn(CPU_NAME,_CPUWriteCFG)(temp);
	cp15_write_c1(temp);
	//    __cpu_fn(CPU_NAME,_CPUPrefetchFlush)();
	__isb();
   
    
    // ycmo: make sure, there is no garbage data in cache during the disable action.
    // to avoid problem when cache enable next time
	invalidate_dcache_all();

	cache_state=0;
#ifdef CONFIG_START_W_MMU //ycmo : make MMU always on
    printf("\rCache disabled\n");
    mmu_state=1;
#else
    printf("\rMMU/Cache disabled\n");
    mmu_state=0;
#endif
}

void fLib_DisableDCache(){
    UINT32 cp15_c1 = cp15_read_c1();
    cp15_c1 &= (~(1<< DCACHE_BIT));

	__dsb(); 
    flush_dcache_all();
	cp15_write_c1(cp15_c1);
	__isb();
    flush_dcache_all();
}


#else //no CONFIG_MMU
void CPU_AllCache_Disable() {return;}
void fLib_DisableDCache(){return;}
void CPU_AllCache_Enable() {
	printf("Error: No MMU Support! \n");
	printf("\t Please check configuration and platform.c \n");
	return;
}
#endif //CONFIG_MMU


void CPU_ChangeETTBBase(int ptb_addr)
{
 UINT32 temp;
    
	//invalidate i/d cache 
	invalidate_icache_dcache_all();

    //invalidate tlb and drain wb
	//	__cpu_fn(CPU_NAME,_CPUInvalidateTLBAll)();  
	cp15_invalidate_utlb_all();
	// __cpu_fn(CPU_NAME,_CPUDrainWriteBuffer)();
	__dsb();
    
    // set TTB, locate at 16K boundary
    // enable data/instruction cache 
/*    temp = ptb_addr & ~0x4000;//16k align
    __cpu_fn(CPU_NAME,_CPUSetTLBBaseAddr)(temp);    */ 
	cp15_set_tlb_base((void *)(ptb_addr & ~0x4000));

	//    __cpu_fn(CPU_NAME,_CPUPrefetchFlush)();
	__isb();
    
}

UINT32 CPUReadVendorID(void)
{
    UINT32 IdReg,VendorID;
 
	// 	IdReg =  __cpu_fn(CPU_NAME,_CPUReadIDReg)();
	IdReg = cp15_read_id_code();
 	VendorID = (IdReg >>24) & 0xff;
 
 	return(VendorID);	
}	   
   
   
UINT32 CPUReadArchID(void)
{
    UINT32 IdReg,ArchID;
 
	// 	IdReg =  __cpu_fn(CPU_NAME,_CPUReadIDReg)();
	IdReg = cp15_read_id_code();
 	ArchID=(IdReg >>16)&0xff;
 
 	return(ArchID);	
}	   
   
UINT32 CPUReadPartNum(void)
{
    UINT32 IdReg,PartNum;
 
	// 	IdReg =  __cpu_fn(CPU_NAME,_CPUReadIDReg)();
	IdReg = cp15_read_id_code();
 	PartNum=(IdReg >>4)&0xfff;
 	
 	return(PartNum);	
}	   
     
UINT32 CPUReadVersion(void)
{
    UINT32 IdReg,Version;
 
	// 	IdReg =  __cpu_fn(CPU_NAME,_CPUReadIDReg)();
	IdReg = cp15_read_id_code();
 	Version=IdReg & 0xf;
 
 	return(Version);	
}	   


void CPUDelay(float sec){

    u32 count = (u32) (sec * (float)get_cpu_clk() / 3);

    printf(" sec=%f, count=%d\n",sec, count);
    __asm volatile(
        "mov r0, #0" "\n"
        "1:" "\n\t"
        "add r0,r0, #1" "\n\t"
        "cmp r0, %[count]" "\n\t"
        "blt 1b" 
        : :[count]"r"(count): "cc", "r0"
    );

    printf("finish: %s \n", __func__);
}
/* ycmo : not yet support
UINT32 CPUGetICacheLineLength(void)
{	
	CP15CTRRegStruct ctr;
	
	*((UINT32 *)&ctr) = __cpu_fn(CPU_NAME,_CPUReadCTR)();
	return CPU_CACHE_LINE_LEN(ctr.ILEN);
}

UINT32 CPUGetDCacheLineLength(void)
{	
	CP15CTRRegStruct ctr;
	
	*((UINT32 *)&ctr) = __cpu_fn(CPU_NAME,_CPUReadCTR)();
	return CPU_CACHE_LINE_LEN(ctr.DLEN);
}


*/
