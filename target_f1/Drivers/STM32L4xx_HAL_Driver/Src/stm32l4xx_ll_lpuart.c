/**
  ******************************************************************************
  * @file    stm32l4xx_ll_lpuart.c
  * @author  MCD Application Team
  * @brief   LPUART LL module driver.
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
#include "stm32l4xx_ll_lpuart.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_bus.h"
#ifdef USE_FULL_ASSERT
#include "stm32_assert.h"
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined (LPUART1)

/** @addtogroup LPUART_LL
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @addtogroup LPUART_LL_Private_Constants
  * @{
  */

/**
  * @}
  */


/* Private macros ------------------------------------------------------------*/
/** @addtogroup LPUART_LL_Private_Macros
  * @{
  */

/* Check of parameters for configuration of LPUART registers                  */

#if defined(USART_PRESC_PRESCALER)
#define IS_LL_LPUART_PRESCALER(__VALUE__)  (((__VALUE__) == LL_LPUART_PRESCALER_DIV1) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV2) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV4) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV6) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV8) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV10) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV12) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV16) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV32) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV64) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV128) \
                                            || ((__VALUE__) == LL_LPUART_PRESCALER_DIV256))

#endif /* USART_PRESC_PRESCALER */
/* __BAUDRATE__ Depending on constraints applicable for LPUART BRR register   */
/*              value :                                                       */
/*                - fck must be in the range [3 x baudrate, 4096 x baudrate]  */
/*                - LPUART_BRR register value should be >= 0x300              */
/*                - LPUART_BRR register value should be <= 0xFFFFF (20 bits)  */
/*              Baudrate specified by the user should belong to [8, 40000000].*/
#define IS_LL_LPUART_BAUDRATE(__BAUDRATE__) (((__BAUDRATE__) <= 40000000U) && ((__BAUDRATE__) >= 8U))

/* __VALUE__ BRR content must be greater than or equal to 0x300. */
#define IS_LL_LPUART_BRR_MIN(__VALUE__)   ((__VALUE__) >= 0x300U)

/* __VALUE__ BRR content must be lower than or equal to 0xFFFFF. */
#define IS_LL_LPUART_BRR_MAX(__VALUE__)   ((__VALUE__) <= 0x000FFFFFU)

#define IS_LL_LPUART_DIRECTION(__VALUE__) (((__VALUE__) == LL_LPUART_DIRECTION_NONE) \
                                           || ((__VALUE__) == LL_LPUART_DIRECTION_RX) \
                                           || ((__VALUE__) == LL_LPUART_DIRECTION_TX) \
                                           || ((__VALUE__) == LL_LPUART_DIRECTION_TX_RX))

#define IS_LL_LPUART_PARITY(__VALUE__) (((__VALUE__) == LL_LPUART_PARITY_NONE) \
                                        || ((__VALUE__) == LL_LPUART_PARITY_EVEN) \
                                        || ((__VALUE__) == LL_LPUART_PARITY_ODD))

#define IS_LL_LPUART_DATAWIDTH(__VALUE__) (((__VALUE__) == LL_LPUART_DATAWIDTH_7B) \
                                           || ((__VALUE__) == LL_LPUART_DATAWIDTH_8B) \
                                           || ((__VALUE__) == LL_LPUART_DATAWIDTH_9B))

#define IS_LL_LPUART_STOPBITS(__VALUE__) (((__VALUE__) == LL_LPUART_STOPBITS_1) \
                                          || ((__VALUE__) == LL_LPUART_STOPBITS_2))

#define IS_LL_LPUART_HWCONTROL(__VALUE__) (((__VALUE__) == LL_LPUART_HWCONTROL_NONE) \
                                           || ((__VALUE__) == LL_LPUART_HWCONTROL_RTS) \
                                           || ((__VALUE__) == LL_LPUART_HWCONTROL_CTS) \
                                           || ((__VALUE__) == LL_LPUART_HWCONTROL_RTS_CTS))

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup LPUART_LL_Exported_Functions
  * @{
  */

/** @addtogroup LPUART_LL_EF_Init
  * @{
  */

/**
  * @brief  De-initialize LPUART registers (Registers restored to their default values).
  * @param  LPUARTx LPUART Instance
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: LPUART registers are de-initialized
  *          - ERROR: not applicable
  */
ErrorStatus LL_LPUART_DeInit(USART_TypeDef *LPUARTx)
{
  ErrorStatus status = SUCCESS;

  /* Check the parameters */
  assert_param(IS_LPUART_INSTANCE(LPUARTx));

  if (LPUARTx == LPUART1)
  {
    /* Force reset of LPUART peripheral */
    LL_APB1_GRP2_ForceReset(LL_APB1_GRP2_PERIPH_LPUART1);

    /* Release reset of LPUART peripheral */
    LL_APB1_GRP2_ReleaseReset(LL_APB1_GRP2_PERIPH_LPUART1);
  }
  else
  {
    status = ERROR;
  }

  return (status);
}

/**
  * @brief  Initialize LPUART registers according to the specified
  *         parameters in LPUART_InitStruct.
  * @note   As some bits in LPUART configuration registers can only be written when the LPUART is disabled (USART_CR1_UE bit =0),
  *         LPUART Peripheral should be in disabled state prior calling this function. Otherwise, ERROR result will be returned.
  * @note   Baud rate value stored in LPUART_InitStruct BaudRate field, should be valid (different from 0).
  * @param  LPUARTx LPUART Instance
  * @param  LPUART_InitStruct pointer to a @ref LL_LPUART_InitTypeDef structure
  *         that contains the configuration information for the specified LPUART peripheral.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: LPUART registers are initialized according to LPUART_InitStruct content
  *          - ERROR: Problem occurred during LPUART Registers initialization
  */
ErrorStatus LL_LPUART_Init(USART_TypeDef *LPUARTx, LL_LPUART_InitTypeDef *LPUART_InitStruct)
{
  ErrorStatus status = ERROR;
  uint32_t periphclk;

  /* Check the parameters */
  assert_param(IS_LPUART_INSTANCE(LPUARTx));
#if defined(USART_PRESC_PRESCALER)
  assert_param(IS_LL_LPUART_PRESCALER(LPUART_InitStruct->PrescalerValue));
#endif /* USART_PRESC_PRESCALER */
  assert_param(IS_LL_LPUART_BAUDRATE(LPUART_InitStruct->BaudRate));
  assert_param(IS_LL_LPUART_DATAWIDTH(LPUART_InitStruct->DataWidth));
  assert_param(IS_LL_LPUART_STOPBITS(LPUART_InitStruct->StopBits));
  assert_param(IS_LL_LPUART_PARITY(LPUART_InitStruct->Parity));
  assert_param(IS_LL_LPUART_DIRECTION(LPUART_InitStruct->TransferDirection));
  assert_param(IS_LL_LPUART_HWCONTROL(LPUART_InitStruct->HardwareFlowControl));

  /* LPUART needs to be in disabled state, in order to be able to configure some bits in
     CRx registers. Otherwise (LPUART not in Disabled state) => return ERROR */
  if (LL_LPUART_IsEnabled(LPUARTx) == 0U)
  {
    /*---------------------------- LPUART CR1 Configuration -----------------------
     * Configure LPUARTx CR1 (LPUART Word Length, Parity and Transfer Direction bits) with parameters:
     * - DataWidth:          USART_CR1_M bits according to LPUART_InitStruct->DataWidth value
     * - Parity:             USART_CR1_PCE, USART_CR1_PS bits according to LPUART_InitStruct->Parity value
     * - TransferDirection:  USART_CR1_TE, USART_CR1_RE bits according to LPUART_InitStruct->TransferDirection value
     */
    MODIFY_REG(LPUARTx->CR1,
               (USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE),
               (LPUART_InitStruct->DataWidth | LPUART_InitStruct->Parity | LPUART_InitStruct->TransferDirection));

    /*---------------------------- LPUART CR2 Configuration -----------------------
     * Configure LPUARTx CR2 (Stop bits) with parameters:
     * - Stop Bits:          USART_CR2_STOP bits according to LPUART_InitStruct->StopBits value.
     */
    LL_LPUART_SetStopBitsLength(LPUARTx, LPUART_InitStruct->StopBits);

    /*---------------------------- LPUART CR3 Configuration -----------------------
     * Configure LPUARTx CR3 (Hardware Flow Control) with parameters:
     * - HardwareFlowControl: USART_CR3_RTSE, USART_CR3_CTSE bits according to LPUART_InitStruct->HardwareFlowControl value.
     */
    LL_LPUART_SetHWFlowCtrl(LPUARTx, LPUART_InitStruct->HardwareFlowControl);

    /*---------------------------- LPUART BRR Configuration -----------------------
     * Retrieve Clock frequency used for LPUART Peripheral
     */
    periphclk = LL_RCC_GetLPUARTClockFreq(LL_RCC_LPUART1_CLKSOURCE);

    /* Configure the LPUART Baud Rate :
    #if defined(USART_PRESC_PRESCALER)
       - prescaler value is required
    #endif
       - valid baud rate value (different from 0) is required
       - Peripheral clock as returned by RCC service, should be valid (different from 0).
    */
    if ((periphclk != LL_RCC_PERIPH_FREQUENCY_NO)
        && (LPUART_InitStruct->BaudRate != 0U))
    {
      status = SUCCESS;
      LL_LPUART_SetBaudRate(LPUARTx,
                            periphclk,
#if defined(USART_PRESC_PRESCALER)
                            LPUART_InitStruct->PrescalerValue,
#endif /* USART_PRESC_PRESCALER */
                            LPUART_InitStruct->BaudRate);

      /* Check BRR is greater than or equal to 0x300 */
      assert_param(IS_LL_LPUART_BRR_MIN(LPUARTx->BRR));

      /* Check BRR is lower than or equal to 0xFFFFF */
      assert_param(IS_LL_LPUART_BRR_MAX(LPUARTx->BRR));
    }

#if defined(USART_PRESC_PRESCALER)
    /*---------------------------- LPUART PRESC Configuration -----------------------
     * Configure LPUARTx PRESC (Prescaler) with parameters:
     * - PrescalerValue: LPUART_PRESC_PRESCALER bits according to LPUART_InitStruct->PrescalerValue value.
     */
    LL_LPUART_SetPrescaler(LPUARTx, LPUART_InitStruct->PrescalerValue);
#endif /* USART_PRESC_PRESCALER */
  }

  return (status);
}

/**
  * @brief Set each @ref LL_LPUART_InitTypeDef field to default value.
  * @param LPUART_InitStruct pointer to a @ref LL_LPUART_InitTypeDef structure
  *                          whose fields will be set to default values.
  * @retval None
  */

void LL_LPUART_StructInit(LL_LPUART_InitTypeDef *LPUART_InitStruct)
{
  /* Set LPUART_InitStruct fields to default values */
#if defined(USART_PRESC_PRESCALER)
  LPUART_InitStruct->PrescalerValue      = LL_LPUART_PRESCALER_DIV1;
#endif /* USART_PRESC_PRESCALER */
  LPUART_InitStruct->BaudRate            = 9600U;
  LPUART_InitStruct->DataWidth           = LL_LPUART_DATAWIDTH_8B;
  LPUART_InitStruct->StopBits            = LL_LPUART_STOPBITS_1;
  LPUART_InitStruct->Parity              = LL_LPUART_PARITY_NONE ;
  LPUART_InitStruct->TransferDirection   = LL_LPUART_DIRECTION_TX_RX;
  LPUART_InitStruct->HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
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

#endif /* defined (LPUART1) */

/**
  * @}
  */

#endif /* USE_FULL_LL_DRIVER */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

