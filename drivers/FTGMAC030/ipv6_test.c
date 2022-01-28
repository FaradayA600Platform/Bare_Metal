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
#include <malloc.h>

#include "ftgmac030.h"
#include "skb_buff.h"
#include "net.h"

static volatile int tx_done = 0;
static int txdes1 = FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_IPV6_PKT |
		    FTGMAC030_TXDES1_UDPCS_EN | FTGMAC030_TXDES1_TCPCS_EN;

static void xmit_isr(FTGMAC030_Control *ctrl)
{
	tx_done = 1;
}

static void xmit_ipv6(FTGMAC030_Control *ctrl, struct inet_addr *inetaddr,
		      struct skb_buff *skb)
{
	struct multi_desc *deses = NULL;
	int i, len;

	pack_ipv6(skb, inetaddr);

	prints("IPv6 payload %d bytes.\n", skb->len - sizeof(struct ipv6_hdr));

	pack_ether(skb);

	tx_done = 0;
	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num],
			    (const uint32_t)((uintptr_t)skb->data), skb->len, txdes1);
	while (!tx_done);

	ftgmac030_delaysecs(1);

	deses = (struct multi_desc *)malloc(3 * sizeof(struct multi_desc));
	memset(deses, 0, 3 * sizeof(struct multi_desc));
	len = skb->len / 3;	
	for(i = 0; i < 3; i++)
	{
		deses[i].phy_tx_addr = (uintptr_t)(skb->data + i * len);

		if (i == 2)
			deses[i].size = (skb->len - i * len);
		else
			deses[i].size = len;
	}

	tx_done = 0;
	ctrl->xmit_multi_desc(&ctrl->tx_queue[ctrl->tx_queue_num], 3, deses, txdes1);
	while (!tx_done);
	free(deses);
}

void ipv6_test_main(FTGMAC030_Control *ctrl)
{
	int i, j;
	static uint8_t * const buff = (uint8_t *)TX_BUF_ADDR;
	struct skb_buff skb;
	struct inet_addr inetaddr;
	struct in6addr addr[3];
	struct udp_hdr *udphdr;
	struct tcp_hdr *tcphdr;
	uint8_t *data;
	
	inetaddr.sa.addr32[0] = htonl(0xfe801234);
	inetaddr.sa.addr32[1] = htonl(0x56789abc);
	inetaddr.sa.addr32[2] = htonl(0x020a35ff);
	inetaddr.sa.addr32[3] = htonl(0xfe000122);

	inetaddr.s_port = htons(5001);
	inetaddr.d_port = htons(4096);

	addr[0].addr32[0] = htonl(0x2001007a);
	addr[0].addr32[1] = htonl(0x00000de1);
	addr[0].addr32[2] = htonl(0x78d00000);
	addr[0].addr32[3] = htonl(0x00000011);

	ctrl->open(ctrl, xmit_isr);

	prints("---Test 1. IPv6 UDP checksum offload test.------------\n");
	for (i = 100; i <= 140; i++) {

		/* fe80:beae:0102:0300::fee3:3ed1 */
		inetaddr.da.addr32[0] = htonl(0xfe80beae);
		inetaddr.da.addr32[1] = htonl(0x01020300 + i);
		inetaddr.da.addr32[2] = htonl(0x00000000);
		inetaddr.da.addr32[3] = htonl((0xfee33ed1 + i));

		init_skb(&skb, buff, 512);

		/* 16-byte alignment */
		skb_push(&skb, (i + 0x10) & (~(0xf)));
		skb_tail(&skb, 0x10 - (i & 0xf));

		data = (uint8_t *) skb.data;
		for(j = 0; j < i; j++)
			data[j] = 0xA0 | j;

		pack_udp(&skb, i, &inetaddr, IPV6_CKSM);
		udphdr = (struct udp_hdr *) skb.data;
		prints("Send UDP chksum 0x%04x, ", udphdr->checksum);
		udphdr->checksum = 0;

		/* Always put DA for checksum calculation  at the last
		 * entry of routing header.
		 */
		memcpy(&addr[2], &inetaddr.da, sizeof(struct in6addr));

		if (i <= 105) {
			prints("no header, ");
			pack_exthdr_noexthdr(&skb);
		} else if (i <= 110) {
			prints("normal, ");
		} else if (i <= 115) {
			prints("routing header(seg left = 0), ");

			pack_exthdr_route(&skb, &addr[0], 3, 0);
		} else if (i <= 120) {
			prints("routing header(seg left = 1), ");

			pack_exthdr_route(&skb, &addr[0], 3, 1);
		} else if (i <= 125) {
			prints("routing header(seg leftt = 2), ");

			pack_exthdr_route(&skb, &addr[0], 3, 2);
		} else if (i <= 130) {
			prints("routing header(seg left = 3), ");

			pack_exthdr_route(&skb, &addr[0], 3, 3);
		} else if (i <= 135) {
			prints("fragment header, ");
			pack_exthdr_frag(&skb);
		} else {
			prints("routing and fragment header, ");
			pack_exthdr_frag(&skb);

			/* Always put inetaddr.da at the last entry of routing header */
			memcpy(&addr[1], &inetaddr.da, sizeof(struct in6addr));

			pack_exthdr_route(&skb, &addr[0], 2, 1);
		}

		memcpy(&addr[1], &inetaddr.da, sizeof(struct in6addr));

		xmit_ipv6(ctrl, &inetaddr, &skb);
	}

	prints("---Test 2. IPv6 TCP checksum offload test.------------\n");
	for (i = 100; i <= 140; i++) {

		/* fe80:beae::c5ff:fee3:3ed1 */
		inetaddr.da.addr32[0] = htonl(0xfe80beae);
		inetaddr.da.addr32[1] = htonl(0x00000000);
		inetaddr.da.addr32[2] = htonl(0x0000c5ff);
		inetaddr.da.addr32[3] = htonl((0xfee33ed1 + i));

		init_skb(&skb, buff, 512);

		/* 16-byte alignment */
		skb_push(&skb, (i + 0x10) & (~(0xf)));
		skb_tail(&skb, 0x10 - (i & 0xf));

		data = (uint8_t *) skb.data;
		for(j = 0; j < i; j++)
			data[j] = 0xA0 | j;

		/* Push room for TCP header */
		j = (i / 18);
		skb_push(&skb, (j << 2) - sizeof(struct tcp_hdr));

		pack_tcp(&skb, i, &inetaddr, j, IPV6_CKSM);
		tcphdr = (struct tcp_hdr *) skb.data;
		prints("Send TCP chksum 0x%04x, ", tcphdr->checksum);
		tcphdr->checksum = 0;

		/* Always put DA for checksum calculation  at the last
		 * entry of routing header.
		 */
		memcpy(&addr[2], &inetaddr.da, sizeof(struct in6addr));

		if (i <= 105) {
			prints("no header, ");
			pack_exthdr_noexthdr(&skb);
		} else if (i <= 110) {
			prints("normal, ");
		} else if (i <= 115) {
			prints("routing header(seg left = 0), ");

			pack_exthdr_route(&skb, &addr[0], 3, 0);
		} else if (i <= 120) {
			prints("routing header(seg left = 1), ");

			pack_exthdr_route(&skb, &addr[0], 3, 1);
		} else if (i <= 125) {
			prints("routing header(seg leftt = 2), ");

			pack_exthdr_route(&skb, &addr[0], 3, 2);
		} else if (i <= 130) {
			prints("routing header(seg left = 3), ");

			pack_exthdr_route(&skb, &addr[0], 3, 3);
		} else if (i <= 135) {
			prints("fragment header, ");
			pack_exthdr_frag(&skb);
		} else {
			prints("routing and fragment header, ");
			pack_exthdr_frag(&skb);

			memcpy(&addr[1], &inetaddr.da, sizeof(struct in6addr));

			pack_exthdr_route(&skb, &addr[0], 2, 1);
		}

		memcpy(&addr[1], &inetaddr.da, sizeof(struct in6addr));

		xmit_ipv6(ctrl, &inetaddr, &skb);
	}
}
