/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_cprm.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2009/12//	Mike Yeh	 Original FTSDC020 code
 * 2010/6/22	BingJiun-Luo	 FTSDC021 code	
 * -----------------------------------------------------------------------------
 */

#include "ftsdc021.h"
#include "ftsdc021_cprm.h"

#define DEBUG_CPRM
//#define WEIRD

extern uint8_t *write_buf;
extern uint8_t *read_buf;

uint8_t auto_ed_cbc;

uint8_t SecretConstant_020[256] = {
	0x3a, 0xd0, 0x9a, 0xb6, 0xf5, 0xc1, 0x16, 0xb7, 0x58, 0xf6, 0xed, 0xe6,
        0xd9, 0x8c, 0x57, 0xfc, 0xfd, 0x4b, 0x9b, 0x47, 0x0e, 0x8e, 0xff, 0xf3,
	0xbb, 0xba, 0x0a, 0x80, 0x15, 0xd7, 0x2b, 0x36, 0x6a, 0x43, 0x5a, 0x89,
	0xb4, 0x5d, 0x71, 0x19, 0x8f, 0xa0, 0x88, 0xb8, 0xe8, 0x8a, 0xc3, 0xae,
	0x7c, 0x4e, 0x3d, 0xb5, 0x96, 0xcc, 0x21, 0x00, 0x1a, 0x6b, 0x12, 0xdb,
	0x1f, 0xe4, 0x11, 0x9d, 0xd3, 0x93, 0x68, 0xb0, 0x7f, 0x3b, 0x52, 0xb9,
	0x94, 0xdd, 0xa5, 0x1b, 0x46, 0x60, 0x31, 0xec, 0xc9, 0xf8, 0xe9, 0x5e,
	0x13, 0x98, 0xbf, 0x27, 0x56, 0x08, 0x91, 0xe3, 0x6f, 0x20, 0x40, 0xb2,
	0x2c, 0xce, 0x02, 0x10, 0xe0, 0x18, 0xd5, 0x6c, 0xde, 0xcd, 0x87, 0x79,
	0xaf, 0xa9, 0x26, 0x50, 0xf2, 0x33, 0x92, 0x6e, 0xc0, 0x3f, 0x39, 0x41,
	0xaa, 0x5b, 0x7d, 0x24, 0x03, 0xd6, 0x2f, 0xeb, 0x0b, 0x99, 0x86, 0x4c,
	0x51, 0x45, 0x8d, 0x2e, 0xef, 0x07, 0x7b, 0xe2, 0x4d, 0x7a, 0xfe, 0x25,
	0x5c, 0x29, 0xa2, 0xa8, 0xb1, 0xf0, 0xb3, 0xc4, 0x30, 0x7e, 0x63, 0x38,
	0xcb, 0xf4, 0x4f, 0xd1, 0xdf, 0x44, 0x32, 0xdc, 0x17, 0x5f, 0x66, 0x2a,
	0x81, 0x9e, 0x77, 0x4a, 0x65, 0x67, 0x34, 0xfa, 0x54, 0x1e, 0x14, 0xbe,
	0x04, 0xf1, 0xa7, 0x9c, 0x8b, 0x37, 0xee, 0x85, 0xab, 0x22, 0x0f, 0x69,
	0xc5, 0xd4, 0x05, 0x84, 0xa4, 0x73, 0x42, 0xa1, 0x64, 0xe1, 0x70, 0x83,
	0x90, 0xc2, 0x48, 0x0d, 0x61, 0x1c, 0xc6, 0x72, 0xfb, 0x76, 0x74, 0xe7,
	0x01, 0xd8, 0xc8, 0xd2, 0x75, 0xa3, 0xcf, 0x28, 0x82, 0x1d, 0x49, 0x35,
	0xc7, 0xbd, 0xca, 0xa6, 0xac, 0x0c, 0x62, 0xad, 0xf9, 0x3c, 0xea, 0x2d,
	0x59, 0xda, 0x3e, 0x97, 0x6d, 0x09, 0xf7, 0x55, 0xe5, 0x23, 0x53, 0x9f,
	0x06, 0xbc, 0x95, 0x78
};

uint32_t row_020[16] = {
	0x87, 0xC6, 0x57, 0xF9, 0x2D, 0xB3, 0x01CD, 0x0126, 0x88, 0x017B, 0xCE,
	0x01D7, 0x0110, 0x25, 0x0157, 0x90 };

uint64_t deviceKey_020[16] = {
	0x30FBBE569A2385uLL, 0x985ABB62CF1471uLL, 0xFFB98DA1B44835uLL,
	0x9A10948698A473uLL, 0x8E997D008CEA49uLL, 0x36079BACF12F54uLL,
	0x4F3AD916FC7D9CuLL, 0xF3BE60D27E6889uLL, 0x0B3066EA731A6CuLL,
	0x8E8CA02F8E88CCuLL, 0xEF0F00A0110A82uLL, 0x82D5B67CB1BF40uLL,
	0x65B50202C6AAFDuLL, 0x2AB7E59200F319uLL, 0xE09B696B057C0EuLL,
	0x939EA04C9C9F15uLL
};

uint32_t Seed_020[2] = { 0x365620B8, 0x1073C077 };

uint8_t SDC_CPRM_GetMKB(uint32_t * MKB_Buf, uint32_t blockcnt, uint8_t MKB_ID, uint16_t Offset)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	/* CMD 55 */
	ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1);

	/* ACMD43 */
	data.act = READ;
	data.bufAddr = (uint32_t *) MKB_Buf;
	data.blkSz = 0x200;
	data.blkCnt = blockcnt;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_GET_MKB;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = (blockcnt << 24) | (MKB_ID << 16) | Offset;
	ftsdc021_send_command(&cmd);

	return ftsdc021_transfer_data(READ, (uint32_t *) MKB_Buf, (blockcnt * 0x200));

}

/* Electronic Code Book Decrypt */
uint8_t SDC_CPRM_ECB_D(uint32_t buflo, uint32_t bufhi,
		     uint32_t mediakey_current_lo, uint32_t mediakey_current_hi, uint32_t * response_lo, uint32_t * response_hi)
{
	// Clear the content of Cipher mode control
	gpReg->Ciph_M_Ctl = 0x00;
//      gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x8;
	gpReg->Ciph_M_Ctl = C2_Decry_ECB;
	gpReg->In_Data_Lo = buflo;
	gpReg->In_Data_Hi = bufhi;
	gpReg->In_Key_Lo = mediakey_current_lo;
	gpReg->In_Key_Hi = mediakey_current_hi;

	while (!(gpReg->Ciph_M_Sts & 0x1)) ;
	gpReg->Ciph_M_Sts = 1;

	*response_lo = gpReg->Out_Data_Lo;
	*response_hi = gpReg->Out_Data_Hi;
 
	printf("ECD_D=> InLo: 0x%08x InHi: 0x%08x KeyLo: 0x%08x, KeyHi:  0x%08x OutLo: 0x%08x OutHi:  0x%08x.\r\n",
		buflo, bufhi, mediakey_current_lo, mediakey_current_hi, *response_lo, *response_hi);

	return 0;
}

/* Electronic Code Book Encrypt */
uint8_t SDC_CPRM_ECB_E(uint32_t buflo, uint32_t bufhi,
		     uint32_t mediakey_current_lo, uint32_t mediakey_current_hi, uint32_t * response_lo, uint32_t * response_hi)
{
	// Clear the content of Cipher mode control
	gpReg->Ciph_M_Ctl = 0x00;
//      gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x2;
	gpReg->Ciph_M_Ctl = C2_Encry_ECB;
	gpReg->In_Data_Lo = buflo;
	gpReg->In_Data_Hi = bufhi;
	gpReg->In_Key_Lo = mediakey_current_lo;
	gpReg->In_Key_Hi = mediakey_current_hi;

	while (!(gpReg->Ciph_M_Sts & 0x1)) ;
	gpReg->Ciph_M_Sts = 1;

	*response_lo = gpReg->Out_Data_Lo;
	*response_hi = gpReg->Out_Data_Hi;

	printf("ECD_E=> InLo: 0x%08x InHi: 0x%08x KeyLo: 0x%08x, KeyHi:  0x%08x OutLo: 0x%08x OutHi:  0x%08x.\r\n",
		buflo, bufhi, mediakey_current_lo, mediakey_current_hi, *response_lo, *response_hi);
	return 0;
}

/* C2 One-way function */
uint8_t SDC_CPRM_One_Way(uint32_t buflo, uint32_t bufhi,
		       uint32_t mediakey_current_lo, uint32_t mediakey_current_hi,
		       uint32_t * response_lo, uint32_t * response_hi)
{
	// Clear the content of Cipher mode control
	gpReg->Ciph_M_Ctl = 0x00;
//      gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x1;
	gpReg->Ciph_M_Ctl = C2_One_Way_Function;
	gpReg->In_Data_Lo = buflo;
	gpReg->In_Data_Hi = bufhi;
	gpReg->In_Key_Lo = mediakey_current_lo;
	gpReg->In_Key_Hi = mediakey_current_hi;

	while (!(gpReg->Ciph_M_Sts & 0x1)) ;
	gpReg->Ciph_M_Sts = 1;

	*response_lo = gpReg->Out_Data_Lo;
	*response_hi = gpReg->Out_Data_Hi;

	printf("One_Way=> InLo: 0x%08x InHi: 0x%08x KeyLo: 0x%08x, KeyHi:  0x%08x OutLo: 0x%08x OutHi:  0x%08x.\r\n",
		buflo, bufhi, mediakey_current_lo, mediakey_current_hi, *response_lo, *response_hi);
	return 0;
}

/* Converted Chipher Block Chaining(C-CBC) Encrypt */
uint8_t SDC_CPRM_Auto_ECBC(uint32_t mediakey_current_lo, uint32_t mediakey_current_hi, uint8_t swap_HL, uint8_t ch_endian)
{
	// Clear the content of Cipher mode control
	gpReg->Ciph_M_Ctl = 0x00;
	//gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x40;
	gpReg->Ciph_M_Ctl = Auto_Encry_CCBC;
	if (swap_HL == 1) {
		gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x400;
	}
	if (ch_endian == 1) {
		gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x200;
	}
	gpReg->In_Key_Lo = mediakey_current_lo;
	gpReg->In_Key_Hi = mediakey_current_hi;
	return 0;
}

uint8_t SDC_CPRM_ECBC(uint32_t buflo, uint32_t bufhi,
		    uint32_t mediakey_current_lo, uint32_t mediakey_current_hi, uint32_t * response_lo, uint32_t * response_hi)
{
	gpReg->Ciph_M_Ctl = 0x00;
//      gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x4;
	gpReg->Ciph_M_Ctl = C2_Encry_CCBC;
	gpReg->In_Data_Lo = buflo;
	gpReg->In_Data_Hi = bufhi;
	gpReg->In_Key_Lo = mediakey_current_lo;
	gpReg->In_Key_Hi = mediakey_current_hi;

	while (!(gpReg->Ciph_M_Sts & 0x1)) ;
	gpReg->Ciph_M_Sts = 1;

	*response_lo = gpReg->Out_Data_Lo;
	*response_hi = gpReg->Out_Data_Hi;

	return 0;
}

/* Converted Chipher Block Chaining(C-CBC) Decrypt */
uint8_t SDC_CPRM_Auto_DCBC(uint32_t mediakey_current_lo, uint32_t mediakey_current_hi, uint8_t swap_HL, uint8_t ch_endian)
{
	// Clear the content of Cipher mode control
	gpReg->Ciph_M_Ctl = 0x00;
//      gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x80;
	gpReg->Ciph_M_Ctl = Auto_Decry_CCBC;
	if (swap_HL == 1) {
		gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x400;
	}
	if (ch_endian == 1) {
		gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x200;
	}
	gpReg->In_Key_Lo = mediakey_current_lo;
	gpReg->In_Key_Hi = mediakey_current_hi;

	return 0;
}

uint8_t SDC_CPRM_DCBC(uint32_t buflo, uint32_t bufhi,
		    uint32_t mediakey_current_lo, uint32_t mediakey_current_hi, uint32_t * response_lo, uint32_t * response_hi)
{
	gpReg->Ciph_M_Ctl = 0x00;
//      gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x10;
	gpReg->Ciph_M_Ctl = C2_Decry_CCBC;
	gpReg->In_Data_Lo = buflo;
	gpReg->In_Data_Hi = bufhi;
	gpReg->In_Key_Lo = mediakey_current_lo;
	gpReg->In_Key_Hi = mediakey_current_hi;

	while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
	gpReg->Ciph_M_Sts = 1;

	*response_lo = (uint32_t) gpReg->Out_Data_Lo;
	*response_hi = (uint32_t) gpReg->Out_Data_Hi;

	return 0;
}

uint8_t SDC_CPRM_RN_generator(uint32_t key_lo, uint32_t key_hi, uint32_t * response_lo, uint32_t * response_hi)
{
	// Clear the content of Cipher mode control
	gpReg->Ciph_M_Ctl = 0x00;
//      gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x20;
	gpReg->Ciph_M_Ctl = C2_Random_Gen;
	gpReg->In_Data_Lo = Seed_020[0];
	gpReg->In_Data_Hi = Seed_020[1];
	gpReg->In_Key_Lo = key_lo;
	gpReg->In_Key_Hi = key_hi;

	while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
	gpReg->Ciph_M_Sts = 1;

	*response_lo = gpReg->Out_Data_Lo;
	*response_hi = gpReg->Out_Data_Hi;

	printf("RN=> KeyLo: 0x%08x, KeyHi:  0x%08x OutLo: 0x%08x OutHi:  0x%08x.\r\n",
                    key_lo, key_hi, *response_lo, *response_hi);

	return 0;
}

uint8_t SDC_CPRM_processMKB(uint32_t * MKB_Buf, uint32_t Blockcnt, uint32_t * km_lo, uint32_t * km_hi)
{
	uint8_t record_type;
	uint8_t column;
	uint32_t function_cr_lo, function_cr_hi;
	int32_t record_length = 0;
	uint32_t remaining_byte = Blockcnt * 512;
	uint32_t current_key_lo = 0, current_key_hi = 0;
	uint32_t respond_lo, respond_hi;
	uint32_t bufferlo, bufferhi;
	uint32_t verificationdata_lo, verificationdata_hi;
	uint8_t *MKB_Buf_p = (uint8_t *) MKB_Buf;

//      printf("%x, %x, %x, %x \r\n", *(MKB_Buf_p), *(MKB_Buf_p+1), *(MKB_Buf_p+2), *(MKB_Buf_p+3));

	while (1) {
		record_type = *(MKB_Buf_p);
		MKB_Buf_p = MKB_Buf_p + 1;
		remaining_byte = remaining_byte - 1;

		if (remaining_byte == 0) {
			printf("No more data in MKB\r\n");
			*km_lo = current_key_lo;
			*km_hi = current_key_hi;
			return 0;
		}

		record_length = (*(MKB_Buf_p) << 16) | (*(MKB_Buf_p + 1) << 8) | *(MKB_Buf_p + 2);
		MKB_Buf_p = MKB_Buf_p + 3;
		remaining_byte = remaining_byte - 3;

		record_length = record_length - 4;

		if (record_length >= 8) {
			bufferhi =
			    (*(MKB_Buf_p) << 24) | (*(MKB_Buf_p + 1) << 16) | (*(MKB_Buf_p + 2) << 8) |
			    *(MKB_Buf_p + 3);
			MKB_Buf_p = MKB_Buf_p + 4;
			remaining_byte = remaining_byte - 4;

			bufferlo =
			    (*(MKB_Buf_p) << 24) | (*(MKB_Buf_p + 1) << 16) | (*(MKB_Buf_p + 2) << 8) |
			    *(MKB_Buf_p + 3);
			MKB_Buf_p = MKB_Buf_p + 4;
			remaining_byte = remaining_byte - 4;

			record_length = record_length - 8;
		} else if (record_length < 0) {
			record_length = 0;
		}
#ifdef DEBUG_CPRM
		printf("record type:0x%x\r\n", record_type);
#endif

		switch (record_type) {
		case 0x82:	// Conditionally Calculate Media Key Record
#ifdef DEBUG_CPRM
			printf("--Conditionally Calculate--\r\n");
#endif

			if (SDC_CPRM_ECB_D
			    (bufferlo, bufferhi, current_key_lo, current_key_hi, &respond_lo, &respond_hi)) {
				printf("Decrypt Error\r\n");
				return 1;
			}
			bufferlo = respond_lo;
			bufferhi = respond_hi;

			if (bufferhi != 0xDEADBEEF) {
				break;
			}

		case 0x01:	// Calculate Media Key Record
#ifdef DEBUG_CPRM
			if (record_type != 0x82) {
				printf("--Calculate Media Key Record--\r\n");
			}
#endif
			// column = the fifth byte of the buffer
			column = bufferlo & 0xFF000000;
			// if the last three bytes of buffer are not 0x000001 or Column >=16
			if ((bufferlo & 0x00FFFFFF) != 0x000001 || column >= 16) {
				break;
			}
			//if((row[column]+1) * 8 > length)
			if (((row_020[column] + 1) * 8) > record_length) {
				break;
			}
			// Skip the cells up to the one I'm interested in
			MKB_Buf_p = MKB_Buf_p + (row_020[column] * 8);
			remaining_byte = remaining_byte - (row_020[column] * 8);

			// Get the cell and update the length
			bufferhi =
			    (*(MKB_Buf_p) << 24) | (*(MKB_Buf_p + 1) << 16) | (*(MKB_Buf_p + 2) << 8) |
			    *(MKB_Buf_p + 3);
			MKB_Buf_p = MKB_Buf_p + 4;
			remaining_byte = remaining_byte - 4;

			bufferlo =
			    (*(MKB_Buf_p) << 24) | (*(MKB_Buf_p + 1) << 16) | (*(MKB_Buf_p + 2) << 8) |
			    *(MKB_Buf_p + 3);
			MKB_Buf_p = MKB_Buf_p + 4;
			remaining_byte = remaining_byte - 4;

			record_length = record_length - 8 - (row_020[column] * 8);
			remaining_byte = remaining_byte - 8;

			if (record_type == 0x82) {
				// Buffer = decrypt(mediakey, buffer);
				if (SDC_CPRM_ECB_D
				    (bufferlo, bufferhi, current_key_lo, current_key_hi, &respond_lo, &respond_hi)) {
					printf("Decrypt Error\r\n");
					return 1;
				}
				bufferlo = respond_lo;
				bufferhi = respond_hi;
			} else {
				// mediaKey is not nil
				if (current_key_lo != 0 && current_key_hi != 0) {
					break;
				}
			}

			// mediaKey = decrypt(deviceKey[column], buffer);
			current_key_lo = (uint32_t) deviceKey_020[0];
			current_key_hi = (uint32_t) (deviceKey_020[0] >> 32);

			function_cr_lo = row_020[column];
			function_cr_hi = column;

			if (SDC_CPRM_ECB_D
			    (bufferlo, bufferhi, current_key_lo, current_key_hi, &respond_lo, &respond_hi)) {
				printf("Decrypt Error\r\n");
				return 1;
			}
			current_key_lo = respond_lo;
			current_key_hi = respond_hi;

#ifdef WEIRD
			current_key_lo = current_key_lo ^ function_cr_lo;
			current_key_hi = current_key_hi ^ function_cr_hi;
#endif

			// Buffer = decrypt(mediaKey, verificationData);
			if (SDC_CPRM_ECB_D
			    (verificationdata_lo, verificationdata_hi, current_key_lo, current_key_hi,
			     &respond_lo, &respond_hi)) {
				printf("Decrypt Error\r\n");
				return 1;
			}
			bufferlo = respond_lo;
			bufferhi = respond_hi;

			// To judge whether the first four bytes is 0xDEADBEEF
			if (bufferhi == 0xDEADBEEF) {
				// return mediakey
				*km_lo = current_key_lo;
				*km_hi = current_key_hi;
				return 0;
			}
			break;

		case 0x02:	// End of Media Key Record
#ifdef DEBUG_CPRM
			printf("--End--\r\n");
#endif

			// return mediakey;
			*km_lo = current_key_lo;
			*km_hi = current_key_hi;
			return 0;

			break;
		case 0x81:	// Verify Media Key Record

#ifdef DEBUG_CPRM
			printf("--Verify--\r\n");
#endif

			// verificationData = getDoubleword();
			/*      verificationdata_lo = ( *(MKB_Buf_p)<< 24) | ( *(MKB_Buf_p+1)<< 16) | ( *(MKB_Buf_p+2)<< 8) | *(MKB_Buf_p+3) ;
			   MKB_Buf_p = MKB_Buf_p + 4;
			   remaining_byte = remaining_byte - 4;

			   verificationdata_hi = ( *(MKB_Buf_p)<< 24) | ( *(MKB_Buf_p+1)<< 16) | ( *(MKB_Buf_p+2)<< 8) | *(MKB_Buf_p+3) ;
			   MKB_Buf_p = MKB_Buf_p + 4;
			   remaining_byte = remaining_byte - 4;

			   record_length = record_length - 8;
			 */
			verificationdata_lo = bufferlo;
			verificationdata_hi = bufferhi;

#ifdef DEBUG_CPRM
			printf("verificationData = %8x%8x\r\n", verificationdata_hi, verificationdata_lo);
#endif

			break;
		default:
#ifdef DEBUG_CPRM
			printf("--Command %x--\r\n", record_type);
			printf("unrecognized command code %x\r\n", record_type);
#endif

			break;
		}		// switch(record_type)
		MKB_Buf_p = MKB_Buf_p + record_length;
	}			// while(1)
}

uint8_t SDC_CPRM_generate_MUK(uint32_t km_lo, uint32_t km_hi, uint32_t * kum_lo, uint32_t * kum_hi)
{
	uint8_t *MID;
	uint32_t ID_lo, ID_hi;
	uint32_t respond_lo, respond_hi;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	// Media ID is 64bits = 8bytes
	MID = read_buf;
	memset(MID, 0, 8);

	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	/* CMD 55 */
	ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1);

	/* ACMD44 */
	data.act = READ;
	data.bufAddr = (uint32_t *) MID;
	data.blkSz = 0x8;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_GET_MID;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	ftsdc021_send_command(&cmd);

	if (ftsdc021_transfer_data(READ, (uint32_t *) MID, 0x8)) {
		printf("CPRM: Get MID read data failed !\r\n");
		return 1;
	}

	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	ID_hi = (MID[0] << 24) | (MID[1] << 16) | (MID[2] << 8) | MID[3];
	ID_lo = (MID[4] << 24) | (MID[5] << 16) | (MID[6] << 8) | MID[7];

#ifdef DEBUG_CPRM
	printf("Media ID:0x%0.8x %0.8x\r\n", ID_hi, ID_lo);
#endif

	// Key is 56-bit wided in generating media unique key.
	km_hi = km_hi & 0x00FFFFFF;
	km_lo = km_lo;

	if (SDC_CPRM_One_Way(ID_lo, ID_hi, km_lo, km_hi, &respond_lo, &respond_hi)) {
		printf("One way function fail in generating media unique key\r\n");
		return 1;
	}

	*kum_lo = respond_lo;
	*kum_hi = (respond_hi & 0x00FFFFFF);

	return 0;
}

/* Authentication and Key Exchange */
uint8_t SDC_CPRM_AKE(uint32_t arg, uint32_t * ks_lo, uint32_t * ks_hi)
{
	uint32_t random_number_lo, random_number_hi;
	uint32_t challenge1[2];
	uint32_t challenge2[2];
	uint32_t response2[2];
	uint32_t response1_cal_from_host[2];
	uint32_t response1_from_card[2];
	uint32_t bitwise_compliment[2];
	uint32_t RN1[2];
	uint32_t RN2[2];
	uint32_t bitorder_changing_tmp;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	if (SDC_CPRM_RN_generator(cardInfo.Kmu_LO, cardInfo.Kmu_HI, &random_number_lo, &random_number_hi)) {
		printf("Failed in SDC_CPRM_RN_generator function\r\n");
		return 1;
	}
	// Step 3a-1 and Step 3a-2
	if (SDC_CPRM_ECB_E(random_number_lo, arg, cardInfo.Kmu_LO, cardInfo.Kmu_HI, &RN1[0], &RN1[1])) {
		printf("Failed in SDC_CPRM_ECB_E function\r\n");
		return 1;
	}

	challenge1[0] = RN1[0];
	challenge1[1] = RN1[1];

	// ******** Sending the Challenge1 to SD Memory Card ********
	// 3a-3
	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	RN1[0] = (uint32_t) (((challenge1[1] & 0xFF) << 24) | ((challenge1[1] & 0xFF00) << 8) |
			   ((challenge1[1] & 0xFF0000) >> 8) | (challenge1[1] & 0xFF000000) >> 24);
	RN1[1] = (uint32_t) (((challenge1[0] & 0xFF) << 24) | ((challenge1[0] & 0xFF00) << 8) |
			   ((challenge1[0] & 0xFF0000) >> 8) | (challenge1[0] & 0xFF000000) >> 24);
	/* CMD 55 */
	if (ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1)) {
		return 1;
	}

	/* ACMD45 */
	data.act = WRITE;
	data.bufAddr = (uint32_t *) &RN1[0];
	data.blkSz = 0x8;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_CER_RN1;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	if (ftsdc021_send_command(&cmd)) {
		printf("CPRM: CER_RN1 command failed !\r\n");
		return 1;
	}

	if (ftsdc021_transfer_data(WRITE, (uint32_t *) & RN1[0], 0x8)) {
		printf("CPRM: Write CER RN1 failed !\r\n");
		return 1;
	}
	printf("SET_CER_RN1: Lo 0x%08x Hi 0x%08x.\r\n", RN1[0], RN1[1]);
	//******** Reading the challenge2 from SD Memory Card ********
	// 3b   
	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	/* CMD 55 */
	if (ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1)) {
		return 1;
	}
	/* ACMD46 */
	data.act = READ;
	data.bufAddr = (uint32_t *) &RN2[0];
	data.blkSz = 0x8;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_CER_RN2;
	cmd.DataPresent = 1;
	cmd.Argu = 0;
	if (ftsdc021_send_command(&cmd)) {
		printf("CPRM: CER_RN2 command failed !\r\n");
		return 1;
	}

	if (ftsdc021_transfer_data(READ, (uint32_t *) & RN2[0], 0x8)) {
		printf("CPRM: Read CER RN2 failed !\r\n");
		return 1;
	}

	bitorder_changing_tmp = RN2[0];
	RN2[0] = (((RN2[1] & 0xFF) << 24) | ((RN2[1] & 0xFF00) << 8) |
		  ((RN2[1] & 0xFF0000) >> 8) | (RN2[1] & 0xFF000000) >> 24);

	RN2[1] = (((bitorder_changing_tmp & 0xFF) << 24) | ((bitorder_changing_tmp & 0xFF00) << 8) |
		  ((bitorder_changing_tmp & 0xFF0000) >> 8) | (bitorder_changing_tmp & 0xFF000000) >> 24);

	challenge2[0] = RN2[0];
	challenge2[1] = RN2[1];

	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);
	printf("GET_CER_RN2: Lo 0x%08x Hi 0x%08x.\r\n", RN2[0], RN2[1]);

	//******** Writing the Response2 to SD Memory Card ********
	// 3c. Calculate Response2 and return it to SD Memmory Card.
	if (SDC_CPRM_One_Way(challenge2[0], challenge2[1], cardInfo.Kmu_LO, cardInfo.Kmu_HI, &RN2[0], &RN2[1])) {
		printf("Fail in SDC_CPRM_One_Way functino\r\n");
		return 1;
	}

	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	response2[0] = (((RN2[1] & 0xFF) << 24) | ((RN2[1] & 0xFF00) << 8) |
			((RN2[1] & 0xFF0000) >> 8) | (RN2[1] & 0xFF000000) >> 24);
	response2[1] = (((RN2[0] & 0xFF) << 24) | ((RN2[0] & 0xFF00) << 8) |
			((RN2[0] & 0xFF0000) >> 8) | (RN2[0] & 0xFF000000) >> 24);
	/* CMD 55 */
	if (ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1)) {
		return 1;
	}
	/* ACMD47 */
	data.act = WRITE;
	data.bufAddr = (uint32_t *) &response2[0];
	data.blkSz = 0x8;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_CER_RES2;
	cmd.DataPresent = 1;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	if (ftsdc021_send_command(&cmd)) {
		printf("CPRM: CER_RES2 command failed !\r\n");
		return 1;
	}
	if (ftsdc021_transfer_data(WRITE, (uint32_t *) & response2[0], 0x8)) {
		printf("CPRM: Read CER RES2 failed !\r\n");
		return 1;
	}
	//******** Reading the response1 from SD Memory Card ********
	//3d
	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);
	printf("SET_CER_RES2: Lo 0x%08x Hi 0x%08x.\r\n", response2[0], response2[1]);

	/* CMD 55 */
	if (ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1)) {
		return 1;
	}
	/* ACMD48 */
	data.act = READ;
	data.bufAddr = (uint32_t *) &response1_from_card[0];
	data.blkSz = 0x8;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_CER_RES1;
	cmd.DataPresent = 1;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	if (ftsdc021_send_command(&cmd)) {
		printf("CPRM: CER_RES1 command failed !\r\n");
		return 1;
	}
	if (ftsdc021_transfer_data(READ, (uint32_t *) & response1_from_card[0], 0x8)) {
		printf("CPRM: Read CER RES1 failed !\r\n");
		return 1;
	}
	// Changing the order due to come from SD memory card. 
	bitorder_changing_tmp = response1_from_card[0];

	response1_from_card[0] =
	    (((response1_from_card[1] & 0xFF) << 24) | ((response1_from_card[1] & 0xFF00) << 8) |
	     ((response1_from_card[1] & 0xFF0000) >> 8) | (response1_from_card[1] & 0xFF000000) >> 24);
	response1_from_card[1] =
	    (((bitorder_changing_tmp & 0xFF) << 24) | ((bitorder_changing_tmp & 0xFF00) << 8) |
	     ((bitorder_changing_tmp & 0xFF0000) >> 8) | (bitorder_changing_tmp & 0xFF000000) >> 24);
	printf("GET_CER_RES1: Lo 0x%08x Hi 0x%08x.\r\n", response1_from_card[0], response1_from_card[1]);

	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);
	// *******************************************

	if (SDC_CPRM_One_Way
	    (challenge1[0], challenge1[1], cardInfo.Kmu_LO, cardInfo.Kmu_HI, &response1_cal_from_host[0],
	     &response1_cal_from_host[1])) {
		printf("Fail in SDC_CPRM_One_Way function\r\n");
		return 1;
	}

	if ((response1_cal_from_host[0] == response1_from_card[0]) &&
	    (response1_cal_from_host[1] == response1_from_card[1])) {
#ifdef DEBUG_CPRM
		printf("Authenication is passed\r\n");
#endif

		// step 3e
		bitwise_compliment[0] = ~(cardInfo.Kmu_LO);
		bitwise_compliment[1] = ~(cardInfo.Kmu_HI);

		// step 3f
		if (SDC_CPRM_One_Way((challenge1[0] ^ challenge2[0]), (challenge1[1] ^ challenge2[1]),
				     bitwise_compliment[0], bitwise_compliment[1], ks_lo, ks_hi)) {
			printf("Fail in SDC_CPRM_One_Way function\r\n");
			return 1;
		}

		*ks_hi = *ks_hi & 0xFFFFFF;
		*ks_lo = *ks_lo;

#ifdef DEBUG_CPRM
		//printf("Controller: 0x%0.8x %0.8x\r\n",response1_cal_from_host_hi, response1_cal_from_host_lo );
		//printf("Device    : 0x%0.8x %0.8x\r\n",response1_from_card_hi, response1_from_card_lo);
		printf("Session Key (56 bits): 0x%0.8x %0.8x\r\n", *ks_hi, *ks_lo);
#endif

		return 0;
	} else {
#ifdef DEBUG_CPRM
		printf("Authenication is failed\r\n");
		printf("Controller: 0x%0.8x %0.8x\r\n", response1_cal_from_host[1], response1_cal_from_host[0]);
		printf("Device    : 0x%0.8x %0.8x\r\n", response1_from_card[1], response1_from_card[0]);
#endif
		return 1;
	}
}

uint8_t SDC_Write_Protected_Sector(SDCardInfo * SDCard, uint32_t StartBlk, uint32_t BlkCount, uint32_t SectorSize, uint8_t *Buf)
{
	return SDC_CPRM_write_protect_area(StartBlk, BlkCount, SectorSize, Buf);
}

uint8_t SDC_CPRM_write_protect_area(uint32_t startblk, uint32_t blockcnt,
				  uint32_t sector_length, uint8_t *data_buf)
{
	uint32_t start_addr;
	uint32_t ks_lo = 0, ks_hi = 0;
	uint32_t arg;
	uint32_t i;
	uint32_t data_buf_swap;
	uint32_t protect_data_lo, protect_data_hi;
	uint8_t *data_buf_p;
	// For controlling the times of writing media-key in the None-auto ECBC
	uint8_t throttle = 0;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	// (cardInfo.CSD_LO >> 14) & 0xF is the field in CSD register indicates the max write data block length
	start_addr = startblk << ((cardInfo.CSD_LO >> 14) & 0xF);

#ifdef DEBUG_CPRM
	printf("===> AKE Write, Arg = %0.8x\r\n", (blockcnt << 24) | ((start_addr >> 9) & 0x00FFFFFF));
#endif

	arg = (blockcnt << 24) | ((start_addr >> 9) & 0x00FFFFFF);

	ftsdc021_ops_send_sd_status(&cardInfo);
	if (SDC_CPRM_AKE(arg, &ks_lo, &ks_hi)) {
		printf("Fail in SDC_CPRM_write_protect_area function\r\n");
		return 1;
	}
#ifdef DEBUG_CPRM
	printf("Session Key: 0x%0.8x %0.8x\r\n", ks_hi, ks_lo);
#endif

	ftsdc021_ops_send_sd_status(&cardInfo);
	if (!cardInfo.sd_status.SECURED_MODE){
		printf("Card not in the Secure mode.\r\n");
		return 1;
	}


	if (auto_ed_cbc == 1) {
		// Auto mode of ECBC
		if (SDC_CPRM_Auto_ECBC(ks_lo, ks_hi, 1, 1)) {
			printf("Fail in SDC_CPRM_Auto_ECBC function.\r\n");
			return 1;
		}
	} else {
		// None-Auto ECBC
		// Encrypt every 8bytes data by ECBC for writing. 
		data_buf_p = data_buf;
		for (i = 0; i < (blockcnt * sector_length) / 8; i++) {
			protect_data_lo = *((uint32_t *) (data_buf_p));
			protect_data_hi = *((uint32_t *) (data_buf_p + 4));

			if (throttle == 0) {
				gpReg->Ciph_M_Ctl = C2_Encry_CCBC;
				gpReg->In_Data_Lo = protect_data_lo;
				gpReg->In_Data_Hi = protect_data_hi;
				gpReg->In_Key_Lo = ks_lo;
				gpReg->In_Key_Hi = ks_hi;

				while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
				gpReg->Ciph_M_Sts = 1;

				*((uint32_t *) (data_buf_p)) = gpReg->Out_Data_Lo;
				*((uint32_t *) (data_buf_p + 4)) = gpReg->Out_Data_Hi;
				throttle = 1;
			} else {
				gpReg->In_Data_Lo = protect_data_lo;
				gpReg->In_Data_Hi = protect_data_hi;

				while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
				gpReg->Ciph_M_Sts = 1;

				*((uint32_t *) (data_buf_p)) = gpReg->Out_Data_Lo;
				*((uint32_t *) (data_buf_p + 4)) = gpReg->Out_Data_Hi;
			}
			//      SDC_CPRM_ECBC( protect_data_lo, protect_data_hi, ks_lo, ks_hi, data_buf_p, (data_buf_p + 4)); 
			data_buf_p = data_buf_p + 8;
		}

		data_buf_p = data_buf;
		for (i = 0; i < (blockcnt * sector_length) / 8; i++) {
			protect_data_lo = *((uint32_t *) (data_buf_p));
			protect_data_hi = *((uint32_t *) (data_buf_p + 4));
			data_buf_swap = protect_data_lo;
			protect_data_lo =
			    (((protect_data_hi & 0xFF) << 24) | ((protect_data_hi & 0xFF00) << 8) |
			     ((protect_data_hi & 0xFF0000) >> 8) | ((protect_data_hi & 0xFF000000) >> 24));
			protect_data_hi =
			    (((data_buf_swap & 0xFF) << 24) | ((data_buf_swap & 0xFF00) << 8) |
			     ((data_buf_swap & 0xFF0000) >> 8) | ((data_buf_swap & 0xFF000000) >> 24));

			/*      protect_data_lo = protect_data_hi;
			   protect_data_hi = data_buf_swap;
			 */
			*((uint32_t *) (data_buf_p)) = protect_data_lo;
			*((uint32_t *) (data_buf_p + 4)) = protect_data_hi;
			data_buf_p = data_buf_p + 8;
		}

	}

	/* CMD 16 */
	/* It's necessary to set the block length before multiple read block for certain card 
	   although the "SD Security Spec" says "no". */
	ftsdc021_ops_set_blocklen(&cardInfo, sector_length);
	/* CMD 55 */
	if (ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1)) {
		return 1;
	}

	/* CMD25 */
	data.act = WRITE;
	data.bufAddr = (uint32_t *) data_buf;
	data.blkSz = sector_length;
	data.blkCnt = blockcnt;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_WRITE_MULTI_BLOCK;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = startblk;
	ftsdc021_send_command(&cmd);
	if (ftsdc021_transfer_data(WRITE, (uint32_t *) data_buf, (blockcnt * sector_length))) {
		return 1;
	}

	// Disable the Auto DCBC function
	gpReg->Ciph_M_Ctl = 0x00;
	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	throttle = 0;
	if (!auto_ed_cbc)  {
		data_buf_p = data_buf;
		for (i = 0; i < (blockcnt * sector_length) / 8; i++) {
			protect_data_lo = *((uint32_t *) (data_buf_p));
			protect_data_hi = *((uint32_t *) (data_buf_p + 4));
			data_buf_swap = protect_data_lo;
			protect_data_lo =
			    (((protect_data_hi & 0xFF) << 24) | ((protect_data_hi & 0xFF00) << 8) |
			     ((protect_data_hi & 0xFF0000) >> 8) | ((protect_data_hi & 0xFF000000) >> 24));
			protect_data_hi =
			    (((data_buf_swap & 0xFF) << 24) | ((data_buf_swap & 0xFF00) << 8) |
			     ((data_buf_swap & 0xFF0000) >> 8) | ((data_buf_swap & 0xFF000000) >> 24));

			*((uint32_t *) (data_buf_p)) = protect_data_lo;
			*((uint32_t *) (data_buf_p + 4)) = protect_data_hi;
			data_buf_p = data_buf_p + 8;
		}

		/*Initial the DCBC engine */
		/*The next-round mediakey will be generated automatically by DCBC engine according to the C-CBC algorithm. */
		/*So, we only initial the mediakey at first time. The media of key in next-round (next 8 bytes) is different with 
		   setting in the initilization */

		data_buf_p = data_buf;
		for (i = 0; i < (blockcnt * sector_length) / 8; i++) {
			protect_data_lo = *((uint32_t *) (data_buf_p));
			protect_data_hi = *((uint32_t *) (data_buf_p + 4));

			if (throttle == 0) {
				gpReg->Ciph_M_Ctl = C2_Decry_CCBC;
				gpReg->In_Data_Lo = protect_data_lo;
				gpReg->In_Data_Hi = protect_data_hi;
				gpReg->In_Key_Lo = ks_lo;
				gpReg->In_Key_Hi = ks_hi;
				/*Busy-wait for the coming of plaintext */
				while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
				gpReg->Ciph_M_Sts = 1;

				*((uint32_t *) (data_buf_p)) = gpReg->Out_Data_Lo;
				*((uint32_t *) (data_buf_p + 4)) = gpReg->Out_Data_Hi;

				throttle = 1;
			} else {
				gpReg->In_Data_Lo = protect_data_lo;
				gpReg->In_Data_Hi = protect_data_hi;

				while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
				gpReg->Ciph_M_Sts = 1;

				*((uint32_t *) (data_buf_p)) = gpReg->Out_Data_Lo;
				*((uint32_t *) (data_buf_p + 4)) = gpReg->Out_Data_Hi;
			}

			data_buf_p = data_buf_p + 8;
		}
	}
	return 0;
}

uint8_t SDC_Read_Protected_Sector(SDCardInfo * SDCard, uint32_t StartBlk, uint32_t BlkCount, uint32_t SectorSize, uint8_t * Buf)
{
	return SDC_CPRM_read_protect_area(StartBlk, BlkCount, SectorSize, Buf);
}

uint8_t SDC_CPRM_read_protect_area(uint32_t startblk, uint32_t blockcnt, uint32_t sector_length, uint8_t * data_buf)
{
	uint32_t start_addr;
	uint32_t ks_lo = 0, ks_hi = 0;
	uint32_t arg;
	uint32_t i;
	uint32_t protect_data_lo, protect_data_hi;
	uint8_t *data_buf_p;
	uint32_t data_buf_swap;
	// For controlling the times of writing media-key in the None-auto DCBC
	uint8_t throttle = 0;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	// (cardInfo.CSD_LO >> 14) & 0xF is the field in CSD register indicate the max write data block length
	start_addr = startblk << ((cardInfo.CSD_LO >> 14) & 0xF);

#ifdef DEBUG_CPRM
	printf("===> AKE Read, Arg = %0.8x\r\n", (blockcnt << 24) | ((start_addr >> 9) & 0x00FFFFFF));
#endif

	arg = (blockcnt << 24) | ((start_addr >> 9) & 0x00FFFFFF);

	ftsdc021_ops_send_sd_status(&cardInfo);

	if (SDC_CPRM_AKE(arg, &ks_lo, &ks_hi)) {
		printf("Fail in SDC_CPRM_read_protect_area function\r\n");
		return 1;
	}
#ifdef DEBUG_CPRM
	printf("Session Key: 0x%0.8x %0.8x\r\n", ks_hi, ks_lo);
#endif

	ftsdc021_ops_send_sd_status(&cardInfo);
	if (!cardInfo.sd_status.SECURED_MODE){
		printf("Card not in the Secure mode.\r\n");
		return 1;
	}

	// Auto mode of DCBC
	if (auto_ed_cbc == 1) {
		if (SDC_CPRM_Auto_DCBC(ks_lo, ks_hi, 1, 1)) {
			printf("Fail in SDC_CPRM_Auto_DCBC function.\r\n");
			return 1;
		}
	}

	/* CMD 16 */
	/* It's necessary to set the block length before multiple read block for certain card 
	   although the "SD Security Spec" says "no". */
	ftsdc021_ops_set_blocklen(&cardInfo, sector_length);
	/* CMD 55 */
	if (ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1)) {
		return 1;
	}
	/* CMD18 */
	data.act = READ;
	data.bufAddr = (uint32_t *) data_buf;
	data.blkSz = sector_length;
	data.blkCnt = blockcnt;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_READ_MULTI_BLOCK;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = startblk;
	ftsdc021_send_command(&cmd);
	if (ftsdc021_transfer_data(READ, (uint32_t *) data_buf, (blockcnt * sector_length))) {
		return 1;
	}


	if (auto_ed_cbc == 1) {
		// Disable the Auto DCBC function
		gpReg->Ciph_M_Ctl = 0;
	} else {
		data_buf_p = data_buf;
		for (i = 0; i < (blockcnt * sector_length) / 8; i++) {
			protect_data_lo = *((uint32_t *) (data_buf_p));
			protect_data_hi = *((uint32_t *) (data_buf_p + 4));
			data_buf_swap = protect_data_lo;
			protect_data_lo =
			    (((protect_data_hi & 0xFF) << 24) | ((protect_data_hi & 0xFF00) << 8) |
			     ((protect_data_hi & 0xFF0000) >> 8) | ((protect_data_hi & 0xFF000000) >> 24));
			protect_data_hi =
			    (((data_buf_swap & 0xFF) << 24) | ((data_buf_swap & 0xFF00) << 8) |
			     ((data_buf_swap & 0xFF0000) >> 8) | ((data_buf_swap & 0xFF000000) >> 24));

			*((uint32_t *) (data_buf_p)) = protect_data_lo;
			*((uint32_t *) (data_buf_p + 4)) = protect_data_hi;
			data_buf_p = data_buf_p + 8;
		}

		/*Initial the DCBC engine */
		/*The next-round mediakey will be generated automatically by DCBC engine according to the C-CBC algorithm. */
		/*So, we only initial the mediakey at first time. The media of key in next-round (next 8 bytes) is different with 
		   setting in the initilization */

		data_buf_p = data_buf;
		for (i = 0; i < (blockcnt * sector_length) / 8; i++) {
			protect_data_lo = *((uint32_t *) (data_buf_p));
			protect_data_hi = *((uint32_t *) (data_buf_p + 4));

			if (throttle == 0) {
				gpReg->Ciph_M_Ctl = C2_Decry_CCBC;
				gpReg->In_Data_Lo = protect_data_lo;
				gpReg->In_Data_Hi = protect_data_hi;
				gpReg->In_Key_Lo = ks_lo;
				gpReg->In_Key_Hi = ks_hi;
				/*Busy-wait for the coming of plaintext */
				while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
				gpReg->Ciph_M_Sts = 1;

				*((uint32_t *) (data_buf_p)) = gpReg->Out_Data_Lo;
				*((uint32_t *) (data_buf_p + 4)) = gpReg->Out_Data_Hi;

				throttle = 1;
			} else {
				gpReg->In_Data_Lo = protect_data_lo;
				gpReg->In_Data_Hi = protect_data_hi;

				while (((gpReg->Ciph_M_Sts) & 0x1) != 1) ;
				gpReg->Ciph_M_Sts = 1;

				*((uint32_t *) (data_buf_p)) = gpReg->Out_Data_Lo;
				*((uint32_t *) (data_buf_p + 4)) = gpReg->Out_Data_Hi;
			}

			data_buf_p = data_buf_p + 8;
		}
	}

	return 0;
}

uint8_t SDC_Erase_Protected_Sector(SDCardInfo * SDCard, uint32_t StartBlk, uint32_t BlkCount, uint32_t SectorSize)
{
	SDC_CPRM_erase_protect_area(StartBlk, BlkCount, SectorSize);
	return 1;
}

uint8_t SDC_CPRM_erase_protect_area(uint32_t startblk, uint32_t blockcnt, uint32_t sector_length)
{
	uint32_t start_addr;
	uint32_t ks_lo = 0, ks_hi = 0;
	uint32_t arg;
	struct mmc_cmd cmd = {0};

	start_addr = startblk << ((cardInfo.CSD_LO >> 14) & 0xF);

#ifdef DEBUG_CPRM
	printf("===> AKE Erase, Arg = %0.8x\r\n", (blockcnt << 24) | ((start_addr >> 9) & 0x00FFFFFF));
#endif

	arg = (blockcnt << 24) | ((start_addr >> 9) & 0x00FFFFFF);

	ftsdc021_ops_send_sd_status(&cardInfo);

	if (SDC_CPRM_AKE(arg, &ks_lo, &ks_hi)) {
		printf("Fail in SDC_CPRM_erase_protect_area function\r\n");
		return 1;
	}
#ifdef DEBUG_CPRM
	printf("Session Key: 0x%0.8x %0.8x\r\n", ks_hi, ks_lo);
#endif

	ftsdc021_ops_send_sd_status(&cardInfo);

	/* CMD 55 */
	if (ftsdc021_ops_app_cmd(&cardInfo, cardInfo.RCA << 16, 1)) {
		return 1;
	}

	/* ACMD 38 */
	cmd.CmdIndex = SDHCI_ERASE;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
	cmd.InhibitDATChk = 1;
	cmd.Argu = arg;
	if (ftsdc021_send_command(&cmd)) {
		printf("CPRM: Erase command failed !\r\n");
		return 1;
	}

	/* The next step is allowed when the card's state backs from programming to transferring state. */
	do {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## ... Get Card Status failed !\r\n");
			return 1;
		}
	} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);

	return 0;
}

uint8_t SDC_CPRM_protect_area_access_test(uint32_t startblk, uint32_t blockcnt)
{
	uint32_t *wr_buf, *rd_buf;
	uint32_t sector_length, i;

	sector_length = 512;

	wr_buf = (uint32_t *)write_buf;
	rd_buf = (uint32_t *)read_buf;

	if (auto_ed_cbc == 1 && cardInfo.FlowSet.UseDMA == PIO) {
		printf("CPRM: Auto ECBC can not use PIO.\r\n");
		return 1;
		/* Auto ECBC or DCBC does not support PIO, force to ADMA */
		//ftsdc021_set_transfer_type(ADMA, 2048, 3);
	}

	// Write the data into protected area, first. 
	if (SDC_CPRM_write_protect_area(startblk, blockcnt, sector_length,
					(uint8_t *)((uintptr_t)wr_buf))) {
		printf("CPRM: Write Failed addr %d cnt %d.\r\n", startblk, blockcnt);
		return 1;
	}

	memset(rd_buf, 0, (blockcnt * sector_length));
	// After writing, we read the data from protected area for comparing.
	if (SDC_CPRM_read_protect_area(startblk, blockcnt, sector_length,
				       (uint8_t *)((uintptr_t)rd_buf))) {
		printf("CPRM: Read Failed addr %d cnt %d.\r\n", startblk, blockcnt);
		return 1;
	}

	for (i = 0; i < (blockcnt * sector_length) / 4; i++) {
		if (*(wr_buf + i) != *(rd_buf + i)) {
			printf("Fail in comparing data, addr %d cnt %d.\r\n", startblk, blockcnt);
			printf("num:%d Source is 0x%0.2x but Dest is 0x%0.2x\r\n", i, *(wr_buf + i), *(rd_buf + i));
			return 1;
		}
	}

	// Erasing the specified protected area which is writed before.
	if (SDC_CPRM_erase_protect_area(startblk, blockcnt, sector_length)) {
		printf("CPRM: Erase Failed addr %d cnt %d.\r\n", startblk, blockcnt);
		return 1;
	}
	// Reading the data from protected area again for verifing the function of erasing.
	if (SDC_CPRM_read_protect_area(startblk, blockcnt, sector_length,
				       (uint8_t *)((uintptr_t)rd_buf))) {
		printf("Fail in SDC_CPRM_read_protect_area function\r\n");
		return 1;
	}

	for (i = 0; i < (blockcnt * sector_length) / 4; i++) {
		if (*(rd_buf + i) != 0) {
			printf("Fail in erasing\r\n");
			return 0;
		}
	}

	printf("CPRM read/write addr %d cnt %d passed.\r\n", startblk, blockcnt);

	return 0;
}

uint8_t SDC_CPRM_Init(void)
{
	uint16_t i;
	uint32_t *MKB_Buf;
	uint32_t km_lo = 0, km_hi = 0;
	uint32_t kum_lo = 0, kum_hi = 0;
	uint32_t Blockcnt = 20;
	uint8_t MKB_ID = 0, tmp;
	uint16_t Offset = 0;

	/* Already Init */
	if (cardInfo.Kmu_LO && cardInfo.Kmu_HI)
		return 0;

	// Set the sec_access_en bit when we want to access secret constant table.
	gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl | 0x100;

	// Write the secret table into
	for (i = 0; i < 256; i++) {
		gpReg->Secr_Table_Port = SecretConstant_020[i];
	}
#ifdef DEBUG_CPRM
	printf("Constant Table:\r\n");
	for (i = 0; i < 256; i++) {
		tmp = gpReg->Secr_Table_Port;
		
		if (tmp != SecretConstant_020[i]) {
			printf("ERR## ... Secret Table Error ..(0x%x, 0x%0x)\r\n",
				    tmp, SecretConstant_020[i]);
			return 1;
		}

		printf("%x ", tmp);
	}
	printf("\r\n");
#endif

	// Closing the sec_access_en bit when we finish the feed
	gpReg->Ciph_M_Ctl = gpReg->Ciph_M_Ctl & ~0x100;
	MKB_Buf = (uint32_t *)read_buf;
	memset(MKB_Buf, 0, Blockcnt * 512);

	if (SDC_CPRM_GetMKB(MKB_Buf, Blockcnt, MKB_ID, Offset)) {
		printf("Getting the MKB from SD is failed.\r\n");
		return 1;
	}

/*	#ifdef DEBUG_CPRM	
		// Show the beginning 1024byte of MKB	
		for(i=0;i<2560;i++)
		{
			printf("0x%x ",*(MKB_Buf+ i));
		}
		printf("\r\n");
	#endif*/

	if (SDC_CPRM_processMKB(MKB_Buf, Blockcnt, &km_lo, &km_hi)) {
		printf("Processing MKB from SD is failed.\r\n");
		return 1;
	}
#ifdef DEBUG_CPRM
	printf("Key after processing MKB %x %x\r\n", km_hi, km_lo);
#endif

	if (SDC_CPRM_generate_MUK(km_lo, km_hi, &kum_lo, &kum_hi)) {
		printf("Getting the Media Unique Key from SD is failed.\r\n");
		return 1;
	}
#ifdef DEBUG_CPRM
	printf("Key from generation unique key is %0.8x %0.8x\r\n", kum_hi, kum_lo);
#endif

	cardInfo.Kmu_LO = kum_lo;
	cardInfo.Kmu_HI = kum_hi;

	return 0;
}

uint8_t SDC_CPRM(uint32_t start, uint32_t cnt)
{
	if (!(gpReg->HWAttributes & 0x40)) {
		printf(" FPGA does not include CPRM module.\r\n");
		return 1;
	}	

		if (SDC_CPRM_Init())
			return 1;

		if (SDC_CPRM_protect_area_access_test(start, cnt))
			return 1;

	return 0;
}
