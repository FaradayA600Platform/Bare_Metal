#include "SoFlexible.h"
#include "portme_ftlcdc210.h"

#define CONFIG_BSP_AARCH64 1

#if defined(CONFIG_BSP_LITE)

#elif defined(CONFIG_BSP_A380)
void disable_dcache(){ CPU_AllCache_Disable(); }

#elif defined(CONFIG_BSP_AARCH64)

char ftldcd210_get_char()
{
	char ch;

	ch = fLib_getchar();

	return ch;
}
#if 0
int getchar(void) 
{ 
    return ftldcd210_get_char(); 
}
#endif
void UnmaskIRQ(unsigned int IRQ)
{
}

void SetIRQmode(unsigned int IRQ,unsigned int edge)
{
}

void EnableIRQ(void)
{
}

void DisableIRQ(void)
{
}

#else

#endif
