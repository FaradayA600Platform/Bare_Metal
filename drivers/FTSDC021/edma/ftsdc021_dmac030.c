#include "ftsdc021.h"
#include <dmac030.h>

int FTSDC021_DMA_LLD_STRUCT_ADDR;

void edma_init(void) {
        InitDMA();
        FTSDC021_DMA_LLD_STRUCT_ADDR = (uintptr_t) malloc(0x2000);
#ifdef CONFIG_PLATFORM_A320
        outw(0x98100028, 0x18000);  //change the GPIO 10-12 function as dma_ack0 and dma_req0
        // Set the GPIO 1 is output
        *((volatile uint32_t *)(0x98700000 + 0x08)) |= 0x2;
#endif
}

uint32_t ftsdc021_req_sel = 2;
uint32_t ftsdc021_dma_channel = 0;
uint32_t ftsdc021_dma_priority = 3;

/**
 * Attention : If use DMA Controller on A320 board, the burst size must
 * be multiple of sector size. Also check Tx and Rx FIFO threshold.
 * Example : Sector Size 512 bytes, then burst size could be 16.
 * 			 Sector Size 520 bytes, then burst size could be 1.
 */

static uint32_t ftsdc021_DMA_getMaxSize(uint32_t src_width, uint32_t busrt_size)
{
	uint32_t sz;

	if (busrt_size == 0)
		sz = (0x200000 - ((1) * (1 << src_width)));
	else
		sz = (0x200000 - ((1 << (busrt_size + 1)) * (1 << src_width)));

	return sz;

}

void ftsdc021_DMA_FillMemLLD(uint32_t LLPSize,  // Link-List address
		    uint32_t LLPCount,  // total link-list node
		    uint32_t SrcAddr,   // source begin address
		    uint32_t DstAddr,   // dest begin address
		    uint32_t Size,      // total bytes
		    uint32_t SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
		    uint32_t DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
		    uint32_t SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
		    uint32_t DstCtrl)   // dest address change : Inc/dec/fixed --> 0/1/2
{
	uint32_t i;
	DMA_LLD_t *LLP = (DMA_LLD_t *)((uintptr_t)FTSDC021_DMA_LLD_STRUCT_ADDR);

	for(i = 0; i < LLPCount ;i++) {
		if(SrcCtrl == AHBDMA_SrcInc)  // increase
			LLP[i].src_addr = (uint32_t) SrcAddr + ((i+1) * LLPSize * (1 << SrcWidth));
		else if(SrcCtrl == AHBDMA_SrcFix)	// fixed
			LLP[i].src_addr = (uint32_t) SrcAddr;

		if(DstCtrl == AHBDMA_DstInc)
			LLP[i].dst_addr = (uint32_t) DstAddr + ((i+1) * LLPSize * (1 << SrcWidth));
		else if(DstCtrl == AHBDMA_DstFix)
			LLP[i].dst_addr = (uint32_t) DstAddr;


		*((uint32_t *)&(LLP[i].llp_ctrl)) = 0;		//init llp_ctrl as 0
		LLP[i].llp_ctrl.dst_ctrl = DstCtrl; ///* destination increment, decrement or fix
		LLP[i].llp_ctrl.src_ctrl = SrcCtrl; ///* source increment, decrement or fix
		LLP[i].llp_ctrl.dst_width = DstWidth; ///* destination transfer size
		LLP[i].llp_ctrl.src_width = SrcWidth; ///* source transfer size

		*((uint32_t *)&(LLP[i].llp)) = 0;
		if (i==(LLPCount-1)) {
			//the last LLP
			LLP[i].llp.link_list_addr = 0;
			LLP[i].llp_ctrl.tc_msk = 0;	// Enable tc status
			LLP[i].TotalSize = (Size - LLPSize * (LLPCount) * (1 << SrcWidth)) / (1 << SrcWidth);
		} else {
			LLP[i].llp.link_list_addr = (uintptr_t)&LLP[i+1];
			LLP[i].llp_ctrl.tc_msk = 1;
			LLP[i].TotalSize = LLPSize;	//the unit is SrcWidth  / (1 << SrcWidth);
		}
	}

	return;
}


/*************************************************************************************
	DMA_NoLLD means there is no Link List Descriptor,
	the size that DMA will work on must smaller than one LLP can move. This function
	only used to configure the registers, not yet starts the DMA. You have to use
	EnableDMAChannel() to start the DMA. And you can use DMA_WaitIntStatus() to
	wait until fihish.
**************************************************************************************/
/*
 * uint32_t Channel,   // use which channel for AHB DMA, 0..7
 * uint32_t SrcAddr,   // source begin address
 * uint32_t DstAddr,   // dest begin address
 * uint32_t Size,      // total bytes
 * uint32_t SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
 * uint32_t DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
 * uint32_t SrcSize,   // source burst size, How many "SrcWidth" will be
 *                     // transmmited at one times ?
 * uint32_t SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
 * uint32_t DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
 * uint32_t Priority)  // priority for this chaanel 0(low)/1/2/3(high)
 */
int ftsdc021_DMA_NoLLD(uint32_t Channel, uint32_t SrcAddr, uint32_t DstAddr,
		       uint32_t Size, uint32_t SrcWidth, uint32_t DstWidth,
		       uint32_t SrcSize, uint32_t SrcCtrl, uint32_t DstCtrl,
		       uint32_t Priority)
{
	DMA_CH_t DMAChannel;

	/* initialize DMAChannel */
	memset(&DMAChannel, 0, sizeof(DMA_CH_t));

	/* program channel CSR */
	DMAChannel.csr.tc_msk = 0;
	DMAChannel.csr.src_tcnt= SrcSize; /* source burst size */
	DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
	DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
	DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
	DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */

	DMA_SetCHCsr(Channel, DMAChannel.csr);

	/* program channel CFG */
	DMAChannel.cfg.int_tc_msk = 1;	// Disable terminal count interrupt, if tc then interrupt
	DMAChannel.cfg.int_err_msk = 1;	// Disable error interrupt
	DMAChannel.cfg.int_abt_msk = 1;	// Disable abort interrupt => modify by Jerry

	if(SrcCtrl == AHBDMA_SrcFix) {
		DMAChannel.cfg.src_hen = 1;
		DMAChannel.cfg.src_rs = ftsdc021_req_sel;
	}

	if(DstCtrl == AHBDMA_DstFix) {
		DMAChannel.cfg.dst_hen = 1;
		DMAChannel.cfg.dst_rs = ftsdc021_req_sel;
	}

	DMAChannel.cfg.ch_gntwin = 0;		//channel grant window
	DMAChannel.cfg.reserved = 0;		//reserved bits are RO
	DMAChannel.cfg.reserved1 = 0;		//reserved1 bits are RO
	DMAChannel.cfg.llp_cnt = 0;		//llp_cnt bits are RO
	DMAChannel.cfg.ch_pri = Priority;	//ChPri-channel arbitration priority

	DMA_SetCHCfg(Channel, DMAChannel.cfg);

	/* program channel llp */
	*((uint32_t *)&(DMAChannel.llp)) = 0;	//no LLP, this is to set link_list_addr = 0
	DMA_CHLinkList(Channel, DMAChannel.llp);

	/* porgram address and size */
	DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, Size / (1 << SrcWidth));

	*((uint32_t *)&(DMAChannel.llp)) = 1;	//this is to set master_id = 1, and link_list_addr = 0

	return 0;
}

/*
 * uint32_t Channel,   // use which channel for AHB DMA, 0..7
 * uint32_t LLPSize,  // Link-List address, an pre-assigned space for link list
 * uint32_t LLPCount,  // total link-list node, if NO link list is needed,
 *                     // LLPCount is 0
 * uint32_t SrcAddr,   // source begin address
 * uint32_t DstAddr,   // dest begin address
 * uint32_t Size,      // total bytes
 * uint32_t SrcWidth,  // source width 8/16/32 bits -> 0/1/2
 * uint32_t DstWidth,  // dest width 8/16/32 bits -> 0/1/2
 * uint32_t SrcSize,   // source burst size, How many "SrcWidth" will be
 *                     // transmmited at one times ?
 * uint32_t SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
 * uint32_t DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
 * uint32_t Priority)  // priority for this chaanel 0(low)/1/2/3(high)
 */
int ftsdc021_DMA_HandShakeMode(uint32_t Channel, uint32_t LLPSize,
			       uint32_t LLPCount, uint32_t SrcAddr,
			       uint32_t DstAddr, uint32_t Size,
			       uint32_t SrcWidth, uint32_t DstWidth,
			       uint32_t SrcSize, uint32_t SrcCtrl,
			       uint32_t DstCtrl, uint32_t Priority)
{
	DMA_CH_t DMAChannel;

	memset(&DMAChannel, 0, sizeof(DMA_CH_t));

	if(LLPCount > 0){
		ftsdc021_DMA_FillMemLLD(LLPSize, LLPCount, SrcAddr, DstAddr,
					Size, SrcWidth, DstWidth, SrcCtrl,
					DstCtrl);
		DMAChannel.csr.src_tcnt= SrcSize;
		DMAChannel.csr.src_width = SrcWidth;
		DMAChannel.csr.dst_width = DstWidth;
		DMAChannel.csr.src_ctrl = SrcCtrl;
		DMAChannel.csr.dst_ctrl = DstCtrl;
		DMA_SetCHCsr(Channel, DMAChannel.csr);

		/* program channel CFG */
		DMAChannel.cfg.int_tc_msk = 1;
		DMAChannel.cfg.int_err_msk = 1;
		DMAChannel.cfg.int_abt_msk = 1;
		if(SrcCtrl == AHBDMA_SrcFix){
			DMAChannel.cfg.src_hen = 1;
			DMAChannel.cfg.src_rs = ftsdc021_req_sel;
		}

		if(DstCtrl == AHBDMA_DstFix){
			DMAChannel.cfg.dst_hen = 1;
			DMAChannel.cfg.dst_rs = ftsdc021_req_sel;
		}

		DMAChannel.cfg.ch_pri = Priority;
		DMA_SetCHCfg(Channel, DMAChannel.cfg);

		/* program channel llp */
		*((uint32_t *)&(DMAChannel.llp)) = 0;  //Init IP llp
		DMAChannel.llp.link_list_addr = FTSDC021_DMA_LLD_STRUCT_ADDR;
		DMA_CHLinkList(Channel, DMAChannel.llp);

		/* porgram address and size */
		DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, LLPSize);
		DMA_SetLDMAddr(DMA030_LDM_BASE);
	} else {	//for LLPCount==1
		if(ftsdc021_DMA_NoLLD(Channel, SrcAddr, DstAddr, Size, SrcWidth,
				      DstWidth, SrcSize, SrcCtrl, DstCtrl,
				      Priority))
			return 1;
	}

	return 0;
}

int ftsdc021_Start_DMA(uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size,
		       uint32_t SrcWidth, uint32_t DstWidth, uint32_t SrcSize,
		       uint32_t SrcCtrl, uint32_t DstCtrl)
{
	uint32_t LLPSize, Count, sts;

	/* program channel */
	DMA_ResetChannel(ftsdc021_dma_channel);
	ClearDMAChannelIntStatus(ftsdc021_dma_channel);

	LLPSize = ftsdc021_DMA_getMaxSize(SrcWidth, SrcSize);
	/* The Size is too large to move in one LLP in the IP */
	if ((Size / (1 << SrcWidth)) > LLPSize) {
		Count = ((Size + (LLPSize - 1) * (1 << SrcWidth)) /
			 (1 << SrcWidth)) / LLPSize;
	} else {
		Count = 1;
	}

	if(ftsdc021_DMA_HandShakeMode(ftsdc021_dma_channel, LLPSize, Count - 1,
				      SrcAddr, DstAddr, Size, SrcWidth,
				      DstWidth, SrcSize, SrcCtrl, DstCtrl,
				      ftsdc021_dma_priority)) {
		return 1;
	}

	EnableDMAChannel(ftsdc021_dma_channel);

	sts = DMA_WaitIntStatus(ftsdc021_dma_channel);
	if (sts & 0x6)
		return 1;

	return 0;
}

int ftsdc021_wait_DMA_done(uint32_t wait_t)
{
        int ret;
        clock_t t0;

        t0 = clock();
        do {
                ret = GetDMAChannelIntStatus(ftsdc021_dma_channel);
                if (ret & 0x2)
                        return -1;
                else if (ret & 0x1)
                        return 0;
        } while (clock() - t0 < wait_t);

        return -1;
}

