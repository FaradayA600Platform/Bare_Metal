
#define CPRM_STATUS_RETRY_COUNT         0x100000
#define MaxRound 						10
#define lrot8(x,n)      				(x << n) | (x >> (8-n));
#define lrot32(x,n)     				((uint32_t)x << n) | ((uint32_t)x >> (32-n));

/* 0x6C: Cipher Control Mode */
#define C2_One_Way_Function				(1 << 0)
#define C2_Encry_ECB					(1 << 1)
#define C2_Encry_CCBC					(1 << 2)
#define C2_Decry_ECB					(1 << 3)
#define C2_Decry_CCBC					(1 << 4)
#define C2_Random_Gen					(1 << 5)
#define Auto_Encry_CCBC					(1 << 6)
#define Auto_Decry_CCBC					(1 << 7)

extern uint8_t SecretConstant[256];
extern volatile uint32_t cmd_index;

uint8_t SDC_CPRM(uint32_t start, uint32_t cnt);
uint8_t SDC_CPRM_Init(void);
uint8_t SDC_GetCardAPPState(void);
uint8_t SDC_CPRM_GetMKB(uint32_t * MKB_Buf, uint32_t Blockcnt, uint8_t MKB_ID, uint16_t Offset);
uint8_t SDC_CPRM_processMKB(uint32_t * MKB_Buf, uint32_t Blockcnt, uint32_t * km_lo, uint32_t * km_hi);
uint8_t SDC_CPRM_generate_MUK(uint32_t km_lo, uint32_t km_hi, uint32_t * kum_lo, uint32_t * kum_hi);
uint8_t SDC_CPRM_AKE(uint32_t u32_arg, uint32_t * u32_ks_lo, uint32_t * u32_ks_hi);
uint8_t SDC_CPRM_write_protect_area(uint32_t u32_startblk, uint32_t u32_blockcnt, uint32_t u32_sector_length,
				  uint8_t * u8_data_buf);
uint8_t SDC_CPRM_read_protect_area(uint32_t u32_startblk, uint32_t u32_blockcnt, uint32_t u32_sector_length,
				 uint8_t * u8_data_buf);
uint8_t SDC_CPRM_erase_protect_area(uint32_t u32_startblk, uint32_t u32_blockcnt, uint32_t u32_sector_length);
uint8_t SDC_CPRM_protect_area_access_test(uint32_t startblk, uint32_t blockcnt);
uint8_t SDC_Read_Protected_Sector(SDCardInfo * SDCard, uint32_t StartBlk, uint32_t BlkCount, uint32_t SectorSize, uint8_t * Buf);
uint8_t SDC_Write_Protected_Sector(SDCardInfo * SDCard, uint32_t StartBlk, uint32_t BlkCount, uint32_t SectorSize, uint8_t * Buf);
uint8_t SDC_Erase_Protected_Sector(SDCardInfo * SDCard, uint32_t StartBlk, uint32_t BlkCount, uint32_t SectorSize);
