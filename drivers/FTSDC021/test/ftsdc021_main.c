/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_main.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2009/12//	Mike Yeh	 Original FTSDC020 code
 * 2010/4/2	BingJiun-Luo	 FTSDC021 code	
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021_test.h"

#define ISHEX(c) ((((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || \
		  ((c >= 'A') && (c < 'F'))) ? 1 : 0)

/* Command functions */
int32_t SDC_mrd32(int32_t argc, char *const argv[]);
int32_t SDC_mwr32(int32_t argc, char *const argv[]);
int32_t SDC_debug_print(int32_t argc, char *const argv[]);

static cmd_t ftsdc021CmdTbl[] = {
	{"mrd32", "<addr> [num]", SDC_mrd32},
	{"mwr32", "<addr> <data> [num [inc]]", SDC_mwr32},
	{"burnIn", "<fix|auto> [mins] [tr] [spd] [blk cnt type] [ran type]",
	 SDC_burnin_test},
	{"abort", "<blkcnt>", SDC_abort_test},
	{"sdio", "", SDC_sdio_test},
	{"cprm", "<0|1>", SDC_cprm_test},
#ifdef FTSDC021_EDMA
	{"trans", "<pio|sdma|adma|edma> [line|bound] [0|1|2|3] [nop] [intr]",
	 SDC_settrans_cmd},
#else
	{"trans", "<pio|sdma|adma> [line|bound] [0|1|2|3] [nop] [intr]",
	 SDC_settrans_cmd},
#endif
	{"setflow", "<autoCmd> <async|sync> <Erasing>", SDC_setflow_cmd},
	{"inf", "<0-2>", SDC_setInfinite_mode},
	{"speed", "<ds|hs|sdr50|sdr104|ddr50> [clock]", SDC_switch_cmd},
	{"clk", "<MHz>", SDC_set_clock},
	{"bclk", "<MHz>", SDC_set_base_clock},
	{"bw", "<1|4|8>", SDC_bus_width_cmd},
	{"csd", "<rd|wr> [offset] [value]\r\n"
		"                  enable CQE:   csd wr 15 1\r\n"
		"                  enable HPI:   csd wr 161 1\r\n"
		"                  enable RST_n: csd wr 162 1\r\n"
		"                  enable BKOPS: csd wr 163 1",
	 SDC_rw_ext_csd_cmd},
#ifdef EMBEDDED_MMC
#ifdef FTSDC021_MMC_CQE
	{"cqe_en", "[0|1]", SDC_CQE_enable},
#endif
	{"packed_test", "", SDC_packed_test},
	{"reliable_test", "", SDC_reliable_write_test},
	{"context_test", "", SDC_context_test},
	{"rpmb", "<setkey|read|write> <addr> <blkcnt>", SDC_rpmb_cmd},
	{"set_time", "<1|2|3> <time>", SDC_set_time},
	{"power", "<off|sleep> [short|long]", SDC_power_off},
	{"discard", "<addr> <blkcnt>", SDC_discard_cmd},
	{"bootSz", "<size(unit:4MB)>", SDC_boot_size_cmd},
	{"bootPart", "<0-2,7>", SDC_boot_part_cmd},
	{"bootAck", "<0|1>", SDC_boot_ack_cmd},
	{"bootrd", "<addr> <blkcnt> [repeat]", SDC_boot_read_cmd},
	{"bootwr", "<addr> <blkcnt> [repeat] [compare]", SDC_boot_write_cmd},
#endif
	{"scr", "", SDC_read_scr_cmd},
	{"stp", "", SDC_send_abort_cmd},
	{"sts", "", SDC_read_card_status},
	{"written_nm", "", SDC_repo_written_num},
	{"tune", "<num>", SDC_tune_cmd},
	{"rd", "<addr> <blkcnt> [repeat]", SDC_read_cmd},
	{"wr", "<addr> <blkcnt> [repeat] [compare]", SDC_write_cmd},
	{"er", "<addr> <blkcnt>", SDC_erase_cmd},
	{"reset", "<all|cmd|dat>", SDC_software_reset},
	{"go", "<norm|alt> <ack> <ds|hs> <1|4|8> <cnt>" , SDC_go_boot_image},
	{"scan", "<ds|hs|sdr12|sdr25|sdr50|sdr104|ddr50|hs200> <1|4|8>", SDC_scan_card_cmd},
	{"adma", "", SDC_ShowDscpTbl},
	{"dbg", "<level>", SDC_debug_print},
	{"dump", "", SDC_dump_registers},
	{"wp", "<set|clr|send|type> <addr>", SDC_write_protection_cmd},
	{"perf", "", SDC_performance_test},
	{"quit", "", 0},
	{0}			/* end of ftsdc021CmdTbl */
};

static uint32_t str_to_hex(int8_t * str, void *num, int32_t digits)
{
	int8_t *value = (int8_t *) num;
	int8_t ch, byte;
	int32_t i = 0, j;

	if ((str[0] == '0') && ((str[1] == 'X') || (str[1] == 'x')))
		str += 2;

	while (str[i] != '\0') {
		if (!ISHEX(str[i]))
			return 0;
		i++;
	}

	if ((i == 0) || (i > digits))
		return 0;

	i--;
	for (j = 0; j < ((digits + 1) / 2); j++)
		*value++ = 0;

#ifdef __BIG_ENDIAN
	value = (int8_t *) num + (digits + 1) / 2 - 1;
#else
	value = (int8_t *) num;
#endif
	while (i >= 0) {
		byte = str[i--] - 48;
		if (byte > 9) {
			byte -= 7;
			if (byte > 0xf)
				byte -= 32;
		}
		if (i >= 0) {
			ch = str[i--] - 48;
			if (ch > 9) {
				ch -= 7;
				if (ch > 0xf)
					ch -= 32;
			}
			byte += ch << 4;
			*value = byte;
#ifdef __BIG_ENDIAN
			value--;
#else
			value++;
#endif
		} else {
			*(int8_t *) value = byte;
			break;
		}
	}
	return 1;
}

/* Usage: mrd32 <addr> [num] */
int32_t SDC_mrd32(int32_t argc, char *const argv[])
{
	int32_t i, j;
	uint32_t addr;
	uint32_t num;
	uint32_t *pd;

	if ((argc < 2) || (argc > 3))
		return 1;

	if (!str_to_hex(argv[1], &addr, sizeof(addr) * 2))
		return 1;

	if (argc == 3) {
		if (!str_to_hex(argv[2], &num, sizeof(num) * 2))
			return 1;
	} else
		num = 1;

	pd = (uint32_t *)((uintptr_t)addr);
	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			printf("0x%08p : ", &pd[i]);

		printf("%08X ", pd[i]);

		if (++j == 4) {
			if ((i + 1) < num)
				printf("\r\n");
			j = 0;
		}
	}

	return (0);
}

/* Usage: mwr32 <addr> <data> [num [inc]] */
int32_t SDC_mwr32(int32_t argc, char *const argv[])
{
	int32_t i, j;
	uint32_t addr;
	uint32_t num;
	uint32_t data;
	int32_t datainc;
	uint32_t *pd;

	if ((argc < 3) || (argc > 5))
		return 1;

	if (!str_to_hex(argv[1], &addr, sizeof(addr) * 2))
		return 1;

	if (!str_to_hex(argv[2], &data, sizeof(data) * 2))
		return 1;

	num = 1;
	datainc = 0;
	if (argc >= 4) {
		if (!str_to_hex(argv[3], &num, sizeof(num) * 2))
			return 1;
		if (argc == 5) {
			if (!str_to_hex(argv[4], &datainc, sizeof(datainc) * 2))
				return 1;
		}
	}

	pd = (uint32_t *)((uintptr_t)addr);
	for (i = 0; i < num; i++) {
		pd[i] = data;
		data += datainc;
	}

	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			printf("0x%08p : ", &pd[i]);

		printf("%08X ", pd[i]);

		if (++j == 4) {
			if ((i + 1) < num)
				printf("\r\n");
			j = 0;
		}
	}

	return (0);
}

/* dbg <level> */
int32_t SDC_debug_print(int32_t argc, char *const argv[])
{
	if (argc != 2)
		return 1;

	dbg_print = atoi(argv[1]);

	return 0;
}

static uint32_t makeargs(int8_t * cmd, int32_t * argcptr, char *** argvptr)
{
	char *cp;
	int32_t argc;
	static char strings[CMDLEN + 1];
	static char *argtable[MAXARGS + 1];

	/*
	 * Copy the string and then break it apart
	 * into separate arguments.
	 */
	strcpy(strings, cmd);
	argc = 0;
	cp = strings;

	while (*cp) {
		if (argc >= MAXARGS) {
			printf("Too many arguments\r\n");
			return 1;
		}

		argtable[argc++] = cp;

		while (*cp && !isblank(*cp))
			cp++;

		while (isblank(*cp))
			*cp++ = '\0';
	}

	argtable[argc] = NULL;

	*argcptr = argc;
	*argvptr = argtable;

	return 0;
}

int32_t ftsdc021_do_help(int32_t argc, char ** argv)
{
	cmd_t *cmdp;

	cmdp = ftsdc021CmdTbl;

	if (argc == 1) {	/* display all ftsdc021 commands */
		while (cmdp->name != 0) {
			printf("  %-15s %-36s\r\n", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else if (argc == 2) {	/*Display argv[1] ftsdc021 command */
		while (cmdp->name != 0) {
			if (strcmp(argv[1], cmdp->name) == 0)
				printf("  %-15s %-36s\r\n", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else
		return 1;

	return 0;
}

uint32_t GetCmd(char *buffer, uint32_t Len)
{
	int offset = 0;
	char c;

	buffer[0] = '\0';
	while (offset < (Len - 1)) {
		c = getchar();

		if (c == '\b')          //backspace
		{
			if (offset > 0) {
				// Rub out the old character & update the console output
				offset--;
				buffer[offset] = 0;

				printf("\b \b");
			}
		} else if (c == 0x7F)     //backspace
	        {
			if (offset > 0) {
				// Rub out the old character & update the console output
				offset--;
				buffer[offset] = 0;

				printf("\b \b");
			}
		}
		else {
			if (c == '\r')
				c = '\n';       // treat \r as \n

			printf("%c", c);

			if (c == '\n')
				break;

			buffer[offset++] = c;
		}
	}

	buffer[offset] = '\0';

	return offset;
}

static uint32_t ExecCmd(char *cmdline)
{
	char **argv;
	int32_t argc;
	cmd_t *cmdp = ftsdc021CmdTbl;

	/* skip leading blank if any */
	while (isblank(*cmdline))
		cmdline++;

	if ((*cmdline == '\0') || makeargs(cmdline, &argc, &argv))
		return 0;

	/*
	 * Now look for the command in the command table, execute
	 * the command if found.
	 */
	while (cmdp->name != 0) {
		if (strcmp(argv[0], cmdp->name) == 0)
			break;
		cmdp++;
	}

	/* no any command match */
	if (cmdp->name == 0) {
		printf("No ftsdc021 command found\r\n");
		ftsdc021_do_help(1, 0);	/* list all support commands */
		return 0;	/* get next command */
	}

	/* quit ftsdc021 command entered */
	if (cmdp->func == 0)
		return 1;	/* quit the program */

	if ((*cmdp->func) (argc, argv)) {
		strcpy(argv[1], argv[0]);

		ftsdc021_do_help(2, argv);	/* list the correct command format */
	}

	return 0;		/* get next command */
}

void FTSDC021_main(void)
{
	char cmdline[CMDLEN];

	disable_dcache();

	printf("\r\n FTSDC021 non-OS FPGA verification code.\r\n");

	SDC_test_init();

#ifdef FTSDC021_MMC_CQE
	cqhci_init();
#endif

#ifdef FTSDC021_EDMA
	edma_init();
#endif

	ftsdc021_init();

	while (1) {
		printf("\r\n");
		printf("FTSDC021:>");

		GetCmd(cmdline, CMDLEN);

		if (ExecCmd(cmdline))
			break;
	}
}
