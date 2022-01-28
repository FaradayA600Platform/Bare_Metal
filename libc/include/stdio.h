/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
//#include <console.h>

#ifndef NULL
#define	NULL	0
#endif

/* End of file character.
   Some things throughout the library rely on this being -1.  */
#ifndef EOF
#define EOF 	(-1)
#endif

#ifndef BUFSIZ
#define	BUFSIZ			1024
#endif

#ifndef FOPEN_MAX
#define	FOPEN_MAX		4
#endif

#ifndef FILENAME_MAX
#define	FILENAME_MAX	1024
#endif

#ifndef SEEK_SET
#define	SEEK_SET		0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR		1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END		2	/* set file offset to EOF plus offset */
#endif

//2020-0825, Leon adding for A600 make
//#ifndef _REENT_ONLY
//2020-0825, Leon adding for A600 make
#define	stdin	(_REENT->_stdin)
#define	stdout	(_REENT->_stdout)
#define	stderr	(_REENT->_stderr)

//2020-0825, Leon adding for A600 make
//#else /* _REENT_ONLY */
//#define	stdin	(_impure_ptr->_stdin)
//#define	stdout	(_impure_ptr->_stdout)
//#define	stderr	(_impure_ptr->_stderr)
//#endif /* _REENT_ONLY */
//2020-0825, Leon adding for A600 make

//#define _MAX_SS     512

//#ifdef CONFIG_VFS_FATFS

#include <ff.h>

/* File system object structure (FATFS) */
#if 0
typedef struct {
        unsigned char    fs_type;                /* FAT sub-type (0:Not mounted) */
        unsigned char    drv;                    /* Physical drive number */
        unsigned char    csize;                  /* Sectors per cluster (1,2,4...128) */
        unsigned char    n_fats;                 /* Number of FAT copies (1,2) */
        unsigned char    wflag;                  /* win[] dirty flag (1:must be written back) */
        unsigned char    fsi_flag;               /* fsinfo dirty flag (1:must be written back) */
        unsigned short    id;                             /* File system mount ID */
        unsigned short    n_rootdir;              /* Number of root directory entries (FAT12/16) */
#if _MAX_SS != 512
        unsigned short    ssize;                  /* Bytes per sector (512, 1024, 2048 or 4096) */
#endif
#if _FS_REENTRANT
        _SYNC_t sobj;                   /* Identifier of sync object */
#endif
#if !_FS_READONLY
        unsigned int   last_clust;             /* Last allocated cluster */
        unsigned int   free_clust;             /* Number of free clusters */
        unsigned int   fsi_sector;             /* fsinfo sector (FAT32) */
#endif
#if _FS_RPATH
        unsigned int   cdir;                   /* Current directory start cluster (0:root) */
#endif
        unsigned int   n_fatent;               /* Number of FAT entries (= number of clusters + 2) */
        unsigned int   fsize;                  /* Sectors per FAT */
        unsigned int   fatbase;                /* FAT start sector */
        unsigned int   dirbase;                /* Root directory start sector (FAT32:Cluster#) */
        unsigned int   database;               /* Data start sector */
        unsigned int   winsect;                /* Current sector appearing in the win[] */
        unsigned char    win[_MAX_SS];   /* Disk access window for Directory, FAT (and Data on tiny cfg) */
} FATFS;

typedef struct {
         FATFS*  fs;                             /* Pointer to the owner file system object */
         unsigned short    id;                             /* Owner file system mount ID */
         unsigned char    flag;                   /* File status flags */
         unsigned char    pad1;
         unsigned int   fptr;                   /* File read/write pointer (0 on file open) */
         unsigned int   fsize;                  /* File size */
         unsigned int   sclust;                 /* File start cluster (0 when fsize==0) */
         unsigned int   clust;                  /* Current cluster */
         unsigned int   dsect;                  /* Current data sector */
 #if !_FS_READONLY
         unsigned int   dir_sect;               /* Sector containing the directory entry */
         unsigned char*   dir_ptr;                /* Ponter to the directory entry in the window */
 #endif
 #if _USE_FASTSEEK
         unsigned int*  cltbl;                  /* Pointer to the cluster link map table (null on file open) */
 #endif
 #if _FS_SHARE
         unsigned char    lockid;                 /* File lock ID (index of file semaphore table) */
 #endif
 #if !_FS_TINY
         unsigned char    buf[_MAX_SS];   /* File data read/write buffer */
 #endif
} FIL;

/* File function return code (FRESULT) */
typedef enum {	
    FR_OK = 0,				/* (0) Succeeded */	
    FR_DISK_ERR,			        /* (1) A hard error occurred in the low level disk I/O layer */	
    FR_INT_ERR,				/* (2) Assertion failed */	
    FR_NOT_READY,			/* (3) The physical drive cannot work */	
    FR_NO_FILE,				/* (4) Could not find the file */	
    FR_NO_PATH,				/* (5) Could not find the path */	
    FR_INVALID_NAME,		/* (6) The path name format is invalid */	
    FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */	
    FR_EXIST,				        /* (8) Access denied due to prohibited access */	
    FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */	
    FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */	
    FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */	
    FR_NOT_ENABLED,			/* (12) The volume has no work area */	
    FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */	
    FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */	
    FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */	
    FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */	
    FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */	
    FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_SHARE */	
    FR_INVALID_PARAMETER,	/* (19) Given parameter is invalid */	
    FR_NO_DISK				/* (20) No Disk present in SD Cage */
} FRESULT;
#endif
typedef FIL FILE;
void   ffeconv(FRESULT rs);	/* FatFS error code convert */
FILE  *fopen(const char *filename, const char *mode);
int    fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int    fflush(FILE *stream);
int    fseek(FILE *stream, long int offset, int whence);
char   *fgets(char *s, int n, FILE *stream);
#define fputc(c, fs)				f_putc(c, fs)
#define fputs(s, fs)				f_puts(s, fs)
#define fprintf(fs, fmt, args...)	f_printf(fs, fmt, ## args)
#define feof(fs)					f_eof(fs)
#define ferror(fs)					f_error(fs)
#define ftell(fs)					(long int)f_tell(fs)

//#endif	/* #ifdef CONFIG_VFS_FATFS */

/* Read/Write characters from/to stdin. */

#ifdef CONFIG_CONSOLE
# define getchar()  console_getc()
# define putchar(c) console_putc(c)
# define gets(s)    console_gets(s)
# define puts(s)    console_puts(s)
#else  /* CONFIG_CONSOLE */
# define getchar()  EOF
# define putchar(c) (0)
# define gets(s)    NULL
# define puts(s)    (0)
#endif /* CONFIG_CONSOLE */

int scanf(const char *format, ...);
int sscanf(const char *str, const char *format, ...);

/*
 * printf(...)
 */
#ifdef CONFIG_LIBC_PRINTF
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
#else
//#define printf(fmt, args...)            do{ }while(0)
//#define sprintf(buf, fmt, args...)      do{ }while(0)
//#define snprintf(buf, sz, fmt, args...) do{ }while(0)
#endif /* CONFIG_LIBC_PRINTF */

#endif	/* #ifndef _STDIO_H */
