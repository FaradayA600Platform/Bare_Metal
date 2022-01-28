/*------------------------------------------------------------------------------
flash.c

copyright 2010-2015 Faraday technology corporation.
------------------------------------------------------------------------------*/
/* include files */
//#include <intrins.h>
//#include "common.h"
#include "SoFlexible.h"
#include <stdlib.h>
#include "types.h"
#include "W25Q64CV.h"
//#define CONFIG_FTDMAC020
#ifdef CONFIG_FTDMAC020
//#include "DrvDMAC020.h"
#include "ftdmac020.h"
#endif
//#include "fdebug.h"
#include "DrvSPI020.h"
//#include "DrvUART010.h"

#define READ_LEN	256
#define WRITE_LEN	256

UINT8 read_buf[READ_LEN];
UINT8 write_buf[WRITE_LEN];
UINT32 ftspi020_command_complete = 0;

/* local variables */
SPI_Flash_T flash_info;
#define min_t(x,y) ( x < y ? x: y )
extern UINT32 spi020_rxfifo_depth(void);
extern void spi020_wait_rx_full(void);
void Read_SPI_Flash_ID(void);
void Read_Page0_Data(void);
void Program_Page0_Data(void);
void Erase_All_Chip(void);
void Erase_64K(void);
void Erase_32K(void);
void Erase_4K(void);
void Read_Status_Register(void);
void Write_Status_Register(void);


void FTSPI020_Handler(void *data)
{
	int intr_status =u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_INTR_ST));
printf(" ISR: FTSPI020_Handler\n");
	if (intr_status & SPI020_CMD_CMPLT) {
		printf(" ISR: Command Complete\n");

		ftspi020_command_complete = 1;
		vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTR_ST), SPI020_CMD_CMPLT);
	} else {
		printf(" ISR: No status\n");
	}
}

int FTSPI020_Test_Main(void)
{
    UINT8 ctr_num, key;
    UINT8 div,spi020_div;
    UINT32 max_clk=40<<20;//40MHz
    UINT32 i, choose;
    char buf[128];
    
	//fLib_SerialInit(DEFAULT_CONSOLE,DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);


    div=2;
    spi020_div=0;
    while((get_ahb_clk()/div) > max_clk){
        div +=2;
        spi020_div++;
    }
    printf("clk=%d div=%d spi020_div=%d \n",get_ahb_clk()/div,div, spi020_div);

    for (i = 0; i < IP_COUNT; ++i)
    {
        printf("%d: SPI_%d\n", i + 1, i);
    }
    printf("0: exit\n");
    printf("which one:");
    gets(buf);
    printf("\r");
    choose = atoi(buf);
    printf("choose SPI_%d\n", choose - 1);
    switch(choose)
    {
        case 1:
            spi_reg_base = SPI_FTSPI020_0_PA_BASE;
            spi_ack_req = SPI020_ACK_REQ_0;
            break;

        case 2:
            spi_reg_base = SPI_FTSPI020_1_PA_BASE;
            spi_ack_req = SPI020_ACK_REQ_1;
            break;
    }
    printf("spi_reg_base = 0x%x, ack_req = 0x%x\n", spi_reg_base, spi_ack_req);
//	spi020_init(SPI_CLK_MODE0,SPI_CLK_DIVIDER_2);
	fLib_ConnectIRQ(46, FTSPI020_Handler);
	spi020_init(SPI_CLK_MODE0,spi020_div);
    printf("\nSPI020 Test Begin...............\n");
    do
    {   
        printf("\n\n");
		printf("+-----------------------------------------+\n");
		printf("|       SPI020 Test w/ SPI Flash          |\n");
		printf("+-----------------------------------------+\n");
		printf("| 0: Get SPI Flash ID                     |\n");
		printf("| 1: Read Page0 Data(256Byte)             |\n");
		printf("| 2: Program Page0 Data(256Byte)          |\n");
		printf("| 3: Erase All Chip                       |\n");
		printf("| 4: Erase 64K(1st 64K: 0 - 0xFFFF)       |\n");
		printf("| 5: Erase 32K(1st 32K: 0 - 0x7FFF)       |\n");
		printf("| 6: Erase  4K(1st  4K: 0 - 0x0FFF)       |\n");
		printf("| 7: Read Status Register                 |\n");
		printf("| 8: Write Status Register                |\n"); 
		printf("+-----------------------------------------+\n");
		printf("| ESC : Quit                              |\n");
		printf("+-----------------------------------------+\n");
		printf("Please Select keystroke 0-8 or q:");
		//fflush(stdout);
		
        key = fLib_getchar();
        fLib_putchar(key);
        printf("\n\n");
        switch(key)
        {
            case '0':
               	Read_SPI_Flash_ID();
               	break;
            case '1':
                Read_Page0_Data();
                break;
            case '2':
                Program_Page0_Data();
                break;
            case '3':
                Erase_All_Chip();
                break;
            case '4':
                Erase_64K();
                break;   
			case '5':
				Erase_32K();
				break; 
			case '6':
				Erase_4K();
				break; 
			case '7':
				Read_Status_Register();
				break; 
			case '8':
				Write_Status_Register();
				break;                
			default:
			break;
    }	
//	}while(key != 0x1b);
	}while(key != 'q');
	printf("\nSPI020 Test w/ SPI Flash\n");
}

/* ===================================
 * Init SPI controller and flash device.
 * Init flash information and register functions.
 * =================================== */
void Read_SPI_Flash_ID(void)
{
	UINT32 probe_90_instruction; 

	unsigned char flash_manu[8];
   	fLib_memset((char *)flash_manu,0,sizeof(flash_manu));

    probe_90_instruction=spi020_flash_probe(&flash_info);
	if(probe_90_instruction)
		printf("Manufacturer ID=0x%02X(%s)\nDevice ID=0x%02X\n", flash_info.manufacturer,flash_manu, flash_info.flash_id);
	else
		printf("Manufacturer ID=0x%02X(%s)\nDevice ID=0x%04X\n", flash_info.manufacturer,flash_manu, flash_info.flash_id);

    switch(flash_info.manufacturer)
    {
    case FLASH_WB_DEV:
    	fLib_memcpy(flash_manu,"WINBOND",7);

        if((flash_info.flash_id == WB_W25P16_ID_9F)||(((flash_info.flash_id)&0xFF) == WB_W25P16_ID_90))
        {
            flash_info.flash_size = 0x200000;/* 2M bytes */
        }
        else if(flash_info.flash_id == WB_W25Q128BVFG_ID)
        {
			flash_info.flash_size = 0x1000000;/* 16M bytes */
        }
        else if(flash_info.flash_id == WB_W25Q64CV_ID_9F)
        {
			flash_info.flash_size = 0x800000;/* 8M bytes */
        }
	  else if(flash_info.flash_id == WB_W25Q256JW_ID_9F)
	  {
	  		flash_info.flash_size = 0x2000000;/* 32M bytes */
	  }
    break;
    default:
    break;
    }

    /* read system version */

	if(probe_90_instruction)
		printf("Manufacturer ID=0x%02X(%s)\nDevice ID=0x%02X\n", flash_info.manufacturer,flash_manu, flash_info.flash_id);
	else
		printf("Manufacturer ID=0x%02X(%s)\nDevice ID=0x%04X\n", flash_info.manufacturer,flash_manu, flash_info.flash_id);

  	if((flash_info.flash_size%1024)==0x00)
	{
		if(((flash_info.flash_size%1024)%1024)==0x00)
		{
			printf("Flash Size: %dByte(%dMByte)\n", flash_info.flash_size,flash_info.flash_size>>10>>10);
		}
		else
		{
			printf("Flash Size: %dByte=%dKByte)\n", flash_info.flash_size,flash_info.flash_size>>10);
		}	
	}
	else
	{
		printf("Flash Size: %dByte\n", flash_info.flash_size);
	}

}

void Read_Page0_Data(void)
{
	UINT32 i;
	UINT8 key;

	UINT32 read_len=READ_LEN;
	UINT32 access_byte;
	UINT32 offset=0;
    printf("0: Using PIO mode\n");
#ifdef CONFIG_FTDMAC020
    printf("1: Using AHB DMA mode\n");
	printf("Please select 0 - 1:\n");
#endif
	key = fLib_getchar();
	fLib_putchar(key);
	printf("\n\n");

	switch(key)
	{
		case '0':
			spi020_flash_read(FLASH_NORMAL, offset, read_len, read_buf);
		break;
		
#ifdef CONFIG_FTDMAC020
		case '1':

		while(read_len>0)
		{
			access_byte=min_t(read_len, spi020_rxfifo_depth());
			spi020_flash_read(FLASH_DMA_READ, offset, access_byte, NULL);
			spi020_wait_rx_full();
			/* 32 bits, mormal mode, src fixed, dst increment */
			/* SPI data port address */
//			dma_channel_set(AHBDMA_DstInc<<3|AHBDMA_SrcFix<<5|AHBDMA_NormalMode<<7|AHBDMA_DstWidth_DWord<<8|AHBDMA_SrcWidth_DWord<<11, 0x0000, SPI020REG_DATAPORT, ((UINT32)read_buf)+offset);
			dma_channel_set(AHBDMA_PriorityHigh<<22|AHBDMA_DstInc<<3|AHBDMA_SrcFix<<5|AHBDMA_HwHandShakeMode<<7|AHBDMA_DstWidth_DWord<<8|AHBDMA_SrcWidth_DWord<<11, 
			DMA020_SRC_HE_ENABLE |(spi_ack_req << 3) , spi_reg_base+SPI020REG_DATAPORT, ((UINT32)read_buf)+offset);
//			dma_channel_set(AHBDMA_PriorityHigh<<22|AHBDMA_DstInc<<3|AHBDMA_SrcFix<<5 |AHBDMA_DstWidth_DWord<<8|AHBDMA_SrcWidth_DWord<<11, 0x0, SPI020REG_DATAPORT, ((UINT32)read_buf)+offset);            
			dma_channel_action(access_byte>>2);/* start DMA, size unit in 4 bytes */
printf("dma_channel_action done\n");
#if 0
			if(ftspi020_command_complete)
			{
				printf("cmd_complete\n");
				ftspi020_command_complete = 0;
			    	vLib_LeWrite32((INT8U * )SPI020REG_INTERRUPT, 0x0);/* disable DMA function */

			}
			else		
#endif
				spi020_dma_read_stop();
			read_len-=access_byte;
			offset+=access_byte;
		}
		break;		
#endif
		default:
			printf("\nPlease select 0~1:");
		break;		
	}
    for(i=0; i < READ_LEN;i++)   //Show Data at UART0 
    {                                                  
        printf("%02x ",read_buf[i]);
        if((i%0x10) == 0x0F)
        printf("\n");
    }

}

void Program_Page0_Data(void)
{
	UINT32 i;
	UINT8 key;

	UINT32 write_len=WRITE_LEN;
	UINT32 access_byte;
	UINT32 offset=0;
	
    printf("Data = 0x00,0x01,0x02....0xFF written to SPI Flash\n");
    for(i=0;i<write_len;i++)//Prepare written data
    {
		write_buf[i]=i;
    }
    printf("0: Using PIO mode\n");
#ifdef CONFIG_FTDMAC020
    printf("1: Using AHB DMA mode\n");
	printf("Please select 0 - 1:\n");
#endif
	key = fLib_getchar();
	fLib_putchar(key);
	printf("\n\n");

	switch(key)
	{
		case '0':
			spi020_flash_write(FLASH_NORMAL, offset, write_len, write_buf);
		break;
#ifdef CONFIG_FTDMAC020
		case '1':

		while(write_len>0)
		{
			access_byte=min_t(write_len, spi020_txfifo_depth());
			printf("txfifo_depth = %d \n", access_byte);
			spi020_flash_write(FLASH_DMA_WRITE, offset, access_byte, NULL);
			spi020_wait_tx_empty();
			/* 32 bits, mormal mode, src increment, dst fixed */
			/* SPI data port address */
//			dma_channel_set(AHBDMA_DstFix<<3|AHBDMA_SrcInc<<5|AHBDMA_NormalMode<<7|AHBDMA_DstWidth_DWord<<8|AHBDMA_SrcWidth_DWord<<11, 0x0000, ((UINT32)write_buf)+offset, SPI020REG_DATAPORT);
			dma_channel_set(AHBDMA_PriorityHigh<<22|AHBDMA_DstFix<<3|AHBDMA_SrcInc<<5|AHBDMA_HwHandShakeMode<<7|AHBDMA_DstWidth_DWord<<8|AHBDMA_SrcWidth_DWord<<11, 
			DMA020_DST_HE_ENABLE |(spi_ack_req << 9), ((UINT32)write_buf)+offset, 
			spi_reg_base+SPI020REG_DATAPORT);

			dma_channel_action(access_byte>>2);/* start DMA, size unit in 4 bytes */
#if 0
			if(ftspi020_command_complete)
			{
				printf("cmd_complete\n");
				ftspi020_command_complete = 0;
			    	vLib_LeWrite32((INT8U * )SPI020REG_INTERRUPT, 0x0);/* disable DMA function */

			}
			else
#endif
				spi020_dma_write_stop();
			write_len-=access_byte;
			offset+=access_byte;
		}
		
		break;		
#endif
		default:
			printf("\nPlease select 0~1:");
		break;		
	}
}
void Erase_All_Chip(void)
{
	spi020_flash_chip_erase();
}
void Erase_64K(void)
{
	spi020_flash_64kErase(0);
}
void Erase_32K(void)
{
	spi020_flash_32kErase(0);
}
void Erase_4K(void)
{
	spi020_flash_4kErase(0);
}
void Read_Status_Register(void)
{
	UINT8 flash_status;
	flash_status = spi020_flash_r_state_OpCode_05();
	printf("Status Register-1 =0x%02X\n",flash_status);
	flash_status = spi020_flash_r_state_OpCode_35();
	printf("Status Register-2 =0x%02X\n",flash_status);
}
void Write_Status_Register(void)
{
	UINT8 buf[10];
	UINT16 Status_Register_1;
	UINT16 Status_Register_2;
	//fLib_memset(buf,0,sizeof(buf));
	printf("Enter value to Status Register-1:0x");
//ycmo	fLib_gets((char *)buf);
	gets((char *)buf);
	Status_Register_1 = strtoul((char *)buf, NULL, 16);
	printf("Enter value to Status Register-2:0x");
//ycmo	fLib_gets((char *)buf);
	gets((char *)buf);
	Status_Register_2 = strtoul((char *)buf, NULL, 16);
	
	spi020_flash_w_state((Status_Register_2<<8)|Status_Register_1);
}
void flash_read(UINT8 type, UINT32 offset, UINT32 len, void *buf)
{
    if(type & FLASH_2X_READ)
        type |= flash_info.support_dual;

    spi020_flash_read(type, offset, len, buf);
}
