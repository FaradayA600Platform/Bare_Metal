#include "SoFlexible.h"
#include "mmu.h"

#define IPMODULE      SCU
#define IPNAME        FTSCU100
#define USER_MEM_SIZE ((CONFIG_MEM_SIZE - SZ_2M) >> 20)
#define VECTOR_RAM (CONFIG_MEM_BASE+CONFIG_MEM_SIZE-SZ_2M)

mm_region board_mmtbl[] =
{
    /*             virt,              phys,             size,   attrs */
    { CONFIG_MEM_BASE, CONFIG_MEM_BASE,  USER_MEM_SIZE,     PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NS},   /* Cached + Buffered */
	{      0x00000000,       0x00000000,       0x10000000, PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE}, /* Uncached + Unbuffered : rom */
	{      0x10000000,       0x10000000,       0x10000000, PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE}, /* Uncached + Unbuffered : xip */
	{      0x20000000,       0x20000000,       0x04000000, PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE}, /* Uncached + Unbuffered : apb */
	{      0x24000000,       0x24000000,       0x04000000, PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE}, /* Uncached + Unbuffered : ahb */
	{      0x28000000,       0x28000000,       0x08000000, PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE}, /* Uncached + Unbuffered : axi */
	{      0x38000000,       0x38000000,       0x08000000, PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE}, /* Uncached + Unbuffered : hyper bus */
	{      0x40000000,       0x40000000,       0x40000000, PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE}, /* Uncached + Unbuffered : pcie */
    { 0 },
};

u32 get_platform_id(void)
{
	return 0xA600; 
}

u32 get_ahb_clk(void)
{
	return (200 * 0x100000);
}

u32 get_apb_clk(void)
{
	return (100 * 0x100000);
}

u32 get_cpu_clk(void)
{
	return (1500 * 0x100000);
}

u32 pin_mux_enable(int ip, int arg)
{
	return 0;
}

u32 get_dev_info(int devId, int arg, u32 *io_base, u32 *irq)
{
	return 0;
}

u32 sysc_enable(int ip, int arg)
{
	return 0;
}

int platform_init(void)
{
    return 0;
}
