/***************************************************************************
* Copyright  Faraday Technology Corp 2015-2016.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:system.h                                                            *
* Description: utility                                             *
* Author: Mark_hs Fu-Tsung Hsu                                             *
****************************************************************************/

#ifndef __ARM_ACLE
#define __ARM_ACLE

#define GIC_DIST_SGIR	0xf00
		
static inline unsigned int Get_CPUID()
{
	unsigned long long core_id;
	asm("mrs %0, mpidr_el1" : "=r" (core_id) : : "cc");
	core_id &= 0xff;
	return (unsigned int)core_id;
}
static inline unsigned int Get_CLUSTERID()
{
	unsigned long long cluster_id;
	asm("mrs %0, mpidr_el1" : "=r" (cluster_id) : : "cc");
	cluster_id = (cluster_id & 0xff00)>>8;
	return (unsigned int)cluster_id;
}
static inline void Send_SGI(unsigned filter_list , unsigned int target_list , unsigned int IRQn)
{
        unsigned int gicd_base;
	unsigned int val; 
	unsigned int i;
        gicd_base = get_periph_base() + 0x1000;
	val = (filter_list & 0x03) << 24 |((target_list & 0xFF) << 16) | (IRQn & 0xf);
	HAL_WRITE_UINT32(gicd_base + GIC_DIST_SGIR , val);
//	for(i = 0;i<50;i++);
}
static inline unsigned int log_2_n_round_up(unsigned int n)
{
    int log2n = -1;
    unsigned int temp = n;

    while (temp) {
        log2n++;
        temp >>= 1;
    }

    if (n & (n - 1))
        return log2n + 1; /* not power of 2 - round up */
    else
        return log2n; /* power of 2 */
}

#endif // __ARM_ACLE