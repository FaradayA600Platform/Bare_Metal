#ifndef __SPEC_ASM_H
#define __SPEC_ASM_H

/*
 * SCTLR_EL1/SCTLR_EL2/SCTLR_EL3 bits definitions
 */
#define CR_M		(1 << 0)	/* MMU enable			*/
#define CR_A		(1 << 1)	/* Alignment abort enable	*/
#define CR_C		(1 << 2)	/* Dcache enable		*/
#define CR_SA		(1 << 3)	/* Stack Alignment Check Enable	*/
#define CR_I		(1 << 12)	/* Icache enable		*/
#define CR_WXN		(1 << 19)	/* Write Permision Imply XN	*/
#define CR_EE		(1 << 25)	/* Exception (Big) Endian	*/

// AARCH64 SPSR
#define AARCH64_SPSR_EL3h   	 0xd	// 2_1101
#define AARCH64_SPSR_EL3t   	 0xc	// 2_1100
#define AARCH64_SPSR_EL2h   	 0x9	// 2_1001
#define AARCH64_SPSR_EL2t   	 0x8	// 2_1000
#define AARCH64_SPSR_EL1h   	 0x5	// 2_0101
#define AARCH64_SPSR_EL1t   	 0x4	// 2_0100
#define AARCH64_SPSR_EL0t   	 0x0	// 2_0000
#define AARCH64_SPSR_RW    	(1 << 4)
#define AARCH64_SPSR_F  	(1 << 6)
#define AARCH64_SPSR_I     	(1 << 7)
#define AARCH64_SPSR_A    	(1 << 8)
#define AARCH64_SPSR_D     	(1 << 9)
#define AARCH64_SPSR_IL     	(1 << 20)
#define AARCH64_SPSR_SS    	(1 << 21)
#define AARCH64_SPSR_V     	(1 << 28)
#define AARCH64_SPSR_C     	(1 << 29)
#define AARCH64_SPSR_Z     	(1 << 30)
#define AARCH64_SPSR_N      	(1 << 31)

#define PSR_MODE_EL0t   0x00000000
#define PSR_MODE_EL1t   0x00000004
#define PSR_MODE_EL1h   0x00000005
#define PSR_MODE_EL2t   0x00000008
#define PSR_MODE_EL2h   0x00000009
#define PSR_MODE_EL3t   0x0000000c
#define PSR_MODE_EL3h   0x0000000d
#define PSR_MODE_MASK   0x0000000f

// Multiprocessor Affinity Register

#define MPIDR_EL1_AFF3_LSB 	32
#define MPIDR_EL1_U        	(1 << 30)
#define MPIDR_EL1_MT        	(1 << 24)
#define MPIDR_EL1_AFF2_LSB  	16
#define MPIDR_EL1_AFF1_LSB  	8
#define MPIDR_EL1_AFF0_LSB  	0
#define MPIDR_EL1_AFF_WIDTH 	8


// Data Cache Zero ID Register

#define DCZID_EL0_BS_LSB    	0
#define DCZID_EL0_BS_WIDTH  	4
#define DCZID_EL0_DZP_LSB   	5
#define DCZID_EL0_DZP       	(1 << 5)


// System Control Register

#define SCTLR_EL1_UCI       	(1 << 26)
#define SCTLR_ELx_EE       	(1 << 25)
#define SCTLR_EL1_E0E      	(1 << 24)
#define SCTLR_ELx_WXN       	(1 << 19)
#define SCTLR_EL1_nTWE      	(1 << 18)
#define SCTLR_EL1_nTWI     	(1 << 16)
#define SCTLR_EL1_UCT      	(1 << 15)
#define SCTLR_EL1_DZE       	(1 << 14)
#define SCTLR_ELx_I       	(1 << 12)
#define SCTLR_EL1_UMA       	(1 << 9)
#define SCTLR_EL1_SED      	(1 << 8)
#define SCTLR_EL1_ITD      	(1 << 7)
#define SCTLR_EL1_THEE      	(1 << 6)
#define SCTLR_EL1_CP15BEN   	(1 << 5)
#define SCTLR_EL1_SA0       	(1 << 4)
#define SCTLR_ELx_SA        	(1 << 3)
#define SCTLR_ELx_C        	(1 << 2)
#define SCTLR_ELx_A        	(1 << 1)
#define SCTLR_ELx_M        	(1 << 0)



// Architectural Feature Access Control Register

#define CPACR_EL1_TTA        (1 << 28)
#define CPACR_EL1_FPEN       (3 << 20)


// Architectural Feature Trap Register

#define CPTR_ELx_TCPAC       (1 << 31)
#define CPTR_ELx_TTA         (1 << 20)
#define CPTR_ELx_TFP         (1 << 10)


// Secure Configuration Register

#define SCR_EL3_TWE      (1 << 13)
#define SCR_EL3_TWI      (1 << 12)
#define SCR_EL3_ST       (1 << 11)
#define SCR_EL3_RW       (1 << 10)
#define SCR_EL3_SIF      (1 << 9)
#define SCR_EL3_HCE      (1 << 8)
#define SCR_EL3_SMD      (1 << 7)
#define SCR_EL3_EA       (1 << 3)
#define SCR_EL3_FIQ      (1 << 2)
#define SCR_EL3_IRQ      (1 << 1)
#define SCR_EL3_NS       (1 << 0)

//cache related
#define CLIDR_CTYPE_DATA_ONLY		2
#define CLIDR_CTYPE_INSTRUCTION_DATA	3
#define CLIDR_CTYPE_UNIFIED		4

//
// Hypervisor Configuration Register
//
#define HCR_EL2_ID   (1 << 33)
#define HCR_EL2_CD   (1 << 32)
#define HCR_EL2_RW   (1 << 31)
#define HCR_EL2_TRVM (1 << 30)
#define HCR_EL2_HVC  (1 << 29)
#define HCR_EL2_TDZ  (1 << 28)

//
// Stage 1 Translation Table descriptor fields
//
#define TT_S1_ATTR_FAULT 		(0b00 << 0)
#define TT_S1_ATTR_BLOCK 		(0b01 << 0) // Level 1/2
#define TT_S1_ATTR_TABLE 		(0b11 << 0) // Level 0/1/2
#define TT_S1_ATTR_PAGE  		(0b11 << 0) // Level 3


#define TT_S1_ATTR_MATTR_LSB 		2

#define TT_S1_ATTR_NS 			(1 << 5)

#define TT_S1_ATTR_AP_RW_PL1 		(0b00 << 6)
#define TT_S1_ATTR_AP_RW_ANY 		(0b01 << 6)
#define TT_S1_ATTR_AP_RO_PL1 		(0b10 << 6)
#define TT_S1_ATTR_AP_RO_ANY 		(0b11 << 6)

#define TT_S1_ATTR_SH_NONE   		(0b00 << 8)
#define TT_S1_ATTR_SH_OUTER  		(0b10 << 8)
#define TT_S1_ATTR_SH_INNER  		(0b11 << 8)

#define TT_S1_ATTR_AF 			(1 << 10)
#define TT_S1_ATTR_nG 			(1 << 11)

#define TT_S1_ATTR_CONTIG 		(1 << 52)
#define TT_S1_ATTR_PXN    		(1 << 53)
#define TT_S1_ATTR_UXN    		(1 << 54)

#define TT_S1_MAIR_DEV_nGnRnE 		0b00000000
#define TT_S1_MAIR_DEV_nGnRE  		0b00000100
#define TT_S1_MAIR_DEV_nGRE   		0b00001000
#define TT_S1_MAIR_DEV_GRE    		0b00001100

//
// Inner and Outer Normal memory attributes use the same bit patterns
// Outer attributes just need to be shifted up
//
#define TT_S1_MAIR_OUTER_SHIFT	 	4

#define TT_S1_MAIR_WT_TRANS_RA  	0b0010

#define TT_S1_MAIR_WB_TRANS_RA  	0b0110
#define TT_S1_MAIR_WB_TRANS_RWA 	0b0111

#define TT_S1_MAIR_WT_RA  		0b1010

#define TT_S1_MAIR_WB_RA  		0b1110
#define TT_S1_MAIR_WB_RWA 		0b1111


#define LEVEL1_ENTRY_SIZE 			8
#define LEVEL2_ENTRY_SIZE			8


#endif
