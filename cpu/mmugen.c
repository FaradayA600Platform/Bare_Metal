
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define TRUE	1
#define FALSE	0


enum  validwords 
{
		BASE_ADDRESS,
		LEVEL_KW,
		VIRTUAL		,
		TO			,
		PHYSICAL	,
		COARSEPAGES	,
		FINEPAGES	,
		SECTION		,
		NO_ACCESS_KW,
		SVC_READWRITE,
		NO_USR_WRITE,
		FULL_ACCESS	,
		DOMAIN		,
		FAULT		,
		CACHEABLE_KW,
		AND			,
		BUFFERABLE_KW,
		NOT			,
		LARGEPAGES	,
		SMALLPAGES	,
		TINYPAGES	,
		POSTPROCESS ,
		UNRECOGNISED
};

typedef struct
{
		unsigned long v_base;   /* first address in virtual section					 */
		unsigned long v_end;			/* last address in virtual section					  */
		unsigned long p_base;   /* corresponding physical base address		  */
		unsigned long entrytype; /* 'P' for pages, 'S' for section					  */
		unsigned long access[4];   /* access types defined in the enum above	   */		
		unsigned long domain;	/* page belong to what doamin 		*/
		unsigned long cb;			   /* cacheable & bufferable status						*/
}mmudata;

typedef struct
{
		long index;
		char *word;
}tparsedata;

struct {
	char *MMUTableStr;
	int MMUTableStrIndex;
	int MMUTableIndex;
	mmudata rule ;
	unsigned long access_index;
	unsigned long *base_address ;
	unsigned long table_entry ;
	long state ;   
	unsigned long level2_base_address;
	unsigned long level2_page_size;
	unsigned long level2_previous_region;//1:fine page 0:coarse page
	unsigned long level2_current_region; //1:fine page 0:coarse page
} global ;

#define SIZE_4K		0x1000


#define MAX_FINE_PAGE_REGION 128

typedef struct {
	unsigned long start;
	unsigned long end;	
}tFine_Page_Region;

unsigned long Total_Fine_Pages;
tFine_Page_Region Fine_Page_Region[MAX_FINE_PAGE_REGION];

tparsedata parsedata[] = {
	{BASE_ADDRESS   ,"BASE_ADDRESS" },
	{LEVEL_KW	 	,"LEVEL"		},
	{VIRTUAL		,"VIRTUAL"		},
	{TO				,"TO"			},
	{PHYSICAL		,"PHYSICAL"		},
	{COARSEPAGES	,"COARSEPAGES"	},
	{FINEPAGES		,"FINEPAGES"	},
	{SECTION		,"SECTION"		},
	{NO_ACCESS_KW	,"NO_ACCESS"	},
	{SVC_READWRITE  ,"SVC_READWRITE"},
	{NO_USR_WRITE   ,"NO_USR_WRITE" },
	{FULL_ACCESS	,"FULL_ACCESS"  },
	{DOMAIN			,"DOMAIN"		},
	{FAULT			,"FAULT"		},
	{CACHEABLE_KW	,"CACHEABLE"	},
	{AND			,"AND"			},
	{BUFFERABLE_KW	,"BUFFERABLE"   },
	{NOT			,"NOT"			},
	{LARGEPAGES		,"LARGEPAGES"   },
	{SMALLPAGES		,"SMALLPAGES"   },
	{TINYPAGES		,"TINYPAGES"	},
	{POSTPROCESS	,"POSTPROCESS"  },
	{UNRECOGNISED   ,"UNRECOGNISED" }
};


//--function prototypes------------------------------------------------------
int  ValidateArgs (int argc, char **argv);
int  ParseRule (void);
int  WriteTableEntries (void);
int  WriteLevel1(void);
int  WriteLevel2(void);
int	  GetWord (char *buffer);
int	  GetWordIndex (char *buffer);
long atox( char *buffer);
int	  stricomp( char *original, char *match );
unsigned long CheckIsFinePage(unsigned long);
unsigned long ALIGN(unsigned long val, unsigned long align);

//--MAIN---------------------------------------------------------------------
int mmugen(unsigned long *mmu_addr, char *mmutab_str)
{
	global.MMUTableStr = mmutab_str;	
	global.MMUTableStrIndex = 0;
	global.MMUTableIndex = 0;
	
	global.state=0;
	Total_Fine_Pages=0;
	global.level2_page_size=0;
	global.level2_previous_region=0;
	global.level2_current_region=0;
	
	
	while( ParseRule() )
	{
		if(WriteTableEntries()==-1)
		{
/*			printf("Write table entries fail.\n"); */
			return FALSE;
		}
	}
/*	printf("MMU table generation complete.\n");		*/
	*mmu_addr = (unsigned long)global.base_address;
		
	return TRUE ;
}

//--PARSERULE----------------------------------------------------------------

int ParseRule (void)
{
	char buffer[20];
	static int	   notted = 0;

	while(GetWord(buffer))
	{
		switch( GetWordIndex(buffer) )
		{
			case BASE_ADDRESS:
				GetWord(buffer);
				global.base_address = (unsigned long *)atox(buffer);
				global.level2_base_address=((unsigned long)global.base_address)+0x4000;
				break;
			case LEVEL_KW:
				GetWord(buffer);
				global.state = atoi(buffer);
				/* check table_entry is a sensible value */
				if(global.table_entry != 0 && global.table_entry != 0x1000)
				{
					printf("Table data error - at end of Level 1, the table was %08lx bytes long (should be 0x4000)\n",global.table_entry<<2);
				}
				break;

			case VIRTUAL	 :
				GetWord(buffer);
				global.rule.v_base =  atox(buffer);
				break;

			case TO				  :
				GetWord(buffer);
				global.rule.v_end =  atox(buffer);
				break;

			case PHYSICAL	:
				GetWord(buffer) ;
				global.rule.p_base =  atox(buffer);
				break;

			case COARSEPAGES			   :
				global.rule.entrytype = 'C';
				global.access_index=0;
				global.rule.access[0] = 0 ;
				global.rule.access[1] = 0 ;
				global.rule.access[2] = 0 ;
				global.rule.access[3] = 0 ;
				global.rule.cb = 0 ;
				//completed rule, so implement it
				return 1 ;
						
			 case FINEPAGES			   :
				global.rule.entrytype = 'I';
				global.access_index=0;
				global.rule.access[0] = 0 ;
				global.rule.access[1] = 0 ;
				global.rule.access[2] = 0 ;
				global.rule.access[3] = 0 ;
				global.rule.cb = 0 ;
				Fine_Page_Region[Total_Fine_Pages].start=global.rule.v_base;
				Fine_Page_Region[Total_Fine_Pages].end=global.rule.v_end ;
				Total_Fine_Pages++;
				//completed rule, so implement it
				return 1 ;
						
			case SECTION	 :
				global.access_index=0;
				global.rule.entrytype = 'S';
				break ;
						
			case NO_ACCESS_KW:
										
				global.rule.access[global.access_index] = 0 ;
				global.access_index++;
				break ;

			case SVC_READWRITE:
				global.rule.access[global.access_index] = 1 ;
				global.access_index++;
				break ;

			case NO_USR_WRITE:
				global.rule.access[global.access_index] = 2 ;
				global.access_index++;
				break ;

			case FULL_ACCESS :
				global.rule.access[global.access_index] = 3 ;
				global.access_index++;
				break ;
			case DOMAIN		:
				GetWord(buffer);
				global.rule.domain=atox(buffer);
				break;

			case FAULT			   :
				global.rule.entrytype = 'F';
				global.access_index=0;
				global.rule.access[0] = 0 ;
				global.rule.access[1] = 0 ;
				global.rule.access[2] = 0 ;
				global.rule.access[3] = 0 ;
				global.rule.cb = 0 ;
				//completed rule, so implement it
				return 1 ;

			case CACHEABLE_KW:
				if(notted)
				{
					global.rule.cb &= 1;
					notted = 0;
				}
				else
				{
					global.rule.cb |= 2 ;
				}
				break ;
			case AND				 :
				/* ignore */ 
				break;
			case BUFFERABLE_KW:
				if(notted)
				{
					global.rule.cb &= 2;
					notted=0;
				}
				else
				{
					global.rule.cb |= 1 ;
				}
				//completed rule, so implement it
				return 1 ;

			case NOT				 :
				notted=1;
				break;

			case LARGEPAGES  :
				global.access_index=0;
				global.rule.entrytype = 'L';
				break ;
			case SMALLPAGES  :
				global.access_index=0;
				global.rule.entrytype = 'S';
				break ;
			case TINYPAGES  :
				global.access_index=0;
				global.rule.entrytype = 'T';
				break ;				
			case POSTPROCESS :
#if 0	//ted		
				GetWord(buffer); //skip ENTRY
				GetWord(buffer); //get address
				fseek( global.outfile, atox(buffer) ,SEEK_SET );
				GetWord(buffer); //skip EQUALS
				GetWord(buffer); //get value
				i = atox(buffer);
				fwrite( &i, 4, 1, global.outfile );
#endif				
				break ;

			case UNRECOGNISED:
				printf("Unrecognised word '%s'\n",buffer);
		}
	}

	// must have hit end of rule file without finding the end of a rule
	return 0;

}


//--WRITETABLEENTRIES--------------------------------------------------------

int WriteTableEntries (void)
{
	switch( global.state )
	{
		case 1: return WriteLevel1();
		case 2: return WriteLevel2();
		default: printf("MMU page table level not set\n");
				 return -1;
	}
}

//--WRITELEVEL1--------------------------------------------------------------

int WriteLevel1(void)
{
	unsigned long fields ;
	unsigned long physical ;
	unsigned long writevalue ;
	unsigned long i ;

	/* Check the table-entry number matches the virtual base address for this section */
	if( global.table_entry != (global.rule.v_base>>20) )
	{
		printf("MMU Table synchronisation error!\n  table_entry = %08lx, v_base = %08lx\n",
			global.table_entry, global.rule.v_base);
		return -1 ;
	}

	if (global.rule.entrytype == 'S' || global.rule.entrytype == 'F')
	{
		if(global.rule.entrytype == 'S')
		{
			fields = 0x12 | (global.rule.access[0]<<10)|(global.rule.domain<<5) | (global.rule.cb<<2) ;
		}
		else
		{
			fields = 0; //generate fault on access
		}
		physical = global.rule.p_base ;
		/* generate all the SECTION entries for this descriptor */
		for (i = global.rule.v_base; i<global.rule.v_end; )
		{
			writevalue = physical|fields ;
//			fwrite( &writevalue, 4, 1, global.outfile );
			global.base_address[global.MMUTableIndex++] = writevalue;
			global.table_entry++ ;
			physical += 0x100000 ;
			/* check for memory wrap-round */
			i+=0x100000 ;
			if (i==0x00000000) break;
		}
	}
	else
	{
		/* Coarse or Fine PAGE entry */
		 
		//level 1 table will maxmimum =4G/1M *4(B)=16KB,so level two table offset is from 0x4000(16K)
				
		if (global.rule.entrytype == 'C' )	  
		{
			//coarse page table split 1M to 256 entries and coarse page table base must 1k alignment (and of course if will be!)
			writevalue = 0x11 +(global.rule.domain<<5)+ global.level2_base_address;
//			fwrite( &writevalue, 4, 1, global.outfile );
			global.base_address[global.MMUTableIndex++] = writevalue;
			global.level2_base_address+=0x400;
			global.table_entry++ ;
		}
		else
		{
			//fine page table split 1M to 1024 entries and fine page table base must 4k alignment				
			global.level2_base_address=ALIGN(global.level2_base_address,SIZE_4K);
		
			writevalue = 0x13 + (global.rule.domain<<5)+global.level2_base_address;
//			fwrite( &writevalue, 4, 1, global.outfile );
			global.base_address[global.MMUTableIndex++] = writevalue;
			global.level2_base_address+=0x1000;
			global.table_entry++ ;
		}
	}  
	return 0 ;
}


//--WRITELEVEL2--------------------------------------------------------------

int WriteLevel2(void)
{
	unsigned long fields ;
	unsigned long physical ;
	unsigned long writevalue ;
	unsigned long i;
	int j,k,n;
	unsigned long IsFinePage=0;

		
	/* work out repeated data fields - note: this line would change if per-page access levels were to be supported*/
	if (global.rule.entrytype == 'T')
		fields = (global.rule.access[0]<< 4)|(global.rule.cb <<2)|0x3;
	else if (global.rule.entrytype == 'L')
		fields = (global.rule.access[3]<< 10)|(global.rule.access[2] << 8)|(global.rule.access[1] << 6)|(global.rule.access[0] << 4)|(global.rule.cb <<2)|0x1;
	else
		fields = (global.rule.access[3]<< 10)|(global.rule.access[2] << 8)|(global.rule.access[1] << 6)|(global.rule.access[0] << 4)|(global.rule.cb <<2)|0x2;

	   
	/* create entries for this descriptor */
	i = global.rule.v_base ;
	//check is belong to fine page region
	IsFinePage=CheckIsFinePage(i);
			  
	 if(IsFinePage)
		 global.level2_current_region=1;
	 else
		 global.level2_current_region=0;
					  
	 if(IsFinePage && global.level2_previous_region==0)				 
	 {
		//check 4k alignment
	 	if((global.level2_page_size & 0xfff)!=0)
		{
			//add stuff 0 to file	
			n=ALIGN(global.level2_page_size,SIZE_4K)-global.level2_page_size;
			for(k=0;k<n/4;k++)
			{
//				fwrite( &null_value, 4, 1, global.outfile );
				global.base_address[global.MMUTableIndex++] = writevalue;
			}
			global.level2_page_size+=n;					 	  
		}
	}		   	
					 	
	physical = global.rule.p_base ;

	while(i<global.rule.v_end)
	{
		if( global.rule.entrytype == 'L' )
		{
			writevalue = physical|fields ;

			if(IsFinePage)
			{ 
				for( j=64; j; j-- )
				{
//					fwrite( &writevalue, 4, 1, global.outfile );
					global.base_address[global.MMUTableIndex++] = writevalue;
					global.level2_page_size+=4;
				}
			}   
			else  
			{
				for( j=16; j; j-- )
				{
//					fwrite( &writevalue, 4, 1, global.outfile );
					global.base_address[global.MMUTableIndex++] = writevalue;
					global.level2_page_size+=4;
				}
			}
						
			physical += 0x10000;
			i += 0x10000;
		}
		else if( global.rule.entrytype == 'S' )
		{
			writevalue = physical|fields ;
		 	if(IsFinePage)
			{ 
				for( j=4; j; j-- )
				{ 
//					fwrite( &writevalue, 4, 1, global.outfile );
					global.base_address[global.MMUTableIndex++] = writevalue;
					global.level2_page_size+=4;
				}
			}
			else
			{
//				fwrite( &writevalue, 4, 1, global.outfile );
				global.base_address[global.MMUTableIndex++] = writevalue;
				global.level2_page_size+=4;
			}   
			physical += 0x1000;
			i += 0x1000;
		}
		else //tiny page
		{
			writevalue = physical|fields ;
//			fwrite( &writevalue, 4, 1, global.outfile );
			global.base_address[global.MMUTableIndex++] = writevalue;
			global.level2_page_size+=4;
			physical += 0x400;
			i += 0x400;   	
		}	
	}

	global.level2_previous_region=global.level2_current_region;
	return 0 ;
}


//--GETWORD------------------------------------------------------------------
// fetch next word, stepping over whitespace and comments
// return 0 if end-of-file

int	GetWord (char *buffer)
{
	int c;
	
	c = (int)global.MMUTableStr[global.MMUTableStrIndex++];
	
	while(c=='\t' || c==' ' || c=='/' || c=='\n')
	{
		if( c=='/') //start of comment
		{
			do
			{
				c = (int)global.MMUTableStr[global.MMUTableStrIndex++];
			} while( c!= '/' ) ;

			// get next character beyond comment
			c = (int)global.MMUTableStr[global.MMUTableStrIndex++];
		}

		while( c=='\t' || c==' ' || c == '\n') 
			c = (int)global.MMUTableStr[global.MMUTableStrIndex++];
	}

	// Check for end of file
	if( c=='\0' ) return 0;

	// Ok, should be at the start of a word
	while ( c!='\t' && c!=' ' && c!='\0' && c!= '\n')
	{
		*buffer++ = (char)c ; 
		c = (int)global.MMUTableStr[global.MMUTableStrIndex++];
	}

	//terminate word
	*buffer = '\0';
		
	return 1;
}

//--GETWORDINDEX-------------------------------------------------------------

int	  GetWordIndex (char *buffer)
{
	int index=0 ;
	
	while ( parsedata[index].index != UNRECOGNISED
			&& stricomp(parsedata[index].word,buffer) )
	{
		index++ ;
	}
		
	return index ;
}

//--ATOX---------------------------------------------------------------------

long atox( char *buffer)
{
	long v=0;
		
	while(*buffer)
	{
		v*=16;
		if(*buffer>='0' && *buffer<='9') v+= *buffer - '0';
		if(*buffer>='a' && *buffer<='f') v+= *buffer - 'a'+10;
		if(*buffer>='A' && *buffer<='F') v+= *buffer - 'A'+10;
		buffer++;
	}
	return v ;
}

//--STRICOMP-----------------------------------------------------------------
/*
		The program originally used stricmp, but this does
		not appear to be included in the ARM C library.
		
		This is not a true stricmp (or strcmpi), since only
		the "match" string is converted to upper case,
		allowing "original" to be in read-only memory.
*/

char ftoupper(char ch)
{
	if(( ch >= 'a') && (ch <= 'z'))
		return ch - 'a' + 'A';
	else
		return ch;
}

int stricomp( char *original, char *match )
{
	char *ptr = match ;

	while(*ptr)
	{
//		*ptr=toupper(*ptr) ;
		*ptr=ftoupper(*ptr) ;
		ptr++ ;
	}
		
	return strcmp(original,match) ;
}

//This code check if the address is belong to fine page region
// modify by fred chien 09/19/02
unsigned long CheckIsFinePage(unsigned long taddress)
{
	int i;
 
	for(i=0;i<Total_Fine_Pages;i++)	
	{
		if((taddress>=Fine_Page_Region[i].start)&&(taddress< Fine_Page_Region[i].end))
  			return 1;
  		else
  			continue; 	
 	}		
		
	return 0;	
}	

//this code will align the address to align
/**********************************************************************/

unsigned long ALIGN(unsigned long val, unsigned long align)
{
	/*
	 * if the value is zero, it is aligned, no matter what the size
	 */
	if (val == 0)
		return val;

	/*
	 * if the value is less than the alignment, return the alignment
	 */
	if (val < align)
		return align;

	/*
	 * finally, if there is need to move the value upwards, do so
	 */
	if ((val & ~(align - 1)) != 0)
		return (((val) + ((align) - 1)) & ~((align) - 1));
	else
		return val;
}
