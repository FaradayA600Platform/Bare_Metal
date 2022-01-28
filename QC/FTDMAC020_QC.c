
extern void FTDMAC020_main(); // ctd code main
//extern void FTDMAC020_main_linkmode(void);
extern void DMA_QC_Main2(void);

void FTDMAC020_QC_Main()
{
		
	FTDMAC020_main();
//	FTDMAC020_main_linkmode();
	//DMA_QC_Main2();
	fLib_printf("End the FTDMAC020_QC test \n");
}
