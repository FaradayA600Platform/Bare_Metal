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

#include "ftgmac030.h"

#define RX_QUEUE_ENTRIES                128
#define NPTX_QUEUE_ENTRIES              128
#define HPTX_QUEUE_ENTRIES              128

typedef enum{
	PHY_TYPE_RTL8211EG,
	PHY_TYPE_RTL8201,
	PHY_TYPE_M88E1111,
	PHY_TYPE_M88E1116R,
	PHY_TYPE_IP101G,
	PHY_TYPE_GENERIC,
	PHY_TYPE_NUM
}phy_type;

struct phy_type_funcs{
	uint32_t (*phy_processing)(FTGMAC030_Control *ctrl);
	void (*phy_auto_neo)(FTGMAC030_Control *ctrl);
	void (*phy_info)(FTGMAC030_Control *ctrl);
	void (*clear_phy_int)(FTGMAC030_Control *ctrl);
	void (*set_phy_int)(FTGMAC030_Control *ctrl);
	int (*change_duplex_speed)(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed);
	uint32_t (*is_link_up)(FTGMAC030_Control *ctrl);
	void (*show_link_status)(FTGMAC030_Control *ctrl);
};

static const struct phy_type_funcs phy_funcs[] = {
	/* Realtek phy RTL8211 */
	{
		.phy_processing = &rtl8211_phy_processing,
		.phy_auto_neo = &rtl8211_phy_auto_neo,
		.phy_info = &rtl8211_phy_info,
		.clear_phy_int = &rtl8211_clear_phy_int,
		.set_phy_int = &rtl8211_set_phy_int,
		.change_duplex_speed = &rtl8211_change_duplex_speed,
		.is_link_up = &rtl8211_is_link_up,
		.show_link_status = &rtl8211_show_link_status,
	},
	/* Realtek phy RTL8201 */
	{
		.phy_processing = &rtl8201_phy_processing,
		.phy_auto_neo = &rtl8201_phy_auto_neo,
		.phy_info = &rtl8201_phy_info,
		.clear_phy_int = &rtl8201_clear_phy_int,
		.set_phy_int = &rtl8201_set_phy_int,
		.change_duplex_speed = &rtl8201_change_duplex_speed,
		.is_link_up = &rtl8201_is_link_up,
		.show_link_status = &rtl8201_show_link_status,
	},
	/* marvell phy M88E1111 */
	{
		.phy_processing = &marvell_phy_processing,
		.phy_auto_neo = &marvell_phy_auto_neo,
		.phy_info = &marvell_phy_info,
		.clear_phy_int = &marvell_clear_phy_int,
		.set_phy_int = &marvell_set_phy_int,
		.change_duplex_speed = &marvell_change_duplex_speed,
		.is_link_up = &marvell_is_link_up,
		.show_link_status = &marvell_show_link_status,
	},
	/* marvell phy M88E1116R */
	{
		.phy_processing = &m88e1116r_config_init,
		.phy_auto_neo = &m88e1116r_config_aneg,
		.phy_info = &m88e1116r_phy_info,
		.clear_phy_int = &marvell_clear_phy_int,
		.set_phy_int = &marvell_set_phy_int,
		.change_duplex_speed = &m88e1116r_change_duplex_speed,
		.is_link_up = &generic_is_link_up,
		.show_link_status = &generic_show_link_status,
	},
	/* ICPLUS phy IP101G */
	{
		.phy_processing = &icplus_phy_processing,
		.phy_auto_neo = &icplus_phy_auto_neo,
		.phy_info = &icplus_phy_info,
		.clear_phy_int = &icplus_clear_phy_int,
		.set_phy_int = &icplus_set_phy_int,
		.change_duplex_speed = &icplus_change_duplex_speed,
		.is_link_up = &icplus_is_link_up,
		.show_link_status = &icplus_show_link_status,
	},
	/* Generic phy driver */
	{
		.phy_processing = &generic_phy_processing,
		.phy_auto_neo = &generic_phy_auto_neo,
		.phy_info = &generic_phy_info,
		.clear_phy_int = &generic_clear_phy_int,
		.set_phy_int = &generic_set_phy_int,
		.change_duplex_speed = &generic_change_duplex_speed,
		.is_link_up = &generic_is_link_up,
		.show_link_status = &generic_show_link_status,
	},
};
#if 0
static XScuGic IntcInstance;
#endif
int mdc_cycthr;

static int initial_interrupt(void *data);
static void ftgmac030_open_control(FTGMAC030_Control *ctrl);
static void alloc_txdes(FTGMAC030_TX_QUEUE *txq, uint32_t top_addr);
static void alloc_rxdes(FTGMAC030_Control *ctrl);
static uint8_t ftgmac030_multicast_ht_bit(uint8_t *mac_addr);

struct FTGMAC030_REGS *mac_regs = (struct FTGMAC030_REGS *) FTGMAC030_REG_BASE;

/*
 * Basic read/write functions
 */
static void iowrite32(uint32_t value, uint32_t addr)
{
	outl(addr, value);
}

static uint32_t ioread32(uint32_t addr)
{
	return inl(addr);
}

/* default source mac address */
/*static const uint8_t mac_address[6] = {0x34,0x34,0x34,0x34,0x34,0x34};*/
static const uint8_t mac_address[6] = {0x08,0x08,0x08,0x08,0x08,0x08};

FTGMAC030_Control *malloc_ftgmac030_control(void)
{
	FTGMAC030_Control *ctrl;
	uint32_t phyid = 0x0;
	uint32_t phy_type = 0x0;

	ctrl = (FTGMAC030_Control *) malloc(sizeof(FTGMAC030_Control));
	if (!ctrl) {
		return NULL;
	}

	memset(ctrl, 0x00, sizeof(FTGMAC030_Control));

	/* initial ftgmac030 interface */
	ctrl->open                    = &ftgmac030_open;
	ctrl->open_maccr              = &ftgmac030_open_maccr;
	ctrl->open_aptc               = &ftgmac030_open_aptc;
	ctrl->xmit                    = &ftgmac030_xmit;
	ctrl->xmit_multi_desc         = &ftgmac030_xmit_multi_desc;
	ctrl->xmit_one_desc           = &ftgmac030_xmit_one_desc;
	ctrl->half_dup_10             = &ftgmac030_half_dup_10;
	ctrl->half_dup_100            = &ftgmac030_half_dup_100;
	ctrl->half_dup_1000           = &ftgmac030_half_dup_1000;
	ctrl->full_dup_10             = &ftgmac030_full_dup_10;
	ctrl->full_dup_100            = &ftgmac030_full_dup_100;
	ctrl->full_dup_1000           = &ftgmac030_full_dup_1000;
	ctrl->set_multicast_address   = &ftgmac030_set_multicast_address;
	ctrl->get_mac_addr            = &ftgmac030_get_mac_addr;
	ctrl->rx_manual               = &ftgmac030_rx_manual;
	ctrl->enter_power_down        = &ftgmac030_enter_power_down;
	ctrl->exit_power_down         = &ftgmac030_exit_power_down;
	ctrl->set_wakeup_mask         = &ftgmac030_set_wakeup_mask;
	ctrl->set_flow_control        = &ftgmac030_set_flow_control;
	ctrl->xmit_pause_frame        = &ftgmac030_xmit_pause_frame;
	ctrl->wait_pause_frame        = &ftgmac030_wait_pause_frame;
	ctrl->scan_phy_addr           = &ftgmac030_scan_phy_addr;
	ctrl->phywrite                = &phywrite16;
	ctrl->phyread                 = &phyread16;
	ctrl->mmdwrite                = &mmdwrite16;
	ctrl->mmdread                 = &mmdread16;
	ctrl->macwrite                = &iowrite32;
	ctrl->macread                 = &ioread32;
	ctrl->fill_txdes              = &ftgmac030_fill_txdes;
	ctrl->phy_loopback            = &ftgmac030_phy_loopback;

	ctrl->tx_queue[NP_TX_QUEUE].txdes_num = NPTX_QUEUE_ENTRIES;
	ctrl->tx_queue[NP_TX_QUEUE].kick_off  = &ftgmac030_kickoff_nptx; 
	ctrl->tx_queue[HP_TX_QUEUE].txdes_num = HPTX_QUEUE_ENTRIES;
	ctrl->tx_queue[HP_TX_QUEUE].kick_off  = &ftgmac030_kickoff_hptx; 
	ctrl->tx_queue_num         = NP_TX_QUEUE;
	ctrl->rxdes_num            = RX_QUEUE_ENTRIES;
	ctrl->wakeup_frame_mask[0] = 0xffffffff;
	ctrl->wakeup_frame_mask[1] = 0xffffffff;
	ctrl->wakeup_frame_mask[2] = 0xffffffff;
	ctrl->wakeup_frame_mask[3] = 0xffffffff;

	/* 0:MII or GMII, 1:RMII, 2: RGMII */
	mdc_cycthr = (mac_regs->gisr > 0) ? 0x68 : 0x34;
	//mdc_cycthr = (400 * 125) / 1000;

	/*ctrl->phyad = FTPHY_ADD;*/
	phyid = ftgmac030_scan_phy_addr(ctrl);
	if ((phyid & (~0xf)) == ((REALTEK_OUI << 10) | (RTL8211_MDL_NUM << 4))) {
		phy_type = PHY_TYPE_RTL8211EG;
	}
	else if ((phyid & (~0xf)) == ((REALTEK_OUI << 10) | (RTL8201_MDL_NUM << 4))) {
		phy_type = PHY_TYPE_RTL8201;
	}
	else if ((phyid & (~0xf)) == ((MARVELL_OUI << 10) | (MARVELL_MDL_88E1111 << 4))) {
		phy_type = PHY_TYPE_M88E1111;
	}
	else if ((phyid & (~0xf)) == ((MARVELL_OUI << 10) | (MARVELL_MDL_88E1116R << 4))) {
		phy_type = PHY_TYPE_M88E1116R;
	}
	else if ((phyid & (~0xf)) == ((ICPLUS_OUI << 10) | (ICPLUS_MDL_NUM << 4))) {
		phy_type = PHY_TYPE_IP101G;
	}
	else {
		prints("Warning: unsupport PHY, id 0x%lu, use generic phy driver\n", phyid);
		phy_type = PHY_TYPE_GENERIC;
	}

	prints("%s: Phy addr=%d mdc_cycthr=0x%x \n", __func__, ctrl->phyad, mdc_cycthr);

	ctrl->phy_processing      = phy_funcs[phy_type].phy_processing;
	ctrl->phy_auto_neo        = phy_funcs[phy_type].phy_auto_neo;
	ctrl->clear_phy_int       = phy_funcs[phy_type].clear_phy_int;
	ctrl->set_phy_int         = phy_funcs[phy_type].set_phy_int;
	ctrl->change_duplex_speed = phy_funcs[phy_type].change_duplex_speed;
	ctrl->show_link_status    = phy_funcs[phy_type].show_link_status;
	ctrl->is_link_up          = phy_funcs[phy_type].is_link_up;

	phy_funcs[phy_type].phy_info(ctrl);
	phy_funcs[phy_type].show_link_status(ctrl);

	return ctrl;
}

void free_ftgmac030_control(FTGMAC030_Control *ctrl)
{
	free((void *)ctrl);
}

uint32_t _32_bit_reverse(uint32_t value)
{
	uint32_t r_value = 0x0;
	int i;
	for (i = 31; i >= 0; i--) {
		r_value |= ((value & (1 << i)) >> i) << (31 - i);
	}
	return r_value;
}

uint8_t _8_bit_reverse(uint8_t value)
{
	uint8_t r_value = 0x0;
	int i;
	for (i = 7; i >= 0; i--) {
		r_value |= ((value & (1 << i)) >> i) << (7 - i);
	}
	return r_value;
}

unsigned int ftgmac030_crc32(char *s, int len)
{
	int perByte;
	int perBit;
	const uint32_t poly = 0x04C11DB7;
	/*const uint32_t poly = 0xDB710641;*/
	uint32_t crc_value = 0xFFFFFFFF;
	uint8_t c;
	for ( perByte = 0; perByte < len; perByte ++ ) {
		c = *(s++);
		for ( perBit = 0; perBit < 8; perBit++ ) {
			crc_value = (crc_value<<1)^
						((((crc_value>>31)^c)&0x01)?poly:0);
			c >>= 1;
		}
		/*prints("crc_value %d 0x%08x\n", perByte, crc_value);*/
	}
	return _32_bit_reverse(~crc_value);
}

int ftgmac030_addrcmp(const uint8_t *addr1, const uint8_t *addr2)
{
	int i;
	for (i = 0;i < 6;i++) {
		if (addr1[i] != addr2[i]) {
			return -1;
		}
	}
	return 0;
}

void ftgmac030_delaysecs(uint32_t i)
{
	/* convert argument seconds to microseconds */
	udelay(i * 1000000);
}

static void alloc_txdes(FTGMAC030_TX_QUEUE *txq, uint32_t top_addr)
{
	int i;
	FTGMAC030_TXDESC *tmp;

	/* use static address (16-byte alignment) */
	txq->txdes_top = 
	txq->txdes_cur = (FTGMAC030_TXDESC *)((uintptr_t)top_addr);

	/* initial nptxdes rings */
	memset(txq->txdes_top, 0, txq->txdes_num * sizeof(FTGMAC030_TXDESC));

	/* calculate last descriptor address */
	tmp = txq->txdes_top + (txq->txdes_num - 1);
	/* set last nptxdes flag */
	tmp->txdes0 |= FTGMAC030_TXDES0_EDOTR;
	tmp->txdes2 = (uint32_t)((uintptr_t) txq->txdes_top);

	/* initial txdes2 for next pointer */
	tmp = txq->txdes_top;
	for (i = 0; i < (txq->txdes_num - 1); i++) {
		tmp->txdes2 = (uint32_t)((uintptr_t)(tmp + 1));
		tmp = (FTGMAC030_TXDESC *)((uintptr_t)tmp->txdes2);
	}
}

void alloc_rxdes(FTGMAC030_Control *ctrl)
{
	int i;
	FTGMAC030_RXDESC *tmp;

	/* use static address (16-byte alignment) */
	ctrl->rxdes_top = (FTGMAC030_RXDESC *)RX_DESC_ADDR;
	ctrl->rxdes_cur = ctrl->rxdes_top;

	/* initial rxdes rings */
	memset(ctrl->rxdes_top, 0, ctrl->rxdes_num * sizeof(FTGMAC030_RXDESC));

	/* initial txdes2 for next pointer */
	tmp = ctrl->rxdes_top;
	for (i = 0; i < (ctrl->rxdes_num - 1); i++) {
		tmp->rxdes3 = RX_BUF_ADDR + (RX_BUF_SIZE * i);
		tmp->rxdes2 = (uint32_t)((uintptr_t)(tmp + 1));
		memset((void *)((uintptr_t)tmp->rxdes3), 0, RX_BUF_SIZE);

		/* For auto polling disable we must set descriptor own bit to software */
		if (FTGMAC030_APTC_RX_CNT(ctrl->aptc) == 0)
			tmp->rxdes0 |= FTGMAC030_RXDES0_RXPKT_RDY;

		tmp = (FTGMAC030_RXDESC *)((uintptr_t)tmp->rxdes2);
	}

	/* set last rxdes flag */
	tmp->rxdes0 |= FTGMAC030_RXDES0_EDORR;
	tmp->rxdes2 = (uint32_t)((uintptr_t)ctrl->rxdes_top);
	tmp->rxdes3 = RX_BUF_ADDR + (RX_BUF_SIZE * i);
	memset((void *)((uintptr_t)tmp->rxdes3), 0, RX_BUF_SIZE);
}

static void ftgmac030_open_control(FTGMAC030_Control *ctrl)
{
	/* set interrupt of phy */
	ctrl->set_phy_int(ctrl);
	ctrl->clear_phy_int(ctrl);

	/*reset_phy();
	phy_auto_neo();*/
	mac_regs->maccr = FTGMAC030_MACCR_SW_RST;
	/* wait for reset compelete */
	while(1 == (mac_regs->maccr & FTGMAC030_MACCR_SW_RST));

	/* 1. allocate system memory for the transmit/receive descriptor ring and buffer */
	alloc_txdes(&ctrl->tx_queue[NP_TX_QUEUE], NPTX_DESC_ADDR);
	alloc_txdes(&ctrl->tx_queue[HP_TX_QUEUE], HPTX_DESC_ADDR);
	alloc_rxdes(ctrl);

	/* 2. set acutal FIFO size to 16K */
	//mac_regs->tfafcr |= (0x1B << 24);
	/* 16K Tx and Rx FIFO size */
	ctrl->tfifo_rsize = 5;
	ctrl->rfifo_rsize = 5;

	if (initial_interrupt((void *) ctrl)) {
		prints("Connect Interrupt Error\n");
		return;
	}

	/* 3. set the normal and high priority transmit ring address register */
	mac_regs->nptxdesc_addr = (uint32_t)((uintptr_t)ctrl->tx_queue[NP_TX_QUEUE].txdes_top);
	mac_regs->hptxdesc_addr = (uint32_t)((uintptr_t)ctrl->tx_queue[HP_TX_QUEUE].txdes_top);

	/* 4. set the receive ring address register */
	mac_regs->rxdesc_addr = (uint32_t)((uintptr_t)ctrl->rxdes_top);
	mac_regs->rbsr = RX_BUF_SIZE;

	/* 5. set interrupt enable register */
	mac_regs->ier = FTGMAC030_INT_DEFAULT;

	/* 6. set mac address */
	mac_regs->mac_ladr = (uint32_t)(*(uint32_t *)mac_address) & 0xffffffff;
	mac_regs->mac_madr = (uint32_t)(*(uint32_t *)(mac_address + 4)) & 0xffff;

	/* 7. set interrupt timer control */
	mac_regs->txitc = FTGMAC030_TXITC_DEFAULT;
	mac_regs->rxitc = FTGMAC030_RXITC_DEFAULT;
	/* 8. set auto polling timer control */
	mac_regs->aptc = ctrl->aptc;
	/* 9. set dma burst */
	//mac_regs->dblac = FTGMAC030_DBLAC_DEFAULT |
	//		  FTGMAC030_DBLAC_RXDES_SIZE((sizeof(FTGMAC030_RXDESC) >> 3)) |
	//		  FTGMAC030_DBLAC_TXDES_SIZE((sizeof(FTGMAC030_TXDESC) >> 3));

	/* 10. Set PTP increment value */
	/* Ask HW for source clock frequency. It is 50 MHz
	 * at time FPGA verified, so period is 20 ns.
	 */
	//mac_regs->ptp_ns_period = 40;
	//mac_regs->ptp_nns_period = 0;

	/* 11. enable txdma */
	mac_regs->maccr = ctrl->maccr;

	ctrl->phy_processing(ctrl);
}

void ftgmac030_open(FTGMAC030_Control *ctrl, void (*cmpl_isr)(FTGMAC030_Control *))
{
	ctrl->maccr = FTGMAC030_MACCR_DEFAULT;
	ctrl->aptc = FTGMAC030_APTC_DEFAULT;
	ctrl->cmpl_isr = cmpl_isr;

	ftgmac030_open_control(ctrl);
}

void ftgmac030_open_maccr(FTGMAC030_Control *ctrl, uint32_t maccr, void (*cmpl_isr)(FTGMAC030_Control *))
{
	ctrl->maccr = maccr;
	ctrl->aptc = FTGMAC030_APTC_DEFAULT;
	ctrl->cmpl_isr = cmpl_isr;

	ftgmac030_open_control(ctrl);
}

void ftgmac030_open_aptc(FTGMAC030_Control *ctrl, uint32_t aptc, void (*cmpl_isr)(FTGMAC030_Control *))
{
	ctrl->maccr = FTGMAC030_MACCR_DEFAULT;
	ctrl->aptc = aptc;
	ctrl->cmpl_isr = cmpl_isr;

	ftgmac030_open_control(ctrl);
}

void ftgmac030_fill_txdes(FTGMAC030_TX_QUEUE *txq, const uint32_t phy_tx_addr, uint32_t size)
{
	FTGMAC030_TXDESC *txdes;

	txdes = txq->txdes_cur;

	/* in this function, tx is always completed by one descriptor */
	if(txdes->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN){
		prints("??? => tx descriptor 0x%x own by HW\n", (uintptr_t)txdes);
		return;
	}
	/* initial txdes */
	txdes->txdes3 = phy_tx_addr;
	txdes->txdes1 = FTGMAC030_TXDES1_DEFAULT;
	/* reset txdes0 */
	txdes->txdes0 &= FTGMAC030_TXDES0_EDOTR;
	txdes->txdes0 |= FTGMAC030_TXDES0_BUF_SIZE(size) |
			 FTGMAC030_TXDES0_FTS |
			 FTGMAC030_TXDES0_LTS |
			 FTGMAC030_TXDES0_TXDMA_OWN;

	txq->txdes_cur = (FTGMAC030_TXDESC *)((uintptr_t)txdes->txdes2);
}

void ftgmac030_xmit_one_desc(FTGMAC030_TX_QUEUE *txq, const uint32_t phy_tx_addr,
			     uint32_t size, uint32_t txdes1)
{
	FTGMAC030_TXDESC *txdes;

	txdes = txq->txdes_cur;

	/* in this function, tx is always completed by one descriptor */
	if(txdes->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN){
		prints("??? => tx descriptor 0x%x own by HW\n", (uintptr_t)txdes);
		return;
	}

	/* initial txdes */
	txdes->txdes3 = phy_tx_addr;
	txdes->txdes1 = txdes1;
	/* reset txdes0 */
	txdes->txdes0 &= FTGMAC030_TXDES0_EDOTR;
	txdes->txdes0 |= FTGMAC030_TXDES0_BUF_SIZE(size) |
			 FTGMAC030_TXDES0_FTS |
			 FTGMAC030_TXDES0_LTS |
			 FTGMAC030_TXDES0_TXDMA_OWN;

	txq->txdes_cur = (FTGMAC030_TXDESC *)((uintptr_t)txdes->txdes2);

	txq->kick_off();
}

void ftgmac030_xmit(FTGMAC030_Control *ctrl, const uint32_t phy_tx_addr, uint32_t size,
		    uint32_t txdes1)
{
	ftgmac030_xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num], phy_tx_addr,
				size, txdes1);
}

void ftgmac030_xmit_multi_desc(FTGMAC030_TX_QUEUE *txq, const uint32_t number, struct multi_desc txdeses[],
			       uint32_t txdes1)
{
	FTGMAC030_TXDESC *tmp = txq->txdes_cur;
	uint32_t i;

	if (number <= 0) {
		prints("??? => number of description is less than zero\n");
		return;
	} else if (1 == number) {
		ftgmac030_xmit_one_desc(txq, txdeses[0].phy_tx_addr,
					txdeses[0].size, FTGMAC030_TXDES1_DEFAULT);
	} else {
		for (i = 0; i < number; i++) {
			if(tmp->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN){
				prints("??? => tx descriptor 0x%x own by HW\n", (int) tmp->txdes0);
				return;
			}

			tmp->txdes3 = txdeses[i].phy_tx_addr;
			/* reset txdes1 (valid only when FTS = 1) */
			tmp->txdes1 = txdes1;
			/* reset txdes0 */
			tmp->txdes0 &= FTGMAC030_TXDES0_EDOTR;
			tmp->txdes0 |= FTGMAC030_TXDES0_TXDMA_OWN | FTGMAC030_TXDES0_BUF_SIZE(txdeses[i].size);

			/* First descriptor */
			if (i == 0) {
				tmp->txdes1 |= FTGMAC030_TXDES1_TX2FIC | FTGMAC030_TXDES1_TXIC;
				tmp->txdes0 |= FTGMAC030_TXDES0_FTS;
			} else if (i == (number - 1)) /* Last descriptor */
				tmp->txdes0 |= FTGMAC030_TXDES0_LTS;

			tmp = (FTGMAC030_TXDESC *)((uintptr_t)tmp->txdes2);
		}

		txq->txdes_cur = tmp;

		txq->kick_off();
	}
}

void ftgmac030_kickoff_nptx(void)
{
	#ifdef PLATFORM_A369
		FA626_CPUCleanInvalidateDCacheAll();
	#endif

	if (0 == (mac_regs->aptc & FTGMAC030_APTC_TX_CNT(0xf)))
		/* kick off normal priority txdma */
		mac_regs->nptxpd = 0xffffffff;
}

void ftgmac030_kickoff_hptx(void)
{
	#ifdef PLATFORM_A369
		FA626_CPUCleanInvalidateDCacheAll();
	#endif

	if (0 == (mac_regs->aptc & FTGMAC030_APTC_TX_CNT(0xf)))
		/* kick off normal priority txdma */
		mac_regs->hptxpd = 0xffffffff;
}

void ftgmac030_phy_loopback(void)
{
}

void ftgmac030_half_dup_10(FTGMAC030_Control *ctrl)
{
	ctrl->change_duplex_speed(ctrl, HALF, _10);
}

void ftgmac030_half_dup_100(FTGMAC030_Control *ctrl)
{
	ctrl->change_duplex_speed(ctrl, HALF, _100);
}

void ftgmac030_half_dup_1000(FTGMAC030_Control *ctrl)
{
	ctrl->change_duplex_speed(ctrl, HALF, _1000);
}

void ftgmac030_full_dup_10(FTGMAC030_Control *ctrl)
{
	ctrl->change_duplex_speed(ctrl, FULL, _10);
}

void ftgmac030_full_dup_100(FTGMAC030_Control *ctrl)
{
	ctrl->change_duplex_speed(ctrl, FULL, _100);
}

void ftgmac030_full_dup_1000(FTGMAC030_Control *ctrl)
{
	ctrl->change_duplex_speed(ctrl, FULL, _1000);
}

static uint8_t ftgmac030_multicast_ht_bit(uint8_t *mac_addr)
{
	int perByte;
	int perBit;
	const uint32_t poly = 0x04C11DB7;
	//uint8_t convert_addr[6];
	uint32_t crc_value = 0xFFFFFFFF;
	uint8_t c;

	/* convert mac addr */
	/*convert_addr[0] = _8_bit_reverse(mac_addr[0]);
	convert_addr[1] = _8_bit_reverse(mac_addr[1]);
	convert_addr[2] = _8_bit_reverse(mac_addr[2]);
	convert_addr[3] = _8_bit_reverse(mac_addr[3]);
	convert_addr[4] = _8_bit_reverse(mac_addr[4]);
	convert_addr[5] = (convert_addr[4] << 1) | (mac_addr[5] & 0x1);*/

	for ( perByte = 0; perByte < 6; perByte ++ ) {
		c = *(mac_addr++);
		//c = convert_addr[perByte];
		for ( perBit = 0; perBit < 8; perBit++ ) {
			crc_value = (crc_value<<1)^
						((((crc_value>>31)^c)&0x01)?poly:0);
			c >>= 1;
		}
	}

	/* covert CRC */
	crc_value = ~crc_value;
	crc_value = _8_bit_reverse(crc_value & 0xff) | 
				(_8_bit_reverse((crc_value >> 8) & 0xff) << 8) |
				(_8_bit_reverse((crc_value >> 16) & 0xff) << 16) |
				(_8_bit_reverse((crc_value >> 24) & 0xff) << 24);

	//crc_value = (((crc_value & 0xf) << 28) | (((crc_value >> 4) & 0xf) << 24));
	/* return MSB 6 bits */
	crc_value = crc_value >> 26;
	//crc_value = crc_value << 1;
	return ((uint8_t)crc_value);
}

uint8_t ftgmac030_set_multicast_address(FTGMAC030_Control *ctrl, uint8_t *mac_addr)
{
	/*
	 * 1. generate crc-32 and get the 6 bits (most significant)
	 * 2. set the registers
	 * 3. wrtie backup the bit to the hashtable0/hashtable1
	 */
	uint8_t crc_6bit = 0, crc_6bit_reserve = 0; /* crc value with 6 bits */
	uint32_t table_num = 0;
	uint32_t set_bit = 0;

	crc_6bit = ftgmac030_multicast_ht_bit(mac_addr);
	crc_6bit_reserve = crc_6bit;

	if(crc_6bit > 31){
		table_num = 1;
		crc_6bit -= 32;
	} else {
		table_num = 0;
	}

	set_bit = (1 << crc_6bit);
	if(0 == table_num)
		mac_regs->maht0 = mac_regs->maht0 | set_bit;
	else
		mac_regs->maht1 = mac_regs->maht1 | set_bit;

	prints("multicast index: 0x%02x maht0 0x%08x maht1 0x%08x\n", crc_6bit_reserve,
		mac_regs->maht0, mac_regs->maht1);

	return crc_6bit_reserve;
}

uint8_t *ftgmac030_get_mac_addr()
{
	return (uint8_t *) mac_address;
}

void ftgmac030_rx_manual(void)
{
	/* kick off rxdma */
	mac_regs->rxpd = 0xffffffff;
}

uint32_t ftgmac030_wakeup_crc32(uint8_t *wakeup_frame, uint32_t size, uint32_t *mask)
{
	int perByte;
	int perBit;
	const uint32_t poly = 0x04C11DB7;
	uint32_t crc_value = 0xFFFFFFFF;
	uint8_t c;
	for ( perByte = 0; perByte < size; perByte ++ ){
		if(perByte >= 128){
			break;
		}

		if((1 << (perByte % 32)) & mask[perByte >> 5]){
			c = *(wakeup_frame++);
			for ( perBit = 0; perBit < 8; perBit++ ){
				crc_value = (crc_value<<1)^
							((((crc_value>>31)^c)&0x01)?poly:0);
				c >>= 1;
			}
		}
		else{
			wakeup_frame++;
		}
	}

	/* covert CRC */
	crc_value = ~crc_value;
	crc_value = _8_bit_reverse(crc_value & 0xff) | 
				(_8_bit_reverse((crc_value >> 8) & 0xff) << 8) |
				(_8_bit_reverse((crc_value >> 16) & 0xff) << 16) |
				(_8_bit_reverse((crc_value >> 24) & 0xff) << 24);

	return (crc_value);
}

void ftgmac030_enter_power_down(FTGMAC030_Control *ctrl, uint32_t power_down_setting,
				uint8_t *wakeup_frame, uint32_t wakeup_frame_size)
{
	/* 1.  set TXDMA_EN (50h.0) = 0 */
	mac_regs->maccr = (mac_regs->maccr & ~FTGMAC030_MACCR_TXDMA);
	/* 2.  poll the dma/fifo state register (3Ch) to wait for tx fifo empty*/
	while (0 == (mac_regs->dmafifos & FTGMAC030_DMAFIFOS_TXFIFO_EMPTY));
	/* 3.  set TXMAC_EN (50h.2) = 0 and RXMAC_EN (50h.3) to terminate the tx/rx */
	mac_regs->maccr = (mac_regs->maccr & ~(FTGMAC030_MACCR_TXMAC | FTGMAC030_MACCR_RXMAC));
	/* 4.  poll the dma/fifo state register (3Ch) to wait for rx fifo empty */
	while(0 == (mac_regs->dmafifos & FTGMAC030_DMAFIFOS_RXFIFO_EMPTY));
	/* 5.  set RXDMA_EN (50h.1) = 0 to stop reception */
	mac_regs->maccr = (mac_regs->maccr & ~FTGMAC030_MACCR_RXDMA);

	/* 7.  program the wake-up frame */
	power_down_setting &= ~FTGMAC030_WOLCR_WAKEUP_SEL(3); /* This is FTGMAC030_WOLCR_WAKEUP1 */
	mac_regs->wakeOnLan_cntrl &= ~FTGMAC030_WOLCR_WAKEUP_SEL(3);
	if (power_down_setting & FTGMAC030_WOLCR_WAKEUP2) {
		mac_regs->wakeOnLan_cntrl |= FTGMAC030_WOLCR_WAKEUP_SEL(1);
		power_down_setting |= FTGMAC030_WOLCR_WAKEUP_SEL(1);
	} else if (power_down_setting & FTGMAC030_WOLCR_WAKEUP3){
		mac_regs->wakeOnLan_cntrl |= FTGMAC030_WOLCR_WAKEUP_SEL(2);
		power_down_setting |= FTGMAC030_WOLCR_WAKEUP_SEL(2);
	} else if (power_down_setting & FTGMAC030_WOLCR_WAKEUP4) {
		mac_regs->wakeOnLan_cntrl |= FTGMAC030_WOLCR_WAKEUP_SEL(3);
		power_down_setting |= FTGMAC030_WOLCR_WAKEUP_SEL(3);
	}

	/* write mask */
	mac_regs->wfbm1 = ctrl->wakeup_frame_mask[0];
	mac_regs->wfbm2 = ctrl->wakeup_frame_mask[1];
	mac_regs->wfbm3 = ctrl->wakeup_frame_mask[2];
	mac_regs->wfbm4 = ctrl->wakeup_frame_mask[3];
	/* calculate crc value */
	mac_regs->wfc_crc = ftgmac030_wakeup_crc32(wakeup_frame, wakeup_frame_size,
						   ctrl->wakeup_frame_mask);
	/* 8. write 32'hFFFF_FFFF to clear wake-on-lan status register (74h) */
	mac_regs->wakeOnLan_sts = 0xffffffff;
	/* 9. program the requested wake-up events and 
	 *    power state into wake-on-lan register (70h) 
	 *    to let FTMAC110 go into the power saving mode
	 */
	mac_regs->wakeOnLan_cntrl = power_down_setting;
	/* 10. set RXMAC_EN (50h.3) = 1 to enable the reception (RCV_ALL has to be enable, too) */
	mac_regs->maccr = (mac_regs->maccr | FTGMAC030_MACCR_RXMAC | FTGMAC030_MACCR_ALLADDR);
}

void ftgmac030_exit_power_down(void)
{
	/* 1. wait for the occurrence of wake-up events */
	while(0 == mac_regs->wakeOnLan_sts);
	/* 2. set RXMAC_EN (50h.3) = 0 to terminate the reception */
	mac_regs->maccr = (mac_regs->maccr & ~FTGMAC030_MACCR_RXMAC);
	/* 3. read the Wake-On-LAN status register (74h) to check which wake-up event happened */

	/* 4. program the Wake-On-LAN register (70h) to let FTMAC110 exit the power-saving mode */
	mac_regs->wakeOnLan_cntrl = mac_regs->wakeOnLan_cntrl & ~0x7f;
	/* 5. set SW_RST (50h.31) = 1 to reset ftgmac030 */
	mac_regs->maccr = FTGMAC030_MACCR_SW_RST;
	/* 6. check SW_RST (50h.31) = 0 to make sure that FTMAC110 has finished reset */
	while(mac_regs->maccr & FTGMAC030_MACCR_SW_RST);
	/* 7. re-initialize FTMAC110 to transmit and receive packets */
}

void ftgmac030_set_wakeup_mask(FTGMAC030_Control *ctrl, uint32_t mask_value, uint32_t mask_index)
{
	if ((mask_index >= 1) && (mask_index <= 4)) {
		ctrl->wakeup_frame_mask[mask_index - 1] = mask_value;
	}
	else {
		prints("mask num must in [1,4]\n");
	}
}

void ftgmac030_set_flow_control(uint16_t pause_time, uint8_t low_thershold, uint8_t high_threshold,
				uint8_t fc_thr_en, uint8_t fc_en)
{
	mac_regs->flow_cntrl &= ~(FTGMAC030_FCR_FCTHR_EN | FTGMAC030_FCR_FC_EN);

	/* Set Pause time */
	mac_regs->flow_cntrl &= ~FTGMAC030_FCR_PAUSE_TIME(0xffff);
	mac_regs->flow_cntrl |= FTGMAC030_FCR_PAUSE_TIME(pause_time);

	if (fc_thr_en) {
		/* Set High threshold */
		mac_regs->flow_cntrl |= FTGMAC030_FCR_HTHR;
		mac_regs->flow_cntrl &= ~FTGMAC030_FCR_FC_H_L(0x7f);
		mac_regs->flow_cntrl |= FTGMAC030_FCR_FC_H_L(high_threshold);

		/* Set Low threshold */
		mac_regs->flow_cntrl &= ~FTGMAC030_FCR_HTHR;
		mac_regs->flow_cntrl &= ~FTGMAC030_FCR_FC_H_L(0x7f);
		mac_regs->flow_cntrl |= FTGMAC030_FCR_FC_H_L(low_thershold);

		/* Enable threshold mode */
		mac_regs->flow_cntrl |= FTGMAC030_FCR_FCTHR_EN;
	}

	if (fc_en)
		mac_regs->flow_cntrl |= FTGMAC030_FCR_FC_EN;
}

void ftgmac030_xmit_pause_frame(void)
{
	mac_regs->flow_cntrl = mac_regs->flow_cntrl | FTGMAC030_FCR_TX_PAUSE;
}

uint32_t ftgmac030_wait_pause_frame(void)
{
	if (mac_regs->flow_cntrl & FTGMAC030_FCR_RX_PAUSE) {
		mac_regs->flow_cntrl |= FTGMAC030_FCR_RX_PAUSE;
		return 1;
	} else
		return 0;
}


void ftgmac030_isr(void * data)
{
	FTGMAC030_Control *ctrl = (FTGMAC030_Control *) data;
	uint32_t int_status;

	/* disable all interrupts for pollings */
	mac_regs->ier = 0;
	int_status = mac_regs->isr;
	/* write 1 clear to reset all interrupts */
	mac_regs->isr = int_status;
	ctrl->clear_phy_int(ctrl);

	if (int_status & FTGMAC030_INT_HPTXBUF_UNAVA) {
		hptxbuf_unava_handler();
	}

	if (int_status & FTGMAC030_INT_PHYSTS_CHG) {
		physts_chg_handler(ctrl);
	}

	if (int_status & FTGMAC030_INT_AHB_ERR) {
		ahb_err_handler();
	}

	if (int_status & FTGMAC030_INT_TPKT_LOST) {
		tpkt_lost_handler();
	}

	if (int_status & FTGMAC030_INT_NPTXBUF_UNAVA) {
		nptxbuf_unava_handler();
	}

	if (int_status & FTGMAC030_INT_TPKT2E) {
		tpkt2e_handler(ctrl);
	}

	if (int_status & FTGMAC030_INT_RPKT_LOST) {
		rpkt_lost_handler();
	}

	if (int_status & FTGMAC030_INT_RXBUF_UNAVA) {
		rxbuf_unava_handler();
	}

	if (int_status & FTGMAC030_INT_RPKT2F) {
		rpkt2f_handler();
	}

	if (int_status & FTGMAC030_INT_RPKT2B) {
		/*
		 * Note.
		 * If two packets or more arrived at the almost same time,
		 * ftgmac030 will trigger interrupt only once.
		 * So we need to handle all received packets in this interrupt.
		 */
		ctrl->rx_packet_cnt = 0;
		do {
			ctrl->rx_packet_cnt++;
			rpkt2b_handler(ctrl);
		} while (ctrl->rxdes_cur->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY);
	}

	/* enable interrupt */
	mac_regs->ier = FTGMAC030_INT_DEFAULT;
}

void hptxbuf_unava_handler(void)
{
	/* prints("hptxbuf_unava_handler in isr\n"); */
}

void physts_chg_handler(FTGMAC030_Control *ctrl)
{
	prints("Link status changed!\n");
	ctrl->phy_processing(ctrl);
}

void ahb_err_handler(void)
{
	prints("ahb_err_handler in isr\n");
}

void tpkt_lost_handler(void)
{
	prints("tpkt_lost_handler in isr\n");
}

void nptxbuf_unava_handler(void)
{
	/*prints("nptxbuf_unava_handler in isr\n");*/
}

void tpkt2e_handler(FTGMAC030_Control *ctrl)
{
	ctrl->cmpl_isr(ctrl);
}

void rpkt_lost_handler(void)
{
	prints("rpkt_lost_handler in isr\n");
}

void rxbuf_unava_handler(void)
{
}

void rpkt2f_handler(void)
{
}

void rpkt2b_handler(FTGMAC030_Control *ctrl)
{
	FTGMAC030_RXDESC *rxdes;
	uint32_t rxbuf_size, copy_size = 0;
	uint8_t  *copy_buf = NULL;

	/* initialize copy receive data buffer */
	memset(ctrl->rx_buff, 0, sizeof(ctrl->rx_buff));
	ctrl->rx_len = 0;
	ctrl->rx_err_happen = 0;

	ctrl->rxdes0 = ctrl->rxdes_cur->rxdes0;
	ctrl->rxdes1 = ctrl->rxdes_cur->rxdes1;

	rxdes = ctrl->rxdes_cur;

	if (rxdes->rxdes0 & FTGMAC030_RXDES0_FRS) {

		/* check rx packet */
		if (rxdes->rxdes0 & FTGMAC030_RXDES0_FIFO_FULL) {
			prints("??? => receive fifo full\n");
			ctrl->rx_err_happen = 1;
		}

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_RX_ODD_NB) {
			prints("??? => receive odd nibbles\n");
			ctrl->rx_err_happen = 1;
		}

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_CRC_ERR) {
			prints("??? => crc error\n");
			ctrl->rx_err_happen = 1;
		}

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_RX_ERR) {
			prints("??? => receive error\n");
			ctrl->rx_err_happen = 1;
		}

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_FTL) {
			prints("??? => frame too long\n");
			ctrl->rx_err_happen = 1;
		}

		if (rxdes->rxdes1 & FTGMAC030_RXDES1_IPCS_FAIL) {
			prints("??? => ip checksum error\n");
			ctrl->rx_err_happen = 1;
		}

		if (rxdes->rxdes1 & FTGMAC030_RXDES1_UDPCS_FAIL) {
			prints("??? => udp checksum error\n");
			ctrl->rx_err_happen = 1;
		}

		if (rxdes->rxdes1 & FTGMAC030_RXDES1_TCPCS_FAIL) {
			prints("??? => tcp checksum error\n");
			ctrl->rx_err_happen = 1;
		}

		if (ctrl->rx_err_happen) {
			ctrl->rx_len = FTGMAC030_RXDES0_VDBC(ctrl->rxdes_cur->rxdes0);
			/* call back */
			ctrl->cmpl_isr(ctrl);
			/* release own of this rxdes */
			rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
			rxdes->rxdes1 = 0x0;
			ctrl->rxdes_cur = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);
			return;
		}
	}
	else {
		/* check the valid interrupt */
		if (mac_regs->rxr_ptr == (unsigned int)((uintptr_t)ctrl->rxdes_cur)) {
			/* skip this interrupt */
			return;
		}
		/* check the first descriptor */
		prints("??? => not the first receive segment descriptor\n");
		/* release own of this rxdes */
		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
		rxdes->rxdes1 = 0x0;
		ctrl->rxdes_cur = (FTGMAC030_RXDESC *)((uintptr_t)ctrl->rxdes_cur->rxdes2);
		return;
	}

	rxbuf_size = mac_regs->rbsr;
	copy_buf = ctrl->rx_buff;
	do {
		/* wait for rx_dma completes the frame reception */
		while(!(rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY));
		copy_size = FTGMAC030_RXDES0_VDBC(rxdes->rxdes0);

		if (copy_size > rxbuf_size) {
			prints("??? => valid bytes %lu > rx buf size %lu\n", copy_size, rxbuf_size);
			return;
		}
		memcpy((void *)((uintptr_t)copy_buf),
		       (void *)((uintptr_t)rxdes->rxdes3), copy_size);
		copy_buf += copy_size;
		ctrl->rx_len += copy_size;

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_LRS) {
			/* release own of this rxdes */
			rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
			rxdes->rxdes1 = 0x0;
			/* clear rx buffer */
			//memset((void *)((uintptr_t)rxdes->rxdes3), 0, ctrl->rx_len);
			//prints("Rx buf address 0x%x\n", rxdes->rxdes3);
			ctrl->rxdes_cur = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);
			break;
		}

		/* release own of this rxdes */
		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
		rxdes->rxdes1 = 0x0;
		/* clear rx buffer */
		memset((void *)((uintptr_t)rxdes->rxdes3), 0, ctrl->rx_len);
		ctrl->rxdes_cur = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);
		rxdes = ctrl->rxdes_cur;
	} while (1);

	/* call back */
	ctrl->cmpl_isr(ctrl);
}

int initial_interrupt(void * data)
{
#if 0
	XScuGic_Config *GicConfig;
	int result;
#endif
#ifdef CONFIG_PLATFORM_A369
	fLib_CloseInt(IRQ_IAmIRQ, IRQ_ftgmac030);
	fLib_SetIntTrig(IRQ_IAmIRQ, IRQ_ftgmac030,H_ACTIVE,LEVEL);
	fLib_ConnectInt(IRQ_IAmIRQ, IRQ_ftgmac030, (PrHandler)ftgmac030_isr);
	fLib_EnableInt(IRQ_IAmIRQ, IRQ_ftgmac030);
#elif defined CONFIG_PLATFORM_AXI || defined CONFIG_PLATFORM_AHB
	irq_set_type(FTGMAC030_IRQ, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler (FTGMAC030_IRQ, ftgmac030_isr, (void *) data);
	irq_set_enable(FTGMAC030_IRQ);
	enable_interrupts(); /* Clear CPU I bit */
#elif defined CONFIG_BOARD_A320
	fLib_CloseInt(IRQ_ftgmac030);
	fLib_SetIntTrig(IRQ_ftgmac030,LEVEL,H_ACTIVE);
	fLib_ConnectInt(IRQ_ftgmac030, ftgmac030_isr);
	fLib_EnableInt(IRQ_ftgmac030);
#else
#if 0
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if (GicConfig == NULL) {
		return -1;
	}

	/* Initialize the SCU and GIC to enable the desired interrupt
	 * configuration.
	 */
	result = XScuGic_CfgInitialize(&IntcInstance, GicConfig,
									GicConfig->CpuBaseAddress);
	if (result != 0) {
		return -1;
	}

	// 0x1: active High level sensitive; 0x3: rising-edge sensitive
	XScuGic_SetPriorityTriggerType(&IntcInstance, IRQ_FTGMAC030, 0, 0x1);

	/* Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device. */
	result = XScuGic_Connect(&IntcInstance, IRQ_FTGMAC030, (Xil_InterruptHandler) ftgmac030_isr, 0);
	if (result != 0) {
		return result;
	}

	XScuGic_Enable(&IntcInstance, IRQ_FTGMAC030);
#endif
#endif

	return 0;
}

void reset_phy(FTGMAC030_Control *ctrl)
{
	uint16_t tmp16;
	int i;
	tmp16 = phyread16(ctrl, MII_REG_BMCR);
	tmp16 |= MII_BMCR_RESET;
	phywrite16(ctrl, tmp16, MII_REG_BMCR);

	do {
		for (i = 0; i < 100000; i++) {};
		tmp16 = phyread16(ctrl, MII_REG_BMCR);
	} while ((tmp16 & MII_BMCR_RESET) > 0) ;

	/* wait link status */
	/*do {
		for (i = 0; i < 100000; i++) {};
		tmp16 = phyread16(ctrl, M1111_REG_SSR);
	} while ((tmp16 & M1111_SSR_LINK) == 0) ;*/
}

uint16_t phyread16(FTGMAC030_Control *ctrl, uint16_t regad)
{
	mac_regs->phycr = (FTGMAC030_PHYCR_PHYRD | FTGMAC030_PHYCR_REGAD(regad) |
			   FTGMAC030_PHYCR_PHYAD(ctrl->phyad) | FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_RD) |
			   FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF) | mdc_cycthr);

	/* wait for read complete */
	while (mac_regs->phycr & FTGMAC030_PHYCR_PHYRD);

	return FTGMAC030_PHYDATA_MIIRDATA(mac_regs->phydata);
}

void phywrite16(FTGMAC030_Control *ctrl, uint16_t value, uint16_t regad)
{
	/* write data to PHYWDATA (0x64h) */
	mac_regs->phydata = FTGMAC030_PHYDATA_MIIWDATA(value);
	/* wait */
	while(mac_regs->phycr & FTGMAC030_PHYCR_PHYWR);

	/* fille out PHYCR */
	mac_regs->phycr = (FTGMAC030_PHYCR_PHYWR | FTGMAC030_PHYCR_REGAD(regad) |
			   FTGMAC030_PHYCR_PHYAD(ctrl->phyad) | FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_WR) |
			   FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF) | mdc_cycthr); 
	/* wait */
	while(mac_regs->phycr & FTGMAC030_PHYCR_PHYWR);
}

uint16_t mmdread16(FTGMAC030_Control *ctrl, uint16_t dev_addr, uint16_t reg_addr)
{
	phywrite16(ctrl, MII_MMD_REGCR_MODE(MII_MMD_MODE_ADDR) | MII_MMD_REGCR_DEVAD(dev_addr), MII_REG_MMD_REGCR);
	phywrite16(ctrl, MII_MMD_ADDAR_DATA(reg_addr), MII_REG_MMD_ADDAR);
	phywrite16(ctrl, MII_MMD_REGCR_MODE(MII_MMD_MODE_NO_POST) | MII_MMD_REGCR_DEVAD(dev_addr), MII_REG_MMD_REGCR);
	return phyread16(ctrl, MII_REG_MMD_ADDAR);
}

void mmdwrite16(FTGMAC030_Control *ctrl, uint16_t value, uint16_t dev_addr, uint16_t reg_addr)
{
	phywrite16(ctrl, MII_MMD_REGCR_MODE(MII_MMD_MODE_ADDR) | MII_MMD_REGCR_DEVAD(dev_addr), MII_REG_MMD_REGCR);
	phywrite16(ctrl, MII_MMD_ADDAR_DATA(reg_addr), MII_REG_MMD_ADDAR);
	phywrite16(ctrl, MII_MMD_REGCR_MODE(MII_MMD_MODE_NO_POST) | MII_MMD_REGCR_DEVAD(dev_addr), MII_REG_MMD_REGCR);
	phywrite16(ctrl, MII_MMD_ADDAR_DATA(value), MII_REG_MMD_ADDAR);
}

static uint32_t ftgmac030_get_phy_id(FTGMAC030_Control *ctrl)
{
	const uint16_t id1 = phyread16(ctrl, MII_REG_ID1);
	const uint16_t id2 = phyread16(ctrl, MII_REG_ID2);
	return ((id1 << 16) | id2);
}

uint32_t ftgmac030_scan_phy_addr(FTGMAC030_Control *ctrl)
{
	/* for rtl8211, no scan from zero */
	int i;
	uint32_t phyid;

	for (i = 0;i < PHY_MAX_ADDR;i++) {
		/* If the phy_id is mostly Fs, there is no device there */
		ctrl->phyad = i;
		phyid = ftgmac030_get_phy_id(ctrl);

		if ((phyid & 0x1fffffff) == 0x1fffffff)
			continue;
		else
			return phyid;
	}

	/* Not valid phyid */
	return 0x1fffffff;
}

void ftgmac030_dump_reg(FTGMAC030_Control *ctrl)
{
	int i;
	int *ptr;
	FTGMAC030_TX_QUEUE *txq;

	prints("\n Registers value:  ");
	for (i = 0; i < 0x100; i+= 4) {

		if ((i & 0xf) == 0)
			prints("\n %03x:  ", i);

		prints("%08lx  ", ioread32(FTGMAC030_REG_BASE + i));
	}

	ptr = (int *) ctrl->rxdes_cur;
	prints("\n Rx Descriptor:  0x%x\n", (unsigned int)((uintptr_t)ptr));
	for (i = 0; i < 4; i++)
		prints("%08x  ", *(ptr + i));

	ptr = (int *) *(ptr + 3);
	prints("\n Rx Buff addr:  0x%x\n", (unsigned int)((uintptr_t) ptr));
		for (i = 0; i < 4; i++)
			prints("%08x  ", *(ptr + i));


	txq = &ctrl->tx_queue[NP_TX_QUEUE];
	ptr = (int *) txq->txdes_cur;
	prints("\n Tx Descriptor - N:  0x%x\n", (unsigned int)((uintptr_t)ptr));
	for (i = 0; i < 4; i++)
		prints("%08x  ", *(ptr + i));

	txq = &ctrl->tx_queue[HP_TX_QUEUE];
	ptr = (int *) txq->txdes_cur;
	prints("\n Tx Descriptor - H:  0x%x\n", (unsigned int)((uintptr_t)ptr));
	for (i = 0; i < 4; i++)
		prints("%08x  ", *(ptr + i));
}
