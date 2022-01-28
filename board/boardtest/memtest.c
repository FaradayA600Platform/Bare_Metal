/***************************************************************************
* Copyright  Faraday Technology Corp 2008-2012.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:memtest.c                                                             *
* Description: SoFlexible board specfic routine                            *
* Author: Fred Chien                                                       *
****************************************************************************/


#include <stdio.h>
#include "SoFlexible.h"
#include "memtest.h"


void MemTestByte(int start, int size, unsigned fill_content);
void MemTestByte1(int start, int size, unsigned fill_content);
void MemTestHalfWord(int start, int size, unsigned fill_content);
void MemTestHalfWord1(int start, int size, unsigned fill_content);
void MemTestWord(int start, int size, unsigned fill_content);
void MemTestWord1(int start, int size, unsigned fill_content);


#define TADDR(base,row,bank,bank_shift,column) ((base)+((row)<<16)+((bank)<<(bank_shift))+(column));

static void write_addr(unsigned base,unsigned row, unsigned bank, unsigned bank_shift, unsigned offset, int inverse){ 
    unsigned addr;
    addr =  TADDR(base,row,bank,bank_shift,offset);
    REG32(addr) = (inverse==1) ? (~addr) : addr;
//    REG32(addr) = addr;
//    printf("write mem[0x%x]=0x%x \n",addr, REG32(addr) );
}
static void check_addr(unsigned base,unsigned row, unsigned bank, unsigned int bank_shift, unsigned offset, int inverse){ 
    unsigned addr,data;
//    addr = base+(row<<16)+(bank<<13);
    addr =  TADDR(base,row,bank,bank_shift,offset);
    data = (inverse==1) ? (~addr) : addr;
//    printf("mem[0x%x]=0x%x \n",addr, REG32(addr) );
    if (REG32(addr) != data){
        printf("mem[0x%x]=0x%x error!!\n",addr, REG32(addr) );
        while (1);
    }
}


extern int DMA_NoLLD(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
UINT32 DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/ fixed --> 0/2
UINT32 DstCtrl,   // dest address change : Inc/ fixed --> 0/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode       // Normal/Hardwire,   0/1
);

void DMA_Start(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 LinkAddr,  // Link-List address
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 *Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32/64/128 bits -> 0/1/2/3/4
UINT32 DstWidth,  // dest width 8/16/32/64/128 bits -> 0/1/2/3/4
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode       // Normal/Hardwire,   0/1
);
extern void EnableDMAChannel(INT32 Channel);
extern void DMA_WaitIntStatus(UINT32 Channel);

UINT32 GetDMAChannelIntStatus(INT32 Channel);
extern void *DMA_Register;

#ifdef DMAC_FTDMAC030_pa_base
#define DMA_FACTOR 1
#else
#define DMA_FACTOR 0
#endif


void mem_addr_bank_row_test(unsigned base,unsigned int bank_shift){
    unsigned int addr, row, bank;
    unsigned int size = 0x02000000;

    unsigned int offset;
    static int inverse=-1; //-1 =>not inverse  ; 1 => inverse

#if DMA_FACTOR
    printf("DMA_Start src=%x dst=%x size=%x \n", CONFIG_MEM_BASE,  (FREE_MEM_BASE&0xf0000000)+0x20000000, size );
//    DMA_NoLLD( 0,   CONFIG_MEM_BASE,  (FREE_MEM_BASE&0xf0000000)+0x10000000, size, 3, 3, 4, 0, 0, 3, 0  );
    DMA_Start( 0, 0xf0000000  ,CONFIG_MEM_BASE,  (FREE_MEM_BASE&0xf0000000)+0x20000000, &size, 2, 2, 4, 0, 0, 3, 0  );

    EnableDMAChannel(0);
#endif

    for(row=0;row<0x400;row+=3){
        for(offset=0;offset<0x100;offset+=4){
            for(bank=0;bank<8;bank++){
//                addr =  TADDR(base,row,bank,offset);
//                REG32(addr) = (inverse==1) ? (~addr) : addr;
                write_addr(base,row,bank,bank_shift,offset,inverse); 
            }
            for(bank=0;bank<4;bank++){
//                addr =  TADDR(base,row+1,bank,offset);
//                REG32(addr) = addr;
                write_addr(base,row+1,bank,bank_shift,offset,inverse); 
            }
        }
        
        for(offset=0;offset<0x100;offset+=4){
            for(bank=4;bank<8;bank++){
//                addr =  TADDR(base,row+1,bank,offset);
//                REG32(addr) = addr;
                write_addr(base,row+1,bank,bank_shift,offset,inverse); 
            }
            for(bank=0;bank<8;bank++){
//                addr =  TADDR(base,row+2,bank,offset);
//                REG32(addr) = addr;
                write_addr(base,row+2,bank,bank_shift,offset,inverse); 
            }
        }
//        printf(" %x ",addr );
    }

#if DMA_FACTOR
    printf("dma finish status = %x \n",GetDMAChannelIntStatus(0) );
#endif

    for(row=0;(row+2)<0x400;row+=3){
        for(offset=0;offset<0x100;offset+=4){
            for(bank=0;bank<8;bank++){
                check_addr(base,row, bank,bank_shift,offset,inverse); 
            }
            for(bank=0;bank<4;bank++){
                check_addr(base,row+1, bank,bank_shift,offset,inverse); 
            }
        }
        
        for(offset=0;offset<0x100;offset+=4){
            for(bank=4;bank<8;bank++){
                check_addr(base,row+1, bank,bank_shift,offset,inverse); 
            }
            for(bank=0;bank<8;bank++){
                check_addr(base,row+2, bank,bank_shift,offset,inverse); 
            }
        }
    }

#if DMA_FACTOR
    printf("dma finish status = %x \n",GetDMAChannelIntStatus(0) );
    DMA_WaitIntStatus(0);
#endif
    inverse *= -1;

}
//row = addr[29:16]    bank=[15:13]
void mem_addr_test(){
    unsigned int addr, row, bank;
    unsigned int base = (FREE_MEM_BASE&0xff000000)+0x01000000;
    unsigned int i;
    
#if DMA_FACTOR
//    dmac303_main();
	DMA_Register = (void *)DMAC_FTDMAC030_pa_base[4];
#endif

    printf("mem test:  base=%x\n", base);

#if 0
    __mdelay(1);
    REG32(DDRC_FTDDR3030_1_PA_BASE) = 0x00E49f03;
    __mdelay(1000);
    printf("row = addr[29:16]    bank=[15:13]\n");
#endif
    for (i=0;i<=0xffff;i++){
        printf("test %d\n",i);
        mem_addr_bank_row_test(base,8);
    }


#if 0
    REG32(DDRC_FTDDR3030_1_PA_BASE) = 0x00E49f23;
    __mdelay(1000);
#endif
#if 0
    printf("row = addr[29:16]    bank=[10:8]\n");
    mem_addr_bank_row_test(base,8);
#endif

}

void Mem_QC_Main()
{

	unsigned int low_start;
	unsigned int iter;
	int test_base, test_size, test_iteration;
		
		
	
	//low_start=test_base;
	//low_start=0x8000000;
	low_start=FREE_MEM_BASE;
	test_size=0x100000;
	printf("test (Mem 0x%x~0x%x)\n",low_start, low_start+test_size-1);
	test_iteration=1;

    printf(" Mem Test Start ....\n");

/*	
	printf("Mem test by WORD = Address ...\n");
    printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
    MemTestWord(low_start, test_size, FILL_CONTENT_ADDRESS); 
	
	printf("(1)write all, (2)read , (3)compare\n");
    MemTestWord1(low_start, test_size, FILL_CONTENT_ADDRESS);     
	
	printf("Mem test by WORD = 0x%8x ...\n",DATA_PATTERN4);
	printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
    MemTestWord(low_start, test_size, DATA_PATTERN4); 
	


    printf(" Mem Test Start .... cache on\n");
#ifdef CONFIG_ARMV8
       enable_cache();
#else
	CPU_AllCache_Enable();	
#endif
*/
	printf("Mem test by WORD = Address ...\n");
    printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
    MemTestWord(low_start, test_size, FILL_CONTENT_ADDRESS); 
	
	printf("(1)write all, (2)read , (3)compare\n");
    MemTestWord1(low_start, test_size, FILL_CONTENT_ADDRESS);     
	
	printf("Mem test by WORD = 0x%8x ...\n",DATA_PATTERN4);
	printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
    MemTestWord(low_start, test_size, DATA_PATTERN4); 
	
	printf("(1)write all, (2)read , (3)compare\n");
    MemTestWord1(low_start, test_size, DATA_PATTERN4);     
	
	printf("Mem test by WORD = 0x%8x ...\n",DATA_PATTERN5 );	
	printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
    MemTestWord(low_start, test_size, DATA_PATTERN5);   
    printf("(1)write all, (2)read , (3)compare\n");
    MemTestWord1(low_start, test_size, DATA_PATTERN4);
    
	printf("Test pass!\r\n");

	

}

//void Mem_Test_Main(int test_base,int test_size,int test_iteration)
void Mem_Test_Main(int test_iteration)
{
    unsigned int low_start;
    unsigned int iter;
    char buf[256];
    unsigned int default_start = FREE_MEM_BASE;
    int test_size=0x100000;
	
            
    printf("Please input the memory start address\n");
    printf("'0' for default address and size: (Mem 0x%x~0x%x)\r\n", default_start, default_start+test_size-1);
    gets(buf);
	low_start = atonum(buf);
    if (low_start==0x0)	//default
    {
        low_start = default_start;
		test_iteration=1;
    }
    else
    {
        printf("Please input the memory size:\n");
        gets(buf);
        test_size = atonum(buf);
        printf("Please input the test iteration:\n");
        gets(buf);
        test_iteration = atonum(buf);
    }
    printf(" Start Mem Test 0x%x ~ 0x%x ....\n", low_start, low_start+test_size-1);

    for(iter=0;iter<test_iteration;iter++)
    {
        //Word Bank Width 32 
        printf("Mem test by WORD = Address ...\n");
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestWord(low_start, test_size, FILL_CONTENT_ADDRESS); 
    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestWord1(low_start, test_size, FILL_CONTENT_ADDRESS);     
    
        printf("Mem test by WORD = 0x%8x ...\n",DATA_PATTERN4);
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestWord(low_start, test_size, DATA_PATTERN4); 
    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestWord1(low_start, test_size, DATA_PATTERN4);     
    
        printf("Mem test by WORD = 0x%8x ...\n",DATA_PATTERN5 );        
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestWord(low_start, test_size, DATA_PATTERN5);   
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestWord1(low_start, test_size, DATA_PATTERN4);     

        //HalfWord Bank Width 16 
        printf("Mem test by HALFWORD = Address ...\n");
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestHalfWord(low_start, test_size, FILL_CONTENT_ADDRESS); 
    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestHalfWord1(low_start, test_size, FILL_CONTENT_ADDRESS);     
    
        printf("Mem test by HALFWORD = 0x%8x ...\n",DATA_PATTERN2);
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestHalfWord(low_start, test_size, DATA_PATTERN2); 
    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestHalfWord1(low_start, test_size, DATA_PATTERN2);     
    
        printf("Mem test by HALFWORD = 0x%8x ...\n",DATA_PATTERN3 );    
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestHalfWord(low_start, test_size, DATA_PATTERN3);    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestHalfWord1(low_start, test_size, DATA_PATTERN3);  

        //Byte Bank Width 8 
        printf("Mem test by BYTE = Address ...\n");
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestByte(low_start, test_size, FILL_CONTENT_ADDRESS); 
    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestByte1(low_start, test_size, FILL_CONTENT_ADDRESS);     
    
        printf("Mem test by BYTE = 0x%8x ...\n",DATA_PATTERN0);
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestByte(low_start, test_size, DATA_PATTERN0); 
    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestByte1(low_start, test_size, DATA_PATTERN0);     
    
        printf("Mem test by BYTE = 0x%8x ...\n",DATA_PATTERN1 );        
        printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestByte(low_start, test_size, DATA_PATTERN1);    
        printf("(1)write all, (2)read , (3)compare\n");
        MemTestByte1(low_start, test_size, DATA_PATTERN1);  
    }
    printf("Test pass!\r\n");
}

void MemTestByte(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned char* ptr;
    
    end = start + size;
    
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
   
    address = start;
    
    // clear all content
    while (address < end)
    {
        outb(address,0x0);
    
      
        if( (* ((unsigned char *) address)) != 0x0){
            nResultStatus = EXIT_TUBE_FAIL;
        }
        address += 1;
    }
    
    address = start;
    
    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;
          
      ptr = (unsigned char*) & content;
       outb(address,ptr[0]);
       outb(address+1,ptr[1]);
       outb(address+2,ptr[2]);
       outb(address+3,ptr[3]); 
                    
          
      ptr = (unsigned char*) &CompareContent;
      ptr[0] = * ((unsigned char *) address);
      address++;
      ptr[1] = * ((unsigned char *) address);      
      address++;      
      ptr[2] = * ((unsigned char *) address);      
      address++;      
      ptr[3] = * ((unsigned char *) address);      
      address++;      
      
      if ( CompareContent  != content){
          nResultStatus = EXIT_TUBE_FAIL;
          printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);
      }
      
      num++;
      if (num >= 0xff){
          num = 0;
          show ++;
                 
      }
    }


    if (nResultStatus == EXIT_TUBE_FAIL)   
    printf("Test Fail\n");

}

void MemTestByte1(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned char* ptr;
    
    end = start + size;
     
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
    
    address = start;
    
    // clear all content
    while (address < end){
         outb(address,0x0);
    
      
      if( (* ((unsigned char *) address)) != 0x0){
          nResultStatus = EXIT_TUBE_FAIL;
      }
      address += 1;
    }
    
    address = start;
     // write all content
    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;

    
          
       ptr = (unsigned char*) & content;
       outb(address,ptr[0]);
       outb(address+1,ptr[1]);
       outb(address+2,ptr[2]);
       outb(address+3,ptr[3]); 
      address +=4;
    }
     address = start;

    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;
          
          
      ptr = (unsigned char*) &CompareContent;
      ptr[0] = * ((unsigned char *) address);
      address++;
      ptr[1] = * ((unsigned char *) address);      
      address++;      
      ptr[2] = * ((unsigned char *) address);      
      address++;      
      ptr[3] = * ((unsigned char *) address);      
      address++;      
      
      if ( CompareContent  != content){
          nResultStatus = EXIT_TUBE_FAIL;
          printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);                
      }
      
      num++;
      if (num >= 0xff){
          num = 0;
          show ++;
                  
      }
    }
    
  

    if (nResultStatus == EXIT_TUBE_FAIL)   
    printf("Test Fail\n");

}

void MemTestHalfWord(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned short* ptr;
    
    end = start + size;
        
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
   
    address = start;
    
    // clear all content
    while (address < end){
        outhw(address,0x0);
        
     if(inhw(address)!=0x0){      
     
          nResultStatus = EXIT_TUBE_FAIL;
      }
      address += 2;
    }
    
    
    address = start;
    
    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;
          
     
      outhw(address,content);
      outhw(address+2,content>>16);
              
          
      ptr = (unsigned short*) &CompareContent;      
      ptr[0] = * ((unsigned short *) address);
      address+=2;
      ptr[1] = * ((unsigned short *) address);      
      address+=2;      
      
      if ( CompareContent  != content){
          nResultStatus = EXIT_TUBE_FAIL;
          printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);          
      }
      
      num++;
      if (num >= 0xff){
          num = 0;
          show ++;
                  
      }
    }
    
   
    
    if (nResultStatus == EXIT_TUBE_FAIL)
    printf("Test Fail\n");

}


void MemTestHalfWord1(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned short* ptr;
    
    end = start + size;

    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
  
    address = start;
    
 
    // clear all content
    while (address < end){
        outhw(address,0x0);
        
     if(inhw(address)!=0x0){      
     
          nResultStatus = EXIT_TUBE_FAIL;
      }
      address += 2;
    }
    
    
    address = start;
    
     
    // write all content
    while (address < end){
        
        
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;
  
          
     
      outhw(address,content);
      outhw(address+2,content>>16);
      address+=4;
      
    }
    
    
     address = start;

     

    //read and compare    
    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;
          
           ptr = (unsigned short*) &CompareContent;      
      ptr[0] = * ((unsigned short *) address);
      address+=2;
      ptr[1] = * ((unsigned short *) address);      
      address+=2;      
      
      if ( CompareContent  != content){
          nResultStatus = EXIT_TUBE_FAIL;
          printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);          
      }
      
      num++;
      if (num >= 0xff){
          num = 0;
          show ++;
                  
      }
    }
    
    
  
    
    if (nResultStatus == EXIT_TUBE_FAIL)
    printf("Test Fail\n");

}

void MemTestWord(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, data;
    int nResultStatus;
    unsigned end;
    
        end = start + size;
  
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;

    address = start;
    
    // clear all content
    while (address < end){
      * ((unsigned *) address) = 0x0;
   
      data = * ((unsigned *) address);
      if( data != 0x0){
          nResultStatus = EXIT_TUBE_FAIL;
      }
      address += 4;
    }
    
    
    address = start;
    
    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;

    
          
      * ((unsigned *) address) = content;      
          
      if ( (* ((unsigned *) address))  != content){
          printf("error address:%x, error data:%x, correct dara:%x\n", address, (* ((unsigned *) address)), content);
          nResultStatus = EXIT_TUBE_FAIL;
      }
      
      
      address += 4;
      num++;
      if (num >= 0xff){
          num = 0;
          show ++;
                  
      }
    }
    
  
    
    if (nResultStatus == EXIT_TUBE_FAIL)
    printf("Test Fail\n");

}

void MemTestWord1(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content;
    int nResultStatus;
    unsigned end;
    
        end = start + size;
  
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
    
  
    address = start;
    
    // write all content
    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;

    
          
      * ((unsigned *) address) = content;      
      address += 4;
    }
    
    address = start;

     

    //read and compare    
    while (address < end){
      if (fill_content == FILL_CONTENT_ADDRESS)
          content = address;
      else
          content = fill_content;
          
    
         
      if ( (* ((unsigned *) address))  != content){
          printf("error address:%x, error data:%x, correct dara:%x\n", address, (* ((unsigned *) address)), content);            
          nResultStatus = EXIT_TUBE_FAIL;
      }
      
      
      address += 4;
      num++;
      if (num >= 0xff){
          num = 0;
          show ++;
                  
      }
    }
    
  

    if (nResultStatus == EXIT_TUBE_FAIL)
    printf("Test Fail\n");

}
