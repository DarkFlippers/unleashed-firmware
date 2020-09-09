/**
  ******************************************************************************
  * @file    stm32l4xx_hal_smartcard_ex.c
  * @author  MCD Application Team
  * @brief   SMARTCARD HAL module driver.
  *          This file provides extended firmware functions to manage the following
  *          functionalities of the SmartCard.
  *           + Initialization and de-initialization functions
  *           + Peripheral Control functions
  *
  @verbatim
  =============================================================================
               ##### SMARTCARD peripheral extended features  #####
  =============================================================================
  [..]
  The Extended SMARTCARD HAL driver can be used as follows:

    (#) After having configured the SMARTCARD basic features with HAL_SMARTCARD_Init(),
        then program SMARTCARD advanced features if required (TX/RX pins swap, TimeOut,
        auto-retry counter,...) in the hsmartcard AdvancedInit structure.

    (#) FIFO mode enabling/disabling and RX/TX FIFO threshold programming.

        -@- When SMARTCARD operates in FIFO mode, FIFO mode must be enabled prior
            starting RX/TX transfers. Also RX/TX FIFO thresholds must be
            configured prior starting RX/TX transfers.

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
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup SMARTCARDEx SMARTCARDEx
  * @brief SMARTCARD Extended HAL module driver
  * @{
  */
#ifdef HAL_SMARTCARD_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* UART RX FIFO depth */
#define RX_FIFO_DEPTH 8U

/* UART TX FIFO depth */
#define TX_FIFO_DEPTH 8U

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
static void SMARTCARDEx_SetNbDataToProcess(SMARTCARD_HandleTypeDef *hsmartcard);

#endif /* USART_CR1_FIFOEN */
/* Exported functions --------------------------------------------------------*/
/** @defgroup SMARTCARDEx_Exported_Functions  SMARTCARD Extended Exported Functions
  * @{
  */

/** @defgroup SMARTCARDEx_Exported_Functions_Group1 Extended Peripheral Control functions
  * @brief    Extended control functions
  *
@verbatim
  ===============================================================================
                      ##### Peripheral Control functions #####
  ===============================================================================
  [..]
  This subsection provides a set of functions allowing to initialize the SMARTCARD.
     (+) HAL_SMARTCARDEx_BlockLength_Config() API allows to configure the Block Length on the fly
     (+) HAL_SMARTCARDEx_TimeOut_Config() API allows to configure the receiver timeout value on the fly
     (+) HAL_SMARTCARDEx_EnableReceiverTimeOut() API enables the receiver timeout feature
     (+) HAL_SMARTCARDEx_DisableReceiverTimeOut() API disables the receiver timeout feature

@endverbatim
  * @{
  */

/** @brief Update on the fly the SMARTCARD block length in RTOR register.
  * @param hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param BlockLength SMARTCARD block length (8-bit long at most)
  * @retval None
  */
void HAL_SMARTCARDEx_BlockLength_Config(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t BlockLength)
{
  MODIFY_REG(hsmartcard->Instance->RTOR, USART_RTOR_BLEN, ((uint32_t)BlockLength << USART_RTOR_BLEN_Pos));
}

/** @brief Update on the fly the receiver timeout value in RTOR register.
  * @param hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param TimeOutValue receiver timeout value in number of baud blocks. The timeout
  *                     value must be less or equal to 0x0FFFFFFFF.
  * @retval None
  */
void HAL_SMARTCARDEx_TimeOut_Config(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t TimeOutValue)
{
  assert_param(IS_SMARTCARD_TIMEOUT_VALUE(hsmartcard->Init.TimeOutValue));
  MODIFY_REG(hsmartcard->Instance->RTOR, USART_RTOR_RTO, TimeOutValue);
}

/** @brief Enable the SMARTCARD receiver timeout feature.
  * @param hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARDEx_EnableReceiverTimeOut(SMARTCARD_HandleTypeDef *hsmartcard)
{
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

    /* Set the USART RTOEN bit */
    SET_BIT(hsmartcard->Instance->CR2, USART_CR2_RTOEN);

    hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmartcard);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/** @brief Disable the SMARTCARD receiver timeout feature.
  * @param hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARDEx_DisableReceiverTimeOut(SMARTCARD_HandleTypeDef *hsmartcard)
{
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

    /* Clear the USART RTOEN bit */
    CLEAR_BIT(hsmartcard->Instance->CR2, USART_CR2_RTOEN);

    hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmartcard);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @}
  */

/** @defgroup SMARTCARDEx_Exported_Functions_Group2 Extended Peripheral IO operation functions
  * @brief   SMARTCARD Transmit and Receive functions
  *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
    [..]
    This subsection provides a set of FIFO mode related callback functions.

    (#) TX/RX Fifos Callbacks:
        (+) HAL_SMARTCARDEx_RxFifoFullCallback()
        (+) HAL_SMARTCARDEx_TxFifoEmptyCallback()

@endverbatim
  * @{
  */

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  SMARTCARD RX Fifo full callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                   the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARDEx_RxFifoFullCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARDEx_RxFifoFullCallback can be implemented in the user file.
   */
}

/**
  * @brief  SMARTCARD TX Fifo empty callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                   the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARDEx_TxFifoEmptyCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARDEx_TxFifoEmptyCallback can be implemented in the user file.
   */
}

#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/** @defgroup SMARTCARD_Exported_Functions_Group3 Extended Peripheral Peripheral Control functions
  *  @brief   SMARTCARD control functions
  *
@verbatim
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to control the SMARTCARD.
     (+) HAL_SMARTCARDEx_EnableFifoMode() API enables the FIFO mode
     (+) HAL_SMARTCARDEx_DisableFifoMode() API disables the FIFO mode
     (+) HAL_SMARTCARDEx_SetTxFifoThreshold() API sets the TX FIFO threshold
     (+) HAL_SMARTCARDEx_SetRxFifoThreshold() API sets the RX FIFO threshold
@endverbatim
  * @{
  */

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Enable the FIFO mode.
  * @param hsmartcard SMARTCARD handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARDEx_EnableFifoMode(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_FIFO_INSTANCE(hsmartcard->Instance));

  /* Process Locked */
  __HAL_LOCK(hsmartcard);

  hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

  /* Save actual SMARTCARD configuration */
  tmpcr1 = READ_REG(hsmartcard->Instance->CR1);

  /* Disable SMARTCARD */
  __HAL_SMARTCARD_DISABLE(hsmartcard);

  /* Enable FIFO mode */
  SET_BIT(tmpcr1, USART_CR1_FIFOEN);
  hsmartcard->FifoMode = SMARTCARD_FIFOMODE_ENABLE;

  /* Restore SMARTCARD configuration */
  WRITE_REG(hsmartcard->Instance->CR1, tmpcr1);

  /* Determine the number of data to process during RX/TX ISR execution */
  SMARTCARDEx_SetNbDataToProcess(hsmartcard);

  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hsmartcard);

  return HAL_OK;
}

/**
  * @brief  Disable the FIFO mode.
  * @param hsmartcard SMARTCARD handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARDEx_DisableFifoMode(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_FIFO_INSTANCE(hsmartcard->Instance));

  /* Process Locked */
  __HAL_LOCK(hsmartcard);

  hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

  /* Save actual SMARTCARD configuration */
  tmpcr1 = READ_REG(hsmartcard->Instance->CR1);

  /* Disable SMARTCARD */
  __HAL_SMARTCARD_DISABLE(hsmartcard);

  /* Enable FIFO mode */
  CLEAR_BIT(tmpcr1, USART_CR1_FIFOEN);
  hsmartcard->FifoMode = SMARTCARD_FIFOMODE_DISABLE;

  /* Restore SMARTCARD configuration */
  WRITE_REG(hsmartcard->Instance->CR1, tmpcr1);

  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hsmartcard);

  return HAL_OK;
}

/**
  * @brief  Set the TXFIFO threshold.
  * @param hsmartcard      SMARTCARD handle.
  * @param Threshold  TX FIFO threshold value
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_TXFIFO_THRESHOLD_1_8
  *            @arg @ref SMARTCARD_TXFIFO_THRESHOLD_1_4
  *            @arg @ref SMARTCARD_TXFIFO_THRESHOLD_1_2
  *            @arg @ref SMARTCARD_TXFIFO_THRESHOLD_3_4
  *            @arg @ref SMARTCARD_TXFIFO_THRESHOLD_7_8
  *            @arg @ref SMARTCARD_TXFIFO_THRESHOLD_8_8
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARDEx_SetTxFifoThreshold(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t Threshold)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_FIFO_INSTANCE(hsmartcard->Instance));
  assert_param(IS_SMARTCARD_TXFIFO_THRESHOLD(Threshold));

  /* Process Locked */
  __HAL_LOCK(hsmartcard);

  hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

  /* Save actual SMARTCARD configuration */
  tmpcr1 = READ_REG(hsmartcard->Instance->CR1);

  /* Disable SMARTCARD */
  __HAL_SMARTCARD_DISABLE(hsmartcard);

  /* Update TX threshold configuration */
  MODIFY_REG(hsmartcard->Instance->CR3, USART_CR3_TXFTCFG, Threshold);

  /* Determine the number of data to process during RX/TX ISR execution */
  SMARTCARDEx_SetNbDataToProcess(hsmartcard);

  /* Restore SMARTCARD configuration */
  MODIFY_REG(hsmartcard->Instance->CR1, USART_CR1_UE, tmpcr1);

  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hsmartcard);

  return HAL_OK;
}

/**
  * @brief  Set the RXFIFO threshold.
  * @param hsmartcard      SMARTCARD handle.
  * @param Threshold  RX FIFO threshold value
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_RXFIFO_THRESHOLD_1_8
  *            @arg @ref SMARTCARD_RXFIFO_THRESHOLD_1_4
  *            @arg @ref SMARTCARD_RXFIFO_THRESHOLD_1_2
  *            @arg @ref SMARTCARD_RXFIFO_THRESHOLD_3_4
  *            @arg @ref SMARTCARD_RXFIFO_THRESHOLD_7_8
  *            @arg @ref SMARTCARD_RXFIFO_THRESHOLD_8_8
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARDEx_SetRxFifoThreshold(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t Threshold)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_FIFO_INSTANCE(hsmartcard->Instance));
  assert_param(IS_SMARTCARD_RXFIFO_THRESHOLD(Threshold));

  /* Process Locked */
  __HAL_LOCK(hsmartcard);

  hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

  /* Save actual SMARTCARD configuration */
  tmpcr1 = READ_REG(hsmartcard->Instance->CR1);

  /* Disable SMARTCARD */
  __HAL_SMARTCARD_DISABLE(hsmartcard);

  /* Update RX threshold configuration */
  MODIFY_REG(hsmartcard->Instance->CR3, USART_CR3_RXFTCFG, Threshold);

  /* Determine the number of data to process during RX/TX ISR execution */
  SMARTCARDEx_SetNbDataToProcess(hsmartcard);

  /* Restore SMARTCARD configuration */
  MODIFY_REG(hsmartcard->Instance->CR1, USART_CR1_UE, tmpcr1);

  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hsmartcard);

  return HAL_OK;
}
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup SMARTCARDEx_Private_Functions  SMARTCARD Extended private Functions
  * @{
  */

#if defined(USART_CR1_FIFOEN)
/**
  * @brief Calculate the number of data to process in RX/TX ISR.
  * @note The RX FIFO depth and the TX FIFO depth is extracted from
  *       the USART configuration registers.
  * @param hsmartcard SMARTCARD handle.
  * @retval None
  */
static void SMARTCARDEx_SetNbDataToProcess(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint8_t rx_fifo_depth;
  uint8_t tx_fifo_depth;
  uint8_t rx_fifo_threshold;
  uint8_t tx_fifo_threshold;
  /* 2 0U/1U added for MISRAC2012-Rule-18.1_b and MISRAC2012-Rule-18.1_d */
  uint8_t numerator[]   = {1U, 1U, 1U, 3U, 7U, 1U, 0U, 0U};
  uint8_t denominator[] = {8U, 4U, 2U, 4U, 8U, 1U, 1U, 1U};

  if (hsmartcard->FifoMode == SMARTCARD_FIFOMODE_DISABLE)
  {
    hsmartcard->NbTxDataToProcess = 1U;
    hsmartcard->NbRxDataToProcess = 1U;
  }
  else
  {
    rx_fifo_depth = RX_FIFO_DEPTH;
    tx_fifo_depth = TX_FIFO_DEPTH;
    rx_fifo_threshold = (uint8_t)(READ_BIT(hsmartcard->Instance->CR3, USART_CR3_RXFTCFG) >> USART_CR3_RXFTCFG_Pos);
    tx_fifo_threshold = (uint8_t)(READ_BIT(hsmartcard->Instance->CR3, USART_CR3_TXFTCFG) >> USART_CR3_TXFTCFG_Pos);
    hsmartcard->NbTxDataToProcess = ((uint16_t)tx_fifo_depth * numerator[tx_fifo_threshold]) / (uint16_t)denominator[tx_fifo_threshold];
    hsmartcard->NbRxDataToProcess = ((uint16_t)rx_fifo_depth * numerator[rx_fifo_threshold]) / (uint16_t)denominator[rx_fifo_threshold];
  }
}

#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

#endif /* HAL_SMARTCARD_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
