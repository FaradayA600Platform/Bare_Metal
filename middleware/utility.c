#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tool/utility.h"

#define SCREEN_WIDTH		80


int substring(char **ptr, char *string, char *pattern)
{   
    int i;

    ptr[0]=(char *)strtok(string, pattern);
    for (i=0;ptr[i]!=NULL;++i)
    {   
    	ptr[i+1]=strtok(NULL,pattern);
    }
    return i;
}


unsigned int atonum(char *val_str)
{
	unsigned long address;

    if (val_str[0] =='0' && val_str[1] == 'x')
    {
		sscanf(val_str, "0x%x\n", &address);
	}
    else
    {
		address = atoi(val_str);
	}

    return address;
}

u32 get_dex(void)
{
#if 0
	char Buffer[32];
	char *start, *end;
	u32 i=0;

	gets(Buffer);
	start = Buffer;

	for (start = Buffer; (! isdigit(*start)) && (*start!=0) ;start++ );

	if (*start ==0){
		printf("format error !!!\n");
		return 0xFFFFFFFF;
	}

	for (end = start; isdigit(*end)  ;end++ );
	*end = 0;

	i = atoi( start );

	return(i);
#else
    return 0;

#endif
}

#define writel(val, addr)            		(*(volatile unsigned int *)(addr) = (val))

void PrintWelcomeMsg(struct burnin_cmd * cmd, int col_width)
{
	int i=0;
    char buf[256];
    char buf1[256];
    unsigned int len;
    unsigned int str_len;
  
    printf("  ----------------------------------------------------------------------------\n");
//	while(1);	
	writel(0x11111111,0x28080000);
	writel(0x22222222,0x28080004);
	writel(0x33333333,0x28080008);
	writel(0x44444444,0x2808000c);	
	writel(0x55555555,0x28080010);
	writel(0x66666666,0x28080014);
	writel(0x77777777,0x28080018);
	writel(0x88888888,0x2808001c);	
	writel(0x99999999,0x28080020);
	writel(0xaaaaaaaa,0x28080024);
	writel(0xbbbbbbbb,0x28080028);
	writel(0xcccccccc,0x2808002c);	
	writel(0xdddddddd,0x28080030);
	writel(0xeeeeeeee,0x28080034);
	writel(0xffffffff,0x28080038);
	writel(0xa5a5a5a5,0x2808003c);	
//	while(1);	
    memset(buf, ' ', sizeof(buf));
	
    len=0;
    for (i=0; cmd[i].burnin_routine != NULL; ++i)
    {
     	str_len = 5 + strlen(cmd[i].string);
     	if (len+str_len>SCREEN_WIDTH)
     	{
      		buf[len]='\0';   		
      		printf("%s\n", buf);
      		memset(buf, ' ', sizeof(buf));
      		len=0;
     	}
     	sprintf(buf1, "(%2d) %s", i+1, cmd[i].string);
     	memcpy(buf+len, buf1, str_len);
     	len += RoundUp(str_len, col_width);
	}
    if (len>0)
    {
     	buf[len]='\0';
     	printf("%s\n", buf);
    }
    //printf("\n");
}


void ManualTesting(struct burnin_cmd * cmd, int col_width, int have_back)
{
    unsigned int id;
    char Buffer[256];
    int argc;
    char *argv[64];
 	int cmd_size=0;
 
 	for (; cmd[cmd_size].burnin_routine; cmd_size++) ;
  
    while(1)
    {
     	PrintWelcomeMsg(cmd, col_width);
     	if (have_back)
     	{
     		printf("( 0) Back to Main Menu\n");
     	}
     	printf("\nCommand>>");
        gets(Buffer);
     	printf("\r");
 
  		argc = substring(argv, Buffer, " \r\n\t");
  		if (argc==1)
  		{
   			id = atoi(argv[0]);
   
   			if (id>0 && id<=cmd_size)
   			{
    			if ((int)cmd[id-1].burnin_routine != 0x1)
					cmd[id-1].burnin_routine();
   			}
   			if (have_back && id==0)
   			{
    			return;
    		}
  		}
    }
}

int mem_dump(unsigned int addr, int size)
{
        int i,j,count;
        count=0;
        printf("%s %d\n",__func__,size);
        while(count<size){
                printf("0x%08x ",addr+count);
                for(i=0;i<4;i++,count+=4)
                        printf("%08X ",*((unsigned int*)(addr+count)));
                printf("\n");
        }
        printf("%s end\n",__func__);

}
