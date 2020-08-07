/**
  ******************************************************************************
  * @file    stm32l4xx_hal_smartcard_ex.h
  * @author  MCD Application Team
  * @brief   Header file of SMARTCARD HAL Extended module.
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
#ifndef STM32L4xx_HAL_SMARTCARD_EX_H
#define STM32L4xx_HAL_SMARTCARD_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup SMARTCARDEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @addtogroup SMARTCARDEx_Exported_Constants  SMARTCARD Extended Exported Constants
  * @{
  */

/** @defgroup SMARTCARDEx_Transmission_Completion_Indication SMARTCARD Transmission Completion Indication
  * @{
  */
#if defined(USART_TCBGT_SUPPORT)
#define SMARTCARD_TCBGT      SMARTCARD_IT_TCBGT /*!< SMARTCARD transmission complete before guard time */
#endif /* USART_TCBGT_SUPPORT */
#define SMARTCARD_TC         SMARTCARD_IT_TC    /*!< SMARTCARD transmission complete (flag raised when guard time has elapsed) */
/**
  * @}
  */

/** @defgroup SMARTCARDEx_Advanced_Features_Initialization_Type SMARTCARD advanced feature initialization type
  * @{
  */
#define SMARTCARD_ADVFEATURE_NO_INIT                 0x00000000U    /*!< No advanced feature initialization                  */
#define SMARTCARD_ADVFEATURE_TXINVERT_INIT           0x00000001U    /*!< TX pin active level inversion                       */
#define SMARTCARD_ADVFEATURE_RXINVERT_INIT           0x00000002U    /*!< RX pin active level inversion                       */
#define SMARTCARD_ADVFEATURE_DATAINVERT_INIT         0x00000004U    /*!< Binary data inversion                               */
#define SMARTCARD_ADVFEATURE_SWAP_INIT               0x00000008U    /*!< TX/RX pins swap                                     */
#define SMARTCARD_ADVFEATURE_RXOVERRUNDISABLE_INIT   0x00000010U    /*!< RX overrun disable                                  */
#define SMARTCARD_ADVFEATURE_DMADISABLEONERROR_INIT  0x00000020U    /*!< DMA disable on Reception Error                      */
#define SMARTCARD_ADVFEATURE_MSBFIRST_INIT           0x00000080U    /*!< Most significant bit sent/received first            */
#if defined(USART_TCBGT_SUPPORT)
#define SMARTCARD_ADVFEATURE_TXCOMPLETION            0x00000100U    /*!< TX completion indication before of after guard time */
#endif /* USART_TCBGT_SUPPORT */
/**
  * @}
  */

#if defined(USART_CR1_FIFOEN)
/** @defgroup SMARTCARDEx_FIFO_mode SMARTCARDEx FIFO mode
  * @brief    SMARTCARD FIFO mode
  * @{
  */
#define SMARTCARD_FIFOMODE_DISABLE        0x00000000U                   /*!< FIFO mode disable */
#define SMARTCARD_FIFOMODE_ENABLE         USART_CR1_FIFOEN              /*!< FIFO mode enable  */
/**
  * @}
  */

/** @defgroup SMARTCARDEx_TXFIFO_threshold_level SMARTCARDEx TXFIFO threshold level
  * @brief    SMARTCARD TXFIFO level
  * @{
  */
#define SMARTCARD_TXFIFO_THRESHOLD_1_8    0x00000000U                               /*!< TXFIFO reaches 1/8 of its depth */
#define SMARTCARD_TXFIFO_THRESHOLD_1_4   USART_CR3_TXFTCFG_0                        /*!< TXFIFO reaches 1/4 of its depth */
#define SMARTCARD_TXFIFO_THRESHOLD_1_2   USART_CR3_TXFTCFG_1                        /*!< TXFIFO reaches 1/2 of its depth */
#define SMARTCARD_TXFIFO_THRESHOLD_3_4   (USART_CR3_TXFTCFG_0|USART_CR3_TXFTCFG_1)  /*!< TXFIFO reaches 3/4 of its depth */
#define SMARTCARD_TXFIFO_THRESHOLD_7_8   USART_CR3_TXFTCFG_2                        /*!< TXFIFO reaches 7/8 of its depth */
#define SMARTCARD_TXFIFO_THRESHOLD_8_8   (USART_CR3_TXFTCFG_2|USART_CR3_TXFTCFG_0)  /*!< TXFIFO becomes empty            */
/**
  * @}
  */

/** @defgroup SMARTCARDEx_RXFIFO_threshold_level SMARTCARDEx RXFIFO threshold level
  * @brief    SMARTCARD RXFIFO level
  * @{
  */
#define SMARTCARD_RXFIFO_THRESHOLD_1_8   0x00000000U                                /*!< RXFIFO FIFO reaches 1/8 of its depth */
#define SMARTCARD_RXFIFO_THRESHOLD_1_4   USART_CR3_RXFTCFG_0                        /*!< RXFIFO FIFO reaches 1/4 of its depth */
#define SMARTCARD_RXFIFO_THRESHOLD_1_2   USART_CR3_RXFTCFG_1                        /*!< RXFIFO FIFO reaches 1/2 of its depth */
#define SMARTCARD_RXFIFO_THRESHOLD_3_4   (USART_CR3_RXFTCFG_0|USART_CR3_RXFTCFG_1)  /*!< RXFIFO FIFO reaches 3/4 of its depth */
#define SMARTCARD_RXFIFO_THRESHOLD_7_8   USART_CR3_RXFTCFG_2                        /*!< RXFIFO FIFO reaches 7/8 of its depth */
#define SMARTCARD_RXFIFO_THRESHOLD_8_8   (USART_CR3_RXFTCFG_2|USART_CR3_RXFTCFG_0)  /*!< RXFIFO FIFO becomes full             */
/**
  * @}
  */

#endif /* USART_CR1_FIFOEN */
/** @defgroup SMARTCARDEx_Flags SMARTCARD Flags
  *        Elements values convention: 0xXXXX
  *           - 0xXXXX  : Flag mask in the ISR register
  * @{
  */
#if defined(USART_TCBGT_SUPPORT)
#define SMARTCARD_FLAG_TCBGT          USART_ISR_TCBGT         /*!< SMARTCARD transmission complete before guard time completion */
#endif /* USART_TCBGT_SUPPORT */
#define SMARTCARD_FLAG_REACK          USART_ISR_REACK         /*!< SMARTCARD receive enable acknowledge flag  */
#define SMARTCARD_FLAG_TEACK          USART_ISR_TEACK         /*!< SMARTCARD transmit enable acknowledge flag */
#define SMARTCARD_FLAG_BUSY           USART_ISR_BUSY          /*!< SMARTCARD busy flag                        */
#define SMARTCARD_FLAG_EOBF           USART_ISR_EOBF          /*!< SMARTCARD end of block flag                */
#define SMARTCARD_FLAG_RTOF           USART_ISR_RTOF          /*!< SMARTCARD receiver timeout flag            */
#if defined(USART_CR1_FIFOEN)
#define SMARTCARD_FLAG_TXE            USART_ISR_TXE_TXFNF     /*!< SMARTCARD transmit data register empty     */
#define SMARTCARD_FLAG_TXFNF          USART_ISR_TXE_TXFNF     /*!< SMARTCARD TXFIFO not full                  */
#else
#define SMARTCARD_FLAG_TXE            USART_ISR_TXE           /*!< SMARTCARD transmit data register empty     */
#endif /* USART_CR1_FIFOEN */
#define SMARTCARD_FLAG_TC             USART_ISR_TC            /*!< SMARTCARD transmission complete            */
#if defined(USART_CR1_FIFOEN)
#define SMARTCARD_FLAG_RXNE           USART_ISR_RXNE_RXFNE    /*!< SMARTCARD read data register not empty     */
#define SMARTCARD_FLAG_RXFNE          USART_ISR_RXNE_RXFNE    /*!< SMARTCARD RXFIFO not empty                 */
#else
#define SMARTCARD_FLAG_RXNE           USART_ISR_RXNE          /*!< SMARTCARD read data register not empty     */
#endif /* USART_CR1_FIFOEN */
#define SMARTCARD_FLAG_IDLE           USART_ISR_IDLE          /*!< SMARTCARD idle line detection              */
#define SMARTCARD_FLAG_ORE            USART_ISR_ORE           /*!< SMARTCARD overrun error                    */
#define SMARTCARD_FLAG_NE             USART_ISR_NE            /*!< SMARTCARD noise error                      */
#define SMARTCARD_FLAG_FE             USART_ISR_FE            /*!< SMARTCARD frame error                      */
#define SMARTCARD_FLAG_PE             USART_ISR_PE            /*!< SMARTCARD parity error                     */
#if defined(USART_CR1_FIFOEN)
#define SMARTCARD_FLAG_TXFE           USART_ISR_TXFE          /*!< SMARTCARD TXFIFO Empty flag                */
#define SMARTCARD_FLAG_RXFF           USART_ISR_RXFF          /*!< SMARTCARD RXFIFO Full flag                 */
#define SMARTCARD_FLAG_RXFT           USART_ISR_RXFT          /*!< SMARTCARD RXFIFO threshold flag            */
#define SMARTCARD_FLAG_TXFT           USART_ISR_TXFT          /*!< SMARTCARD TXFIFO threshold flag            */
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/** @defgroup SMARTCARDEx_Interrupt_definition SMARTCARD Interrupts Definition
  *        Elements values convention: 000ZZZZZ0XXYYYYYb
  *           - YYYYY  : Interrupt source position in the XX register (5 bits)
  *           - XX  : Interrupt source register (2 bits)
  *                 - 01: CR1 register
  *                 - 10: CR2 register
  *                 - 11: CR3 register
  *           - ZZZZZ  : Flag position in the ISR register(5 bits)
  * @{
  */
#define SMARTCARD_IT_PE                     0x0028U           /*!< SMARTCARD parity error interruption                 */
#define SMARTCARD_IT_TXE                    0x0727U           /*!< SMARTCARD transmit data register empty interruption */
#if defined(USART_CR1_FIFOEN)
#define SMARTCARD_IT_TXFNF                  0x0727U           /*!< SMARTCARD TX FIFO not full interruption             */
#endif /* USART_CR1_FIFOEN */
#define SMARTCARD_IT_TC                     0x0626U           /*!< SMARTCARD transmission complete interruption        */
#define SMARTCARD_IT_RXNE                   0x0525U           /*!< SMARTCARD read data register not empty interruption */
#if defined(USART_CR1_FIFOEN)
#define SMARTCARD_IT_RXFNE                  0x0525U           /*!< SMARTCARD RXFIFO not empty interruption             */
#endif /* USART_CR1_FIFOEN */
#define SMARTCARD_IT_IDLE                   0x0424U           /*!< SMARTCARD idle line detection interruption          */

#define SMARTCARD_IT_ERR                    0x0060U           /*!< SMARTCARD error interruption         */
#define SMARTCARD_IT_ORE                    0x0300U           /*!< SMARTCARD overrun error interruption */
#define SMARTCARD_IT_NE                     0x0200U           /*!< SMARTCARD noise error interruption   */
#define SMARTCARD_IT_FE                     0x0100U           /*!< SMARTCARD frame error interruption   */

#define SMARTCARD_IT_EOB                    0x0C3BU           /*!< SMARTCARD end of block interruption     */
#define SMARTCARD_IT_RTO                    0x0B3AU           /*!< SMARTCARD receiver timeout interruption */
#if defined(USART_TCBGT_SUPPORT)
#define SMARTCARD_IT_TCBGT                  0x1978U           /*!< SMARTCARD transmission complete before guard time completion interruption */
#endif /* USART_TCBGT_SUPPORT */

#if defined(USART_CR1_FIFOEN)
#define SMARTCARD_IT_RXFF                    0x183FU          /*!< SMARTCARD RXFIFO full interruption                  */
#define SMARTCARD_IT_TXFE                    0x173EU          /*!< SMARTCARD TXFIFO empty interruption                 */
#define SMARTCARD_IT_RXFT                    0x1A7CU          /*!< SMARTCARD RXFIFO threshold reached interruption     */
#define SMARTCARD_IT_TXFT                    0x1B77U          /*!< SMARTCARD TXFIFO threshold reached interruption     */
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/** @defgroup SMARTCARDEx_IT_CLEAR_Flags SMARTCARD Interruption Clear Flags
  * @{
  */
#define SMARTCARD_CLEAR_PEF                 USART_ICR_PECF    /*!< SMARTCARD parity error clear flag          */
#define SMARTCARD_CLEAR_FEF                 USART_ICR_FECF    /*!< SMARTCARD framing error clear flag         */
#define SMARTCARD_CLEAR_NEF                 USART_ICR_NECF    /*!< SMARTCARD noise error detected clear flag  */
#define SMARTCARD_CLEAR_OREF                USART_ICR_ORECF   /*!< SMARTCARD overrun error clear flag         */
#define SMARTCARD_CLEAR_IDLEF               USART_ICR_IDLECF  /*!< SMARTCARD idle line detected clear flag    */
#if defined(USART_CR1_FIFOEN)
#define SMARTCARD_CLEAR_TXFECF              USART_ICR_TXFECF  /*!< TXFIFO empty Clear Flag                    */
#endif /* USART_CR1_FIFOEN */
#define SMARTCARD_CLEAR_TCF                 USART_ICR_TCCF    /*!< SMARTCARD transmission complete clear flag */
#if defined(USART_TCBGT_SUPPORT)
#define SMARTCARD_CLEAR_TCBGTF              USART_ICR_TCBGTCF /*!< SMARTCARD transmission complete before guard time completion clear flag */
#endif /* USART_TCBGT_SUPPORT */
#define SMARTCARD_CLEAR_RTOF                USART_ICR_RTOCF   /*!< SMARTCARD receiver time out clear flag     */
#define SMARTCARD_CLEAR_EOBF                USART_ICR_EOBCF   /*!< SMARTCARD end of block clear flag          */
/**
  * @}
  */

/**
  * @}
  */
/* Exported macros -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/** @defgroup SMARTCARDEx_Private_Macros SMARTCARD Extended Private Macros
  * @{
  */

/** @brief  Set the Transmission Completion flag
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @note  If TCBGT (Transmission Complete Before Guard Time) flag is not available or if
  *        AdvancedInit.TxCompletionIndication is not already filled, the latter is forced
  *        to SMARTCARD_TC (transmission completion indication when guard time has elapsed).
  * @retval None
  */
#if defined(USART_TCBGT_SUPPORT)
#define SMARTCARD_TRANSMISSION_COMPLETION_SETTING(__HANDLE__)                                                \
  do {                                                                                                       \
    if (HAL_IS_BIT_CLR((__HANDLE__)->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_TXCOMPLETION))        \
    {                                                                                                        \
      (__HANDLE__)->AdvancedInit.TxCompletionIndication = SMARTCARD_TC;                                      \
    }                                                                                                        \
    else                                                                                                     \
    {                                                                                                        \
      assert_param(IS_SMARTCARD_TRANSMISSION_COMPLETION((__HANDLE__)->AdvancedInit.TxCompletionIndication)); \
    }                                                                                                        \
  } while(0U)
#else
#define SMARTCARD_TRANSMISSION_COMPLETION_SETTING(__HANDLE__)         \
  do {                                                                \
    (__HANDLE__)->AdvancedInit.TxCompletionIndication = SMARTCARD_TC; \
  } while(0U)
#endif /* USART_TCBGT_SUPPORT */

/** @brief  Return the transmission completion flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @note  Based on AdvancedInit.TxCompletionIndication setting, return TC or TCBGT flag.
  *        When TCBGT flag (Transmission Complete Before Guard Time) is not available, TC flag is
  *        reported.
  * @retval Transmission completion flag
  */
#if defined(USART_TCBGT_SUPPORT)
#define SMARTCARD_TRANSMISSION_COMPLETION_FLAG(__HANDLE__)  \
  (((__HANDLE__)->AdvancedInit.TxCompletionIndication == SMARTCARD_TC) ? (SMARTCARD_FLAG_TC) :  (SMARTCARD_FLAG_TCBGT))
#else
#define SMARTCARD_TRANSMISSION_COMPLETION_FLAG(__HANDLE__)    (SMARTCARD_FLAG_TC)
#endif /* USART_TCBGT_SUPPORT */


/** @brief Ensure that SMARTCARD frame transmission completion used flag is valid.
  * @param __TXCOMPLETE__ SMARTCARD frame transmission completion used flag.
  * @retval SET (__TXCOMPLETE__ is valid) or RESET (__TXCOMPLETE__ is invalid)
  */
#if defined(USART_TCBGT_SUPPORT)
#define IS_SMARTCARD_TRANSMISSION_COMPLETION(__TXCOMPLETE__) (((__TXCOMPLETE__) == SMARTCARD_TCBGT) || \
                                                              ((__TXCOMPLETE__) == SMARTCARD_TC))
#else
#define IS_SMARTCARD_TRANSMISSION_COMPLETION(__TXCOMPLETE__) ((__TXCOMPLETE__) == SMARTCARD_TC)
#endif /* USART_TCBGT_SUPPORT */

#if defined(USART_CR1_FIFOEN)
/** @brief Ensure that SMARTCARD FIFO mode is valid.
  * @param __STATE__ SMARTCARD FIFO mode.
  * @retval SET (__STATE__ is valid) or RESET (__STATE__ is invalid)
  */
#define IS_SMARTCARD_FIFOMODE_STATE(__STATE__) (((__STATE__) == SMARTCARD_FIFOMODE_DISABLE ) || \
                                                ((__STATE__) == SMARTCARD_FIFOMODE_ENABLE))

/** @brief Ensure that SMARTCARD TXFIFO threshold level is valid.
  * @param __THRESHOLD__ SMARTCARD TXFIFO threshold level.
  * @retval SET (__THRESHOLD__ is valid) or RESET (__THRESHOLD__ is invalid)
  */
#define IS_SMARTCARD_TXFIFO_THRESHOLD(__THRESHOLD__) (((__THRESHOLD__) == SMARTCARD_TXFIFO_THRESHOLD_1_8) || \
                                                      ((__THRESHOLD__) == SMARTCARD_TXFIFO_THRESHOLD_1_4) || \
                                                      ((__THRESHOLD__) == SMARTCARD_TXFIFO_THRESHOLD_1_2) || \
                                                      ((__THRESHOLD__) == SMARTCARD_TXFIFO_THRESHOLD_3_4) || \
                                                      ((__THRESHOLD__) == SMARTCARD_TXFIFO_THRESHOLD_7_8) || \
                                                      ((__THRESHOLD__) == SMARTCARD_TXFIFO_THRESHOLD_8_8))

/** @brief Ensure that SMARTCARD RXFIFO threshold level is valid.
  * @param __THRESHOLD__ SMARTCARD RXFIFO threshold level.
  * @retval SET (__THRESHOLD__ is valid) or RESET (__THRESHOLD__ is invalid)
  */
#define IS_SMARTCARD_RXFIFO_THRESHOLD(__THRESHOLD__) (((__THRESHOLD__) == SMARTCARD_RXFIFO_THRESHOLD_1_8) || \
                                                      ((__THRESHOLD__) == SMARTCARD_RXFIFO_THRESHOLD_1_4) || \
                                                      ((__THRESHOLD__) == SMARTCARD_RXFIFO_THRESHOLD_1_2) || \
                                                      ((__THRESHOLD__) == SMARTCARD_RXFIFO_THRESHOLD_3_4) || \
                                                      ((__THRESHOLD__) == SMARTCARD_RXFIFO_THRESHOLD_7_8) || \
                                                      ((__THRESHOLD__) == SMARTCARD_RXFIFO_THRESHOLD_8_8))

#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SMARTCARDEx_Exported_Functions
  * @{
  */

/* Initialization and de-initialization functions  ****************************/
/* IO operation methods *******************************************************/

/** @addtogroup SMARTCARDEx_Exported_Functions_Group1
  * @{
  */

/* Peripheral Control functions ***********************************************/
void              HAL_SMARTCARDEx_BlockLength_Config(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t BlockLength);
void              HAL_SMARTCARDEx_TimeOut_Config(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t TimeOutValue);
HAL_StatusTypeDef HAL_SMARTCARDEx_EnableReceiverTimeOut(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARDEx_DisableReceiverTimeOut(SMARTCARD_HandleTypeDef *hsmartcard);

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SMARTCARDEx_Exported_Functions_Group2
  * @{
  */

/* IO operation functions *****************************************************/
#if defined(USART_CR1_FIFOEN)
void HAL_SMARTCARDEx_RxFifoFullCallback(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARDEx_TxFifoEmptyCallback(SMARTCARD_HandleTypeDef *hsmartcard);
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/** @addtogroup SMARTCARDEx_Exported_Functions_Group3
  * @{
  */

/* Peripheral Control functions ***********************************************/
#if defined(USART_CR1_FIFOEN)
HAL_StatusTypeDef HAL_SMARTCARDEx_EnableFifoMode(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARDEx_DisableFifoMode(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARDEx_SetTxFifoThreshold(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t Threshold);
HAL_StatusTypeDef HAL_SMARTCARDEx_SetRxFifoThreshold(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t Threshold);
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_SMARTCARD_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
