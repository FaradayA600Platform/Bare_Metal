#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SoFlexible.h"
extern void FTSDC021_main();

void FTSDC021_QC_Main()
{
	printf("FTSDC021_QC_Main start\r\n");
	FTSDC021_main(); // main is the same as old pbs code
	printf("==End of FTSDC021 QC test==\n");
}
