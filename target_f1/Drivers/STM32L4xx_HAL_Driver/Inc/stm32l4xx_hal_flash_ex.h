/**
  ******************************************************************************
  * @file    stm32l4xx_hal_flash_ex.h
  * @author  MCD Application Team
  * @brief   Header file of FLASH HAL Extended module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L4xx_HAL_FLASH_EX_H
#define __STM32L4xx_HAL_FLASH_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup FLASHEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#if defined (FLASH_CFGR_LVEN)
/** @addtogroup FLASHEx_Exported_Constants
  * @{
  */
/** @defgroup FLASHEx_LVE_PIN_CFG FLASHEx LVE pin configuration
  * @{
  */
#define FLASH_LVE_PIN_CTRL     0x00000000U       /*!< LVE FLASH pin controlled by power controller       */
#define FLASH_LVE_PIN_FORCED   FLASH_CFGR_LVEN   /*!< LVE FLASH pin enforced to low (external SMPS used) */
/**
  * @}
  */

/**
  * @}
  */
#endif /* FLASH_CFGR_LVEN */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup FLASHEx_Exported_Functions
  * @{
  */

/* Extended Program operation functions  *************************************/
/** @addtogroup FLASHEx_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError);
HAL_StatusTypeDef HAL_FLASHEx_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit);
HAL_StatusTypeDef HAL_FLASHEx_OBProgram(FLASH_OBProgramInitTypeDef *pOBInit);
void              HAL_FLASHEx_OBGetConfig(FLASH_OBProgramInitTypeDef *pOBInit);
/**
  * @}
  */

#if defined (FLASH_CFGR_LVEN)
/** @addtogroup FLASHEx_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef HAL_FLASHEx_ConfigLVEPin(uint32_t ConfigLVE);
/**
  * @}
  */
#endif /* FLASH_CFGR_LVEN */

/**
  * @}
  */

/* Private function ----------------------------------------------------------*/
/** @addtogroup FLASHEx_Private_Functions FLASHEx Private Functions
 * @{
 */
void FLASH_PageErase(uint32_t Page, uint32_t Banks);
void FLASH_FlushCaches(void);
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/**
  @cond 0
  */
#if defined (FLASH_CFGR_LVEN)
#define IS_FLASH_LVE_PIN(CFG)  (((CFG) == FLASH_LVE_PIN_CTRL) || ((CFG) == FLASH_LVE_PIN_FORCED))
#endif /* FLASH_CFGR_LVEN */
/**
  @endcond
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_HAL_FLASH_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
