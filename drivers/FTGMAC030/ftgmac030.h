/**
 * (C) Copyright 2014 Faraday Technology
 * BingYao Luo <bjluo@faraday-tech.com>
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
#ifndef __FTGMAC030_H__
#define __FTGMAC030_H__

#include <portme_ftgmac030.h>

/*
 * ftgmac030 register offset definition 
 */
struct FTGMAC030_REGS {
	volatile unsigned int isr;
	volatile unsigned int ier;
	volatile unsigned int mac_madr;
	volatile unsigned int mac_ladr; /* 0xC */
	volatile unsigned int maht0;
	volatile unsigned int maht1;
	volatile unsigned int nptxpd;
	volatile unsigned int rxpd; /*0x1C */
	volatile unsigned int nptxdesc_addr;
	volatile unsigned int rxdesc_addr;
	volatile unsigned int hptxpd;
	volatile unsigned int hptxdesc_addr; /*0x2C */
	volatile unsigned int txitc;
	volatile unsigned int rxitc;
	volatile unsigned int aptc;
	volatile unsigned int dblac; /* 0x3c */
	volatile unsigned int dmafifos; /* 0x40 */
	volatile unsigned int rsvd1; /* 0x44 */
	volatile unsigned int tfafcr;
	volatile unsigned int rbsr; /* 0x4c */
	volatile unsigned int maccr;
	volatile unsigned int macsr;
	volatile unsigned int tm;
	volatile unsigned int rsvd2; /* 0x5C */
	volatile unsigned int phycr;
	volatile unsigned int phydata;
	volatile unsigned int flow_cntrl;
	volatile unsigned int back_pressure; /* 0x6C */
	volatile unsigned int wakeOnLan_cntrl;
	volatile unsigned int wakeOnLan_sts;
	volatile unsigned int wfc_crc;
	volatile unsigned int rsvd3; /* 0x7c */
	volatile unsigned int wfbm1;
	volatile unsigned int wfbm2;
	volatile unsigned int wfbm3;
	volatile unsigned int wfbm4; /* 0x8c */
	volatile unsigned int nptxr_ptr;
	volatile unsigned int hptxr_prt;
	volatile unsigned int rxr_ptr;
	volatile unsigned int rsvd4;
	volatile unsigned int tpkt_cnt; /* 0xA0 */
	volatile unsigned int tx_m_s_col;
	volatile unsigned int tx_e_f_cnt;
	volatile unsigned int tx_l_u_cnt;
	volatile unsigned int rpkt_cnt; /* 0xB0 */
	volatile unsigned int bropkt_cnt;
	volatile unsigned int mulpkt_cnt;
	volatile unsigned int rpf_aep_cnt;
	volatile unsigned int runt; /* 0xC0 */
	volatile unsigned int crcer_ftl_cnt;
	volatile unsigned int rcol_rlost_cnt;
	volatile unsigned int bistr;
	volatile unsigned int bmrcr; /* 0xD0 */
	volatile unsigned int rsvd5[4]; /* 0xD4 ~ 0xE3 */
	volatile unsigned int acir; /* 0xE4 AXI additional Control Information */
	volatile unsigned int gisr; /* 0xE8 GMAC Interface Selection */
	volatile unsigned int rsvd6;
	volatile unsigned int eee; /* 0xF0 */
	volatile unsigned int revr;
	volatile unsigned int fear;
	volatile unsigned int rsvd7[1];
	volatile unsigned int ptp_rx_addr; /* 0x100 PTP Destination Unicast IP Address on Rx path */
	volatile unsigned int ptp_tx_addr; /* 0x104  PTP Destination Unicast IP Address on Tx path */
	volatile unsigned int rsvd8[2];
	volatile unsigned int ptp_tx_sec; /* 0x110 PTP TX Event Frame Time-stamp for seconds */
	volatile unsigned int ptp_tx_nsec; /* 0x114 PTP TX Event Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_rx_sec; /* 0x118 PTP RX Event Frame Time-stamp for seconds */
	volatile unsigned int ptp_rx_nsec; /* 0x11C PTP RX Event Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_tx_p_sec; /* 0x120 PTP TX Peer Frame Time-stamp for seconds */
	volatile unsigned int ptp_tx_p_nsec; /* 0x124 PTP TX Peer Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_rx_p_sec; /* 0x128 PTP RX Peer Frame Time-stamp for seconds */
	volatile unsigned int ptp_rx_p_nsec; /* 0x12C PTP RX Peer Frame Time-stamp for nanoseconds */
	volatile unsigned int ptp_nnsec_tmr; /* 0x130 PTP Timer Nano-nansecond */
	volatile unsigned int ptp_nsec_tmr; /* 0x134 PTP Timer nanosecond */
	volatile unsigned int ptp_sec_tmr; /* 0x138 PTP Second */
	volatile unsigned int ptp_ns_period; /* 0x13C PTP period increment, max is 255 ns */
	volatile unsigned int ptp_nns_period; /* 0x140 PTP period increment, max is 0.999999999 ns */
	volatile unsigned int ptp_offset; /* 0x144 PTP period offset increment */
	volatile unsigned int ptp_tmr_adj; /* 0x148 PTP Timer Adjustment */
};

extern struct FTGMAC030_REGS *mac_regs;

/*
 * Interrupt Status Register 0x00 and 0x04
 */
#define FTGMAC030_INT_PDELAY_RESP_OUT (1 << 24)
#define FTGMAC030_INT_PDELAY_RESP_IN  (1 << 23)
#define FTGMAC030_INT_PDELAY_REQ_OUT  (1 << 22)
#define FTGMAC030_INT_PDELAY_REQ_IN   (1 << 21)
#define FTGMAC030_INT_DELAY_REQ_OUT   (1 << 20)
#define FTGMAC030_INT_DELAY_REQ_IN    (1 << 19)
#define FTGMAC030_INT_SYNC_OUT        (1 << 18)
#define FTGMAC030_INT_SYNC_IN         (1 << 17)
#define FTGMAC030_INT_TSU_SEC_INC     (1 << 16)
#define FTGMAC030_INT_RX_LPI_IN       (1 << 12)
#define FTGMAC030_INT_RX_LPI_EXIT     (1 << 11)
#define FTGMAC030_INT_HPTXBUF_UNAVA   (1 << 10)
#define FTGMAC030_INT_PHYSTS_CHG      (1 << 9)
#define FTGMAC030_INT_AHB_ERR         (1 << 8)
#define FTGMAC030_INT_TPKT_LOST       (1 << 7)
#define FTGMAC030_INT_NPTXBUF_UNAVA   (1 << 6)
#define FTGMAC030_INT_TPKT2F          (1 << 5)
#define FTGMAC030_INT_TPKT2E          (1 << 4)
#define FTGMAC030_INT_RPKT_LOST       (1 << 3)
#define FTGMAC030_INT_RXBUF_UNAVA     (1 << 2)
#define FTGMAC030_INT_RPKT2F          (1 << 1)
#define FTGMAC030_INT_RPKT2B          (1 << 0)

#define FTGMAC030_INT_DEFAULT ( \
				FTGMAC030_INT_RPKT2B | \
				FTGMAC030_INT_RXBUF_UNAVA | \
				FTGMAC030_INT_RPKT_LOST | \
				FTGMAC030_INT_TPKT2E | \
				FTGMAC030_INT_TPKT2F | \
				FTGMAC030_INT_TPKT_LOST | \
				FTGMAC030_INT_AHB_ERR | \
				FTGMAC030_INT_PHYSTS_CHG \
				)

#define FTGMAC030_INT_ALL ( \
				FTGMAC030_INT_RPKT2B | \
				FTGMAC030_INT_RPKT2F | \
				FTGMAC030_INT_RXBUF_UNAVA | \
				FTGMAC030_INT_RPKT_LOST | \
				FTGMAC030_INT_TPKT2E | \
				FTGMAC030_INT_TPKT2F | \
				FTGMAC030_INT_NPTXBUF_UNAVA | \
				FTGMAC030_INT_TPKT_LOST | \
				FTGMAC030_INT_AHB_ERR | \
				FTGMAC030_INT_PHYSTS_CHG | \
				FTGMAC030_INT_HPTXBUF_UNAVA \
				)
/*
 * MAC Most Significant Address Register 0x08
 */
#define FTGMAC030_MADR(x) ((x & 0xffff) << 0)

/*
 * MAC Least Siginificant Address Register 0x0C
 */
#define FTGMAC030_LADR(x) ((x & 0xffffffff) << 0)

/*
 * TX Interrupt Timer Control Register 0x30
 */
#define FTGMAC030_TXITC_TIME_SEL     (1 << 16)
#define FTGMAC030_TXITC_CYL(x)       ((x & 0xff) << 8)
#define FTGMAC030_TXITC_THR(x)       ((x & 0x7) << 4)
#define FTGMAC030_TXITC_THR_UNIT(x)  ((x & 0x3) << 0)

#define FTGMAC030_TXITC_DEFAULT       FTGMAC030_TXITC_THR(1)

/*
 * RX Interrupt Timer Control Register 0x34
 */
#define FTGMAC030_RXITC_RST(x)       ((x & 0xff) << 16)
#define FTGMAC030_RXITC_TIME_SEL     (1 << 16)
#define FTGMAC030_RXITC_CYL(x)       ((x & 0xff) << 8)
#define FTGMAC030_RXITC_THR(x)       ((x & 0x7) << 4)
#define FTGMAC030_RXITC_THR_UNIT(x)  ((x & 0x3) << 0)

#define FTGMAC030_RXITC_DEFAULT       FTGMAC030_RXITC_THR(1)

/*
 * Automatic Polling Timer Control Register 0x34
 */
#define FTGMAC030_APTC_TX_TIME   (1 << 12)
#define FTGMAC030_APTC_TX_CNT(x) ((x & 0xf) << 8)
#define FTGMAC030_APTC_RX_TIME   (1 << 4)
#define FTGMAC030_APTC_RX_CNT(x) ((x & 0xf) << 0)

#define FTGMAC030_APTC_DEFAULT FTGMAC030_APTC_RX_CNT(1)

/*
 * DMA Burst Length and Arbitration Control Register 0x38
 */
#define FTGMAC030_DBLAC_IFG_INC        (1 << 23)
#define FTGMAC030_DBLAC_IFG_CNT(x)     ((x & 0x7) << 20)
#define FTGMAC030_DBLAC_TXDES_SIZE(x)  ((x & 0xf) << 16)
#define FTGMAC030_DBLAC_RXDES_SIZE(x)  ((x & 0xf) << 12)
#define FTGMAC030_DBLAC_TXBST_SIZE(x)  ((x & 0x3) << 10)
#define FTGMAC030_DBLAC_RXBST_SIZE(x)  ((x & 0x3) << 8)
#define FTGMAC030_DBLAC_RX_THR_EN      (1 << 6)
#define FTGMAC030_DBLAC_RXFIFO_HTHR(x) ((x & 0x7) << 3)
#define FTGMAC030_DBLAC_RXFIFO_LTHR(x) ((x & 0x7) << 0)

#define FTGMAC030_DBLAC_DEFAULT FTGMAC030_DBLAC_RXFIFO_LTHR(2) | \
				FTGMAC030_DBLAC_RXFIFO_HTHR(6) | \
				FTGMAC030_DBLAC_RX_THR_EN | \
				FTGMAC030_DBLAC_RXBST_SIZE(3) | \
				FTGMAC030_DBLAC_TXBST_SIZE(3)
/*
 * DMA/FIFO State Register 0x3c
 */
#define FTGMAC030_DMAFIFOS_TXD_REQ       (1 << 31)
#define FTGMAC030_DMAFIFOS_RXD_REQ       (1 << 30)
#define FTGMAC030_DMAFIFOS_DARB_TXGNT    (1 << 29)
#define FTGMAC030_DMAFIFOS_DARB_RXGNT    (1 << 28)
#define FTGMAC030_DMAFIFOS_TXFIFO_EMPTY  (1 << 27)
#define FTGMAC030_DMAFIFOS_RXFIFO_EMPTY  (1 << 26)
#define FTGMAC030_DMAFIFOS_TXDMA3_SM(x)  ((x & 0xf) << 18)
#define FTGMAC030_DMAFIFOS_TXDMA2_SM(x)  ((x & 0xf) << 16)
#define FTGMAC030_DMAFIFOS_TXDMA1_SM(x)  ((x & 0xf) << 12)
#define FTGMAC030_DMAFIFOS_RXDMA3_SM(x)  ((x & 0xf) << 8)
#define FTGMAC030_DMAFIFOS_RXDMA2_SM(x)  ((x & 0xf) << 4)
#define FTGMAC030_DMAFIFOS_RXDMA1_SM(x)  ((x & 0xf) << 0)

/*
 * Revision Register 0x40
 */
#define FTGMAC030_REV_B1(x) ((x >> 16) & 0xff)
#define FTGMAC030_REV_B2(x) ((x >> 8) & 0xff)
#define FTGMAC030_REV_B3(x) (x & 0xff)

/*
 * Feature Register 0xF8
 */
#define FTGMAC030_FEAR_TFIFO_RSIZE(x) ((x & 0x7) >> 4)
#define FTGMAC030_FEAR_RFIFO_RSIZE(x) ((x & 0x7) >> 0)

/*
 * Transmit Priority Arbitration and FIFO Control Register 0x48
 */
#define FTGMAC030_TPAFCR_TFIFO_SIZE(x)  ((x & 0x7) << 27)
#define FTGMAC030_TPAFCR_RFIFO_SIZE(x)  ((x & 0x7) << 24)
#define FTGMAC030_TPAFCR_EARLY_TXTHR(x) ((x & 0xff) << 16)
#define FTGMAC030_TPAFCR_EARLY_RXTHR(x) ((x & 0xff) << 8)
#define FTGMAC030_TPAFCR_HPKT_THR(x)    ((x & 0xf) << 4)
#define FTGMAC030_TPAFCR_NPKT_THR(x)    ((x & 0xf) << 0)

/*
 * Receive Buffer Size Register 0x4c
 */
#define FTGMAC030_RBSR_RXBUF_SIZE(x) ((x & 0x3fff) << 0)

/*
 * MAC Control Register 0x50
 */
#define FTGMAC030_MACCR_SW_RST         (1 << 31)
#define FTGMAC030_MACCR_FULLDUP        (1 << 26)
#define FTGMAC030_MACCR_SPEED_MASK     (0x3 << 24)
#define FTGMAC030_MACCR_SPEED_1000     (2 << 24)
#define FTGMAC030_MACCR_SPEED_100      (1 << 24)
#define FTGMAC030_MACCR_SPEED_10       (0 << 24)
#define FTGMAC030_MACCR_HPTXR          (1 << 22)
#define FTGMAC030_MACCR_LOOPBACK       (1 << 21)
#define FTGMAC030_MACCR_PTP_EN         (1 << 20)
#define FTGMAC030_MACCR_REMOVE_VLAN    (1 << 18)
#define FTGMAC030_MACCR_CRC_APD        (1 << 17)
#define FTGMAC030_MACCR_DROP_CRC_ERR   (1 << 16)
#define FTGMAC030_MACCR_ENRX_IN_HALFTX (1 << 14)
#define FTGMAC030_MACCR_JUMBO_LF       (1 << 13)
#define FTGMAC030_MACCR_RX_RUNT        (1 << 12)
#define FTGMAC030_MACCR_BROADPKT       (1 << 11)
#define FTGMAC030_MACCR_MULTIPKT       (1 << 10)
#define FTGMAC030_MACCR_HT_EN          (1 << 9)
#define FTGMAC030_MACCR_ALLADDR        (1 << 8)
#define FTGMAC030_MACCR_RXMAC          (1 << 3)
#define FTGMAC030_MACCR_TXMAC          (1 << 2)
#define FTGMAC030_MACCR_RXDMA          (1 << 1)
#define FTGMAC030_MACCR_TXDMA          (1 << 0)

#define FTGMAC030_MACCR_DEFAULT ( \
					FTGMAC030_MACCR_RX_RUNT | \
					FTGMAC030_MACCR_CRC_APD | \
					FTGMAC030_MACCR_FULLDUP | \
					FTGMAC030_MACCR_HPTXR | \
					FTGMAC030_MACCR_TXDMA | \
					FTGMAC030_MACCR_RXMAC | \
					FTGMAC030_MACCR_RXDMA | \
					FTGMAC030_MACCR_TXMAC)

/*
 * MAC Status Register 0x54
 */
#define FTGMAC030_MACSR_COL_EXCEED
#define FTGMAC030_MACSR_LATE_COL
#define FTGMAC030_MACSR_XPKT_LOST
#define FTGMAC030_MACSR_XPKT_OK
#define FTGMAC030_MACSR_RUNT
#define FTGMAC030_MACSR_FTL
#define FTGMAC030_MACSR_CRC_ERR
#define FTGMAC030_MACSR_RPKT_LOST
#define FTGMAC030_MACSR_RPKT_SAVE
#define FTGMAC030_MACSR_COL
#define FTGMAC030_MACSR_BROADCAST
#define FTGMAC030_MACSR_MULTICAST

/*
 * Test Mode Register 0x58
 */
#define FTGMAC030_TM_PTIMER_TEST    (1 << 20)
#define FTGMAC030_TM_ITIMER_TEST    (1 << 19)
#define FTGMAC030_TM_TEST_COL       (1 << 15)
#define FTGMAC030_TM_TEST_BKOFF(x)  ((x & 0x3ff) << 5)
#define FTGMAC030_TM_TEST_EXSTHR(x) ((x & 0x1f) << 0)

/*
 * PHY Control Register 0x60
 */
#define FTGMAC030_MDIO_SOF            1
#define FTGMAC030_MDIO_EXT_SOF        0
#define FTGMAC030_MDIO_OP_RD          2
#define FTGMAC030_MDIO_OP_WR          1

#define FTGMAC030_PHYCR_PHYWR         (1 << 27)
#define FTGMAC030_PHYCR_PHYRD         (1 << 26)
#define FTGMAC030_PHYCR_REGAD(x)      ((x & 0x1f) << 21)
#define FTGMAC030_PHYCR_PHYAD(x)      ((x & 0x1f) << 16)
#define FTGMAC030_PHYCR_OP(x)         ((x & 0x3) << 14)
#define FTGMAC030_PHYCR_SOF(x)        ((x & 0x3) << 12)

/*
 * PHY Data Register 0x64
 */
#define FTGMAC030_PHYDATA_MIIRDATA(x) ((x & 0xffff0000) >> 16)
#define FTGMAC030_PHYDATA_MIIWDATA(x) ((x & 0xffff) >> 0)

/*
 * Flow Control Register 0x68
 */
#define FTGMAC030_FCR_PAUSE_TIME(x) ((x & 0xffff) << 16)
#define FTGMAC030_FCR_FC_H_L(x)     ((x & 0x7f) << 9)
#define FTGMAC030_FCR_HTHR          (1 << 8)
#define FTGMAC030_FCR_RX_PAUSE      (1 << 4)
#define FTGMAC030_FCR_TXPAUSED      (1 << 3)
#define FTGMAC030_FCR_FCTHR_EN      (1 << 2)
#define FTGMAC030_FCR_TX_PAUSE      (1 << 1)
#define FTGMAC030_FCR_FC_EN         (1 << 0)

/*
 * Back Pressure Register 0x6c
 */
#define ftgmac030_BPR_BK_LOW(x)    ((x & 0x7f) << 8)
#define ftgmac030_BPR_BKJAM_LEN(x) ((x & 0xf) << 4)
#define ftgmac030_BPR_BKADR_MODE   (1 << 1)
#define ftgmac030_BPR_BKEN         (1 << 0)

/*
 * Wake-On-LAN Control Register 0x70
 */
#define FTGMAC030_WOLCR_WOL_TYPE(x)   ((x & 0x3) << 24)
#define FTGMAC030_WOLCR_SW_PDNPHY     (1 << 18)
#define FTGMAC030_WOLCR_WAKEUP_SEL(x) ((x & 0x3) << 16)
#define FTGMAC030_WOLCR_WAKEUP4       (1 << 6)
#define FTGMAC030_WOLCR_WAKEUP3       (1 << 5)
#define FTGMAC030_WOLCR_WAKEUP2       (1 << 4)
#define FTGMAC030_WOLCR_WAKEUP1       (1 << 3)
#define FTGMAC030_WOLCR_MAGICPKT      (1 << 2)
#define FTGMAC030_WOLCR_LINKCHG1      (1 << 1)
#define FTGMAC030_WOLCR_LINKCHG0      (1 << 0)

/*
 * Wake-On-LAN Status Register 0x74
 */
#define FTGMAC030_WOLSR_WAKEUP4  (1 << 6)
#define FTGMAC030_WOLSR_WAKEUP3  (1 << 5)
#define FTGMAC030_WOLSR_WAKEUP2  (1 << 4)
#define FTGMAC030_WOLSR_WAKEUP1  (1 << 3)
#define FTGMAC030_WOLSR_MAGICPKT (1 << 2)
#define FTGMAC030_WOLSR_LINKCHG1 (1 << 1)
#define FTGMAC030_WOLSR_LINKCHG0 (1 << 0)

/*
 * Transmit descriptor, aligned to 16 bytes
 */
typedef struct _ftgmac030_txdes {
	unsigned int txdes0;
	unsigned int txdes1;
	unsigned int txdes2; /* reserved */
	unsigned int txdes3;
} FTGMAC030_TXDESC;

#define FTGMAC030_TXDES0_TXDMA_OWN   (1 << 31)
#define FTGMAC030_TXDES0_FTS         (1 << 29)
#define FTGMAC030_TXDES0_LTS         (1 << 28)
#define FTGMAC030_TXDES0_CRC_ERR     (1 << 19)
#define FTGMAC030_TXDES0_EDOTR       (1 << 15)
#define FTGMAC030_TXDES0_BUF_SIZE(x) ((x & 0x3fff) << 0)

#define FTGMAC030_TXDES1_TXIC         (1 << 31)
#define FTGMAC030_TXDES1_TX2FIC       (1 << 30)
#define FTGMAC030_TXDES1_LLC_PKT      (1 << 22)
#define FTGMAC030_TXDES1_IPV6_PKT     (1 << 20)
#define FTGMAC030_TXDES1_OTHER_PKT    (2 << 20)
#define FTGMAC030_TXDES1_IPCS_EN      (1 << 19)
#define FTGMAC030_TXDES1_UDPCS_EN     (1 << 18)
#define FTGMAC030_TXDES1_TCPCS_EN     (1 << 17)
#define FTGMAC030_TXDES1_INS_VLAN     (1 << 16)
#define FTGMAC030_TXDES1_VLAN_TAGC(x) ((x & 0xffff) << 0)

#define FTGMAC030_TXDES1_DEFAULT (FTGMAC030_TXDES1_TXIC | FTGMAC030_TXDES1_TX2FIC)
/*
 * Receive descriptor, aligned to 16 bytes
 */
typedef struct _ftgmac030_rxdes {
	unsigned int rxdes0;
	unsigned int rxdes1;
	unsigned int rxdes2; /* reserved */
	unsigned int rxdes3;
} FTGMAC030_RXDESC;

#define FTGMAC030_RXDES0_RXPKT_RDY    (1 << 31)
#define FTGMAC030_RXDES0_FRS          (1 << 29)
#define FTGMAC030_RXDES0_LRS          (1 << 28)
#define FTGMAC030_RXDES0_PAUSE_FRAME  (1 << 25)
#define FTGMAC030_RXDES0_PAUSE_OPCODE (1 << 24)
#define FTGMAC030_RXDES0_FIFO_FULL    (1 << 23)
#define FTGMAC030_RXDES0_RX_ODD_NB    (1 << 22)
#define FTGMAC030_RXDES0_RUNT         (1 << 21)
#define FTGMAC030_RXDES0_FTL          (1 << 20)
#define FTGMAC030_RXDES0_CRC_ERR      (1 << 19)
#define FTGMAC030_RXDES0_RX_ERR       (1 << 18)
#define FTGMAC030_RXDES0_BROADCAST    (1 << 17)
#define FTGMAC030_RXDES0_MULTICAST    (1 << 16)
#define FTGMAC030_RXDES0_EDORR        (1 << 15)
#define FTGMAC030_RXDES0_VDBC(x)      ((x & 0x3fff) << 0)

#define FTGMAC030_RXDES1_IPCS_FAIL     (1 << 27)
#define FTGMAC030_RXDES1_UDPCS_FAIL    (1 << 26)
#define FTGMAC030_RXDES1_TCPCS_FAIL    (1 << 25)
#define FTGMAC030_RXDES1_VLAN_AVA      (1 << 24)
#define FTGMAC030_RXDES1_DF            (1 << 23)
#define FTGMAC030_RXDES1_LLC_PKT       (1 << 22)
#define FTGMAC030_RXDES1_PROTL_TYPE(x) ((x >> 20) & 0x3)
#define FTGMAC030_RXDES1_IP6_TYPE(x)   ((x >> 19) & 0x1)
#define FTGMAC030_RXDES1_VLAN_PRIO(x)  ((x & 7) <<  13)
#define FTGMAC030_RXDES1_VLAN_CFI(x)   ((x & 1) <<  12)
#define FTGMAC030_RXDES1_VLAN_VID(x)   (x & 0x0fff)
#define FTGMAC030_RXDES1_VLAN_TAGC(x)  (x & 0xffff)

#define FTGMAC030_RXDES1_PROTL_NOTIP   0
#define FTGMAC030_RXDES1_PROTL_IP4     1
#define FTGMAC030_RXDES1_PROTL_TCPIP   2
#define FTGMAC030_RXDES1_PROTL_UDPIP   3

/* 
 * ftgmac030 phy register offset definition
 */
#define PHY_MAX_ADDR                  32

#define MII_REG_BMCR               0
#define MII_REG_BMSR               1
#define MII_REG_ID1                2
#define MII_REG_ID2                3
#define MII_REG_ANAR               4
#define MII_REG_ANLPAR             5
#define MII_REG_ANER               6
#define MII_REG_ANNPTR             7
#define MII_REG_ANLPNP             8
#define MII_REG_1000_CR            9
#define MII_REG_1000_SR            10
#define MII_REG_MMD_REGCR          13
#define MII_REG_MMD_ADDAR          14

/*
 * Basic mode control register (00)
 */
#define MII_BMCR_SPEED_M          (1 << 6)
#define MII_BMCR_COLLISION        (1 << 7)
#define MII_BMCR_DUPLEX           (1 << 8)
#define MII_BMCR_RESTART_AUTO_NEG (1 << 9)
#define MII_BMCR_ISOLATE          (1 << 10)
#define MII_BMCR_POWER_DOWN       (1 << 11)
#define MII_BMCR_AUTO_NEG_EN      (1 << 12)
#define MII_BMCR_SPEED_L          (1 << 13)
#define MII_BMCR_LOOPBACK         (1 << 14)
#define MII_BMCR_RESET            (1 << 15)

/*
 * Basic mode status register (01)
 */
#define MII_BMSR_EXTENDED          (1 << 0)
#define MII_BMSR_JABBER            (1 << 1)
#define MII_BMSR_LINK_STATUS       (1 << 2)
#define MII_BMSR_AUTO_NEG          (1 << 3)
#define MII_BMSR_REMOTE_FAULT      (1 << 4)
#define MII_BMSR_AUTO_NEG_COMPLETE (1 << 5)
#define MII_BMSR_MF_PREAMBLE       (1 << 6)
#define MII_BMSR_10_T_HALF         (1 << 11)
#define MII_BMSR_10_T_FULL         (1 << 12)
#define MII_BMSR_100_TX_HALF       (1 << 13)
#define MII_BMSR_100_TX_FULL       (1 << 14)
#define MII_BMSR_100_T4            (1 << 15)

/*
 * PHY identifier register 1, read only (02)
 */
#define MII_ID1_OUI_DEFAULT_MSB         ((MII_DEFAULT_OUI >> 6) & 0xffff)

/*
 * PHY identifier register 2, read only (03)
 */
#define MII_ID2_MDL_REV(x)      (x & 0x1f)
#define MII_ID2_MDL_NUM(x)      ((x >> 4) & 0x3f)
#define MII_ID2_OUI_LSB(x)      ((x >> 10) & 0x3f)
#define MII_ID2_OUI_DEFAULT_LSB (MII_DEFAULT_OUI & 0x3f)

/*
 * Auto negotiation advertisement register (04)
 */
#define MII_ANAR_SELECTOR(x) ((x & 0x1f) << 0)
#define MII_ANAR_10_HD       (1 << 5)
#define MII_ANAR_10_FD       (1 << 6)
#define MII_ANAR_100_HD      (1 << 7)
#define MII_ANAR_100_FD      (1 << 8)
#define MII_ANAR_T4          (1 << 9)
#define MII_ANAR_ASM(x)      ((x & 0x3) << 10)
#define MII_ANAR_RF          (1 << 13)
#define MII_ANAR_ACK         (1 << 14)
#define MII_ANAR_NP          (1 << 15)

/*
 * Auto negotiation link partner ability register (05)
 */
#define MII_ANLPAR_SELECTOR(x) ((x & 0x1f) << 0)
#define MII_ANLPAR_10_HD       (1 << 5)
#define MII_ANLPAR_10_FD       (1 << 6)
#define MII_ANLPAR_100_HD       (1 << 7)
#define MII_ANLPAR_100_FD       (1 << 8)
#define MII_ANLPAR_T4          (1 << 9)
#define MII_ANLPAR_PAUSE       (1 << 10)
#define MII_ANLPAR_LP_DIR      (1 << 11)
#define MII_ANLPAR_RF          (1 << 13)
#define MII_ANLPAR_ACK         (1 << 14)
#define MII_ANLPAR_NP          (1 << 15)

/*
 * Auto-Nego Expansion Register (06)
 */
#define MII_ANER_PAGE_RX (1 << 1)

/*
 * 1000 Base-T control register (09)
 */
#define MII_1000_CR_HD (1 << 8)
#define MII_1000_CR_FD (1 << 9)

/*
 * 1000 Base-T status register (10)
 */
#define MII_1000_SR_HD (1 << 10)
#define MII_1000_SR_FD (1 << 11)

/*
 * MMD access control register (13)
 */
#define MII_MMD_REGCR_DEVAD(x) ((x & 0x1f) << 0)
#define MII_MMD_REGCR_MODE(x)  ((x & 0x3) << 14)
#define MII_MMD_MODE_ADDR       0x0
#define MII_MMD_MODE_NO_POST    0x1
#define MII_MMD_MODE_READ_WRITE 0x2
#define MII_MMD_MODE_WRITE_ONLY 0x3

/*
 * MMD access address data register (14)
 */
#define MII_MMD_ADDAR_DATA(x)  ((x) & 0xffff)

typedef enum {
	NP_TX_QUEUE = 0,
	HP_TX_QUEUE,
	NUM_TX_QUEUE
}TX_QUEUE;


/* 
 * private data of ftgmac030 device, not be used by programmers 
 */
typedef enum{
	FULL = 0,
	HALF = 1
}DUPLEX;

typedef enum{
	_1000 = 0,
	_100  = 1,
	_10   = 2
}SPEED;

typedef struct mac_ctrl FTGMAC030_Control;

typedef struct _ftgmac030_tx_queue {
	FTGMAC030_TXDESC *txdes_top;
	FTGMAC030_TXDESC *txdes_cur;
	uint32_t txdes_num;
	void (*kick_off)(void);
} FTGMAC030_TX_QUEUE;

/* 
 * multiple descriptor structure 
 */
struct multi_desc {
	uint32_t phy_tx_addr;
	uint32_t size;
};

#define MAX_PKT_SIZE                    0x8000 /* support for jumbo frame */
#define RX_BUF_SIZE                     4096 /* support for jumbo frame */

struct mac_ctrl {
	/* Parameters for open mac controller */
	uint32_t maccr;
	uint32_t aptc;
	void (*cmpl_isr)(FTGMAC030_Control *ctrl);
	void (*open)(FTGMAC030_Control *ctrl, void (*cmpl_isr)(FTGMAC030_Control *ctrl)); /* default open */
	void (*open_maccr)(FTGMAC030_Control *ctrl, uint32_t maccr,
			   void (*cmpl_isr)(FTGMAC030_Control *));/* open with customized maccr */
	void (*open_aptc)(FTGMAC030_Control *ctrl, uint32_t aptc,
			   void (*cmpl_isr)(FTGMAC030_Control *));/* open with customized aptc */
	void (*xmit) (FTGMAC030_Control *ctrl, uint32_t phy_tx_addr, uint32_t size, uint32_t txdes1);
	void (*xmit_multi_desc)(FTGMAC030_TX_QUEUE *txq, const uint32_t number, struct multi_desc txdeses[], uint32_t txdes1);
	void (*xmit_one_desc)(FTGMAC030_TX_QUEUE *txq, const uint32_t phy_tx_addr, uint32_t size, uint32_t txdes1);
	void (*half_dup_10)(FTGMAC030_Control *ctrl);
	void (*half_dup_100)(FTGMAC030_Control *ctrl);
	void (*half_dup_1000)(FTGMAC030_Control *ctrl);
	void (*full_dup_10)(FTGMAC030_Control *ctrl);
	void (*full_dup_100)(FTGMAC030_Control *ctrl);
	void (*full_dup_1000)(FTGMAC030_Control *ctrl);
	uint8_t (*set_multicast_address)(FTGMAC030_Control *ctrl, uint8_t *mac_addr);
	uint8_t *(*get_mac_addr)(void);
	void (*rx_manual)(void);
	void (*enter_power_down)(FTGMAC030_Control *ctrl, uint32_t power_down_setting,
				 uint8_t *wakeup_frame, uint32_t wakeup_frame_size);
	void (*exit_power_down)(void);
	void (*set_wakeup_mask)(FTGMAC030_Control *ctrl, uint32_t mask_value, uint32_t mask_index);
	void (*set_flow_control)(uint16_t pause_time, uint8_t high_threshold, uint8_t low_threshold,
				 uint8_t fc_thr_en, uint8_t fc_en);
	void (*xmit_pause_frame)(void);
	uint32_t (*wait_pause_frame)(void);
	uint32_t (*is_link_up)(FTGMAC030_Control *ctrl);
	uint32_t (*scan_phy_addr)(FTGMAC030_Control *ctrl);
	uint32_t (*phy_processing)(FTGMAC030_Control *ctrl);
	void (*phy_auto_neo)(FTGMAC030_Control *ctrl);
	void (*clear_phy_int)(FTGMAC030_Control *ctrl);
	void (*set_phy_int)(FTGMAC030_Control *ctrl);
	int (*change_duplex_speed)(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);

	uint8_t rx_buff[MAX_PKT_SIZE];
	uint32_t rx_len;
	uint32_t rxdes0;
	uint32_t rxdes1;
	uint32_t rx_err_happen;
	void (*show_link_status)(FTGMAC030_Control *ctrl);

	/* for command line interface */
	void (*phywrite)(FTGMAC030_Control *ctrl, uint16_t value, uint16_t regad);
	uint16_t (*phyread)(FTGMAC030_Control *ctrl, uint16_t regad);
	uint16_t (*mmdread)(FTGMAC030_Control *ctrl, uint16_t dev_addr, uint16_t reg_addr);
	void (*mmdwrite)(FTGMAC030_Control *ctrl, uint16_t value, uint16_t dev_addr, uint16_t reg_addr);

	void (*macwrite)(uint32_t value, uint32_t addr);
	uint32_t (*macread)(uint32_t addr);
	void (*clear_tx_finish)(void);
	void (*fill_txdes)(FTGMAC030_TX_QUEUE *txq, const uint32_t phy_tx_addr, uint32_t size);
	void (*phy_loopback)(void);

	FTGMAC030_TX_QUEUE tx_queue[NUM_TX_QUEUE];
	uint32_t tx_queue_num;
	FTGMAC030_RXDESC *rxdes_top;
	FTGMAC030_RXDESC *rxdes_cur;
	uint32_t rxdes_num;
	uint16_t phyad;
	uint32_t wakeup_frame_mask[4];

	/* green ethernet parameters */
	uint16_t eee_ar_value;
	/* value 1 -> enter lpi mode, value 0 -> exit lpi mode*/
	uint32_t rx_lpi_mode;
	uint32_t tx_lpi_mode;
	uint32_t tfifo_rsize;
	uint32_t rfifo_rsize;

	uint32_t default_speed;

	uint32_t rx_packet_cnt;
};

/**********************************************************************************************************
 * public function calls
 **********************************************************************************************************/
extern FTGMAC030_Control *malloc_ftgmac030_control(void);
extern void free_ftgmac030_control(FTGMAC030_Control *ctrl);
extern void ftgmac030_isr(void *);
extern unsigned char ftgmac030_pause(void);
extern unsigned int ftgmac030_crc32(char *s, int len);
extern int ftgmac030_addrcmp(const uint8_t *addr1, const uint8_t *addr2);
extern void ftgmac030_delaysecs(uint32_t i);
extern uint32_t isM1111(void);
extern void show_hw_config(void);
extern char ftuart_kbhit(void);
extern int scan_string(char *buf);
extern char ftuart_getc(void);
/*extern uint16_t e_phyread16(uint16_t regad);
extern void e_phywrite16(uint16_t value, uint16_t regad);
extern void e_auto_neo(void);*/
uint32_t ftgmac030_scan_phy_addr(FTGMAC030_Control *ctrl);

/**********************************************************************************************************
 * private function calls
 **********************************************************************************************************/
uint16_t phyread16(FTGMAC030_Control *ctrl, uint16_t regad);
void phywrite16(FTGMAC030_Control *ctrl, uint16_t value, uint16_t regad);
uint16_t mmdread16(FTGMAC030_Control *ctrl, uint16_t dev_addr, uint16_t reg_addr);
void mmdwrite16(FTGMAC030_Control *ctrl, uint16_t value, uint16_t dev_addr, uint16_t reg_addr);
void reset_phy(FTGMAC030_Control *ctrl);
unsigned int phy_show_link_status(FTGMAC030_Control *ctrl);
uint32_t _32_bit_reverse(uint32_t value);
uint8_t _8_bit_reverse(uint8_t value);

uint32_t ftgmac030_bit_reserve(uint32_t value);

/*
 * ftgmac030 device interface implementation
 */
void ftgmac030_open(FTGMAC030_Control *ctrl, void (*cmpl_isr)(FTGMAC030_Control *));
void ftgmac030_open_maccr(FTGMAC030_Control *ctrl, uint32_t maccr, void (*cmpl_isr)(FTGMAC030_Control *));
void ftgmac030_open_aptc(FTGMAC030_Control *ctrl, uint32_t aptc, void (*cmpl_isr)(FTGMAC030_Control *));
void ftgmac030_xmit(FTGMAC030_Control *ctrl, const uint32_t phy_tx_addr, uint32_t size, uint32_t txdes1);
void ftgmac030_xmit_multi_desc(FTGMAC030_TX_QUEUE *txq, const uint32_t number, struct multi_desc txdeses[], uint32_t txdes1);
void ftgmac030_xmit_one_desc(FTGMAC030_TX_QUEUE *txq, const uint32_t phy_tx_addr, uint32_t size, uint32_t txdes1);
void ftgmac030_half_dup_10(FTGMAC030_Control *ctrl);
void ftgmac030_half_dup_100(FTGMAC030_Control *ctrl);
void ftgmac030_half_dup_1000(FTGMAC030_Control *ctrl);
void ftgmac030_full_dup_10(FTGMAC030_Control *ctrl);
void ftgmac030_full_dup_100(FTGMAC030_Control *ctrl);
void ftgmac030_full_dup_1000(FTGMAC030_Control *ctrl);
uint8_t ftgmac030_set_multicast_address(FTGMAC030_Control *ctrl, uint8_t *mac_addr);
uint8_t *ftgmac030_get_mac_addr(void);
void ftgmac030_rx_manual(void);
void ftgmac030_enter_power_down(FTGMAC030_Control *ctrl, uint32_t power_down_setting, uint8_t *wakeup_frame, uint32_t wakeup_frame_size);
void ftgmac030_exit_power_down(void);
void ftgmac030_set_wakeup_mask(FTGMAC030_Control *ctrl, uint32_t mask_value, uint32_t mask_index);
void ftgmac030_set_flow_control(uint16_t pause_time, uint8_t high_low, uint8_t threshold, uint8_t fc_thr_en, uint8_t fc_en);
void ftgmac030_xmit_pause_frame(void);
uint32_t ftgmac030_wait_pause_frame(void);
void ftgmac030_wait_link_up(FTGMAC030_Control *ctrl);
uint32_t ftgmac030_is_link_up(FTGMAC030_Control *ctrl);
void ftgmac030_show_link_status(FTGMAC030_Control *ctrl);
void ftgmac030_fill_txdes(FTGMAC030_TX_QUEUE *txq, const uint32_t phy_tx_addr, uint32_t size);
void ftgmac030_kickoff_nptx(void);
void ftgmac030_kickoff_hptx(void);
void ftgmac030_phy_loopback(void);

/*
 * wake up frame related function calls
 */
uint32_t ftgmac030_wakeup_crc32(uint8_t *wakeup_frame, uint32_t size, uint32_t *mask);

/*
 * isr handler function calls
 */
void hptxbuf_unava_handler(void);
void physts_chg_handler(FTGMAC030_Control *ctrl);
void ahb_err_handler(void);
void tpkt_lost_handler(void);
void nptxbuf_unava_handler(void);
void tpkt2e_handler(FTGMAC030_Control *ctrl);
void rpkt_lost_handler(void);
void rxbuf_unava_handler(void);
void rpkt2f_handler(void);
void rpkt2b_handler(FTGMAC030_Control *ctrl);

/* Vendor PHY related functions */
#define REALTEK_OUI       0x000732
#define RTL8211_MDL_NUM   0x11

extern uint32_t rtl8211_phy_processing(FTGMAC030_Control *ctrl);
extern void rtl8211_phy_auto_neo(FTGMAC030_Control *ctrl);
extern void rtl8211_phy_info(FTGMAC030_Control *ctrl);
extern void rtl8211_clear_phy_int(FTGMAC030_Control *ctrl);
extern void rtl8211_set_phy_int(FTGMAC030_Control *ctrl);
extern void rtl8211_disable_green_ethernet(FTGMAC030_Control *ctrl);
extern int rtl8211_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);
extern uint32_t rtl8211_is_link_up(FTGMAC030_Control *ctrl);
extern void rtl8211_show_link_status(FTGMAC030_Control *ctrl);

#define RTL8201_MDL_NUM   0x1

extern uint32_t rtl8201_phy_processing(FTGMAC030_Control *ctrl);
extern void rtl8201_phy_auto_neo(FTGMAC030_Control *ctrl);
extern void rtl8201_phy_info(FTGMAC030_Control *ctrl);
extern void rtl8201_clear_phy_int(FTGMAC030_Control *ctrl);
extern void rtl8201_set_phy_int(FTGMAC030_Control *ctrl);
extern void rtl8201_disable_green_ethernet(FTGMAC030_Control *ctrl);
extern int rtl8201_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);
extern uint32_t rtl8201_is_link_up(FTGMAC030_Control *ctrl);
extern void rtl8201_show_link_status(FTGMAC030_Control *ctrl);

#define MARVELL_OUI       0x005043
#define MARVELL_MDL_88E1111   0xC /* 2'b 001100 */

extern uint32_t marvell_phy_processing(FTGMAC030_Control *ctrl);
extern void marvell_phy_auto_neo(FTGMAC030_Control *ctrl);
extern void marvell_phy_info(FTGMAC030_Control *ctrl);
extern void marvell_clear_phy_int(FTGMAC030_Control *ctrl);
extern void marvell_set_phy_int(FTGMAC030_Control *ctrl);
extern int marvell_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);
extern uint32_t marvell_is_link_up(FTGMAC030_Control *ctrl);
extern void marvell_show_link_status(FTGMAC030_Control *ctrl);

#define MARVELL_MDL_88E1116R   0x24 /* 2'b 100100 */

void m88e1116r_phy_info(FTGMAC030_Control *ctrl);
uint32_t m88e1116r_config_init(FTGMAC030_Control *ctrl);
void m88e1116r_config_aneg(FTGMAC030_Control *ctrl);
int m88e1116r_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);

#define ICPLUS_OUI        0x90C3
#define ICPLUS_MDL_NUM    0x5

extern uint32_t icplus_phy_processing(FTGMAC030_Control *ctrl);
extern void icplus_phy_auto_neo(FTGMAC030_Control *ctrl);
extern void icplus_phy_info(FTGMAC030_Control *ctrl);
extern void icplus_clear_phy_int(FTGMAC030_Control *ctrl);
extern void icplus_set_phy_int(FTGMAC030_Control *ctrl);
extern int icplus_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);
extern uint32_t icplus_is_link_up(FTGMAC030_Control *ctrl);
extern void icplus_show_link_status(FTGMAC030_Control *ctrl);

extern uint32_t generic_phy_processing(FTGMAC030_Control *ctrl);
extern void generic_phy_auto_neo(FTGMAC030_Control *ctrl);
extern void generic_phy_info(FTGMAC030_Control *ctrl);
extern void generic_clear_phy_int(FTGMAC030_Control *ctrl);
extern void generic_set_phy_int(FTGMAC030_Control *ctrl);
extern int generic_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);
extern uint32_t generic_is_link_up(FTGMAC030_Control *ctrl);
extern void generic_show_link_status(FTGMAC030_Control *ctrl);

/* Generic MII registers. */
#define MII_BMCR		0x00	/* Basic mode control register */
#define MII_BMSR		0x01	/* Basic mode status register  */
#define MII_PHYSID1		0x02	/* PHYS ID 1                   */
#define MII_PHYSID2		0x03	/* PHYS ID 2                   */
#define MII_ADVERTISE		0x04	/* Advertisement control reg   */
#define MII_LPA			0x05	/* Link partner ability reg    */
#define MII_EXPANSION		0x06	/* Expansion register          */
#define MII_CTRL1000		0x09	/* 1000BASE-T control          */
#define MII_STAT1000		0x0a	/* 1000BASE-T status           */
#define	MII_MMD_CTRL		0x0d	/* MMD Access Control Register */
#define	MII_MMD_DATA		0x0e	/* MMD Access Data Register */
#define MII_ESTATUS		0x0f	/* Extended Status             */
#define MII_DCOUNTER		0x12	/* Disconnect counter          */
#define MII_FCSCOUNTER		0x13	/* False carrier counter       */
#define MII_NWAYTEST		0x14	/* N-way auto-neg test reg     */
#define MII_RERRCOUNTER		0x15	/* Receive error counter       */
#define MII_SREVISION		0x16	/* Silicon revision            */
#define MII_RESV1		0x17	/* Reserved...                 */
#define MII_LBRERROR		0x18	/* Lpback, rx, bypass error    */
#define MII_PHYADDR		0x19	/* PHY address                 */
#define MII_RESV2		0x1a	/* Reserved...                 */
#define MII_TPISTATUS		0x1b	/* TPI status for 10mbps       */
#define MII_NCONFIG		0x1c	/* Network interface config    */

/* Basic mode control register. */
#define BMCR_RESV		0x003f	/* Unused...                   */
#define BMCR_SPEED1000		0x0040	/* MSB of Speed (1000)         */
#define BMCR_CTST		0x0080	/* Collision test              */
#define BMCR_FULLDPLX		0x0100	/* Full duplex                 */
#define BMCR_ANRESTART		0x0200	/* Auto negotiation restart    */
#define BMCR_ISOLATE		0x0400	/* Isolate data paths from MII */
#define BMCR_PDOWN		0x0800	/* Enable low power state      */
#define BMCR_ANENABLE		0x1000	/* Enable auto negotiation     */
#define BMCR_SPEED100		0x2000	/* Select 100Mbps              */
#define BMCR_LOOPBACK		0x4000	/* TXD loopback bits           */
#define BMCR_RESET		0x8000	/* Reset to default state      */

/* Basic mode status register. */
#define BMSR_ERCAP		0x0001	/* Ext-reg capability          */
#define BMSR_JCD		0x0002	/* Jabber detected             */
#define BMSR_LSTATUS		0x0004	/* Link status                 */
#define BMSR_ANEGCAPABLE	0x0008	/* Able to do auto-negotiation */
#define BMSR_RFAULT		0x0010	/* Remote fault detected       */
#define BMSR_ANEGCOMPLETE	0x0020	/* Auto-negotiation complete   */
#define BMSR_RESV		0x00c0	/* Unused...                   */
#define BMSR_ESTATEN		0x0100	/* Extended Status in R15      */
#define BMSR_100HALF2		0x0200	/* Can do 100BASE-T2 HDX       */
#define BMSR_100FULL2		0x0400	/* Can do 100BASE-T2 FDX       */
#define BMSR_10HALF		0x0800	/* Can do 10mbps, half-duplex  */
#define BMSR_10FULL		0x1000	/* Can do 10mbps, full-duplex  */
#define BMSR_100HALF		0x2000	/* Can do 100mbps, half-duplex */
#define BMSR_100FULL		0x4000	/* Can do 100mbps, full-duplex */
#define BMSR_100BASE4		0x8000	/* Can do 100mbps, 4k packets  */

/* Advertisement control register. */
#define ADVERTISE_SLCT		0x001f	/* Selector bits               */
#define ADVERTISE_CSMA		0x0001	/* Only selector supported     */
#define ADVERTISE_10HALF	0x0020	/* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL	0x0020	/* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL	0x0040	/* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF	0x0040	/* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF	0x0080	/* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE	0x0080	/* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL	0x0100	/* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM	0x0100	/* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4	0x0200	/* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP	0x0400	/* Try for pause               */
#define ADVERTISE_PAUSE_ASYM	0x0800	/* Try for asymetric pause     */
#define ADVERTISE_RESV		0x1000	/* Unused...                   */
#define ADVERTISE_RFAULT	0x2000	/* Say we can detect faults    */
#define ADVERTISE_LPACK		0x4000	/* Ack link partners response  */
#define ADVERTISE_NPAGE		0x8000	/* Next page bit               */

#define ADVERTISE_FULL		(ADVERTISE_100FULL | ADVERTISE_10FULL | \
				  ADVERTISE_CSMA)
#define ADVERTISE_ALL		(ADVERTISE_10HALF | ADVERTISE_10FULL | \
				  ADVERTISE_100HALF | ADVERTISE_100FULL)

/* Link partner ability register. */
#define LPA_SLCT		0x001f	/* Same as advertise selector  */
#define LPA_10HALF		0x0020	/* Can do 10mbps half-duplex   */
#define LPA_1000XFULL		0x0020	/* Can do 1000BASE-X full-duplex */
#define LPA_10FULL		0x0040	/* Can do 10mbps full-duplex   */
#define LPA_1000XHALF		0x0040	/* Can do 1000BASE-X half-duplex */
#define LPA_100HALF		0x0080	/* Can do 100mbps half-duplex  */
#define LPA_1000XPAUSE		0x0080	/* Can do 1000BASE-X pause     */
#define LPA_100FULL		0x0100	/* Can do 100mbps full-duplex  */
#define LPA_1000XPAUSE_ASYM	0x0100	/* Can do 1000BASE-X pause asym*/
#define LPA_100BASE4		0x0200	/* Can do 100mbps 4k packets   */
#define LPA_PAUSE_CAP		0x0400	/* Can pause                   */
#define LPA_PAUSE_ASYM		0x0800	/* Can pause asymetrically     */
#define LPA_RESV		0x1000	/* Unused...                   */
#define LPA_RFAULT		0x2000	/* Link partner faulted        */
#define LPA_LPACK		0x4000	/* Link partner acked us       */
#define LPA_NPAGE		0x8000	/* Next page bit               */

/* Expansion register for auto-negotiation. */
#define EXPANSION_NWAY		0x0001	/* Can do N-way auto-nego      */
#define EXPANSION_LCWP		0x0002	/* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE	0x0004	/* This enables npage words    */
#define EXPANSION_NPCAPABLE	0x0008	/* Link partner supports npage */
#define EXPANSION_MFAULTS	0x0010	/* Multiple faults detected    */
#define EXPANSION_RESV		0xffe0	/* Unused...                   */

#define ESTATUS_1000_TFULL	0x2000	/* Can do 1000BT Full          */
#define ESTATUS_1000_THALF	0x1000	/* Can do 1000BT Half          */

/* N-way test register. */
#define NWAYTEST_RESV1		0x00ff	/* Unused...                   */
#define NWAYTEST_LOOPBACK	0x0100	/* Enable loopback for N-way   */
#define NWAYTEST_RESV2		0xfe00	/* Unused...                   */

/* 1000BASE-T Control register */
#define ADVERTISE_1000FULL	0x0200  /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF	0x0100  /* Advertise 1000BASE-T half duplex */
#define CTL1000_AS_MASTER	0x0800
#define CTL1000_ENABLE_MASTER	0x1000

/* 1000BASE-T Status register */
#define LPA_1000LOCALRXOK	0x2000	/* Link partner local receiver status */
#define LPA_1000REMRXOK		0x1000	/* Link partner remote receiver status */
#define LPA_1000FULL		0x0800	/* Link partner 1000BASE-T full duplex */
#define LPA_1000HALF		0x0400	/* Link partner 1000BASE-T half duplex */

#define LPA_DUPLEX		(LPA_10FULL | LPA_100FULL)
#define LPA_100			(LPA_100FULL | LPA_100HALF | LPA_100BASE4)

#endif
