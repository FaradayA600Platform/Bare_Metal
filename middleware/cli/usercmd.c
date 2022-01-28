#include <stdio.h>
#include "icli.h"
#include "porting.h"
//#include "cpe.h"
#include "stdlib.h"
#include "SoFlexible.h"
#include "serial.h"
//#include "fLib.h"

/* extern */
extern char     *atob(char *str,unsigned int *ptrnum,unsigned int base,int seg);		/* ascii to binary */
extern int      help(int,char **,struct com_tab_lst);

#define MAX_CMND_TBL_SZ	60

struct com_tab_lst work_cmd_table[MAX_CMND_TBL_SZ];  /* ram copy of command table */
struct com_tab_lst *work_ctp;		/* current pointer into working */

void init_commands(struct com_tab_lst *ct_ptr)
{
 	struct com_tab_lst *cp;
	for( cp = ct_ptr ; cp->cmdt_name ; cp++ )
	{ 
	    *( struct com_tab_lst *)work_ctp++ = *(struct com_tab_lst*)cp;
	    if ( cp->cmdt_init_routine != 0 )
		    (*cp->cmdt_init_routine)();
    }
	work_ctp->cmdt_name = 0;
}


static unsigned int print_addr=0x100,print_sz=1;
int read_32b(int argc,char *argv[])
{
    unsigned int    *mem_read=0;
    unsigned        address=0,sz;
    unsigned int    size=1;
    int i;

    if(argc!=2&&argc!=3&&argc!=1)
    {
        printf("Incorrect parameter!\r\n");
        return 0;
    }

    if(argc!=1)
    {
        if(argv[1][0]!='0'||argv[1][1]!='x')
        {
            printf("Incorrect Parameter\r\n");    
            return  0; 
        }
        if (*atob(argv[1], &address, 16, 0)) 
        { 
            printf("bad address: %s\n", argv[1]);
            return  0; 
        }
        mem_read = (unsigned int *)address;
        
        if(argc>2)
        {
            if (*atob(argv[2], &sz, 16, 0)) 
            { 
                printf("bad value: %s\n", argv[1]);
                return  0; 
            }
            size=(unsigned int)sz;
            if(size<1)
                size=1;            
        }
        else
            size=1;
                
    }
    else
    {
        if(!print_addr)
            mem_read=(unsigned int *)0x0;
        else
            mem_read = (unsigned int *)print_addr;        

        size=print_sz;
        if(size<1)
            size=1;
    }
    
    printf("0x%x : ",(UINT32)mem_read);
    
    for(i=0;i<size;i++)
    {
        if( ((unsigned int)mem_read&0x0000000f)==0x0 && (i!=0))
        {
            printf("\r\n");
            printf("0x%x : ",(UINT32)mem_read);
            printf("%8x ",*mem_read);
        }
        else
            printf("%8x ",*mem_read);
            
        mem_read++;
    }
    print_sz = size;
    print_addr = (unsigned int)mem_read;
    
    printf("\r\n");
    return 1;   
}

int write_32b(int argc,char *argv[])
{   
    unsigned int    *mem_write,mem_value;
    unsigned        address,value;

    if (*atob(argv[1], &address, 16,0)) 
    { 
        printf("bad address: %s\n", argv[1]);
        return  0; 
    }

    if (*atob(argv[2], &value,16,0)) 
    { 
        printf("bad value: %s\r\n", argv[2]);
        return  0; 
    }

    mem_value = (unsigned int)value;
    mem_write = (unsigned int *)address;   
    *mem_write = mem_value;

/*    printf("write addr 0x%x : 0x%x\r\n",mem_write,*mem_write);    */
    return 1;   
}

extern jmp_code(unsigned int);
int go_cmd(int argc,char *argv[])
{
    unsigned int    *mem_write;
    unsigned        address;

    if (*atob(argv[1], &address, 16,0)) 
    { 
        printf("bad address: %s\n", argv[1]);
        return  0; 
    }       
    mem_write = (unsigned int *)address;    
    
    jmp_code(mem_write);
    return  1; 
}

int get_baudrate(unsigned baudrate)
{
    unsigned int value=0;
    switch(baudrate)
    {
        case 9600:
            value = BAUD_9600;
            break;
        case 14400:
            value = BAUD_14400;
            break;
        case 38400:
            value = BAUD_38400;
            break;
        case 57600:
            value = BAUD_57600;
            break;
        case 115200:
            value = BAUD_115200;
            break;                                    
        default:
            value =0;
            break;
    }
    return value;
}

extern void fLib_InitSerial(unsigned int port, unsigned int baudRate);
int baud_change(int argc,char *argv[])
{
    unsigned int    baudrate;
    unsigned        address;
    unsigned int    baud_value=0;
    
    if (*atob(argv[1], &address, 10,0)) 
    { 
        printf("bad baudrate: %s\n", argv[1]);
        return  0; 
    }       
    baudrate = (unsigned int)address;
    
    if((baud_value=get_baudrate(baudrate))==0)
    {
        printf("Not support baudrate:%d\r\n",baudrate);
        return 1;
    }
    printf("Set baudrate to %d\r\n",baudrate);
    fLib_SerialInit(DebugSerialPort, baud_value, 0, 0, 8);
    //fLib_InitSerial((unsigned int)HOST_COMPORT,(int)baud_value);
    return 1;
}

#if 0
#define DEBUG_PORT 0x902ffffc
void Output2DebugPort(unsigned num)
{
   *((unsigned* ) DEBUG_PORT) = num;
}
#else
extern void Output2DebugPort(unsigned num);
#endif

unsigned char dummy_data[1000];
int dummy1(int i,char c)
{
	i=c+2;
	if((i%100)>10)
		i=i>>c;
	else
		i=i<<c;	
	dummy_data[0]=(char)i;
	dummy_data[2]=(char)(i+c);
	return dummy_data[2];
}

unsigned int crc;
void cnt_crc(unsigned int cnt)
{
	int k;
	for(k=0;k<cnt;k++)
	{
		crc+=dummy_data[k];
	}
}


//data load to 0x500000
unsigned char *out; //0x808000
void to_out(unsigned int cnt)
{
	int k;
	for(k=0;k<cnt;k++)
	{
		*out=dummy_data[k];
		out++;
	}
}

extern char exit_notify;

void exit_cmd(int argc,char *argv[])
{
    exit_notify=1;
}

static struct com_tab_lst command_tab[] = {    
    { "go",go_cmd,0,                      "go address                     (go to address)"},    
    { "r32",read_32b,0,                   "r32 address num                (read 32 bit data)"},
    { "w32",write_32b,0,                  "w32 address data               (write 32 bit data)"},
    { "baud",  baud_change,0,	         "baud buadrate                  (change baudrate)"},
    { "exit",  exit_cmd,0,	             "exit                           (exit CLI)"},
	{ "help", help,0,	                 "help" },
	{ 0, 0, 0, "" }
};


/*
** init_cmd_lst_tab()
*/
void init_cmd_lst_tab(void)
{
	work_ctp = work_cmd_table;
	work_ctp->cmdt_name = 0;
	init_commands(command_tab);
}

