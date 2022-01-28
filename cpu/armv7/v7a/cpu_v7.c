
__inline void
isb(void)
{
	__asm volatile(
		"isb"
	);
}

__inline void
dsb(void)
{
	__asm volatile(
		"dsb"
	);
}

__inline void
dmb(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"dmb"
	);
}

__inline void
wfi(void)
{
	unsigned int	zero = 0;

	__asm volatile(
		"wfi"
	);
}
