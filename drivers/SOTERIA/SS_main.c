/*------------------------------------------------------------------------------
flash.c

copyright 2010-2015 Faraday technology corporation.
------------------------------------------------------------------------------*/
/* include files */
#include "SoFlexible.h"
#include "utility.h"
#include "SS_msg.h"

#define MAX_CMD_LEN	50

static int	ssMainPrintDebugMessage = 0;

SS_MSG_CTRL	*ss = (SS_MSG_CTRL *)CRYPTO_SOTERIA_PA_BASE;

UINT32	testBuf[4][1024/4];

/*====== AES test key data ======*/
UINT32	_aes_test_key[8] = {
	0xFFFF0000, 0xFFFF1111, 0xFFFF2222, 0xFFFF3333, 0xFFFF4444, 0xFFFF5555, 0xFFFF6666, 0xFFFF7777
	};
UINT32	_aes_test_key_2[8] = {
	0xFFFF0001, 0xFFFF1111, 0xFFFF2222, 0xFFFF3333, 0xFFFF4444, 0xFFFF5555, 0xFFFF6666, 0xFFFF7777
	};
UINT32	_aes_test_iv[4] = {
	0x11112222, 0x33334444, 0x55556666, 0x77778888
	};

/*====== RSA test key data ======*/
/***** 256 bit *****/
#if (RSA_MAX_LEN==1024)
UINT32	_rsa_test_modulus_256[8] = {
	0xd7a8ea5d, 0xc559e986, 0x61b2d2cd, 0x9eff88d1, 0x9ced2760, 0x11de6a1f, 0x6fee780d, 0x5e628939
	};
UINT32	_rsa_test_public_256[8] = {
	0x00010001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	};
UINT32	_rsa_test_private_256[8] = {
	0x4a55a381, 0x947de411, 0xa13e69f6, 0xd868b4d2, 0x63cca1c2, 0xc62aa9f8, 0x39938cb6, 0x01e2fa57
	};
UINT32	_rsa_test_plain_256[8] = {
	0xbc5dd8bf, 0x14fd89c2, 0x9b027749, 0xf697c2f0, 0xc2f28e0e, 0x9bc2f39b, 0x009d0af3, 0x00000000
	};
UINT32	_rsa_test_cipher_256[8] = {
	0xce61de0f, 0xd13f8db3, 0xe2316789, 0x802a6b49, 0x8e5644cb, 0x77fd6a3f, 0x5f413aab, 0x15b41a75
	};
#endif
/***** 512 bit *****/
UINT32	_rsa_test_modulus_512[16] = {
	0xcd7729f9, 0xa0ef20fb, 0xe7346a72, 0xf56d76ad, 0x1c632421, 0x41502d22, 0x9981a652, 0xf7ad45ed,
	0x8102984c, 0x0837e10e, 0xfd1152a0, 0x6b5e7b0d, 0xe39cc982, 0x0cae80a9, 0xd8bc2d91, 0x9cbd4fe5
	};
UINT32	_rsa_test_public_512[16] = {
	0x00010001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	};
UINT32	_rsa_test_private_512[16] = {
	0x05d33141, 0x0629fef0, 0x5f033dbc, 0x8883017c, 0x8037fc75, 0x53c1a610, 0x808ab540, 0x0d81eb17,
	0x6a62b977, 0xb7a43cce, 0x0cbafda0, 0xec6f5600, 0xb7106a46, 0x750c8886, 0x698d5fbd, 0x34d1064f
	};
UINT32	_rsa_test_plain_512[16] = {
	0xf996afff, 0x0eeb9d0b, 0x8a0eed9e, 0xfd89c2ed, 0x02774914, 0x97c2f09b, 0xf28e0ef6, 0xc2f39bc2,
	0x9d0af39b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	};
UINT32	_rsa_test_cipher_512[16] = {
	0x57dbacb8, 0x52529c74, 0xc0ace16b, 0xd66a28d6, 0x233fb9dc, 0x311f701c, 0xf20c6677, 0x5f9c061e,
	0x3b3772f6, 0x0373c9e6, 0xcdeb0692, 0x2a5f8eb0, 0x8987027c, 0x8e1b2604, 0x896e6a44, 0x887786bf
	};
/***** 1024 bit *****/
UINT32	_rsa_test_modulus_1024[32] = {
	0x3ec8ccbb, 0xfd762776, 0x3049bf66, 0xf17d5999,0x63056c10, 0xeca8305f, 0x5dc6f729, 0xeafd81fe,
	0x6508116b, 0x9ae3750f, 0xc98e2a8f, 0x22d25527,0xed872c1a, 0x8315af5a, 0xad9167b5, 0xc0536ffc,
	0x0f462a14, 0x2a184832, 0xb44a9525, 0xdf533827,0xf47ab37b, 0xcf9742dc, 0x4ec89ce1, 0x9095d779,
	0x00b3ab59, 0x204f332c, 0xca2a9847, 0xfa22c14d,0x0c9684f0, 0xfb00fd1f, 0xa479c966, 0x73533ede
	};
UINT32	_rsa_test_public_1024[32] = {
	0x00010001, 0x00000000, 0x00000000, 0x00000000,0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,0x00000000, 0x00000000, 0x00000000, 0x00000000
	};
UINT32	_rsa_test_private_1024[32] = {
	0x8de896a9, 0xc4350598, 0xb338dd0f, 0x00c8c54a,0xba146fb6, 0x80aa7b65, 0x2620cc14, 0x809756ec,
	0x8b125ffd, 0xe4321282, 0x61babb80, 0x59dbd0fd,0x4b914872, 0xc9d03d2d, 0xabedae0f, 0xc1c728ab,
	0x4fc36d16, 0x5910f13c, 0x2e1ad88c, 0x8d5e08c4,0x63caf0f6, 0x5594ee7d, 0xf22349f5, 0x074e91cb,
	0x2216ce72, 0x40a91be9, 0x4a818697, 0x4df0c726,0xba2ceebb, 0xd7701d1c, 0x9000af9e, 0x028757c0
	};
UINT32	_rsa_test_plain_1024[32] = {
	0xc0920278, 0x05c19503, 0x4905eb95, 0xbb58d480,0x14fd89c2, 0x9b027749, 0xf697c2f0, 0xc2f28e0e,
	0x9bc2f39b, 0x009d0af3, 0x00000000, 0x00000000,0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,0x00000000, 0x00000000, 0x00000000, 0x00000000
	};
UINT32	_rsa_test_cipher_1024[32] = {
	0xa661ad65, 0xb14f250f, 0x1f5db2b7, 0x7165a1b3,0x48fe4ff3, 0x6152b2f0, 0x1b8cc363, 0x6ab97ecf,
	0x5a3a8b64, 0x415ca171, 0x4437280d, 0x55846027,0x3fa61904, 0x2b72dca2, 0x9fd43385, 0x5a720807,
	0x9bbd78f7, 0x5eb290ca, 0x31d36e6a, 0x81839d24,0x802b2834, 0x04f7d8d5, 0xe634f2c9, 0xb62d319b,
	0x85dd23d1, 0x153dcf03, 0xfe5b0485, 0x8ad6724e,0x5f2f354f, 0xa10962c5, 0x97655220, 0x5837663f
	};
/***** 2048 bit *****/
#if (RSA_MAX_LEN==2048)
UINT32	_rsa_test_modulus_2048[64] = {
	0x05f4737f, 0x84cf590d, 0x4b685c36, 0x132dc06b, 0xc32c6a17, 0x462eddb0, 0x5d237aac, 0x50236c19,
	0x8f7d4f80, 0x44860cbe, 0x467f7a14, 0x8b4ded51, 0x05625533, 0x36830fbd, 0xc67a60e3, 0x8483f0c8,
	0x8d614870, 0xc4c3bd1d, 0x940be83e, 0x360c2f10, 0xd227051a, 0x763a699f, 0xd70367fd, 0xec536394,
	0xf893e040, 0x66c14c1c, 0xf1af1ef5, 0x8b8072e6, 0xb90d3237, 0xfa78336b, 0xba0c1899, 0x28e433b7,
	0xcbd1fcb3, 0x3c6e7c10, 0x698828b9, 0xa101a2c2, 0x156d6868, 0xbb8349ab, 0xf8cd5363, 0x50a7c2bd,
	0xd9748f48, 0x69c8e4ea, 0x2f13eed8, 0x238dc42a, 0xdf1e457b, 0xfcd7e28f, 0xcaed4b7b, 0x1efdaa99,
	0x507162e7, 0xbf476a20, 0x2f6f1aa4, 0xdcdaaf82, 0xa84636bd, 0xec14af50, 0xf76057d8, 0x16414f02,
	0x71775cfa, 0xe29e57f6, 0xf677a8a6, 0x3d4a5355, 0xf1e17c00, 0x604a9db3, 0x524c6d32, 0xe32d2558
	};
UINT32	_rsa_test_public_2048[64] = {
	0x00010001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	};
UINT32	_rsa_test_private_2048[64] = {
	0xeef42321, 0xc63d5760, 0xa544454e, 0x4e4f1131, 0x5f257e7f, 0x2787d03e, 0x664a3e9f, 0x2cdb372e,
	0x60d6e4e6, 0x600506a1, 0x88218ddc, 0xd1776f4a, 0x4dbd3ede, 0x54e81e88, 0xc197750d, 0x41831845,
	0x222ac69e, 0x26bf5711, 0x1c6b91ad, 0x335e0b07, 0x9152f6ac, 0xf927914d, 0x99824d2d, 0x1a056f5a,
	0x36a25c58, 0xf5da7cf5, 0xb72741d0, 0x3500ad85, 0x81306cf2, 0x8e18ee0a, 0x4ccab080, 0x0b69ab06,
	0x2d2e05fa, 0x36625c94, 0x49c44ebb, 0x242c64ac, 0x94b78634, 0xbcf4223f, 0x9a5f3d8f, 0x5d775f9a,
	0x7e36a5fe, 0x0b1a4bea, 0x44e7e128, 0x5a134989, 0x11fca7c3, 0x63ab11b4, 0x2f5e6504, 0xd5adb607,
	0x7d500650, 0x3ec41ca0, 0xb1daa0ad, 0xff08c4b4, 0xbb7d5024, 0x1c7faf5f, 0xa90a6a73, 0xe794bca5,
	0x9b7eff42, 0x8f43197b, 0x7cdcff4c, 0xe64f2453, 0x4d729c98, 0xb87e8d3a, 0xfd700f02, 0x37402c9b
	};
UINT32	_rsa_test_plain_2048[64] = {
	0xeb9103bf, 0x0278490a, 0x9503c092, 0xeb9505c1, 0xd6844905, 0x89c2bc58, 0x774914fd, 0xc2f09b02,
	0x8e0ef697, 0xf39bc2f2, 0x0af39bc2, 0x0000009d, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	};
UINT32	_rsa_test_cipher_2048[64] = {
	0x05d26c04, 0xf5cd1d97, 0x35374412, 0x729a8038, 0x1b69c267, 0x6692dea9, 0x1397a45c, 0xb65e9fa1,
	0x39f69aed, 0x0ba93c74, 0xf1ab1b20, 0x1ec867cc, 0x2ff30ae7, 0xb6a0722d, 0xca5f95f8, 0x62554388,
	0x74d80785, 0xe33aed8d, 0x89a5a8ff, 0x5fb9cca1, 0x1ccf17ee, 0x8331c68b, 0x83ce8df5, 0x7ef3ee76,
	0x514e09d0, 0x6522d4b2, 0x04e87d6c, 0xf33e239b, 0x8fbf777e, 0x6307d77f, 0x94e26922, 0xdef69afb,
	0xdbde3ece, 0x845afd38, 0xd98b9a91, 0x37ac3cc9, 0xda620df9, 0xaac5361f, 0x61d1b824, 0x7d29bf6f,
	0x03a378f5, 0xb343f30e, 0xa58fd180, 0x43428c03, 0x30b4b63a, 0xe4adf88c, 0xe222ac3d, 0x0f08e07d,
	0x57b156c5, 0x6e452575, 0xe606b05f, 0xbf22c003, 0x90aec421, 0x59e8394f, 0xffcc39ff, 0x7187603a,
	0xf8355a54, 0xdf9b1f61, 0x145b0fcb, 0x61389e97, 0x36db34ef, 0xc1acbeb8, 0x83e753ff, 0x69f11030
	};
#endif

/*====== SHA/HMAC test key data ======*/
UINT32	hmac_key[8] = {
	0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777, 0x88888888
	};
UINT32	hmac_plain[16] = {
	0xAABB0000, 0xAABB1111, 0xAABB2222, 0xAABB3333, 0xAABB4444, 0xAABB5555, 0xAABB6666, 0xAABB7777,
	0xAABB8888, 0xAABB9999, 0xAABBAAAA, 0xAABBBBBB, 0xAABBCCCC, 0xAABBDDDD, 0xAABBEEEE, 0xAABBFFFF
	};
UINT32	hmac_digest[8] = {
	0xb14367e2, 0xb0e324ed, 0x92caacf4, 0xeb8e286a, 0xa8e3d94f, 0x743442c8, 0xeedca946, 0xb221f1de
	};
UINT32	sha_digest[8] = {
	0xb3be4fa4, 0x40011b32, 0x481d6c0b, 0xbfcb8605, 0x34301afd, 0x37a81f3b, 0x27a928a4, 0x28991838
	};


#define TEST_BLOCK_MODE	1

#define SecureSS_IRQ_NO		CRYPTO_SOTERIA_IRQ
static volatile UINT32		secureIrqStatus;

void SecureSS_Handler(void)
{
	if(ss->busy)
		return;
	secureIrqStatus = ss->int_status;
	ss->int_status = secureIrqStatus; /* clear interrupt source */
	ss->int_en = 0;
	//fLib_printf("[ISR] secureIrqStatus = 0x%08x\n", secureIrqStatus);
}

typedef enum {
    SS_SUCCESS = 0,
    SS_FAIL = 1,
    SS_CANCEL = 2,
    SS_PROCESSING = 3,
    SS_BUSY = 4
} SS_Status;
char *SS_String[] = {"Success", "Fail", "Cancel", "Processing", "Busy"};

SS_Status SS_CheckFinish(void)
{
	if(secureIrqStatus == 0)
		return SS_PROCESSING;
	else if(secureIrqStatus & INTST_FAIL)
		return SS_FAIL;
	else if(secureIrqStatus & INTST_CANCEL)
		return SS_CANCEL;
	else
		return SS_SUCCESS;
}

SS_Status SS_Start(SEC_MSG *msg)
{
	if(ss->busy)
		return SS_BUSY;
	secureIrqStatus = 0;
	fLib_EnableIRQ(SecureSS_IRQ_NO);
	ss->int_status = 0x03; /* clear sec_subsys interrupt */
	ss->int_en = 0x01; /* enable SEC_SUBSYS interrupt */
	ss->msg = *msg;
	ss->start = 1;
	//fLib_printf("[SS_Start]\n");

	return SS_PROCESSING;
}

SS_Status SS_StartWaitComplete(SEC_MSG *msg)
{
	while(ss->busy); /* wait_sec_subsys_not_busy */
	secureIrqStatus = 0;
	fLib_EnableIRQ(SecureSS_IRQ_NO);
	ss->int_status = 0x03; /* clear sec_subsys interrupt */
	ss->int_en = 0x01; /* enable SEC_SUBSYS interrupt */
	ss->msg = *msg;
	ss->start = 1;
	//fLib_printf("[SS_StartWaitComplete]\n");

	while(secureIrqStatus==0); /* wait secure subsystem completed */
	if(secureIrqStatus & INTST_FAIL)
		return SS_FAIL;
	else if(secureIrqStatus & INTST_CANCEL)
		return SS_CANCEL;
	else
		return SS_SUCCESS;
}

void SS_Abort(void)
{
	ss->cancel = 1;
}

void SS_MemDump(UINT32 addr, UINT32 len, UINT32 showAddr)
{
	UINT32	count = (len+3)/4;
	UINT32	*ptrMem = (UINT32 *)addr;
	char	tmpStr[100];

	while(count)
	{
		switch(count)
		{
			case 3:
				sprintf(tmpStr, "%08X  %08x %08x %08x\n\r",(UINT32)showAddr,  *ptrMem, *(ptrMem+1), *(ptrMem+2));
				count = 0;
				break;
			case 2:
				sprintf(tmpStr, "%08X  %08x %08x\n\r",(UINT32)showAddr,  *ptrMem, *(ptrMem+1));
				count = 0;
				break;
			case 1:
				sprintf(tmpStr, "%08X  %08x\n\r",(UINT32)showAddr,  *ptrMem);
				count = 0;
				break;
			default:
				sprintf(tmpStr, "%08X  %08x %08x %08x %08x\n\r",(UINT32)showAddr,  *ptrMem, *(ptrMem+1), *(ptrMem+2), *(ptrMem+3));
				ptrMem=ptrMem+4;
				showAddr+=16;
				count-=4;
				break;
		}
		fLib_printf("%s", tmpStr);
	}
}

void SS_MemFill(UINT32 addr, UINT32 value, UINT32 len)
{
	UINT32	count = (len+3)/4;
	UINT32	*ptrMem = (UINT32 *)addr;

	while(count)
	{
		*ptrMem = value;
		count--;
		ptrMem++;
	}
}

BOOL SS_ReadWriteMemory(char *str)
{
	char		*tmpStr;
	UINT32	addr;
	UINT32	value;
	UINT32	len;

	//tmpStr = mainGetNextStringStart(str);
	tmpStr = str;
	//if(memcmp(tmpStr, "read", strlen("read")) == 0)
	if(mainCheckCmd("dump", &tmpStr) == TRUE)
	{
		//tmpStr = mainGetNextStringStart(&tmpStr[strlen("read")]);
		if(*tmpStr == 0)
			return FALSE;
		addr = mainStringToNum(tmpStr, &tmpStr);
		//tmpStr = mainGetNextStringStart(tmpStr);
		if(*tmpStr == 0)
			len = 4;
		else
			len = mainStringToNum(tmpStr, &tmpStr);
		fLib_printf("Address   Memory Dump\n\r");
		fLib_printf("========  ===================================\n\r");
		SS_MemDump(addr, len, addr);
		//value = read32(addr);
		//fLib_printf("Read 0x%x = 0x%x\n\r", addr, value);
	}
	//else if(memcmp(tmpStr, "write", strlen("write")) == 0)
	else if(mainCheckCmd("fill", &tmpStr) == TRUE)
	{
		//tmpStr = mainGetNextStringStart(&tmpStr[strlen("write")]);
		if(*tmpStr == 0)
			return FALSE;
		addr = mainStringToNum(tmpStr, &tmpStr);
		//tmpStr = mainGetNextStringStart(tmpStr);
		if(*tmpStr == 0)
			return FALSE;
		value = mainStringToNum(tmpStr, &tmpStr);
		//tmpStr = mainGetNextStringStart(tmpStr);
		if(*tmpStr == 0)
			len = 4;
		else
			len = mainStringToNum(tmpStr, &tmpStr);
		fLib_printf("Fill memory 0x%x = 0x%x, length %d\n\r", addr, value, len);
		SS_MemFill(addr, value, len);
		//write32(value, addr);
		//fLib_printf("write 0x%x = 0x%x\n\r", addr, value);
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SS_AES_Test(char *tmpStr)
{
	MSG_AES		aes;
	SS_Status	ssStatus;
	int		index;
	int		checkResult;
	int		testLen;

	memset(&aes, 0, sizeof(aes));
	aes.msg_op = SS_MSG_AES;
	/* operation mode */
	if(mainCheckCmd("ecb", &tmpStr) == TRUE)
		aes.cfg.mode = AES_MSG_ECB;
	else if(mainCheckCmd("cbc", &tmpStr) == TRUE)
		aes.cfg.mode = AES_MSG_CBC;
	else if(mainCheckCmd("ctr", &tmpStr) == TRUE)
		aes.cfg.mode = AES_MSG_CTR;
	else
		return FALSE;
	/* key length */
	if(mainCheckCmd("128", &tmpStr) == TRUE)
		aes.cfg.keySize = AES_MSG_KEY128;
	else if(mainCheckCmd("192", &tmpStr) == TRUE)
		aes.cfg.keySize  = AES_MSG_KEY192;
	else if(mainCheckCmd("256", &tmpStr) == TRUE)
		aes.cfg.keySize  = AES_MSG_KEY256;
	else
		return FALSE;
	/* test length */
	if(*tmpStr != 0)
		testLen = mainStringToNum(tmpStr, &tmpStr);
	else
		testLen = 1024;
	if(testLen<0 || testLen>1024)
		{fLib_printf("[Fail] Invalid test length %d\n", testLen); return FALSE;}
	aes.cfg.decrypt  = AES_MSG_ENCRYPT;/* Encrypt or Decrypt */
	aes.in_len = testLen;
	for(index=0; index<1024/4; index++)
		testBuf[0][index] = 0x12340000+index;
	aes.in_data = (UINT32)testBuf[0];
	aes.out_data = (UINT32)testBuf[1];
	aes.aes_key = (UINT32)_aes_test_key;
	aes.aes_iv = (UINT32)_aes_test_iv;

	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&aes);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] AES Encrypt\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&aes) != SS_PROCESSING)
		{fLib_printf("[Fail] AES Encrypt 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] AES Encrypt 2\n", SS_String[ssStatus]); return FALSE;}
	#endif

	if(ssMainPrintDebugMessage)
	{
		fLib_printf("Plaintext:\n");
		mainMemDump((UINT32)testBuf[0], testLen+16, (UINT32)testBuf[0]);
		fLib_printf("encrypted Ciphertext:\n");
		mainMemDump((UINT32)testBuf[1], testLen+16, (UINT32)testBuf[1]);
	}

	aes.cfg.decrypt  = AES_MSG_DECRYPT;/* Encrypt or Decrypt */
	aes.in_data = (UINT32)testBuf[1];
	aes.out_data = (UINT32)testBuf[2];
	aes.in_len = ((testLen+15)/16)*16;

	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&aes);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] AES Decrypt\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&aes) != SS_PROCESSING)
		{fLib_printf("[Fail] AES Decrypt 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] AES Decrypt 2\n", SS_String[ssStatus]); return FALSE;}
	#endif
	if(ssMainPrintDebugMessage)
	{
		fLib_printf("decrypted data:\n");
		mainMemDump((UINT32)testBuf[2], testLen+16, (UINT32)testBuf[2]);
	}

	/* compare data */
	checkResult = TRUE;
	for(index=0; index<testLen/4; index++)
	{
		if(testBuf[0][index] != testBuf[2][index])
		{
			fLib_printf("[Fail] Data mismatch: index=%d [0x%08x, 0x%08x]\n", index, testBuf[0][index], testBuf[2][index]);
			checkResult = FALSE;
		}
	}
	if(checkResult == TRUE)
		fLib_printf("Data compare: Pass\n");

	return TRUE;
}

BOOL SS_RSA_Test(char *tmpStr)
{
	MSG_RSA	rsa;
	SS_Status	ssStatus;
	UINT32	*privateKey;
	UINT32	*plainText;
	UINT32	*cipherText;
	int		ciphertextLen;
	int		checkResult, checkLen, index;
	int		cipherCheck;

	memset(&rsa, 0, sizeof(rsa));
	rsa.msg_op = SS_MSG_RSA;
	/* operation mode */
	if(mainCheckCmd("1024", &tmpStr) == TRUE)
	{
		rsa.cfg.mode = 2;
		rsa.in_data = (UINT32)_rsa_test_plain_1024;
		rsa.in_len = sizeof(_rsa_test_plain_1024);
		rsa.modulus = (UINT32)_rsa_test_modulus_1024;
		rsa.rsa_key = (UINT32)_rsa_test_public_1024;
		privateKey = _rsa_test_private_1024;
		ciphertextLen = 1024/8;
		cipherText = _rsa_test_cipher_1024;
	}
	else if(mainCheckCmd("512", &tmpStr) == TRUE)
	{
		rsa.cfg.mode = 1;
		rsa.in_data = (UINT32)_rsa_test_plain_512;
		rsa.in_len = sizeof(_rsa_test_plain_512);
		rsa.modulus = (UINT32)_rsa_test_modulus_512;
		rsa.rsa_key = (UINT32)_rsa_test_public_512;
		privateKey = _rsa_test_private_512;
		ciphertextLen = 512/8;
		cipherText = _rsa_test_cipher_512;
	}
	#if (RSA_MAX_LEN==2048)
	else if(mainCheckCmd("2048", &tmpStr) == TRUE)
	{
		rsa.cfg.mode = 3;
		rsa.in_data = (UINT32)_rsa_test_plain_2048;
		rsa.in_len = sizeof(_rsa_test_plain_2048);
		rsa.modulus = (UINT32)_rsa_test_modulus_2048;
		rsa.rsa_key = (UINT32)_rsa_test_public_2048;
		privateKey = _rsa_test_private_2048;
		ciphertextLen = 2048/8;
		cipherText = _rsa_test_cipher_2048;
	}
	#endif
	#if (RSA_MAX_LEN==1024)
	else if(mainCheckCmd("256", &tmpStr) == TRUE)
	{
		rsa.cfg.mode = 0;
		rsa.in_data = (UINT32)_rsa_test_plain_256;
		rsa.in_len = sizeof(_rsa_test_plain_256);
		rsa.modulus = (UINT32)_rsa_test_modulus_256;
		rsa.rsa_key = (UINT32)_rsa_test_public_256;
		privateKey = _rsa_test_private_256;
		ciphertextLen = 256/8;
		cipherText = _rsa_test_cipher_256;
	}
	#endif
	else
		return FALSE;

	if(mainCheckCmd("cipherChk", &tmpStr) == TRUE)
		cipherCheck = 1;
	else
		cipherCheck = 0;

	rsa.out_data = (UINT32)testBuf[0];
	checkLen = rsa.in_len;
	plainText = (UINT32 *)rsa.in_data;

	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&rsa);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] RSA Encrypt\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&rsa) != SS_PROCESSING)
		{fLib_printf("[Fail] RSA Encrypt 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] RSA Encrypt 2\n", SS_String[ssStatus]); return FALSE;}
	#endif
//	fLib_printf("encrypted Ciphertext:\n");
//	mainMemDump((UINT32)testBuf[0], ciphertextLen, (UINT32)testBuf[0]);

	rsa.in_data = (UINT32)testBuf[0];
	rsa.in_len = ciphertextLen;
	rsa.rsa_key = (UINT32)privateKey;
	rsa.out_data = (UINT32)testBuf[1];

	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&rsa);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] RSA Decrypt\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&rsa) != SS_PROCESSING)
		{fLib_printf("[Fail] RSA Decrypt 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] RSA Decrypt 2\n", SS_String[ssStatus]); return FALSE;}
	#endif
//	fLib_printf("Decrypted Ciphertext:\n");
//	mainMemDump((UINT32)testBuf[1], checkLen, (UINT32)testBuf[1]);

	/* compare data */
	checkResult = TRUE;
	for(index=0; index<checkLen/4; index++)
	{
		if(plainText[index] != testBuf[1][index])
		{
			fLib_printf("[Fail] Data mismatch: index=%d [0x%08x, 0x%08x]\n", index, plainText[index], testBuf[1][index]);
			checkResult = FALSE;
		}
	}
	if(checkResult == TRUE)
		fLib_printf("Data compare: Pass\n");

	if(cipherCheck == 1)
	{
		checkResult = TRUE;
		for(index=0; index<checkLen/4; index++)
		{
			if(cipherText[index] != testBuf[0][index])
			{
				fLib_printf("[Fail] Cipher check: index=%d [0x%08x, 0x%08x]\n", index, cipherText[index], testBuf[0][index]);
				checkResult = FALSE;
			}
		}
		if(checkResult == TRUE)
			fLib_printf("Cipher test: Pass\n");
	}

	return TRUE;
}

BOOL SS_SHA_HMAC_Test(int mode, char *tmpStr)
{
	MSG_SHA	sha;
	SS_Status	ssStatus;
	int		index;
	int		checkResult;
	UINT32	*checkOutput;

	memset(&sha, 0, sizeof(sha));
	sha.msg_op = SS_MSG_SHA;
	sha.cfg.mode = mode;
	checkOutput = (mode==SHA_MODE_HMAC) ? hmac_digest : sha_digest;
	sha.in_data = (UINT32)hmac_plain;
	sha.in_len = sizeof(hmac_plain);
	sha.out_data = (UINT32)testBuf[0];
	if(mode==SHA_MODE_HMAC)
		sha.key = (UINT32)hmac_key;

	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&sha);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] %s\n", SS_String[ssStatus], (mode==SHA_MODE_HMAC) ? "HMAC" : "SHA"); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&sha) != SS_PROCESSING)
		{fLib_printf("[Fail] %s 1\n", (mode==SHA_MODE_HMAC) ? "HMAC" : "SHA"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] %s 2\n", SS_String[ssStatus], (mode==SHA_MODE_HMAC) ? "HMAC" : "SHA"); return FALSE;}
	#endif
	fLib_printf("Digest:\n");
	mainMemDump((UINT32)testBuf[0], 8*4, (UINT32)testBuf[0]);

	/* compare data */
	checkResult = TRUE;
	for(index=0; index<8; index++)
	{
		if(testBuf[0][index] != checkOutput[index])
		{
			fLib_printf("[Fail] Data mismatch: index=%d [0x%08x, 0x%08x]\n", index, checkOutput[index], testBuf[0][index]);
			checkResult = FALSE;
		}
	}
	if(checkResult == TRUE)
		fLib_printf("Data compare: Pass\n");

	return TRUE;
}

BOOL SS_TRNG_Test(char *tmpStr)
{
	MSG_TRNG	trng;
	SS_Status	ssStatus;

	memset(&trng, 0, sizeof(trng));
	trng.msg_op = SS_MSG_TRNG;
	if(tmpStr[0] == 0)
		trng.req_len = 16;
	else
		trng.req_len = mainStringToNum(tmpStr, &tmpStr);
	trng.out_data = (UINT32)testBuf[0];
	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&trng);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] TRNG, len=%d\n", SS_String[ssStatus], trng.req_len); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&trng) != SS_PROCESSING)
		{fLib_printf("[Fail] TRNG 1, len=%d\n", trng.req_len); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] TRNG 2, len=%d\n", SS_String[ssStatus], trng.req_len); return FALSE;}
	#endif
	fLib_printf("Random Number:\n");
	mainMemDump((UINT32)testBuf[0], trng.req_len, (UINT32)testBuf[0]);

	return TRUE;
}

BOOL SS_CRC_Test(char *tmpStr)
{
	MSG_CRC	crc;
	SS_Status	ssStatus;
	UINT32	*crcSrc;
	UINT32	srcCount;
	UINT32	crcResult;

	memset(&crc, 0, sizeof(crc));
	crcSrc = testBuf[0];
	srcCount = 0;
	crc.msg_op = SS_MSG_CRC;
	while(*tmpStr)
	{
		*crcSrc = mainStringToNum(tmpStr, &tmpStr);
		crcSrc++;
		srcCount+=4;
	}
	fLib_printf("Input length=%d, data:\n", srcCount);
	mainMemDump((UINT32)testBuf[0], srcCount, (UINT32)testBuf[0]);

	crc.in_len = srcCount;
	crc.in_data = (UINT32)testBuf[0];
	crc.out_data = (UINT32)&crcResult;
	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&crc);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] CRC\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&crc) != SS_PROCESSING)
		{fLib_printf("[Fail] CRC 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] CRC 2\n", SS_String[ssStatus]); return FALSE;}
	#endif
	fLib_printf("Calculated CRC: 0x%08x\n", crcResult);

	return TRUE;
}

BOOL SS_KeyShow(char *tmpStr)
{
	BOOL	showAesKey, showRsa256_2048Key, showRsa512Key, showRsa1024Key, showHmacKey;

	showAesKey = showRsa256_2048Key = showRsa512Key = showRsa1024Key = showHmacKey = FALSE;
	if(*tmpStr == 0)
		showAesKey = showRsa256_2048Key = showRsa512Key = showRsa1024Key = showHmacKey = TRUE;
	else
	{
		while(*tmpStr != 0)
		{
			if(mainCheckCmd("aes", &tmpStr) == TRUE)
				showAesKey = TRUE;
			#if (RSA_MAX_LEN==1024)
			else if(mainCheckCmd("rsa 256", &tmpStr) == TRUE)
				showRsa256_2048Key = TRUE;
			#endif
			else if(mainCheckCmd("rsa 512", &tmpStr) == TRUE)
				showRsa512Key = TRUE;
			else if(mainCheckCmd("rsa 1024", &tmpStr) == TRUE)
				showRsa1024Key = TRUE;
			#if (RSA_MAX_LEN==2048)
			else if(mainCheckCmd("rsa 2048", &tmpStr) == TRUE)
				showRsa256_2048Key = TRUE;
			#endif
			else if(mainCheckCmd("rsa", &tmpStr) == TRUE)
				showRsa256_2048Key = showRsa512Key = showRsa1024Key = TRUE;
			else if(mainCheckCmd("hmac", &tmpStr) == TRUE)
				showHmacKey = TRUE;
			else
				return FALSE;
		}
	}
	if(showAesKey)
	{
		fLib_printf("AES Key:\n");
		SS_MemDump((UINT32)_aes_test_key, sizeof(_aes_test_key), (UINT32)_aes_test_key);
		fLib_printf("AES IV:\n");
		SS_MemDump((UINT32)_aes_test_iv, sizeof(_aes_test_iv), (UINT32)_aes_test_iv);
	}
	#if (RSA_MAX_LEN==1024)
	if(showRsa256_2048Key)
	{
		fLib_printf("RSA-256 Modulus:\n");
		SS_MemDump((UINT32)_rsa_test_modulus_256, sizeof(_rsa_test_modulus_256), (UINT32)_rsa_test_modulus_256);
		fLib_printf("RSA-256 Public Key:\n");
		SS_MemDump((UINT32)_rsa_test_public_256, sizeof(_rsa_test_public_256), (UINT32)_rsa_test_public_256);
		fLib_printf("RSA-256 Private Key:\n");
		SS_MemDump((UINT32)_rsa_test_private_256, sizeof(_rsa_test_private_256), (UINT32)_rsa_test_private_256);
	}
	#endif
	if(showRsa512Key)
	{
		fLib_printf("RSA-512 Modulus:\n");
		SS_MemDump((UINT32)_rsa_test_modulus_512, sizeof(_rsa_test_modulus_512), (UINT32)_rsa_test_modulus_512);
		fLib_printf("RSA-512 Public Key:\n");
		SS_MemDump((UINT32)_rsa_test_public_512, sizeof(_rsa_test_public_512), (UINT32)_rsa_test_public_512);
		fLib_printf("RSA-512 Private Key:\n");
		SS_MemDump((UINT32)_rsa_test_private_512, sizeof(_rsa_test_private_512), (UINT32)_rsa_test_private_512);
	}
	if(showRsa1024Key)
	{
		fLib_printf("RSA-1024 Modulus:\n");
		SS_MemDump((UINT32)_rsa_test_modulus_1024, sizeof(_rsa_test_modulus_1024), (UINT32)_rsa_test_modulus_1024);
		fLib_printf("RSA-1024 Public Key:\n");
		SS_MemDump((UINT32)_rsa_test_public_1024, sizeof(_rsa_test_public_1024), (UINT32)_rsa_test_public_1024);
		fLib_printf("RSA-1024 Private Key:\n");
		SS_MemDump((UINT32)_rsa_test_private_1024, sizeof(_rsa_test_private_1024), (UINT32)_rsa_test_private_1024);
	}
	#if (RSA_MAX_LEN==2048)
	if(showRsa256_2048Key)
	{
		fLib_printf("RSA-2048 Modulus:\n");
		SS_MemDump((UINT32)_rsa_test_modulus_2048, sizeof(_rsa_test_modulus_2048), (UINT32)_rsa_test_modulus_2048);
		fLib_printf("RSA-2048 Public Key:\n");
		SS_MemDump((UINT32)_rsa_test_public_2048, sizeof(_rsa_test_public_2048), (UINT32)_rsa_test_public_2048);
		fLib_printf("RSA-2048 Private Key:\n");
		SS_MemDump((UINT32)_rsa_test_private_2048, sizeof(_rsa_test_private_2048), (UINT32)_rsa_test_private_2048);
	}
	#endif
	if(showHmacKey)
	{
		fLib_printf("HMAC Key:\n");
		SS_MemDump((UINT32)hmac_key, sizeof(hmac_key), (UINT32)hmac_key);
	}

	return TRUE;
}

BOOL SS_SetKeyRSA(char *tmpStr)
{
	void	*nKeys[]= {_rsa_test_modulus_512, _rsa_test_modulus_1024, _rsa_test_modulus_2048};
	void	*dKeys[]= {_rsa_test_private_512, _rsa_test_private_1024, _rsa_test_private_2048};
	void	*eKeys[]= {_rsa_test_public_512, _rsa_test_public_1024, _rsa_test_public_2048};
	void	*plains[]= {_rsa_test_plain_512, _rsa_test_plain_1024, _rsa_test_plain_2048};
	void	*ciphers[]= {_rsa_test_cipher_512, _rsa_test_cipher_1024, _rsa_test_cipher_2048};
	int		keyIndex;
	int		maxCount;
	UINT32	*keyPtr;
	char	*pStr;

	//fLib_printf("String= %s\n", tmpStr);
	/* Key index */
	if(mainCheckCmd("1024", &tmpStr) == TRUE)
		{keyIndex = 1; maxCount=1024/8/4;}
	else if(mainCheckCmd("512", &tmpStr) == TRUE)
		{keyIndex = 0; maxCount=512/8/4;}
	else if(mainCheckCmd("2048", &tmpStr) == TRUE)
		{keyIndex = 2; maxCount=2048/8/4;}
	else
		return FALSE;
	/* key type */
	if(mainCheckCmd("N", &tmpStr) == TRUE)
		keyPtr = nKeys[keyIndex];
	else if(mainCheckCmd("D", &tmpStr) == TRUE)
		keyPtr = dKeys[keyIndex];
	else if(mainCheckCmd("E", &tmpStr) == TRUE)
		keyPtr = eKeys[keyIndex];
	else if(mainCheckCmd("Plain", &tmpStr) == TRUE)
		keyPtr = plains[keyIndex];
	else if(mainCheckCmd("Cipher", &tmpStr) == TRUE)
		keyPtr = ciphers[keyIndex];
	else
		return FALSE;
	/* fill in key */
	memset(keyPtr, 0, maxCount*4);
	keyIndex = 0;
	while((strlen(tmpStr)>0) && (keyIndex<maxCount))
	{
		if(strlen(tmpStr) > 8)
			pStr = &tmpStr[strlen(tmpStr)-8];
		else
			pStr = tmpStr;
		keyPtr[keyIndex] = strtoul(pStr, NULL, 16);
		pStr[0] = 0;
		keyIndex++;
	}

	return TRUE;
}

//UINT32	imageBuffer[0x1000/4];
//char	tmpStringBuf[100];
BOOL SS_pluginTest(char *tmpStr)
{
	UINT32	pluginMsg[sizeof(SEC_MSG)/4];
	SS_Status	ssStatus;
//	int		index;
	UINT32	*pFlashImage;
	char	*tmpStringBuf = (char *)testBuf[0];

	fLib_printf("string: %s\n", tmpStr);
	pFlashImage = (UINT32 *)(0x0C000000+0x100000);
//	for(index=0; index<0x1000/4; index++)
//		imageBuffer[index] = pFlashImage[index];
//	SS_MemDump((UINT32)imageBuffer, 0x100, (UINT32)imageBuffer);

	memset(&pluginMsg, 0, sizeof(pluginMsg));
	pluginMsg[0] = 0xAB;
//	pluginMsg[1] = (UINT32)imageBuffer; /* send SRAM buffer address */
	pluginMsg[1] = (UINT32)pFlashImage; /* send SPI XIP address */
	pluginMsg[2] = (UINT32)0x1000;
	pluginMsg[3] = (UINT32)0x00044000;
	strcpy(tmpStringBuf, tmpStr);
	pluginMsg[4] = (UINT32)tmpStringBuf;
	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&pluginMsg);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] Plugin\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&pluginMsg) != SS_PROCESSING)
		{fLib_printf("[Fail] Plugin 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] Plugin 2\n", SS_String[ssStatus]); return FALSE;}
	#endif
	fLib_printf("Plugin complete\n");

	return TRUE;

}

BOOL SS_MainTest(char *tmpStr)
{
	fLib_printf("0x0C000000:\n");
	SS_MemDump(0x0C000000, 0x100, 0x0C000000);
	fLib_printf("0x0C100000:\n");
	SS_MemDump(0x0C100000, 0x100, 0x0C100000);

	return TRUE;
}

BOOL SS_eFuseTest(char *str)
{
	MSG_EFUSE_OP	efuse;
	SS_Status	ssStatus;
	char		*tmpStr;
	UINT32	addr;
	UINT32	value;
	int	len, tmpLen;
	UINT32	*tmpBuf = testBuf[0];

	memset(&efuse, 0, sizeof(efuse));
	efuse.msg_op = SS_MSG_EFUSE_OP;

	tmpStr = str;
	if(mainCheckCmd("dump", &tmpStr) == TRUE)
	{
		efuse.cfg = EFUSE_READ_OP;
		if(*tmpStr == 0)
			return FALSE;
		addr = mainStringToNum(tmpStr, &tmpStr);
		if(*tmpStr == 0)
			len = 4;
		else
			len = mainStringToNum(tmpStr, &tmpStr);
		tmpLen = len;
		while(tmpLen > 0)
		{
			efuse.address = addr;
			#if (TEST_BLOCK_MODE==1)
			ssStatus = SS_StartWaitComplete((SEC_MSG *)&efuse);
			if(ssStatus != SS_SUCCESS)
				{fLib_printf("[%s] eFuse\n", SS_String[ssStatus]); return FALSE;}
			#else
				if(SS_Start((SEC_MSG *)&efuse) != SS_PROCESSING)
					{fLib_printf("[Fail] eFuse 1\n"); return FALSE;}
				while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
				ssStatus = SS_CheckFinish();
				if(ssStatus != SS_SUCCESS)
					{fLib_printf("[%s] eFuse 2\n", SS_String[ssStatus]); return FALSE;}
			#endif
			*tmpBuf = ss->msg.efuse_op.data;
			tmpBuf++;
			addr+=4;
			tmpLen-=4;
		}
		fLib_printf("Address   Memory Dump\n\r");
		fLib_printf("========  ===================================\n\r");
		SS_MemDump((UINT32)testBuf[0], len, (UINT32)0);
	}
	else if(mainCheckCmd("fill", &tmpStr) == TRUE)
	{
		efuse.cfg = EFUSE_WRITE_OP;
		if(*tmpStr == 0)
			return FALSE;
		addr = mainStringToNum(tmpStr, &tmpStr);
		if(*tmpStr == 0)
			return FALSE;
		value = mainStringToNum(tmpStr, &tmpStr);
		if(*tmpStr == 0)
			len = 4;
		else
			len = mainStringToNum(tmpStr, &tmpStr);
		fLib_printf("Fill eFuse 0x%x = 0x%x, length %d\n\r", addr, value, len);
		while(len > 0)
		{
			efuse.address = addr;
			efuse.data = value;
			#if (TEST_BLOCK_MODE==1)
			ssStatus = SS_StartWaitComplete((SEC_MSG *)&efuse);
			if(ssStatus != SS_SUCCESS)
				{fLib_printf("[%s] eFuse\n", SS_String[ssStatus]); return FALSE;}
			#else
				if(SS_Start((SEC_MSG *)&efuse) != SS_PROCESSING)
					{fLib_printf("[Fail] eFuse 1\n"); return FALSE;}
				while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
				ssStatus = SS_CheckFinish();
				if(ssStatus != SS_SUCCESS)
					{fLib_printf("[%s] eFuse 2\n", SS_String[ssStatus]); return FALSE;}
			#endif
			addr+=4;
			len-=4;
		}
	}
	else if(mainCheckCmd("clear", &tmpStr) == TRUE)
	{
		efuse.cfg = EFUSE_CLEAR_OP;
		#if (TEST_BLOCK_MODE==1)
		ssStatus = SS_StartWaitComplete((SEC_MSG *)&efuse);
		if(ssStatus != SS_SUCCESS)
			{fLib_printf("[%s] eFuse\n", SS_String[ssStatus]); return FALSE;}
		#else
			if(SS_Start((SEC_MSG *)&efuse) != SS_PROCESSING)
				{fLib_printf("[Fail] eFuse 1\n"); return FALSE;}
			while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
			ssStatus = SS_CheckFinish();
			if(ssStatus != SS_SUCCESS)
				{fLib_printf("[%s] eFuse 2\n", SS_String[ssStatus]); return FALSE;}
		#endif
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SS_revokeKeyTest(char *str)
{
	MSG_REVOKE_KEY	revokekey;
	SS_Status	ssStatus;

	memset(&revokekey, 0, sizeof(revokekey));
	revokekey.msg_op = SS_MSG_REVOKE_KEY;
	if(*str == 0)
		return FALSE;
	revokekey.magicNo1 = REVOKEKEY_MG1;
	revokekey.magicNo2 = REVOKEKEY_MG2;
	revokekey.revokeKeyId = mainStringToNum(str, &str);

	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&revokekey);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] revoke key\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&revokekey) != SS_PROCESSING)
		{fLib_printf("[Fail] revoke key 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] revoke key 2\n", SS_String[ssStatus]); return FALSE;}
	#endif

	return TRUE;
}

BOOL SS_dbgCtrlTest(char *str)
{
	MSG_DBG_CTRL	dbgctrl;
	SS_Status	ssStatus;

	memset(&dbgctrl, 0, sizeof(dbgctrl));
	dbgctrl.msg_op = SS_MSG_DBG_CTRL;
	dbgctrl.magicNo1 = DBGCTRL_MG1;
	dbgctrl.magicNo2 = DBGCTRL_MG2;

	#if (TEST_BLOCK_MODE==1)
	ssStatus = SS_StartWaitComplete((SEC_MSG *)&dbgctrl);
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] DBG CTRL\n", SS_String[ssStatus]); return FALSE;}
	#else
	if(SS_Start((SEC_MSG *)&dbgctrl) != SS_PROCESSING)
		{fLib_printf("[Fail] DBG CTRL 1\n"); return FALSE;}
	while(SS_CheckFinish() == SS_PROCESSING); /* wait secure subsystem completed */
	ssStatus = SS_CheckFinish();
	if(ssStatus != SS_SUCCESS)
		{fLib_printf("[%s] DBG CTRL 2\n", SS_String[ssStatus]); return FALSE;}
	#endif

	return TRUE;
}

void SS_DelayLoop(UINT32 loop)
{
	while(loop)
	{
		loop--;
	}
}

BOOL SS_abortTest(char *tmpStr)
{
	SEC_MSG		ssMsg;
	MSG_AES		*aes;
	MSG_RSA		*rsa;
	MSG_SHA		*sha;
	MSG_TRNG	*trng;
	MSG_CRC		*crc;
	SS_Status	ssStatus;
	UINT32		delayCnt;

	memset(&ssMsg, 0, sizeof(ssMsg));

	if(mainCheckCmd("aes", &tmpStr) == TRUE)
	{
		aes = (MSG_AES *)&ssMsg;
		aes->msg_op = SS_MSG_AES;
		aes->cfg.mode = AES_MSG_ECB;
		aes->cfg.keySize  = AES_MSG_KEY256;
		aes->cfg.decrypt  = AES_MSG_ENCRYPT;/* Encrypt or Decrypt */
		aes->in_len = 1024;
		aes->in_data = (UINT32)testBuf[0];
		aes->out_data = (UINT32)testBuf[1];
		aes->aes_key = (UINT32)_aes_test_key;
		aes->aes_iv = (UINT32)_aes_test_iv;
	}
	else if(mainCheckCmd("rsa", &tmpStr) == TRUE)
	{
		rsa = (MSG_RSA *)&ssMsg;
		rsa->msg_op = SS_MSG_RSA;
		rsa->cfg.mode = 2;
		rsa->in_data = (UINT32)_rsa_test_plain_1024;
		rsa->in_len = sizeof(_rsa_test_plain_1024);
		rsa->modulus = (UINT32)_rsa_test_modulus_1024;
		rsa->rsa_key = (UINT32)_rsa_test_public_1024;
		rsa->out_data = (UINT32)testBuf[0];
	}
	else if(mainCheckCmd("sha", &tmpStr) == TRUE)
	{
		sha = (MSG_SHA *)&ssMsg;
		sha->msg_op = SS_MSG_SHA;
		sha->cfg.mode = SHA_MODE_SHA;
		sha->in_data = (UINT32)hmac_plain;
		sha->in_len = sizeof(hmac_plain);
		sha->out_data = (UINT32)testBuf[0];

	}
	else if(mainCheckCmd("trng", &tmpStr) == TRUE)
	{
		trng = (MSG_TRNG *)&ssMsg;
		trng->msg_op = SS_MSG_TRNG;
		trng->req_len = 1024;
		trng->out_data = (UINT32)testBuf[0];
	}
	else if(mainCheckCmd("crc", &tmpStr) == TRUE)
	{
		crc = (MSG_CRC *)&ssMsg;
		crc->msg_op = SS_MSG_CRC;
		crc->in_len = 1024;
		crc->in_data = (UINT32)testBuf[0];
		crc->out_data = (UINT32)testBuf[1];
	}
	else
		return FALSE;

	ssStatus = SS_Start((SEC_MSG *)&ssMsg);
	delayCnt = 0x10000;
	if(*tmpStr != 0)
		delayCnt = mainStringToNum(tmpStr, &tmpStr);
	SS_DelayLoop(delayCnt);
	SS_Abort();
	while(ssStatus == SS_PROCESSING)
	{
		ssStatus = SS_CheckFinish();
	}
	fLib_printf("Status = %d, [%s]\n", ssStatus, SS_String[ssStatus]);

	return TRUE;
}

void SS_dumpMenu(void)
{
	fLib_printf("+-----------------------------------------+\n");
	fLib_printf("?) This help menu\n");
	fLib_printf("mem dump <addr> <len>\n");
	fLib_printf("mem fill <addr> <data> <len>\n");
	fLib_printf("aes [ecb | cbc | ctr] <128 | 192 | 256> <len>\n");
	fLib_printf("rsa [%d | %d | %d] {cipherChk}\n", RSA_MAX_LEN/4, RSA_MAX_LEN/2, RSA_MAX_LEN);
	fLib_printf("sha\n");
	fLib_printf("hmac\n");
	fLib_printf("trng <len>\n");
	fLib_printf("crc {data}\n");
	fLib_printf("key {aes | rsa {%d | %d | %d} | hmac}\n", RSA_MAX_LEN/4, RSA_MAX_LEN/2, RSA_MAX_LEN);
	fLib_printf("setkey rsa [%d | %d | %d] [N | D | E | Plain | Cipher] {key}\n", RSA_MAX_LEN/4, RSA_MAX_LEN/2, RSA_MAX_LEN);
	fLib_printf("plugin <string>\n");
	fLib_printf("test\n");
	fLib_printf("efuse [clear | dump <addr> <len> | fill <addr> <data> <len>]\n");
	fLib_printf("revoke key <id>\n");
	fLib_printf("dbg ctrl\n");
	fLib_printf("abort [aes | rsa | sha | trng | crc] <delayCnt>\n");
	fLib_printf("dbgmessage\n");
	fLib_printf("+-----------------------------------------+\n");
}

void executeCmd(char *cmd)
{
	char	*tmpStr;
	BOOL	retVal = TRUE;

	tmpStr = cmd;
	if(tmpStr[0] == 0)
		;/* NULL string do nothing */
	else if(mainCheckCmd("?", &tmpStr) == TRUE)
		SS_dumpMenu();
	else if(mainCheckCmd("mem", &tmpStr) == TRUE)
		retVal = SS_ReadWriteMemory(tmpStr);
	else if(mainCheckCmd("aes", &tmpStr) == TRUE)
		retVal = SS_AES_Test(tmpStr);
	else if(mainCheckCmd("rsa", &tmpStr) == TRUE)
		retVal = SS_RSA_Test(tmpStr);
	else if(mainCheckCmd("sha", &tmpStr) == TRUE)
		retVal = SS_SHA_HMAC_Test(SHA_MODE_SHA, tmpStr);
	else if(mainCheckCmd("hmac", &tmpStr) == TRUE)
		retVal = SS_SHA_HMAC_Test(SHA_MODE_HMAC, tmpStr);
	else if(mainCheckCmd("trng", &tmpStr) == TRUE)
		retVal = SS_TRNG_Test(tmpStr);
	else if(mainCheckCmd("crc", &tmpStr) == TRUE)
		retVal = SS_CRC_Test(tmpStr);
	else if(mainCheckCmd("key", &tmpStr) == TRUE)
		retVal = SS_KeyShow(tmpStr);
	else if(mainCheckCmd("setkey rsa", &tmpStr) == TRUE)
		retVal = SS_SetKeyRSA(tmpStr);
	else if(mainCheckCmd("plugin", &tmpStr) == TRUE)
		retVal = SS_pluginTest(tmpStr);
	else if(mainCheckCmd("test", &tmpStr) == TRUE)
		retVal = SS_MainTest(tmpStr);
	else if(mainCheckCmd("efuse", &tmpStr) == TRUE)
		retVal = SS_eFuseTest(tmpStr);
	else if(mainCheckCmd("revoke key", &tmpStr) == TRUE)
		retVal = SS_revokeKeyTest(tmpStr);
	else if(mainCheckCmd("dbg ctrl", &tmpStr) == TRUE)
		retVal = SS_dbgCtrlTest(tmpStr);
	else if(mainCheckCmd("abort", &tmpStr) == TRUE)
		retVal = SS_abortTest(tmpStr);
	else if(mainCheckCmd("dbgmessage", &tmpStr) == TRUE)
		ssMainPrintDebugMessage = (!ssMainPrintDebugMessage);
	else
		fLib_printf("Unknown CMD %s\n", cmd);

	if(retVal == FALSE)
		fLib_printf("Fail to run CMD %s\n", cmd);
}

void SS_main(void)
{
	char	getCmd[MAX_CMD_LEN];

	fLib_Int_Init();
	fLib_ConnectIRQ(SecureSS_IRQ_NO, SecureSS_Handler);

	SS_dumpMenu();
	fLib_printf("CMD> ");
	while(1)
	{
		fLib_scanf(getCmd);
		executeCmd(getCmd);
		fLib_printf("CMD> ");
	}
}
