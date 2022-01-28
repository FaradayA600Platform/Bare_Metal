/**
  ******************************************************************************
  * @file    ftgmac030_hal_eth.h
  * @author  B.C. Chen
  * @version V1.0.0
  * @date    3-Aug-2021
  * @brief   Header file of ETH HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center> COPYRIGHT(c) 2021 Faraday Technology Corporation </center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FTGMAC030_HAL_ETH_H
#define __FTGMAC030_HAL_ETH_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "types.h"

typedef struct
{
	 uint32_t   ISR;            /* 0x00 */
	 uint32_t   IER;            /* 0x04 */
	 uint32_t   MAC_MADR;       /* 0x08 */
	 uint32_t   MAC_LADR;       /* 0x0c */
	 uint32_t   MAHT0;          /* 0x10 */
	 uint32_t   MAHT1;          /* 0x14 */
	 uint32_t   NPTXPD;         /* 0x18 */
	 uint32_t   RXPD;           /* 0x1c */
	 uint32_t   NPTXR_BADR;     /* 0x20 */
	 uint32_t   RXR_BADR;       /* 0x24 */
	 uint32_t   HPTXPD;         /* 0x28 */
	 uint32_t   HPTXR_BADR;     /* 0x2c */
	 uint32_t   TXITC;          /* 0x30 */
	 uint32_t   RXITC;          /* 0x34 */
	 uint32_t   APTC;           /* 0x38 */
	 uint32_t   DBLAC;          /* 0x3c */
	 uint32_t   DMAFIFOS;       /* 0x40 */
	     uint32_t   RESV0;          /* 0x44 */
	 uint32_t   TPAFCR;         /* 0x48 */
	 uint32_t   RBSR;           /* 0x4c */
	 uint32_t   MACCR;          /* 0x50 */
	 uint32_t   MACSR;          /* 0x54 */
	 uint32_t   TM;             /* 0x58 */
	     uint32_t   RESV1;          /* 0x5c */ /* not defined in spec */
	 uint32_t   PHYCR;          /* 0x60 */
	 uint32_t   PHYDATA;        /* 0x64 */
	 uint32_t   FCR;            /* 0x68 */
	 uint32_t   BPR;            /* 0x6c */
	 uint32_t   WOLCR;          /* 0x70 */
	 uint32_t   WOLSR;          /* 0x74 */
	 uint32_t   WFCRC;          /* 0x78 */
	     uint32_t   RESV2;          /* 0x7c */ /* not defined in spec */
	 uint32_t   WFBM1;          /* 0x80 */
	 uint32_t   WFBM2;          /* 0x84 */
	 uint32_t   WFBM3;          /* 0x88 */
	 uint32_t   WFBM4;          /* 0x8c */
	 uint32_t   NPTXR_PTR;      /* 0x90 */
	 uint32_t   HPTXR_PTR;      /* 0x94 */
	 uint32_t   RXR_PTR;        /* 0x98 */
	     uint32_t   RESV3;          /* 0x9c */ /* not defined in spec */
	 uint32_t   TX_CNT;         /* 0xa0 */
	 uint32_t   TX_MCOL_SCOL;   /* 0xa4 */
	 uint32_t   TX_ECOL_FAIL;   /* 0xa8 */
	 uint32_t   TX_LCOL_UND;    /* 0xac */
	 uint32_t   RX_CNT;         /* 0xb0 */
	 uint32_t   RX_BC;          /* 0xb4 */
	 uint32_t   RX_MC;          /* 0xb8 */
	 uint32_t   RX_PF_AEP;      /* 0xbc */
	 uint32_t   RX_RUNT;        /* 0xc0 */
	 uint32_t   RX_CRCER_FTL;   /* 0xc4 */
	 uint32_t   RX_COL_LOST;    /* 0xc8 */
	 uint32_t   BIST;           /* 0xcc */
	     uint32_t   RESV[6];        /* 0xd0~0xe4 */       
	 uint32_t   GISR;           /* 0xe8 */        
} ETH_TypeDef;

/*
 * Interrupt status register & interrupt enable register
 */
#define FTGMAC030_INT_RPKT_BUF                  (1 << 0)
#define FTGMAC030_INT_RPKT_FIFO                 (1 << 1)
#define FTGMAC030_INT_NO_RXBUF                  (1 << 2)
#define FTGMAC030_INT_RPKT_LOST                 (1 << 3)
#define FTGMAC030_INT_XPKT_ETH                  (1 << 4)
#define FTGMAC030_INT_XPKT_FIFO                 (1 << 5)
#define FTGMAC030_INT_NO_NPTXBUF                (1 << 6)
#define FTGMAC030_INT_XPKT_LOST                 (1 << 7)
#define FTGMAC030_INT_AHB_ERR                   (1 << 8)
#define FTGMAC030_INT_PHYSTS_CHG                (1 << 9)
#define FTGMAC030_INT_NO_HPTXBUF                (1 << 10)
#define FTGMAC030_INT_SYNC_IN                   (1 << 17)
#define FTGMAC030_INT_SYNC_OUT                  (1 << 18)
#define FTGMAC030_INT_DELAY_REQ_IN              (1 << 19)
#define FTGMAC030_INT_DELAY_REQ_OUT             (1 << 20)
#define FTGMAC030_INT_PDELAY_REQ_IN             (1 << 21)
#define FTGMAC030_INT_PDELAY_REQ_OUT            (1 << 22)
#define FTGMAC030_INT_PDELAY_RESP_IN            (1 << 23)
#define FTGMAC030_INT_PDELAY_RESP_OUT           (1 << 24)
#define FTGMAC030_INT_TX_TMSP_VALID             (FTGMAC030_INT_SYNC_OUT | FTGMAC030_INT_DELAY_REQ_OUT | \
                                                 FTGMAC030_INT_PDELAY_REQ_OUT | FTGMAC030_INT_PDELAY_RESP_OUT)
#define FTGMAC030_INT_RX_TMSP_VALID             (FTGMAC030_INT_SYNC_IN | FTGMAC030_INT_DELAY_REQ_IN | \
                                                 FTGMAC030_INT_PDELAY_REQ_IN | FTGMAC030_INT_PDELAY_RESP_IN)

/*
 * TX Interrupt timer control register (30h)
 */
#define FTGMAC030_ITC_TXINT_THR_UNIT(x)         (((x) & 0x3) << 0)
#define FTGMAC030_ITC_TXINT_THR(x)              (((x) & 0x7) << 4)
#define FTGMAC030_ITC_TXINT_CYC(x)              (((x) & 0xff) << 8)
#define FTGMAC030_ITC_TXINT_TIME_SEL            (1 << 16)

/*
 * RX Interrupt timer control register (34h)
 */
#define FTGMAC030_ITC_RXINT_THR_UNIT(x)         (((x) & 0x3) << 0)
#define FTGMAC030_ITC_RXINT_THR(x)              (((x) & 0x7) << 4)
#define FTGMAC030_ITC_RXINT_CYC(x)              (((x) & 0xff) << 8)
#define FTGMAC030_ITC_RXINT_TIME_SEL            (1 << 16)
#define FTGMAC030_ITC_RXINT_RST(x)              (((x) & 0xff) << 20)

/*
 * Automatic polling timer control register (38h)
 */
#define FTGMAC030_APTC_RXPOLL_CNT(x)            (((x) & 0xf) << 0)
#define FTGMAC030_APTC_RXPOLL_TIME_SEL          (1 << 4)
#define FTGMAC030_APTC_TXPOLL_CNT(x)            (((x) & 0xf) << 8)
#define FTGMAC030_APTC_TXPOLL_TIME_SEL          (1 << 12)

/*
 * DMA burst length and arbitration control register (3Ch)
 */
#define FTGMAC030_DBLAC_RXFIFO_LTHR(x)          (((x) & 0x7) << 0)
#define FTGMAC030_DBLAC_RXFIFO_HTHR(x)          (((x) & 0x7) << 3)
#define FTGMAC030_DBLAC_RX_THR_EN               (1 << 6)
#define FTGMAC030_DBLAC_RXBURST_SIZE(x)         (((x) & 0x3) << 8)
#define FTGMAC030_DBLAC_TXBURST_SIZE(x)         (((x) & 0x3) << 10)
#define FTGMAC030_DBLAC_RXDES_SIZE(x)           (((x) & 0xf) << 12)
#define FTGMAC030_DBLAC_TXDES_SIZE(x)           (((x) & 0xf) << 16)
#define FTGMAC030_DBLAC_IFG_CNT(x)              (((x) & 0x7) << 20)
#define FTGMAC030_DBLAC_IFG_INC                 (1 << 23)

/*
 * DMA FIFO status register (40h)
 */
#define FTGMAC030_DMAFIFOS_RXDMA1_SM(dmafifos)  ((dmafifos) & 0xf)
#define FTGMAC030_DMAFIFOS_RXDMA2_SM(dmafifos)  (((dmafifos) >> 4) & 0xf)
#define FTGMAC030_DMAFIFOS_RXDMA3_SM(dmafifos)  (((dmafifos) >> 8) & 0x7)
#define FTGMAC030_DMAFIFOS_TXDMA1_SM(dmafifos)  (((dmafifos) >> 12) & 0xf)
#define FTGMAC030_DMAFIFOS_TXDMA2_SM(dmafifos)  (((dmafifos) >> 16) & 0x3)
#define FTGMAC030_DMAFIFOS_TXDMA3_SM(dmafifos)  (((dmafifos) >> 18) & 0xf)
#define FTGMAC030_DMAFIFOS_RXFIFO_EMPTY         (1 << 26)
#define FTGMAC030_DMAFIFOS_TXFIFO_EMPTY         (1 << 27)
#define FTGMAC030_DMAFIFOS_RXDMA_GRANT          (1 << 28)
#define FTGMAC030_DMAFIFOS_TXDMA_GRANT          (1 << 29)
#define FTGMAC030_DMAFIFOS_RXDMA_REQ            (1 << 30)
#define FTGMAC030_DMAFIFOS_TXDMA_REQ            (1 << 31)

/*
 * Transmit Priority Arbitration and FIFO Control Register (48h)
 */
#define FTGMAC030_TPAFCR_TFIFO_SIZE(x)          ((x & 0x7) << 27)
#define FTGMAC030_TPAFCR_RFIFO_SIZE(x)          ((x & 0x7) << 24)
#define FTGMAC030_TPAFCR_TFIFO_VAL(x)           ((x >> 27) & 0x7)
#define FTGMAC030_TPAFCR_RFIFO_VAL(x)           ((x >> 24) & 0x7)

/*
 * Receive buffer size register (4ch)
 */
#define FTGMAC030_RBSR_SIZE(x)                  ((x) & 0x3fff)

/*
 * MAC control register (50h)
 */
#define FTGMAC030_MACCR_TXDMA_EN                (1 << 0)
#define FTGMAC030_MACCR_RXDMA_EN                (1 << 1)
#define FTGMAC030_MACCR_TXMAC_EN                (1 << 2)
#define FTGMAC030_MACCR_RXMAC_EN                (1 << 3)
#define FTGMAC030_MACCR_DIS_IPV6_PKTREC         (1 << 7)
#define FTGMAC030_MACCR_RX_ALL                  (1 << 8)
#define FTGMAC030_MACCR_HT_MULTI_EN             (1 << 9)
#define FTGMAC030_MACCR_RX_MULTIPKT             (1 << 10)
#define FTGMAC030_MACCR_RX_BROADPKT             (1 << 11)
#define FTGMAC030_MACCR_RX_RUNT                 (1 << 12)
#define FTGMAC030_MACCR_JUMBO_LF                (1 << 13)
#define FTGMAC030_MACCR_ENRX_IN_HALFTX          (1 << 14)
#define FTGMAC030_MACCR_DISCARD_CRCERR          (1 << 16)
#define FTGMAC030_MACCR_CRC_APD                 (1 << 17)
#define FTGMAC030_MACCR_REMOVE_VLAN             (1 << 18)
#define FTGMAC030_MACCR_PTP_EN                  (1 << 20)
#define FTGMAC030_MACCR_LOOP_EN                 (1 << 21)
#define FTGMAC030_MACCR_HPTXR_EN                (1 << 22)
#define FTGMAC030_MACCR_GIGA_MODE               (2 << 24)
#define FTGMAC030_MACCR_FAST_MODE               (1 << 24)
#define FTGMAC030_MACCR_SPEED_VAL(x)            ((x >> 24) & 0x3)
#define FTGMAC030_MACCR_FULLDUP                 (1 << 26)
#define FTGMAC030_MACCR_SW_RST                  (1 << 31)

/*
 * PHY control register (60h)
 */
#define FTGMAC030_PHYCR_MDC_CYCTHR_MASK         0x3f
#define FTGMAC030_PHYCR_MDC_CYCTHR(x)           ((x) & 0x3f)
#define FTGMAC030_PHYCR_OP(x)                   (((x) & 0x3) << 14)
#define FTGMAC030_PHYCR_ST(x)                   (((x) & 0x3) << 12)
#define FTGMAC030_PHYCR_PHYAD(x)                (((x) & 0x1f) << 16)
#define FTGMAC030_PHYCR_REGAD(x)                (((x) & 0x1f) << 21)
#define FTGMAC030_PHYCR_MIIRD                   (1 << 26)
#define FTGMAC030_PHYCR_MIIWR                   (1 << 27)

/*
 * PHY data register (64h)
 */
#define FTGMAC030_PHYDATA_MIIWDATA(x)           ((x) & 0xffff)
#define FTGMAC030_PHYDATA_MIIRDATA(phydata)     (((phydata) >> 16) & 0xffff)

/*
 * Flow Control Register, FCR (68h)
 */
#define FTGMAC030_FCR_PAUSE_TIME(x)             ((x & 0xffff) << 16)
#define FTGMAC030_FCR_FC_H_L(x)                 ((x & 0x7f) << 9)
#define FTGMAC030_FCR_HTHR                      (1 << 8)
#define FTGMAC030_FCR_RX_PAUSE                  (1 << 4)
#define FTGMAC030_FCR_TXPAUSED                  (1 << 3)
#define FTGMAC030_FCR_FCTHR_EN                  (1 << 2)
#define FTGMAC030_FCR_TX_PAUSE                  (1 << 1)
#define FTGMAC030_FCR_FC_EN                     (1 << 0)

/*
 * EEE register (F0h)
 */
#define FTGMAC030_EEE_TX_ENTER_MASK             0xFFF
#define FTGMAC030_EEE_TX_ENTER_SHIFT            20
#define FTGMAC030_EEE_TX_ENTER                  (FTGMAC030_EEE_TX_ENTER_MASK << FTGMAC030_EEE_TX_ENTER_SHIFT)
#define FTGMAC030_EEE_TX_LPI                    (1 << 16)
#define FTGMAC030_EEE_TX_WAKEUP                 (0xFFFF)

/*
 * Feautre register (0xF8h)
 */
#define FTGMAC030_FEAR_PTP                      (1 << 18)

/*
 * Transmit descriptor, aligned to 16 bytes
 */
typedef struct ftgmac030_txdes {
	uint32_t    txdes0;
	uint32_t    txdes1;
	uint32_t    txdes2;	/* not used by HW */
	uint32_t    txdes3;	/* TXBUF_BADR */
} ETH_DMATxDescTypeDef;

#define FTGMAC030_TXDES0_TXBUF_SIZE(x)          ((x) & 0x3fff)
#define FTGMAC030_TXDES0_EDOTR                  (1 << 15)
#define FTGMAC030_TXDES0_CRC_ERR                (1 << 19)
#define FTGMAC030_TXDES0_LTS                    (1 << 28)
#define FTGMAC030_TXDES0_FTS                    (1 << 29)
#define FTGMAC030_TXDES0_TXDMA_OWN              (1 << 31)

#define FTGMAC030_TXDES1_VLANTAG_CI(x)          ((x) & 0xffff)
#define FTGMAC030_TXDES1_INS_VLANTAG            (1 << 16)
#define FTGMAC030_TXDES1_TCP_CHKSUM             (1 << 17)
#define FTGMAC030_TXDES1_UDP_CHKSUM             (1 << 18)
#define FTGMAC030_TXDES1_IP_CHKSUM              (1 << 19)
#define FTGMAC030_TXDES1_IPV6_PKT               (1 << 21)
#define FTGMAC030_TXDES1_LLC                    (1 << 22)
#define FTGMAC030_TXDES1_TX2FIC                 (1 << 30)
#define FTGMAC030_TXDES1_TXIC                   (1 << 31)

/*
 * Receive descriptor, aligned to 16 bytes
 */
typedef struct ftgmac030_rxdes {
	uint32_t    rxdes0;
	uint32_t    rxdes1;
	uint32_t    rxdes2;	/* not used by HW */
	uint32_t    rxdes3;	/* RXBUF_BADR */
} ETH_DMARxDescTypeDef;

#define FTGMAC030_RXDES0_VDBC(x)                ((x) & 0x3fff)
#define FTGMAC030_RXDES0_EDORR                  (1 << 15)
#define FTGMAC030_RXDES0_MULTICAST              (1 << 16)
#define FTGMAC030_RXDES0_BROADCAST              (1 << 17)
#define FTGMAC030_RXDES0_RX_ERR                 (1 << 18)
#define FTGMAC030_RXDES0_CRC_ERR                (1 << 19)
#define FTGMAC030_RXDES0_FTL                    (1 << 20)
#define FTGMAC030_RXDES0_RUNT                   (1 << 21)
#define FTGMAC030_RXDES0_RX_ODD_NB              (1 << 22)
#define FTGMAC030_RXDES0_FIFO_FULL              (1 << 23)
#define FTGMAC030_RXDES0_PAUSE_OPCODE           (1 << 24)
#define FTGMAC030_RXDES0_PAUSE_FRAME            (1 << 25)
#define FTGMAC030_RXDES0_LRS                    (1 << 28)
#define FTGMAC030_RXDES0_FRS                    (1 << 29)
#define FTGMAC030_RXDES0_RXPKT_RDY              (1 << 31)

#define FTGMAC030_RXDES0_EOP                    (FTGMAC030_RXDES0_FRS | FTGMAC030_RXDES0_LRS)

#define FTGMAC030_RXDES1_IPV6                   (1 << 19)
#define FTGMAC030_RXDES1_VLANTAG_CI             0xffff
#define FTGMAC030_RXDES1_PROT_MASK              (0x3 << 20)
#define FTGMAC030_RXDES1_PROT_NONIP             (0x0 << 20)
#define FTGMAC030_RXDES1_PROT_IP                (0x1 << 20)
#define FTGMAC030_RXDES1_PROT_TCPIP             (0x2 << 20)
#define FTGMAC030_RXDES1_PROT_UDPIP             (0x3 << 20)
#define FTGMAC030_RXDES1_LLC                    (1 << 22)
#define FTGMAC030_RXDES1_DF                     (1 << 23)
#define FTGMAC030_RXDES1_VLANTAG_AVAIL          (1 << 24)
#define FTGMAC030_RXDES1_TCP_CHKSUM_ERR         (1 << 25)
#define FTGMAC030_RXDES1_UDP_CHKSUM_ERR         (1 << 26)
#define FTGMAC030_RXDES1_IP_CHKSUM_ERR          (1 << 27)
#define FTGMAC030_RXDES1_PTP_MASK               (0x3 << 28)
#define FTGMAC030_RXDES1_PTP_NO_TMSTMP          (0x1 << 28)
#define FTGMAC030_RXDES1_PTP_EVENT_TMSTMP       (0x2 << 28)
#define FTGMAC030_RXDES1_PTP_PEER_TMSTMP        (0x3 << 28)

#define PHY_INTERFACE_MII                       0x0
#define PHY_INTERFACE_GMII                      0x0
#define PHY_INTERFACE_RMII                      0x1
#define PHY_INTERFACE_RGMII                     0x2

#define PHY_READ_TO                             0x0000FFFFU
#define PHY_WRITE_TO                            0x0000FFFFU

/* This defines the bits that are set in the Interrupt Enabled
 * Register.
 */
#define INT_MASK_ALL_ENABLED                    (FTGMAC030_INT_RPKT_LOST    | \
                                                 FTGMAC030_INT_AHB_ERR      | \
                                                 FTGMAC030_INT_RPKT_BUF     | \
                                                 FTGMAC030_INT_XPKT_ETH     | \
                                                 FTGMAC030_INT_NO_RXBUF)

/** 
  * @brief  HAL Status structures definition  
  */  
typedef enum 
{
	HAL_OK       = 0x00U,
	HAL_ERROR    = 0x01U,
	HAL_BUSY     = 0x02U,
	HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

/** 
  * @brief  HAL Lock structures definition  
  */
typedef enum 
{
	HAL_UNLOCKED = 0x00,
	HAL_LOCKED   = 0x01  
} HAL_LockTypeDef;

#define __HAL_LOCK(__HANDLE__)                  \
	do{                                         \
		if((__HANDLE__)->Lock == HAL_LOCKED)    \
		{                                       \
			return HAL_BUSY;                    \
		}                                       \
		else                                    \
		{                                       \
			(__HANDLE__)->Lock = HAL_LOCKED;    \
		}                                       \
	}while (0U)

#define __HAL_UNLOCK(__HANDLE__)                \
	do{                                         \
		(__HANDLE__)->Lock = HAL_UNLOCKED;      \
	}while (0U)

/** @addtogroup FTGMAC030_HAL_Driver
  * @{
  */

/** @addtogroup ETH
  * @{
  */ 
  
/** @addtogroup ETH_Private_Macros
  * @{
  */
#define IS_ETH_PHY_ADDRESS(ADDRESS)     ((ADDRESS) <= 0x20)
#define IS_ETH_AUTONEGOTIATION(CMD)     (((CMD) == ETH_AUTONEGOTIATION_ENABLE) || \
                                         ((CMD) == ETH_AUTONEGOTIATION_DISABLE))
#define IS_ETH_SPEED(SPEED)             (((SPEED) == ETH_SPEED_10M) || \
                                         ((SPEED) == ETH_SPEED_100M) || \
                                         ((SPEED) == ETH_SPEED_1000M))
#define IS_ETH_DUPLEX_MODE(MODE)        (((MODE) == ETH_MODE_FULLDUPLEX) || \
                                         ((MODE) == ETH_MODE_HALFDUPLEX))
#define IS_ETH_RX_MODE(MODE)            (((MODE) == ETH_RXPOLLING_MODE) || \
                                         ((MODE) == ETH_RXINTERRUPT_MODE))
#define IS_ETH_CHECKSUM_MODE(MODE)      (((MODE) == ETH_CHECKSUM_BY_HARDWARE) || \
                                         ((MODE) == ETH_CHECKSUM_BY_SOFTWARE))
#define IS_ETH_MEDIA_INTERFACE(MODE)    (((MODE) == ETH_MEDIA_INTERFACE_MII) || \
                                         ((MODE) == ETH_MEDIA_INTERFACE_RMII) || \
                                         ((MODE) == ETH_MEDIA_INTERFACE_RGMII))
#define IS_ETH_MAC_ADDRESS0123(ADDRESS) (((ADDRESS) == ETH_MAC_ADDRESS0) || \
                                         ((ADDRESS) == ETH_MAC_ADDRESS1) || \
                                         ((ADDRESS) == ETH_MAC_ADDRESS2) || \
                                         ((ADDRESS) == ETH_MAC_ADDRESS3))
#define IS_ETH_MAC_ADDRESS123(ADDRESS)  (((ADDRESS) == ETH_MAC_ADDRESS1) || \
                                         ((ADDRESS) == ETH_MAC_ADDRESS2) || \
                                         ((ADDRESS) == ETH_MAC_ADDRESS3))
/**
  * @}
  */

/** @addtogroup ETH_Private_Defines
  * @{
  */
/* Delay to wait when writing to some Ethernet registers */
#define ETH_REG_WRITE_DELAY             ((uint32_t)0x00000001U)

/* Ethernet Errors */
#define  ETH_SUCCESS                    ((uint32_t)0U)
#define  ETH_ERROR                      ((uint32_t)1U)
/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/ 
/** @defgroup ETH_Exported_Types ETH Exported Types
  * @{
  */

/** 
  * @brief  HAL State structures definition  
  */ 
typedef enum
{
	HAL_ETH_STATE_RESET             = 0x00U,    /*!< Peripheral not yet Initialized or disabled         */
	HAL_ETH_STATE_READY             = 0x01U,    /*!< Peripheral Initialized and ready for use           */
	HAL_ETH_STATE_BUSY              = 0x02U,    /*!< an internal process is ongoing                     */
	HAL_ETH_STATE_BUSY_TX           = 0x12U,    /*!< Data Transmission process is ongoing               */
	HAL_ETH_STATE_BUSY_RX           = 0x22U,    /*!< Data Reception process is ongoing                  */
	HAL_ETH_STATE_BUSY_TX_RX        = 0x32U,    /*!< Data Transmission and Reception process is ongoing */
	HAL_ETH_STATE_BUSY_WR           = 0x42U,    /*!< Write process is ongoing                           */
	HAL_ETH_STATE_BUSY_RD           = 0x82U,    /*!< Read process is ongoing                            */
	HAL_ETH_STATE_TIMEOUT           = 0x03U,    /*!< Timeout state                                      */
	HAL_ETH_STATE_ERROR             = 0x04U     /*!< Reception process is ongoing                       */
}HAL_ETH_StateTypeDef;

/** 
  * @brief  ETH Init Structure definition  
  */
typedef struct
{
	uint32_t                AutoNegotiation;    /*!< Selects or not the AutoNegotiation mode for the external PHY
	                                                 The AutoNegotiation allows an automatic setting of the Speed (10/100Mbps)
	                                                 and the mode (half/full-duplex).
	                                                 This parameter can be a value of @ref ETH_AutoNegotiation */
                                                
	uint32_t                Speed;              /*!< Sets the Ethernet speed: 10/100 Mbps.
	                                                 This parameter can be a value of @ref ETH_Speed */
                                                
	uint32_t                DuplexMode;         /*!< Selects the MAC duplex mode: Half-Duplex or Full-Duplex mode
	                                                 This parameter can be a value of @ref ETH_Duplex_Mode */
                                                
	uint16_t                PhyAddress;         /*!< Ethernet PHY address.
	                                                 This parameter must be a number between Min_Data = 0 and Max_Data = 32 */
                                                
	uint8_t                 *MACAddr;           /*!< MAC Address of used Hardware: must be pointer on an array of 6 bytes */
                                                
	uint32_t                RxMode;             /*!< Selects the Ethernet Rx mode: Polling mode, Interrupt mode.
	                                                 This parameter can be a value of @ref ETH_Rx_Mode */
                                                
	uint32_t                ChecksumMode;       /*!< Selects if the checksum is check by hardware or by software. 
	                                                 This parameter can be a value of @ref ETH_Checksum_Mode */
                                                
	uint32_t                MediaInterface;     /*!< Selects the media-independent interface or the reduced media-independent interface. 
	                                                 This parameter can be a value of @ref ETH_Media_Interface */
                                                
	uint32_t                MDCCycThr;

} ETH_InitTypeDef;


/** 
  * @brief  Received Frame Informations structure definition
  */ 
typedef struct  
{
	ETH_DMARxDescTypeDef     *FSRxDesc;         /*!< First Segment Rx Desc */

	ETH_DMARxDescTypeDef     *LSRxDesc;         /*!< Last Segment Rx Desc */

	uint32_t                 SegCount;          /*!< Segment count */

	uint32_t                 length;            /*!< Frame length */

	uint32_t                 buffer;            /*!< Frame buffer */

} ETH_DMARxFrameInfos;


/** 
  * @brief  ETH Handle Structure definition  
  */
typedef struct
{
  ETH_TypeDef                *Instance;         /*!< Register base address       */
  
  ETH_InitTypeDef            Init;              /*!< Ethernet Init Configuration */
  
  uint32_t                   LinkStatus;        /*!< Ethernet link status        */
  
  ETH_DMARxDescTypeDef       *RxDesc;           /*!< Rx descriptor to Get        */
  
  ETH_DMATxDescTypeDef       *TxDesc;           /*!< Tx descriptor to Set        */
  
  ETH_DMARxFrameInfos        RxFrameInfos;      /*!< last Rx frame infos         */
  
  HAL_ETH_StateTypeDef       State;             /*!< ETH communication state     */
  
  HAL_LockTypeDef            Lock;              /*!< ETH Lock                    */

} ETH_HandleTypeDef;

 /**
  * @}
  */

/** @defgroup ETH_Buffers_setting ETH Buffers setting
  * @{
  */ 
#define ETH_MAX_PACKET_SIZE         ((uint32_t)1524U)   /*!< ETH_HEADER + ETH_EXTRA + ETH_VLAN_TAG + ETH_MAX_ETH_PAYLOAD + ETH_CRC */
#define ETH_HEADER                  ((uint32_t)14U)     /*!< 6 byte Dest addr, 6 byte Src addr, 2 byte length/type */
#define ETH_CRC                     ((uint32_t)4U)      /*!< Ethernet CRC */
#define ETH_EXTRA                   ((uint32_t)2U)      /*!< Extra bytes in some cases */   
#define ETH_VLAN_TAG                ((uint32_t)4U)      /*!< optional 802.1q VLAN Tag */
#define ETH_MIN_ETH_PAYLOAD         ((uint32_t)46U)     /*!< Minimum Ethernet payload size */
#define ETH_MAX_ETH_PAYLOAD         ((uint32_t)1500U)   /*!< Maximum Ethernet payload size */
#define ETH_JUMBO_FRAME_PAYLOAD     ((uint32_t)9000U)   /*!< Jumbo frame payload size */      

 /* Ethernet driver receive buffers are organized in a chained linked-list, when
    an Ethernet packet is received, the Rx-DMA will transfer the packet from RxFIFO
    to the driver receive buffers memory.

    Depending on the size of the received Ethernet packet and the size of 
    each Ethernet driver receive buffer, the received packet can take one or more
    Ethernet driver receive buffer. 

    In below are defined the size of one Ethernet driver receive buffer ETH_RX_BUF_SIZE 
    and the total count of the driver receive buffers ETH_RXBUFNB.

    The configured value for ETH_RX_BUF_SIZE and ETH_RXBUFNB are only provided as 
    example, they can be reconfigured in the application layer to fit the application 
    needs */ 

/* Here we configure each Ethernet driver receive buffer to fit the Max size Ethernet
   packet */
#ifndef ETH_RX_BUF_SIZE
 #define ETH_RX_BUF_SIZE            ETH_MAX_PACKET_SIZE 
#endif

/* 5 Ethernet driver receive buffers are used (in a chained linked list)*/ 
#ifndef ETH_RXBUFNB
 #define ETH_RXBUFNB                ((uint32_t)10U)     /*  10 Rx buffers of size ETH_RX_BUF_SIZE */
#endif


/* Ethernet driver transmit buffers are organized in a chained linked-list, when
   an Ethernet packet is transmitted, Tx-DMA will transfer the packet from the 
   driver transmit buffers memory to the TxFIFO.

   Depending on the size of the Ethernet packet to be transmitted and the size of 
   each Ethernet driver transmit buffer, the packet to be transmitted can take 
   one or more Ethernet driver transmit buffer. 

   In below are defined the size of one Ethernet driver transmit buffer ETH_TX_BUF_SIZE 
   and the total count of the driver transmit buffers ETH_TXBUFNB.

   The configured value for ETH_TX_BUF_SIZE and ETH_TXBUFNB are only provided as 
   example, they can be reconfigured in the application layer to fit the application 
   needs */ 

/* Here we configure each Ethernet driver transmit buffer to fit the Max size Ethernet
   packet */
#ifndef ETH_TX_BUF_SIZE 
 #define ETH_TX_BUF_SIZE            ETH_MAX_PACKET_SIZE
#endif

/* 5 Ethernet driver transmit buffers are used (in a chained linked list)*/ 
#ifndef ETH_TXBUFNB
 #define ETH_TXBUFNB                ((uint32_t)5U)      /* 5  Tx buffers of size ETH_TX_BUF_SIZE */
#endif

/**
 * @}
 */

 /** @defgroup ETH_AutoNegotiation ETH AutoNegotiation 
  * @{
  */
#define ETH_AUTONEGOTIATION_ENABLE  ((uint32_t)0x00000001U)
#define ETH_AUTONEGOTIATION_DISABLE ((uint32_t)0x00000000U)
/**
  * @}
  */

/** @defgroup ETH_Speed ETH Speed 
  * @{
  */
#define ETH_SPEED_10M               ((uint32_t)0x00000000U)
#define ETH_SPEED_100M              ((uint32_t)0x01000000U)
#define ETH_SPEED_1000M             ((uint32_t)0x02000000U)
/**
  * @}
  */

/** @defgroup ETH_Duplex_Mode ETH Duplex Mode
  * @{
  */
#define ETH_MODE_FULLDUPLEX         ((uint32_t)0x04000000U)
#define ETH_MODE_HALFDUPLEX         ((uint32_t)0x00000000U)
/**
  * @}
  */

/** @defgroup ETH_Rx_Mode ETH Rx Mode
  * @{
  */
#define ETH_RXPOLLING_MODE          ((uint32_t)0x00000000U)
#define ETH_RXINTERRUPT_MODE        ((uint32_t)0x00000001U)
/**
  * @}
  */

/** @defgroup ETH_Checksum_Mode ETH Checksum Mode
  * @{
  */ 
#define ETH_CHECKSUM_BY_HARDWARE    ((uint32_t)0x00000000U)
#define ETH_CHECKSUM_BY_SOFTWARE    ((uint32_t)0x00000001U)
/**
  * @}
  */

/** @defgroup ETH_Media_Interface ETH Media Interface
  * @{
  */ 
#define ETH_MEDIA_INTERFACE_MII     ((uint32_t)0x00000000U)
#define ETH_MEDIA_INTERFACE_RMII    ((uint32_t)0x00000001U)
#define ETH_MEDIA_INTERFACE_RGMII   ((uint32_t)0x00000002U)
/**
  * @}
  */

/** @defgroup ETH_MAC_addresses ETH MAC addresses
  * @{
  */ 
#define ETH_MAC_ADDRESS0            ((uint32_t)0x00000000U)
#define ETH_MAC_ADDRESS1            ((uint32_t)0x00000008U)
#define ETH_MAC_ADDRESS2            ((uint32_t)0x00000010U)
#define ETH_MAC_ADDRESS3            ((uint32_t)0x00000018U)

/** 
  * @brief  Enables the specified Ethernet DMA interrupts.
  * @param  __HANDLE__   : ETH Handle
  * @param  __INTERRUPT__: specifies the Ethernet DMA interrupt sources to be
  *   enabled @ref ETH_DMA_Interrupts
  * @retval None
  */
#define __HAL_ETH_DMA_ENABLE_IT(__HANDLE__, __INTERRUPT__)      ((__HANDLE__)->Instance->IER |= (__INTERRUPT__))

/**
  * @brief  Disables the specified Ethernet DMA interrupts.
  * @param  __HANDLE__   : ETH Handle
  * @param  __INTERRUPT__: specifies the Ethernet DMA interrupt sources to be
  *   disabled. @ref ETH_DMA_Interrupts
  * @retval None
  */
#define __HAL_ETH_DMA_DISABLE_IT(__HANDLE__, __INTERRUPT__)     ((__HANDLE__)->Instance->IER &= ~(__INTERRUPT__))
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @addtogroup ETH_Exported_Functions
  * @{
  */

/* Initialization and de-initialization functions  ****************************/

/** @addtogroup ETH_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef HAL_ETH_Init(ETH_HandleTypeDef *heth);
HAL_StatusTypeDef HAL_ETH_DeInit(ETH_HandleTypeDef *heth);
HAL_StatusTypeDef HAL_ETH_DMATxDescListInit(ETH_HandleTypeDef *heth, ETH_DMATxDescTypeDef *DMATxDescTab, uint8_t* TxBuff, uint32_t TxBuffCount);
HAL_StatusTypeDef HAL_ETH_DMARxDescListInit(ETH_HandleTypeDef *heth, ETH_DMARxDescTypeDef *DMARxDescTab, uint8_t *RxBuff, uint32_t RxBuffCount);
/**
  * @}
  */

/* IO operation functions  ****************************************************/

/** @addtogroup ETH_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef HAL_ETH_TransmitFrame(ETH_HandleTypeDef *heth, uint32_t FrameLength);
HAL_StatusTypeDef HAL_ETH_GetReceivedFrame(ETH_HandleTypeDef *heth);
/* Communication with PHY functions*/
HAL_StatusTypeDef HAL_ETH_ReadPHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t *RegValue);
HAL_StatusTypeDef HAL_ETH_WritePHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t RegValue);
/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_ETH_GetReceivedFrame_IT(ETH_HandleTypeDef *heth);
void HAL_ETH_IRQHandler(ETH_HandleTypeDef *heth);
/* Callback in non blocking modes (Interrupt) */
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth);
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth);
void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth);
/**
  * @}
  */

/* Peripheral Control functions  **********************************************/

/** @addtogroup ETH_Exported_Functions_Group3
  * @{
  */
HAL_StatusTypeDef HAL_ETH_Start(ETH_HandleTypeDef *heth);
HAL_StatusTypeDef HAL_ETH_Stop(ETH_HandleTypeDef *heth);
HAL_StatusTypeDef HAL_ETH_ConfigMAC(ETH_HandleTypeDef *heth);
/**
  * @}
  */ 

/* Peripheral State functions  ************************************************/

/** @addtogroup ETH_Exported_Functions_Group4
  * @{
  */
HAL_ETH_StateTypeDef HAL_ETH_GetState(ETH_HandleTypeDef *heth);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif	/* __FTGMAC030_HAL_ETH_H */

/************ (C) COPYRIGHT Faraday Technology Corporation *****END OF FILE****/
