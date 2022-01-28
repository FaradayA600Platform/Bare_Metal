/*
 * Faraday FTDMAC030 DMA non os test c file
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
/******************************************************************************
 * Include files
 *****************************************************************************/
#include "DMA030.h"

/******************************************************************************
 * Define Constants
 *****************************************************************************/

/******************************************************************************
 * Gobal variables
 *****************************************************************************/
unsigned int   dma_instance_N;

DMA_Reg_st *DMA_Register;// = (DMA_Reg_st *)DMA030_BASE;

/******************************************************************************
 *Public functions
 *****************************************************************************/
int IsDMAChannelEnable(int Channel)
{
   return ((DMA_Register->dma_ch_enable >> Channel) & 0x1);
}

unsigned int GetDMAIntStatus(void)
{
   return DMA_Register->dma_int;
}

unsigned int GetDMAChannelIntStatus(int Channel)
{
   unsigned int IntStatus = 0;

   if((DMA_Register->dma_int >> Channel) & 0x01)
   {
      if((DMA_Register->dma_int_tc >> Channel) & 0x01)
         IntStatus |= 1;
      if((DMA_Register->dma_int_err >> Channel) & 0x01)
         IntStatus |= 2;
   }

   return IntStatus;
}

int GetDMAEnableStatus(void)
{
   return DMA_Register->dma_ch_enable;
}

void EnableDMAChannel(int Channel)
{
   volatile unsigned int reg;

   reg = *(unsigned int *)&DMA_Register->dma_ch[Channel].csr;
   reg |= DMA_CSR_CH_EN;
#ifndef  DMA_INT_POLLING
   bWaitIntCB=1;
#endif
   *(unsigned int *)&DMA_Register->dma_ch[Channel].csr = reg;
}

void EnableDMAEndian()
{
   volatile unsigned int reg;
   
   reg=inw(DMA030_BASE+DMA_ENDIAN);
   reg |= 1<<16;
   outw(DMA030_BASE+DMA_ENDIAN,reg);
}

void EnableDMAChannelEndianConverter(int Channel)
{
   volatile unsigned int reg;
  
   reg=inw(DMA030_BASE+DMA_ENDIAN);
   reg |= 1<<Channel;
   outw(DMA030_BASE+DMA_ENDIAN,reg);
}
 
void DisableDMAChannelEndianConverter(int Channel)
{
   volatile unsigned int reg;

   outw(DMA030_BASE+DMA_ENDIAN,(0<<Channel));
}

void EnableDMAChannelWriteOnly(int Channel)
{
   volatile unsigned int reg;

   reg = *(unsigned int *)&DMA_Register->dma_ch[Channel].cfg;
   reg |= DMA_CSR_WRITE_ONLY;
   *(unsigned int *)&DMA_Register->dma_ch[Channel].cfg = reg;
}

void EnableDMAChannelUnalign(int Channel)
{
   volatile unsigned int reg;

   reg = *(unsigned int *)&DMA_Register->dma_ch[Channel].cfg;
   reg |= DMA_CSR_UNALIGN;
   *(unsigned int *)&DMA_Register->dma_ch[Channel].cfg = reg;
}

void DisableDMAChannel(int Channel)
{
   volatile unsigned int reg;

   reg = *(unsigned int *)&DMA_Register->dma_ch[Channel].csr;
   reg &= DMA_CSR_CH_EN;  
   *(unsigned int *)&DMA_Register->dma_ch[Channel].csr = reg;
}

void ClearDMAChannelIntStatus(int Channel)
{
   unsigned int ctrl;

   ctrl = 1 << Channel;
   DMA_Register->dma_int_tc_clr = 1 << Channel;

   ctrl = (1<<Channel) | (1<<(Channel+16));
   DMA_Register->dma_int_err_clr = 1 << Channel;
}

void SetDMAChannelCfg(int Channel, DMA_CH_CSR_t Csr)
{
   DMA_Register->dma_ch[Channel].csr = Csr;
}

DMA_CH_CSR_t GetDMAChannelCfg(int Channel)
{
   return DMA_Register->dma_ch[Channel].csr;
}

void DMA_CHIntMask(int Channel, DMA_CH_CFG_t Mask)
{
   DMA_Register->dma_ch[Channel].cfg = Mask;
}

void DMA_CHLinkList(int Channel, DMA_CH_LLP_t LLP)
{
   DMA_Register->dma_ch[Channel].llp = LLP;
}

void DMA_CHDataCtrl(int Channel, unsigned int SrcAddr, unsigned int DstAddr, unsigned int Size)
{
   DMA_Register->dma_ch[Channel].src_addr = SrcAddr;
   DMA_Register->dma_ch[Channel].dst_addr = DstAddr;
   DMA_Register->dma_ch[Channel].size = Size;
}

void DMA_CHDataCtrl_2D(int Channel, unsigned int SrcAddr, unsigned int DstAddr, unsigned int XTcnt, unsigned int YTcnt, unsigned int DstStride, unsigned int SrcStride)
{
   DMA_Register->dma_ch[Channel].src_addr = SrcAddr;
   DMA_Register->dma_ch[Channel].dst_addr = DstAddr;
   DMA_Register->dma_ch[Channel].size = (YTcnt << 16) | XTcnt;
   DMA_Register->dma_ch[Channel].stride = (DstStride << 16) | SrcStride;
}

void DMA_SetInterrupt(unsigned int channel, unsigned int tcintr, unsigned int errintr, unsigned int abtintr)
{
   DMA_CH_CFG_t cfg;

   if(tcintr)
      cfg.int_tc_msk = 0;  // Enable terminal count interrupt
   else
      cfg.int_tc_msk = 1;  // Disable terminal count interrupt

   if(errintr)
      cfg.int_err_msk = 0; // Enable error interrupt
   else
      cfg.int_err_msk = 1; // Disable error interrupt

   if(abtintr)
      cfg.int_abt_msk = 0; // Enable abort interrupt
   else
      cfg.int_abt_msk = 1; // Disable abort interrupt

   DMA_CHIntMask(channel, cfg);
}

void DMA_ResetChannel(unsigned char channel)
{
   unsigned int base = DMA030_BASE+DMA_CHANNEL0_BASE+channel*DMA_CHANNEL_OFFSET;

   outw(base+DMA_CHANNEL_CSR_OFFSET    ,0);
   outw(base+DMA_CHANNEL_CFG_OFFSET    ,7);
   outw(base+DMA_CHANNEL_SRCADDR_OFFSET,0);
   outw(base+DMA_CHANNEL_DSTADDR_OFFSET,0);
   outw(base+DMA_CHANNEL_LLP_OFFSET    ,0);
   outw(base+DMA_CHANNEL_SIZE_OFFSET   ,0);
}

void DMA_ClearAllInterrupt(void)
{
   outw(DMA030_BASE+DMA_INT_TC_CLR,0xFF);
   outw(DMA030_BASE+DMA_INT_ERRABT_CLR,0xFF00FF);
}

void DMA_SetWriteOnlyValue(int Value)
{
   outw(DMA030_BASE+DMA_WRITE_ONLY,Value);
}

void DMA_EnableFeatureUnalign(void)
{
   volatile unsigned int reg;
   
   reg=inw(DMA030_BASE+DMA_FEATURE);
   reg |= DMA_FEATURE_UNALIGN;
   outw(DMA030_BASE+DMA_FEATURE,reg);
}

unsigned int DMA_UnalignFeature(void)
{
   volatile unsigned int reg;
   
   reg=inw(DMA030_BASE+DMA_FEATURE);
   reg &= DMA_FEATURE_UNALIGN;
   if(reg)
      return 1;
   else
      return 0;
}
