#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <types.h>
#include "SoFlexible.h"

//#define COL_WIDTH			26				/// each col is 26 character
#define COL_WIDTH			80				/// each col is 86 character
#ifndef CONFIG_ARMV8
#ifdef __thumb/* Define Thumb Semihosting SWI */
#define SemiSWI 0xAB
#else/* Define ARM Semihosting SWI */
#define SemiSWI 0x123456
#endif

#ifndef COMP_GCC
__swi(SemiSWI) void _Write0(unsigned op, const char *string);
__swi(SemiSWI) void _WriteC(unsigned op, const char *c);
#define WriteC(c) _WriteC (0x3,c)
#define Write0(string) _Write0 (0x4,string)

//void Write0(const char *string) { _Write0 (0x4,string); }
//void WriteC(const char *string) { _Write0 (0x3,string); }
#endif
#endif

extern void fLib_Int_Init(void);
extern jmp_code(unsigned int);

#include "main_menu.c"



//extern FILE *my_open(int i, char *ptr);
extern int do_semi(int i, char *ptr);

struct open_data
{
	char *filename;
	int mode;
	int length;
};

struct write_data
{
	int handle;
	char *write_ptr;
	int num;
};

struct close_data
{
	int handle;
};

extern u32 get_version_id();
extern u32 get_ahb_clk();
extern u32 get_apb_clk();
extern u32 get_cpu_clk();

__attribute__((weak)) int main(void)
{
	unsigned int *p;

  	// ycmo: new lib crt will initialize data/bss section again, so some data is cleaned
    // init here again
	fLib_Int_Init();
	
    platform_init();

	printf("Build time: %s %s\n", __DATE__, __TIME__);
	printf("platform: %x\n", get_platform_id());
	printf("ahb: %d MHz\n", get_ahb_clk()/1024/1024);
	printf("apb: %d MHz\n", get_apb_clk()/1024/1024);
	printf("cpu: %d MHz\n", get_cpu_clk()/1024/1024);

#if 0
    printf("Start to set PLL0 clk\n");
    writel(0xc0, 0x13008060);
    printf("set PLL0 boot xor\n");
    writel(0x64230000, 0x13000030);
    printf("set PLL0 disable\n");
    writel(0x100, 0x13000020);
    printf("set PLL0 update\n");
    writel(0x100, 0x13000020);
    printf("set PLL0 update\n");
    writel(0x64230001, 0x13000030);
    printf("set PLL0 nable\n");
    writel(0x100, 0x13000020);
    printf("Check PLL0 speed\n");
#endif
	printf("\n"); 
	ManualTesting(burnin_cmd_value, COL_WIDTH, 0);    /* never return */
}


void Show_Led(int val)
{
	int led_val;
	int i;
	
	led_val = (val&0x1)<<0;
	val >>= 1;
	led_val |= ((val&0x1)<<1);
	val >>= 1;
	led_val |= ((val&0x1)<<4);
	val >>= 1;
	led_val |= ((val&0x1)<<7);
	val >>= 1;
	led_val |= ((val&0x1)<<8);
	val >>= 1;
	led_val |= ((val&0x1)<<11);
	val >>= 1;
	led_val |= ((val&0x1)<<14);
	val >>= 1;
	led_val |= ((val&0x1)<<15);

	//for (i=0; i<300000; ++i)
	{
		writew(led_val, 0x21000000);
	}
	for (i=0; i<3000000; ++i)			// do_delay
	{
		;
	}
}


void LED_Test(void)
{
	int val;
	
	for (val=0; val<0x10; ++val)
	{
		Show_Led(val);
	}
	for (val=0x0; val<0x100; val+=0x10)
	{
		Show_Led(val);
	}
}


void Switch_LCD(void)
{
	char buf[256];
	int fg_lcd=0;
	unsigned int fmpsrg;
	
	printf("Enter FG_LCD:");
	gets(buf);
	printf("\r");
	fg_lcd = atoi(buf);
	fmpsrg = readl(0x92000238);
	fmpsrg = (fmpsrg & 0xcff) | (fg_lcd<<8);
	writel(fmpsrg, 0x92000238);
	if (fg_lcd==2)
	{
		printf("make sure SW10 on & SW12 off to connect ice (new board)\n");
	}
	else if (fg_lcd==0)
	{
		printf("make sure SW10 off & SW12 on to connect ice (new board)\n");
	}
}


void LCD_clock(void)
{
	char buf[128];
	int val;
	
	printf("enternal or external: [0/1], 0 ==> internal, 1==>external ");
	gets(buf);
	printf("\n");
	val = atoi(buf);
	if (val==0)		// internal
	{
		val = readl(0x92000228);
		val = (val & 0xfffffffc);
		writel(val, 0x92000228);
		printf("input clock div: ");
		gets(buf);
		printf("\n");
		val = atoi(buf);
		
		writel(val<<8, 0x94A0010c);
	}
	else		// external
	{
		val = readl(0x92000228);
		val = (val & 0xfffffffc) | 0x2;
		writel(val, 0x92000228);
	}
	// enable lcd, to let lcd pixel clock out
	writel(1, 0x94A00000);
}

void run_uboot(void)
{
//	jmp_code(0x20200000);
}
