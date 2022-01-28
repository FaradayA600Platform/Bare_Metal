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

#define IP_SRC 0xc0a84310 /* 192.168.67.16  */
#define IP_DST 0xc0a84312 /* 192.168.67.18 */

static int enter_isr = 0;

static void offload_isr(FTGMAC030_Control *ctrl)
{
	enter_isr = 1;
}

#define IP_HEADER_MIN 20
#define ETH_HEADER_LEN 14

#define TCP_FLAG_FIN (1 << 0)
#define TCP_FLAG_SYN (1 << 1)
#define TCP_FLAG_RST (1 << 2)
#define TCP_FLAG_PSH (1 << 3)
#define TCP_FLAG_ACK (1 << 4)
#define TCP_FLAG_URG (1 << 5)
#define TCP_FLAG_ECN (1 << 6)
#define TCP_FLAG_CWR (1 << 7)
#define TCP_FLAG_NON (1 << 8)
#define TCP_HEADER_MIN 20

#if 0
static void send_llc_packet_normal(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	uint32_t offset = 0;
	offset += populate_eth_header(buff, MAC_SRC, MAC_DST, 0x0056);
	//offset = 12;
	for (offset; offset < len; offset++) {
		buff[offset] = 0x00 & 0xff;
	}
	prints("->Send %d byte normal packet:\n", len);
	ctrl->open(ctrl, offload_isr);
	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num], (uint32_t)buff, len,
			    FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_LLC_PKT);
	ftgmac030_delaysecs(1);
}

static void send_llc_packet_normal_ip(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	uint32_t offset = 0;
	offset += populate_eth_header(buff, MAC_SRC, MAC_DST, 0x0056);
	//offset = 12;
	for (offset; offset < len; offset++) {
		buff[offset] = 0x00 & 0xff;
	}
	prints("->Send %d byte normal packet:\n", len);
	ctrl->open(ctrl, offload_isr);
	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num], (uint32_t)buff, len,
			    FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_IPCS_EN);
	ftgmac030_delaysecs(1);
}

static void send_llc_packet_ip(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	uint32_t offset = 0;
	offset += populate_eth_header(buff, MAC_SRC, MAC_DST, 0x0056);
	//offset = 12;
	for (offset; offset < 32; offset++) {
		buff[offset] = 0x01 & 0xff;
	}
	for (offset; offset < len; offset++) {
		buff[offset] = 0x00 & 0xff;
	}
	prints("->Send %d byte normal packet:\n", len);
	ctrl->open(ctrl, offload_isr);
	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num], (uint32_t)buff, len,
			    FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_LLC_PKT);
	ftgmac030_delaysecs(1);
}

static void send_llc_packet_udp(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	uint32_t offset = 0;
	offset += populate_eth_header(buff, MAC_SRC, MAC_DST, 0x0000);
	offset = 12;
	for (offset; offset < len; offset++) {
		buff[offset] = offset & 0xff;
	}
	prints("->Send %d byte normal packet:\n", len);
	ctrl->open(ctrl, offload_isr);
	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num], (uint32_t)buff, len,
			    FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_LLC_PKT |
			    FTGMAC030_TXDES1_UDPCS_EN);
	ftgmac030_delaysecs(1);
}

static void send_llc_packet_tcp(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	uint32_t offset = 0;
	offset += populate_eth_header(buff, MAC_SRC, MAC_DST, 0x0000);
	offset = 12;
	for (offset; offset < len; offset++) {
		buff[offset] = offset & 0xff;
	}
	prints("->Send %d byte normal packet:\n", len);
	ctrl->open(ctrl, offload_isr);
	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num], (uint32_t)buff, len,
			    FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_LLC_PKT |
			    FTGMAC030_TXDES1_TCPCS_EN);
	ftgmac030_delaysecs(1);
}
#endif

static void send_ip_packet(FTGMAC030_Control *ctrl, uint8_t *buff, int len,
			   uint8_t header_len, int vlan_tag)
{
	struct skb_buff skb;
	struct inet_addr inetaddr;
	struct ipv4_hdr *iphdr;
	uint8_t *data;
	struct multi_desc *deses = NULL;
	unsigned int txdes1;
	int i = 0, count = 0;

	inetaddr.v4_sa.addr32 = htonl(IP_SRC);
	inetaddr.v4_da.addr32 = htonl(IP_DST);

	init_skb(&skb, buff, len);

	i = len - sizeof(struct eth_hdr) - (header_len << 2);
	skb_push(&skb, i);
	/* payload */
	data = (uint8_t *)skb.data;
	for (i = len - skb.len; i < len; i ++) {
		*data = i & 0xff;
		data++;
	}

	count = header_len << 2;
	count = count - IP_HEADER_MIN;
	skb_push(&skb, count);
	data = (uint8_t *)skb.data;
	/* IP Header option */
	for (i = 1; i <= count; i++) {
		data[i-1] = (0xA0 | i);
	}

	pack_ipv4(&skb, &inetaddr, header_len);
	iphdr = (struct ipv4_hdr *) skb.data;
	prints("Send IP packet %d bytes, header length %d, checksum 0x%04x\n",
	       len, header_len, iphdr->checksum);
	/* Let HW calculate the checksum */
	iphdr->checksum = 0;

	pack_ether(&skb);

	enter_isr = 0;
	txdes1 = FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_IPCS_EN |
		 FTGMAC030_TXDES1_UDPCS_EN | FTGMAC030_TXDES1_TCPCS_EN;
	if (vlan_tag)
		txdes1 |= (FTGMAC030_TXDES1_INS_VLAN |
			   FTGMAC030_TXDES1_VLAN_TAGC(vlan_tag));

	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num],
			    (const uint32_t)((uintptr_t)skb.data), skb.len, txdes1);
	while (!enter_isr);

	ftgmac030_delaysecs(1);

	deses = (struct multi_desc *)malloc(3 * sizeof(struct multi_desc));
	memset(deses, 0, 3 * sizeof(struct multi_desc));
	count = skb.len / 3;	
	for(i = 0; i < 3; i++)
	{
		deses[i].phy_tx_addr = (uint32_t)((uintptr_t)(skb.data + i * count));

		if (i == 2)
			deses[i].size = (skb.len - i * count);
		else
			deses[i].size = count;
	}
	
	enter_isr = 0;
	ctrl->xmit_multi_desc(&ctrl->tx_queue[ctrl->tx_queue_num], 3, deses, txdes1);
	while (!enter_isr);
	free(deses);
}

static void send_udp_packet(FTGMAC030_Control *ctrl, uint8_t *buff, int len, int vlan_tag)
{
	struct skb_buff skb;
	struct inet_addr inetaddr;
	struct ipv4_hdr *iphdr;
	struct udp_hdr *udphdr;
	uint8_t *data;
	struct multi_desc *deses = NULL;
	unsigned int txdes1;
	int i = 0, count = 0;

	inetaddr.v4_sa.addr32 = htonl(IP_SRC);
	inetaddr.v4_da.addr32 = htonl(IP_DST);
	inetaddr.s_port = htons(8235);
	inetaddr.d_port = htons(8234);

	count = len - sizeof(struct eth_hdr) - sizeof(struct ipv4_hdr) -
                sizeof(struct udp_hdr);
	init_skb(&skb, buff, len);
	skb_push(&skb, count);

	i = sizeof(struct eth_hdr) + sizeof(struct ipv4_hdr) +
                sizeof(struct udp_hdr);
	data = (uint8_t *)skb.data;
	for (; i < count; i++) {
		*data = i & 0xff;
		data++;
	}

	prints("Send UDP packet %d bytes, ", len);
	pack_udp(&skb, count, &inetaddr, IPV4_CKSM);
	udphdr = (struct udp_hdr *) skb.data;
	prints("UDP checksum 0x%04x, ", udphdr->checksum);
	/* Let HW calculate the checksum */
	udphdr->checksum = 0;

	pack_ipv4(&skb, &inetaddr, 5);
	iphdr = (struct ipv4_hdr *) skb.data;
	prints("IP checksum 0x%04x\n", iphdr->checksum);
	/* Let HW calculate the checksum */
	iphdr->checksum = 0;

	pack_ether(&skb);

	enter_isr = 0;
	txdes1 = FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_IPCS_EN |
		 FTGMAC030_TXDES1_UDPCS_EN | FTGMAC030_TXDES1_TCPCS_EN;
	if (vlan_tag)
		txdes1 |= (FTGMAC030_TXDES1_INS_VLAN |
			   FTGMAC030_TXDES1_VLAN_TAGC(vlan_tag));

	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num],
			    (uint32_t)((uintptr_t)skb.data), skb.len, txdes1);
	while (!enter_isr);

	ftgmac030_delaysecs(1);

	deses = (struct multi_desc *)malloc(3 * sizeof(struct multi_desc));
	memset(deses, 0, 3 * sizeof(struct multi_desc));
	count = skb.len / 3;	
	for(i = 0; i < 3; i++)
	{
		deses[i].phy_tx_addr = (uint32_t)((uintptr_t)(skb.data + i * count));

		if (i == 2)
			deses[i].size = (skb.len - i * count);
		else
			deses[i].size = count;
	}
	
	enter_isr = 0;
	ctrl->xmit_multi_desc(&ctrl->tx_queue[ctrl->tx_queue_num], 3, deses, txdes1);
	while (!enter_isr);
	free(deses);
}

static void send_tcp_packet(FTGMAC030_Control *ctrl, uint8_t *buff, int len,
			    uint8_t tcp_header_len, int vlan_tag)
{
	struct skb_buff skb;
	struct inet_addr inetaddr;
	struct ipv4_hdr *iphdr;
	struct tcp_hdr *tcphdr;
	uint8_t *data;
	struct multi_desc *deses = NULL;
	unsigned int txdes1;
	int i = 0, count = 0;

	inetaddr.v4_sa.addr32 = htonl(IP_SRC);
	inetaddr.v4_da.addr32 = htonl(IP_DST);
	inetaddr.s_port = htons(8235);
	inetaddr.d_port = htons(8234);

	init_skb(&skb, buff, len);

	i = len - sizeof(struct eth_hdr) - IP_HEADER_MIN - (tcp_header_len << 2);
	skb_push(&skb, i);
	/* payload */
	data = (uint8_t *)skb.data;
	for (i = len - skb.len; i < len; i ++) {
		*data = i & 0xff;
		data++;
	}

	count = tcp_header_len << 2;
	count = count - TCP_HEADER_MIN;
	skb_push(&skb, count);
	data = (uint8_t *)skb.data;
	/* TCP Header option */
	for (i = 1; i <= count; i++) {
		data[i-1] = (0xA0 | i);
	}

	prints("Send TCP packet %d bytes, ", len);
	count = len - sizeof(struct eth_hdr) - IP_HEADER_MIN - (tcp_header_len << 2);
	pack_tcp(&skb, count, &inetaddr, tcp_header_len,IPV4_CKSM);
	tcphdr = (struct tcp_hdr *) skb.data;
	prints("TCP checksum 0x%04x, ", tcphdr->checksum);
	/* Let HW calculate the checksum */
	tcphdr->checksum = 0;

	pack_ipv4(&skb, &inetaddr, 5);
	iphdr = (struct ipv4_hdr *) skb.data;
	prints("IP checksum 0x%04x\n", iphdr->checksum);
	/* Let HW calculate the checksum */
	iphdr->checksum = 0;

	pack_ether(&skb);

	enter_isr = 0;
	txdes1 = FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_IPCS_EN |
		 FTGMAC030_TXDES1_UDPCS_EN | FTGMAC030_TXDES1_TCPCS_EN;
	if (vlan_tag)
		txdes1 |= (FTGMAC030_TXDES1_INS_VLAN |
			   FTGMAC030_TXDES1_VLAN_TAGC(vlan_tag));

	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num],
			    (const uint32_t)((uintptr_t)skb.data), skb.len, txdes1);
	while (!enter_isr);

	ftgmac030_delaysecs(1);

	deses = (struct multi_desc *)malloc(3 * sizeof(struct multi_desc));
	memset(deses, 0, 3 * sizeof(struct multi_desc));
	count = skb.len / 3;	
	for(i = 0; i < 3; i++)
	{
		deses[i].phy_tx_addr = (uint32_t)((uintptr_t)(skb.data + i * count));

		if (i == 2)
			deses[i].size = (skb.len - i * count);
		else
			deses[i].size = count;
	}
	
	enter_isr = 0;
	ctrl->xmit_multi_desc(&ctrl->tx_queue[ctrl->tx_queue_num], 3, deses, txdes1);
	while (!enter_isr);
	free(deses);
}
#if 0
static void send_llc_packet(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	struct skb_buff skb;
	uint8_t *data;
	int i = 0, count = 0;

	init_skb(&skb, buff, len);

	count = len - sizeof(struct eth_hdr);
	skb_push(&skb, (len - sizeof(struct eth_hdr)));
	data = (uint8_t *)skb.data;
	for (i = 1; i <= count; i++) {
		*data = (0xA0 | i);
		data++;
	}

	pack_ether(&skb);

	/* Checksum will not be offload by MAC automatically when setting LLC_PKT. */
	prints("->Send %d byte llc packet:\n", len);
	ctrl->xmit_one_desc(&ctrl->tx_queue[ctrl->tx_queue_num],
			    (uint32_t)((uintptr_t)skb.data), skb.len,
			    FTGMAC030_TXDES1_DEFAULT | FTGMAC030_TXDES1_OTHER_PKT |
			    FTGMAC030_TXDES1_LLC_PKT);
	while (!enter_isr);
	ftgmac030_delaysecs(1);
}
#endif
void offload_test_main(FTGMAC030_Control *ctrl)
{
	static uint8_t *const buff = (uint8_t *) (TX_BUF_ADDR + 2);
	uint32_t i, tag;

	memset(buff, 0, MAX_PKT_SIZE);

	prints("Please run corresponding remote program first and press any key to "
	       "continue...\n");
	uart_getc();

	/*prints("---Test 1. LLC IP packets offload test.---------------------------\n");
	for (i = 100; i < 120; i++) {
		send_llc_packet_ip(ctrl, buff, i);
	}

	prints("---Test 2. LLC UDP packets offload test.----------------------------\n");
	for (i = 100; i < 120; i++) {
		send_llc_packet_udp(ctrl, buff, i);
	}

	prints("---Test 3. LLC TCP packets offload test.----------------------------\n");
	for (i = 100; i < 120; i++) {
		send_llc_packet_tcp(ctrl, buff, i);
	}

	send_llc_packet(ctrl, buff, 100);

	*/

	ctrl->open(ctrl, offload_isr);

	prints("---Test 1. IPv4 checksum offload test.------------------------------\n");
	for (i = 5; i <= 15; i++) {
		send_ip_packet(ctrl, buff, 100 + i, i, 0);
	}

	prints("---Test 2. UDP checksum offload test.-------------------------------\n");
	for (i = 100; i < 120; i++) {
		send_udp_packet(ctrl, buff, i, 0);
	}

	prints("---Test 3. TCP checksum offload test.-------------------------------\n");
	for (i = 5; i <= 15; i++) {
		send_tcp_packet(ctrl, buff, 100 + i, i, 0);
	}

	prints("---Test 4. IPv4 Insert VLAN tag test.-------------------------------\n");
	for (i = 5; i <= 15; i++) {
		tag = i * 273;
		tag |= (i << 13);
		send_ip_packet(ctrl, buff, 100 + i, i, tag);
	}

	prints("---Test 5. UDP Insert VLAN tag test.--------------------------------\n");
	for (i = 100; i < 120; i++) {
		tag = i;
		tag |= (i << 13);
		send_udp_packet(ctrl, buff, i, tag);
	}

	prints("---Test 6. TCP Insert VLAN tag test.--------------------------------\n");
	for (i = 5; i <= 15; i++) {
		tag = i * 273;
		tag |= (i << 13);
		send_tcp_packet(ctrl, buff, 100 + i, i, tag);
	}
}
