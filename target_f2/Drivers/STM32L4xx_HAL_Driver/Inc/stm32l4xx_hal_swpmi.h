/**
  ******************************************************************************
  * @file    stm32l4xx_hal_swpmi.h
  * @author  MCD Application Team
  * @brief   Header file of SWPMI HAL module.
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
#ifndef STM32L4xx_HAL_SWPMI_H
#define STM32L4xx_HAL_SWPMI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

#if defined(SWPMI1)

/** @addtogroup SWPMI
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup SWPMI_Exported_Types SWPMI Exported Types
  * @{
  */

/**
  * @brief SWPMI Init Structure definition
  */
typedef struct
{
  uint32_t VoltageClass;             /*!< Specifies the SWP Voltage Class.
                                          This parameter can be a value of @ref SWPMI_Voltage_Class */

  uint32_t BitRate;                  /*!< Specifies the SWPMI Bitrate.
                                          This parameter must be a number between 0 and 63U.
                                          The Bitrate is computed using the following formula:
                                          SWPMI_freq = SWPMI_clk / (((BitRate) + 1)  * 4)
                                          */

  uint32_t TxBufferingMode;          /*!< Specifies the transmission buffering mode.
                                          This parameter can be a value of @ref SWPMI_Tx_Buffering_Mode */

  uint32_t RxBufferingMode;          /*!< Specifies the reception buffering mode.
                                          This parameter can be a value of @ref SWPMI_Rx_Buffering_Mode */

}SWPMI_InitTypeDef;


/**
  * @brief HAL SWPMI State structures definition
  */
typedef enum
{
  HAL_SWPMI_STATE_RESET             = 0x00,    /*!< Peripheral Reset state                             */
  HAL_SWPMI_STATE_READY             = 0x01,    /*!< Peripheral Initialized and ready for use           */
  HAL_SWPMI_STATE_BUSY              = 0x02,    /*!< an internal process is ongoing                     */
  HAL_SWPMI_STATE_BUSY_TX           = 0x12,    /*!< Data Transmission process is ongoing               */
  HAL_SWPMI_STATE_BUSY_RX           = 0x22,    /*!< Data Reception process is ongoing                  */
  HAL_SWPMI_STATE_BUSY_TX_RX        = 0x32,    /*!< Data Transmission and Reception process is ongoing */
  HAL_SWPMI_STATE_TIMEOUT           = 0x03,    /*!< Timeout state                                      */
  HAL_SWPMI_STATE_ERROR             = 0x04     /*!< Error                                              */
}HAL_SWPMI_StateTypeDef;

/**
  * @brief  SWPMI handle Structure definition
  */
#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
typedef struct __SWPMI_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_SWPMI_REGISTER_CALLBACKS */
{
  SWPMI_TypeDef                  *Instance;     /*!< SWPMI registers base address         */

  SWPMI_InitTypeDef              Init;          /*!< SWPMI communication parameters       */

  uint32_t                       *pTxBuffPtr;   /*!< Pointer to SWPMI Tx transfer Buffer  */

  uint32_t                       TxXferSize;    /*!< SWPMI Tx Transfer size               */

  uint32_t                       TxXferCount;   /*!< SWPMI Tx Transfer Counter            */

  uint32_t                       *pRxBuffPtr;   /*!< Pointer to SWPMI Rx transfer Buffer  */

  uint32_t                       RxXferSize;    /*!< SWPMI Rx Transfer size               */

  uint32_t                       RxXferCount;   /*!< SWPMI Rx Transfer Counter            */

  DMA_HandleTypeDef              *hdmatx;       /*!< SWPMI Tx DMA Handle parameters       */

  DMA_HandleTypeDef              *hdmarx;       /*!< SWPMI Rx DMA Handle parameters       */

  HAL_LockTypeDef                Lock;          /*!< SWPMI object                         */

  __IO HAL_SWPMI_StateTypeDef    State;         /*!< SWPMI communication state            */

  __IO uint32_t                  ErrorCode;     /*!< SWPMI Error code                     */

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
  void (*RxCpltCallback)     (struct __SWPMI_HandleTypeDef *hswpmi); /*!< SWPMI receive complete callback */
  void (*RxHalfCpltCallback) (struct __SWPMI_HandleTypeDef *hswpmi); /*!< SWPMI receive half complete callback */
  void (*TxCpltCallback)     (struct __SWPMI_HandleTypeDef *hswpmi); /*!< SWPMI transmit complete callback */
  void (*TxHalfCpltCallback) (struct __SWPMI_HandleTypeDef *hswpmi); /*!< SWPMI transmit half complete callback */
  void (*ErrorCallback)      (struct __SWPMI_HandleTypeDef *hswpmi); /*!< SWPMI error callback */
  void (*MspInitCallback)    (struct __SWPMI_HandleTypeDef *hswpmi); /*!< SWPMI MSP init callback */
  void (*MspDeInitCallback)  (struct __SWPMI_HandleTypeDef *hswpmi); /*!< SWPMI MSP de-init callback */
#endif

}SWPMI_HandleTypeDef;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
/**
  * @brief  SWPMI callback ID enumeration definition
  */
typedef enum
{
  HAL_SWPMI_RX_COMPLETE_CB_ID     = 0x00U, /*!< SWPMI receive complete callback ID */
  HAL_SWPMI_RX_HALFCOMPLETE_CB_ID = 0x01U, /*!< SWPMI receive half complete callback ID */
  HAL_SWPMI_TX_COMPLETE_CB_ID     = 0x02U, /*!< SWPMI transmit complete callback ID */
  HAL_SWPMI_TX_HALFCOMPLETE_CB_ID = 0x03U, /*!< SWPMI transmit half complete callback ID */
  HAL_SWPMI_ERROR_CB_ID           = 0x04U, /*!< SWPMI error callback ID */
  HAL_SWPMI_MSPINIT_CB_ID         = 0x05U, /*!< SWPMI MSP init callback ID */
  HAL_SWPMI_MSPDEINIT_CB_ID       = 0x06U  /*!< SWPMI MSP de-init callback ID */
}HAL_SWPMI_CallbackIDTypeDef;

/**
  * @brief  SWPMI callback pointer definition
  */
typedef void (*pSWPMI_CallbackTypeDef)(SWPMI_HandleTypeDef *hswpmi);
#endif

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup SWPMI_Exported_Constants SWPMI Exported Constants
  * @{
  */

/**
  * @defgroup SWPMI_Error_Code SWPMI Error Code Bitmap
  * @{
  */
#define HAL_SWPMI_ERROR_NONE                  ((uint32_t)0x00000000) /*!< No error              */
#define HAL_SWPMI_ERROR_CRC                   ((uint32_t)0x00000004) /*!< frame error           */
#define HAL_SWPMI_ERROR_OVR                   ((uint32_t)0x00000008) /*!< Overrun error         */
#define HAL_SWPMI_ERROR_UDR                   ((uint32_t)0x0000000C) /*!< Underrun error        */
#define HAL_SWPMI_ERROR_DMA                   ((uint32_t)0x00000010) /*!< DMA transfer error    */
#define HAL_SWPMI_ERROR_TIMEOUT               ((uint32_t)0x00000020) /*!< Transfer timeout      */
#define HAL_SWPMI_ERROR_TXBEF_TIMEOUT         ((uint32_t)0x00000040) /*!< End Tx buffer timeout */
#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
#define HAL_SWPMI_ERROR_INVALID_CALLBACK      ((uint32_t)0x00000100) /*!< Invalid callback error */
#endif
/**
  * @}
  */

/** @defgroup SWPMI_Voltage_Class SWPMI Voltage Class
  * @{
  */
#define SWPMI_VOLTAGE_CLASS_C                ((uint32_t)0x00000000)
#define SWPMI_VOLTAGE_CLASS_B                SWPMI_OR_CLASS
/**
  * @}
  */

/** @defgroup SWPMI_Tx_Buffering_Mode SWPMI Tx Buffering Mode
  * @{
  */
#define SWPMI_TX_NO_SOFTWAREBUFFER           ((uint32_t)0x00000000)
#define SWPMI_TX_SINGLE_SOFTWAREBUFFER       ((uint32_t)0x00000000)
#define SWPMI_TX_MULTI_SOFTWAREBUFFER        SWPMI_CR_TXMODE
/**
  * @}
  */

/** @defgroup SWPMI_Rx_Buffering_Mode SWPMI Rx Buffering Mode
  * @{
  */
#define SWPMI_RX_NO_SOFTWAREBUFFER           ((uint32_t)0x00000000)
#define SWPMI_RX_SINGLE_SOFTWAREBUFFER       ((uint32_t)0x00000000)
#define SWPMI_RX_MULTI_SOFTWAREBUFFER        SWPMI_CR_RXMODE
/**
  * @}
  */

/** @defgroup SWPMI_Flags SWPMI Status Flags
  *        Elements values convention: 0xXXXXXXXX
  *           - 0xXXXXXXXX  : Flag mask in the ISR register
  * @{
  */
#define SWPMI_FLAG_RXBFF                 SWPMI_ISR_RXBFF
#define SWPMI_FLAG_TXBEF                 SWPMI_ISR_TXBEF
#define SWPMI_FLAG_RXBERF                SWPMI_ISR_RXBERF
#define SWPMI_FLAG_RXOVRF                SWPMI_ISR_RXOVRF
#define SWPMI_FLAG_TXUNRF                SWPMI_ISR_TXUNRF
#define SWPMI_FLAG_RXNE                  SWPMI_ISR_RXNE
#define SWPMI_FLAG_TXE                   SWPMI_ISR_TXE
#define SWPMI_FLAG_TCF                   SWPMI_ISR_TCF
#define SWPMI_FLAG_SRF                   SWPMI_ISR_SRF
#define SWPMI_FLAG_SUSP                  SWPMI_ISR_SUSP
#define SWPMI_FLAG_DEACTF                SWPMI_ISR_DEACTF
/**
  * @}
  */

/** @defgroup SWPMI_Interrupt_definition SWPMI Interrupts Definition
  *        Elements values convention: 0xXXXX
  *           - 0xXXXX  : Flag mask in the IER register
  * @{
  */
#define SWPMI_IT_SRIE                    SWPMI_IER_SRIE
#define SWPMI_IT_TCIE                    SWPMI_IER_TCIE
#define SWPMI_IT_TIE                     SWPMI_IER_TIE
#define SWPMI_IT_RIE                     SWPMI_IER_RIE
#define SWPMI_IT_TXUNRIE                 SWPMI_IER_TXUNRIE
#define SWPMI_IT_RXOVRIE                 SWPMI_IER_RXOVRIE
#define SWPMI_IT_RXBERIE                 SWPMI_IER_RXBERIE
#define SWPMI_IT_TXBEIE                  SWPMI_IER_TXBEIE
#define SWPMI_IT_RXBFIE                  SWPMI_IER_RXBFIE
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup SWPMI_Exported_Macros SWPMI Exported Macros
  * @{
  */

/** @brief Reset SWPMI handle state.
  * @param  __HANDLE__ specifies the SWPMI Handle.
  * @retval None
  */
#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
#define __HAL_SWPMI_RESET_HANDLE_STATE(__HANDLE__) do{                                            \
                                                     (__HANDLE__)->State = HAL_SWPMI_STATE_RESET; \
                                                     (__HANDLE__)->MspInitCallback = NULL;        \
                                                     (__HANDLE__)->MspDeInitCallback = NULL;      \
                                                   } while(0)
#else
#define __HAL_SWPMI_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_SWPMI_STATE_RESET)
#endif

/**
  * @brief  Enable the SWPMI peripheral.
  * @param  __HANDLE__ SWPMI handle
  * @retval None
  */
#define __HAL_SWPMI_ENABLE(__HANDLE__)   SET_BIT((__HANDLE__)->Instance->CR, SWPMI_CR_SWPACT)

/**
  * @brief  Disable the SWPMI peripheral.
  * @param  __HANDLE__ SWPMI handle
  * @retval None
  */
#define __HAL_SWPMI_DISABLE(__HANDLE__)  CLEAR_BIT((__HANDLE__)->Instance->CR, SWPMI_CR_SWPACT)

/** @brief  Check whether the specified SWPMI flag is set or not.
  * @param  __HANDLE__ specifies the SWPMI Handle.
  * @param  __FLAG__: specifies the flag to check.
  *        This parameter can be one of the following values:
  *            @arg SWPMI_FLAG_RXBFF  Receive buffer full flag.
  *            @arg SWPMI_FLAG_TXBEF  Transmit buffer empty flag.
  *            @arg SWPMI_FLAG_RXBERF  Receive CRC error flag.
  *            @arg SWPMI_FLAG_RXOVRF  Receive overrun error flag.
  *            @arg SWPMI_FLAG_TXUNRF  Transmit underrun error flag.
  *            @arg SWPMI_FLAG_RXNE  Receive data register not empty.
  *            @arg SWPMI_FLAG_TXE  Transmit data register empty.
  *            @arg SWPMI_FLAG_TCF  Transfer complete flag.
  *            @arg SWPMI_FLAG_SRF  Slave resume flag.
  *            @arg SWPMI_FLAG_SUSP  SUSPEND flag.
  *            @arg SWPMI_FLAG_DEACTF  DEACTIVATED flag.
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_SWPMI_GET_FLAG(__HANDLE__, __FLAG__) (READ_BIT((__HANDLE__)->Instance->ISR, (__FLAG__)) == (__FLAG__))

/** @brief  Clear the specified SWPMI ISR flag.
  * @param  __HANDLE__ specifies the SWPMI Handle.
  * @param  __FLAG__: specifies the flag to clear.
  *        This parameter can be one of the following values:
  *            @arg SWPMI_FLAG_RXBFF  Receive buffer full flag.
  *            @arg SWPMI_FLAG_TXBEF  Transmit buffer empty flag.
  *            @arg SWPMI_FLAG_RXBERF  Receive CRC error flag.
  *            @arg SWPMI_FLAG_RXOVRF  Receive overrun error flag.
  *            @arg SWPMI_FLAG_TXUNRF  Transmit underrun error flag.
  *            @arg SWPMI_FLAG_TCF  Transfer complete flag.
  *            @arg SWPMI_FLAG_SRF  Slave resume flag.
  * @retval None
  */
#define __HAL_SWPMI_CLEAR_FLAG(__HANDLE__, __FLAG__) WRITE_REG((__HANDLE__)->Instance->ICR, (__FLAG__))

/** @brief  Enable the specified SWPMI interrupt.
  * @param  __HANDLE__ specifies the SWPMI Handle.
  * @param  __INTERRUPT__ specifies the SWPMI interrupt source to enable.
  *          This parameter can be one of the following values:
  *            @arg SWPMI_IT_SRIE  Slave resume interrupt.
  *            @arg SWPMI_IT_TCIE  Transmit complete interrupt.
  *            @arg SWPMI_IT_TIE   Transmit interrupt.
  *            @arg SWPMI_IT_RIE   Receive interrupt.
  *            @arg SWPMI_IT_TXUNRIE  Transmit underrun error interrupt.
  *            @arg SWPMI_IT_RXOVRIE  Receive overrun error interrupt.
  *            @arg SWPMI_IT_RXBEIE  Receive CRC error interrupt.
  *            @arg SWPMI_IT_TXBEIE   Transmit buffer empty interrupt.
  *            @arg SWPMI_IT_RXBFIE   Receive buffer full interrupt.
  * @retval None
  */
#define __HAL_SWPMI_ENABLE_IT(__HANDLE__, __INTERRUPT__)   SET_BIT((__HANDLE__)->Instance->IER, (__INTERRUPT__))

/** @brief  Disable the specified SWPMI interrupt.
  * @param  __HANDLE__ specifies the SWPMI Handle.
  * @param  __INTERRUPT__ specifies the SWPMI interrupt source to disable.
  *          This parameter can be one of the following values:
  *            @arg SWPMI_IT_SRIE  Slave resume interrupt.
  *            @arg SWPMI_IT_TCIE  Transmit complete interrupt.
  *            @arg SWPMI_IT_TIE   Transmit interrupt.
  *            @arg SWPMI_IT_RIE   Receive interrupt.
  *            @arg SWPMI_IT_TXUNRIE  Transmit underrun error interrupt.
  *            @arg SWPMI_IT_RXOVRIE  Receive overrun error interrupt.
  *            @arg SWPMI_IT_RXBEIE  Receive CRC error interrupt.
  *            @arg SWPMI_IT_TXBEIE   Transmit buffer empty interrupt.
  *            @arg SWPMI_IT_RXBFIE   Receive buffer full interrupt.
  * @retval None
  */
#define __HAL_SWPMI_DISABLE_IT(__HANDLE__, __INTERRUPT__)  CLEAR_BIT((__HANDLE__)->Instance->IER, (__INTERRUPT__))

/** @brief  Check whether the specified SWPMI interrupt has occurred or not.
  * @param  __HANDLE__ specifies the SWPMI Handle.
  * @param  __IT__ specifies the SWPMI interrupt to check.
  *          This parameter can be one of the following values:
  *            @arg SWPMI_IT_SRIE  Slave resume interrupt.
  *            @arg SWPMI_IT_TCIE  Transmit complete interrupt.
  *            @arg SWPMI_IT_TIE   Transmit interrupt.
  *            @arg SWPMI_IT_RIE   Receive interrupt.
  *            @arg SWPMI_IT_TXUNRIE  Transmit underrun error interrupt.
  *            @arg SWPMI_IT_RXOVRIE  Receive overrun error interrupt.
  *            @arg SWPMI_IT_RXBERIE  Receive CRC error interrupt.
  *            @arg SWPMI_IT_TXBEIE   Transmit buffer empty interrupt.
  *            @arg SWPMI_IT_RXBFIE   Receive buffer full interrupt.
  * @retval The new state of __IT__ (TRUE or FALSE).
  */
#define __HAL_SWPMI_GET_IT(__HANDLE__, __IT__)  (READ_BIT((__HANDLE__)->Instance->ISR,(__IT__)) == (__IT__))

/** @brief  Check whether the specified SWPMI interrupt source is enabled or not.
  * @param  __HANDLE__ specifies the SWPMI Handle.
  * @param  __IT__ specifies the SWPMI interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg SWPMI_IT_SRIE  Slave resume interrupt.
  *            @arg SWPMI_IT_TCIE  Transmit complete interrupt.
  *            @arg SWPMI_IT_TIE   Transmit interrupt.
  *            @arg SWPMI_IT_RIE   Receive interrupt.
  *            @arg SWPMI_IT_TXUNRIE  Transmit underrun error interrupt.
  *            @arg SWPMI_IT_RXOVRIE  Receive overrun error interrupt.
  *            @arg SWPMI_IT_RXBERIE  Receive CRC error interrupt.
  *            @arg SWPMI_IT_TXBEIE   Transmit buffer empty interrupt.
  *            @arg SWPMI_IT_RXBFIE   Receive buffer full interrupt.
  * @retval The new state of __IT__ (TRUE or FALSE).
  */
#define __HAL_SWPMI_GET_IT_SOURCE(__HANDLE__, __IT__) ((READ_BIT((__HANDLE__)->Instance->IER, (__IT__)) == (__IT__)) ? SET : RESET)

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup SWPMI_Exported_Functions SWPMI Exported Functions
  * @{
  */
/* Initialization/de-initialization functions  ********************************/
HAL_StatusTypeDef HAL_SWPMI_Init(SWPMI_HandleTypeDef *hswpmi);
HAL_StatusTypeDef HAL_SWPMI_DeInit(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_MspInit(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_MspDeInit(SWPMI_HandleTypeDef *hswpmi);

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
/* SWPMI callbacks register/unregister functions ********************************/
HAL_StatusTypeDef HAL_SWPMI_RegisterCallback(SWPMI_HandleTypeDef        *hswpmi,
                                             HAL_SWPMI_CallbackIDTypeDef CallbackID,
                                             pSWPMI_CallbackTypeDef      pCallback);
HAL_StatusTypeDef HAL_SWPMI_UnRegisterCallback(SWPMI_HandleTypeDef        *hswpmi,
                                               HAL_SWPMI_CallbackIDTypeDef CallbackID);
#endif

/* IO operation functions *****************************************************/
HAL_StatusTypeDef HAL_SWPMI_Transmit(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SWPMI_Receive(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SWPMI_Transmit_IT(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SWPMI_Receive_IT(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SWPMI_Transmit_DMA(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SWPMI_Receive_DMA(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SWPMI_DMAStop(SWPMI_HandleTypeDef *hswpmi);
HAL_StatusTypeDef HAL_SWPMI_EnableLoopback(SWPMI_HandleTypeDef *hswpmi);
HAL_StatusTypeDef HAL_SWPMI_DisableLoopback(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_IRQHandler(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_TxCpltCallback(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_TxHalfCpltCallback(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_RxCpltCallback(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_RxHalfCpltCallback(SWPMI_HandleTypeDef *hswpmi);
void              HAL_SWPMI_ErrorCallback(SWPMI_HandleTypeDef *hswpmi);

/* Peripheral Control and State functions  ************************************/
HAL_SWPMI_StateTypeDef HAL_SWPMI_GetState(SWPMI_HandleTypeDef *hswpmi);
uint32_t               HAL_SWPMI_GetError(SWPMI_HandleTypeDef *hswpmi);

/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/** @defgroup SWPMI_Private_Types SWPMI Private Types
  * @{
  */

/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/** @defgroup SWPMI_Private_Variables SWPMI Private Variables
  * @{
  */

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup SWPMI_Private_Constants SWPMI Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup SWPMI_Private_Macros SWPMI Private Macros
  * @{
  */


#define IS_SWPMI_VOLTAGE_CLASS(__CLASS__)    (((__CLASS__) == SWPMI_VOLTAGE_CLASS_C) || \
                                              ((__CLASS__) == SWPMI_VOLTAGE_CLASS_B))

#define IS_SWPMI_BITRATE_VALUE(__VALUE__)    (((__VALUE__) <= 63U))


#define IS_SWPMI_TX_BUFFERING_MODE(__MODE__) (((__MODE__) == SWPMI_TX_NO_SOFTWAREBUFFER) || \
                                              ((__MODE__) == SWPMI_TX_MULTI_SOFTWAREBUFFER))


#define IS_SWPMI_RX_BUFFERING_MODE(__MODE__) (((__MODE__) == SWPMI_RX_NO_SOFTWAREBUFFER) || \
                                              ((__MODE__) == SWPMI_RX_MULTI_SOFTWAREBUFFER))

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

#endif /* STM32L4xx_HAL_SWPMI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
