/**
  ******************************************************************************
  * @file    stm32l4xx_hal_flash_ramfunc.h
  * @author  MCD Application Team
  * @brief   Header file of FLASH RAMFUNC driver.
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
#ifndef __STM32L4xx_FLASH_RAMFUNC_H
#define __STM32L4xx_FLASH_RAMFUNC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup FLASH_RAMFUNC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup FLASH_RAMFUNC_Exported_Functions
  * @{
  */

/** @addtogroup FLASH_RAMFUNC_Exported_Functions_Group1
  * @{
  */
/* Peripheral Control functions  ************************************************/
__RAM_FUNC  HAL_FLASHEx_EnableRunPowerDown(void);
__RAM_FUNC  HAL_FLASHEx_DisableRunPowerDown(void);
#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
__RAM_FUNC  HAL_FLASHEx_OB_DBankConfig(uint32_t DBankConfig);
#endif
/**
  * @}
  */

/**
  * @}
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

#endif /* __STM32L4xx_FLASH_RAMFUNC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
