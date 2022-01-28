#include <stdint.h>
#include <stdio.h>
#include "cpat/FLibARM.h"

/*
 * AXI_APB_BASE and MessageBox_BASE is defined in FLibARM.h
 */
//#define AXI_APB_BASE  0x90a00000 
//#define MessageBox_BASE AXI_APB_BASE + 0x00080000
//#define DEBUG_ADDR MessageBox_BASE+0x8

#define SEMIHOSTING 0

// ---------------------------------------------------------------------------
// func: lock
// ---------------------------------------------------------------------------
void lock(void)
{}

// ---------------------------------------------------------------------------
// func: unlock
// ---------------------------------------------------------------------------
void unlock(void)
{}


// ---------------------------------------------------------------------------
// func: sw
// ---------------------------------------------------------------------------
void sw(WORD addr, WORD data, BYTE type)
{
    switch( type ){
        case HSIZE_BYTE:
            *((BYTE  *) addr) = (BYTE) data;
            break;
        case HSIZE_HWORD:
            *((HWORD *) addr) = (HWORD) data;
            break;
        case HSIZE_WORD:
            *((WORD  *) addr) = (WORD) data;
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// func: finish
// ---------------------------------------------------------------------------
void finish(void)
{
//    sw(MessageBox_BASE,0x01234567,HSIZE_WORD);
     
//    sw(MessageBox_BASE,0x01234567,HSIZE_WORD);
//    printf("%s:%s \n",__FILE__,__func__);
//    printf("%s \n",__func__);
    fLib_printf("%s \n",__func__);
}

// ---------------------------------------------------------------------------
// func: pass
// ---------------------------------------------------------------------------
void pass(void)
{
#if SEMIHOSTING
    _swiwrite0("pass\n");
    while ( 1 );
#else
//    sw(MessageBox_BASE,0x01234568,HSIZE_WORD);
//    printf("%s:%s \n",__FILE__,__func__);
    fLib_printf("%s:%s \n",__FILE__,__func__);
#endif
}

// ---------------------------------------------------------------------------
// func: fail
// ---------------------------------------------------------------------------
void fail(void)
{
#if SEMIHOSTING
    _swiwrite0("fail\n");
    while ( 1 );
#else
//    sw(MessageBox_BASE,0x01234569,HSIZE_WORD);
//    printf("%s:%s \n",__FILE__,__func__);
    fLib_printf("%s:%s \n",__FILE__,__func__);
#endif
}

// ---------------------------------------------------------------------------
// func: warn
// ---------------------------------------------------------------------------
void warn(void)
{
//    sw(MessageBox_BASE,0x01234570,HSIZE_WORD);
//    printf("%s:%s \n",__FILE__,__func__);
    fLib_printf("%s:%s \n",__FILE__,__func__);
}

// ---------------------------------------------------------------------------
// func: srdata
// ---------------------------------------------------------------------------
void srdata (WORD addr, WORD *data)
{
  *data=*(WORD *) addr;
}

// ---------------------------------------------------------------------------
// func: srdata_hw
// ---------------------------------------------------------------------------
void srdata_hw(WORD addr, WORD *data)
{
  *data=*(HWORD *)addr; 
}

// ---------------------------------------------------------------------------
// func: srdata_b
// ---------------------------------------------------------------------------
void srdata_b(WORD addr, WORD *data)
{
  *data=*(BYTE *)addr;
}

// ---------------------------------------------------------------------------
// func: sr
// ---------------------------------------------------------------------------
void sr(WORD addr, BYTE type)
{
    volatile int   i;

    switch( type ){
        case HSIZE_BYTE:
            i = *((BYTE *) addr);
            break; 
        case HSIZE_HWORD:
            i = *((HWORD *) addr);
            break;
        case HSIZE_WORD:
            i = *((WORD  *) addr);
            break;
        default:
            break;
    } 
}

// ---------------------------------------------------------------------------
// func: src
// ---------------------------------------------------------------------------
void src( WORD addr, WORD data, BYTE type)
{
    switch(type){
        case HSIZE_BYTE:
            if( data != *((BYTE  *) addr))
                fail();
            break;
        case HSIZE_HWORD:
            if( data != *((HWORD *) addr))
                fail();
            break;
        case HSIZE_WORD:
            if( data != *((WORD  *) addr)) 
                fail();
            break;    
    }  
}

// ---------------------------------------------------------------------------
// func: bw
// ---------------------------------------------------------------------------
void bw(WORD f_addr,WORD f_data0,WORD f_data1,WORD f_data2, WORD f_data3)
{
   WORD addr[4];
   int i;
   int mask=3;
   int incr;
   int offset;
   incr=1<<2;
   mask=mask<<2;
   for (i=0;i<=3;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (HSIZE_WORD){
     case HSIZE_BYTE:
           *((BYTE  *) addr[0]) = (BYTE) f_data0;
           *((BYTE  *) addr[1]) = (BYTE) f_data1;
           *((BYTE  *) addr[2]) = (BYTE) f_data2;
           *((BYTE  *) addr[3]) = (BYTE) f_data3;
           break;
    case HSIZE_HWORD:
           *((HWORD *) addr[0]) = (HWORD) f_data0;
           *((HWORD *) addr[1]) = (HWORD) f_data1;
           *((HWORD *) addr[2]) = (HWORD) f_data2;
           *((HWORD *) addr[3]) = (HWORD) f_data3;
           break;
    case HSIZE_WORD:
           *((WORD  *) addr[0]) = (WORD) f_data0;
           *((WORD  *) addr[1]) = (WORD) f_data1;
           *((WORD  *) addr[2]) = (WORD) f_data2;
           *((WORD  *) addr[3]) = (WORD) f_data3;
           break;
    default:
           break;
    }
}

// ---------------------------------------------------------------------------
// func: br
// ---------------------------------------------------------------------------
void br( WORD f_addr )
{
   WORD addr[4];
   int i;
   int mask=3;
   int incr;
   int offset;
   volatile WORD data0,data1,data2,data3;
   incr=1<<2;
   mask=mask<<2;
   for (i=0;i<=3;i++) {
       addr[i]=f_addr;
       offset=f_addr & mask;
       if (offset==mask) {
         f_addr=~mask & f_addr;
       }
       else { f_addr=f_addr+incr;}
   }

   switch (HSIZE_WORD){
       case HSIZE_BYTE:
           data0=*((BYTE  *) addr[0]) ;
           data1=*((BYTE  *) addr[1]) ;
           data2=*((BYTE  *) addr[2]) ;
           data3=*((BYTE  *) addr[3]) ;
           break;
       case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           break;
       case HSIZE_WORD:
           data0=*((WORD  *) addr[0]) ;
           data1=*((WORD  *) addr[1]) ;
           data2=*((WORD  *) addr[2]) ;
           data3=*((WORD  *) addr[3]) ;
           break;
       default:
           break;
   }
}

// ---------------------------------------------------------------------------
// func: brc
// ---------------------------------------------------------------------------
void brc(WORD f_addr, WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3)
{
   WORD addr[4];
   int i;
   int mask=3;
   int incr;
   int offset;
   WORD data0,data1,data2,data3;
   incr=1<<2;
   mask=mask<<2;
   for (i=0;i<=3;i++) {
       addr[i]=f_addr;
       offset=f_addr & mask;
       if (offset==mask) {
         f_addr=~mask & f_addr;
       }
       else { f_addr=f_addr+incr;}
   }
   switch (HSIZE_WORD){
       case HSIZE_BYTE:
           data0=*((BYTE  *) addr[0]) ;
           data1=*((BYTE  *) addr[1]) ;
           data2=*((BYTE  *) addr[2]) ;
           data3=*((BYTE  *) addr[3]) ;
           break;
       case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           break;
       case HSIZE_WORD:
           data0=*((WORD  *) addr[0]) ;
           data1=*((WORD  *) addr[1]) ;
           data2=*((WORD  *) addr[2]) ;
           data3=*((WORD  *) addr[3]) ;
           break;
       default:
           break;
   }
   if (data0 != f_data0 || data1 != f_data1 || data2 != f_data2 || data3 != f_data3){
     fail();
   }
} 

// ---------------------------------------------------------------------------
// func: idle
// ---------------------------------------------------------------------------
void idle( unsigned long addr)
{   
   volatile WORD data;
   data=*(WORD *)addr;
}

// ---------------------------------------------------------------------------
// func: idlen
// ---------------------------------------------------------------------------
void idlen( unsigned long int addr,  int idle_count)
{
   int i;
   WORD data;
   for (i=0;i<=idle_count;i++)
   {
     data=*(volatile WORD *)addr;
   }
   /* do nothing */
}

// ---------------------------------------------------------------------------
// func: bw_i4
// ---------------------------------------------------------------------------
void bw_i4( WORD f_size, WORD f_addr, HWORD f_bsycnt, WORD f_data0,
            WORD f_data1, WORD f_data2, WORD f_data3)
{
  WORD addr[4];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=3;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
   case  HSIZE_BYTE : 
            *((BYTE *)addr[0])=(BYTE)f_data0;
            *((BYTE *)addr[1])=(BYTE)f_data1;
            *((BYTE *)addr[2])=(BYTE)f_data2;
            *((BYTE *)addr[3])=(BYTE)f_data3;
            break;                  
   case  HSIZE_HWORD :
            *((HWORD *)addr[0])=(HWORD)f_data0;
            *((HWORD *)addr[1])=(HWORD)f_data1;
            *((HWORD *)addr[2])=(HWORD)f_data2;
            *((HWORD *)addr[3])=(HWORD)f_data3;
            break;
   case  HSIZE_WORD :  
            *((WORD *)addr[0])=(WORD)f_data0;
            *((WORD *)addr[1])=(WORD)f_data1;
            *((WORD *)addr[2])=(WORD)f_data2;
            *((WORD *)addr[3])=(WORD)f_data3;
            break;
        default :
            break; 
  }
}

// ---------------------------------------------------------------------------
// func: bw_i8
// ---------------------------------------------------------------------------
void bw_i8( WORD f_size, WORD f_addr, HWORD f_bsycnt, 
            WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3,
            WORD f_data4, WORD f_data5, WORD f_data6, WORD f_data7)
{
  WORD addr[8];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=7;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
   case  HSIZE_BYTE :
            *(BYTE *)addr[0]=(BYTE)f_data0;
            *(BYTE *)addr[1]=(BYTE)f_data1;
            *(BYTE *)addr[2]=(BYTE)f_data2;
            *(BYTE *)addr[3]=(BYTE)f_data3;
            *(BYTE *)addr[4]=(BYTE)f_data4;
            *(BYTE *)addr[5]=(BYTE)f_data5;
            *(BYTE *)addr[6]=(BYTE)f_data6;
            *(BYTE *)addr[7]=(BYTE)f_data7;
            break;
   case  HSIZE_HWORD:
            *(HWORD *)addr[0]=(HWORD)f_data0;
            *(HWORD *)addr[1]=(HWORD)f_data1;
            *(HWORD *)addr[2]=(HWORD)f_data2;
            *(HWORD *)addr[3]=(HWORD)f_data3;
            *(HWORD *)addr[4]=(HWORD)f_data4;
            *(HWORD *)addr[5]=(HWORD)f_data5;
            *(HWORD *)addr[6]=(HWORD)f_data6;
            *(HWORD *)addr[7]=(HWORD)f_data7;
            break;
   case  HSIZE_WORD :
            *(WORD *)addr[0]=(WORD)f_data0;
            *(WORD *)addr[1]=(WORD)f_data1;
            *(WORD *)addr[2]=(WORD)f_data2;
            *(WORD *)addr[3]=(WORD)f_data3;
            *(WORD *)addr[4]=(WORD)f_data4;
            *(WORD *)addr[5]=(WORD)f_data5;
            *(WORD *)addr[6]=(WORD)f_data6;
            *(WORD *)addr[7]=(WORD)f_data7;
            break;
        default :
            break;
  }
}

// ---------------------------------------------------------------------------
// func: bw_i16
// ---------------------------------------------------------------------------
void bw_i16( WORD f_size, WORD f_addr, HWORD f_bsycnt, 
             WORD f_data0, WORD f_data1,WORD f_data2, WORD f_data3,
             WORD f_data4, WORD f_data5, WORD f_data6, WORD f_data7,
             WORD f_data8,WORD f_data9,WORD f_data10,WORD f_data11,
             WORD f_data12,WORD f_data13, WORD f_data14,WORD f_data15)
{
  WORD addr[16];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=15;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
   case  HSIZE_BYTE :
            *(BYTE *)addr[0]=(BYTE )f_data0;
            *(BYTE *)addr[1]=(BYTE )f_data1;
            *(BYTE *)addr[2]=(BYTE )f_data2;
            *(BYTE *)addr[3]=(BYTE )f_data3;
            *(BYTE *)addr[4]=(BYTE )f_data4;
            *(BYTE *)addr[5]=(BYTE )f_data5;
            *(BYTE *)addr[6]=(BYTE )f_data6;
            *(BYTE *)addr[7]=(BYTE )f_data7;
            *(BYTE *)addr[8]=(BYTE )f_data8;
            *(BYTE *)addr[9]=(BYTE )f_data9;
            *(BYTE *)addr[10]=(BYTE )f_data10;
            *(BYTE *)addr[11]=(BYTE )f_data11;
            *(BYTE *)addr[12]=(BYTE )f_data12;
            *(BYTE *)addr[13]=(BYTE )f_data13;
            *(BYTE *)addr[14]=(BYTE )f_data14;
            *(BYTE *)addr[15]=(BYTE )f_data15;
            break;
   case  HSIZE_HWORD:
            *(HWORD *)addr[0]=(HWORD )f_data0;
            *(HWORD *)addr[1]=(HWORD )f_data1;
            *(HWORD *)addr[2]=(HWORD )f_data2;
            *(HWORD *)addr[3]=(HWORD )f_data3;
            *(HWORD *)addr[4]=(HWORD )f_data4; 
            *(HWORD *)addr[5]=(HWORD )f_data5;
            *(HWORD *)addr[6]=(HWORD )f_data6;
            *(HWORD *)addr[7]=(HWORD )f_data7;
            *(HWORD *)addr[8]=(HWORD )f_data8;
            *(HWORD *)addr[9]=(HWORD )f_data9;
            *(HWORD *)addr[10]=(HWORD )f_data10;
            *(HWORD *)addr[11]=(HWORD )f_data11;
            *(HWORD *)addr[12]=(HWORD )f_data12;
            *(HWORD *)addr[13]=(HWORD )f_data13;
            *(HWORD *)addr[14]=(HWORD )f_data14;
            *(HWORD *)addr[15]=(HWORD )f_data15;
            break;
   case  HSIZE_WORD :
            *(WORD *)addr[0]=(WORD )f_data0;
            *(WORD *)addr[1]=(WORD )f_data1;
            *(WORD *)addr[2]=(WORD )f_data2;
            *(WORD *)addr[3]=(WORD )f_data3;
            *(WORD *)addr[4]=(WORD )f_data4;
            *(WORD *)addr[5]=(WORD )f_data5;
            *(WORD *)addr[6]=(WORD )f_data6;
            *(WORD *)addr[7]=(WORD )f_data7;
            *(WORD *)addr[8]=(WORD )f_data8;
            *(WORD *)addr[9]=(WORD )f_data9;
            *(WORD *)addr[10]=(WORD )f_data10;
            *(WORD *)addr[11]=(WORD )f_data11;
            *(WORD *)addr[12]=(WORD )f_data12;
            *(WORD *)addr[13]=(WORD )f_data13;
            *(WORD *)addr[14]=(WORD )f_data14;
            *(WORD *)addr[15]=(WORD )f_data15;
            break;
        default :
            break;
  }
}

// ---------------------------------------------------------------------------
// func: br_i4
// ---------------------------------------------------------------------------
void br_i4( WORD f_size, WORD f_addr, HWORD f_bsycnt)
{
  volatile WORD  rdata3, rdata2, rdata1, rdata0;
  WORD addr[4];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=3;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
     case HSIZE_BYTE  :
       rdata0=*(BYTE *)addr[0];
       rdata1=*(BYTE *)addr[1];
       rdata2=*(BYTE *)addr[2];
       rdata3=*(BYTE *)addr[3];
       break;
     case HSIZE_HWORD :
       rdata0=*(HWORD *)addr[0];
       rdata1=*(HWORD *)addr[1];
       rdata2=*(HWORD *)addr[2];
       rdata3=*(HWORD *)addr[3];
       break;
     case HSIZE_WORD  :
       rdata0=*(WORD *)addr[0];
       rdata1=*(WORD *)addr[1];
       rdata2=*(WORD *)addr[2];
       rdata3=*(WORD *)addr[3];
       break;  
     default          :
       break;
  }
}

// ---------------------------------------------------------------------------
// func: br_i8
// ---------------------------------------------------------------------------
void br_i8( WORD f_size, WORD f_addr, HWORD f_bsycnt)
{
  volatile WORD rdata0, rdata1, rdata2, rdata3,
                rdata4, rdata5, rdata6, rdata7;
  WORD addr[8];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=7;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
     case HSIZE_BYTE  :
       rdata0=*(BYTE *)addr[0];
       rdata1=*(BYTE *)addr[1];
       rdata2=*(BYTE *)addr[2];
       rdata3=*(BYTE *)addr[3];
       rdata4=*(BYTE *)addr[4];
       rdata5=*(BYTE *)addr[5];
       rdata6=*(BYTE *)addr[6];
       rdata7=*(BYTE *)addr[7];
       break;
     case HSIZE_HWORD :  
       rdata0=*(HWORD *)addr[0];
       rdata1=*(HWORD *)addr[1];
       rdata2=*(HWORD *)addr[2];
       rdata3=*(HWORD *)addr[3];
       rdata4=*(HWORD *)addr[4];
       rdata5=*(HWORD *)addr[5];
       rdata6=*(HWORD *)addr[6];
       rdata7=*(HWORD *)addr[7];
       break;
     case HSIZE_WORD  :
       rdata0=*(WORD *)addr[0];
       rdata1=*(WORD *)addr[1];
       rdata2=*(WORD *)addr[2];
       rdata3=*(WORD *)addr[3];
       rdata4=*(WORD *)addr[4];
       rdata5=*(WORD *)addr[5];
       rdata6=*(WORD *)addr[6];
       rdata7=*(WORD *)addr[7];
       break;
     default          :
       break;
  }

}

// ---------------------------------------------------------------------------
// func: br_i16
// ---------------------------------------------------------------------------
void br_i16( WORD f_size, WORD f_addr, HWORD f_bsycnt)
{
  volatile WORD rdata0, rdata1, rdata2, rdata3, 
     rdata4, rdata5, rdata6, rdata7,rdata8,rdata9, 
     rdata10, rdata11, rdata12, rdata13, rdata14, rdata15;
  WORD addr[16];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=15;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
     case HSIZE_BYTE  :
       rdata0=*(BYTE *)addr[0];
       rdata1=*(BYTE *)addr[1];
       rdata2=*(BYTE *)addr[2];
       rdata3=*(BYTE *)addr[3];
       rdata4=*(BYTE *)addr[4];
       rdata5=*(BYTE *)addr[5];
       rdata6=*(BYTE *)addr[6];
       rdata7=*(BYTE *)addr[7];
       rdata8=*(BYTE *)addr[8];
       rdata9=*(BYTE *)addr[9];
       rdata10=*(BYTE *)addr[10];
       rdata11=*(BYTE *)addr[11];
       rdata12=*(BYTE *)addr[12];
       rdata13=*(BYTE *)addr[13];
       rdata14=*(BYTE *)addr[14];
       rdata15=*(BYTE *)addr[15];
       break;
     case HSIZE_HWORD :
       rdata0=*(HWORD *)addr[0];
       rdata1=*(HWORD *)addr[1];
       rdata2=*(HWORD *)addr[2];
       rdata3=*(HWORD *)addr[3];
       rdata4=*(HWORD *)addr[4];
       rdata5=*(HWORD *)addr[5];
       rdata6=*(HWORD *)addr[6];
       rdata7=*(HWORD *)addr[7];
       rdata8=*(HWORD *)addr[8];
       rdata9=*(HWORD *)addr[9];
       rdata10=*(HWORD *)addr[10];
       rdata11=*(HWORD *)addr[11];
       rdata12=*(HWORD *)addr[12];
       rdata13=*(HWORD *)addr[13];
       rdata14=*(HWORD *)addr[14];
       rdata15=*(HWORD *)addr[15];
       break;
     case HSIZE_WORD  :
       rdata0=*(WORD *)addr[0];
       rdata1=*(WORD *)addr[1];
       rdata2=*(WORD *)addr[2];
       rdata3=*(WORD *)addr[3];
       rdata4=*(WORD *)addr[4];
       rdata5=*(WORD *)addr[5];
       rdata6=*(WORD *)addr[6];
       rdata7=*(WORD *)addr[7];
       rdata8=*(WORD *)addr[8];
       rdata9=*(WORD *)addr[9];
       rdata10=*(WORD *)addr[10];
       rdata11=*(WORD *)addr[11];
       rdata12=*(WORD *)addr[12];
       rdata13=*(WORD *)addr[13];
       rdata14=*(WORD *)addr[14];
       rdata15=*(WORD *)addr[15];
       break;
     default          :
       break;
  }
}

// ---------------------------------------------------------------------------
// func: brc_i4
// ---------------------------------------------------------------------------
void brc_i4( WORD f_size, WORD f_addr, HWORD f_bsycnt, 
             WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3)
{
  WORD  rdata3, rdata2, rdata1, rdata0;
  WORD addr[4];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=3;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
     case HSIZE_BYTE  :
       rdata0=*(BYTE *)addr[0];
       rdata1=*(BYTE *)addr[1];
       rdata2=*(BYTE *)addr[2];
       rdata3=*(BYTE *)addr[3];
       break;
     case HSIZE_HWORD :
       rdata0=*(HWORD *)addr[0];
       rdata1=*(HWORD *)addr[1];
       rdata2=*(HWORD *)addr[2];
       rdata3=*(HWORD *)addr[3];
       break;
     case HSIZE_WORD  :
       rdata0=*(WORD *)addr[0];
       rdata1=*(WORD *)addr[1];
       rdata2=*(WORD *)addr[2];
       rdata3=*(WORD *)addr[3];
       break;
     default          :
       break;
  }
    if( (rdata0 != f_data0) || (rdata1 != f_data1) || (rdata2 != f_data2) ||(rdata3 != f_data3)  )
     fail();
}

// ---------------------------------------------------------------------------
// func: brc_i8
// ---------------------------------------------------------------------------
void brc_i8( WORD f_size, WORD f_addr, HWORD f_bsycnt,
             WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3,
             WORD f_data4, WORD f_data5, WORD f_data6, WORD f_data7)
{
  WORD rdata7, rdata6, rdata5, rdata4, rdata3, rdata2, rdata1, rdata0;
  WORD addr[8];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=7;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
     case HSIZE_BYTE  :
       rdata0=*(BYTE *)addr[0];
       rdata1=*(BYTE *)addr[1];
       rdata2=*(BYTE *)addr[2];
       rdata3=*(BYTE *)addr[3];
       rdata4=*(BYTE *)addr[4];
       rdata5=*(BYTE *)addr[5];
       rdata6=*(BYTE *)addr[6];
       rdata7=*(BYTE *)addr[7];
       break;
     case HSIZE_HWORD :
       rdata0=*(HWORD *)addr[0];
       rdata1=*(HWORD *)addr[1];
       rdata2=*(HWORD *)addr[2];
       rdata3=*(HWORD *)addr[3];
       rdata4=*(HWORD *)addr[4];
       rdata5=*(HWORD *)addr[5];
       rdata6=*(HWORD *)addr[6];
       rdata7=*(HWORD *)addr[7];
       break;
     case HSIZE_WORD  :
       rdata0=*(WORD *)addr[0];
       rdata1=*(WORD *)addr[1];
       rdata2=*(WORD *)addr[2];
       rdata3=*(WORD *)addr[3];
       rdata4=*(WORD *)addr[4];
       rdata5=*(WORD *)addr[5];
       rdata6=*(WORD *)addr[6];
       rdata7=*(WORD *)addr[7];
       break;
     default          :
       break;
  }
    if( (rdata0 != f_data0) || (rdata1 != f_data1) || (rdata2 != f_data2) ||(rdata3 != f_data3)  )
     fail(); 
    if( (rdata4 != f_data4) || (rdata5 != f_data5) || (rdata6 != f_data6) ||(rdata7 != f_data7)  )
     fail();
}

// ---------------------------------------------------------------------------
// func: brc_i16
// ---------------------------------------------------------------------------
void brc_i16 ( WORD f_size, WORD f_addr, HWORD f_bsycnt,
               WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3,
               WORD f_data4, WORD f_data5, WORD f_data6, WORD f_data7,
               WORD f_data8, WORD f_data9, WORD f_data10, WORD f_data11,
               WORD f_data12, WORD f_data13, WORD f_data14, WORD f_data15)
{
  WORD rdata0, rdata1, rdata2, rdata3, rdata4, rdata5, rdata6, rdata7,
       rdata8, rdata9, rdata10, rdata11, rdata12, rdata13, rdata14, rdata15;
  WORD addr[16];
  int i;
  int incr;
  incr=1<<f_size;
  for (i=0;i<=15;i++) {
    addr[i]=f_addr;
    f_addr=f_addr+incr;
  }
  switch (f_size) {
     case HSIZE_BYTE  :
       rdata0=*(BYTE *)addr[0];
       rdata1=*(BYTE *)addr[1];
       rdata2=*(BYTE *)addr[2];
       rdata3=*(BYTE *)addr[3];
       rdata4=*(BYTE *)addr[4];
       rdata5=*(BYTE *)addr[5];
       rdata6=*(BYTE *)addr[6];
       rdata7=*(BYTE *)addr[7];
       rdata8=*(BYTE *)addr[8];
       rdata9=*(BYTE *)addr[9];
       rdata10=*(BYTE *)addr[10];
       rdata11=*(BYTE *)addr[11];
       rdata12=*(BYTE *)addr[12];
       rdata13=*(BYTE *)addr[13];
       rdata14=*(BYTE *)addr[14];
       rdata15=*(BYTE *)addr[15];
       break;
     case HSIZE_HWORD :
       rdata0=*(HWORD *)addr[0];
       rdata1=*(HWORD *)addr[1];
       rdata2=*(HWORD *)addr[2];
       rdata3=*(HWORD *)addr[3];
       rdata4=*(HWORD *)addr[4];
       rdata5=*(HWORD *)addr[5];
       rdata6=*(HWORD *)addr[6];
       rdata7=*(HWORD *)addr[7];
       rdata8=*(HWORD *)addr[8];
       rdata9=*(HWORD *)addr[9];
       rdata10=*(HWORD *)addr[10];
       rdata11=*(HWORD *)addr[11];
       rdata12=*(HWORD *)addr[12];
       rdata13=*(HWORD *)addr[13];
       rdata14=*(HWORD *)addr[14];
       rdata15=*(HWORD *)addr[15];
       break;
     case HSIZE_WORD  :
       rdata0=*(WORD *)addr[0];
       rdata1=*(WORD *)addr[1];
       rdata2=*(WORD *)addr[2];
       rdata3=*(WORD *)addr[3];
       rdata4=*(WORD *)addr[4];
       rdata5=*(WORD *)addr[5];
       rdata6=*(WORD *)addr[6];
       rdata7=*(WORD *)addr[7];
       rdata8=*(WORD *)addr[8];
       rdata9=*(WORD *)addr[9];
       rdata10=*(WORD *)addr[10];
       rdata11=*(WORD *)addr[11];
       rdata12=*(WORD *)addr[12];
       rdata13=*(WORD *)addr[13];
       rdata14=*(WORD *)addr[14];
       rdata15=*(WORD *)addr[15];
       break;
     default          :
       break;
  }
    if( (rdata0 != f_data0) || (rdata1 != f_data1) || (rdata2 != f_data2) ||(rdata3 != f_data3))
     fail();
    if( (rdata4 != f_data4) || (rdata5 != f_data5) || (rdata6 != f_data6) ||(rdata7 != f_data7))
     fail();
    if( (rdata8 != f_data8) || (rdata9 != f_data9) || (rdata10 != f_data10) ||(rdata11 != f_data11))
     fail();
    if( (rdata12 != f_data12) || (rdata13 != f_data13) || (rdata14 != f_data14) ||(rdata15 != f_data15))
     fail();


}

// ---------------------------------------------------------------------------
// func: bw_w4
// ---------------------------------------------------------------------------
void bw_w4( WORD f_size, WORD f_addr, HWORD f_bsycnt, 
            WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3)
{
   WORD addr[4];
   int i;
   int mask=3;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=3;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           *((BYTE  *) addr[0]) = (BYTE) f_data0;
           *((BYTE  *) addr[1]) = (BYTE) f_data1;
           *((BYTE  *) addr[2]) = (BYTE) f_data2;
           *((BYTE  *) addr[3]) = (BYTE) f_data3;
           break;
    case HSIZE_HWORD:
           *((HWORD *) addr[0]) = (HWORD) f_data0;
           *((HWORD *) addr[1]) = (HWORD) f_data1;
           *((HWORD *) addr[2]) = (HWORD) f_data2;
           *((HWORD *) addr[3]) = (HWORD) f_data3;
           break;
    case HSIZE_WORD:
           *((WORD  *) addr[0]) = (WORD) f_data0;
           *((WORD  *) addr[1]) = (WORD) f_data1;
           *((WORD  *) addr[2]) = (WORD) f_data2;
           *((WORD  *) addr[3]) = (WORD) f_data3; 
           break;
    default:
           break;
   }
}

// ---------------------------------------------------------------------------
// func: bw_w8
// ---------------------------------------------------------------------------
void bw_w8( WORD f_size, WORD f_addr, HWORD f_bsycnt,
            WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3,
            WORD f_data4, WORD f_data5, WORD f_data6, WORD f_data7)
{
   WORD addr[8];
   int i;
   int mask=7;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=7;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           *((BYTE  *) addr[0]) = (BYTE) f_data0;
           *((BYTE  *) addr[1]) = (BYTE) f_data1;
           *((BYTE  *) addr[2]) = (BYTE) f_data2;
           *((BYTE  *) addr[3]) = (BYTE) f_data3;
           *((BYTE  *) addr[4]) = (BYTE) f_data4;
           *((BYTE  *) addr[5]) = (BYTE) f_data5;
           *((BYTE  *) addr[6]) = (BYTE) f_data6;
           *((BYTE  *) addr[7]) = (BYTE) f_data7;
           break;
    case HSIZE_HWORD:
           *((HWORD *) addr[0]) = (HWORD) f_data0;
           *((HWORD *) addr[1]) = (HWORD) f_data1;
           *((HWORD *) addr[2]) = (HWORD) f_data2;
           *((HWORD *) addr[3]) = (HWORD) f_data3;
           *((HWORD *) addr[4]) = (HWORD) f_data4;
           *((HWORD *) addr[5]) = (HWORD) f_data5;
           *((HWORD *) addr[6]) = (HWORD) f_data6;
           *((HWORD *) addr[7]) = (HWORD) f_data7;
           break;
    case HSIZE_WORD:
           *((WORD  *) addr[0]) = (WORD) f_data0;
           *((WORD  *) addr[1]) = (WORD) f_data1;
           *((WORD  *) addr[2]) = (WORD) f_data2;
           *((WORD  *) addr[3]) = (WORD) f_data3;
           *((WORD  *) addr[4]) = (WORD) f_data4;
           *((WORD  *) addr[5]) = (WORD) f_data5;
           *((WORD  *) addr[6]) = (WORD) f_data6;
           *((WORD  *) addr[7]) = (WORD) f_data7;
           break;
    default:
           break;
   }
}

// ---------------------------------------------------------------------------
// func: bw_w16
// ---------------------------------------------------------------------------
void bw_w16( WORD f_size, WORD f_addr, HWORD f_bsycnt,
             WORD f_data0,  WORD f_data1,  WORD f_data2,  WORD f_data3,
             WORD f_data4,  WORD f_data5,  WORD f_data6,  WORD f_data7,
             WORD f_data8,  WORD f_data9,  WORD f_data10, WORD f_data11,
             WORD f_data12, WORD f_data13, WORD f_data14, WORD f_data15)
{
   WORD addr[16];
   int i;
   int mask=0xf;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=15;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           *((BYTE  *) addr[0]) = (BYTE) f_data0;
           *((BYTE  *) addr[1]) = (BYTE) f_data1;
           *((BYTE  *) addr[2]) = (BYTE) f_data2;
           *((BYTE  *) addr[3]) = (BYTE) f_data3;
           *((BYTE  *) addr[4]) = (BYTE) f_data4;
           *((BYTE  *) addr[5]) = (BYTE) f_data5;
           *((BYTE  *) addr[6]) = (BYTE) f_data6;
           *((BYTE  *) addr[7]) = (BYTE) f_data7;
           *((BYTE  *) addr[8]) = (BYTE) f_data8;
           *((BYTE  *) addr[9]) = (BYTE) f_data9;
           *((BYTE  *) addr[10]) = (BYTE) f_data10;
           *((BYTE  *) addr[11]) = (BYTE) f_data11;
           *((BYTE  *) addr[12]) = (BYTE) f_data12;
           *((BYTE  *) addr[13]) = (BYTE) f_data13;
           *((BYTE  *) addr[14]) = (BYTE) f_data14;
           *((BYTE  *) addr[15]) = (BYTE) f_data15;
           break;
    case HSIZE_HWORD:
           *((HWORD *) addr[0]) = (HWORD) f_data0;
           *((HWORD *) addr[1]) = (HWORD) f_data1;
           *((HWORD *) addr[2]) = (HWORD) f_data2;
           *((HWORD *) addr[3]) = (HWORD) f_data3;
           *((HWORD *) addr[4]) = (HWORD) f_data4;
           *((HWORD *) addr[5]) = (HWORD) f_data5;
           *((HWORD *) addr[6]) = (HWORD) f_data6;
           *((HWORD *) addr[7]) = (HWORD) f_data7;
           *((HWORD *) addr[8]) = (HWORD) f_data8;
           *((HWORD *) addr[9]) = (HWORD) f_data9;
           *((HWORD *) addr[10]) = (HWORD) f_data10;
           *((HWORD *) addr[11]) = (HWORD) f_data11;
           *((HWORD *) addr[12]) = (HWORD) f_data12;
           *((HWORD *) addr[13]) = (HWORD) f_data13;
           *((HWORD *) addr[14]) = (HWORD) f_data14;
           *((HWORD *) addr[15]) = (HWORD) f_data15;
           break;
    case HSIZE_WORD:
           *((WORD  *) addr[0]) = (WORD) f_data0;
           *((WORD  *) addr[1]) = (WORD) f_data1;
           *((WORD  *) addr[2]) = (WORD) f_data2;
           *((WORD  *) addr[3]) = (WORD) f_data3;
           *((WORD  *) addr[4]) = (WORD) f_data4;
           *((WORD  *) addr[5]) = (WORD) f_data5;
           *((WORD  *) addr[6]) = (WORD) f_data6;
           *((WORD  *) addr[7]) = (WORD) f_data7;
           *((WORD  *) addr[8]) = (WORD) f_data8;
           *((WORD  *) addr[9]) = (WORD) f_data9;
           *((WORD  *) addr[10]) = (WORD) f_data10;
           *((WORD  *) addr[11]) = (WORD) f_data11;
           *((WORD  *) addr[12]) = (WORD) f_data12;
           *((WORD  *) addr[13]) = (WORD) f_data13;
           *((WORD  *) addr[14]) = (WORD) f_data14;
           *((WORD  *) addr[15]) = (WORD) f_data15;
           break;
    default:
           break;
   }
}

// ---------------------------------------------------------------------------
// func: br_w4
// ---------------------------------------------------------------------------
void br_w4( WORD f_size, WORD f_addr, HWORD f_bsycnt)
{
   WORD addr[4];
   int i;
   int mask=3;
   int incr;
   int offset;
   volatile WORD data0,data1,data2,data3;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=3;i++) {
       addr[i]=f_addr;
       offset=f_addr & mask;
       if (offset==mask) {
       f_addr=~mask & f_addr;
       }
       else { f_addr=f_addr+incr;}
   }
   switch (f_size){
       case HSIZE_BYTE:
           data0=*((BYTE  *) addr[0]) ;
           data1=*((BYTE  *) addr[1]) ;
           data2=*((BYTE  *) addr[2]) ;
           data3=*((BYTE  *) addr[3]) ;
           break;
       case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           break;
       case HSIZE_WORD:
           data0=*((WORD  *) addr[0]) ;
           data1=*((WORD  *) addr[1]) ;
           data2=*((WORD  *) addr[2]) ;
           data3=*((WORD  *) addr[3]) ; 
           break;
       default:
           break;
   }
}

// ---------------------------------------------------------------------------
// func: br_w8
// ---------------------------------------------------------------------------
void br_w8 (WORD f_size, WORD f_addr, HWORD f_bsycnt)
{
   WORD addr[8];
   volatile WORD data0,data1,data2,data3,data4,data5,data6,data7;
   int i;
   int mask=7;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=7;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           data0=*((BYTE *) addr[0]) ;
           data1=*((BYTE *) addr[1]) ;
           data2=*((BYTE *) addr[2]) ;
           data3=*((BYTE *) addr[3]) ;
           data4=*((BYTE *) addr[4]) ;
           data5=*((BYTE *) addr[5]) ;
           data6=*((BYTE *) addr[6]) ;
           data7=*((BYTE *) addr[7]) ;
           break;
    case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           data4=*((HWORD *) addr[4]) ;
           data5=*((HWORD *) addr[5]) ;
           data6=*((HWORD *) addr[6]) ;
           data7=*((HWORD *) addr[7]) ;
           break;
    case HSIZE_WORD:
           data0=*((WORD *) addr[0]) ;
           data1=*((WORD *) addr[1]) ;
           data2=*((WORD *) addr[2]) ;
           data3=*((WORD *) addr[3]) ;
           data4=*((WORD *) addr[4]) ;
           data5=*((WORD *) addr[5]) ;
           data6=*((WORD *) addr[6]) ;
           data7=*((WORD *) addr[7]) ;
           break;
    default:
           break;
   }
}

// ---------------------------------------------------------------------------
// func: br_w16
// ---------------------------------------------------------------------------
void br_w16(WORD f_size, WORD f_addr, HWORD f_bsycnt)
{
   WORD addr[16];
   volatile WORD data0,data1,data2,data3,data4,data5,data6,data7,
                 data8,data9,data10,data11,data12,data13,data14,data15;
   int i;
   int mask=0xf;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=15;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           data0=*((BYTE *) addr[0]) ;
           data1=*((BYTE *) addr[1]) ;
           data2=*((BYTE *) addr[2]) ;
           data3=*((BYTE *) addr[3]) ;
           data4=*((BYTE *) addr[4]) ;
           data5=*((BYTE *) addr[5]) ;
           data6=*((BYTE *) addr[6]) ;
           data7=*((BYTE *) addr[7]) ;
           data8=*((BYTE *) addr[8]) ;
           data9=*((BYTE *) addr[9]) ;
           data10=*((BYTE *) addr[10]) ;
           data11=*((BYTE *) addr[11]) ;
           data12=*((BYTE *) addr[12]) ;
           data13=*((BYTE *) addr[13]) ;
           data14=*((BYTE *) addr[14]) ;
           data15=*((BYTE *) addr[15]) ;
           break;
    case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           data4=*((HWORD *) addr[4]) ;
           data5=*((HWORD *) addr[5]) ;
           data6=*((HWORD *) addr[6]) ;
           data7=*((HWORD *) addr[7]) ;
           data8=*((HWORD *) addr[8]) ;
           data9=*((HWORD *) addr[9]) ;
           data10=*((HWORD *) addr[10]) ;
           data11=*((HWORD *) addr[11]) ;
           data12=*((HWORD *) addr[12]) ;
           data13=*((HWORD *) addr[13]) ;
           data14=*((HWORD *) addr[14]) ;
           data15=*((HWORD *) addr[15]) ;
           break;
    case HSIZE_WORD:
           data0=*((WORD *) addr[0]) ;
           data1=*((WORD *) addr[1]) ;
           data2=*((WORD *) addr[2]) ;
           data3=*((WORD *) addr[3]) ;
           data4=*((WORD *) addr[4]) ;
           data5=*((WORD *) addr[5]) ;
           data6=*((WORD *) addr[6]) ;
           data7=*((WORD *) addr[7]) ;
           data8=*((WORD *) addr[8]) ;
           data9=*((WORD *) addr[9]) ;
           data10=*((WORD *) addr[10]) ;
           data11=*((WORD *) addr[11]) ;
           data12=*((WORD *) addr[12]) ;
           data13=*((WORD *) addr[13]) ;
           data14=*((WORD *) addr[14]) ;
           data15=*((WORD *) addr[15]) ;
           break;
     default :
           break;
   
  }
}

// ---------------------------------------------------------------------------
// func: brc_w4
// ---------------------------------------------------------------------------
void brc_w4(WORD f_size, WORD f_addr, HWORD f_bsycnt,
            WORD f_data0,WORD f_data1,WORD f_data2,WORD f_data3)
{
   WORD addr[4];
   volatile WORD data0,data1,data2,data3;
   int i;
   int mask=0x3;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=3;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           data0=*((BYTE *) addr[0]) ;
           data1=*((BYTE *) addr[1]) ;
           data2=*((BYTE *) addr[2]) ;
           data3=*((BYTE *) addr[3]) ;
           break;
   
    case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           break;
    case HSIZE_WORD:
           data0=*((WORD *) addr[0]) ;
           data1=*((WORD *) addr[1]) ;
           data2=*((WORD *) addr[2]) ;
           data3=*((WORD *) addr[3]) ;
           break;
     default :
           break;
     }
   if (data0!=f_data0 | data1!=f_data1 || data2!=f_data2 || data3!=f_data3)
     fail();
   
}

// ---------------------------------------------------------------------------
// func: brc_w8
// ---------------------------------------------------------------------------
void brc_w8(WORD f_size, WORD f_addr, HWORD f_bsycnt,
            WORD f_data0,WORD f_data1,WORD f_data2,WORD f_data3,
            WORD f_data4,WORD f_data5,WORD f_data6,WORD f_data7)
{
   WORD addr[8];
   volatile WORD data0,data1,data2,data3,data4,data5,data6,data7;
   int i;
   int mask=0x7;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=7;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           data0=*((BYTE *) addr[0]) ;
           data1=*((BYTE *) addr[1]) ;
           data2=*((BYTE *) addr[2]) ;
           data3=*((BYTE *) addr[3]) ;
           data4=*((BYTE *) addr[4]) ;
           data5=*((BYTE *) addr[5]) ;
           data6=*((BYTE *) addr[6]) ;
           data7=*((BYTE *) addr[7]) ;
           break;
    case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           data4=*((HWORD *) addr[4]) ;
           data5=*((HWORD *) addr[5]) ;
           data6=*((HWORD *) addr[6]) ;
           data7=*((HWORD *) addr[7]) ;
           break;
    case HSIZE_WORD:
           data0=*((WORD *) addr[0]) ;
           data1=*((WORD *) addr[1]) ;
           data2=*((WORD *) addr[2]) ;
           data3=*((WORD *) addr[3]) ;
           data4=*((WORD *) addr[4]) ;
           data5=*((WORD *) addr[5]) ;
           data6=*((WORD *) addr[6]) ;
           data7=*((WORD *) addr[7]) ;
          break;
     default :
           break;
     }
   if (data0!=f_data0 | data1!=f_data1 || data2!=f_data2 || 
      data3!=f_data3 || data4!=f_data4 || data5!=f_data5 ||  
      data4!=f_data4 || data5!=f_data5)
     fail();
}

// ---------------------------------------------------------------------------
// func: brc_w16
// ---------------------------------------------------------------------------
void brc_w16(WORD f_size, WORD f_addr, HWORD f_bsycnt,
             WORD f_data0, WORD f_data1, WORD f_data2, WORD f_data3,
             WORD f_data4, WORD f_data5, WORD f_data6, WORD f_data7,
             WORD f_data8, WORD f_data9, WORD f_data10,WORD f_data11,
             WORD f_data12,WORD f_data13,WORD f_data14,WORD f_data15)
{
   WORD addr[16];
   volatile WORD data0,data1,data2,data3,data4,data5,data6,data7,
                 data8,data9,data10,data11,data12,data13,data14,data15;
   int i;
   int mask=0xf;
   int incr;
   int offset;
   incr=1<<f_size;
   mask=mask<<f_size;
   for (i=0;i<=15;i++) {
      addr[i]=f_addr;
      offset=f_addr & mask;
      if (offset==mask) {
        f_addr=~mask & f_addr;
      }
      else { f_addr=f_addr+incr;}
   }
   switch (f_size){
     case HSIZE_BYTE:
           data0=*((BYTE *) addr[0]) ;
           data1=*((BYTE *) addr[1]) ;
           data2=*((BYTE *) addr[2]) ;
           data3=*((BYTE *) addr[3]) ;
           data4=*((BYTE *) addr[4]) ;
           data5=*((BYTE *) addr[5]) ;
           data6=*((BYTE *) addr[6]) ;
           data7=*((BYTE *) addr[7]) ;
           data8=*((BYTE *) addr[8]) ;
           data9=*((BYTE *) addr[9]) ;
           data10=*((BYTE *) addr[10]) ;
           data11=*((BYTE *) addr[11]) ;
           data12=*((BYTE *) addr[12]) ;
           data13=*((BYTE *) addr[13]) ;
           data14=*((BYTE *) addr[14]) ;
           data15=*((BYTE *) addr[15]) ;
           break;
    case HSIZE_HWORD:
           data0=*((HWORD *) addr[0]) ;
           data1=*((HWORD *) addr[1]) ;
           data2=*((HWORD *) addr[2]) ;
           data3=*((HWORD *) addr[3]) ;
           data4=*((HWORD *) addr[4]) ;
           data5=*((HWORD *) addr[5]) ;
           data6=*((HWORD *) addr[6]) ;
           data7=*((HWORD *) addr[7]) ;
           data8=*((HWORD *) addr[8]) ;
           data9=*((HWORD *) addr[9]) ;
           data10=*((HWORD *) addr[10]) ;
           data11=*((HWORD *) addr[11]) ;
           data12=*((HWORD *) addr[12]) ;
           data13=*((HWORD *) addr[13]) ;
           data14=*((HWORD *) addr[14]) ;
           data15=*((HWORD *) addr[15]) ;
           break;
    case HSIZE_WORD:
           data0=*((WORD *) addr[0]) ;
           data1=*((WORD *) addr[1]) ;
           data2=*((WORD *) addr[2]) ;
           data3=*((WORD *) addr[3]) ;
           data4=*((WORD *) addr[4]) ;
           data5=*((WORD *) addr[5]) ;
           data6=*((WORD *) addr[6]) ;
           data7=*((WORD *) addr[7]) ;
           data8=*((WORD *) addr[8]) ;
           data9=*((WORD *) addr[9]) ;
           data10=*((WORD *) addr[10]) ;
           data11=*((WORD *) addr[11]) ;
           data12=*((WORD *) addr[12]) ;
           data13=*((WORD *) addr[13]) ;
           data14=*((WORD *) addr[14]) ;
           data15=*((WORD *) addr[15]) ;
           break;
     default :
           break;
     }
   if (data0!=f_data0 | data1!=f_data1 || data2!=f_data2 || 
      data3!=f_data3 || data4!=f_data4 || data5!=f_data5 || 
      data6!=f_data6 || data7!=f_data7 || data8!=f_data8 || 
      data9!=f_data9 || data10!=f_data10 || data11!=f_data11 || 
      data12!=f_data12 || data13!=f_data13 || data14!=f_data14 || data15!=f_data15)
     fail();
}

// ---------------------------------------------------------------------------
// trick-box func: display
// ---------------------------------------------------------------------------
#define SWAP4(x) (WORD)(((x)<<24) | (((x)>>24)&0xFF) | \
                                (((x)<<8)&0xFF0000) | (((x)>>8)&0xFF00))
// TODO:
// display with "ptr" not word aligned may cause exception 
#if 1
void display( char *ptr)
{
    /*
    int i;
    int j;

    i=0;
    j=0;
    //start flag
    sw(MessageBox_BASE,0xdddd1111,HSIZE_WORD);
    // data transmit
    while (ptr[i]!='\0') {
        i++;
    }
    j=i/4;
     for (i=0;i<=j*4;i+=4) {
      sw(MessageBox_BASE+0x04,SWAP4(*((WORD *)&ptr[i])),HSIZE_WORD);
     }
    //end flag
    sw(MessageBox_BASE,0xddddeeee,HSIZE_WORD);
    */
//    printf("%s \n",ptr);
    fLib_printf("%s \n",ptr);
}
#endif
/*
// ---------------------------------------------------------------------------
// trick-box func: printf
// ---------------------------------------------------------------------------
void printf( char *ptr )
{
    int i;

    i=0;
    //start flag
    sw(MessageBox_BASE,0xdddd1111,HSIZE_WORD);
    // data transmit
    while (ptr[i]!='\0'){
        sw(MessageBox_BASE+0x04,ptr[i],HSIZE_BYTE);
        i++;
    }
    //end flag
    sw(MessageBox_BASE,0xddddeeee,HSIZE_WORD);
}
*/

// ---------------------------------------------------------------------------
// trick-box func: exit_tube
// ---------------------------------------------------------------------------
void exit_tube( unsigned char  flag )
{
    switch (flag){
    case 0x99:
        finish();
        break;
    case 0x66:
        fail();
        break;
    case 0x88:
        pass();
        break;
    default :
        finish();
        break;
    }
}

/*
// ---------------------------------------------------------------------------
// trick-box func: set_tbox_p (set trickbox parameter)
// ---------------------------------------------------------------------------
void set_tbox_p(WORD n,WORD value)
{
  //sw(AXI_APB_BASE+0x130,0x00000008,HSIZE_WORD);
  sw(MessageBox_BASE+0x00001000+n*4,value,HSIZE_WORD);
}
*/

#if SEMIHOSTING
#define DBGPOPRT_MSG    "Debug Info: "
char    debug_buffer[] = {"Debug Info:                \n"};
#endif

// ---------------------------------------------------------------------------
// trick-box func: debugport
// ---------------------------------------------------------------------------
void  debugport(uint32_t code)
{
#if SEMIHOSTING
    utoh(code, debug_buffer+12);
    _swiwrite0(debug_buffer);
#else
//    *( (unsigned long *) (DEBUG_ADDR) ) = code;
    fLib_printf("%s:%s code=%d\n",__FILE__,__func__,code);
#endif
}

// ---------------------------------------------------------------------------
// trick-box func: ext_model
// ---------------------------------------------------------------------------
void ext_model(int num)
{
//    *( (unsigned long *) (MessageBox_BASE) ) =0xeeee0000+num;
    fLib_printf("%s:%s num=%d\n",__FILE__,__func__,num);
}

// ---------------------------------------------------------------------------
// trick-box func: m_gpio_set_dir
// ---------------------------------------------------------------------------
void m_gpio_set_dir (unsigned long dir)
{
    /*
    sw(MessageBox_BASE+0x00001000,0x00000002,HSIZE_WORD);//command:set direction
    sw(MessageBox_BASE+0x00001004,dir,HSIZE_WORD);//direction
    */
    fLib_printf("%s:%s dir=%d\n",__FILE__,__func__,dir);
    ext_model(0);//trigger gpio model
}

// ---------------------------------------------------------------------------
// trick-box func: m_gpio_get_pin
// ---------------------------------------------------------------------------
void m_gpio_get_pin()
{
    /*
    sw(MessageBox_BASE+0x00001000,0x00000003,HSIZE_WORD);//command:get gpio_in
    */
    fLib_printf("%s:%s \n",__FILE__,__func__);
    ext_model(0);//trigger gpio model
}

// ---------------------------------------------------------------------------
// trick-box func: m_gpio_output
// ---------------------------------------------------------------------------
void m_gpio_output(unsigned long data,unsigned long dir)
{
    /*
    sw(MessageBox_BASE+0x00001000,0x00000001,HSIZE_WORD);//command:output mode
    sw(MessageBox_BASE+0x00001004,dir,HSIZE_WORD);//direction
    sw(MessageBox_BASE+0x00001008,data,HSIZE_WORD);//output data
    */
    fLib_printf("%s:%s  data=%d dir=%d\n",__FILE__,__func__,data,dir);
    ext_model(0);//trigger gpio model
}

#if SEMIHOSTING

/* SWI numbers for RDP (Demon) monitor.  */
#define SWI_WriteC                 0x0
#define SWI_Write0                 0x2
#define SWI_ReadC                  0x4
#define SWI_CLI                    0x5
#define SWI_GetEnv                 0x10
#define SWI_Exit                   0x11
#define SWI_EnterOS                0x16

#define SWI_GetErrno               0x60
#define SWI_Clock                  0x61
#define SWI_Time                   0x63
#define SWI_Remove                 0x64
#define SWI_Rename                 0x65
#define SWI_Open                   0x66

#define SWI_Close                  0x68
#define SWI_Write                  0x69
#define SWI_Read                   0x6a
#define SWI_Seek                   0x6b
#define SWI_Flen                   0x6c

#define SWI_IsTTY                  0x6e
#define SWI_TmpNam                 0x6f
#define SWI_InstallHandler         0x70
#define SWI_GenerateError          0x71

#define SYS_WRITEC      (0x03)
#define SYS_WRITE0      (0x04)

const char str_array[] = {"0123456789abcdef"};

void
utoh(uint32_t value, char *buf)
{
    int shift;

    for (shift = 28; shift >=0; ++buf, shift -=4 )
    {
        *buf = str_array[ ((value & (0xf << shift))>>shift) ];
    }
/*
    buf[0] = str_array[(value & (0xf<<32)) >> 32];
    buf[1] = str_array[(value & (0xf<<28)) >> 28];
    buf[2] = str_array[(value & (0xf<<24)) >> 24];
    buf[3] = str_array[(value & (0xf<<20)) >> 20];
    buf[4] = str_array[(value & (0xf<<16)) >> 16];
    buf[5] = str_array[(value & (0xf<<12)) >> 12];
    buf[6] = str_array[(value & (0xf<<8)) >> 8];
    buf[7] = str_array[(value & (0xf<<4)) >> 4];*/
}

int
_swiwrite0(char * ptr)
{
    register r0 asm("r0");
    register r1 asm("r1");
    r0 = SYS_WRITE0;
    r1 = (int)ptr;
    asm ("swi %a3" : "=r" (r0) : "0"(r0), "r"(r1), "i"(0x123456));
    return  (r0);
}

int
_swiwritec(char * ptr)
{
    register r0 asm("r0");
    register r1 asm("r1");
    r0 = SYS_WRITE0;
    r1 = (int)ptr;
    asm ("swi %a3" : "=r" (r0) : "0"(r0), "r"(r1), "i"(0x123456));
    return  (r0);
}

#endif// if SEMIHOSTING


