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
#define DMA_EVENT_TEST

#define READ_LEN  256

static unsigned char read_buf[READ_LEN];
static unsigned char dma_read_buf[READ_LEN];

#define min_t(x,y) ( x < y ? x: y )

extern DMA_Reg_st *DMA_Register;

volatile unsigned int   bWaitIntCB;
volatile unsigned int IntStatus;

/*define test item, follow lists is variable change by users*/
#define DMA_2D_Transfer // 2D or 1D
//#define DMA_INT_POLLING  //Interrupt or polling
#define AHBDMA_MAX_LLDSIZE       0x100 //0x3FFFF0  //4M-4,


// The mark has to be taken off to enable the performance test, if you want to do it.
//#define DMA_PERFORMANCE_TEST
#define DMA_BURNIN_TEST
#ifdef DMA_PERFORMANCE_TEST
#include "timer.h"
#define TICK_HZ 100
#endif

unsigned int AddrBaseInc = 0x200000;

#ifdef DMA_2D_Transfer
 char str_fun[10]="2d";
#else
 char str_fun[10]="1d"; 
#endif

/******************************************************************************
 * internal functions
 *****************************************************************************/
void DMA_SetCHCsr(int Channel, DMA_CH_CSR_t Csr)
{
   DMA_Register->dma_ch[Channel].csr = Csr;
}

DMA_CH_CSR_t DMA_GetCHCsr(int Channel)
{
   return DMA_Register->dma_ch[Channel].csr;
}

#ifndef DMA_INT_POLLING
void DMA_InterruptHandler(void)
{
   unsigned int   reg, Channel=0;

   bWaitIntCB=0;
   reg=GetDMAIntStatus();
   if(reg>0)
   {
      for(Channel=0;Channel<16;Channel++)
      {
         if((reg>>Channel)&0x01)
         {
            ClearDMAChannelIntStatus(Channel);
            DisableDMAChannel(Channel);
         }
      }
   }
   return;
}
#endif

void DMA_SetCHCfg(int Channel, DMA_CH_CFG_t Cfg)
{
   DMA_Register->dma_ch[Channel].cfg = Cfg;
}

int DMA_SetLDMAddr(int Address)
{
   DMA_Register->dma_ldm = Address;
}

int DMA_ISChannelEnable(int Channel)
{
   DMA_CH_CSR_t reg;
   reg = GetDMAChannelCfg(Channel);
   return ((*(unsigned int*)&reg)&DMA_CSR_CH_EN);
}

void DMA_WaitIntStatus(unsigned int Channel)
{
   unsigned int choffset;
   unsigned int Status;

#ifndef DMA_INT_POLLING
   //wait interrupt handler to clear the interrupt for hardware interrupt
   choffset = 1 << Channel;

   while(bWaitIntCB);
#else
   choffset = 1 << Channel;
   while(1) 
   {
      IntStatus = GetDMAChannelIntStatus(Channel);
      if ((IntStatus & 0x01) != 0) 
      {
         ClearDMAChannelIntStatus(Channel);  // clear interrupt
         Status = GetDMAChannelIntStatus(Channel);
         if (Status!=0)
            printf("Channel:%d, Cannot clear Terminal Count Interrupt Register !!\n",Channel);
         break;
      }
      else if ((IntStatus & 0x02) != 0)
      {
         printf("Channel:%d, DMA has some error !!\n",Channel);
         printf("(read transaction or the write transaction) is received on the AXI bus, or  watchdog timer reaches the watchdog timer value!\n",Channel);
         break;
      }
   }

   ClearDMAChannelIntStatus(Channel);
   DisableDMAChannel(Channel);
#endif
}

int DMA_GetAXIBusWidth()
{
   unsigned int   Hw_feature,AxiWidth;
   DMA_FEATURE_t *Feature;
   
   Hw_feature=inw((unsigned int *)(DMA030_BASE+DMA_FEATURE));
   Feature=(DMA_FEATURE_t *)&Hw_feature;
   if((Feature->d_width) >2)
      printf("AXI data bus width greater than 128 bits doesn't support\n");
   
   AxiWidth=(Feature->d_width)+2;
   return   AxiWidth;
}
int DMA_GetDepthOfFIFO()
{
   unsigned int   Hw_feature;
   DMA_FEATURE_t *Feature;
   
   Hw_feature=inw((unsigned int *)(DMA030_BASE+DMA_FEATURE));
   Feature=(DMA_FEATURE_t *)&Hw_feature;
   if((Feature->dfifo_depth) >4)
      printf("The depth of data FIFO greater than 128!!\n");
   
   return Feature->dfifo_depth;
}

int CompareMemory_byte(unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int Size)
{
   int i = 0;
   unsigned char *SrcAddr_byte=(unsigned char*)SrcAddr;
   unsigned char *DstAddr_byte=(unsigned char*)DstAddr;

   if(memcmp(SrcAddr, DstAddr, Size) != 0)
   {
      for(i = 0; i < Size; i++)
      {
         if(SrcAddr_byte[i] != DstAddr_byte[i])
         {
            printf("\n========>DstAddr:0x%08X = 0x%08X, should be 0x%08X\n", (unsigned int)&DstAddr_byte[i], DstAddr_byte[i], SrcAddr_byte[i]);
            printf("IntStatus = %x\n",IntStatus);
            return FALSE;
         }
      }
   }
   return TRUE;
}
int Compare_Endian_halfword(unsigned int *SrcAddr_halfword, unsigned int *DstAddr_halfword,unsigned int Size)
{
   unsigned int  SrcAddr_halfword_temp = 0;
   unsigned char*  Src_byte;
   unsigned char*  Dst_byte;
   int i,j;

   for(i = 0; i < (Size/4); i++)
   {
      SrcAddr_halfword_temp = *(SrcAddr_halfword + i);
      SrcAddr_halfword_temp=((SrcAddr_halfword_temp>>8) & 0x00FF00FF) | ((SrcAddr_halfword_temp<<8) & 0xFF00FF00);
      if(((i+1) == (Size/4))&&(Size%4)) //last 
      {
            return CompareMemory_byte(&(*(SrcAddr_halfword+i)),&(*(DstAddr_halfword+i)),(Size%4));
      }
      else if(DstAddr_halfword[i] != SrcAddr_halfword_temp)
      {
         printf("\n========>DstAddr:0x%08X = 0x%08X, should be 0x%08X\n", (unsigned int)&DstAddr_halfword[i], DstAddr_halfword[i], SrcAddr_halfword_temp);
         printf("IntStatus = %x\n",IntStatus);
         return FALSE;
      }
   }
   return TRUE;
}

int Compare_Endian_word(unsigned int *SrcAddr_word, unsigned int *DstAddr_word,unsigned int Size)
{
   unsigned int  SrcAddr_word_temp = 0;
   unsigned char*  Src_byte;
   unsigned char*  Dst_byte;
   int i,j; 
   
   for(i = 0; i < (Size/4); i++)
   {
      SrcAddr_word_temp = *(SrcAddr_word + i);
      SrcAddr_word_temp=(SrcAddr_word_temp>>24) | ((SrcAddr_word_temp<<8) & 0x00FF0000) |((SrcAddr_word_temp>>8) & 0x0000FF00) | (SrcAddr_word_temp<<24);
      
      if(((i+1) == (Size/4))&&(Size%4)) //last 
      {
         return CompareMemory_byte(&(*(SrcAddr_word+i)),&(*(DstAddr_word+i)),(Size%4));
      }
      else if(*(DstAddr_word+i) != SrcAddr_word_temp)
      {
         printf("\n========>DstAddr:0x%08X = 0x%08X, should be 0x%08X\n", (unsigned int)&DstAddr_word[i], DstAddr_word[i], SrcAddr_word_temp);
         printf("IntStatus = %x\n",IntStatus);
         return FALSE;
      }
   }
   return TRUE;
}

int CompareMemoryEndian(unsigned int* SrcAddr, unsigned int* DstAddr,unsigned int SrcWidth, unsigned int Size)
{
   unsigned int   DMA_DATA;
   unsigned int*  SrcAddr_halfword=SrcAddr;
   unsigned int*  DstAddr_halfword=DstAddr;
   
   unsigned int*  SrcAddr_word=SrcAddr;
   unsigned int*  DstAddr_word=DstAddr;
   
   switch(SrcWidth)
   {
      case 0:
         return   CompareMemory(SrcAddr,DstAddr,Size);
         break;
      case 1:
         return Compare_Endian_halfword(SrcAddr_halfword,DstAddr_halfword,Size);
         break;
      case 2:
      case 3:
      case 4:
         return Compare_Endian_word(SrcAddr_word,DstAddr_word,Size);
         break;
      default:
         printf("\nError source width\n");
         return FALSE;
         break;
   }
}

int CompareMemory(unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int Size)
{
   int i = 0;
      
   if(memcmp(SrcAddr, DstAddr, Size) != 0)
   {
      for(i = 0; i < (Size/4); i++)
      {
         if(DstAddr[i] != SrcAddr[i])
         {
            printf("\n========>DstAddr:0x%08X(0x%08X) != SrcAddr:0x%08X(0x%08X)\n", (unsigned int)&DstAddr[i], DstAddr[i], (unsigned int)&SrcAddr[i], SrcAddr[i]);
            printf("IntStatus = %x\n",IntStatus);
            return FALSE;
         }
      }
   }
   return TRUE;
}

int CompareMemorywithValue( unsigned int *DstAddr, unsigned int Size, unsigned int Value )
{
   int i = 0;
   unsigned int temp;
   int Errorflag=0;

   for(i = 0; i < (Size/4); i++)
   {
      temp=DstAddr[i];
      
      if(temp!=Value)
      {
         printf("\n========>DstAddr:0x%x = 0x%08X, should be 0x%08X\n", &DstAddr[i], temp, Value); 
         Errorflag=1;
      }
   }
   
   if(Errorflag)
      return FALSE;
   return TRUE;
}

int AlignPackMultiple(unsigned int SrcWidth,unsigned int DstWidth)
{
   if(DstWidth>SrcWidth)
      return (1 <<DstWidth)/ (1<<SrcWidth);
   else
      return 0;
}

/*************** DMA 1D Test Function *****************************************/
// This function will start the DMA and return until finish
void DMA_Start(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int LinkAddr,  // Link-List address
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int *Size,     // total bytes
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode,      // Normal/Hardwire,   0/1
int unalign_flag  // Unalign test mode flag, 0/1
)
{
   unsigned int transfer_size,LengthMultiple;
   unsigned int LLPSize, Count;
   DMA_LLD_t *LLDLinkAddr;
   DisableDMAChannel(Channel);
   LLPSize = AHBDMA_MAX_LLDSIZE;
   
   //printf("[DMA_Start] Channel=0x%x LinkAddr=0x%x SrcAddr=0x%x DstAddr=0x%x Size=0x%x\n",Channel,LinkAddr,SrcAddr,DstAddr,*Size);
   //printf("[DMA_Start] SrcWidth=0x%x DstWidth=0x%x SrcSize=0x%x SrcCtrl=0x%x DstCtrl=0x%x\n",SrcWidth,DstWidth,SrcSize,SrcCtrl,DstCtrl);
   //printf("[DMA_Start] Priority=0x%x Mode=0x%x unalign_flag=0x%x \n",Priority,Mode,unalign_flag);
      
   if(!unalign_flag)
   {
      LengthMultiple=AlignPackMultiple(SrcWidth,DstWidth);
      transfer_size = ((unsigned int)*Size) / (1 << SrcWidth);
      if(LengthMultiple)
      {
         if(transfer_size %LengthMultiple)  //align mode data pack handle
         {
            transfer_size= LengthMultiple* (transfer_size /LengthMultiple);// transfer_size(trasfer count=>get a round number
            *Size=transfer_size*(1 << SrcWidth);
            printf("Channel%d has Data Packing. Transfer size=0x%x\n",Channel,*Size);
         }
      }
      *Size=transfer_size*(1 << SrcWidth);
   }
   else
      transfer_size = ((unsigned int)*Size);
   
   if(transfer_size>LLPSize)
   {  //the Size is too large to move in one LLP in the IP
      Count = (transfer_size + (LLPSize-1)) / LLPSize;
      LLDLinkAddr = (DMA_LLD_t *)LinkAddr;
   }
   else
   {
      Count=1;
      LinkAddr = 0;
   }
   
   if(Mode == DMA_CSR_MODE_HANDSHAKE)
   {
      if(DMA_HandShakeMode(Channel, LinkAddr, Count, SrcAddr, DstAddr, transfer_size, SrcWidth, DstWidth,
                  SrcSize, SrcCtrl, DstCtrl, Priority, Mode) == FALSE)
      {
         if(LinkAddr)
            free((void*)LinkAddr);
         return;
      }
   }
   else
   {
      if(DMA_NormalMode(Channel, LinkAddr, Count, SrcAddr, DstAddr, transfer_size, SrcWidth, DstWidth,
                  SrcSize, SrcCtrl, DstCtrl, Priority, Mode,unalign_flag,LLPSize)==FALSE)
      {
         if(LinkAddr)
            free((void*)LinkAddr);
         return;
      }
   }
   
#ifdef DMA_PERFORMANCE_TEST
{
   unsigned int StartTick, EndTick;
   
   T1_Tick = 0;
   if(unalign_flag)
      EnableDMAChannelUnalign(Channel);
   fLib_CloseInt(IRQ_TIMER1);
   fLib_Timer_Init(1, TICK_HZ, Timer1_Tick);
   StartTick = fLib_CurrentT1Tick();

   EnableDMAChannel(Channel);
   DMA_WaitIntStatus(Channel);

   EndTick = fLib_CurrentT1Tick();
   if ( EndTick != StartTick)
   {
      printk("\nspeed:%7d B/s,Size:0x%x,SW:%d,DW:%d,SSize:%d",(int)(Size/((double)(EndTick-StartTick)/(double)TICK_HZ)), Size, SrcWidth, DstWidth, SrcSize);
   }
   else
   {
      printk("\n!!--StartTick=%d and EndTick=%d are the same\n",StartTick, EndTick);
   }
   fLib_Timer_Close(1);
}
#endif

#ifndef DMA_BURNIN_TEST
   if(Mode == DMA_CSR_MODE_NORMAL)
   {
      EnableDMAChannel(Channel);
      DMA_WaitIntStatus(Channel);
      if(LinkAddr)
         free((void*)LinkAddr);
   }
#endif
   return;
}

// This function will start the DMA and return until finish
void DMA_FillChannelReg(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int Size,      // total bytes
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode,       // Normal/Hardwire,   0/1
int unalign_flag
)
{
   unsigned int transfer_size = Size;
   unsigned int LLPSize, Count;
   DMA_LLD_t *LinkAddr;
   DisableDMAChannel(Channel);
   
   LLPSize = AHBDMA_MAX_LLDSIZE;
   if((transfer_size/(1 << SrcWidth))>LLPSize)//the Size is too large to move in one LLP in the IP
   {
      Count = ((transfer_size + (LLPSize-1) * (1 << SrcWidth)) / (1 << SrcWidth)) / LLPSize;
        LinkAddr = (DMA_LLD_t *)(DMA030_LLD_SYSMEM+Channel*Size); //ycmo: for different channel 
   }
   else
   {
      Count=1;
      LinkAddr = 0;
   }
   
   if(DMA_NormalMode(Channel, LinkAddr, Count, SrcAddr, DstAddr, transfer_size, SrcWidth, DstWidth,
                  SrcSize, SrcCtrl, DstCtrl, Priority, Mode,unalign_flag,LLPSize)==FALSE)
   {
      if(LinkAddr)
         free((void*)LinkAddr);
      return;
   }
   return;
}

void DMA_FillMemLLD(
unsigned int LinkAddr,  // Link-List address
unsigned int LLPCount,  // total link-list node
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int Size,      // total bytes
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4   todo note 1 When the unalign mode is enabled, the source data width must be set to be the same value as the data bus width 2. When the write only mode is set, the source data width must be set to be the same value as the destination data width and no less than 32 bits.
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4     todo note 1 When the unalign mode is enabled, the source data width must be set to be the same value as the data bus width 2. When the write only mode is set, the source data width must be set to be the same value as the destination data width and no less than 32 bits.
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2  todo?? now only inc/fix , and note that in the 2D operation, the burst type can only be incremental 
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2    todo?? now only inc/fix , and note that in the 2D operation, the burst type can only be incremental
int UnalignFlag
)
{
   unsigned int i, LLPSize;
   DMA_LLD_t *LLP = (DMA_LLD_t *)LinkAddr;
   
   //printf("[%s] SrcWidth=0x%x DstWidth=0x%x UnalignFlag=0x%x LLPCount=%d\n",__func__,SrcWidth,DstWidth,UnalignFlag,LLPCount);
   
   if ((LLPCount<1)||(LinkAddr==0)) //LLPCount must >=1
      return;

   LLPSize = AHBDMA_MAX_LLDSIZE;
   for(i = 0; i < LLPCount ;i++)
   {
      if(SrcCtrl == AHBDMA_SrcInc)// increase
      {  
         LLP[i].src_addr = (unsigned int)SrcAddr + (i * LLPSize * (1 << SrcWidth));
         if(UnalignFlag)
         {
            LLP[i].src_addr = (unsigned int)SrcAddr + (i * LLPSize);
         }
      }
      else if(SrcCtrl == AHBDMA_SrcFix)// fixed
         LLP[i].src_addr = (unsigned int)SrcAddr;

      if(DstCtrl == AHBDMA_DstInc)// increase
      {
         LLP[i].dst_addr = (unsigned int)DstAddr + (i * LLPSize * (1 << SrcWidth));
         if(UnalignFlag)
         {
            LLP[i].dst_addr = (unsigned int)DstAddr + (i * LLPSize );
         }           
      }
      else if(DstCtrl == AHBDMA_DstFix)// fixed
         LLP[i].dst_addr = (unsigned int)DstAddr;

      *((unsigned int *)&(LLP[i].llp_ctrl)) = 0;      //init llp_ctrl as 0
      LLP[i].llp_ctrl.dst_ctrl = DstCtrl; ///* destination increment, decrement or fix 
      LLP[i].llp_ctrl.src_ctrl = SrcCtrl; ///* source increment, decrement or fix 
      
      if (UnalignFlag)
      {
         LLP[i].llp_ctrl.dst_width = DMA_GetAXIBusWidth(); ///* destination transfer size 
         LLP[i].llp_ctrl.src_width = DMA_GetAXIBusWidth(); ///* source transfer size 
      }
      else
      {
         LLP[i].llp_ctrl.dst_width = DstWidth; ///* destination transfer size 
         LLP[i].llp_ctrl.src_width = SrcWidth; ///* source transfer size 
      }

      *((unsigned int *)&(LLP[i].llp)) = 0;     //init llp as 0
      if(i==(LLPCount-1))
      {
         //the last LLP
         LLP[i].llp.link_list_addr = 0;
         LLP[i].llp_ctrl.tc_msk = 0;   // Enable tc status
         LLP[i].TotalSize = Size - LLPSize * i ;
      }else{
         LLP[i].llp.link_list_addr = (unsigned int)&LLP[i+1];
         LLP[i].llp_ctrl.tc_msk = 1;
         LLP[i].TotalSize = LLPSize;   //the unit is SrcWidth  / (1 << SrcWidth);
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
int DMA_NoLLD(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int Size,      // total bytes
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode,       // Normal/Hardwire,   0/1
int   unalign_flag
)
{
   DMA_CH_t DMAChannel;
   unsigned int Size_temp=Size;

   if (Size_temp > AHBDMA_MAX_LLDSIZE)//2**22-4, now, one LLP support 4M-1(22 bits)
   {
      printf("size > %d Please use DMA_NormalMode()\n", AHBDMA_MAX_LLDSIZE);
      return FALSE;
   }
   /* clear channel */
   DMA_ResetChannel(Channel);
   ClearDMAChannelIntStatus(Channel);
   /* initialize DMAChannel */
   memset(&DMAChannel, 0, sizeof(DMA_CH_t));
   /* program channel CSR */
   DMAChannel.csr.tc_msk = 0;  /* no linked list, no mask, there will be interrupt (Do not mask the terminal count status update)*/
   DMAChannel.csr.src_tcnt= SrcSize; /* source burst size */
   DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
   DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
   DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
   DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
#ifdef DMA_EVENT_TEST //ycmo: a little dirty, wait for CTD's fix
   if(Channel == 0)
   {
      DMAChannel.csr.exp_en = 1;
      DMAChannel.csr.sevent_en = 1;
      DMAChannel.csr.ch_sevent = 3;
   } 
   else if(Channel == 1)
   {
      DMAChannel.csr.exp_en = 1;
      DMAChannel.csr.wevent_en = 1;
      DMAChannel.csr.ch_wevent = 8;
   }
#endif
   DMA_SetCHCsr(Channel, DMAChannel.csr);

   /* program channel CFG */
   DMAChannel.cfg.int_tc_msk = 0;   // Disable terminal count interrupt, if tc then interrupt
   DMAChannel.cfg.int_err_msk = 0;  // Disable error interrupt
   DMAChannel.cfg.int_abt_msk = 0;  // Disable abort interrupt => modify by Jerry
   if(Mode==AHBDMA_NormalMode)
   {
      DMAChannel.cfg.src_hen = 0;
      DMAChannel.cfg.src_rs = 0;
      DMAChannel.cfg.dst_hen= 0;
      DMAChannel.cfg.dst_rs = 0;
   }else{//should modify if enable handshake mode
      //AHBDMA_HwHandShakeMode, to do...
      if(SrcCtrl == AHBDMA_SrcFix)
      {
         DMAChannel.cfg.src_hen = 1;
         DMAChannel.cfg.src_rs = 3;
      }
      else if(DstCtrl == AHBDMA_DstFix)
      {
         DMAChannel.cfg.dst_hen = 1;
         DMAChannel.cfg.dst_rs = 2;
      }
   }
   
   DMAChannel.cfg.ch_gntwin = 0;    //channel grant window 
   DMAChannel.cfg.reserved = 0;  //reserved bits are RO
   DMAChannel.cfg.reserved1 = 0; //reserved1 bits are RO
   DMAChannel.cfg.llp_cnt = 0;      //llp_cnt bits are RO
   DMAChannel.cfg.ch_pri = Priority;   //ChPri-channel arbitration priority
   
   DMA_SetCHCfg(Channel, DMAChannel.cfg);

   /* program channel llp */
   *((unsigned int *)&(DMAChannel.llp)) = 0; //no LLP, this is to set link_list_addr = 0
   DMA_CHLinkList(Channel, DMAChannel.llp);
   DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, Size);
   *((unsigned int *)&(DMAChannel.llp)) = 1; //this is to set master_id = 1, and link_list_addr = 0
   return TRUE;
}

// LinkAddr is an pre-assigned space for link list, and the size should be (at least)
// (LLPCount-1)*sizeof(DMA_LLD_t). The last one LLP is not used.
// LLPCount is the total number of the link list
// parameter size must be smaller than LLPCount*LLPSize
//
int DMA_NormalMode(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int LinkAddr,  // Link-List address, an pre-assigned space for link list
unsigned int LLPCount,  // total link-list node, if NO link list is needed, LLPCount is 0
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int Size,      // total bytes
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode,       // Normal/Hardwire,   0/1
int   unalign_flag,
unsigned int des_LLD
)
{
   DMA_CH_t DMAChannel;
   unsigned int LLPSize;
   unsigned int xCount,yCount;
   LLPSize=AHBDMA_MAX_LLDSIZE;
   
   /* program channel */
   DMA_ResetChannel(Channel);
   ClearDMAChannelIntStatus(Channel);
   memset(&DMAChannel, 0, sizeof(DMA_CH_t));
   
   if(LinkAddr != 0)
   {
      //printf("[DMA_NormalMode] Call DMA_FillMemLLD!!\n");
      DMA_FillMemLLD( LinkAddr, LLPCount, SrcAddr, DstAddr, Size, SrcWidth, DstWidth, SrcCtrl, DstCtrl, unalign_flag);
      /* program channel CSR */
      DMAChannel.csr.ch_en= 0; /* not yet enable */
      DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
      DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
      DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
      DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
      DMAChannel.csr.src_tcnt= SrcSize; /* source burst size */
      DMAChannel.csr.tc_msk = 1; /* disable terminal count */

      DMA_SetCHCsr(Channel, DMAChannel.csr);
   
      /* program channel CFG */
      DMAChannel.cfg.int_tc_msk = 0;   // Enable tc status, if tc_msk is disabled, this bit is useless
      DMAChannel.cfg.int_err_msk = 0;
      DMAChannel.cfg.int_abt_msk = 0;

      DMAChannel.cfg.src_rs= 0;
      DMAChannel.cfg.src_hen= 0;
      DMAChannel.cfg.dst_rs= 0;
      DMAChannel.cfg.dst_hen= 0;
      DMAChannel.cfg.ch_pri = Priority;
      DMAChannel.cfg.ch_gntwin = 0;    //grant window RO
      DMAChannel.cfg.reserved1 = 0; //reserved1 bits are RO
      DMAChannel.cfg.llp_cnt = 0;      //llp_cnt bits are RO
      //DMAChannel.cfg.reserved2 = 0;  //reserved2 bits are RO
           
      DMA_SetCHCfg(Channel, DMAChannel.cfg);

      /* program channel llp */
      *((unsigned int *)&(DMAChannel.llp)) = 0;    //Init IP llp
      DMAChannel.llp.link_list_addr = LinkAddr;
      
      DMA_CHLinkList(Channel, DMAChannel.llp);

      /* porgram address and size */
      DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, LLPSize);

      DMA_SetLDMAddr(DMA030_LDM_BASE);
   }
   else
   {   //for LLPCount==1,
      //printf("[DMA_NormalMode] Call DMA_NoLLD!!\n");
      if(DMA_NoLLD(Channel, SrcAddr, DstAddr, Size, SrcWidth, DstWidth,
                  SrcSize, SrcCtrl, DstCtrl, Priority, Mode, unalign_flag)==FALSE)
         return FALSE;
   }
   return TRUE;
}

int DMA_MemToMem0_Burnin(unsigned int ChannelNum, unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int Size, int Endian_flag)
{
   unsigned int i, SrcWidth, DstWidth, SrcSize,AxiWidth,Bus128Flag;
   unsigned int Src_sel, Dst_sel;//0:fix 1:incremental
   DMA_FEATURE_t *Feature;
   unsigned int *TempAddr;
   unsigned char str[128];
   
   unsigned int Channel;
   unsigned int *SrcAddr_tmp = SrcAddr;
   unsigned int *DstAddr_tmp = DstAddr;
   unsigned int LinkAddr = DMA030_LLD_SYSMEM;
   unsigned int LinkAddr_tmp = LinkAddr;
   unsigned int Size_temp=Size;
   unsigned int total_channel;
   
   Bus128Flag=0;
   if(Endian_flag)
      EnableDMAEndian();
   
   AxiWidth=DMA_GetAXIBusWidth();
   if((AxiWidth==4)&&(DMA_GetDepthOfFIFO()<=2))
      Bus128Flag=1;
   printf("\nDMA:Src=0x%08x, Dst=0x%08x, Size=0x%x\n", 
                        (unsigned int)SrcAddr, (unsigned int)DstAddr, (unsigned int)Size);
                        
   for(SrcWidth = 0; SrcWidth <= AxiWidth; SrcWidth++)
   { 
      for(DstWidth = 0; DstWidth <= AxiWidth; DstWidth++)
      {
         if(Bus128Flag)
         {
            if((SrcWidth==0)&&(DstWidth ==4))
               continue;
         }
         printf("SrcWidth=%d, DstWidth=%d\n",SrcWidth, DstWidth);
         SrcAddr_tmp = SrcAddr;
         DstAddr_tmp = DstAddr;
         LinkAddr_tmp = LinkAddr;
         Size_temp=Size;
         
         for(Channel=0; Channel<CHANNEL_NUM; Channel++)
         {
            for(i = 0; i < (Size/4); i++)
               SrcAddr_tmp[i] = (unsigned int)(SrcAddr_tmp + i);
            SrcSize = 0;
            Size_temp=Size;
            
            memset(DstAddr_tmp, 0, Size);
            
            DMA_Start( Channel, LinkAddr_tmp, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, &Size_temp, SrcWidth, DstWidth, SrcSize,
                   AHBDMA_SrcInc, AHBDMA_DstInc, AHBDMA_PriorityLow,
                   DMA_CSR_MODE_NORMAL, 0);
            SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
            DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
            LinkAddr_tmp = (unsigned int)((unsigned int)LinkAddr_tmp + (unsigned int)AddrBaseInc);
         }
         
       #ifndef DMA_PERFORMANCE_TEST
         for(Channel=0; Channel<CHANNEL_NUM; Channel++)
         {
            if(Endian_flag)
            {
               EnableDMAChannelEndianConverter(Channel); 
            }
            EnableDMAChannel(Channel);
         }
         
         for(Channel=0; Channel<CHANNEL_NUM; Channel++)
            DMA_WaitIntStatus(Channel);
            
         if(Endian_flag)
               DisableDMAChannelEndianConverter(Channel);
      #endif
      
         SrcAddr_tmp = SrcAddr;
         DstAddr_tmp = DstAddr;
         
         for(Channel=0; Channel< CHANNEL_NUM; Channel++)
         {
            if(Endian_flag)
            {
               if(CompareMemoryEndian(SrcAddr_tmp, DstAddr_tmp,SrcWidth, Size_temp)==FALSE)
               {
                  printf("\nSrc(Inc)-->Dst(Inc) Failed");
                  return FALSE;
               }
            }
            else
            {  
               if(CompareMemory(SrcAddr_tmp, DstAddr_tmp, Size_temp)==FALSE)
               {
                  printf("\nSrc(Inc)-->Dst(Inc) Failed");
                  return FALSE;
               }
            }
            
            printf("DMA:Channel:%d, Src=0x%08x, Dst=0x%08x, Size=0x%x, SrcWidth=0x%x, DstWidth=0x%x\n", 
                          Channel, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, (unsigned int)Size_temp, (unsigned int)SrcWidth, (unsigned int)DstWidth);
            printf("-->Pass\n");
            SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
            DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
         }
         printf("\n");
      }
   }
   return TRUE;
}

int DMA_MemToMem0_Unalign_Burnin(unsigned int ChannelNum, unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int Size)
{
   unsigned int i, SrcWidth, DstWidth, SrcSize,AxiWidth,Bus128Flag;
   unsigned int Src_sel, Dst_sel, Hw_feature;
   DMA_FEATURE_t *Feature;
   unsigned int Temp;
   unsigned char str[128];
   
   unsigned int Channel;
   unsigned int *SrcAddr_tmp = SrcAddr;
   unsigned int *DstAddr_tmp = DstAddr;
   unsigned int LinkAddr = DMA030_LLD_SYSMEM;
   unsigned int LinkAddr_tmp = LinkAddr;
       
   Bus128Flag=0;    
   if(!DMA_UnalignFeature())
   {
      printf("DMAC doesn't support unalign mode\n");
      return FALSE;
   }  
   printf("Start DMA memory to memory unalign 1-D test\n");
   
   AxiWidth=DMA_GetAXIBusWidth();
   
   SrcWidth=AxiWidth;
   DstWidth=SrcWidth;

   SrcAddr_tmp = SrcAddr;
   DstAddr_tmp = DstAddr;
   LinkAddr_tmp = LinkAddr;
   printf("\nDMA:Src=0x%08x, Dst=0x%08x, Size=0x%x\n", 
      (unsigned int)SrcAddr, (unsigned int)DstAddr, (unsigned int)Size);
   printf("SrcWidth%d, DstWidth%d\n",SrcWidth, DstWidth);
   
   for(Channel=0; Channel<CHANNEL_NUM; Channel++)
   {  
      for(i = 0; i < (Size/4); i++)
         SrcAddr_tmp[i] = (unsigned int)(SrcAddr_tmp + i);
      SrcSize = 0;
      
      memset(DstAddr_tmp, 0, Size);
      DMA_Start( Channel, LinkAddr_tmp, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, &Size, SrcWidth, DstWidth, SrcSize,
         AHBDMA_SrcInc, AHBDMA_DstInc, AHBDMA_PriorityLow,
         DMA_CSR_MODE_NORMAL,1);
      SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
      DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
      LinkAddr_tmp = (unsigned int)((unsigned int)LinkAddr_tmp + (unsigned int)AddrBaseInc);
   }
      
   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
   {  
      EnableDMAChannelUnalign(Channel);
      EnableDMAChannel(Channel); 
   }
   
   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
      DMA_WaitIntStatus(Channel);
   
   SrcAddr_tmp = SrcAddr;
   DstAddr_tmp = DstAddr;
   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
   {
      if(CompareMemory(SrcAddr_tmp, DstAddr_tmp, Size)==FALSE)
      {
         printf("\nSrc(Inc)-->Dst(Inc) Failed!! Channel=%d\n",Channel);
         return FALSE;
      }
   
      printf("DMA:Channel:%d, Src=0x%08x, Dst=0x%08x, Size=0x%x, SrcWidth=0x%x, DstWidth=0x%x\n", 
         Channel, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, (unsigned int)Size, (unsigned int)SrcWidth, (unsigned int)DstWidth);
      printf("-->Pass\n");
      SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
      DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
   }
   printf("\n");
   return TRUE;
}

/*************** DMA 2D Test Function *****************************************/

// This function will start the DMA and return until finish
unsigned int DMA_Start_2D(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int LinkAddr,  // Link-List address
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int* XTcnt,
unsigned int YTcnt,
unsigned int SrcStride,
unsigned int DstStride,
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode,       // Normal/Hardwire,   0/1
int unalign_flag,
unsigned int des_max_xcnt, 
unsigned int des_max_ycnt
)
{
   unsigned int transfer_size;
   unsigned int LLPSize, Count;
   int LengthMultiple=0;

   DisableDMAChannel(Channel);

   if(!unalign_flag)
   {
      LengthMultiple=AlignPackMultiple(SrcWidth,DstWidth);
      transfer_size = ((unsigned short)*XTcnt) / (1 << SrcWidth);
      if(LengthMultiple)
      {
         if(transfer_size % LengthMultiple)
         {
            transfer_size= LengthMultiple* (transfer_size /LengthMultiple);// transfer_size(trasfer count=>get a round number
            *XTcnt=transfer_size*(1 << SrcWidth);
            printf("Channel%d has Data Packing. Transfer XCNT=0x%x\n",Channel,*XTcnt);
         }
      }
      *XTcnt=transfer_size*(1 << SrcWidth);  
   }
   else
      transfer_size = *XTcnt;
   
   LLPSize = des_max_xcnt;
   if(transfer_size > LLPSize)
   {  //the Size is too large to move in one LLP in the IP
      Count = (transfer_size + (LLPSize - 1)) / LLPSize;
      //LinkAddr =  DMA030_LLD_SYSMEM;
   }
   else
   {
      Count=1;
      LinkAddr = 0;
   }
   if(DMA_NormalMode_2D(Channel, LinkAddr, Count-1, SrcAddr, DstAddr, transfer_size, YTcnt, SrcStride, DstStride, SrcWidth, DstWidth,
                  SrcSize, SrcCtrl, DstCtrl, Priority, Mode,unalign_flag,des_max_xcnt, des_max_ycnt)==FALSE){
      if(LinkAddr)
         free((void*)LinkAddr);
      return;
   }
#ifdef DMA_PERFORMANCE_TEST
{
   unsigned int StartTick, EndTick;
   T1_Tick = 0;
   fLib_Timer_Init(1, TICK_HZ, Timer1_Tick);
   StartTick = fLib_CurrentT1Tick();

   EnableDMAChannel(Channel);
   DMA_WaitIntStatus(Channel);

   EndTick = fLib_CurrentT1Tick();
   if ( EndTick != StartTick){
      printk("\nspeed:%7d B/s,Size:0x%x,SW:%d,DW:%d,SSize:%d",(int)((XTcnt*YTcnt)/((double)(EndTick-StartTick)/(double)TICK_HZ)), (XTcnt*YTcnt), SrcWidth, DstWidth, SrcSize);
   }else{
      printk("\n!!--StartTick=%d and EndTick=%d are the same\n",StartTick, EndTick);
   }
   fLib_Timer_Close(1);
}
#endif

   return ;
}

void DMA_FillMemLLD_2D(
unsigned int LinkAddr,  // Link-List address
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int XTcnt,      //
unsigned int YTcnt,
unsigned int SrcStride,
unsigned int DstStride,
unsigned int SrcWidth,  // source width 8/16/32 bits -> 0/1/2
unsigned int DstWidth,  // dest width 8/16/32 bits -> 0/1/2
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
int unalign_flag,
unsigned int des_max_xcnt, 
unsigned int des_max_ycnt
)
{
   unsigned int y, x;
   unsigned int llp_cnt = 0;
   unsigned int xllp_cnt,yllp_cnt;  // total link-list node

   unsigned int srcaddr_tmp = SrcAddr;
   unsigned int dstaddr_tmp = DstAddr;
   DMA_LLD_t *LLP=(DMA_LLD_t *)LinkAddr;

   if (LinkAddr==0) //LLPCount must >=1
      return ;
   xllp_cnt=1;
   yllp_cnt=1;

    if(XTcnt > des_max_xcnt)  //the Size is too large to move in one LLP in the IP
      xllp_cnt=((XTcnt )+(des_max_xcnt - 1))/des_max_xcnt;

   if(YTcnt > des_max_ycnt)
      yllp_cnt=((YTcnt )+ (des_max_ycnt - 1))/des_max_ycnt;

   for(y = 0; y < yllp_cnt; y++) 
   {
      for(x = 0; x < xllp_cnt; x++)
      {
         if((x==0)&&(y==0))
            continue;
         if(unalign_flag)
         {
            LLP[llp_cnt].src_addr = (unsigned int)srcaddr_tmp + x * des_max_xcnt;
            LLP[llp_cnt].dst_addr = (unsigned int)dstaddr_tmp + x * des_max_xcnt;
         }
         else
         {
            LLP[llp_cnt].src_addr = (unsigned int)srcaddr_tmp + x * des_max_xcnt*(1<<SrcWidth);
            LLP[llp_cnt].dst_addr = (unsigned int)dstaddr_tmp + x * des_max_xcnt*(1<<SrcWidth);
         }

         *((unsigned int *)&(LLP[llp_cnt].llp_ctrl)) = 0;  //init llp_ctrl as 0
         LLP[llp_cnt].llp_ctrl.dst_ctrl = DstCtrl; ///* destination increment, decrement or fix 
         LLP[llp_cnt].llp_ctrl.src_ctrl = SrcCtrl; ///* source increment, decrement or fix 
         LLP[llp_cnt].llp_ctrl.dst_width = DstWidth; ///* destination transfer size 
         LLP[llp_cnt].llp_ctrl.src_width = SrcWidth; ///* source transfer size 
         LLP[llp_cnt].Stride = (DstStride << 16) | SrcStride;
         *((unsigned int *)&(LLP[llp_cnt].llp)) = 0;  //init llp as 0
         LLP[llp_cnt].llp.link_list_addr = (unsigned int)&LLP[llp_cnt + 1];
         LLP[llp_cnt].llp_ctrl.tc_msk = 1;
         LLP[llp_cnt].llp_ctrl.exp_en=1;
         LLP[llp_cnt].llp_ctrl.twod_en=1;

         if(llp_cnt == (yllp_cnt * xllp_cnt-2 ))//the last LLP
         {
            LLP[llp_cnt].llp.link_list_addr = 0;
            LLP[llp_cnt].llp_ctrl.tc_msk = 0; // Enable tc status
            if((YTcnt%des_max_ycnt)&&(XTcnt%des_max_xcnt) )
               LLP[llp_cnt].TotalSize =(YTcnt%des_max_ycnt)<<16 |(XTcnt%des_max_xcnt);
            else if((!(YTcnt%des_max_ycnt))&&(XTcnt%des_max_xcnt) )
               LLP[llp_cnt].TotalSize = (des_max_ycnt << 16) |(XTcnt%des_max_xcnt);
            else if((YTcnt%des_max_ycnt)&&(!(XTcnt%des_max_xcnt)) )
               LLP[llp_cnt].TotalSize =(YTcnt%des_max_ycnt)<<16 |(des_max_xcnt);
            else if((!(YTcnt%des_max_ycnt))&&(!(XTcnt%des_max_xcnt)) )
               LLP[llp_cnt].TotalSize = (des_max_ycnt << 16) | des_max_xcnt;
            break;
         }
         else if(x==(xllp_cnt-1))
         {
            if(XTcnt%des_max_xcnt)
               LLP[llp_cnt].TotalSize =(des_max_ycnt << 16) |(XTcnt%des_max_xcnt);
            else
               LLP[llp_cnt].TotalSize = (des_max_ycnt << 16) | des_max_xcnt;
         }
         else if(y==(yllp_cnt-1))
         {
            if(YTcnt%des_max_ycnt)
               LLP[llp_cnt].TotalSize =(YTcnt%des_max_ycnt)<<16 |des_max_xcnt;
            else
               LLP[llp_cnt].TotalSize = (des_max_ycnt << 16) | des_max_xcnt;
         }
         else
         {
            LLP[llp_cnt].TotalSize = (des_max_ycnt << 16) | des_max_xcnt; //the unit is SrcWidth  / (1 << SrcWidth);
         }
         llp_cnt ++;
      }

      if(llp_cnt == (yllp_cnt * xllp_cnt-1))//the last llP
         break;

      srcaddr_tmp = SrcAddr+ (y+1)*SrcStride*des_max_ycnt;
      dstaddr_tmp = DstAddr+ (y+1)*DstStride*des_max_ycnt;
   }
   return;
}

int DMA_NoLLD_2D(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int XTcnt,      //
unsigned int YTcnt,
unsigned int SrcStride,
unsigned int DstStride,
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode,       // Normal/Hardwire,   0/1
int   unalign_flag,
unsigned int des_max_xcnt, 
unsigned int des_max_ycnt
)
{
   DMA_CH_t DMAChannel;

   if (XTcnt > des_max_xcnt )
   {
      printf("size > %d Please use DMA_NormalMode()\n", des_max_xcnt);
      return FALSE;
   }
   /* clear channel */
   DMA_ResetChannel(Channel);
   ClearDMAChannelIntStatus(Channel);
        memset(&DMAChannel, 0, sizeof(DMA_CH_t));

   /* program channel CSR */
   DMAChannel.csr.tc_msk = 0;  /* no linked list, no mask, there will be interrupt */
   DMAChannel.csr.src_tcnt= SrcSize; /* source burst size */
   DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
   DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
   DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
   DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
   DMAChannel.csr.twod_en = 1;
   DMAChannel.csr.exp_en = 1;
   DMA_SetCHCsr(Channel, DMAChannel.csr);

   /* program channel CFG */
   DMAChannel.cfg.int_tc_msk = 0;   // Enable terminal count interrupt, if tc then interrupt
   DMAChannel.cfg.int_err_msk = 0;  // Enable error interrupt
   DMAChannel.cfg.int_abt_msk = 0;  // Enable abort interrupt => modify by Jerry
   if(Mode==AHBDMA_NormalMode)
   {
      DMAChannel.cfg.src_hen = 0;
      DMAChannel.cfg.src_rs = 0;
      DMAChannel.cfg.dst_hen= 0;
      DMAChannel.cfg.dst_rs = 0;
   }
   else
   {//should modify if enable handshake mode
      //AHBDMA_HwHandShakeMode, to do...
      if(SrcCtrl == AHBDMA_SrcFix)
      {
         DMAChannel.cfg.src_hen = 1;
         DMAChannel.cfg.src_rs = 3;
      }
      else if(DstCtrl == AHBDMA_DstFix)
      {
         DMAChannel.cfg.dst_hen = 1;
         DMAChannel.cfg.dst_rs = 2;
      }
   }
   DMAChannel.cfg.ch_gntwin = 0;    //channel grant window 
   DMAChannel.cfg.reserved = 0;  //reserved bits are RO
   DMAChannel.cfg.reserved1 = 0; //reserved1 bits are RO
   DMAChannel.cfg.llp_cnt = 0;      //llp_cnt bits are RO
   DMAChannel.cfg.ch_pri = Priority;   //ChPri-channel arbitration priority

   DMA_SetCHCfg(Channel, DMAChannel.cfg);

   /* program channel llp */
   *((unsigned int *)&(DMAChannel.llp)) = 0; //no LLP, this is to set link_list_addr = 0
   DMA_CHLinkList(Channel, DMAChannel.llp);
   DMA_CHDataCtrl_2D(Channel, SrcAddr, DstAddr, XTcnt, YTcnt, DstStride, SrcStride);
   *((unsigned int *)&(DMAChannel.llp)) = 1; //this is to set master_id = 1, and link_list_addr = 0
   return TRUE;
}


int CompareMemory_2D(unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int XTcnt, unsigned int YTcnt, unsigned int SrcStride, unsigned int  DstStride,unsigned int SrcWidth, int Endian_flag)
{
   int i = 0,j=0;

   for(j = 0; j < YTcnt; j++)
   {
      if(Endian_flag)
      {
         return CompareMemoryEndian(&SrcAddr[SrcStride*j/4],&DstAddr[DstStride*j/4], SrcWidth,XTcnt);
      }
      else
      {
         for(i = 0; i < (XTcnt / 4); i++)
         {
            if(DstAddr[i+(DstStride*j)/4] != SrcAddr[i+(SrcStride*j)/4]){
               printf("\n========>DstAddr:0x%08X = 0x%08X, should be 0x%08X\n", (unsigned int)&DstAddr[i+DstStride*j/4], DstAddr[i+DstStride*j/4], SrcAddr[i+SrcStride*j/4]);
                  printf("IntStatus = %x\n",IntStatus);
               return FALSE;
            }
         }
      }
   }
   return TRUE;
}

int CompareMemory_2D_unalign(unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int XTcnt, unsigned int YTcnt, unsigned int SrcStride, unsigned int  DstStride)
{
   int i = 0;
   int j = 0;
   unsigned char* SrcAddr_byte=(unsigned char*)SrcAddr; 
   unsigned char* DstAddr_byte=(unsigned char*)DstAddr; 

   for(j = 0; j < YTcnt; j++)
   {
      for(i = 0; i <XTcnt; i++)
      {
         if(DstAddr_byte[i+(DstStride*j)] != SrcAddr_byte[i+(SrcStride*j)])
         {
            printf("\n========>DstAddr:0x%08X = 0x%08X, should be 0x%08X\n", (unsigned int)&DstAddr_byte[i+DstStride*j], DstAddr_byte[i+DstStride*j], SrcAddr_byte[i+SrcStride*j]);
            printf("YTcnt=0x%x\n",j);
            printf("IntStatus = %x\n",IntStatus);
            return FALSE;
         }
      }
   }
   return TRUE;
}

int DMA_NormalMode_2D(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int LinkAddr,  // Link-List address, an pre-assigned space for link list
unsigned int LLPCount,  // total link-list node, if NO link list is needed, LLPCount is 0
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int XTcnt, 
unsigned int YTcnt, 
unsigned int SrcStride, 
unsigned int DstStride,
unsigned int SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode,       // Normal/Hardwire,   0/1
int   unalign_flag,
unsigned int des_max_xcnt, 
unsigned int des_max_ycnt
)
{
   DMA_CH_t DMAChannel;
   unsigned int LLPSize;//=AHBDMA_MAX_LLDSIZE;
   unsigned int xCount,yCount;
   LLPSize=AHBDMA_MAX_LLDSIZE;

   /* program channel */
   DMA_ResetChannel(Channel);
   ClearDMAChannelIntStatus(Channel);
   memset(&DMAChannel, 0, sizeof(DMA_CH_t));

   if((XTcnt <= des_max_xcnt)&&(YTcnt <= des_max_ycnt))
      LinkAddr = 0; 

   if(LinkAddr != 0)
   {
      //printf("[DMA_NormalMode_2D] Call DMA_FillMemLLD_2D!!\n");
      DMA_FillMemLLD_2D( LinkAddr, SrcAddr, DstAddr, XTcnt, YTcnt, SrcStride, DstStride, SrcWidth, DstWidth, SrcCtrl, DstCtrl,unalign_flag,des_max_xcnt,des_max_ycnt);

      /* program channel CSR */
      DMAChannel.csr.ch_en= 0; /* not yet enable */
      DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
      DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
      DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
      DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
      DMAChannel.csr.src_tcnt= SrcSize; /* source burst size */
      DMAChannel.csr.tc_msk = 1; /* disable terminal count */

      DMAChannel.csr.twod_en = 1;
      DMAChannel.csr.exp_en = 1;

      DMA_SetCHCsr(Channel, DMAChannel.csr);

      /* program channel CFG */
      DMAChannel.cfg.int_tc_msk = 0;   // Enable tc status, if tc_msk is disabled, this bit is useless
      DMAChannel.cfg.int_err_msk = 0;
      DMAChannel.cfg.int_abt_msk = 0;

      DMAChannel.cfg.src_rs= 0;
      DMAChannel.cfg.src_hen= 0;
      DMAChannel.cfg.dst_rs= 0;
      DMAChannel.cfg.dst_hen= 0;
      DMAChannel.cfg.ch_pri = Priority;
      DMAChannel.cfg.ch_gntwin = 0;    //grant window RO
      DMAChannel.cfg.reserved1 = 0; //reserved1 bits are RO
      DMAChannel.cfg.llp_cnt = 0;      //llp_cnt bits are RO

      DMA_SetCHCfg(Channel, DMAChannel.cfg);

      /* program channel llp */
      *((unsigned int *)&(DMAChannel.llp)) = 0;    //Init IP llp
      DMAChannel.llp.link_list_addr = LinkAddr;
      
      DMA_CHLinkList(Channel, DMAChannel.llp);

      /* porgram address and size */

      if(XTcnt >= des_max_xcnt)  //the Size is too large to move in one LLP in the IP
         xCount=des_max_xcnt;
      else
         xCount=XTcnt;
      if(YTcnt >= des_max_ycnt)  //the Size is too large to move in one LLP in the IP
         yCount=des_max_ycnt;
      else
         yCount=YTcnt;     
      DMA_CHDataCtrl_2D(Channel, SrcAddr, DstAddr, xCount, yCount, DstStride, SrcStride);

      DMA_SetLDMAddr(DMA030_LDM_BASE);
   }
   else
   {   //for LLPCount==1,
      //printf("[DMA_NormalMode_2D] Call DMA_NoLLD_2D!!\n");
      if(DMA_NoLLD_2D(Channel, SrcAddr, DstAddr, XTcnt, YTcnt, SrcStride, DstStride, SrcWidth, DstWidth,
                  SrcSize, SrcCtrl, DstCtrl, Priority, Mode,  unalign_flag,des_max_xcnt,des_max_ycnt)==FALSE)
         return FALSE;
   }
   return TRUE;
}

int DMA_MemToMem2D(unsigned int XTcnt, unsigned int YTcnt, unsigned int SrcStride, 
         unsigned int DstStride,int Endian_flag, unsigned int des_max_xcnt, unsigned int des_max_ycnt)
{
   unsigned int i, SrcWidth, DstWidth, SrcSize,AxiWidth,Bus128Flag;
   unsigned int Src_sel, Dst_sel;//0:fix 1:incremental
   DMA_FEATURE_t *Feature;
   unsigned int *TempAddr;
   unsigned char str[128];
   unsigned int *SrcAddr=(unsigned int *)SRCADDR; 
   unsigned int *DstAddr=(unsigned int *)DSTADDR;
   unsigned int temp;
   unsigned int *SrcAddr_tmp = SrcAddr;
   unsigned int *DstAddr_tmp = DstAddr;
   unsigned int LinkAddr = DMA030_LLD_SYSMEM;
   unsigned int LinkAddr_tmp = LinkAddr;
   int unalign=0;
   int Channel;
   unsigned int XTcnt_temp=XTcnt; //In order to handle align mode data packing 
   Bus128Flag=0;

   if(Endian_flag)
      EnableDMAEndian();

   AxiWidth=DMA_GetAXIBusWidth();

   if((AxiWidth==4)&&(DMA_GetDepthOfFIFO()<=2))
      Bus128Flag=1;
   printf("\nDMA: SrcAddr=0x%x DstAddr=0x%x,Total XTcnt =0x%x, Total YTcnt=0x%x, SrcStride=0x%x, DstStride=0x%x,Desciptor xcnt=0x%x,Desciptor ycnt=0x%x", 
         (unsigned int)SrcAddr, (unsigned int)DstAddr, (unsigned int)XTcnt, (unsigned int)YTcnt, (unsigned int)SrcStride, (unsigned int)DstStride,des_max_xcnt,des_max_ycnt);

   for(SrcWidth = 0; SrcWidth <= AxiWidth; SrcWidth++) 
   {
      for(DstWidth = 0; DstWidth <= AxiWidth; DstWidth++)
      {
         if(Bus128Flag)
         {
            if((SrcWidth==0)&&(DstWidth ==4))
            continue;
         }

         SrcAddr_tmp = SrcAddr;
         DstAddr_tmp = DstAddr;
         LinkAddr_tmp = LinkAddr;

         printf("\nSrcWidth=%d, DstWidth=%d\n", SrcWidth, DstWidth);
         for(Channel=0; Channel< CHANNEL_NUM; Channel++)
         {
            SrcSize = 0;
            XTcnt_temp=XTcnt;
            for(i=0;i<YTcnt;i++)
               memset(DstAddr+(DstStride*i)/4, 0, XTcnt_temp);

            DMA_Start_2D( Channel,LinkAddr_tmp, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, &XTcnt_temp, YTcnt, SrcStride, DstStride, SrcWidth, DstWidth, SrcSize,AHBDMA_SrcInc, AHBDMA_DstInc, AHBDMA_PriorityLow,DMA_CSR_MODE_NORMAL, unalign, des_max_xcnt, des_max_ycnt);
            
            SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
            DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
            LinkAddr_tmp = (unsigned int)((unsigned int)LinkAddr_tmp + (unsigned int)AddrBaseInc);
         }

         for(Channel=0; Channel<CHANNEL_NUM; Channel++)
         {
            if(Endian_flag)
               EnableDMAChannelEndianConverter(Channel); 
            EnableDMAChannel(Channel);
         }

         for(Channel=0; Channel<CHANNEL_NUM; Channel++)
            DMA_WaitIntStatus(Channel);

         SrcAddr_tmp = SrcAddr;
         DstAddr_tmp = DstAddr;

         if(Endian_flag)
            DisableDMAChannelEndianConverter(Channel);

         for(Channel=0; Channel< CHANNEL_NUM; Channel++)
         {
            if(CompareMemory_2D(SrcAddr_tmp, DstAddr_tmp, XTcnt_temp, YTcnt, SrcStride, DstStride,SrcWidth,Endian_flag)==FALSE)
            {
               printf("\nSrc(Inc)-->Dst(Inc) Failed");
               printf("\nFail:Src=0x%08x, Dst=0x%08x, XTcnt=0x%x, YTcnt=0x%x,DescriptorXcnt=0x%x, DescriptorYcnt=0x%x , SrcStride=0x%x, DstStride=0x%x\n",(unsigned int)SrcAddr, (unsigned int)DstAddr, (unsigned int)XTcnt, (unsigned int)YTcnt, des_max_xcnt, des_max_ycnt, (unsigned int)SrcStride, (unsigned int)DstStride);
               return FALSE;
            }

            printf("DMA:Channel:%d, Src=0x%08x, Dst=0x%08x, Xcount=0x%x,Ycount=0x%x, SrcWidth=0x%x, DstWidth=0x%x\n", 
                       Channel, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, (unsigned int)XTcnt_temp,  (unsigned int)YTcnt,(unsigned int)SrcWidth, (unsigned int)DstWidth);
            printf("-->Pass\n");

            SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
            DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
         }
      }
   }
   printf("DMA_MemToMem2D Test OK, XTcnt:0x%x, YTcnt:0x%x\n",  XTcnt,YTcnt);
   return TRUE;
}

int DMA_MemToMem2D_Unalign(unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int XTcnt, 
            unsigned int YTcnt, unsigned int SrcStride, unsigned int DstStride, unsigned int des_max_xcnt, unsigned int des_max_ycnt)
{
   unsigned int i, SrcWidth, DstWidth, SrcSize,AxiWidth,Bus128Flag;
   unsigned int Src_sel, Dst_sel;
   DMA_FEATURE_t *Feature;
   unsigned int *TempAddr;
   unsigned char str[128];

   unsigned int temp;
   unsigned int *SrcAddr_tmp = SrcAddr;
   unsigned int *DstAddr_tmp = DstAddr;
   unsigned int LinkAddr = DMA030_LLD_SYSMEM;
   unsigned int LinkAddr_tmp = LinkAddr;
   int unalign=1;
   int Channel;
   unsigned int XTcnt_temp=XTcnt;

   Bus128Flag=0;

   AxiWidth=DMA_GetAXIBusWidth();

   SrcWidth=AxiWidth;
   DstWidth=SrcWidth;

   printf("\nSrc=0x%08x, Dst=0x%08x, XTcnt=0x%x, YTcnt=0x%x,DescriptorXcnt=0x%x, DescriptorYcnt=0x%x , SrcStride=0x%x, DstStride=0x%x\n",(unsigned int)SrcAddr, (unsigned int)DstAddr, (unsigned int)XTcnt, (unsigned int)YTcnt, des_max_xcnt, des_max_ycnt, (unsigned int)SrcStride, (unsigned int)DstStride);

   SrcAddr_tmp = SrcAddr;
   DstAddr_tmp = DstAddr;
   LinkAddr_tmp = LinkAddr;

   printf("\nSrcWidth=%d, DstWidth=%d\n", SrcWidth, DstWidth);
   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
   {
      SrcSize = 0;
      XTcnt_temp=XTcnt;
      for(i=0;i<YTcnt;i++)
         memset(DstAddr_tmp+(DstStride*i)/4, 0, XTcnt_temp);

      DMA_Start_2D( Channel, LinkAddr_tmp,(unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, &XTcnt_temp, YTcnt, SrcStride, DstStride, SrcWidth, DstWidth, SrcSize,
            AHBDMA_SrcInc, AHBDMA_DstInc, AHBDMA_PriorityLow,
            DMA_CSR_MODE_NORMAL, unalign, des_max_xcnt, des_max_ycnt);

      SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
      DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
      LinkAddr_tmp = (unsigned int)((unsigned int)LinkAddr_tmp + (unsigned int)AddrBaseInc);
   }

   for(Channel=0; Channel<CHANNEL_NUM; Channel++)
   {
      EnableDMAChannelUnalign(Channel);
      EnableDMAChannel(Channel);
   }

   for(Channel=0; Channel<CHANNEL_NUM; Channel++)
   {
      DMA_WaitIntStatus(Channel);
   }

   SrcAddr_tmp = SrcAddr;
   DstAddr_tmp = DstAddr;

   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
   {
      if(CompareMemory_2D_unalign(SrcAddr_tmp, DstAddr_tmp, XTcnt_temp, YTcnt, SrcStride, DstStride)==FALSE)
      {
         printf("Src(Inc)-->Dst(Inc) Failed!!");
         return FALSE;
      }
      printf("DMA:Channel:%d, Src=0x%08x, Dst=0x%08x, Xcount=0x%x,Ycount=0x%x, SrcWidth=0x%x, DstWidth=0x%x\n", 
                 Channel, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, (unsigned int)XTcnt_temp,  (unsigned int)YTcnt,(unsigned int)SrcWidth, (unsigned int)DstWidth);
      printf("-->Pass\n");
      
      SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
      DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
   }
   printf("DMA_MemToMem2D Test OK, XTcnt:0x%x, YTcnt:0x%x\n",  XTcnt,YTcnt);
   return TRUE;
}

int DMA_WriteConstant_Test(unsigned int ChannelNum, unsigned int *SrcAddr, unsigned int *DstAddr, unsigned int Size)
{
   unsigned int i, SrcWidth, DstWidth, SrcSize,AxiWidth;
   unsigned int Src_sel, Dst_sel, FIFO_Dep;
   DMA_FEATURE_t *Feature;
   unsigned int *TempAddr;
   unsigned char str[128];
   unsigned int AddrBaseInc = 0x200000;
   unsigned int Channel;
   unsigned int *SrcAddr_tmp = SrcAddr;
   unsigned int *DstAddr_tmp = DstAddr;
   unsigned int LinkAddr = DMA030_LLD_SYSMEM;
   unsigned int LinkAddr_tmp = LinkAddr;
   unsigned int ConstValue=0xffffffff;

   printf("\nPlease input write only mode constant value:");
   scanf( "%d", &ConstValue );

   DMA_SetWriteOnlyValue(ConstValue);

   AxiWidth=DMA_GetAXIBusWidth();
   SrcWidth=AxiWidth;
   DstWidth =SrcWidth; //source data width must eqaul to destination data width
   SrcAddr_tmp = SrcAddr;
   DstAddr_tmp = DstAddr;
   LinkAddr_tmp = LinkAddr;
   for(Channel=0; Channel<CHANNEL_NUM; Channel++)
   {
      printf("SrcAddr=0x%x,DstAddr=0x%x\n",SrcAddr_tmp,DstAddr_tmp);
      printf("\nSrcWidth%d, DstWidth%d, Channel%d\n",SrcWidth, DstWidth, Channel);
      for(i = 0; i < (Size/4); i++)
         SrcAddr_tmp[i] = (unsigned int)(SrcAddr_tmp + i);
      SrcSize = 0;
      fLib_DebugPrintString(str);
      memset(DstAddr_tmp, 0, Size);
      DMA_Start( Channel, LinkAddr_tmp, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, &Size, SrcWidth, DstWidth, SrcSize,
             AHBDMA_SrcInc, AHBDMA_DstInc, AHBDMA_PriorityLow,
             DMA_CSR_MODE_NORMAL,0);
      SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
      DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
      LinkAddr_tmp = (unsigned int)((unsigned int)LinkAddr_tmp + (unsigned int)AddrBaseInc);
   }

   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
   {
      EnableDMAChannelWriteOnly(Channel);
      EnableDMAChannel(Channel);
   }

   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
   {
      DMA_WaitIntStatus(Channel);
   }

   SrcAddr_tmp = SrcAddr;
   DstAddr_tmp = DstAddr;
   for(Channel=0; Channel< CHANNEL_NUM; Channel++)
   {
      if(CompareMemorywithValue(DstAddr_tmp, Size,ConstValue)==FALSE)
      {
         printf("\nDMA Write Only Mode Test fail, Size:0x%x\n", Size);
         return FALSE;
      }
      printf("\nDMA:Channel:%d, Src=0x%08x, Dst=0x%08x, Size=0x%x, SrcWidth=0x%x, DstWidth=0x%x\n", 
                    Channel, (unsigned int)SrcAddr_tmp, (unsigned int)DstAddr_tmp, (unsigned int)Size, (unsigned int)SrcWidth, (unsigned int)DstWidth);
      printf("-->Pass\n");
      SrcAddr_tmp = (unsigned int*)((unsigned int)SrcAddr_tmp + (unsigned int)AddrBaseInc);
      DstAddr_tmp = (unsigned int*)((unsigned int)DstAddr_tmp + (unsigned int)AddrBaseInc);
   }

   printf("\nDMA Write Only Mode Test Pass, Size:0x%x\n", Size);
   return; 
}

// LinkAddr is an pre-assigned space for link list, and the size should be (at least)
// (LLPCount-1)*sizeof(DMA_LLD_t). The last one LLP is not used.
// LLPCount is the total number of the link list
// parameter size must be smaller than LLPCount*LLPSize
int DMA_HandShakeMode(
unsigned int Channel,   // use which channel for AHB DMA, 0..7
unsigned int LinkAddr,  // Link-List address, an pre-assigned space for link list
unsigned int LLPCount,  // total link-list node, if NO link list is needed, LLPCount is 0
unsigned int SrcAddr,   // source begin address
unsigned int DstAddr,   // dest begin address
unsigned int Size,      // total bytes
unsigned int SrcWidth,  // source width 8/16/32 bits -> 0/1/2
unsigned int DstWidth,  // dest width 8/16/32 bits -> 0/1/2
unsigned int SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
unsigned int SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
unsigned int DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
unsigned int Priority,  // priority for this chaanel 0(low)/1/2/3(high)
unsigned int Mode       // Normal/Hardwire,   0/1
)
{
   DMA_CH_t DMAChannel;
   unsigned int LLPSize;//=AHBDMA_MAX_LLDSIZE;
   unsigned int xCount,yCount;

   LLPSize=AHBDMA_MAX_LLDSIZE;

   // program channel 
   DMA_ResetChannel(Channel);
   ClearDMAChannelIntStatus(Channel);
   memset(&DMAChannel, 0, sizeof(DMA_CH_t));

   if(LinkAddr != 0)
   {
      DMA_FillMemLLD( LinkAddr, LLPCount, SrcAddr, DstAddr, Size, SrcWidth, DstWidth, SrcCtrl, DstCtrl, 0);
      DMAChannel.csr.tc_msk = 0;  /* no linked list, no mask, there will be interrupt */
      DMAChannel.csr.src_tcnt= SrcSize; /* source burst size */
      DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
      DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
      DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
      DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
      DMAChannel.csr.tc_msk = 1;
      DMA_SetCHCsr(Channel, DMAChannel.csr);

      /* program channel CFG */
      DMAChannel.cfg.int_tc_msk = 0;   // Enable terminal count interrupt, if tc then interrupt
      DMAChannel.cfg.int_err_msk = 0;  // Enable error interrupt
      DMAChannel.cfg.int_abt_msk = 0;  // Enable abort interrupt => modify by Jerry
      if(Mode==AHBDMA_NormalMode)
      {
         DMAChannel.cfg.src_hen = 0;
         DMAChannel.cfg.src_rs = 0;
         DMAChannel.cfg.dst_hen= 0;
         DMAChannel.cfg.dst_rs = 0;
      }
      else
      {//should modify if enable handshake mode
         //AHBDMA_HwHandShakeMode, to do...
         if(SrcCtrl == AHBDMA_SrcFix)
         {
            DMAChannel.cfg.src_hen = 1;
            DMAChannel.cfg.src_rs = 3;
         }
         else if(DstCtrl == AHBDMA_DstFix)
         {
            DMAChannel.cfg.dst_hen = 1;
            DMAChannel.cfg.dst_rs = 2;
         }
      }

      DMAChannel.cfg.ch_gntwin = 0;    //channel grant window 
      DMAChannel.cfg.reserved = 0;  //reserved bits are RO
      DMAChannel.cfg.reserved1 = 0; //reserved1 bits are RO
      DMAChannel.cfg.llp_cnt = 0;      //llp_cnt bits are RO
      DMAChannel.cfg.ch_pri = Priority;   //ChPri-channel arbitration priority

      DMA_SetCHCfg(Channel, DMAChannel.cfg);
      /* program channel llp */
      *((unsigned int *)&(DMAChannel.llp)) = 0;    //Init IP llp
      DMAChannel.llp.link_list_addr = LinkAddr;
      DMA_CHLinkList(Channel, DMAChannel.llp);

      /* porgram address and size */
      DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, LLPSize);
      DMA_SetLDMAddr(DMA030_LDM_BASE);
   }
   else
   {   //for LLPCount==1,
      if(DMA_NoLLD(Channel, SrcAddr, DstAddr, Size, SrcWidth, DstWidth,
               SrcSize, SrcCtrl, DstCtrl, Priority, Mode,0)==FALSE)
      return FALSE;
   }
   return TRUE;
}

/* always use channel 0 */
void dma030_channel_set(unsigned int dmaSrcAddr, unsigned int dmaDstAddr)
{
   unsigned int base = DMA030_BASE+DMA_CHANNEL0_BASE;
   unsigned int dmaRegCSR = 0;
   unsigned int dmaRegCFG = 0;

   //            SrcTcnt     TCMask    SrcWidth    DstWidth    SrcCtrl    DstCtrl 
   dmaRegCSR |= (0x5<<29) | (0x1<<28) | (0x2<<25) | (0x2<<22) | (0x2<<20) | (0x0<<18);

   //            SrcHen     SrcRS=4
   dmaRegCFG |= (0x1<<7) | (0x4<<3);

   //printf("[dma030_channel_set] dmaRegCSR=0x%x dmaRegCFG=0x%x dmaSrcAddr=0x%x dmaDstAddr=0x%x\n",dmaRegCSR,dmaRegCFG,dmaSrcAddr,dmaDstAddr);
   DMA_ResetChannel(0);
   ClearDMAChannelIntStatus(0);

   outw(base+DMA_CHANNEL_CSR_OFFSET    ,dmaRegCSR);
   outw(base+DMA_CHANNEL_CFG_OFFSET    ,dmaRegCFG);
   outw(base+DMA_CHANNEL_SRCADDR_OFFSET,dmaSrcAddr);
   outw(base+DMA_CHANNEL_DSTADDR_OFFSET,dmaDstAddr);
}

void dma030_channel_action(unsigned int size)
{
   unsigned int base = DMA030_BASE+DMA_CHANNEL0_BASE;
   unsigned int dmaRegCSR = 0;
   unsigned int dmaInt_status = 0;

   dmaRegCSR = inw(base+DMA_CHANNEL_CSR_OFFSET);
   dmaRegCSR |= (0x1<<16);

   outw(base+DMA_CHANNEL_SIZE_OFFSET   ,size);
   outw(base+DMA_CHANNEL_CSR_OFFSET    ,dmaRegCSR);

#ifndef DMA_INT_POLLING //test IRQ handler   
   while(bWaitIntCB);
#else 
   dmaInt_status = inw(DMA030_BASE);
   dmaInt_status &= 0x1;
   while (dmaInt_status == 0)
   {
      dmaInt_status = inw(DMA030_BASE);
      dmaInt_status &= 0x1;
   }
#endif

   dmaRegCSR |= (0x0<<16);
   outw(base+DMA_CHANNEL_CSR_OFFSET    ,dmaRegCSR);
}

/*************** DMA Test Function ********************************************/

int  DMA_NormalModeTest()
{
   unsigned int SrcAddr;
   unsigned int DstAddr;
   unsigned int Size;
   unsigned int ChannelNum = 4;

   //Test DMA 1D Align Mode
   printf("DMA Test: Test DMA 1D Align Mode!! .....");
   Size=0x200;
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   if(!DMA_MemToMem0_Burnin(ChannelNum, (unsigned int *)SrcAddr, (unsigned int *)DstAddr, Size,0))
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }

   //Test DMA 1D Unalign Mode
   printf("DMA Test: Test DMA 1D Unalign Mode!! .....");
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   Size=0x200;
   if(!DMA_MemToMem0_Unalign_Burnin(ChannelNum, (unsigned int *)SrcAddr, (unsigned int *)DstAddr, Size))
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }

   //Test DMA 2D Align Mode
   printf("DMA Test: Test DMA 2D Align Mode!! .....");
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   if(!DMA_MemToMem2D(0x704, 0x960, 0x780, 0x780 ,0,0x1000,0x1000)) // no link list for fullhan. XTcnt/YTcnt = 704/960; SrcStride/DstStride = 780/780; 
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }

   //Test DMA 2D Unalign Mode
   printf("DMA Test: Test DMA 2D Unalign Mode!! .....");
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   if(!DMA_MemToMem2D_Unalign((unsigned int *)SrcAddr, (unsigned int *)DstAddr, 0x12, 0x3, 0x100,0x100,0x20,0x20 ))
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }
   return;
}

int  DMA_PriorityTest()
{
   unsigned int i,AxiWidth;
   unsigned int Size = 0x3fff0;
   unsigned int *SrcAddr = (unsigned int*)SRCADDR;
   unsigned int *DstAddr = (unsigned int*)DSTADDR;
   unsigned int SrcWidth = 0;
   unsigned int DstWidth = 0;
   unsigned int SrcSize = 0;
   unsigned int FIFO_Dep = 0;
   DMA_FEATURE_t *Feature;

   for(i = 0; i < (Size / 4 * 4); i++)
      SrcAddr[i] = (unsigned int)SrcAddr + i;

   AxiWidth=DMA_GetAXIBusWidth();

   for(SrcWidth = 0; SrcWidth <= AxiWidth; SrcWidth++)
   { 
      for(DstWidth = 0; DstWidth <= AxiWidth; DstWidth++)
      { 
         for(i=0;i<CHANNEL_NUM;i++)
         {
            memset(DstAddr + Size * i, 0, Size);
            DMA_FillChannelReg(i, (unsigned int)(SrcAddr + Size * i), (unsigned int)(DstAddr + Size * i), Size, SrcWidth, DstWidth, SrcSize,
                  AHBDMA_SrcInc, AHBDMA_DstInc, i % 2,
                  DMA_CSR_MODE_NORMAL,0);
            printf("\nDMA:Channel:%d, Size=0x%x, SrcWidth=0x%x, DstWidth=0x%x, Priority=%d\n", 
                  i, (unsigned int)Size, (unsigned int)SrcWidth, (unsigned int)DstWidth, i % 2);
         }

         for(i=0;i<CHANNEL_NUM;i++)
            EnableDMAChannel(i);
         for(i=0;i<CHANNEL_NUM;i++)
            DMA_WaitIntStatus(i);

         for(i=0;i<CHANNEL_NUM;i++)
         {
            if(CompareMemory(SrcAddr + Size * i, DstAddr + Size * i, Size) == FALSE)
            {
               printf("\nSrc(Inc)-->Dst(Inc) Failed\n");
               return FALSE;
            }
         }
         printf("-->Pass\n");
      }
   }
   return;
}

int  DMA_EventTest()
{
   unsigned int u32Mode = 1;
   unsigned char ch1;
   int i;
   unsigned int *SrcAddr1 = (unsigned int*)SRCADDR;
   unsigned int *DstAddr1 = (unsigned int*)DSTADDR;
   unsigned int *DstAddr2 = (unsigned int*)DSTADDR2;
   unsigned int SrcWidth = 0;
   unsigned int DstWidth = 0;
   unsigned int SrcSize = 0;
   unsigned int FIFO_Dep;
   DMA_FEATURE_t *Feature;
   int Size = 0x100;

   memset(DstAddr1, 0, Size);
   memset(DstAddr2, 0, Size);

   for(i = 0; i < (Size/4); i++)
      SrcAddr1[i] = (unsigned int)(SrcAddr1 + i);
   
   //from Src1  to Dst1
   DMA_Start(0, DMA030_LLD_SYSMEM, (unsigned int)SrcAddr1, (unsigned int)DstAddr1, &Size, SrcWidth, DstWidth, SrcSize,
             AHBDMA_SrcInc, AHBDMA_DstInc, AHBDMA_PriorityLow,
             DMA_CSR_MODE_NORMAL,0);
   EnableDMAChannel(0);
   
   //from Dst1 to Dst2
   DMA_Start(1, DMA030_LLD_SYSMEM + 0x100000, (unsigned int)DstAddr1, (unsigned int)DstAddr2, &Size, SrcWidth, DstWidth, SrcSize,
             AHBDMA_SrcInc, AHBDMA_DstInc, AHBDMA_PriorityLow,
             DMA_CSR_MODE_NORMAL,0);   
   EnableDMAChannel(1);
   DMA_WaitIntStatus(0);
   DMA_WaitIntStatus(1);
   
   if(CompareMemory(SrcAddr1, DstAddr2, Size)==FALSE)
   {
      printf("\nSrc(Inc)-->Dst(Inc) Failed");
      return FALSE;
   }
   printf("\nDMA Event Test OK!!");
   return;
}

int DMA_WriteConstantTest()
{
   unsigned int ChannelNum = 4;
   unsigned int SrcAddr=SRCADDR; 
   unsigned int DstAddr=DSTADDR;
   unsigned int Size=32;;

   DMA_WriteConstant_Test(ChannelNum, (unsigned int *)SrcAddr, (unsigned int *)DstAddr, Size);
   
   return;
}

int  DMA_EndianTest()
{
   unsigned int SrcAddr;
   unsigned int DstAddr;
   unsigned int Size;
   unsigned int ChannelNum = 4;
   int Endian_flag = 1;

   //Test DMA 1D Align Mode
   printf("DMA Endian Test: Test DMA 1D Align Mode!! .....");
   Size=0x200;
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   if(!DMA_MemToMem0_Burnin(ChannelNum, (unsigned int *)SrcAddr, (unsigned int *)DstAddr, Size, Endian_flag))
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }
   
   //Test DMA 1D Unalign Mode
   printf("DMA Test: Test DMA 1D Unalign Mode!! .....");
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   Size=30;
   if(!DMA_MemToMem0_Unalign_Burnin(ChannelNum, (unsigned int *)SrcAddr, (unsigned int *)DstAddr, Size))
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }

   //Test DMA 2D Align Mode
   printf("DMA Endian Test: Test DMA 2D Align Mode!! .....");
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   if(!DMA_MemToMem2D(0x704, 0x960, 0x780, 0x780, Endian_flag, 0x1000, 0x1000)) // no link list for fullhan. XTcnt/YTcnt = 704/960; SrcStride/DstStride = 780/780; 
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }

   //Test DMA 2D Unalign Mode
   printf("DMA Test: Test DMA 2D Unalign Mode!! .....");
   SrcAddr=SRCADDR; 
   DstAddr=DSTADDR;
   if(!DMA_MemToMem2D_Unalign((unsigned int *)SrcAddr, (unsigned int *)DstAddr, 0x12, 0x3, 0x100,0x100,0x20,0x20 ))
   {
      printf(" Fail!!\n");
   }
   else
   {
      printf(" OK!!\n");
   }
   return;
}

#if 0	//20220120@BC: FTSPI020 related function undefined.
int DMA_HandshakeTest()
{
   unsigned int i;
   unsigned int SrcWidth, DstWidth, SrcSize,AxiWidth;
   unsigned int read_len = READ_LEN;
   unsigned int access_byte;
   unsigned int offset=0;
   
   AxiWidth = DMA_GetAXIBusWidth();
   SrcWidth = AxiWidth;
   DstWidth = SrcWidth; 
   SrcSize = 0;
   
   //First, using PIO mode to read data
   spi020_flash_read(FLASH_NORMAL, offset, read_len, read_buf);
    
   //Second, using DMA mode to read data
   //printf("[DMA_HandshakeTest] SPI DMA mode: read_len = %d\n",read_len);
   while(read_len>0)
   {
      access_byte = min_t(read_len, spi020_rxfifo_depth());
      //printf("[DMA_HandshakeTest] SPI DMA mode: access_byte = %d\n",access_byte);
      spi020_flash_read(FLASH_DMA_READ, offset, access_byte, 0);
      spi020_wait_rx_full();

      dma030_channel_set(SPI020REG_DATAPORT, ((unsigned int)dma_read_buf)+offset);
      dma030_channel_action(access_byte>>2);// start DMA, size unit in 4 bytes 
      
      spi020_dma_read_stop();
      read_len-=access_byte;
      offset+=access_byte;
   }
   
   //Compare PIO mode buffer and DMA mode buffer
   if (CompareMemory_byte((unsigned int*)(&read_buf), (unsigned int*)(&dma_read_buf),read_len) == TRUE)
   {
      printf("PASS!!\n");
   }
   else
   {
      printf("FAIL!!\n");
   }
   return;
}
#endif

int DMA_BurninTest()
{
   while(1)
   {
      //Normal mode test
      DMA_NormalModeTest();
      
      //Priority Test
      DMA_PriorityTest();
      
      //Event Test
      DMA_EventTest();
      
      //Write Constant Test
      DMA_WriteConstantTest();
      
      // Endian Conversion Test
      DMA_EndianTest();
      
      //Handshake Test with SPI020 Test
      //DMA_HandshakeTest();
   }
   return;
}

/*************** DMA Main Function ********************************************/

int FTDMAC030_main()
{
   int  j,src_temp=0,des_temp=0,Endian_flag=0;
   int  i,k,l,des_stride=0,src_stride=0,Xcount=0,Ycount=0,size_temp=0,ysize_temp=0 ,des_max_xcnt,des_max_ycnt;
   unsigned int option,normal_option;

#if (DMAC_FTDMAC030_COUNT>1)
   printf("Select dma number (0~%d) ? ",DMAC_FTDMAC030_COUNT-1);
   scanf( "%d", &dma_instance_N );
#endif
   printf("\n");
   printf("SRCADDR          = 0x%x \n",SRCADDR);
   printf("DSTADDR          = 0x%x \n",DSTADDR);
   printf("DSTADDR2         = 0x%x \n",DSTADDR2);
   printf("DMA030_LLD_SYSMEM= 0x%x \n",DMA030_LLD_SYSMEM);
   printf("DMA030_LDM_BASE  = 0x%x \n",DMA030_LDM_BASE);
#ifndef DMA_INT_POLLING //test IRQ handler
   printf("DMA030 IRQ Mode  = IRQ Mode\n");
#else
   printf("DMA030 IRQ Mode  = Polling Mode\n");
#endif
   DMA_Register = (DMA_Reg_st *)DMA030_BASE;
   
   fLib_Int_Init();
   
   DMA_ClearAllInterrupt();
#ifndef DMA_INT_POLLING //test IRQ handler
   fLib_CloseIRQ(IRQ_DMA030);

   if (!fLib_ConnectIRQ(IRQ_DMA030, DMA_InterruptHandler))
      return FALSE;

   fLib_EnableIRQ(IRQ_DMA030);
   bWaitIntCB=1;
#else
   fLib_CloseIRQ(IRQ_DMA030); 
#endif

   do
   {
      printf("\n\r");
      printf("(0) AXI DMA Normal Mode Test\n");
      printf("(1) AXI DMA Normal Mode Priority Test\n");
      printf("(2) AXI DMA Normal Mode Event Test\n");
      printf("(3) AXI DMA Normal Mode Write Constant Test\n");
      printf("(4) AXI DMA Normal Mode Endian Conversion Test\n");
#if 0	//20220102@BC: Wait to fix DMA_HandshakeTest().
      printf("(5) AXI DMA Normal Mode Handshake Test with SPI020\n");
#endif
      printf("(6) AXI DMA Normal Mode Burn-in Test (ALL)\n");
      scanf( "%d", &option );
      printf("\n");
      if(option <= 6)
         break;
   }while(1);
   
   switch( option )
   {
      case 0:
         printf("DMA Normal Mode Test\n");
         DMA_NormalModeTest();
         printf("DMA Normal Mode Test Finish!!\n");
         break;
      case 1:
         printf("DMA Priority Test\n");
         DMA_PriorityTest();
         printf("DMA Priority Test Finish!!\n");
         break;
      case 2:
         printf("DMA Event Test\n");
         DMA_EventTest();
         printf("DMA Event Test Finish!!\n");
         break;
      case 3:
         printf("DMA Write Constant Test\n");
         DMA_WriteConstantTest();
         printf("DMA Write Constant Test Finish!!\n");
         break;
      case 4:
         printf("DMA Normal Mode Endian Conversion Test\n");
         DMA_EndianTest();
         printf("DMA Normal Mode Endian Conversion Test Finish!!\n");
         break;
#if 0	//20220102@BC: Wait to fix DMA_HandshakeTest().
      case 5:
         printf("DMA Normal Mode Handshake Test with SPI020 Test\n");
         DMA_HandshakeTest();
         printf("DMA Normal Mode Handshake Test with SPI020 Test Finish!!\n");
         break;
#endif
      case 6:
         printf("DMA Normal Mode Burn-in Test (ALL)\n");
         DMA_BurninTest();
         printf("DMA Normal Mode Burn-in Test (ALL) Finish!!\n");
         break;
      default:
         break;
   }
   return 1;
}

