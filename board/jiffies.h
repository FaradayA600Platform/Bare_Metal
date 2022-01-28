#ifndef ___SOFIA_jiffies_h___
#define ___SOFIA_jiffies_h___


#ifdef CLOCKS_PER_SEC
#undef CLOCKS_PER_SEC
#endif


#define CLOCKS_PER_SEC 1000 //msec
void __mdelay(unsigned long msec);

UINT64 jiffies(void);

void init_jiffies(void);


#endif // ___SOFIA_jiffies_h___
