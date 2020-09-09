/**
  ******************************************************************************
  * @file    stm32l4xx_hal_usart_ex.c
  * @author  MCD Application Team
  * @brief   Extended USART HAL module driver.
  *          This file provides firmware functions to manage the following extended
  *          functionalities of the Universal Synchronous Receiver Transmitter Peripheral (USART).
  *           + Peripheral Control functions
  *
  *
  @verbatim
  ==============================================================================
               ##### USART peripheral extended features  #####
  ==============================================================================

    (#) FIFO mode enabling/disabling and RX/TX FIFO threshold programming.

        -@- When USART operates in FIFO mode, FIFO mode must be enabled prior
            starting RX/TX transfers. Also RX/TX FIFO thresholds must be
            configured prior starting RX/TX transfers.

    (#) Slave mode enabling/disabling and NSS pin configuration.

        -@- When USART operates in Slave mode, Slave mode must be enabled prior
            starting RX/TX transfers.

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

/** @defgroup USARTEx USARTEx
  * @brief USART Extended HAL module driver
  * @{
  */

#ifdef HAL_USART_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
/* UART RX FIFO depth */
#define RX_FIFO_DEPTH 8U

/* UART TX FIFO depth */
#define TX_FIFO_DEPTH 8U

#endif /* USART_CR1_FIFOEN */
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
/** @defgroup USARTEx_Private_Functions USARTEx Private Functions
  * @{
  */
static void USARTEx_SetNbDataToProcess(USART_HandleTypeDef *husart);
/**
  * @}
  */
#endif /* USART_CR1_FIFOEN */

/* Exported functions --------------------------------------------------------*/

/** @defgroup USARTEx_Exported_Functions  USARTEx Exported Functions
  * @{
  */

/** @defgroup USARTEx_Exported_Functions_Group1 IO operation functions
  * @brief Extended USART Transmit/Receive functions
  *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
    This subsection provides a set of FIFO mode related callback functions.

    (#) TX/RX Fifos Callbacks:
        (+) HAL_USARTEx_RxFifoFullCallback()
        (+) HAL_USARTEx_TxFifoEmptyCallback()

@endverbatim
  * @{
  */

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  USART RX Fifo full callback.
  * @param  husart USART handle.
  * @retval None
  */
__weak void HAL_USARTEx_RxFifoFullCallback(USART_HandleTypeDef *husart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(husart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_USARTEx_RxFifoFullCallback can be implemented in the user file.
   */
}

/**
  * @brief  USART TX Fifo empty callback.
  * @param  husart USART handle.
  * @retval None
  */
__weak void HAL_USARTEx_TxFifoEmptyCallback(USART_HandleTypeDef *husart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(husart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_USARTEx_TxFifoEmptyCallback can be implemented in the user file.
   */
}
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/** @defgroup USARTEx_Exported_Functions_Group2 Peripheral Control functions
  * @brief    Extended Peripheral Control functions
  *
@verbatim
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================
    [..] This section provides the following functions:
     (+) HAL_USARTEx_EnableSPISlaveMode() API enables the SPI slave mode
     (+) HAL_USARTEx_DisableSPISlaveMode() API disables the SPI slave mode
     (+) HAL_USARTEx_ConfigNSS API configures the Slave Select input pin (NSS)
     (+) HAL_USARTEx_EnableFifoMode() API enables the FIFO mode
     (+) HAL_USARTEx_DisableFifoMode() API disables the FIFO mode
     (+) HAL_USARTEx_SetTxFifoThreshold() API sets the TX FIFO threshold
     (+) HAL_USARTEx_SetRxFifoThreshold() API sets the RX FIFO threshold


@endverbatim
  * @{
  */

#if defined(USART_CR2_SLVEN)
/**
  * @brief  Enable the SPI slave mode.
  * @note When the USART operates in SPI slave mode, it handles data flow using
  *       the serial interface clock derived from the external SCLK signal
  *       provided by the external master SPI device.
  * @note In SPI slave mode, the USART must be enabled before starting the master
  *       communications (or between frames while the clock is stable). Otherwise,
  *       if the USART slave is enabled while the master is in the middle of a
  *       frame, it will become desynchronized with the master.
  * @note The data register of the slave needs to be ready before the first edge
  *       of the communication clock or before the end of the ongoing communication,
  *       otherwise the SPI slave will transmit zeros.
  * @param husart      USART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_USARTEx_EnableSlaveMode(USART_HandleTypeDef *husart)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_SPI_SLAVE_INSTANCE(husart->Instance));

  /* Process Locked */
  __HAL_LOCK(husart);

  husart->State = HAL_USART_STATE_BUSY;

  /* Save actual USART configuration */
  tmpcr1 = READ_REG(husart->Instance->CR1);

  /* Disable USART */
  __HAL_USART_DISABLE(husart);

  /* In SPI slave mode mode, the following bits must be kept cleared:
  - LINEN and CLKEN bit in the USART_CR2 register
  - HDSEL, SCEN and IREN bits in the USART_CR3 register.*/
  CLEAR_BIT(husart->Instance->CR2, (USART_CR2_LINEN | USART_CR2_CLKEN));
  CLEAR_BIT(husart->Instance->CR3, (USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN));

  /* Enable SPI slave mode */
  SET_BIT(husart->Instance->CR2, USART_CR2_SLVEN);

  /* Restore USART configuration */
  WRITE_REG(husart->Instance->CR1, tmpcr1);

  husart->SlaveMode = USART_SLAVEMODE_ENABLE;

  husart->State = HAL_USART_STATE_READY;

  /* Enable USART */
  __HAL_USART_ENABLE(husart);

  /* Process Unlocked */
  __HAL_UNLOCK(husart);

  return HAL_OK;
}

/**
  * @brief  Disable the SPI slave mode.
  * @param husart      USART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_USARTEx_DisableSlaveMode(USART_HandleTypeDef *husart)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_SPI_SLAVE_INSTANCE(husart->Instance));

  /* Process Locked */
  __HAL_LOCK(husart);

  husart->State = HAL_USART_STATE_BUSY;

  /* Save actual USART configuration */
  tmpcr1 = READ_REG(husart->Instance->CR1);

  /* Disable USART */
  __HAL_USART_DISABLE(husart);

  /* Disable SPI slave mode */
  CLEAR_BIT(husart->Instance->CR2, USART_CR2_SLVEN);

  /* Restore USART configuration */
  WRITE_REG(husart->Instance->CR1, tmpcr1);

  husart->SlaveMode = USART_SLAVEMODE_ENABLE;

  husart->State = HAL_USART_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(husart);

  return HAL_OK;
}

/**
  * @brief  Configure the Slave Select input pin (NSS).
  * @note Software NSS management: SPI slave will always be selected and NSS
  *       input pin will be ignored.
  * @note Hardware NSS management: the SPI slave selection depends on NSS
  *       input pin. The slave is selected when NSS is low and deselected when
  *       NSS is high.
  * @param husart      USART handle.
  * @param NSSConfig   NSS configuration.
  *          This parameter can be one of the following values:
  *            @arg @ref USART_NSS_HARD
  *            @arg @ref USART_NSS_SOFT
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_USARTEx_ConfigNSS(USART_HandleTypeDef *husart, uint32_t NSSConfig)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_SPI_SLAVE_INSTANCE(husart->Instance));
  assert_param(IS_USART_NSS(NSSConfig));

  /* Process Locked */
  __HAL_LOCK(husart);

  husart->State = HAL_USART_STATE_BUSY;

  /* Save actual USART configuration */
  tmpcr1 = READ_REG(husart->Instance->CR1);

  /* Disable USART */
  __HAL_USART_DISABLE(husart);

  /* Program DIS_NSS bit in the USART_CR2 register */
  MODIFY_REG(husart->Instance->CR2, USART_CR2_DIS_NSS, NSSConfig);

  /* Restore USART configuration */
  WRITE_REG(husart->Instance->CR1, tmpcr1);

  husart->State = HAL_USART_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(husart);

  return HAL_OK;
}
#endif /* USART_CR2_SLVEN */

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Enable the FIFO mode.
  * @param husart      USART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_USARTEx_EnableFifoMode(USART_HandleTypeDef *husart)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_FIFO_INSTANCE(husart->Instance));

  /* Process Locked */
  __HAL_LOCK(husart);

  husart->State = HAL_USART_STATE_BUSY;

  /* Save actual USART configuration */
  tmpcr1 = READ_REG(husart->Instance->CR1);

  /* Disable USART */
  __HAL_USART_DISABLE(husart);

  /* Enable FIFO mode */
  SET_BIT(tmpcr1, USART_CR1_FIFOEN);
  husart->FifoMode = USART_FIFOMODE_ENABLE;

  /* Restore USART configuration */
  WRITE_REG(husart->Instance->CR1, tmpcr1);

  /* Determine the number of data to process during RX/TX ISR execution */
  USARTEx_SetNbDataToProcess(husart);

  husart->State = HAL_USART_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(husart);

  return HAL_OK;
}

/**
  * @brief  Disable the FIFO mode.
  * @param husart      USART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_USARTEx_DisableFifoMode(USART_HandleTypeDef *husart)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_FIFO_INSTANCE(husart->Instance));

  /* Process Locked */
  __HAL_LOCK(husart);

  husart->State = HAL_USART_STATE_BUSY;

  /* Save actual USART configuration */
  tmpcr1 = READ_REG(husart->Instance->CR1);

  /* Disable USART */
  __HAL_USART_DISABLE(husart);

  /* Enable FIFO mode */
  CLEAR_BIT(tmpcr1, USART_CR1_FIFOEN);
  husart->FifoMode = USART_FIFOMODE_DISABLE;

  /* Restore USART configuration */
  WRITE_REG(husart->Instance->CR1, tmpcr1);

  husart->State = HAL_USART_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(husart);

  return HAL_OK;
}

/**
  * @brief  Set the TXFIFO threshold.
  * @param husart      USART handle.
  * @param Threshold  TX FIFO threshold value
  *          This parameter can be one of the following values:
  *            @arg @ref USART_TXFIFO_THRESHOLD_1_8
  *            @arg @ref USART_TXFIFO_THRESHOLD_1_4
  *            @arg @ref USART_TXFIFO_THRESHOLD_1_2
  *            @arg @ref USART_TXFIFO_THRESHOLD_3_4
  *            @arg @ref USART_TXFIFO_THRESHOLD_7_8
  *            @arg @ref USART_TXFIFO_THRESHOLD_8_8
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_USARTEx_SetTxFifoThreshold(USART_HandleTypeDef *husart, uint32_t Threshold)
{
  uint32_t tmpcr1;

  /* Check parameters */
  assert_param(IS_UART_FIFO_INSTANCE(husart->Instance));
  assert_param(IS_USART_TXFIFO_THRESHOLD(Threshold));

  /* Process Locked */
  __HAL_LOCK(husart);

  husart->State = HAL_USART_STATE_BUSY;

  /* Save actual USART configuration */
  tmpcr1 = READ_REG(husart->Instance->CR1);

  /* Disable USART */
  __HAL_USART_DISABLE(husart);

  /* Update TX threshold configuration */
  MODIFY_REG(husart->Instance->CR3, USART_CR3_TXFTCFG, Threshold);

  /* Determine the number of data to process during RX/TX ISR execution */
  USARTEx_SetNbDataToProcess(husart);

  /* Restore USART configuration */
  WRITE_REG(husart->Instance->CR1, tmpcr1);

  husart->State = HAL_USART_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(husart);

  return HAL_OK;
}

/**
  * @brief  Set the RXFIFO threshold.
  * @param husart      USART handle.
  * @param Threshold  RX FIFO threshold value
  *          This parameter can be one of the following values:
  *            @arg @ref USART_RXFIFO_THRESHOLD_1_8
  *            @arg @ref USART_RXFIFO_THRESHOLD_1_4
  *            @arg @ref USART_RXFIFO_THRESHOLD_1_2
  *            @arg @ref USART_RXFIFO_THRESHOLD_3_4
  *            @arg @ref USART_RXFIFO_THRESHOLD_7_8
  *            @arg @ref USART_RXFIFO_THRESHOLD_8_8
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_USARTEx_SetRxFifoThreshold(USART_HandleTypeDef *husart, uint32_t Threshold)
{
  uint32_t tmpcr1;

  /* Check the parameters */
  assert_param(IS_UART_FIFO_INSTANCE(husart->Instance));
  assert_param(IS_USART_RXFIFO_THRESHOLD(Threshold));

  /* Process Locked */
  __HAL_LOCK(husart);

  husart->State = HAL_USART_STATE_BUSY;

  /* Save actual USART configuration */
  tmpcr1 = READ_REG(husart->Instance->CR1);

  /* Disable USART */
  __HAL_USART_DISABLE(husart);

  /* Update RX threshold configuration */
  MODIFY_REG(husart->Instance->CR3, USART_CR3_RXFTCFG, Threshold);

  /* Determine the number of data to process during RX/TX ISR execution */
  USARTEx_SetNbDataToProcess(husart);

  /* Restore USART configuration */
  WRITE_REG(husart->Instance->CR1, tmpcr1);

  husart->State = HAL_USART_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(husart);

  return HAL_OK;
}
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup USARTEx_Private_Functions
  * @{
  */

#if defined(USART_CR1_FIFOEN)
/**
  * @brief Calculate the number of data to process in RX/TX ISR.
  * @note The RX FIFO depth and the TX FIFO depth is extracted from
  *       the USART configuration registers.
  * @param husart USART handle.
  * @retval None
  */
static void USARTEx_SetNbDataToProcess(USART_HandleTypeDef *husart)
{
  uint8_t rx_fifo_depth;
  uint8_t tx_fifo_depth;
  uint8_t rx_fifo_threshold;
  uint8_t tx_fifo_threshold;
  /* 2 0U/1U added for MISRAC2012-Rule-18.1_b and MISRAC2012-Rule-18.1_d */
  uint8_t numerator[]   = {1U, 1U, 1U, 3U, 7U, 1U, 0U, 0U};
  uint8_t denominator[] = {8U, 4U, 2U, 4U, 8U, 1U, 1U, 1U};

  if (husart->FifoMode == USART_FIFOMODE_DISABLE)
  {
    husart->NbTxDataToProcess = 1U;
    husart->NbRxDataToProcess = 1U;
  }
  else
  {
    rx_fifo_depth = RX_FIFO_DEPTH;
    tx_fifo_depth = TX_FIFO_DEPTH;
    rx_fifo_threshold = (uint8_t)((READ_BIT(husart->Instance->CR3, USART_CR3_RXFTCFG) >> USART_CR3_RXFTCFG_Pos) & 0xFFU);
    tx_fifo_threshold = (uint8_t)((READ_BIT(husart->Instance->CR3, USART_CR3_TXFTCFG) >> USART_CR3_TXFTCFG_Pos) & 0xFFU);
    husart->NbTxDataToProcess = ((uint16_t)tx_fifo_depth * numerator[tx_fifo_threshold]) / (uint16_t)denominator[tx_fifo_threshold];
    husart->NbRxDataToProcess = ((uint16_t)rx_fifo_depth * numerator[rx_fifo_threshold]) / (uint16_t)denominator[rx_fifo_threshold];
  }
}
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

#endif /* HAL_USART_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
