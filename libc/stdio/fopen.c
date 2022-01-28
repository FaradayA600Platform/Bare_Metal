/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

//#include <virgil.h>
//#include <ff.h>
#include <stdio.h>

static FILE flist[FOPEN_MAX];

FILE *fopen(const char *filename, const char *mode)
{
	int i, am;
	FILE *fp;
	BYTE flag;
	FRESULT rs;

	for (i = 0; i < FOPEN_MAX; ++i) {
		fp = flist + i;
		if (fp->fs == NULL)
			break;
	}
	if (i >= FOPEN_MAX) {
		errno = ENFILE;
		return NULL;
	}

	am = 0;	/* append mode */
	flag = 0;
	for (i = 0; mode[i]; ++i) {
		switch (mode[i]) {
		case 'r':
			flag |= FA_READ;
			break;
#if !_FS_READONLY
		case 'a':
			++am;
		case 'w':
			flag |= FA_WRITE | FA_OPEN_ALWAYS;
			break;
#endif
		case '+':
			puts("fopen: mode '+' is not supported\n");
			errno = EINVAL;
			return NULL;
		}
	}

	rs = f_open(fp, filename, flag);

	ffeconv(rs);

#if CONFIG_VFS_FATFS_MINIMIZE < 3
	if (errno == 0 && am)
		f_lseek(fp, f_size(fp));
#endif

	return errno ? NULL : fp;
}
