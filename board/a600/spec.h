/*
 *  linux/include/asm-arm/arch-faraday/platform-a600/spec.h
 *
 *  Faraday Platform Specification  (Automatically generated by "autospec", do not edit)
 *
 *  Copyright (C) 2013 Faraday Corp. (http://www.faraday-tech.com)
 *
 *  Platform Hierarchy :
 *
 *    A600
 *     +- CPU_CA9MP  (0)
 *     +- ERAM_FTEMC030  (0)
 *     +- LCD_FTLCDC210  (0)
 *     +- DMAC_FTDMAC030  (0)
 *     +- DMAC_FTDMAC030  (1)
 *     +- RAM_FTDMAC030  (0)
 *     +- RAM_FTDMAC030  (1)
 *     +- RAM_FTDMAC030  (2)
 *     +- DDRC_FTDDR440  (0)
 *     +- SDC_FTSDC021  (0)
 *     +- SPI_FTSPI020  (0)
 *     +- GPIO_FTGPIO010  (0)
 *     +- IIC_FTIIC010  (0)
 *     +- IIC_FTIIC010  (1)
 *     +- UART_FTUART010  (0)
 *     +- UART_FTUART010  (1)
 *     +- UART_FTUART010  (2)
 *     +- SSP_FTSSP010  (0)
 *     +- SSP_FTSSP010  (1)
 *     +- SSP_FTSSP010  (2)
 *     +- SCU_FTSCU100  (0)
 *     +- TIMER_FTPWMTMR010  (0)
 *     +- WDT_FTWDT011  (0)
 */

#ifndef __A600_PLATFORM_SPECIFICATION__
#define __A600_PLATFORM_SPECIFICATION__

#define PLATFORM_NAME	"Faraday A600"

/*
 * Component counts
 */

/* CPU */
#define CPU_COUNT	1
/* ERAM */
#define ERAM_COUNT	1
#define ERAM_FTEMC030_COUNT	1
/* LCD */
#define LCD_COUNT	1
#define LCD_FTLCDC210_COUNT	1
/* DMAC */
#define DMAC_COUNT	2
#define DMAC_FTDMAC030_COUNT	2
/* RAM */
#define RAM_COUNT	2
#define RAM_FTDMAC030_COUNT	2
/* DDRC */
#define DDRC_COUNT	1
#define DDRC_FTDDR440_COUNT	1
/* SDC */
#define SDC_COUNT	2
#define SDC_FTSDC021_COUNT	2
/* SPI */
#define SPI_COUNT	1
#define SPI_FTSPI020_COUNT	1
/* GPIO */
#define GPIO_COUNT	1
#define GPIO_FTGPIO010_COUNT	1
/* IIC */
#define IIC_COUNT	4
#define IIC_FTIIC010_COUNT	4
/* UART */
#define UART_COUNT	4
#define UART_FTUART010_COUNT	4
/* SSP */
#define SSP_COUNT	3
#define SSP_FTSSP010_COUNT	3
/* SCU */
#define SCU_COUNT	1
#define SCU_FTSCU100_COUNT	1
/* TIMER */
#define TIMER_COUNT	1
#define TIMER_FTPWMTMR010_COUNT	2
/* WDT */
#define WDT_COUNT	1
#define WDT_FTWDT011_COUNT	1

/*
 * Hierarchial Component IDs
 */


/*
 * Number of interrupts, including IRQ and FIQ
 */

#define PLATFORM_IRQ_TOTALCOUNT	108
#define PLATFORM_FIQ_TOTALCOUNT	64

#define PLATFORM_IRQ_BASE	0
#define PLATFORM_FIQ_BASE	64
#define PLATFORM_INTERRUPTS	128

/*
 * IRQ/FIQ trigger level and trigger mode
 */

#define PLATFORM_IRQ_TRIGGER_MODE	0x0000E000
#define PLATFORM_IRQ_TRIGGER_LEVEL	0xFFFFFFFFFFFFFFFF
#define PLATFORM_FIQ_TRIGGER_MODE	0x00000000
#define PLATFORM_FIQ_TRIGGER_LEVEL	0xFFFFFFFFFFFFFFFF

/*
 * Interrupt numbers of Hierarchical Architecture
 */


/*
 * Interrrupt numbers
 */

/* LCD */
#define LCD_FTLCDC210_IRQ_COUNT	1
#define LCD_FTLCDC210_IRQ	10
#define LCD_FTLCDC210_0_IRQ	10

/* DMAC */
#define DMAC_FTDMAC030_IRQ_COUNT	1
#define DMAC_FTDMAC030_IRQ0			1
#define DMAC_FTDMAC030_0_IRQ0		1
#define DMAC_FTDMAC030_IRQ1			2
#define DMAC_FTDMAC030_0_IRQ1		2
#define DMAC_FTDMAC030_IRQ2			3
#define DMAC_FTDMAC030_0_IRQ2		3
#define DMAC_FTDMAC030_1_IRQ0		21

/* SDC */
#define SDC_FTSDC021_IRQ_COUNT	1
#define SDC_FTSDC021_IRQ	25
#define SDC_FTSDC021_0_IRQ	25
#define SDC_FTSDC021_1_IRQ	26

/* SPI */
#define SPI_FTSPI020_IRQ_COUNT	1
#define SPI_FTSPI020_IRQ	51
#define SPI_FTSPI020_0_IRQ	51

/* GPIO */
#define GPIO_FTGPIO010_IRQ_COUNT	1
#define GPIO_FTGPIO010_IRQ	48
#define GPIO_FTGPIO010_0_IRQ	48

/* IIC */
#define IIC_FTIIC010_IRQ_COUNT	4
#define IIC_FTIIC010_IRQ	27
#define IIC_FTIIC010_0_IRQ	27
#define IIC_FTIIC010_1_IRQ	28
#define IIC_FTIIC010_2_IRQ	29
#define IIC_FTIIC010_3_IRQ	30

/* UART */
#define UART_FTUART010_IRQ_COUNT	4
#define UART_FTUART010_IRQ	41
#define UART_FTUART010_0_IRQ	41
#define UART_FTUART010_1_IRQ	42
#define UART_FTUART010_2_IRQ	43
#define UART_FTUART010_3_IRQ	44

/* SSP */
#define SSP_FTSSP010_IRQ_COUNT	3
#define SSP_FTSSP010_IRQ	45
#define SSP_FTSSP010_0_IRQ	45		// SPI
#define SSP_FTSSP010_1_IRQ	46		// SPI
#define SSP_FTSSP010_2_IRQ	47		// I2S

/* SCU */
#define SCU_FTSCU100_IRQ_COUNT	1
#define SCU_FTSCU100_IRQ	0
#define SCU_FTSCU100_0_IRQ	0

/* TIMER */
#define TIMER_FTPWMTMR010_IRQ_COUNT	8
#define TIMER_FTPWMTMR010_IRQ0		32
#define TIMER_FTPWMTMR010_IRQ1		33
#define TIMER_FTPWMTMR010_IRQ2		34
#define TIMER_FTPWMTMR010_IRQ3		35
#define TIMER_FTPWMTMR010_IRQ4		36
#define TIMER_FTPWMTMR010_IRQ5		37
#define TIMER_FTPWMTMR010_IRQ6		38
#define TIMER_FTPWMTMR010_IRQ7		39
#define TIMER_FTPWMTMR010_0_IRQ0	40

#define TIMER_FTPWMTMR010_1_IRQ0	53
#define TIMER_FTPWMTMR010_1_IRQ1	54
#define TIMER_FTPWMTMR010_1_IRQ2	55
#define TIMER_FTPWMTMR010_1_IRQ3	56
#define TIMER_FTPWMTMR010_1_IRQ4	57
#define TIMER_FTPWMTMR010_1_IRQ5	58
#define TIMER_FTPWMTMR010_1_IRQ6	59
#define TIMER_FTPWMTMR010_1_IRQ7	60
#define TIMER_FTPWMTMR010_1_0_IRQ0	61

/* WDT */
#define WDT_FTWDT011_IRQ_COUNT	2
#define WDT_FTWDT011_IRQ		31
#define WDT_FTWDT011_0_IRQ	31
#define WDT_FTWDT011_1_IRQ	52

/* GMAC */
#define GMAC_FTGMAC030_IRQ_COUNT	2
#define GMAC_FTGMAC030_IRQ0	19
#define GMAC_FTGMAC030_0_IRQ0	19
#define GMAC_FTGMAC030_IRQ1	20
#define GMAC_FTGMAC030_0_IRQ1	20

/* SOTERIA */
#define CRYPTO_SOTERIA_IRQ_COUNT	1
#define CRYPTO_SOTERIA_IRQ	5
#define CRYPTO_SOTERIA_0_IRQ0	5

/*
 * Base addresses
 */

/* CPU */
#define CPU_MEM_PA_COUNT	1
#define CPU_MEM_PA_BASE	0x80000000
#define CPU_MEM_PA_LIMIT	0xFFFFFFFF
#define CPU_MEM_PA_SIZE	0x80000000
#define CPU_MEM_0_PA_BASE	0x80000000
#define CPU_MEM_0_PA_LIMIT	0xFFFFFFFF
#define CPU_MEM_0_PA_SIZE	0x80000000

/* ERAM */
#define ERAM_FTEMC030_PA_COUNT	1
#define ERAM_FTEMC030_PA_BASE	0x28000000
#define ERAM_FTEMC030_PA_LIMIT	0x2807FFFF
#define ERAM_FTEMC030_PA_SIZE	0x00080000
#define ERAM_FTEMC030_0_PA_BASE	0x28000000
#define ERAM_FTEMC030_0_PA_LIMIT	0x2807FFFF
#define ERAM_FTEMC030_0_PA_SIZE	0x00080000

/* LCD */
#define LCD_FTLCDC210_PA_COUNT	1
#define LCD_FTLCDC210_PA_BASE	0x2A900000
#define LCD_FTLCDC210_PA_LIMIT	0x2A9FFFFF
#define LCD_FTLCDC210_PA_SIZE	0x00100000
#define LCD_FTLCDC210_0_PA_BASE	0x2A900000
#define LCD_FTLCDC210_0_PA_LIMIT	0x2A9FFFFF
#define LCD_FTLCDC210_0_PA_SIZE	0x00100000

/* DMAC */
#define DMAC_FTDMAC030_PA_COUNT	2
#define DMAC_FTDMAC030_PA_BASE	0x2D100000
#define DMAC_FTDMAC030_PA_LIMIT	0x2D1FFFFF
#define DMAC_FTDMAC030_PA_SIZE	0x00100000
#define DMAC_FTDMAC030_0_PA_BASE	0x2D100000
#define DMAC_FTDMAC030_0_PA_LIMIT	0x2D1FFFFF
#define DMAC_FTDMAC030_0_PA_SIZE	0x00100000
#define DMAC_FTDMAC030_1_PA_BASE	0x2A800000
#define DMAC_FTDMAC030_1_PA_LIMIT	0x2A8FFFFF
#define DMAC_FTDMAC030_1_PA_SIZE	0x00100000

/* RAM */
#define RAM_FTDMAC030_PA_COUNT	2
#define RAM_FTDMAC030_PA_BASE	0x28400000
#define RAM_FTDMAC030_PA_LIMIT	0x284FFFFF
#define RAM_FTDMAC030_PA_SIZE	0x00100000
#define RAM_FTDMAC030_0_PA_BASE	0x28400000
#define RAM_FTDMAC030_0_PA_LIMIT	0x284FFFFF
#define RAM_FTDMAC030_0_PA_SIZE	0x00100000
#define RAM_FTDMAC030_1_PA_BASE	0x28D00000
#define RAM_FTDMAC030_1_PA_LIMIT	0x28DFFFFF
#define RAM_FTDMAC030_1_PA_SIZE	0x00100000

/* DDRC */
#define DDRC_FTDDR440_PA_COUNT	1
#define DDRC_FTDDR440_PA_BASE	0x2A600000
#define DDRC_FTDDR440_PA_LIMIT	0x2A6FFFFF
#define DDRC_FTDDR440_PA_SIZE	0x00100000
#define DDRC_FTDDR440_0_PA_BASE	0x2A600000
#define DDRC_FTDDR440_0_PA_LIMIT	0x2A6FFFFF
#define DDRC_FTDDR440_0_PA_SIZE	0x00100000

/* SDC */
#define SDC_FTSDC021_PA_COUNT	2
#define SDC_FTSDC021_PA_BASE	0x24300000
#define SDC_FTSDC021_PA_LIMIT	0x243FFFFF
#define SDC_FTSDC021_PA_SIZE	0x00100000
#define SDC_FTSDC021_0_PA_BASE	0x24300000
#define SDC_FTSDC021_0_PA_LIMIT	0x243FFFFF
#define SDC_FTSDC021_0_PA_SIZE	0x00100000
#define SDC_FTSDC021_1_PA_BASE	0x24400000
#define SDC_FTSDC021_1_PA_LIMIT	0x244FFFFF
#define SDC_FTSDC021_1_PA_SIZE	0x00100000

/* SPI */
#define SPI_FTSPI020_PA_COUNT	1
#define SPI_FTSPI020_PA_BASE	0x28300000
#define SPI_FTSPI020_PA_LIMIT	0x28300FFF
#define SPI_FTSPI020_PA_SIZE	0x00001000
#define SPI_FTSPI020_0_PA_BASE	0x28300000
#define SPI_FTSPI020_0_PA_LIMIT	0x28300FFF
#define SPI_FTSPI020_0_PA_SIZE	0x00001000

/* GPIO */
#define GPIO_FTGPIO010_PA_COUNT	1
#define GPIO_FTGPIO010_PA_BASE	0x20E00000
#define GPIO_FTGPIO010_PA_LIMIT	0x20EFFFFF
#define GPIO_FTGPIO010_PA_SIZE	0x00100000
#define GPIO_FTGPIO010_0_PA_BASE	0x20E00000
#define GPIO_FTGPIO010_0_PA_LIMIT	0x20EFFFFF
#define GPIO_FTGPIO010_0_PA_SIZE	0x00100000

/* IIC */
#define IIC_FTIIC010_PA_COUNT	4
#define IIC_FTIIC010_PA_BASE	0x20100000
#define IIC_FTIIC010_PA_LIMIT	0x201FFFFF
#define IIC_FTIIC010_PA_SIZE	0x00100000
#define IIC_FTIIC010_0_PA_BASE	0x20100000
#define IIC_FTIIC010_0_PA_LIMIT	0x201FFFFF
#define IIC_FTIIC010_0_PA_SIZE	0x00100000
#define IIC_FTIIC010_1_PA_BASE	0x20200000
#define IIC_FTIIC010_1_PA_LIMIT	0x202FFFFF
#define IIC_FTIIC010_1_PA_SIZE	0x00100000
#define IIC_FTIIC010_2_PA_BASE	0x20300000
#define IIC_FTIIC010_2_PA_LIMIT	0x203FFFFF
#define IIC_FTIIC010_2_PA_SIZE	0x00100000
#define IIC_FTIIC010_3_PA_BASE	0x20400000
#define IIC_FTIIC010_3_PA_LIMIT	0x204FFFFF
#define IIC_FTIIC010_3_PA_SIZE	0x00100000

/* UART */
#define UART_FTUART010_PA_COUNT	4
#define UART_FTUART010_PA_BASE	0x20700000		// from document "A600_block_diagram 20200713.vsdx"
#define UART_FTUART010_PA_LIMIT	0x207FFFFF
#define UART_FTUART010_PA_SIZE	0x00100000
#define UART_FTUART010_0_PA_BASE	0x20700000
#define UART_FTUART010_0_PA_LIMIT	0x207FFFFF
#define UART_FTUART010_0_PA_SIZE	0x00100000
#define UART_FTUART010_1_PA_BASE	0x20800000
#define UART_FTUART010_1_PA_LIMIT	0x208FFFFF
#define UART_FTUART010_1_PA_SIZE	0x00100000
#define UART_FTUART010_2_PA_BASE	0x20900000
#define UART_FTUART010_2_PA_LIMIT	0x209FFFFF
#define UART_FTUART010_2_PA_SIZE	0x00100000
#define UART_FTUART010_3_PA_BASE	0x20A00000
#define UART_FTUART010_3_PA_LIMIT	0x20AFFFFF
#define UART_FTUART010_3_PA_SIZE	0x00100000

/* SSP */
#define SSP_FTSSP010_PA_COUNT	3
#define SSP_FTSSP010_PA_BASE	0x20B00000
#define SSP_FTSSP010_PA_LIMIT	0x20BFFFFF
#define SSP_FTSSP010_PA_SIZE	0x00100000
#define SSP_FTSSP010_0_PA_BASE	0x20B00000
#define SSP_FTSSP010_0_PA_LIMIT	0x20BFFFFF
#define SSP_FTSSP010_0_PA_SIZE	0x00100000
#define SSP_FTSSP010_1_PA_BASE	0x20C00000
#define SSP_FTSSP010_1_PA_LIMIT	0x20CFFFFF
#define SSP_FTSSP010_1_PA_SIZE	0x00100000
#define SSP_FTSSP010_2_PA_BASE	0x20D00000
#define SSP_FTSSP010_2_PA_LIMIT	0x20DFFFFF
#define SSP_FTSSP010_2_PA_SIZE	0x00100000

/* SCU */
#define SCU_FTSCU100_PA_COUNT	1
#define SCU_FTSCU100_PA_BASE	0x2D000000
#define SCU_FTSCU100_PA_LIMIT	0x2D0FFFFF
#define SCU_FTSCU100_PA_SIZE	0x00100000
#define SCU_FTSCU100_0_PA_BASE	0x2D000000
#define SCU_FTSCU100_0_PA_LIMIT	0x2D0FFFFF
#define SCU_FTSCU100_0_PA_SIZE	0x00100000

/* TIMER */
#define TIMER_FTPWMTMR010_PA_COUNT		2
#define TIMER_FTPWMTMR010_PA_BASE		0x20600000
#define TIMER_FTPWMTMR010_PA_LIMIT		0x206FFFFF
#define TIMER_FTPWMTMR010_PA_SIZE		0x00100000
#define TIMER_FTPWMTMR010_0_PA_BASE		0x20600000
#define TIMER_FTPWMTMR010_0_PA_LIMIT	0x206FFFFF
#define TIMER_FTPWMTMR010_0_PA_SIZE		0x00100000
#define TIMER_FTPWMTMR010_1_PA_BASE		0x2D600000
#define TIMER_FTPWMTMR010_1_PA_LIMIT	0x2D6FFFFF
#define TIMER_FTPWMTMR010_1_PA_SIZE		0x00100000

/* WDT */
#define WDT_FTWDT011_PA_COUNT	2
#define WDT_FTWDT011_PA_BASE		0x20500000
#define WDT_FTWDT011_PA_LIMIT	0x205FFFFF
#define WDT_FTWDT011_PA_SIZE	0x00100000
#define WDT_FTWDT011_0_PA_BASE	0x20500000
#define WDT_FTWDT011_0_PA_LIMIT	0x205FFFFF
#define WDT_FTWDT011_0_PA_SIZE	0x00100000
#define WDT_FTWDT011_1_PA_BASE	0x2D500000
#define WDT_FTWDT011_1_PA_LIMIT	0x2D5FFFFF
#define WDT_FTWDT011_1_PA_SIZE	0x00100000

/* GMAC */
#define GMAC_FTGMAC030_PA_COUNT	2
#define GMAC_FTGMAC030_PA_BASE  0x2AA00000
#define GMAC_FTGMAC030_PA_LIMIT	0x2AAFFFFF
#define GMAC_FTGMAC030_PA_SIZE	0x00100000
#define GMAC_FTGMAC030_0_PA_BASE  0x2AA00000
#define GMAC_FTGMAC030_0_PA_LIMIT	0x2AAFFFFF
#define GMAC_FTGMAC030_0_PA_SIZE	0x00100000
#define GMAC_FTGMAC030_1_PA_BASE  0x2AB00000
#define GMAC_FTGMAC030_1_PA_LIMIT	0x2ABFFFFF
#define GMAC_FTGMAC030_1_PA_SIZE	0x00100000

/* SOTERIA */
#define CRYPTO_SOTERIA_PA_COUNT	1
#define CRYPTO_SOTERIA_PA_BASE	CRYPTO_SOTERIA_0_PA_BASE
#define CRYPTO_SOTERIA_PA_LIMIT	CRYPTO_SOTERIA_0_PA_LIMIT
#define CRYPTO_SOTERIA_PA_SIZE	CRYPTO_SOTERIA_0_PA_SIZE
#define CRYPTO_SOTERIA_0_PA_BASE	0x28C00000
#define CRYPTO_SOTERIA_0_PA_LIMIT	0x28CFFFFF
#define CRYPTO_SOTERIA_0_PA_SIZE	0x00100000
/* GIC400 */
#define GICD_BASE 0x28601000
#define GICR_BASE 0x28602000
#define CPU_PERIPHERAL_BASE 0x28600000

/*
 * C definitions
 */
#ifndef __ASSEMBLER__

//SoFia: #include <linux/linkage.h>

//SoFia: typedef asmlinkage unsigned interrupt_dispatch_function(unsigned);

/*
 * Interrrupt numbers
 */

/* LCD */
extern const unsigned char LCD_FTLCDC210_irq[LCD_FTLCDC210_COUNT];
/* DMAC */
extern const unsigned char DMAC_FTDMAC030_irq[DMAC_FTDMAC030_COUNT][DMAC_FTDMAC030_IRQ_COUNT];
/* SDC */
extern const unsigned char SDC_FTSDC021_irq[SDC_FTSDC021_COUNT];
/* SPI */
extern const unsigned char SPI_FTSPI020_irq[SPI_FTSPI020_COUNT];
/* GPIO */
extern const unsigned char GPIO_FTGPIO010_irq[GPIO_FTGPIO010_COUNT];
/* IIC */
extern const unsigned char IIC_FTIIC010_irq[IIC_FTIIC010_COUNT];
/* UART */
extern const unsigned char UART_FTUART010_irq[UART_FTUART010_COUNT];
/* SSP */
extern const unsigned char SSP_FTSSP010_irq[SSP_FTSSP010_COUNT];
/* SCU */
extern const unsigned char SCU_FTSCU100_irq[SCU_FTSCU100_COUNT];
/* TIMER */
extern const unsigned char TIMER_FTPWMTMR010_irq[TIMER_FTPWMTMR010_COUNT][TIMER_FTPWMTMR010_IRQ_COUNT];
/* WDT */
extern const unsigned char WDT_FTWDT011_irq[WDT_FTWDT011_COUNT];

/*
 * Base addresses
 */

/* CPU */
extern const unsigned CPU_MEM_pa_base[CPU_MEM_PA_COUNT];
extern const unsigned CPU_MEM_pa_limit[CPU_MEM_PA_COUNT];
extern const unsigned CPU_MEM_pa_size[CPU_MEM_PA_COUNT];
/* ERAM */
extern const unsigned ERAM_FTEMC030_pa_base[ERAM_FTEMC030_COUNT];
extern const unsigned ERAM_FTEMC030_pa_limit[ERAM_FTEMC030_COUNT];
extern const unsigned ERAM_FTEMC030_pa_size[ERAM_FTEMC030_COUNT];
/* LCD */
extern const unsigned LCD_FTLCDC210_pa_base[LCD_FTLCDC210_COUNT];
extern const unsigned LCD_FTLCDC210_pa_limit[LCD_FTLCDC210_COUNT];
extern const unsigned LCD_FTLCDC210_pa_size[LCD_FTLCDC210_COUNT];
/* DMAC */
extern const unsigned DMAC_FTDMAC030_pa_base[DMAC_FTDMAC030_COUNT];
extern const unsigned DMAC_FTDMAC030_pa_limit[DMAC_FTDMAC030_COUNT];
extern const unsigned DMAC_FTDMAC030_pa_size[DMAC_FTDMAC030_COUNT];
/* RAM */
extern const unsigned RAM_FTDMAC030_pa_base[RAM_FTDMAC030_COUNT];
extern const unsigned RAM_FTDMAC030_pa_limit[RAM_FTDMAC030_COUNT];
extern const unsigned RAM_FTDMAC030_pa_size[RAM_FTDMAC030_COUNT];
/* DDRC */
extern const unsigned DDRC_FTDDR440_pa_base[DDRC_FTDDR440_COUNT];
extern const unsigned DDRC_FTDDR440_pa_limit[DDRC_FTDDR440_COUNT];
extern const unsigned DDRC_FTDDR440_pa_size[DDRC_FTDDR440_COUNT];
/* SDC */
extern const unsigned SDC_FTSDC021_pa_base[SDC_FTSDC021_COUNT];
extern const unsigned SDC_FTSDC021_pa_limit[SDC_FTSDC021_COUNT];
extern const unsigned SDC_FTSDC021_pa_size[SDC_FTSDC021_COUNT];
/* SPI */
extern const unsigned SPI_FTSPI020_pa_base[SPI_FTSPI020_COUNT];
extern const unsigned SPI_FTSPI020_pa_limit[SPI_FTSPI020_COUNT];
extern const unsigned SPI_FTSPI020_pa_size[SPI_FTSPI020_COUNT];
/* GPIO */
extern const unsigned GPIO_FTGPIO010_pa_base[GPIO_FTGPIO010_COUNT];
extern const unsigned GPIO_FTGPIO010_pa_limit[GPIO_FTGPIO010_COUNT];
extern const unsigned GPIO_FTGPIO010_pa_size[GPIO_FTGPIO010_COUNT];
/* IIC */
extern const unsigned IIC_FTIIC010_pa_base[IIC_FTIIC010_COUNT];
extern const unsigned IIC_FTIIC010_pa_limit[IIC_FTIIC010_COUNT];
extern const unsigned IIC_FTIIC010_pa_size[IIC_FTIIC010_COUNT];
/* UART */
extern const unsigned UART_FTUART010_pa_base[UART_FTUART010_COUNT];
extern const unsigned UART_FTUART010_pa_limit[UART_FTUART010_COUNT];
extern const unsigned UART_FTUART010_pa_size[UART_FTUART010_COUNT];
/* SSP */
extern const unsigned SSP_FTSSP010_pa_base[SSP_FTSSP010_COUNT];
extern const unsigned SSP_FTSSP010_pa_limit[SSP_FTSSP010_COUNT];
extern const unsigned SSP_FTSSP010_pa_size[SSP_FTSSP010_COUNT];
/* SCU */
extern const unsigned SCU_FTSCU100_pa_base[SCU_FTSCU100_COUNT];
extern const unsigned SCU_FTSCU100_pa_limit[SCU_FTSCU100_COUNT];
extern const unsigned SCU_FTSCU100_pa_size[SCU_FTSCU100_COUNT];
/* TIMER */
extern const unsigned TIMER_FTPWMTMR010_pa_base[TIMER_FTPWMTMR010_COUNT];
extern const unsigned TIMER_FTPWMTMR010_pa_limit[TIMER_FTPWMTMR010_COUNT];
extern const unsigned TIMER_FTPWMTMR010_pa_size[TIMER_FTPWMTMR010_COUNT];
/* WDT */
extern const unsigned WDT_FTWDT011_pa_base[WDT_FTWDT011_COUNT];
extern const unsigned WDT_FTWDT011_pa_limit[WDT_FTWDT011_COUNT];
extern const unsigned WDT_FTWDT011_pa_size[WDT_FTWDT011_COUNT];

/* IO mapping */
#define PLATFORM_IO_DESC_NR 0
struct map_desc;
//SoFia: extern const struct map_desc platform_io_desc[PLATFORM_IO_DESC_NR];

#endif /* ifndef __ASSEMBLER__ */

#endif /* __A380_PLATFORM_SPECIFICATION__ */

