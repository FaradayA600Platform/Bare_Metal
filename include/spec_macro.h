
#ifndef _spec_macro_H_
#define _spec_macro_H_

#define ____glue(name,fn)	name##fn
#define __glue(name,fn)		____glue(name,fn)

/*
 * Macros for retrieving IP related information
 */
#define IP_IDENTIFIER  __glue(__glue(IPMODULE,_),__glue(IPNAME,_))

#define IP_COUNT       __glue(IP_IDENTIFIER,COUNT)

#define IP_IRQ_COUNT   __glue(IP_IDENTIFIER,IRQ_COUNT)
#define IP_IRQ(n)      __glue(__glue(IP_IDENTIFIER,n),_IRQ)
#define IP_irq         __glue(IP_IDENTIFIER,irq)

#define IP_PA_COUNT    __glue(IP_IDENTIFIER,PA_COUNT)
#define IP_PA_BASE(n)  __glue(__glue(IP_IDENTIFIER,n),_PA_BASE)
#define IP_PA_LIMIT(n) __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT)
#define IP_PA_SIZE(n)  __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE)
#define IP_pa_base     __glue(IP_IDENTIFIER,pa_base)
#define IP_pa_limit    __glue(IP_IDENTIFIER,pa_limit)
#define IP_pa_size     __glue(IP_IDENTIFIER,pa_size)

#define IP_VA_COUNT    __glue(IP_IDENTIFIER,VA_COUNT)
#define IP_VA_BASE(n)  __glue(__glue(IP_IDENTIFIER,n),_VA_BASE)
#define IP_VA_LIMIT(n) __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT)
#define IP_VA_SIZE(n)  __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE)
#define IP_va_base     __glue(IP_IDENTIFIER,va_base)
#define IP_va_limit    __glue(IP_IDENTIFIER,va_limit)
#define IP_va_size     __glue(IP_IDENTIFIER,va_size)

/*
 * Facility macros
 */
/* IRQ0~7 */
#define IP_IRQ0(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ0)
#define IP_IRQ1(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ1)
#define IP_IRQ2(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ2)
#define IP_IRQ3(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ3)
#define IP_IRQ4(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ4)
#define IP_IRQ5(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ5)
#define IP_IRQ6(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ6)
#define IP_IRQ7(n)     __glue(__glue(IP_IDENTIFIER,n),_IRQ7)

/* PA_BASE0~7 */
#define IP_PA_BASE0(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE0)
#define IP_PA_BASE1(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE1)
#define IP_PA_BASE2(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE2)
#define IP_PA_BASE3(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE3)
#define IP_PA_BASE4(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE4)
#define IP_PA_BASE5(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE5)
#define IP_PA_BASE6(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE6)
#define IP_PA_BASE7(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_BASE7)

/* PA_LIMIT0~7 */
#define IP_PA_LIMIT0(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT0)
#define IP_PA_LIMIT1(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT1)
#define IP_PA_LIMIT2(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT2)
#define IP_PA_LIMIT3(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT3)
#define IP_PA_LIMIT4(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT4)
#define IP_PA_LIMIT5(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT5)
#define IP_PA_LIMIT6(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT6)
#define IP_PA_LIMIT7(n)    __glue(__glue(IP_IDENTIFIER,n),_PA_LIMIT7)

/* PA_SIZE0~7 */
#define IP_PA_SIZE0(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE0)
#define IP_PA_SIZE1(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE1)
#define IP_PA_SIZE2(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE2)
#define IP_PA_SIZE3(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE3)
#define IP_PA_SIZE4(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE4)
#define IP_PA_SIZE5(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE5)
#define IP_PA_SIZE6(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE6)
#define IP_PA_SIZE7(n)     __glue(__glue(IP_IDENTIFIER,n),_PA_SIZE7)

/* VA_BASE0~7 */
#define IP_VA_BASE0(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE0)
#define IP_VA_BASE1(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE1)
#define IP_VA_BASE2(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE2)
#define IP_VA_BASE3(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE3)
#define IP_VA_BASE4(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE4)
#define IP_VA_BASE5(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE5)
#define IP_VA_BASE6(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE6)
#define IP_VA_BASE7(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_BASE7)

/* VA_LIMIT0~7 */
#define IP_VA_LIMIT0(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT0)
#define IP_VA_LIMIT1(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT1)
#define IP_VA_LIMIT2(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT2)
#define IP_VA_LIMIT3(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT3)
#define IP_VA_LIMIT4(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT4)
#define IP_VA_LIMIT5(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT5)
#define IP_VA_LIMIT6(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT6)
#define IP_VA_LIMIT7(n)    __glue(__glue(IP_IDENTIFIER,n),_VA_LIMIT7)

/* VA_SIZE0~7 */
#define IP_VA_SIZE0(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE0)
#define IP_VA_SIZE1(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE1)
#define IP_VA_SIZE2(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE2)
#define IP_VA_SIZE3(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE3)
#define IP_VA_SIZE4(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE4)
#define IP_VA_SIZE5(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE5)
#define IP_VA_SIZE6(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE6)
#define IP_VA_SIZE7(n)     __glue(__glue(IP_IDENTIFIER,n),_VA_SIZE7)

#endif

