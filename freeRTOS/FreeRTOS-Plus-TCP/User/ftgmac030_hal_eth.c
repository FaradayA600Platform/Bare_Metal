/**
  ******************************************************************************
  * @file    ftgmac030_hal_eth.c
  * @author  B.C. Chen
  * @version V1.0.0
  * @date    3-Aug-2021
  * @brief   ETH HAL module driver.
  *          This file provides firmware functions to manage the following 
  *          functionalities of the Ethernet (ETH) peripheral:
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions 
  *           + Peripheral State and Errors functions
  *
  @verbatim
  ==============================================================================
                    ##### How to use this driver #####
  ==============================================================================
    [..]
      (#)Declare a ETH_HandleTypeDef handle structure, for example:
         ETH_HandleTypeDef  heth;

      (#)Fill parameters of Init structure in heth handle

      (#)Call HAL_ETH_Init() API to initialize the Ethernet peripheral (MAC, DMA, ...) 

      (#)Initialize the ETH low level resources through the HAL_ETH_MspInit() API:
          (##) Configure Ethernet pin-out
          (##) Configure Ethernet NVIC interrupt (IT mode)

      (#)Initialize Ethernet DMA Descriptors in chain mode and point to allocated buffers:
          (##) HAL_ETH_DMATxDescListInit(); for Transmission process
          (##) HAL_ETH_DMARxDescListInit(); for Reception process

      (#)Enable MAC and DMA transmission and reception:
          (##) HAL_ETH_Start();

      (#)Prepare ETH DMA TX Descriptors and give the hand to ETH DMA to transfer 
         the frame to MAC TX FIFO:
         (##) HAL_ETH_TransmitFrame();

      (#)Poll for a received frame in ETH RX DMA Descriptors and get received 
         frame parameters
         (##) HAL_ETH_GetReceivedFrame(); (should be called into an infinite loop)

      (#) Get a received frame when an ETH RX interrupt occurs:
         (##) HAL_ETH_GetReceivedFrame_IT(); (called in IT mode only)

      (#) Communicate with external PHY device:
         (##) Read a specific register from the PHY  
              HAL_ETH_ReadPHYRegister();
         (##) Write data to a specific RHY register:
              HAL_ETH_WritePHYRegister();

      (#) Configure the Ethernet MAC after ETH peripheral initialization
          HAL_ETH_ConfigMAC(); all MAC parameters should be filled.

      -@- The PTP protocol and the DMA descriptors ring mode are not supported
          in this driver

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center> COPYRIGHT(c) 2021 Faraday Technology Corporation </center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
//#include "common_include.h"
#include "SoFlexible.h"
#include "FreeRTOS.h"

/* FT includes. */
#include "ftgmac030_hal_eth.h"

int lUDPLoggingPrintf( const char *pcFormatString, ... );

/** @defgroup ETH ETH
  * @brief ETH HAL module driver
  * @{
  */

#if !defined( ARRAY_SIZE )
	#define ARRAY_SIZE( x ) ( sizeof ( x ) / sizeof ( x )[ 0 ] )
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @defgroup ETH_Private_Functions ETH Private Functions
  * @{
  */
static void ETH_MACAddressConfig(ETH_HandleTypeDef *heth, uint32_t MacAddr, uint8_t *Addr);
static void ETH_MACReceptionEnable(ETH_HandleTypeDef *heth);
static void ETH_MACReceptionDisable(ETH_HandleTypeDef *heth);
static void ETH_MACTransmissionEnable(ETH_HandleTypeDef *heth);
static void ETH_MACTransmissionDisable(ETH_HandleTypeDef *heth);
static void ETH_DMATransmissionEnable(ETH_HandleTypeDef *heth);
static void ETH_DMATransmissionDisable(ETH_HandleTypeDef *heth);
static void ETH_DMAReceptionEnable(ETH_HandleTypeDef *heth);
static void ETH_DMAReceptionDisable(ETH_HandleTypeDef *heth);
static void ETH_FlushTransmitFIFO(ETH_HandleTypeDef *heth);

extern uint32_t HAL_GetTick(void);
extern void HAL_Delay(uint32_t Delay);

/**
  * @}
  */
/* Private functions ---------------------------------------------------------*/

/** @defgroup ETH_Exported_Functions ETH Exported Functions
  * @{
  */

/** @defgroup ETH_Exported_Functions_Group1 Initialization and de-initialization functions
  *  @brief   Initialization and Configuration functions
  *
  @verbatim
  ===============================================================================
            ##### Initialization and de-initialization functions #####
  ===============================================================================
  [..]  This section provides functions allowing to:
      (+) Initialize and configure the Ethernet peripheral
      (+) De-initialize the Ethernet peripheral

  @endverbatim
  * @{
  */
extern void vMACBProbePhy ( void );

/**
  * @brief  Initializes the Ethernet MAC and DMA according to default
  *         parameters.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_Init(ETH_HandleTypeDef *heth)
{
	uint32_t tmpreg = 0;
	uint32_t hclk = AHB_CLK;

	/* Check the ETH peripheral state */
	if( heth == NULL )
	{
		return HAL_ERROR;
	}

	/* Check parameters */
	configASSERT(IS_ETH_AUTONEGOTIATION(heth->Init.AutoNegotiation));
	configASSERT(IS_ETH_RX_MODE(heth->Init.RxMode));
	configASSERT(IS_ETH_CHECKSUM_MODE(heth->Init.ChecksumMode));
	configASSERT(IS_ETH_MEDIA_INTERFACE(heth->Init.MediaInterface));

	/* Allocate lock resource and initialize it */
	heth->Lock = HAL_UNLOCKED;
	
	//fLib_Pinctrl_SetMux(GROUP_GMAC030);
	
	/* Calculate MDC cycle threshold */
	heth->Init.MDCCycThr = (400 * (hclk / 1000000)) / 1000;

	/* MAC interface selection */
	heth->Instance->GISR = heth->Init.MediaInterface;

	/* Ethernet Software reset */
	/* Set the SWR bit: resets all MAC subsystem internal registers and logic */
	/* After reset all the registers holds their respective reset values */
	/* Also enable EDFE: Enhanced descriptor format enable. */
	heth->Instance->MACCR = FTGMAC030_MACCR_SW_RST;

	/* Wait for software reset */
	while ( (heth->Instance->MACCR & FTGMAC030_MACCR_SW_RST) == FTGMAC030_MACCR_SW_RST )
	{
	}

	/* Initialise the MACB and set all PHY properties */
	vMACBProbePhy();

	/* Config MAC */
	HAL_ETH_ConfigMAC(heth);

	/* Set ETH HAL State to Ready */
	heth->State = HAL_ETH_STATE_READY;

	/* Return function status */
	return HAL_OK;
}

/**
  * @brief  De-Initializes the ETH peripheral.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_DeInit(ETH_HandleTypeDef *heth)
{
	/* Set the ETH peripheral state to BUSY */
	heth->State = HAL_ETH_STATE_BUSY;

	/* Set ETH HAL state to Disabled */
	heth->State = HAL_ETH_STATE_RESET;

	/* Release Lock */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return HAL_OK;
}

/**
  * @brief  Initializes the DMA Tx descriptors in chain mode.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  DMATxDescTab: Pointer to the first Tx desc list
  * @param  TxBuff: Pointer to the first TxBuffer list
  * @param  TxBuffCount: Number of the used Tx desc in the list
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_DMATxDescListInit(ETH_HandleTypeDef *heth, ETH_DMATxDescTypeDef *pxDMATable, uint8_t *ucDataBuffer, uint32_t ulBufferCount)
{
	uint32_t i = 0;
	ETH_DMATxDescTypeDef *pxDMADescriptor;

	/* Process Locked */
	__HAL_LOCK( heth );

	/* Set the ETH peripheral state to BUSY */
	heth->State = HAL_ETH_STATE_BUSY;

	/* Set the TxDesc pointer with the first one of the pxDMATable list */
	heth->TxDesc = pxDMATable;

	/* Fill each DMA descriptor with the right values */
	for( i=0; i < ulBufferCount; i++ )
	{
		/* Get the pointer on the ith member of the descriptor list */
		pxDMADescriptor = pxDMATable + i;

		pxDMADescriptor->txdes0 = ( uint32_t ) 0u;

		/* Set Buffer1 address pointer */
		if( ucDataBuffer != NULL )
		{
			pxDMADescriptor->txdes3 = ( uint32_t )( &ucDataBuffer[ i * ETH_TX_BUF_SIZE ] );
		}
		else
		{
			/* Buffer space is not provided because it uses zero-copy transmissions. */
			pxDMADescriptor->txdes3 = ( uint32_t ) 0u;
		}

		if (heth->Init.ChecksumMode == ETH_CHECKSUM_BY_HARDWARE)
		{
			/* Set the DMA Tx descriptors checksum insertion for TCP, UDP, and ICMP */
			pxDMADescriptor->txdes1 = ( uint32_t )( FTGMAC030_TXDES1_UDP_CHKSUM | FTGMAC030_TXDES1_TCP_CHKSUM | FTGMAC030_TXDES1_IP_CHKSUM );
		}
		else
		{
			pxDMADescriptor->txdes1 = ( uint32_t ) 0u;
		}

		/* Initialize the next descriptor with the Next Descriptor Polling Enable */
		if( i < ( ulBufferCount - 1 ) )
		{
			/* Set next descriptor address register with next descriptor base address */
			pxDMADescriptor->txdes2 = ( uint32_t )( pxDMATable + i + 1 );
		}
		else
		{
			/* For last descriptor, set next descriptor address register equal to the first descriptor base address */
			pxDMADescriptor->txdes2 = ( uint32_t ) pxDMATable;
		}
	}

	pxDMATable[ETH_TXBUFNB-1].txdes0 |= FTGMAC030_TXDES0_EDOTR;

	/* Set Transmit Descriptor List Address Register */
	heth->Instance->NPTXR_BADR = ( uint32_t ) pxDMATable;

	/* Set ETH HAL State to Ready */
	heth->State= HAL_ETH_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return HAL_OK;
}

/**
  * @brief  Initializes the DMA Rx descriptors in chain mode.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  DMARxDescTab: Pointer to the first Rx desc list
  * @param  RxBuff: Pointer to the first RxBuffer list
  * @param  RxBuffCount: Number of the used Rx desc in the list
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_DMARxDescListInit(ETH_HandleTypeDef *heth, ETH_DMARxDescTypeDef *pxDMATable, uint8_t *ucDataBuffer, uint32_t ulBufferCount)
{
	uint32_t i = 0;
	ETH_DMARxDescTypeDef *pxDMADescriptor;

	/* Process Locked */
	__HAL_LOCK( heth );

	/* Set the ETH peripheral state to BUSY */
	heth->State = HAL_ETH_STATE_BUSY;

	/* Set the RxDesc pointer with the first one of the pxDMATable list */
	heth->RxDesc = pxDMATable;

	/* Fill each DMA descriptor with the right values */
	for(i=0; i < ulBufferCount; i++)
	{
		/* Get the pointer on the ith member of the descriptor list */
		pxDMADescriptor = pxDMATable+i;

		/* Set Own bit of the Rx descriptor Status */
		pxDMADescriptor->rxdes0 &= ~FTGMAC030_RXDES0_RXPKT_RDY;

		/* Set Buffer1 address pointer */
		if( ucDataBuffer != NULL )
		{
			pxDMADescriptor->rxdes3 = ( uint32_t )( &ucDataBuffer[ i * ETH_RX_BUF_SIZE ] );
		}
		else
		{
			/* Buffer space is not provided because it uses zero-copy reception. */
			pxDMADescriptor->rxdes3 = ( uint32_t ) 0u;
		}

		/* Initialize the next descriptor with the Next Descriptor Polling Enable */
		if(i < (ulBufferCount-1))
		{
			/* Set next descriptor address register with next descriptor base address */
			pxDMADescriptor->rxdes2 = ( uint32_t )( pxDMATable + i + 1 );
		}
		else
		{
			/* For last descriptor, set next descriptor address register equal to the first descriptor base address */
			pxDMADescriptor->rxdes2 = ( uint32_t ) pxDMATable;
		}
	}

	pxDMATable[ETH_RXBUFNB-1].rxdes0 |= FTGMAC030_RXDES0_EDORR;

	/* Set Receive Descriptor List Address Register */
	heth->Instance->RXR_BADR = ( uint32_t ) pxDMATable;

	/* Set ETH HAL State to Ready */
	heth->State= HAL_ETH_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return HAL_OK;
}

/**
  * @}
  */

/** @defgroup ETH_Exported_Functions_Group2 IO operation functions
  *  @brief   Data transfers functions
  *
  @verbatim
  ==============================================================================
                          ##### IO operation functions #####
  ==============================================================================
  [..]  This section provides functions allowing to:
        (+) Transmit a frame
            HAL_ETH_TransmitFrame();
        (+) Receive a frame
            HAL_ETH_GetReceivedFrame();
            HAL_ETH_GetReceivedFrame_IT();
        (+) Read from an External PHY register
            HAL_ETH_ReadPHYRegister();
        (+) Write to an External PHY register
            HAL_ETH_WritePHYRegister();

  @endverbatim

  * @{
  */

/**
  * @brief  Sends an Ethernet frame.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  FrameLength: Amount of data to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_TransmitFrame(ETH_HandleTypeDef *heth, uint32_t FrameLength)
{
	uint32_t bufcount = 0, size = 0, i = 0;
	ETH_DMATxDescTypeDef *pxDmaTxDesc = heth->TxDesc;

	/* Process Locked */
	__HAL_LOCK( heth );

	/* Set the ETH peripheral state to BUSY */
	heth->State = HAL_ETH_STATE_BUSY;

	if( FrameLength == 0 )
	{
		/* Set ETH HAL state to READY */
		heth->State = HAL_ETH_STATE_READY;

		/* Process Unlocked */
		__HAL_UNLOCK( heth );

		return HAL_ERROR;
	}

	/* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
	if( pxDmaTxDesc->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN )
	{
		/* OWN bit set */
		heth->State = HAL_ETH_STATE_BUSY_TX;

		/* Process Unlocked */
		__HAL_UNLOCK( heth );

		return HAL_ERROR;
	}

	/* Get the number of needed Tx buffers for the current frame, rounding up. */
	bufcount = ( FrameLength + ETH_TX_BUF_SIZE - 1 ) / ETH_TX_BUF_SIZE;

	if (bufcount == 1)
	{
		/* Set LAST and FIRST segment */
		pxDmaTxDesc->txdes0 |= FTGMAC030_TXDES0_FTS | FTGMAC030_TXDES0_LTS;
		/* Set frame size */
		pxDmaTxDesc->txdes0 |= FTGMAC030_TXDES0_TXBUF_SIZE(FrameLength);
		/* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
		pxDmaTxDesc->txdes0 |= FTGMAC030_TXDES0_TXDMA_OWN;
		/* Point to next descriptor */
		heth->TxDesc = ( ETH_DMATxDescTypeDef * ) ( heth->TxDesc->txdes2 );
	}
	else
	{
		for( i = 0; i < bufcount; i++ )
		{
			/* Clear FIRST and LAST segment bits */
		uint32_t ulStatus = heth->TxDesc->txdes0 & ~( FTGMAC030_TXDES0_FTS | FTGMAC030_TXDES0_LTS );

			if( i == 0 )
			{
				/* Setting the first segment bit */
				heth->TxDesc->txdes0 = ulStatus | FTGMAC030_TXDES0_FTS;
			}

			/* Program size */
			if (i < (bufcount-1))
			{
				heth->TxDesc->txdes0 |= FTGMAC030_TXDES0_TXBUF_SIZE(ETH_TX_BUF_SIZE);
			}
			else
			{
				/* Setting the last segment bit */
				heth->TxDesc->txdes0 = ulStatus | FTGMAC030_TXDES0_LTS;
				size = FrameLength - (bufcount - 1) * ETH_TX_BUF_SIZE;
				heth->TxDesc->txdes0 |= FTGMAC030_TXDES0_TXBUF_SIZE(size);
			}

			/* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
			heth->TxDesc->txdes0 |= FTGMAC030_TXDES0_TXDMA_OWN;
			/* point to next descriptor */
			heth->TxDesc = (ETH_DMATxDescTypeDef *)( heth->TxDesc->txdes2 );
		}
	}

	__asm volatile( "dmb sy" );

	/* Set ETH HAL State to Ready */
	heth->State = HAL_ETH_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return HAL_OK;
}

/**
  * @brief  Checks for received frames.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_GetReceivedFrame_IT( ETH_HandleTypeDef *heth )
{
	return HAL_ETH_GetReceivedFrame( heth );
}

HAL_StatusTypeDef HAL_ETH_GetReceivedFrame( ETH_HandleTypeDef *heth )
{
uint32_t ulCounter = 0;
ETH_DMARxDescTypeDef *pxDescriptor = heth->RxDesc;
HAL_StatusTypeDef xResult = HAL_ERROR;

	/* Process Locked */
	__HAL_LOCK( heth );

	/* Check the ETH state to BUSY */
	heth->State = HAL_ETH_STATE_BUSY;

	/* Scan descriptors owned by CPU */
	while( ( ( pxDescriptor->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY ) == 0ul ) && ( ulCounter < ETH_RXBUFNB ) )
	{
	uint32_t ulStatus = pxDescriptor->rxdes0;

		/* Just for security. */
		ulCounter++;

		if( ( ulStatus & ( FTGMAC030_RXDES0_FRS | FTGMAC030_RXDES0_LRS ) ) == ( uint32_t )FTGMAC030_RXDES0_FRS )
		{
			/* First segment in frame, but not the last. */
			heth->RxFrameInfos.FSRxDesc = pxDescriptor;
			heth->RxFrameInfos.LSRxDesc = ( ETH_DMARxDescTypeDef *)NULL;
			heth->RxFrameInfos.SegCount = 1;
			/* Point to next descriptor. */
			pxDescriptor = (ETH_DMARxDescTypeDef*) (pxDescriptor->rxdes2);
		}
		else if( ( ulStatus & ( FTGMAC030_RXDES0_FRS | FTGMAC030_RXDES0_LRS ) ) == 0ul )
		{
			/* This is an intermediate segment, not first, not last. */
			/* Increment segment count. */
			heth->RxFrameInfos.SegCount++;
			/* Move to the next descriptor. */
			pxDescriptor = (ETH_DMARxDescTypeDef*) (pxDescriptor->rxdes2);
		}
		/* Must be a last segment */
		else
		{
			/* This is the last segment. */
			/* Check if last segment is first segment: one segment contains the frame */
			if( heth->RxFrameInfos.SegCount == 0 )
			{
				/* Remember the first segment. */
				heth->RxFrameInfos.FSRxDesc = pxDescriptor;
			}

			/* Increment segment count */
			heth->RxFrameInfos.SegCount++;

			/* Remember the last segment. */
			heth->RxFrameInfos.LSRxDesc = pxDescriptor;

			/* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
			heth->RxFrameInfos.length =	FTGMAC030_RXDES0_VDBC ( ulStatus ) - 4;

			/* Get the address of the buffer start address */
			heth->RxFrameInfos.buffer = heth->RxFrameInfos.FSRxDesc->rxdes3;

			/* Point to next descriptor */
			heth->RxDesc = (ETH_DMARxDescTypeDef*) (pxDescriptor->rxdes2);;

			/* Return OK status: a packet was received. */
			xResult = HAL_OK;
			break;
		}
	}

	/* Set ETH HAL State to Ready */
	heth->State = HAL_ETH_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return xResult;
}

void HAL_ETH_IRQHandler(ETH_HandleTypeDef *heth)
{
	uint32_t dmasr;

	dmasr = heth->Instance->ISR & INT_MASK_ALL_ENABLED;
	heth->Instance->ISR = dmasr;

//	lUDPLoggingPrintf( "HAL_ETH_IRQHandler: ISR=0x%x\n", dmasr );

	/* Frame received */
	if( ( dmasr & ( FTGMAC030_INT_RPKT_BUF | FTGMAC030_INT_NO_RXBUF ) ) != 0 )
	{
		/* Receive complete callback */
		HAL_ETH_RxCpltCallback( heth );
	}
	/* Frame transmitted */
	if( ( dmasr & ( FTGMAC030_INT_XPKT_ETH ) ) != 0 )
	{
		/* Transfer complete callback */
		HAL_ETH_TxCpltCallback( heth );
	}
	/* ETH DMA Error */
	if( ( dmasr & ( FTGMAC030_INT_RPKT_LOST | FTGMAC030_INT_AHB_ERR ) ) != 0 )
	{
		lUDPLoggingPrintf( "[ISR] = 0x%x: %s%s\n", dmasr,
		                   dmasr & FTGMAC030_INT_RPKT_LOST ?
		                   "RPKT_LOST " : "",
		                   dmasr & FTGMAC030_INT_AHB_ERR ?
		                   "AHB_ERR " : "" );
	}
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
__attribute__((weak)) void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the HAL_ETH_TxCpltCallback could be implemented in the user file
  */
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
__attribute__((weak)) void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the HAL_ETH_TxCpltCallback could be implemented in the user file
  */
}

/**
  * @brief  Ethernet transfer error callbacks
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
__attribute__((weak)) void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the HAL_ETH_TxCpltCallback could be implemented in the user file
  */
}

/**
  * @brief  Reads a PHY register
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param PHYReg: PHY register address, is the index of one of the 32 PHY register.
  *                This parameter can be one of the following values:
  *                   PHY_BCR: Transceiver Basic Control Register,
  *                   PHY_BSR: Transceiver Basic Status Register.
  *                   More PHY register could be read depending on the used PHY
  * @param RegValue: PHY register value
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_ReadPHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t *RegValue)
{
uint32_t tmpreg;
uint32_t tickstart = 0;
HAL_StatusTypeDef xResult;

	/* Check parameters */
	configASSERT(IS_ETH_PHY_ADDRESS(heth->Init.PhyAddress));

	/* Check the ETH peripheral state */
	if( heth->State == HAL_ETH_STATE_BUSY_RD )
	{
		xResult = HAL_BUSY;
	}
	else
	{
		__HAL_LOCK( heth );

		/* Set ETH HAL State to BUSY_RD */
		heth->State = HAL_ETH_STATE_BUSY_RD;

		/* Prepare the PHY Control register value */
		tmpreg = FTGMAC030_PHYCR_ST(1) | FTGMAC030_PHYCR_OP(2) |
		         FTGMAC030_PHYCR_PHYAD(heth->Init.PhyAddress) |
		         FTGMAC030_PHYCR_REGAD(PHYReg) |
		         FTGMAC030_PHYCR_MIIRD | FTGMAC030_PHYCR_MDC_CYCTHR(heth->Init.MDCCycThr);

		/* Write the result value into the PHY Control register */
		heth->Instance->PHYCR = tmpreg;

		/* Get tick */
		tickstart = HAL_GetTick();

		/* Check for the Busy flag */
		while( 1 )
		{
			tmpreg = heth->Instance->PHYCR;

			if( ( tmpreg & FTGMAC030_PHYCR_MIIRD ) == 0ul )
			{
				*RegValue = FTGMAC030_PHYDATA_MIIRDATA(heth->Instance->PHYDATA);
				xResult = HAL_OK;
				break;
			}
			/* Check for the Timeout */
			if( ( HAL_GetTick( ) - tickstart ) > PHY_READ_TO )
			{
				xResult = HAL_TIMEOUT;
				break;
			}

		}

		/* Set ETH HAL State to READY */
		heth->State = HAL_ETH_STATE_READY;

		/* Process Unlocked */
		__HAL_UNLOCK( heth );
	}

	if( xResult != HAL_OK )
	{
		lUDPLoggingPrintf( "ReadPHY: %d\n", xResult );
	}

	/* Return function status */
	return xResult;
}

/**
  * @brief  Writes to a PHY register.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  PHYReg: PHY register address, is the index of one of the 32 PHY register.
  *          This parameter can be one of the following values:
  *             PHY_BCR: Transceiver Control Register.
  *             More PHY register could be written depending on the used PHY
  * @param  RegValue: the value to write
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_WritePHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t RegValue)
{
uint32_t tmpreg = 0;
uint32_t tickstart = 0;
HAL_StatusTypeDef xResult;

	/* Check parameters */
	configASSERT( IS_ETH_PHY_ADDRESS( heth->Init.PhyAddress ) );

	/* Check the ETH peripheral state */
	if( heth->State == HAL_ETH_STATE_BUSY_WR )
	{
		xResult = HAL_BUSY;
	}
	else
	{
		__HAL_LOCK( heth );

		/* Set ETH HAL State to BUSY_WR */
		heth->State = HAL_ETH_STATE_BUSY_WR;

		/* Prepare the PHY Control register value */
		tmpreg = FTGMAC030_PHYCR_ST(1) | FTGMAC030_PHYCR_OP(1) |
		         FTGMAC030_PHYCR_PHYAD(heth->Init.PhyAddress) |
		         FTGMAC030_PHYCR_REGAD(PHYReg) |
		         FTGMAC030_PHYCR_MIIWR | FTGMAC030_PHYCR_MDC_CYCTHR(heth->Init.MDCCycThr);

		/* Give the value to the PHY Data register */
		heth->Instance->PHYDATA = FTGMAC030_PHYDATA_MIIWDATA(RegValue);

		/* Write the result value into the PHY Control register */
		heth->Instance->PHYCR = tmpreg;

		/* Get tick */
		tickstart = HAL_GetTick();

		/* Check for the Busy flag */
		while( 1 )
		{
			tmpreg = heth->Instance->PHYCR;

			if( ( tmpreg & FTGMAC030_PHYCR_MIIWR ) == 0ul )
			{
				xResult = HAL_OK;
				break;
			}
			/* Check for the Timeout */
			if( ( HAL_GetTick( ) - tickstart ) > PHY_WRITE_TO )
			{
				xResult = HAL_TIMEOUT;
				break;
			}
		}

		/* Set ETH HAL State to READY */
		heth->State = HAL_ETH_STATE_READY;
		/* Process Unlocked */
		__HAL_UNLOCK( heth );
	}

	if( xResult != HAL_OK )
	{
		lUDPLoggingPrintf( "WritePHY: %d\n", xResult );
	}

	/* Return function status */
	return xResult;
}

/**
  * @}
  */

/** @defgroup ETH_Exported_Functions_Group3 Peripheral Control functions
 *  @brief    Peripheral Control functions
 *
@verbatim
 ===============================================================================
                  ##### Peripheral Control functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Enable MAC and DMA transmission and reception.
          HAL_ETH_Start();
      (+) Disable MAC and DMA transmission and reception.
          HAL_ETH_Stop();
      (+) Set the MAC configuration in runtime mode
          HAL_ETH_ConfigMAC();

@endverbatim
  * @{
  */

 /**
  * @brief  Enables Ethernet MAC and DMA reception/transmission
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_Start( ETH_HandleTypeDef *heth )
{
	/* Process Locked */
	__HAL_LOCK( heth );

	/* Set the ETH peripheral state to BUSY */
	heth->State = HAL_ETH_STATE_BUSY;

	/* Enable transmit state machine of the MAC for transmission on the MII */
	ETH_MACTransmissionEnable( heth );

	/* Enable receive state machine of the MAC for reception from the MII */
	ETH_MACReceptionEnable( heth );

	/* Flush Transmit FIFO */
	ETH_FlushTransmitFIFO( heth );

	/* Start DMA transmission */
	ETH_DMATransmissionEnable( heth );

	/* Start DMA reception */
	ETH_DMAReceptionEnable( heth );

	/* Set the ETH state to READY*/
	heth->State = HAL_ETH_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return HAL_OK;
}

/**
  * @brief  Stop Ethernet MAC and DMA reception/transmission
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_Stop(ETH_HandleTypeDef *heth)
{
	/* Process Locked */
	__HAL_LOCK( heth );

	/* Set the ETH peripheral state to BUSY */
	heth->State = HAL_ETH_STATE_BUSY;

	/* Stop DMA transmission */
	ETH_DMATransmissionDisable( heth );

	/* Stop DMA reception */
	ETH_DMAReceptionDisable( heth );

	/* Disable receive state machine of the MAC for reception from the MII */
	ETH_MACReceptionDisable( heth );

	/* Flush Transmit FIFO */
	ETH_FlushTransmitFIFO( heth );

	/* Disable transmit state machine of the MAC for transmission on the MII */
	ETH_MACTransmissionDisable( heth );

	/* Set the ETH state*/
	heth->State = HAL_ETH_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return HAL_OK;
}

static void prvWriteMACCR( ETH_HandleTypeDef *heth, uint32_t ulValue)
{
	/* Enable the MAC transmission */
	heth->Instance->MACCR = ulValue;

	/* Wait until the write operation will be taken into account:
	   at least four TX_CLK/RX_CLK clock cycles.
	   Read it back, wait a ms and */
	( void ) heth->Instance->MACCR;

	HAL_Delay( ETH_REG_WRITE_DELAY );

	heth->Instance->MACCR = ulValue;
}

/**
  * @brief  Set ETH MAC Configuration.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  macconf: MAC Configuration structure
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETH_ConfigMAC(ETH_HandleTypeDef *heth)
{
	uint32_t tmpreg = 0;

	/* Process Locked */
	__HAL_LOCK( heth );

	/* Set the ETH peripheral state to BUSY */
	heth->State= HAL_ETH_STATE_BUSY;

	configASSERT(IS_ETH_SPEED(heth->Init.Speed));
	configASSERT(IS_ETH_DUPLEX_MODE(heth->Init.DuplexMode));

	/* config receive automatic poll time */
	heth->Instance->APTC = FTGMAC030_APTC_RXPOLL_CNT(1);

	/* config receive buffer size register */
	heth->Instance->RBSR = FTGMAC030_RBSR_SIZE(ETH_MAX_PACKET_SIZE);

	/* Set the ETHERNET MACCR value */
	tmpreg = FTGMAC030_MACCR_CRC_APD |
			 FTGMAC030_MACCR_RX_RUNT |
			 FTGMAC030_MACCR_RX_BROADPKT;
	if ( heth->Init.DuplexMode == ETH_MODE_FULLDUPLEX )
		tmpreg |= FTGMAC030_MACCR_FULLDUP;
	if ( heth->Init.Speed == ETH_SPEED_100M )
		tmpreg |= FTGMAC030_MACCR_FAST_MODE;

	/* Write to ETHERNET MACCR */
	prvWriteMACCR( heth, tmpreg );

	if(heth->Init.RxMode == ETH_RXINTERRUPT_MODE)
	{
		/* Enable the Ethernet Rx Interrupt */
		__HAL_ETH_DMA_ENABLE_IT(( heth ), INT_MASK_ALL_ENABLED);
	}

	/* Initialize MAC address in ethernet MAC */
	ETH_MACAddressConfig(heth, ETH_MAC_ADDRESS0, heth->Init.MACAddr);

	/* Set the ETH state to Ready */
	heth->State= HAL_ETH_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK( heth );

	/* Return function status */
	return HAL_OK;
}

/** @defgroup ETH_Exported_Functions_Group4 Peripheral State functions
  *  @brief   Peripheral State functions
  *
  @verbatim
  ===============================================================================
                         ##### Peripheral State functions #####
  ===============================================================================
  [..]
  This subsection permits to get in run-time the status of the peripheral
  and the data flow.
       (+) Get the ETH handle state:
           HAL_ETH_GetState();


  @endverbatim
  * @{
  */

/**
  * @brief  Return the ETH HAL state
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval HAL state
  */
HAL_ETH_StateTypeDef HAL_ETH_GetState(ETH_HandleTypeDef *heth)
{
	/* Return ETH state */
	return heth->State;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup ETH_Private_Functions
  * @{
  */

/**
  * @brief  Configures the selected MAC address.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  MacAddr: The MAC address to configure
  *          This parameter can be one of the following values:
  *             @arg ETH_MAC_Address0: MAC Address0
  *             @arg ETH_MAC_Address1: MAC Address1
  *             @arg ETH_MAC_Address2: MAC Address2
  *             @arg ETH_MAC_Address3: MAC Address3
  * @param  Addr: Pointer to MAC address buffer data (6 bytes)
  * @retval HAL status
  */
static void ETH_MACAddressConfig(ETH_HandleTypeDef *heth, uint32_t MacAddr, uint8_t *Addr)
{
	uint32_t tmpreg;

	/* Check the parameters */
	configASSERT( IS_ETH_MAC_ADDRESS0123( MacAddr ) );

	/* Calculate the selected MAC address high register */
	tmpreg = ( ( uint32_t )Addr[ 0 ] << 8) | (uint32_t)Addr[ 1 ];
	/* Load the selected MAC address high register */
	heth->Instance->MAC_MADR = tmpreg;

	/* Calculate the selected MAC address low register */
	tmpreg = ( ( uint32_t )Addr[ 2 ] << 24 ) | ( ( uint32_t )Addr[ 3 ] << 16 ) | ( ( uint32_t )Addr[ 4 ] << 8 ) | Addr[ 5 ];

	/* Load the selected MAC address low register */
	heth->Instance->MAC_LADR = tmpreg;
}

/**
  * @brief  Enables the MAC transmission.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_MACTransmissionEnable(ETH_HandleTypeDef *heth)
{
	uint32_t tmpreg = heth->Instance->MACCR | FTGMAC030_MACCR_TXMAC_EN;

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Disables the MAC transmission.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_MACTransmissionDisable(ETH_HandleTypeDef *heth)
{
	uint32_t tmpreg = heth->Instance->MACCR & ~( FTGMAC030_MACCR_TXMAC_EN );

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Enables the MAC reception.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_MACReceptionEnable(ETH_HandleTypeDef *heth)
{
	uint32_t tmpreg = heth->Instance->MACCR | FTGMAC030_MACCR_RXMAC_EN;

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Disables the MAC reception.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_MACReceptionDisable(ETH_HandleTypeDef *heth)
{
	uint32_t tmpreg = heth->Instance->MACCR & ~( FTGMAC030_MACCR_RXMAC_EN);

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Enables the DMA transmission.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_DMATransmissionEnable(ETH_HandleTypeDef *heth)
{
	/* Enable the DMA transmission */
	uint32_t tmpreg = heth->Instance->MACCR | FTGMAC030_MACCR_TXDMA_EN;

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Disables the DMA transmission.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_DMATransmissionDisable(ETH_HandleTypeDef *heth)
{
	/* Disable the DMA transmission */
	uint32_t tmpreg = heth->Instance->MACCR & ~( FTGMAC030_MACCR_TXDMA_EN );

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Enables the DMA reception.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_DMAReceptionEnable(ETH_HandleTypeDef *heth)
{
	/* Enable the DMA reception */
	uint32_t tmpreg = heth->Instance->MACCR | FTGMAC030_MACCR_RXDMA_EN;

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Disables the DMA reception.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
static void ETH_DMAReceptionDisable(ETH_HandleTypeDef *heth)
{
	/* Disable the DMA reception */
	uint32_t tmpreg = heth->Instance->MACCR & ~( FTGMAC030_MACCR_RXDMA_EN );

	prvWriteMACCR( heth, tmpreg );
}

/**
  * @brief  Clears the ETHERNET transmit FIFO.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void ETH_FlushTransmitFIFO(ETH_HandleTypeDef *heth)
{
	/* Set the Flush Transmit FIFO bit */
}

/**
  * @}
  */

/************ (C) COPYRIGHT Faraday Technology Corporation *****END OF FILE****/
