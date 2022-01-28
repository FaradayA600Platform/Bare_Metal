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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "net.h"
#include "ftgmac030.h"
#if 0
static uint16_t pr_checksum(uint8_t *buff, int size, uint16_t init_check)
{
	int i = 0;
	uint32_t value = 0x0;
	uint16_t checksum = init_check;

	while(i < size) {
		prints("0x%04x\n", ((buff[i] << 8) | buff[i+1]));
		value = checksum + ((buff[i] << 8) | buff[i+1]);
		i += 2;
		checksum = (value & 0x0000ffff) + (value >> 16);
	}
	return checksum;
}
#endif

static uint16_t checksum(uint8_t *buff, int size, uint16_t init_check)
{
	int i = 0;
	uint32_t value = 0x0;
	uint16_t checksum = init_check;

	while(i < size) {
		value = checksum + ((buff[i] << 8) | buff[i+1]);
		i += 2;
		checksum = (value & 0x0000ffff) + (value >> 16);
	}
	return checksum;
}

static uint8_t MAC_SRC[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
static uint8_t MAC_DST[6] = {0x02,0x02,0x02,0x02,0x02,0x02};

void pack_ether(struct skb_buff *skb)
{
	struct eth_hdr *ethhdr;
	skb_push(skb, ETH_LEN);
	ethhdr = (struct eth_hdr *)skb->data;
	memcpy(ethhdr->sa, MAC_DST, 6);
	memcpy(ethhdr->da, MAC_SRC, 6);
	ethhdr->proto = htons(skb->proto);
}

void pack_ether_hwaddr(struct skb_buff *skb, uint8_t *sa, uint8_t *da)
{
	struct eth_hdr *ethhdr;
	skb_push(skb, ETH_LEN);
	ethhdr = (struct eth_hdr *)skb->data;
	memcpy(ethhdr->sa, da, 6);
	memcpy(ethhdr->da, sa, 6);
	ethhdr->proto = htons(skb->proto);
}

void pack_exthdr_route(struct skb_buff *skb, struct in6addr *r_addr,
		       int num_addr, unsigned char seg_left)
{
	struct ipv6_route_hdr *route;
	skb_push(skb, sizeof(struct ipv6_route_hdr) + num_addr * sizeof(struct in6addr));
	route = (struct ipv6_route_hdr *)skb->data;
	route->next_header = skb->proto;
	route->hdr_ext_len = num_addr << 1;
	route->type = 0;
	route->seg_left = seg_left;
	skb->proto = NEXTHDR_EXT_ROUTE;
	memcpy(route + 1, r_addr, num_addr * sizeof(struct in6addr));
}

void pack_exthdr_frag(struct skb_buff *skb)
{
	struct ipv6_frag_hdr *frag;
	skb_push(skb, sizeof(struct ipv6_frag_hdr));
	frag = (struct ipv6_frag_hdr *)skb->data;
	frag->next_hdr = skb->proto;
	skb->proto = NEXTHDR_EXT_FRAGMENT;
}

void pack_exthdr_noexthdr(struct skb_buff *skb)
{
	skb->proto = NEXTHDR_NO_NEXTHDR;
}

void pack_ipv6(struct skb_buff *skb, struct inet_addr *inetaddr)
{
	struct ipv6_hdr *ipv6hdr;
	struct ipv6_jumbo_hdr *jumbohdr;

	/* skb->len is payload currently*/
	if (skb->len <= MAX_PAYLOAD) {
		skb_push(skb, sizeof(struct ipv6_hdr));
		ipv6hdr = (struct ipv6_hdr *)skb->data;

		ipv6hdr->version = 6;
		ipv6hdr->payload = htons(skb->len - sizeof(struct ipv6_hdr));
		ipv6hdr->next_header = skb->proto;
		skb->proto = ETH_IPV6;

		memcpy(ipv6hdr->sa.addr8, &inetaddr->sa, sizeof(struct in6addr));
		memcpy(ipv6hdr->da.addr8, &inetaddr->da, sizeof(struct in6addr));
	}
	else {
		skb_push(skb, sizeof(struct ipv6_hdr));
		skb_push(skb, sizeof(struct ipv6_jumbo_hdr));
		ipv6hdr = (struct ipv6_hdr *)skb->data;
		jumbohdr = (struct ipv6_jumbo_hdr *)(ipv6hdr + 1);
		ipv6hdr->version = 6;
		ipv6hdr->payload = 0;
		ipv6hdr->next_header = NEXTHDR_HOP;
		memcpy(ipv6hdr->sa.addr8, &inetaddr->sa, sizeof(struct in6addr));
		memcpy(ipv6hdr->da.addr8, &inetaddr->da, sizeof(struct in6addr));

		jumbohdr->hop.next_hdr = skb->proto;
		jumbohdr->hop.hdr_ext_len = 6;
		jumbohdr->tlv.type = 0xc2;
		jumbohdr->tlv.len = 4;
		jumbohdr->payload_len = skb->len;
	}
}

void pack_ipv4(struct skb_buff *skb, struct inet_addr *inetaddr, unsigned char header_len)
{
	struct ipv4_hdr *ipv4hdr;


	skb_push(skb, sizeof(struct ipv4_hdr));
	ipv4hdr = (struct ipv4_hdr *)skb->data;

	ipv4hdr->version = 4;
	ipv4hdr->ihl = header_len;
	ipv4hdr->len = htons(skb->len);
	ipv4hdr->ttl = 0x1;
	ipv4hdr->protocol = skb->proto;
	ipv4hdr->frag_offset = htons(FLAG_DF);
	skb->proto = ETH_IPV4;

	memcpy(ipv4hdr->sa.addr8, &inetaddr->v4_sa, sizeof(struct in4addr));
	memcpy(ipv4hdr->da.addr8, &inetaddr->v4_da, sizeof(struct in4addr));

	/* calculate checksum */
	{
		uint16_t tmp16;
		tmp16 = checksum((uint8_t *)ipv4hdr, (header_len << 2), 0);
		ipv4hdr->checksum = ~htons(tmp16);
	}
}

void pack_udp(struct skb_buff *skb, uint32_t length, struct inet_addr *inetaddr, int cksm)
{
	struct udp_hdr *udphdr;
	skb_push(skb, sizeof(struct udp_hdr));
	udphdr = (struct udp_hdr *)skb->data;
	udphdr->s_port = inetaddr->s_port;
	udphdr->d_port = inetaddr->d_port;
	udphdr->length = htons(length + sizeof(struct udp_hdr));
	skb->proto = NEXTHDR_UDP;
	/* calculate checksum */
	{
		uint16_t tmp16;
		uint16_t n_hdr = htons(NEXTHDR_UDP);
		uint32_t udp_len32;
		uint16_t udp_len16;
		if ((cksm == IPV6_CKSM) || (cksm == IPV6_ERR_CKSM)) {
			udp_len32 = htonl(length + sizeof(struct udp_hdr));
			tmp16 = checksum(inetaddr->sa.addr8, sizeof(struct in6addr), 0);
			tmp16 = checksum(inetaddr->da.addr8, sizeof(struct in6addr), tmp16);
			tmp16 = checksum((uint8_t *)&udp_len32, sizeof(udp_len32), tmp16);
		}
		else if ((cksm == IPV4_CKSM) || (cksm == IPV4_ERR_CKSM)){
			udp_len16 = htons(length + sizeof(struct udp_hdr));
			tmp16 = checksum(inetaddr->v4_sa.addr8, sizeof(struct in4addr), 0);
			tmp16 = checksum(inetaddr->v4_da.addr8, sizeof(struct in4addr), tmp16);
			tmp16 = checksum((uint8_t *)&udp_len16, sizeof(udp_len16), tmp16);
		}
		else {
			prints("Error: unknow checksum type!\n");
		}
		
		tmp16 = checksum((uint8_t *)&n_hdr, sizeof(n_hdr), tmp16);
		tmp16 = checksum((uint8_t *)udphdr, length + sizeof(struct udp_hdr), tmp16);
		udphdr->checksum = ~htons(tmp16);

		if ((cksm == IPV4_ERR_CKSM) || (cksm == IPV6_ERR_CKSM))
			udphdr->checksum++;
	}
}

void pack_tcp(struct skb_buff *skb, uint32_t length, struct inet_addr *inetaddr,
	      uint8_t header_len, int cksm)
{
	struct tcp_hdr *tcphdr;

	skb_push(skb, sizeof(struct tcp_hdr));
	tcphdr = (struct tcp_hdr *)skb->data;
	tcphdr->s_port = inetaddr->s_port;
	tcphdr->d_port = inetaddr->d_port;
	/* In case not 4 byte alignment will cause data abort */
	tcphdr->seq1 = htons((0 >> 16) & 0xffff);
	tcphdr->seq2 = htons(0 & 0xffff);
	tcphdr->ack_seq1 = htons((0 >> 16) & 0xffff);
	tcphdr->ack_seq2 = htons(0 & 0xffff);
	tcphdr->doff = header_len;
	skb->proto = NEXTHDR_TCP;

	/* calculate checksum */
	{
		uint16_t tmp16;
		uint16_t n_hdr = htons(NEXTHDR_TCP);
		uint32_t tcp_len32;
		uint16_t tcp_len16;

		if ((cksm == IPV6_CKSM) || (cksm == IPV6_ERR_CKSM)) {
			tcp_len32 = htonl(length + (header_len << 2));
			tmp16 = checksum(inetaddr->sa.addr8, sizeof(struct in6addr), 0);
			tmp16 = checksum(inetaddr->da.addr8, sizeof(struct in6addr), tmp16);
			tmp16 = checksum((uint8_t *)&tcp_len32, sizeof(tcp_len32), tmp16);
		}
		else if ((cksm == IPV4_CKSM) || (cksm == IPV4_ERR_CKSM)) {
			tcp_len16 = htons(length + (header_len << 2));
			tmp16 = checksum(inetaddr->v4_sa.addr8, sizeof(struct in4addr), 0); 
			tmp16 = checksum(inetaddr->v4_da.addr8, sizeof(struct in4addr), tmp16);
			tmp16 = checksum((uint8_t *)&tcp_len16, sizeof(tcp_len16), tmp16);
		}
		else {
			prints("Error: unknow checksum type!\n");
		}

		tmp16 = checksum((uint8_t *)&n_hdr, sizeof(n_hdr), tmp16);
		tmp16 = checksum((uint8_t *)tcphdr, length + (header_len << 2), tmp16);
		tcphdr->checksum = ~htons(tmp16);

		if ((cksm == IPV4_ERR_CKSM) || (cksm == IPV6_ERR_CKSM))
			tcphdr->checksum++;
	}
	/* calculate checksum */
	/*{
		uint16_t tmp16;
		uint16_t n_hdr = htons(NEXTHDR_TCP);
		uint32_t tcp_len = __be32(length + sizeof(struct tcp_hdr));
		tmp16 = checksum(inetaddr->sa.addr8, sizeof(struct in6addr), 0);
		tmp16 = checksum(inetaddr->da.addr8, sizeof(struct in6addr), tmp16);
		tmp16 = checksum((uint8_t *)&tcp_len, sizeof(tcp_len), tmp16);
		tmp16 = checksum((uint8_t *)&n_hdr, sizeof(n_hdr), tmp16);
		tmp16 = checksum((uint8_t *)tcphdr, length + sizeof(struct tcp_hdr), tmp16);
		tcphdr->checksum = ~htons(tmp16);
	}*/
}
