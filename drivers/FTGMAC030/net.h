#ifndef __NET_H__
#define __NET_H__

#include <stdint.h>
#include "skb_buff.h"

struct eth_hdr {
	unsigned char sa[6];
	unsigned char da[6];
	unsigned short proto;
};

struct in4addr {
	union {
		unsigned char addr8[4];
		unsigned int addr32;
	};
};

struct in6addr {
	union {
		unsigned char addr8[16];
		unsigned int addr32[4];
	};
};

struct inet_addr {
	struct in4addr v4_sa;
	struct in4addr v4_da;
	struct in6addr sa;
	struct in6addr da;
	unsigned short s_port;
	unsigned short d_port;
};

#define FLAG_DF 0x4000
#define FLAG_MF 0x2000
struct ipv4_hdr {
	unsigned char
		ihl: 4,
		version:4;
	unsigned char services;
	unsigned short len;
	unsigned short id;
	unsigned short frag_offset;
	unsigned char ttl;
	unsigned char protocol;
	unsigned short checksum;
	struct in4addr sa;
	struct in4addr da;
};

struct ipv6_hdr {
	unsigned char
		priority: 4,
		version:4;
	unsigned char flow_label[3];
	
	unsigned short payload;
	unsigned char next_header;
	unsigned char hop_limit;

	struct in6addr sa;
	struct in6addr da;
};

struct ipv6_route_hdr {
	unsigned char next_header;
	unsigned char hdr_ext_len;
	unsigned char type;
	unsigned char seg_left;
	unsigned int reserved;
};

struct ipv6_frag_hdr {
	unsigned char next_hdr;
	unsigned char reserved;
	unsigned short
		m: 1,
		res: 2,
		offset: 13;
	unsigned int id;
};

struct ipv6_tlv {
	unsigned char type;
	unsigned char len;
};

struct ipv6_hop_hdr {
	unsigned char next_hdr;
	unsigned char hdr_ext_len;
};

struct ipv6_jumbo_hdr {
	struct ipv6_hop_hdr hop;
	struct ipv6_tlv tlv;
	unsigned int payload_len;
};

struct tcp_hdr {
	unsigned short s_port;
	unsigned short d_port;
	/* In case not 4 byte alignment will cause data abort */
	unsigned short seq1;
	unsigned short seq2;
	unsigned short ack_seq1;
	unsigned short ack_seq2;
	unsigned short
		res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		ece:1,
		cwr:1;
	unsigned short window;
	unsigned short checksum;
	unsigned short urg_ptr;
};

struct udp_hdr {
	unsigned short s_port;
	unsigned short d_port;
	unsigned short length;
	unsigned short checksum;
};

#define MAX_PAYLOAD 65536
#define ETH_IPV4 0x0800
#define ETH_IPV6 0x86DD
#define ETH_PTP  0x88F7
#define ETH_LEN 14

#define PROTO_TCP 6
#define PROTO_UDP 17

#define NEXTHDR_TCP PROTO_TCP
#define NEXTHDR_UDP PROTO_UDP
#define NEXTHDR_HOP 0
#define NEXTHDR_EXT_ROUTE 43
#define NEXTHDR_EXT_FRAGMENT 44
#define NEXTHDR_NO_NEXTHDR 59

enum {IPV4_CKSM = 0, IPV6_CKSM, IPV4_ERR_CKSM, IPV6_ERR_CKSM};

/* Level 2*/
extern void pack_ether(struct skb_buff *skb);
void pack_ether_hwaddr(struct skb_buff *skb, uint8_t *sa, uint8_t *da);
/* Level 3 IPv6 */
extern void pack_exthdr_route(struct skb_buff *skb, struct in6addr *r_addr, int num_addr, unsigned char seg_left);
extern void pack_exthdr_frag(struct skb_buff *skb);
extern void pack_exthdr_noexthdr(struct skb_buff *skb);
extern void pack_ipv6(struct skb_buff *skb, struct inet_addr *inetaddr);
extern void pack_ipv4(struct skb_buff *skb, struct inet_addr *inetaddr, unsigned char header_len);
/* Level 4 UDP */
extern void pack_udp(struct skb_buff *skb, uint32_t length, struct inet_addr *inetaddr, int cksm);
/* Level 4 TCP */
extern void pack_tcp(struct skb_buff *skb, uint32_t length, struct inet_addr *inetaddr, uint8_t header_len, int cksm);

#endif
