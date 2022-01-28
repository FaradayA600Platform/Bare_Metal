#ifndef __FLibARM
#define __FLibARM

#define AXI_APB_BASE  0x90a00000 
#define MessageBox_BASE AXI_APB_BASE + 0x00080000
#define DEBUG_ADDR MessageBox_BASE+0x8

#define Mode_USR 0x10 	     /* User Mode */
#define Mode_FIQ 0x11 	     /* FIQ Mode */
#define Mode_IRQ 0x12 	     /* IRQ Mode */
#define Mode_SVC 0x13 	     /* Supervisor Mode */
#define Mode_ABT 0x17 	     /* Abort Mode */
#define Mode_UND 0x1B 	     /* Undefined Mode */
#define Mode_SYS 0x1F 	     /* System Mode */

#define I_BIT    0x80	     /* when I bit is set, IRQ is disabled */
#define F_BIT    0x40
#define T_BIT    0x20

#ifndef __ASSEMBLER__
#include <stdint.h>
//#include "IP_BASE.h"

//============================
//  exit tube define constant
//============================
#define PASS          0x88
#define FAIL          0x66
#define EXIT          0x99

//===========================
// Size Type
//===========================
#define HSIZE_WORD    0x2
#define HSIZE_HWORD   0x1
#define HSIZE_HALFWORD   0x1
#define HSIZE_BYTE    0x0

#define WORD  unsigned int
#define HWORD unsigned short
#define BYTE  unsigned char

//======================
// exit tube function
//======================
extern void finish(void);
extern void pass(void);
extern void fail(void);
extern void warn(void);
extern void exit_tube( unsigned char);
extern void display (char *msg);
//#define display(MSG)    printf(MSG)
//extern void printf (char *msg);
extern void debugport(uint32_t code);
extern void set_tbox_p(WORD ,WORD);
extern void ext_model(int);

//======================
// gpio model function
//======================
extern void m_gpio_set_dir(unsigned long);
extern void m_gpio_get_pin(void); 
extern void m_gpio_output(unsigned long,unsigned long);

//===========================
// BFM command
//===========================
void lock(void);
void unlock(void);
extern  void  sw(WORD,WORD,BYTE );
extern  void  sr(WORD,BYTE );
extern  void  src(WORD,WORD,BYTE );
extern  void br( WORD );
extern  void brc( WORD , WORD, WORD , WORD , WORD );
extern  void bw( WORD , WORD , WORD , WORD , WORD );
extern  void srdata(WORD,WORD *);
extern  void srdata_hw(WORD,WORD *);
extern  void srdata_b(WORD,WORD *);

extern  void idle(unsigned long int );
extern  void idlen(unsigned long int , int );
extern  void bw_i4( WORD , WORD , HWORD , WORD , WORD , WORD , WORD );
extern  void bw_i8( WORD , WORD , HWORD , WORD , WORD , WORD , WORD ,WORD, WORD , WORD , WORD);
extern  void bw_i16( WORD , WORD , HWORD , WORD , WORD , WORD , WORD ,WORD, WORD , WORD , WORD, WORD , WORD , WORD , WORD ,WORD, WORD , WORD , WORD);

extern  void br_i4( WORD , WORD , HWORD);
extern  void br_i8( WORD , WORD , HWORD);
extern  void br_i16( WORD ,WORD , HWORD); 

extern  void brc_i4( WORD , WORD , HWORD , WORD , WORD , WORD , WORD );
extern  void brc_i8( WORD, WORD, HWORD, WORD, WORD, WORD, WORD, WORD , WORD , WORD , WORD );
extern  void brc_i16( WORD , WORD , HWORD , WORD , WORD , WORD , WORD ,WORD, WORD , WORD , WORD, WORD ,WORD , WORD , WORD ,WORD, WORD , WORD , WORD);

extern void bw_w4( WORD, WORD , HWORD, WORD , WORD , WORD , WORD);
extern void bw_w8( WORD, WORD , HWORD, WORD , WORD , WORD , WORD, WORD , WORD , WORD , WORD);
extern void bw_w16( WORD, WORD , HWORD, WORD , WORD , WORD , WORD, WORD , WORD , WORD , WORD, WORD , WORD , WORD , WORD, WORD , WORD , WORD , WORD);

extern void br_w4( WORD, WORD , HWORD);
extern void br_w8( WORD, WORD , HWORD);
extern void br_w16( WORD, WORD , HWORD);

extern void brc_w4( WORD, WORD , HWORD, WORD , WORD , WORD , WORD);
extern void brc_w8( WORD, WORD , HWORD, WORD , WORD , WORD , WORD, WORD , WORD , WORD , WORD);
extern void brc_w16( WORD, WORD , HWORD, WORD , WORD , WORD , WORD, WORD , WORD , WORD , WORD, WORD ,WORD , WORD , WORD, WORD , WORD , WORD , WORD);

int tube (void);
void exit_tube( unsigned char  flag );

#define DebugPort   debugport
// HAL APIs, modified from eCos
#define HAL_READ_UINT8( _register_, _value_ ) \
        ((_value_) = *((volatile uint8_t *)(_register_)))

#define HAL_WRITE_UINT8( _register_, _value_ ) \
        (*((volatile uint8_t *)(_register_)) = (_value_))

#define HAL_WRITEOR_UINT8( _register_, _value_ ) \
        (*((volatile uint8_t *)(_register_)) |= (_value_))


#define HAL_READ_UINT16( _register_, _value_ ) \
        ((_value_) = *((volatile uint16_t *)(_register_)))

#define HAL_WRITE_UINT16( _register_, _value_ ) \
        (*((volatile uint16_t *)(_register_)) = (_value_))

#define HAL_WRITEOR_UINT16( _register_, _value_ ) \
        (*((volatile uint16_t *)(_register_)) |= (_value_))

  
#define HAL_READ_UINT32( _register_, _value_ ) \
        ((_value_) = *((volatile uint32_t *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ ) \
        (*((volatile uint32_t *)(_register_)) = (_value_))

#define HAL_WRITEOR_UINT32( _register_, _value_ ) \
        (*((volatile uint32_t *)(_register_)) |= (_value_))

#define HAL_WRITEAND_UINT32( _register_, _value_ ) \
        (*((volatile uint32_t *)(_register_)) &= (_value_))
#endif  // __ASSEMBLER

#endif

