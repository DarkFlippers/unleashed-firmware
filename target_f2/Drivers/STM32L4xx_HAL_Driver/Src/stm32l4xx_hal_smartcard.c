/**
  ******************************************************************************
  * @file    stm32l4xx_hal_smartcard.c
  * @author  MCD Application Team
  * @brief   SMARTCARD HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the SMARTCARD peripheral:
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *           + Peripheral State and Error functions
  *
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
  [..]
    The SMARTCARD HAL driver can be used as follows:

    (#) Declare a SMARTCARD_HandleTypeDef handle structure (eg. SMARTCARD_HandleTypeDef hsmartcard).
    (#) Associate a USART to the SMARTCARD handle hsmartcard.
    (#) Initialize the SMARTCARD low level resources by implementing the HAL_SMARTCARD_MspInit() API:
        (++) Enable the USARTx interface clock.
        (++) USART pins configuration:
             (+++) Enable the clock for the USART GPIOs.
             (+++) Configure the USART pins (TX as alternate function pull-up, RX as alternate function Input).
        (++) NVIC configuration if you need to use interrupt process (HAL_SMARTCARD_Transmit_IT()
             and HAL_SMARTCARD_Receive_IT() APIs):
             (+++) Configure the USARTx interrupt priority.
             (+++) Enable the NVIC USART IRQ handle.
        (++) DMA Configuration if you need to use DMA process (HAL_SMARTCARD_Transmit_DMA()
             and HAL_SMARTCARD_Receive_DMA() APIs):
             (+++) Declare a DMA handle structure for the Tx/Rx channel.
             (+++) Enable the DMAx interface clock.
             (+++) Configure the declared DMA handle structure with the required Tx/Rx parameters.
             (+++) Configure the DMA Tx/Rx channel.
             (+++) Associate the initialized DMA handle to the SMARTCARD DMA Tx/Rx handle.
             (+++) Configure the priority and enable the NVIC for the transfer complete interrupt on the DMA Tx/Rx channel.

    (#) Program the Baud Rate, Parity, Mode(Receiver/Transmitter), clock enabling/disabling and accordingly,
        the clock parameters (parity, phase, last bit), prescaler value, guard time and NACK on transmission
        error enabling or disabling in the hsmartcard handle Init structure.

    (#) If required, program SMARTCARD advanced features (TX/RX pins swap, TimeOut, auto-retry counter,...)
        in the hsmartcard handle AdvancedInit structure.

    (#) Initialize the SMARTCARD registers by calling the HAL_SMARTCARD_Init() API:
        (++) This API configures also the low level Hardware GPIO, CLOCK, CORTEX...etc)
             by calling the customized HAL_SMARTCARD_MspInit() API.
        [..]
        (@) The specific SMARTCARD interrupts (Transmission complete interrupt,
             RXNE interrupt and Error Interrupts) will be managed using the macros
             __HAL_SMARTCARD_ENABLE_IT() and __HAL_SMARTCARD_DISABLE_IT() inside the transmit and receive process.

    [..]
    [..] Three operation modes are available within this driver :

     *** Polling mode IO operation ***
     =================================
     [..]
       (+) Send an amount of data in blocking mode using HAL_SMARTCARD_Transmit()
       (+) Receive an amount of data in blocking mode using HAL_SMARTCARD_Receive()

     *** Interrupt mode IO operation ***
     ===================================
     [..]
       (+) Send an amount of data in non-blocking mode using HAL_SMARTCARD_Transmit_IT()
       (+) At transmission end of transfer HAL_SMARTCARD_TxCpltCallback() is executed and user can
            add his own code by customization of function pointer HAL_SMARTCARD_TxCpltCallback()
       (+) Receive an amount of data in non-blocking mode using HAL_SMARTCARD_Receive_IT()
       (+) At reception end of transfer HAL_SMARTCARD_RxCpltCallback() is executed and user can
            add his own code by customization of function pointer HAL_SMARTCARD_RxCpltCallback()
       (+) In case of transfer Error, HAL_SMARTCARD_ErrorCallback() function is executed and user can
            add his own code by customization of function pointer HAL_SMARTCARD_ErrorCallback()

     *** DMA mode IO operation ***
     ==============================
     [..]
       (+) Send an amount of data in non-blocking mode (DMA) using HAL_SMARTCARD_Transmit_DMA()
       (+) At transmission end of transfer HAL_SMARTCARD_TxCpltCallback() is executed and user can
            add his own code by customization of function pointer HAL_SMARTCARD_TxCpltCallback()
       (+) Receive an amount of data in non-blocking mode (DMA) using HAL_SMARTCARD_Receive_DMA()
       (+) At reception end of transfer HAL_SMARTCARD_RxCpltCallback() is executed and user can
            add his own code by customization of function pointer HAL_SMARTCARD_RxCpltCallback()
       (+) In case of transfer Error, HAL_SMARTCARD_ErrorCallback() function is executed and user can
            add his own code by customization of function pointer HAL_SMARTCARD_ErrorCallback()

     *** SMARTCARD HAL driver macros list ***
     ========================================
     [..]
       Below the list of most used macros in SMARTCARD HAL driver.

       (+) __HAL_SMARTCARD_GET_FLAG : Check whether or not the specified SMARTCARD flag is set
       (+) __HAL_SMARTCARD_CLEAR_FLAG : Clear the specified SMARTCARD pending flag
       (+) __HAL_SMARTCARD_ENABLE_IT: Enable the specified SMARTCARD interrupt
       (+) __HAL_SMARTCARD_DISABLE_IT: Disable the specified SMARTCARD interrupt
       (+) __HAL_SMARTCARD_GET_IT_SOURCE: Check whether or not the specified SMARTCARD interrupt is enabled

     [..]
       (@) You can refer to the SMARTCARD HAL driver header file for more useful macros

    ##### Callback registration #####
    ==================================

    [..]
    The compilation define USE_HAL_SMARTCARD_REGISTER_CALLBACKS when set to 1
    allows the user to configure dynamically the driver callbacks.

    [..]
    Use Function @ref HAL_SMARTCARD_RegisterCallback() to register a user callback.
    Function @ref HAL_SMARTCARD_RegisterCallback() allows to register following callbacks:
    (+) TxCpltCallback            : Tx Complete Callback.
    (+) RxCpltCallback            : Rx Complete Callback.
    (+) ErrorCallback             : Error Callback.
    (+) AbortCpltCallback         : Abort Complete Callback.
    (+) AbortTransmitCpltCallback : Abort Transmit Complete Callback.
    (+) AbortReceiveCpltCallback  : Abort Receive Complete Callback.
    (+) RxFifoFullCallback        : Rx Fifo Full Callback.
    (+) TxFifoEmptyCallback       : Tx Fifo Empty Callback.
    (+) MspInitCallback           : SMARTCARD MspInit.
    (+) MspDeInitCallback         : SMARTCARD MspDeInit.
    This function takes as parameters the HAL peripheral handle, the Callback ID
    and a pointer to the user callback function.

    [..]
    Use function @ref HAL_SMARTCARD_UnRegisterCallback() to reset a callback to the default
    weak (surcharged) function.
    @ref HAL_SMARTCARD_UnRegisterCallback() takes as parameters the HAL peripheral handle,
    and the Callback ID.
    This function allows to reset following callbacks:
    (+) TxCpltCallback            : Tx Complete Callback.
    (+) RxCpltCallback            : Rx Complete Callback.
    (+) ErrorCallback             : Error Callback.
    (+) AbortCpltCallback         : Abort Complete Callback.
    (+) AbortTransmitCpltCallback : Abort Transmit Complete Callback.
    (+) AbortReceiveCpltCallback  : Abort Receive Complete Callback.
    (+) RxFifoFullCallback        : Rx Fifo Full Callback.
    (+) TxFifoEmptyCallback       : Tx Fifo Empty Callback.
    (+) MspInitCallback           : SMARTCARD MspInit.
    (+) MspDeInitCallback         : SMARTCARD MspDeInit.

    [..]
    By default, after the @ref HAL_SMARTCARD_Init() and when the state is HAL_SMARTCARD_STATE_RESET
    all callbacks are set to the corresponding weak (surcharged) functions:
    examples @ref HAL_SMARTCARD_TxCpltCallback(), @ref HAL_SMARTCARD_RxCpltCallback().
    Exception done for MspInit and MspDeInit functions that are respectively
    reset to the legacy weak (surcharged) functions in the @ref HAL_SMARTCARD_Init()
    and @ref HAL_SMARTCARD_DeInit() only when these callbacks are null (not registered beforehand).
    If not, MspInit or MspDeInit are not null, the @ref HAL_SMARTCARD_Init() and @ref HAL_SMARTCARD_DeInit()
    keep and use the user MspInit/MspDeInit callbacks (registered beforehand).

    [..]
    Callbacks can be registered/unregistered in HAL_SMARTCARD_STATE_READY state only.
    Exception done MspInit/MspDeInit that can be registered/unregistered
    in HAL_SMARTCARD_STATE_READY or HAL_SMARTCARD_STATE_RESET state, thus registered (user)
    MspInit/DeInit callbacks can be used during the Init/DeInit.
    In that case first register the MspInit/MspDeInit user callbacks
    using @ref HAL_SMARTCARD_RegisterCallback() before calling @ref HAL_SMARTCARD_DeInit()
    or @ref HAL_SMARTCARD_Init() function.

    [..]
    When The compilation define USE_HAL_SMARTCARD_REGISTER_CALLBACKS is set to 0 or
    not defined, the callback registration feature is not available
    and weak (surcharged) callbacks are used.


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

/** @defgroup SMARTCARD SMARTCARD
  * @brief HAL SMARTCARD module driver
  * @{
  */

#ifdef HAL_SMARTCARD_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup SMARTCARD_Private_Constants SMARTCARD Private Constants
  * @{
  */
#define SMARTCARD_TEACK_REACK_TIMEOUT               1000U      /*!< SMARTCARD TX or RX enable acknowledge time-out value  */

#if defined(USART_CR1_FIFOEN)
#define USART_CR1_FIELDS      ((uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS   | \
                                          USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8| \
                                          USART_CR1_FIFOEN ))                                         /*!< USART CR1 fields of parameters set by SMARTCARD_SetConfig API */
#else
#define USART_CR1_FIELDS      ((uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS   | \
                                          USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8))             /*!< USART CR1 fields of parameters set by SMARTCARD_SetConfig API */
#endif /* USART_CR1_FIFOEN */

#define USART_CR2_CLK_FIELDS  ((uint32_t)(USART_CR2_CLKEN | USART_CR2_CPOL | USART_CR2_CPHA | \
                                          USART_CR2_LBCL))                                            /*!< SMARTCARD clock-related USART CR2 fields of parameters */

#define USART_CR2_FIELDS      ((uint32_t)(USART_CR2_RTOEN | USART_CR2_CLK_FIELDS | USART_CR2_STOP))   /*!< USART CR2 fields of parameters set by SMARTCARD_SetConfig API */

#if defined(USART_CR1_FIFOEN)
#define USART_CR3_FIELDS      ((uint32_t)(USART_CR3_ONEBIT | USART_CR3_NACK | USART_CR3_SCARCNT | \
                                          USART_CR3_TXFTCFG | USART_CR3_RXFTCFG ))                    /*!< USART CR3 fields of parameters set by SMARTCARD_SetConfig API */
#else
#define USART_CR3_FIELDS      ((uint32_t)(USART_CR3_ONEBIT | USART_CR3_NACK | USART_CR3_SCARCNT))     /*!< USART CR3 fields of parameters set by SMARTCARD_SetConfig API */
#endif /* USART_CR1_FIFOEN */

#define USART_BRR_MIN    0x10U        /*!< USART BRR minimum authorized value */

#define USART_BRR_MAX    0x0000FFFFU  /*!< USART BRR maximum authorized value */
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @addtogroup SMARTCARD_Private_Functions
  * @{
  */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
void SMARTCARD_InitCallbacksToDefault(SMARTCARD_HandleTypeDef *hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */
static HAL_StatusTypeDef SMARTCARD_SetConfig(SMARTCARD_HandleTypeDef *hsmartcard);
static void SMARTCARD_AdvFeatureConfig(SMARTCARD_HandleTypeDef *hsmartcard);
static HAL_StatusTypeDef SMARTCARD_CheckIdleState(SMARTCARD_HandleTypeDef *hsmartcard);
static HAL_StatusTypeDef SMARTCARD_WaitOnFlagUntilTimeout(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t Flag,
                                                          FlagStatus Status, uint32_t Tickstart, uint32_t Timeout);
static void SMARTCARD_EndTxTransfer(SMARTCARD_HandleTypeDef *hsmartcard);
static void SMARTCARD_EndRxTransfer(SMARTCARD_HandleTypeDef *hsmartcard);
static void SMARTCARD_DMATransmitCplt(DMA_HandleTypeDef *hdma);
static void SMARTCARD_DMAReceiveCplt(DMA_HandleTypeDef *hdma);
static void SMARTCARD_DMAError(DMA_HandleTypeDef *hdma);
static void SMARTCARD_DMAAbortOnError(DMA_HandleTypeDef *hdma);
static void SMARTCARD_DMATxAbortCallback(DMA_HandleTypeDef *hdma);
static void SMARTCARD_DMARxAbortCallback(DMA_HandleTypeDef *hdma);
static void SMARTCARD_DMATxOnlyAbortCallback(DMA_HandleTypeDef *hdma);
static void SMARTCARD_DMARxOnlyAbortCallback(DMA_HandleTypeDef *hdma);
static void SMARTCARD_TxISR(SMARTCARD_HandleTypeDef *hsmartcard);
#if defined(USART_CR1_FIFOEN)
static void SMARTCARD_TxISR_FIFOEN(SMARTCARD_HandleTypeDef *hsmartcard);
#endif /* USART_CR1_FIFOEN */
static void SMARTCARD_EndTransmit_IT(SMARTCARD_HandleTypeDef *hsmartcard);
static void SMARTCARD_RxISR(SMARTCARD_HandleTypeDef *hsmartcard);
#if defined(USART_CR1_FIFOEN)
static void SMARTCARD_RxISR_FIFOEN(SMARTCARD_HandleTypeDef *hsmartcard);
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup SMARTCARD_Exported_Functions SMARTCARD Exported Functions
  * @{
  */

/** @defgroup SMARTCARD_Exported_Functions_Group1 Initialization and de-initialization functions
  * @brief    Initialization and Configuration functions
  *
@verbatim
  ==============================================================================
              ##### Initialization and Configuration functions #####
  ==============================================================================
  [..]
  This subsection provides a set of functions allowing to initialize the USARTx
  associated to the SmartCard.
  (+) These parameters can be configured:
      (++) Baud Rate
      (++) Parity: parity should be enabled, frame Length is fixed to 8 bits plus parity
      (++) Receiver/transmitter modes
      (++) Synchronous mode (and if enabled, phase, polarity and last bit parameters)
      (++) Prescaler value
      (++) Guard bit time
      (++) NACK enabling or disabling on transmission error

  (+) The following advanced features can be configured as well:
      (++) TX and/or RX pin level inversion
      (++) data logical level inversion
      (++) RX and TX pins swap
      (++) RX overrun detection disabling
      (++) DMA disabling on RX error
      (++) MSB first on communication line
      (++) Time out enabling (and if activated, timeout value)
      (++) Block length
      (++) Auto-retry counter
  [..]
  The HAL_SMARTCARD_Init() API follows the USART synchronous configuration procedures
  (details for the procedures are available in reference manual).

@endverbatim

  The USART frame format is given in the following table:

    Table 1. USART frame format.
    +---------------------------------------------------------------+
    | M1M0 bits |  PCE bit  |            USART frame                |
    |-----------------------|---------------------------------------|
    |     01    |    1      |    | SB | 8 bit data | PB | STB |     |
    +---------------------------------------------------------------+


  * @{
  */

/**
  * @brief  Initialize the SMARTCARD mode according to the specified
  *         parameters in the SMARTCARD_HandleTypeDef and initialize the associated handle.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Init(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Check the SMARTCARD handle allocation */
  if (hsmartcard == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the USART associated to the SMARTCARD handle */
  assert_param(IS_SMARTCARD_INSTANCE(hsmartcard->Instance));

  if (hsmartcard->gState == HAL_SMARTCARD_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hsmartcard->Lock = HAL_UNLOCKED;

#if USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1
    SMARTCARD_InitCallbacksToDefault(hsmartcard);

    if (hsmartcard->MspInitCallback == NULL)
    {
      hsmartcard->MspInitCallback = HAL_SMARTCARD_MspInit;
    }

    /* Init the low level hardware */
    hsmartcard->MspInitCallback(hsmartcard);
#else
    /* Init the low level hardware : GPIO, CLOCK */
    HAL_SMARTCARD_MspInit(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */
  }

  hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

  /* Disable the Peripheral to set smartcard mode */
  CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

  /* In SmartCard mode, the following bits must be kept cleared:
  - LINEN in the USART_CR2 register,
  - HDSEL and IREN  bits in the USART_CR3 register.*/
  CLEAR_BIT(hsmartcard->Instance->CR2, USART_CR2_LINEN);
  CLEAR_BIT(hsmartcard->Instance->CR3, (USART_CR3_HDSEL | USART_CR3_IREN));

  /* set the USART in SMARTCARD mode */
  SET_BIT(hsmartcard->Instance->CR3, USART_CR3_SCEN);

  /* Set the SMARTCARD Communication parameters */
  if (SMARTCARD_SetConfig(hsmartcard) == HAL_ERROR)
  {
    return HAL_ERROR;
  }

  /* Set the SMARTCARD transmission completion indication */
  SMARTCARD_TRANSMISSION_COMPLETION_SETTING(hsmartcard);

  if (hsmartcard->AdvancedInit.AdvFeatureInit != SMARTCARD_ADVFEATURE_NO_INIT)
  {
    SMARTCARD_AdvFeatureConfig(hsmartcard);
  }

  /* Enable the Peripheral */
  SET_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

  /* TEACK and/or REACK to check before moving hsmartcard->gState and hsmartcard->RxState to Ready */
  return (SMARTCARD_CheckIdleState(hsmartcard));
}

/**
  * @brief  DeInitialize the SMARTCARD peripheral.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_DeInit(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Check the SMARTCARD handle allocation */
  if (hsmartcard == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the USART/UART associated to the SMARTCARD handle */
  assert_param(IS_SMARTCARD_INSTANCE(hsmartcard->Instance));

  hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY;

  /* Disable the Peripheral */
  CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

  WRITE_REG(hsmartcard->Instance->CR1, 0x0U);
  WRITE_REG(hsmartcard->Instance->CR2, 0x0U);
  WRITE_REG(hsmartcard->Instance->CR3, 0x0U);
  WRITE_REG(hsmartcard->Instance->RTOR, 0x0U);
  WRITE_REG(hsmartcard->Instance->GTPR, 0x0U);

  /* DeInit the low level hardware */
#if USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1
  if (hsmartcard->MspDeInitCallback == NULL)
  {
    hsmartcard->MspDeInitCallback = HAL_SMARTCARD_MspDeInit;
  }
  /* DeInit the low level hardware */
  hsmartcard->MspDeInitCallback(hsmartcard);
#else
  HAL_SMARTCARD_MspDeInit(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */

  hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
  hsmartcard->gState    = HAL_SMARTCARD_STATE_RESET;
  hsmartcard->RxState   = HAL_SMARTCARD_STATE_RESET;

  /* Process Unlock */
  __HAL_UNLOCK(hsmartcard);

  return HAL_OK;
}

/**
  * @brief  Initialize the SMARTCARD MSP.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_MspInit(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_MspInit can be implemented in the user file
   */
}

/**
  * @brief  DeInitialize the SMARTCARD MSP.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_MspDeInit(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_MspDeInit can be implemented in the user file
   */
}

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User SMARTCARD Callback
  *         To be used instead of the weak predefined callback
  * @param  hsmartcard smartcard handle
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_SMARTCARD_TX_COMPLETE_CB_ID Tx Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_RX_COMPLETE_CB_ID Rx Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_ERROR_CB_ID Error Callback ID
  *           @arg @ref HAL_SMARTCARD_ABORT_COMPLETE_CB_ID Abort Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_ABORT_TRANSMIT_COMPLETE_CB_ID Abort Transmit Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_ABORT_RECEIVE_COMPLETE_CB_ID Abort Receive Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_RX_FIFO_FULL_CB_ID Rx Fifo Full Callback ID
  *           @arg @ref HAL_SMARTCARD_TX_FIFO_EMPTY_CB_ID Tx Fifo Empty Callback ID
  *           @arg @ref HAL_SMARTCARD_MSPINIT_CB_ID MspInit Callback ID
  *           @arg @ref HAL_SMARTCARD_MSPDEINIT_CB_ID MspDeInit Callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_RegisterCallback(SMARTCARD_HandleTypeDef *hsmartcard,
                                                 HAL_SMARTCARD_CallbackIDTypeDef CallbackID, pSMARTCARD_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }
  /* Process locked */
  __HAL_LOCK(hsmartcard);

  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    switch (CallbackID)
    {

      case HAL_SMARTCARD_TX_COMPLETE_CB_ID :
        hsmartcard->TxCpltCallback = pCallback;
        break;

      case HAL_SMARTCARD_RX_COMPLETE_CB_ID :
        hsmartcard->RxCpltCallback = pCallback;
        break;

      case HAL_SMARTCARD_ERROR_CB_ID :
        hsmartcard->ErrorCallback = pCallback;
        break;

      case HAL_SMARTCARD_ABORT_COMPLETE_CB_ID :
        hsmartcard->AbortCpltCallback = pCallback;
        break;

      case HAL_SMARTCARD_ABORT_TRANSMIT_COMPLETE_CB_ID :
        hsmartcard->AbortTransmitCpltCallback = pCallback;
        break;

      case HAL_SMARTCARD_ABORT_RECEIVE_COMPLETE_CB_ID :
        hsmartcard->AbortReceiveCpltCallback = pCallback;
        break;

#if defined(USART_CR1_FIFOEN)
      case HAL_SMARTCARD_RX_FIFO_FULL_CB_ID :
        hsmartcard->RxFifoFullCallback = pCallback;
        break;

      case HAL_SMARTCARD_TX_FIFO_EMPTY_CB_ID :
        hsmartcard->TxFifoEmptyCallback = pCallback;
        break;
#endif /* USART_CR1_FIFOEN */

      case HAL_SMARTCARD_MSPINIT_CB_ID :
        hsmartcard->MspInitCallback = pCallback;
        break;

      case HAL_SMARTCARD_MSPDEINIT_CB_ID :
        hsmartcard->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (hsmartcard->gState == HAL_SMARTCARD_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_SMARTCARD_MSPINIT_CB_ID :
        hsmartcard->MspInitCallback = pCallback;
        break;

      case HAL_SMARTCARD_MSPDEINIT_CB_ID :
        hsmartcard->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsmartcard);

  return status;
}

/**
  * @brief  Unregister an SMARTCARD callback
  *         SMARTCARD callback is redirected to the weak predefined callback
  * @param  hsmartcard smartcard handle
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_SMARTCARD_TX_COMPLETE_CB_ID Tx Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_RX_COMPLETE_CB_ID Rx Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_ERROR_CB_ID Error Callback ID
  *           @arg @ref HAL_SMARTCARD_ABORT_COMPLETE_CB_ID Abort Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_ABORT_TRANSMIT_COMPLETE_CB_ID Abort Transmit Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_ABORT_RECEIVE_COMPLETE_CB_ID Abort Receive Complete Callback ID
  *           @arg @ref HAL_SMARTCARD_RX_FIFO_FULL_CB_ID Rx Fifo Full Callback ID
  *           @arg @ref HAL_SMARTCARD_TX_FIFO_EMPTY_CB_ID Tx Fifo Empty Callback ID
  *           @arg @ref HAL_SMARTCARD_MSPINIT_CB_ID MspInit Callback ID
  *           @arg @ref HAL_SMARTCARD_MSPDEINIT_CB_ID MspDeInit Callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_UnRegisterCallback(SMARTCARD_HandleTypeDef *hsmartcard,
                                                   HAL_SMARTCARD_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hsmartcard);

  if (HAL_SMARTCARD_STATE_READY == hsmartcard->gState)
  {
    switch (CallbackID)
    {
      case HAL_SMARTCARD_TX_COMPLETE_CB_ID :
        hsmartcard->TxCpltCallback = HAL_SMARTCARD_TxCpltCallback;                       /* Legacy weak TxCpltCallback            */
        break;

      case HAL_SMARTCARD_RX_COMPLETE_CB_ID :
        hsmartcard->RxCpltCallback = HAL_SMARTCARD_RxCpltCallback;                       /* Legacy weak RxCpltCallback            */
        break;

      case HAL_SMARTCARD_ERROR_CB_ID :
        hsmartcard->ErrorCallback = HAL_SMARTCARD_ErrorCallback;                         /* Legacy weak ErrorCallback             */
        break;

      case HAL_SMARTCARD_ABORT_COMPLETE_CB_ID :
        hsmartcard->AbortCpltCallback = HAL_SMARTCARD_AbortCpltCallback;                 /* Legacy weak AbortCpltCallback         */
        break;

      case HAL_SMARTCARD_ABORT_TRANSMIT_COMPLETE_CB_ID :
        hsmartcard->AbortTransmitCpltCallback = HAL_SMARTCARD_AbortTransmitCpltCallback; /* Legacy weak AbortTransmitCpltCallback */
        break;

      case HAL_SMARTCARD_ABORT_RECEIVE_COMPLETE_CB_ID :
        hsmartcard->AbortReceiveCpltCallback = HAL_SMARTCARD_AbortReceiveCpltCallback;   /* Legacy weak AbortReceiveCpltCallback  */
        break;

#if defined(USART_CR1_FIFOEN)
      case HAL_SMARTCARD_RX_FIFO_FULL_CB_ID :
        hsmartcard->RxFifoFullCallback = HAL_SMARTCARDEx_RxFifoFullCallback;             /* Legacy weak RxFifoFullCallback        */
        break;

      case HAL_SMARTCARD_TX_FIFO_EMPTY_CB_ID :
        hsmartcard->TxFifoEmptyCallback = HAL_SMARTCARDEx_TxFifoEmptyCallback;           /* Legacy weak TxFifoEmptyCallback       */
        break;
#endif /* USART_CR1_FIFOEN */

      case HAL_SMARTCARD_MSPINIT_CB_ID :
        hsmartcard->MspInitCallback = HAL_SMARTCARD_MspInit;                             /* Legacy weak MspInitCallback           */
        break;

      case HAL_SMARTCARD_MSPDEINIT_CB_ID :
        hsmartcard->MspDeInitCallback = HAL_SMARTCARD_MspDeInit;                         /* Legacy weak MspDeInitCallback         */
        break;

      default :
        /* Update the error code */
        hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_SMARTCARD_STATE_RESET == hsmartcard->gState)
  {
    switch (CallbackID)
    {
      case HAL_SMARTCARD_MSPINIT_CB_ID :
        hsmartcard->MspInitCallback = HAL_SMARTCARD_MspInit;
        break;

      case HAL_SMARTCARD_MSPDEINIT_CB_ID :
        hsmartcard->MspDeInitCallback = HAL_SMARTCARD_MspDeInit;
        break;

      default :
        /* Update the error code */
        hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsmartcard);

  return status;
}
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup SMARTCARD_Exported_Functions_Group2 IO operation functions
  * @brief    SMARTCARD Transmit and Receive functions
  *
@verbatim
  ==============================================================================
                         ##### IO operation functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to manage the SMARTCARD data transfers.

  [..]
    Smartcard is a single wire half duplex communication protocol.
    The Smartcard interface is designed to support asynchronous protocol Smartcards as
    defined in the ISO 7816-3 standard. The USART should be configured as:
    (+) 8 bits plus parity: where M=1 and PCE=1 in the USART_CR1 register
    (+) 1.5 stop bits when transmitting and receiving: where STOP=11 in the USART_CR2 register.

  [..]
    (+) There are two modes of transfer:
        (++) Blocking mode: The communication is performed in polling mode.
             The HAL status of all data processing is returned by the same function
             after finishing transfer.
        (++) Non-Blocking mode: The communication is performed using Interrupts
             or DMA, the relevant API's return the HAL status.
             The end of the data processing will be indicated through the
             dedicated SMARTCARD IRQ when using Interrupt mode or the DMA IRQ when
             using DMA mode.
        (++) The HAL_SMARTCARD_TxCpltCallback(), HAL_SMARTCARD_RxCpltCallback() user callbacks
             will be executed respectively at the end of the Transmit or Receive process
             The HAL_SMARTCARD_ErrorCallback() user callback will be executed when a communication
             error is detected.

    (+) Blocking mode APIs are :
        (++) HAL_SMARTCARD_Transmit()
        (++) HAL_SMARTCARD_Receive()

    (+) Non Blocking mode APIs with Interrupt are :
        (++) HAL_SMARTCARD_Transmit_IT()
        (++) HAL_SMARTCARD_Receive_IT()
        (++) HAL_SMARTCARD_IRQHandler()

    (+) Non Blocking mode functions with DMA are :
        (++) HAL_SMARTCARD_Transmit_DMA()
        (++) HAL_SMARTCARD_Receive_DMA()

    (+) A set of Transfer Complete Callbacks are provided in non Blocking mode:
        (++) HAL_SMARTCARD_TxCpltCallback()
        (++) HAL_SMARTCARD_RxCpltCallback()
        (++) HAL_SMARTCARD_ErrorCallback()

    (#) Non-Blocking mode transfers could be aborted using Abort API's :
        (+) HAL_SMARTCARD_Abort()
        (+) HAL_SMARTCARD_AbortTransmit()
        (+) HAL_SMARTCARD_AbortReceive()
        (+) HAL_SMARTCARD_Abort_IT()
        (+) HAL_SMARTCARD_AbortTransmit_IT()
        (+) HAL_SMARTCARD_AbortReceive_IT()

    (#) For Abort services based on interrupts (HAL_SMARTCARD_Abortxxx_IT), a set of Abort Complete Callbacks are provided:
        (+) HAL_SMARTCARD_AbortCpltCallback()
        (+) HAL_SMARTCARD_AbortTransmitCpltCallback()
        (+) HAL_SMARTCARD_AbortReceiveCpltCallback()

    (#) In Non-Blocking mode transfers, possible errors are split into 2 categories.
        Errors are handled as follows :
       (+) Error is considered as Recoverable and non blocking : Transfer could go till end, but error severity is
           to be evaluated by user : this concerns Frame Error, Parity Error or Noise Error in Interrupt mode reception .
           Received character is then retrieved and stored in Rx buffer, Error code is set to allow user to identify error type,
           and HAL_SMARTCARD_ErrorCallback() user callback is executed. Transfer is kept ongoing on SMARTCARD side.
           If user wants to abort it, Abort services should be called by user.
       (+) Error is considered as Blocking : Transfer could not be completed properly and is aborted.
           This concerns Frame Error in Interrupt mode tranmission, Overrun Error in Interrupt mode reception and all errors in DMA mode.
           Error code is set to allow user to identify error type, and HAL_SMARTCARD_ErrorCallback() user callback is executed.

@endverbatim
  * @{
  */

/**
  * @brief  Send an amount of data in blocking mode.
  * @note   When FIFO mode is enabled, writing a data in the TDR register adds one
  *         data to the TXFIFO. Write operations to the TDR register are performed
  *         when TXFNF flag is set. From hardware perspective, TXFNF flag and
  *         TXE are mapped on the same bit-field.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param  pData pointer to data buffer.
  * @param  Size amount of data to be sent.
  * @param  Timeout  Timeout duration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Transmit(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size,
                                         uint32_t Timeout)
{
  uint32_t tickstart;
  uint8_t  *ptmpdata = pData;

  /* Check that a Tx process is not already ongoing */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    if ((ptmpdata == NULL) || (Size == 0U))
    {
      return  HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY_TX;

    /* Init tickstart for timeout management */
    tickstart = HAL_GetTick();

    /* Disable the Peripheral first to update mode for TX master */
    CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

    /* Disable Rx, enable Tx */
    CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_RE);
    SET_BIT(hsmartcard->Instance->RQR, (uint16_t)SMARTCARD_RXDATA_FLUSH_REQUEST);
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_TE);

    /* Enable the Peripheral */
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

    hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
    hsmartcard->TxXferSize = Size;
    hsmartcard->TxXferCount = Size;

    while (hsmartcard->TxXferCount > 0U)
    {
      hsmartcard->TxXferCount--;
      if (SMARTCARD_WaitOnFlagUntilTimeout(hsmartcard, SMARTCARD_FLAG_TXE, RESET, tickstart, Timeout) != HAL_OK)
      {
        return HAL_TIMEOUT;
      }
      hsmartcard->Instance->TDR = (uint8_t)(*ptmpdata & 0xFFU);
      ptmpdata++;
    }
    if (SMARTCARD_WaitOnFlagUntilTimeout(hsmartcard, SMARTCARD_TRANSMISSION_COMPLETION_FLAG(hsmartcard), RESET, tickstart,
                                         Timeout) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }
    /* Re-enable Rx at end of transmission if initial mode is Rx/Tx */
    if (hsmartcard->Init.Mode == SMARTCARD_MODE_TX_RX)
    {
      /* Disable the Peripheral first to update modes */
      CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);
      SET_BIT(hsmartcard->Instance->CR1, USART_CR1_RE);
      /* Enable the Peripheral */
      SET_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);
    }

    /* At end of Tx process, restore hsmartcard->gState to Ready */
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
  * @brief  Receive an amount of data in blocking mode.
  * @note   When FIFO mode is enabled, the RXFNE flag is set as long as the RXFIFO
  *         is not empty. Read operations from the RDR register are performed when
  *         RXFNE flag is set. From hardware perspective, RXFNE flag and
  *         RXNE are mapped on the same bit-field.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param  pData pointer to data buffer.
  * @param  Size amount of data to be received.
  * @param  Timeout Timeout duration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Receive(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size,
                                        uint32_t Timeout)
{
  uint32_t tickstart;
  uint8_t  *ptmpdata = pData;

  /* Check that a Rx process is not already ongoing */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_READY)
  {
    if ((ptmpdata == NULL) || (Size == 0U))
    {
      return  HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
    hsmartcard->RxState   = HAL_SMARTCARD_STATE_BUSY_RX;

    /* Init tickstart for timeout management */
    tickstart = HAL_GetTick();

    hsmartcard->RxXferSize = Size;
    hsmartcard->RxXferCount = Size;

    /* Check the remain data to be received */
    while (hsmartcard->RxXferCount > 0U)
    {
      hsmartcard->RxXferCount--;

      if (SMARTCARD_WaitOnFlagUntilTimeout(hsmartcard, SMARTCARD_FLAG_RXNE, RESET, tickstart, Timeout) != HAL_OK)
      {
        return HAL_TIMEOUT;
      }
      *ptmpdata = (uint8_t)(hsmartcard->Instance->RDR & (uint8_t)0x00FF);
      ptmpdata++;
    }

    /* At end of Rx process, restore hsmartcard->RxState to Ready */
    hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

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
  * @brief  Send an amount of data in interrupt mode.
  * @note   When FIFO mode is disabled, USART interrupt is generated whenever
  *         USART_TDR register is empty, i.e one interrupt per data to transmit.
  * @note   When FIFO mode is enabled, USART interrupt is generated whenever
  *         TXFIFO threshold reached. In that case the interrupt rate depends on
  *         TXFIFO threshold configuration.
  * @note   This function sets the hsmartcard->TxIsr function pointer according to
  *         the FIFO mode (data transmission processing depends on FIFO mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param  pData pointer to data buffer.
  * @param  Size amount of data to be sent.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Transmit_IT(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size)
{
  /* Check that a Tx process is not already ongoing */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
    hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY_TX;

    hsmartcard->pTxBuffPtr  = pData;
    hsmartcard->TxXferSize  = Size;
    hsmartcard->TxXferCount = Size;
    hsmartcard->TxISR       = NULL;

    /* Disable the Peripheral first to update mode for TX master */
    CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

    /* Disable Rx, enable Tx */
    CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_RE);
    SET_BIT(hsmartcard->Instance->RQR, (uint16_t)SMARTCARD_RXDATA_FLUSH_REQUEST);
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_TE);

    /* Enable the Peripheral */
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

    /* Configure Tx interrupt processing */
#if defined(USART_CR1_FIFOEN)
    if (hsmartcard->FifoMode == SMARTCARD_FIFOMODE_ENABLE)
    {
      /* Set the Tx ISR function pointer */
      hsmartcard->TxISR = SMARTCARD_TxISR_FIFOEN;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Enable the SMARTCARD Error Interrupt: (Frame error) */
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

      /* Enable the TX FIFO threshold interrupt */
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_TXFTIE);
    }
    else
    {
      /* Set the Tx ISR function pointer */
      hsmartcard->TxISR = SMARTCARD_TxISR;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Enable the SMARTCARD Error Interrupt: (Frame error) */
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

      /* Enable the SMARTCARD Transmit Data Register Empty Interrupt */
      SET_BIT(hsmartcard->Instance->CR1, USART_CR1_TXEIE_TXFNFIE);
    }
#else
    /* Set the Tx ISR function pointer */
    hsmartcard->TxISR = SMARTCARD_TxISR;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmartcard);

    /* Enable the SMARTCARD Error Interrupt: (Frame error) */
    SET_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

    /* Enable the SMARTCARD Transmit Data Register Empty Interrupt */
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_TXEIE);
#endif /* USART_CR1_FIFOEN */

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Receive an amount of data in interrupt mode.
  * @note   When FIFO mode is disabled, USART interrupt is generated whenever
  *         USART_RDR register can be read, i.e one interrupt per data to receive.
  * @note   When FIFO mode is enabled, USART interrupt is generated whenever
  *         RXFIFO threshold reached. In that case the interrupt rate depends on
  *         RXFIFO threshold configuration.
  * @note   This function sets the hsmartcard->RxIsr function pointer according to
  *         the FIFO mode (data reception processing depends on FIFO mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param  pData pointer to data buffer.
  * @param  Size amount of data to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Receive_IT(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size)
{
  /* Check that a Rx process is not already ongoing */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
    hsmartcard->RxState   = HAL_SMARTCARD_STATE_BUSY_RX;

    hsmartcard->pRxBuffPtr = pData;
    hsmartcard->RxXferSize = Size;
    hsmartcard->RxXferCount = Size;

    /* Configure Rx interrupt processing */
#if defined(USART_CR1_FIFOEN)
    if ((hsmartcard->FifoMode == SMARTCARD_FIFOMODE_ENABLE) && (Size >= hsmartcard->NbRxDataToProcess))
    {
      /* Set the Rx ISR function pointer */
      hsmartcard->RxISR = SMARTCARD_RxISR_FIFOEN;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Enable the SMARTCART Parity Error interrupt and RX FIFO Threshold interrupt */
      SET_BIT(hsmartcard->Instance->CR1, USART_CR1_PEIE);
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_RXFTIE);
    }
    else
    {
      /* Set the Rx ISR function pointer */
      hsmartcard->RxISR = SMARTCARD_RxISR;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Enable the SMARTCARD Parity Error and Data Register not empty Interrupts */
      SET_BIT(hsmartcard->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE_RXFNEIE);
    }
#else
    /* Set the Rx ISR function pointer */
    hsmartcard->RxISR = SMARTCARD_RxISR;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmartcard);

    /* Enable the SMARTCARD Parity Error and Data Register not empty Interrupts */
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);
#endif /* USART_CR1_FIFOEN */

    /* Enable the SMARTCARD Error Interrupt: (Frame error, noise error, overrun error) */
    SET_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Send an amount of data in DMA mode.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param  pData pointer to data buffer.
  * @param  Size amount of data to be sent.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Transmit_DMA(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size)
{
  /* Check that a Tx process is not already ongoing */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->gState = HAL_SMARTCARD_STATE_BUSY_TX;

    hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
    hsmartcard->pTxBuffPtr = pData;
    hsmartcard->TxXferSize = Size;
    hsmartcard->TxXferCount = Size;

    /* Disable the Peripheral first to update mode for TX master */
    CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

    /* Disable Rx, enable Tx */
    CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_RE);
    SET_BIT(hsmartcard->Instance->RQR, (uint16_t)SMARTCARD_RXDATA_FLUSH_REQUEST);
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_TE);

    /* Enable the Peripheral */
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);

    /* Set the SMARTCARD DMA transfer complete callback */
    hsmartcard->hdmatx->XferCpltCallback = SMARTCARD_DMATransmitCplt;

    /* Set the SMARTCARD error callback */
    hsmartcard->hdmatx->XferErrorCallback = SMARTCARD_DMAError;

    /* Set the DMA abort callback */
    hsmartcard->hdmatx->XferAbortCallback = NULL;

    /* Enable the SMARTCARD transmit DMA channel */
    if (HAL_DMA_Start_IT(hsmartcard->hdmatx, (uint32_t)hsmartcard->pTxBuffPtr, (uint32_t)&hsmartcard->Instance->TDR,
                         Size) == HAL_OK)
    {
      /* Clear the TC flag in the ICR register */
      CLEAR_BIT(hsmartcard->Instance->ICR, USART_ICR_TCCF);

      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Enable the UART Error Interrupt: (Frame error) */
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

      /* Enable the DMA transfer for transmit request by setting the DMAT bit
         in the SMARTCARD associated USART CR3 register */
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAT);

      return HAL_OK;
    }
    else
    {
      /* Set error code to DMA */
      hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_DMA;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Restore hsmartcard->State to ready */
      hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

      return HAL_ERROR;
    }
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Receive an amount of data in DMA mode.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @param  pData pointer to data buffer.
  * @param  Size amount of data to be received.
  * @note   The SMARTCARD-associated USART parity is enabled (PCE = 1),
  *         the received data contain the parity bit (MSB position).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Receive_DMA(SMARTCARD_HandleTypeDef *hsmartcard, uint8_t *pData, uint16_t Size)
{
  /* Check that a Rx process is not already ongoing */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(hsmartcard);

    hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
    hsmartcard->RxState   = HAL_SMARTCARD_STATE_BUSY_RX;

    hsmartcard->pRxBuffPtr = pData;
    hsmartcard->RxXferSize = Size;

    /* Set the SMARTCARD DMA transfer complete callback */
    hsmartcard->hdmarx->XferCpltCallback = SMARTCARD_DMAReceiveCplt;

    /* Set the SMARTCARD DMA error callback */
    hsmartcard->hdmarx->XferErrorCallback = SMARTCARD_DMAError;

    /* Set the DMA abort callback */
    hsmartcard->hdmarx->XferAbortCallback = NULL;

    /* Enable the DMA channel */
    if (HAL_DMA_Start_IT(hsmartcard->hdmarx, (uint32_t)&hsmartcard->Instance->RDR, (uint32_t)hsmartcard->pRxBuffPtr,
                         Size) == HAL_OK)
    {
      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Enable the SMARTCARD Parity Error Interrupt */
      SET_BIT(hsmartcard->Instance->CR1, USART_CR1_PEIE);

      /* Enable the SMARTCARD Error Interrupt: (Frame error, noise error, overrun error) */
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

      /* Enable the DMA transfer for the receiver request by setting the DMAR bit
         in the SMARTCARD associated USART CR3 register */
      SET_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAR);

      return HAL_OK;
    }
    else
    {
      /* Set error code to DMA */
      hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_DMA;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmartcard);

      /* Restore hsmartcard->State to ready */
      hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

      return HAL_ERROR;
    }
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Abort ongoing transfers (blocking mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @note   This procedure could be used for aborting any ongoing transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable SMARTCARD Interrupts (Tx and Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Abort(SMARTCARD_HandleTypeDef *hsmartcard)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable RTOIE, EOBIE, TXEIE, TCIE, RXNE, PE, RXFT, TXFT and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1,
            (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE | USART_CR1_TXEIE_TXFNFIE | USART_CR1_TCIE | USART_CR1_RTOIE |
             USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE | USART_CR3_TXFTIE));
#else
  /* Disable RTOIE, EOBIE, TXEIE, TCIE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1,
            (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RTOIE | USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* Disable the SMARTCARD DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAT))
  {
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAT);

    /* Abort the SMARTCARD DMA Tx channel : use blocking DMA Abort API (no callback) */
    if (hsmartcard->hdmatx != NULL)
    {
      /* Set the SMARTCARD DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      hsmartcard->hdmatx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(hsmartcard->hdmatx) != HAL_OK)
      {
        if (HAL_DMA_GetError(hsmartcard->hdmatx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Disable the SMARTCARD DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAR);

    /* Abort the SMARTCARD DMA Rx channel : use blocking DMA Abort API (no callback) */
    if (hsmartcard->hdmarx != NULL)
    {
      /* Set the SMARTCARD DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      hsmartcard->hdmarx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(hsmartcard->hdmarx) != HAL_OK)
      {
        if (HAL_DMA_GetError(hsmartcard->hdmarx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Reset Tx and Rx transfer counters */
  hsmartcard->TxXferCount = 0U;
  hsmartcard->RxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                             SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                             SMARTCARD_CLEAR_EOBF);

  /* Restore hsmartcard->gState and hsmartcard->RxState to Ready */
  hsmartcard->gState  = HAL_SMARTCARD_STATE_READY;
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

  /* Reset Handle ErrorCode to No Error */
  hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Transmit transfer (blocking mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @note   This procedure could be used for aborting any ongoing Tx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable SMARTCARD Interrupts (Tx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_AbortTransmit(SMARTCARD_HandleTypeDef *hsmartcard)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable TCIE, TXEIE and TXFTIE interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_TXEIE_TXFNFIE | USART_CR1_TCIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_TXFTIE);
#else
  /* Disable TXEIE and TCIE interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
#endif /* USART_CR1_FIFOEN */

  /* Check if a receive process is ongoing or not. If not disable ERR IT */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_READY)
  {
    /* Disable the SMARTCARD Error Interrupt: (Frame error) */
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
  }

  /* Disable the SMARTCARD DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAT))
  {
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAT);

    /* Abort the SMARTCARD DMA Tx channel : use blocking DMA Abort API (no callback) */
    if (hsmartcard->hdmatx != NULL)
    {
      /* Set the SMARTCARD DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      hsmartcard->hdmatx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(hsmartcard->hdmatx) != HAL_OK)
      {
        if (HAL_DMA_GetError(hsmartcard->hdmatx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Reset Tx transfer counter */
  hsmartcard->TxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard, SMARTCARD_CLEAR_FEF);

  /* Restore hsmartcard->gState to Ready */
  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Receive transfer (blocking mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @note   This procedure could be used for aborting any ongoing Rx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable SMARTCARD Interrupts (Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_AbortReceive(SMARTCARD_HandleTypeDef *hsmartcard)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable RTOIE, EOBIE, RXNE, PE, RXFT, TXFT and  ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE | USART_CR1_RTOIE | USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE));
#else
  /* Disable RTOIE, EOBIE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_RTOIE | USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* Check if a Transmit process is ongoing or not. If not disable ERR IT */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    /* Disable the SMARTCARD Error Interrupt: (Frame error) */
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
  }

  /* Disable the SMARTCARD DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAR);

    /* Abort the SMARTCARD DMA Rx channel : use blocking DMA Abort API (no callback) */
    if (hsmartcard->hdmarx != NULL)
    {
      /* Set the SMARTCARD DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      hsmartcard->hdmarx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(hsmartcard->hdmarx) != HAL_OK)
      {
        if (HAL_DMA_GetError(hsmartcard->hdmarx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Reset Rx transfer counter */
  hsmartcard->RxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                             SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                             SMARTCARD_CLEAR_EOBF);

  /* Restore hsmartcard->RxState to Ready */
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Abort ongoing transfers (Interrupt mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @note   This procedure could be used for aborting any ongoing transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable SMARTCARD Interrupts (Tx and Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_Abort_IT(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint32_t abortcplt = 1U;

#if defined(USART_CR1_FIFOEN)
  /* Disable RTOIE, EOBIE, TXEIE, TCIE, RXNE, PE, RXFT, TXFT and  ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1,
            (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE | USART_CR1_TXEIE_TXFNFIE | USART_CR1_TCIE | USART_CR1_RTOIE |
             USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE | USART_CR3_TXFTIE));
#else
  /* Disable RTOIE, EOBIE, TXEIE, TCIE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1,
            (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RTOIE | USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* If DMA Tx and/or DMA Rx Handles are associated to SMARTCARD Handle, DMA Abort complete callbacks should be initialised
     before any call to DMA Abort functions */
  /* DMA Tx Handle is valid */
  if (hsmartcard->hdmatx != NULL)
  {
    /* Set DMA Abort Complete callback if SMARTCARD DMA Tx request if enabled.
       Otherwise, set it to NULL */
    if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAT))
    {
      hsmartcard->hdmatx->XferAbortCallback = SMARTCARD_DMATxAbortCallback;
    }
    else
    {
      hsmartcard->hdmatx->XferAbortCallback = NULL;
    }
  }
  /* DMA Rx Handle is valid */
  if (hsmartcard->hdmarx != NULL)
  {
    /* Set DMA Abort Complete callback if SMARTCARD DMA Rx request if enabled.
       Otherwise, set it to NULL */
    if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
    {
      hsmartcard->hdmarx->XferAbortCallback = SMARTCARD_DMARxAbortCallback;
    }
    else
    {
      hsmartcard->hdmarx->XferAbortCallback = NULL;
    }
  }

  /* Disable the SMARTCARD DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAT))
  {
    /* Disable DMA Tx at UART level */
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAT);

    /* Abort the SMARTCARD DMA Tx channel : use non blocking DMA Abort API (callback) */
    if (hsmartcard->hdmatx != NULL)
    {
      /* SMARTCARD Tx DMA Abort callback has already been initialised :
         will lead to call HAL_SMARTCARD_AbortCpltCallback() at end of DMA abort procedure */

      /* Abort DMA TX */
      if (HAL_DMA_Abort_IT(hsmartcard->hdmatx) != HAL_OK)
      {
        hsmartcard->hdmatx->XferAbortCallback = NULL;
      }
      else
      {
        abortcplt = 0U;
      }
    }
  }

  /* Disable the SMARTCARD DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAR);

    /* Abort the SMARTCARD DMA Rx channel : use non blocking DMA Abort API (callback) */
    if (hsmartcard->hdmarx != NULL)
    {
      /* SMARTCARD Rx DMA Abort callback has already been initialised :
         will lead to call HAL_SMARTCARD_AbortCpltCallback() at end of DMA abort procedure */

      /* Abort DMA RX */
      if (HAL_DMA_Abort_IT(hsmartcard->hdmarx) != HAL_OK)
      {
        hsmartcard->hdmarx->XferAbortCallback = NULL;
        abortcplt = 1U;
      }
      else
      {
        abortcplt = 0U;
      }
    }
  }

  /* if no DMA abort complete callback execution is required => call user Abort Complete callback */
  if (abortcplt == 1U)
  {
    /* Reset Tx and Rx transfer counters */
    hsmartcard->TxXferCount = 0U;
    hsmartcard->RxXferCount = 0U;

    /* Clear ISR function pointers */
    hsmartcard->RxISR = NULL;
    hsmartcard->TxISR = NULL;

    /* Reset errorCode */
    hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;

    /* Clear the Error flags in the ICR register */
    __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                               SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                               SMARTCARD_CLEAR_EOBF);

    /* Restore hsmartcard->gState and hsmartcard->RxState to Ready */
    hsmartcard->gState  = HAL_SMARTCARD_STATE_READY;
    hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

    /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
    /* Call registered Abort complete callback */
    hsmartcard->AbortCpltCallback(hsmartcard);
#else
    /* Call legacy weak Abort complete callback */
    HAL_SMARTCARD_AbortCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
  }

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Transmit transfer (Interrupt mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @note   This procedure could be used for aborting any ongoing Tx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable SMARTCARD Interrupts (Tx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_AbortTransmit_IT(SMARTCARD_HandleTypeDef *hsmartcard)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable TCIE, TXEIE and TXFTIE interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_TXEIE_TXFNFIE | USART_CR1_TCIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_TXFTIE);
#else
  /* Disable TXEIE and TCIE interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
#endif /* USART_CR1_FIFOEN */

  /* Check if a receive process is ongoing or not. If not disable ERR IT */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_READY)
  {
    /* Disable the SMARTCARD Error Interrupt: (Frame error) */
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
  }

  /* Disable the SMARTCARD DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAT))
  {
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAT);

    /* Abort the SMARTCARD DMA Tx channel : use non blocking DMA Abort API (callback) */
    if (hsmartcard->hdmatx != NULL)
    {
      /* Set the SMARTCARD DMA Abort callback :
         will lead to call HAL_SMARTCARD_AbortCpltCallback() at end of DMA abort procedure */
      hsmartcard->hdmatx->XferAbortCallback = SMARTCARD_DMATxOnlyAbortCallback;

      /* Abort DMA TX */
      if (HAL_DMA_Abort_IT(hsmartcard->hdmatx) != HAL_OK)
      {
        /* Call Directly hsmartcard->hdmatx->XferAbortCallback function in case of error */
        hsmartcard->hdmatx->XferAbortCallback(hsmartcard->hdmatx);
      }
    }
    else
    {
      /* Reset Tx transfer counter */
      hsmartcard->TxXferCount = 0U;

      /* Clear TxISR function pointers */
      hsmartcard->TxISR = NULL;

      /* Restore hsmartcard->gState to Ready */
      hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

      /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
      /* Call registered Abort Transmit Complete Callback */
      hsmartcard->AbortTransmitCpltCallback(hsmartcard);
#else
      /* Call legacy weak Abort Transmit Complete Callback */
      HAL_SMARTCARD_AbortTransmitCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
    }
  }
  else
  {
    /* Reset Tx transfer counter */
    hsmartcard->TxXferCount = 0U;

    /* Clear TxISR function pointers */
    hsmartcard->TxISR = NULL;

    /* Clear the Error flags in the ICR register */
    __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard, SMARTCARD_CLEAR_FEF);

    /* Restore hsmartcard->gState to Ready */
    hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

    /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
    /* Call registered Abort Transmit Complete Callback */
    hsmartcard->AbortTransmitCpltCallback(hsmartcard);
#else
    /* Call legacy weak Abort Transmit Complete Callback */
    HAL_SMARTCARD_AbortTransmitCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
  }

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Receive transfer (Interrupt mode).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @note   This procedure could be used for aborting any ongoing Rx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable SMARTCARD Interrupts (Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMARTCARD_AbortReceive_IT(SMARTCARD_HandleTypeDef *hsmartcard)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable RTOIE, EOBIE, RXNE, PE, RXFT and  ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE | USART_CR1_RTOIE | USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE));
#else
  /* Disable RTOIE, EOBIE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_RTOIE | USART_CR1_EOBIE));
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* Check if a Transmit process is ongoing or not. If not disable ERR IT */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
  {
    /* Disable the SMARTCARD Error Interrupt: (Frame error) */
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
  }

  /* Disable the SMARTCARD DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAR);

    /* Abort the SMARTCARD DMA Rx channel : use non blocking DMA Abort API (callback) */
    if (hsmartcard->hdmarx != NULL)
    {
      /* Set the SMARTCARD DMA Abort callback :
         will lead to call HAL_SMARTCARD_AbortCpltCallback() at end of DMA abort procedure */
      hsmartcard->hdmarx->XferAbortCallback = SMARTCARD_DMARxOnlyAbortCallback;

      /* Abort DMA RX */
      if (HAL_DMA_Abort_IT(hsmartcard->hdmarx) != HAL_OK)
      {
        /* Call Directly hsmartcard->hdmarx->XferAbortCallback function in case of error */
        hsmartcard->hdmarx->XferAbortCallback(hsmartcard->hdmarx);
      }
    }
    else
    {
      /* Reset Rx transfer counter */
      hsmartcard->RxXferCount = 0U;

      /* Clear RxISR function pointer */
      hsmartcard->RxISR = NULL;

      /* Clear the Error flags in the ICR register */
      __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                                 SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                                 SMARTCARD_CLEAR_EOBF);

      /* Restore hsmartcard->RxState to Ready */
      hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

      /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
      /* Call registered Abort Receive Complete Callback */
      hsmartcard->AbortReceiveCpltCallback(hsmartcard);
#else
      /* Call legacy weak Abort Receive Complete Callback */
      HAL_SMARTCARD_AbortReceiveCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
    }
  }
  else
  {
    /* Reset Rx transfer counter */
    hsmartcard->RxXferCount = 0U;

    /* Clear RxISR function pointer */
    hsmartcard->RxISR = NULL;

    /* Clear the Error flags in the ICR register */
    __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                               SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                               SMARTCARD_CLEAR_EOBF);

    /* Restore hsmartcard->RxState to Ready */
    hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

    /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
    /* Call registered Abort Receive Complete Callback */
    hsmartcard->AbortReceiveCpltCallback(hsmartcard);
#else
    /* Call legacy weak Abort Receive Complete Callback */
    HAL_SMARTCARD_AbortReceiveCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
  }

  return HAL_OK;
}

/**
  * @brief  Handle SMARTCARD interrupt requests.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
void HAL_SMARTCARD_IRQHandler(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint32_t isrflags   = READ_REG(hsmartcard->Instance->ISR);
  uint32_t cr1its     = READ_REG(hsmartcard->Instance->CR1);
  uint32_t cr3its     = READ_REG(hsmartcard->Instance->CR3);
  uint32_t errorflags;
  uint32_t errorcode;

  /* If no error occurs */
  errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE | USART_ISR_RTOF));
  if (errorflags == 0U)
  {
    /* SMARTCARD in mode Receiver ---------------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
    if (((isrflags & USART_ISR_RXNE_RXFNE) != 0U)
        && (((cr1its & USART_CR1_RXNEIE_RXFNEIE) != 0U)
            || ((cr3its & USART_CR3_RXFTIE) != 0U)))
#else
    if (((isrflags & USART_ISR_RXNE) != 0U)
        && ((cr1its & USART_CR1_RXNEIE) != 0U))
#endif /* USART_CR1_FIFOEN */
    {
      if (hsmartcard->RxISR != NULL)
      {
        hsmartcard->RxISR(hsmartcard);
      }
      return;
    }
  }

  /* If some errors occur */
#if defined(USART_CR1_FIFOEN)
  if ((errorflags != 0U)
      && ((((cr3its & (USART_CR3_RXFTIE | USART_CR3_EIE)) != 0U)
           || ((cr1its & (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE)) != 0U))))
#else
  if ((errorflags != 0U)
      && (((cr3its & USART_CR3_EIE) != 0U)
          || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != 0U)))
#endif /* USART_CR1_FIFOEN */
  {
    /* SMARTCARD parity error interrupt occurred -------------------------------------*/
    if (((isrflags & USART_ISR_PE) != 0U) && ((cr1its & USART_CR1_PEIE) != 0U))
    {
      __HAL_SMARTCARD_CLEAR_IT(hsmartcard, SMARTCARD_CLEAR_PEF);

      hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_PE;
    }

    /* SMARTCARD frame error interrupt occurred --------------------------------------*/
    if (((isrflags & USART_ISR_FE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
    {
      __HAL_SMARTCARD_CLEAR_IT(hsmartcard, SMARTCARD_CLEAR_FEF);

      hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_FE;
    }

    /* SMARTCARD noise error interrupt occurred --------------------------------------*/
    if (((isrflags & USART_ISR_NE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
    {
      __HAL_SMARTCARD_CLEAR_IT(hsmartcard, SMARTCARD_CLEAR_NEF);

      hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_NE;
    }

    /* SMARTCARD Over-Run interrupt occurred -----------------------------------------*/
#if defined(USART_CR1_FIFOEN)
    if (((isrflags & USART_ISR_ORE) != 0U)
        && (((cr1its & USART_CR1_RXNEIE_RXFNEIE) != 0U)
            || ((cr3its & USART_CR3_RXFTIE) != 0U)
            || ((cr3its & USART_CR3_EIE) != 0U)))
#else
    if (((isrflags & USART_ISR_ORE) != 0U)
        && (((cr1its & USART_CR1_RXNEIE) != 0U)
            || ((cr3its & USART_CR3_EIE) != 0U)))
#endif /* USART_CR1_FIFOEN */
    {
      __HAL_SMARTCARD_CLEAR_IT(hsmartcard, SMARTCARD_CLEAR_OREF);

      hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_ORE;
    }

    /* SMARTCARD receiver timeout interrupt occurred -----------------------------------------*/
    if (((isrflags & USART_ISR_RTOF) != 0U) && ((cr1its & USART_CR1_RTOIE) != 0U))
    {
      __HAL_SMARTCARD_CLEAR_IT(hsmartcard, SMARTCARD_CLEAR_RTOF);

      hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_RTO;
    }

    /* Call SMARTCARD Error Call back function if need be --------------------------*/
    if (hsmartcard->ErrorCode != HAL_SMARTCARD_ERROR_NONE)
    {
      /* SMARTCARD in mode Receiver ---------------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
      if (((isrflags & USART_ISR_RXNE_RXFNE) != 0U)
          && (((cr1its & USART_CR1_RXNEIE_RXFNEIE) != 0U)
              || ((cr3its & USART_CR3_RXFTIE) != 0U)))
#else
      if (((isrflags & USART_ISR_RXNE) != 0U)
          && ((cr1its & USART_CR1_RXNEIE) != 0U))
#endif /* USART_CR1_FIFOEN */
      {
        if (hsmartcard->RxISR != NULL)
        {
          hsmartcard->RxISR(hsmartcard);
        }
      }

      /* If Error is to be considered as blocking :
          - Receiver Timeout error in Reception
          - Overrun error in Reception
          - any error occurs in DMA mode reception
      */
      errorcode = hsmartcard->ErrorCode;
      if ((HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
          || ((errorcode & (HAL_SMARTCARD_ERROR_RTO | HAL_SMARTCARD_ERROR_ORE)) != 0U))
      {
        /* Blocking error : transfer is aborted
           Set the SMARTCARD state ready to be able to start again the process,
           Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
        SMARTCARD_EndRxTransfer(hsmartcard);

        /* Disable the SMARTCARD DMA Rx request if enabled */
        if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
        {
          CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAR);

          /* Abort the SMARTCARD DMA Rx channel */
          if (hsmartcard->hdmarx != NULL)
          {
            /* Set the SMARTCARD DMA Abort callback :
               will lead to call HAL_SMARTCARD_ErrorCallback() at end of DMA abort procedure */
            hsmartcard->hdmarx->XferAbortCallback = SMARTCARD_DMAAbortOnError;

            /* Abort DMA RX */
            if (HAL_DMA_Abort_IT(hsmartcard->hdmarx) != HAL_OK)
            {
              /* Call Directly hsmartcard->hdmarx->XferAbortCallback function in case of error */
              hsmartcard->hdmarx->XferAbortCallback(hsmartcard->hdmarx);
            }
          }
          else
          {
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
            /* Call registered user error callback */
            hsmartcard->ErrorCallback(hsmartcard);
#else
            /* Call legacy weak user error callback */
            HAL_SMARTCARD_ErrorCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
          }
        }
        else
        {
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
          /* Call registered user error callback */
          hsmartcard->ErrorCallback(hsmartcard);
#else
          /* Call legacy weak user error callback */
          HAL_SMARTCARD_ErrorCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
        }
      }
      /* other error type to be considered as blocking :
          - Frame error in Transmission
      */
      else if ((hsmartcard->gState == HAL_SMARTCARD_STATE_BUSY_TX)
               && ((errorcode & HAL_SMARTCARD_ERROR_FE) != 0U))
      {
        /* Blocking error : transfer is aborted
           Set the SMARTCARD state ready to be able to start again the process,
           Disable Tx Interrupts, and disable Tx DMA request, if ongoing */
        SMARTCARD_EndTxTransfer(hsmartcard);

        /* Disable the SMARTCARD DMA Tx request if enabled */
        if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAT))
        {
          CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAT);

          /* Abort the SMARTCARD DMA Tx channel */
          if (hsmartcard->hdmatx != NULL)
          {
            /* Set the SMARTCARD DMA Abort callback :
               will lead to call HAL_SMARTCARD_ErrorCallback() at end of DMA abort procedure */
            hsmartcard->hdmatx->XferAbortCallback = SMARTCARD_DMAAbortOnError;

            /* Abort DMA TX */
            if (HAL_DMA_Abort_IT(hsmartcard->hdmatx) != HAL_OK)
            {
              /* Call Directly hsmartcard->hdmatx->XferAbortCallback function in case of error */
              hsmartcard->hdmatx->XferAbortCallback(hsmartcard->hdmatx);
            }
          }
          else
          {
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
            /* Call registered user error callback */
            hsmartcard->ErrorCallback(hsmartcard);
#else
            /* Call legacy weak user error callback */
            HAL_SMARTCARD_ErrorCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
          }
        }
        else
        {
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
          /* Call registered user error callback */
          hsmartcard->ErrorCallback(hsmartcard);
#else
          /* Call legacy weak user error callback */
          HAL_SMARTCARD_ErrorCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
        }
      }
      else
      {
        /* Non Blocking error : transfer could go on.
           Error is notified to user through user error callback */
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
        /* Call registered user error callback */
        hsmartcard->ErrorCallback(hsmartcard);
#else
        /* Call legacy weak user error callback */
        HAL_SMARTCARD_ErrorCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
        hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;
      }
    }
    return;

  } /* End if some error occurs */

  /* SMARTCARD in mode Receiver, end of block interruption ------------------------*/
  if (((isrflags & USART_ISR_EOBF) != 0U) && ((cr1its & USART_CR1_EOBIE) != 0U))
  {
    hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;
    __HAL_UNLOCK(hsmartcard);
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
    /* Call registered Rx complete callback */
    hsmartcard->RxCpltCallback(hsmartcard);
#else
    /* Call legacy weak Rx complete callback */
    HAL_SMARTCARD_RxCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
    /* Clear EOBF interrupt after HAL_SMARTCARD_RxCpltCallback() call for the End of Block information
       to be available during HAL_SMARTCARD_RxCpltCallback() processing */
    __HAL_SMARTCARD_CLEAR_IT(hsmartcard, SMARTCARD_CLEAR_EOBF);
    return;
  }

  /* SMARTCARD in mode Transmitter ------------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
  if (((isrflags & USART_ISR_TXE_TXFNF) != 0U)
      && (((cr1its & USART_CR1_TXEIE_TXFNFIE) != 0U)
          || ((cr3its & USART_CR3_TXFTIE) != 0U)))
#else
  if (((isrflags & USART_ISR_TXE) != 0U)
      && ((cr1its & USART_CR1_TXEIE) != 0U))
#endif /* USART_CR1_FIFOEN */
  {
    if (hsmartcard->TxISR != NULL)
    {
      hsmartcard->TxISR(hsmartcard);
    }
    return;
  }

  /* SMARTCARD in mode Transmitter (transmission end) ------------------------*/
  if (__HAL_SMARTCARD_GET_IT(hsmartcard, hsmartcard->AdvancedInit.TxCompletionIndication) != RESET)
  {
    if (__HAL_SMARTCARD_GET_IT_SOURCE(hsmartcard, hsmartcard->AdvancedInit.TxCompletionIndication) != RESET)
    {
      SMARTCARD_EndTransmit_IT(hsmartcard);
      return;
    }
  }

#if defined(USART_CR1_FIFOEN)
  /* SMARTCARD TX Fifo Empty occurred ----------------------------------------------*/
  if (((isrflags & USART_ISR_TXFE) != 0U) && ((cr1its & USART_CR1_TXFEIE) != 0U))
  {
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
    /* Call registered Tx Fifo Empty Callback */
    hsmartcard->TxFifoEmptyCallback(hsmartcard);
#else
    /* Call legacy weak Tx Fifo Empty Callback */
    HAL_SMARTCARDEx_TxFifoEmptyCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
    return;
  }

  /* SMARTCARD RX Fifo Full occurred ----------------------------------------------*/
  if (((isrflags & USART_ISR_RXFF) != 0U) && ((cr1its & USART_CR1_RXFFIE) != 0U))
  {
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
    /* Call registered Rx Fifo Full Callback */
    hsmartcard->RxFifoFullCallback(hsmartcard);
#else
    /* Call legacy weak Rx Fifo Full Callback */
    HAL_SMARTCARDEx_RxFifoFullCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
    return;
  }
#endif /* USART_CR1_FIFOEN */
}

/**
  * @brief  Tx Transfer completed callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_TxCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_TxCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_RxCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_RxCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  SMARTCARD error callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_ErrorCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_ErrorCallback can be implemented in the user file.
   */
}

/**
  * @brief  SMARTCARD Abort Complete callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_AbortCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_AbortCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  SMARTCARD Abort Complete callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_AbortTransmitCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_AbortTransmitCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  SMARTCARD Abort Receive Complete callback.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
__weak void HAL_SMARTCARD_AbortReceiveCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmartcard);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMARTCARD_AbortReceiveCpltCallback can be implemented in the user file.
   */
}

/**
  * @}
  */

/** @defgroup SMARTCARD_Exported_Functions_Group4 Peripheral State and Errors functions
  * @brief    SMARTCARD State and Errors functions
  *
@verbatim
  ==============================================================================
                  ##### Peripheral State and Errors functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to return the State of SmartCard
    handle and also return Peripheral Errors occurred during communication process
     (+) HAL_SMARTCARD_GetState() API can be helpful to check in run-time the state
         of the SMARTCARD peripheral.
     (+) HAL_SMARTCARD_GetError() checks in run-time errors that could occur during
         communication.

@endverbatim
  * @{
  */

/**
  * @brief  Return the SMARTCARD handle state.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval SMARTCARD handle state
  */
HAL_SMARTCARD_StateTypeDef HAL_SMARTCARD_GetState(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Return SMARTCARD handle state */
  uint32_t temp1;
  uint32_t temp2;
  temp1 = (uint32_t)hsmartcard->gState;
  temp2 = (uint32_t)hsmartcard->RxState;

  return (HAL_SMARTCARD_StateTypeDef)(temp1 | temp2);
}

/**
  * @brief  Return the SMARTCARD handle error code.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval SMARTCARD handle Error Code
  */
uint32_t HAL_SMARTCARD_GetError(SMARTCARD_HandleTypeDef *hsmartcard)
{
  return hsmartcard->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup SMARTCARD_Private_Functions SMARTCARD Private Functions
  * @{
  */

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
/**
  * @brief  Initialize the callbacks to their default values.
  * @param  hsmartcard SMARTCARD handle.
  * @retval none
  */
void SMARTCARD_InitCallbacksToDefault(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Init the SMARTCARD Callback settings */
  hsmartcard->TxCpltCallback            = HAL_SMARTCARD_TxCpltCallback;            /* Legacy weak TxCpltCallback            */
  hsmartcard->RxCpltCallback            = HAL_SMARTCARD_RxCpltCallback;            /* Legacy weak RxCpltCallback            */
  hsmartcard->ErrorCallback             = HAL_SMARTCARD_ErrorCallback;             /* Legacy weak ErrorCallback             */
  hsmartcard->AbortCpltCallback         = HAL_SMARTCARD_AbortCpltCallback;         /* Legacy weak AbortCpltCallback         */
  hsmartcard->AbortTransmitCpltCallback = HAL_SMARTCARD_AbortTransmitCpltCallback; /* Legacy weak AbortTransmitCpltCallback */
  hsmartcard->AbortReceiveCpltCallback  = HAL_SMARTCARD_AbortReceiveCpltCallback;  /* Legacy weak AbortReceiveCpltCallback  */
#if defined(USART_CR1_FIFOEN)
  hsmartcard->RxFifoFullCallback        = HAL_SMARTCARDEx_RxFifoFullCallback;      /* Legacy weak RxFifoFullCallback        */
  hsmartcard->TxFifoEmptyCallback       = HAL_SMARTCARDEx_TxFifoEmptyCallback;     /* Legacy weak TxFifoEmptyCallback       */
#endif /* USART_CR1_FIFOEN */

}
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACKS */

/**
  * @brief  Configure the SMARTCARD associated USART peripheral.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval HAL status
  */
static HAL_StatusTypeDef SMARTCARD_SetConfig(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint32_t tmpreg;
  SMARTCARD_ClockSourceTypeDef clocksource;
  HAL_StatusTypeDef ret = HAL_OK;
#if defined(USART_PRESC_PRESCALER)
  const uint16_t SMARTCARDPrescTable[12] = {1U, 2U, 4U, 6U, 8U, 10U, 12U, 16U, 32U, 64U, 128U, 256U};
#endif /* USART_PRESC_PRESCALER */

  /* Check the parameters */
  assert_param(IS_SMARTCARD_INSTANCE(hsmartcard->Instance));
  assert_param(IS_SMARTCARD_BAUDRATE(hsmartcard->Init.BaudRate));
  assert_param(IS_SMARTCARD_WORD_LENGTH(hsmartcard->Init.WordLength));
  assert_param(IS_SMARTCARD_STOPBITS(hsmartcard->Init.StopBits));
  assert_param(IS_SMARTCARD_PARITY(hsmartcard->Init.Parity));
  assert_param(IS_SMARTCARD_MODE(hsmartcard->Init.Mode));
  assert_param(IS_SMARTCARD_POLARITY(hsmartcard->Init.CLKPolarity));
  assert_param(IS_SMARTCARD_PHASE(hsmartcard->Init.CLKPhase));
  assert_param(IS_SMARTCARD_LASTBIT(hsmartcard->Init.CLKLastBit));
  assert_param(IS_SMARTCARD_ONE_BIT_SAMPLE(hsmartcard->Init.OneBitSampling));
  assert_param(IS_SMARTCARD_NACK(hsmartcard->Init.NACKEnable));
  assert_param(IS_SMARTCARD_TIMEOUT(hsmartcard->Init.TimeOutEnable));
  assert_param(IS_SMARTCARD_AUTORETRY_COUNT(hsmartcard->Init.AutoRetryCount));
#if defined(USART_PRESC_PRESCALER)
  assert_param(IS_SMARTCARD_CLOCKPRESCALER(hsmartcard->Init.ClockPrescaler));
#endif /* USART_PRESC_PRESCALER */

  /*-------------------------- USART CR1 Configuration -----------------------*/
  /* In SmartCard mode, M and PCE are forced to 1 (8 bits + parity).
   * Oversampling is forced to 16 (OVER8 = 0).
   * Configure the Parity and Mode:
   *  set PS bit according to hsmartcard->Init.Parity value
   *  set TE and RE bits according to hsmartcard->Init.Mode value */
#if defined(USART_CR1_FIFOEN)
  tmpreg = (uint32_t) hsmartcard->Init.Parity | hsmartcard->Init.Mode;
  tmpreg |= (uint32_t) hsmartcard->Init.WordLength | hsmartcard->FifoMode;
#else
  tmpreg = (uint32_t)(hsmartcard->Init.Parity | hsmartcard->Init.Mode | hsmartcard->Init.WordLength);
#endif /* USART_CR1_FIFOEN */
  MODIFY_REG(hsmartcard->Instance->CR1, USART_CR1_FIELDS, tmpreg);

  /*-------------------------- USART CR2 Configuration -----------------------*/
  tmpreg = hsmartcard->Init.StopBits;
  /* Synchronous mode is activated by default */
  tmpreg |= (uint32_t) USART_CR2_CLKEN | hsmartcard->Init.CLKPolarity;
  tmpreg |= (uint32_t) hsmartcard->Init.CLKPhase | hsmartcard->Init.CLKLastBit;
  tmpreg |= (uint32_t) hsmartcard->Init.TimeOutEnable;
  MODIFY_REG(hsmartcard->Instance->CR2, USART_CR2_FIELDS, tmpreg);

  /*-------------------------- USART CR3 Configuration -----------------------*/
  /* Configure
   * - one-bit sampling method versus three samples' majority rule
   *   according to hsmartcard->Init.OneBitSampling
   * - NACK transmission in case of parity error according
   *   to hsmartcard->Init.NACKEnable
   * - autoretry counter according to hsmartcard->Init.AutoRetryCount */

  tmpreg = (uint32_t) hsmartcard->Init.OneBitSampling | hsmartcard->Init.NACKEnable;
  tmpreg |= ((uint32_t)hsmartcard->Init.AutoRetryCount << USART_CR3_SCARCNT_Pos);
  MODIFY_REG(hsmartcard->Instance->CR3, USART_CR3_FIELDS, tmpreg);

#if defined(USART_PRESC_PRESCALER)
  /*--------------------- SMARTCARD clock PRESC Configuration ----------------*/
  /* Configure
  * - SMARTCARD Clock Prescaler: set PRESCALER according to hsmartcard->Init.ClockPrescaler value */
  MODIFY_REG(hsmartcard->Instance->PRESC, USART_PRESC_PRESCALER, hsmartcard->Init.ClockPrescaler);
#endif /* USART_PRESC_PRESCALER */

  /*-------------------------- USART GTPR Configuration ----------------------*/
  tmpreg = (hsmartcard->Init.Prescaler | ((uint32_t)hsmartcard->Init.GuardTime << USART_GTPR_GT_Pos));
  MODIFY_REG(hsmartcard->Instance->GTPR, (uint16_t)(USART_GTPR_GT | USART_GTPR_PSC), (uint16_t)tmpreg);

  /*-------------------------- USART RTOR Configuration ----------------------*/
  tmpreg = ((uint32_t)hsmartcard->Init.BlockLength << USART_RTOR_BLEN_Pos);
  if (hsmartcard->Init.TimeOutEnable == SMARTCARD_TIMEOUT_ENABLE)
  {
    assert_param(IS_SMARTCARD_TIMEOUT_VALUE(hsmartcard->Init.TimeOutValue));
    tmpreg |= (uint32_t) hsmartcard->Init.TimeOutValue;
  }
  MODIFY_REG(hsmartcard->Instance->RTOR, (USART_RTOR_RTO | USART_RTOR_BLEN), tmpreg);

  /*-------------------------- USART BRR Configuration -----------------------*/
  SMARTCARD_GETCLOCKSOURCE(hsmartcard, clocksource);
  tmpreg =   0U;
  switch (clocksource)
  {
    case SMARTCARD_CLOCKSOURCE_PCLK1:
#if defined(USART_PRESC_PRESCALER)
      tmpreg = (uint16_t)(((HAL_RCC_GetPCLK1Freq() / SMARTCARDPrescTable[hsmartcard->Init.ClockPrescaler]) + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#else
      tmpreg = (uint16_t)((HAL_RCC_GetPCLK1Freq() + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#endif /* USART_PRESC_PRESCALER */
      break;
    case SMARTCARD_CLOCKSOURCE_PCLK2:
#if defined(USART_PRESC_PRESCALER)
      tmpreg = (uint16_t)(((HAL_RCC_GetPCLK2Freq() / SMARTCARDPrescTable[hsmartcard->Init.ClockPrescaler]) + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#else
      tmpreg = (uint16_t)((HAL_RCC_GetPCLK2Freq() + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#endif /* USART_PRESC_PRESCALER */
      break;
    case SMARTCARD_CLOCKSOURCE_HSI:
#if defined(USART_PRESC_PRESCALER)
      tmpreg = (uint16_t)(((HSI_VALUE / SMARTCARDPrescTable[hsmartcard->Init.ClockPrescaler]) + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#else
      tmpreg = (uint16_t)((HSI_VALUE + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#endif /* USART_PRESC_PRESCALER */
      break;
    case SMARTCARD_CLOCKSOURCE_SYSCLK:
#if defined(USART_PRESC_PRESCALER)
      tmpreg = (uint16_t)(((HAL_RCC_GetSysClockFreq() / SMARTCARDPrescTable[hsmartcard->Init.ClockPrescaler]) + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#else
      tmpreg = (uint16_t)((HAL_RCC_GetSysClockFreq() + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#endif /* USART_PRESC_PRESCALER */
      break;
    case SMARTCARD_CLOCKSOURCE_LSE:
#if defined(USART_PRESC_PRESCALER)
      tmpreg = (uint16_t)(((uint16_t)(LSE_VALUE / SMARTCARDPrescTable[hsmartcard->Init.ClockPrescaler]) + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#else
      tmpreg = (uint16_t)((LSE_VALUE + (hsmartcard->Init.BaudRate / 2U)) / hsmartcard->Init.BaudRate);
#endif /* USART_PRESC_PRESCALER */
      break;
    default:
      ret = HAL_ERROR;
      break;
  }

  /* USARTDIV must be greater than or equal to 0d16 */
  if ((tmpreg >= USART_BRR_MIN) && (tmpreg <= USART_BRR_MAX))
  {
    hsmartcard->Instance->BRR = tmpreg;
  }
  else
  {
    ret = HAL_ERROR;
  }

#if defined(USART_CR1_FIFOEN)
  /* Initialize the number of data to process during RX/TX ISR execution */
  hsmartcard->NbTxDataToProcess = 1U;
  hsmartcard->NbRxDataToProcess = 1U;
#endif /* USART_CR1_FIFOEN */

  /* Clear ISR function pointers */
  hsmartcard->RxISR   = NULL;
  hsmartcard->TxISR   = NULL;

  return ret;
}


/**
  * @brief Configure the SMARTCARD associated USART peripheral advanced features.
  * @param hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                   the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_AdvFeatureConfig(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Check whether the set of advanced features to configure is properly set */
  assert_param(IS_SMARTCARD_ADVFEATURE_INIT(hsmartcard->AdvancedInit.AdvFeatureInit));

  /* if required, configure TX pin active level inversion */
  if (HAL_IS_BIT_SET(hsmartcard->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_TXINVERT_INIT))
  {
    assert_param(IS_SMARTCARD_ADVFEATURE_TXINV(hsmartcard->AdvancedInit.TxPinLevelInvert));
    MODIFY_REG(hsmartcard->Instance->CR2, USART_CR2_TXINV, hsmartcard->AdvancedInit.TxPinLevelInvert);
  }

  /* if required, configure RX pin active level inversion */
  if (HAL_IS_BIT_SET(hsmartcard->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_RXINVERT_INIT))
  {
    assert_param(IS_SMARTCARD_ADVFEATURE_RXINV(hsmartcard->AdvancedInit.RxPinLevelInvert));
    MODIFY_REG(hsmartcard->Instance->CR2, USART_CR2_RXINV, hsmartcard->AdvancedInit.RxPinLevelInvert);
  }

  /* if required, configure data inversion */
  if (HAL_IS_BIT_SET(hsmartcard->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_DATAINVERT_INIT))
  {
    assert_param(IS_SMARTCARD_ADVFEATURE_DATAINV(hsmartcard->AdvancedInit.DataInvert));
    MODIFY_REG(hsmartcard->Instance->CR2, USART_CR2_DATAINV, hsmartcard->AdvancedInit.DataInvert);
  }

  /* if required, configure RX/TX pins swap */
  if (HAL_IS_BIT_SET(hsmartcard->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_SWAP_INIT))
  {
    assert_param(IS_SMARTCARD_ADVFEATURE_SWAP(hsmartcard->AdvancedInit.Swap));
    MODIFY_REG(hsmartcard->Instance->CR2, USART_CR2_SWAP, hsmartcard->AdvancedInit.Swap);
  }

  /* if required, configure RX overrun detection disabling */
  if (HAL_IS_BIT_SET(hsmartcard->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_RXOVERRUNDISABLE_INIT))
  {
    assert_param(IS_SMARTCARD_OVERRUN(hsmartcard->AdvancedInit.OverrunDisable));
    MODIFY_REG(hsmartcard->Instance->CR3, USART_CR3_OVRDIS, hsmartcard->AdvancedInit.OverrunDisable);
  }

  /* if required, configure DMA disabling on reception error */
  if (HAL_IS_BIT_SET(hsmartcard->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_DMADISABLEONERROR_INIT))
  {
    assert_param(IS_SMARTCARD_ADVFEATURE_DMAONRXERROR(hsmartcard->AdvancedInit.DMADisableonRxError));
    MODIFY_REG(hsmartcard->Instance->CR3, USART_CR3_DDRE, hsmartcard->AdvancedInit.DMADisableonRxError);
  }

  /* if required, configure MSB first on communication line */
  if (HAL_IS_BIT_SET(hsmartcard->AdvancedInit.AdvFeatureInit, SMARTCARD_ADVFEATURE_MSBFIRST_INIT))
  {
    assert_param(IS_SMARTCARD_ADVFEATURE_MSBFIRST(hsmartcard->AdvancedInit.MSBFirst));
    MODIFY_REG(hsmartcard->Instance->CR2, USART_CR2_MSBFIRST, hsmartcard->AdvancedInit.MSBFirst);
  }

}

/**
  * @brief Check the SMARTCARD Idle State.
  * @param hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                   the configuration information for the specified SMARTCARD module.
  * @retval HAL status
  */
static HAL_StatusTypeDef SMARTCARD_CheckIdleState(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint32_t tickstart;

  /* Initialize the SMARTCARD ErrorCode */
  hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;

  /* Init tickstart for timeout management */
  tickstart = HAL_GetTick();

  /* Check if the Transmitter is enabled */
  if ((hsmartcard->Instance->CR1 & USART_CR1_TE) == USART_CR1_TE)
  {
    /* Wait until TEACK flag is set */
    if (SMARTCARD_WaitOnFlagUntilTimeout(hsmartcard, USART_ISR_TEACK, RESET, tickstart,
                                         SMARTCARD_TEACK_REACK_TIMEOUT) != HAL_OK)
    {
      /* Timeout occurred */
      return HAL_TIMEOUT;
    }
  }
  /* Check if the Receiver is enabled */
  if ((hsmartcard->Instance->CR1 & USART_CR1_RE) == USART_CR1_RE)
  {
    /* Wait until REACK flag is set */
    if (SMARTCARD_WaitOnFlagUntilTimeout(hsmartcard, USART_ISR_REACK, RESET, tickstart,
                                         SMARTCARD_TEACK_REACK_TIMEOUT) != HAL_OK)
    {
      /* Timeout occurred */
      return HAL_TIMEOUT;
    }
  }

  /* Initialize the SMARTCARD states */
  hsmartcard->gState  = HAL_SMARTCARD_STATE_READY;
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hsmartcard);

  return HAL_OK;
}

/**
  * @brief  Handle SMARTCARD Communication Timeout.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                   the configuration information for the specified SMARTCARD module.
  * @param  Flag Specifies the SMARTCARD flag to check.
  * @param  Status The new Flag status (SET or RESET).
  * @param  Tickstart Tick start value
  * @param  Timeout Timeout duration.
  * @retval HAL status
  */
static HAL_StatusTypeDef SMARTCARD_WaitOnFlagUntilTimeout(SMARTCARD_HandleTypeDef *hsmartcard, uint32_t Flag,
                                                          FlagStatus Status, uint32_t Tickstart, uint32_t Timeout)
{
  /* Wait until flag is set */
  while ((__HAL_SMARTCARD_GET_FLAG(hsmartcard, Flag) ? SET : RESET) == Status)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        /* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
#if defined(USART_CR1_FIFOEN)
        CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE | USART_CR1_TXEIE_TXFNFIE));
#else
        CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_TXEIE));
#endif /* USART_CR1_FIFOEN */
        CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

        hsmartcard->gState  = HAL_SMARTCARD_STATE_READY;
        hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(hsmartcard);
        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}


/**
  * @brief  End ongoing Tx transfer on SMARTCARD peripheral (following error detection or Transmit completion).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_EndTxTransfer(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Disable TXEIE, TCIE and ERR (Frame error, noise error, overrun error) interrupts */
#if defined(USART_CR1_FIFOEN)
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_TXEIE_TXFNFIE | USART_CR1_TCIE));
#else
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
#endif /* USART_CR1_FIFOEN */
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

  /* At end of Tx process, restore hsmartcard->gState to Ready */
  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;
}


/**
  * @brief  End ongoing Rx transfer on UART peripheral (following error detection or Reception completion).
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_EndRxTransfer(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
#if defined(USART_CR1_FIFOEN)
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE));
#else
  CLEAR_BIT(hsmartcard->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
#endif /* USART_CR1_FIFOEN */
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

  /* At end of Rx process, restore hsmartcard->RxState to Ready */
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;
}


/**
  * @brief  DMA SMARTCARD transmit process complete callback.
  * @param  hdma Pointer to a DMA_HandleTypeDef structure that contains
  *              the configuration information for the specified DMA module.
  * @retval None
  */
static void SMARTCARD_DMATransmitCplt(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);
  hsmartcard->TxXferCount = 0U;

  /* Disable the DMA transfer for transmit request by resetting the DMAT bit
  in the SMARTCARD associated USART CR3 register */
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAT);

  /* Enable the SMARTCARD Transmit Complete Interrupt */
  __HAL_SMARTCARD_ENABLE_IT(hsmartcard, hsmartcard->AdvancedInit.TxCompletionIndication);
}

/**
  * @brief  DMA SMARTCARD receive process complete callback.
  * @param  hdma Pointer to a DMA_HandleTypeDef structure that contains
  *              the configuration information for the specified DMA module.
  * @retval None
  */
static void SMARTCARD_DMAReceiveCplt(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);
  hsmartcard->RxXferCount = 0U;

  /* Disable PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_PEIE);
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);

  /* Disable the DMA transfer for the receiver request by resetting the DMAR bit
     in the SMARTCARD associated USART CR3 register */
  CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_DMAR);

  /* At end of Rx process, restore hsmartcard->RxState to Ready */
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered Rx complete callback */
  hsmartcard->RxCpltCallback(hsmartcard);
#else
  /* Call legacy weak Rx complete callback */
  HAL_SMARTCARD_RxCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}

/**
  * @brief  DMA SMARTCARD communication error callback.
  * @param  hdma Pointer to a DMA_HandleTypeDef structure that contains
  *              the configuration information for the specified DMA module.
  * @retval None
  */
static void SMARTCARD_DMAError(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);

  /* Stop SMARTCARD DMA Tx request if ongoing */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_BUSY_TX)
  {
    if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAT))
    {
      hsmartcard->TxXferCount = 0U;
      SMARTCARD_EndTxTransfer(hsmartcard);
    }
  }

  /* Stop SMARTCARD DMA Rx request if ongoing */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_BUSY_RX)
  {
    if (HAL_IS_BIT_SET(hsmartcard->Instance->CR3, USART_CR3_DMAR))
    {
      hsmartcard->RxXferCount = 0U;
      SMARTCARD_EndRxTransfer(hsmartcard);
    }
  }

  hsmartcard->ErrorCode |= HAL_SMARTCARD_ERROR_DMA;
#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered user error callback */
  hsmartcard->ErrorCallback(hsmartcard);
#else
  /* Call legacy weak user error callback */
  HAL_SMARTCARD_ErrorCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}

/**
  * @brief  DMA SMARTCARD communication abort callback, when initiated by HAL services on Error
  *         (To be called at end of DMA Abort procedure following error occurrence).
  * @param  hdma DMA handle.
  * @retval None
  */
static void SMARTCARD_DMAAbortOnError(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);
  hsmartcard->RxXferCount = 0U;
  hsmartcard->TxXferCount = 0U;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered user error callback */
  hsmartcard->ErrorCallback(hsmartcard);
#else
  /* Call legacy weak user error callback */
  HAL_SMARTCARD_ErrorCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}

/**
  * @brief  DMA SMARTCARD Tx communication abort callback, when initiated by user
  *         (To be called at end of DMA Tx Abort procedure following user abort request).
  * @note   When this callback is executed, User Abort complete call back is called only if no
  *         Abort still ongoing for Rx DMA Handle.
  * @param  hdma DMA handle.
  * @retval None
  */
static void SMARTCARD_DMATxAbortCallback(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);

  hsmartcard->hdmatx->XferAbortCallback = NULL;

  /* Check if an Abort process is still ongoing */
  if (hsmartcard->hdmarx != NULL)
  {
    if (hsmartcard->hdmarx->XferAbortCallback != NULL)
    {
      return;
    }
  }

  /* No Abort process still ongoing : All DMA channels are aborted, call user Abort Complete callback */
  hsmartcard->TxXferCount = 0U;
  hsmartcard->RxXferCount = 0U;

  /* Reset errorCode */
  hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;

  /* Clear the Error flags in the ICR register */
  __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                             SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                             SMARTCARD_CLEAR_EOBF);

  /* Restore hsmartcard->gState and hsmartcard->RxState to Ready */
  hsmartcard->gState  = HAL_SMARTCARD_STATE_READY;
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered Abort complete callback */
  hsmartcard->AbortCpltCallback(hsmartcard);
#else
  /* Call legacy weak Abort complete callback */
  HAL_SMARTCARD_AbortCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}


/**
  * @brief  DMA SMARTCARD Rx communication abort callback, when initiated by user
  *         (To be called at end of DMA Rx Abort procedure following user abort request).
  * @note   When this callback is executed, User Abort complete call back is called only if no
  *         Abort still ongoing for Tx DMA Handle.
  * @param  hdma DMA handle.
  * @retval None
  */
static void SMARTCARD_DMARxAbortCallback(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);

  hsmartcard->hdmarx->XferAbortCallback = NULL;

  /* Check if an Abort process is still ongoing */
  if (hsmartcard->hdmatx != NULL)
  {
    if (hsmartcard->hdmatx->XferAbortCallback != NULL)
    {
      return;
    }
  }

  /* No Abort process still ongoing : All DMA channels are aborted, call user Abort Complete callback */
  hsmartcard->TxXferCount = 0U;
  hsmartcard->RxXferCount = 0U;

  /* Reset errorCode */
  hsmartcard->ErrorCode = HAL_SMARTCARD_ERROR_NONE;

  /* Clear the Error flags in the ICR register */
  __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                             SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                             SMARTCARD_CLEAR_EOBF);

  /* Restore hsmartcard->gState and hsmartcard->RxState to Ready */
  hsmartcard->gState  = HAL_SMARTCARD_STATE_READY;
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered Abort complete callback */
  hsmartcard->AbortCpltCallback(hsmartcard);
#else
  /* Call legacy weak Abort complete callback */
  HAL_SMARTCARD_AbortCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}


/**
  * @brief  DMA SMARTCARD Tx communication abort callback, when initiated by user by a call to
  *         HAL_SMARTCARD_AbortTransmit_IT API (Abort only Tx transfer)
  *         (This callback is executed at end of DMA Tx Abort procedure following user abort request,
  *         and leads to user Tx Abort Complete callback execution).
  * @param  hdma DMA handle.
  * @retval None
  */
static void SMARTCARD_DMATxOnlyAbortCallback(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);

  hsmartcard->TxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard, SMARTCARD_CLEAR_FEF);

  /* Restore hsmartcard->gState to Ready */
  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered Abort Transmit Complete Callback */
  hsmartcard->AbortTransmitCpltCallback(hsmartcard);
#else
  /* Call legacy weak Abort Transmit Complete Callback */
  HAL_SMARTCARD_AbortTransmitCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}

/**
  * @brief  DMA SMARTCARD Rx communication abort callback, when initiated by user by a call to
  *         HAL_SMARTCARD_AbortReceive_IT API (Abort only Rx transfer)
  *         (This callback is executed at end of DMA Rx Abort procedure following user abort request,
  *         and leads to user Rx Abort Complete callback execution).
  * @param  hdma DMA handle.
  * @retval None
  */
static void SMARTCARD_DMARxOnlyAbortCallback(DMA_HandleTypeDef *hdma)
{
  SMARTCARD_HandleTypeDef *hsmartcard = (SMARTCARD_HandleTypeDef *)(hdma->Parent);

  hsmartcard->RxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_SMARTCARD_CLEAR_FLAG(hsmartcard,
                             SMARTCARD_CLEAR_OREF | SMARTCARD_CLEAR_NEF | SMARTCARD_CLEAR_PEF | SMARTCARD_CLEAR_FEF | SMARTCARD_CLEAR_RTOF |
                             SMARTCARD_CLEAR_EOBF);

  /* Restore hsmartcard->RxState to Ready */
  hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered Abort Receive Complete Callback */
  hsmartcard->AbortReceiveCpltCallback(hsmartcard);
#else
  /* Call legacy weak Abort Receive Complete Callback */
  HAL_SMARTCARD_AbortReceiveCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}

/**
  * @brief  Send an amount of data in non-blocking mode.
  * @note   Function called under interruption only, once
  *         interruptions have been enabled by HAL_SMARTCARD_Transmit_IT()
  *         and when the FIFO mode is disabled.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_TxISR(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Check that a Tx process is ongoing */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_BUSY_TX)
  {
    if (hsmartcard->TxXferCount == 0U)
    {
      /* Disable the SMARTCARD Transmit Data Register Empty Interrupt */
#if defined(USART_CR1_FIFOEN)
      CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_TXEIE_TXFNFIE);
#else
      CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_TXEIE);
#endif /* USART_CR1_FIFOEN */

      /* Enable the SMARTCARD Transmit Complete Interrupt */
      __HAL_SMARTCARD_ENABLE_IT(hsmartcard, hsmartcard->AdvancedInit.TxCompletionIndication);
    }
    else
    {
      hsmartcard->Instance->TDR = (uint8_t)(*hsmartcard->pTxBuffPtr & 0xFFU);
      hsmartcard->pTxBuffPtr++;
      hsmartcard->TxXferCount--;
    }
  }
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Send an amount of data in non-blocking mode.
  * @note   Function called under interruption only, once
  *         interruptions have been enabled by HAL_SMARTCARD_Transmit_IT()
  *         and when the FIFO mode is enabled.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_TxISR_FIFOEN(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint16_t   nb_tx_data;

  /* Check that a Tx process is ongoing */
  if (hsmartcard->gState == HAL_SMARTCARD_STATE_BUSY_TX)
  {
    for (nb_tx_data = hsmartcard->NbTxDataToProcess ; nb_tx_data > 0U ; nb_tx_data--)
    {
      if (hsmartcard->TxXferCount == 0U)
      {
        /* Disable the SMARTCARD Transmit Data Register Empty Interrupt */
        CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_TXEIE_TXFNFIE);

        /* Enable the SMARTCARD Transmit Complete Interrupt */
        __HAL_SMARTCARD_ENABLE_IT(hsmartcard, hsmartcard->AdvancedInit.TxCompletionIndication);
      }
      else if (READ_BIT(hsmartcard->Instance->ISR, USART_ISR_TXE_TXFNF) != 0U)
      {
        hsmartcard->Instance->TDR = (uint8_t)(*hsmartcard->pTxBuffPtr & 0xFFU);
        hsmartcard->pTxBuffPtr++;
        hsmartcard->TxXferCount--;
      }
      else
      {
        /* Nothing to do */
      }
    }
  }
}

#endif /* USART_CR1_FIFOEN */
/**
  * @brief  Wrap up transmission in non-blocking mode.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_EndTransmit_IT(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Disable the SMARTCARD Transmit Complete Interrupt */
  __HAL_SMARTCARD_DISABLE_IT(hsmartcard, hsmartcard->AdvancedInit.TxCompletionIndication);

  /* Check if a receive process is ongoing or not. If not disable ERR IT */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_READY)
  {
    /* Disable the SMARTCARD Error Interrupt: (Frame error) */
    CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
  }

  /* Re-enable Rx at end of transmission if initial mode is Rx/Tx */
  if (hsmartcard->Init.Mode == SMARTCARD_MODE_TX_RX)
  {
    /* Disable the Peripheral first to update modes */
    CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_RE);
    /* Enable the Peripheral */
    SET_BIT(hsmartcard->Instance->CR1, USART_CR1_UE);
  }

  /* Tx process is ended, restore hsmartcard->gState to Ready */
  hsmartcard->gState = HAL_SMARTCARD_STATE_READY;

  /* Clear TxISR function pointer */
  hsmartcard->TxISR = NULL;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
  /* Call registered Tx complete callback */
  hsmartcard->TxCpltCallback(hsmartcard);
#else
  /* Call legacy weak Tx complete callback */
  HAL_SMARTCARD_TxCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
}

/**
  * @brief  Receive an amount of data in non-blocking mode.
  * @note   Function called under interruption only, once
  *         interruptions have been enabled by HAL_SMARTCARD_Receive_IT()
  *         and when the FIFO mode is disabled.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_RxISR(SMARTCARD_HandleTypeDef *hsmartcard)
{
  /* Check that a Rx process is ongoing */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_BUSY_RX)
  {
    *hsmartcard->pRxBuffPtr = (uint8_t)(hsmartcard->Instance->RDR & (uint8_t)0xFF);
    hsmartcard->pRxBuffPtr++;

    hsmartcard->RxXferCount--;
    if (hsmartcard->RxXferCount == 0U)
    {
#if defined(USART_CR1_FIFOEN)
      CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_RXNEIE_RXFNEIE);
#else
      CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_RXNEIE);
#endif /* USART_CR1_FIFOEN */

      /* Check if a transmit process is ongoing or not. If not disable ERR IT */
      if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
      {
        /* Disable the SMARTCARD Error Interrupt: (Frame error, noise error, overrun error) */
        CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
      }

      /* Disable the SMARTCARD Parity Error Interrupt */
      CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_PEIE);

      hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

      /* Clear RxISR function pointer */
      hsmartcard->RxISR = NULL;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
      /* Call registered Rx complete callback */
      hsmartcard->RxCpltCallback(hsmartcard);
#else
      /* Call legacy weak Rx complete callback */
      HAL_SMARTCARD_RxCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
    }
  }
  else
  {
    /* Clear RXNE interrupt flag */
    __HAL_SMARTCARD_SEND_REQ(hsmartcard, SMARTCARD_RXDATA_FLUSH_REQUEST);
  }
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Receive an amount of data in non-blocking mode.
  * @note   Function called under interruption only, once
  *         interruptions have been enabled by HAL_SMARTCARD_Receive_IT()
  *         and when the FIFO mode is enabled.
  * @param  hsmartcard Pointer to a SMARTCARD_HandleTypeDef structure that contains
  *                    the configuration information for the specified SMARTCARD module.
  * @retval None
  */
static void SMARTCARD_RxISR_FIFOEN(SMARTCARD_HandleTypeDef *hsmartcard)
{
  uint16_t   nb_rx_data;
  uint16_t rxdatacount;

  /* Check that a Rx process is ongoing */
  if (hsmartcard->RxState == HAL_SMARTCARD_STATE_BUSY_RX)
  {
    for (nb_rx_data = hsmartcard->NbRxDataToProcess ; nb_rx_data > 0U ; nb_rx_data--)
    {
      *hsmartcard->pRxBuffPtr = (uint8_t)(hsmartcard->Instance->RDR & (uint8_t)0xFF);
      hsmartcard->pRxBuffPtr++;

      hsmartcard->RxXferCount--;
      if (hsmartcard->RxXferCount == 0U)
      {
        CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_RXNEIE_RXFNEIE);

        /* Check if a transmit process is ongoing or not. If not disable ERR IT */
        if (hsmartcard->gState == HAL_SMARTCARD_STATE_READY)
        {
          /* Disable the SMARTCARD Error Interrupt: (Frame error, noise error, overrun error) */
          CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_EIE);
        }

        /* Disable the SMARTCARD Parity Error Interrupt */
        CLEAR_BIT(hsmartcard->Instance->CR1, USART_CR1_PEIE);

        hsmartcard->RxState = HAL_SMARTCARD_STATE_READY;

        /* Clear RxISR function pointer */
        hsmartcard->RxISR = NULL;

#if (USE_HAL_SMARTCARD_REGISTER_CALLBACKS == 1)
        /* Call registered Rx complete callback */
        hsmartcard->RxCpltCallback(hsmartcard);
#else
        /* Call legacy weak Rx complete callback */
        HAL_SMARTCARD_RxCpltCallback(hsmartcard);
#endif /* USE_HAL_SMARTCARD_REGISTER_CALLBACK */
      }
    }

    /* When remaining number of bytes to receive is less than the RX FIFO
    threshold, next incoming frames are processed as if FIFO mode was
    disabled (i.e. one interrupt per received frame).
    */
    rxdatacount = hsmartcard->RxXferCount;
    if (((rxdatacount != 0U)) && (rxdatacount < hsmartcard->NbRxDataToProcess))
    {
      /* Disable the UART RXFT interrupt*/
      CLEAR_BIT(hsmartcard->Instance->CR3, USART_CR3_RXFTIE);

      /* Update the RxISR function pointer */
      hsmartcard->RxISR = SMARTCARD_RxISR;

      /* Enable the UART Data Register Not Empty interrupt */
      SET_BIT(hsmartcard->Instance->CR1, USART_CR1_RXNEIE_RXFNEIE);
    }
  }
  else
  {
    /* Clear RXNE interrupt flag */
    __HAL_SMARTCARD_SEND_REQ(hsmartcard, SMARTCARD_RXDATA_FLUSH_REQUEST);
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
