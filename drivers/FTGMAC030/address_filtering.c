#include "ftgmac030.h"

/* An address that matches MAC_ADR (08h ~ 0Fh) */
static uint8_t match_addr[6]               = {0x08, 0x08, 0x08, 0x08, 0x08, 0x08};
/* A multicast address which exists in hashtable (10h, 14h) */
static uint8_t multicast_hashtable_addr[6] = {0x07, 0x07, 0x07, 0x07, 0x07, 0x07};
/* A multicast address which does not exists in hash table */
static uint8_t multicast_addr[6]           = {0x09, 0x09, 0x09, 0x09, 0x09, 0x09};
/* A broadcast address */
static uint8_t broadcast_addr[6]           = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
/* A common address */
static uint8_t other_addr[6]               = {0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

/*
 * Terminated packet format:
 * DA(6-byte)-SA(6-byte)-TerminatedData(12-byte)
 */
static uint8_t terminated_data[12] = {	0x01, 0x12, 0x23, 0x34, 0xF5, 0xE6,
											0xD7, 0xC8, 0xB9, 0x3A, 0x7B, 0x2C};

static volatile uint32_t enter_isr = 0;
static volatile uint8_t g_rx_len = 0; /* a global rx length variable */
static volatile uint8_t g_rx_buff[RX_BUF_SIZE]; /* a global rx buffer pointer */

static volatile uint8_t *g_mac_addr = NULL;

#define EXCEPTADDR_NUM 2
static uint8_t except_addrs[EXCEPTADDR_NUM][6] = {
	{0x33, 0x33, 0x00, 0x00, 0x00, 0x01},
	{0x01, 0x00, 0x5e, 0x00, 0x00, 0xfc},
};

static int check_except_addrs(FTGMAC030_Control *ctrl)
{
	int i;
	for (i = 0; i < EXCEPTADDR_NUM; i ++) {
		if (0 == ftgmac030_addrcmp(ctrl->rx_buff, except_addrs[i])) {
			return 1;
		}
	}
	return 0;
}

static void wait_packet(uint8_t *mac_addr)
{
	g_mac_addr = mac_addr;
	prints("Wait for a packet which of header is %02x:%02x:%02x:%02x:%02x:%02x\n",
	mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	while(0 == enter_isr);
	enter_isr = 0;
	g_mac_addr = NULL;
}

static void wait_terminated_packet(FTGMAC030_Control *ctrl)
{
	int i;
	g_mac_addr = ctrl->get_mac_addr();
	prints("Wait for a terminated packet.\n");
	while(0 == enter_isr);
	enter_isr = 0;
	
	g_mac_addr = NULL;
	for(i = 0;i < 12;i++){
		if((g_rx_buff+12)[i] != terminated_data[i]){
			break;
		}
	}
	if(i < 12){
		prints("Received a packet which is not a terminated packet -> Failed!\n");
	}
}

static void filtering_isr(FTGMAC030_Control *ctrl)
{
	prints("Receive a packet which of header is %02x:%02x:%02x:%02x:%02x:%02x\n",
	ctrl->rx_buff[0], ctrl->rx_buff[1], ctrl->rx_buff[2], 
	ctrl->rx_buff[3], ctrl->rx_buff[4], ctrl->rx_buff[5]);

	if(g_mac_addr){
		if (check_except_addrs(ctrl)) {
				prints("An except address!\n");
		}
		else {
			if(0 == ftgmac030_addrcmp(ctrl->rx_buff, (uint8_t *)g_mac_addr)){
				enter_isr = 1;
				memcpy((void *)g_rx_buff, ctrl->rx_buff, ctrl->rx_len);
				g_rx_len = ctrl->rx_len;
				prints(" -> Passed!\n");
			}
			else{
				prints(" -> Failed!\n");
			}
		}
	}
}

void filtering_test_main(FTGMAC030_Control *ctrl)
{
	/*
	 * 1. group A: -receive packets which of destination address matches MAC_ADR (08h ~ 0Fh)
	 * 2. group B: -receive packets which of destination address matches MAC_ADR (08h ~ 0Fh)
	 *             -receive packets which of destination address is a multicast address and exists in hash table
	 * 3. group C: -receive packets which of destination address matches MAC_ADR (08h ~ 0Fh)
	 *             -receive packets which of destination address is a broadcast address
	 * 4. group D: -receive packets which of destination address matches MAC_ADR (08h ~ 0Fh)
	 *             -receive packets which of destination address is a multicast address and exists in hash table
	 *             -receive packets which of destination address is a broadcast address
	 * 5. group E: -receive packets which of destination address matches MAC_ADR (08h ~ 0Fh)
	 *             -receive packets which of destination address is a multicast address
	 * 6. group F: -receive all packets
	 */
	prints("Enter Address Filtering test. Speed: 100 Mbps\n");
	prints("---1. Set FTGMAC030 as address group A.-----------------------------------------\n");
	/* open as default */
	ctrl->open(ctrl, filtering_isr);
	wait_packet(match_addr);
	wait_terminated_packet(ctrl);
	prints("---2. Set FTGMAC030 as address group B.-----------------------------------------\n");
	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_HT_EN, filtering_isr);
	ctrl->set_multicast_address(ctrl, multicast_hashtable_addr);
	wait_packet(match_addr);
	wait_packet(multicast_hashtable_addr);
	wait_terminated_packet(ctrl);
	prints("---3. Set FTGMAC030 as address group C.-----------------------------------------\n");
	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_BROADPKT, filtering_isr);
	wait_packet(match_addr);
	wait_packet(broadcast_addr);
	wait_terminated_packet(ctrl);
	prints("---4. Set FTGMAC030 as address group D.-----------------------------------------\n");
	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_HT_EN | FTGMAC030_MACCR_BROADPKT, filtering_isr);
	ctrl->set_multicast_address(ctrl, multicast_hashtable_addr);
	wait_packet(match_addr);
	wait_packet(multicast_hashtable_addr);
	wait_packet(broadcast_addr);
	wait_terminated_packet(ctrl);
	prints("---5. Set FTGMAC030 as address group E.-----------------------------------------\n");
	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_MULTIPKT, filtering_isr);
	wait_packet(match_addr);
	wait_packet(multicast_addr);
	wait_packet(multicast_hashtable_addr);
	wait_packet(broadcast_addr);
	wait_terminated_packet(ctrl);
	prints("---6. Set FTGMAC030 as address group F.-----------------------------------------\n");
	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_ALLADDR, filtering_isr);
	ctrl->set_multicast_address(ctrl, multicast_hashtable_addr);
	wait_packet(match_addr);
	wait_packet(multicast_addr);
	wait_packet(multicast_hashtable_addr);
	wait_packet(broadcast_addr);
	wait_packet(other_addr);
	wait_terminated_packet(ctrl);

	prints("Finish Address Filtering test.\n");
}
