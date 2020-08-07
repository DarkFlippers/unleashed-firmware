/**
  ******************************************************************************
  * @file    stm32l4xx_hal_usart_ex.h
  * @author  MCD Application Team
  * @brief   Header file of USART HAL Extended module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_USART_EX_H
#define STM32L4xx_HAL_USART_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup USARTEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup USARTEx_Exported_Constants USARTEx Exported Constants
  * @{
  */

/** @defgroup USARTEx_Word_Length USARTEx Word Length
  * @{
  */
#define USART_WORDLENGTH_7B                  ((uint32_t)USART_CR1_M1)   /*!< 7-bit long USART frame */
#define USART_WORDLENGTH_8B                  0x00000000U                /*!< 8-bit long USART frame */
#define USART_WORDLENGTH_9B                  ((uint32_t)USART_CR1_M0)   /*!< 9-bit long USART frame */
/**
  * @}
  */

#if defined(USART_CR2_SLVEN)
/** @defgroup USARTEx_Slave_Select_management USARTEx Slave Select Management
  * @{
  */
#define USART_NSS_HARD                        0x00000000U          /*!< SPI slave selection depends on NSS input pin              */
#define USART_NSS_SOFT                        USART_CR2_DIS_NSS    /*!< SPI slave is always selected and NSS input pin is ignored */
/**
  * @}
  */


/** @defgroup USARTEx_Slave_Mode USARTEx Synchronous Slave mode enable
  * @brief    USART SLAVE mode
  * @{
  */
#define USART_SLAVEMODE_DISABLE   0x00000000U     /*!< USART SPI Slave Mode Enable  */
#define USART_SLAVEMODE_ENABLE    USART_CR2_SLVEN /*!< USART SPI Slave Mode Disable */
/**
  * @}
  */
#endif /* USART_CR2_SLVEN */

#if defined(USART_CR1_FIFOEN)
/** @defgroup USARTEx_FIFO_mode USARTEx FIFO  mode
  * @brief    USART FIFO  mode
  * @{
  */
#define USART_FIFOMODE_DISABLE        0x00000000U                   /*!< FIFO mode disable */
#define USART_FIFOMODE_ENABLE         USART_CR1_FIFOEN              /*!< FIFO mode enable  */
/**
  * @}
  */

/** @defgroup USARTEx_TXFIFO_threshold_level USARTEx TXFIFO threshold level
  * @brief    USART TXFIFO level
  * @{
  */
#define USART_TXFIFO_THRESHOLD_1_8   0x00000000U                               /*!< TXFIFO reaches 1/8 of its depth */
#define USART_TXFIFO_THRESHOLD_1_4   USART_CR3_TXFTCFG_0                       /*!< TXFIFO reaches 1/4 of its depth */
#define USART_TXFIFO_THRESHOLD_1_2   USART_CR3_TXFTCFG_1                       /*!< TXFIFO reaches 1/2 of its depth */
#define USART_TXFIFO_THRESHOLD_3_4   (USART_CR3_TXFTCFG_0|USART_CR3_TXFTCFG_1) /*!< TXFIFO reaches 3/4 of its depth */
#define USART_TXFIFO_THRESHOLD_7_8   USART_CR3_TXFTCFG_2                       /*!< TXFIFO reaches 7/8 of its depth */
#define USART_TXFIFO_THRESHOLD_8_8   (USART_CR3_TXFTCFG_2|USART_CR3_TXFTCFG_0) /*!< TXFIFO becomes empty            */
/**
  * @}
  */

/** @defgroup USARTEx_RXFIFO_threshold_level USARTEx RXFIFO threshold level
  * @brief    USART RXFIFO level
  * @{
  */
#define USART_RXFIFO_THRESHOLD_1_8   0x00000000U                               /*!< RXFIFO FIFO reaches 1/8 of its depth */
#define USART_RXFIFO_THRESHOLD_1_4   USART_CR3_RXFTCFG_0                       /*!< RXFIFO FIFO reaches 1/4 of its depth */
#define USART_RXFIFO_THRESHOLD_1_2   USART_CR3_RXFTCFG_1                       /*!< RXFIFO FIFO reaches 1/2 of its depth */
#define USART_RXFIFO_THRESHOLD_3_4   (USART_CR3_RXFTCFG_0|USART_CR3_RXFTCFG_1) /*!< RXFIFO FIFO reaches 3/4 of its depth */
#define USART_RXFIFO_THRESHOLD_7_8   USART_CR3_RXFTCFG_2                       /*!< RXFIFO FIFO reaches 7/8 of its depth */
#define USART_RXFIFO_THRESHOLD_8_8   (USART_CR3_RXFTCFG_2|USART_CR3_RXFTCFG_0) /*!< RXFIFO FIFO becomes full             */
/**
  * @}
  */
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup USARTEx_Private_Macros USARTEx Private Macros
  * @{
  */

/** @brief  Report the USART clock source.
  * @param  __HANDLE__ specifies the USART Handle.
  * @param  __CLOCKSOURCE__ output variable.
  * @retval the USART clocking source, written in __CLOCKSOURCE__.
  */
#if defined (STM32L432xx) || defined (STM32L442xx)
#define USART_GETCLOCKSOURCE(__HANDLE__,__CLOCKSOURCE__)       \
  do {                                                         \
    if((__HANDLE__)->Instance == USART1)                       \
    {                                                          \
      switch(__HAL_RCC_GET_USART1_SOURCE())                    \
      {                                                        \
        case RCC_USART1CLKSOURCE_PCLK2:                        \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_PCLK2;         \
          break;                                               \
        case RCC_USART1CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_HSI;           \
          break;                                               \
        case RCC_USART1CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_SYSCLK;        \
          break;                                               \
        case RCC_USART1CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_LSE;           \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_UNDEFINED;     \
          break;                                               \
      }                                                        \
    }                                                          \
    else if((__HANDLE__)->Instance == USART2)                  \
    {                                                          \
      switch(__HAL_RCC_GET_USART2_SOURCE())                    \
      {                                                        \
        case RCC_USART2CLKSOURCE_PCLK1:                        \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_PCLK1;         \
          break;                                               \
        case RCC_USART2CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_HSI;           \
          break;                                               \
        case RCC_USART2CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_SYSCLK;        \
          break;                                               \
        case RCC_USART2CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_LSE;           \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_UNDEFINED;     \
          break;                                               \
      }                                                        \
    }                                                          \
    else                                                       \
    {                                                          \
      (__CLOCKSOURCE__) = USART_CLOCKSOURCE_UNDEFINED;         \
    }                                                          \
  } while(0)
#else
#define USART_GETCLOCKSOURCE(__HANDLE__,__CLOCKSOURCE__)       \
  do {                                                         \
    if((__HANDLE__)->Instance == USART1)                       \
    {                                                          \
      switch(__HAL_RCC_GET_USART1_SOURCE())                    \
      {                                                        \
        case RCC_USART1CLKSOURCE_PCLK2:                        \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_PCLK2;         \
          break;                                               \
        case RCC_USART1CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_HSI;           \
          break;                                               \
        case RCC_USART1CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_SYSCLK;        \
          break;                                               \
        case RCC_USART1CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_LSE;           \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_UNDEFINED;     \
          break;                                               \
      }                                                        \
    }                                                          \
    else if((__HANDLE__)->Instance == USART2)                  \
    {                                                          \
      switch(__HAL_RCC_GET_USART2_SOURCE())                    \
      {                                                        \
        case RCC_USART2CLKSOURCE_PCLK1:                        \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_PCLK1;         \
          break;                                               \
        case RCC_USART2CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_HSI;           \
          break;                                               \
        case RCC_USART2CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_SYSCLK;        \
          break;                                               \
        case RCC_USART2CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_LSE;           \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_UNDEFINED;     \
          break;                                               \
      }                                                        \
    }                                                          \
    else if((__HANDLE__)->Instance == USART3)                  \
    {                                                          \
      switch(__HAL_RCC_GET_USART3_SOURCE())                    \
      {                                                        \
        case RCC_USART3CLKSOURCE_PCLK1:                        \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_PCLK1;         \
          break;                                               \
        case RCC_USART3CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_HSI;           \
          break;                                               \
        case RCC_USART3CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_SYSCLK;        \
          break;                                               \
        case RCC_USART3CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_LSE;           \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = USART_CLOCKSOURCE_UNDEFINED;     \
          break;                                               \
      }                                                        \
    }                                                          \
    else                                                       \
    {                                                          \
      (__CLOCKSOURCE__) = USART_CLOCKSOURCE_UNDEFINED;         \
    }                                                          \
  } while(0)
#endif /* STM32L432xx || STM32L442xx */

/** @brief  Compute the USART mask to apply to retrieve the received data
  *         according to the word length and to the parity bits activation.
  * @note   If PCE = 1, the parity bit is not included in the data extracted
  *         by the reception API().
  *         This masking operation is not carried out in the case of
  *         DMA transfers.
  * @param  __HANDLE__ specifies the USART Handle.
  * @retval None, the mask to apply to USART RDR register is stored in (__HANDLE__)->Mask field.
  */
#define USART_MASK_COMPUTATION(__HANDLE__)                            \
  do {                                                                \
    if ((__HANDLE__)->Init.WordLength == USART_WORDLENGTH_9B)         \
    {                                                                 \
      if ((__HANDLE__)->Init.Parity == USART_PARITY_NONE)             \
      {                                                               \
        (__HANDLE__)->Mask = 0x01FFU;                                 \
      }                                                               \
      else                                                            \
      {                                                               \
        (__HANDLE__)->Mask = 0x00FFU;                                 \
      }                                                               \
    }                                                                 \
    else if ((__HANDLE__)->Init.WordLength == USART_WORDLENGTH_8B)    \
    {                                                                 \
      if ((__HANDLE__)->Init.Parity == USART_PARITY_NONE)             \
      {                                                               \
        (__HANDLE__)->Mask = 0x00FFU;                                 \
      }                                                               \
      else                                                            \
      {                                                               \
        (__HANDLE__)->Mask = 0x007FU;                                 \
      }                                                               \
    }                                                                 \
    else if ((__HANDLE__)->Init.WordLength == USART_WORDLENGTH_7B)    \
    {                                                                 \
      if ((__HANDLE__)->Init.Parity == USART_PARITY_NONE)             \
      {                                                               \
        (__HANDLE__)->Mask = 0x007FU;                                 \
      }                                                               \
      else                                                            \
      {                                                               \
        (__HANDLE__)->Mask = 0x003FU;                                 \
      }                                                               \
    }                                                                 \
    else                                                              \
    {                                                                 \
      (__HANDLE__)->Mask = 0x0000U;                                   \
    }                                                                 \
  } while(0U)


/**
  * @brief Ensure that USART frame length is valid.
  * @param __LENGTH__ USART frame length.
  * @retval SET (__LENGTH__ is valid) or RESET (__LENGTH__ is invalid)
  */
#define IS_USART_WORD_LENGTH(__LENGTH__) (((__LENGTH__) == USART_WORDLENGTH_7B) || \
                                          ((__LENGTH__) == USART_WORDLENGTH_8B) || \
                                          ((__LENGTH__) == USART_WORDLENGTH_9B))

#if defined(USART_CR2_SLVEN)
/**
  * @brief Ensure that USART Negative Slave Select (NSS) pin management is valid.
  * @param __NSS__ USART Negative Slave Select pin management.
  * @retval SET (__NSS__ is valid) or RESET (__NSS__ is invalid)
  */
#define IS_USART_NSS(__NSS__) (((__NSS__) == USART_NSS_HARD) || \
                               ((__NSS__) == USART_NSS_SOFT))

/**
  * @brief Ensure that USART Slave Mode is valid.
  * @param __STATE__ USART Slave Mode.
  * @retval SET (__STATE__ is valid) or RESET (__STATE__ is invalid)
  */
#define IS_USART_SLAVEMODE(__STATE__)   (((__STATE__) == USART_SLAVEMODE_DISABLE ) || \
                                         ((__STATE__) == USART_SLAVEMODE_ENABLE))
#endif /* USART_CR2_SLVEN */

#if defined(USART_CR1_FIFOEN)
/**
  * @brief Ensure that USART FIFO mode is valid.
  * @param __STATE__ USART FIFO mode.
  * @retval SET (__STATE__ is valid) or RESET (__STATE__ is invalid)
  */
#define IS_USART_FIFO_MODE_STATE(__STATE__) (((__STATE__) == USART_FIFOMODE_DISABLE ) || \
                                             ((__STATE__) == USART_FIFOMODE_ENABLE))

/**
  * @brief Ensure that USART TXFIFO threshold level is valid.
  * @param __THRESHOLD__ USART TXFIFO threshold level.
  * @retval SET (__THRESHOLD__ is valid) or RESET (__THRESHOLD__ is invalid)
  */
#define IS_USART_TXFIFO_THRESHOLD(__THRESHOLD__)  (((__THRESHOLD__) == USART_TXFIFO_THRESHOLD_1_8)  || \
                                                   ((__THRESHOLD__) == USART_TXFIFO_THRESHOLD_1_4)  || \
                                                   ((__THRESHOLD__) == USART_TXFIFO_THRESHOLD_1_2)  || \
                                                   ((__THRESHOLD__) == USART_TXFIFO_THRESHOLD_3_4)  || \
                                                   ((__THRESHOLD__) == USART_TXFIFO_THRESHOLD_7_8)  || \
                                                   ((__THRESHOLD__) == USART_TXFIFO_THRESHOLD_8_8))

/**
  * @brief Ensure that USART RXFIFO threshold level is valid.
  * @param __THRESHOLD__ USART RXFIFO threshold level.
  * @retval SET (__THRESHOLD__ is valid) or RESET (__THRESHOLD__ is invalid)
  */
#define IS_USART_RXFIFO_THRESHOLD(__THRESHOLD__)  (((__THRESHOLD__) == USART_RXFIFO_THRESHOLD_1_8)  || \
                                                   ((__THRESHOLD__) == USART_RXFIFO_THRESHOLD_1_4)  || \
                                                   ((__THRESHOLD__) == USART_RXFIFO_THRESHOLD_1_2)  || \
                                                   ((__THRESHOLD__) == USART_RXFIFO_THRESHOLD_3_4)  || \
                                                   ((__THRESHOLD__) == USART_RXFIFO_THRESHOLD_7_8)  || \
                                                   ((__THRESHOLD__) == USART_RXFIFO_THRESHOLD_8_8))
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup USARTEx_Exported_Functions
  * @{
  */

/** @addtogroup USARTEx_Exported_Functions_Group1
  * @{
  */

/* IO operation functions *****************************************************/
#if defined(USART_CR1_FIFOEN)
void HAL_USARTEx_RxFifoFullCallback(USART_HandleTypeDef *husart);
void HAL_USARTEx_TxFifoEmptyCallback(USART_HandleTypeDef *husart);
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/** @addtogroup USARTEx_Exported_Functions_Group2
  * @{
  */

/* Peripheral Control functions ***********************************************/
#if defined(USART_CR2_SLVEN)
HAL_StatusTypeDef HAL_USARTEx_EnableSlaveMode(USART_HandleTypeDef *husart);
HAL_StatusTypeDef HAL_USARTEx_DisableSlaveMode(USART_HandleTypeDef *husart);
HAL_StatusTypeDef HAL_USARTEx_ConfigNSS(USART_HandleTypeDef *husart, uint32_t NSSConfig);
#endif /* USART_CR2_SLVEN */
#if defined(USART_CR1_FIFOEN)
HAL_StatusTypeDef HAL_USARTEx_EnableFifoMode(USART_HandleTypeDef *husart);
HAL_StatusTypeDef HAL_USARTEx_DisableFifoMode(USART_HandleTypeDef *husart);
HAL_StatusTypeDef HAL_USARTEx_SetTxFifoThreshold(USART_HandleTypeDef *husart, uint32_t Threshold);
HAL_StatusTypeDef HAL_USARTEx_SetRxFifoThreshold(USART_HandleTypeDef *husart, uint32_t Threshold);
#endif /* USART_CR1_FIFOEN */

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

#endif /* STM32L4xx_HAL_USART_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
