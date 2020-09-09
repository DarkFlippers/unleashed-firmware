/**
  ******************************************************************************
  * @file    stm32l4xx_ll_crs.h
  * @author  MCD Application Team
  * @brief   CRS LL module driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#if defined(USE_FULL_LL_DRIVER)

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_crs.h"
#include "stm32l4xx_ll_bus.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(CRS)

/** @defgroup CRS_LL CRS
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup CRS_LL_Exported_Functions
  * @{
  */

/** @addtogroup CRS_LL_EF_Init
  * @{
  */

/**
  * @brief  De-Initializes CRS peripheral registers to their default reset values.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: CRS registers are de-initialized
  *          - ERROR: not applicable
  */
ErrorStatus LL_CRS_DeInit(void)
{
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_CRS);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_CRS);

  return  SUCCESS;
}



/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* defined(CRS) */

/**
  * @}
  */
  
#endif /* USE_FULL_LL_DRIVER */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
