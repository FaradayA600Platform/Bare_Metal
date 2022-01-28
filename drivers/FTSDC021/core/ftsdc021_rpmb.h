#ifndef FTSDC021_RPMB_H
#define FTSDC021_RPMB_H

#include "sha256.h"

#ifndef __KERNEL__
#define __KERNEL__
#endif

/* Request codes */
#define RPMB_REQ_KEY		1
#define RPMB_REQ_WCOUNTER	2
#define RPMB_REQ_WRITE_DATA	3
#define RPMB_REQ_READ_DATA	4
#define RPMB_REQ_STATUS		5

/* Response code */
#define RPMB_RESP_KEY		0x0100
#define RPMB_RESP_WCOUNTER	0x0200
#define RPMB_RESP_WRITE_DATA	0x0300
#define RPMB_RESP_READ_DATA	0x0400

/* Error codes */
#define RPMB_OK			0
#define RPMB_ERR_GENERAL	1
#define RPMB_ERR_AUTH		2
#define RPMB_ERR_COUNTER	3
#define RPMB_ERR_ADDRESS	4
#define RPMB_ERR_WRITE		5
#define RPMB_ERR_READ		6
#define RPMB_ERR_KEY		7
#define RPMB_ERR_CNT_EXPIRED	0x80
#define RPMB_ERR_MSK		0x7

/* Sizes of RPMB data frame */
#define RPMB_SZ_STUFF		196
#define RPMB_SZ_MAC		32
#define RPMB_SZ_DATA		256
#define RPMB_SZ_NONCE		16

#define SHA256_BLOCK_SIZE	64

#define swab16(x) \
        ((uint16_t)( \
                (((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
                (((uint16_t)(x) & (uint16_t)0xff00U) >> 8) ))
#define swab32(x) \
        ((uint32_t)( \
                (((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
                (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
                (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
                (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24) ))

#ifdef __ARMEB__
/* Big endian CPU */
#define cpu_to_be16(x) (uint16_t)(x)
#define be16_to_cpu(x) (uint16_t)(x)
#define cpu_to_be32(x) (uint32_t)(x)
#define be32_to_cpu(x) (uint32_t)(x)
#else
/* Little endian CPU */
#define cpu_to_be16(x) (uint16_t)swab16((uint16_t)(x))
#define be16_to_cpu(x) (uint16_t)swab16((uint16_t)(x))
#define cpu_to_be32(x) (uint32_t)swab32((uint32_t)(x))
#define be32_to_cpu(x) (uint32_t)swab32((uint32_t)(x))
#endif

/* Error messages */
static const uint8_t * const rpmb_err_msg[] = {
	"",
	"General failure",
	"Authentication failure",
	"Counter failure",
	"Address failure",
	"Write failure",
	"Read failure",
	"Authentication key not yet programmed",
};

/* Structure of RPMB data frame. */
struct s_rpmb {
	uint8_t stuff[RPMB_SZ_STUFF];
	uint8_t mac[RPMB_SZ_MAC];
	uint8_t data[RPMB_SZ_DATA];
	uint8_t nonce[RPMB_SZ_NONCE];
	uint32_t write_counter;
	uint16_t address;
	uint16_t block_count;
	uint16_t result;
	uint16_t request;
};

uint32_t mmc_rpmb_set_key(SDCardInfo *card, void *key);
uint32_t mmc_rpmb_read(SDCardInfo *card, void *addr, uint16_t blk, uint16_t cnt, uint8_t *key);
uint32_t mmc_rpmb_write(SDCardInfo *card, void *addr, uint16_t blk, uint16_t cnt, uint8_t *key);

#endif
