/*****************************************************************
* Copyright  Faraday Technology Corp 2020.  All rights reserved. *
*----------------------------------------------------------------*
* Name: SS_msg.h
* Description: message structure and prototype for secure subsystem.
* Author: Shantai Chen
******************************************************************/
#ifndef __SS_MSG_H__ 
#define __SS_MSG_H__ 
#include "types.h"

#define SS_MSG_BASEADDR		0x50C80000

typedef struct _msg_aes{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_AES */
	struct{
		UINT32	mode:16;
			#define AES_MSG_ECB	1
			#define AES_MSG_CBC	2
			#define AES_MSG_CTR		4
		UINT32	keySize:15;
			#define AES_MSG_KEY128	1
			#define AES_MSG_KEY192	2
			#define AES_MSG_KEY256	3
		UINT32	decrypt:1;
			#define AES_MSG_ENCRYPT	0
			#define AES_MSG_DECRYPT	1
	}	cfg;				/* offset 0x04 */
	UINT32	in_len;		/* offset 0x08 */
	UINT32	in_data;		/* offset 0x0C */
	UINT32	out_data;	/* offset 0x10 */
	UINT32	aes_key;	/* offset 0x14 */
	UINT32	aes_iv;		/* offset 0x18 */
	UINT32	rsv_1C;		/* offset 0x1C */
}MSG_AES;

typedef struct _msg_sha{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_SHA */
	struct{
		UINT32	mode:16;
			#define SHA_MODE_SHA	1
			#define SHA_MODE_HMAC	2
		UINT32	reserved:16;
	}	cfg;				/* offset 0x04 */
	UINT32	in_len;		/* offset 0x08 */
	UINT32	in_data;		/* offset 0x0C */
	UINT32	out_data;	/* offset 0x10 */
	UINT32	key;		/* offset 0x14 */
	UINT32	rsv_18;		/* offset 0x18 */
	UINT32	rsv_1C;		/* offset 0x1C */
}MSG_SHA;

#define RSA_MAX_LEN	2048	/* 1024 or 2048 */
typedef struct _msg_rsa{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_RSA */
	struct{
		UINT32	mode:16;	/* 0:RSA256, 1:RSA512, 2:RSA1024, 3:RSA2048 */
		UINT32	reserved:16;
	}	cfg;				/* offset 0x04 */
	UINT32	in_len;		/* offset 0x08 */
	UINT32	in_data;		/* offset 0x0C */
	UINT32	out_data;	/* offset 0x10 */
	UINT32	rsa_key;	/* offset 0x14 */
	UINT32	modulus;	/* offset 0x18 */
	UINT32	rsv_1C;		/* offset 0x1C */
}MSG_RSA;

typedef struct _msg_trng{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_TRNG */
	struct{
		UINT32	reserved:32;
	}	cfg;				/* offset 0x04 */
	UINT32	req_len;		/* offset 0x08 */
	UINT32	rsv_0C;		/* offset 0x0C */
	UINT32	out_data;	/* offset 0x10 */
	UINT32	rsv_14;		/* offset 0x14 */
	UINT32	rsv_18;		/* offset 0x18 */
	UINT32	rsv_1C;		/* offset 0x1C */
}MSG_TRNG;

typedef struct _msg_crc{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_CRC */
	UINT32	rsv_04;	/* offset 0x04 */
	UINT32	in_len;		/* offset 0x08 */
	UINT32	in_data;		/* offset 0x0C */
	UINT32	out_data;	/* offset 0x10 */
	UINT32	rsv_14;		/* offset 0x14 */
	UINT32	rsv_18;		/* offset 0x18 */
	UINT32	rsv_1C;		/* offset 0x1C */
}MSG_CRC;

typedef struct _msg_revoke_key{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_REVOKE_KEY */
	UINT32	rsv_04;		/* offset 0x04 */
	UINT32	magicNo1;	/* offset 0x08 */
		#define REVOKEKEY_MG1	0x31415926
	UINT32	magicNo2;	/* offset 0x0C */
		#define REVOKEKEY_MG2	0x53589793
	UINT32	revokeKeyId;	/* offset 0x10 */
	UINT32	rsv_14;		/* offset 0x14 */
	UINT32	rsv_18;		/* offset 0x18 */
	UINT32	rsv_1C;		/* offset 0x1C */
}MSG_REVOKE_KEY;

typedef struct _msg_debug_ctrl{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_DBG_CTRL */
	UINT32	rsv_04;		/* offset 0x04 */
	UINT32	magicNo1;	/* offset 0x08, it should be 0x31415926 */
		#define DBGCTRL_MG1	0x31415926
	UINT32	magicNo2;	/* offset 0x0C, it should be 0x53589793 */
		#define DBGCTRL_MG2	0x53589793
	UINT32	rsv_10;		/* offset 0x10 */
	UINT32	rsv_14;		/* offset 0x14 */
	UINT32	rsv_18;		/* offset 0x18 */
	UINT32	rsv_1C;		/* offset 0x1C */
}MSG_DBG_CTRL;

typedef struct _msg_efuse_op{
	UINT32	msg_op;	/* offset 0x00, SS_MSG_EFUSE_OP */
	UINT32	cfg;			/* offset 0x04, 0=read, 1=write, 3=clear */
		#define EFUSE_READ_OP		0x0
		#define EFUSE_WRITE_OP	0x1
		#define EFUSE_CLEAR_OP	0x3
	UINT32	address;	/* offset 0x08, efuse address */
	UINT32	rsv_0C;		/* offset 0x0C */
	UINT32	rsv_10;		/* offset 0x10 */
	UINT32	rsv_14;		/* offset 0x14 */
	UINT32	rsv_18;		/* offset 0x18 */
	UINT32	data;		/* offset 0x1C */
}MSG_EFUSE_OP;

typedef union {
	MSG_AES		aes;
	MSG_SHA	sha;
	MSG_RSA		rsa;
	MSG_TRNG	trng;
	MSG_CRC		crc;
	MSG_REVOKE_KEY	revoke_key;
	MSG_DBG_CTRL	dbg_ctrl;
	MSG_EFUSE_OP	efuse_op;
} SEC_MSG;

#define SS_MSG_AES		(0x01)
#define SS_MSG_SHA		(0x02)
#define SS_MSG_RSA		(0x03)
#define SS_MSG_TRNG		(0x04)
#define SS_MSG_CRC		(0x05)
#define SS_MSG_REVOKE_KEY	(0x06)
#define SS_MSG_DBG_CTRL	(0x07)
#define SS_MSG_EFUSE_OP	(0x08)
#define SS_MSG_SYSC	(0xAB) /* for debug */

typedef struct _ss_msg_ctrl{
	SEC_MSG	msg;	/* offset 0x00~0x1C */
	UINT32	start;	/* offset 0x20 */
	UINT32	cancel;	/* offset 0x24 */
	UINT32	busy;	/* offset 0x28 */
	UINT32	int_en;	/* offset 0x2C */
	UINT32	int_status;	/* offset 0x30 */
		#define INTST_DONE		BIT0
		#define INTST_FAIL		BIT1
		#define INTST_CANCEL	BIT2
	UINT32	rsv[3];	/* offset 0x34~0x3C */
	UINT32	freq;	/* offset 0x40 */
}SS_MSG_CTRL;


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

#endif /* __SS_MSG_H__ */

