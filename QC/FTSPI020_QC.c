
#include "SoFlexible.h"

extern void FTSPI020_Test_Main(); 
extern void Read_SPI_Flash_ID();
extern void Read_Page0_Data();
extern void Program_Page0_Data();
extern void Erase_64K();


void FTSPI020_QC_Main()
{
	FTSPI020_Test_Main();  
	fLib_printf("End the FTSPI020_QC test \n");
}
