/**
  ******************************************************************************
  * @file    stm32l4xx_hal_mmc_ex.c
  * @author  MCD Application Team
  * @brief   MMC card Extended HAL module driver.
  *          This file provides firmware functions to manage the following 
  *          functionalities of the Secure Digital (MMC) peripheral:
  *           + Extended features functions
  *         
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
  [..]
   The MMC Extension HAL driver can be used as follows:
   (+) Configure Buffer0 and Buffer1 start address and Buffer size using HAL_MMCEx_ConfigDMAMultiBuffer() function.

   (+) Start Read and Write for multibuffer mode using HAL_MMCEx_ReadBlocksDMAMultiBuffer() and HAL_MMCEx_WriteBlocksDMAMultiBuffer() functions.
   
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup MMCEx MMCEx
  * @brief MMC Extended HAL module driver
  * @{
  */

#ifdef HAL_MMC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup MMCEx_Exported_Types MMCEx Exported Types
  * @{
  */

/** @defgroup MMCEx_Exported_Types_Group1 MMC Internal DMA Buffer structure
 *  @brief   Multibuffer functions 
 *
@verbatim    
  ==============================================================================
          ##### Multibuffer functions #####
  ==============================================================================
  [..]  
    This section provides functions allowing to configure the multibuffer mode and start read and write 
    multibuffer mode for MMC HAL driver.
      
@endverbatim
  * @{
  */

/**
  * @brief  Configure DMA Dual Buffer mode. The Data transfer is managed by an Internal DMA.
  * @param  hmmc: MMC handle
  * @param  pDataBuffer0: Pointer to the buffer0 that will contain/receive the transfered data
  * @param  pDataBuffer1: Pointer to the buffer1 that will contain/receive the transfered data
  * @param  BufferSize: Size of Buffer0 in Blocks. Buffer0 and Buffer1 must have the same size.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MMCEx_ConfigDMAMultiBuffer(MMC_HandleTypeDef *hmmc, uint32_t * pDataBuffer0, uint32_t * pDataBuffer1, uint32_t BufferSize)
{
  if(hmmc->State == HAL_MMC_STATE_READY)
  {
    hmmc->Instance->IDMABASE0= (uint32_t) pDataBuffer0 ;
    hmmc->Instance->IDMABASE1= (uint32_t) pDataBuffer1 ;
    hmmc->Instance->IDMABSIZE= (uint32_t) (BLOCKSIZE * BufferSize);
    
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}
  
/**
  * @brief  Reads block(s) from a specified address in a card. The received Data will be stored in Buffer0 and Buffer1.
  *         Buffer0, Buffer1 and BufferSize need to be configured by function HAL_MMCEx_ConfigDMAMultiBuffer before call this function.
  * @param  hmmc: MMC handle
  * @param  BlockAdd: Block Address from where data is to be read  
  * @param  NumberOfBlocks: Total number of blocks to read
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MMCEx_ReadBlocksDMAMultiBuffer(MMC_HandleTypeDef *hmmc, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t DmaBase0_reg, DmaBase1_reg;
  uint32_t errorstate;
  uint32_t add = BlockAdd;
  
  if(hmmc->State == HAL_MMC_STATE_READY)
  {
    if((BlockAdd + NumberOfBlocks) > (hmmc->MmcCard.LogBlockNbr))
    {
      hmmc->ErrorCode |= HAL_MMC_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }
    
    DmaBase0_reg = hmmc->Instance->IDMABASE0;
    DmaBase1_reg = hmmc->Instance->IDMABASE1;
    if ((hmmc->Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U))
    {
      hmmc->ErrorCode = HAL_MMC_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }
    
    /* Initialize data control register */
    hmmc->Instance->DCTRL = 0;
    
    hmmc->ErrorCode = HAL_MMC_ERROR_NONE;
    hmmc->State = HAL_MMC_STATE_BUSY;

    if ((hmmc->MmcCard.CardType) != MMC_HIGH_CAPACITY_CARD)
    {
      add *= 512U;
    }
    
    /* Configure the MMC DPSM (Data Path State Machine) */ 
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = MMC_BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_DISABLE;
    (void)SDMMC_ConfigData(hmmc->Instance, &config);
    
    hmmc->Instance->DCTRL |= SDMMC_DCTRL_FIFORST;
    
    __SDMMC_CMDTRANS_ENABLE( hmmc->Instance);
    
    hmmc->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0; 

     __HAL_MMC_ENABLE_IT(hmmc, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND | SDMMC_FLAG_IDMATE | SDMMC_FLAG_IDMABTC));
   
    /* Read Blocks in DMA mode */
    hmmc->Context = (MMC_CONTEXT_READ_MULTIPLE_BLOCK | MMC_CONTEXT_DMA);
    
    /* Read Multi Block command */
    errorstate = SDMMC_CmdReadMultiBlock(hmmc->Instance, add);
    if(errorstate != HAL_MMC_ERROR_NONE)
    {
      hmmc->State = HAL_MMC_STATE_READY;
      hmmc->ErrorCode |= errorstate;
      return HAL_ERROR;
    }
    
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
   
}

/**
  * @brief  Write block(s) to a specified address in a card. The transfered Data are stored in Buffer0 and Buffer1.
  *         Buffer0, Buffer1 and BufferSize need to be configured by function HAL_MMCEx_ConfigDMAMultiBuffer before call this function.
  * @param  hmmc: MMC handle
  * @param  BlockAdd: Block Address from where data is to be read  
  * @param  NumberOfBlocks: Total number of blocks to read
  * @retval HAL status
*/
HAL_StatusTypeDef HAL_MMCEx_WriteBlocksDMAMultiBuffer(MMC_HandleTypeDef *hmmc, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t DmaBase0_reg, DmaBase1_reg;
  uint32_t add = BlockAdd;
  
  if(hmmc->State == HAL_MMC_STATE_READY)
  {
    if((BlockAdd + NumberOfBlocks) > (hmmc->MmcCard.LogBlockNbr))
    {
      hmmc->ErrorCode |= HAL_MMC_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }
    
    DmaBase0_reg = hmmc->Instance->IDMABASE0;
    DmaBase1_reg = hmmc->Instance->IDMABASE1;
    if ((hmmc->Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U))
    {
      hmmc->ErrorCode = HAL_MMC_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }
    
    /* Initialize data control register */
    hmmc->Instance->DCTRL = 0;
    
    hmmc->ErrorCode = HAL_MMC_ERROR_NONE;
    
    hmmc->State = HAL_MMC_STATE_BUSY;

    if ((hmmc->MmcCard.CardType) != MMC_HIGH_CAPACITY_CARD)
    {
      add *= 512U;
    }
    
    /* Configure the MMC DPSM (Data Path State Machine) */ 
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = MMC_BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_DISABLE;
    (void)SDMMC_ConfigData(hmmc->Instance, &config);
    
    __SDMMC_CMDTRANS_ENABLE( hmmc->Instance);
    
    hmmc->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0; 
 
    __HAL_MMC_ENABLE_IT(hmmc, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND | SDMMC_FLAG_IDMATE | SDMMC_FLAG_IDMABTC));
   
    /* Write Blocks in DMA mode */
    hmmc->Context = (MMC_CONTEXT_WRITE_MULTIPLE_BLOCK | MMC_CONTEXT_DMA);
    
    /* Write Multi Block command */
    errorstate = SDMMC_CmdWriteMultiBlock(hmmc->Instance, add);
    if(errorstate != HAL_MMC_ERROR_NONE)
    {
      hmmc->State = HAL_MMC_STATE_READY;
      hmmc->ErrorCode |= errorstate;
      return HAL_ERROR;
    }
    
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }  
}

  
/**
  * @brief  Change the DMA Buffer0 or Buffer1 address on the fly.
  * @param  hmmc:           pointer to a MMC_HandleTypeDef structure.
  * @param  Buffer:        the buffer to be changed, This parameter can be one of 
  *                        the following values: MMC_DMA_BUFFER0 or MMC_DMA_BUFFER1
  * @param  pDataBuffer:   The new address
  * @note   The BUFFER0 address can be changed only when the current transfer use
  *         BUFFER1 and the BUFFER1 address can be changed only when the current 
  *         transfer use BUFFER0.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MMCEx_ChangeDMABuffer(MMC_HandleTypeDef *hmmc, HAL_MMCEx_DMABuffer_MemoryTypeDef Buffer, uint32_t *pDataBuffer)
{
  if(Buffer == MMC_DMA_BUFFER0)
  {
    /* change the buffer0 address */
    hmmc->Instance->IDMABASE0 = (uint32_t)pDataBuffer;
  }
  else
  {
    /* change the memory1 address */
    hmmc->Instance->IDMABASE1 = (uint32_t)pDataBuffer;
  }
  
  return HAL_OK;
}

/**
  * @brief Read DMA Buffer 0 Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
__weak void HAL_MMCEx_Read_DMADoubleBuffer0CpltCallback(MMC_HandleTypeDef *hmmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hmmc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MMCEx_Read_DMADoubleBuffer0CpltCallback can be implemented in the user file
   */
}

/**
  * @brief Read DMA Buffer 1 Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
__weak void HAL_MMCEx_Read_DMADoubleBuffer1CpltCallback(MMC_HandleTypeDef *hmmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hmmc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MMCEx_Read_DMADoubleBuffer1CpltCallback can be implemented in the user file
   */
}

/**
  * @brief Write DMA Buffer 0 Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
__weak void HAL_MMCEx_Write_DMADoubleBuffer0CpltCallback(MMC_HandleTypeDef *hmmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hmmc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MMCEx_Write_DMADoubleBuffer0CpltCallback can be implemented in the user file
   */
}

/**
  * @brief Write DMA Buffer 1 Transfer completed callbacks
  * @param hmmc: MMC handle
  * @retval None
  */
__weak void HAL_MMCEx_Write_DMADoubleBuffer1CpltCallback(MMC_HandleTypeDef *hmmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hmmc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MMCEx_Write_DMADoubleBuffer0CpltCallback can be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_MMC_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
