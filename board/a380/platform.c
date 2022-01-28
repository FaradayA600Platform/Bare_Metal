#include "SoFlexible.h"

#define IPMODULE      SCU
#define IPNAME        FTSCU100
#define USER_MEM_SIZE ((CONFIG_MEM_SIZE - SZ_2M) >> 20)
#define VECTOR_RAM (CONFIG_MEM_BASE+CONFIG_MEM_SIZE-SZ_2M)

mmap_t board_mmtbl[] =
{
    /*             VA,              PA,             SZ,   CB, RSVD */
    { CONFIG_MEM_BASE, CONFIG_MEM_BASE,  USER_MEM_SIZE,    3,    0 },   /* Cached + Buffered */
    {      0x00000000,      0x00000000,            16,    0,    0 },    /* smc sram & rom */
    {      0x90000000,      0x90000000,            256,    0,    0 },   /* Uncached + Unbuffered : peripheral io map */
    {      0xC0000000,      0xC0000000,            256,    0,    0 },   /* Uncached + Unbuffered : peripheral io map */
    {      0xD0000000,      0xD0000000,            256,    0,    0 },    
#if defined(CONFIG_HIGH_VECTOR)&&defined(CONFIG_START_W_MMU)
    {      0xFFFF0000, VECTOR_RAM,              1,    3, 0 }, /* Cached + Buffered */
#endif
#if defined(CONFIG_NORMAL_VECTOR)&&defined(CONFIG_START_W_MMU)
    {      0x0, VECTOR_RAM,              1,    3, 0 }, /* Cached + Buffered */
#endif
#if defined(CONFIG_REDIRECT_VECTOR_BASE)
    { CONFIG_REDIRECT_VECTOR_BASE, CONFIG_REDIRECT_VECTOR_BASE,   1,    3, 0 }, /* Cached + Buffered */
#endif
    { 0 },
};

u32 get_platform_id(void)
{
	return 0xA380; // readl(IP_PA_BASE(0));
}

u32 get_ahb_clk(void)
{
	return (198 * 0x100000);
}

u32 get_apb_clk(void)
{
	return (99 * 0x100000);
}


u32 get_cpu_clk(void)
{
	return (200 * 0x100000);
}

// return the clock for the ip
u32 pin_mux_enable(int ip, int arg)
{
	u32	val;

	switch(ip)
	{
		case EXT_INT:
			val = readl(IP_PA_BASE(0)+0x1280);
			val &= (~(BIT0));
			val |= (BIT1);
			writel(val, IP_PA_BASE(0)+0x1280);
			break;
		case IP_SSP0:/* for audio */
			/* switch pin mux */
			val = readl(IP_PA_BASE(0)+0x1128);
//			val &= (~(BIT29|BIT28|BIT27|BIT26|BIT25|BIT24|BIT23|BIT22));
			val &= (~(BIT29|BIT28|BIT27|BIT26|BIT23|BIT22));
			writel(val, IP_PA_BASE(0)+0x1128);
			val = readl(IP_PA_BASE(0)+0x1120);
			val &= (~(BIT23|BIT22));
			val |= (BIT22);
			writel(val, IP_PA_BASE(0)+0x1120);
			/* select clock source */
			val = readl(IP_PA_BASE(0)+0x1114);
			val &= (~(BIT1|BIT0));
			val |= (arg);
			writel(val, IP_PA_BASE(0)+0x1114);
			/* enable clock gating */
			val = readl(IP_PA_BASE(0)+0x1200);
			val |= (BIT12);
			writel(val, IP_PA_BASE(0)+0x1200);
			val = readl(IP_PA_BASE(0)+0x1204);
			val |= (BIT0);
			writel(val, IP_PA_BASE(0)+0x1204);
			break;
		case IP_SSP1:/* for touch panel */
			/* switch pin mux */
			val = readl(IP_PA_BASE(0)+0x112C);
			val &= (~(BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2));
			val |= (BIT8|BIT6|BIT4|BIT2);
			writel(val, IP_PA_BASE(0)+0x112C);
			val = readl(IP_PA_BASE(0)+0x1124);
			val &= (~(BIT13|BIT12));
			val |= (BIT13);
			//val |= (BIT13|BIT12);
			writel(val, IP_PA_BASE(0)+0x1124);
			/* select clock source */
			val = readl(IP_PA_BASE(0)+0x1114);
			val &= (~(BIT3|BIT2));
			val |= (arg);
			writel(val, IP_PA_BASE(0)+0x1114);
			/* enable clock gating */
			val = readl(IP_PA_BASE(0)+0x1200);
			val |= (BIT13);
			writel(val, IP_PA_BASE(0)+0x1200);
			val = readl(IP_PA_BASE(0)+0x1204);
			val |= (BIT1);
			writel(val, IP_PA_BASE(0)+0x1204);
			break;

		default:
			break;
	}

	return 0;
}

// --------------------------------------------------------------------
//      return ==>      clock for the ip
// --------------------------------------------------------------------
u32 get_dev_info(int devId, int arg, u32 *io_base, u32 *irq)
{
    int clock_source=0;
    char buf[128];

    switch(devId)
    {
        case OTG_HOST:
            return 1;
        case OTG_DEV:
            return 0;
        case DMA_NORMAL:
            return 0;
        case DMA_ACP:
            return 1;
        case DMA_PCIE:
            return 2;

        case AUDIO_DEV:
            #undef IPMODULE
            #undef IPNAME
            #define IPMODULE                SSP
            #define IPNAME                  FTSSP010

            printf("please select clock source, 0) internal 199M, 1) internal 13.88M, 2)external:");
            gets(buf);
            clock_source = atoi(buf);
		printf("\n");

            *io_base = IP_PA_BASE(0);
            *irq = -1;                              //*irq = IP_IRQ(0);
            return pin_mux_enable(IP_SSP0, clock_source);

        case SPI_FLASH:
            #undef IPMODULE
            #undef IPNAME
            #define IPMODULE                SSP
            #define IPNAME                  FTSSP010

            *io_base = IP_PA_BASE(0);
            *irq = -1;                              //*irq = IP_IRQ(0);
            return pin_mux_enable(IP_SSP2, 0);              // clock_source is internal

        case EXT_TOUCH_DEV:
            #undef IPMODULE
            #undef IPNAME
            #define IPMODULE                SSP
            #define IPNAME                  FTSSP010

            printf("please select clock source, 0) internal 199M, 1) internal 13.88M, 2)external:");
            gets(buf);
            clock_source = atoi(buf);
		printf("\n");

            *io_base = IP_PA_BASE(1);
            *irq = IP_IRQ(1);

            return pin_mux_enable(IP_SSP1, clock_source);


        default:
            return -1;
    }
}

u32 sysc_enable(int ip, int arg)
{
    u32 val=0;
    u32 clk;


	switch(ip)
	{
		case IP_SERDES:
            //sgmii serdes
   			val = readl(IP_PA_BASE(0)+0x1300);		
			val &= (~(BIT16|BIT17|BIT18|BIT19)); 
   			writel(val, IP_PA_BASE(0)+0x1300);

   			val = readl(IP_PA_BASE(0)+0x674);		
			val |= BIT23; 
   			writel(val, IP_PA_BASE(0)+0x674);

			return 0; 

		case IP_EXTAHB:
			return  get_ahb_clk()*2/3;

		case EXT_INT:

			return  val;

		case F_SLEEP: //turn off all EXTAHB bus request pin
			break;

		case IP_GPIO0:
			break;

		case IP_GPIO1:
			break;

		case IP_UART3:			// arg==0 ==> internal clock, arg==1 ==> external clock
    		return clk;

		case IP_SSP0:			// arg==0 ==> internal clock, arg==1 ==> external clock
    		return clk;

		case IP_SSP1:
			break;
		
		case IP_LCD:
			return get_ahb_clk();

		case IP_SDC0:		// ¥i¥H FG_LCD = 0 ¤]¥i¥H FG_LCD = 1
            break;

		case IP_MCP220:
			return 0;				// nothing need to do for a369
		
		case IP_IDE:
			return 0;
			
		default:
            printf("%s: no such IP number : %d \n", __func__,ip);
			for (;;) ;		// not complete yet
	}

}

#if defined(CONFIG_CPU_CA9) && defined(CONFIG_EXTERNAL_GIC)
#include "gic.h"
#endif
int platform_init(void)
{
    int i;

#if defined(CONFIG_CPU_CA9) && defined(CONFIG_EXTERNAL_GIC)
	#ifdef CONFIG_EXT_GIC_IRQ_N
	i = CONFIG_EXT_GIC_IRQ_N;
	#else
	i = 90;/* default for zynq daughter board */
	#endif
	GIC_DIST_REG32(ICDISERn + (i / 32) * 4) |= (0x00000001<<(i%32)); /* enable irq */
#endif

    return 0;
}



