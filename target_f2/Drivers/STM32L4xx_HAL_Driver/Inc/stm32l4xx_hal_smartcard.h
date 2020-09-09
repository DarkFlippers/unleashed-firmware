/**
  ******************************************************************************
  * @file    stm32l4xx_hal_smartcard.h
  * @author  MCD Application Team
  * @brief   Header file of SMARTCARD HAL module.
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
#ifndef STM32L4xx_HAL_SMARTCARD_H
#define STM32L4xx_HAL_SMARTCARD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup SMARTCARD
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup SMARTCARD_Exported_Types SMARTCARD Exported Types
  * @{
  */

/**
  * @brief SMARTCARD Init Structure definition
  */
typedef struct
{
  uint32_t BaudRate;                  /*!< Configures the SmartCard communication baud rate.
                                           The baud rate register is computed using the following formula:
                                              Baud Rate Register = ((usart_ker_ckpres) / ((hsmartcard->Init.BaudRate)))
                                           where usart_ker_ckpres is the USART input clock divided by a prescaler */

  uint32_t WordLength;                /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter @ref SMARTCARD_Word_Length can only be set to 9 (8 data + 1 parity bits). */

  uint32_t StopBits;                  /*!< Specifies the number of stop bits.
                                           This parameter can be a value of @ref SMARTCARD_Stop_Bits. */

  uint16_t Parity;                    /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref SMARTCARD_Parity
                                           @note The parity is enabled by default (PCE is forced to 1).
                                                 Since the WordLength is forced to 8 bits + parity, M is
                                                 forced to 1 and the parity bit is the 9th bit. */

  uint16_t Mode;                      /*!< Specifies whether the Receive or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref SMARTCARD_Mode */

  uint16_t CLKPolarity;               /*!< Specifies the steady state of the serial clock.
                                           This parameter can be a value of @ref SMARTCARD_Clock_Polarity */

  uint16_t CLKPhase;                  /*!< Specifies the clock transition on which the bit capture is made.
                                           This parameter can be a value of @ref SMARTCARD_Clock_Phase */

  uint16_t CLKLastBit;                /*!< Specifies whether the clock pulse corresponding to the last transmitted
                                           data bit (MSB) has to be output on the SCLK pin in synchronous mode.
                                           This parameter can be a value of @ref SMARTCARD_Last_Bit */

  uint16_t OneBitSampling;            /*!< Specifies whether a single sample or three samples' majority vote is selected.
                                           Selecting the single sample method increases the receiver tolerance to clock
                                           deviations. This parameter can be a value of @ref SMARTCARD_OneBit_Sampling. */

  uint8_t  Prescaler;                 /*!< Specifies the SmartCard Prescaler.
                                           This parameter can be any value from 0x01 to 0x1F. Prescaler value is multiplied
                                           by 2 to give the division factor of the source clock frequency */

  uint8_t  GuardTime;                 /*!< Specifies the SmartCard Guard Time applied after stop bits. */

  uint16_t NACKEnable;                /*!< Specifies whether the SmartCard NACK transmission is enabled
                                           in case of parity error.
                                           This parameter can be a value of @ref SMARTCARD_NACK_Enable */

  uint32_t TimeOutEnable;             /*!< Specifies whether the receiver timeout is enabled.
                                            This parameter can be a value of @ref SMARTCARD_Timeout_Enable*/

  uint32_t TimeOutValue;              /*!< Specifies the receiver time out value in number of baud blocks:
                                           it is used to implement the Character Wait Time (CWT) and
                                           Block Wait Time (BWT). It is coded over 24 bits. */

  uint8_t BlockLength;                /*!< Specifies the SmartCard Block Length in T=1 Reception mode.
                                           This parameter can be any value from 0x0 to 0xFF */

  uint8_t AutoRetryCount;             /*!< Specifies the SmartCard auto-retry count (number of retries in
                                            receive and transmit mode). When set to 0, retransmission is
                                            disabled. Otherwise, its maximum value is 7 (before signalling
                                            an error) */

#if defined(USART_PRESC_PRESCALER)
  uint32_t ClockPrescaler;            /*!< Specifies the prescaler value used to divide the USART clock source.
                                           This parameter can be a value of @ref SMARTCARD_ClockPrescaler. */

#endif /* USART_PRESC_PRESCALER */
} SMARTCARD_InitTypeDef;

/**
  * @brief  SMARTCARD advanced features initalization structure definition
  */
typedef struct
{
  uint32_t AdvFeatureInit;            /*!< Specifies which advanced SMARTCARD features is initialized. Several
                                           advanced features may be initialized at the same time. This parameter
                                           can be a value of @ref SMARTCARDEx_Advanced_Features_Initialization_Type */

  uint32_t TxPinLevelInvert;          /*!< Specifies whether the TX pin active level is inverted.
                                           This parameter can be a value of @ref SMARTCARD_Tx_Inv  */

  uint32_t RxPinLevelInvert;          /*!< Specifies whether the RX pin active level is inverted.
                                           This parameter can be a value of @ref SMARTCARD_Rx_Inv  */

  uint32_t DataInvert;                /*!< Specifies whether data are inverted (positive/direct logic
                                           vs negative/inverted logic).
                                           This parameter can be a value of @ref SMARTCARD_Data_Inv */

  uint32_t Swap;                      /*!< Specifies whether TX and RX pins are swapped.
                                           This parameter can be a value of @ref SMARTCARD_Rx_Tx_Swap */

  uint32_t OverrunDisable;            /*!< Specifies whether the reception overrun detection is disabled.
                                           This parameter can be a value of @ref SMARTCARD_Overrun_Disable */

  uint32_t DMADisableonRxError;       /*!< Specifies whether the DMA is disabled in case of reception error.
                                           This parameter can be a value of @ref SMARTCARD_DMA_Disable_on_Rx_Error */

  uint32_t MSBFirst;                  /*!< Specifies whether MSB is sent first on UART line.
                                           This parameter can be a value of @ref SMARTCARD_MSB_First */

  uint16_t TxCompletionIndication;     /*!< Specifies which transmission completion indication is used: before (when
                                            relevant flag is available) or once guard time period has elapsed.
                                           This parameter can be a value of @ref SMARTCARDEx_Transmission_Completion_Indication. */
} SMARTCARD_AdvFeatureInitTypeDef;

/**
  * @brief HAL SMARTCARD State definition
  * @note  HAL SMARTCARD State value is a combination of 2 different substates: gState and RxState (see @ref SMARTCARD_State_Definition).
  *        - gState contains SMARTCARD state information related to global Handle management
  *          and also information related to Tx operations.
  *          gState value coding follow below described bitmap :
  *          b7-b6  Error information
  *             00 : No Error
  *             01 : (Not Used)
  *             10 : Timeout
  *             11 : Error
  *          b5     Peripheral initialization status
  *             0  : Reset (Peripheral not initialized)
  *             1  : Init done (Peripheral not initialized. HAL SMARTCARD Init function already called)
  *          b4-b3  (not used)
  *             xx : Should be set to 00
  *          b2     Intrinsic process state
  *             0  : Ready
  *             1  : Busy (Peripheral busy with some configuration or internal operations)
  *          b1     (not used)
  *             x  : Should be set to 0
  *          b0     Tx state
  *             0  : Ready (no Tx operation ongoing)
  *             1  : Busy (Tx operation ongoing)
  *        - RxState contains information related to Rx operations.
  *          RxState value coding follow below described bitmap :
  *          b7-b6  (not used)
  *             xx : Should be set to 00
  *          b5     Peripheral initialization status
  *             0  : Reset (Peripheral not initialized)
  *             1  : Init done (Peripheral not initialized)
  *          b4-b2  (not used)
  *            xxx : Should be set to 000
  *          b1     Rx state
  *             0  : Ready (no Rx operation ongoing)
  *             1  : Busy (Rx operation ongoing)
  *          b0     (not used)
  *             x  : Should be set to 0.
  */
typedef uint32_t HAL_SMARTCARD_StateTypeDef;

/**
  * @brief  SMARTCARD handle Structure definition
  */
typedef struct __SMARTCARD_HandleTypeDef
{
  USART_TypeDef                     *Instance;             /*!< USART registers base address                          */

  SMARTCARD_InitTypeDef             Init;                  /*!< SmartCard communication parameters                    */

  SMARTCARD_AdvFeatureInitTypeDef   AdvancedInit;          /*!< SmartCard advanced features initialization parameters */

  uint8_t                           *pTxBuffPtr;           /*!< Pointer to SmartCard Tx transfer Buffer               */

  uint16_t                          TxXferSize;            /*!< SmartCard Tx Transfer size                            */

  __IO uint16_t                     TxXferCount;           /*!< SmartCard Tx Transfer Counter                         */

  uint8_t                           *pRxBuffPtr;           /*!< Pointer to SmartCard Rx transfer Buffer               */

  uint16_t                          RxXferSize;            /*!< SmartCard Rx Transfer size                            */

  __IO uint16_t                     RxXferCount;           /*!< SmartCard Rx Transfer Counter                         */

#if defined(USART_CR1_FIFOEN)
  uint16_t                          NbRxDataToProcess;     /*!< Number of data to process during RX ISR execution     */

  uint16_t                          NbTxDataToProcess;     /*!< Number of data to process during TX ISR execution     */

  uint32_t                          FifoMode;              /*!< Specifies if the FIFO mode will be used.
                                                                This parameter can be a value of @ref SMARTCARDEx_FIFO_mode. */
#endif /* USART_CR1_FIFOEN */

  void (*RxISR)(struct __SMARTCARD_HandleTypeDef *huart);  /*!< Function pointer on Rx IRQ handler                    */

  void (*TxISR)(struct __SMARTCARD_HandleTypeDef *huart);  /*!< Function pointer on Tx IRQ handler                    */

  DMA_HandleTypeDef                 *hdmatx;               /*!< SmartCard Tx DMA Handle parameters                    */

  DMA_HandleTypeDef                 *hdmarx;               /*!< SmartCard Rx DMA Handle parameters                    */

  HAL_LockTypeDef                   Lock;                  /*!< Locking object                                        */

  __IO HAL_SMARTCARD_StateTypeDef   gState;                /*!< SmartCard state information related to global Handle management
                                                                and also related to Tx operations.
                                                                This parameter can be a value of @ref HAL_SMARTCARD_StateTypeDef */

  __IO HAL_SMARTCARD_StateTypeDef   RxState;               /*!< SmartCard state information related to Rx operations.
                                                                This parameter can be a value of @ref HAL_SMARTCARD_StateTypeDef */

  __IO uint32_t                     ErrorCode;             /*!< SmartCard Error code                                  */

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  void (* TxCpltCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);            /*!< SMARTCARD Tx Complete Callback             */

  void (* RxCpltCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);            /*!< SMARTCARD Rx Complete Callback             */

  void (* ErrorCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);             /*!< SMARTCARD Error Callback                   */

  void (* AbortCpltCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);         /*!< SMARTCARD Abort Complete Callback          */

  void (* AbortTransmitCpltCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard); /*!< SMARTCARD Abort Transmit Complete Callback */

  void (* AbortReceiveCpltCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);  /*!< SMARTCARD Abort Receive Complete Callback  */

#if defined(USART_CR1_FIFOEN)
  void (* RxFifoFullCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);        /*!< SMARTCARD Rx Fifo Full Callback            */

  void (* TxFifoEmptyCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);       /*!< SMARTCARD Tx Fifo Empty Callback           */

#endif /* USART_CR1_FIFOEN */
  void (* MspInitCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);           /*!< SMARTCARD Msp Init callback                */

  void (* MspDeInitCallback)(struct __SMARTCARD_HandleTypeDef *hsmartcard);         /*!< SMARTCARD Msp DeInit callback              */
#endif  /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */

} SMARTCARD_HandleTypeDef;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL SMARTCARD Callback ID enumeration definition
  */
typedef enum
{
  HAL_SMARTCARD_TX_COMPLETE_CB_ID             = 0x00U,    /*!< SMARTCARD Tx Complete Callback ID             */
  HAL_SMARTCARD_RX_COMPLETE_CB_ID             = 0x01U,    /*!< SMARTCARD Rx Complete Callback ID             */
  HAL_SMARTCARD_ERROR_CB_ID                   = 0x02U,    /*!< SMARTCARD Error Callback ID                   */
  HAL_SMARTCARD_ABORT_COMPLETE_CB_ID          = 0x03U,    /*!< SMARTCARD Abort Complete Callback ID          */
  HAL_SMARTCARD_ABORT_TRANSMIT_COMPLETE_CB_ID = 0x04U,    /*!< SMARTCARD Abort Transmit Complete Callback ID */
  HAL_SMARTCARD_ABORT_RECEIVE_COMPLETE_CB_ID  = 0x05U,    /*!< SMARTCARD Abort Receive Complete Callback ID  */
#if defined(USART_CR1_FIFOEN)
  HAL_SMARTCARD_RX_FIFO_FULL_CB_ID            = 0x06U,    /*!< SMARTCARD Rx Fifo Full Callback ID            */
  HAL_SMARTCARD_TX_FIFO_EMPTY_CB_ID           = 0x07U,    /*!< SMARTCARD Tx Fifo Empty Callback ID           */
#endif /* USART_CR1_FIFOEN */

  HAL_SMARTCARD_MSPINIT_CB_ID                 = 0x08U,    /*!< SMARTCARD MspInit callback ID                 */
  HAL_SMARTCARD_MSPDEINIT_CB_ID               = 0x09U     /*!< SMARTCARD MspDeInit callback ID               */

} HAL_SMARTCARD_CallbackIDTypeDef;

/**
  * @brief  HAL SMARTCARD Callback pointer definition
  */
typedef  void (*pSMARTCARD_CallbackTypeDef)(SMARTCARD_HandleTypeDef *hsmartcard);  /*!< pointer to an SMARTCARD callback function */

#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */

/**
  * @brief  SMARTCARD clock sources
  */
typedef enum
{
  SMARTCARD_CLOCKSOURCE_PCLK1     = 0x00U, /*!< PCLK1 clock source         */
  SMARTCARD_CLOCKSOURCE_PCLK2     = 0x01U, /*!< PCLK2 clock source         */
  SMARTCARD_CLOCKSOURCE_HSI       = 0x02U, /*!< HSI clock source           */
  SMARTCARD_CLOCKSOURCE_SYSCLK    = 0x04U, /*!< SYSCLK clock source        */
  SMARTCARD_CLOCKSOURCE_LSE       = 0x08U, /*!< LSE clock source           */
  SMARTCARD_CLOCKSOURCE_UNDEFINED = 0x10U  /*!< undefined clock source     */
} SMARTCARD_ClockSourceTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup SMARTCARD_Exported_Constants  SMARTCARD Exported Constants
  * @{
  */

/** @defgroup SMARTCARD_State_Definition SMARTCARD State Code Definition
  * @{
  */
#define HAL_SMARTCARD_STATE_RESET            0x00000000U                     /*!< Peripheral is not initialized
                                                                                  Value is allowed for gState and RxState */
#define HAL_SMARTCARD_STATE_READY            0x00000020U                     /*!< Peripheral Initialized and ready for use
                                                                                  Value is allowed for gState and RxState */
#define HAL_SMARTCARD_STATE_BUSY             0x00000024U                     /*!< an internal process is ongoing
                                                                                  Value is allowed for gState only */
#define HAL_SMARTCARD_STATE_BUSY_TX          0x00000021U                     /*!< Data Transmission process is ongoing
                                                                                  Value is allowed for gState only */
#define HAL_SMARTCARD_STATE_BUSY_RX          0x00000022U                     /*!< Data Reception process is ongoing
                                                                                  Value is allowed for RxState only */
#define HAL_SMARTCARD_STATE_BUSY_TX_RX       0x00000023U                     /*!< Data Transmission and Reception process is ongoing
                                                                                  Not to be used for neither gState nor RxState.
                                                                                  Value is result of combination (Or) between gState and RxState values */
#define HAL_SMARTCARD_STATE_TIMEOUT          0x000000A0U                     /*!< Timeout state
                                                                                  Value is allowed for gState only */
#define HAL_SMARTCARD_STATE_ERROR            0x000000E0U                     /*!< Error
                                                                                  Value is allowed for gState only */
/**
  * @}
  */

/** @defgroup SMARTCARD_Error_Definition SMARTCARD Error Code Definition
  * @{
  */
#define HAL_SMARTCARD_ERROR_NONE             ((uint32_t)0x00000000U)         /*!< No error                */
#define HAL_SMARTCARD_ERROR_PE               ((uint32_t)0x00000001U)         /*!< Parity error            */
#define HAL_SMARTCARD_ERROR_NE               ((uint32_t)0x00000002U)         /*!< Noise error             */
#define HAL_SMARTCARD_ERROR_FE               ((uint32_t)0x00000004U)         /*!< frame error             */
#define HAL_SMARTCARD_ERROR_ORE              ((uint32_t)0x00000008U)         /*!< Overrun error           */
#define HAL_SMARTCARD_ERROR_DMA              ((uint32_t)0x00000010U)         /*!< DMA transfer error      */
#define HAL_SMARTCARD_ERROR_RTO              ((uint32_t)0x00000020U)         /*!< Receiver TimeOut error  */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
#define HAL_SMARTCARD_ERROR_INVALID_CALLBACK ((uint32_t)0x00000040U)         /*!< Invalid Callback error  */
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @defgroup SMARTCARD_Word_Length SMARTCARD Word Length
  * @{
  */
#define SMARTCARD_WORDLENGTH_9B             USART_CR1_M0                    /*!< SMARTCARD frame length */
/**
  * @}
  */

/** @defgroup SMARTCARD_Stop_Bits SMARTCARD Number of Stop Bits
  * @{
  */
#define SMARTCARD_STOPBITS_0_5              USART_CR2_STOP_0                /*!< SMARTCARD frame with 0.5 stop bit  */
#define SMARTCARD_STOPBITS_1_5              USART_CR2_STOP                  /*!< SMARTCARD frame with 1.5 stop bits */
/**
  * @}
  */

/** @defgroup SMARTCARD_Parity SMARTCARD Parity
  * @{
  */
#define SMARTCARD_PARITY_EVEN               USART_CR1_PCE                   /*!< SMARTCARD frame even parity */
#define SMARTCARD_PARITY_ODD                (USART_CR1_PCE | USART_CR1_PS)  /*!< SMARTCARD frame odd parity  */
/**
  * @}
  */

/** @defgroup SMARTCARD_Mode SMARTCARD Transfer Mode
  * @{
  */
#define SMARTCARD_MODE_RX                   USART_CR1_RE                    /*!< SMARTCARD RX mode        */
#define SMARTCARD_MODE_TX                   USART_CR1_TE                    /*!< SMARTCARD TX mode        */
#define SMARTCARD_MODE_TX_RX                (USART_CR1_TE |USART_CR1_RE)    /*!< SMARTCARD RX and TX mode */
/**
  * @}
  */

/** @defgroup SMARTCARD_Clock_Polarity SMARTCARD Clock Polarity
  * @{
  */
#define SMARTCARD_POLARITY_LOW              0x00000000U                     /*!< SMARTCARD frame low polarity  */
#define SMARTCARD_POLARITY_HIGH             USART_CR2_CPOL                  /*!< SMARTCARD frame high polarity */
/**
  * @}
  */

/** @defgroup SMARTCARD_Clock_Phase SMARTCARD Clock Phase
  * @{
  */
#define SMARTCARD_PHASE_1EDGE               0x00000000U                     /*!< SMARTCARD frame phase on first clock transition  */
#define SMARTCARD_PHASE_2EDGE               USART_CR2_CPHA                  /*!< SMARTCARD frame phase on second clock transition */
/**
  * @}
  */

/** @defgroup SMARTCARD_Last_Bit SMARTCARD Last Bit
  * @{
  */
#define SMARTCARD_LASTBIT_DISABLE           0x00000000U                     /*!< SMARTCARD frame last data bit clock pulse not output to SCLK pin */
#define SMARTCARD_LASTBIT_ENABLE            USART_CR2_LBCL                  /*!< SMARTCARD frame last data bit clock pulse output to SCLK pin     */
/**
  * @}
  */

/** @defgroup SMARTCARD_OneBit_Sampling SMARTCARD One Bit Sampling Method
  * @{
  */
#define SMARTCARD_ONE_BIT_SAMPLE_DISABLE    0x00000000U                     /*!< SMARTCARD frame one-bit sample disabled */
#define SMARTCARD_ONE_BIT_SAMPLE_ENABLE     USART_CR3_ONEBIT                /*!< SMARTCARD frame one-bit sample enabled  */
/**
  * @}
  */

/** @defgroup SMARTCARD_NACK_Enable SMARTCARD NACK Enable
  * @{
  */
#define SMARTCARD_NACK_DISABLE              0x00000000U                     /*!< SMARTCARD NACK transmission disabled  */
#define SMARTCARD_NACK_ENABLE               USART_CR3_NACK                  /*!< SMARTCARD NACK transmission enabled */
/**
  * @}
  */

/** @defgroup SMARTCARD_Timeout_Enable SMARTCARD Timeout Enable
  * @{
  */
#define SMARTCARD_TIMEOUT_DISABLE           0x00000000U                     /*!< SMARTCARD receiver timeout disabled */
#define SMARTCARD_TIMEOUT_ENABLE            USART_CR2_RTOEN                 /*!< SMARTCARD receiver timeout enabled  */
/**
  * @}
  */

#if defined(USART_PRESC_PRESCALER)
/** @defgroup SMARTCARD_ClockPrescaler  Clock Prescaler
  * @{
  */
#define SMARTCARD_PRESCALER_DIV1    0x00000000U  /*!< fclk_pres = fclk     */
#define SMARTCARD_PRESCALER_DIV2    0x00000001U  /*!< fclk_pres = fclk/2   */
#define SMARTCARD_PRESCALER_DIV4    0x00000002U  /*!< fclk_pres = fclk/4   */
#define SMARTCARD_PRESCALER_DIV6    0x00000003U  /*!< fclk_pres = fclk/6   */
#define SMARTCARD_PRESCALER_DIV8    0x00000004U  /*!< fclk_pres = fclk/8   */
#define SMARTCARD_PRESCALER_DIV10   0x00000005U  /*!< fclk_pres = fclk/10  */
#define SMARTCARD_PRESCALER_DIV12   0x00000006U  /*!< fclk_pres = fclk/12  */
#define SMARTCARD_PRESCALER_DIV16   0x00000007U  /*!< fclk_pres = fclk/16  */
#define SMARTCARD_PRESCALER_DIV32   0x00000008U  /*!< fclk_pres = fclk/32  */
#define SMARTCARD_PRESCALER_DIV64   0x00000009U  /*!< fclk_pres = fclk/64  */
#define SMARTCARD_PRESCALER_DIV128  0x0000000AU  /*!< fclk_pres = fclk/128 */
#define SMARTCARD_PRESCALER_DIV256  0x0000000BU  /*!< fclk_pres = fclk/256 */
/**
  * @}
  */

#endif /* USART_PRESC_PRESCALER */
/** @defgroup SMARTCARD_Tx_Inv SMARTCARD advanced feature TX pin active level inversion
  * @{
  */
#define SMARTCARD_ADVFEATURE_TXINV_DISABLE  0x00000000U                  /*!< TX pin active level inversion disable */
#define SMARTCARD_ADVFEATURE_TXINV_ENABLE   USART_CR2_TXINV              /*!< TX pin active level inversion enable  */
/**
  * @}
  */

/** @defgroup SMARTCARD_Rx_Inv SMARTCARD advanced feature RX pin active level inversion
  * @{
  */
#define SMARTCARD_ADVFEATURE_RXINV_DISABLE  0x00000000U                  /*!< RX pin active level inversion disable */
#define SMARTCARD_ADVFEATURE_RXINV_ENABLE   USART_CR2_RXINV              /*!< RX pin active level inversion enable  */
/**
  * @}
  */

/** @defgroup SMARTCARD_Data_Inv SMARTCARD advanced feature Binary Data inversion
  * @{
  */
#define SMARTCARD_ADVFEATURE_DATAINV_DISABLE  0x00000000U                /*!< Binary data inversion disable */
#define SMARTCARD_ADVFEATURE_DATAINV_ENABLE   USART_CR2_DATAINV          /*!< Binary data inversion enable  */
/**
  * @}
  */

/** @defgroup SMARTCARD_Rx_Tx_Swap SMARTCARD advanced feature RX TX pins swap
  * @{
  */
#define SMARTCARD_ADVFEATURE_SWAP_DISABLE   0x00000000U                  /*!< TX/RX pins swap disable */
#define SMARTCARD_ADVFEATURE_SWAP_ENABLE    USART_CR2_SWAP               /*!< TX/RX pins swap enable  */
/**
  * @}
  */

/** @defgroup SMARTCARD_Overrun_Disable SMARTCARD advanced feature Overrun Disable
  * @{
  */
#define SMARTCARD_ADVFEATURE_OVERRUN_ENABLE   0x00000000U                /*!< RX overrun enable  */
#define SMARTCARD_ADVFEATURE_OVERRUN_DISABLE  USART_CR3_OVRDIS           /*!< RX overrun disable */
/**
  * @}
  */

/** @defgroup SMARTCARD_DMA_Disable_on_Rx_Error SMARTCARD advanced feature DMA Disable on Rx Error
  * @{
  */
#define SMARTCARD_ADVFEATURE_DMA_ENABLEONRXERROR   0x00000000U           /*!< DMA enable on Reception Error  */
#define SMARTCARD_ADVFEATURE_DMA_DISABLEONRXERROR  USART_CR3_DDRE        /*!< DMA disable on Reception Error */
/**
  * @}
  */

/** @defgroup SMARTCARD_MSB_First   SMARTCARD advanced feature MSB first
  * @{
  */
#define SMARTCARD_ADVFEATURE_MSBFIRST_DISABLE      0x00000000U           /*!< Most significant bit sent/received first disable */
#define SMARTCARD_ADVFEATURE_MSBFIRST_ENABLE       USART_CR2_MSBFIRST    /*!< Most significant bit sent/received first enable  */
/**
  * @}
  */

/** @defgroup SMARTCARD_Request_Parameters SMARTCARD Request Parameters
  * @{
  */
#define SMARTCARD_RXDATA_FLUSH_REQUEST      USART_RQR_RXFRQ              /*!< Receive data flush request */
#define SMARTCARD_TXDATA_FLUSH_REQUEST      USART_RQR_TXFRQ              /*!< Transmit data flush request */
/**
  * @}
  */

/** @defgroup SMARTCARD_Interruption_Mask SMARTCARD interruptions flags mask
  * @{
  */
#define SMARTCARD_IT_MASK                   0x001FU   /*!< SMARTCARD interruptions flags mask  */
#define SMARTCARD_CR_MASK                   0x00E0U   /*!< SMARTCARD control register mask     */
#define SMARTCARD_CR_POS                    5U        /*!< SMARTCARD control register position */
#define SMARTCARD_ISR_MASK                  0x1F00U   /*!< SMARTCARD ISR register mask         */
#define SMARTCARD_ISR_POS                   8U        /*!< SMARTCARD ISR register position     */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup SMARTCARD_Exported_Macros  SMARTCARD Exported Macros
  * @{
  */

/** @brief  Reset SMARTCARD handle states.
  * @param  __HANDLE__ SMARTCARD handle.
  * @retval None
  */
#if USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1
#define __HAL_SMARTCARD_RESET_HANDLE_STATE(__HANDLE__)  do{                                                       \
                                                            (__HANDLE__)->gState = HAL_SMARTCARD_STATE_RESET;     \
                                                            (__HANDLE__)->RxState = HAL_SMARTCARD_STATE_RESET;    \
                                                            (__HANDLE__)->MspInitCallback = NULL;                 \
                                                            (__HANDLE__)->MspDeInitCallback = NULL;               \
                                                          } while(0U)
#else
#define __HAL_SMARTCARD_RESET_HANDLE_STATE(__HANDLE__)  do{                                                       \
                                                            (__HANDLE__)->gState = HAL_SMARTCARD_STATE_RESET;     \
                                                            (__HANDLE__)->RxState = HAL_SMARTCARD_STATE_RESET;    \
                                                          } while(0U)
#endif /*USE_HAL_SMARTCARD_REGISTER_CALLBACKS  */

/** @brief  Flush the Smartcard Data registers.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_FLUSH_DRREGISTER(__HANDLE__)                      \
  do{                                                                     \
    SET_BIT((__HANDLE__)->Instance->RQR, SMARTCARD_RXDATA_FLUSH_REQUEST); \
    SET_BIT((__HANDLE__)->Instance->RQR, SMARTCARD_TXDATA_FLUSH_REQUEST); \
  } while(0U)

/** @brief  Clear the specified SMARTCARD pending flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __FLAG__ specifies the flag to check.
  *          This parameter can be any combination of the following values:
  *            @arg @ref SMARTCARD_CLEAR_PEF    Parity error clear flag
  *            @arg @ref SMARTCARD_CLEAR_FEF    Framing error clear flag
  *            @arg @ref SMARTCARD_CLEAR_NEF    Noise detected clear flag
  *            @arg @ref SMARTCARD_CLEAR_OREF   OverRun error clear flag
  *            @arg @ref SMARTCARD_CLEAR_IDLEF  Idle line detected clear flag
  *            @arg @ref SMARTCARD_CLEAR_TCF    Transmission complete clear flag
  *            @arg @ref SMARTCARD_CLEAR_TCBGTF Transmission complete before guard time clear flag
  *            @arg @ref SMARTCARD_CLEAR_RTOF   Receiver timeout clear flag
  *            @arg @ref SMARTCARD_CLEAR_EOBF   End of block clear flag
#if defined(USART_CR1_FIFOEN)
  *            @arg @ref SMARTCARD_CLEAR_TXFECF TXFIFO empty Clear flag
#endif
  * @retval None
  */
#define __HAL_SMARTCARD_CLEAR_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->Instance->ICR = (__FLAG__))

/** @brief  Clear the SMARTCARD PE pending flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_CLEAR_PEFLAG(__HANDLE__)   __HAL_SMARTCARD_CLEAR_FLAG((__HANDLE__), SMARTCARD_CLEAR_PEF)

/** @brief  Clear the SMARTCARD FE pending flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_CLEAR_FEFLAG(__HANDLE__)   __HAL_SMARTCARD_CLEAR_FLAG((__HANDLE__), SMARTCARD_CLEAR_FEF)

/** @brief  Clear the SMARTCARD NE pending flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_CLEAR_NEFLAG(__HANDLE__)   __HAL_SMARTCARD_CLEAR_FLAG((__HANDLE__), SMARTCARD_CLEAR_NEF)

/** @brief  Clear the SMARTCARD ORE pending flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_CLEAR_OREFLAG(__HANDLE__)   __HAL_SMARTCARD_CLEAR_FLAG((__HANDLE__), SMARTCARD_CLEAR_OREF)

/** @brief  Clear the SMARTCARD IDLE pending flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_CLEAR_IDLEFLAG(__HANDLE__)   __HAL_SMARTCARD_CLEAR_FLAG((__HANDLE__), SMARTCARD_CLEAR_IDLEF)

/** @brief  Check whether the specified Smartcard flag is set or not.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __FLAG__ specifies the flag to check.
  *        This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_FLAG_TCBGT Transmission complete before guard time flag (when flag available)
  *            @arg @ref SMARTCARD_FLAG_REACK Receive enable acknowledge flag
  *            @arg @ref SMARTCARD_FLAG_TEACK Transmit enable acknowledge flag
  *            @arg @ref SMARTCARD_FLAG_BUSY  Busy flag
  *            @arg @ref SMARTCARD_FLAG_EOBF  End of block flag
  *            @arg @ref SMARTCARD_FLAG_RTOF  Receiver timeout flag
  *            @arg @ref SMARTCARD_FLAG_TXE   Transmit data register empty flag
  *            @arg @ref SMARTCARD_FLAG_TC    Transmission complete flag
  *            @arg @ref SMARTCARD_FLAG_RXNE  Receive data register not empty flag
  *            @arg @ref SMARTCARD_FLAG_IDLE  Idle line detection flag
  *            @arg @ref SMARTCARD_FLAG_ORE   Overrun error flag
  *            @arg @ref SMARTCARD_FLAG_NE    Noise error flag
  *            @arg @ref SMARTCARD_FLAG_FE    Framing error flag
  *            @arg @ref SMARTCARD_FLAG_PE    Parity error flag
  *            @arg @ref SMARTCARD_FLAG_TXFNF TXFIFO not full flag
  *            @arg @ref SMARTCARD_FLAG_RXFNE RXFIFO not empty flag
  *            @arg @ref SMARTCARD_FLAG_TXFE  TXFIFO Empty flag
  *            @arg @ref SMARTCARD_FLAG_RXFF  RXFIFO Full flag
  *            @arg @ref SMARTCARD_FLAG_RXFT  SMARTCARD RXFIFO threshold flag
  *            @arg @ref SMARTCARD_FLAG_TXFT  SMARTCARD TXFIFO threshold flag
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_SMARTCARD_GET_FLAG(__HANDLE__, __FLAG__) (((__HANDLE__)->Instance->ISR & (__FLAG__)) == (__FLAG__))

/** @brief  Enable the specified SmartCard interrupt.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __INTERRUPT__ specifies the SMARTCARD interrupt to enable.
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_IT_EOB    End of block interrupt
  *            @arg @ref SMARTCARD_IT_RTO    Receive timeout interrupt
  *            @arg @ref SMARTCARD_IT_TXE    Transmit data register empty interrupt
  *            @arg @ref SMARTCARD_IT_TC     Transmission complete interrupt
  *            @arg @ref SMARTCARD_IT_TCBGT  Transmission complete before guard time interrupt (when interruption available)
  *            @arg @ref SMARTCARD_IT_RXNE   Receive data register not empty interrupt
  *            @arg @ref SMARTCARD_IT_IDLE   Idle line detection interrupt
  *            @arg @ref SMARTCARD_IT_PE     Parity error interrupt
  *            @arg @ref SMARTCARD_IT_ERR    Error interrupt(frame error, noise error, overrun error)
  *            @arg @ref SMARTCARD_IT_TXFNF  TX FIFO not full interruption
  *            @arg @ref SMARTCARD_IT_RXFNE  RXFIFO not empty interruption
  *            @arg @ref SMARTCARD_IT_RXFF   RXFIFO full interruption
  *            @arg @ref SMARTCARD_IT_TXFE   TXFIFO empty interruption
  *            @arg @ref SMARTCARD_IT_RXFT   RXFIFO threshold reached interruption
  *            @arg @ref SMARTCARD_IT_TXFT   TXFIFO threshold reached interruption
  * @retval None
  */
#define __HAL_SMARTCARD_ENABLE_IT(__HANDLE__, __INTERRUPT__)   (((((__INTERRUPT__) & SMARTCARD_CR_MASK) >> SMARTCARD_CR_POS) == 1U)? ((__HANDLE__)->Instance->CR1 |= ((uint32_t)1U << ((__INTERRUPT__) & SMARTCARD_IT_MASK))): \
                                                                ((((__INTERRUPT__) & SMARTCARD_CR_MASK) >> SMARTCARD_CR_POS) == 2U)? ((__HANDLE__)->Instance->CR2 |= ((uint32_t)1U << ((__INTERRUPT__) & SMARTCARD_IT_MASK))): \
                                                                ((__HANDLE__)->Instance->CR3 |= ((uint32_t)1U << ((__INTERRUPT__) & SMARTCARD_IT_MASK))))

/** @brief  Disable the specified SmartCard interrupt.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __INTERRUPT__ specifies the SMARTCARD interrupt to disable.
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_IT_EOB    End of block interrupt
  *            @arg @ref SMARTCARD_IT_RTO    Receive timeout interrupt
  *            @arg @ref SMARTCARD_IT_TXE    Transmit data register empty interrupt
  *            @arg @ref SMARTCARD_IT_TC     Transmission complete interrupt
  *            @arg @ref SMARTCARD_IT_TCBGT  Transmission complete before guard time interrupt (when interruption available)
  *            @arg @ref SMARTCARD_IT_RXNE   Receive data register not empty interrupt
  *            @arg @ref SMARTCARD_IT_IDLE   Idle line detection interrupt
  *            @arg @ref SMARTCARD_IT_PE     Parity error interrupt
  *            @arg @ref SMARTCARD_IT_ERR    Error interrupt(frame error, noise error, overrun error)
  *            @arg @ref SMARTCARD_IT_TXFNF  TX FIFO not full interruption
  *            @arg @ref SMARTCARD_IT_RXFNE  RXFIFO not empty interruption
  *            @arg @ref SMARTCARD_IT_RXFF   RXFIFO full interruption
  *            @arg @ref SMARTCARD_IT_TXFE   TXFIFO empty interruption
  *            @arg @ref SMARTCARD_IT_RXFT   RXFIFO threshold reached interruption
  *            @arg @ref SMARTCARD_IT_TXFT   TXFIFO threshold reached interruption
  * @retval None
  */
#define __HAL_SMARTCARD_DISABLE_IT(__HANDLE__, __INTERRUPT__)  (((((__INTERRUPT__) & SMARTCARD_CR_MASK) >> SMARTCARD_CR_POS) == 1U)? ((__HANDLE__)->Instance->CR1 &= ~ ((uint32_t)1U << ((__INTERRUPT__) & SMARTCARD_IT_MASK))): \
                                                                ((((__INTERRUPT__) & SMARTCARD_CR_MASK) >> SMARTCARD_CR_POS) == 2U)? ((__HANDLE__)->Instance->CR2 &= ~ ((uint32_t)1U << ((__INTERRUPT__) & SMARTCARD_IT_MASK))): \
                                                                ((__HANDLE__)->Instance->CR3 &= ~ ((uint32_t)1U << ((__INTERRUPT__) & SMARTCARD_IT_MASK))))

/** @brief  Check whether the specified SmartCard interrupt has occurred or not.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __INTERRUPT__ specifies the SMARTCARD interrupt to check.
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_IT_EOB    End of block interrupt
  *            @arg @ref SMARTCARD_IT_RTO    Receive timeout interrupt
  *            @arg @ref SMARTCARD_IT_TXE    Transmit data register empty interrupt
  *            @arg @ref SMARTCARD_IT_TC     Transmission complete interrupt
  *            @arg @ref SMARTCARD_IT_TCBGT  Transmission complete before guard time interrupt (when interruption available)
  *            @arg @ref SMARTCARD_IT_RXNE   Receive data register not empty interrupt
  *            @arg @ref SMARTCARD_IT_IDLE   Idle line detection interrupt
  *            @arg @ref SMARTCARD_IT_PE     Parity error interrupt
  *            @arg @ref SMARTCARD_IT_ERR    Error interrupt(frame error, noise error, overrun error)
  *            @arg @ref SMARTCARD_IT_TXFNF  TX FIFO not full interruption
  *            @arg @ref SMARTCARD_IT_RXFNE  RXFIFO not empty interruption
  *            @arg @ref SMARTCARD_IT_RXFF   RXFIFO full interruption
  *            @arg @ref SMARTCARD_IT_TXFE   TXFIFO empty interruption
  *            @arg @ref SMARTCARD_IT_RXFT   RXFIFO threshold reached interruption
  *            @arg @ref SMARTCARD_IT_TXFT   TXFIFO threshold reached interruption
  * @retval The new state of __INTERRUPT__ (SET or RESET).
  */
#define __HAL_SMARTCARD_GET_IT(__HANDLE__, __INTERRUPT__) ((((__HANDLE__)->Instance->ISR\
                                                             & ((uint32_t)0x01U << (((__INTERRUPT__) & SMARTCARD_ISR_MASK)>> SMARTCARD_ISR_POS))) != 0U) ? SET : RESET)

/** @brief  Check whether the specified SmartCard interrupt source is enabled or not.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __INTERRUPT__ specifies the SMARTCARD interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_IT_EOB    End of block interrupt
  *            @arg @ref SMARTCARD_IT_RTO    Receive timeout interrupt
  *            @arg @ref SMARTCARD_IT_TXE    Transmit data register empty interrupt
  *            @arg @ref SMARTCARD_IT_TC     Transmission complete interrupt
  *            @arg @ref SMARTCARD_IT_TCBGT  Transmission complete before guard time interrupt (when interruption available)
  *            @arg @ref SMARTCARD_IT_RXNE   Receive data register not empty interrupt
  *            @arg @ref SMARTCARD_IT_IDLE   Idle line detection interrupt
  *            @arg @ref SMARTCARD_IT_PE     Parity error interrupt
  *            @arg @ref SMARTCARD_IT_ERR    Error interrupt(frame error, noise error, overrun error)
  *            @arg @ref SMARTCARD_IT_TXFNF  TX FIFO not full interruption
  *            @arg @ref SMARTCARD_IT_RXFNE  RXFIFO not empty interruption
  *            @arg @ref SMARTCARD_IT_RXFF   RXFIFO full interruption
  *            @arg @ref SMARTCARD_IT_TXFE   TXFIFO empty interruption
  *            @arg @ref SMARTCARD_IT_RXFT   RXFIFO threshold reached interruption
  *            @arg @ref SMARTCARD_IT_TXFT   TXFIFO threshold reached interruption
  * @retval The new state of __INTERRUPT__ (SET or RESET).
  */
#define __HAL_SMARTCARD_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) ((((((((__INTERRUPT__) & SMARTCARD_CR_MASK) >> SMARTCARD_CR_POS) == 0x01U)? (__HANDLE__)->Instance->CR1 : \
                                                                     (((((__INTERRUPT__) & SMARTCARD_CR_MASK) >> SMARTCARD_CR_POS) == 0x02U)? (__HANDLE__)->Instance->CR2 : \
                                                                      (__HANDLE__)->Instance->CR3)) & ((uint32_t)0x01U << (((uint16_t)(__INTERRUPT__)) & SMARTCARD_IT_MASK)))  != 0U) ? SET : RESET)

/** @brief  Clear the specified SMARTCARD ISR flag, in setting the proper ICR register flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __IT_CLEAR__ specifies the interrupt clear register flag that needs to be set
  *                       to clear the corresponding interrupt.
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_CLEAR_PEF    Parity error clear flag
  *            @arg @ref SMARTCARD_CLEAR_FEF    Framing error clear flag
  *            @arg @ref SMARTCARD_CLEAR_NEF    Noise detected clear flag
  *            @arg @ref SMARTCARD_CLEAR_OREF   OverRun error clear flag
  *            @arg @ref SMARTCARD_CLEAR_IDLEF  Idle line detection clear flag
  *            @arg @ref SMARTCARD_CLEAR_TXFECF TXFIFO empty Clear Flag
  *            @arg @ref SMARTCARD_CLEAR_TCF    Transmission complete clear flag
  *            @arg @ref SMARTCARD_CLEAR_TCBGTF Transmission complete before guard time clear flag (when flag available)
  *            @arg @ref SMARTCARD_CLEAR_RTOF   Receiver timeout clear flag
  *            @arg @ref SMARTCARD_CLEAR_EOBF   End of block clear flag
  * @retval None
  */
#define __HAL_SMARTCARD_CLEAR_IT(__HANDLE__, __IT_CLEAR__) ((__HANDLE__)->Instance->ICR |= (uint32_t)(__IT_CLEAR__))

/** @brief  Set a specific SMARTCARD request flag.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __REQ__ specifies the request flag to set
  *          This parameter can be one of the following values:
  *            @arg @ref SMARTCARD_RXDATA_FLUSH_REQUEST Receive data flush Request
  *            @arg @ref SMARTCARD_TXDATA_FLUSH_REQUEST Transmit data flush Request
  * @retval None
  */
#define __HAL_SMARTCARD_SEND_REQ(__HANDLE__, __REQ__) ((__HANDLE__)->Instance->RQR |= (uint16_t)(__REQ__))

/** @brief  Enable the SMARTCARD one bit sample method.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_ONE_BIT_SAMPLE_ENABLE(__HANDLE__) ((__HANDLE__)->Instance->CR3|= USART_CR3_ONEBIT)

/** @brief  Disable the SMARTCARD one bit sample method.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_ONE_BIT_SAMPLE_DISABLE(__HANDLE__) ((__HANDLE__)->Instance->CR3\
                                                            &= (uint32_t)~((uint32_t)USART_CR3_ONEBIT))

/** @brief  Enable the USART associated to the SMARTCARD Handle.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_ENABLE(__HANDLE__)               ((__HANDLE__)->Instance->CR1 |=  USART_CR1_UE)

/** @brief  Disable the USART associated to the SMARTCARD Handle
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @retval None
  */
#define __HAL_SMARTCARD_DISABLE(__HANDLE__)              ((__HANDLE__)->Instance->CR1 &=  ~USART_CR1_UE)

/**
  * @}
  */

/* Private macros -------------------------------------------------------------*/
/** @defgroup SMARTCARD_Private_Macros SMARTCARD Private Macros
  * @{
  */

/** @brief  Report the SMARTCARD clock source.
  * @param  __HANDLE__ specifies the SMARTCARD Handle.
  * @param  __CLOCKSOURCE__ output variable.
  * @retval the SMARTCARD clocking source, written in __CLOCKSOURCE__.
  */
#if defined (STM32L432xx) || defined (STM32L442xx)
#define SMARTCARD_GETCLOCKSOURCE(__HANDLE__,__CLOCKSOURCE__)   \
  do {                                                         \
    if((__HANDLE__)->Instance == USART1)                       \
    {                                                          \
      switch(__HAL_RCC_GET_USART1_SOURCE())                    \
      {                                                        \
        case RCC_USART1CLKSOURCE_PCLK2:                        \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_PCLK2;     \
          break;                                               \
        case RCC_USART1CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_HSI;       \
          break;                                               \
        case RCC_USART1CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_SYSCLK;    \
          break;                                               \
        case RCC_USART1CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_LSE;       \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_UNDEFINED; \
          break;                                               \
      }                                                        \
    }                                                          \
    else if((__HANDLE__)->Instance == USART2)                  \
    {                                                          \
      switch(__HAL_RCC_GET_USART2_SOURCE())                    \
      {                                                        \
        case RCC_USART2CLKSOURCE_PCLK1:                        \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_PCLK1;     \
          break;                                               \
        case RCC_USART2CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_HSI;       \
          break;                                               \
        case RCC_USART2CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_SYSCLK;    \
          break;                                               \
        case RCC_USART2CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_LSE;       \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_UNDEFINED; \
          break;                                               \
      }                                                        \
    }                                                          \
    else                                                       \
    {                                                          \
      (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_UNDEFINED;     \
    }                                                          \
  } while(0)
#else
#define SMARTCARD_GETCLOCKSOURCE(__HANDLE__,__CLOCKSOURCE__)   \
  do {                                                         \
    if((__HANDLE__)->Instance == USART1)                       \
    {                                                          \
      switch(__HAL_RCC_GET_USART1_SOURCE())                    \
      {                                                        \
        case RCC_USART1CLKSOURCE_PCLK2:                        \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_PCLK2;     \
          break;                                               \
        case RCC_USART1CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_HSI;       \
          break;                                               \
        case RCC_USART1CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_SYSCLK;    \
          break;                                               \
        case RCC_USART1CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_LSE;       \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_UNDEFINED; \
          break;                                               \
      }                                                        \
    }                                                          \
    else if((__HANDLE__)->Instance == USART2)                  \
    {                                                          \
      switch(__HAL_RCC_GET_USART2_SOURCE())                    \
      {                                                        \
        case RCC_USART2CLKSOURCE_PCLK1:                        \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_PCLK1;     \
          break;                                               \
        case RCC_USART2CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_HSI;       \
          break;                                               \
        case RCC_USART2CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_SYSCLK;    \
          break;                                               \
        case RCC_USART2CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_LSE;       \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_UNDEFINED; \
          break;                                               \
      }                                                        \
    }                                                          \
    else if((__HANDLE__)->Instance == USART3)                  \
    {                                                          \
      switch(__HAL_RCC_GET_USART3_SOURCE())                    \
      {                                                        \
        case RCC_USART3CLKSOURCE_PCLK1:                        \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_PCLK1;     \
          break;                                               \
        case RCC_USART3CLKSOURCE_HSI:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_HSI;       \
          break;                                               \
        case RCC_USART3CLKSOURCE_SYSCLK:                       \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_SYSCLK;    \
          break;                                               \
        case RCC_USART3CLKSOURCE_LSE:                          \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_LSE;       \
          break;                                               \
        default:                                               \
          (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_UNDEFINED; \
          break;                                               \
      }                                                        \
    }                                                          \
    else                                                       \
    {                                                          \
      (__CLOCKSOURCE__) = SMARTCARD_CLOCKSOURCE_UNDEFINED;     \
    }                                                          \
  } while(0)
#endif /* STM32L432xx || STM32L442xx */

/** @brief  Check the Baud rate range.
  * @note   The maximum Baud Rate is derived from the maximum clock on L4 (120 MHz)
  *         divided by the oversampling used on the SMARTCARD (i.e. 16).
  * @param  __BAUDRATE__ Baud rate set by the configuration function.
  * @retval Test result (TRUE or FALSE)
  */
#define IS_SMARTCARD_BAUDRATE(__BAUDRATE__) ((__BAUDRATE__) < 7500001U)

/** @brief  Check the block length range.
  * @note   The maximum SMARTCARD block length is 0xFF.
  * @param  __LENGTH__ block length.
  * @retval Test result (TRUE or FALSE)
  */
#define IS_SMARTCARD_BLOCKLENGTH(__LENGTH__) ((__LENGTH__) <= 0xFFU)

/** @brief  Check the receiver timeout value.
  * @note   The maximum SMARTCARD receiver timeout value is 0xFFFFFF.
  * @param  __TIMEOUTVALUE__ receiver timeout value.
  * @retval Test result (TRUE or FALSE)
  */
#define IS_SMARTCARD_TIMEOUT_VALUE(__TIMEOUTVALUE__)    ((__TIMEOUTVALUE__) <= 0xFFFFFFU)

/** @brief  Check the SMARTCARD autoretry counter value.
  * @note   The maximum number of retransmissions is 0x7.
  * @param  __COUNT__ number of retransmissions.
  * @retval Test result (TRUE or FALSE)
  */
#define IS_SMARTCARD_AUTORETRY_COUNT(__COUNT__)         ((__COUNT__) <= 0x7U)

/** @brief Ensure that SMARTCARD frame length is valid.
  * @param __LENGTH__ SMARTCARD frame length.
  * @retval SET (__LENGTH__ is valid) or RESET (__LENGTH__ is invalid)
  */
#define IS_SMARTCARD_WORD_LENGTH(__LENGTH__) ((__LENGTH__) == SMARTCARD_WORDLENGTH_9B)

/** @brief Ensure that SMARTCARD frame number of stop bits is valid.
  * @param __STOPBITS__ SMARTCARD frame number of stop bits.
  * @retval SET (__STOPBITS__ is valid) or RESET (__STOPBITS__ is invalid)
  */
#define IS_SMARTCARD_STOPBITS(__STOPBITS__) (((__STOPBITS__) == SMARTCARD_STOPBITS_0_5) ||\
                                             ((__STOPBITS__) == SMARTCARD_STOPBITS_1_5))

/** @brief Ensure that SMARTCARD frame parity is valid.
  * @param __PARITY__ SMARTCARD frame parity.
  * @retval SET (__PARITY__ is valid) or RESET (__PARITY__ is invalid)
  */
#define IS_SMARTCARD_PARITY(__PARITY__) (((__PARITY__) == SMARTCARD_PARITY_EVEN) || \
                                         ((__PARITY__) == SMARTCARD_PARITY_ODD))

/** @brief Ensure that SMARTCARD communication mode is valid.
  * @param __MODE__ SMARTCARD communication mode.
  * @retval SET (__MODE__ is valid) or RESET (__MODE__ is invalid)
  */
#define IS_SMARTCARD_MODE(__MODE__) ((((__MODE__) & 0xFFF3U) == 0x00U) && ((__MODE__) != 0x00U))

/** @brief Ensure that SMARTCARD frame polarity is valid.
  * @param __CPOL__ SMARTCARD frame polarity.
  * @retval SET (__CPOL__ is valid) or RESET (__CPOL__ is invalid)
  */
#define IS_SMARTCARD_POLARITY(__CPOL__) (((__CPOL__) == SMARTCARD_POLARITY_LOW)\
                                         || ((__CPOL__) == SMARTCARD_POLARITY_HIGH))

/** @brief Ensure that SMARTCARD frame phase is valid.
  * @param __CPHA__ SMARTCARD frame phase.
  * @retval SET (__CPHA__ is valid) or RESET (__CPHA__ is invalid)
  */
#define IS_SMARTCARD_PHASE(__CPHA__) (((__CPHA__) == SMARTCARD_PHASE_1EDGE) || ((__CPHA__) == SMARTCARD_PHASE_2EDGE))

/** @brief Ensure that SMARTCARD frame last bit clock pulse setting is valid.
  * @param __LASTBIT__ SMARTCARD frame last bit clock pulse setting.
  * @retval SET (__LASTBIT__ is valid) or RESET (__LASTBIT__ is invalid)
  */
#define IS_SMARTCARD_LASTBIT(__LASTBIT__) (((__LASTBIT__) == SMARTCARD_LASTBIT_DISABLE) || \
                                           ((__LASTBIT__) == SMARTCARD_LASTBIT_ENABLE))

/** @brief Ensure that SMARTCARD frame sampling is valid.
  * @param __ONEBIT__ SMARTCARD frame sampling.
  * @retval SET (__ONEBIT__ is valid) or RESET (__ONEBIT__ is invalid)
  */
#define IS_SMARTCARD_ONE_BIT_SAMPLE(__ONEBIT__) (((__ONEBIT__) == SMARTCARD_ONE_BIT_SAMPLE_DISABLE) || \
                                                 ((__ONEBIT__) == SMARTCARD_ONE_BIT_SAMPLE_ENABLE))

/** @brief Ensure that SMARTCARD NACK transmission setting is valid.
  * @param __NACK__ SMARTCARD NACK transmission setting.
  * @retval SET (__NACK__ is valid) or RESET (__NACK__ is invalid)
  */
#define IS_SMARTCARD_NACK(__NACK__) (((__NACK__) == SMARTCARD_NACK_ENABLE) || \
                                     ((__NACK__) == SMARTCARD_NACK_DISABLE))

/** @brief Ensure that SMARTCARD receiver timeout setting is valid.
  * @param __TIMEOUT__ SMARTCARD receiver timeout setting.
  * @retval SET (__TIMEOUT__ is valid) or RESET (__TIMEOUT__ is invalid)
  */
#define IS_SMARTCARD_TIMEOUT(__TIMEOUT__) (((__TIMEOUT__) == SMARTCARD_TIMEOUT_DISABLE) || \
                                           ((__TIMEOUT__) == SMARTCARD_TIMEOUT_ENABLE))

#if defined(USART_PRESC_PRESCALER)
/** @brief Ensure that SMARTCARD clock Prescaler is valid.
  * @param __CLOCKPRESCALER__ SMARTCARD clock Prescaler value.
  * @retval SET (__CLOCKPRESCALER__ is valid) or RESET (__CLOCKPRESCALER__ is invalid)
  */
#define IS_SMARTCARD_CLOCKPRESCALER(__CLOCKPRESCALER__) (((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV1)   || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV2)   || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV4)   || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV6)   || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV8)   || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV10)  || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV12)  || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV16)  || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV32)  || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV64)  || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV128) || \
                                                         ((__CLOCKPRESCALER__) == SMARTCARD_PRESCALER_DIV256))

#endif /* USART_PRESC_PRESCALER */
/** @brief Ensure that SMARTCARD advanced features initialization is valid.
  * @param __INIT__ SMARTCARD advanced features initialization.
  * @retval SET (__INIT__ is valid) or RESET (__INIT__ is invalid)
  */
#define IS_SMARTCARD_ADVFEATURE_INIT(__INIT__) ((__INIT__) <= (SMARTCARD_ADVFEATURE_NO_INIT                | \
                                                               SMARTCARD_ADVFEATURE_TXINVERT_INIT          | \
                                                               SMARTCARD_ADVFEATURE_RXINVERT_INIT          | \
                                                               SMARTCARD_ADVFEATURE_DATAINVERT_INIT        | \
                                                               SMARTCARD_ADVFEATURE_SWAP_INIT              | \
                                                               SMARTCARD_ADVFEATURE_RXOVERRUNDISABLE_INIT  | \
                                                               SMARTCARD_ADVFEATURE_DMADISABLEONERROR_INIT | \
                                                               SMARTCARD_ADVFEATURE_MSBFIRST_INIT))

/** @brief Ensure that SMARTCARD frame TX inversion setting is valid.
  * @param __TXINV__ SMARTCARD frame TX inversion setting.
  * @retval SET (__TXINV__ is valid) or RESET (__TXINV__ is invalid)
  */
#define IS_SMARTCARD_ADVFEATURE_TXINV(__TXINV__) (((__TXINV__) == SMARTCARD_ADVFEATURE_TXINV_DISABLE) || \
                                                  ((__TXINV__) == SMARTCARD_ADVFEATURE_TXINV_ENABLE))

/** @brief Ensure that SMARTCARD frame RX inversion setting is valid.
  * @param __RXINV__ SMARTCARD frame RX inversion setting.
  * @retval SET (__RXINV__ is valid) or RESET (__RXINV__ is invalid)
  */
#define IS_SMARTCARD_ADVFEATURE_RXINV(__RXINV__) (((__RXINV__) == SMARTCARD_ADVFEATURE_RXINV_DISABLE) || \
                                                  ((__RXINV__) == SMARTCARD_ADVFEATURE_RXINV_ENABLE))

/** @brief Ensure that SMARTCARD frame data inversion setting is valid.
  * @param __DATAINV__ SMARTCARD frame data inversion setting.
  * @retval SET (__DATAINV__ is valid) or RESET (__DATAINV__ is invalid)
  */
#define IS_SMARTCARD_ADVFEATURE_DATAINV(__DATAINV__) (((__DATAINV__) == SMARTCARD_ADVFEATURE_DATAINV_DISABLE) || \
                                                      ((__DATAINV__) == SMARTCARD_ADVFEATURE_DATAINV_ENABLE))

/** @brief Ensure that SMARTCARD frame RX/TX pins swap setting is valid.
  * @param __SWAP__ SMARTCARD frame RX/TX pins swap setting.
  * @retval SET (__SWAP__ is valid) or RESET (__SWAP__ is invalid)
  */
#define IS_SMARTCARD_ADVFEATURE_SWAP(__SWAP__) (((__SWAP__) == SMARTCARD_ADVFEATURE_SWAP_DISABLE) || \
                                                ((__SWAP__) == SMARTCARD_ADVFEATURE_SWAP_ENABLE))

/** @brief Ensure that SMARTCARD frame overrun setting is valid.
  * @param __OVERRUN__ SMARTCARD frame overrun setting.
  * @retval SET (__OVERRUN__ is valid) or RESET (__OVERRUN__ is invalid)
  */
#define IS_SMARTCARD_OVERRUN(__OVERRUN__) (((__OVERRUN__) == SMARTCARD_ADVFEATURE_OVERRUN_ENABLE) || \
                                           ((__OVERRUN__) == SMARTCARD_ADVFEATURE_OVERRUN_DISABLE))

/** @brief Ensure that SMARTCARD DMA enabling or disabling on error setting is valid.
  * @param __DMA__ SMARTCARD DMA enabling or disabling on error setting.
  * @retval SET (__DMA__ is valid) or RESET (__DMA__ is invalid)
  */
#define IS_SMARTCARD_ADVFEATURE_DMAONRXERROR(__DMA__) (((__DMA__) == SMARTCARD_ADVFEATURE_DMA_ENABLEONRXERROR) || \
                                                       ((__DMA__) == SMARTCARD_ADVFEATURE_DMA_DISABLEONRXERROR))

/** @brief Ensure that SMARTCARD frame MSB first setting is valid.
  * @param __MSBFIRST__ SMARTCARD frame MSB first setting.
  * @retval SET (__MSBFIRST__ is valid) or RESET (__MSBFIRST__ is invalid)
  */
#define IS_SMARTCARD_ADVFEATURE_MSBFIRST(__MSBFIRST__) (((__MSBFIRST__) == SMARTCARD_ADVFEATURE_MSBFIRST_DISABLE) || \
                                                        ((__MSBFIRST__) == SMARTCARD_ADVFEATURE_MSBFIRST_ENABLE))

/** @brief Ensure that SMARTCARD request parameter is valid.
  * @param __PARAM__ SMARTCARD request parameter.
  * @retval SET (__PARAM__ is valid) or RESET (__PARAM__ is invalid)
  */
#define IS_SMARTCARD_REQUEST_PARAMETER(__PARAM__) (((__PARAM__) == SMARTCARD_RXDATA_FLUSH_REQUEST) || \
                                                   ((__PARAM__) == SMARTCARD_TXDATA_FLUSH_REQUEST))

/**
  * @}
  */

/* Include SMARTCARD HAL Extended module */
#include "stm32l4xx_hal_smartcard_ex.h"

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SMARTCARD_Exported_Functions
  * @{
  */

/* Initialization and de-initialization functions  ****************************/
/** @addtogroup SMARTCARD_Exported_Functions_Group1
  * @{
  */

HAL_StatusTypeDef HAL_SMARTCARD_Init(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARD_DeInit(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_MspInit(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_MspDeInit(SMARTCARD_HandleTypeDef *hsmartcard);

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
/* Callbacks Register/UnRegister functions  ***********************************/
HAL_StatusTypeDef HAL_SMARTCARD_RegisterCallback(SMARTCARD_HandleTypeDef *hsmartcard,
                                                 HAL_SMARTCARD_CallbackIDTypeDef CallbackID, pSMARTCARD_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_SMARTCARD_UnRegisterCallback(SMARTCARD_HandleTypeDef *hsmartcard,
                                                   HAL_SMARTCARD_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */

/**
  * @}
  */

/* IO operation functions *****************************************************/
/** @addtogroup SMARTCARD_Exported_Functions_Group2
  * @{
  */

HAL_StatusTypeDef HAL_SMARTCARD_Transmit(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size,
                                         uint32_t Timeout);
HAL_StatusTypeDef HAL_SMARTCARD_Receive(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size,
                                        uint32_t Timeout);
HAL_StatusTypeDef HAL_SMARTCARD_Transmit_IT(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SMARTCARD_Receive_IT(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SMARTCARD_Transmit_DMA(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SMARTCARD_Receive_DMA(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size);
/* Transfer Abort functions */
HAL_StatusTypeDef HAL_SMARTCARD_Abort(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARD_AbortTransmit(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARD_AbortReceive(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARD_Abort_IT(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARD_AbortTransmit_IT(SMARTCARD_HandleTypeDef *hsmartcard);
HAL_StatusTypeDef HAL_SMARTCARD_AbortReceive_IT(SMARTCARD_HandleTypeDef *hsmartcard);

void HAL_SMARTCARD_IRQHandler(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_TxCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_RxCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_ErrorCallback(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_AbortCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_AbortTransmitCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard);
void HAL_SMARTCARD_AbortReceiveCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard);

/**
  * @}
  */

/* Peripheral State and Error functions ***************************************/
/** @addtogroup SMARTCARD_Exported_Functions_Group4
  * @{
  */

HAL_SMARTCARD_StateTypeDef HAL_SMARTCARD_GetState(SMARTCARD_HandleTypeDef *hsmartcard);
uint32_t                   HAL_SMARTCARD_GetError(SMARTCARD_HandleTypeDef *hsmartcard);

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

#endif /* STM32L4xx_HAL_SMARTCARD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
