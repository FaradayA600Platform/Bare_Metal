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
#include "arm_acle.h"
#include "mmu.h"
#include "cache.h"

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
unsigned long tlb_fillptr;

#define MEMORY_ATTRIBUTES	((0x00 << (0 * 8)) | (0x88 << (1 * 8)) | \
				 (0xcc << (2 * 8)) | (0xff << (3 * 8)))
#define max(a,b) (((a)>(b))?(a):(b))
				 
#ifdef CONFIG_MMU
int	cache_state=0;//global cache state(0:off 1:on)
static int	mmu_state=0;  //global mmu state(0:off 1:on)
//#define PTB_LOCATION        0x03000000 //page table base
#define MAX_PTE_ENTRIES 512

extern mm_region board_mmtbl[]; //define in Board/xxx/platform.c

u64 get_tcr(int el, u64 *pips, u64 *pva_bits)
{
	u64 max_addr = 0;
	u64 ips, va_bits;
	u64 tcr;
	int i;

	/* Find the largest address we need to support */
	for (i = 0; board_mmtbl[i].size || board_mmtbl[i].attrs; i++)
		max_addr = max(max_addr, board_mmtbl[i].virt + board_mmtbl[i].size);

	/* Calculate the maximum physical (and thus virtual) address */
	if (max_addr > (1ULL << 44)) {
		ips = 5;
		va_bits = 48;
	} else  if (max_addr > (1ULL << 42)) {
		ips = 4;
		va_bits = 44;
	} else  if (max_addr > (1ULL << 40)) {
		ips = 3;
		va_bits = 42;
	} else  if (max_addr > (1ULL << 36)) {
		ips = 2;
		va_bits = 40;
	} else  if (max_addr > (1ULL << 32)) {
		ips = 1;
		va_bits = 36;
	} else {
		ips = 0;
		va_bits = 32;
	}

	if (el == 1) {
		tcr = TCR_EL1_RSVD | (ips << 32) | TCR_EPD1_DISABLE;
	} else if (el == 2) {
		tcr = TCR_EL2_RSVD | (ips << 16);
	} else {
		tcr = TCR_EL3_RSVD | (ips << 16);
	}

	/* PTWs cacheable, inner/outer WBWA and inner shareable */
	tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;
	tcr |= TCR_T0SZ(va_bits);

	if (pips)
		*pips = ips;
	if (pva_bits)
		*pva_bits = va_bits;

	return tcr;
}

static int pte_type(u64 *pte)
{
	return *pte & PTE_TYPE_MASK;
}


static u64 *find_pte(u64 addr, int level)
{
	int start_level = 0;
	u64 *pte;
	u64 idx;
	u64 va_bits;
	int i;

//	printf("addr=%llx level=%d\n", addr, level);

	get_tcr(0, NULL, &va_bits);
	if (va_bits < 39)
		start_level = 1;

	if (level < start_level)
		return NULL;

	/* Walk through all page table levels to find our PTE */
	pte = (u64*)PTB_LOCATION;
	for (i = start_level; i < 4; i++) {
		idx = (addr >> level2shift(i)) & 0x1FF;
		pte += idx;
//		printf("idx=%llx PTE %p at level %d: %llx\n", idx, pte, i, *pte);

		/* Found it */
		if (i == level)
			return pte;
		/* PTE is no table (either invalid or block), can't traverse */
		if (pte_type(pte) != PTE_TYPE_TABLE)
			return NULL;
		/* Off to the next level */
		pte = (u64*)(*pte & 0x0000fffffffff000ULL);
	}

	/* Should never reach here */
	return NULL;
}

/* Returns and creates a new full table (512 entries) */
static u64 *create_table(void)
{
	u64 *new_table = (u64*)tlb_fillptr;
	u64 pt_len = MAX_PTE_ENTRIES * sizeof(u64);

	/* Allocate MAX_PTE_ENTRIES pte entries */
	tlb_fillptr += pt_len;

	/* Mark all entries as invalid */
	memset(new_table, 0, pt_len);

	return new_table;
}

static void set_pte_table(u64 *pte, u64 *table)
{
	/* Point *pte to the new table */
//	printf("Setting %p to addr=%p\n", pte, table);
	*pte = PTE_TYPE_TABLE | (ulong)table;
}

/* Returns the LSB number for a PTE on level <level> */
int level2shift(int level)
{
	/* Page is 12 bits wide, every level translates 9 bits */
	return (12 + 9 * (3 - level));
}

/* Add one mm_region map entry to the page tables */
static void add_map(mm_region *map)
{
	u64 *pte;
	u64 virt = map->virt;
	u64 phys = map->phys;
	u64 size = map->size;
	u64 attrs = map->attrs | PTE_TYPE_BLOCK | PTE_BLOCK_AF;
	u64 blocksize;
	int level;
	u64 *new_table;

	while (size) {
		pte = find_pte(virt, 0);
		if (pte && (pte_type(pte) == PTE_TYPE_FAULT)) {
//			printf("Creating table for virt 0x%llx\n", virt);
			new_table = create_table();
			set_pte_table(pte, new_table);
		}
        
		for (level = 1; level < 4; level++) {
			pte = find_pte(virt, level);
			if (!pte)
				printf("pte not found\n");

			blocksize = 1ULL << level2shift(level);
//			printf("Checking if pte fits for virt=%llx size=%llx blocksize=%llx\n",
//			      virt, size, blocksize);
			if (size >= blocksize && !(virt & (blocksize - 1))) {
				/* Page fits, create block PTE */
//				printf("Setting PTE %p to block virt=%llx\n",
//				      pte, virt);
				*pte = phys | attrs;
				virt += blocksize;
				phys += blocksize;
				size -= blocksize;
				break;
			} else if (pte_type(pte) == PTE_TYPE_FAULT) {
				/* Page doesn't fit, create subpages */
//				printf("Creating subtable for virt 0x%llx blksize=%llx\n",
//				      virt, blocksize);
				new_table = create_table();
				set_pte_table(pte, new_table);
			} else if (pte_type(pte) == PTE_TYPE_BLOCK) {
				printf("Split block into subtable for virt 0x%llx blksize=0x%llx\n",
				      virt, blocksize);
			}
		}
	}
}

void MMU_Enable()
{
	unsigned temp,ctrl;
	unsigned int el = current_el();
    
	mmu_init();

	/* point TTBR to the new table */
	set_ttbr_tcr_mair(el, PTB_LOCATION, get_tcr(el, NULL, NULL),
			  MEMORY_ATTRIBUTES);
       enable_cache();
	mmu_state = 1; //ycmo: useless if mmu turned on in start.s, because it will be overwrite by C Runtime, work around in CPU_AllCache_Enable

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

void mmu_init(void)
{
    unsigned int i, j, pa, *p;

    /* Translation Table must be aligned on 16K boundary */
//    unsigned int ttb = 0x4000;
    unsigned long ttb = PTB_LOCATION;

    tlb_fillptr = ttb;
    // 1. build Translation Table (Section translation)
//    memset((void *)ttb, 0, 0x100000);
    create_table();
    for (i = 0; board_mmtbl[i].size; ++i)
    {
#if 0
        p = (unsigned int *)(ttb + ((board_mmtbl[i].virt >> 20) << 2));
        for (j = 0; j < board_mmtbl[i].size; ++j)
        {
            int ap = 3; // Access permission
            pa = board_mmtbl[i].phys + (j << 20);
            p[j] = (pa & 0xFFF00000) | (ap << 10) | (board_mmtbl[i].attrs) | 0x12;
        }
#else
	add_map(&board_mmtbl[i]);

#endif
    }
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
	MMU_Enable();
	//printf("\rMMU/Cache enabled\n");
	mmu_state=1;
	cache_state=1;
}
void CPU_AllCache_Disable()
{
    UINT32 temp=0;
    UINT32 temp_v=0;
   
     printf("\r disable_dcache\n");  
    disable_dcache();
     printf("\r disable_icache\n");  
    disable_icache();
    cache_state=0;
    printf("\r Cache disabled\n");
    mmu_state=1;

}

void fLib_DisableDCache(){

    disable_dcache();
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
