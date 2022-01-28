#include "portme_ftdmac030.h"

#if defined(CONFIG_BSP_LITE)

#elif defined(CONFIG_BSP_A380)
void disable_dcache(){ CPU_AllCache_Disable(); }

#else

#endif
