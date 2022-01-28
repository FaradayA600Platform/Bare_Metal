#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "sizes.h"






#ifdef CONFIG_MMU

#define PTB_LOCATION (CONFIG_MEM_BASE+CONFIG_MEM_SIZE-SZ_1M)

#endif





// --------------------------------------------------------------------
//		IP id (for pin_mux)
// --------------------------------------------------------------------
#define IP_SSP0                     1
#define IP_SSP1                     2
#define IP_LCD                      3
#define IP_SDC0                     4
#define IP_SDC1                     5
#define IP_MCP100                   6
#define IP_MCP220                   7
#define IP_IDE                      8
#define IP_UART3                    9
#define IP_GPIO1                    10
#define IP_GPIO0                    11
#define F_SLEEP                     12
#define IP_EXTAHB                   13
#define EXT_INT                     14
#define IP_SERDES                   15

#define IP_SSP2                      1000


/*  -------------------------------------------------------------------------------
 *   Clock rate
 *  ------------------------------------------------------------------------------- 
 */

//ycmo : define in platform_xxx.c
#define AHB_CLK					get_ahb_clk()
#define APB_CLK					get_apb_clk()



// --------------------------------------------------------------------
//		platform api
// --------------------------------------------------------------------
u32 get_platform_id();
u32 get_ahb_clk();
u32 get_apb_clk();
u32 get_cpu_clk();
u32 get_dev_info(int devId, int arg, u32 *io_base, u32 *irq);
u32 pin_mux_enable(int ip, int arg);
u32 sysc_enable(int ip, int arg);
u32 dma_reqack_num(u32 base_addr);



#define AUDIO_DEV					100			// get audio codec's ssp io base and enable pinmux
#define EXT_TOUCH_DEV				101			// get external touch's ssp io base and enable pinmux
#define TVEncode_DEV				102
#define SPI_FLASH					103			// get spi flash's ssp io base and enable pinmux
#define NOR_FLASH_DEV				104
#define GEM_SERDES                  105
#define OTG_HOST                    106
#define OTG_DEV                     107
#define DMA_NORMAL                  108
#define DMA_ACP                     109
#define DMA_PCIE                    110

#include "platform.h"


#endif
