#include <string.h>
#include "ftgmac030.h"
#include "net.h"

static const uint8_t MAC_SRC[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
static const uint8_t MAC_DST[6] = {0x02,0x02,0x02,0x02,0x02,0x02};

static volatile uint32_t enter_isr = 0;

static void wait_ipcksm_err(FTGMAC030_Control *ctrl)
{
	prints("\rWait for IP checksum error packet\n");

	while(0 == enter_isr);

	enter_isr = 0;

	if (FTGMAC030_RXDES1_PROTL_TYPE(ctrl->rxdes1) != FTGMAC030_RXDES1_PROTL_IP4)
		prints(" -> Failed! protocol type error! rxdes1: 0x%08lx\n", ctrl->rxdes1);

	if (ctrl->rx_err_happen && (ctrl->rxdes1 & FTGMAC030_RXDES1_IPCS_FAIL)) {
		prints(" -> Passed! rxdes1: 0x%08lx\n", ctrl->rxdes1);
	} else {
		prints(" -> Failed! rxdes1: 0x%08lx\n", ctrl->rxdes1);
	}
}

static void wait_udpcksm_err(FTGMAC030_Control *ctrl)
{
	prints("\rWait for UDP checksum error packet\n");

	while(0 == enter_isr);

	enter_isr = 0;

	if (FTGMAC030_RXDES1_PROTL_TYPE(ctrl->rxdes1) != FTGMAC030_RXDES1_PROTL_UDPIP)
		prints(" -> Failed! protocol type error! rxdes1: 0x%08lx\n", ctrl->rxdes1);

	if (ctrl->rx_err_happen && (ctrl->rxdes1 & FTGMAC030_RXDES1_UDPCS_FAIL)) {
		prints(" -> Passed! rxdes1: 0x%08lu\n", ctrl->rxdes1);
	} else {
		prints(" -> Failed! rxdes1: 0x%08lu\n", ctrl->rxdes1);
	}
}

static void wait_tcpcksm_err(FTGMAC030_Control *ctrl)
{
	prints("\rWait for TCP checksum error packet\n");

	while(0 == enter_isr);

	enter_isr = 0;

	if (FTGMAC030_RXDES1_PROTL_TYPE(ctrl->rxdes1) != FTGMAC030_RXDES1_PROTL_TCPIP)
		prints(" -> Failed! protocol type error! rxdes1: 0x%08lu\n", ctrl->rxdes1);

	if (ctrl->rx_err_happen && (ctrl->rxdes1 & FTGMAC030_RXDES1_TCPCS_FAIL)){
		prints(" -> Passed! rxdes1: 0x%08lu\n", ctrl->rxdes1);
	} else {
		prints(" -> Failed! rxdes1: 0x%08lu\n", ctrl->rxdes1);
	}
}

#define BUFSIZE 4096
static void wait_vlan(FTGMAC030_Control *ctrl, unsigned short vtag)
{
	prints("\rWait for vlan tag: 0x%04x\n", vtag);

	while(0 == enter_isr);

	enter_isr = 0;

	/* check vlan tag in RXDES1 */
	if (vtag != FTGMAC030_RXDES1_VLAN_TAGC(ctrl->rxdes1)) {
		prints("vtag error!\n");
		goto err;
	}

	/* check VLAN removal */
	if (ctrl->rx_buff[12] != 0x08 || ctrl->rx_buff[13] != 0x00) {
		prints("VLAN removal error!: 0x%02x%02x\n", ctrl->rx_buff[12], ctrl->rx_buff[13]);
		goto err;
	}
	prints(" -> Passed!\n");
	return;
err:
	prints(" -> Failed!\n");
	prints("rxdes1: 0x%08lu\n", ctrl->rxdes1);
}

static void rxdes1_isr(FTGMAC030_Control *ctrl)
{
	if(0 == ftgmac030_addrcmp(MAC_DST, ctrl->rx_buff) || ctrl->rx_err_happen){
		enter_isr = 1;
	}
}

void rxdes1_test_main(FTGMAC030_Control *ctrl)
{
	int i;

	prints("---Enter checksum error test.-----------------------------------------------\n");
	prints("---Please run host checksum error test.-------------------------------------\n");

	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_ALLADDR,
			 rxdes1_isr);
	wait_ipcksm_err(ctrl);
	wait_udpcksm_err(ctrl);
	wait_tcpcksm_err(ctrl);

	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_ALLADDR | FTGMAC030_MACCR_REMOVE_VLAN,
			 rxdes1_isr);
	for (i = 0; i < 15; i ++) {
		wait_vlan(ctrl, 
		FTGMAC030_RXDES1_VLAN_PRIO(i) |
		FTGMAC030_RXDES1_VLAN_CFI(i) |
		FTGMAC030_RXDES1_VLAN_VID(i));
	}
}
