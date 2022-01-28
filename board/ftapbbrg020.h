#ifndef	__FTAPBBRG020_API_H
#define __FTAPBBRG020_API_H

#define APB_CHANNEL_A					0
#define APB_CHANNEL_B					1
#define APB_CHANNEL_C					2
#define APB_CHANNEL_D					3


///////Data Width///////
#define AHB2APB_DMA_WORD				0x0
#define AHB2APB_DMA_HWORD				0x100000
#define AHB2APB_DMA_BYTE				0x200000
#define AHB2APB_DMA_DWIDTHSEL			0x300000

	///////Source address Incremental/////////
#define AHB2APB_DMA_SRC_FIX				0x0
#define AHB2APB_DMA_SRC_INC1			0x100
#define AHB2APB_DMA_SRC_INC2			0x200
#define AHB2APB_DMA_SRC_INC4			0x300
#define AHB2APB_DMA_SRC_DEC1			0x500
#define AHB2APB_DMA_SRC_DEC2			0x600
#define AHB2APB_DMA_SRC_DEC4			0x700
#define AHB2APB_DMA_SRC_INCSEL			0x700

	///////Destination address Incremental///////
#define AHB2APB_DMA_DES_FIX				0x0
#define AHB2APB_DMA_DES_INC1			0x1000
#define AHB2APB_DMA_DES_INC2			0x2000
#define AHB2APB_DMA_DES_INC4			0x3000
#define AHB2APB_DMA_DES_DEC1			0x5000
#define AHB2APB_DMA_DES_DEC2			0x6000
#define AHB2APB_DMA_DES_DEC4			0x7000
#define AHB2APB_DMA_DES_INCSEL			0x7000

	///////Destination address Selection///////
#define AHB2APB_DMA_DES_APB				0x0
#define AHB2APB_DMA_DES_AHB				0x80
#define AHB2APB_DMA_DES_SEL				0x80

	///////Source address Selection///////
#define AHB2APB_DMA_SRC_APB				0x0
#define AHB2APB_DMA_SRC_AHB				0x40
#define AHB2APB_DMA_SRC_SEL				0x40


//ycmo: platform depend
	///////Request Select///////
#define AHB2APB_DMA_REQ_NULL			0x0
#define AHB2APB_DMA_REQ_SDRam			0x0
#define AHB2APB_DMA_REQ_CFC				0x10000
#define AHB2APB_DMA_REQ_SSP1			0x20000

#define AHB2APB_DMA_REQ_SSP1_TX			0x70000
#define AHB2APB_DMA_REQ_SSP1_RX			0x80000


#define AHB2APB_DMA_REQ_SD				0x50000
#define AHB2APB_DMA_REQ_I2S				0x60000
#define AHB2APB_DMA_REQ_AC97			0x70000
#define AHB2APB_DMA_REQ_SSP2			0x80000

#define ExtAPB_DMA_REQ_EXT0				0xc0000
#define ExtAPB_DMA_REQ_EXT1				0xf0000

#define AHB2APB_DMA_REQ_UART1			0x90000
#define AHB2APB_DMA_REQ_UART2			0xa0000
#define AHB2APB_DMA_REQ_SMMC			0xc0000

#define AHB2APB_DMA_REQ_USB				0xd0000
#define AHB2APB_DMA_REQ_FUSB220			0xe0000  //modify by silas
#define AHB2APB_DMA_REQ_IrDA			0xb0000
#define AHB2APB_DMA_REQ_MMSC			0xf0000
#define AHB2APB_DMA_REQ_SEL				0x0f0f0000//modify by silas



////////////funtion define///////////////////

extern void fLib_APBDMA_SetSrcAddr(UINT32 port,UINT32 addr);
extern void fLib_APBDMA_SetDesAddr(UINT32 port,UINT32 addr);
extern void fLib_APBDMA_SetCycle(UINT32 port,UINT32 cycle);
extern void fLib_APBDMA_SetDataWidth(UINT32 port,UINT32 width);
extern void fLib_APBDMA_SetRequest(UINT32 port,UINT32 Reguest);
extern void fLib_APBDMA_SetDesInc(UINT32 port,UINT32 increment);
extern void fLib_APBDMA_SetSrcInc(UINT32 port,UINT32 increment);
extern void fLib_APBDMA_SetDesSel(UINT32 port,UINT32 select);
extern void fLib_APBDMA_SetSrcSel(UINT32 port,UINT32 select);
extern void fLib_APBDMA_EnableErrINT(UINT32 port,UINT32 enable);
extern void fLib_APBDMA_EnableBurst(UINT32 port,UINT32 enable);
extern void fLib_APBDMA_EnableFinINT(UINT32 port,UINT32 enable);
extern void fLib_APBDMA_EnableTrans(UINT32 port,UINT32 enable);
extern void fLib_APBDMA_Init(UINT32 port,UINT32 SrcAddr,UINT32 DesAddr,UINT32 cycle,UINT32 width,UINT32 burst);
extern void fLib_APBDMA_Config(UINT32 port,UINT32 request,UINT32 SrcInc,UINT32 DesInc,UINT32 SrcSel,UINT32 DesSel);
extern void fLib_APBDMA_Interrupt(UINT32 port,UINT32 error,UINT32 finish);
extern void fLib_APBDMA_ClrStatus(UINT32 port);
extern UINT32 fLib_APBDMA_ChkErr(UINT32 port);
extern UINT32 fLib_APBDMA_ChkFinish(UINT32 port);
extern void fLib_APBDMA_Init_Quick(UINT32 port,UINT32* SrcAddr,UINT32* DesAddr,UINT32 cycle,UINT32 dwConfigValue);

extern void fLib_APBDMA_ClrErrFinishStatus(UINT32 port);

#endif
