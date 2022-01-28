/***************************************************************************
* Copyright  Faraday Technology Corp 2004-2005.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:FTAES020_QC.c.c                                                     *
* Description: QC testing program                           		       *
* Author: ycmo                                                   		   *
* Date: 2004/03/9                                                          *
* Version:0.10                                                             *
*--------------------------------------------------------------------------*
****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SoFlexible.h"

#include "FTAES020/es.h"

#define IPMODULE AES
#define IPNAME FTAES020

extern UINT32 volatile aes_base, aes_irq;
extern UINT32 Plain[2][16] ;
extern UINT8 volatile DMA_INT_OK;
extern UINT8 volatile DMA_STOP_OK;

extern UINT32 Cipher[15][16] ;
extern UINT32 DES_PlainData[5][6] ;
extern UINT32 DES_Key[5][2] ;
extern UINT32 DES_IV[5][2] ;
extern UINT32 Triple_DES_PlainData[5][6] ;
extern UINT32 Triple_DES_CipherData[5][6] ;
extern UINT32 DES_CipherData[5][6] ;
extern BOOL ES_USE_INT;

extern void fLib_ESIntHandler();

void FTAES020_QC_Main()
{
	UINT32 id;
    char Buffer[256];
	int i, loop = 50000, per = 10000;
	static init=0;

	if(!init){
		fLib_Int_Init();

		aes_base =  IP_PA_BASE(0);
		aes_irq = IP_IRQ(0);
		fLib_printf("%s aes_base = 0x%x, aes_irq = 0x%x\n", __func__, aes_base, aes_irq);
		ES_WordEndianChange(Plain, 2*16);
		ES_WordEndianChange(Cipher, 15*16);

		ES_WordEndianChange(DES_PlainData, 5*6);
		ES_WordEndianChange(DES_CipherData, 5*6);

		ES_WordEndianChange(Triple_DES_PlainData, 5*6);
		ES_WordEndianChange(Triple_DES_CipherData, 5*6);
		init = 1;
	}

	ES_USE_INT = TRUE;  //TRUE;//

	/////////////////////////// interrupt setting /////////////////////////////

		printf("Test DMA Interrupt\n");

		DMA_INT_OK = 0;

		fLib_CloseIRQ(aes_irq);
	   	if (!fLib_ConnectIRQ(aes_irq, fLib_ESIntHandler))
			return ;

        fLib_SetIRQmode(aes_irq, LEVEL);

		//RichardLin Perforamce Relate
	 	ES_SetFIFO_Threshold(1, 1);

	    ES_WriteReg(SEC_IntrEnable, Data_done);//Enable interrupt

	    fLib_EnableIRQ(aes_irq);



	//ECB_mode
	printf("=== Test AES 128 ECB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(ECB_mode, Algorithm_AES_128);
	AES_DMA_D(ECB_mode, Algorithm_AES_128);

	printf("=== Test AES 192 ECB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(ECB_mode, Algorithm_AES_192);
	AES_DMA_D(ECB_mode, Algorithm_AES_192);

	printf("=== Test AES 256 ECB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(ECB_mode, Algorithm_AES_256);
	AES_DMA_D(ECB_mode, Algorithm_AES_256);

	//CBC_mode
	printf("=== Test AES 192 CBC_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CBC_mode, Algorithm_AES_128);
	AES_DMA_D(CBC_mode, Algorithm_AES_128);

	printf("=== Test AES 192 CBC_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CBC_mode, Algorithm_AES_192);
	AES_DMA_D(CBC_mode, Algorithm_AES_192);

	if (!((i+1)%per)) printf("=== Test AES 192 CBC_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CBC_mode, Algorithm_AES_256);
	AES_DMA_D(CBC_mode, Algorithm_AES_256);

	//OFB_mode
	printf("=== Test AES 192 OFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(OFB_mode, Algorithm_AES_128);
	AES_DMA_D(OFB_mode, Algorithm_AES_128);

	printf("=== Test AES 192 OFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(OFB_mode, Algorithm_AES_192);
	AES_DMA_D(OFB_mode, Algorithm_AES_192);

	printf("=== Test AES 192 OFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(OFB_mode, Algorithm_AES_256);
	AES_DMA_D(OFB_mode, Algorithm_AES_256);

	//CTR_mode
	printf("=== Test AES 192 CTR_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CTR_mode, Algorithm_AES_128);
	AES_DMA_D(CTR_mode, Algorithm_AES_128);

	printf("=== Test AES 192 CTR_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CTR_mode, Algorithm_AES_192);
	AES_DMA_D(CTR_mode, Algorithm_AES_192);

	printf("=== Test AES 192 CTR_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CTR_mode, Algorithm_AES_256);
	AES_DMA_D(CTR_mode, Algorithm_AES_256);

	//CFB_mode
	printf("=== Test AES 192 CFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CFB_mode, Algorithm_AES_128);
	AES_DMA_D(CFB_mode, Algorithm_AES_128);

	printf("=== Test AES 192 CFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CFB_mode, Algorithm_AES_192);
	AES_DMA_D(CFB_mode, Algorithm_AES_192);

	printf("=== Test AES 192 CFB_mode DMA encrypt / decrypt\n");
	AES_DMA_E(CFB_mode, Algorithm_AES_256);
	AES_DMA_D(CFB_mode, Algorithm_AES_256);

	printf("Pass !!\n", i+1);
	printf("End the FTAES020_QC test \n"); 
	return ;
}
