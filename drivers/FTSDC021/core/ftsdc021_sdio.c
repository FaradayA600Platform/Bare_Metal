/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_sdio.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2019/12	Sanjin Liu	 Original FTSDC021 initial code for SDIO
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021.h"
#include "ftsdc021_sdio.h"

extern uint8_t *read_buf;

uint32_t ftsdc021_sdio_set_bus_width(SDCardInfo * card, uint32_t width)
{
	uint32_t err;
	uint8_t ctrl;

	err = ftsdc021_sdio_io_rw_direct(card, 0, 0, 0x7, 0, &ctrl);
	if (err)
		return err;

	/* Set 4 bit */
	if (width == 4) {
		ctrl |= 0x2;
	}

	err = ftsdc021_sdio_io_rw_direct(card, 1, 0, 0x7, ctrl, NULL);
	if (err)
		return err;

	return 0;
}

uint32_t ftsdc021_sdio_set_bus_speed(SDCardInfo * card, uint8_t speed)
{
	uint32_t err;
	uint8_t ctrl;

	err = ftsdc021_sdio_io_rw_direct(card, 0, 0, 0x13, 0, &ctrl);
	if (err)
		return err;

	/* Bit 0 Support High Speed, Bit 1: Enable High Speed */
	if (speed == 1) {
		if (!(ctrl & 0x1)) {
			printf("ERR:## ... High speed is not supported.\r\n");
			return 1;
		}

		ctrl |= 0x2;
	} else
		ctrl &= ~0x2;

	err = ftsdc021_sdio_io_rw_direct(card, 1, 0, 0x13, ctrl, NULL);
	if (err)
		return err;

	return 0;
}

uint32_t ftsdc021_sdio_read_cis(SDCardInfo *card, uint32_t fn)
{
	uint8_t resp;
	uint32_t ret, i, ptr = 0;

	/*
	 * Note that this works for the common CIS (function number 0) as
	 * well as a function's CIS * since SDIO_CCCR_CIS and SDIO_FBR_CIS
	 * have the same offset.
	 */
	for (i = 0; i < 3; i++) {
		ret = ftsdc021_sdio_io_rw_direct(card, 0, 0, fn * 0x100 + 0x09 + i, 0, &resp);
		if (ret) {
			printf("SDIO RW DIRECT: read CIS pointer failed.\r\n");
			return ret;
		}
		ptr |= resp << (i * 8);
	}

	printf("Function %d CIS pointer: 0x%x.\r\n", fn, ptr);
	do {
		uint8_t tpl_code, tpl_link, tpl_data;

		ret = ftsdc021_sdio_io_rw_direct(card, 0, 0, ptr++, 0, &tpl_code);
		if (ret) {
			printf("SDIO RW DIRECT: read TPL code failed.\r\n");
			break;
		}

		/* 0xff means we're done */
		if (tpl_code == 0xff)
			break;

		/* null entries have no link field or data */
		if (tpl_code == 0x00)
			continue;

		ret = ftsdc021_sdio_io_rw_direct(card, 0, 0, ptr++, 0, &tpl_link);
		if (ret) {
			printf("SDIO RW DIRECT: read TPL link failed.\r\n");
			break;
		}

		/* a size of 0xff also means we're done */
		if (tpl_link == 0xff)
			break;

		printf("\r\nTPL code: 0x%x, TPL link: 0x%x.\r\n", tpl_code, tpl_link);
		printf("TPL data:", tpl_code, tpl_link);
		for (i = 0; i < tpl_link; i++) {
			if (i % 4 == 0)
				printf("\r\n");
			ret = ftsdc021_sdio_io_rw_direct(card, 0, 0, ptr + i,
							 0, &tpl_data);
			if (ret) {
				printf("SDIO RW DIRECT: read TPL data failed.\r\n");
				break;
			}
			printf("%02x ", tpl_data);
		}
		ptr += tpl_link;
	} while (!ret);

	printf("\r\n");

	return ret;
}

uint32_t ftsdc021_sdio_Card_Info(SDCardInfo * card)
{
	uint8_t resp;
	uint8_t i = 0;
	uint32_t *data_from_cmd53;
	uint32_t err;

	/* CMD 52 */
	err = ftsdc021_sdio_io_rw_direct(card, 0, 0, 0x100, 0, &resp);
	if (err) {
		printf("SDIO RW DIRECT: address 0x100 failed.\r\n");
		return err;
	}

	switch (resp & 0xf) {
	case 0x0:
		printf("No SDIO standard interface supported by this function\r\n");
		break;

	case 0x1:
		printf("SDIO Standard UART\r\n");
		break;

	case 0x2:
		printf("SDIO Type-A for BT Standard\r\n");
		break;

	case 0x3:
		printf("SDIO Type-B for BT Standard\r\n");
		break;

	case 0x4:
		printf("SDIO GPS Standard\r\n");
		break;

	case 0x5:
		printf("SDIO Camera Standard\r\n");
		break;

	case 0x6:
		printf("SDIO PHS Standard\r\n");
		break;

	case 0x7:
		printf("SDIO WLAN Standard\r\n");
		break;

	case 0x8:
		printf("Embedded SDIO-ATA Standard\r\n");
		break;

	case 0xF:
		printf("Externed SDIO Standard\r\n");
		break;
	default:
		break;
	}

	// Show the SDIO Card Capability from the address 0x08 in function 0.
	err = ftsdc021_sdio_io_rw_direct(card, 0, 0, 0x8, 0, &resp);
	if (err) {
		printf("SDIO RW DIRECT: address 0x8 failed.\r\n");
		return err;
	}

	i = 0;
	printf("Card Capability:\r\n");
	do {
		printf("%s:", SDIO_capability[i].name);
		if (resp & (1 << i)) {
			SDIO_capability[i].support = 1;
		} else {
			SDIO_capability[i].support = 0;
		}
		printf("%d, ", SDIO_capability[i].support);
		i++;
	} while (i < 8);

	printf("\r\n\r\n");

	// Show the SDIO Card Bus Interface Control from the address 0x07 in function 0.
	err = ftsdc021_sdio_io_rw_direct(card, 0, 0, 0x7, 0, &resp);
	if (err) {
		printf("SDIO RW DIRECT: address 0x7 failed.\r\n");
		return err;
	}

	i = 0;
	printf("Bus Interface Control:\r\n");
	do {
		printf("%s:", SDIO_bus_interface_control[i].name);

		/* Bus width is located at [1:0] 2 bits */
		if (i == 0) {
			SDIO_bus_interface_control[0].support = 1 << (resp & 0x3);
			i += 5;
		} else {
			if (resp & (1 << i))
				SDIO_bus_interface_control[i].support = 1;
			else
				SDIO_bus_interface_control[i].support = 0;

			i++;
		}

		printf("%d, ", SDIO_bus_interface_control[0].support);
	} while (i < 8);

	printf("\r\n");

	/* Fetching the multiple bytes from beginning 64bytes of CCCR through
	 * CMD52 for comparing. */
	printf("CCCR from CMD52:\r\n");
	for (i = 0; i < 64; i++) {
		err = ftsdc021_sdio_io_rw_direct(card, 0, 0, i, 0, &resp);
		if (err) {
			printf("SDIO RW DIRECT: address 0x%x failed.\r\n", i);
			return err;
		}

		if (i % 4 == 0) {
			printf("0x");
		}

		printf("%x ", resp);

		if (i % 4 == 3) {
			printf("\r\n");
		}
	}
	printf("\r\n");

	/* Fetching the multiple bytes from beginning 64bytes of CCCR through
	 * CMD53. */
	data_from_cmd53 = (uint32_t *)read_buf;
	// CMD53 block mode
	/* CMD52 : Setting the Blocksize(4 bytes) for card if SMB feature in
	 * card capability is support. */
	err = ftsdc021_sdio_io_rw_direct(card, 1, 0, 0x10, 4, NULL);
	if (err)
		return err;

	memset(data_from_cmd53, 0, 64);
	err = ftsdc021_sdio_io_rw_extended(card, 0, 0, 0x00, 1,
					   (uint32_t *)data_from_cmd53, 16, 4);
	if (err)
		return err;

	printf("CCCR from CMD53(block mode):");
	for (i = 0; i < 16; i++) {
		if (i % 4 == 0)
			printf("\r\n 0x%08p:", &data_from_cmd53[i]);

		printf("0x%08x ", data_from_cmd53[i]);
	}
	printf("\r\n");

	// CMD53 byte mode
	memset(data_from_cmd53, 0, 64);
	err = ftsdc021_sdio_io_rw_extended(card, 0, 0, 0x00, 1,
					   (uint32_t *)data_from_cmd53, 1, 64);
	if (err)
		return err;

	printf("CCCR from CMD53(byte mode):");
	for (i = 0; i < 16; i++) {
		if (i % 4 == 0)
			printf("\r\n 0x%08p:", &data_from_cmd53[i]);

		printf("0x%08x ", data_from_cmd53[i]);
	}
	printf("\r\n\r\n");

	ftsdc021_sdio_read_cis(card, 0);
	ftsdc021_sdio_read_cis(card, 1);
	ftsdc021_sdio_read_cis(card, 2);

	return 1;
}

uint32_t ftsdc021_init_sdio(uint32_t ocr)
{
	uint32_t rocr, err = 0;

	if (ftsdc021_uhs_mode_support())
		ocr |= (1 << 24);
reinit:
	/* CMD5 error, is it SD card ? */
	err = ftsdc021_sdio_send_io_op_cond(&cardInfo, ocr, &rocr);
	if (err)
		return err;

	cardInfo.Num_IO_Func = (rocr >> 28) & 0x7;
	cardInfo.Memory_Present = (rocr >> 27) & 0x1;
	cardInfo.OCR = rocr;
	if (cardInfo.Memory_Present == 1) {
		cardInfo.CardType = MEMORY_SDIO_COMBO;
		printf("ERR## Combo card is not support\r\n");
		return 1;
	}
	printf("Found SDIO Card .... \r\n");

	/* Accept IO Signal Voltage switch to 1.8v */
	if (cardInfo.OCR & (1 << 24)) {
		err = ftsdc021_signal_voltage_switch();
		if (err) {
			printf("ERR## ... 1.8v Signal Enable Failed.\r\n");
			ocr &= ~(1 << 24);
			goto reinit;
		}
		printf("Switching to S18V IO level success. \r\n");
	} else {
		printf("SD/SDIO Card does not support S18V. \r\n");
	}

	err = ftsdc021_ops_send_rca(&cardInfo);
	if (err) {
		printf("ERR## ... Send RCA(CMD3) !\r\n");
		return err;
	}

	err = ftsdc021_ops_select_card(&cardInfo);
	if (err) {
		printf("ERR## ... Select Card(CMD7) !\r\n");
		return err;
	}

	cardInfo.card_init = TRUE;

	return err;
}

uint32_t ftsdc021_attach_sdio(void)
{
	uint32_t err, ocr, rocr;
	uint32_t resp;

	if (cardInfo.already_init == TRUE &&
	    cardInfo.CardType == SDIO_TYPE_CARD) {
		/* CMD 52: Reset the SDIO only card.(Write the specified bit
		 * after fetching the content.) */
		err = ftsdc021_sdio_io_rw_direct(&cardInfo, 0, 0, 0x06, 0, NULL);
		if (err) {
			printf("FTSDC021: send CMD52 failed!\r\n");
			return 1;
		}
		resp = (uint32_t) gpReg->CmdRespLo;
		if (dbg_print > 1)
			printf("FTSDC021: (CMD52) Card Status --> %x\r\n",
				    (resp & 0xF0));
		/* Setting the reset bit in 6th byte of function 0 */
		resp = resp & 0xF;
		resp = resp | 0x8;

		err = ftsdc021_sdio_io_rw_direct(&cardInfo, 1, 0, 0x06, resp,
						 (uint8_t *)((uintptr_t) &resp));
		if (err) {
			printf("FTSDC021: send CMD52 failed!\r\n");
			return 1;
		}

		return 0;
	}

	/*
	 * SDIO CMD5
	 */
	err = ftsdc021_sdio_send_io_op_cond(&cardInfo, 0, &ocr);
	if (err)
		return err;

	cardInfo.CardType = SDIO_TYPE_CARD;

	rocr = ftsdc021_select_voltage(ocr);
        if (!rocr)
		return 1;

	/*
	 * Detect and init the card.
	 */
	err = ftsdc021_init_sdio(rocr);

	return err;
}
