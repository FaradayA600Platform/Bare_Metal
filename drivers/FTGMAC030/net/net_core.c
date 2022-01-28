/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

/*
 * General Desription:
 *
 * The user interface supports commands for BOOTP, RARP, and TFTP.
 * Also, we support ARP internally. Depending on available data,
 * these interact as follows:
 *
 * BOOTP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *			- name of bootfile
 *	Next step:	ARP
 *
 * LINK_LOCAL:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *	Next step:	ARP
 *
 * RARP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *	Next step:	ARP
 *
 * ARP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *	We want:	- TFTP server ethernet address
 *	Next step:	TFTP
 *
 * DHCP:
 *
 *     Prerequisites:	- own ethernet address
 *     We want:		- IP, Netmask, ServerIP, Gateway IP
 *			- bootfilename, lease time
 *     Next step:	- TFTP
 *
 * TFTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *			- TFTP server ethernet address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * NFS:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * SNTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *	We want:	- network time
 *	Next step:	none
 */
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "net_core.h"
#include "arp.h"
#include "tftp.h"
#include "ping.h"

/** BOOTP EXTENTIONS **/

/* Our subnet mask (0=unknown) */
IPaddr_t	NetOurSubnetMask;
/* Our gateways IP address */
IPaddr_t	NetOurGatewayIP;
/* Our hostname */
char		NetOurHostName[32] = {0,};
/* Our bootpath */
char		NetOurRootPath[64] = {0,};
/* Our bootfile size in blocks */
ushort		NetBootFileSize;

/** END OF BOOTP EXTENTIONS **/

/* The actual transferred size of the bootfile (in bytes) */
uint32_t	NetBootFileXferSize;
/* Our ethernet address */
unchar		NetOurEther[6];
/* Boot server enet address */
unchar		NetServerEther[6];
/* Our IP addr (0 = unknown) */
IPaddr_t	NetOurIP;
/* Server IP addr (0 = unknown) */
IPaddr_t	NetServerIP;
/* Current receive packet */
unchar *NetRxPacket;
/* Current rx packet length */
int		NetRxPacketLen;
/* IP packet ID */
unsigned	NetIPID;
/* Ethernet bcast address */
unchar		NetBcastAddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unchar		NetEtherNullAddr[6];

/* Network loop state */
enum net_loop_state net_state;
/* Tried all network devices */
int		NetRestartWrap;
/* Network loop restarted */
static int	NetRestarted;
/* At least one device configured */
static int	NetDevExists;

/* XXX in both little & big endian machines 0xFFFF == ntohs(-1) */
/* default is without VLAN */
ushort		NetOurVLAN = 0xFFFF;
/* ditto */
ushort		NetOurNativeVLAN = 0xFFFF;

/* Boot File name */
char		BootFile[128];

static unchar PktBuf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN];

/* Receive packet */
unchar *NetRxPackets[PKTBUFSRX];

/* Current UDP RX packet handler */
static rxhand_f *udp_packet_handler;
/* Current ARP RX packet handler */
static rxhand_f *arp_packet_handler;
#ifdef CONFIG_CMD_TFTPPUT
/* Current ICMP rx handler */
static rxhand_icmp_f *packet_icmp_handler;
#endif
/* Current timeout handler */
static thand_f *timeHandler;
/* Time base value */
static uint64_t	timeStart;
/* Current timeout value */
static ulong	timeDelta;
/* THE transmit packet */
unchar *NetTxPacket;

static int net_check_prereq(enum proto_t protocol);

static int NetTryCount;

/**********************************************************************/

static void NetInitLoop(void)
{
	NetOurIP = string_to_ip("10.0.0.179");
	NetOurGatewayIP = string_to_ip("10.0.0.254");
	NetOurSubnetMask = string_to_ip("255.255.255.0");
	NetServerIP = string_to_ip("10.0.0.128");
	memcpy(NetOurEther, eth_get_dev()->enetaddr, 6);
}

static void net_clear_handlers(void)
{
	net_set_udp_handler(NULL);
	net_set_arp_handler(NULL);
	NetSetTimeout(0, NULL);
}

static void net_cleanup_loop(void)
{
	net_clear_handlers();
}

void net_init(void)
{
	static int first_call = 1;

	if (first_call) {
		/*
		 *	Setup packet buffers, aligned correctly.
		 */
		int i;

		NetTxPacket = &PktBuf[0] + (PKTALIGN - 1);
		NetTxPacket -= (ulong)NetTxPacket % PKTALIGN;
		for (i = 0; i < PKTBUFSRX; i++)
			NetRxPackets[i] = NetTxPacket + (i + 1) * PKTSIZE_ALIGN;

		ArpInit();
		net_clear_handlers();

		/* Only need to setup buffer pointers once. */
		first_call = 0;
	}

	NetInitLoop();
}

/**********************************************************************/
/*
 *	Main network processing loop.
 */

int NetLoop(enum proto_t protocol)
{
	int ret = -1;
	uint64_t t1;

	NetRestarted = 0;
	NetDevExists = 0;
	NetTryCount = 1;
	debug_cond(DEBUG_INT_STATE, "--- NetLoop Entry\n");

	net_init();

restart:
	net_set_state(NETLOOP_CONTINUE);

	/*
	 *	Start the ball rolling with the given start function.  From
	 *	here on, this code is a state machine driven by received
	 *	packets and timer events.
	 */
	debug_cond(DEBUG_INT_STATE, "--- NetLoop Init\n");
	NetInitLoop();

	switch (net_check_prereq(protocol)) {
	case 1:
	case 2:
		return -1;

	case 0:
		NetDevExists = 1;
		NetBootFileXferSize = 0;
		switch (protocol) {
		case TFTPGET:
#ifdef CONFIG_CMD_TFTPPUT
		case TFTPPUT:
#endif
			/* always use ARP to get server ethernet address */
			TftpStart(protocol);
			break;
		case PING:
			ping_start();
			break;
		default:
			break;
		}

		break;
	}

	/*
	 *	Main packet reception loop.  Loop receiving packets until
	 *	someone sets `net_state' to a state that terminates.
	 */
	for (;;) {
		/*
		 *	Check the ethernet for a new packet.  The ethernet
		 *	receive routine will process it.
		 */
		eth_rx();

		/*
		 *	Abort if q was pressed.
		 */
		if ('q' == uart_kbhit()) {
			/* cancel any ARP that may not have completed */
			NetArpWaitPacketIP = 0;

			net_cleanup_loop();

			prints("\nAbort\n");
			/* include a debug print as well incase the debug
			   messages are directed to stderr */
			debug_cond(DEBUG_INT_STATE, "--- NetLoop Abort!\n");
			goto done;
		}

		ArpTimeoutCheck();

		/*
		 *	Check for a timeout, and run the timeout handler
		 *	if we have one.
		 */
		t1 = ftgmac030_get_time();
		if (timeHandler && ((t1 - timeStart) > timeDelta)) {
			thand_f *x;

			debug_cond(DEBUG_INT_STATE, "--- NetLoop timeout\n");
			x = timeHandler;
			timeHandler = (thand_f *)0;
			(*x)();
		}


		switch (net_state) {

		case NETLOOP_RESTART:
			NetRestarted = 1;
			goto restart;

		case NETLOOP_SUCCESS:
			net_cleanup_loop();
			if (NetBootFileXferSize > 0) {
				prints("Bytes transferred = %ld (%lx hex)\n",
					NetBootFileXferSize,
					NetBootFileXferSize);
			}

			ret = NetBootFileXferSize;
			debug_cond(DEBUG_INT_STATE, "--- NetLoop Success!\n");
			goto done;

		case NETLOOP_FAIL:
			net_cleanup_loop();
			debug_cond(DEBUG_INT_STATE, "--- NetLoop Fail!\n");
			goto done;

		case NETLOOP_CONTINUE:
			continue;
		}
	}

done:
#ifdef CONFIG_CMD_TFTPPUT
	/* Clear out the handlers */
	net_set_udp_handler(NULL);
	net_set_icmp_handler(NULL);
#endif
	return ret;
}

/**********************************************************************/
/*
 *	Miscelaneous bits.
 */

static void dummy_handler(unchar *pkt, unsigned dport,
			IPaddr_t sip, unsigned sport,
			unsigned len)
{
}

rxhand_f *net_get_udp_handler(void)
{
	return udp_packet_handler;
}

void net_set_udp_handler(rxhand_f *f)
{
	debug_cond(DEBUG_INT_STATE, "--- NetLoop UDP handler set (%p)\n", f);
	if (f == NULL)
		udp_packet_handler = dummy_handler;
	else
		udp_packet_handler = f;
}

rxhand_f *net_get_arp_handler(void)
{
	return arp_packet_handler;
}

void net_set_arp_handler(rxhand_f *f)
{
	debug_cond(DEBUG_INT_STATE, "--- NetLoop ARP handler set (%p)\n", f);
	if (f == NULL)
		arp_packet_handler = dummy_handler;
	else
		arp_packet_handler = f;
}

#ifdef CONFIG_CMD_TFTPPUT
void net_set_icmp_handler(rxhand_icmp_f *f)
{
	packet_icmp_handler = f;
}
#endif

void
NetSetTimeout(uint32_t iv, thand_f *f)
{
	if (iv == 0) {
		debug_cond(DEBUG_INT_STATE,
			"--- NetLoop timeout handler cancelled\n");
		timeHandler = (thand_f *)0;
	} else {
		debug_cond(DEBUG_INT_STATE,
			"--- NetLoop timeout handler set (%p)\n", f);
		timeHandler = f;
		timeStart = ftgmac030_get_time();
		timeDelta = iv * COUNTS_PER_SECOND / 1000;
	}
}

int NetSendUDPPacket(unchar *ether, IPaddr_t dest, int dport, int sport,
		int payload_len)
{
	unchar *pkt;
	int eth_hdr_size;
	int pkt_hdr_size;

	/* make sure the NetTxPacket is initialized (NetInit() was called) */
//	assert(NetTxPacket != NULL);
	if (NetTxPacket == NULL)
		return -1;

	/* convert to new style broadcast */
	if (dest == 0)
		dest = 0xFFFFFFFF;

	/* if broadcast, make the ether address a broadcast and don't do ARP */
	if (dest == 0xFFFFFFFF)
		ether = NetBcastAddr;

	pkt = (unchar *)NetTxPacket;

	eth_hdr_size = NetSetEther(pkt, ether, PROT_IP);
	pkt += eth_hdr_size;
	net_set_udp_header(pkt, dest, dport, sport, payload_len);
	pkt_hdr_size = eth_hdr_size + IP_UDP_HDR_SIZE;

	/* if MAC address was not discovered yet, do an ARP request */
	if (memcmp(ether, NetEtherNullAddr, 6) == 0) {
		debug_cond(DEBUG_DEV_PKT, "sending ARP for %pI4\n", &dest);

		/* save the ip and eth addr for the packet to send after arp */
		NetArpWaitPacketIP = dest;
		NetArpWaitPacketMAC = ether;

		/* size of the waiting packet */
		NetArpWaitTxPacketSize = pkt_hdr_size + payload_len;

		/* and do the ARP request */
		NetArpWaitTry = 1;
		NetArpWaitTimerStart = ftgmac030_get_time();
		ArpRequest();
		return 1;	/* waiting */
	} else {
		debug_cond(DEBUG_DEV_PKT, "sending UDP to %pI4/%pM\n", &dest, ether);
		NetSendPacket(NetTxPacket, pkt_hdr_size + payload_len);
		return 0;	/* transmitted */
	}
}

#ifdef CONFIG_IP_DEFRAG
/*
 * This function collects fragments in a single packet, according
 * to the algorithm in RFC815. It returns NULL or the pointer to
 * a complete packet, in static storage
 */
#ifndef CONFIG_NET_MAXDEFRAG
#define CONFIG_NET_MAXDEFRAG 16384
#endif
/*
 * MAXDEFRAG, above, is chosen in the config file and  is real data
 * so we need to add the NFS overhead, which is more than TFTP.
 * To use sizeof in the internal unnamed structures, we need a real
 * instance (can't do "sizeof(struct rpc_t.u.reply))", unfortunately).
 * The compiler doesn't complain nor allocates the actual structure
 */
static struct rpc_t rpc_specimen;
#define IP_PKTSIZE (CONFIG_NET_MAXDEFRAG + sizeof(rpc_specimen.u.reply))

#define IP_MAXUDP (IP_PKTSIZE - IP_HDR_SIZE)

/*
 * this is the packet being assembled, either data or frag control.
 * Fragments go by 8 bytes, so this union must be 8 bytes long
 */
struct hole {
	/* first_byte is address of this structure */
	uint16_t last_byte;	/* last byte in this hole + 1 (begin of next hole) */
	uint16_t next_hole;	/* index of next (in 8-b blocks), 0 == none */
	uint16_t prev_hole;	/* index of prev, 0 == none */
	uint16_t unused;
};

static struct ip_udp_hdr *__NetDefragment(struct ip_udp_hdr *ip, int *lenp)
{
	static unchar pkt_buff[IP_PKTSIZE] __aligned(PKTALIGN);
	static uint16_t first_hole, total_len;
	struct hole *payload, *thisfrag, *h, *newh;
	struct ip_udp_hdr *localip = (struct ip_udp_hdr *)pkt_buff;
	unchar *indata = (unchar *)ip;
	int offset8, start, len, done = 0;
	uint16_t ip_off = ntohs(ip->ip_off);

	/* payload starts after IP header, this fragment is in there */
	payload = (struct hole *)(pkt_buff + IP_HDR_SIZE);
	offset8 =  (ip_off & IP_OFFS);
	thisfrag = payload + offset8;
	start = offset8 * 8;
	len = ntohs(ip->ip_len) - IP_HDR_SIZE;

	if (start + len > IP_MAXUDP) /* fragment extends too far */
		return NULL;

	if (!total_len || localip->ip_id != ip->ip_id) {
		/* new (or different) packet, reset structs */
		total_len = 0xffff;
		payload[0].last_byte = ~0;
		payload[0].next_hole = 0;
		payload[0].prev_hole = 0;
		first_hole = 0;
		/* any IP header will work, copy the first we received */
		memcpy(localip, ip, IP_HDR_SIZE);
	}

	/*
	 * What follows is the reassembly algorithm. We use the payload
	 * array as a linked list of hole descriptors, as each hole starts
	 * at a multiple of 8 bytes. However, last byte can be whatever value,
	 * so it is represented as byte count, not as 8-byte blocks.
	 */

	h = payload + first_hole;
	while (h->last_byte < start) {
		if (!h->next_hole) {
			/* no hole that far away */
			return NULL;
		}
		h = payload + h->next_hole;
	}

	/* last fragment may be 1..7 bytes, the "+7" forces acceptance */
	if (offset8 + ((len + 7) / 8) <= h - payload) {
		/* no overlap with holes (dup fragment?) */
		return NULL;
	}

	if (!(ip_off & IP_FLAGS_MFRAG)) {
		/* no more fragmentss: truncate this (last) hole */
		total_len = start + len;
		h->last_byte = start + len;
	}

	/*
	 * There is some overlap: fix the hole list. This code doesn't
	 * deal with a fragment that overlaps with two different holes
	 * (thus being a superset of a previously-received fragment).
	 */

	if ((h >= thisfrag) && (h->last_byte <= start + len)) {
		/* complete overlap with hole: remove hole */
		if (!h->prev_hole && !h->next_hole) {
			/* last remaining hole */
			done = 1;
		} else if (!h->prev_hole) {
			/* first hole */
			first_hole = h->next_hole;
			payload[h->next_hole].prev_hole = 0;
		} else if (!h->next_hole) {
			/* last hole */
			payload[h->prev_hole].next_hole = 0;
		} else {
			/* in the middle of the list */
			payload[h->next_hole].prev_hole = h->prev_hole;
			payload[h->prev_hole].next_hole = h->next_hole;
		}

	} else if (h->last_byte <= start + len) {
		/* overlaps with final part of the hole: shorten this hole */
		h->last_byte = start;

	} else if (h >= thisfrag) {
		/* overlaps with initial part of the hole: move this hole */
		newh = thisfrag + (len / 8);
		*newh = *h;
		h = newh;
		if (h->next_hole)
			payload[h->next_hole].prev_hole = (h - payload);
		if (h->prev_hole)
			payload[h->prev_hole].next_hole = (h - payload);
		else
			first_hole = (h - payload);

	} else {
		/* fragment sits in the middle: split the hole */
		newh = thisfrag + (len / 8);
		*newh = *h;
		h->last_byte = start;
		h->next_hole = (newh - payload);
		newh->prev_hole = (h - payload);
		if (newh->next_hole)
			payload[newh->next_hole].prev_hole = (newh - payload);
	}

	/* finally copy this fragment and possibly return whole packet */
	memcpy((unchar *)thisfrag, indata + IP_HDR_SIZE, len);
	if (!done)
		return NULL;

	localip->ip_len = htons(total_len);
	*lenp = total_len + IP_HDR_SIZE;
	return localip;
}

static inline struct ip_udp_hdr *NetDefragment(struct ip_udp_hdr *ip, int *lenp)
{
	uint16_t ip_off = ntohs(ip->ip_off);
	if (!(ip_off & (IP_OFFS | IP_FLAGS_MFRAG)))
		return ip; /* not a fragment */
	return __NetDefragment(ip, lenp);
}

#else /* !CONFIG_IP_DEFRAG */

static inline struct ip_udp_hdr *NetDefragment(struct ip_udp_hdr *ip, int *lenp)
{
	uint16_t ip_off = ntohs(ip->ip_off);
	if (!(ip_off & (IP_OFFS | IP_FLAGS_MFRAG)))
		return ip; /* not a fragment */
	return NULL;
}
#endif

/**
 * Receive an ICMP packet. We deal with REDIRECT and PING here, and silently
 * drop others.
 *
 * @parma ip	IP packet containing the ICMP
 */
static void receive_icmp(struct ip_udp_hdr *ip, int len,
			IPaddr_t src_ip, struct ethernet_hdr *et)
{
	struct icmp_hdr *icmph = (struct icmp_hdr *)&ip->udp_src;

	switch (icmph->type) {
	case ICMP_REDIRECT:
		if (icmph->code != ICMP_REDIR_HOST)
			return;
		prints(" ICMP Host Redirect to %pI4 ",
			&icmph->un.gateway);
		break;
	default:
		ping_receive(et, ip, len);
#ifdef CONFIG_CMD_TFTPPUT
		if (packet_icmp_handler)
			packet_icmp_handler(icmph->type, icmph->code,
				ntohs(ip->udp_dst), src_ip, ntohs(ip->udp_src),
				icmph->un.data, ntohs(ip->udp_len));
#endif
		break;
	}
}

void
NetReceive(unchar *inpkt, int len)
{
	struct ethernet_hdr *et;
	struct ip_udp_hdr *ip;
	IPaddr_t dst_ip;
	IPaddr_t src_ip;
	int eth_proto;
#if defined(CONFIG_CMD_CDP)
	int iscdp;
#endif
	ushort cti = 0, vlanid = VLAN_NONE, myvlanid, mynvlanid;

	debug_cond(DEBUG_NET_PKT, "packet received\n");

	NetRxPacket = inpkt;
	NetRxPacketLen = len;
	et = (struct ethernet_hdr *)inpkt;

	/* too small packet? */
	if (len < ETHER_HDR_SIZE)
		return;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;
	mynvlanid = ntohs(NetOurNativeVLAN);
	if (mynvlanid == (ushort)-1)
		mynvlanid = VLAN_NONE;

	eth_proto = ntohs(et->et_protlen);

	if (eth_proto < 1514) {
		struct e802_hdr *et802 = (struct e802_hdr *)et;
		/*
		 *	Got a 802.2 packet.  Check the other protocol field.
		 *	XXX VLAN over 802.2+SNAP not implemented!
		 */
		eth_proto = ntohs(et802->et_prot);

		ip = (struct ip_udp_hdr *)(inpkt + E802_HDR_SIZE);
		len -= E802_HDR_SIZE;

	} else if (eth_proto != PROT_VLAN) {	/* normal packet */
		ip = (struct ip_udp_hdr *)(inpkt + ETHER_HDR_SIZE);
		len -= ETHER_HDR_SIZE;

	} else {			/* VLAN packet */
		struct vlan_ethernet_hdr *vet =
			(struct vlan_ethernet_hdr *)et;

		debug_cond(DEBUG_NET_PKT, "VLAN packet received\n");

		/* too small packet? */
		if (len < VLAN_ETHER_HDR_SIZE)
			return;

		/* if no VLAN active */
		if ((ntohs(NetOurVLAN) & VLAN_IDMASK) == VLAN_NONE)
			return;

		cti = ntohs(vet->vet_tag);
		vlanid = cti & VLAN_IDMASK;
		eth_proto = ntohs(vet->vet_type);

		ip = (struct ip_udp_hdr *)(inpkt + VLAN_ETHER_HDR_SIZE);
		len -= VLAN_ETHER_HDR_SIZE;
	}

	debug_cond(DEBUG_NET_PKT, "Receive from protocol 0x%x\n", eth_proto);

	if ((myvlanid & VLAN_IDMASK) != VLAN_NONE) {
		if (vlanid == VLAN_NONE)
			vlanid = (mynvlanid & VLAN_IDMASK);
		/* not matched? */
		if (vlanid != (myvlanid & VLAN_IDMASK))
			return;
	}

	switch (eth_proto) {

	case PROT_ARP:
		ArpReceive(et, ip, len);
		break;

#ifdef CONFIG_CMD_RARP
	case PROT_RARP:
		rarp_receive(ip, len);
		break;
#endif
	case PROT_IP:
		debug_cond(DEBUG_NET_PKT, "Got IP\n");
		/* Before we start poking the header, make sure it is there */
		if (len < IP_UDP_HDR_SIZE) {
			debug("len bad %d < %lu\n", len,
				(ulong)IP_UDP_HDR_SIZE);
			return;
		}
		/* Check the packet length */
		if (len < ntohs(ip->ip_len)) {
			debug("len bad %d < %d\n", len, ntohs(ip->ip_len));
			return;
		}
		len = ntohs(ip->ip_len);
		debug_cond(DEBUG_NET_PKT, "len=%d, v=%02x\n", len, ip->ip_hl_v & 0xff);

		/* Can't deal with anything except IPv4 */
		if ((ip->ip_hl_v & 0xf0) != 0x40)
			return;
		/* Can't deal with IP options (headers != 20 bytes) */
		if ((ip->ip_hl_v & 0x0f) > 0x05)
			return;
		/* Check the Checksum of the header */
		if (!NetCksumOk((unchar *)ip, IP_HDR_SIZE / 2)) {
			debug("checksum bad\n");
			return;
		}
		/* If it is not for us, ignore it */
		dst_ip = NetReadIP(&ip->ip_dst);
		if (NetOurIP && dst_ip != NetOurIP && dst_ip != 0xFFFFFFFF) {
				return;
		}
		/* Read source IP address for later use */
		src_ip = NetReadIP(&ip->ip_src);
		/*
		 * The function returns the unchanged packet if it's not
		 * a fragment, and either the complete packet or NULL if
		 * it is a fragment (if !CONFIG_IP_DEFRAG, it returns NULL)
		 */
		ip = NetDefragment(ip, &len);
		if (!ip)
			return;
		/*
		 * watch for ICMP host redirects
		 *
		 * There is no real handler code (yet). We just watch
		 * for ICMP host redirect messages. In case anybody
		 * sees these messages: please contact me
		 * (wd@denx.de), or - even better - send me the
		 * necessary fixes :-)
		 *
		 * Note: in all cases where I have seen this so far
		 * it was a problem with the router configuration,
		 * for instance when a router was configured in the
		 * BOOTP reply, but the TFTP server was on the same
		 * subnet. So this is probably a warning that your
		 * configuration might be wrong. But I'm not really
		 * sure if there aren't any other situations.
		 *
		 * Simon Glass <sjg@chromium.org>: We get an ICMP when
		 * we send a tftp packet to a dead connection, or when
		 * there is no server at the other end.
		 */
		if (ip->ip_p == IPPROTO_ICMP) {
			receive_icmp(ip, len, src_ip, et);
			return;
		} else if (ip->ip_p != IPPROTO_UDP) {	/* Only UDP packets */
			return;
		}

		debug_cond(DEBUG_NET_PKT,
				"received UDP (to=%pI4, from=%pI4, len=%d)\n",
				&dst_ip, &src_ip, len);

#ifdef CONFIG_UDP_CHECKSUM
		if (ip->udp_xsum != 0) {
			ulong   xsum;
			ushort *sumptr;
			ushort  sumlen;

			xsum  = ip->ip_p;
			xsum += (ntohs(ip->udp_len));
			xsum += (ntohl(ip->ip_src) >> 16) & 0x0000ffff;
			xsum += (ntohl(ip->ip_src) >>  0) & 0x0000ffff;
			xsum += (ntohl(ip->ip_dst) >> 16) & 0x0000ffff;
			xsum += (ntohl(ip->ip_dst) >>  0) & 0x0000ffff;

			sumlen = ntohs(ip->udp_len);
			sumptr = (ushort *) &(ip->udp_src);

			while (sumlen > 1) {
				ushort sumdata;

				sumdata = *sumptr++;
				xsum += ntohs(sumdata);
				sumlen -= 2;
			}
			if (sumlen > 0) {
				ushort sumdata;

				sumdata = *(unsigned char *) sumptr;
				sumdata = (sumdata << 8) & 0xff00;
				xsum += sumdata;
			}
			while ((xsum >> 16) != 0) {
				xsum = (xsum & 0x0000ffff) +
				       ((xsum >> 16) & 0x0000ffff);
			}
			if ((xsum != 0x00000000) && (xsum != 0x0000ffff)) {
				prints(" UDP wrong checksum %08lx %08x\n",
					xsum, ntohs(ip->udp_xsum));
				return;
			}
		}
#endif

		/*
		 *	IP header OK.  Pass the packet to the current handler.
		 */
		(*udp_packet_handler)((unchar *)ip + IP_UDP_HDR_SIZE,
				ntohs(ip->udp_dst),
				src_ip,
				ntohs(ip->udp_src),
				ntohs(ip->udp_len) - UDP_HDR_SIZE);
		break;
	}
}


/**********************************************************************/

static int net_check_prereq(enum proto_t protocol)
{
	switch (protocol) {
		/* Fall through */
	case PING:
		if (NetPingIP == 0) {
			puts("*** ERROR: ping address not given\n");
			return 1;
		}
		goto common;
	case TFTPGET:
	case TFTPPUT:
		if (NetServerIP == 0) {
			puts("*** ERROR: `serverip' not set\n");
			return 1;
		}
common:
		if (NetOurIP == 0) {
			puts("*** ERROR: `ipaddr' not set\n");
			return 1;
		}

		if (memcmp(NetOurEther, "\0\0\0\0\0\0", 6) == 0) {
			puts("*** ERROR: `ethaddr' not set\n");
			return 2;
		}
		/* Fall through */
	default:
		return 0;
	}
	return 0;		/* OK */
}
/**********************************************************************/

int
NetCksumOk(unchar *ptr, int len)
{
	return !((NetCksum(ptr, len) + 1) & 0xfffe);
}


unsigned
NetCksum(unchar *ptr, int len)
{
	ulong	xsum;
	ushort *p = (ushort *)ptr;

	xsum = 0;
	while (len-- > 0)
		xsum += *p++;
	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);
	return xsum & 0xffff;
}

int
NetEthHdrSize(void)
{
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	return ((myvlanid & VLAN_IDMASK) == VLAN_NONE) ? ETHER_HDR_SIZE :
		VLAN_ETHER_HDR_SIZE;
}

int
NetSetEther(unchar *xet, unchar * addr, uint prot)
{
	struct ethernet_hdr *et = (struct ethernet_hdr *)xet;
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	memcpy(et->et_dest, addr, 6);
	memcpy(et->et_src, NetOurEther, 6);
	if ((myvlanid & VLAN_IDMASK) == VLAN_NONE) {
		et->et_protlen = htons(prot);
		return ETHER_HDR_SIZE;
	} else {
		struct vlan_ethernet_hdr *vet =
			(struct vlan_ethernet_hdr *)xet;

		vet->vet_vlan_type = htons(PROT_VLAN);
		vet->vet_tag = htons((0 << 5) | (myvlanid & VLAN_IDMASK));
		vet->vet_type = htons(prot);
		return VLAN_ETHER_HDR_SIZE;
	}
}

int net_update_ether(struct ethernet_hdr *et, unchar *addr, uint prot)
{
	ushort protlen;

	memcpy(et->et_dest, addr, 6);
	memcpy(et->et_src, NetOurEther, 6);
	protlen = ntohs(et->et_protlen);
	if (protlen == PROT_VLAN) {
		struct vlan_ethernet_hdr *vet =
			(struct vlan_ethernet_hdr *)et;
		vet->vet_type = htons(prot);
		return VLAN_ETHER_HDR_SIZE;
	} else if (protlen > 1514) {
		et->et_protlen = htons(prot);
		return ETHER_HDR_SIZE;
	} else {
		/* 802.2 + SNAP */
		struct e802_hdr *et802 = (struct e802_hdr *)et;
		et802->et_prot = htons(prot);
		return E802_HDR_SIZE;
	}
}

void net_set_ip_header(unchar *pkt, IPaddr_t dest, IPaddr_t source)
{
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)pkt;

	/*
	 *	Construct an IP header.
	 */
	/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_hl_v  = 0x45;
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(IP_FLAGS_DFRAG);	/* Don't fragment */
	ip->ip_ttl   = 255;
	ip->ip_sum   = 0;
	/* already in network byte order */
	NetCopyIP((void *)&ip->ip_src, &source);
	/* already in network byte order */
	NetCopyIP((void *)&ip->ip_dst, &dest);
}

void net_set_udp_header(unchar *pkt, IPaddr_t dest, int dport, int sport,
			int len)
{
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)pkt;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		pkt[IP_UDP_HDR_SIZE + len] = 0;

	net_set_ip_header(pkt, dest, NetOurIP);
	ip->ip_len   = htons(IP_UDP_HDR_SIZE + len);
	ip->ip_p     = IPPROTO_UDP;
	ip->ip_sum   = ~NetCksum((unchar *)ip, IP_HDR_SIZE >> 1);

	ip->udp_src  = htons(sport);
	ip->udp_dst  = htons(dport);
	ip->udp_len  = htons(UDP_HDR_SIZE + len);
	ip->udp_xsum = 0;
}

void copy_filename(char *dst, const char *src, int size)
{
	if (*src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && *src && (*src != '"'))
		*dst++ = *src++;
	*dst = '\0';
}

void ip_to_string(IPaddr_t x, char *s)
{
	x = ntohl(x);
	sprintf(s, "%d.%d.%d.%d",
		(int) ((x >> 24) & 0xff),
		(int) ((x >> 16) & 0xff),
		(int) ((x >> 8) & 0xff), (int) ((x >> 0) & 0xff)
	);
}

void VLAN_to_string(ushort x, char *s)
{
	x = ntohs(x);

	if (x == (ushort)-1)
		x = VLAN_NONE;

	if (x == VLAN_NONE)
		strcpy(s, "none");
	else
		sprintf(s, "%d", x & VLAN_IDMASK);
}

ushort string_to_VLAN(const char *s)
{
	ushort id;

	if (s == NULL)
		return htons(VLAN_NONE);

	if (*s < '0' || *s > '9')
		id = VLAN_NONE;
	else
		id = (ushort)strtoul(s, NULL, 10);

	return htons(id);
}
