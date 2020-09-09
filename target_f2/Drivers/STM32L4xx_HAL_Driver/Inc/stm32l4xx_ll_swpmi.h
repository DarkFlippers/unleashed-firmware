/**
  ******************************************************************************
  * @file    stm32l4xx_ll_swpmi.h
  * @author  MCD Application Team
  * @brief   Header file of SWPMI LL module.
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
#ifndef STM32L4xx_LL_SWPMI_H
#define STM32L4xx_LL_SWPMI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(SWPMI1)

/** @defgroup SWPMI_LL SWPMI
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup SWPMI_LL_Private_Macros SWPMI Private Macros
  * @{
  */
/**
  * @}
  */
#endif /*USE_FULL_LL_DRIVER*/

/* Exported types ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup SWPMI_LL_ES_INIT SWPMI Exported Init structure
  * @{
  */

/**
  * @brief  SWPMI Init structures definition
  */
typedef struct
{
  uint32_t VoltageClass;           /*!< Specifies the SWP Voltage Class.
                                        This parameter can be a value of @ref SWPMI_LL_EC_VOLTAGE_CLASS

                                        This feature can be modified afterwards using unitary function @ref LL_SWPMI_SetVoltageClass. */

  uint32_t BitRatePrescaler;       /*!< Specifies the SWPMI bitrate prescaler.
                                        This parameter must be a number between Min_Data=0 and Max_Data=63U.

                                        The value can be calculated thanks to helper macro @ref __LL_SWPMI_CALC_BITRATE_PRESCALER

                                        This feature can be modified afterwards using unitary function @ref LL_SWPMI_SetBitRatePrescaler. */

  uint32_t TxBufferingMode;        /*!< Specifies the transmission buffering mode.
                                        This parameter can be a value of @ref SWPMI_LL_EC_SW_BUFFER_TX

                                        This feature can be modified afterwards using unitary function @ref LL_SWPMI_SetTransmissionMode. */

  uint32_t RxBufferingMode;        /*!< Specifies the reception buffering mode.
                                        This parameter can be a value of @ref SWPMI_LL_EC_SW_BUFFER_RX

                                        This feature can be modified afterwards using unitary function @ref LL_SWPMI_SetReceptionMode. */
} LL_SWPMI_InitTypeDef;

/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/* Exported constants --------------------------------------------------------*/
/** @defgroup SWPMI_LL_Exported_Constants SWPMI Exported Constants
  * @{
  */

/** @defgroup SWPMI_LL_EC_CLEAR_FLAG Clear Flags Defines
  * @brief    Flags defines which can be used with LL_SWPMI_WriteReg function
  * @{
  */
#define LL_SWPMI_ICR_CRXBFF                SWPMI_ICR_CRXBFF  /*!< Clear receive buffer full flag     */
#define LL_SWPMI_ICR_CTXBEF                SWPMI_ICR_CTXBEF  /*!< Clear transmit buffer empty flag   */
#define LL_SWPMI_ICR_CRXBERF               SWPMI_ICR_CRXBERF /*!< Clear receive CRC error flag       */
#define LL_SWPMI_ICR_CRXOVRF               SWPMI_ICR_CRXOVRF /*!< Clear receive overrun error flag   */
#define LL_SWPMI_ICR_CTXUNRF               SWPMI_ICR_CTXUNRF /*!< Clear transmit underrun error flag */
#define LL_SWPMI_ICR_CTCF                  SWPMI_ICR_CTCF    /*!< Clear transfer complete flag       */
#define LL_SWPMI_ICR_CSRF                  SWPMI_ICR_CSRF    /*!< Clear slave resume flag            */
/**
  * @}
  */

/** @defgroup SWPMI_LL_EC_GET_FLAG Get Flags Defines
  * @brief    Flags defines which can be used with LL_SWPMI_ReadReg function
  * @{
  */
#define LL_SWPMI_ISR_RXBFF                 SWPMI_ISR_RXBFF   /*!< Receive buffer full flag           */
#define LL_SWPMI_ISR_TXBEF                 SWPMI_ISR_TXBEF   /*!< Transmit buffer empty flag         */
#define LL_SWPMI_ISR_RXBERF                SWPMI_ISR_RXBERF  /*!< Receive CRC error flag             */
#define LL_SWPMI_ISR_RXOVRF                SWPMI_ISR_RXOVRF  /*!< Receive overrun error flag         */
#define LL_SWPMI_ISR_TXUNRF                SWPMI_ISR_TXUNRF  /*!< Transmit underrun error flag       */
#define LL_SWPMI_ISR_RXNE                  SWPMI_ISR_RXNE    /*!< Receive data register not empty    */
#define LL_SWPMI_ISR_TXE                   SWPMI_ISR_TXE     /*!< Transmit data register empty       */
#define LL_SWPMI_ISR_TCF                   SWPMI_ISR_TCF     /*!< Transfer complete flag             */
#define LL_SWPMI_ISR_SRF                   SWPMI_ISR_SRF     /*!< Slave resume flag                  */
#define LL_SWPMI_ISR_SUSP                  SWPMI_ISR_SUSP    /*!< SUSPEND flag                       */
#define LL_SWPMI_ISR_DEACTF                SWPMI_ISR_DEACTF  /*!< DEACTIVATED flag                   */
/**
  * @}
  */

/** @defgroup SWPMI_LL_EC_IT IT Defines
  * @brief    IT defines which can be used with LL_SWPMI_ReadReg and  LL_SWPMI_WriteReg functions
  * @{
  */
#define LL_SWPMI_IER_SRIE                  SWPMI_IER_SRIE    /*!< Slave resume interrupt enable            */
#define LL_SWPMI_IER_TCIE                  SWPMI_IER_TCIE    /*!< Transmit complete interrupt enable       */
#define LL_SWPMI_IER_TIE                   SWPMI_IER_TIE     /*!< Transmit interrupt enable                */
#define LL_SWPMI_IER_RIE                   SWPMI_IER_RIE     /*!< Receive interrupt enable                 */
#define LL_SWPMI_IER_TXUNRIE               SWPMI_IER_TXUNRIE /*!< Transmit underrun error interrupt enable */
#define LL_SWPMI_IER_RXOVRIE               SWPMI_IER_RXOVRIE /*!< Receive overrun error interrupt enable   */
#define LL_SWPMI_IER_RXBERIE               SWPMI_IER_RXBERIE /*!< Receive CRC error interrupt enable       */
#define LL_SWPMI_IER_TXBEIE                SWPMI_IER_TXBEIE  /*!< Transmit buffer empty interrupt enable   */
#define LL_SWPMI_IER_RXBFIE                SWPMI_IER_RXBFIE  /*!< Receive buffer full interrupt enable     */
/**
  * @}
  */

/** @defgroup SWPMI_LL_EC_SW_BUFFER_RX SW BUFFER RX
  * @{
  */
#define LL_SWPMI_SW_BUFFER_RX_SINGLE ((uint32_t)0x00000000)  /*!< Single software buffer mode for reception */
#define LL_SWPMI_SW_BUFFER_RX_MULTI  SWPMI_CR_RXMODE         /*!< Multi software buffermode for reception   */
/**
  * @}
  */

/** @defgroup SWPMI_LL_EC_SW_BUFFER_TX SW BUFFER TX
  * @{
  */
#define LL_SWPMI_SW_BUFFER_TX_SINGLE ((uint32_t)0x00000000)  /*!< Single software buffer mode for transmission */
#define LL_SWPMI_SW_BUFFER_TX_MULTI  SWPMI_CR_TXMODE         /*!< Multi software buffermode for transmission   */
/**
  * @}
  */

/** @defgroup SWPMI_LL_EC_VOLTAGE_CLASS VOLTAGE CLASS
  * @{
  */
#define LL_SWPMI_VOLTAGE_CLASS_C     ((uint32_t)0x00000000)  /*!< SWPMI_IO uses directly VDD voltage to operate in class C          */
#define LL_SWPMI_VOLTAGE_CLASS_B     SWPMI_OR_CLASS          /*!< SWPMI_IO uses an internal voltage regulator to operate in class B */
/**
  * @}
  */

/** @defgroup SWPMI_LL_EC_DMA_REG_DATA DMA register data
  * @{
  */
#define LL_SWPMI_DMA_REG_DATA_TRANSMIT     (uint32_t)0       /*!< Get address of data register used for transmission */
#define LL_SWPMI_DMA_REG_DATA_RECEIVE      (uint32_t)1       /*!< Get address of data register used for reception    */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup SWPMI_LL_Exported_Macros SWPMI Exported Macros
  * @{
  */

/** @defgroup SWPMI_LL_EM_WRITE_READ Common Write and read registers Macros
  * @{
  */

/**
  * @brief  Write a value in SWPMI register
  * @param  __INSTANCE__ SWPMI Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_SWPMI_WriteReg(__INSTANCE__, __REG__, __VALUE__) WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
  * @brief  Read a value in SWPMI register
  * @param  __INSTANCE__ SWPMI Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_SWPMI_ReadReg(__INSTANCE__, __REG__) READ_REG(__INSTANCE__->__REG__)
/**
  * @}
  */

/** @defgroup SWPMI_LL_EM_BitRate Bit rate calculation helper Macros
  * @{
  */

/**
  * @brief  Helper macro to calculate bit rate value to set in BRR register (@ref LL_SWPMI_SetBitRatePrescaler function)
  * @note ex: @ref __LL_SWPMI_CALC_BITRATE_PRESCALER(2000000, 80000000);
  * @param  __FSWP__ Within the following range: from 100 Kbit/s up to 2Mbit/s (in bit/s)
  * @param  __FSWPCLK__ PCLK or HSI frequency (in Hz)
  * @retval Bitrate prescaler (BRR register)
  */
#define __LL_SWPMI_CALC_BITRATE_PRESCALER(__FSWP__, __FSWPCLK__)   ((uint32_t)(((__FSWPCLK__) / ((__FSWP__) * 4)) - 1))

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup SWPMI_LL_Exported_Functions SWPMI Exported Functions
  * @{
  */

/** @defgroup SWPMI_LL_EF_Configuration Configuration
  * @{
  */

/**
  * @brief  Set Reception buffering mode
  * @note   If Multi software buffer mode is chosen, RXDMA bits must also be set.
  * @rmtoll CR           RXMODE        LL_SWPMI_SetReceptionMode
  * @param  SWPMIx SWPMI Instance
  * @param  RxBufferingMode This parameter can be one of the following values:
  *         @arg @ref LL_SWPMI_SW_BUFFER_RX_SINGLE
  *         @arg @ref LL_SWPMI_SW_BUFFER_RX_MULTI
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_SetReceptionMode(SWPMI_TypeDef *SWPMIx, uint32_t RxBufferingMode)
{
  MODIFY_REG(SWPMIx->CR, SWPMI_CR_RXMODE, RxBufferingMode);
}

/**
  * @brief  Get Reception buffering mode
  * @rmtoll CR           RXMODE        LL_SWPMI_GetReceptionMode
  * @param  SWPMIx SWPMI Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_SWPMI_SW_BUFFER_RX_SINGLE
  *         @arg @ref LL_SWPMI_SW_BUFFER_RX_MULTI
  */
__STATIC_INLINE uint32_t LL_SWPMI_GetReceptionMode(SWPMI_TypeDef *SWPMIx)
{
  return (uint32_t)(READ_BIT(SWPMIx->CR, SWPMI_CR_RXMODE));
}

/**
  * @brief  Set Transmission buffering mode
  * @note   If Multi software buffer mode is chosen, TXDMA bits must also be set.
  * @rmtoll CR           TXMODE        LL_SWPMI_SetTransmissionMode
  * @param  SWPMIx SWPMI Instance
  * @param  TxBufferingMode This parameter can be one of the following values:
  *         @arg @ref LL_SWPMI_SW_BUFFER_TX_SINGLE
  *         @arg @ref LL_SWPMI_SW_BUFFER_TX_MULTI
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_SetTransmissionMode(SWPMI_TypeDef *SWPMIx, uint32_t TxBufferingMode)
{
  MODIFY_REG(SWPMIx->CR, SWPMI_CR_TXMODE, TxBufferingMode);
}

/**
  * @brief  Get Transmission buffering mode
  * @rmtoll CR           TXMODE        LL_SWPMI_GetTransmissionMode
  * @param  SWPMIx SWPMI Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_SWPMI_SW_BUFFER_TX_SINGLE
  *         @arg @ref LL_SWPMI_SW_BUFFER_TX_MULTI
  */
__STATIC_INLINE uint32_t LL_SWPMI_GetTransmissionMode(SWPMI_TypeDef *SWPMIx)
{
  return (uint32_t)(READ_BIT(SWPMIx->CR, SWPMI_CR_TXMODE));
}

/**
  * @brief  Enable loopback mode
  * @rmtoll CR           LPBK          LL_SWPMI_EnableLoopback
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableLoopback(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->CR, SWPMI_CR_LPBK);
}

/**
  * @brief  Disable loopback mode
  * @rmtoll CR           LPBK          LL_SWPMI_DisableLoopback
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableLoopback(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->CR, SWPMI_CR_LPBK);
}

/**
  * @brief  Activate Single wire protocol bus (SUSPENDED or ACTIVATED state)
  * @note   SWP bus stays in the ACTIVATED state as long as there is a communication
  *         with the slave, either in transmission or in reception. The SWP bus switches back
  *         to the SUSPENDED state as soon as there is no more transmission or reception
  *         activity, after 7 idle bits.
  * @rmtoll CR           SWPACT        LL_SWPMI_Activate
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_Activate(SWPMI_TypeDef *SWPMIx)
{
  /* In order to activate SWP again, the software must clear DEACT bit*/
  CLEAR_BIT(SWPMIx->CR, SWPMI_CR_DEACT);

  /* Set SWACT bit */
  SET_BIT(SWPMIx->CR, SWPMI_CR_SWPACT);
}

/**
  * @brief  Check if Single wire protocol bus is in ACTIVATED state.
  * @rmtoll CR           SWPACT        LL_SWPMI_Activate
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActivated(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->CR, SWPMI_CR_SWPACT) == (SWPMI_CR_SWPACT)) ? 1UL : 0UL);
}

/**
  * @brief  Deactivate immediately Single wire protocol bus (immediate transition to
  *         DEACTIVATED state)
  * @rmtoll CR           SWPACT        LL_SWPMI_Deactivate
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_Deactivate(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->CR, SWPMI_CR_SWPACT);
}

/**
  * @brief  Request a deactivation of Single wire protocol bus (request to go in DEACTIVATED
  *         state if no resume from slave)
  * @rmtoll CR           DEACT         LL_SWPMI_RequestDeactivation
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_RequestDeactivation(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->CR, SWPMI_CR_DEACT);
}

/**
  * @brief  Set Bitrate prescaler SWPMI_freq = SWPMI_clk / (((BitRate) + 1)  * 4)
  * @rmtoll BRR          BR            LL_SWPMI_SetBitRatePrescaler
  * @param  SWPMIx SWPMI Instance
  * @param  BitRatePrescaler A number between Min_Data=0 and Max_Data=63U
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_SetBitRatePrescaler(SWPMI_TypeDef *SWPMIx, uint32_t BitRatePrescaler)
{
  WRITE_REG(SWPMIx->BRR, BitRatePrescaler);
}

/**
  * @brief  Get Bitrate prescaler
  * @rmtoll BRR          BR            LL_SWPMI_GetBitRatePrescaler
  * @param  SWPMIx SWPMI Instance
  * @retval A number between Min_Data=0 and Max_Data=63U
  */
__STATIC_INLINE uint32_t LL_SWPMI_GetBitRatePrescaler(SWPMI_TypeDef *SWPMIx)
{
  return (uint32_t)(READ_BIT(SWPMIx->BRR, SWPMI_BRR_BR));
}

/**
  * @brief  Set SWP Voltage Class
  * @rmtoll OR           CLASS         LL_SWPMI_SetVoltageClass
  * @param  SWPMIx SWPMI Instance
  * @param  VoltageClass This parameter can be one of the following values:
  *         @arg @ref LL_SWPMI_VOLTAGE_CLASS_C
  *         @arg @ref LL_SWPMI_VOLTAGE_CLASS_B
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_SetVoltageClass(SWPMI_TypeDef *SWPMIx, uint32_t VoltageClass)
{
  MODIFY_REG(SWPMIx->OR, SWPMI_OR_CLASS, VoltageClass);
}

/**
  * @brief  Get SWP Voltage Class
  * @rmtoll OR           CLASS         LL_SWPMI_GetVoltageClass
  * @param  SWPMIx SWPMI Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_SWPMI_VOLTAGE_CLASS_C
  *         @arg @ref LL_SWPMI_VOLTAGE_CLASS_B
  */
__STATIC_INLINE uint32_t LL_SWPMI_GetVoltageClass(SWPMI_TypeDef *SWPMIx)
{
  return (uint32_t)(READ_BIT(SWPMIx->OR, SWPMI_OR_CLASS));
}

/**
  * @}
  */

/** @defgroup SWPMI_LL_EF_FLAG_Management FLAG_Management
  * @{
  */

/**
  * @brief  Check if the last word of the frame under reception has arrived in SWPMI_RDR.
  * @rmtoll ISR          RXBFF         LL_SWPMI_IsActiveFlag_RXBF
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_RXBF(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_RXBFF) == (SWPMI_ISR_RXBFF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Frame transmission buffer has been emptied
  * @rmtoll ISR          TXBEF         LL_SWPMI_IsActiveFlag_TXBE
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_TXBE(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_TXBEF) == (SWPMI_ISR_TXBEF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if CRC error in reception has been detected
  * @rmtoll ISR          RXBERF        LL_SWPMI_IsActiveFlag_RXBER
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_RXBER(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_RXBERF) == (SWPMI_ISR_RXBERF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Overrun in reception has been detected
  * @rmtoll ISR          RXOVRF        LL_SWPMI_IsActiveFlag_RXOVR
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_RXOVR(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_RXOVRF) == (SWPMI_ISR_RXOVRF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if underrun error in transmission has been detected
  * @rmtoll ISR          TXUNRF        LL_SWPMI_IsActiveFlag_TXUNR
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_TXUNR(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_TXUNRF) == (SWPMI_ISR_TXUNRF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Receive data register not empty (it means that Received data is ready
  *         to be read in the SWPMI_RDR register)
  * @rmtoll ISR          RXNE          LL_SWPMI_IsActiveFlag_RXNE
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_RXNE(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_RXNE) == (SWPMI_ISR_RXNE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Transmit data register is empty (it means that Data written in transmit
  *         data register SWPMI_TDR has been transmitted and SWPMI_TDR can be written to again)
  * @rmtoll ISR          TXE           LL_SWPMI_IsActiveFlag_TXE
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_TXE(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_TXE) == (SWPMI_ISR_TXE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Both transmission and reception are completed and SWP is switched to
  *         the SUSPENDED state
  * @rmtoll ISR          TCF           LL_SWPMI_IsActiveFlag_TC
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_TC(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_TCF) == (SWPMI_ISR_TCF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if a Resume by slave state has been detected during the SWP bus SUSPENDED
  *         state
  * @rmtoll ISR          SRF           LL_SWPMI_IsActiveFlag_SR
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_SR(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_SRF) == (SWPMI_ISR_SRF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if SWP bus is in SUSPENDED or DEACTIVATED state
  * @rmtoll ISR          SUSP          LL_SWPMI_IsActiveFlag_SUSP
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_SUSP(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_SUSP) == (SWPMI_ISR_SUSP)) ? 1UL : 0UL);
}

/**
  * @brief  Check if SWP bus is in DEACTIVATED state
  * @rmtoll ISR          DEACTF        LL_SWPMI_IsActiveFlag_DEACT
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsActiveFlag_DEACT(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->ISR, SWPMI_ISR_DEACTF) == (SWPMI_ISR_DEACTF)) ? 1UL : 0UL);
}

/**
  * @brief  Clear receive buffer full flag
  * @rmtoll ICR          CRXBFF        LL_SWPMI_ClearFlag_RXBF
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_ClearFlag_RXBF(SWPMI_TypeDef *SWPMIx)
{
  WRITE_REG(SWPMIx->ICR, SWPMI_ICR_CRXBFF);
}

/**
  * @brief  Clear transmit buffer empty flag
  * @rmtoll ICR          CTXBEF        LL_SWPMI_ClearFlag_TXBE
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_ClearFlag_TXBE(SWPMI_TypeDef *SWPMIx)
{
  WRITE_REG(SWPMIx->ICR, SWPMI_ICR_CTXBEF);
}

/**
  * @brief  Clear receive CRC error flag
  * @rmtoll ICR          CRXBERF       LL_SWPMI_ClearFlag_RXBER
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_ClearFlag_RXBER(SWPMI_TypeDef *SWPMIx)
{
  WRITE_REG(SWPMIx->ICR, SWPMI_ICR_CRXBERF);
}

/**
  * @brief  Clear receive overrun error flag
  * @rmtoll ICR          CRXOVRF       LL_SWPMI_ClearFlag_RXOVR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_ClearFlag_RXOVR(SWPMI_TypeDef *SWPMIx)
{
  WRITE_REG(SWPMIx->ICR, SWPMI_ICR_CRXOVRF);
}

/**
  * @brief  Clear transmit underrun error flag
  * @rmtoll ICR          CTXUNRF       LL_SWPMI_ClearFlag_TXUNR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_ClearFlag_TXUNR(SWPMI_TypeDef *SWPMIx)
{
  WRITE_REG(SWPMIx->ICR, SWPMI_ICR_CTXUNRF);
}

/**
  * @brief  Clear transfer complete flag
  * @rmtoll ICR          CTCF          LL_SWPMI_ClearFlag_TC
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_ClearFlag_TC(SWPMI_TypeDef *SWPMIx)
{
  WRITE_REG(SWPMIx->ICR, SWPMI_ICR_CTCF);
}

/**
  * @brief  Clear slave resume flag
  * @rmtoll ICR          CSRF          LL_SWPMI_ClearFlag_SR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_ClearFlag_SR(SWPMI_TypeDef *SWPMIx)
{
  WRITE_REG(SWPMIx->ICR, SWPMI_ICR_CSRF);
}

/**
  * @}
  */

/** @defgroup SWPMI_LL_EF_IT_Management IT_Management
  * @{
  */

/**
  * @brief  Enable Slave resume interrupt
  * @rmtoll IER          SRIE          LL_SWPMI_EnableIT_SR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_SR(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_SRIE);
}

/**
  * @brief  Enable Transmit complete interrupt
  * @rmtoll IER          TCIE          LL_SWPMI_EnableIT_TC
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_TC(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_TCIE);
}

/**
  * @brief  Enable Transmit interrupt
  * @rmtoll IER          TIE           LL_SWPMI_EnableIT_TX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_TX(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_TIE);
}

/**
  * @brief  Enable Receive interrupt
  * @rmtoll IER          RIE           LL_SWPMI_EnableIT_RX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_RX(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_RIE);
}

/**
  * @brief  Enable Transmit underrun error interrupt
  * @rmtoll IER          TXUNRIE       LL_SWPMI_EnableIT_TXUNR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_TXUNR(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_TXUNRIE);
}

/**
  * @brief  Enable Receive overrun error interrupt
  * @rmtoll IER          RXOVRIE       LL_SWPMI_EnableIT_RXOVR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_RXOVR(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_RXOVRIE);
}

/**
  * @brief  Enable Receive CRC error interrupt
  * @rmtoll IER          RXBERIE       LL_SWPMI_EnableIT_RXBER
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_RXBER(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_RXBERIE);
}

/**
  * @brief  Enable Transmit buffer empty interrupt
  * @rmtoll IER          TXBEIE        LL_SWPMI_EnableIT_TXBE
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_TXBE(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_TXBEIE);
}

/**
  * @brief  Enable Receive buffer full interrupt
  * @rmtoll IER          RXBFIE        LL_SWPMI_EnableIT_RXBF
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableIT_RXBF(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->IER, SWPMI_IER_RXBFIE);
}

/**
  * @brief  Disable Slave resume interrupt
  * @rmtoll IER          SRIE          LL_SWPMI_DisableIT_SR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_SR(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_SRIE);
}

/**
  * @brief  Disable Transmit complete interrupt
  * @rmtoll IER          TCIE          LL_SWPMI_DisableIT_TC
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_TC(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_TCIE);
}

/**
  * @brief  Disable Transmit interrupt
  * @rmtoll IER          TIE           LL_SWPMI_DisableIT_TX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_TX(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_TIE);
}

/**
  * @brief  Disable Receive interrupt
  * @rmtoll IER          RIE           LL_SWPMI_DisableIT_RX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_RX(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_RIE);
}

/**
  * @brief  Disable Transmit underrun error interrupt
  * @rmtoll IER          TXUNRIE       LL_SWPMI_DisableIT_TXUNR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_TXUNR(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_TXUNRIE);
}

/**
  * @brief  Disable Receive overrun error interrupt
  * @rmtoll IER          RXOVRIE       LL_SWPMI_DisableIT_RXOVR
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_RXOVR(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_RXOVRIE);
}

/**
  * @brief  Disable Receive CRC error interrupt
  * @rmtoll IER          RXBERIE       LL_SWPMI_DisableIT_RXBER
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_RXBER(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_RXBERIE);
}

/**
  * @brief  Disable Transmit buffer empty interrupt
  * @rmtoll IER          TXBEIE        LL_SWPMI_DisableIT_TXBE
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_TXBE(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_TXBEIE);
}

/**
  * @brief  Disable Receive buffer full interrupt
  * @rmtoll IER          RXBFIE        LL_SWPMI_DisableIT_RXBF
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableIT_RXBF(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->IER, SWPMI_IER_RXBFIE);
}

/**
  * @brief  Check if Slave resume interrupt is enabled
  * @rmtoll IER          SRIE          LL_SWPMI_IsEnabledIT_SR
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_SR(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_SRIE) == (SWPMI_IER_SRIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Transmit complete interrupt is enabled
  * @rmtoll IER          TCIE          LL_SWPMI_IsEnabledIT_TC
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_TC(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_TCIE) == (SWPMI_IER_TCIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Transmit interrupt is enabled
  * @rmtoll IER          TIE           LL_SWPMI_IsEnabledIT_TX
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_TX(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_TIE) == (SWPMI_IER_TIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Receive interrupt is enabled
  * @rmtoll IER          RIE           LL_SWPMI_IsEnabledIT_RX
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_RX(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_RIE) == (SWPMI_IER_RIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Transmit underrun error interrupt is enabled
  * @rmtoll IER          TXUNRIE       LL_SWPMI_IsEnabledIT_TXUNR
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_TXUNR(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_TXUNRIE) == (SWPMI_IER_TXUNRIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Receive overrun error interrupt is enabled
  * @rmtoll IER          RXOVRIE       LL_SWPMI_IsEnabledIT_RXOVR
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_RXOVR(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_RXOVRIE) == (SWPMI_IER_RXOVRIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Receive CRC error interrupt is enabled
  * @rmtoll IER          RXBERIE       LL_SWPMI_IsEnabledIT_RXBER
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_RXBER(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_RXBERIE) == (SWPMI_IER_RXBERIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Transmit buffer empty interrupt is enabled
  * @rmtoll IER          TXBEIE        LL_SWPMI_IsEnabledIT_TXBE
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_TXBE(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_TXBEIE) == (SWPMI_IER_TXBEIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if Receive buffer full interrupt is enabled
  * @rmtoll IER          RXBFIE        LL_SWPMI_IsEnabledIT_RXBF
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledIT_RXBF(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->IER, SWPMI_IER_RXBFIE) == (SWPMI_IER_RXBFIE)) ? 1UL : 0UL);
}

/**
  * @}
  */

/** @defgroup SWPMI_LL_EF_DMA_Management DMA_Management
  * @{
  */

/**
  * @brief  Enable DMA mode for reception
  * @rmtoll CR           RXDMA         LL_SWPMI_EnableDMAReq_RX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableDMAReq_RX(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->CR, SWPMI_CR_RXDMA);
}

/**
  * @brief  Disable DMA mode for reception
  * @rmtoll CR           RXDMA         LL_SWPMI_DisableDMAReq_RX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableDMAReq_RX(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->CR, SWPMI_CR_RXDMA);
}

/**
  * @brief  Check if DMA mode for reception is enabled
  * @rmtoll CR           RXDMA         LL_SWPMI_IsEnabledDMAReq_RX
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledDMAReq_RX(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->CR, SWPMI_CR_RXDMA) == (SWPMI_CR_RXDMA)) ? 1UL : 0UL);
}

/**
  * @brief  Enable DMA mode for transmission
  * @rmtoll CR           TXDMA         LL_SWPMI_EnableDMAReq_TX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableDMAReq_TX(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->CR, SWPMI_CR_TXDMA);
}

/**
  * @brief  Disable DMA mode for transmission
  * @rmtoll CR           TXDMA         LL_SWPMI_DisableDMAReq_TX
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableDMAReq_TX(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->CR, SWPMI_CR_TXDMA);
}

/**
  * @brief  Check if DMA mode for transmission is enabled
  * @rmtoll CR           TXDMA         LL_SWPMI_IsEnabledDMAReq_TX
  * @param  SWPMIx SWPMI Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SWPMI_IsEnabledDMAReq_TX(SWPMI_TypeDef *SWPMIx)
{
  return ((READ_BIT(SWPMIx->CR, SWPMI_CR_TXDMA) == (SWPMI_CR_TXDMA)) ? 1UL : 0UL);
}

/**
  * @brief  Get the data register address used for DMA transfer
  * @rmtoll TDR          TD            LL_SWPMI_DMA_GetRegAddr\n
  *         RDR          RD            LL_SWPMI_DMA_GetRegAddr
  * @param  SWPMIx SWPMI Instance
  * @param  Direction This parameter can be one of the following values:
  *         @arg @ref LL_SWPMI_DMA_REG_DATA_TRANSMIT
  *         @arg @ref LL_SWPMI_DMA_REG_DATA_RECEIVE
  * @retval Address of data register
  */
__STATIC_INLINE uint32_t LL_SWPMI_DMA_GetRegAddr(SWPMI_TypeDef *SWPMIx, uint32_t Direction)
{
  uint32_t data_reg_addr;

  if (Direction == LL_SWPMI_DMA_REG_DATA_TRANSMIT)
  {
    /* return address of TDR register */
    data_reg_addr = (uint32_t)&(SWPMIx->TDR);
  }
  else
  {
    /* return address of RDR register */
    data_reg_addr = (uint32_t)&(SWPMIx->RDR);
  }

  return data_reg_addr;
}

/**
  * @}
  */

/** @defgroup SWPMI_LL_EF_Data_Management Data_Management
  * @{
  */

/**
  * @brief  Retrieve number of data bytes present in payload of received frame
  * @rmtoll RFL          RFL           LL_SWPMI_GetReceiveFrameLength
  * @param  SWPMIx SWPMI Instance
  * @retval Value between Min_Data=0x00 and Max_Data=0x1F
  */
__STATIC_INLINE uint32_t LL_SWPMI_GetReceiveFrameLength(SWPMI_TypeDef *SWPMIx)
{
  return (uint32_t)(READ_BIT(SWPMIx->RFL, SWPMI_RFL_RFL));
}

/**
  * @brief  Transmit Data Register
  * @rmtoll TDR          TD            LL_SWPMI_TransmitData32
  * @param  SWPMIx SWPMI Instance
  * @param  TxData Value between Min_Data=0x00000000 and Max_Data=0xFFFFFFFF
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_TransmitData32(SWPMI_TypeDef *SWPMIx, uint32_t TxData)
{
  WRITE_REG(SWPMIx->TDR, TxData);
}

/**
  * @brief  Receive Data Register
  * @rmtoll RDR          RD            LL_SWPMI_ReceiveData32
  * @param  SWPMIx SWPMI Instance
  * @retval Value between Min_Data=0x00000000 and Max_Data=0xFFFFFFFF
  */
__STATIC_INLINE uint32_t LL_SWPMI_ReceiveData32(SWPMI_TypeDef *SWPMIx)
{
  return (uint32_t)(READ_BIT(SWPMIx->RDR, SWPMI_RDR_RD));
}

/**
  * @brief  Enable SWP Transceiver Bypass
  * @note   The external interface for SWPMI is SWPMI_IO
  *         (SWPMI_RX, SWPMI_TX and SWPMI_SUSPEND signals are not available on GPIOs)
  * @rmtoll OR           TBYP          LL_SWPMI_EnableTXBypass
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_EnableTXBypass(SWPMI_TypeDef *SWPMIx)
{
  CLEAR_BIT(SWPMIx->OR, SWPMI_OR_TBYP);
}

/**
  * @brief  Disable SWP Transceiver Bypass
  * @note   SWPMI_RX, SWPMI_TX and SWPMI_SUSPEND signals are available as alternate
  *         function on GPIOs. This configuration is selected to connect an external transceiver
  * @rmtoll OR           TBYP          LL_SWPMI_DisableTXBypass
  * @param  SWPMIx SWPMI Instance
  * @retval None
  */
__STATIC_INLINE void LL_SWPMI_DisableTXBypass(SWPMI_TypeDef *SWPMIx)
{
  SET_BIT(SWPMIx->OR, SWPMI_OR_TBYP);
}

/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup SWPMI_LL_EF_Init Initialization and de-initialization functions
  * @{
  */

ErrorStatus LL_SWPMI_DeInit(SWPMI_TypeDef *SWPMIx);
ErrorStatus LL_SWPMI_Init(SWPMI_TypeDef *SWPMIx, LL_SWPMI_InitTypeDef *SWPMI_InitStruct);
void        LL_SWPMI_StructInit(LL_SWPMI_InitTypeDef *SWPMI_InitStruct);

/**
  * @}
  */
#endif /*USE_FULL_LL_DRIVER*/

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

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_LL_SWPMI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
