/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

//#include <virgil.h>
#include <ff.h>

void ffeconv(FRESULT rs)
{
	switch (rs) {
	case FR_OK:
		errno = 0;
		break;
	case FR_DISK_ERR:
	case FR_INT_ERR:
	case FR_MKFS_ABORTED:
		errno = EIO;
		break;
	case FR_INVALID_PARAMETER:
	case FR_INVALID_OBJECT:
	case FR_INVALID_DRIVE:
		errno = EINVAL;
		break;
	case FR_NOT_ENABLED:
	case FR_NO_FILESYSTEM:
	case FR_NOT_READY:
	case FR_TOO_MANY_OPEN_FILES:
		errno = ENXIO;
		break;
	case FR_NO_FILE:
	case FR_NO_PATH:
		errno = ENOENT;
		break;
	case FR_INVALID_NAME:
		errno = ENAMETOOLONG;
		break;
	case FR_EXIST:
		errno = EMFILE;
		break;
	case FR_DENIED:
	case FR_LOCKED:
		errno = EACCES;
		break;
	case FR_WRITE_PROTECTED:
		errno = EROFS;
		break;
	case FR_TIMEOUT:
		errno = EAGAIN;
		break;
	case FR_NOT_ENOUGH_CORE:
		errno = ENOMEM;
		break;
	}
}
