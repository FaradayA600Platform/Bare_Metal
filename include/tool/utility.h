#ifndef UTILITY_H
#define UTILITY_H


#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"

//
#define MAX_CMD_LEN	128
#define	CMDLEN			50
#define	MAXARGS			20
//

#define divRoundDown(n,s)   ((n) / (s))
#define divRoundUp(n,s)     ((n+s-1)/(s))

#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))

#define RoundUp(val, units) \
		((((unsigned long)(val) + ((units) - 1)) / (units)) * (units))
#define RoundDown(val, units) \
		(((unsigned long)(val)/(units))*(units))
		

int substring(char **ptr, char *string, char *pattern);
unsigned int atonum(char *val_str);

#define Min(a,b)  (((a) < (b)) ? (a) : (b))
#define Max(a,b)  (((a) > (b)) ? (a) : (b))


struct burnin_cmd
{
	char    *string;					/* command name */	
	void    (*burnin_routine)();		/* implementing routine */
};

void ManualTesting(struct burnin_cmd * cmd, int col_width, int have_back);

int mem_dump(unsigned int addr, int size);

u32 get_dex(void);

#ifdef __cplusplus
}
#endif



#endif
