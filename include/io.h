#ifndef IO_H
#define IO_H


#define REG32(adr)              (*(volatile UINT32 *)(adr))

#define readl(addr)				(*(volatile unsigned int *)(addr))
#define writel(val, addr)		(*(volatile unsigned int *)(addr) = (val))

#define read32(addr)			(*(volatile unsigned int *)(addr))
#define write32(val, addr)		(*(volatile unsigned int *)(addr) = (val))

#define readw(addr)				(*(volatile unsigned short *)(addr))
#define writew(val, addr)		(*(volatile unsigned short *)(addr) = (val))

#define readb(addr)				(*(volatile unsigned char *)(addr))
#define writeb(val, addr)		(*(volatile unsigned char *)(addr) = (val))



#endif // IO_H
