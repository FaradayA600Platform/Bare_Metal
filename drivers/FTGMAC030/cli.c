/**
 * (C) Copyright 2013 Faraday Technology
 * BingYao Luo <bjluo@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "ftgmac030.h"
#include "net/net_core.h"

struct cmd {
	char *name;
	char *usage;
	int(*func) (int argc, char * const argv[]);
};

typedef struct cmd cmd_t;

#define FMT_UINT	"%08X"
#define CMDLEN	50

extern int FTGMAC030_main(int argc, char * const argv[]);

int do_rd32(int argc, char * const  argv[]);
int do_wr32(int argc, char * const  argv[]);
int do_tftpboot(int argc, char * const  argv[]);
int do_ping(int argc, char * const  argv[]);
int enable_caches(int argc, char * const  argv[]);
int disable_caches(int argc, char * const  argv[]);
int do_runapp(int argc, char * const  argv[]);
int ftgmac030_do_help(int argc, char * const argv[], cmd_t * tbl);

cmd_t main_cmd_tbl[] = {
			{"md", "<addr> [num]", do_rd32},
			{"mw", "<addr> <data> [num [inc]]", do_wr32},
			{"gmac","FTGMAC030 commands mode", FTGMAC030_main},
			{"tftpboot", "<filename> [loadaddr] [serverip]", do_tftpboot},
			{"ping", "<ipaddr>", do_ping},
			{"en_cache", "Enable D-cache and I-cache", enable_caches},
			{"dis_cache", "Disable D-cache and I-cache", disable_caches},
			{"go", "<addr>", do_runapp},
			{"?", "print all cmds", 0},
			{0}
			};

/* Usage: md <addr> [num] */
int do_rd32(int argc, char * const  argv[])
{
	uint *pd;
	int i, j;
	uint addr;
	int num;
	int data;

	if ((argc < 2) || (argc > 3))
		return 1;

	addr = strtol(argv[1], 0, 0);

	if (argc == 3)
		num = strtol(argv[2], 0, 0);
	else
		num = 4;

	pd = (uint *) ((uintptr_t) addr);
	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			prints(FMT_UINT ": ", (uintptr_t) pd);

		data = *pd++;
		prints("%08X ", data);

		if (++j == 4) {
			if ((i + 1) < num)
				prints("\n");
			j = 0;
		}
	}

	return (0);
}

/* Usage: mw <addr> <data> [num [inc]] */
int do_wr32(int argc, char * const  argv[])
{
	uint *pd;
	int i, j;
	uint addr;
	int num;
	int data;
	int datainc;

	if ((argc < 3) || (argc > 5))
		return 1;

	addr = strtol(argv[1], 0, 0);
	data = strtol(argv[2], 0, 0);

	num = 1;
	datainc = 0;
	if (argc >= 4) {
		num = strtol(argv[3], 0, 0);

		if (argc == 5)
			datainc = strtol(argv[4], 0, 0);
	}

	pd = (uint *) ((uintptr_t) addr);
	for (i = 0; i < num; i++) {
		*pd++ = data;
		data += datainc;
	}

	pd = (uint *) ((uintptr_t) addr);
	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			prints(FMT_UINT ": ", (uintptr_t) pd);

		data = *pd++;
		prints("%08X ", data);

		if (++j == 4) {
			if ((i + 1) < num)
				prints("\n");
			j = 0;
		}
	}

	return (0);
}

int enable_caches(int argc, char * const  argv[])
{
	ftgmac030_enable_caches();
	return 0;
}

int disable_caches(int argc, char * const  argv[])
{

	ftgmac030_disable_caches();
	return 0;
}

#define CONFIG_MACH_TYPE            758 /* Faraday */
uint32_t load_addr = 0x2000000;

/* tftpboot <filename> [loadaddr] [serverip] */
int do_tftpboot(int argc, char * const  argv[])
{
	void (*kernel_entry)(int zero, int arch, uint params);

	if (argc < 2)
		return 1;

	if (argc == 3)
		load_addr = strtoul(argv[2], NULL, 16);

	if (argc == 4)
		NetServerIP = string_to_ip((const char *) argv[3]);

	/* Check if we had MAC controller */
	if (!eth_initialize(argv[1]))
		return 0;

	if (NetLoop(TFTPGET) < 0) {
		prints("Download error\n");
		return 0;
	}

	disable_caches(0, 0);

	kernel_entry = (void (*)(int, int, uint)) load_addr;

	/* Never return */
	kernel_entry(0, CONFIG_MACH_TYPE, 0);
	return 0;
}

/* app <addr> */
int do_runapp(int argc, char * const  argv[])
{
	uint32_t exec_addr;

	void (*entry)(void);

	if (argc < 2)
		return 1;

	exec_addr = strtoul(argv[1], NULL, 16);

	prints("go 0x%lx\n", exec_addr);

	disable_caches(0, 0);

	entry = (void (*)(void)) exec_addr;

	/* Never return */
	entry();

	return 0;
}
/* ping <ipaddr> */
int do_ping(int argc, char * const argv[])
{
        if (argc < 2)
                return -1;

	/* Check if we had MAC controller */
	if (!eth_initialize(NULL))
		return 0;

	NetPingIP = string_to_ip(argv[1]);
	if (NetPingIP == 0)
		return 1;

	if (NetLoop(PING) < 0) {
		prints("ping failed; host %s is not alive\n", argv[1]);
		return 1;
	}

	prints("host %s is alive\n", argv[1]);

	return 0;
}

int ftgmac030_do_help(int argc, char * const argv[], cmd_t * tbl)
{
	cmd_t *cmdp = &tbl[0];

	if (argc == 1) {	/* display all commands */
		while (cmdp->name != 0) {
			prints("  %-15s %-26s\n\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else if (argc == 2) {	/*Display argv[0] command */
		while (cmdp->name != 0) {
			if (strcmp(argv[0], cmdp->name) == 0)
				prints("  %-15s %-26s\n\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else {
		return 1;
	}

	return 0;
}
static int   argc;
static char *argv[16];
int ftgmac030_cmd_exec(char *line, cmd_t * tbl)
{
	int i;
	char *tok;
	cmd_t *cmd;

	argc = 0;

	tok  = strtok (line, " \r\n\t");
	while(tok) {
		argv[argc++] = tok;
		tok = strtok (NULL, " \r\n\t");
	}

	if (argc == 0)
		return 0;

	if (!strcmp("quit",argv[0]))
		return 1;

	for (i = 0; ; ++i) {
		cmd = &tbl[i];
		if ((cmd->name == NULL) || !strcmp("help", argv[0]) ) {
			ftgmac030_do_help(1, 0, tbl);	/* list all support commands */
			return 0;
		} else if (!strcmp(cmd->name, argv[0]) && cmd->func) {
			if (cmd->func(argc, argv))
				ftgmac030_do_help (2, argv, tbl);

			break;
		}
	}

	return 0;
}

/*
int main(void)
{
	char cmdstr[CMDLEN];

	prints("FTGMAC030 Bare-Metal code\n");

	while (1) {

		prints("\nroot-t:> ");

		scans(cmdstr);

		ftgmac030_cmd_exec(cmdstr, main_cmd_tbl);
	}

	return 0;
}
*/
