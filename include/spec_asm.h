
#define IRQ_MASK 0x4
#define FIQ_MASK 0x24
#define REMAP_REG 0x88

#define SZ_8K			(1 << 13)	
#define SZ_128K		(1 << 17)	
#define SZ_256K		(1 << 18)	
#define SZ_512K		(1 << 19)	
#define SZ_4M			(1 << 22)	
#define SZ_8M			(1 << 23)	

#define T_BIT 0x20
#define F_BIT 0x40
#define I_BIT 0x80 

#define NoINTS (I_BIT | F_BIT)
#define MskINTS NoINTS
#define AllIRQs		0xFF
#define ModeMask		0x1F		
#define SVC32Mode		0x13
#define IRQ32Mode		0x12
#define FIQ32Mode		0x11
#define User32Mode		0x10
#define Sys32Mode		0x1F
#define Abort32Mode		0x17
#define Undef32Mode		0x1B
#define UserStackSize		SZ_512K
#define SVCStackSize		SZ_256K
#define IRQStackSize		SZ_128K
#define UndefStackSize	SZ_8K
#define FIQStackSize		SZ_128K
#define AbortStackSize	SZ_8K
#define HeapSize        SZ_4M


