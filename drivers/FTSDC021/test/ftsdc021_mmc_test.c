/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_mmc_test.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2020/1/10	Sanjin Liu	 Original FTSDC021 code
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021_test.h"

static uint32_t *packed_hdr;

extern uint8_t *write_buf;
extern uint8_t *read_buf;

/* Some EXT_CSD field require Vendor Command to write */
/* csd <rd|wr> [offset] [value] */
int32_t SDC_rw_ext_csd_cmd(int32_t argc, char *const argv[])
{
	uint32_t off, val;
	uint8_t *buf;

	if (cardInfo.CardType != MEMORY_CARD_TYPE_MMC &&
	    cardInfo.CardType != MEMORY_eMMC ) {
		printf(" Extended CSD only available at MMC card.\r\n");
		return 1;
	}

	if (argc < 2)
		return 1;

	if (strcmp(argv[1], "wr") == 0) {

		if (argc != 4)
			return 1;

		off = atoi(argv[2]);
		val = atoi(argv[3]);

		if (off > 191) {
			printf(" ERR## EXT_CSD cannot be modified over "
				    "offset 191.\r\n");
			return 1;
		}

		if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
			printf(" ERR## ... MMC: Get EXT-CSD from card is "
				    "failed\r\n");
			return 1;
		}

		/* Set CMDQ_MODE_EN to enable/disable command queue mode */
		if (off == 15 && cardInfo.EXT_CSD_MMC.CMDQ_SUPPORT == 0) {
			printf(" ERR## ... MMC: Command queuing is not "
				    "supported\r\n");
				return 1;
		}

		if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, off,
					val, 1)) {
			printf(" ERR## ... Set offset %d val 0x%x "
			    "Faild.\r\n", off, val);
			return 1;
		}

		/* Enable Command queue engine */
		if (off == 15) {
			if (val == 1)
				cqhci_enable();
			else
				cqhci_disable(1);
		}
	}
	else if (strcmp(argv[1], "rd") == 0 && argc == 2) {
		ftsdc021_mmc_read_ext_csd();
	}
	else if (strcmp(argv[1], "rd") == 0 && argc == 3) {
		if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
			printf(" ERR## ... MMC: Get EXT-CSD from card is "
				    "failed\r\n");
			return 1;
		}
		off = atoi(argv[2]);
		buf = (uint8_t *) &(cardInfo.EXT_CSD_MMC);
		printf(" Get EXT-CSD offset=%d value=0x%x\r\n", off,
			    buf[off]);
        }
	return 0;
}

/*cqe_en [0|1] */
int32_t SDC_CQE_enable(int32_t argc, char *const argv[])
{
	uint32_t enable;

	if (cardInfo.CardType != MEMORY_CARD_TYPE_MMC &&
	    cardInfo.CardType != MEMORY_eMMC &&
	    cardInfo.EXT_CSD_MMC.EXT_CSD_REV >= 8) {
		printf(" ERR## ... MMC: Command queuing is only supported "
			    "in eMMC v5.1 or later\r\n");
		return 1;
	}
	/* Set CMDQ_MODE_EN to enable/disable command queue mode */
	if (cardInfo.EXT_CSD_MMC.CMDQ_SUPPORT == 0) {
		printf(" ERR## ... MMC: Command queuing is not "
			    "supported\r\n");
		return 1;
	}

	if (argc == 2)
		enable = atoi(argv[1]) & 0x1;
	else
		enable = 1;

	if (!cardInfo.cqe_en && enable == 1) {
		ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 1, 1);
		cqhci_enable();
	}
	else if (cardInfo.cqe_en && !enable) {
		cqhci_disable(1);
		ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 0, 1);
	}
	return 0;
}

/* enable packed commands test */
static uint32_t packed_test_en = 0;
int32_t SDC_packed_test(int32_t argc, char *const argv[])
{
	uint8_t off, val;

	if (cardInfo.CardType != MEMORY_CARD_TYPE_MMC &&
	    cardInfo.CardType != MEMORY_eMMC &&
	    cardInfo.EXT_CSD_MMC.EXT_CSD_REV >= 6) {
		printf(" Packed commands are only supported in eMMC v4.5 "
			    "or later\r\n");
		return 1;
	}

	if (cardInfo.EXT_CSD_MMC.MAX_PACKED_READS >= 5 &&
	    cardInfo.EXT_CSD_MMC.MAX_PACKED_WRITES >= 3) {
		ftsdc021_mmc_read_ext_csd();
		off = EXT_CSD_EXCEPTION_EVENTS_CTRL;
		val = cardInfo.EXT_CSD_MMC.EXCEPTION_EVENTS_CTRL[0] |
		      EXT_CSD_EXP_PACKED_EN;
		if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, off,
					val, 1)) {
			printf(" ERR## ... enable packed event Failed.\r\n");
			return 1;
		}
		else {
			uint32_t hdr_sz = cardInfo.EXT_CSD_MMC.DATA_SECTOR_SIZE ?
					4096 : 512;
			packed_test_en = 1;
			packed_hdr = (uint32_t *)write_buf;
			write_buf = (uint8_t*)(packed_hdr + hdr_sz /
					     sizeof(uint32_t));
		}
	}
	else {
		printf(" Max packed reads must >= 5 and max packed writes "
			    "must >= 3\r\n");
		return 1;
	}

	//Do auto burnIn test.
	cardInfo.FlowSet.autoCmd = 2;
	infinite_mode = 0;
	adma2_insert_nop = 0;
	adma2_use_interrupt = 0;

	ftsdc021_set_transfer_type(ADMA, 0, 3);
	SDC_burnin_speed_and_width(3, 3, 0, 0);
	return 0;
}

/* bootSz <size> */
int32_t SDC_boot_size_cmd(int32_t argc, char *const argv[])
{
	uint32_t size;
	clock_t t0;
	
	if (argc < 2 || argc > 3) {
		return 1;
	}
	
	if(atoi(argv[1]) <= 0){
		return 1;
	}
	else{
		size = atoi(argv[1]);
	}
	
	/* Send CMD62(vendor defined) to change boot partition size */
	ftsdc021_mmc_send_vendor_cmd(0xEFAC62EC);
	ftsdc021_mmc_send_vendor_cmd(0x00CBAEA7);
	
	/* Boot area setting
	 *  Formula: 
	 *  Argument (boot size) = Number of Super block for boot partition / 2
	 *  We set the 5 to argument for setting the 10 super blocks for boot
	 *  partition. According to the flash data sheet, the size of each super
	 *  block is 2MB. Therefore, the total size of boot partition is 20MB
	 *  (10 super blocks * 2MB/per super block). */
	ftsdc021_mmc_send_vendor_cmd(size);
	
	/* Wait the busy bit is clear */ 
	t0 = clock();
	while (clock() - t0 < 100);
	
	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card is "
			    "FAILED\r\n");
		return 1;
	}
	
	cardInfo.numOfBootBlocks = (cardInfo.EXT_CSD_MMC.Reserved6 << 8 |
				    cardInfo.EXT_CSD_MMC.BOOT_SIZE_MULTI)
				   * 128 * 1024;
	printf(" The size of boot partition is %d bytes ",
		    cardInfo.numOfBootBlocks);

	/* Each blocks has 512 bytes */
	cardInfo.numOfBootBlocks >>= 9;
	printf("(%d Blocks).\r\n", cardInfo.numOfBootBlocks);
	
	return 0;
}

/* bootPart <0-2, 7> */
int32_t SDC_boot_part_cmd(int32_t argc, char *const argv[])
{
	uint32_t partition;

	if (argc != 2) 
		return 1;
	
	partition = atoi(argv[1]) & 0x7;

	if((partition >  2) && (partition <  7))
		return 1;
	
	/* Enable Boot partition */
	if (!ftsdc021_mmc_set_bootmode(partition))
		printf(" Enable Boot partition %d success.\r\n", partition);

	return 0;
}

/* bootAck <0|1> */
int32_t SDC_boot_ack_cmd(int32_t argc, char *const argv[])
{
	uint32_t ack;

	if (argc != 2)
		return 1;

	ack = atoi(argv[1]) & 0x1;

	if (ack >  2)
		return 1;

	/* Set Boot Ack */
	if (!ftsdc021_mmc_set_bootack(ack))
		printf(" Set Boot ack %d success.\r\n", ack);

	return 0;
}

/* bootrd <addr> <blkcnt> [repeat] */
int32_t SDC_boot_read_cmd(int32_t argc, char *const argv[])
{
	uint32_t i, addr, cnt, repeat;
	uint8_t part;
	clock_t t0, t1;

	if (argc < 3) {
		return 1;
	}

	addr = atoi(argv[1]);
	cnt = atoi(argv[2]);

	if (addr + cnt > cardInfo.numOfBootBlocks) {
		/* For FPGA verification purpose, just give warning and allow
		 * the operation. */
		printf(" WARN## ...  Out of Range (%d + %d) > %d.\r\n", addr,
			    cnt, cardInfo.numOfBootBlocks);
	}

	if (argc == 4)
		repeat = atoi(argv[3]);
	else
		repeat = 1;
		
	/* Send CMD6 to set the PARTITION_ACCESS bit in the 179th byte of
	 * EXT_CSD to access boot partition */
	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card is "
			    "FAILED\r\n");
		return 1;
	}
	part = (cardInfo.EXT_CSD_MMC.PARTITION_CONF >> 3) & 7;
	printf(" Setting Partition Access for boot partition %d.\r\n",
		    part);
	ftsdc021_mmc_set_partition_access(part);

	/* Check if we have enough buffer */
	if ((cnt * (1 << rd_bl_len)) > FTSDC021_BUFFER_LENGTH)
		cnt = FTSDC021_BUFFER_LENGTH / (1 << rd_bl_len);

	/* CMD 16 */
	ftsdc021_ops_set_blocklen(&cardInfo, (1 << rd_bl_len));

	for (i = 0 ; i < repeat; i++) {
		memset(read_buf, 0, cnt * (1 << rd_bl_len));

		printf(" Read %d blocks from address %d ", cnt, addr);

		t0 = clock();
		if (!ftsdc021_card_read(addr, cnt, read_buf)) {
			t1 = clock();
			printf(" success. (%d ms)\r\n", (t1 - t0));
		} else
			printf(" FAILED.\r\n");

		addr += cnt;
	}
	
	/* Send CMD6 to set the PARTITION_ACCESS bit in the 179th byte of
	 * EXT_CSD to access boot partition*/
	ftsdc021_mmc_set_partition_access(0);

	return 0;
}

/* bootwr <addr> <blkcnt> [repeat] [compare] */
int32_t SDC_boot_write_cmd(int32_t argc, char *const argv[])
{
	uint32_t i, addr, cnt, repeat;
	uint8_t part;
	clock_t t0, t1;

	if (argc < 3) {
		return 1;
	}

	addr = atoi(argv[1]);
	cnt = atoi(argv[2]);

	if (addr + cnt > cardInfo.numOfBootBlocks) {
		/* For FPGA verification purpose, just give warning and allow
		 * the operation. */
		printf(" WARN## ...  Out of Range (%d + %d) > %d.\r\n",
			    addr, cnt, cardInfo.numOfBootBlocks);
		return 0;
	}

	/* Check if we have enough buffer */
	if ((cnt * (1 << wr_bl_len)) > FTSDC021_BUFFER_LENGTH)
		cnt = FTSDC021_BUFFER_LENGTH / (1 << wr_bl_len);

	SDC_PrepareWB(0);

	if (argc > 3)
		repeat = atoi(argv[3]);
	else
		repeat = 1;
		
	/* Send CMD6 to set the PARTITION_ACCESS bit in the 179th byte of
	 * EXT_CSD to access boot partition*/
	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card is "
			    "FAILED\r\n");
		return 1;
	}
	part = (cardInfo.EXT_CSD_MMC.PARTITION_CONF >> 3) & 7;
	printf(" Setting Partition Access for boot partition %d.\r\n", part);
	ftsdc021_mmc_set_partition_access(part);
	
	/* send CMD16 for (1 << wr_bl_len) Bytes fixed block length */
	ftsdc021_ops_set_blocklen(&cardInfo, (1 << wr_bl_len));

	for (i=0 ; i < repeat; i++) {

		printf(" Write %d blocks to address %d, ", cnt, addr);

		if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000)) 
			return 1;

		t0 = clock();
		if (ftsdc021_card_write(addr, cnt, write_buf)) {
			printf("FAILED.\r\n");
			if (cardInfo.FlowSet.UseDMA == ADMA)
				SDC_ShowDscpTbl(1, 0);
			return 0;
		}
		t1 = clock();
		if (argc == 5) {
			memset(read_buf, 0, cnt * (1 << rd_bl_len));

			if (ftsdc021_card_read(addr, cnt, read_buf)) {
				printf(", read back FAILED.\r\n");

				if (cardInfo.FlowSet.UseDMA == ADMA)
					SDC_ShowDscpTbl(1, 0);
				return 0;
			}

			if ((cnt << rd_bl_len) >
			    SDC_data_check(cnt, (uint32_t *)write_buf,
					   (uint32_t *)read_buf)) {
				return 0;
			}
			printf(", compare success. (%d ms)\r\n", (t1 - t0));
		} else {
			printf("success. (%d ms)\r\n", (t1 - t0));
		}

		addr += cnt;
	}
	
	/* Send CMD6 to clear the PARTITION_ACCESS bit in the 179th byte of
	 * EXT_CSD*/
	ftsdc021_mmc_set_partition_access(0);

	return 0;
}

/** 
 * This will boot from Boot partition 1 or 2. Depend on
 * previous settings.
 * Make sure "image" already written to these partitions.
 * Please refer to User-Guide.
 *
 * Linux Image (~8MB) = 16510 blocks.
 * u-boot Image (110 KB) = 230 blocks.
 *
 * go <norm|alt> <ack> <ds|hs> <1|4|8> <cnt>
 */
typedef void entrypoint_fn(void);

entrypoint_fn *image_start;

int32_t SDC_go_boot_image(int32_t argc, char *const argv[])
{
	uint8_t boot, ack, boot_spd, boot_wd;
	uint32_t blockNr;

	if (argc != 6)
		return 1;
	
	if (strcmp(argv[1], "alt") == 0)
		boot = 2;
	else
		boot = 1;

	ack = atoi(argv[2]);

	/* Change this value to suit your image size*/

	if (strcmp(argv[3], "ds") == 0)
		boot_spd = 0;
	else if (strcmp(argv[3], "hs") == 0)
		boot_spd = 1;
	else {
		printf(" WARN## ...  Invalid speed. We use normal speed instead.\r\n");
		boot_spd = 0;
	}

	boot_wd = atoi(argv[4]);
	switch(boot_wd) {
		default:
			printf(" WARN## ...  Invalid Bus width. We use 1-bit instead.\r\n");
		case 1:
			boot_wd = 0;
			break;
		case 4:
			boot_wd = 1;
			break;
		case 8:
			boot_wd = 2;
			break;
	}

	blockNr = atoi(argv[5]);

	printf(" Entering boot mode %s %d blocks ...\r\n",
		    (boot==1) ? "Normal": "Alternate", blockNr);
	if (ftsdc021_mmc_boot(boot, ack, boot_spd, boot_wd, (uint32_t *)read_buf,
			      blockNr)) {
		printf(" Boot Mode FAILED.\r\n");
		return 1;
	}

	/* setup bank 1 flash */    
	/*
	*((unsigned int *)0x90200008) = 0x10400052;
	*((unsigned int *)0x9020000C) = 0x000FF3FF;
	*/

	/* Never return */
	printf(" Jump to boot code (0x%08p)\r\n", read_buf);
	image_start = (void *)read_buf;
	image_start();

	/* Just to avoid warning */
	return 0;
}

int32_t SDC_write_protection_cmd(int32_t argc, char *const argv[])
{
	uint32_t wp_buf[2], ret, addr;

	if ((cardInfo.CardType != MEMORY_CARD_TYPE_MMC) && (cardInfo.CardType != MEMORY_eMMC)) {
		printf("Write protection test needs the eMMC card\r\n");
		return 1;
	}

	if (argc != 3) {
		return 1;
	}

	addr = atoi(argv[2]);

	if (strcmp(argv[1], "set") == 0) {
		ret = ftsdc021_mmc_set_write_prot(&cardInfo, addr);
		printf("Set protection bits! respLo=%x\r\n", (uint32_t) cardInfo.respLo);
	}
	else if (strcmp(argv[1], "clr") == 0) {
		ret = ftsdc021_mmc_clear_write_prot(&cardInfo, addr);
		printf("Clear protection bits! respLo=%x\r\n", (uint32_t) cardInfo.respLo);
	}
	else if (strcmp(argv[1], "send") == 0) {
		wp_buf[0] = 0;
		ret = ftsdc021_mmc_send_write_prot(&cardInfo, wp_buf, addr);
		printf("Protection bits (@0x%d respLo=%x) 0x%08x\r\n", addr,
			    (uint32_t) cardInfo.respLo, wp_buf[0]);
	}
	else if (strcmp(argv[1], "type") == 0) {
		wp_buf[0] = 0;
		wp_buf[1] = 0;
		ret = ftsdc021_mmc_send_write_prot_type(&cardInfo, wp_buf, addr);
		printf("Protection type (@0x%d respLo=%x) 0x%08x 0x%08x\r\n", addr,
			    (uint32_t) cardInfo.respLo, wp_buf[1], wp_buf[0]);
	}

	return 0;
}

int32_t SDC_reliable_write_test(int32_t argc, char *const argv[])
{
	uint32_t rc = 0;

	if (cardInfo.CardType != MEMORY_CARD_TYPE_MMC && cardInfo.CardType != MEMORY_eMMC &&
	    cardInfo.EXT_CSD_MMC.EXT_CSD_REV >= 5) {
		printf(" Reliable write is only supported in eMMC v4.41 or later\r\n");
		return 1;
	}

	cardInfo.reliable_write_en = 1;
	SDC_readWrite_burnin(&rc, 0xFFFF, 0, 0);
	return 0;
}

int32_t SDC_context_test(int32_t argc, char *const argv[])
{
	uint32_t max_context_id, i;
	uint8_t ori_value;

	if (cardInfo.CardType != MEMORY_CARD_TYPE_MMC && cardInfo.CardType != MEMORY_eMMC &&
	    cardInfo.EXT_CSD_MMC.EXT_CSD_REV >= 6) {
		printf(" Context is only supported in eMMC v4.5 or later\r\n");
		return 1;
	}

	max_context_id = cardInfo.EXT_CSD_MMC.CONTEXT_CAPABILITIES & 0xf;
	printf(" Enable context management. (max_context_id=%d)\r\n", max_context_id);
	if (cardInfo.cqe_en && cardInfo.cqe_on) cqhci_off(1);
	/* Enable all context */
	for (i = 0; i < max_context_id; i++) {
		ori_value = cardInfo.EXT_CSD_MMC.CONTEXT_CONF[i];
		//Set CONTEXT_CONF
		if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL,
					37 + i * 8, ori_value | 0x3, 1)) {
			printf(" ERR## ... activate contex %d Failed.\r\n", i + 1);
			return 1;
		}
	}
	cardInfo.max_context_id = max_context_id;
	if (cardInfo.cqe_en && !cardInfo.cqe_on) cqhci_on();

	return 0;
}

/*
power off notification for eMMC
power <off|sleep> [short|long]
ex. power off
    power off short
    power off log
    power sleep
*/
int32_t SDC_power_off(int32_t argc, char *const argv[])
{
	uint8_t off, val, pwr;
	uint32_t timeout, ret, clock;

	if (argc < 2) {
		return 1;
	}

	pwr = host.power;
	clock = host.clock;
	off = EXT_CSD_POWER_OFF_NOTIFICATION;
	//Check if POWER_OFF_NOTIFICATION is enabled.
	if (!cardInfo.EXT_CSD_MMC.POWER_OFF_NOTIFICATION) {
		if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, off, 1, 1)) {
			printf("Can not set POWER_OFF_NOTIFICATION to 1.\r\n");
			return 1;
		}
	}

	if (strcmp(argv[1], "off") == 0) {
		if (argv[2] && strcmp(argv[2], "long") == 0) {
			//Cal power off long timeout
			timeout = cardInfo.EXT_CSD_MMC.POWER_OFF_LONG_TIME * 10;
			//POWER_OFF_LONG
			val = 3;
		}
		else {
			//POWER_OFF_SHORT
			val = 2;
		}
		if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, off, val, 0)) {
			printf("Can not set POWER_OFF_NOTIFICATION to %d.\r\n", val);
			return 1;
		}

		/* Spec says POWER_OFF_NOTIFICATION has changed to either 0x02 or 0x03,
		 * host may safely power off the device. But in Samsung doesn't change.
		 * Trace Linux code, there are no POWER_OFF_NOTIFICATION checking codes
		 * after set POWER_OFF_NOTIFICATION. So, mark the checking codes.
		if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
			printf(" ERR## ... MMC: Get EXT-CSD from card is failed\r\n");
			return 1;
		}

		if (cardInfo.EXT_CSD_MMC.POWER_OFF_NOTIFICATION != val) {
			printf("Failed to set POWER_OFF_NOTIFICATION to %d(%d).\r\n", val,
				    cardInfo.EXT_CSD_MMC.POWER_OFF_NOTIFICATION);
			return 1;
		}
		*/
		ftsdc021_SetPower(-1);
	}
	else if (strcmp(argv[1], "sleep") == 0) {
		timeout = (1 << cardInfo.EXT_CSD_MMC.SLEEP_NOTIFICATION_TIME) * 10 / 1000;
		if (timeout == 0)
			timeout = 1;
		val = 4;
		if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, off, val, 0)) {
			printf("Can not set POWER_OFF_NOTIFICATION to %d.\r\n", val);
			return 1;
		}

		/* Spec says POWER_OFF_NOTIFICATION has changed to either 0x02 or 0x03,
		 * host may safely power off the device. But in Samsung doesn't change.
		 * Trace Linux code, there are no POWER_OFF_NOTIFICATION checking codes
		 * after set POWER_OFF_NOTIFICATION. So, mark the checking codes.
		if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
			printf(" ERR## ... MMC: Get EXT-CSD from card is failed\r\n");
			return 1;
		}

		if (cardInfo.EXT_CSD_MMC.POWER_OFF_NOTIFICATION != val) {
			printf("Enter sleep failed.\r\n");
			return 1;
		}
		*/

		// stop clock
		ftsdc021_SetSDClock(clock);
		// Set power control
		ftsdc021_SetPower(-1);
		printf("Enter sleep ok, press 'q' to resume.\r\n");

		while (1) {
			if ('q' == getchar())
				break;
		}
		ftsdc021_HCReset(SDHCI_SOFTRST_ALL);
		ftsdc021_scan_cards();
		ftsdc021_set_bus_width(4);
		ftsdc021_set_bus_speed_mode(1);
	}
	return 0;
}

/*
RPMB test for eMMC4.4 later
The key can be set only once. We use 32 bytes 0s as the default key.
rpmb setkey
rpmb read <addr> <blkcnt>
rpmb write <addr> <count>
*/
int32_t SDC_rpmb_cmd(int32_t argc, char *const argv[])
{
	uint32_t addr, blkcnt, ret;

	if (cardInfo.CardType != MEMORY_eMMC) {
		printf("RPMB only support in eMMC\r\n");
		return 1;
	}
	if (argc < 2)
		return 1;

	//Enable RPMB partition access.
	ftsdc021_mmc_set_partition_access(3);
	cardInfo.RPMB_access = 1;

	if (strcmp(argv[1], "setkey") == 0) {
		if(mmc_rpmb_set_key(&cardInfo, rpmb_key))
			printf("Error: Key is already programmed ?\r\n");
		else
			printf("Success: Key is programmed and only "
				    "once. \r\n");
	}
	else if (strcmp(argv[1], "read") == 0) {
		if (argc < 4)
			return 1;
		addr = atoi(argv[2]);
		blkcnt = atoi(argv[3]);
		printf("RPMB: read command addr = %d, blkcnt = %d!\r\n",
			    addr, blkcnt);
		ret = mmc_rpmb_read(&cardInfo, read_buf, addr, blkcnt,
				    (uint8_t *)rpmb_key);
		if (ret) {
			printf("Error: read command error!\r\n");
			return 1;
		}
	}
	else if (strcmp(argv[1], "write") == 0) {
		if (argc < 4)
			return 1;
		addr = atoi(argv[2]);
		blkcnt = atoi(argv[3]);
		ret = mmc_rpmb_write(&cardInfo, read_buf, addr, blkcnt,
				     (uint8_t *)rpmb_key);
		if (ret) {
			printf("Error: write command error!\r\n");
			return 1;
		}
	}
}

/*
 * set_time <1|2|3> <time>
 * 1: Absolute time
 *	SECONDS_PASSED is the number of seconds that passed from January 1st,
 *	year 0 AD till the current time in UTC
 * 2: Set-base for relative time
 *	SECONDS_PASSED is ignored, and the time of sending the SET_TIME command
 *	with this parameter is set as a reference for future relative time
 * 3: Relative time
 *	SECONDS_PASSED is the number of seconds that passed since the most
 *	recent call to SET_TIME with the parameter RTC_INFO_TYPE=2
*/
int32_t SDC_set_time(int32_t argc, char *const argv[])
{
	if (cardInfo.CardType != MEMORY_eMMC) {
		printf("RTC only support in eMMC\r\n");
		return 1;
	}
	if (argc < 2)
		return 1;

	if (ftsdc021_mmc_set_time(&cardInfo, (uint8_t)atoi(argv[1]),
				  atol(argv[2])))
		printf("Setting eMMC RTC time is failed\r\n");
	else
		printf("Setting eMMC RTC time is succeeded\r\n");
}
