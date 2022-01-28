#ifndef _CPRM_H
#define _CPRM_H					1

//#define CPE_ES_BASE		0x90f00000//0x90A00000
//#define IRQ_ES			23//21

/* register */

#define SEC_EncryptControl		0x00000000
#define SEC_SlaveRunEnable		0x00000004
#define SEC_FIFOStatus			0x00000008
#define SEC_PErrStatus			0x0000000C
#define SEC_DESKey1H			0x00000010


#define SEC_DESKey1L			0x00000014
#define SEC_DESKey2H			0x00000018
#define SEC_DESKey2L			0x0000001C
#define SEC_DESKey3H			0x00000020
#define SEC_DESKey3L			0x00000024
#define SEC_AESKey6				0x00000028
#define SEC_AESKey7				0x0000002C
#define SEC_DESIVH				0x00000030

#define SEC_DESIVL				0x00000034
#define SEC_AESIV2				0x00000038
#define SEC_AESIV3				0x0000003C
#define SEC_INFIFOPort			0x00000040
#define SEC_OutFIFOPort			0x00000044
#define SEC_DMASrc				0x00000048
#define SEC_DMADes				0x0000004C

#define SEC_DMATrasSize			0x00000050
#define SEC_DMACtrl				0x00000054
#define SEC_FIFOThreshold		0x00000058
#define SEC_IntrEnable			0x0000005C
#define SEC_IntrStatus			0x00000060
#define SEC_MaskedIntrStatus	0x00000064
#define SEC_ClearIntrStatus		0x00000068

#define SEC_LAST_IV0			0x00000080
#define SEC_LAST_IV1			0x00000084
#define SEC_LAST_IV2			0x00000088
#define SEC_LAST_IV3			0x0000008c

/* bit mapping of command register */
//SEC_EncryptControl
#define Parity_check			0x100
#define First_block				0x80
#define ECB_mode				0x00
#define CBC_mode				0x10
#define CTR_mode				0x20
#define CFB_mode				0x40
#define OFB_mode				0x50
#define Algorithm_DES			0x0
#define Algorithm_Triple_DES	0x2
#define Algorithm_AES_128		0x8
#define Algorithm_AES_192		0xA
#define Algorithm_AES_256		0xC
#define Decrypt_Stage			0x1
#define Encrypt_Stage			0x0

#define Special_mode			0x8000


//SEC_FIFOStatus
#define Out_FIFO_full			0x8
#define Out_FIFO_empty			0x4
#define In_FIFO_full			0x2
#define In_FIFO_empty			0x1

//SEC_DMACtrl

//RichardLin
//#define DMA_Stop				0x100
#define DMA_Stop				0x04

#define DMA_receive_error		0x2
#define DMA_Enable				0x1

//SEC_IntrStatus
#define DMA_stop_int			0x4
#define Data_receive_error		0x2
#define Data_done				0x1


/********************************************************************/
/* Global variables */
/********************************************************************/

typedef struct
{
	UINT32 encryp_mode;

	UINT32 DESKey1H;
	UINT32 DESKey1L;

	UINT32 DESKey2H;
	UINT32 DESKey2L;

	UINT32 DESKey3H;
	UINT32 DESKey3L;

	UINT32 AESKey6;
	UINT32 AESKey7;

	UINT32 DESIVH;
	UINT32 DESIVL;
	UINT32 AESIV2;
	UINT32 AESIV3;

}EncryptStruct;

#define DMAenable 	0x00000000
#define ES_READY 	0x00000000

BOOL AES_DMA_E(UINT32 mode, UINT32 Algorithm);
BOOL AES_DMA_D(UINT32 mode, UINT32 Algorithm);

BOOL DES_DMA_E(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *CipherDataPtr);
BOOL DES_DMA_D(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *CipherDataPtr);

BOOL Triple_DES_DMA_E(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *CipherDataPtr);
BOOL Triple_DES_DMA_D(UINT32 mode, UINT32 Size, UINT32 *IVPtr, UINT32 *KeyPtr, UINT32 *PlainDataPtr, UINT32 *CipherDataPtr);

BOOL ES_Large_DMA(UINT32 Stage, UINT32 mode, UINT32 Algorithm, UINT32 *InBuf, UINT32 *OutBuf, int Size);

BOOL Compare_Data(UINT32 Word_Size, UINT32 *Buf_1, UINT32 *Buf_2);

void ES_WriteReg(UINT32 Offset, UINT32 RegValue);
UINT32 ES_ReadReg(UINT32 Offset);

void ES_SetFIFO_Threshold(UINT32 InFIFO_Threshold, UINT32 OutFIFO_Threshold);
BOOL Key_DMA_Test(UINT32 mode);

void WordEndianChange(UINT32 *dt, INT32 len); //louis@20070612

#endif
