#include <stdio.h>
#include <types.h>
#include "icli.h"
#include "porting.h"

char    exit_notify=0;
//static char         *helpstr[] = {0};
struct com_tab_lst  *decode_cmd(struct com_tab_lst *cmd_table, char *name);/* command lookup rt. */
static char         linebuf[HIST_SIZE][LINESIZE]; /* command history buf. */
static int          cl;				/* current command line */
int                 default_base;

int tokenize(char *cmdline, struct argv_array *argv)
{
	int cnt,argc;
	char c,*strptr;

	argv->argv_ptrs[0] = argv->der_strings;
	cnt = argc = 0;

	while(*cmdline==0x20)
	   cmdline++;
	while ( (c = *cmdline)!=0 )
	{ 
	    strptr = argv->argv_ptrs[argc];
	    if( *cmdline == '"' )
		{ 
		    cmdline++;
		    while (*cmdline && (*cmdline != '"') && (cnt < MAXSTRLNGTH))
		    { 
		        *strptr++ = *cmdline++;
		        cnt++; 
		    }
		    if (*cmdline ==  '"' )
		        cmdline++;
		}
        else 
		{ 
		    while(((c = *cmdline) != ' ') && ( c != '(' ) && (c != ')')
				&& (c != ',') && ( c != 0) && (c != '\t') )
		    { 
		        *strptr++ = *cmdline++;
		        cnt++; 
		    }
		    if ( c != 0 )
		        cmdline++;
		}
	    
	    *strptr++ = 0;
	    while ((*cmdline==0x20) || *cmdline == ',' || *cmdline == '\t' || *cmdline == '(' || *cmdline == ')' )
		    cmdline++;
		    
        argc++;
        
        if (argc >= MAXARGS)     /* evelyn, 940728 */
	        return(--argc);
	        
	    argv->argv_ptrs[argc] = strptr;
	}
	return(argc);
}

extern void hist_init(void);
extern void s_b_init(void);
extern void get_line(char *sptr);
extern void usage(struct com_tab_lst *cmdt);

void cli(struct com_tab_lst *cmd_table, char *prompt)
{
	register struct com_tab_lst *cmdt;
	int     argc,i;
	//int     *switches;
	struct argv_array argv;
	int     command_count;

    hist_init();
    s_b_init();
    printf("\n");
	while(1)
	{
		printf(prompt);
		command_count = 1;
		i = (cl)%HIST_SIZE;
		get_line(linebuf[i]);
		argc = tokenize(linebuf[i], &argv);
		if (argc == 0)
			continue;
		else
			cl++;
			
		cmdt = decode_cmd(cmd_table, argv.argv_ptrs[0]);
		if (!cmdt)
		{
			printf("%s: Command not found.\n", argv.argv_ptrs[0]);
			continue;
		}
		while ( command_count-- > 0)
		{
			if (!(*cmdt->cmdt_routine)(argc, argv.argv_ptrs, cmd_table))
			{
				command_count = 0;
				usage(cmdt);
			}
		}
		if(exit_notify)
		{
		    exit_notify=0;
		    return;
		}
	}
	return;
}


int execute_cmd_str(char *str, struct com_tab_lst *cmd_table)
{
	register struct com_tab_lst *cmdt;
	int argc;
	struct argv_array argv;
	argc = tokenize(str, &argv);
	if (argc == 0)
	  return 0;
	  
	cmdt = decode_cmd(cmd_table, argv.argv_ptrs[0]);
	if (!cmdt)
	{ 
	    printf("%s: Command not found.\n", argv.argv_ptrs[0]);
	    return  0; 
	}
	if (!(*cmdt->cmdt_routine)(argc, argv.argv_ptrs, cmd_table))
	       usage(cmdt);
	return 0;
}


void get_line(char *sptr)
{
	gets(sptr);
	printf("\r");
}


void hist_init()
{
	cl = 0;
	
}


struct com_tab_lst *decode_cmd(struct com_tab_lst *cmd_table, char *name)
{
	register struct com_tab_lst *cmdt;

	for (cmdt = cmd_table; cmdt->cmdt_name; cmdt++)
	  if(streq(cmdt->cmdt_name,name))
			return cmdt;
	return((struct com_tab_lst *)0);
}


void usage(struct com_tab_lst *cmdt)
{
	printf("The Usage: %s\n", cmdt->cmdt_usage);
}


int help(int argc, char **argv, struct com_tab_lst *cmd_table)
{
	register struct com_tab_lst *cmdt;
	int lnum = 0;

	if (argc > 1) 
	{
		while (--argc > 0) 
		{
			argv++;
			cmdt = decode_cmd(cmd_table, *argv);
			if (cmdt)
				printf("%s\n", cmdt->cmdt_usage);
			else
				printf("no command: %s\n", *argv);
		}
		return 0;
	}
	printf("Command list:\n");
	for (cmdt = cmd_table; cmdt->cmdt_name; cmdt++)
    { 
        if(lnum++ == SCREENSIZE)
			lnum = 0;
		  printf("\t%s\n", cmdt->cmdt_usage); 
    }
	return 0;
}

void s_b_init()
{
	default_base = 16;
}

extern void init_cmd_lst_tab(void);
#define PROMPT "\rCLI>"
void CLI_Test_Main()
{
	extern struct com_tab_lst   work_cmd_table[];
	init_cmd_lst_tab();
	cli(work_cmd_table, PROMPT);
}
