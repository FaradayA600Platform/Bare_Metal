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
#include "skb_buff.h"
#include "net.h"
#include "ptp.h"

/**
 * Index 0xa = 0x0111
 *  -   0x01 : Hop Limit
 *  -   0x11 : UDP Next Header
 * Index 0x1B, 0x1C = 0x4001
 *  -   0x0140 : UDP Source port, ptp-general(320)
 * Index 0x1D = 0x4a00
 *  -   74 : UDP Length
 * Index 0x1E = 0xb603
 * -  0x03b6 : UDP Checksum correct
 */
uint16_t announce[] = {
		   0x3333, 0x0000, 0x8101, 0x75fe, 0x9510, 0x2811, 0xdd86, 0x0060,
		   0x0000, 0x4a00, 0x0111, 0x80fe, 0x0000, 0x0000, 0x0000, 0x75fc,
		   0xff10, 0x95fe, 0x2811, 0x0eff, 0x0000, 0x0000, 0x0000, 0x0000,
		   0x0000, 0x0000, 0x8101, 0x4001, 0x4001, 0x4a00, 0xb603, 0x020b,
		   0x4000, 0x0000, 0x0800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		   0x0000, 0x75fe, 0xff10, 0x95fe, 0x2811, 0x0100, 0x0000, 0x0105,
		   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2300, 0x8000, 0xfef8,
		   0xffff, 0xfe80, 0x1075, 0xfeff, 0x1195, 0x0028, 0xa000, 0x0000,
		  };

/**
 * Index 0xa = 0x0100
 *  -   0x01 : Hop Limit
 *  -   0x00 : IPv6 hop-by-hop option Next Header
 * Index 0x1B = 0x003a
 *  -   0x00 : Length, 0 is 8 bytes
 *  -   0x3a : ICMPv6 Next Header
 */
uint16_t icmpv6[] = {
		  0x3333, 0x0000, 0x8101, 0x75fe, 0x9510, 0x2811, 0xdd86, 0x0060,
		  0x0000, 0x3800, 0x0100, 0x80fe, 0x0000, 0x0000, 0x0000, 0x8b90,
		  0xff87, 0xaafe, 0xbc3c, 0x02ff, 0x0000, 0x0000, 0x0000, 0x0000,
		  0x0000, 0x0000, 0x1600, 0x003a, 0x0205, 0x0000, 0x0001, 0x008f,
		  0x0917, 0x0000, 0x0200, 0x0004, 0x0000, 0x02ff, 0x0000, 0x0000,
		  0x0000, 0x0000, 0x0000, 0x0000, 0x6b00, 0x0004, 0x0000, 0x0eff,
		  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8101,
		  };  

typedef enum {SYNC_MSG, DELAY_REQ_MSG, PDELAY_REQ_MSG, PDELAY_RESP_MSG} ptp_msg_type;

char *msg_str[4] = {"Sync", "Delay_Req", "PDelay_Req", "PDelay_Resp"};
ptp_msg_type msg_rcv;

uint64_t st0;
unsigned long long pt0;
int isr_exit;

static void ptp_rx_isr(FTGMAC030_Control *ctrl)
{
	int off;
	uint8_t *buf;

	prints("=================== \n");

	buf = (uint8_t *)ctrl->rx_buff;
	msg_rcv = 0xFF;

	off = sizeof(struct eth_hdr);
	if (buf[12] == 0x88 && buf[13] == 0xf7) { /* PTP over Ethernet */
		prints("Rx eth ");
		msg_rcv = buf[off] & 0xf;
	} else { /* PTP over IPv4 or IPv6 */
		if (buf[12] == 0x08 && buf[13] == 0x00) {
			prints("Rx ipv4 ");
			off += sizeof(struct ipv4_hdr);
		} else if (buf[12] == 0x86 && buf[13] == 0xdd) {
			prints("Rx ipv6 ");
			off += sizeof(struct ipv6_hdr);
		} else {
			prints("Not valid Ethernet protocol\n");
		}

		/* Check UDP source port is PTP-event */
		if (buf[off] == 0x01 && buf[off+1] == 0x3f) {
			off += sizeof(struct udp_hdr);
			msg_rcv = buf[off] & 0xf;
		} else {
			prints("Not PTP message\n");
		}
	}
#if 0
{
	int i = 0;

	/*prints("rx packet crc = 0x%08x\n", ftgmac030_crc32(ctrl->rx_buff, ctrl->rx_len-4));*/
	prints("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");
	prints("-----------------------------------------------\n");
	for (i = 0;i < ctrl->rx_len; i++) {
		prints("%02x ", buf[i]);
		if (((i + 1) % 16) == 0) {
			prints("\r\n");
		}
	}

	if (((i + 1) % 16) != 0) {
			prints("\r\n");
	}
}
#endif

	if (msg_rcv <= PDELAY_RESP_MSG)
		prints("Msg Type: %s\n", msg_str[msg_rcv]);
	else
		prints("Msg Type : %x\n", msg_rcv);

	prints("PTP time is %u.%09u (ISR)\n", mac_regs->ptp_sec_tmr,  mac_regs->ptp_nsec_tmr);

	isr_exit = 1;
}

static void ptp_tx_isr(FTGMAC030_Control *ctrl)
{
	isr_exit = 1;
}

void xmit_event_messages(FTGMAC030_Control *ctrl, uint32_t phy_tx_addr,
			 uint32_t size, int wait_isr)
{
	int i;
	unsigned int t0_s, t0_ns, t1_s, t1_ns;
	signed long long t_diff;

	if (wait_isr & (FTGMAC030_INT_PDELAY_RESP_OUT |
			FTGMAC030_INT_PDELAY_REQ_OUT)) {
		t0_s = mac_regs->ptp_tx_p_sec;
		t0_ns = mac_regs->ptp_tx_p_nsec;
	} else {
		t0_s = mac_regs->ptp_tx_sec;
		t0_ns = mac_regs->ptp_tx_nsec;
	}

	for (i=0; i < 3; i++) {
#if 0
		icmpv6[0x1B] = 0x0099;
		ctrl->xmit(ctrl, (uint32_t) &icmpv6, sizeof(icmpv6),
			   (FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_UDPCS_EN |
			    FTGMAC030_TXDES1_TCPCS_EN));

		while (!(mac_regs->isr & FTGMAC030_INT_TPKT2E));

		icmpv6[0x1E] = 0x9699;
		ctrl->xmit(ctrl, (uint32_t) &announce, sizeof(announce),
			   (FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_UDPCS_EN |
			    FTGMAC030_TXDES1_TCPCS_EN));

		while (!(mac_regs->isr & FTGMAC030_INT_TPKT2E));

		if (mac_regs->isr & wait_isr)
			prints("Interrupt 0x%08x\n", wait_isr);

		ftgmac030_delaysecs(1);
#endif

		isr_exit = 0;

		ctrl->xmit(ctrl, phy_tx_addr, size, 
			   FTGMAC030_TXDES1_DEFAULT);

		while (!isr_exit);

		if (wait_isr & (FTGMAC030_INT_PDELAY_RESP_OUT |
				FTGMAC030_INT_PDELAY_REQ_OUT)) {
			t1_s = mac_regs->ptp_tx_p_sec;
			t1_ns = mac_regs->ptp_tx_p_nsec;
		} else {
			t1_s = mac_regs->ptp_tx_sec;
			t1_ns = mac_regs->ptp_tx_nsec;
		}

		/* unit in nanosecond */
		if (t0_ns > 999999999) {
			prints("timestamp ns larger than 999999999 ns(t0: %d)\n", t0_ns);
		} else if (t1_ns > 999999999) {
			prints("timestamp ns larger than 999999999 ns(t1: %d)\n", t1_ns);
		} else if (!t1_s || !t1_ns) {
			prints("t0 %d s %d ns, t1 %d s %d ns\n", t0_s, t0_ns, t1_s, t1_ns);
		} else {

			/* second not increment, substract the nanoscond part */
			if (t1_s - t0_s == 0) {
				t_diff = t1_ns - t0_ns;
			} else if (t1_s - t0_s == 1) {
				t_diff = 999999999 - t0_ns;
				t_diff += t1_ns;
			} else {
				t_diff = 999999999 - t0_ns;
				t_diff += (t1_s - t0_s - 1) * 1000000000;
				t_diff += t1_ns;
			}
			prints("t0 %d s %d ns, t1 %d s %d ns, tmstmp diff %lld ns\n",
			       t0_s, t0_ns, t1_s, t1_ns , t_diff);

		}

		t0_s = t1_s;
		t0_ns = t1_ns;

		ftgmac030_delaysecs(2);
	}
}

void ptp_txtsmp_test(FTGMAC030_Control *ctrl)
{
	struct ptp_message ptpmsg;
	uint8_t sa[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
	uint8_t da[6] = {0x01, 0x1b, 0x19, 0x00, 0x00, 0x00};
	Octet clockIdentity[8] = {0xfe, 0x75, 0x10, 0xff, 0xfe, 0x95, 0x11, 0x28};
	struct skb_buff skb;
	struct inet_addr inetaddr;
	int i, msg_type, msg_len, wait_isr;
	struct eth_hdr *ethd;

	memset(&ptpmsg, 0, sizeof(struct ptp_message));
	memcpy(&ptpmsg.header.sourcePortIdentity.clockIdentity, clockIdentity,
	       sizeof(clockIdentity));
	ptpmsg.header.sourcePortIdentity.portNumber = htons(0x0001);
	ptpmsg.header.versionPTP = 0x2;
	ptpmsg.header.flagField[0] = 0x2;
	//ptpmsg.sync.originTimestamp.seconds_lsb = htonl(207);
	//ptpmsg.sync.originTimestamp.nanoseconds = htonl(260272000);
	
	/* IPv6 address */
	inetaddr.sa.addr32[0] = htonl(0xfe800000);
	inetaddr.sa.addr32[1] = htonl(0x00000000);
	inetaddr.sa.addr32[2] = htonl(0xfc7510ff);
	inetaddr.sa.addr32[3] = htonl(0xfe951128);
	inetaddr.da.addr32[0] = htonl(0xff0e0000);
	inetaddr.da.addr32[1] = htonl(0x00000000);
	inetaddr.da.addr32[2] = htonl(0x00000000);
	inetaddr.da.addr32[3] = htonl(0x00000181);

	/* IPv4 address */
	inetaddr.v4_sa.addr32 = htonl(0x0a000020);
#if 0
	inetaddr.v4_da.addr32 = htonl(0xe0000181);
#else
	inetaddr.v4_da.addr32 = htonl(0x0a000012);
#endif

	/* PTP port number */
	inetaddr.s_port = htons(319);
	inetaddr.d_port = htons(319);

	for (msg_type=0; msg_type < 4; msg_type++) {

		if (msg_type == 0) {
			msg_len = sizeof(struct sync_msg);
			wait_isr = FTGMAC030_INT_SYNC_OUT;
		} else if (msg_type == 1) {
			msg_len = sizeof(struct delay_req_msg);
			wait_isr = FTGMAC030_INT_DELAY_REQ_OUT;
		} else if (msg_type == 2) {
			msg_len = sizeof(struct pdelay_req_msg);
			wait_isr = FTGMAC030_INT_PDELAY_REQ_OUT;
		} else {
			msg_len = sizeof(struct pdelay_resp_msg);
			wait_isr = FTGMAC030_INT_PDELAY_RESP_OUT;
		}

		ptpmsg.header.messageType = msg_type;
		ptpmsg.header.messageLength = htons(msg_len);
			
		for (i=0; i < 3; i++) {

			init_skb(&skb, (uint8_t *)TX_BUF_ADDR, 512);
			/* 16-byte alignment */
			skb_push(&skb, (msg_len + 0x10) & (~(0xf)));
			skb_tail(&skb, 0x10 - (msg_len & 0xf));
			memcpy(skb.data, (uint8_t *) &ptpmsg, msg_len);

			if (i == 2) {
				skb.proto = ETH_PTP;
				pack_ether_hwaddr(&skb, sa, da);
				prints("%s message over Ethernet\n", msg_str[msg_type]);
			} else {
				pack_udp(&skb, msg_len, &inetaddr, i);

				if (!i) {
					prints("%s message over IPv4\n",
					       msg_str[msg_type]);
					pack_ipv4(&skb, &inetaddr, 5);
				} else {
					prints("%s message over IPv6\n",
					       msg_str[msg_type]);
					pack_ipv6(&skb, &inetaddr);
				}
			
				pack_ether(&skb);
			}

			if (skb.len < 60)
				skb.len = 60;

			xmit_event_messages(ctrl, (uint32_t)((uintptr_t)skb.data),
					    skb.len, wait_isr);

			ftgmac030_delaysecs(2);
		}
	}

	/* Send terminate packet to remote program before
	 * doing PTP Rx test.
	 * Intently to make the Ethernet protocol to invalid
	 */
	ethd = (struct eth_hdr *)skb.data;
	ethd->proto = 0xabcd;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)skb.data), 64, FTGMAC030_TXDES1_DEFAULT);
}

void ptp_rxtsmp_test(FTGMAC030_Control *ctrl)
{
	unsigned int t0_s, t0_ns, t1_s, t1_ns;
	uint64_t t0, t1;
	signed long long t_diff;

	t0 = ftgmac030_get_time() / COUNTS_PER_MICRO_SECOND;
	t0_s = mac_regs->ptp_rx_sec;
	t0_ns = mac_regs->ptp_rx_nsec;
	isr_exit = 0;
	do {

		if (isr_exit) {

			t1 = ftgmac030_get_time() / COUNTS_PER_MICRO_SECOND;

			if (msg_rcv == SYNC_MSG) {
				t1_s = mac_regs->ptp_rx_sec;
				t1_ns = mac_regs->ptp_rx_nsec;
				prints("SNYC: %d s %d ns, ", t1_s, t1_ns);
			} else if (msg_rcv == DELAY_REQ_MSG) {
				t1_s = mac_regs->ptp_rx_sec;
				t1_ns = mac_regs->ptp_rx_nsec;
				prints("DELAY_REQ: %d s %d ns, ", t1_s, t1_ns);
			} else if (msg_rcv == PDELAY_REQ_MSG) {
				t1_s = mac_regs->ptp_rx_p_sec;
				t1_ns = mac_regs->ptp_rx_p_nsec;
				prints("PDELAY_REQ: %d s %d ns, ", t1_s, t1_ns);
			} else if (msg_rcv == PDELAY_RESP_MSG) {
				t1_s = mac_regs->ptp_rx_p_sec;
				t1_ns = mac_regs->ptp_rx_p_nsec;
				prints("PDELAY_RESP: %d s %d ns, ", t1_s, t1_ns);
			} else {
				prints("not a ptp msg, ");
				break;
			}
			
			/* second not increment, substract the nanoscond part */
			if (t1_s - t0_s == 0) {
				t_diff = t1_ns - t0_ns;
			} else if (t1_s - t0_s == 1) {
				t_diff = 999999999 - t0_ns;
				t_diff += t1_ns;
			} else {
				t_diff = 999999999 - t0_ns;
				t_diff += (t1_s - t0_s - 1) * 1000000000;
				t_diff += t1_ns;
			}

			prints("sys diff %lld us, tmstp diff %lld ns\n",
					t1 - t0, t_diff);
			t0 = t1;
			t0_s = t1_s;
			t0_ns = t1_ns;

			isr_exit = 0;

			prints("################# \n");
		}

		if ('q' == uart_kbhit())
			break;
	} while (1);
}

void timer_callback(void * TimerInstance)
{
	uint64_t st1;
	unsigned long long pt1;
	signed long long s_diff, p_diff;

	st1 = ftgmac030_get_time();
	pt1 = ((unsigned long long)mac_regs->ptp_sec_tmr * 1000000000ull) +
		   mac_regs->ptp_nsec_tmr;

	s_diff = st1 - st0;
	p_diff = pt1 - pt0;

	prints("sys diff %4.9f, ptp diff %4.9f\n",
			((s_diff * 1.0) / COUNTS_PER_SECOND),
			((p_diff * 1.0) / 1000000000));

	pt0 = ((unsigned long long)mac_regs->ptp_sec_tmr * 1000000000ull) +
		   mac_regs->ptp_nsec_tmr;
	st0 = ftgmac030_get_time();
}

extern int giga_mode;

void ptp_test_main(FTGMAC030_Control *ctrl)
{
	prints("\n--- PTP timer test---\n");

#if PTP_TEST_TIMER_ONLY

	mac_regs->ptp_nns_period = 0;
	mac_regs->ptp_ns_period = 40;
	mac_regs->ptp_nnsec_tmr = 0;
	mac_regs->ptp_nsec_tmr = 0;
	mac_regs->ptp_sec_tmr = 0;

	mac_regs->maccr |= FTGMAC030_MACCR_PTP_EN;

	pt0 = ((unsigned long long)mac_regs->ptp_sec_tmr * 1000000000ull) +
			mac_regs->ptp_nsec_tmr;
	st0 = ftgmac030_get_time();
	ftgmac030_enable_timer_interrupt();
	ftgmac030_start_timer();
#else
	int speed;

	ctrl->open(ctrl, ptp_rx_isr);

	mac_regs->ptp_nns_period = 0;
	mac_regs->ptp_ns_period = 40;
	mac_regs->ptp_nnsec_tmr = 0;
	mac_regs->ptp_nsec_tmr = 0;
	mac_regs->ptp_sec_tmr = 0;

	mac_regs->maccr |= FTGMAC030_MACCR_PTP_EN;

	/* _1000 = 0, _100  = 1, _10   = 2 */
	speed = 0;//(!giga_mode) ? 1 : 0;
	for (;speed < 1; speed++) {

		if ('q' == uart_kbhit())
			break;

		prints("\n--- PTP test in %d/Full duplex mode.---\n",
			(speed == 2) ? 10: (speed == 1) ? 100 : 1000);

		/* disable transmit interrupts for pollings
		mac_regs->ier &= ~(FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F | FTGMAC030_INT_RXBUF_UNAVA |
				   FTGMAC030_INT_RPKT_LOST | FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);
		*/
		//if (ctrl->change_duplex_speed(ctrl, FULL, speed))
			//continue;
		mac_regs->ptp_rx_addr = 0x0A0000b3;
		mac_regs->ptp_tx_addr = 0x0A000012;

		prints("------ PTP Rx Timestamp test ------\n");
		ptp_rxtsmp_test(ctrl);
	}

	speed = 0;//(!giga_mode) ? 1 : 0;
	for (;speed < 1; speed++) {

		if ('q' == uart_kbhit())
			break;

		prints("\n--- PTP test in %d/Full duplex mode.---\n",
			(speed == 2) ? 10: (speed == 1) ? 100 : 1000);

		/* disable transmit interrupts for pollings
		mac_regs->ier &= ~(FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F | FTGMAC030_INT_RXBUF_UNAVA |
				   FTGMAC030_INT_RPKT_LOST | FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);
		*/
		//if (ctrl->change_duplex_speed(ctrl, FULL, speed))
			//continue;

		ctrl->cmpl_isr = ptp_tx_isr;

		prints("\n------ PTP Tx Timestamp test ------\n");
		ptp_txtsmp_test(ctrl);
	}
#endif
}
