/**
  ******************************************************************************
  * @file    stm32l4xx_hal_sd_ex.c
  * @author  MCD Application Team
  * @brief   SD card Extended HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Secure Digital (SD) peripheral:
  *           + Extended features functions
  *
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
  [..]
   The SD Extension HAL driver can be used as follows:
   (+) Set card in High Speed mode using HAL_SDEx_HighSpeed() function.
   (+) Configure Buffer0 and Buffer1 start address and Buffer size using HAL_SDEx_ConfigDMAMultiBuffer() function.
   (+) Start Read and Write for multibuffer mode using HAL_SDEx_ReadBlocksDMAMultiBuffer() and HAL_SDEx_WriteBlocksDMAMultiBuffer() functions.

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
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

/** @defgroup SDEx SDEx
  * @brief SD Extended HAL module driver
  * @{
  */

#ifdef HAL_SD_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern uint32_t SD_HighSpeed(SD_HandleTypeDef *hsd);
/* Exported functions --------------------------------------------------------*/
/** @addtogroup SDEx_Exported_Functions
  * @{
  */

/** @addtogroup SDEx_Exported_Functions_Group1
 *  @brief   High Speed function
 *
@verbatim
  ==============================================================================
          ##### High Speed function #####
  ==============================================================================
  [..]
    This section provides function allowing to configure the card in High Speed mode.

@endverbatim
  * @{
  */

/**
  * @brief  Switches the SD card to High Speed mode.
  *         This API must be used after "Transfer State"
  * @note   This operation should be followed by the configuration
  *         of PLL to have SDMMCCK clock between 50 and 120 MHz
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
uint32_t HAL_SDEx_HighSpeed(SD_HandleTypeDef *hsd)
{
  return SD_HighSpeed (hsd);
}

/**
  * @brief  Enable/Disable the SD Transceiver 1.8V Mode Callback.
  * @param  status: Voltage Switch State
  * @retval None
  */
__weak void HAL_SDEx_DriveTransceiver_1_8V_Callback(FlagStatus status)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(status);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_SD_EnableTransciver could be implemented in the user file
   */
}

/**
  * @}
  */

/** @addtogroup SDEx_Exported_Functions_Group2
 *  @brief   Multibuffer functions
 *
@verbatim
  ==============================================================================
          ##### Multibuffer functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to configure the multibuffer mode and start read and write
    multibuffer mode for SD HAL driver.

@endverbatim
  * @{
  */

/**
  * @brief  Configure DMA Dual Buffer mode. The Data transfer is managed by an Internal DMA.
  * @param  hsd: SD handle
  * @param  pDataBuffer0: Pointer to the buffer0 that will contain/receive the transfered data
  * @param  pDataBuffer1: Pointer to the buffer1 that will contain/receive the transfered data
  * @param  BufferSize: Size of Buffer0 in Blocks. Buffer0 and Buffer1 must have the same size.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SDEx_ConfigDMAMultiBuffer(SD_HandleTypeDef *hsd, uint32_t *pDataBuffer0, uint32_t *pDataBuffer1, uint32_t BufferSize)
{
  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->Instance->IDMABASE0= (uint32_t) pDataBuffer0;
    hsd->Instance->IDMABASE1= (uint32_t) pDataBuffer1;
    hsd->Instance->IDMABSIZE= (uint32_t) (BLOCKSIZE * BufferSize);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Reads block(s) from a specified address in a card. The received Data will be stored in Buffer0 and Buffer1.
  *         Buffer0, Buffer1 and BufferSize need to be configured by function HAL_SDEx_ConfigDMAMultiBuffer before call this function.
  * @param  hsd: SD handle
  * @param  BlockAdd: Block Address from where data is to be read
  * @param  NumberOfBlocks: Total number of blocks to read
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SDEx_ReadBlocksDMAMultiBuffer(SD_HandleTypeDef *hsd, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t DmaBase0_reg, DmaBase1_reg;
  uint32_t add = BlockAdd;

  if(hsd->State == HAL_SD_STATE_READY)
  {
    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    DmaBase0_reg = hsd->Instance->IDMABASE0;
    DmaBase1_reg = hsd->Instance->IDMABASE1;
    if ((hsd->Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U))
    {
      hsd->ErrorCode = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0;
    /* Clear old Flags*/
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    hsd->ErrorCode = HAL_SD_ERROR_NONE;
    hsd->State = HAL_SD_STATE_BUSY;

    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      add *= 512U;
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_DISABLE;
    (void)SDMMC_ConfigData(hsd->Instance, &config);

    hsd->Instance->DCTRL |= SDMMC_DCTRL_FIFORST;

    __SDMMC_CMDTRANS_ENABLE( hsd->Instance);

    hsd->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0;

    __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND | SDMMC_IT_IDMABTC));

    /* Read Blocks in DMA mode */
    hsd->Context = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

    /* Read Multi Block command */
    errorstate = SDMMC_CmdReadMultiBlock(hsd->Instance, add);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      hsd->State = HAL_SD_STATE_READY;
      hsd->ErrorCode |= errorstate;
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
  *         Buffer0, Buffer1 and BufferSize need to be configured by function HAL_SDEx_ConfigDMAMultiBuffer before call this function.
  * @param  hsd: SD handle
  * @param  BlockAdd: Block Address from where data is to be read
  * @param  NumberOfBlocks: Total number of blocks to read
  * @retval HAL status
*/
HAL_StatusTypeDef HAL_SDEx_WriteBlocksDMAMultiBuffer(SD_HandleTypeDef *hsd, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t DmaBase0_reg, DmaBase1_reg;
  uint32_t add = BlockAdd;

  if(hsd->State == HAL_SD_STATE_READY)
  {
    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    DmaBase0_reg = hsd->Instance->IDMABASE0;
    DmaBase1_reg = hsd->Instance->IDMABASE1;
    if ((hsd->Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U))
    {
      hsd->ErrorCode = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0;

    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    hsd->State = HAL_SD_STATE_BUSY;

    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      add *= 512U;
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_DISABLE;
    (void)SDMMC_ConfigData(hsd->Instance, &config);

    __SDMMC_CMDTRANS_ENABLE( hsd->Instance);

    hsd->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0;

    __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND | SDMMC_IT_IDMABTC));

    /* Write Blocks in DMA mode */
    hsd->Context = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

    /* Write Multi Block command */
    errorstate = SDMMC_CmdWriteMultiBlock(hsd->Instance, add);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      hsd->State = HAL_SD_STATE_READY;
      hsd->ErrorCode |= errorstate;
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
  * @param  hsd:           pointer to a SD_HandleTypeDef structure.
  * @param  Buffer:        the buffer to be changed, This parameter can be one of
  *                        the following values: SD_DMA_BUFFER0 or SD_DMA_BUFFER1
  * @param  pDataBuffer:   The new address
  * @note   The BUFFER0 address can be changed only when the current transfer use
  *         BUFFER1 and the BUFFER1 address can be changed only when the current
  *         transfer use BUFFER0.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SDEx_ChangeDMABuffer(SD_HandleTypeDef *hsd, HAL_SDEx_DMABuffer_MemoryTypeDef Buffer, uint32_t *pDataBuffer)
{
  if(Buffer == SD_DMA_BUFFER0)
  {
    /* change the buffer0 address */
    hsd->Instance->IDMABASE0 = (uint32_t)pDataBuffer;
  }
  else
  {
    /* change the memory1 address */
    hsd->Instance->IDMABASE1 = (uint32_t)pDataBuffer;
  }

  return HAL_OK;
}

/**
  * @brief Read DMA Buffer 0 Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
__weak void HAL_SDEx_Read_DMADoubleBuffer0CpltCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SDEx_Read_DMADoubleBuffer0CpltCallback can be implemented in the user file
   */
}

/**
  * @brief Read DMA Buffer 1 Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
__weak void HAL_SDEx_Read_DMADoubleBuffer1CpltCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SDEx_Read_DMADoubleBuffer1CpltCallback can be implemented in the user file
   */
}

/**
  * @brief Write DMA Buffer 0 Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
__weak void HAL_SDEx_Write_DMADoubleBuffer0CpltCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SDEx_Write_DMADoubleBuffer0CpltCallback can be implemented in the user file
   */
}

/**
  * @brief Write DMA Buffer 1 Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
__weak void HAL_SDEx_Write_DMADoubleBuffer1CpltCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SDEx_Write_DMADoubleBuffer0CpltCallback can be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_SD_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
