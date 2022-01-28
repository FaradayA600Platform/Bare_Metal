/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_sdio_test.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2020/1/10	Sanjin Liu	 FTSDC021 SDIO test code
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021_test.h"

int32_t SDC_sdio_test(int32_t argc, char *const argv[])
{
	if (cardInfo.CardType != SDIO_TYPE_CARD) {
		printf(" No SDIO Card insert.\r\n");
		return 0;
	}

	ftsdc021_sdio_Card_Info(&cardInfo);

	return 0;
}
