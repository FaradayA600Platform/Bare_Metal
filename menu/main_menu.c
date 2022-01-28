#ifndef NULL
	#define NULL		0
#endif


extern void A500_QC_Main();
extern void A600_QC_Main();
extern void CLI_Test_Main();

extern void legacy_Main(void);

struct burnin_cmd burnin_cmd_value[] = {
#ifdef CONFIG_QC
	//{"FIE3500 QC", A500_QC_Main},
	{"FXI3FA017A QC", A600_QC_Main},
#endif
	{"Command Line Interface", CLI_Test_Main},
	{"Legacy",legacy_Main},
	{"", NULL}
};
