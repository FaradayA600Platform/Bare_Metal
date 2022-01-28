#ifndef __PORTME_I2C_H
#define __PORTME_I2C_H

#define IRQ                     0
#define FIQ                     1
#define TIMER_CLOCK             APB_CLK
#define IRQ_TYPE_LEVEL_HIGH     LEVEL

#define prints(fmt, args...)    fLib_printf(fmt, ## args)

#endif /* __PORTME_I2C_H */