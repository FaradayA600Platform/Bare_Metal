#include <stdio.h>
#include <types.h>
#include "porting.h"

#define __ 255
static unsigned char hextab[256] = {
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, __, __, __, __, __, __,
  __, 10, 11, 12, 13, 14, 15, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, 10, 11, 12, 13, 14, 15, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
};

int a_con_bin(unsigned char c)
{
	return *(hextab+c);
}

char *atoui(char *str,unsigned int *ptrnum,unsigned int base)
{

	unsigned int next_nib;
	unsigned int accum,tmp;
	unsigned int max_accum = ~0 / base;

	accum = 0;
	while ( (next_nib = a_con_bin(*str)) < base )
    { 
        if( accum > max_accum )
		{
		    printf("Integer Overflow\n");
		    break; 
		}
	    tmp = (accum*base) + next_nib;
	    if ( tmp < accum )
		{
		    printf("Integer Overflow\n");
		    break; 
		}
	    accum = tmp;
	    str++;
	}
	*ptrnum = accum;
	return ( str );
}


char *ator(char *str,unsigned int *ptrnum,unsigned int base)
{
	unsigned int num;
	int minus;
	char *end_str;

	num = 0;
	while(*str==0x20)
	    str++;
	if(*str == '-')
	{ 
	    minus = 1;
	    str++; 
	}
	else
	    minus = 0;

	if ( *str == '0' )
	   switch (str[1])
	   {
    		case 'x':
    		case 'X':
    		   end_str = atoui(str + 2,&num,16);
    		   break;
    		case 'o':
    		case 'O':
    		   end_str = atoui(str + 2,&num,8);
    		   break;
    		case 'd':
    		case 'D':
    		   end_str = atoui(str + 2,&num,10);
    		   break;
    		default:
    		   end_str = atoui(str,&num,base);
    		   break;
	   }
	else
	   end_str = atoui(str, &num, base );

	if (minus) 
	{
	    if ((int)num < 0)
		    printf("Signed Overflow\n");
	    else
		    num = -num;
	}

	*ptrnum = num;
	return(end_str);
}


char *atob(char *str,unsigned int *ptrnum,unsigned int base,int seg)

{
    unsigned int num;
    char *end_str; /* KD added static */

    end_str = ator (str, &num, base);
    if (base == 16 && num <= 0x1fffffff)
	num |= seg;
    *ptrnum = num;
    return end_str;
}