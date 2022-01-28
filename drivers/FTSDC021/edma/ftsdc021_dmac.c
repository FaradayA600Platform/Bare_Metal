#include "../core/ftsdc021.h"
#include <dmac020.h>

int FTSDC021_DMA_LLD_STRUCT_ADDR;

void edma_init(void) {
        fLib_InitDMA(FALSE, FALSE, 1);
	FTSDC021_DMA_LLD_STRUCT_ADDR = (uintptr_t) malloc(0x2000);
#ifdef CONFIG_PLATFORM_A320
        outw(0x98100028, 0x18000);  //change the GPIO 10-12 function as dma_ack0 and dma_req0
        // Set the GPIO 1 is output
        *((volatile uint32_t *)(0x98700000 + 0x08)) |= 0x2;
#endif
}

uint32_t ftsdc021_dma_channel = 0;

/* DMA_NoLLD means there is no Link List Descriptor, the size that DMA will
 * work on must smaller than one LLP can move. This function only used to
 * configure the registers, not yet starts the DMA. You have to use
 * fLib_EnableDMAChannel() to start the DMA. And you can use DMA_WaitIntStatus()
 * to wait until fihish. */
/* SrcAddr: source begin address
 * DstAddr: dest begin address
 * Size: total bytes
 * SrcSize: source burst size
 * SrcCtrl: source address change : Inc/dec/fixed --> 0/1/2
 * DstCtrl: dest address change : Inc/dec/fixed --> 0/1/2 */
static uint32_t ftsdc021_DMA_NoLLD(uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size,
				 uint32_t SrcWidth, uint32_t DstWidth,
				 uint32_t SrcSize, uint32_t SrcCtrl, uint32_t DstCtrl)
{
	fLib_DMA_CH_t DMAChannel;

	/* program channel CSR */
	DMAChannel.csr.tc_msk = 0;
	DMAChannel.csr.dst_sel = 0;
	DMAChannel.csr.src_sel = 0;
	DMAChannel.csr.reserved0 = 0;
	DMAChannel.csr.priority = 3;
	DMAChannel.csr.prot = 0;
	DMAChannel.csr.src_size = SrcSize;
	DMAChannel.csr.abt = 0;
	DMAChannel.csr.reserved1 = 0;
	DMAChannel.csr.src_width = SrcWidth;
	DMAChannel.csr.dst_width = DstWidth;
	DMAChannel.csr.mode = 1;
	DMAChannel.csr.src_ctrl = SrcCtrl;
	DMAChannel.csr.dst_ctrl = DstCtrl;

	fLib_SetDMAChannelCfg(ftsdc021_dma_channel, DMAChannel.csr);

	/* program channel CFG */
	DMAChannel.cfg.int_tc_msk = 0;
	DMAChannel.cfg.int_err_msk = 0;
	DMAChannel.cfg.int_abt_msk = 0;
	DMAChannel.cfg.busy = 0;
	DMAChannel.cfg.reserved1 = 0;
	DMAChannel.cfg.llp_cnt = 0;
	DMAChannel.cfg.reserved2 = 0;

	fLib_DMA_CHIntMask(ftsdc021_dma_channel, DMAChannel.cfg);

	/* program channel llp */
	DMAChannel.link_list_addr = 0;  //no LLP, this is to set link_list_addr = 0
	fLib_DMA_CHLinkList(ftsdc021_dma_channel, DMAChannel.link_list_addr);

	/* porgram address and size */
	fLib_DMA_CHDataCtrl(ftsdc021_dma_channel, SrcAddr, DstAddr, Size / (1 << SrcWidth));

	DMAChannel.link_list_addr = 1;

	return 0;
}

/*
 * LLPCount: total link-list node
 * SrcAddr: source begin address
 * DstAddr: dest begin address
 * Size: total bytes
 * SrcCtrl: source address change : Inc/dec/fixed --> 0/1/2
 * DstCtrl: dest address change : Inc/dec/fixed --> 0/1/2
 */
static void ftsdc021_DMA_FillMemLLD(uint32_t LLPSize, uint32_t LLPCount,
				    uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size,
				    uint32_t SrcWidth, uint32_t DstWidth,
				    uint32_t SrcCtrl, uint32_t DstCtrl)
{
	uint32_t i;
	fLib_DMA_LLD_t *LLP = (fLib_DMA_LLD_t *) FTSDC021_DMA_LLD_STRUCT_ADDR;

	for (i = 0; i < LLPCount; i++) {
		if (SrcCtrl == AHBDMA_SrcInc)	// increase
			LLP[i].src_addr = (uint32_t)SrcAddr +
					  ((i + 1) * LLPSize * (1 << SrcWidth));
		else if (SrcCtrl == AHBDMA_SrcDec)	// decrease
			LLP[i].src_addr = (uint32_t)SrcAddr - ((i + 1) *
					   LLPSize * (1 << SrcWidth));
		else if (SrcCtrl == AHBDMA_SrcFix)	// fixed
			LLP[i].src_addr = (uint32_t)SrcAddr;

		if (DstCtrl == AHBDMA_DstInc)
			LLP[i].dst_addr = (uint32_t)DstAddr + ((i + 1) *
					   LLPSize * (1 << DstWidth));
		else if (DstCtrl == AHBDMA_DstDec)	// Decrease
			LLP[i].dst_addr = (uint32_t)DstAddr - ((i + 1) *
					   LLPSize * (1 << DstWidth));
		else if (DstCtrl == AHBDMA_DstFix)
			LLP[i].dst_addr = (uint32_t)DstAddr;

		*((uint32_t *) & (LLP[i].llp_ctrl)) = 0;
		LLP[i].llp_ctrl.dst_sel = 0;
		LLP[i].llp_ctrl.src_sel = 0;
		LLP[i].llp_ctrl.dst_ctrl = DstCtrl;
		LLP[i].llp_ctrl.src_ctrl = SrcCtrl;
		LLP[i].llp_ctrl.dst_width = DstWidth;
		LLP[i].llp_ctrl.src_width = SrcWidth;

		LLP[i].link_list_addr = 0;
		if (i == (LLPCount - 1)) {
			//the last LLP
			LLP[i].link_list_addr = 0;
			LLP[i].llp_ctrl.tc_msk = 0;	// Enable tc status
#if defined CONFIG_PLATFORM_AHB
			LLP[i].size = (Size - LLPSize * (LLPCount) *
				      (1 << SrcWidth)) / (1 << SrcWidth);
#elif defined CONFIG_PLATFORM_A320
			LLP[i].llp_ctrl.size = (Size - LLPSize * (LLPCount) *
					       (1 << SrcWidth)) / (1 << SrcWidth);
#else
#error  "Specify correct platform name."
#endif
		} else {
			LLP[i].link_list_addr = ((uint32_t) & LLP[i + 1]) >> 2;
			LLP[i].llp_ctrl.tc_msk = 1;
#if defined CONFIG_PLATFORM_AHB
			LLP[i].size = LLPSize;
#elif defined CONFIG_PLATFORM_A320
			LLP[i].llp_ctrl.size = LLPSize;
#else
#error  "Specify correct platform name."
#endif
		}
	}

	return;
}

static uint32_t ftsdc021_DMA_NormalMode(uint32_t LLPSize, uint32_t LLPCount,
				      uint32_t SrcAddr, uint32_t DstAddr,
				      uint32_t Size, uint32_t SrcWidth,
				      uint32_t DstWidth, uint32_t SrcSize,
				      uint32_t SrcCtrl, uint32_t DstCtrl)
{
	fLib_DMA_CH_t DMAChannel;

	fLib_DMA_ResetChannel(ftsdc021_dma_channel);
	fLib_ClearDMAChannelIntStatus(ftsdc021_dma_channel);

	if (LLPCount >= 1) {
		ftsdc021_DMA_FillMemLLD(LLPSize, LLPCount, SrcAddr, DstAddr,
					Size, SrcWidth, DstWidth, SrcCtrl,
					DstCtrl);
		/* program channel CSR */
		DMAChannel.csr.enable = 0;
		DMAChannel.csr.dst_sel = 0;
		DMAChannel.csr.src_sel = 0;
		DMAChannel.csr.dst_ctrl = DstCtrl;
		DMAChannel.csr.src_ctrl = SrcCtrl;
		DMAChannel.csr.mode = 1;
		DMAChannel.csr.dst_width = DstWidth;
		DMAChannel.csr.src_width = SrcWidth;
		DMAChannel.csr.reserved1 = 0;
		DMAChannel.csr.abt = 0;
		DMAChannel.csr.src_size = SrcSize;
		DMAChannel.csr.prot = 0;
		DMAChannel.csr.priority = 3;
		DMAChannel.csr.reserved0 = 0;
		DMAChannel.csr.tc_msk = 1;

		fLib_SetDMAChannelCfg(ftsdc021_dma_channel, DMAChannel.csr);

		/* program channel CFG */
		DMAChannel.cfg.int_tc_msk = 0;
		DMAChannel.cfg.int_err_msk = 0;
		DMAChannel.cfg.int_abt_msk = 0;

		DMAChannel.cfg.busy = 0;
		DMAChannel.cfg.reserved1 = 0;
		DMAChannel.cfg.llp_cnt = 0;
		DMAChannel.cfg.reserved2 = 0;

		fLib_DMA_CHIntMask(ftsdc021_dma_channel, DMAChannel.cfg);

		/* program channel llp */
		DMAChannel.link_list_addr = 0;  //Init IP llp
		DMAChannel.link_list_addr = FTSDC021_DMA_LLD_STRUCT_ADDR >> 2;
		fLib_DMA_CHLinkList(ftsdc021_dma_channel,
				    DMAChannel.link_list_addr);

		/* porgram address and size */
		fLib_DMA_CHDataCtrl(ftsdc021_dma_channel, SrcAddr, DstAddr,
				    LLPSize);

	} else {
		return ftsdc021_DMA_NoLLD(SrcAddr, DstAddr, Size, SrcWidth,
					  DstWidth, SrcSize, SrcCtrl, DstCtrl);
	}

	return 0;
}

static uint32_t ftsdc021_data_getDmacMaxSize(uint32_t src_width, uint32_t burst)
{
	uint32_t sz;
	
	if (burst == 0)
		sz = (4096 - ((1) * (1 << src_width)));
	else
		sz = (4096 - ((1 << (burst + 1)) * (1 << src_width)));

	return sz;

}

int ftsdc021_Start_DMA(uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size,
                       uint32_t SrcWidth, uint32_t DstWidth, uint32_t SrcSize,
                       uint32_t SrcCtrl, uint32_t DstCtrl)
{
	uint32_t LLPSize, Count;

	fLib_DisableDMAChannel(ftsdc021_dma_channel);

	LLPSize = ftsdc021_data_getDmacMaxSize(SrcWidth, SrcSize);
	if ((Size / (1 << SrcWidth)) > LLPSize) {
		Count = ((Size + (LLPSize - 1) * (1 << SrcWidth)) /
			 (1 << SrcWidth)) / LLPSize;
	} else {
		Count = 1;
	}

	if (ftsdc021_DMA_NormalMode(LLPSize, Count - 1, SrcAddr, DstAddr, Size,
				    SrcWidth, DstWidth, SrcSize, SrcCtrl,
				    DstCtrl)) {
		return 1;
	}

	fLib_EnableDMAChannel(ftsdc021_dma_channel);

	return 0;
}

int ftsdc021_wait_DMA_done(uint32_t wait_t)
{
	uint32_t ret;
	clock_t t0;

	t0 = clock();
	do {
		ret = fLib_GetDMAChannelIntStatus(ftsdc021_dma_channel);
		if (ret & 0x2)
			return -1;
		else if (ret & 0x1)
			return 0;
	} while (clock() - t0 < wait_t);

	return -1;
}
