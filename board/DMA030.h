
#ifndef __AHB_DMA030_H
#define __AHB_DMA030_H



extern volatile UINT32	bWaitIntCB;

#define IPMODULE 	DMAC
#define IPNAME   	FTDMAC030

#if 0
unsigned *dma030_base_table = DMAC_FTDMAC030_pa_base;
unsigned **dma030_irq_table = DMAC_FTDMAC030_irq;
unsigned *dma030_ldm_base_table = RAM_FTDMAC030_pa_base;
#endif

extern UINT32	dma_instance_N;

#define DMA030_BASE					DMAC_FTDMAC030_pa_base[dma_instance_N]
#if (DMAC_FTDMAC030_COUNT>1)
#define IRQ_DMA030					DMAC_FTDMAC030_irq[dma_instance_N][0] 
#else
#define IRQ_DMA030					DMAC_FTDMAC030_irq[0] 
#endif


#define DMA030_LLD_SYSMEM			(FREE_MEM_BASE+0x3000000)

#ifdef RAM_FTDMAC030_PA_BASE
    #define DMA030_LDM_BASE				RAM_FTDMAC030_pa_base[dma_instance_N]
#else
    #define DMA030_LDM_BASE				0   //no LDM in system
#endif


#define CHANNEL_NUM CONFIG_FTDMAC030_CHANNEL_COUNT

#define SRCADDR		FREE_MEM_BASE 
#define DSTADDR		(FREE_MEM_BASE+0x1000000)
#define DSTADDR2	(FREE_MEM_BASE+0x2000000)

#if 0
#ifdef CONFIG_PLATFORM_FH36

#define DMA030_BASE					IP_PA_BASE(0)
#define DMA030_LDM_BASE				0x00000000
#define DMA030_LLD_SYSMEM				0x88000000
#define IRQ_DMA030					IP_IRQ(0) //31
#define CHANNEL_NUM 4

#elif CONFIG_PLATFORM_LTE
#define DMA030_BASE					IP_PA_BASE(4)
#define DMA030_LDM_BASE				(FREE_MEM_BASE+0x3000000)
#define DMA030_LLD_SYSMEM			(FREE_MEM_BASE+0x4000000)
#define IRQ_DMA030					16
#define CHANNEL_NUM 8

#else //other platform
#define DMA030_BASE					IP_PA_BASE(0)
#define DMA030_LDM_BASE				(FREE_MEM_BASE+0x3000000)
#define DMA030_LLD_SYSMEM			(FREE_MEM_BASE+0x4000000)
#define IRQ_DMA030					IP_IRQ(0)
#define CHANNEL_NUM 4
#endif
#endif

/* ycmo
#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80
#define BIT8			0x0100
#define BIT9			0x0200
#define BIT10		0x0400
#define BIT11		0x0800
#define BIT12		0x1000
#define BIT13		0x2000
#define BIT14		0x4000
#define BIT15		0x8000
#define BIT16		0x010000
#define BIT17		0x020000
#define BIT18		0x040000
#define BIT19		0x080000
#define BIT20		0x100000
#define BIT21		0x200000
#define BIT22		0x400000
#define BIT23		0x800000
#define BIT24		0x01000000
#define BIT25		0x02000000
#define BIT26		0x04000000
#define BIT27		0x08000000
#define BIT28		0x10000000
#define BIT29		0x20000000
#define BIT30		0x40000000
#define BIT31		0x80000000
*/

//ycmo
#undef DMA_CSR_TC_MSK
#undef DMA_CSR_SRC_WIDTH_8
#undef DMA_CSR_SRC_WIDTH_16
#undef DMA_CSR_SRC_WIDTH_32
#undef DMA_CSR_DST_WIDTH_8
#undef DMA_CSR_DST_WIDTH_16
#undef DMA_CSR_DST_WIDTH_32
#undef AHBDMA_SrcFix
#undef AHBDMA_DstFix
#undef AHBDMA_PriorityHigh

/* registers */
#define DMA_INT						0x0
#define DMA_INT_TC					0x4
#define DMA_INT_TC_CLR				0x8
#define DMA_INT_ERRABT				0xC
#define DMA_INT_ERRABT_CLR			0x10
#define DMA_TC						0x14
#define DMA_ERRABT					0x18
#define DMA_CH_EN					0x1C
#define DMA_CH_SYNC				0x20
#define DMA_LDM_BASE				0x24
#define DMA_WDT					0x28
#define DMA_GE						0x2C
#define DMA_FEATURE					0x38 



#define DMA_CHANNEL_OFFSET			0x20
#define DMA_CHANNEL0_BASE			0x100
#define DMA_CHANNEL1_BASE			0x120
#define DMA_CHANNEL2_BASE			0x140
#define DMA_CHANNEL3_BASE			0x160
#define DMA_CHANNEL4_BASE			0x180
#define DMA_CHANNEL5_BASE			0x1a0
#define DMA_CHANNEL6_BASE			0x1c0
#define DMA_CHANNEL7_BASE			0x1e0

#define DMA_CHANNEL_CSR_OFFSET		0x0
#define DMA_CHANNEL_CFG_OFFSET		0x4
#define DMA_CHANNEL_SRCADDR_OFFSET	0x8
#define DMA_CHANNEL_DSTADDR_OFFSET	0xc
#define DMA_CHANNEL_LLP_OFFSET		0x10
#define DMA_CHANNEL_SIZE_OFFSET		0x14
#define DMA_CHANNEL_STRIDE_OFFSET		0x18


/* bit mapping of main configuration status register(CSR 0x24) */
#define DMA_CSR_M1ENDIAN			0x00000004
#define DMA_CSR_M0ENDIAN			0x00000002
#define DMA_CSR_DMACEN				0x00000001

/* bit mapping of channel control register 0x100 */
#define DMA_CSR_SRCTCNY_1		0x0
#define DMA_CSR_SRCTCNY_2		BIT29
#define DMA_CSR_SRCTCNY_4		BIT30
#define DMA_CSR_SRCTCNY_8		BIT30 | BIT29
#define DMA_CSR_SRCTCNY_16	BIT31
#define DMA_CSR_SRCTCNY_32	BIT31 | BIT29
#define DMA_CSR_SRCTCNY_64	BIT31 | BIT30
#define DMA_CSR_SRCTCNY_128	BIT31 | BIT30 | BIT29

#define DMA_CSR_TC_MSK				BIT28

#define DMA_CSR_SRC_WIDTH_8		0x0
#define DMA_CSR_SRC_WIDTH_16		BIT25
#define DMA_CSR_SRC_WIDTH_32		BIT26
#define DMA_CSR_SRC_WIDTH_64		BIT25 |  BIT26


#define DMA_CSR_DST_WIDTH_8		0x0
#define DMA_CSR_DST_WIDTH_16		BIT22
#define DMA_CSR_DST_WIDTH_32		BIT23
#define DMA_CSR_DST_WIDTH_64		BIT22 | BIT23


#define DMA_CSR_SRCCTRL_Fix		BIT20
#define DMA_CSR_DSTCTRL_Fix		BIT18
#define DMA_CSR_WDT_EN			BIT17
#define DMA_CSR_CH_EN				BIT16
#define DMA_CSR_EXP_EN				BIT15
#define DMA_CSR_2D_EN				BIT14
#define DMA_CSR_WEVENT_EN			BIT13
#define DMA_CSR_SEVENT_EN			BIT12
#define DMA_CSR_WSYNC				BIT8
/*END  bit mapping of channel control register 0x100 */

/* bit mapping of channel control register 0x104 */

#define DMA_CSR_CHPRI_HI			BIT28
/* hardware feature register*/


/*END  bit mapping of channel control register 0x104 */
#define DMA_CSR_CHPRJ_HIGHEST		0x00C00000
#define DMA_CSR_CHPRJ_2ND			0x00800000
#define DMA_CSR_CHPRJ_3RD			0x00400000
#define DMA_CSR_PRTO3				0x00200000
#define DMA_CSR_PRTO2				0x00100000
#define DMA_CSR_PRTO1				0x00080000


#define DMA_CSR_ABT				0x00008000



#define DMA_CSR_MODE_NORMAL			0x00000000
#define DMA_CSR_MODE_HANDSHAKE		0x00000080

#define DMA_CSR_SRC_INCREMENT		0x00000000
#define DMA_CSR_SRC_DECREMENT		0x00000020
#define DMA_CSR_SRC_FIX				0x00000040

#define DMA_CSR_DST_INCREMENT		0x00000000
#define DMA_CSR_DST_DECREMENT		0x00000008
#define DMA_CSR_DST_FIX				0x00000010

#define DMA_CSR_SRC_SEL				0x00000004
#define DMA_CSR_DST_SEL				0x00000002
#define DMA_CSR_CH_ENABLE			0x00000001

#define DMA_CSR_DMA_FF_TH			0x07000000   //new
#define DMA_CSR_CHPR1				0x00C00000
#define DMA_CSR_SRC_SIZE			0x00070000
#define DMA_CSR_SRC_WIDTH			0x00003800
#define DMA_CSR_DST_WIDTH			0x00000700
#define DMA_CSR_SRCAD_CTL			0x00000060
#define DMA_CSR_DSTAD_CTL			0x00000018

/* bit mapping of channel configuration register 0x104 */
#define DMA_CFG_INT_ABT_MSK	        0x00000004   // new
#define DMA_CFG_INT_ERR_MSK	        0x00000002   // new
#define DMA_CFG_INT_TC_MSK	        0x00000001   // new

///////////////////////////////////////////////////
/* bit mapping of Linked List Control Descriptor */

#define DMA_LLP_DMA_FF_TH			0xE0000000   // new

#define DMA_LLP_TC_MSK				0x10000000

#define DMA_LLP_SRC_WIDTH_8			0x00000000
#define DMA_LLP_SRC_WIDTH_16		0x02000000
#define DMA_LLP_SRC_WIDTH_32		0x04000000
#define DMA_LLP_SRC_WIDTH_64		0x06000000   // new

#define DMA_LLP_DST_WIDTH_8			0x00000000
#define DMA_LLP_DST_WIDTH_16		0x00400000
#define DMA_LLP_DST_WIDTH_32		0x00800000
#define DMA_LLP_DST_WIDTH_64		0x00C00000   // new

#define DMA_LLP_SRC_INCREMENT		0x00000000
#define DMA_LLP_SRC_DECREMENT		0x00100000
#define DMA_LLP_SRC_FIX				0x00200000

#define DMA_LLP_DST_INCREMENT		0x00000000
#define DMA_LLP_DST_DECREMENT		0x00040000
#define DMA_LLP_DST_FIX				0x00080000

#define DMA_LLP_SRC_SEL				0x00020000
#define DMA_LLP_DST_SEL				0x00010000

///////////////////////////////////////#define DMA_MAX_SIZE				0x10000
///////////////////////////////////////#define DMA_CHANNEL_NUMBER			8

/////////////////////////// AHB DMA Define //////////////////////////////////
#define AHBDMA_Channel0					0
#define AHBDMA_Channel1					1
#define AHBDMA_Channel2					2
#define AHBDMA_Channel3					3
#define AHBDMA_Channel4					4
#define AHBDMA_Channel5					5
#define AHBDMA_Channel6					6
#define AHBDMA_Channel7					7

#define AHBDMA_SrcWidth_Byte			0
#define AHBDMA_SrcWidth_Word			1
#define AHBDMA_SrcWidth_DWord			2

#define AHBDMA_DstWidth_Byte			0
#define AHBDMA_DstWidth_Word			1
#define AHBDMA_DstWidth_DWord			2

#define AHBDMA_Burst1					0
#define AHBDMA_Burst4					1
#define AHBDMA_Burst8					2
#define AHBDMA_Burst16					3
#define AHBDMA_Burst32					4
#define AHBDMA_Burst64					5
#define AHBDMA_Burst128					6
#define AHBDMA_Burst256					7

#define AHBDMA_NormalMode				0
#define AHBDMA_HwHandShakeMode			1

#define AHBDMA_SrcInc					0
#define AHBDMA_SrcFix					2

#define AHBDMA_DstInc					0
#define AHBDMA_DstFix					2

#define AHBDMA_PriorityLow				0
#define AHBDMA_PriorityHigh				1



// feature register
typedef struct
{
	UINT32 ch_num		:3;
	UINT32 reserved3	:1;
    UINT32 d_width		:1;
	UINT32 reserved5	:3;
	UINT32 dfifo_depth	:3;
	UINT32 reserved11	:1;
	UINT32 pri_on		:1;
	UINT32 reserved13	:3;
	UINT32 pri_num		:4;
	UINT32 ldm_on		:1;
	UINT32 reserved21	:3;
	UINT32 ldm_depth	:2;
	UINT32 reserved26	:2;
	UINT32 cmd_depth	:2;
	UINT32 reserved30	:2;
} DMA_FEATURE_t;	

typedef struct
{
	UINT32 ch_wevent:8;
	UINT32 wsync:1;
	UINT32 ch_sevent:3;
	UINT32 sevent_en:1;
	UINT32 wevent_en:1;
	UINT32 twod_en:1;
	UINT32 exp_en:1;
	UINT32 ch_en:1;
	UINT32 wdt_en:1;
	UINT32 dst_ctrl:2;
	UINT32 src_ctrl:2;
	UINT32 dst_width:3;
	UINT32 src_width:3;
	UINT32 tc_msk:1;
	UINT32 src_tcnt:3;
}DMA_CH_CSR_t;//DMAC030

typedef struct
{
	UINT32 int_tc_msk:1;
	UINT32 int_err_msk:1;
	UINT32 int_abt_msk:1;   
	UINT32 src_rs:4;	
	UINT32 src_hen:1;
	UINT32 reserved:1;
	UINT32 dst_rs:4;
	UINT32 dst_hen:1;
	UINT32 reserved1:2;
	UINT32 llp_cnt:4;
	UINT32 ch_gntwin:8;
	UINT32 ch_pri:1;
	UINT32 reserved2:3;
}DMA_CH_CFG_t;//DMAC030

typedef struct
{
	UINT32 link_list_addr:32;
}DMA_CH_LLP_t;

typedef struct
{
	UINT32 ch_wevent:8;
	UINT32 wsync:1;
	UINT32 ch_sevent:3;
	UINT32 sevent_en:1;
	UINT32 wevent_en:1;
	UINT32 twod_en:1;
	UINT32 exp_en:1;
	UINT32 ch_en:1;
	UINT32 wdt_en:1;
	UINT32 dst_ctrl:2;
	UINT32 src_ctrl:2;
	UINT32 dst_width:3;
	UINT32 src_width:3;
	UINT32 tc_msk:1;
	UINT32 src_tcnt:3;		//new
}DMA_LLP_CTRL_t;

typedef struct
{
	volatile DMA_CH_CSR_t csr;
	volatile DMA_CH_CFG_t cfg;
	volatile UINT32 src_addr;
	volatile UINT32 dst_addr;
	volatile DMA_CH_LLP_t llp;
	volatile UINT32 size;		//the unit is SrcWidth
	volatile UINT32 stride;
	volatile UINT32 dummy;
}DMA_CH_t;

typedef struct
{
	UINT32 src_addr;
	UINT32 dst_addr;
	DMA_CH_LLP_t llp;
	DMA_LLP_CTRL_t llp_ctrl;
	UINT32 TotalSize;		//new, the unit is SrcWidth, only 21 bits is used
	UINT32 Stride;
	UINT32 Dummy[2];		//this is for program to look nice.
}DMA_LLD_t;

/*typedef struct
{
	UINT64 src_addr;
	//UINT32 dst_addr;
	//DMA_CH_LLP_t llp;
	//DMA_LLP_CTRL_t llp_ctrl;
	UINT64 llp;
	UINT64 TotalSize;		//new, the unit is SrcWidth, only 21 bits is used
	//UINT32 Stride;
	UINT64 Dummy;		//this is for program to look nice.
}DMA_LLD_t_64;
*/
typedef struct
{
	volatile UINT32 dma_int;
	volatile UINT32 dma_int_tc;
	volatile UINT32 dma_int_tc_clr;
	volatile UINT32 dma_int_err;
	volatile UINT32 dma_int_err_clr;
	volatile UINT32 dma_tc;
	volatile UINT32 dma_err;
	volatile UINT32 dma_ch_enable;
	volatile UINT32 dma_sync_pi;
	volatile UINT32 dma_ldm;
	volatile UINT32 dma_wdt;
	volatile UINT32 dma_ge;
	volatile UINT32 dma_plverr;
	volatile UINT32 dma_rn;
	volatile UINT32 dma_hf;
	volatile UINT32 dma_ldm_flag[4];

    volatile UINT32 dummy1[45];  
	volatile DMA_CH_t dma_ch[8];
}DMA_Reg_st;


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

extern int    IsDMAChannelBusy(INT32 Channel);
extern int    IsDMAChannelEnable(INT32 Channel);
extern UINT32 GetDMAIntStatus(void);
extern UINT32 GetDMAChannelIntStatus(INT32 Channel);
extern int    GetDMABusyStatus(void);
extern int    GetDMAEnableStatus(void);

extern void   InitDMA(UINT32 M0_BigEndian, UINT32 M1_BigEndian, UINT32 Sync);
extern void  EnableDMASync(void);
extern void   EnableDMAChannel(INT32 Channel);

extern void   DisableDMAChannel(INT32 Channel);  

extern void   ClearDMAChannelIntStatus(INT32 Channel);

extern void   SetDMAChannelCfg(INT32 Channel, DMA_CH_CSR_t Csr);
extern DMA_CH_CSR_t GetDMAChannelCfg(INT32 Channel);
extern void   DMA_CHIntMask(INT32 Channel, DMA_CH_CFG_t Mask);
extern void   DMA_CHLinkList(INT32 Channel, DMA_CH_LLP_t LLP);
extern void   DMA_CHDataCtrl(INT32 Channel, UINT32 SrcAddr, UINT32 DstAddr, UINT32 Size);
extern void   DMA_CHDataCtrl_2D(INT32 Channel, UINT32 SrcAddr, UINT32 DstAddr, UINT32 XTcnt, UINT32 YTcnt, UINT32 DstStride, UINT32 SrcStride);



extern void DMA_SetInterrupt(UINT32 channel, UINT32 tcintr, UINT32 errintr, UINT32 abtintr);
extern void DMA_ResetChannel(UINT8 channel);
extern void DMA_ClearAllInterrupt(void);

void DMA_WaitIntStatus(UINT32 Channel);
int DMA_ISChannelEnable(INT32 Channel);
int DMATest_new(UINT32 mode);
int CompareMemory(UINT32 *SrcAddr, UINT32 *DstAddr, UINT32 Size);
int CheckContent(UINT32 *DstAddr, UINT32 Size);

#endif
