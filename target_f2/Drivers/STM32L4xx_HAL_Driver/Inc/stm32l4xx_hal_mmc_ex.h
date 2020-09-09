/**
  ******************************************************************************
  * @file    stm32l4xx_hal_mmc_ex.h
  * @author  MCD Application Team
  * @brief   Header file of SD HAL extended module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_MMC_EX_H
#define STM32L4xx_HAL_MMC_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup MMCEx
  * @brief SD HAL extended module driver
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/
/** @addtogroup MMCEx_Exported_Types
  * @{
  */

/** @addtogroup MMCEx_Exported_Types_Group1
  * @{
  */ 
typedef enum
{
  MMC_DMA_BUFFER0      = 0x00U,    /*!< selects MMC internal DMA Buffer 0     */
  MMC_DMA_BUFFER1      = 0x01U,    /*!< selects MMC internal DMA Buffer 1     */

}HAL_MMCEx_DMABuffer_MemoryTypeDef;


/** 
  * @}
  */
  
/** 
  * @}
  */  
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup MMCEx_Exported_Functions MMCEx Exported Functions
  * @{
  */
  
/** @defgroup MMCEx_Exported_Functions_Group1 MultiBuffer functions
  * @{
  */
HAL_StatusTypeDef HAL_MMCEx_ConfigDMAMultiBuffer(MMC_HandleTypeDef *hmmc, uint32_t * pDataBuffer0, uint32_t * pDataBuffer1, uint32_t BufferSize);
HAL_StatusTypeDef HAL_MMCEx_ReadBlocksDMAMultiBuffer(MMC_HandleTypeDef *hmmc, uint32_t BlockAdd, uint32_t NumberOfBlocks);
HAL_StatusTypeDef HAL_MMCEx_WriteBlocksDMAMultiBuffer(MMC_HandleTypeDef *hmmc, uint32_t BlockAdd, uint32_t NumberOfBlocks);
HAL_StatusTypeDef HAL_MMCEx_ChangeDMABuffer(MMC_HandleTypeDef *hmmc, HAL_MMCEx_DMABuffer_MemoryTypeDef Buffer, uint32_t *pDataBuffer);

void HAL_MMCEx_Read_DMADoubleBuffer0CpltCallback(MMC_HandleTypeDef *hmmc);
void HAL_MMCEx_Read_DMADoubleBuffer1CpltCallback(MMC_HandleTypeDef *hmmc);
void HAL_MMCEx_Write_DMADoubleBuffer0CpltCallback(MMC_HandleTypeDef *hmmc);
void HAL_MMCEx_Write_DMADoubleBuffer1CpltCallback(MMC_HandleTypeDef *hmmc);

/**
  * @}
  */
  
/**
  * @}
  */
  
/* Private types -------------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private functions prototypes ----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
  
/**
  * @}
  */

/**
  * @}
  */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#ifdef __cplusplus
}
#endif


#endif /* STM32L4xx_HAL_MMCEx_H */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
