#ifndef UTILITY_H
#define UTILITY_H


#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"

#ifndef MAX_CMD_LEN
#define MAX_CMD_LEN	128
#endif
#define	CMDLEN			50
#define	MAXARGS			20
	
#define divRoundDown(n,s)   ((n) / (s))
#define divRoundUp(n,s)     ((n+s-1)/(s))

#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))

#define RoundUp(val, units) \
		((((unsigned long)(val) + ((units) - 1)) / (units)) * (units))
#define RoundDown(val, units) \
		(((unsigned long)(val)/(units))*(units))
		

#define REG32(adr)             *(volatile UINT32 *)(adr)
#define Min(a,b)  (((a) < (b)) ? (a) : (b))
#define Max(a,b)  (((a) > (b)) ? (a) : (b))

#ifndef	isblank
#define	isblank(ch)	(((ch) == ' ') || ((ch) == '\t'))
#endif

#define  be16_to_cpu(x) Swap16(x)
#define  cpu_to_be16(x) Swap16(x)
#define  BE16_TO_CPU(x) Swap16(x)
#define  CPU_TO_BE16(x) Swap16(x)

#define  be32_to_cpu(x) swap32(x)
#define  cpu_to_be32(x) swap32(x)
#define  BE32_TO_CPU(x) swap32(x)
#define  CPU_TO_BE32(x) swap32(x)

#define Swap16(u16) ((U16)(((U16)(u16) >> 8) |\
                           ((U16)(u16) << 8)))
#define Swap32(u32) ((U32)(((U32)Swap16((U32)(u32) >> 16)) |\
                           ((U32)Swap16((U32)(u32)) << 16)))
#define swap32(u32) Swap32(u32)

#define TEST_PROGRAM_MAX 20
#define REMAP0	(0<<24)
#define REMAP1	(1<<24)
#define REMAP2	(2<<24)
#define REMAP3	(3<<24)
#define REMAP4	(4<<24)
#define REMAP5	(5<<24)
#define REMAP6	(6<<24)
#define REMAP7	(7<<24)
#define SELF_TEST_REBOOT vLib_LeWrite32((UINT8 *)(SCU_FTSCU010_PA_BASE+PWR_MOD), REMAP0|REBOOT)
#define RESULT_CALL_BACK_ADDR (void *)(*(volatile unsigned int *)0x20023FFC)

enum REPORT_CMD{
	RETURN_INDEX = 1
};

extern unsigned long mainStringToNum(char *str, char **next);
extern BOOL mainCheckCmd(char *cmd, char **str);
extern void mainMemDump(UINT32 addr, UINT32 len, UINT32 showAddr);
extern void mainMemDumpByte(UINT32 addr, UINT32 len, UINT32 showAddr);

#ifdef __cplusplus
}
#endif



#endif
