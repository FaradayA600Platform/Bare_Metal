#ifndef __IRQ
#define __IRQ
//-----------------------------------------------------------------------------
//I2C
//-----------------------------------------------------------------------------
#define 	I2C_Data_L	32
#define 	I2C_Com_L	8

volatile int	I2C_Data_Length		;
volatile int	I2C_Com_Length		;
volatile int	I2C_R_W			;
volatile int	I2C_Data_Index		;
volatile int	I2C_Com_Index		;
volatile int	I2C_Command[I2C_Com_L]	;
volatile int	I2C_Command_Con[I2C_Com_L];
volatile int	I2C_Data[I2C_Data_L]	;
volatile int	I2C_Data_Con[I2C_Data_L];
//-----------------------------------------------------------------------------
//SMMC
//-----------------------------------------------------------------------------
#define 	SMMC_Data_L	128		//512 Bytes

volatile int	SMMC_IRQ_Status		;
volatile int	SMMC_Data_Index		;
volatile int	SMMC_Data_Length	;
volatile int	SMMC_Data[SMMC_Data_L]	;

//-----------------------------------------------------------------------------
//SSP1
//-----------------------------------------------------------------------------
#define 	SSP1_Data_L	35

volatile int	SSP1_Length		;
volatile int	SSP1_R_Index		;
volatile int	SSP1_W_Index		;
volatile int	SSP1_Data_R[SSP1_Data_L];
volatile int	SSP1_Data_W[SSP1_Data_L];

//-----------------------------------------------------------------------------
//SSP2
//-----------------------------------------------------------------------------
#define 	SSP2_Data_L	35

volatile int	SSP2_Length		;
volatile int	SSP2_R_Index		;
volatile int	SSP2_W_Index		;
volatile int	SSP2_Data_R[SSP2_Data_L];
volatile int	SSP2_Data_W[SSP2_Data_L];

//-----------------------------------------------------------------------------
//SSP3
//-----------------------------------------------------------------------------
#define 	SSP3_Data_L	35

volatile int	SSP3_Length		;
volatile int	SSP3_R_Index		;
volatile int	SSP3_W_Index		;
volatile int	SSP3_Data_R[SSP3_Data_L];
volatile int	SSP3_Data_W[SSP3_Data_L];

//-----------------------------------------------------------------------------
//CFC
//-----------------------------------------------------------------------------
#define 	CFC_Data_L	512

volatile int	CFC_IRQ_Status		;
volatile int	CFC_Card_Status		;

//-----------------------------------------------------------------------------
//GPIO
//-----------------------------------------------------------------------------
volatile int	GPIO_IN_Data		;
volatile int	GPIO_IRQ_Status		;

//-----------------------------------------------------------------------------
// SD
//-----------------------------------------------------------------------------
volatile int 	FLAG_DATA_END;

//-----------------------------------------------------------------------------
//TM2
//-----------------------------------------------------------------------------
volatile int 	TM2_INT_Status;
#endif
