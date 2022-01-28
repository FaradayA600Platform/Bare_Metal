
#include "SoFlexible.h"

#ifdef CONFIG_FTTMR010
#include "fttmr010.h"
static fttmr_t * const	TIMER	= (fttmr_t *) TIMER_FTTMR010_PA_BASE;
#endif

#ifdef CONFIG_FTPWMTMR010
#include "ftpwmtmr010.h"
static ftpwmtmr_t * const TIMER	= (ftpwmtmr_t *) TIMER_FTPWMTMR010_PA_BASE;
#endif


#ifdef CONFIG_JIFFIES_TMR_EXTCLK

#define EXTCLOCK 32768  //ycmo: depend on platform
#define TMR_INPUT_CLK  EXTCLOCK 

#else

#define TMR_INPUT_CLK  get_apb_clk()

#endif


#ifdef CONFIG_FTRTC011

#include "ftrtc.h"

#else

#define ftrtc_t void

static UINT32	pre_fttmr_tick=0;
static UINT32	fttmr_overflow_count=0; //not always correct, if it isn't accessed over one overflow period

#endif
//#define	A320_RTC_BASE		0x98600000

//static ftpmu_t * const	A320_PMU	= (ftpmu_t *) A320_PMU_BASE;
//static ftrtc_t * const	A320_RTC	= (ftrtc_t *) A320_RTC_BASE;


/*
 * We use timer to adjust period smaller than 1 second. 
 *
 *       0                    1                    2                    3                    4
 * RTC   |--------------------|--------------------|--------------------|--------------------|-----
 *       |                    |                    |                    |                    |
 *       |    0               |    1               |   0/2              |    1               |   0/2
 * timer -----|--a--------------c--|------d-----------b-|--a--------------c--|------d-----------b-|
 */

#define	IS_ODD(x)	((x) & 1)
#define	IS_EVEN(x)	(!IS_ODD(x))

static void
init_jiffies_timer(ftrtc_t *rtc)
{
    unsigned data;
	/* use timer3 to calculate jiffies */
#ifdef CONFIG_FTRTC011
	TIMER->timer3_load	= EXTCLOCK * 2 - 1;	/* 2 seconds */
#else //no rtc
	TIMER->timer3_load	= 0xffffffff;	/* max */
	TIMER->timer3_counter	= 0xffffffff;	/* max */
#endif

#if defined(CONFIG_FTTMR010)
	TIMER->timer3_match1	= 0;
	TIMER->timer3_match2	= 0;

	/* we don't want timer3 to issue interrupts */
	TIMER->interrupt_mask	= FTTMR_TM3_MATCH1
				| FTTMR_TM3_MATCH2
				| FTTMR_TM3_OVERFLOW;
#   ifdef CONFIG_JIFFIES_TMR_EXTCLK
	TIMER->cr |= FTTMR_TM3_CLOCK;	/* use external clock */
#   endif
	TIMER->cr |= FTTMR_TM3_ENABLE;

#elif defined(CONFIG_FTPWMTMR010)
	TIMER->timer3_compare = 0;

    data = 0;

#   ifdef CONFIG_JIFFIES_TMR_EXTCLK
    data |= FTPWMTMR_CLKSRC;/* use external clock */
#   endif
    data |= FTPWMTMR_UPDATE ; //reset value
    data |= FTPWMTMR_AUTOLOAD;
    data |= FTPWMTMR_START;

	TIMER->timer3_cr = data;

#else
    #error "unsupport timer controller version "
#endif


#ifdef CONFIG_FTRTC011
	/* sync rtc & timer3 as good as we can */
	TIMER->timer3_counter	= IS_ODD(rtc->second) ? EXTCLOCK - 1 : EXTCLOCK * 2 - 1;
#endif
}

void
init_jiffies(void)
{
#ifdef CONFIG_FTRTC011
//	enable_oscl(A320_PMU);
	ftrtc_enable(A320_RTC);
	init_jiffies_timer(RTC);
#else
	pre_fttmr_tick=0;
	fttmr_overflow_count=0; 
	init_jiffies_timer(0);
#endif
}


UINT64
jiffies(void)
{
	UINT64	clocks;
#ifdef CONFIG_FTRTC011
	/* take a snapshot first */
	time_t	second	= RTC->second;
	UINT64	ticks	= EXTCLOCK * 2 - TIMER->timer3_counter;	/* note that we use a count-down timer */
	time_t	minute	= RTC->minute;
	time_t	hour	= RTC->hour;
	time_t	day	= RTC->day;
	time_t	second2	= A320_RTC->second;

	/*
	 * god damn RTC design, be careful
	 */
	if (second != second2) {
		/*
		 * RTC carry-in at the interval between reading registers
		 * redo it!
		 */
		return jiffies();
	}

	/*
	 * calculate clocks based on RTC first
	 */
	clocks = (day * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second) * CLOCKS_PER_SEC;

	/*
	 * adjust the period smaller than 1 second
	 */
	if (IS_EVEN(second)) {
		if (ticks < EXTCLOCK) {	/* case a */
			clocks += ticks * CLOCKS_PER_SEC / EXTCLOCK;
		} else {		/* case b */
			clocks += ticks * CLOCKS_PER_SEC / EXTCLOCK - CLOCKS_PER_SEC * 2;
		}
	} else {			/* case c & d */
		clocks += ticks * CLOCKS_PER_SEC / EXTCLOCK - CLOCKS_PER_SEC;
	}
#else //no CONFIG_FTRTC011
    UINT64  global_ticks;
	UINT32	fttmr_ticks = 0xffffffff - TIMER->timer3_counter;	/* note that we use a count-down timer */

    if (pre_fttmr_tick > fttmr_ticks) {
        fttmr_overflow_count ++;
    }
    global_ticks =  (fttmr_overflow_count << 32) + fttmr_ticks;


	clocks = (global_ticks * CLOCKS_PER_SEC) / TMR_INPUT_CLK ;


    pre_fttmr_tick = fttmr_ticks;

#endif //CONFIG_FTRTC011
	return clocks;
}

#define msec_to_jiffies(msec)   (msec / (1000/CLOCKS_PER_SEC))



void __mdelay(unsigned long msec)
{
    unsigned long long start;
    unsigned tmo;

    start = jiffies();        /* get current timestamp */
    tmo = msec_to_jiffies(msec);   /* convert usecs to ticks */
    while ((jiffies() - start) < tmo) ;           /* loop till time has passed */
}

//ycmo: adaptable for u-boot
void udelay(unsigned long usec)
{
    __mdelay(usec/1000);

}

void jiffies_test(void){

    unsigned long long x;

    init_jiffies();

    printf("jiffies=0x%llx ,%llu\n",x=jiffies(), x);
    printf("jiffies=0x%llx ,%llu\n",x=jiffies(), x);

    printf("mdelay 1s \n",jiffies());
    __mdelay(1000);

    printf("jiffies=0x%llx ,%llu\n",x=jiffies(), x);

    printf("mdelay 5s \n",jiffies());
    __mdelay(1000*5);
    printf("jiffies=0x%llx ,%llu\n",x=jiffies(), x);

    printf("restart init_jiffies \n");
    init_jiffies();
    printf("jiffies=0x%llx ,%llu\n",x=jiffies(), x);
    printf("jiffies=0x%llx ,%llu\n",x=jiffies(), x);

    printf("mdelay 30s \n");
    __mdelay(1000*30);
    printf("jiffies=0x%llx ,%llu\n",x=jiffies(), x);

    return;

}


