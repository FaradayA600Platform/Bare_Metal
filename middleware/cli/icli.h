#ifndef _ICLI_H_
#define _ICLI_H_

#include <string.h>

#define	streq(x,y)	(strcmp(x, y) == 0)

#define SCREENSIZE 21
#define HIST_SIZE	10
#define	LINESIZE	128
#define MAXARGS		16
#define MAXSTRLNGTH	256

/*
** range type for parse_range()
*/
#define	ADDR_RANGE	0		/*start_addr-end_addr */
#define	CNT_RANGE	1		/* start_addr/count */
#define	ERROR_RANGE	-1		/* syntax error */
/*
**	argv/argc structure definitions
*/
struct argv_array {
	char	*argv_ptrs[MAXARGS];
	char	der_strings[MAXSTRLNGTH];
	};

struct com_tab_lst 
{
	char *cmdt_name;		/* command name */
	int (*cmdt_routine)();		/* implementing routine */
	int (*cmdt_init_routine)();	/* initialization routine */
	char *cmdt_usage;		/* syntax */
};

typedef char cmdarray[HIST_SIZE][LINESIZE];

#endif /* _ICLI_H_ */

