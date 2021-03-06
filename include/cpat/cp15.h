#ifndef __ASM_ARM_CP15_H
#define __ASM_ARM_CP15_H


/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M	(1 << 0)	/* MMU enable				*/
#define CR_A	(1 << 1)	/* Alignment abort enable		*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_W	(1 << 3)	/* Write buffer enable			*/
#define CR_P	(1 << 4)	/* 32-bit exception handler		*/
#define CR_D	(1 << 5)	/* 32-bit data address range		*/
#define CR_L	(1 << 6)	/* Implementation defined		*/
#define CR_B	(1 << 7)	/* Big endian				*/
#define CR_S	(1 << 8)	/* System MMU protection		*/
#define CR_R	(1 << 9)	/* ROM MMU protection			*/
#define CR_F	(1 << 10)	/* Implementation defined		*/
#define CR_Z	(1 << 11)	/* Implementation defined		*/
#define CR_I	(1 << 12)	/* Icache enable			*/
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR	(1 << 14)	/* Round Robin cache replacement	*/
#define CR_L4	(1 << 15)	/* LDR pc can set T bit			*/
#define CR_DT	(1 << 16)
#define CR_IT	(1 << 18)
#define CR_ST	(1 << 19)
#define CR_FI	(1 << 21)	/* Fast interrupt (lower latency mode)	*/
#define CR_U	(1 << 22)	/* Unaligned access operation		*/
#define CR_XP	(1 << 23)	/* Extended page tables			*/
#define CR_VE	(1 << 24)	/* Vectored interrupts			*/
#define CR_EE	(1 << 25)	/* Exception (Big) Endian		*/
#define CR_TRE	(1 << 28)	/* TEX remap enable			*/
#define CR_AFE	(1 << 29)	/* Access flag enable			*/
#define CR_TE	(1 << 30)	/* Thumb exception enable		*/

/* CCSIDR */
#define CCSIDR_LINE_SIZE_OFFSET         0
#define CCSIDR_LINE_SIZE_MASK           0x7
#define CCSIDR_ASSOCIATIVITY_OFFSET     3
#define CCSIDR_ASSOCIATIVITY_MASK       (0x3FF << 3)
#define CCSIDR_NUM_SETS_OFFSET          13
#define CCSIDR_NUM_SETS_MASK            (0x7FFF << 13)
/*
 * Values for InD field in CSSELR
 * Selects the type of cache
 */
#define ARMV7_CSSELR_IND_DATA_UNIFIED   0
#define ARMV7_CSSELR_IND_INSTRUCTION    1

/* Values for Ctype fields in CLIDR */
#define ARMV7_CLIDR_CTYPE_NO_CACHE              0
#define ARMV7_CLIDR_CTYPE_INSTRUCTION_ONLY      1
#define ARMV7_CLIDR_CTYPE_DATA_ONLY             2
#define ARMV7_CLIDR_CTYPE_INSTRUCTION_DATA      3
#define ARMV7_CLIDR_CTYPE_UNIFIED               4


#define ARMV7_DCACHE_INVAL_ALL          1
#define ARMV7_DCACHE_CLEAN_INVAL_ALL    2
#define ARMV7_DCACHE_INVAL_RANGE        3
#define ARMV7_DCACHE_CLEAN_INVAL_RANGE  4





// borrowed from /xen/xen/include/asm-arm/cpregs.h
#ifndef __ASM_ARM_CPREGS_H
#define __ASM_ARM_CPREGS_H

//#include <xen/stringify.h>
#define __stringify(x...)   __stringify_1(x)
#define __stringify_1(x...) #x
/* Co-processor registers */

/* Layout as used in assembly, with src/dest registers mixed in */
#define __CP32(r, coproc, opc1, crn, crm, opc2) coproc, opc1, r, crn, crm, opc2
#define __CP64(r1, r2, coproc, opc, crm) coproc, opc, r1, r2, crm
#define CP32(r, name...) __CP32(r, name)
#define CP64(r, name...) __CP64(r, name)

/* Stringified for inline assembly */
#define LOAD_CP32(r, name...)  "mrc " __stringify(CP32(%r, name)) ";"
#define STORE_CP32(r, name...) "mcr " __stringify(CP32(%r, name)) ";"
#define LOAD_CP64(r, name...)  "mrrc " __stringify(CP64(%r, %H##r, name)) ";"
#define STORE_CP64(r, name...) "mcrr " __stringify(CP64(%r, %H##r, name)) ";"

/* C wrappers */
#define READ_CP32(name...) ({                                   \
    register uint32_t _r;                                       \
    asm volatile(LOAD_CP32(0, name) : "=r" (_r));               \
    _r; })

#define WRITE_CP32(v, name...) do {                             \
    register uint32_t _r = (v);                                 \
    asm volatile(STORE_CP32(0, name) : : "r" (_r));             \
} while (0)

#define READ_CP64(name...) ({                                   \
    register uint64_t _r;                                       \
    asm volatile(LOAD_CP64(0, name) : "=r" (_r));               \
    _r; })

#define WRITE_CP64(v, name...) do {                             \
    register uint64_t _r = (v);                                 \
    asm volatile(STORE_CP64(0, name) : : "r" (_r));             \
} while (0)

#define __HSR_CPREG_c0  0
#define __HSR_CPREG_c1  1
#define __HSR_CPREG_c2  2
#define __HSR_CPREG_c3  3
#define __HSR_CPREG_c4  4
#define __HSR_CPREG_c5  5
#define __HSR_CPREG_c6  6
#define __HSR_CPREG_c7  7
#define __HSR_CPREG_c8  8
#define __HSR_CPREG_c9  9
#define __HSR_CPREG_c10 10
#define __HSR_CPREG_c11 11
#define __HSR_CPREG_c12 12
#define __HSR_CPREG_c13 13
#define __HSR_CPREG_c14 14
#define __HSR_CPREG_c15 15

#define __HSR_CPREG_0   0
#define __HSR_CPREG_1   1
#define __HSR_CPREG_2   2
#define __HSR_CPREG_3   3
#define __HSR_CPREG_4   4
#define __HSR_CPREG_5   5
#define __HSR_CPREG_6   6
#define __HSR_CPREG_7   7

#define _HSR_CPREG32(cp,op1,crn,crm,op2) \
    ((__HSR_CPREG_##crn) << HSR_CP32_CRN_SHIFT) | \
    ((__HSR_CPREG_##crm) << HSR_CP32_CRM_SHIFT) | \
    ((__HSR_CPREG_##op1) << HSR_CP32_OP1_SHIFT) | \
    ((__HSR_CPREG_##op2) << HSR_CP32_OP2_SHIFT)

#define _HSR_CPREG64(cp,op1,crm) \
    ((__HSR_CPREG_##crm) << HSR_CP64_CRM_SHIFT) | \
    ((__HSR_CPREG_##op1) << HSR_CP64_OP1_SHIFT)

/* Encode a register as per HSR ISS pattern */
#define HSR_CPREG32(X) _HSR_CPREG32(X)
#define HSR_CPREG64(X) _HSR_CPREG64(X)

/*
 * Order registers by Coprocessor-> CRn-> Opcode 1-> CRm-> Opcode 2
 *
 * This matches the ordering used in the ARM as well as the groupings
 * which the CP registers are allocated in.
 *
 * This is slightly different to the form of the instruction
 * arguments, which are cp,opc1,crn,crm,opc2.
 */

/* Coprocessor 14 */

/* CP14 CR0: */
#define TEECR           p14,6,c0,c0,0   /* ThumbEE Configuration Register */

/* CP14 CR1: */
#define TEEHBR          p14,6,c1,c0,0   /* ThumbEE Handler Base Register */
#define JOSCR           p14,7,c1,c0,0   /* Jazelle OS Control Register */

/* CP14 CR2: */
#define JMCR            p14,7,c2,c0,0   /* Jazelle Main Configuration Register */


/* Coprocessor 15 */

/* CP15 CR0: CPUID and Cache Type Registers */
#define MIDR            p15,0,c0,c0,0   /* Main ID Register */
#define MPIDR           p15,0,c0,c0,5   /* Multiprocessor Affinity Register */
#define ID_PFR0         p15,0,c0,c1,0   /* Processor Feature Register 0 */
#define ID_PFR1         p15,0,c0,c1,1   /* Processor Feature Register 1 */
#define ID_DFR0         p15,0,c0,c1,2   /* Debug Feature Register 0 */
#define ID_AFR0         p15,0,c0,c1,3   /* Auxiliary Feature Register 0 */
#define ID_MMFR0        p15,0,c0,c1,4   /* Memory Model Feature Register 0 */
#define ID_MMFR1        p15,0,c0,c1,5   /* Memory Model Feature Register 1 */
#define ID_MMFR2        p15,0,c0,c1,6   /* Memory Model Feature Register 2 */
#define ID_MMFR3        p15,0,c0,c1,7   /* Memory Model Feature Register 3 */
#define ID_ISAR0        p15,0,c0,c2,0   /* ISA Feature Register 0 */
#define ID_ISAR1        p15,0,c0,c2,1   /* ISA Feature Register 1 */
#define ID_ISAR2        p15,0,c0,c2,2   /* ISA Feature Register 2 */
#define ID_ISAR3        p15,0,c0,c2,3   /* ISA Feature Register 3 */
#define ID_ISAR4        p15,0,c0,c2,4   /* ISA Feature Register 4 */
#define ID_ISAR5        p15,0,c0,c2,5   /* ISA Feature Register 5 */
#define CCSIDR          p15,1,c0,c0,0   /* Cache Size ID Registers */
#define CLIDR           p15,1,c0,c0,1   /* Cache Level ID Register */
#define CSSELR          p15,2,c0,c0,0   /* Cache Size Selection Register */

/* CP15 CR1: System Control Registers */
#define SCTLR           p15,0,c1,c0,0   /* System Control Register */
#define ACTLR           p15,0,c1,c0,1   /* Auxiliary Control Register */
#define CPACR           p15,0,c1,c0,2   /* Coprocessor Access Control Register */
#define SCR             p15,0,c1,c1,0   /* Secure Configuration Register */
#define NSACR           p15,0,c1,c1,2   /* Non-Secure Access Control Register */
#define HSCTLR          p15,4,c1,c0,0   /* Hyp. System Control Register */
#define HCR             p15,4,c1,c1,0   /* Hyp. Configuration Register */

/* CP15 CR2: Translation Table Base and Control Registers */
#define TTBR0           p15,0,c2,c0,0   /* Translation Table Base Reg. 0 */
#define TTBR1           p15,0,c2,c0,1   /* Translation Table Base Reg. 1 */
#define TTBCR           p15,0,c2,c0,2   /* Translatation Table Base Control Register */
#define HTTBR           p15,4,c2        /* Hyp. Translation Table Base Register */
#define HTCR            p15,4,c2,c0,2   /* Hyp. Translation Control Register */
#define VTCR            p15,4,c2,c1,2   /* Virtualization Translation Control Register */
#define VTTBR           p15,6,c2        /* Virtualization Translation Table Base Register */

/* CP15 CR3: Domain Access Control Register */
#define DACR            p15,0,c3,c0,0   /* Domain Access Control Register */

/* CP15 CR4: */

/* CP15 CR5: Fault Status Registers */
#define DFSR            p15,0,c5,c0,0   /* Data Fault Status Register */
#define IFSR            p15,0,c5,c0,1   /* Instruction Fault Status Register */
#define ADFSR           p15,0,c5,c1,0   /* Auxiliary Data Fault Status Register */
#define AIFSR           p15,0,c5,c1,1   /* Auxiliary Instruction Fault Status Register */
#define HSR             p15,4,c5,c2,0   /* Hyp. Syndrome Register */

/* CP15 CR6: Fault Address Registers */
#define DFAR            p15,0,c6,c0,0   /* Data Fault Address Register  */
#define IFAR            p15,0,c6,c0,2   /* Instruction Fault Address Register */
#define HDFAR           p15,4,c6,c0,0   /* Hyp. Data Fault Address Register */
#define HIFAR           p15,4,c6,c0,2   /* Hyp. Instruction Fault Address Register */
#define HPFAR           p15,4,c6,c0,4   /* Hyp. IPA Fault Address Register */

/* CP15 CR7: Cache and address translation operations */
#define PAR             p15,0,c7        /* Physical Address Register */
#define LPWFI           p15,0,c7,c0,4   /* wait for interrupt */
#define ICIALLUIS       p15,0,c7,c1,0   /* Invalidate all instruction caches to PoU inner shareable */
#define BPIALLIS        p15,0,c7,c1,6   /* Invalidate entire branch predictor array inner shareable */
#define ICIALLU         p15,0,c7,c5,0   /* Invalidate all instruction caches to PoU */
#define ICIMVAU         p15,0,c7,c5,1   /* Invalidate instruction caches by MVA to PoU */
#define BPIALL          p15,0,c7,c5,6   /* Invalidate entire branch predictor array */
#define BPIMVA          p15,0,c7,c5,7   /* Invalidate MVA from branch predictor array */
#define DCIMVAC         p15,0,c7,c6,1   /* Invalidate data cache line by MVA to PoC */
#define DCISW           p15,0,c7,c2,1   /* Invalidate data cache line by set/way */
#define ATS1CPR         p15,0,c7,c8,0   /* Address Translation Stage 1. Non-Secure Kernel Read */
#define ATS1CPW         p15,0,c7,c8,1   /* Address Translation Stage 1. Non-Secure Kernel Write */
#define ATS1CUR         p15,0,c7,c8,2   /* Address Translation Stage 1. Non-Secure User Read */
#define ATS1CUW         p15,0,c7,c8,3   /* Address Translation Stage 1. Non-Secure User Write */
#define ATS12NSOPR      p15,0,c7,c8,4   /* Address Translation Stage 1+2 Non-Secure Kernel Read */
#define ATS12NSOPW      p15,0,c7,c8,5   /* Address Translation Stage 1+2 Non-Secure Kernel Write */
#define ATS12NSOUR      p15,0,c7,c8,6   /* Address Translation Stage 1+2 Non-Secure User Read */
#define ATS12NSOUW      p15,0,c7,c8,7   /* Address Translation Stage 1+2 Non-Secure User Write */
#define DCCMVAC         p15,0,c7,c10,1  /* Clean data or unified cache line by MVA to PoC */
#define DCCSW           p15,0,c7,c10,2  /* Clean data cache line by set/way */
#define DCCMVAU         p15,0,c7,c11,1  /* Clean data cache line by MVA to PoU */
#define DCCIMVA         p15,0,c7,c14,1  /* Clean and invalidate data cache line by MVA */
#define DCCISW          p15,0,c7,c14,2  /* Clean and invalidate data cache line by set/way */
#define ATS1HR          p15,4,c7,c8,0   /* Address Translation Stage 1 Hyp. Read */
#define ATS1HW          p15,4,c7,c8,1   /* Address Translation Stage 1 Hyp. Write */

/* CP15 CR8: TLB maintenance operations */
#define TLBIALLIS       p15,0,c8,c3,0   /* Invalidate entire TLB innrer shareable */
#define TLBIMVAIS       p15,0,c8,c3,1   /* Invalidate unified TLB entry by MVA inner shareable */
#define TLBIASIDIS      p15,0,c8,c3,2   /* Invalidate unified TLB by ASID match inner shareable */
#define TLBIMVAAIS      p15,0,c8,c3,3   /* Invalidate unified TLB entry by MVA all ASID inner shareable */
#define ITLBIALL        p15,0,c8,c5,0   /* Invalidate instruction TLB */
#define ITLBIMVA        p15,0,c8,c5,1   /* Invalidate instruction TLB entry by MVA */
#define ITLBIASID       p15,0,c8,c5,2   /* Invalidate instruction TLB by ASID match */
#define DTLBIALL        p15,0,c8,c6,0   /* Invalidate data TLB */
#define DTLBIMVA        p15,0,c8,c6,1   /* Invalidate data TLB entry by MVA */
#define DTLBIASID       p15,0,c8,c6,2   /* Invalidate data TLB by ASID match */
#define TLBIALL         p15,0,c8,c7,0   /* invalidate unified TLB */
#define TLBIMVA         p15,0,c8,c7,1   /* invalidate unified TLB entry by MVA */
#define TLBIASID        p15,0,c8,c7,2   /* invalid unified TLB by ASID match */
#define TLBIMVAA        p15,0,c8,c7,3   /* invalidate unified TLB entries by MVA all ASID */
#define TLBIALLHIS      p15,4,c8,c3,0   /* Invalidate Entire Hyp. Unified TLB inner shareable */
#define TLBIMVAHIS      p15,4,c8,c3,1   /* Invalidate Unified Hyp. TLB by MVA inner shareable */
#define TLBIALLNSNHIS   p15,4,c8,c3,4   /* Invalidate Entire Non-Secure Non-Hyp. Unified TLB inner shareable */
#define TLBIALLH        p15,4,c8,c7,0   /* Invalidate Entire Hyp. Unified TLB */
#define TLBIMVAH        p15,4,c8,c7,1   /* Invalidate Unified Hyp. TLB by MVA */
#define TLBIALLNSNH     p15,4,c8,c7,4   /* Invalidate Entire Non-Secure Non-Hyp. Unified TLB */

/* CP15 CR9: */

/* CP15 CR10: */
#define MAIR0           p15,0,c10,c2,0  /* Memory Attribute Indirection Register 0 AKA PRRR */
#define MAIR1           p15,0,c10,c2,1  /* Memory Attribute Indirection Register 1 AKA NMRR */
#define HMAIR0          p15,4,c10,c2,0  /* Hyp. Memory Attribute Indirection Register 0 */
#define HMAIR1          p15,4,c10,c2,1  /* Hyp. Memory Attribute Indirection Register 1 */

/* CP15 CR11: DMA Operations for TCM Access */

/* CP15 CR12:  */
#define VBAR            p15,0,c12,c0,0  /* Vector Base Address Register */
#define HVBAR           p15,4,c12,c0,0  /* Hyp. Vector Base Address Register */

/* CP15 CR13:  */
#define FCSEIDR         p15,0,c13,c0,0  /* FCSE Process ID Register */
#define CONTEXTIDR      p15,0,c13,c0,1  /* Context ID Register */
#define TPIDRURW        p15,0,c13,c0,2  /* Software Thread ID, User, R/W */
#define TPIDRURO        p15,0,c13,c0,3  /* Software Thread ID, User, R/O */
#define TPIDRPRW        p15,0,c13,c0,4  /* Software Thread ID, Priveleged */
#define HTPIDR          p15,4,c13,c0,2  /* HYp Software Thread Id Register */

/* CP15 CR14:  */
#define CNTPCT          p15,0,c14       /* Time counter value */
#define CNTFRQ          p15,0,c14,c0,0  /* Time counter frequency */
#define CNTKCTL         p15,0,c14,c1,0  /* Time counter kernel control */
#define CNTP_TVAL       p15,0,c14,c2,0  /* Physical Timer value */
#define CNTP_CTL        p15,0,c14,c2,1  /* Physical Timer control register */
#define CNTVCT          p15,1,c14       /* Time counter value + offset */
#define CNTP_CVAL       p15,2,c14       /* Physical Timer comparator */
#define CNTV_CVAL       p15,3,c14       /* Virt. Timer comparator */
#define CNTVOFF         p15,4,c14       /* Time counter offset */
#define CNTHCTL         p15,4,c14,c1,0  /* Time counter hyp. control */
#define CNTHP_TVAL      p15,4,c14,c2,0  /* Hyp. Timer value */
#define CNTHP_CTL       p15,4,c14,c2,1  /* Hyp. Timer control register */
#define CNTV_TVAL       p15,0,c14,c3,0  /* Virt. Timer value */
#define CNTV_CTL        p15,0,c14,c3,1  /* Virt. TImer control register */
#define CNTHP_CVAL      p15,6,c14       /* Hyp. Timer comparator */

/* CP15 CR15: Implementation Defined Registers */

#endif  // __ASM_ARM_CPREGS_H

#endif

