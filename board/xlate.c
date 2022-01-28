#include "SoFlexible.h"

void enable_interrupts()
{
	//__enable_irq();
}

void disable_interrupts()
{
	//__disable_irq();
}

int irq_install_handler(unsigned int irq, void *handler, void *arg)
{
	fLib_ConnectIRQ(irq, handler, arg);
}

int irq_set_type(unsigned int irq, unsigned int type)
{
	fLib_SetIRQmode(irq, type);
}

int irq_set_enable(unsigned int irq)
{
	fLib_EnableIRQ(irq);
}

int irq_set_disable(unsigned int irq)
{
	fLib_DisableIRQ(irq);
}


unsigned int get_timer(unsigned int timer_n)
{
}