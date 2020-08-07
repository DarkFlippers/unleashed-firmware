/**
  ******************************************************************************
  * @file    stm32l4xx_hal_sai_ex.c
  * @author  MCD Application Team
  * @brief   SAI Extended HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionality of the SAI Peripheral Controller:
  *           + Modify PDM microphone delays.
  *
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

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */
#ifdef HAL_SAI_MODULE_ENABLED
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)

/** @defgroup SAIEx SAIEx
  * @brief SAI Extended HAL module driver
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup SAIEx_Private_Defines SAIEx Extended Private Defines
  * @{
  */
#define SAI_PDM_DELAY_MASK          0x77U
#define SAI_PDM_DELAY_OFFSET        8U
#define SAI_PDM_RIGHT_DELAY_OFFSET  4U
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup SAIEx_Exported_Functions SAIEx Extended Exported Functions
  * @{
  */

/** @defgroup SAIEx_Exported_Functions_Group1 Peripheral Control functions
  * @brief    SAIEx control functions
  *
@verbatim
 ===============================================================================
                 ##### Extended features functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Modify PDM microphone delays

@endverbatim
  * @{
  */

/**
  * @brief  Configure PDM microphone delays.
  * @param  hsai SAI handle.
  * @param  pdmMicDelay Microphone delays configuration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SAIEx_ConfigPdmMicDelay(SAI_HandleTypeDef *hsai, SAIEx_PdmMicDelayParamTypeDef *pdmMicDelay)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t offset;

  /* Check that SAI sub-block is SAI1 sub-block A */
  if (hsai->Instance != SAI1_Block_A)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check microphone delay parameters */
    assert_param(IS_SAI_PDM_MIC_PAIRS_NUMBER(pdmMicDelay->MicPair));
    assert_param(IS_SAI_PDM_MIC_DELAY(pdmMicDelay->LeftDelay));
    assert_param(IS_SAI_PDM_MIC_DELAY(pdmMicDelay->RightDelay));

    /* Compute offset on PDMDLY register according mic pair number */
    offset = SAI_PDM_DELAY_OFFSET * (pdmMicDelay->MicPair - 1U);

    /* Check SAI state and offset */
    if ((hsai->State != HAL_SAI_STATE_RESET) && (offset <= 24U))
    {
      /* Reset current delays for specified microphone */
      SAI1->PDMDLY &= ~(SAI_PDM_DELAY_MASK << offset);

      /* Apply new microphone delays */
      SAI1->PDMDLY |= (((pdmMicDelay->RightDelay << SAI_PDM_RIGHT_DELAY_OFFSET) | pdmMicDelay->LeftDelay) << offset);
    }
    else
    {
      status = HAL_ERROR;
    }
  }
  return status;
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

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
#endif /* HAL_SAI_MODULE_ENABLED */
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
