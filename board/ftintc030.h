#include <stdio.h>
#include "SoFlexible.h"

#define         CONFIG_SPI_TRARGET_INVERSE      1

#define 	IPMODULE			INTC
#define 	IPNAME				FTINTC030
#define 	FTINTC030_BASE			IP_PA_BASE(0) 

#ifdef CONFIG_PLATFORM_A380
//#define		FTINTC030_SPI_CPU0_ID   0x600	// depend on IP configuration
#define		FTINTC030_SPI_CPU0_ID   0x300	// depend on IP configuration
#define		FTINTC030_SPI_CPU1_ID   0x10	// depend on IP configuration
#define		FTINTC030_SPI_CPU2_ID   0x20	// depend on IP configuration
#define		FTINTC030_SPI_CPU3_ID   0x30	// depend on IP configuration
#endif
#define		FTINTC030_PPI_SGI_NUM   32
#define		FTINTC030_PPI_NUM		16
#define		FTINTC030_SGI_NUM		16

// FTINTC030 Register Definition
#define 	FTINTC030_SPI_SRC0		0x0
#define 	FTINTC030_SPI_ENABLE    0x04
#define 	FTINTC030_SPI_CLEAR 	0x08
#define 	FTINTC030_SPI_MODE		0x0C
#define 	FTINTC030_SPI_LEVEL		0x10
#define		FTINTC030_DEBOUNCE 		0x18
#define 	FTINTC030_SPI_SRC1		0x20
#define		FTINTC030_DEBOUNCE0		0x58
#define		FTINTC030_DEBOUNCE1		0x5C
#define 	FTINTC030_SPI_PRIORITY	0x240
#define 	FTINTC030_SPI_TARGET    0x420
#define		FTINTC030_PPI_SRC		0x640
#define		FTINTC030_SGI_PRIORITY	0x660
#define 	FTINTC030_PPI_PRIORITY	0x670
#define		FTINTC030_SGI_CONTROL	0x680
#define		FTINTC030_PRIORITY_MASK	0x684
#define		FTINTC030_BINARY_POINT	0x688
#define		FTINTC030_INTCTL		0x69C
#define		FTINTC030_CPU0MATCH		0x6A0
#define		FTINTC030_SPICONFIG		0x200
#define 	FTINTC030_ACTIVEBIT		0x600
#define		FTINTC030_ACK			0x68C
#define		FTINTC030_EOI			0x690
#define		FTINTC030_FEATURE       0x700

typedef struct{
	volatile UINT32 spi_src;
	volatile UINT32 spi_en;
	volatile UINT32 spi_clr;
	volatile UINT32 spi_mode;
	volatile UINT32 spi_level;
	volatile UINT32 spi_pending;
	volatile UINT32 dummy[2];
}INTC030_SPI_Reg;

typedef struct{
	volatile UINT32 ppi_src;
	volatile UINT32 ppi_en;
	volatile UINT32 ppi_sgi_clr;
	volatile UINT32 ppi_sgi_mode;
	volatile UINT32 ppi_sgi_level;
	volatile UINT32 ppi_sgi_pending;
	volatile UINT32 ppi_debounce;
}INTC030_PPI_SGI_Reg;

typedef struct{
	volatile INTC030_SPI_Reg spi_reg[2];
	volatile UINT32 dummy[6];
	volatile UINT32 spi0_debounce;
	volatile UINT32 spi1_debounce;
}INTC030_Reg;

struct Interrupt_Vector
{
	void (*handler)(UINT8);
};
