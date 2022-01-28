#ifndef GIC400_H_
#define GIC400_H_

/*
#define GIC_DIST_BASE  0xF8F01000
#define GIC_CPU_BASE   0xF8F00100
*/
//////////////////////////////
#define GIC_CPU_OFFSET  0x2000
#define GIC_DIST_OFFSET 0x1000

//GIC CPU
#define ICCICR    0x00  /* ICCICR: CPU Interface Control Register */
#define ICCPMR    0x04  /* ICCPMR: Interrupt Priority Mask Register */
#define ICCBPR    0x08  /* ICCBPR: Binary Point Register */
#define ICCIAR    0x0c  /* ICCIAR: Interrupt Acknowledge Register */
#define ICCEOIR   0x10  /* ICCEOIR: End Of Interrupt Register */
#define ICCRPR    0x14  /* ICCRPR: Running Priority Register */
#define ICCHPIR   0x18  /* ICCHPIR: Highest Pending Interrupt Register */
#define ICCABPR   0x1c  /* Aliased Non-secure Binary Point Register */
#define ICCIIDR   0xfc  /* CPU Interface Implementer Identification Register */

 
#define ICDDCR    0x00    /* ICDDCR: Distributor Control Register */
#define ICDICTR   0x04    /* ICDICTR: Interrupt Controller Type Register */
#define ICDIIDR   0x08    /* ICDIIDR: Distributor Implementer Identification Register */
#define ICDISRn   0x80    /* ICDISRn: Interrupt Security Registers */
#define ICDISERn  0x100   /* ICDISERn: Interrupt Set-Enable Registers */
#define ICDICERn  0x180   /* ICDICERn: Interrupt Clear-Enable Registers */
#define ICDISPRn  0x200   /* ICDISPRn: Interrupt Set-Pending Registers */
#define ICDICPRn  0x280   /* ICDICPRn: Interrupt Clear-Pending Registers */
#define ICDABRn   0x300   /* ICDABRn: Active Bit Registers */
#define ICDIPRn   0x400   /* ICDIPRn: Interrupt Priority Registers */
#define ICDIPTRn  0x800   /* ICDIPTRn: Interrupt Processor Targets Registers */
#define ICDICFRn  0xc00   /* ICDICFRn: Interrupt Configuration Registers */             


#define GIC_CPU_CTRL            0x00   /* ICCICR: CPU Interface Control Register */
#define GIC_CPU_PRIMASK         0x04   /* ICCPMR: Interrupt Priority Mask Register */
#define GIC_CPU_BINPOINT        0x08   /* ICCBPR: Binary Point Register */
#define GIC_CPU_INTACK          0x0c   /* ICCIAR: Interrupt Acknowledge Register */
#define GIC_CPU_EOI             0x10   /* ICCEOIR: End Of Interrupt Register */
#define GIC_CPU_RUNNINGPRI      0x14   /* ICCRPR: Running Priority Register */
#define GIC_CPU_HIGHPRI         0x18   /* ICCHPIR: Highest Pending Interrupt Register */
                                       /* Aliased Non-secure Binary Point Register */
                                       /* CPU Interface Implementer Identification Register */



#define GIC_DIST_CTRL           0x000  /* ICDDCR: Distributor Control Register */
#define GIC_DIST_CTR            0x004  /* ICDICTR: Interrupt Controller Type Register */
                                       /* ICDIIDR: Distributor Implementer Identification Register */
                                       /* ICDISRn: Interrupt Security Registers */
#define GIC_DIST_ENABLE_SET     0x100  /* ICDISERn: Interrupt Set-Enable Registers */
#define GIC_DIST_ENABLE_CLEAR   0x180  /* ICDICERn: Interrupt Clear-Enable Registers */
#define GIC_DIST_PENDING_SET    0x200  /* ICDISPRn: Interrupt Set-Pending Registers */
#define GIC_DIST_PENDING_CLEAR  0x280  /* ICDICPRn: Interrupt Clear-Pending Registers */
#define GIC_DIST_ACTIVE_BIT     0x300  /* ICDABRn: Active Bit Registers */
#define GIC_DIST_PRI            0x400  /* ICDIPRn: Interrupt Priority Registers */
#define GIC_DIST_TARGET         0x800  /* ICDIPTRn: Interrupt Processor Targets Registers */
#define GIC_DIST_CONFIG         0xc00  /* ICDICFRn: Interrupt Configuration Registers */             
#define GIC_DIST_SOFTINT        0xf00


void gic_init(void);

#endif
