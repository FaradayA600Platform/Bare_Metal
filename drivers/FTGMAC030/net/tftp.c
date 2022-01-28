/*
 * Copyright 1994, 1995, 2000 Neil Russell.
 * (See License)
 * Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 * Copyright 2011 Comelit Group SpA,
 *                Luca Ceresoli <luca.ceresoli@comelit.it>
 */

#include <stdlib.h>

#include "net_core.h"
#include "tftp.h"

/* defined at lib/cli.c */
extern ulong load_addr;

/* Well known TFTP port # */
#define WELL_KNOWN_PORT	69
/* Millisecs to timeout for lost pkt */
#define TIMEOUT		5000UL
#ifndef	CONFIG_NET_RETRY_COUNT
/* # of timeouts before giving up */
# define TIMEOUT_COUNT	10
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT * 2)
#endif
/* Number of "loading" hashes per line (for checking the image size) */
#define HASHES_PER_LINE	65

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_OACK	6

static ulong TftpTimeoutMSecs = TIMEOUT;
static int TftpTimeoutCountMax = TIMEOUT_COUNT;
static uint64_t time_start;   /* Record time we started tftp */

/*
 * These globals govern the timeout behavior when attempting a connection to a
 * TFTP server. TftpRRQTimeoutMSecs specifies the number of milliseconds to
 * wait for the server to respond to initial connection. Second global,
 * TftpRRQTimeoutCountMax, gives the number of such connection retries.
 * TftpRRQTimeoutCountMax must be non-negative and TftpRRQTimeoutMSecs must be
 * positive. The globals are meant to be set (and restored) by code needing
 * non-standard timeout behavior when initiating a TFTP transfer.
 */
ulong TftpRRQTimeoutMSecs = TIMEOUT;
int TftpRRQTimeoutCountMax = TIMEOUT_COUNT;

enum {
	TFTP_ERR_UNDEFINED           = 0,
	TFTP_ERR_FILE_NOT_FOUND      = 1,
	TFTP_ERR_ACCESS_DENIED       = 2,
	TFTP_ERR_DISK_FULL           = 3,
	TFTP_ERR_UNEXPECTED_OPCODE   = 4,
	TFTP_ERR_UNKNOWN_TRANSFER_ID  = 5,
	TFTP_ERR_FILE_ALREADY_EXISTS = 6,
};

static IPaddr_t TftpRemoteIP;
/* The UDP port at their end */
static int	TftpRemotePort;
/* The UDP port at our end */
static int	TftpOurPort;
static int	TftpTimeoutCount;
/* packet sequence number */
static ulong	TftpBlock;
/* last packet sequence number received */
static ulong	TftpLastBlock;
/* count of sequence number wraparounds */
static ulong	TftpBlockWrap;
/* memory offset due to wrapping */
static ulong	TftpBlockWrapOffset;
static int	TftpState;
#ifdef CONFIG_TFTP_TSIZE
/* The file size reported by the server */
static int	TftpTsize;
/* The number of hashes we printed */
static short	TftpNumchars;
#endif
#ifdef CONFIG_CMD_TFTPPUT
static int	TftpWriting;	/* 1 if writing, else 0 */
static int	TftpFinalBlock;	/* 1 if we have sent the last block */
#else
#define TftpWriting	0
#endif

#define STATE_SEND_RRQ	1
#define STATE_DATA	2
#define STATE_TOO_LARGE	3
#define STATE_BAD_MAGIC	4
#define STATE_OACK	5
#define STATE_RECV_WRQ	6
#define STATE_SEND_WRQ	7

/* default TFTP block size */
#define TFTP_BLOCK_SIZE		512
/* sequence number is 16 bit */
#define TFTP_SEQUENCE_SIZE	((ulong)(1<<16))

#define DEFAULT_NAME_LEN	(8 + 4 + 1)
static char default_filename[DEFAULT_NAME_LEN];

#ifndef CONFIG_TFTP_FILE_NAME_MAX_LEN
#define MAX_LEN 128
#else
#define MAX_LEN CONFIG_TFTP_FILE_NAME_MAX_LEN
#endif

static char tftp_filename[MAX_LEN];

/* 512 is poor choice for ethernet, MTU is typically 1500.
 * Minus eth.hdrs thats 1468.  Can get 2x better throughput with
 * almost-MTU block sizes.  At least try... fall back to 512 if need be.
 * (but those using CONFIG_IP_DEFRAG may want to set a larger block in cfg file)
 */
#ifdef CONFIG_TFTP_BLOCKSIZE
#define TFTP_MTU_BLOCKSIZE CONFIG_TFTP_BLOCKSIZE
#else
#define TFTP_MTU_BLOCKSIZE 1468
#endif

static unsigned short TftpBlkSize = TFTP_BLOCK_SIZE;
static unsigned short TftpBlkSizeOption = TFTP_MTU_BLOCKSIZE;

static inline void
store_block(int block, unchar *src, unsigned len)
{
	ulong offset = block * TftpBlkSize + TftpBlockWrapOffset;
	ulong newsize = offset + len;
#ifdef CONFIG_SYS_DIRECT_FLASH_TFTP
	int i, rc = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		/* start address in flash? */
		if (flash_info[i].flash_id == FLASH_UNKNOWN)
			continue;
		if (load_addr + offset >= flash_info[i].start[0]) {
			rc = 1;
			break;
		}
	}

	if (rc) { /* Flash is destination for this packet */
		rc = flash_write((char *)src, (ulong)(load_addr+offset), len);
		if (rc) {
			flash_perror(rc);
			net_set_state(NETLOOP_FAIL);
			return;
		}
	} else
#endif /* CONFIG_SYS_DIRECT_FLASH_TFTP */
	{
		(void)memcpy((void *)(load_addr + offset), src, len);
	}

	if (NetBootFileXferSize < newsize)
		NetBootFileXferSize = newsize;
}

/* Clear our state ready for a new transfer */
static void new_transfer(void)
{
	TftpLastBlock = 0;
	TftpBlockWrap = 0;
	TftpBlockWrapOffset = 0;
#ifdef CONFIG_CMD_TFTPPUT
	TftpFinalBlock = 0;
#endif
}

#ifdef CONFIG_CMD_TFTPPUT
/**
 * Load the next block from memory to be sent over tftp.
 *
 * @param block	Block number to send
 * @param dst	Destination buffer for data
 * @param len	Number of bytes in block (this one and every other)
 * @return number of bytes loaded
 */
static int load_block(unsigned block, unchar *dst, unsigned len)
{
	/* We may want to get the final block from the previous set */
	ulong offset = ((int)block - 1) * len + TftpBlockWrapOffset;
	ulong tosend = len;

	tosend = min(NetBootFileXferSize - offset, tosend);
	(void)memcpy(dst, (void *)(save_addr + offset), tosend);
	debug("%s: block=%d, offset=%ld, len=%d, tosend=%ld\n", __func__,
		block, offset, len, tosend);
	return tosend;
}
#endif

static void TftpSend(void);
static void TftpTimeout(void);

/**********************************************************************/

static void show_block_marker(void)
{
#ifdef CONFIG_TFTP_TSIZE
	if (TftpTsize) {
		ulong pos = TftpBlock * TftpBlkSize + TftpBlockWrapOffset;

		while (TftpNumchars < pos * 50 / TftpTsize) {
			putchar('#');
			TftpNumchars++;
		}
	} else
#endif
	{
		if (((TftpBlock - 1) % 10) == 0)
			putchar('#');
		else if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0)
			puts("\n\t ");
	}
}

/**
 * restart the current transfer due to an error
 *
 * @param msg	Message to print for user
 */
static void restart(const char *msg)
{
	prints("\n%s; starting again\n", msg);
}

/*
 * Check if the block number has wrapped, and update progress
 *
 * TODO: The egregious use of global variables in this file should be tidied.
 */
static void update_block_number(void)
{
	/*
	 * RFC1350 specifies that the first data packet will
	 * have sequence number 1. If we receive a sequence
	 * number of 0 this means that there was a wrap
	 * around of the (16 bit) counter.
	 */
	if (TftpBlock == 0) {
		TftpBlockWrap++;
		TftpBlockWrapOffset += TftpBlkSize * TFTP_SEQUENCE_SIZE;
		TftpTimeoutCount = 0; /* we've done well, reset thhe timeout */
	} else {
		show_block_marker();
	}
}

/* The TFTP get or put is complete */
static void tftp_complete(void)
{
	uint64_t t;
#ifdef CONFIG_TFTP_TSIZE
	/* Print hash marks for the last packet received */
	while (TftpTsize && TftpNumchars < 49) {
		putchar('#');
		TftpNumchars++;
	}
#endif
	t = ftgmac030_get_time();
	if ((t - time_start) > 0) {
		puts("\n\t ");	/* Line up with "Loading: " */
		prints(" Transfer Size is 0x%lx Bytes ", NetBootFileXferSize);
	}
	puts("\ndone\n");
	net_set_state(NETLOOP_SUCCESS);
}

static void
TftpSend(void)
{
	unchar *pkt;
	unchar *xp;
	int len = 0;
	ushort *s;

	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	pkt = NetTxPacket + NetEthHdrSize() + IP_UDP_HDR_SIZE;

	switch (TftpState) {
	case STATE_SEND_RRQ:
	case STATE_SEND_WRQ:
		xp = pkt;
		s = (ushort *)pkt;
#ifdef CONFIG_CMD_TFTPPUT
		*s++ = htons(TftpState == STATE_SEND_RRQ ? TFTP_RRQ :
			TFTP_WRQ);
#else
		*s++ = htons(TFTP_RRQ);
#endif
		pkt = (unchar *)s;
		strcpy((char *)pkt, tftp_filename);
		pkt += strlen(tftp_filename) + 1;
		strcpy((char *)pkt, "octet");
		pkt += 5 /*strlen("octet")*/ + 1;
		strcpy((char *)pkt, "timeout");
		pkt += 7 /*strlen("timeout")*/ + 1;
		sprintf((char *)pkt, "%lu", TftpTimeoutMSecs / 1000);
		debug("send option \"timeout %s\"\n", (char *)pkt);
		pkt += strlen((char *)pkt) + 1;
#ifdef CONFIG_TFTP_TSIZE
		pkt += sprintf((char *)pkt, "tsize%c%lu%c",
				0, NetBootFileXferSize, 0);
#endif
		/* try for more effic. blk size */
		pkt += sprintf((char *)pkt, "blksize%c%d%c",
				0, TftpBlkSizeOption, 0);
		len = pkt - xp;
		break;

	case STATE_OACK:
	case STATE_RECV_WRQ:
	case STATE_DATA:
		xp = pkt;
		s = (ushort *)pkt;
		s[0] = htons(TFTP_ACK);
		s[1] = htons(TftpBlock);
		pkt = (unchar *)(s + 2);
#ifdef CONFIG_CMD_TFTPPUT
		if (TftpWriting) {
			int toload = TftpBlkSize;
			int loaded = load_block(TftpBlock, pkt, toload);

			s[0] = htons(TFTP_DATA);
			pkt += loaded;
			TftpFinalBlock = (loaded < toload);
		}
#endif
		len = pkt - xp;
		break;

	case STATE_TOO_LARGE:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
			*s++ = htons(3);

		pkt = (unchar *)s;
		strcpy((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;

	case STATE_BAD_MAGIC:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(2);
		pkt = (unchar *)s;
		strcpy((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		break;
	}

	NetSendUDPPacket(NetServerEther, TftpRemoteIP, TftpRemotePort,
			 TftpOurPort, len);
}

#ifdef CONFIG_CMD_TFTPPUT
static void icmp_handler(unsigned type, unsigned code, unsigned dest,
			 IPaddr_t sip, unsigned src, unchar *pkt, unsigned len)
{
	if (type == ICMP_NOT_REACH && code == ICMP_NOT_REACH_PORT) {
		/* Oh dear the other end has gone away */
		restart("TFTP server died");
	}
}
#endif

static void
TftpHandler(unchar *pkt, unsigned dest, IPaddr_t sip, unsigned src,
	    unsigned len)
{
	ushort proto;
	ushort *s;
	int i;

	if (dest != TftpOurPort) {
		return;
	}
	if (TftpState != STATE_SEND_RRQ && src != TftpRemotePort &&
	    TftpState != STATE_RECV_WRQ && TftpState != STATE_SEND_WRQ)
		return;

	if (len < 2)
		return;
	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	s = (ushort *)pkt;
	proto = *s++;
	pkt = (unchar *)s;
	switch (ntohs(proto)) {

	case TFTP_RRQ:
		break;

	case TFTP_ACK:
#ifdef CONFIG_CMD_TFTPPUT
		if (TftpWriting) {
			if (TftpFinalBlock) {
				tftp_complete();
			} else {
				/*
				 * Move to the next block. We want our block
				 * count to wrap just like the other end!
				 */
				int block = ntohs(*s);
				int ack_ok = (TftpBlock == block);

				TftpBlock = (unsigned short)(block + 1);
				update_block_number();
				if (ack_ok)
					TftpSend(); /* Send next data block */
			}
		}
#endif
		break;

	default:
		break;

#ifdef CONFIG_CMD_TFTPSRV
	case TFTP_WRQ:
		debug("Got WRQ\n");
		TftpRemoteIP = sip;
		TftpRemotePort = src;
		TftpOurPort = 1024 + (get_timer(0) % 3072);
		new_transfer();
		TftpSend(); /* Send ACK(0) */
		break;
#endif

	case TFTP_OACK:
		debug("Got OACK: %s %s\n",
			pkt,
			pkt + strlen((char *)pkt) + 1);
		TftpState = STATE_OACK;
		TftpRemotePort = src;
		/*
		 * Check for 'blksize' option.
		 * Careful: "i" is signed, "len" is unsigned, thus
		 * something like "len-8" may give a *huge* number
		 */
		for (i = 0; i+8 < len; i++) {
			if (strcmp((char *)pkt+i, "blksize") == 0) {
				TftpBlkSize = (unsigned short)
					strtoul((char *)pkt+i+8, NULL,
						       10);
				debug("Blocksize ack: %s, %d\n",
					(char *)pkt+i+8, TftpBlkSize);
			}
#ifdef CONFIG_TFTP_TSIZE
			if (strcmp((char *)pkt+i, "tsize") == 0) {
				TftpTsize = strtoul((char *)pkt+i+6,
						    NULL, 10);
				debug("size = %s, %d\n",
					 (char *)pkt+i+6, TftpTsize);
			}
#endif
		}

#ifdef CONFIG_CMD_TFTPPUT
		if (TftpWriting) {
			/* Get ready to send the first block */
			TftpState = STATE_DATA;
			TftpBlock++;
		}
#endif
		TftpSend(); /* Send ACK or first data block */
		break;
	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = ntohs(*(ushort *)pkt);

		update_block_number();

		if (TftpState == STATE_SEND_RRQ)
			debug("Server did not acknowledge timeout option!\n");

		if (TftpState == STATE_SEND_RRQ || TftpState == STATE_OACK ||
		    TftpState == STATE_RECV_WRQ) {
			/* first block received */
			TftpState = STATE_DATA;
			TftpRemotePort = src;
			new_transfer();

			if (TftpBlock != 1) {	/* Assertion */
				prints("\nTFTP error: "
				       "First block is not block 1 (%ld)\n"
				       "Starting again\n\n",
					TftpBlock);
				break;
			}
		}

		if (TftpBlock == TftpLastBlock) {
			/*
			 *	Same block again; ignore it.
			 */
			break;
		}

		TftpLastBlock = TftpBlock;
		TftpTimeoutCountMax = TIMEOUT_COUNT;
		NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);

		store_block(TftpBlock - 1, pkt + 2, len);

		/*
		 *	Acknowledge the block just received, which will prompt
		 *	the remote for the next one.
		 */

		TftpSend();

		if (len < TftpBlkSize)
			tftp_complete();
		break;

	case TFTP_ERROR:
		prints("\nTFTP error: '%s' (%d)\n",
		       pkt + 2, ntohs(*(ushort *)pkt));

		switch (ntohs(*(ushort *)pkt)) {
		case TFTP_ERR_FILE_NOT_FOUND:
		case TFTP_ERR_ACCESS_DENIED:
			puts("Not retrying...\n");
			net_set_state(NETLOOP_FAIL);
			break;
		case TFTP_ERR_UNDEFINED:
		case TFTP_ERR_DISK_FULL:
		case TFTP_ERR_UNEXPECTED_OPCODE:
		case TFTP_ERR_UNKNOWN_TRANSFER_ID:
		case TFTP_ERR_FILE_ALREADY_EXISTS:
		default:
			puts("Starting again\n\n");
			break;
		}
		break;
	}
}


static void
TftpTimeout(void)
{
	if (++TftpTimeoutCount > TftpTimeoutCountMax) {
		restart("Retry count exceeded");
	} else {
		puts("T ");
		NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);
		if (TftpState != STATE_RECV_WRQ)
			TftpSend();
	}
}


void TftpStart(enum proto_t protocol)
{
	prints("TFTP blocksize = %i, timeout = %ld ms\n",
		TftpBlkSizeOption, TftpTimeoutMSecs);

	TftpRemoteIP = NetServerIP;
	if (BootFile[0] == '\0') {
		sprintf(default_filename, "%02lx%02lx%02lx%02lx.img",
			NetOurIP & 0xFF,
			(NetOurIP >>  8) & 0xFF,
			(NetOurIP >> 16) & 0xFF,
			(NetOurIP >> 24) & 0xFF);

		strncpy(tftp_filename, default_filename, MAX_LEN);
		tftp_filename[MAX_LEN-1] = 0;

		prints("*** Warning: no boot file name; using '%s'\n",
			tftp_filename);
	} else {
		char *p = strchr(BootFile, ':');

		if (p == NULL) {
			strncpy(tftp_filename, BootFile, MAX_LEN);
			tftp_filename[MAX_LEN-1] = 0;
		} else {
			TftpRemoteIP = string_to_ip(BootFile);
			strncpy(tftp_filename, p + 1, MAX_LEN);
			tftp_filename[MAX_LEN-1] = 0;
		}
	}

	prints("Using %s device\n", eth_get_name());
	prints("TFTP %s server %lx; our IP address is %lx",
#ifdef CONFIG_CMD_TFTPPUT
	       protocol == TFTPPUT ? "to" : "from",
#else
		"from",
#endif
		TftpRemoteIP, NetOurIP);

	/* Check if we need to send across this subnet */
	if (NetOurGatewayIP && NetOurSubnetMask) {
		IPaddr_t OurNet	= NetOurIP    & NetOurSubnetMask;
		IPaddr_t RemoteNet	= TftpRemoteIP & NetOurSubnetMask;

		if (OurNet != RemoteNet)
			prints("; sending through gateway %lx",
			       NetOurGatewayIP);
	}
	putchar('\n');

	prints("Filename '%s'.", tftp_filename);

	if (NetBootFileSize) {
		prints(" Size is 0x%x Bytes = ", NetBootFileSize<<9);
	}

	putchar('\n');
#ifdef CONFIG_CMD_TFTPPUT
	TftpWriting = (protocol == TFTPPUT);
	if (TftpWriting) {
		prints("Save address: 0x%lx\n", save_addr);
		prints("Save size:    0x%lx\n", save_size);
		NetBootFileXferSize = save_size;
		puts("Saving: *\b");
		TftpState = STATE_SEND_WRQ;
		new_transfer();
	} else
#endif
	{
		prints("Load address: 0x%lx\n", load_addr);
		puts("Loading: *\b");
		TftpState = STATE_SEND_RRQ;
	}

	time_start = ftgmac030_get_time();
	TftpTimeoutCountMax = TftpRRQTimeoutCountMax;

	NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);
	net_set_udp_handler(TftpHandler);
#ifdef CONFIG_CMD_TFTPPUT
	net_set_icmp_handler(icmp_handler);
#endif
	TftpRemotePort = WELL_KNOWN_PORT;
	TftpTimeoutCount = 0;
	/* Use a pseudo-random port unless a specific port is set */
	TftpOurPort = 1024 + (rand() % 3072);

	TftpBlock = 0;

	/* zero out server ether in case the server ip has changed */
	memset(NetServerEther, 0, 6);
	/* Revert TftpBlkSize to dflt */
	TftpBlkSize = TFTP_BLOCK_SIZE;
#ifdef CONFIG_TFTP_TSIZE
	TftpTsize = 0;
	TftpNumchars = 0;
#endif

	TftpSend();
}

#ifdef CONFIG_CMD_TFTPSRV
void
TftpStartServer(void)
{
	tftp_filename[0] = 0;

	prints("Using %s device\n", eth_get_name());
	prints("Listening for TFTP transfer on %x\n", NetOurIP);
	prints("Load address: 0x%lx\n", load_addr);

	puts("Loading: *\b");

	TftpTimeoutCountMax = TIMEOUT_COUNT;
	TftpTimeoutCount = 0;
	TftpTimeoutMSecs = TIMEOUT;
	NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);

	/* Revert TftpBlkSize to dflt */
	TftpBlkSize = TFTP_BLOCK_SIZE;
	TftpBlock = 0;
	TftpOurPort = WELL_KNOWN_PORT;

#ifdef CONFIG_TFTP_TSIZE
	TftpTsize = 0;
	TftpNumchars = 0;
#endif

	TftpState = STATE_RECV_WRQ;
	net_set_udp_handler(TftpHandler);
}
#endif /* CONFIG_CMD_TFTPSRV */
