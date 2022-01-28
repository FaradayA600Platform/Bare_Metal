/*------------------------------------------------------------------------------
spi020.C

Copyright 2010-2015 Faraday Technology Corporation.
------------------------------------------------------------------------------*/
//#include "common.h"
#include "SoFlexible.h"
#include "W25Q64CV.h"
#include "types.h"
#include "DrvSPI020.h"
#define min_t(x,y) ( x < y ? x: y )

#define DMAC_CSR        0x24
#define DMAC_SYNC		0x28
#define DMAC_Cn_CSR     0x100
#define DMAC_Cn_CFG     0x104
#define DMAC_Cn_SrcAddr 0x108
#define DMAC_Cn_DstAddr 0x10C
#define DMAC_Cn_SIZE    0x114

#define DMAMAINCSR				(DMAC_FTDMAC020_PA_BASE+0x24)
#define DMAINT					(DMAC_FTDMAC020_PA_BASE+0x00)
#define DMATCREG				(DMAC_FTDMAC020_PA_BASE+0x04)
#define DMATCCLEAR				(DMAC_FTDMAC020_PA_BASE+0x08)


#define DMACEN      BIT0        //DMA controller enable
#define DMACSYNC	BIT0		//DMA channel 0 SYNC 
#define CH_EN       BIT0        //Channel Enable


UINT32 ReqReset=0;
UINT32  spi_reg_base;
UINT32  spi_ack_req;
void spi020_set_commands(UINT32 cmd0, UINT32 cmd1, UINT32 cmd2, UINT32 cmd3)
{

    vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_CMD0), cmd0);
    vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_CMD1), cmd1);
    vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_CMD2), cmd2);
    vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_CMD3), cmd3);

}

/* Wait until command complete */
void spi020_qc_wait_command_complete(void)
{
    UINT32  reg;
    do {
        reg = u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_INTR_ST));
    } while ((reg & SPI020_CMD_CMPLT)==0x0);
//  vLib_LeWrite32(SPI020REG_INTR_ST, (reg | SPI020_CMD_CMPLT));/* clear command complete status */
    vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTR_ST), SPI020_CMD_CMPLT);/* clear command complete status */
}

/* Wait until the rx fifo ready */
void spi020_wait_rx_full(void)
{
    while(!(u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_STATUS)) & SPI020_RXFIFO_READY));
}

/* Wait until the tx fifo ready */
void spi020_wait_tx_empty(void)
{
    while(!(u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_STATUS)) & SPI020_TXFIFO_READY));
}

/* Get the rx fifo depth, unit in byte */
UINT32 spi020_rxfifo_depth(void)
{
    return ((u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_FEATURE)) & SPI020_RX_DEPTH) >> (8-2));
}

/* Get the tx fifo depth, unit in byte */
UINT32 spi020_txfifo_depth(void)
{
    return ((u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_FEATURE)) & SPI020_TX_DEPTH) << 2);
}

void spi020_read_data(/*UINT8*/UINT32 *buf, UINT32 length)
{
    UINT32  access_byte;//, tmp_read;

    while(length > 0)
    {
        spi020_wait_rx_full();
        access_byte = min_t(length, spi020_rxfifo_depth());
        length -= access_byte;
        while(access_byte > 0)
        {
        	*buf= u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_DATAPORT));
            buf ++;
            if(access_byte>=4)
            	access_byte -= 4;
            else
            	access_byte=0;
        	#if 0
            switch(access_byte)
            {
            case 1:
                tmp_read = u32Lib_LeRead32((INT8U * )SPI020REG_DATAPORT);
                *buf = tmp_read&0xFF;
                access_byte = 0;//break while loop 
                break;
            case 2:
                tmp_read = u32Lib_LeRead32((INT8U * )SPI020REG_DATAPORT);
                *buf = tmp_read&0xFF;
                buf++;
                *buf = (tmp_read&0xFF00)>>8;
                access_byte = 0;// break while loop 
                break;
            case 3:// read chip id will use this case 
                tmp_read = u32Lib_LeRead32((INT8U * )SPI020REG_DATAPORT);
                *buf = tmp_read&0x00FF;
                buf++;
                *buf = (tmp_read&0xFF00)>>8;
                buf++;
                *buf = (tmp_read&0xFF0000)>>16;
                access_byte = 0;// break while loop 
                break;
            default:// access_byte>=4 
                *(UINT32 *)buf= u32Lib_LeRead32((INT8U * )SPI020REG_DATAPORT);
                buf +=4;
                access_byte -= 4;
                break;
            }
            #endif
        }
    }
}

void spi020_check_status_til_ready_2(void)
{
//  main_delay_count(0x5100);

    /* fill in command 0~3 */
    spi020_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);
//  main_delay_count(0x80);
    /* wait for command complete */
    spi020_qc_wait_command_complete();
}

void spi020_check_status_til_ready(void)
{
    /* savecodesize, move into here */
    spi020_qc_wait_command_complete();

    /* read status */
    spi020_check_status_til_ready_2();

}

void spi020_write_control(UINT8 enable)
{
    /* fill in command 0~3 */
    if (enable)
    {
        spi020_set_commands(SPI020_06_CMD0, SPI020_06_CMD1, SPI020_06_CMD2, SPI020_06_CMD3);
    }
    else
    {
        spi020_set_commands(SPI020_04_CMD0, SPI020_04_CMD1, SPI020_04_CMD2, SPI020_04_CMD3);
    }
    /* wait for command complete */
    spi020_qc_wait_command_complete();
}

void spi020_write_data(UINT8 *buf, UINT32 length)
{
    UINT32  access_byte, i;
    UINT32  value = 0;
    
    /* This function assume length is multiple of 4 */

    while(length > 0)
    {
        spi020_wait_tx_empty();
        access_byte = min_t(length, spi020_txfifo_depth());
        length -= access_byte;
        while(access_byte > 0)
        {
            if(access_byte < 4)
            {
                printf("write data = 0x%x\n", *((UINT8 *)buf));   
                vLib_LeWrite8((INT8U * )(spi_reg_base+SPI020REG_DATAPORT), *((UINT8 *)buf));
                buf += 1;
                access_byte -= 1;
            }
            else
            {
                vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_DATAPORT), *((UINT32 *)buf));
                buf += 4;
                access_byte -= 4;
            }
        }
    }
}

void spi020_init(SPI_CLK_MODE spi_clk_mode,UINT8 spi_clk_divider)
{
  UINT32  reg;
printf("spi020_init\n");
    /* SPI020 reset command/FIFO and reset state machine */
 // reg = u32Lib_LeRead32(SPI020REG_CONTROL);
  vLib_LeWrite32((spi_reg_base+SPI020REG_CONTROL), SPI020_ABORT);
printf("spi020_init 1\n");
	/* Wait reset completion */
	do
	{
		if((u32Lib_LeRead32((spi_reg_base+SPI020REG_CONTROL))&SPI020_ABORT)==0x00)
printf("spi020_init 2\n");
			break;
	}while(1);
    /* Set control register */
printf("spi020_init 3\n");
  reg = u32Lib_LeRead32((spi_reg_base+SPI020REG_CONTROL));
  reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
  reg |= spi_clk_mode | spi_clk_divider;
  vLib_LeWrite32((spi_reg_base+SPI020REG_CONTROL), reg);

   vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTERRUPT), 0x2);// enable command complete
}

void spi020_flash_read(UINT8 type, UINT32 offset, UINT32 len, void *buf)
{
    UINT32               *read_buf;//UINT8               *read_buf;

    if (type & FLASH_DMA_READ)
    {
        vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTERRUPT), SPI020_DMA_EN);// enable DMA function 
//		return;
    }

    /* fill in command 0~3 */
    if (type & FLASH_DUAL_READ)
    {
        spi020_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
    }
    else if(type & FLASH_2XIO_READ)
    {
        spi020_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
    }
    else if(type & FLASH_FIRST_READ)
    {
        spi020_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
    }
    else/* normal read */
    {
        spi020_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
    }

    if (type & FLASH_DMA_READ)
    {
//		vLib_LeWrite32(SPI020REG_INTERRUPT, SPI020_DMA_EN);// enable DMA function 
        return;
    }

    read_buf = (UINT32 *)buf;//read_buf = (UINT8 *)buf;
    spi020_read_data(read_buf, len);/* read data */
    spi020_qc_wait_command_complete();/* wait for command complete */
//  spi020_check_status_til_ready();
}

void spi020_dma_read_stop(void)
{
    spi020_qc_wait_command_complete();/* wait for command complete */
//  spi020_check_status_til_ready();

    vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTERRUPT), 0x0);/* disable DMA function */
}

void spi020_flash_write(UINT8 type, UINT32 offset, UINT32 len, void *buf)
{
    UINT8       *write_buf;

    /* This function does not take care about 4 bytes alignment */
    /* if ((UINT32)(para->buf) % 4) return 1; */

//  spi020_check_status_til_ready_2();

    spi020_write_control(1);/* send write enabled */

    if (type & FLASH_DMA_WRITE)
    {
//        	vLib_LeWrite32((INT8U * )SPI020REG_INTERRUPT, SPI020_DMA_EN | BIT1 | BIT8 | BIT9 | BIT12 | BIT13);/* enable DMA function */
		vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTERRUPT), SPI020_DMA_EN );/* enable DMA function */
//		return;
    }

    /* fill in command 0~3 */
    spi020_set_commands(offset, SPI020_02_CMD1, len, SPI020_02_CMD3);

    if (type & FLASH_DMA_WRITE)
    {
//		vLib_LeWrite32(SPI020REG_INTERRUPT, SPI020_DMA_EN | BIT1 | BIT8 | BIT9 | BIT12 | BIT13);/* enable DMA function */
		vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTERRUPT), SPI020_DMA_EN );/* enable DMA function */
        return;
    }

    write_buf = (UINT8 *)buf;
    spi020_write_data(write_buf, len);
//  spi020_qc_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
    spi020_check_status_til_ready();
    return;
}

void spi020_dma_write_stop(void)
{
    spi020_qc_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
    vLib_LeWrite32((INT8U * )(spi_reg_base+SPI020REG_INTERRUPT), 0x0);/* disable DMA function */
//  spi020_check_status_til_ready();
    spi020_check_status_til_ready_2();
}

void spi020_flash_chip_erase(void)
{
//  spi020_check_status_til_ready_2();

    spi020_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    spi020_set_commands(SPI020_C7_CMD0, SPI020_C7_CMD1, SPI020_C7_CMD2, SPI020_C7_CMD3);
    /* wait for command complete */
//  spi020_qc_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
    /* wait for flash ready */
    spi020_check_status_til_ready();
}

void spi020_flash_64kErase(UINT32 offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_64K) return 1; */

//  spi020_check_status_til_ready_2();

    spi020_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    spi020_set_commands(offset, SPI020_D8_CMD1, SPI020_D8_CMD2, SPI020_D8_CMD3);
    /* wait for command complete */
//  spi020_qc_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
//  main_delay_count(0x120000);
    spi020_check_status_til_ready();
//  spi020_check_status_til_ready_2();
}

void spi020_flash_32kErase(UINT32 offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_64K) return 1; */

//  spi020_check_status_til_ready_2();

    spi020_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    spi020_set_commands(offset, SPI020_52_CMD1, SPI020_52_CMD2, SPI020_52_CMD3);
    /* wait for command complete */
//  spi020_qc_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
//  main_delay_count(0x120000);
    spi020_check_status_til_ready();
//  spi020_check_status_til_ready_2();
}

void spi020_flash_4kErase(UINT32 offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_4K) return 1; */

//  spi020_check_status_til_ready_2();

    spi020_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    spi020_set_commands(offset, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
    /* wait for command complete */
//  spi020_qc_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
//  main_delay_count(0x120000);
    spi020_check_status_til_ready();
//  spi020_check_status_til_ready_2();
}
void spi020_flash_w_state(UINT16 value)
{
    /* fill in command 0~3 */
    spi020_set_commands(SPI020_50_CMD0, SPI020_50_CMD1, SPI020_50_CMD2, SPI020_50_CMD3);
    /* wait for command complete */
    spi020_qc_wait_command_complete();

    spi020_write_control(1);/* send write enabled */
    /* fill in command 0~3 */
    spi020_set_commands(SPI020_01_CMD0, SPI020_01_CMD1, SPI020_01_CMD2, SPI020_01_CMD3);
    /* write data */
    spi020_write_data((UINT8 *)&value, 0x2);
    spi020_check_status_til_ready();
}

UINT8 spi020_flash_r_state_OpCode_05(void)
{
    UINT8   tmpbuf;
    /* fill in command 0~3 */
    //spi020_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3&0xFFFFFFFE);
    spi020_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);//bessel:wait interrupt instead of delay
    //main_delay_count(1000);
    spi020_qc_wait_command_complete();
    tmpbuf = u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_READ_ST));
    return tmpbuf;
}
UINT8 spi020_flash_r_state_OpCode_35(void)
{
    UINT8   tmpbuf;
    /* fill in command 0~3 */
    //spi020_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3&0xFFFFFFFE);
    spi020_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3);//bessel:wait interrupt instead of delay
    //main_delay_count(1000);
    spi020_qc_wait_command_complete();
    tmpbuf = u32Lib_LeRead32((INT8U * )(spi_reg_base+SPI020REG_READ_ST));
    return tmpbuf;
}

UINT32 spi020_flash_probe(SPI_Flash_T *flash)
{
    UINT32  chip_id=0;

	UINT32  probe_90_instruction=0;

    /* fill in command 0~3 */
	//Read Manufacturer and Device Identification by JEDEC ID(0x9F)
    spi020_set_commands(SPI020_9F_CMD0, SPI020_9F_CMD1, SPI020_9F_CMD2, SPI020_9F_CMD3);
    /* read data */
    spi020_read_data(/*(UINT8 *)*/&chip_id, 0x3);
    /* wait for command complete */
    spi020_qc_wait_command_complete();

    //flash->manufacturer = (chip_id>>24);
    flash->manufacturer = (UINT8)chip_id;
    if(flash->manufacturer==0x00 || flash->manufacturer==0xFF)
    {
       /* fill in command 0~3 */
       //Read Manufacturer and Device Identification by 0x90
        spi020_set_commands(SPI020_90_CMD0, SPI020_90_CMD1, SPI020_90_CMD2, SPI020_90_CMD3);
        /* read data */
        spi020_read_data(/*(UINT8 *)*/&chip_id, 0x02/*0x4*/);
        /* wait for command complete */
        spi020_qc_wait_command_complete();
        //flash->manufacturer = (chip_id>>24);
        flash->manufacturer = (UINT8)chip_id;
        probe_90_instruction=1;
    }
    flash->flash_id = (chip_id>>8);
    return probe_90_instruction;
}

#ifdef CONFIG_FTDMAC020
/* always use channel 0 */
void dma_channel_set(UINT32 dmaRegCSR, UINT32 dmaRegCFG, UINT32 dmaSrcAddr, UINT32 dmaDstAddr)
{
    vLib_LeWrite32((UINT8 *)(DMAC_FTDMAC020_PA_BASE+DMAC_CSR), DMACEN); //bessel:DMA controller enable
    vLib_LeWrite32((UINT8 *)(DMAC_FTDMAC020_PA_BASE+DMAC_SYNC), DMACSYNC); //steve : DMAC SYNC enable
    vLib_LeWrite32((UINT8 *)(DMAC_FTDMAC020_PA_BASE+DMAC_Cn_CSR), dmaRegCSR);
    vLib_LeWrite32((UINT8 *)(DMAC_FTDMAC020_PA_BASE+DMAC_Cn_CFG), dmaRegCFG);
    vLib_LeWrite32((UINT8 *)(DMAC_FTDMAC020_PA_BASE+DMAC_Cn_SrcAddr), dmaSrcAddr);
    vLib_LeWrite32((UINT8 *)(DMAC_FTDMAC020_PA_BASE+DMAC_Cn_DstAddr), dmaDstAddr);
}

void dma_channel_action(UINT32 size)
{

    vLib_LeWrite32(DMAC_FTDMAC020_PA_BASE+DMAC_Cn_SIZE, size);/* fill in transfer size */
    vLib_LeWrite32(DMAC_FTDMAC020_PA_BASE+DMAC_Cn_CSR, (u32Lib_LeRead32((UINT8 *)(DMAC_FTDMAC020_PA_BASE+DMAC_Cn_CSR)))|CH_EN);/* channel enable */

#if 1
    while((u32Lib_LeRead32(DMAINT)&0x01)==0) {
        /*putchar('.')*/;   /* check the channel is finished */
    }
    vLib_LeWrite32(DMATCCLEAR, 0x1);
#else
    while((u32Lib_LeRead32(DMATCREG)&0x01)==0) {
        /*putchar('.')*/;   /* check the channel is finished */
    }
    vLib_LeWrite32(DMATCCLEAR, 0x1);
#endif
}
#endif

