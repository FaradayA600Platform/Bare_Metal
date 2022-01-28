/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  cmd_ftssp010.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       DESCRIPTION
 * 2015/06      BingYao      commands to do verification   
 * -------------------------------------------------------------------------
 */
#include <malloc.h>

#include "ftssp010.h"

typedef enum {
	FFMT_SSP,
	FFMT_SPI,
	FFMT_MICRO,
	FFMT_I2S,
	FFMT_LCD,
	FFMT_FLASH,
} FFMT_TYPE;

const char *ffmt_string[] = { "SSP", "SPI", "MicroWire", "I2S", "LCD", "SPI Flash"};

struct ctrl {
	int addr;
	int tx;
	int rx;
	char *tx_buf;
	char *rx_buf;
};

//LCD test Function
unsigned int buffer[64];
int write_pointer = 0;
int read_pointer = 0;
unsigned int buffer_r[64];
int write_pointer_r = 0;
int read_pointer_r = 0;

void fifo_reset(void);
unsigned int rfifo_read(void);
void rfifo_write(unsigned int datain);
unsigned int fifo_read(void);
void fifo_write(unsigned int datain);
int ssp_master_burst_write_fifo(unsigned int burst_len, unsigned int threshold);
void ssp_master_write_fifo(void);
void ssp_slave_burst_write_fifo(unsigned int burst_len, unsigned int threshold);
void ssp_slave_write_fifo(void);

//Do Command
int32_t do_ftssp010_set_txside(int32_t argc, char *const argv[]);
int32_t do_ftssp010_tx(int32_t argc, char *const argv[]);
int32_t do_ftssp010_set_sdl(int32_t argc, char *const argv[]);
int32_t do_ftssp010_set_pcl(int32_t argc, char *const argv[]);
int32_t do_ftssp010_burnin(int32_t argc, char *const argv[]);
int32_t do_ftssp010_i2stest(int32_t argc, char *const argv[]);
int32_t do_ftssp010_lcdtest(int32_t argc, char *const argv[]);
int32_t do_ftssp010_spiflash(int32_t argc, char *const argv[]);

static int argc;
static char *argv[CMDLEN];

struct ctrl master, slave;
FFMT_TYPE ffmt;
int buf_len = 1024;

static cmd_t ftssp010_cmd_tbl[] = {
	{"txside", "<master|slave|both>", do_ftssp010_set_txside},
	{"sdl", "<len>", do_ftssp010_set_sdl},
	{"pcl", "<len>", do_ftssp010_set_pcl},
	{"tx", "<len> <i2s|spi|ssp> [spi mode]", do_ftssp010_tx},
	{"i2stest", "<len> <PDL> <DPDL> <SDL(1~15)> <STEREO(0|1)> <FSJSTFY(0|1> <SCLKDIV>", do_ftssp010_i2stest},
	{"lcdtest", "<mode> <len> <D/C Reg Control(0|1)> <3|4 line mode(0|1)> <SCLKDIV>", do_ftssp010_lcdtest},
	{"flashtest", "<len> <sdl (4~128)> <flashtx bit(0|1)>", do_ftssp010_spiflash},
	{"burnin", "<i2s|spi|ssp|lcd|flash> <execute times (0 == Infinite, 1 == 1, ...)>", do_ftssp010_burnin},
	{"quit", "", 0},
	{0}
};

extern int pcl;
extern int sdl_in_bytes;
extern int sclkdiv;
extern int master_transmit_done;

// txside <master|slave|both>
int32_t do_ftssp010_set_txside(int32_t argc, char *const argv[])
{
	if (argc != 2)
		return 1;

	if (strcmp(argv[1], "master") == 0) {
		master.tx = 1;
		master.rx = 0;
		slave.tx = 0;
		slave.rx = 1;
	} else if (strcmp(argv[1], "slave") == 0) {
		master.tx = 0;
		master.rx = 1;
		slave.tx = 1;
		slave.rx = 0;
	} else if (strcmp(argv[1], "both") == 0) { 
		master.tx = 1;
		master.rx = 1;
		slave.tx = 1;
		slave.rx = 1;
	} else {
		printf(" Argument value not correct\n\r");
		return 1;
	}

	printf("Master addr 0x%x tx %d rx %d\n\r" \
	       "Slave addr 0x%x tx %d rx %d\n\r",
			master.addr, master.tx, master.rx,
			slave.addr, slave.tx, slave.rx);
	return 0;
}

// sdl <len>
int32_t do_ftssp010_set_sdl(int32_t argc, char *const argv[])
{
	int sdl;

	if (argc != 2)
		return 1;

	sdl = strtol(argv[1], 0 , 0);
	if (ftssp010_set_data_length(FTSSP010_REG_BASE_S, sdl))
		return 1;

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl))
		return 1;
	

	sdl_in_bytes = sdl;

	return 0;
}

// pcl <len>
int32_t do_ftssp010_set_pcl(int32_t argc, char *const argv[])
{
	if (argc != 2)
		return 1;

	pcl = strtol(argv[1], 0 , 0);

	if (ftssp010_set_pcl(FTSSP010_REG_BASE_S, pcl, 1))
		return 1;

	if (ftssp010_set_pcl(FTSSP010_REG_BASE_M, pcl, 1))
		return 1;

	return 0;
}

static int do_ftssp010_transfer_bi(int len)
{
	int i, j, bytes, len1, len2, ret;
	int frsync, fifo, index;
	char *tbuf1, *rbuf1, *tbuf2, *rbuf2;

	printf(" %s-%s: Bi-direct transfer %d bytes(sdl %d,",
		ftssp010_get_fformat_string(master.addr),
		ftssp010_get_fformat_string(slave.addr),
		len, sdl_in_bytes);

	if (ffmt == FFMT_SPI || ffmt == FFMT_SSP) {
		printf("pcl %d)\n\r", pcl);
	} else if (ffmt == FFMT_I2S) {
		printf("%s, pdl_r %d pdl_l %d at %s)\n\r",
			stereo ? "stereo" : "mono", pdl_r,
			pdl_l, pad_data_back ? "back" : "front");
	}

	tbuf1 = master.tx_buf;
	rbuf1 = master.rx_buf;
	tbuf2 = slave.tx_buf;
	rbuf2 = slave.rx_buf;

	// Number of FIFO bytes to tansfer sdl in one frame/sync
	fifo = (sdl_in_bytes + 3) & ~0x3;

	frsync = len / sdl_in_bytes;
	index = fifo * frsync;
	index >>= 2;
	for (i = 0; i < index; i++) {
		*((int *)rbuf1 + i) = 0xAAAAAAAA;
		*((int *)rbuf2 + i) = 0xAAAAAAAA;
	}

	printf(" - Start Transfer !!! Press 'q' to leave\n\r");
	while (len) {

		len1 = len2 = len;
		ret = ftssp010_transfer_data_bidirect(
				master.addr, tbuf1, rbuf1, &len1,
			   	slave.addr, tbuf2, rbuf2, &len2);
		if (ret)
			goto error;

		if (len1 != len2) {
			printf(" -- Master remain %d bytes, slave %d bytes\n\r",
				len1, len2);
			goto error;
		}

		bytes = len - len1;
		//Number of frame/sync(s) to transfer "bytes".
		frsync = bytes / sdl_in_bytes;
		index = fifo * frsync;
		for (i = 0; i < index;) {
			for (j = i; j < (i + sdl_in_bytes); j++) { 
				if(tbuf1[j] != rbuf2[j]) {
					printf(" -- Compare master tbuf 0x%x "
						"slave rbuf 0x%x len %d bytes"
						" failed\n\r", (int)tbuf1,
						(int)rbuf2, bytes);
					printf(" --- i = %d, tbuf(0x%x) = "
						"0x%x, rbuf(0x%x) = 0x%x\n\r",
						j, &tbuf1[j], tbuf1[j],&rbuf2[j],
						rbuf2[j]);
					goto error;
				}
			}

			i = (i + sdl_in_bytes + 3) & ~0x3;
		}
		tbuf1 += index;
		rbuf2 += index;

		bytes = len - len2;
		//Number of frame/sync(s) to transfer "bytes".
		frsync = bytes / sdl_in_bytes;
		index = fifo * frsync;
		for (i = 0; i < index;) {
			for (j = i; j < (i + sdl_in_bytes); j++) { 
				if(tbuf2[j] != rbuf1[j]) {
					printf(" -- Compare slave tbuf 0x%x "
						"master rbuf 0x%x len %d bytes"
						"failed\n\r", (int)tbuf2,
						(int)rbuf1, bytes);
					printf(" --- i = %d, tbuf(0x%x) = 0x%x"
						", rbuf(0x%x) = 0x%x \n\r", j,
						&tbuf2[j], tbuf2[j],&rbuf1[j],
						rbuf1[j]);
					goto error;
				}
			}

			i = (i + sdl_in_bytes + 3) & ~0x3;
		}
		tbuf2 += index;
		rbuf1 += index;

		len -= bytes;
		// Press 'q' to leave
		if ('q' == kbhit()) {
			printf(" - User quit the test!\n\r");
			return 1;
		}
	}
	printf(" - Transfer complete!\n\r");
	return 0;

error:
	return 1;
}

static int do_ftssp010_transfer(int tx_addr, char *tbuf,
				int rx_addr, char *rbuf, int len)
{
	int i, j, bytes;
	int frsync, fifo, index;
	int *tmp;

	printf(" %s(0x%x)-%s(0x%x): Tx %d bytes(sdl %d, ",
		ftssp010_get_fformat_string(tx_addr), tx_addr,
		ftssp010_get_fformat_string(rx_addr), rx_addr,
		len, sdl_in_bytes);

	if (ffmt == FFMT_SPI || ffmt == FFMT_SSP) {
		printf("pcl %d)\n\r", pcl);
	} else if (ffmt == FFMT_I2S) {
		printf("%s, pdl_r %d pdl_l %d at %s)\n\r",
		       stereo ? "stereo" : "mono", pdl_r,
		       pdl_l, pad_data_back ? "back" : "front");
	}

	// Number of FIFO bytes to tansfer sdl in one frame/sync
	fifo = (sdl_in_bytes + 3) & ~0x3;

	frsync = len / sdl_in_bytes;
	index = fifo * frsync;
	index >>= 2;
	tmp = (int *) rbuf;
	for (i = 0; i < index; i++)
		*(tmp + i) = 0xAAAAAAAA;

	printf(" - Start Transfer !!! Press 'q' to leave\n\r");
	while (len) {

		bytes = ftssp010_transfer_data(tx_addr, (const void*)tbuf,
					       rx_addr, (void *)rbuf, len);

		//unlikely happens, it is a bug
		if (bytes > len) {
			printf(" -- Want to transfer %d bytes, actual done %d bytes\n\r",
				len, bytes);
			goto error;
		}

		//Number of frame/sync(s) to transfer "bytes".
		frsync = bytes / sdl_in_bytes;

		index = fifo * frsync;

		for (i = 0; i < index;) {
			for (j = i; j < (i + sdl_in_bytes); j++) { 
				if(tbuf[j] != rbuf[j]) {
					printf(" -- Compare tbuf 0x%x rbuf 0x%x"
						" len %d bytes failed\n\r",
						(int)tbuf, (int)rbuf, bytes);
					printf(" --- i = %d, tbuf(0x%x) = 0x%x,"
						" rbuf(0x%x) = 0x%x\n\r", j,
						&tbuf[j], tbuf[j], &rbuf[j],
						rbuf[j]);
					goto error;
				}
			}

			i = (i + sdl_in_bytes + 3) & ~0x3;
		}

		len -= bytes;
		tbuf += index;
		rbuf += index;
		// Press 'q' to leave
		if ('q' == kbhit()) {
			printf(" - User quit the test!\n\r");
			return 1;
		}
	}
	printf(" - Transfer complete!\n\r");
	return 0;

error:
	return 1;
}

static uint32_t do_ftssp010_get_addrs(int *tx_addr, char **tx_buf,
				   int *rx_addr, char **rx_buf)
{
	if (master.tx && !master.rx) {
		printf("Master transmit, Slave receive\n\r");
		*tx_addr = master.addr;
		*tx_buf = master.tx_buf;
		*rx_addr = slave.addr;
		*rx_buf = slave.rx_buf;
	} else if (slave.tx && !slave.rx) {
		printf("Slave transmit, Master receive\n\r");
		*tx_addr = slave.addr;
		*tx_buf = slave.tx_buf;
		*rx_addr = master.addr;
		*rx_buf = master.rx_buf;
	} else if (master.tx && slave.tx) {
		printf("Master transmit/receive, Slave transmit/receive\n\r");
		return 2;
	} else {
		printf(" Not valid tx/rx combination\n\r");
		return 0;
	}

	return 1;
}

// tx <len> <i2s|spi|ssp> <mode>
int32_t do_ftssp010_tx(int32_t argc, char *const argv[])
{
	char *tx_buf;
	char *rx_buf;
	int tx_addr, rx_addr;
	int i, len, mode;

	if (argc < 3)
		return 1;

	len = strtol(argv[1], 0 , 0);
	/* multiple of sdl_in_bytes bytes */
	len = (len + sdl_in_bytes) - (len % sdl_in_bytes);

	if (argc == 4)
		mode = strtol(argv[3], 0 , 0) & 0x3;
	else
		mode = SPI_MODE_0;

	if (strcmp(argv[2], "i2s") == 0) {
		ffmt = FFMT_I2S;
		ftssp010_i2s_init(0);
	} else if (strcmp(argv[2], "spi") == 0) {
		ffmt = FFMT_SPI;
		ftssp010_spi_init(SPI_CS_LOW, 0, mode);
	} else if (strcmp(argv[2], "ssp") == 0) {
		ffmt = FFMT_SSP;
		ftssp010_ssp_init();
	}

	i = do_ftssp010_get_addrs(&tx_addr, &tx_buf,
				  &rx_addr, &rx_buf);

	if (i == 2)
		do_ftssp010_transfer_bi(len);
	else if (i == 1)
		do_ftssp010_transfer(tx_addr, tx_buf, rx_addr, rx_buf, len);

	return 0;
}

int ftssp010_ssptest(unsigned int len)
{
	char *tx_buf;
	char *rx_buf;
	int tx_addr, rx_addr;
	int bidirect;

	ffmt = FFMT_SSP;
	ftssp010_ssp_init();

	bidirect = do_ftssp010_get_addrs(&tx_addr, &tx_buf, &rx_addr, &rx_buf);
	if (!bidirect)
		return 0;

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_S);
	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_M);

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_S, sdl_in_bytes))
		return 0;

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl_in_bytes))
		return 0;

	/* multiple of sdl_in_bytes bytes */
	len = (len + sdl_in_bytes) - (len % sdl_in_bytes);

	if (bidirect == 2) {
		if (do_ftssp010_transfer_bi(len))
			return 0;
	} else {
		if (do_ftssp010_transfer(tx_addr, tx_buf, rx_addr, rx_buf, len))
			return 0;
	}

	return 1;
}

int ftssp010_spitest(unsigned int len, unsigned int mode)
{
	char *tx_buf;
	char *rx_buf;
	int tx_addr, rx_addr;
	int bidirect;

	ffmt = FFMT_SPI;
	pcl &= FTSSP010_CR3_PCL_MASK;
	ftssp010_spi_init(SPI_CS_LOW, 0, mode);

	bidirect = do_ftssp010_get_addrs(&tx_addr, &tx_buf, &rx_addr, &rx_buf);
	if (!bidirect)
		return 0;

	if (ftssp010_set_pcl(FTSSP010_REG_BASE_S, pcl, 0))
		return 0;
	if (ftssp010_set_pcl(FTSSP010_REG_BASE_M, pcl, 0))
		return 0;

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_S);
	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_M);

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_S, sdl_in_bytes))
		return 0;

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl_in_bytes))
		return 0;

	/* multiple of sdl_in_bytes bytes */
	len = (len + sdl_in_bytes) - (len % sdl_in_bytes);

	if (bidirect == 2) {
		if (do_ftssp010_transfer_bi(len))
			return 0;
	} else {
		if (do_ftssp010_transfer(tx_addr, tx_buf, rx_addr, rx_buf, len))
			return 0;
	}

	return 1;
}

int ftssp010_i2stest(unsigned int len)
{
	char *tx_buf;
	char *rx_buf;
	int tx_addr, rx_addr;
	int bidirect;

	ffmt = FFMT_I2S;
	ftssp010_i2s_init(0);

	bidirect = do_ftssp010_get_addrs(&tx_addr, &tx_buf, &rx_addr, &rx_buf);
	if (!bidirect)
		return 0;

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_S);
	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_M);

	ftssp010_set_pdl(FTSSP010_REG_BASE_S);
	ftssp010_set_pdl(FTSSP010_REG_BASE_M);

	ftssp010_set_fsjtfy(FTSSP010_REG_BASE_S);
	ftssp010_set_fsjtfy(FTSSP010_REG_BASE_M);

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_S, sdl_in_bytes))
		return 0;

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl_in_bytes))
		return 0;

	/* multiple of sdl_in_bytes bytes */
	len = (len + sdl_in_bytes) - (len % sdl_in_bytes);

	if (bidirect == 2) {
		if (do_ftssp010_transfer_bi(len))
			return 0;
	} else {
		if (do_ftssp010_transfer(tx_addr, tx_buf, rx_addr, rx_buf, len))
			return 0;
	}

	return 1;
}

int32_t do_ftssp010_i2stest(int32_t argc, char *const argv[])
{
	unsigned int len, status;

	if (argc != 8)
		return 1;

	ffmt = FFMT_I2S;
	len = strtol(argv[1], 0 , 0); //Data Length Count
	pdl_r = strtol(argv[2], 0 , 0);
	pdl_l = strtol(argv[3], 0 , 0);
	sdl_in_bytes = strtol(argv[4], 0 , 0);
	stereo = strtol(argv[5], 0 , 0);
	pad_data_back = strtol(argv[6], 0 , 0);
	sclkdiv = strtol(argv[7], 0 , 0);
	status = ftssp010_i2stest(len);
	if(status == 1){
		printf("\n\r******************************************");
		printf("\n\rSPI I2S Transfer and Receive Test Done !!!");
		printf("\n\r******************************************");
	} else {
		printf("\n\r**********************************************");
		printf("\n\rSPI I2S Transfer and Receive Test not Done !!!");
		printf("\n\r**********************************************");
	}
	return 0;
}

int ftssp010_do_help(int argc, char * const argv[], cmd_t * tbl)
{
	cmd_t *cmdp = &tbl[0];

	if (argc == 1) {	/* display all commands */
		while (cmdp->name != 0) {
			printf("  %-18s %-26s\n\r\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else if (argc == 2) {	/*Display argv[0] command */
		while (cmdp->name != 0) {
			if (strcmp(argv[0], cmdp->name) == 0)
				printf("  %-18s %-26s\n\r\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else {
		return 1;
	}

	return 0;
}
//int ftssp010_main(int argc, char * const argv[])
int cmd_check(char *line, cmd_t *tbl)
{
	int i;
	char *token ,*temp_cmd;
	cmd_t *cmd;

	/* Clean argc and argv */
	argc = 0;
	for (i = 0; i < CMDLEN; i++)
		argv[i] = NULL;

	/* Copy the command string */
	temp_cmd = (char *)malloc(sizeof(char)* CMDLEN);
	if (temp_cmd == NULL) {
		printf("malloc error!\n");
		return 0;
	}
	strcpy(temp_cmd ,line);

	/* Divide the command string into tokens for argv[] */
	token = strtok(temp_cmd, " \r\n\r\t");
	while(token) {
		argv[argc++] = token;
		token = strtok(NULL, " \r\n\r\t");
	}

	if (argc == 0)
		return 0;

	/* Compare the input string to command table */
	for (i = 0; ; ++i) {
		cmd = &tbl[i];

		if (cmd->name == NULL) {
			break;
		}
		else if (!strcmp(cmd->name, argv[0]) && cmd->func) {
			free(temp_cmd);
			return 0;
		}
	}

	free(temp_cmd);
	return 1;
}

#define BS_KEY		0x08 //Backspace
#define LF_KEY		0x0a //Line Feed
#define CR_KEY		0x0d //Carriage Return
#define ES_KEY		0x1b //Escape
#define LSB_KEY		0x5b //Left Square Bracket
#define UP_KEY		0xe048
#define DOWN_KEY	0xe050
#define DEL_KEY		0x7f //Delete

int ftssp010_scan_string(char *buf)
{
	char *cp;
	char ch;
	int count = 0;

	cp = buf;

	do {
		ch = getchar();
		switch(ch) {
		case CR_KEY:
			if(count < CMDLEN) {
				*cp = '\0';
				printf("\n\r");
			}
			break;
		case LF_KEY:
			if(count < CMDLEN) {
				*cp = '\0';
				printf("\n\r");
			}
			break;
		case BS_KEY:
		case DEL_KEY:
			if(count) {
				count--;
				*(--cp) = '\0';
				printf("\b \b");
			}
			break;
		default:
			if( ch > 0x1F && ch < 0x7F && count < CMDLEN) {
				*cp = (char) ch;
				cp++;
				count++;
				printf("%c", ch);
			}
			break;
		}
	} while(ch != CR_KEY && ch != LF_KEY);
	return count;
}

int ftssp010_cmd_exec(char *line, cmd_t *tbl)
{
	int i, ret;
	char *token, temp_cmd[CMDLEN];
	cmd_t *cmd;

	/* Clean argc and argv */
	argc = 0;
	for (i = 0; i < CMDLEN; i++)
		argv[i] = NULL;

	/* Copy the command string */
	strcpy(temp_cmd ,line);

	/* Divide the command string into tokens for argv[] */
	token = strtok(temp_cmd, " \r\n\r\t");
	while(token) {
		argv[argc++] = token;
		token = strtok(NULL, " \r\n\r\t");
	}

	if (argc == 0)
		return 0;

	if (!strcmp("quit",argv[0]))
		return 1;

	/* Compare the input string to command table */
	for (i = 0; ; ++i) {
		cmd = &tbl[i];
		if ((cmd->name == NULL) || !strcmp("help", argv[0]) ) {
			/* list all support commands when 'help' command or match the last NULL command */
			ftssp010_do_help(1, 0, tbl);
			printf("Please enter <command> -h to see usage in detail.\n\r");

			goto out;
		}
		else if (!strcmp(cmd->name, argv[0]) && cmd->func) {
			/* Execute the function when the command was matched */
			ret = cmd->func(argc, argv);
			if (ret == 0 /* || ret == FTPCIE_CMD_HELP */ || ret == (int)NULL) {
				//ftssp010_do_help (2, argv, tbl); //Dump the latest command info
				break;
			}
			else {
				printf("The command was executed unsuccessfully..(Return number: %d)\n\r", ret);
				break;
			}
		}
	}

out:
	return 0;
}

void cmp_check(unsigned int cmp1, unsigned int cmp2)
{
	if (cmp1 != cmp2) {
		printf("\n\rCompare failed!!! cmp1 : 0x%x != cmp2 : 0x%x", cmp1, cmp2);
		while(1);
	}
}

void cmp_check_debug(unsigned int cmp1, unsigned int cmp2, unsigned int messageId)
{
	if (cmp1 != cmp2) {
		printf("\n\rDebug %d...Compare failed!!! cmp1 : 0x%x != cmp2 : 0x%x", messageId, cmp1, cmp2);
		while(1);
	}
}

void fifo_write(unsigned int datain)
{
	extern unsigned int buffer[64];
	extern int write_pointer;
	//Write data into FIFO
	buffer[write_pointer] = datain;
	//Increase Write Pointer
	if(write_pointer == 63){
		write_pointer = 0;
	} else {
		write_pointer++;
	}
}

unsigned int fifo_read(void)
{
	extern unsigned int buffer[64];
	extern int read_pointer;
	unsigned int dataout;
	//Read data From FIFO
	dataout = buffer[read_pointer];
	//Increase Read Pointer
	if(read_pointer == 63){
		read_pointer = 0;
	} else {
		read_pointer++;
	}
	return dataout;
}


void rfifo_write(unsigned int datain)
{
	extern unsigned int buffer_r[64];
	extern int write_pointer_r;
	//Write data into FIFO
	buffer_r[write_pointer_r] = datain;
	//Increase Write Pointer
	if(write_pointer_r == 63){
		write_pointer_r = 0;
	} else {
		write_pointer_r++;
	}
}


unsigned int rfifo_read(void)
{
	extern unsigned int buffer_r[64];
	extern int read_pointer_r;
	unsigned int dataout;
	//Read data From FIFO
	dataout = buffer_r[read_pointer_r];
	//Increase Read Pointer
	if(read_pointer_r == 63){
		read_pointer_r = 0;
	} else {
		read_pointer_r++;
	}
	return dataout;
}

void fifo_reset(void)
{
	extern int write_pointer;
	extern int write_pointer_r;
	extern int read_pointer;
	extern int read_pointer_r;
	write_pointer = 0;
	write_pointer_r = 0;
	read_pointer = 0;
	read_pointer_r = 0;
}

void ftssp010_isr(void *data)
{
	unsigned int status;

	status = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_INTR_STS);
	if (status & FTSSP010_INTSTS_TXCI)
		master_transmit_done = 1;
	//printf("\n\rInterrupt status: 0x%08x!!", status);
	return;
}

int ftssp010_lcdtest(unsigned int mode, unsigned int len,
					 unsigned int dcx, unsigned int lineMode){
	int i, j, r, tfhold, rfhold;
	unsigned int cr0, cr1, cr2, cr3, ssp_status, sdl_value;
	unsigned int wdata, rdata, rdata_cmp, lcddcx, spicontx;
	tfhold = 8;//Threshold for TX
	rfhold = 2;//Threshold for RX
	printf("\n\r******************************************");
	printf("\n\rStart SPI LCD Transfer and Receive Test!!!");
	printf("\n\r******************************************");
	printf("\n\rlcd_test len dcx linemode : %d, %d, %d",len ,dcx ,lineMode);
	//Clear FIFO
	cr2 = FTSSP010_CR2_TXFCLR | FTSSP010_CR2_RXFCLR;
	outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);//Master
	outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);//Slave
	printf("\n\rRESET Master & Slave, clear FIFO");
	fifo_reset();

	//Setting Master CR0
	lcddcx = (unsigned int)rand() & 0x1;
	spicontx = (rand() % 2) << 19;//SPI continuous transfer control
	if(dcx == 0) {//D/X Control by write data
		cr0 = FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_MSTR_SPI | FTSSP010_CR0_FSPO | mode | spicontx;
		printf("\n\rSPI Master, SCLKPO/PH : %d, SPICONTX : %d, D/C Control by write data", mode, spicontx);
	} else {//D/X Control by register
		cr0 = FTSSP010_CR0_LCDDCXS | (lcddcx << 21) |
				FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_MSTR_SPI | FTSSP010_CR0_FSPO | mode | spicontx;
		printf("\n\rSPI Master, SCLKPO/PH : %d, SPICONTX : %d, D/C Control by register, LCDDX : %d", mode, spicontx, lcddcx);
	}

	//cr0 = FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_MSTR_SPI | mode;
	outl(cr0, FTSSP010_REG_BASE_M + FTSSP010_REG_CR0);//Master
	printf("\n\rSPI Master CR0 : 0x%x", cr0);
	cmp_check(cr0, inl(FTSSP010_REG_BASE_M + FTSSP010_REG_CR0));
	//Setting Slave CR0
	cr0 = FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_SLV_SPI | FTSSP010_CR0_FSPO | mode;
	printf("\n\rSPI Slave, SCLKPO/PH : %d", mode);
	outl(cr0, FTSSP010_REG_BASE_S + FTSSP010_REG_CR0);//Slave
	printf("\n\rSPI Slave CR0 : 0x%x", cr0);
	cmp_check(cr0, inl(FTSSP010_REG_BASE_S + FTSSP010_REG_CR0));

	//dcx = 0, 3 wire mode only support 9 bits, 4 wire mode only support 8bits
	if (dcx == 0)
		sdl_in_bytes = 1;

	//3 wire mode, only support 9 bits or 36bits
	if (lineMode == 0 && sdl_in_bytes > 1)
		sdl_in_bytes = 4;

	//Setting CR1
	if(lineMode == 0) {//3 wire mode : 9 bit
		sdl_value =  sdl_in_bytes * 9 - 1;
		cr1 = FTSSP010_CR1_SCLKDIV(sclkdiv) | FTSSP010_CR1_SDL(sdl_value);
	} else {//4 wire mode : 1~4 bytes
		sdl_value = (sdl_in_bytes << 3) - 1;
		cr1 = FTSSP010_CR1_SCLKDIV(sclkdiv) | FTSSP010_CR1_SDL(sdl_value);
	}

	//Setting Master CR1
	outl(cr1, FTSSP010_REG_BASE_M + FTSSP010_REG_CR1);
	printf("\n\rSPI Master CR1 : 0x%x, SCLKDIV = %d, SDL = %d",
				cr1, sclkdiv, ((cr1 & FTSSP010_CR1_SDL_MASK) >> 16) + 1);
	cmp_check(cr1, inl(FTSSP010_REG_BASE_M + FTSSP010_REG_CR1));
	//Setting Slave CR1
	//when 3 wire mode and sdl=36bits, slave must be set as 9 bits
	if (lineMode == 0 && sdl_in_bytes == 4)
		cr1 = FTSSP010_CR1_SCLKDIV(sclkdiv) | FTSSP010_CR1_SDL(8);
	outl(cr1, FTSSP010_REG_BASE_S + FTSSP010_REG_CR1);
	printf("\n\rSPI Slave CR1 : 0x%x, SCLKDIV = %d, SDL = %d",
				cr1, sclkdiv, ((cr1 & FTSSP010_CR1_SDL_MASK) >> 16) + 1);
	cmp_check(cr1, inl(FTSSP010_REG_BASE_S + FTSSP010_REG_CR1));
	//Setting CR3
	cr3 = 0;
	outl(cr3, FTSSP010_REG_BASE_M + FTSSP010_REG_CR3);
	outl(cr3, FTSSP010_REG_BASE_S + FTSSP010_REG_CR3);
	//Setting INTRCR
	outl(0x0, FTSSP010_REG_BASE_M + FTSSP010_REG_INTR_CR);

	//Setting CR2, start enable SSP
	//Setting Slave CR2
	cr2 = FTSSP010_CR2_RXEN | FTSSP010_CR2_SSPEN;
	outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);
	printf("\n\rSPI Slave CR2 : 0x%x, RXEN, SSP Enable", cr2);
	cmp_check(cr2, inl(FTSSP010_REG_BASE_S + FTSSP010_REG_CR2));
	//Setting Master CR2
	cr2 = FTSSP010_CR2_TXEN | FTSSP010_CR2_TXDOE | FTSSP010_CR2_SSPEN;
	outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);
	printf("\n\rSPI Master CR2 : 0x%x, TXEN, SSP Enable", cr2);
	cmp_check(cr2, inl(FTSSP010_REG_BASE_M + FTSSP010_REG_CR2));


	printf("\n\r******************************************");
	printf("\n\r  Start Transfer!!! Press 'q' to leave");
	printf("\n\r******************************************");
	//Start transfer
	if (sdl_in_bytes == 1) {
		for(i = 0, r = 0; i < len;){
			ssp_status = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_STS);
			//Master Transfer
			if((FTSSP010_STS_TFVE(ssp_status) <= tfhold)){
				//If write threshold is smaller than tfhold,
				//write data into Master TXFIFO.
				if((dcx == 0) & (lineMode == 0)){ //DCX controlled by data and 3 wire mode
					//In this mode, we can write any number of lcddcx.
					//Because it is easy to control and test.
					wdata = (rand() & 0x1FF);
				} else {
					//In other mode, we have to decide D/CX first.
					//In D/CX control by register mode the LCDDCX has already write in CR0
					wdata = (rand() & 0xFF) | (lcddcx << 8);//Write 9 bit data
				}
				outl(wdata, FTSSP010_REG_BASE_M + FTSSP010_REG_DATA_PORT);
				fifo_write(wdata);
				i++;
				if((i % 1000000) == 0){
					printf("\n\rSPI Master Send count %d   : 0x%x, LCD : %d",
							i, wdata & 0xFF, (wdata & 0x100) >> 8);
				}
			}
			//Slave Receive
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= rfhold)){
				//If read threshold is larger than rfhold,
				//read data from Slave RXFIFO.
				rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
				rdata_cmp = fifo_read();
				if((dcx == 0) & (lineMode == 0)){ //DCX controlled by data and 3 wire mode
					//In this mode, we can check lcddcx in DR.
					cmp_check(rdata & 0x1FF, rdata_cmp & 0x1FF);
				} else if ((dcx == 1) & (lineMode == 0)){//DCX controlled by register and 3 wire mode
					//In this mode, we should check lcddcx by register.
					//The lcddcx register value has already write in fifo.
					cmp_check(rdata & 0x1FF, rdata_cmp & 0x1FF);
				} else {
					//In 4 wire mode, we have to check lcddcx on D/CX PIN.
					cmp_check(rdata & 0xFF, rdata_cmp & 0xFF); //8 bit in DR
					cmp_check(lcddcx, (inl(0x43F00000) & 0x100) >> 8); //Read Master LCD pin
				}
				r++;
				if((r % 1000000) == 0){
					if(lineMode == 0){
						printf("\n\rSPI Slave Receive count %d : 0x%x, LCD : %d",
								r, rdata & 0xFF, (rdata & 0x100) >> 8);
					} else {
						printf("\n\rSPI Slave Receive count %d : 0x%x, LCD : %d",
								r, rdata & 0xFF, lcddcx);
					}
				}
			}
			// Press 'q' to leave
			if ('q' == kbhit())
				goto out;
		}
		printf("\n\r*********************************");
		printf("\n\rSPI LCD Transfer Done!!!");
		printf("\n\rPrint last data in Slave RXFIFO.");
		printf("\n\r*********************************");
		while(write_pointer != read_pointer){ //Compare the last of the data in fifo
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= 1)){
				//If read threshold is larger than rfhold,
				//read data from Slave RXFIFO.
				rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
				rdata_cmp = fifo_read();
				if((dcx == 0) & (lineMode == 0)){ //DCX controlled by data and 3 wire mode
					//In this mode, we can check lcddcx in DR.
					cmp_check(rdata & 0x1FF, rdata_cmp & 0x1FF);
				} else if ((dcx == 1) & (lineMode == 0)){//DCX controlled by register and 3 wire mode
					//In this mode, we should check lcddcx by register.
					//The lcddcx register value has already write in fifo.
					cmp_check(rdata & 0x1FF, rdata_cmp & 0x1FF);
				} else {
					//In 4 wire mode, we have to check lcddcx on D/CX PIN.
					cmp_check(rdata & 0xFF, rdata_cmp & 0xFF); //8 bit in DR
					cmp_check(lcddcx, (inl(0x43F00000) & 0x100) >> 8); //Read Master LCD pin
				}
				r++;
				if(lineMode == 0){
					printf("\n\rSPI Slave Receive count %d : 0x%x, LCD : %d",
							r, rdata & 0xFF, (rdata & 0x100) >> 8);
				} else {
					printf("\n\rSPI Slave Receive count %d : 0x%x, LCD : %d",
							r, rdata & 0xFF, lcddcx);
				}
			}
			// Press 'q' to leave
			if ('q' == kbhit())
				goto out;
		}
	} else if (sdl_in_bytes == 4 && lineMode == 0){ // 3 wire mode, sdl_in_bytes == 4 bytes, dcx = 1
		for(i = 0, r = 0; i < len;){
			ssp_status = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_STS);
			//Master Transfer
			//1 Master write = 4 Slave read, so let threshold <= 4.
			//Otherwise, the slave will lost data.
			if((FTSSP010_STS_TFVE(ssp_status) <= 4)){
				//If write threshold is smaller than tfhold,
				//write data into Master TXFIFO.
				wdata = rand();
				outl(wdata, FTSSP010_REG_BASE_M + FTSSP010_REG_DATA_PORT);
				fifo_write(((wdata >> 24) & 0xFF) | (lcddcx << 8));
				fifo_write(((wdata >> 16) & 0xFF) | (lcddcx << 8));
				fifo_write(((wdata >> 8) & 0xFF) | (lcddcx << 8));
				fifo_write((wdata & 0xFF) | (lcddcx << 8));
				i++;
				if((i % 1000000) == 0){
					printf("\n\rSPI LCD Master Send count %d", i);
				}
			}
			//Slave Receive
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if (FTSSP010_STS_RFVE(ssp_status) >= rfhold){
				//If read threshold is larger than rfhold,
				//read data from Slave RXFIFO.
				for (j = 0; j < FTSSP010_STS_RFVE(ssp_status); j++) {
					rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
					rdata_cmp = fifo_read();
					cmp_check(rdata & 0x1FF, rdata_cmp & 0x1FF);
					r++;
					if((r % 1000000) == 0){
						printf("\n\rSPI LCD Slave Receive count %d : LCD : %d", r, lcddcx);
					}
				}
			}
			// Press 'q' to leave
			if ('q' == kbhit())
				goto out;
		}
		printf("\n\r*********************************");
		printf("\n\rSPI LCD Transfer Done!!!");
		printf("\n\rPrint last data in Slave RXFIFO.");
		printf("\n\r*********************************");
		while(write_pointer != read_pointer){ //Compare the last of the data in fifo
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= 1)){
				//If read threshold is larger than rfhold,
				//read data from Slave RXFIFO.
				rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
				rdata_cmp = fifo_read();
				cmp_check(rdata & 0x1FF, rdata_cmp & 0x1FF);
				r++;
				printf("\n\rSPI LCD Slave Receive count %d : 0x%x, LCD : %d, WP : %d, RP : %d",
						r, rdata & 0x1FF, lcddcx, write_pointer, read_pointer);
			}
			// Press 'q' to leave
			if ('q' == kbhit())
				goto out;
		}
	} else { // 4 wire mode, sdl_in_bytes > 1 bytes, dcx = 1
		for(i = 0, r = 0; i < len;){
			ssp_status = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_STS);
			//Master Transfer
			if((FTSSP010_STS_TFVE(ssp_status) <= tfhold)){
				//If write threshold is smaller than tfhold,
				//write data into Master TXFIFO.
				wdata = (rand() & (0xFFFFFFFF >> (31- sdl_value)));

				outl(wdata, FTSSP010_REG_BASE_M + FTSSP010_REG_DATA_PORT);
				fifo_write(wdata);
				i++;
				if((i % 1000000) == 0){
					printf("\n\rSPI LCD Master Send count %d", i);
				}
			}
			//Slave Receive
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= rfhold)){
				//If read threshold is larger than rfhold,
				//read data from Slave RXFIFO.
				rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
				rdata_cmp = fifo_read();
				//In 4 wire mode, we have to check lcddcx on D/CX PIN.
				cmp_check(rdata & (0xFFFFFFFF >> (31- sdl_value)), rdata_cmp);
				cmp_check(lcddcx, (inl(0x43F00000) & 0x100) >> 8); //Read Master LCD pin
				r++;
				if((r % 1000000) == 0){
					printf("\n\rSPI LCD Slave Receive count %d : LCD : %d", r, lcddcx);
				}
			}
			// Press 'q' to leave
			if ('q' == kbhit())
				goto out;
		}
		printf("\n\r*********************************");
		printf("\n\rSPI LCD Transfer Done!!!");
		printf("\n\rPrint last data in Slave RXFIFO.");
		printf("\n\r*********************************");
		while(write_pointer != read_pointer){ //Compare the last of the data in fifo
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= 1)){
				//If read threshold is larger than rfhold,
				//read data from Slave RXFIFO.
				rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
				rdata_cmp = fifo_read();
				//In 4 wire mode, we have to check lcddcx on D/CX PIN.
				cmp_check(rdata & (0xFFFFFFFF >> (31- sdl_value)), rdata_cmp);
				cmp_check(lcddcx, (inl(0x43F00000) & 0x100) >> 8); //Read Master LCD pin
				r++;
				printf("\n\rSPI LCD Slave Receive count %d : 0x%x, LCD : %d",
						r, rdata & (0xFFFFFFFF >> (31- sdl_value)), lcddcx);
			}
			// Press 'q' to leave
			if ('q' == kbhit())
				goto out;
		}
	}
	//Clear FIFO
	cr2 = FTSSP010_CR2_TXFCLR | FTSSP010_CR2_RXFCLR;
	outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);//Master
	outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);//Slave
	printf("\n\rRESET Master & Slave, clear FIFO\n\n\n\r");
	fifo_reset();
	return 1;
out:
	//Clear FIFO
	cr2 = FTSSP010_CR2_TXFCLR | FTSSP010_CR2_RXFCLR;
	outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);//Master
	outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);//Slave
	printf("\n\rRESET Master & Slave, clear FIFO\n\n\n\r");
	fifo_reset();
	return 0;

}

int32_t do_ftssp010_lcdtest(int32_t argc, char *const argv[])
{
	unsigned int len, mode,dcx, lineMode, status;

	if (argc != 6)
		return 1;

	//ffmt = FFMT_SPI;
	mode = strtol(argv[1], 0 , 0); //Data Length Count
	len = strtol(argv[2], 0 , 0); //Data Length Count
	dcx = strtol(argv[3], 0 , 0); //D/X Control by write data
	lineMode = strtol(argv[4], 0 , 0);
	sclkdiv = strtol(argv[5], 0 , 0);
	status = ftssp010_lcdtest(mode, len, dcx, lineMode);
	if(status == 1){
		printf("\n\r******************************************");
		printf("\n\rSPI LCD Transfer and Receive Test Done !!!");
		printf("\n\r******************************************");
	} else {
		printf("\n\r**********************************************");
		printf("\n\rSPI LCD Transfer and Receive Test not Done !!!");
		printf("\n\r**********************************************");
	}
	return 0;
}

void ssp_slave_write_fifo(void){
	unsigned int wdata;

	wdata = rand();
	outl(wdata, FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
	rfifo_write(wdata);//Use receive fifo to compare data.
}

void ssp_slave_burst_write_fifo(unsigned int burst_len, unsigned int threshold){
	unsigned int ssp_status;
	int wc;

	ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
	if(FTSSP010_STS_TFVE(ssp_status) <= threshold){
		for(wc = 0; wc < burst_len; wc++){//Prepare Slave's transmit data
			ssp_slave_write_fifo();
		}
	}
}

void ssp_master_write_fifo(void){
	unsigned int wdata;

	wdata = rand();
	outl(wdata, FTSSP010_REG_BASE_M + FTSSP010_REG_DATA_PORT);
	fifo_write(wdata);//Use receive fifo to compare data.
}
int ssp_master_burst_write_fifo(unsigned int burst_len, unsigned int threshold){
	unsigned int ssp_status;
	int wc;

	ssp_status = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_STS);
	if( (FTSSP010_STS_TFVE(ssp_status) <= threshold) &&
		(FTSSP010_STS_RFVE(ssp_status) <= (16))){
		for(wc = 0; wc < burst_len; wc++){//Prepare Slave's transmit data
			ssp_master_write_fifo();
		}
		return 1;
	}
	return 0;
}

int ftssp010_spiflash(unsigned int mode, unsigned int len,
					  unsigned int sdl, unsigned int flashtx,
					  unsigned int sclk_div){
	unsigned int mw, mr, sw, sr, tfhold, sdl_value;
	unsigned int cr0, cr1, cr2, cr3, ssp_status, wdata, rdata, rdata_cmp;
	int srcount, swcount, w;

	tfhold = 24;//Threshold for TX
	printf("\n\r********************************************");
	printf("\n\rStart SPI FLASH Transfer and Receive Test!!!");
	printf("\n\r********************************************");
	//SPI Flash is a half-duplex protocol,
	//so both master and slave should prepare transfer data.

	//Clear FIFO
	cr2 = FTSSP010_CR2_TXFCLR | FTSSP010_CR2_RXFCLR | FTSSP010_CR2_FS;
	outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);//Master
	outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);//Slave
	printf("\n\rRESET Master & Slave, clear FIFO");
	fifo_reset();

	//Setting Master CR0
	if(flashtx == 0) {//Continuous read when no transmit data
		cr0 = FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_MSTR_SPI | FTSSP010_CR0_SPI_FLASH |
			  FTSSP010_CR0_FSPO | mode;
	} else {//Stop read when no transmit data
		cr0 = FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_MSTR_SPI | FTSSP010_CR0_SPI_FLASH |
			  FTSSP010_CR0_FLASHTX | FTSSP010_CR0_FSPO | mode;
	}
	printf("\n\rSPI Master, SCLKPO/PH : %d, flashtx : %d", mode, flashtx);
	outl(cr0, FTSSP010_REG_BASE_M + FTSSP010_REG_CR0);//Master
	printf("\n\rSPI Master CR0 : 0x%x", cr0);
	cmp_check(cr0, inl(FTSSP010_REG_BASE_M + FTSSP010_REG_CR0));

	//Setting Slave CR0
	//In flash mode, the slave is no need to set flash bit.
	cr0 = FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_SLV_SPI | FTSSP010_CR0_FSPO | mode;
	printf("\n\rSPI Slave, SCLKPO/PH : %d", mode);
	outl(cr0, FTSSP010_REG_BASE_S + FTSSP010_REG_CR0);//Slave
	printf("\n\rSPI Slave CR0 : 0x%x", cr0);
	cmp_check(cr0, inl(FTSSP010_REG_BASE_S + FTSSP010_REG_CR0));

	//Setting CR1
	sdl_value = sdl - 1;
	//SCLKDIV 2~0xffff
	cr1 = FTSSP010_CR1_SCLKDIV(sclk_div) | FTSSP010_CR1_SDL(sdl_value);
	//Setting Master CR1
	outl(cr1, FTSSP010_REG_BASE_M + FTSSP010_REG_CR1);
	printf("\n\rSPI Master CR1 : 0x%x, SCLKDIV = %d, SDL = %d",
				cr1, sclk_div, ((cr1 & FTSSP010_CR1_SDL_MASK) >> 16) + 1);
	cmp_check(cr1, inl(FTSSP010_REG_BASE_M + FTSSP010_REG_CR1));
	//Setting Slave CR1
	outl(cr1, FTSSP010_REG_BASE_S + FTSSP010_REG_CR1);
	printf("\n\rSPI Slave CR1 : 0x%x, SCLKDIV = %d, SDL = %d",
				cr1, sclk_div, ((cr1 & FTSSP010_CR1_SDL_MASK) >> 16) + 1);
	cmp_check(cr1, inl(FTSSP010_REG_BASE_S + FTSSP010_REG_CR1));

	//Setting CR3
	cr3 = 0;
	outl(cr3, FTSSP010_REG_BASE_M + FTSSP010_REG_CR3);
	outl(cr3, FTSSP010_REG_BASE_S + FTSSP010_REG_CR3);

	//Setting INTRCR
	if (flashtx) {
		outl(FTSSP010_INTCR_TXCIEN, FTSSP010_REG_BASE_M + FTSSP010_REG_INTR_CR);
		master_transmit_done = 0;
	}
	else {
		outl(0, FTSSP010_REG_BASE_M + FTSSP010_REG_INTR_CR);
	}
	//In half-duplex mode, both side data should be ready for transmit.

	//The flash mode with FLASHTX = 0, Master can receive data when no transmit data
	unsigned int word_len;
	word_len = (sdl_value / 32) + 1;
	//Data mask.
	unsigned int txlen = 0;
	unsigned int data_mask_0W;
	unsigned int data_mask_1W;
	unsigned int data_mask_2W;
	unsigned int data_mask_3W;

	//Data mask. For example, sdl = 5 bit, the data_mask_0W should be 0x1f.
	if(sdl <= 32){
		data_mask_0W = 0xFFFFFFFF >> (32 - sdl);
		data_mask_1W = 0x0;
		data_mask_2W = 0x0;
		data_mask_3W = 0x0;
	} else if(sdl <= 64){
		data_mask_0W = 0xFFFFFFFF;
		data_mask_1W = 0xFFFFFFFF >> (64 - sdl);
		data_mask_2W = 0x0;
		data_mask_3W = 0x0;
	} else if(sdl <= 96){
		data_mask_0W = 0xFFFFFFFF;
		data_mask_1W = 0xFFFFFFFF;
		data_mask_2W = 0xFFFFFFFF >> (96 - sdl);
		data_mask_3W = 0x0;
	} else{
		data_mask_0W = 0xFFFFFFFF;
		data_mask_1W = 0xFFFFFFFF;
		data_mask_2W = 0xFFFFFFFF;
		data_mask_3W = 0xFFFFFFFF >> (128 - sdl);
	}

	//flashtx = 0, Master will continuous read data when no TX data.
	//flashtx = 1, Master will stop read data when no TX data.
	if(flashtx == 0){
		//In this mode, the TX data usually has a smaller length.
		//And read data from slave.
		while((txlen == 0) | (txlen > len)){
			txlen = rand() % 9;//1~8 data length
		}
		for(mw = 0; mw < txlen * word_len; mw++){
			wdata = rand();
			outl(wdata, FTSSP010_REG_BASE_M + FTSSP010_REG_DATA_PORT);
			fifo_write(wdata);
		}
		for(sw = 0; sw < 32; sw++){//Prepare Slave's transmit data, push full.
			ssp_slave_write_fifo();
		}
		do {
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
		} while ((FTSSP010_STS_TFVE(ssp_status) < 30));

		//Setting CR2, start enable SSP
		//Setting Slave CR2
		cr2 = FTSSP010_CR2_TXEN | FTSSP010_CR2_TXDOE | FTSSP010_CR2_RXEN | FTSSP010_CR2_SSPEN;
		outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);
		printf("\n\rSPI Slave CR2 : 0x%x, TXEN & RXEN, SSP Enable", cr2);
		cmp_check(cr2, inl(FTSSP010_REG_BASE_S + FTSSP010_REG_CR2));
		//Setting Master CR2
		printf("\n\rSPI Master CR2 : 0x%x, TXEN & RXEN, SSP Enable", cr2);
		printf("\n\r******************************************");
		printf("\n\r  Start Transfer !!! Press 'q' to leave");
		printf("\n\r******************************************");
		outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);
		cmp_check_debug(cr2, inl(FTSSP010_REG_BASE_M + FTSSP010_REG_CR2),0xFFFFFF);
		sr = 0;//Use for counting slave receive data length
		for(mr = 0; mr < len * word_len;){//Use for counting data length
			//Slave Transmit data, do not let TX fifo empty
			ssp_slave_burst_write_fifo(8, tfhold);
			//Slave Receive data
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= word_len)){
				//Read All data in Slave receive fifo
				for(srcount = 0; srcount < (FTSSP010_STS_RFVE(ssp_status)/word_len);srcount++){
					for(w = 0; w < word_len; w++){
						rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
						if(sr < txlen * word_len){//No need to compare data, when Master has no TX data.
							rdata_cmp = fifo_read();
							if(w == 0){
								cmp_check_debug(rdata & data_mask_0W, rdata_cmp & data_mask_0W, sr + 1000);
							} else if(w == 1){
								cmp_check_debug(rdata & data_mask_1W, rdata_cmp & data_mask_1W, sr + 1000);
							} else if(w == 2){
								cmp_check_debug(rdata & data_mask_2W, rdata_cmp & data_mask_2W, sr + 1000);
							} else if(w == 3){
								cmp_check_debug(rdata & data_mask_3W, rdata_cmp & data_mask_3W, sr + 1000);
							}
						}
						sr++;
						if((sr % 1000000) == 0){
							unsigned int KByteCount, KByte;
							KByte = sr/(8192 * word_len);
							KByteCount = sdl * KByte;
							printf("\n\rSPI Slave Receive Data count %d, %d KB", sr, KByteCount );
						}
					}
				}
			}
			//Slave Transmit data, do not let TX fifo empty
			ssp_slave_burst_write_fifo(8, tfhold);
			//Master Receive data
			ssp_status = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_STS);
			//if((FTSSP010_STS_RFVE(ssp_status) >= word_len)){
			if((FTSSP010_STS_RFVE(ssp_status) >= 4)){
				for(w = 0; w < word_len; w++){
					rdata = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_DATA_PORT);
					rdata_cmp = rfifo_read();
					if(w == 0){
						cmp_check_debug(rdata & data_mask_0W, rdata_cmp & data_mask_0W,mr);
					} else if(w == 1){
						cmp_check_debug(rdata & data_mask_1W, rdata_cmp & data_mask_1W,mr);
					} else if(w == 2){
						cmp_check_debug(rdata & data_mask_2W, rdata_cmp & data_mask_2W,mr);
					} else if(w == 3){
						cmp_check_debug(rdata & data_mask_3W, rdata_cmp & data_mask_3W,mr);
					}
					mr++;//Receive data until transmit done.
				}
			}
		}
		// Press 'q' to leave
		if ('q' == kbhit())
			goto out;

	} else {
		//In this mode, the TX data number is controlled by master.
		//If no TX data in master's TXFIFO, the bus will idle.

		//Prepare Slave Transmit data, do not let TX fifo empty
		ssp_slave_burst_write_fifo(8, 8);
		do {
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
		} while ((FTSSP010_STS_TFVE(ssp_status) < 8));

		//Setting CR2, start enable SSP
		//Setting Slave CR2
		cr2 = FTSSP010_CR2_TXEN | FTSSP010_CR2_TXDOE | FTSSP010_CR2_RXEN | FTSSP010_CR2_SSPEN;
		outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);
		printf("\n\rSPI Slave CR2 : 0x%x, TXEN & RXEN, SSP Enable", cr2);
		cmp_check(cr2, inl(FTSSP010_REG_BASE_S + FTSSP010_REG_CR2));
		//Setting Master CR2
		printf("\n\rSPI Master CR2 : 0x%x, TXEN & RXEN, SSP Enable", cr2);
		printf("\n\r******************************************");
		printf("\n\r  Start Transfer!!! Press 'q' to leave");
		printf("\n\r******************************************");

		outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);
		cmp_check_debug(cr2, inl(FTSSP010_REG_BASE_M + FTSSP010_REG_CR2),0xFFFFFF);
		sr = 0;//Use for counting Slave receive data length
		mr = 0;//Use for counting Master receive data length
		mw = 0;//Use for counting Master write counter
		for(; mr < len * word_len;){//Use for counting data length
			ssp_slave_burst_write_fifo(8, 16);
			do {
				ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			} while ((FTSSP010_STS_TFVE(ssp_status) < 8));
			//Master Transmit data
			if(mw < (len * word_len)){
				//Fill up master transmit fifo
				if (((len * word_len) - mw) >= 8){
					//If write fifo is success, Master write counter plus eight
					if(ssp_master_burst_write_fifo(8, 8)){
						mw += 8;
					}
				} else {
					//If write fifo is success, Master write counter plus one
					if(ssp_master_burst_write_fifo(1, 8)){
						mw += 1;
					}
				}
			}
			//Slave Transmit data, do not let TX fifo empty
			//ssp_slave_burst_write_fifo(8, 16);
			//Slave Receive data
			ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= word_len)){
				//Read All data in Slave receive fifo
				for(srcount = 0; srcount < (FTSSP010_STS_RFVE(ssp_status)/word_len);srcount++){
					//Read data with sdl data length
					for(w = 0; w < word_len; w++){
						rdata = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_DATA_PORT);
						rdata_cmp = fifo_read();
						//printf("\n\rSlave read : 0x%x", rdata);
						if(w == 0){
							cmp_check_debug(rdata & data_mask_0W, rdata_cmp & data_mask_0W, sr + 1000);
						} else if(w == 1){
							cmp_check_debug(rdata & data_mask_1W, rdata_cmp & data_mask_1W, sr + 1000);
						} else if(w == 2){
							cmp_check_debug(rdata & data_mask_2W, rdata_cmp & data_mask_2W, sr + 1000);
						} else if(w == 3){
							cmp_check_debug(rdata & data_mask_3W, rdata_cmp & data_mask_3W, sr + 1000);
						}
						sr++;
						if((sr % 1000000) == 0){
							unsigned int KByteCount, KByte;
							KByte = sr/(8192 * word_len);
							KByteCount = sdl * KByte;
							printf("\n\rSPI Slave Receive Data count %d, %d KB", sr, KByteCount );
						}
					}
				}
			}
			ssp_status = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_STS);
			if((FTSSP010_STS_RFVE(ssp_status) >= word_len)){
				//Read All data in Master receive fifo
				for(swcount = 0; swcount < (FTSSP010_STS_RFVE(ssp_status)/word_len);swcount++){
					//Read data with sdl data length
					for(w = 0; w < word_len; w++){
						ssp_status = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_STS);
						rdata = inl(FTSSP010_REG_BASE_M + FTSSP010_REG_DATA_PORT);
						rdata_cmp = rfifo_read();
						if(w == 0){
							cmp_check_debug(rdata & data_mask_0W, rdata_cmp & data_mask_0W,mr);
						} else if(w == 1){
							cmp_check_debug(rdata & data_mask_1W, rdata_cmp & data_mask_1W,mr);
						} else if(w == 2){
							cmp_check_debug(rdata & data_mask_2W, rdata_cmp & data_mask_2W,mr);
						} else if(w == 3){
							cmp_check_debug(rdata & data_mask_3W, rdata_cmp & data_mask_3W,mr);
						}
						mr++;
					}
				}
			}
			// Press 'q' to leave
			if ('q' == kbhit())
				goto out;
		}
		while (!master_transmit_done);
	}

	//Clear FIFO
	cr2 = FTSSP010_CR2_TXFCLR | FTSSP010_CR2_RXFCLR;
	outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);//Master
	outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);//Slave
	printf("\n\rRESET Master & Slave, clear FIFO\n\n\n\r");
	return 1;
out :
	//Clear FIFO
	cr2 = FTSSP010_CR2_TXFCLR | FTSSP010_CR2_RXFCLR;
	outl(cr2, FTSSP010_REG_BASE_M + FTSSP010_REG_CR2);//Master
	outl(cr2, FTSSP010_REG_BASE_S + FTSSP010_REG_CR2);//Slave
	printf("\n\rRESET Master & Slave, clear FIFO\n\n\n\r");
	return 0;
}

int32_t do_ftssp010_spiflash(int32_t argc, char *const argv[])
{
	unsigned int len, mode, flashtx, sdl, status, sclk_div;

	if (argc != 6)
		return 1;

	//SPI Flash is a half-duplex protocol,
	//so both master and slave should prepare transfer data.
	mode = strtol(argv[1], 0 , 0);;//SCLKPH/PO
	len = strtol(argv[2], 0 , 0); //Data Length Count
	sdl = strtol(argv[3], 0 , 0); //Serial data Length Count
	flashtx = strtol(argv[4], 0 , 0);//Flash transmit control
	sclk_div = strtol(argv[5], 0 , 0);//Flash transmit control
	if((sdl < 4) || (sdl > 128)){
		printf("\n\rError : The sdl value must in 4 ~ 128\n\r");
		return 0;
	}
	status = ftssp010_spiflash(mode, len, sdl, flashtx, sclk_div);
	if(status == 1){
		printf("\n\r******************************************");
		printf("\n\rSPI LCD Transfer and Receive Test Done !!!");
		printf("\n\r******************************************");
	} else {
		printf("\n\r**********************************************");
		printf("\n\rSPI LCD Transfer and Receive Test not Done !!!");
		printf("\n\r**********************************************");
	}
	return 0;
}

int32_t do_ftssp010_burnin(int32_t argc, char *const argv[])
{
	unsigned int len, mode, dcx, lineMode, status, flashtx, sdl, count, xtimes;

	if (argc != 3)
		return 1;

	if (strcmp(argv[1], "i2s") == 0)
		ffmt = FFMT_I2S;
	else if (strcmp(argv[1], "spi") == 0)
		ffmt = FFMT_SPI;
	else if (strcmp(argv[1], "ssp") == 0)
		ffmt = FFMT_SSP;
	else if (strcmp(argv[1], "lcd") == 0)
		ffmt = FFMT_LCD;
	else if (strcmp(argv[1], "flash") == 0)
		ffmt = FFMT_FLASH;

	count = 0;
	xtimes = strtol(argv[2], 0 , 0); //Data Length Count
	while((count < xtimes) || (xtimes == 0)){
		sclkdiv = (rand() % (64 - 2)) + 2;
		if (ffmt == FFMT_I2S) {
			sdl_in_bytes = rand() % 16 + 1;
			len = (rand() % 200) + 1;
			/* multiple of sdl_in_bytes bytes */
			len = (len + sdl_in_bytes) - (len % sdl_in_bytes);
			pdl_r = rand() & 0xff;
			pdl_l = rand() & 0xff;
			stereo = rand() % 2;
			pad_data_back = rand() % 2;
			status = ftssp010_i2stest(len);
		} else if (ffmt == FFMT_SPI) {
			sdl_in_bytes = rand() % 16 + 1;
			len = (rand() % 200) + 1;
			/* multiple of sdl_in_bytes bytes */
			len = (len + sdl_in_bytes) - (len % sdl_in_bytes);
			mode = rand() % 4;//SCLKPH/PO
			pcl = rand() & FTSSP010_CR3_PCL_MASK;
			status = ftssp010_spitest(len, mode);
		} else if (ffmt == FFMT_SSP) {
			sdl_in_bytes = rand() % 16 + 1;
			len = (rand() % 200) + 1;
			/* multiple of sdl_in_bytes bytes */
			len = (len + sdl_in_bytes) - (len % sdl_in_bytes);
			status = ftssp010_ssptest(len);
		} else if (ffmt == FFMT_LCD) {
			mode = rand() % 4;//SCLKPH/PO
			len = rand() % 1000000 + 1; //Data Length Count
			dcx = rand() % 2;//D/X Control by write data
			lineMode = rand() % 2;//3 or 4 wire mode
			sdl_in_bytes = rand() % 4 + 1;
			status = ftssp010_lcdtest(mode, len, dcx, lineMode);
		} else if (ffmt == FFMT_FLASH) {
			mode = rand() % 4;//SCLKPH/PO
			len = rand() % 100000 + 1; //Data Length Count
			sdl = (rand() % (128 - 4)) + 4;
			flashtx = rand() % 2;
			sclkdiv = (rand() % (64 - 6)) + 6;
			status = ftssp010_spiflash(mode, len, sdl, flashtx, sclkdiv);
		}

		count++;

		if(status == 1){
			printf("\n\r*******************************************************");
			printf("\n\r%s Transfer and Receive Test Done Number : %d", ffmt_string[ffmt], count);
			printf("\n\r*******************************************************\n\r");
		} else {
			printf("\n\r*************************************************");
			printf("\n\rExit %s Transfer and Receive Burnin Test !!!", ffmt_string[ffmt]);
			printf("\n\r*************************************************\n\r");
			break;
		}
	}
	printf("\n\r*****************************************************");
	printf("\n\r%s Transfer and Receive Burnin Test All Done !!!", ffmt_string[ffmt]);
	printf("\n\r*****************************************************\n\r");
	return 0;
}

int FTSSP010_Test_Main()
{
	int ret;
	int i;
	char cmdstr[CMDLEN];

	ret = ftssp010_setup_interrupt(FTSSP010_IRQ, ftssp010_isr);
	if (ret)
		printf("Setup interrupt Failed (%d)\r\n", ret);

	ftssp010_platform_init();

	// Default to master tx, slave rx, half duplex
	master.addr = FTSSP010_REG_BASE_M;
	master.tx = 1;
	master.rx = 0;
	master.tx_buf = (char *) memalign(16, buf_len);
	if (!master.tx_buf) {
		printf("Allocate master.tx_buf %d bytes failed\n\r", buf_len);
		return 1;
	}
	master.rx_buf = (char *) memalign(16, (buf_len << 1));
	if (!master.rx_buf) {
		printf("Allocate master.rx_buf %d bytes failed\n\r", buf_len);
		return 1;
	}
	printf(" master addr: 0x%x, tx_buf 0x%x, rx_buf 0x%x, len %d\n\r",
		master.addr, (int)master.tx_buf, (int)master.rx_buf, buf_len);


	slave.addr = FTSSP010_REG_BASE_S;
	slave.tx = 0;
	slave.rx = 1;
	slave.tx_buf = (char *) memalign(16, (buf_len << 1));
	if (!slave.tx_buf) {
		printf("Allocate slave.tx_buf %d bytes failed\n\r", (buf_len << 1));
		return 1;
	}
	slave.rx_buf = (char *) memalign(16, buf_len);
	if (!slave.rx_buf) {
		printf("Allocate slave.rx_buf %d bytes failed\n\r", buf_len);
		return 1;
	}
	printf(" slave addr: 0x%x, : tx_buf 0x%x, rx_buf 0x%x, len %d\n\r",
		slave.addr, (int)slave.tx_buf, (int) slave.rx_buf, buf_len);

	// Prepare the pattern for writing
	for (i = 0; i < buf_len; i++) {
		*(master.tx_buf + i) = i;
		*(slave.tx_buf + i) = i;
	}
	printf("\n");
	while (1) {
		printf("\rftssp010:> ");

		ftssp010_scan_string(cmdstr);
		if (ftssp010_cmd_exec(cmdstr, ftssp010_cmd_tbl))
			break;
	}

	free((char *)master.tx_buf);
	free((char *)master.rx_buf);
	free((char *)slave.tx_buf);
	free((char *)slave.rx_buf);
	return 0;
}
