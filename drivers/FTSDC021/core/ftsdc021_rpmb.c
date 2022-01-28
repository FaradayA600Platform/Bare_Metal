/*
 * Copyright 2014, Staubli Faverges
 * Pierre Aubert
 *
 * eMMC- Replay Protected Memory Block
 * According to JEDEC Standard No. 84-A441
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "ftsdc021.h"
#include "ftsdc021_rpmb.h"

static uint32_t mmc_set_blockcount(SDCardInfo *card, uint32_t blockcount, uint32_t is_rel_write)
{
        struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SET_BLOCK_COUNT;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = blockcount & 0x0000FFFF;
        if (is_rel_write)
                cmd.Argu |= 1 << 31;

        return ftsdc021_send_command(&cmd);
}

static uint32_t mmc_rpmb_request(SDCardInfo *card, struct s_rpmb *s, uint32_t count,
			       uint32_t is_rel_write)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};
	uint32_t ret;

	ret = mmc_set_blockcount(card, count, is_rel_write);
	if (ret) {
		printf("RPMB:ftsdc021_ops_app_set_blk_cnt-> %d\r\n", ret);
		return ret;
	}

	cmd.CmdIndex = SDHCI_WRITE_MULTI_BLOCK;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;

        data.act = WRITE;
        data.bufAddr = (uint32_t *) s;
        data.blkSz = FTSDC021_DEF_BLK_LEN;
        data.blkCnt = 1;
        cmd.data = &data;

	ret = ftsdc021_send_command(&cmd);
	if (ret) {
		printf("RPMB:ftsdc021_send_command-> %d\r\n", ret);
		return ret;
	}
	ret = ftsdc021_transfer_data(WRITE, (uint32_t *) s, FTSDC021_DEF_BLK_LEN);
	if (ret) {
		printf("RPMB:ftsdc021_transfer_data-> %d\r\n", ret);
		return ret;
	}
	return 0;
}
static uint32_t mmc_rpmb_response(SDCardInfo *card, struct s_rpmb *s, uint16_t expected)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};
	uint32_t ret;

	ret = mmc_set_blockcount(card, 1, 0);
	if (ret) {
		printf("RPMB:ftsdc021_ops_app_set_blk_cnt-> %d\r\n", ret);
		return ret;
	}
	cmd.CmdIndex = SDHCI_READ_MULTI_BLOCK;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;

        data.act = READ;
        data.bufAddr = (uint32_t *) s;
        data.blkSz = FTSDC021_DEF_BLK_LEN;
        data.blkCnt = 1;
        cmd.data = &data;

	ret = ftsdc021_send_command(&cmd);
	if (ret) {
		printf("RPMB:ftsdc021_send_command-> %d\r\n", ret);
		return ret;
	}
	ret = ftsdc021_transfer_data(READ, (uint32_t *) s, FTSDC021_DEF_BLK_LEN);
	if (ret) {
		printf("RPMB:ftsdc021_transfer_data-> %d\r\n", ret);
		return ret;
	}
	/* Check the response and the status */
	if (be16_to_cpu(s->request) != expected) {
		printf("RPMB:response= %x\r\n", be16_to_cpu(s->request));
		return 1;
	}
	ret = be16_to_cpu(s->result);
	if (ret) {
		printf("%s %s\r\n", rpmb_err_msg[ret & RPMB_ERR_MSK],
			    (ret & RPMB_ERR_CNT_EXPIRED) ? "Write counter has expired" : "");
	}

	/* Return the status of the command */
	return ret;
}

static uint32_t mmc_rpmb_status(SDCardInfo *card, uint16_t expected)
{
	struct s_rpmb *rpmb_frame;

	rpmb_frame = (struct s_rpmb *) malloc(sizeof(struct s_rpmb));
	/* Fill the request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_STATUS);
	if (mmc_rpmb_request(card, rpmb_frame, 1, 0))
		return 1;

	/* Read the result */
	return mmc_rpmb_response(card, rpmb_frame, expected);
}

static void rpmb_hmac(uint8_t *key, uint8_t *buff, int32_t len, uint8_t *output)
{
	sha256_context ctx;
	uint32_t i;
	uint8_t k_ipad[SHA256_BLOCK_SIZE];
	uint8_t k_opad[SHA256_BLOCK_SIZE];

	sha256_starts(&ctx);

	/* According to RFC 4634, the HMAC transform looks like:
	   SHA(K XOR opad, SHA(K XOR ipad, text))

	   where K is an n byte key.
	   ipad is the byte 0x36 repeated blocksize times
	   opad is the byte 0x5c repeated blocksize times
	   and text is the data being protected.
	*/

	for (i = 0; i < RPMB_SZ_MAC; i++) {
		k_ipad[i] = key[i] ^ 0x36;
		k_opad[i] = key[i] ^ 0x5c;
	}
	/* remaining pad bytes are '\0' XOR'd with ipad and opad values */
	for ( ; i < SHA256_BLOCK_SIZE; i++) {
		k_ipad[i] = 0x36;
		k_opad[i] = 0x5c;
	}
	sha256_update(&ctx, k_ipad, SHA256_BLOCK_SIZE);
	sha256_update(&ctx, buff, len);
	sha256_finish(&ctx, output);

	/* Init context for second pass */
	sha256_starts(&ctx);

	/* start with outer pad */
	sha256_update(&ctx, k_opad, SHA256_BLOCK_SIZE);

	/* then results of 1st hash */
	sha256_update(&ctx, output, RPMB_SZ_MAC);

	/* finish up 2nd pass */
	sha256_finish(&ctx, output);
}

uint32_t mmc_rpmb_get_counter(SDCardInfo *card, uint32_t *pcounter)
{
	uint32_t ret;
	struct s_rpmb *rpmb_frame;

	rpmb_frame = (struct s_rpmb *) malloc(sizeof(struct s_rpmb));
	/* Fill the request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_WCOUNTER);
	if (mmc_rpmb_request(card, rpmb_frame, 1, 0))
		return 1;

	/* Read the result */
	ret = mmc_rpmb_response(card, rpmb_frame, RPMB_RESP_WCOUNTER);
	if (ret)
		return ret;

	*pcounter = be32_to_cpu(rpmb_frame->write_counter);
	return 0;
}

uint32_t mmc_rpmb_set_key(SDCardInfo *card, void *key)
{
	struct s_rpmb *rpmb_frame;

	rpmb_frame = (struct s_rpmb *) malloc(sizeof(struct s_rpmb));
	/* Fill the request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_KEY);
	memcpy(rpmb_frame->mac, key, RPMB_SZ_MAC);

	if (mmc_rpmb_request(card, rpmb_frame, 1, 1))
		return 1;

	/* read the operation status */
	return mmc_rpmb_status(card, RPMB_RESP_KEY);
}

uint32_t mmc_rpmb_read(SDCardInfo *card, void *addr, uint16_t blk, uint16_t cnt, uint8_t *key)
{
	struct s_rpmb *rpmb_frame;
	int i;

	rpmb_frame = (struct s_rpmb *) malloc(sizeof(struct s_rpmb));
	for (i = 0; i < cnt; i++) {
		/* Fill the request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_READ_DATA);
		if (mmc_rpmb_request(card, rpmb_frame, 1, 0))
			break;

		/* Read the result */
		if (mmc_rpmb_response(card, rpmb_frame, RPMB_RESP_READ_DATA))
			break;

		/* Check the HMAC if key is provided */
		if (key) {
			uint8_t ret_hmac[RPMB_SZ_MAC];

			rpmb_hmac(key, rpmb_frame->data, 284, ret_hmac);
			if (memcmp(ret_hmac, rpmb_frame->mac, RPMB_SZ_MAC)) {
				printf("MAC error on block #%d\r\n", i);
				break;
			}
		}
		/* Copy data */
		memcpy(addr + i * RPMB_SZ_DATA, rpmb_frame->data, RPMB_SZ_DATA);
	}
	if (i != cnt)
		return 1;
	else
		return 0;
}

uint32_t mmc_rpmb_write(SDCardInfo *card, void *addr, uint16_t blk, uint16_t cnt, uint8_t *key)
{
	struct s_rpmb *rpmb_frame;
	uint32_t wcount;
	uint32_t i;

	rpmb_frame = (struct s_rpmb *) malloc(sizeof(struct s_rpmb));
	for (i = 0; i < cnt; i++) {
		if (mmc_rpmb_get_counter(card, &wcount)) {
			printf("Cannot read RPMB write counter\r\n");
			break;
		}

		/* Fill the request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		memcpy(rpmb_frame->data, addr + i * RPMB_SZ_DATA, RPMB_SZ_DATA);
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->block_count = cpu_to_be16(1);
		rpmb_frame->write_counter = cpu_to_be32(wcount);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_WRITE_DATA);
		/* Computes HMAC */
		rpmb_hmac(key, rpmb_frame->data, 284, rpmb_frame->mac);

		if (mmc_rpmb_request(card, rpmb_frame, 1, 1))
			break;

		/* Get status */
		if (mmc_rpmb_status(card, RPMB_RESP_WRITE_DATA))
			break;
	}
	if (i != cnt)
		return 1;
	else
		return 0;
}
