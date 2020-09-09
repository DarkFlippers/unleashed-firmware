/**
  ******************************************************************************
  * @file    stm32l4xx_ll_swpmi.c
  * @author  MCD Application Team
  * @brief   SWPMI LL module driver.
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
#include "stm32l4xx_ll_swpmi.h"
#include "stm32l4xx_ll_bus.h"
#ifdef  USE_FULL_ASSERT
#include "stm32_assert.h"
#else
#define assert_param(expr) ((void)0U)
#endif

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(SWPMI1)

/** @addtogroup SWPMI_LL
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/** @addtogroup SWPMI_LL_Private_Macros
  * @{
  */

#define IS_LL_SWPMI_BITRATE_VALUE(__VALUE__) (((__VALUE__) <= 63U))

#define IS_LL_SWPMI_SW_BUFFER_RX(__VALUE__) (((__VALUE__) == LL_SWPMI_SW_BUFFER_RX_SINGLE) \
                                          || ((__VALUE__) == LL_SWPMI_SW_BUFFER_RX_MULTI))

#define IS_LL_SWPMI_SW_BUFFER_TX(__VALUE__) (((__VALUE__) == LL_SWPMI_SW_BUFFER_TX_SINGLE) \
                                          || ((__VALUE__) == LL_SWPMI_SW_BUFFER_TX_MULTI))

#define IS_LL_SWPMI_VOLTAGE_CLASS(__VALUE__) (((__VALUE__) == LL_SWPMI_VOLTAGE_CLASS_C) \
                                           || ((__VALUE__) == LL_SWPMI_VOLTAGE_CLASS_B))

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SWPMI_LL_Exported_Functions
  * @{
  */

/** @addtogroup SWPMI_LL_EF_Init
  * @{
  */

/**
  * @brief  De-initialize the SWPMI peripheral registers to their default reset values.
  * @param  SWPMIx SWPMI Instance
  * @retval An ErrorStatus enumeration value
  *          - SUCCESS: SWPMI registers are de-initialized
  *          - ERROR: Not applicable
  */
ErrorStatus LL_SWPMI_DeInit(SWPMI_TypeDef *SWPMIx)
{
  ErrorStatus status = SUCCESS;

  /* Check the parameter */
  assert_param(IS_SWPMI_INSTANCE(SWPMIx));

  if (SWPMIx == SWPMI1)
  {
    LL_APB1_GRP2_ForceReset(LL_APB1_GRP2_PERIPH_SWPMI1);
    LL_APB1_GRP2_ReleaseReset(LL_APB1_GRP2_PERIPH_SWPMI1);
  }
  else
  {
    status = ERROR;
  }

  return status;
}

/**
  * @brief  Initialize the SWPMI peripheral according to the specified parameters in the SWPMI_InitStruct.
  * @note   As some bits in SWPMI configuration registers can only be written when the SWPMI is deactivated (SWPMI_CR_SWPACT bit = 0),
  *         SWPMI IP should be in deactivated state prior calling this function. Otherwise, ERROR result will be returned.
  * @param  SWPMIx           SWPMI Instance
  * @param  SWPMI_InitStruct pointer to a @ref LL_SWPMI_InitTypeDef structure that contains
  *                          the configuration information for the SWPMI peripheral.
  * @retval An ErrorStatus enumeration value
  *          - SUCCESS: SWPMI registers are initialized
  *          - ERROR: SWPMI registers are not initialized
  */
ErrorStatus LL_SWPMI_Init(SWPMI_TypeDef *SWPMIx, LL_SWPMI_InitTypeDef *SWPMI_InitStruct)
{
  ErrorStatus status = SUCCESS;

  /* Check the parameters */
  assert_param(IS_SWPMI_INSTANCE(SWPMIx));
  assert_param(IS_LL_SWPMI_BITRATE_VALUE(SWPMI_InitStruct->BitRatePrescaler));
  assert_param(IS_LL_SWPMI_SW_BUFFER_TX(SWPMI_InitStruct->TxBufferingMode));
  assert_param(IS_LL_SWPMI_SW_BUFFER_RX(SWPMI_InitStruct->RxBufferingMode));
  assert_param(IS_LL_SWPMI_VOLTAGE_CLASS(SWPMI_InitStruct->VoltageClass));

  /* SWPMI needs to be in deactivated state, in order to be able to configure some bits */
  if (LL_SWPMI_IsActivated(SWPMIx) == 0U)
  {
    /* Configure the BRR register (Bitrate) */
    LL_SWPMI_SetBitRatePrescaler(SWPMIx, SWPMI_InitStruct->BitRatePrescaler);

    /* Configure the voltage class */
    LL_SWPMI_SetVoltageClass(SWPMIx, SWPMI_InitStruct->VoltageClass);

    /* Set the new configuration of the SWPMI peripheral */
    MODIFY_REG(SWPMIx->CR,
              (SWPMI_CR_RXMODE | SWPMI_CR_TXMODE),
              (SWPMI_InitStruct->TxBufferingMode | SWPMI_InitStruct->RxBufferingMode));
  }
  /* Else (SWPMI not in deactivated state => return ERROR) */
  else
  {
    status = ERROR;
  }

  return status;
}

/**
  * @brief  Set each @ref LL_SWPMI_InitTypeDef field to default value.
  * @param  SWPMI_InitStruct pointer to a @ref LL_SWPMI_InitTypeDef structure that contains
  *                          the configuration information for the SWPMI peripheral.
  * @retval None
  */
void LL_SWPMI_StructInit(LL_SWPMI_InitTypeDef *SWPMI_InitStruct)
{
  /* Set SWPMI_InitStruct fields to default values */
  SWPMI_InitStruct->VoltageClass     = LL_SWPMI_VOLTAGE_CLASS_C;
  SWPMI_InitStruct->BitRatePrescaler = (uint32_t)0x00000001;
  SWPMI_InitStruct->TxBufferingMode  = LL_SWPMI_SW_BUFFER_TX_SINGLE;
  SWPMI_InitStruct->RxBufferingMode  = LL_SWPMI_SW_BUFFER_RX_SINGLE;
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

#endif /* SWPMI1 */

/**
  * @}
  */

#endif /* USE_FULL_LL_DRIVER */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
