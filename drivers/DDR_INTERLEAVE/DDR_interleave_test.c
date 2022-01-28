/*------------------------------------------------------------------------------
DDR_interleave.c

copyright 2020-2021 Faraday technology corporation.
------------------------------------------------------------------------------*/
/* include files */
#include "SoFlexible.h"

#define DDR_BASE        0x80000000
#define DDR_TEST_START  0x80000000
#define DDR_TEST_SIZE   0x20000000

void DDR_interleave_main(void)
{
	unsigned int val, errs;
	int i, j, page;

	printf("DDR interleave test(start=0x%08x, end=0x%x)\n", DDR_TEST_START, DDR_TEST_START + DDR_TEST_SIZE);

	printf(" - Turn On interleave...");
	//turn on interleave
	val = readl(0x2D080000 + 0x00000148);
	val |= 0x00000002;
	writel(val, 0x2D080000 + 0x00000148);

	//change RANK0_BASE for FTDDRC440
	val = readl(0x2A620000 + 0x0000003C) & ~0xFF000000;
	val |= 0x04000000;
	writel(val, 0x2A620000 + 0x0000003C);
	printf("done\n");

	printf(" - Filling address pattern...");
	//fill address pattern
	for (i = 0; i < DDR_TEST_SIZE; i += 4) {
		writel((DDR_TEST_START + i), (DDR_TEST_START + i));
	}
	printf("done\n");

	printf(" - Turn off interleave...");
	//turn off interleave
	val = readl(0x2D080000 + 0x00000148);
	val &= ~0x00000002;
	writel(val, 0x2D080000 + 0x00000148);

	//change RANK0_BASE for FTDDRC440
	val = readl(0x2A620000 + 0x0000003C) & ~0xFF000000;
	val |= 0x08000000;
	writel(val, 0x2A620000 + 0x0000003C);
	printf("done\n");

	printf(" - Checking data...");
	errs = 0;
	page = 0;;
	for (i = 0; i < DDR_TEST_SIZE/2; i += 4096) {
		//do the data compare on the DDR_0 (0x8000_0000~0xBFFF_FFFF)
		for (j = 0; j < 4096; j += 4) {
			if ((DDR_TEST_START + page*4096 + j) != readl(DDR_BASE + (DDR_TEST_START-DDR_BASE)/2 + i + j)) {
				printf("*ERROR* ddr0 compare failed on the addr: 0x%08x(expected: 0x%08x, real: 0x%08x)\n",
				       (DDR_TEST_START + i + j), (DDR_TEST_START + page*4096 + j),
				       readl(DDR_BASE + (DDR_TEST_START-DDR_BASE)/2 + i + j));
				errs ++;
			}
		}
		page ++;
		
		//do the data compare on the DDR_1 (0xC000_0000~0xFFFF_FFFF)
		for (j = 0; j < 4096; j += 4) {
			if ((DDR_TEST_START + page*4096 + j) != readl(DDR_BASE + (DDR_TEST_START-DDR_BASE)/2 + i + j + 0x40000000)) {
				printf("*ERROR* ddr1 compare failed on the addr: 0x%08x(expected: 0x%08x, real: 0x%08x)\n",
				       (DDR_TEST_START + i + j + 0x40000000), (DDR_TEST_START + page*4096 + j),
				       readl(DDR_BASE + (DDR_TEST_START-DDR_BASE)/2 + i + j + 0x40000000));
				errs ++;
			}
		}
		page ++;
	}
	printf("done\n");

	if (errs == 0)
		printf("DDR interleave test COMPLETED SUCCESSFULLY\n");
	else
		printf("DDR interleave test COMPLETED with %d ERRORS\n", errs);
}