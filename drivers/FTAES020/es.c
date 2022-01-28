/***************************************************************************
* Copyright  Faraday Technology Corp 2004-2005.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:main.c                                                              *
* Description: CPE Burn IN testing program                    		       *
* Author: W.D.Shih                                              		   *
* Date: 2004/03/9                                                          *
* Version:0.10                                                             *
*--------------------------------------------------------------------------*
****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//RichrdLin
#include "SoFlexible.h"

#include "es.h"

//RichrdLin
#define IPMODULE AES
#define IPNAME FTAES020


extern void WordEndianChange(UINT32 *dt, INT32 len);
extern int ParseCmd(char* Buffer);

BOOL ES_DMA_test_func(void);

//volatile BOOL ES_USE_INT = TRUE; // Encrypt Security using interrput (TRUE) or polling (FALSE) mode
BOOL ES_USE_INT = FALSE; // Encrypt Security using interrput (TRUE) or polling (FALSE) mode
UINT8 volatile DMA_INT_OK;
UINT8 volatile DMA_STOP_OK;
UINT32 volatile aes_base, aes_irq;


UINT32 DES_IV[5][2] = {
					 //ECB
					 {0x00000000, 0x00000000},
					 //CBC
					 {0x12345678, 0x90abcdef},
					 //CTR
					 {0x12345678, 0x90abcdef},
					 //OFB
					 {0x12345678, 0x90abcdef},
					 //CFB8
					 {0x12345678, 0x90abcdef}};

UINT32 DES_Key[5][2] = {
					 //ECB
					 {0x01010101, 0x01010101},
					 //CBC
					 {0x01234567, 0x89abcdef},
					 //CTR
					 {0x01234567, 0x89abcdef},
					 //OFB
					 {0x01234567, 0x89abcdef},
					 //CFB8
					 {0x01234567, 0x89abcdef}};

UINT32 DES_PlainData[5][6] = {
					 //ECB
					 {0x80000000, 0x00000000, 0x40000000, 0x00000000, 0x20000000, 0x00000000},
					 //CBC
					 {0x4e6f7720, 0x69732074, 0x68652074, 0x696d6520, 0x666f7220, 0x616c6c20},
					 //CTR
					 {0x4e6f7720, 0x69732074, 0x43e93400, 0x8c389c0f, 0x68378849, 0x9a7c05f6},
					 //OFB
					 {0x4e6f7720, 0x69732074, 0x43e93400, 0x8c389c0f, 0x68378849, 0x9a7c05f6},
					 //CFB8
					 {0x4e6f7720, 0x69732074, 0, 0, 0, 0},};

UINT32 DES_CipherData[5][6] = {
					 //ECB
					 {0x95F8A5E5, 0xDD31D900, 0xDD7F121C, 0xA5015619, 0x2E865310, 0x4F3834EA},
					 //CBC
					 {0xe5c7cdde, 0x872bf27c, 0x43e93400, 0x8c389c0f, 0x68378849, 0x9a7c05f6},
					 //CTR
					 {0xf3096249, 0xc7f46e51, 0x3db698d4, 0x1a9cb508, 0xf4777a9d, 0x7ba806a3},
					 //OFB
					 {0xf3096249, 0xc7f46e51, 0x1e7e5e50, 0xcbbec410, 0x3335a18a, 0xde4a9115},
					 //CFB8
					 {0xF31fda07, 0x011462ee, 0,		  0,		  0,		  0},};

UINT32 Triple_DES_IV[5][2] = {
					 //ECB
					 {0x00000000, 0x00000000},
					 //CBC
					 {0x12345678, 0x90abcdef},
					 //CTR
					 {0x12345678, 0x90abcdef},
					 //OFB
					 {0x12345678, 0x90abcdef},
					 //CFB8
					 {0x12345678, 0x90abcdef}};

UINT32 Triple_DES_Key[5][6] = {
					 //ECB
					 {0x01234567, 0x89abcdef, 0x23456789, 0xabcdef01, 0x456789ab, 0xcdef0123},
					 //CBC
					 {0x01234567, 0x89abcdef, 0x23456789, 0xabcdef01, 0x456789ab, 0xcdef0123},
					 //CTR
					 {0x01234567, 0x89abcdef, 0x23456789, 0xabcdef01, 0x456789ab, 0xcdef0123},
					 //OFB
					 {0x01234567, 0x89abcdef, 0x23456789, 0xabcdef01, 0x456789ab, 0xcdef0123},
					 //CFB8
					 {0x01234567, 0x89abcdef, 0x23456789, 0xabcdef01, 0x456789ab, 0xcdef0123},};

UINT32 Triple_DES_PlainData[5][6] = {
					 //ECB
					 {0x4e6f7720, 0x69732074, 0x43e93400, 0x8c389c0f, 0x68378849, 0x9a7c05f6},
					 //CBC
					 {0x4e6f7720, 0x69732074, 0x43e93400, 0x8c389c0f, 0x68378849, 0x9a7c05f6},
					 //CTR
					 {0x4e6f7720, 0x69732074, 0x43e93400, 0x8c389c0f, 0x68378849, 0x9a7c05f6},
					 //OFB
					 {0x4e6f7720, 0x69732074, 0x43e93400, 0x8c389c0f, 0x68378849, 0x9a7c05f6},
					 //CFB8
					 {0x0000007f, 0 		, 0			, 0			, 0			, 0			},};

UINT32 Triple_DES_CipherData[5][6] = {
					 //ECB
					 {0x314f8327, 0xfa7a09a8, 0xd5895fad, 0xe98faedf, 0x98f470eb, 0x3553a5da},
					 //CBC
					 {0xf3c0ff02, 0x6c023089, 0xc43add8f, 0xd8cd5e43, 0x2bfd41d3, 0x130bcf40},
					 //CTR
					 {0xee7ec75c, 0x1a101301, 0xc9e6da0b, 0x9d0c9e68, 0x21620426, 0xb8c34113},
					 //OFB
					 {0xee7ec75c, 0x1a101301, 0xb1067574, 0xe7337728, 0x89ba7090, 0x1736d15f},
					 //CFB8
					 {0xa0d463a4, 0xa0633d1f, 0			, 0			, 0			, 0			},};


//////////////////////////////////////////////////////////////////////////


UINT32 Key[16][8] = {0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, 0		   , 0		   , 0         , 0,
					 0x8e73b0f7, 0xda0e6452, 0xc810f32b, 0x809079e5, 0x62f8ead2, 0x522c6b7b, 0		   , 0,
					 0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781, 0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4,

					 0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, 0		   , 0		   , 0         , 0,
					 0x8e73b0f7, 0xda0e6452, 0xc810f32b, 0x809079e5, 0x62f8ead2, 0x522c6b7b, 0		   , 0,
					 0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781, 0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4,

					 0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, 0		   , 0		   , 0         , 0,
					 0x8e73b0f7, 0xda0e6452, 0xc810f32b, 0x809079e5, 0x62f8ead2, 0x522c6b7b, 0		   , 0,
					 0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781, 0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4,

					 0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, 0		   , 0		   , 0         , 0,
					 0x8e73b0f7, 0xda0e6452, 0xc810f32b, 0x809079e5, 0x62f8ead2, 0x522c6b7b, 0		   , 0,
					 0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781, 0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4,

					 0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, 0		   , 0		   , 0         , 0,
					 0x8e73b0f7, 0xda0e6452, 0xc810f32b, 0x809079e5, 0x62f8ead2, 0x522c6b7b, 0		   , 0,
					 0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781, 0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4,

					 0xE297F8CC, 0x32B3FE60, 0x6CBC078E, 0x03568BD2, 0x86BAEA6E, 0x60D35D84, 0		   , 0};

UINT32 IV[16][4] = {0, 0, 0, 0,
				    0, 0, 0, 0,
				    0, 0, 0, 0,

				    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
				    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
				    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,

				    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
				    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
				    0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,

				 	0xf0f1f2f3, 0xf4f5f6f7, 0xf8f9fafb, 0xfcfdfeff,
					0xf0f1f2f3, 0xf4f5f6f7, 0xf8f9fafb, 0xfcfdfeff,
					0xf0f1f2f3, 0xf4f5f6f7, 0xf8f9fafb, 0xfcfdfeff,

					0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
					0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
					0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,

					0x25B73AD8, 0x931FA221, 0x740240BE, 0x25B1BA10};

UINT32 Plain[2][16] = {0x6bc1bee2, 0x2e409f96, 0xe93d7e11, 0x7393172a,
					   0xae2d8a57, 0x1e03ac9c, 0x9eb76fac, 0x45af8e51,
					   0x30c81c46, 0xa35ce411, 0xe5fbc119, 0x1a0a52ef,
					   0xf69f2445, 0xdf4f9b17, 0xad2b417b, 0xe66c3710,
					   //CFB_Plain
 					   0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
 					   0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a//, 0xae, 0x2d
 					   };

UINT32 Cipher[15][16] = {//ECB_AES128_Encrypt
						0x3ad77bb4, 0x0d7a3660, 0xa89ecaf3, 0x2466ef97,
					    0xf5d3d585, 0x03b9699d, 0xe785895a, 0x96fdbaaf,
					    0x43b1cd7f, 0x598ece23, 0x881b00e3, 0xed030688,
					    0x7b0c785e, 0x27e8ad3f, 0x82232071, 0x04725dd4,
					    //ECB_AES192_Encrypt
					    0xbd334f1d, 0x6e45f25f, 0xf712a214, 0x571fa5cc,
					    0x97410484, 0x6d0ad3ad, 0x7734ecb3, 0xecee4eef,
					    0xef7afd22, 0x70e2e60a, 0xdce0ba2f, 0xace6444e,
					    0x9a4b41ba, 0x738d6c72, 0xfb166916, 0x03c18e0e,
					    //ECB_AES256_Encrypt
					    0xf3eed1bd, 0xb5d2a03c, 0x064b5a7e, 0x3db181f8,
					    0x591ccb10, 0xd410ed26, 0xdc5ba74a, 0x31362870,
					    0xb6ed21b9, 0x9ca6f4f9, 0xf153e7b1, 0xbeafed1d,
					    0x23304b7a, 0x39f9f3ff, 0x067d8d8f, 0x9e24ecc7,

					    //CBC_AES128_Encrypt
					    0x7649abac, 0x8119b246, 0xcee98e9b, 0x12e9197d,
					    0x5086cb9b, 0x507219ee, 0x95db113a, 0x917678b2,
					    0x73bed6b8, 0xe3c1743b, 0x7116e69e, 0x22229516,
					    0x3ff1caa1, 0x681fac09, 0x120eca30, 0x7586e1a7,
					    //CBC_AES192_Encrypt
					    0x4f021db2, 0x43bc633d, 0x7178183a, 0x9fa071e8,
					    0xb4d9ada9, 0xad7dedf4, 0xe5e73876, 0x3f69145a,
					    0x571b2420, 0x12fb7ae0, 0x7fa9baac, 0x3df102e0,
					    0x08b0e279, 0x88598881, 0xd920a9e6, 0x4f5615cd,
					    //CBC_AES256_Encrypt
					    0xf58c4c04, 0xd6e5f1ba, 0x779eabfb, 0x5f7bfbd6,
					    0x9cfc4e96, 0x7edb808d, 0x679f777b, 0xc6702c7d,
					    0x39f23369, 0xa9d9bacf, 0xa530e263, 0x04231461,
					    0xb2eb05e2, 0xc39be9fc, 0xda6c1907, 0x8c6a9d1b,

						//OFB_AES128_Encrypt
						0x3b3fd92e, 0xb72dad20, 0x333449f8, 0xe83cfb4a,
					    0x7789508d, 0x16918f03, 0xf53c52da, 0xc54ed825,
					    0x9740051e, 0x9c5fecf6, 0x4344f7a8, 0x2260edcc,
					    0x304c6528, 0xf659c778, 0x66a510d9, 0xc1d6ae5e,
					    //OFB_AES192_Encrypt
					    0xcdc80d6f, 0xddf18cab, 0x34c25909, 0xc99a4174,
					    0xfcc28b8d, 0x4c63837c, 0x09e81700, 0xc1100401,
					    0x8d9a9aea, 0xc0f6596f, 0x559c6d4d, 0xaf59a5f2,
					    0x6d9f2008, 0x57ca6c3e, 0x9cac524b, 0xd9acc92a,
					    //OFB_AES256_Encrypt
					    0xdc7e84bf, 0xda79164b, 0x7ecd8486, 0x985d3860,
					    0x4febdc67 ,0x40d20b3a, 0xc88f6ad8, 0x2a4fb08d,
					    0x71ab47a0 ,0x86e86eed, 0xf39d1c5b, 0xba97c408,
					    0x0126141d ,0x67f37be8, 0x538f5a8b, 0xe740e484,

						//CTR_AES128_Encrypt
						0x874d6191, 0xb620e326, 0x1bef6864, 0x990db6ce,
					    0x9806f66b, 0x7970fdff, 0x8617187b, 0xb9fffdff,
					    0x5ae4df3e, 0xdbd5d35e, 0x5b4f0902, 0x0db03eab,
					    0x1e031dda, 0x2fbe03d1, 0x792170a0, 0xf3009cee,
					    //CTR_AES192_Encrypt
					    0x1abc9324, 0x17521ca2, 0x4f2b0459, 0xfe7e6e0b,
					    0x090339ec, 0x0aa6faef, 0xd5ccc2c6, 0xf4ce8e94,
					    0x1e36b26b, 0xd1ebc670, 0xd1bd1d66, 0x5620abf7,
					    0x4f78a7f6, 0xd2980958, 0x5a97daec, 0x58c6b050,
					    //CTR_AES256_Encrypt
					    0x601ec313, 0x775789a5, 0xb7a7f504, 0xbbf3d228,
					    0xf443e3ca, 0x4d62b59a, 0xca84e990, 0xcacaf5c5,
					    0x2b0930da, 0xa23de94c, 0xe87017ba, 0x2d84988d,
					    0xdfc9c58d, 0xb67aada6, 0x13c2dd08, 0x457941a6,

					    //CFB8_AES128_Encrypt
						0x3b79424c, 0x9c0dd436, 0xbace9e0e, 0xd4586a4f, //0x32, 0xb9,
						0,          0,          0,          0,
						0,          0,          0,          0,
						0,          0,          0,          0,

						//CFB8_AES192_Encrypt
						0xcda2521e, 0xf0a905ca, 0x44cd057c, 0xbf0d47a0, //0x67, 0x8a,
						0,          0,          0,          0,
						0,          0,          0,          0,
						0,          0,          0,          0,

						//CFB8_AES256_Encrypt
						0xdc1f1a85, 0x20a64db5, 0x5fcc8ac5, 0x54844e88,//, 0x97, 0x0
						0,          0,          0,          0,
						0,          0,          0,          0,
						0,          0,          0,          0,
					    };

/***************************************************************************
***************************************************************************/

void ES_WriteReg(UINT32 Offset, UINT32 RegValue)
{
	*((UINT32 *)(aes_base + Offset)) = RegValue;
}

UINT32 ES_ReadReg(UINT32 Offset)
{
	return *((UINT32 *)(aes_base + Offset));
}

void ES_SetFIFO_Threshold(UINT32 InFIFO_Threshold, UINT32 OutFIFO_Threshold)
{
UINT32 data;

	data = ((OutFIFO_Threshold & 0xFF) << 8) | InFIFO_Threshold & 0xFF;
  	ES_WriteReg(SEC_FIFOThreshold, data);
}

BOOL ES_WaitSecurityReady()
{
	UINT32 Status;
	while(1)
	{
		Status = ES_ReadReg(SEC_IntrStatus);

		if(Status & Data_done)
			return TRUE;
	}
	return FALSE;
}


BOOL AES_DMA_E(UINT32 mode, UINT32 Algorithm)
{
	UINT32 i, *Buf, group;
	switch(mode)
	{
		case ECB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 0;
			if(Algorithm == Algorithm_AES_192)
				group = 1;
			if(Algorithm == Algorithm_AES_256)
				group = 2;
			break;

		case CBC_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 3;
			if(Algorithm == Algorithm_AES_192)
				group = 4;
			if(Algorithm == Algorithm_AES_256)
				group = 5;
			break;

		case OFB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 6;
			if(Algorithm == Algorithm_AES_192)
				group = 7;
			if(Algorithm == Algorithm_AES_256)
				group = 8;
			break;

		case CTR_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 9;
			if(Algorithm == Algorithm_AES_192)
				group = 10;
			if(Algorithm == Algorithm_AES_256)
				group = 11;
			break;

		case CFB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 12;
			if(Algorithm == Algorithm_AES_192)
				group = 13;
			if(Algorithm == Algorithm_AES_256)
				group = 14;
			break;

		default:
			break;
	}
	Buf = malloc(16 * 4);
//       printf("Buf = 0x%p\n", Buf);
	//1.	Set EncryptControl register
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm | mode | Encrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm | mode | Encrypt_Stage);
	//2.	Set Initial vector IV

	ES_WriteReg(SEC_DESIVH, IV[group][0]);
	ES_WriteReg(SEC_DESIVL, IV[group][1]);
	ES_WriteReg(SEC_AESIV2, IV[group][2]);
	ES_WriteReg(SEC_AESIV3, IV[group][3]);
	//3.	Set Key value
	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, Key[group][i]);
	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, Plain[0]);
	ES_WriteReg(SEC_DMADes, Buf);
	if((group / 12) >= 1)
		ES_WriteReg(SEC_DMATrasSize, 16);
	else
		ES_WriteReg(SEC_DMATrasSize, 16 * 4);
	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);
	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
			if(!ES_WaitSecurityReady())
				return FALSE;
	}
	if((group / 12) >= 1)
		Compare_Data(4, Cipher[group], Buf);
	else
		Compare_Data(16, Cipher[group], Buf);
	free(Buf);
	return TRUE;
}

BOOL AES_DMA_D(UINT32 mode, UINT32 Algorithm)
{
	UINT32 i, *Buf, group;
	switch(mode)
	{
		case ECB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 0;
			if(Algorithm == Algorithm_AES_192)
				group = 1;
			if(Algorithm == Algorithm_AES_256)
				group = 2;
			break;

		case CBC_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 3;
			if(Algorithm == Algorithm_AES_192)
				group = 4;
			if(Algorithm == Algorithm_AES_256)
				group = 5;
			break;

		case OFB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 6;
			if(Algorithm == Algorithm_AES_192)
				group = 7;
			if(Algorithm == Algorithm_AES_256)
				group = 8;
			break;

		case CTR_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 9;
			if(Algorithm == Algorithm_AES_192)
				group = 10;
			if(Algorithm == Algorithm_AES_256)
				group = 11;
			break;

		case CFB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 12;
			if(Algorithm == Algorithm_AES_192)
				group = 13;
			if(Algorithm == Algorithm_AES_256)
				group = 14;
			break;

		default:
			break;
	}

	Buf = malloc(16 * 4);

	//1.	Set EncryptControl register
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm | mode | Decrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm | mode | Decrypt_Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, IV[group][0]);
	ES_WriteReg(SEC_DESIVL, IV[group][1]);
	ES_WriteReg(SEC_AESIV2, IV[group][2]);
	ES_WriteReg(SEC_AESIV3, IV[group][3]);

	//3.	Set Key value
	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, Key[group][i]);

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, Cipher[group]);
	ES_WriteReg(SEC_DMADes, Buf);

	if((group / 12) >= 1)
		ES_WriteReg(SEC_DMATrasSize, 16);
	else
		ES_WriteReg(SEC_DMATrasSize, 16 * 4);

	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
			if(!ES_WaitSecurityReady())
				return FALSE;
	}

	if((group / 12) >= 1)
		Compare_Data(4, Plain[0], Buf);
	else
		Compare_Data(16, Plain[0], Buf);

	free(Buf);
	return TRUE;
}


BOOL Triple_DES_DMA_E(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *CipherDataPtr)
{
	UINT32 i, group;

	switch(mode)
	{
		case ECB_mode:
			group = 0;
			break;

		case CBC_mode:
			group = 1;
			break;

		case OFB_mode:
			group = 2;
			break;

		case CTR_mode:
			group = 3;
			break;

		case CFB_mode:
			group = 4;
			break;

		default:
			break;
	}

	//1.	Set EncryptControl register
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_Triple_DES | mode | Encrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_Triple_DES | mode | Encrypt_Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, *IVPtr++);
	ES_WriteReg(SEC_DESIVL, *IVPtr);

	//3.	Set Key value
	for(i = 0; i < 6; i++, KeyPtr++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, *KeyPtr);

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, PlainDataPtr);
	ES_WriteReg(SEC_DMADes, CipherDataPtr);

	ES_WriteReg(SEC_DMATrasSize, Size);

	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
			if(!ES_WaitSecurityReady())
				return FALSE;
	}

	return TRUE;
}

BOOL Triple_DES_DMA_D(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *CipherDataPtr)
{
	UINT32 i;
	//1.	Set EncryptControl register
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_Triple_DES | mode | Decrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_Triple_DES | mode | Decrypt_Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, *IVPtr++);
	ES_WriteReg(SEC_DESIVL, *IVPtr);

	//3.	Set Key value
	for(i = 0; i < 6; i++, KeyPtr++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, *KeyPtr);

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, PlainDataPtr);
	ES_WriteReg(SEC_DMADes, CipherDataPtr);

	ES_WriteReg(SEC_DMATrasSize, Size);

	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
		if(!ES_WaitSecurityReady())
			return FALSE;
	}

	return TRUE;
}

BOOL DES_DMA_E(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *Buf)
{
	UINT32 i;

	//1.	Set EncryptControl register
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_DES | mode | Encrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_DES | mode | Encrypt_Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, *IVPtr);
	ES_WriteReg(SEC_DESIVL, *(IVPtr + 1));

	//3.	Set Key value
	for(i = 0; i < 2; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, *(KeyPtr + i));

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, PlainDataPtr);
	ES_WriteReg(SEC_DMADes, Buf);

	ES_WriteReg(SEC_DMATrasSize, Size);
	
	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
		if(!ES_WaitSecurityReady())
			return FALSE;
	}

	return TRUE;
}

BOOL DES_DMA_D(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *Buf)
{
	UINT32 i;

	//1.	Set EncryptControl register
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_DES | mode | Decrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_DES | mode | Decrypt_Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, *IVPtr);
	ES_WriteReg(SEC_DESIVL, *(IVPtr + 1));

	//3.	Set Key value
	for(i = 0; i < 2; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, *(KeyPtr + i));

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, PlainDataPtr);
	ES_WriteReg(SEC_DMADes, Buf);

	ES_WriteReg(SEC_DMATrasSize, Size);

	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
		if(!ES_WaitSecurityReady())
			return FALSE;
	}

	return TRUE;
}


BOOL Compare_Data(UINT32 Word_Size, UINT32 *Buf_1, UINT32 *Buf_2)
{
	UINT32 i, Flag;

	Flag = TRUE;
	for(i = 0; i < Word_Size; i++)
	{
		if(*(Buf_1 + i) != *(Buf_2 + i))
		{
			printf("num %d Output = %0.8x, correct = %0.8x\n",i, *(Buf_1 + i), *(Buf_2 + i));
			Flag = FALSE;
		}
	}

	if(Flag == FALSE)
	{
		printf("Compare Cipher Fail\n");
		//RichardLin
		while(1){
		}
	}
	else
	{
		//printf("Compare Cipher Pass\n");
	}

	return Flag;
}

BOOL ES_Large_DMA(UINT32 Stage, UINT32 mode, UINT32 Algorithm, UINT32 *InBuf, UINT32 *OutBuf, int Size)
{
	#define RichardLin_Test_ES_Large_DMA
	
	UINT32 i, group, section, data;

	switch(mode)
	{
		case ECB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 0;
			else if(Algorithm == Algorithm_AES_192)
					group = 1;
				 else if(Algorithm == Algorithm_AES_256)
					group = 2;
				 else
				 	group = 1;
			break;

		case CBC_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 3;
			else if(Algorithm == Algorithm_AES_192)
					group = 4;
				 else if(Algorithm == Algorithm_AES_256)
					group = 5;
				 else
				 	group = 4;
			break;

		case OFB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 6;
			else if(Algorithm == Algorithm_AES_192)
					group = 7;
				 else if(Algorithm == Algorithm_AES_256)
					group = 8;
				 else
				 	group = 7;
			break;

		case CTR_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 9;
			else if(Algorithm == Algorithm_AES_192)
					group = 10;
				 else if(Algorithm == Algorithm_AES_256)
					group = 11;
				 else
				 	group = 10;
			break;

		case CFB_mode:
			if(Algorithm == Algorithm_AES_128)
				group = 12;
			else if(Algorithm == Algorithm_AES_192)
					group = 13;
				 else if(Algorithm == Algorithm_AES_256)
					group = 14;
				 else
				 	group = 13;
			break;

		case Special_mode:
			group = 15;
			break;

		default:
			break;
	}

	if(Algorithm == Algorithm_AES_256)
		section = 16;//32 X N
	else if(Algorithm == Algorithm_AES_192)
		section = 16;//24 X N
	else if(Algorithm == Algorithm_AES_128)
		section = 16;//16 X N	--4080pass, 256pass,64
	else
		section = 8;//512;//8;	//8 X N

	//1.	Set EncryptControl register
	if(mode == ECB_mode)
	{
		ES_WriteReg(SEC_EncryptControl, Algorithm | mode | Stage);
	}
	else
		if(mode != Special_mode)
			ES_WriteReg(SEC_EncryptControl, First_block | Algorithm | mode | Stage);
		else
			ES_WriteReg(SEC_EncryptControl, First_block | Algorithm | CFB_mode | Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, IV[group][0]);
	ES_WriteReg(SEC_DESIVL, IV[group][1]);
	ES_WriteReg(SEC_AESIV2, IV[group][2]);
	ES_WriteReg(SEC_AESIV3, IV[group][3]);

	//printf("Key = %x\n",Key[group][0]);
	//3.	Set Key value
	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, Key[group][i]);

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, InBuf);
	ES_WriteReg(SEC_DMADes, OutBuf);

	while(1)
	{
		if(Size >= section)
			ES_WriteReg(SEC_DMATrasSize, section);
		else
			ES_WriteReg(SEC_DMATrasSize, Size);

		Size -= section;

		//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
		ES_WriteReg(SEC_DMACtrl, DMA_Enable);

		//7.	Wait transfer size is complete
		if(ES_USE_INT)
		{
			while(1)
			{
				if(DMA_INT_OK == 1)
				{
					DMA_INT_OK = 0;
					break;
				}
			}
		}
		else
		{
			if(!ES_WaitSecurityReady())
				return FALSE;
		}

		if(Size <= 0)
			break;

		// Update IV value
		ES_WriteReg(SEC_DESIVH, ES_ReadReg(SEC_LAST_IV0));
		ES_WriteReg(SEC_DESIVL, ES_ReadReg(SEC_LAST_IV1));
		ES_WriteReg(SEC_AESIV2, ES_ReadReg(SEC_LAST_IV2));
		ES_WriteReg(SEC_AESIV3, ES_ReadReg(SEC_LAST_IV3));

		InBuf += section;
		OutBuf += section;

		#ifdef RichardLin_Test_ES_Large_DMA
		#else
		data = ES_ReadReg(SEC_DMASrc);
		if(data != (UINT32)InBuf)
			printf("SEC_DMASrc error, reg = %x, correct = %x\n", data,(UINT32)InBuf);

		data = ES_ReadReg(SEC_DMADes);
		if(data != (UINT32)OutBuf)
			printf("SEC_DMADes error, reg = %x, correct = %x\n", data,(UINT32)OutBuf);
		#endif

		//ES_WriteReg(SEC_DMASrc, InBuf);
		//ES_WriteReg(SEC_DMADes, OutBuf);
	}
	//Do_Delay(100);
	return TRUE;
}

BOOL Key_DMA_Test(UINT32 mode)
{
	UINT32 i, *Buf, group;

	Buf = malloc(16 * 4);
	//printf("Buf addr = %x\n",(UINT32)Buf);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, IV[5][0]);
	ES_WriteReg(SEC_DESIVL, IV[5][1]);
	ES_WriteReg(SEC_AESIV2, IV[5][2]);
	ES_WriteReg(SEC_AESIV3, IV[5][3]);

	if(mode == ECB_mode)
		group = 2;
	else if(mode == CBC_mode)
		group = 5;
	else
	{
		printf("mode error!\n");
		return FALSE;
	}

	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, Key[group][i]);

//	Algorithm_AES_128 Encrypt
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_AES_128 | mode | Encrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_128 | mode | Encrypt_Stage);

	ES_WriteReg(SEC_DMASrc, Plain[0]);
	ES_WriteReg(SEC_DMADes, Buf);
	ES_WriteReg(SEC_DMATrasSize, 16 * 4);
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
			if(!ES_WaitSecurityReady())
				return FALSE;
	}

//	Algorithm_AES_128 Decrypt
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_AES_128 | mode | Decrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_128 | mode | Decrypt_Stage);

	ES_WriteReg(SEC_DMASrc, Cipher[group]);
	ES_WriteReg(SEC_DMADes, Buf);
	ES_WriteReg(SEC_DMATrasSize, 16 * 4);
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
			if(!ES_WaitSecurityReady())
				return FALSE;
	}

////////////////////////////////////////////////////////////////////////////////////////////

//	Algorithm_AES_256 Encrypt
	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_AES_256 | mode | Encrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_256 | mode | Encrypt_Stage);

	ES_WriteReg(SEC_DMASrc, Plain[0]);
	ES_WriteReg(SEC_DMADes, Buf);
	ES_WriteReg(SEC_DMATrasSize, 16 * 4);
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
			if(!ES_WaitSecurityReady())
				return FALSE;
	}

	Compare_Data(16, Cipher[group], Buf);

//	Algorithm_AES_256 Decrypt

	if(mode == ECB_mode)
		ES_WriteReg(SEC_EncryptControl, Algorithm_AES_256 | mode | Decrypt_Stage);
	else
		ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_256 | mode | Decrypt_Stage);

	ES_WriteReg(SEC_DMASrc, Cipher[group]);
	ES_WriteReg(SEC_DMADes, Buf);
	ES_WriteReg(SEC_DMATrasSize, 16 * 4);
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
			if(!ES_WaitSecurityReady())
				return FALSE;
	}

	Compare_Data(16, Plain[0], Buf);

	free(Buf);
	return TRUE;
}

void fLib_ESIntHandler()
{
	#define RichardLin_Test_fLib_ESIntHandler //OKed in 081210 
	
	UINT32 Status, Result;

	#ifdef RichardLin_Test_fLib_ESIntHandler
	#else
	// this ip used level trigger, so we do not clear interrupt controller source
	fLib_ClearInt(IRQ_IAmIRQ, aes_irq); 
	#endif
	
	Status = ES_ReadReg(SEC_MaskedIntrStatus);

	Result = ES_ReadReg(SEC_IntrStatus);

	if ((Status & DMA_stop_int) != 0)
	{
		DMA_STOP_OK = 1;
		ES_WriteReg(SEC_ClearIntrStatus, DMA_stop_int);
	}

	if((Status & Data_receive_error) != 0)
	{
		ES_WriteReg(SEC_ClearIntrStatus, Data_receive_error);
		fLib_DebugPrintString("=> Data_receive_error INT");
	}

	if ((Status & Data_done) != 0)
	{
		DMA_INT_OK = 1;
		ES_WriteReg(SEC_ClearIntrStatus, Data_done);
	}

	//fLib_ClearInt(aes_irq);
}



BOOL MPCAAHBC_AES_test_func()
{
	UINT32 i, group, len, fail=0;
	UINT32 j, INT_Status;
	UINT8 *Buf, *tmpBuf1, *tmpBuf2;

	len = 4080;

	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        	fLib_SetIRQmode(aes_irq, LEVEL);
	 	ES_SetFIFO_Threshold(1, 1);
		ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt
		fLib_EnableIRQ( aes_irq);
	}
	else
	{
		DMA_INT_OK = 0;
		ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt
    		printf("Test DMA Polling\n");
	}
	Buf = (UINT8 *) malloc(len);
	tmpBuf1 = (UINT8 *) malloc(len);
	tmpBuf2 = (UINT8 *) malloc(len);

	for (i=0; i<len; i++)
		*(Buf+i) = i;

	//1.	Set EncryptControl register
	ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_128 | CBC_mode | Encrypt_Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, 0);
	ES_WriteReg(SEC_DESIVL, 1);
	ES_WriteReg(SEC_AESIV2, 2);
	ES_WriteReg(SEC_AESIV3, 3);

	//3.	Set Key value
	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, 2*i);

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, Buf);
	ES_WriteReg(SEC_DMADes, tmpBuf1);
	ES_WriteReg(SEC_DMATrasSize, len);

	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
		while(1)
		{
			INT_Status = ES_ReadReg(SEC_IntrStatus);
			if((INT_Status & Data_done) != 0)
			{
				DMA_INT_OK = 1;
				ES_WriteReg(SEC_ClearIntrStatus, Data_done);
			}
			if(DMA_INT_OK)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
j=0;
while (++j<=50000)
{
	ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_128 | CBC_mode | Decrypt_Stage);

	ES_WriteReg(SEC_DESIVH, 0);
	ES_WriteReg(SEC_DESIVL, 1);
	ES_WriteReg(SEC_AESIV2, 2);
	ES_WriteReg(SEC_AESIV3, 3);

	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, 2*i);

	ES_WriteReg(SEC_DMASrc, tmpBuf1);
	ES_WriteReg(SEC_DMADes, tmpBuf2);

	ES_WriteReg(SEC_DMATrasSize, len);
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	if(ES_USE_INT)
	{
		while(1)
		{
			volatile unsigned int ttemp;
			while (DMA_INT_OK == 0) {
				*((volatile unsigned int *) (0x92000080)) = 0x00004000;
				ttemp = *((volatile unsigned int *) (0x92000080));
				ttemp = *((volatile unsigned int *) (0x92000084));
				ttemp = *((volatile unsigned int *) (0x92000088));
				ttemp = *((volatile unsigned int *) (0x92000080));

			}

			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}
	else
	{
		// polling INT_Status to make sure DMA is finished.
		while(1)
                {    
                        INT_Status = ES_ReadReg(SEC_IntrStatus);
                        if((INT_Status & Data_done) != 0)
                        {    
                                DMA_INT_OK = 1; 
                                ES_WriteReg(SEC_ClearIntrStatus, Data_done);
                        }    
                        if(DMA_INT_OK)
                        {    
                                DMA_INT_OK = 0; 
                                break;
                        }    
                }    
	}
	if (memcmp(tmpBuf2, Buf, len)) {
		int start=0, stop=0;
		fail=1;
		printf("\ncompare failed at %d times\n\n", j);
		for (i=0; i<len; i++) {
			if ( *(Buf+i) != *(tmpBuf2+i) ) {
				if (!start&&!stop)
					start = stop = i;
				else
					stop++;
			} else if (start||stop)
				break;
		}
		printf("char failed from %d to %d\n", start, stop);
		goto exitt;
	}
	if (!(j%10000))
		printf("pass %d times\n", j);
}

if (!fail)
	printf("\nTotal %d times passed\n", j-1);
exitt:
	free(Buf);
	free(tmpBuf1);
	free(tmpBuf2);

//===========================================================
#if 1
	len=16;
	Buf = (UINT8 *) malloc(len);
	tmpBuf1 = (UINT8 *) malloc(len);
	tmpBuf2 = (UINT8 *) malloc(len);


	for (i=0; i<len; i++)
		*(Buf+i) = i;

	//1.	Set EncryptControl register
	ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_128 | CBC_mode | Encrypt_Stage);

	//2.	Set Initial vector IV
	ES_WriteReg(SEC_DESIVH, 0);
	ES_WriteReg(SEC_DESIVL, 1);
	ES_WriteReg(SEC_AESIV2, 2);
	ES_WriteReg(SEC_AESIV3, 3);

	//3.	Set Key value
	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, 2*i);

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, Buf);
	ES_WriteReg(SEC_DMADes, tmpBuf1);

	ES_WriteReg(SEC_DMATrasSize, len);

	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	//7.	Wait transfer size is complete
	if(ES_USE_INT)
	{
		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}

	ES_WriteReg(SEC_EncryptControl, First_block | Algorithm_AES_128 | CBC_mode | Decrypt_Stage);

	ES_WriteReg(SEC_DESIVH, 0);
	ES_WriteReg(SEC_DESIVL, 1);
	ES_WriteReg(SEC_AESIV2, 2);
	ES_WriteReg(SEC_AESIV3, 3);

	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, 2*i);

	ES_WriteReg(SEC_DMASrc, tmpBuf1);
	ES_WriteReg(SEC_DMADes, tmpBuf2);

	ES_WriteReg(SEC_DMATrasSize, len);

	ES_WriteReg(SEC_DMACtrl, DMA_Enable);

	if(ES_USE_INT)
	{
		UINT32 ttimes = 0;

		while(1)
		{
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				break;
			}
		}
	}

	if (memcmp(tmpBuf2, Buf, len)) {
/*
		printf("\ncompare failed!!\n\n");

printf("Buf     : ");
for (i=0; i<len; i++)
	printf("%02x ", *(Buf+i));
printf("\n");
printf("tmpBuf1 : ");
for (i=0; i<len; i++)
	printf("%02x ", *(tmpBuf1+i));
printf("\n");
printf("tmpBuf2 : ");
for (i=0; i<len; i++)
	printf("%02x ", *(tmpBuf2+i));
printf("\n");
*/
	}

#endif
	free(Buf);
	free(tmpBuf1);
	free(tmpBuf2);

	return TRUE;
}


BOOL ES_AES_test_func()
{
	int i, loop = 50000, per = 10000;
	//ES_USE_INT = FALSE;  //TRUE;//

	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

		//RichardLin Perforamce Relate
	 	ES_SetFIFO_Threshold(1, 1);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");


	for (i=0; i<loop; i++) {

	//ECB_mode
	if (!((i)%per)) printf("=== Test AES 128 ECB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(ECB_mode, Algorithm_AES_128);
	AES_DMA_D(ECB_mode, Algorithm_AES_128);

	if (!((i)%per)) printf("=== Test AES 192 ECB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(ECB_mode, Algorithm_AES_192);
	AES_DMA_D(ECB_mode, Algorithm_AES_192);

	if (!((i+1)%per)) printf("=== Test AES 256 ECB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(ECB_mode, Algorithm_AES_256);
	AES_DMA_D(ECB_mode, Algorithm_AES_256);

	//CBC_mode
	if (!((i+1)%per)) printf("=== Test AES 192 CBC_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CBC_mode, Algorithm_AES_128);
	AES_DMA_D(CBC_mode, Algorithm_AES_128);

	if (!((i+1)%per)) printf("=== Test AES 192 CBC_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CBC_mode, Algorithm_AES_192);
	AES_DMA_D(CBC_mode, Algorithm_AES_192);

	if (!((i+1)%per)) printf("=== Test AES 192 CBC_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CBC_mode, Algorithm_AES_256);
	AES_DMA_D(CBC_mode, Algorithm_AES_256);

	//OFB_mode
	if (!((i+1)%per)) printf("=== Test AES 192 OFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(OFB_mode, Algorithm_AES_128);
	AES_DMA_D(OFB_mode, Algorithm_AES_128);

	if (!((i+1)%per)) printf("=== Test AES 192 OFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(OFB_mode, Algorithm_AES_192);
	AES_DMA_D(OFB_mode, Algorithm_AES_192);

	if (!((i+1)%per)) printf("=== Test AES 192 OFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(OFB_mode, Algorithm_AES_256);
	AES_DMA_D(OFB_mode, Algorithm_AES_256);

	//CTR_mode
	if (!((i+1)%per)) printf("=== Test AES 192 CTR_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CTR_mode, Algorithm_AES_128);
	AES_DMA_D(CTR_mode, Algorithm_AES_128);

	if (!((i+1)%per)) printf("=== Test AES 192 CTR_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CTR_mode, Algorithm_AES_192);
	AES_DMA_D(CTR_mode, Algorithm_AES_192);

	if (!((i+1)%per)) printf("=== Test AES 192 CTR_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CTR_mode, Algorithm_AES_256);
	AES_DMA_D(CTR_mode, Algorithm_AES_256);

	//CFB_mode
	if (!((i+1)%per)) printf("=== Test AES 192 CFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CFB_mode, Algorithm_AES_128);
	AES_DMA_D(CFB_mode, Algorithm_AES_128);

	if (!((i+1)%per)) printf("=== Test AES 192 CFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CFB_mode, Algorithm_AES_192);
	AES_DMA_D(CFB_mode, Algorithm_AES_192);

	if (!((i+1)%per)) printf("=== Test AES 192 CFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CFB_mode, Algorithm_AES_256);
	AES_DMA_D(CFB_mode, Algorithm_AES_256);

	if (!((i+1)%per)) printf("Pass %d times\n", i+1);
	}

	return TRUE;
}


BOOL ES_DES_test_func()
{
	int i, loop = 50000, per = 10000;
	UINT32 *Buf, Size;

	//ES_USE_INT = FALSE;  //TRUE;//

	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

	 	ES_SetFIFO_Threshold(1, 1);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");

	Size = 24;
	Buf = malloc(Size);
	for (i=0; i<loop; i++) {

	if (!((i+1)%per)) printf("=== Test DES ECB_mode DMA encrypt / decrypt\n");
	DES_DMA_E(ECB_mode, Size, DES_IV[0], DES_Key[0], DES_PlainData[0], Buf);
	Compare_Data(Size / 4, Buf, DES_CipherData[0]);
	DES_DMA_D(ECB_mode, Size, DES_IV[0], DES_Key[0], DES_CipherData[0], Buf);
	Compare_Data(Size / 4, Buf, DES_PlainData[0]);

	if (!((i+1)%per)) printf("=== Test DES CBC_mode DMA encrypt / decrypt\n");
	DES_DMA_E(CBC_mode, Size, DES_IV[1], DES_Key[1], DES_PlainData[1], Buf);
	Compare_Data(Size / 4, Buf, DES_CipherData[1]);
	DES_DMA_D(CBC_mode, Size, DES_IV[1], DES_Key[1], DES_CipherData[1], Buf);
	Compare_Data(Size / 4, Buf, DES_PlainData[1]);

	if (!((i+1)%per)) printf("=== Test DES CTR_mode DMA encrypt / decrypt\n");
	DES_DMA_E(CTR_mode, Size, DES_IV[2], DES_Key[2], DES_PlainData[2], Buf);
	Compare_Data(Size / 4, Buf, DES_CipherData[2]);
	DES_DMA_D(CTR_mode, Size, DES_IV[2], DES_Key[2], DES_CipherData[2], Buf);
	Compare_Data(Size / 4, Buf, DES_PlainData[2]);

	if (!((i+1)%per)) printf("=== Test DES OFB_mode DMA encrypt / decrypt\n");
	DES_DMA_E(OFB_mode, Size, DES_IV[3], DES_Key[3], DES_PlainData[3], Buf);
	Compare_Data(Size / 4, Buf, DES_CipherData[3]);
	DES_DMA_D(OFB_mode, Size, DES_IV[3], DES_Key[3], DES_CipherData[3], Buf);
	Compare_Data(Size / 4, Buf, DES_PlainData[3]);

	if (!((i+1)%per)) printf("=== Test DES CFB_mode DMA encrypt / decrypt\n");
	DES_DMA_E(CFB_mode, 8, DES_IV[4], DES_Key[4], DES_PlainData[4], Buf);
	Compare_Data(8 / 4, Buf, DES_CipherData[4]);
	DES_DMA_D(CFB_mode, 8, DES_IV[4], DES_Key[4], DES_CipherData[4], Buf);
	Compare_Data(8 / 4, Buf, DES_PlainData[4]);

	//////////////////////////////////////////////////////////////////////////////

	if (!((i+1)%per)) printf("=== Test Triple_DES ECB_mode DMA encrypt / decrypt\n");
	Triple_DES_DMA_E(ECB_mode, Size, Triple_DES_IV[0], Triple_DES_Key[0], Triple_DES_PlainData[0], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_CipherData[0]);
	Triple_DES_DMA_D(ECB_mode, Size, Triple_DES_IV[0], Triple_DES_Key[0], Triple_DES_CipherData[0], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_PlainData[0]);

	if (!((i+1)%per)) printf("=== Test Triple_DES CBC_mode DMA encrypt / decrypt\n");
	Triple_DES_DMA_E(CBC_mode, Size, Triple_DES_IV[1], Triple_DES_Key[1], Triple_DES_PlainData[1], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_CipherData[1]);
	Triple_DES_DMA_D(CBC_mode, Size, Triple_DES_IV[1], Triple_DES_Key[1], Triple_DES_CipherData[1], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_PlainData[1]);

	if (!((i+1)%per)) printf("=== Test Triple_DES CTR_mode DMA encrypt / decrypt\n");
	Triple_DES_DMA_E(CTR_mode, Size, Triple_DES_IV[2], Triple_DES_Key[2], Triple_DES_PlainData[2], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_CipherData[2]);
	Triple_DES_DMA_D(CTR_mode, Size, Triple_DES_IV[2], Triple_DES_Key[2], Triple_DES_CipherData[2], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_PlainData[2]);

	if (!((i+1)%per)) printf("=== Test Triple_DES OFB_mode DMA encrypt / decrypt\n");
	Triple_DES_DMA_E(OFB_mode, Size, Triple_DES_IV[3], Triple_DES_Key[3], Triple_DES_PlainData[3], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_CipherData[3]);
	Triple_DES_DMA_D(OFB_mode, Size, Triple_DES_IV[3], Triple_DES_Key[3], Triple_DES_CipherData[3], Buf);
	Compare_Data(Size / 4, Buf, Triple_DES_PlainData[3]);

	if (!((i+1)%per)) printf("=== Test Triple_DES CFB_mode DMA encrypt / decrypt\n");
	Triple_DES_DMA_E(CFB_mode, 8, Triple_DES_IV[4], Triple_DES_Key[4], Triple_DES_PlainData[4], Buf);
	Compare_Data(8 / 4, Buf, Triple_DES_CipherData[4]);
	Triple_DES_DMA_D(CFB_mode, 8, Triple_DES_IV[4], Triple_DES_Key[4], Triple_DES_CipherData[4], Buf);
	Compare_Data(8 / 4, Buf, Triple_DES_PlainData[4]);

	if (!((i+1)%per)) printf("Pass %d times\n", i+1);
	}
	free(Buf);
	return TRUE;
}

BOOL ES_selftest_func()
{
UINT32 *Buf, Size, i, loop_times;
UINT32 *Tmp;

	loop_times = 50000;

	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

	 	ES_SetFIFO_Threshold(1, 1);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");


	Size = 32;
	Buf = malloc(Size);


	/////////////////////////DES/////////////////////////////////////////////////////
	Size = 24;
	Tmp = malloc(Size);

	printf("=== Test DES ECB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test DES CBC_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CBC_mode, Algorithm_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CBC_mode, Algorithm_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CBC_mode, Algorithm_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CBC_mode, Algorithm_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test DES OFB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test DES CTR_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CTR_mode, Algorithm_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CTR_mode, Algorithm_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CTR_mode, Algorithm_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CTR_mode, Algorithm_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test DES CFB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CFB_mode, Algorithm_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CFB_mode, Algorithm_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CFB_mode, Algorithm_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CFB_mode, Algorithm_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	free(Tmp);

	/////////////////////////Triple_DES/////////////////////////////////////////////////////
	Size = 24;
	Tmp = malloc(Size);

	printf("=== Test Triple_DES ECB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_Triple_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_Triple_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_Triple_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_Triple_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Triple_DES CBC_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CBC_mode, Algorithm_Triple_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CBC_mode, Algorithm_Triple_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CBC_mode, Algorithm_Triple_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CBC_mode, Algorithm_Triple_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Triple_DES OFB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_Triple_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_Triple_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_Triple_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_Triple_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Triple_DES CTR_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CTR_mode, Algorithm_Triple_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CTR_mode, Algorithm_Triple_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CTR_mode, Algorithm_Triple_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CTR_mode, Algorithm_Triple_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Triple_DES CFB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CFB_mode, Algorithm_Triple_DES, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CFB_mode, Algorithm_Triple_DES, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CFB_mode, Algorithm_Triple_DES, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CFB_mode, Algorithm_Triple_DES, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	free(Tmp);

	/////////////////////////Algorithm_AES_256/////////////////////////////////////////////////////
	Size = 32;
	Tmp = malloc(Size);

	printf("=== Test Algorithm_AES_256 ECB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_AES_128, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_AES_128, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_AES_128, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_AES_128, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Algorithm_AES_256 CBC_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CBC_mode, Algorithm_AES_256, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CBC_mode, Algorithm_AES_256, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CBC_mode, Algorithm_AES_256, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CBC_mode, Algorithm_AES_256, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Algorithm_AES_256 OFB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_AES_256, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_AES_256, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_AES_256, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_AES_256, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Algorithm_AES_256 CTR_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CTR_mode, Algorithm_AES_256, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CTR_mode, Algorithm_AES_256, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CTR_mode, Algorithm_AES_256, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CTR_mode, Algorithm_AES_256, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	printf("=== Test Algorithm_AES_256 CFB_mode DMA encrypt / decrypt\n");
	ES_Large_DMA(Decrypt_Stage, CFB_mode, Algorithm_AES_256, DES_PlainData[1], Buf, Size);
	ES_Large_DMA(Encrypt_Stage, CFB_mode, Algorithm_AES_256, Buf, Tmp, Size);

	for(i = 0; i < loop_times; i++)
	{
		ES_Large_DMA(Decrypt_Stage, CFB_mode, Algorithm_AES_256, Tmp, Buf, Size);
		ES_Large_DMA(Encrypt_Stage, CFB_mode, Algorithm_AES_256, Buf, Tmp, Size);
	}
	Compare_Data(Size / 4, Tmp, DES_PlainData[1]);

	free(Tmp);


	free(Buf);

	return TRUE;
}

BOOL ES_keytest_func()
{
	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

	 	ES_SetFIFO_Threshold(1, 1);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");

	printf("=== Algorithm_AES_128/256 ECB mode DMA encrypt / decrypt\n");
	Key_DMA_Test(ECB_mode);

	printf("=== Algorithm_AES_128/256 CBC mode DMA encrypt / decrypt\n");
	Key_DMA_Test(CBC_mode);

	return TRUE;
}
#ifndef  CONFIG_CPU_CA53
int fsizeof(const char *fname)
{
 	FILE *f;
 	int L;

 	f=fopen(fname, "rb");

 	if(f==NULL)
  		return -1;

 	fseek(f, 0, SEEK_END);
 	L = ftell(f);
 	fclose(f);

 	return L;
}
#endif
BOOL ES_large_encrypt_func()
{
UINT32 	*InBuf, *OutBuf, *ResultBuf;
FILE 	*In_stream, *Out_stream;
int 	In_File_Size, Out_File_Size;
#ifndef  CONFIG_CPU_CA53
	//ES_USE_INT = FALSE;  //TRUE;//

	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

	 	ES_SetFIFO_Threshold(128, 128);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");

	// Read P.txt file
	In_File_Size = fsizeof("..\\..\\..\\..\\pattern\\P.txt");
	if (In_File_Size < 0)
	{
		printf("P.txt File could not be opened\n");
		return FALSE;
	}
	else
	{
		printf("P.txt File_Size = %d\n",In_File_Size);

		InBuf = malloc(In_File_Size);

		In_stream = fopen( "..\\..\\..\\..\\pattern\\P.txt", "rb" );
		fread(InBuf, In_File_Size, 1, In_stream);
		fclose(In_stream);
	}

	// Read AES_ECB.fcp file
	Out_File_Size = fsizeof("..\\..\\..\\..\\pattern\\AES_ECB.fcp");
	if (Out_File_Size < 0)
	{
		printf("AES_ECB.fcp File could not be opened\n");

		free(InBuf);
		return FALSE;
	}
	else
	{
		printf("AES_ECB.fcp File_Size = %d\n",Out_File_Size);

		OutBuf = malloc(Out_File_Size);

		Out_stream = fopen( "..\\..\\..\\..\\pattern\\AES_ECB.fcp", "rb" );
		fread(OutBuf, Out_File_Size, 1, Out_stream);
		fclose(Out_stream);
	}

	ResultBuf = malloc(In_File_Size);


	printf("InBuf addr = 0x%0.8x,OutBuf addr = 0x%0.8x,ResultBuf addr = 0x%0.8x\n",(UINT32) InBuf,(UINT32) OutBuf,(UINT32) ResultBuf);
	printf("=== AES_128 ECB mode Encrypt function\n");
	ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_AES_128, InBuf, ResultBuf, In_File_Size);
	Compare_Data(In_File_Size / 4, ResultBuf, OutBuf);

	printf("=== AES_128 ECB mode Decrypt function\n");
	ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_AES_128, OutBuf, ResultBuf, In_File_Size);
	Compare_Data(In_File_Size / 4, ResultBuf, InBuf);

//////////////////////////////////////////////////////////////////////////////////////

	// Read 3DES_OFB.fcp file
	Out_File_Size = fsizeof("..\\..\\..\\..\\pattern\\3DES_OFB.fcp");
	if (Out_File_Size < 0)
	{
		printf("3DES_OFB.fcp File could not be opened\n");

		free(InBuf);
		free(OutBuf);
		free(ResultBuf);

		return FALSE;
	}
	else
	{
		printf("3DES_OFB.fcp File_Size = %d\n",Out_File_Size);

		Out_stream = fopen( "..\\..\\..\\..\\pattern\\3DES_OFB.fcp", "rb" );
		fread(OutBuf, Out_File_Size, 1, Out_stream);
		fclose(Out_stream);
	}

	printf("===  Triple_DES OFB mode Encrypt function\n");
	ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_Triple_DES, InBuf, ResultBuf, In_File_Size);
	Compare_Data(In_File_Size / 4, ResultBuf, OutBuf);

	printf("=== Triple_DES OFB mode Decrypt function\n");
	ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_Triple_DES, OutBuf, ResultBuf, In_File_Size);
	Compare_Data(In_File_Size / 4, ResultBuf, InBuf);

	free(InBuf);
	free(OutBuf);
	free(ResultBuf);
#endif
	return TRUE;
}


BOOL ES_Burn_in_func()
{
UINT32 	*InBuf, *OutBuf, *ResultBuf, *ResultBuf_1;
UINT32	i, loop = 50000, per = 1000;
FILE 	*In_stream, *Out_stream;
int 	In_File_Size, Out_File_Size;
#ifndef  CONFIG_CPU_CA53
	//ES_USE_INT = FALSE;  //TRUE;//

	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

	 	ES_SetFIFO_Threshold(128, 128);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");

	// Read P.txt file
	In_File_Size = fsizeof("..\\..\\..\\..\\pattern\\P.txt");
	if (In_File_Size < 0)
	{
		printf("P.txt File could not be opened\n");
		return FALSE;
	}
	else
	{
		printf("P.txt File_Size = %d\n",In_File_Size);

		InBuf = malloc(In_File_Size);

		In_stream = fopen( "..\\..\\..\\..\\pattern\\P.txt", "rb" );
		fread(InBuf, In_File_Size, 1, In_stream);
		fclose(In_stream);
	}

	// Read AES_ECB.fcp file
	Out_File_Size = fsizeof("..\\..\\..\\..\\pattern\\AES_ECB.fcp");
	if (Out_File_Size < 0)
	{
		printf("AES_ECB.fcp File could not be opened\n");

		free(InBuf);
		return FALSE;
	}
	else
	{
		printf("AES_ECB.fcp File_Size = %d\n",Out_File_Size);

		OutBuf = malloc(Out_File_Size);

		Out_stream = fopen( "..\\..\\..\\..\\pattern\\AES_ECB.fcp", "rb" );
		fread(OutBuf, Out_File_Size, 1, Out_stream);
		fclose(Out_stream);
	}

	ResultBuf = malloc(In_File_Size);
	ResultBuf_1 = malloc(In_File_Size);


	printf("InBuf addr = 0x%0.8x,OutBuf addr = 0x%0.8x,ResultBuf addr = 0x%0.8x\n",(UINT32)InBuf, (UINT32)OutBuf,(UINT32) ResultBuf);
	printf("===  AES 128 ECB mode Encrypt/Decrypt Burn-In %d times test\n", loop);

	ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_AES_128, InBuf, ResultBuf, In_File_Size);

	for(i = 0; i < loop; i++)
	{
#if 1
		ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_AES_128, ResultBuf, ResultBuf_1, In_File_Size);
		ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_AES_128, ResultBuf_1, ResultBuf, In_File_Size);

		Compare_Data(In_File_Size / 4, ResultBuf, OutBuf);
		if (!((i+1)%per)) printf("Pass %d times\n", i+1);
#else
		for(j = 0; j < k; j++)
		{
			ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_AES_128, ResultBuf, ResultBuf_1, In_File_Size);
			ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_AES_128, ResultBuf_1, ResultBuf, In_File_Size);
		}

		printf("times %d\n", k);
		Compare_Data(In_File_Size / 4, ResultBuf, OutBuf);

		k += 1;
#endif
	}

	//Compare_Data(In_File_Size / 4, ResultBuf, OutBuf);

//////////////////////////////////////////////////////////////////////////////////////

	// Read 3DES_OFB.fcp file
	Out_File_Size = fsizeof("..\\..\\..\\..\\pattern\\3DES_OFB.fcp");
	if (Out_File_Size < 0)
	{
		printf("3DES_OFB.fcp File could not be opened\n");

		free(InBuf);
		free(OutBuf);
		free(ResultBuf);
		free(ResultBuf_1);

		return FALSE;
	}
	else
	{
		printf("3DES_OFB.fcp File_Size = %d\n",Out_File_Size);

		Out_stream = fopen( "..\\..\\..\\..\\pattern\\3DES_OFB.fcp", "rb" );
		fread(OutBuf, Out_File_Size, 1, Out_stream);
		fclose(Out_stream);
	}

	printf("===  Triple_DES OFB mode Encrypt/Decrypt Burn-In %d times test\n", loop);

	ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_Triple_DES, InBuf, ResultBuf, In_File_Size);

	for(i = 0; i < loop; i++)
	{
		ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_Triple_DES, ResultBuf, ResultBuf_1, In_File_Size);
		ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_Triple_DES, ResultBuf_1, ResultBuf, In_File_Size);

		Compare_Data(In_File_Size / 4, ResultBuf, OutBuf);
		if (!((i+1)%per)) printf("Pass %d times\n", i+1);
	}

	//Compare_Data(In_File_Size / 4, ResultBuf, OutBuf);

	free(InBuf);
	free(OutBuf);
	free(ResultBuf);
	free(ResultBuf_1);
#endif
	return TRUE;
}


BOOL ES_pattern_test_func()
{
UINT32 	*InBuf, *OutBuf, *ResultBuf;
FILE 	*In_stream, *Out_stream;
int 	In_File_Size, Out_File_Size;
UINT32	i, j, Flag;
char 	InName[3][40] = {"..\\..\\..\\..\\pattern\\1","..\\..\\..\\..\\pattern\\2","..\\..\\..\\..\\pattern\\3"};
char 	OutName[3][40] = {"..\\..\\..\\..\\pattern\\1.fcp","..\\..\\..\\..\\pattern\\2.fcp","..\\..\\..\\..\\pattern\\3.fcp"};
#ifndef  CONFIG_CPU_CA53
	//ES_USE_INT = FALSE;  //TRUE;

	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

		//RichardLin Perforamce Relate
	 	ES_SetFIFO_Threshold(1, 1);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");


	In_stream = malloc(8);
	Out_stream = malloc(8);

	In_File_Size = 64;
	InBuf = malloc(In_File_Size);
	printf("InBuf addr = 0x%08x\n",(UINT32)InBuf);

	Out_File_Size = 64;
	OutBuf = malloc(Out_File_Size);
	printf("OutBuf addr = 0x%08x\n",(UINT32)OutBuf);

	ResultBuf = malloc(In_File_Size);
	printf("ResultBuf addr = 0x%08x\n", (UINT32)ResultBuf);

	for(j = 0; j < 3; j++)
	{
		In_stream = fopen( InName[j], "rb" );
		if (In_stream == NULL)
		{
			printf("In buf File could not be opened\n");

			free(InBuf);
			free(OutBuf);
			free(ResultBuf);
			return FALSE;
		}
		else
		{
			fread(InBuf, In_File_Size, 1, In_stream);
			fclose(In_stream);
		}

		Out_stream = fopen( OutName[j], "rb" );
		if (Out_stream == NULL)
		{
			printf("Out buf File could not be opened\n");

			free(InBuf);
			free(OutBuf);
			free(ResultBuf);
			return FALSE;
		}
		else
		{
			fread(OutBuf, Out_File_Size, 1, Out_stream);
			fclose(Out_stream);
		}

////////////////////////////////////////////////////////////////////

		printf("Encrypt Test pattern %d\n", j+1);
		if(j == 0)
			ES_Large_DMA(Encrypt_Stage, ECB_mode, Algorithm_AES_128, InBuf, ResultBuf, In_File_Size);
		if(j == 1)
			ES_Large_DMA(Encrypt_Stage, CBC_mode, Algorithm_AES_128, InBuf, ResultBuf, In_File_Size);
		if(j == 2)
			ES_Large_DMA(Encrypt_Stage, OFB_mode, Algorithm_AES_192, InBuf, ResultBuf, In_File_Size);

		Flag = TRUE;
		for(i = 0; i < In_File_Size/4; i++)
		{
			if(*(ResultBuf + i) != *(OutBuf + i))
			{
				printf("num % d=>ResultBuf = %0.8x, OutBuf = %0.8x\n",i, *(ResultBuf + i), *(OutBuf + i));
				Flag = FALSE;
				i = In_File_Size;
			}
		}

		if(Flag == FALSE)
			printf("Compare Data Fail\n");
		else
			printf("Compare Data Pass\n");

////////////////////////////////////////////////////////////////////

		printf("Decrypt Test pattern %d\n", j+1);
		if(j == 0)
			ES_Large_DMA(Decrypt_Stage, ECB_mode, Algorithm_AES_128, OutBuf, ResultBuf, In_File_Size);
		if(j == 1)
			ES_Large_DMA(Decrypt_Stage, CBC_mode, Algorithm_AES_128, OutBuf, ResultBuf, In_File_Size);
		if(j == 2)
			ES_Large_DMA(Decrypt_Stage, OFB_mode, Algorithm_AES_192, OutBuf, ResultBuf, In_File_Size);

		Flag = TRUE;
		for(i = 0; i < In_File_Size/4; i++)
		{
			if(*(ResultBuf + i) != *(InBuf + i))
			{
				printf("num % d=>ResultBuf = %0.8x, OutBuf = %0.8x\n",i, *(ResultBuf + i), *(InBuf + i));
				Flag = FALSE;
				i = In_File_Size;
			}
		}

		if(Flag == FALSE)
			printf("Compare Data Fail\n");
		else
			printf("Compare Data Pass\n");
	}

	free(InBuf);
	free(OutBuf);
	free(ResultBuf);
#endif
	return TRUE;
}

BOOL ES_DMA_test_func()
{
	UINT32 i, *Buf, Result, loop = 0x10;

	int return_val= TRUE;

	
	/////////////////////////// interrupt setting /////////////////////////////

	if(ES_USE_INT)
	{
		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;
		DMA_STOP_OK= 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ( aes_irq, fLib_ESIntHandler))
			return FALSE;

        fLib_SetIRQmode(aes_irq, LEVEL);

	 	ES_SetFIFO_Threshold(1, 1);

		ES_WriteReg(SEC_IntrEnable, Data_done | DMA_stop_int);//Verify stop function
		//ES_WriteReg(SEC_IntrEnable, Data_done);//Verify stop mask function function
		//ES_WriteReg(SEC_IntrEnable, DMA_Stop);//Verify done mask function function

	    fLib_EnableIRQ( aes_irq);
    }
    else
    	printf("Test DMA Polling\n");


	//ECB_mode
	printf("Test AES ECB_mode DMA\n");

	Buf = malloc(16 * 4);

	//1.	Set EncryptControl register
	ES_WriteReg(SEC_EncryptControl, Algorithm_AES_128 | ECB_mode | Encrypt_Stage);

	//3.	Set Key value
	for(i = 0; i < 8; i++)
		ES_WriteReg(SEC_DESKey1H + 4 * i, Key[0][i]);

	//5.	Set DMA related register
	ES_WriteReg(SEC_DMASrc, Plain[0]);
	ES_WriteReg(SEC_DMADes, Buf);

	//RIchardLin XXX
	//ES_WriteReg(SEC_DMATrasSize, 0xFFF);
	ES_WriteReg(SEC_DMATrasSize, 0xFFF);

	//6.	Set DmaEn bit of DMAStatus to 1 to active DMA engine
	ES_WriteReg(SEC_DMACtrl, DMA_Enable);//=======================verify IntrStatus and MaskedIntrStatus
	printf("DMA Trasfer Size=%0x\n", ES_ReadReg(SEC_DMATrasSize));
	ES_WriteReg(SEC_DMACtrl, DMA_Stop);
	
	//7.	Wait transfer size is complete
	//RichardLin 081210
	if(ES_USE_INT)
	{
		while(1)
		{
//			printf("DMA Trasfer Size=%0x\n", ES_ReadReg(SEC_DMATrasSize));
			
			if(DMA_STOP_OK == 1)
			{
				DMA_STOP_OK = 0;
				printf("end by stoping this job\n");
				break;
			}
			if(DMA_INT_OK == 1)
			{
				DMA_INT_OK = 0;
				return_val= FALSE;
				printf("end by finidhing this job\n");
				break;
			}
		}
	}
	else
	{
			printf("not testing stop when using polling, just let this job finish \n");
			return_val= FALSE;
	}

	printf("SEC_IntrStatus = %x\n",ES_ReadReg(SEC_IntrStatus));
	printf("SEC_MaskedIntrStatus = %x\n",ES_ReadReg(SEC_MaskedIntrStatus));
	if(ES_ReadReg(SEC_DMATrasSize) != 0){
		printf("DMA_Stop Trasfer Size error!\n");
		return_val= FALSE;
	}
	else{
		printf("DMA_Stop Trasfer Size pass!\n");
	}
	Result = ES_ReadReg(SEC_FIFOStatus);
	printf("FIFO Status = %d\n",Result);

	free(Buf);

	if(return_val== TRUE){
		printf("DMA Stop Trasfer --> Pass\n");
	}
	else{
		printf("DMA Stop Trasfer --> FALSE\n");
	}
	
	return return_val;
}


void ES_USE_INT_Enable(){
	ES_USE_INT= 1; 
}

void ES_USE_INT_Disable(){
	ES_USE_INT= 0; 
	// Legend Hsiao
	fLib_CloseIRQ(aes_irq);
}


#define A320            1
#define A321            2

struct burnin_cmd_aes
{
    int     index;
	char    *string;		/* command name */	
	void    (*burnin_routine)();		/* implementing routine */
	char    exist;          /* IP exist */
};

struct burnin_cmd_aes security_cmd_value[] = {
    { 1,"Simple AES Security Test\n",ES_AES_test_func, A320},
    { 2,"Simple DES & Triple-DES Security Test\n",ES_DES_test_func, A320},
    { 3,"Security DMA function Test\n",ES_DMA_test_func, A320},
    { 4,"Security Self Endrypt/Decrypt Test\n",ES_selftest_func, A320},
    { 5,"Security Not Change Key Special Test\n",ES_keytest_func, A320},
    { 6,"AES/FPGA AHBC test\n",MPCAAHBC_AES_test_func, A320}, /*chris added for FPGA test*/
#ifdef CONFIG_SEMIHOSTING_TEST
    { 7,"(semihosting)-Security Large-Data AES Encrypt / Decrypt Test\n",ES_large_encrypt_func, A320},
    { 8,"(semihosting)-Security Pattern Encrypt Test\n",ES_pattern_test_func, A320},
    { 9,"(semihosting)-Security Burn-in Test\n",ES_Burn_in_func, A320},
#endif
    { 10,"Using Interrupt Mode\n", ES_USE_INT_Enable, A320},
    { 11,"Not using Interrupt Mode (Polling Mode)\n",ES_USE_INT_Disable, A320},
    { 99,"Return Main Menu\n",0, 1},
    { 0,"",0,0}
};


void PrintMsg()
{
    struct burnin_cmd_aes *burnin_temp=security_cmd_value;
    printf("\n\r  ----------------------------------------------------------------------------\n");
    printf("\r   Faraday Technology Corporation CPE Burnin Program");
    printf("\n");
    printf("  ----------------------------------------------------------------------------\n");

    while(burnin_temp->index)
    {
        if(burnin_temp->exist)
        {
            printf("(%2d)",burnin_temp->index);
            printf(" %s",burnin_temp->string);
        }
        burnin_temp++;
    }
}

void ES_WordEndianChange(UINT32 *dt, INT32 len)
{
	UINT32	ul;

	for(;len > 0;len--, dt++)
	{
		ul = *dt;
		((UINT8 *)dt)[0] = ((UINT8 *)&ul)[3];
		((UINT8 *)dt)[1] = ((UINT8 *)&ul)[2];
		((UINT8 *)dt)[2] = ((UINT8 *)&ul)[1];
		((UINT8 *)dt)[3] = ((UINT8 *)&ul)[0];
	}
}

int ES_ParseCmd(char* Buffer)
{
   char seps[]  = " ";
   char *token;
   char Cmd[256];
   UINT32 i;

   /* Establish string and get the first token: */
   token = strtok( Buffer, seps );
   if( token == NULL )
        return 0;

   strcpy(Cmd, token);

   i = atoi( Cmd );

   return i;
}

void FTAES020_Test_Main()
{
	UINT32 id;
    char Buffer[256];
//	static init=0;
    	UINT32 init=0;
printf("%s init= %d\n", __func__, init);
	if(!init){
		fLib_Int_Init();

		aes_base =  IP_PA_BASE(0);
		aes_irq = IP_IRQ(0);
              printf("%s aes_base = 0x%x, aes_irq = 0x%x\n", __func__, aes_base, aes_irq);
		ES_WordEndianChange(Plain, 2*16);
		ES_WordEndianChange(Cipher, 15*16);

		ES_WordEndianChange(DES_PlainData, 5*6);
		ES_WordEndianChange(DES_CipherData, 5*6);

		ES_WordEndianChange(Triple_DES_PlainData, 5*6);
		ES_WordEndianChange(Triple_DES_CipherData, 5*6);
	}

 	//PrintMsg();

    while(1)
    {
        struct burnin_cmd_aes *burnin_temp=security_cmd_value;
        PrintMsg();
	    printf("\nCommand>>");
        gets(Buffer);
   		printf("\r");

   		id = ES_ParseCmd(Buffer);

        if(id==0)
            PrintMsg();
        else
        {
			if (id==99)
				return ;
            while(id>=burnin_temp->index)
            {
                if((id==burnin_temp->index) && (burnin_temp->exist))
                {
                    (*burnin_temp->burnin_routine)();
                    break;
                }
                burnin_temp++;
            }
        }
    }
}
