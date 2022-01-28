
#include "portme_ftuart010.h"

#if defined(CONFIG_BSP_LITE)

unsigned int Do_Delay(unsigned int num)
{
   unsigned int i;
   unsigned int j;
   
   for(i=num;i>0;i--)
   {
      j += i;
      j++;
   }
   
   return j;
}

unsigned int inw(unsigned int *port)
{
   return *port;
}

void outw(unsigned int *port,unsigned int data)
{
   *port=data;
}

#elif defined(CONFIG_BSP_A380)
void disable_dcache(){ CPU_AllCache_Disable(); }

#else

#endif
