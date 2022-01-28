/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#ifndef _CTYPE_H
#define _CTYPE_H	1

/*
 * NOTE! This ctype does not handle EOF like the standard C
 * library is required to.
 */

/* ASCII ords of \t, \f, \n, \r, and \v are 9, 12, 10, 13, 11 respectively. */
#define isspace(c) \
	((sizeof(c) == sizeof(char)) \
	 ? ((((c) == ' ') || (((unsigned char)((c) - 9)) <= (13 - 9)))) \
	 : ((((c) == ' ') || (((unsigned int)((c) - 9)) <= (13 - 9)))))
#define isblank(c) (((c) == ' ') || ((c) == '\t'))
#define isdigit(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)((c) - '0')) < 10) \
	 : (((unsigned int)((c) - '0')) < 10))
#define isxdigit(c) \
	(isdigit(c) \
	 || ((sizeof(c) == sizeof(char)) \
		 ? (((unsigned char)((((c)) | 0x20) - 'a')) < 6) \
		 : (((unsigned int)((((c)) | 0x20) - 'a')) < 6)))
#define iscntrl(c) \
	((sizeof(c) == sizeof(char)) \
	 ? ((((unsigned char)(c)) < 0x20) || ((c) == 0x7f)) \
	 : ((((unsigned int)(c)) < 0x20) || ((c) == 0x7f)))
#define isalpha(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)(((c) | 0x20) - 'a')) < 26) \
	 : (((unsigned int)(((c) | 0x20) - 'a')) < 26))
#define isalnum(c) (isalpha(c) || isdigit(c))
#define isprint(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)((c) - 0x20)) <= (0x7e - 0x20)) \
	 : (((unsigned int)((c) - 0x20)) <= (0x7e - 0x20)))
#define islower(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)((c) - 'a')) < 26) \
	 : (((unsigned int)((c) - 'a')) < 26))
#define isupper(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)((c) - 'A')) < 26) \
	 : (((unsigned int)((c) - 'A')) < 26))
#define ispunct(c) \
	((!isalnum(c)) \
	 && ((sizeof(c) == sizeof(char)) \
		 ? (((unsigned char)((c) - 0x21)) <= (0x7e - 0x21)) \
		 : (((unsigned int)((c) - 0x21)) <= (0x7e - 0x21))))
#define isgraph(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)((c) - 0x21)) <= (0x7e - 0x21)) \
	 : (((unsigned int)((c) - 0x21)) <= (0x7e - 0x21)))

#define tolower(c) (isupper(c) ? ((c) | 0x20) : (c))
#define toupper(c) (islower(c) ? ((c) ^ 0x20) : (c))

/* Now some non-ansi/iso c99 macros. */
#define isascii(c) (((c) & ~0x7f) == 0)
#define toascii(c) ((c) & 0x7f)

#endif /* _CTYPE_H_ */
