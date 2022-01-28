/*
 * Faraday FTDMAC030 DMA non os test driver header file
 *
 * (C) Copyright 2020 Faraday Technology
 * Jack Chain <jack_ch@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __AHB_DMA030_H
#define __AHB_DMA030_H

/******************************************************************************
 * Define Constants
 *****************************************************************************/

#include "portme_ftdmac030.h"

extern volatile unsigned int  bWaitIntCB;

#define IPMODULE  DMAC
#define IPNAME    FTDMAC030

extern unsigned int  dma_instance_N;

#define DMA030_BASE             DMAC_FTDMAC030_pa_base[dma_instance_N]
#if (DMAC_FTDMAC030_COUNT>1)
#define IRQ_DMA030              DMAC_FTDMAC030_irq[dma_instance_N][0] 
#else
#define IRQ_DMA030              DMAC_FTDMAC030_irq[0] 
#endif


#define DMA030_LLD_SYSMEM       (FREE_MEM_BASE+0x3000000)

#ifdef RAM_FTDMAC030_PA_BASE
    #define DMA030_LDM_BASE      RAM_FTDMAC030_pa_base[dma_instance_N]
#else
    #define DMA030_LDM_BASE      0   //no LDM in system
#endif


#define CHANNEL_NUM CONFIG_FTDMAC030_CHANNEL_COUNT

#define SRCADDR                  FREE_MEM_BASE 
#define DSTADDR                  (FREE_MEM_BASE+0x1000000)
#define DSTADDR2                 (FREE_MEM_BASE+0x2000000)

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
#define DMA_INT                  0x0
#define DMA_INT_TC               0x4
#define DMA_INT_TC_CLR           0x8
#define DMA_INT_ERRABT           0xC
#define DMA_INT_ERRABT_CLR       0x10
#define DMA_TC                   0x14
#define DMA_ERRABT               0x18
#define DMA_CH_EN                0x1C
#define DMA_CH_SYNC              0x20
#define DMA_LDM_BASE             0x24
#define DMA_WDT                  0x28
#define DMA_GE                   0x2C
#define DMA_APB_SLAVE_ERR        0x30
#define DMA_FEATURE              0x38

#define DMA_ENDIAN               0x4c
#define DMA_WRITE_ONLY           0x50


#define DMA_CHANNEL_OFFSET       0x20
#define DMA_CHANNEL0_BASE        0x100
#define DMA_CHANNEL1_BASE        0x120
#define DMA_CHANNEL2_BASE        0x140
#define DMA_CHANNEL3_BASE        0x160
#define DMA_CHANNEL4_BASE        0x180
#define DMA_CHANNEL5_BASE        0x1a0
#define DMA_CHANNEL6_BASE        0x1c0
#define DMA_CHANNEL7_BASE        0x1e0

#define DMA_CHANNEL_CSR_OFFSET      0x0
#define DMA_CHANNEL_CFG_OFFSET      0x4
#define DMA_CHANNEL_SRCADDR_OFFSET  0x8
#define DMA_CHANNEL_DSTADDR_OFFSET  0xc
#define DMA_CHANNEL_LLP_OFFSET      0x10
#define DMA_CHANNEL_SIZE_OFFSET     0x14
#define DMA_CHANNEL_STRIDE_OFFSET   0x18


/* bit mapping of main configuration status register(CSR 0x24) */
#define DMA_CSR_M1ENDIAN         0x00000004
#define DMA_CSR_M0ENDIAN         0x00000002
#define DMA_CSR_DMACEN           0x00000001

/* bit mapping of channel control register 0x100 */
#define DMA_CSR_SRCTCNY_1        0x0
#define DMA_CSR_SRCTCNY_2        0x20000000
#define DMA_CSR_SRCTCNY_4        0x40000000
#define DMA_CSR_SRCTCNY_8        0x40000000 | 0x20000000
#define DMA_CSR_SRCTCNY_16       0x80000000
#define DMA_CSR_SRCTCNY_32       0x80000000 | 0x20000000
#define DMA_CSR_SRCTCNY_64       0x80000000 | 0x40000000
#define DMA_CSR_SRCTCNY_128      0x80000000 | 0x40000000 | 0x20000000

#define DMA_CSR_TC_MSK           0x10000000

#define DMA_CSR_SRC_WIDTH_8      0x0
#define DMA_CSR_SRC_WIDTH_16     0x02000000
#define DMA_CSR_SRC_WIDTH_32     0x04000000
#define DMA_CSR_SRC_WIDTH_64     0x02000000 |  0x04000000
#define DMA_CSR_SRC_WIDTH_128    0x08000000

#define DMA_CSR_DST_WIDTH_8      0x0
#define DMA_CSR_DST_WIDTH_16     0x400000
#define DMA_CSR_DST_WIDTH_32     0x800000
#define DMA_CSR_DST_WIDTH_64     0x400000 | 0x800000
#define DMA_CSR_DST_WIDTH_128    0x01000000

#define DMA_CSR_SRCCTRL_Fix      0x100000
#define DMA_CSR_DSTCTRL_Fix      0x040000
#define DMA_CSR_WDT_EN           0x020000
#define DMA_CSR_CH_EN            0x010000
#define DMA_CSR_EXP_EN           0x8000
#define DMA_CSR_2D_EN            0x4000
#define DMA_CSR_WEVENT_EN        0x2000
#define DMA_CSR_SEVENT_EN        0x1000
#define DMA_CSR_WSYNC            0x0100
/*END  bit mapping of channel control register 0x100 */

/* bit mapping of channel control register 0x104 */
#define DMA_CSR_UNALIGN          0x80000000
#define DMA_CSR_WRITE_ONLY       0x40000000
#define DMA_CSR_CHPRI_HI         0x10000000
/* hardware feature register*/
#define DMA_FEATURE_UNALIGN      0x08

/*END  bit mapping of channel control register 0x104 */
#define DMA_CSR_CHPRJ_HIGHEST    0x00C00000
#define DMA_CSR_CHPRJ_2ND        0x00800000
#define DMA_CSR_CHPRJ_3RD        0x00400000
#define DMA_CSR_PRTO3            0x00200000
#define DMA_CSR_PRTO2            0x00100000
#define DMA_CSR_PRTO1            0x00080000

#define DMA_CSR_ABT              0x00008000

#define DMA_CSR_MODE_NORMAL      0x00000000
#define DMA_CSR_MODE_HANDSHAKE   0x00000080

#define DMA_CSR_SRC_INCREMENT    0x00000000
#define DMA_CSR_SRC_DECREMENT    0x00000020
#define DMA_CSR_SRC_FIX          0x00000040

#define DMA_CSR_DST_INCREMENT    0x00000000
#define DMA_CSR_DST_DECREMENT    0x00000008
#define DMA_CSR_DST_FIX          0x00000010

#define DMA_CSR_SRC_SEL          0x00000004
#define DMA_CSR_DST_SEL          0x00000002
#define DMA_CSR_CH_ENABLE        0x00000001

#define DMA_CSR_DMA_FF_TH        0x07000000
#define DMA_CSR_CHPR1            0x00C00000
#define DMA_CSR_SRC_SIZE         0x00070000
#define DMA_CSR_SRC_WIDTH        0x00003800
#define DMA_CSR_DST_WIDTH        0x00000700
#define DMA_CSR_SRCAD_CTL        0x00000060
#define DMA_CSR_DSTAD_CTL        0x00000018

/* bit mapping of channel configuration register 0x104 */
#define DMA_CFG_INT_ABT_MSK      0x00000004   // new
#define DMA_CFG_INT_ERR_MSK      0x00000002   // new
#define DMA_CFG_INT_TC_MSK       0x00000001   // new

///////////////////////////////////////////////////
/* bit mapping of Linked List Control Descriptor */

#define DMA_LLP_DMA_FF_TH        0xE0000000   // new

#define DMA_LLP_TC_MSK           0x10000000

#define DMA_LLP_SRC_WIDTH_8      0x00000000
#define DMA_LLP_SRC_WIDTH_16     0x02000000
#define DMA_LLP_SRC_WIDTH_32     0x04000000
#define DMA_LLP_SRC_WIDTH_64     0x06000000   // new

#define DMA_LLP_DST_WIDTH_8      0x00000000
#define DMA_LLP_DST_WIDTH_16     0x00400000
#define DMA_LLP_DST_WIDTH_32     0x00800000
#define DMA_LLP_DST_WIDTH_64     0x00C00000   // new

#define DMA_LLP_SRC_INCREMENT    0x00000000
#define DMA_LLP_SRC_DECREMENT    0x00100000
#define DMA_LLP_SRC_FIX          0x00200000

#define DMA_LLP_DST_INCREMENT    0x00000000
#define DMA_LLP_DST_DECREMENT    0x00040000
#define DMA_LLP_DST_FIX          0x00080000

#define DMA_LLP_SRC_SEL          0x00020000
#define DMA_LLP_DST_SEL          0x00010000

/////////////////////////// AHB DMA Define //////////////////////////////////
#define AHBDMA_Channel0             0
#define AHBDMA_Channel1             1
#define AHBDMA_Channel2             2
#define AHBDMA_Channel3             3
#define AHBDMA_Channel4             4
#define AHBDMA_Channel5             5
#define AHBDMA_Channel6             6
#define AHBDMA_Channel7             7

#define AHBDMA_SrcWidth_Byte        0
#define AHBDMA_SrcWidth_Word        1
#define AHBDMA_SrcWidth_DWord       2

#define AHBDMA_DstWidth_Byte        0
#define AHBDMA_DstWidth_Word        1
#define AHBDMA_DstWidth_DWord       2

#define AHBDMA_Burst1               0
#define AHBDMA_Burst4               1
#define AHBDMA_Burst8               2
#define AHBDMA_Burst16              3
#define AHBDMA_Burst32              4
#define AHBDMA_Burst64              5
#define AHBDMA_Burst128             6
#define AHBDMA_Burst256             7

#define AHBDMA_NormalMode           0
#define AHBDMA_HwHandShakeMode      1

#define AHBDMA_SrcInc               0
#define AHBDMA_SrcFix               2

#define AHBDMA_DstInc               0
#define AHBDMA_DstFix               2

#define AHBDMA_PriorityLow          0
#define AHBDMA_PriorityHigh         1

#define AHBDMA_MAX_LLDSIZE          0x100


// feature register
typedef struct
{
   unsigned int ch_num     :3;
   unsigned int reserved3  :1;
   unsigned int d_width    :2;
   unsigned int reserved6  :2;
   unsigned int dfifo_depth:3;
   unsigned int reserved11 :1;
   unsigned int pri_on     :1;
   unsigned int reserved13 :3;
   unsigned int pri_num    :4;
   unsigned int ldm_on     :1;
   unsigned int reserved21 :3;
   unsigned int ldm_depth  :2;
   unsigned int reserved26 :2;
   unsigned int cmd_depth  :2;
   unsigned int reserved30 :2;
} DMA_FEATURE_t;  

typedef struct
{
   unsigned int ch_wevent  :8;
   unsigned int wsync      :1;
   unsigned int ch_sevent  :3;
   unsigned int sevent_en  :1;
   unsigned int wevent_en  :1;
   unsigned int twod_en    :1;
   unsigned int exp_en     :1;
   unsigned int ch_en      :1;
   unsigned int wdt_en     :1;
   unsigned int dst_ctrl   :2;
   unsigned int src_ctrl   :2;
   unsigned int dst_width  :3;
   unsigned int src_width  :3;
   unsigned int tc_msk     :1;
   unsigned int src_tcnt   :3;
}DMA_CH_CSR_t;//DMAC030

typedef struct
{
   unsigned int int_tc_msk :1;
   unsigned int int_err_msk:1;
   unsigned int int_abt_msk:1;
   unsigned int src_rs     :4;
   unsigned int src_hen    :1;
   unsigned int reserved   :1;
   unsigned int dst_rs     :4;
   unsigned int dst_hen    :1;
   unsigned int reserved1  :2;
   unsigned int llp_cnt    :4;
   unsigned int ch_gntwin  :8;
   unsigned int ch_pri     :1;
   unsigned int reserved29 :1;
   unsigned int wo_mode    :1;
   unsigned int Unalign_Mode  :1;
}DMA_CH_CFG_t;//DMAC030

typedef struct
{
   unsigned int link_list_addr:32;
}DMA_CH_LLP_t;

typedef struct
{
   unsigned int ch_wevent  :8;
   unsigned int wsync      :1;
   unsigned int ch_sevent  :3;
   unsigned int sevent_en  :1;
   unsigned int wevent_en  :1;
   unsigned int twod_en    :1;
   unsigned int exp_en     :1;
   unsigned int ch_en      :1;
   unsigned int wdt_en     :1;
   unsigned int dst_ctrl   :2;
   unsigned int src_ctrl   :2;
   unsigned int dst_width  :3;
   unsigned int src_width  :3;
   unsigned int tc_msk     :1;
   unsigned int src_tcnt   :3;
}DMA_LLP_CTRL_t;

typedef struct
{
   volatile DMA_CH_CSR_t csr;
   volatile DMA_CH_CFG_t cfg;
   volatile unsigned int src_addr;
   volatile unsigned int dst_addr;
   volatile DMA_CH_LLP_t llp;
   volatile unsigned int size;      //the unit is SrcWidth
   volatile unsigned int stride;
   volatile unsigned int dummy;
}DMA_CH_t;

typedef struct
{
   unsigned int src_addr;
   unsigned int dst_addr;
   DMA_CH_LLP_t llp;
   DMA_LLP_CTRL_t llp_ctrl;
   unsigned int TotalSize;    //new, the unit is SrcWidth, only 21 bits is used
   unsigned int Stride;
   unsigned int Dummy[2];     //this is for program to look nice.
}DMA_LLD_t;

typedef struct
{
   volatile unsigned int dma_int;
   volatile unsigned int dma_int_tc;
   volatile unsigned int dma_int_tc_clr;
   volatile unsigned int dma_int_err;
   volatile unsigned int dma_int_err_clr;
   volatile unsigned int dma_tc;
   volatile unsigned int dma_err;
   volatile unsigned int dma_ch_enable;
   volatile unsigned int dma_sync_pi;
   volatile unsigned int dma_ldm;
   volatile unsigned int dma_wdt;
   volatile unsigned int dma_ge;
   volatile unsigned int dma_plverr;
   volatile unsigned int dma_rn;
   volatile unsigned int dma_hf;
   volatile unsigned int dma_ldm_flag[4];
   volatile unsigned int dma_big_endian;
   volatile unsigned int dma_writeonly;

   volatile unsigned int dummy1[43];

   volatile DMA_CH_t dma_ch[8];
}DMA_Reg_st;


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
extern int    IsDMAChannelBusy(int Channel);
extern int    IsDMAChannelEnable(int Channel);
extern unsigned int GetDMAIntStatus(void);
extern unsigned int GetDMAChannelIntStatus(int Channel);
extern int    GetDMABusyStatus(void);
extern int    GetDMAEnableStatus(void);

extern void   InitDMA(unsigned int M0_BigEndian, unsigned int M1_BigEndian, unsigned int Sync);
extern void   EnableDMAChannel(int Channel);
extern void   EnableDMAChannelEndianConverter(int Channel);
extern void   DisableDMAChannelEndianConverter(int Channel);

extern void   DisableDMAChannel(int Channel);  

extern void   ClearDMAChannelIntStatus(int Channel);

extern void   SetDMAChannelCfg(int Channel, DMA_CH_CSR_t Csr);
extern        DMA_CH_CSR_t GetDMAChannelCfg(int Channel);
extern void   DMA_CHIntMask(int Channel, DMA_CH_CFG_t Mask);
extern void   DMA_CHLinkList(int Channel, DMA_CH_LLP_t LLP);
extern void   DMA_CHDataCtrl(int Channel, unsigned int SrcAddr, unsigned int DstAddr, unsigned int Size);
extern void   DMA_CHDataCtrl_2D(int Channel, unsigned int SrcAddr, unsigned int DstAddr, unsigned int XTcnt, unsigned int YTcnt, unsigned int DstStride, unsigned int SrcStride);
extern void   EnableDMAChannelUnalign(int Channel);

extern void   DMA_SetInterrupt(unsigned int channel, unsigned int tcintr, unsigned int errintr, unsigned int abtintr);
extern void   DMA_ResetChannel(unsigned char channel);
extern void   DMA_ClearAllInterrupt(void);
extern void   DMA_SetWriteOnlyValue(int Value);

void DMA_WaitIntStatus(unsigned int Channel);
int DMA_ISChannelEnable(int Channel);
int DMATest_new(unsigned int mode);
int CompareMemory(unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int Size);
int CheckContent(unsigned int *DstAddr, unsigned int Size);

#endif
