/**
  ******************************************************************************
  * @file    stm32l4xx_hal_swpmi.c
  * @author  MCD Application Team
  * @brief   SWPMI HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Single Wire Protocol Master Interface (SWPMI).
  *           + Initialization and Configuration
  *           + Data transfers functions
  *           + DMA transfers management
  *           + Interrupts and flags management
  @verbatim
 ===============================================================================
                        ##### How to use this driver #####
 ===============================================================================
  [..]
     The SWPMI HAL driver can be used as follows:

    (#) Declare a SWPMI_HandleTypeDef handle structure (eg. SWPMI_HandleTypeDef hswpmi).

    (#) Initialize the SWPMI low level resources by implementing the HAL_SWPMI_MspInit() API:
        (##) Enable the SWPMIx interface clock with __HAL_RCC_SWPMIx_CLK_ENABLE().
        (##) SWPMI IO configuration:
            (+++) Enable the clock for the SWPMI GPIO.
            (+++) Configure these SWPMI pins as alternate function pull-up.
        (##) NVIC configuration if you need to use interrupt process (HAL_SWPMI_Transmit_IT()
             and HAL_SWPMI_Receive_IT() APIs):
            (+++) Configure the SWPMIx interrupt priority with HAL_NVIC_SetPriority().
            (+++) Enable the NVIC SWPMI IRQ handle with HAL_NVIC_EnableIRQ().

        (##) DMA Configuration if you need to use DMA process (HAL_SWPMI_Transmit_DMA()
             and HAL_SWPMI_Receive_DMA() APIs):
            (+++) Declare a DMA handle structure for the Tx/Rx channels.
            (+++) Enable the DMAx interface clock.
            (+++) Configure the declared DMA handle structure with the required
                  Tx/Rx parameters.
            (+++) Configure the DMA Tx/Rx channels and requests.
            (+++) Associate the initialized DMA handle to the SWPMI DMA Tx/Rx handle.
            (+++) Configure the priority and enable the NVIC for the transfer complete
                  interrupt on the DMA Tx/Rx channels.

    (#) Program the Bite Rate, Tx Buffering mode, Rx Buffering mode in the Init structure.

    (#) Enable the SWPMI peripheral by calling the HAL_SWPMI_Init() function.

  [..]
    Three operation modes are available within this driver :

    *** Polling mode IO operation ***
    =================================
    [..]
      (+) Send an amount of data in blocking mode using HAL_SWPMI_Transmit()
      (+) Receive an amount of data in blocking mode using HAL_SWPMI_Receive()

    *** Interrupt mode IO operation ***
    ===================================
    [..]
      (+) Send an amount of data in non-blocking mode using HAL_SWPMI_Transmit_IT()
      (+) At transmission end of transfer HAL_SWPMI_TxCpltCallback() is executed and user can
          add his own code by customization of function pointer HAL_SWPMI_TxCpltCallback()
      (+) Receive an amount of data in non-blocking mode using HAL_SWPMI_Receive_IT()
      (+) At reception end of transfer HAL_SWPMI_RxCpltCallback() is executed and user can
          add his own code by customization of function pointer HAL_SWPMI_RxCpltCallback()
      (+) In case of flag error, HAL_SWPMI_ErrorCallback() function is executed and user can
          add his own code by customization of function pointer HAL_SWPMI_ErrorCallback()

    *** DMA mode IO operation ***
    =============================
    [..]
      (+) Send an amount of data in non-blocking mode (DMA) using HAL_SWPMI_Transmit_DMA()
      (+) At transmission end of transfer HAL_SWPMI_TxCpltCallback() is executed and user can
          add his own code by customization of function pointer HAL_SWPMI_TxCpltCallback()
      (+) Receive an amount of data in non-blocking mode (DMA) using HAL_SWPMI_Receive_DMA()
      (+) At reception end of transfer HAL_SWPMI_RxCpltCallback() is executed and user can
          add his own code by customization of function pointer HAL_SWPMI_RxCpltCallback()
      (+) In case of flag error, HAL_SWPMI_ErrorCallback() function is executed and user can
          add his own code by customization of function pointer HAL_SWPMI_ErrorCallback()
      (+) Stop the DMA Transfer using HAL_SWPMI_DMAStop()

    *** SWPMI HAL driver additional function list ***
    ===============================================
    [..]
      Below the list the others API available SWPMI HAL driver :

      (+) HAL_SWPMI_EnableLoopback(): Enable the loopback mode for test purpose only
      (+) HAL_SWPMI_DisableLoopback(): Disable the loopback mode

    *** SWPMI HAL driver macros list ***
    ==================================
    [..]
      Below the list of most used macros in SWPMI HAL driver :

      (+) __HAL_SWPMI_ENABLE(): Enable the SWPMI peripheral
      (+) __HAL_SWPMI_DISABLE(): Disable the SWPMI peripheral
      (+) __HAL_SWPMI_ENABLE_IT(): Enable the specified SWPMI interrupts
      (+) __HAL_SWPMI_DISABLE_IT(): Disable the specified SWPMI interrupts
      (+) __HAL_SWPMI_GET_IT_SOURCE(): Check if the specified SWPMI interrupt source is
          enabled or disabled
      (+) __HAL_SWPMI_GET_FLAG(): Check whether the specified SWPMI flag is set or not

    *** Callback registration ***
    =============================
    [..]
      The compilation define USE_HAL_SWPMI_REGISTER_CALLBACKS when set to 1
      allows the user to configure dynamically the driver callbacks.
    [..]
      Use function HAL_SWPMI_RegisterCallback() to register a user callback. It allows
      to register the following callbacks:
      (+) RxCpltCallback     : SWPMI receive complete.
      (+) RxHalfCpltCallback : SWPMI receive half complete.
      (+) TxCpltCallback     : SWPMI transmit complete.
      (+) TxHalfCpltCallback : SWPMI transmit half complete.
      (+) ErrorCallback      : SWPMI error.
      (+) MspInitCallback    : SWPMI MspInit.
      (+) MspDeInitCallback  : SWPMI MspDeInit.
    [..]
    This function takes as parameters the HAL peripheral handle, the callback ID
    and a pointer to the user callback function.
    [..]
    Use function HAL_SWPMI_UnRegisterCallback() to reset a callback to the default
    weak (surcharged) function.
    HAL_SWPMI_UnRegisterCallback() takes as parameters the HAL peripheral handle,
    and the callback ID.
    This function allows to reset following callbacks:
      (+) RxCpltCallback     : SWPMI receive complete.
      (+) RxHalfCpltCallback : SWPMI receive half complete.
      (+) TxCpltCallback     : SWPMI transmit complete.
      (+) TxHalfCpltCallback : SWPMI transmit half complete.
      (+) ErrorCallback      : SWPMI error.
      (+) MspInitCallback    : SWPMI MspInit.
      (+) MspDeInitCallback  : SWPMI MspDeInit.
    [..]
    By default, after the HAL_SWPMI_Init and if the state is HAL_SWPMI_STATE_RESET
    all callbacks are reset to the corresponding legacy weak (surcharged) functions:
    examples HAL_SWPMI_RxCpltCallback(), HAL_SWPMI_ErrorCallback().
    Exception done for MspInit and MspDeInit callbacks that are respectively
    reset to the legacy weak (surcharged) functions in the HAL_SWPMI_Init
    and HAL_SWPMI_DeInit only when these callbacks are null (not registered beforehand).
    If not, MspInit or MspDeInit are not null, the HAL_SWPMI_Init and HAL_SWPMI_DeInit
    keep and use the user MspInit/MspDeInit callbacks (registered beforehand).
    [..]
    Callbacks can be registered/unregistered in READY state only.
    Exception done for MspInit/MspDeInit callbacks that can be registered/unregistered
    in READY or RESET state, thus registered (user) MspInit/DeInit callbacks can be used
    during the Init/DeInit.
    In that case first register the MspInit/MspDeInit user callbacks
    using HAL_SWPMI_RegisterCallback before calling @ref HAL_SWPMI_DeInit
    or HAL_SWPMI_Init function.
    [..]
    When the compilation define USE_HAL_SWPMI_REGISTER_CALLBACKS is set to 0 or
    not defined, the callback registering feature is not available
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

#if defined(SWPMI1)

/** @defgroup SWPMI SWPMI
  * @brief HAL SWPMI module driver
  * @{
  */
#ifdef HAL_SWPMI_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @addtogroup SWPMI_Private_Constants SWPMI Private Constants
  * @{
  */
#define SWPMI_TIMEOUT_VALUE                   22000U   /* End of transmission timeout */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SWPMI_DMATransmitCplt(DMA_HandleTypeDef *hdma);
static void SWPMI_DMATxHalfCplt(DMA_HandleTypeDef *hdma);
static void SWPMI_DMAReceiveCplt(DMA_HandleTypeDef *hdma);
static void SWPMI_DMARxHalfCplt(DMA_HandleTypeDef *hdma);
static void SWPMI_DMAError(DMA_HandleTypeDef *hdma);
static void SWPMI_DMAAbortOnError(DMA_HandleTypeDef *hdma);
static void SWPMI_Transmit_IT(SWPMI_HandleTypeDef *hswpmi);
static void SWPMI_EndTransmit_IT(SWPMI_HandleTypeDef *hswpmi);
static void SWPMI_Receive_IT(SWPMI_HandleTypeDef *hswpmi);
static void SWPMI_EndReceive_IT(SWPMI_HandleTypeDef *hswpmi);
static void SWPMI_EndTransmitReceive_IT(SWPMI_HandleTypeDef *hswpmi);
static HAL_StatusTypeDef SWPMI_WaitOnFlagSetUntilTimeout(SWPMI_HandleTypeDef *hswpmi, uint32_t Flag, uint32_t Tickstart, uint32_t Timeout);

/* Exported functions --------------------------------------------------------*/

/** @defgroup SWPMI_Exported_Functions SWPMI Exported Functions
  * @{
  */

/** @defgroup SWPMI_Exported_Group1 Initialization/de-initialization methods
  *  @brief    Initialization and Configuration functions
  *
@verbatim
 ===============================================================================
            ##### Initialization and Configuration functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize and configure the SWPMI peripheral.
      (+) De-initialize the SWPMI peripheral.

@endverbatim
  * @{
  */

/**
  * @brief Initialize the SWPMI peripheral according to the specified parameters in the SWPMI_InitTypeDef.
  * @param hswpmi SWPMI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_Init(SWPMI_HandleTypeDef *hswpmi)
{
  HAL_StatusTypeDef status = HAL_OK;
  __IO uint32_t wait_loop_index = 0U;

  /* Check the SWPMI handle allocation */
  if(hswpmi == NULL)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameters */
    assert_param(IS_SWPMI_VOLTAGE_CLASS(hswpmi->Init.VoltageClass));
    assert_param(IS_SWPMI_BITRATE_VALUE(hswpmi->Init.BitRate));
    assert_param(IS_SWPMI_TX_BUFFERING_MODE(hswpmi->Init.TxBufferingMode));
    assert_param(IS_SWPMI_RX_BUFFERING_MODE(hswpmi->Init.RxBufferingMode));

    if(hswpmi->State == HAL_SWPMI_STATE_RESET)
    {
      /* Allocate lock resource and initialize it */
      hswpmi->Lock = HAL_UNLOCKED;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
      /* Reset callback pointers to the weak predefined callbacks */
      hswpmi->RxCpltCallback     = HAL_SWPMI_RxCpltCallback;
      hswpmi->RxHalfCpltCallback = HAL_SWPMI_RxHalfCpltCallback;
      hswpmi->TxCpltCallback     = HAL_SWPMI_TxCpltCallback;
      hswpmi->TxHalfCpltCallback = HAL_SWPMI_TxHalfCpltCallback;
      hswpmi->ErrorCallback      = HAL_SWPMI_ErrorCallback;

      /* Init the low level hardware : GPIO, CLOCK, NVIC and DMA */
      if(hswpmi->MspInitCallback == NULL)
      {
        hswpmi->MspInitCallback = HAL_SWPMI_MspInit;
      }
      hswpmi->MspInitCallback(hswpmi);
#else
      /* Init the low level hardware : GPIO, CLOCK, NVIC and DMA */
      HAL_SWPMI_MspInit(hswpmi);
#endif
    }

    hswpmi->State = HAL_SWPMI_STATE_BUSY;

    /* Disable SWPMI interface */
    CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);

    /* Clear all SWPMI interface flags */
    WRITE_REG(hswpmi->Instance->ICR, 0x019F);

    /* Apply Voltage class selection */
    MODIFY_REG(hswpmi->Instance->OR, SWPMI_OR_CLASS, hswpmi->Init.VoltageClass);

    /* If Voltage class B, apply 300 µs delay */
    if(hswpmi->Init.VoltageClass == SWPMI_VOLTAGE_CLASS_B)
    {
      /* Insure 300 µs wait to insure SWPMI_IO output not higher than 1.8V */
      /* Wait loop initialization and execution                            */
      /* Note: Variable divided by 4 to compensate partially CPU processing cycles. */
      wait_loop_index = (300U * (SystemCoreClock / (1000000U * 4U))) + 150U;
      while(wait_loop_index != 0U)
      {
        wait_loop_index--;
      }
    }

    /* Configure the BRR register (Bitrate) */
    WRITE_REG(hswpmi->Instance->BRR, hswpmi->Init.BitRate);

    /* Apply SWPMI CR configuration */
    MODIFY_REG(hswpmi->Instance->CR, \
               SWPMI_CR_RXDMA | SWPMI_CR_TXDMA  | SWPMI_CR_RXMODE | SWPMI_CR_TXMODE, \
               hswpmi->Init.TxBufferingMode | hswpmi->Init.RxBufferingMode);

    hswpmi->ErrorCode = HAL_SWPMI_ERROR_NONE;
    hswpmi->State = HAL_SWPMI_STATE_READY;

    /* Enable SWPMI peripheral */
    SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);
  }

  return status;
}

/**
  * @brief De-initialize the SWPMI peripheral.
  * @param hswpmi SWPMI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_DeInit(SWPMI_HandleTypeDef *hswpmi)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the SWPMI handle allocation */
  if(hswpmi == NULL)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameters */
    assert_param(IS_SWPMI_INSTANCE(hswpmi->Instance));

    hswpmi->State = HAL_SWPMI_STATE_BUSY;

    /* Disable SWPMI interface */
    CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);

    /* Disable Loopback mode */
    CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_LPBK);


    /* DeInit the low level hardware: GPIO, CLOCK, NVIC and DMA */
#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
    if(hswpmi->MspDeInitCallback == NULL)
    {
      hswpmi->MspDeInitCallback = HAL_SWPMI_MspDeInit;
    }
    hswpmi->MspDeInitCallback(hswpmi);
#else
    HAL_SWPMI_MspDeInit(hswpmi);
#endif

    hswpmi->ErrorCode = HAL_SWPMI_ERROR_NONE;
    hswpmi->State = HAL_SWPMI_STATE_RESET;

    /* Release Lock */
    __HAL_UNLOCK(hswpmi);
  }

  return status;
}

/**
  * @brief Initialize the SWPMI MSP.
  * @param hswpmi SWPMI handle
  * @retval None
  */
__weak void HAL_SWPMI_MspInit(SWPMI_HandleTypeDef *hswpmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hswpmi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SWPMI_MspInit can be implemented in the user file
   */
}

/**
  * @brief DeInitialize the SWPMI MSP.
  * @param hswpmi SWPMI handle
  * @retval None
  */
__weak void HAL_SWPMI_MspDeInit(SWPMI_HandleTypeDef *hswpmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hswpmi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SWPMI_MspDeInit can be implemented in the user file
   */
}

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a user SWPMI callback
  *         to be used instead of the weak predefined callback.
  * @param  hswpmi SWPMI handle.
  * @param  CallbackID ID of the callback to be registered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_SWPMI_RX_COMPLETE_CB_ID receive complete callback ID.
  *           @arg @ref HAL_SWPMI_RX_HALFCOMPLETE_CB_ID receive half complete callback ID.
  *           @arg @ref HAL_SWPMI_TX_COMPLETE_CB_ID transmit complete callback ID.
  *           @arg @ref HAL_SWPMI_TX_HALFCOMPLETE_CB_ID transmit half complete callback ID.
  *           @arg @ref HAL_SWPMI_ERROR_CB_ID error callback ID.
  *           @arg @ref HAL_SWPMI_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_SWPMI_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @param  pCallback pointer to the callback function.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_SWPMI_RegisterCallback(SWPMI_HandleTypeDef        *hswpmi,
                                           HAL_SWPMI_CallbackIDTypeDef CallbackID,
                                           pSWPMI_CallbackTypeDef      pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    /* update the error code */
    hswpmi->ErrorCode |= HAL_SWPMI_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  else
  {
    if(hswpmi->State == HAL_SWPMI_STATE_READY)
    {
      switch (CallbackID)
      {
      case HAL_SWPMI_RX_COMPLETE_CB_ID :
        hswpmi->RxCpltCallback = pCallback;
        break;
      case HAL_SWPMI_RX_HALFCOMPLETE_CB_ID :
        hswpmi->RxHalfCpltCallback = pCallback;
        break;
      case HAL_SWPMI_TX_COMPLETE_CB_ID :
        hswpmi->TxCpltCallback = pCallback;
        break;
      case HAL_SWPMI_TX_HALFCOMPLETE_CB_ID :
        hswpmi->TxHalfCpltCallback = pCallback;
        break;
      case HAL_SWPMI_ERROR_CB_ID :
        hswpmi->ErrorCallback = pCallback;
        break;
      case HAL_SWPMI_MSPINIT_CB_ID :
        hswpmi->MspInitCallback = pCallback;
        break;
      case HAL_SWPMI_MSPDEINIT_CB_ID :
        hswpmi->MspDeInitCallback = pCallback;
        break;
      default :
        /* update the error code */
        hswpmi->ErrorCode |= HAL_SWPMI_ERROR_INVALID_CALLBACK;
        /* update return status */
        status = HAL_ERROR;
        break;
      }
    }
    else if(hswpmi->State == HAL_SWPMI_STATE_RESET)
    {
      switch (CallbackID)
      {
      case HAL_SWPMI_MSPINIT_CB_ID :
        hswpmi->MspInitCallback = pCallback;
        break;
      case HAL_SWPMI_MSPDEINIT_CB_ID :
        hswpmi->MspDeInitCallback = pCallback;
        break;
      default :
        /* update the error code */
        hswpmi->ErrorCode |= HAL_SWPMI_ERROR_INVALID_CALLBACK;
        /* update return status */
        status = HAL_ERROR;
        break;
      }
    }
    else
    {
      /* update the error code */
      hswpmi->ErrorCode |= HAL_SWPMI_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
    }
  }
  return status;
}

/**
  * @brief  Unregister a user SWPMI callback.
  *         SWPMI callback is redirected to the weak predefined callback.
  * @param  hswpmi SWPMI handle.
  * @param  CallbackID ID of the callback to be unregistered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_SWPMI_RX_COMPLETE_CB_ID receive complete callback ID.
  *           @arg @ref HAL_SWPMI_RX_HALFCOMPLETE_CB_ID receive half complete callback ID.
  *           @arg @ref HAL_SWPMI_TX_COMPLETE_CB_ID transmit complete callback ID.
  *           @arg @ref HAL_SWPMI_TX_HALFCOMPLETE_CB_ID transmit half complete callback ID.
  *           @arg @ref HAL_SWPMI_ERROR_CB_ID error callback ID.
  *           @arg @ref HAL_SWPMI_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_SWPMI_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_SWPMI_UnRegisterCallback(SWPMI_HandleTypeDef        *hswpmi,
                                             HAL_SWPMI_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(hswpmi->State == HAL_SWPMI_STATE_READY)
  {
    switch (CallbackID)
    {
    case HAL_SWPMI_RX_COMPLETE_CB_ID :
      hswpmi->RxCpltCallback = HAL_SWPMI_RxCpltCallback;
      break;
    case HAL_SWPMI_RX_HALFCOMPLETE_CB_ID :
      hswpmi->RxHalfCpltCallback = HAL_SWPMI_RxHalfCpltCallback;
      break;
    case HAL_SWPMI_TX_COMPLETE_CB_ID :
      hswpmi->TxCpltCallback = HAL_SWPMI_TxCpltCallback;
      break;
    case HAL_SWPMI_TX_HALFCOMPLETE_CB_ID :
      hswpmi->TxHalfCpltCallback = HAL_SWPMI_TxHalfCpltCallback;
      break;
    case HAL_SWPMI_ERROR_CB_ID :
      hswpmi->ErrorCallback = HAL_SWPMI_ErrorCallback;
      break;
    case HAL_SWPMI_MSPINIT_CB_ID :
      hswpmi->MspInitCallback = HAL_SWPMI_MspInit;
      break;
    case HAL_SWPMI_MSPDEINIT_CB_ID :
      hswpmi->MspDeInitCallback = HAL_SWPMI_MspDeInit;
      break;
    default :
      /* update the error code */
      hswpmi->ErrorCode |= HAL_SWPMI_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
      break;
    }
  }
  else if(hswpmi->State == HAL_SWPMI_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_SWPMI_MSPINIT_CB_ID :
      hswpmi->MspInitCallback = HAL_SWPMI_MspInit;
      break;
    case HAL_SWPMI_MSPDEINIT_CB_ID :
      hswpmi->MspDeInitCallback = HAL_SWPMI_MspDeInit;
      break;
    default :
      /* update the error code */
      hswpmi->ErrorCode |= HAL_SWPMI_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
      break;
    }
  }
  else
  {
    /* update the error code */
    hswpmi->ErrorCode |= HAL_SWPMI_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  return status;
}
#endif /* USE_HAL_SWPMI_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup SWPMI_Exported_Group2 IO operation methods
  *  @brief SWPMI Transmit/Receive functions
  *
@verbatim
 ===============================================================================
                      ##### IO operation methods #####
 ===============================================================================
 [..]
    This subsection provides a set of functions allowing to manage the SWPMI
     data transfers.

    (#) There are two modes of transfer:
       (++) Blocking mode: The communication is performed in polling mode.
            The HAL status of all data processing is returned by the same function
            after finishing transfer.
       (++) Non-Blocking mode: The communication is performed using Interrupts
           or DMA. The end of the data processing will be indicated through the
           dedicated SWPMI Interrupt handler (HAL_SWPMI_IRQHandler()) when using Interrupt mode or
           the selected DMA channel interrupt handler when using DMA mode.
           The HAL_SWPMI_TxCpltCallback(), HAL_SWPMI_RxCpltCallback() user callbacks
           will be executed respectively at the end of the transmit or receive process.
           The HAL_SWPMI_ErrorCallback() user callback will be executed when a communication error is detected.

    (#) Blocking mode API's are:
        (++) HAL_SWPMI_Transmit()
        (++) HAL_SWPMI_Receive()

    (#) Non-Blocking mode API's with Interrupt are:
        (++) HAL_SWPMI_Transmit_IT()
        (++) HAL_SWPMI_Receive_IT()
        (++) HAL_SWPMI_IRQHandler()

    (#) Non-Blocking mode API's with DMA are:
        (++) HAL_SWPMI_Transmit_DMA()
        (++) HAL_SWPMI_Receive_DMA()
        (++) HAL_SWPMI_DMAPause()
        (++) HAL_SWPMI_DMAResume()
        (++) HAL_SWPMI_DMAStop()

    (#) A set of Transfer Complete Callbacks are provided in Non-Blocking mode:
        (++) HAL_SWPMI_TxHalfCpltCallback()
        (++) HAL_SWPMI_TxCpltCallback()
        (++) HAL_SWPMI_RxHalfCpltCallback()
        (++) HAL_SWPMI_RxCpltCallback()
        (++) HAL_SWPMI_ErrorCallback()

    (#) The capability to launch the above IO operations in loopback mode for
        user application verification:
        (++) HAL_SWPMI_EnableLoopback()
        (++) HAL_SWPMI_DisableLoopback()

@endverbatim
  * @{
  */

/**
  * @brief  Transmit an amount of data in blocking mode.
  * @param  hswpmi pointer to a SWPMI_HandleTypeDef structure that contains
  *                the configuration information for SWPMI module.
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_Transmit(SWPMI_HandleTypeDef *hswpmi, uint32_t* pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SWPMI_StateTypeDef tmp_state;
  uint32_t *ptmp_data;
  uint32_t tmp_size;

  if((pData == NULL ) || (Size == 0U))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Process Locked */
    __HAL_LOCK(hswpmi);

    tmp_state = hswpmi->State;
    if((tmp_state == HAL_SWPMI_STATE_READY) || (tmp_state == HAL_SWPMI_STATE_BUSY_RX))
    {
      /* Check if a non-blocking receive process is ongoing or not */
      if(tmp_state == HAL_SWPMI_STATE_READY)
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX;

        /* Disable any transmitter interrupts */
        __HAL_SWPMI_DISABLE_IT(hswpmi, SWPMI_IT_TCIE | SWPMI_IT_TIE | SWPMI_IT_TXUNRIE | SWPMI_IT_TXBEIE);

        /* Disable any transmitter flags */
        __HAL_SWPMI_CLEAR_FLAG(hswpmi, SWPMI_FLAG_TXBEF | SWPMI_FLAG_TXUNRF | SWPMI_FLAG_TCF);

        /* Enable SWPMI peripheral if not */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);
      }
      else
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX_RX;
      }

      ptmp_data = pData;
      tmp_size = Size;
      do
      {
        /* Wait the TXE to write data */
        if(HAL_IS_BIT_SET(hswpmi->Instance->ISR, SWPMI_FLAG_TXE))
        {
          hswpmi->Instance->TDR = *ptmp_data;
          ptmp_data++;
          tmp_size--;
        }
        else
        {
          /* Check for the Timeout */
          if(Timeout != HAL_MAX_DELAY)
          {
            if(((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U))
            {
              status = HAL_TIMEOUT;
              break;
            }
          }
        }
      } while(tmp_size != 0U);

      /* Wait on TXBEF flag to be able to start a second transfer */
      if(SWPMI_WaitOnFlagSetUntilTimeout(hswpmi, SWPMI_FLAG_TXBEF, tickstart, Timeout) != HAL_OK)
      {
        /* Timeout occurred */
        hswpmi->ErrorCode |= HAL_SWPMI_ERROR_TXBEF_TIMEOUT;

        status = HAL_TIMEOUT;
      }

      if(status == HAL_OK)
      {
        /* Check if a non-blocking receive Process is ongoing or not */
        if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX_RX)
        {
          hswpmi->State = HAL_SWPMI_STATE_BUSY_RX;
        }
        else
        {
          hswpmi->State = HAL_SWPMI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_BUSY;
    }
  }

  if((status != HAL_OK) && (status != HAL_BUSY))
  {
    hswpmi->State = HAL_SWPMI_STATE_READY;
  }
  /* Process Unlocked */
  __HAL_UNLOCK(hswpmi);

  return status;
}

/**
  * @brief  Receive an amount of data in blocking mode.
  * @param  hswpmi pointer to a SWPMI_HandleTypeDef structure that contains
  *                the configuration information for SWPMI module.
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be received
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_Receive(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SWPMI_StateTypeDef tmp_state;
  uint32_t *ptmp_data;
  uint32_t tmp_size;

  if((pData == NULL ) || (Size == 0U))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Process Locked */
    __HAL_LOCK(hswpmi);

    tmp_state = hswpmi->State;
    if((tmp_state == HAL_SWPMI_STATE_READY) || (tmp_state == HAL_SWPMI_STATE_BUSY_TX))
    {
      /* Check if a non-blocking transmit process is ongoing or not */
      if(tmp_state == HAL_SWPMI_STATE_READY)
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_RX;

        /* Disable any receiver interrupts */
        CLEAR_BIT(hswpmi->Instance->IER, SWPMI_IT_SRIE | SWPMI_IT_RIE | SWPMI_IT_RXBERIE | SWPMI_IT_RXOVRIE | SWPMI_IT_RXBFIE);

        /* Enable SWPMI peripheral if not */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);
      }
      else
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX_RX;
      }

      ptmp_data = pData;
      tmp_size = Size;
      do
      {
        /* Wait the RXNE to read data */
        if(HAL_IS_BIT_SET(hswpmi->Instance->ISR, SWPMI_FLAG_RXNE))
        {
          *ptmp_data = hswpmi->Instance->RDR;
          ptmp_data++;
          tmp_size--;
        }
        else
        {
          /* Check for the Timeout */
          if(Timeout != HAL_MAX_DELAY)
          {
            if(((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U))
            {
              status = HAL_TIMEOUT;
              break;
            }
          }
        }
      } while(tmp_size != 0U);

      if(status == HAL_OK)
      {
        if(HAL_IS_BIT_SET(hswpmi->Instance->ISR, SWPMI_FLAG_RXBFF))
        {
          /* Clear RXBFF at end of reception */
          WRITE_REG(hswpmi->Instance->ICR, SWPMI_FLAG_RXBFF);
        }

        /* Check if a non-blocking transmit Process is ongoing or not */
        if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX_RX)
        {
          hswpmi->State = HAL_SWPMI_STATE_BUSY_TX;
        }
        else
        {
          hswpmi->State = HAL_SWPMI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_BUSY;
    }
  }

  if((status != HAL_OK) && (status != HAL_BUSY))
  {
    hswpmi->State = HAL_SWPMI_STATE_READY;
  }
  /* Process Unlocked */
  __HAL_UNLOCK(hswpmi);

  return status;
}

/**
  * @brief  Transmit an amount of data in non-blocking mode with interrupt.
  * @param  hswpmi pointer to a SWPMI_HandleTypeDef structure that contains
  *                the configuration information for SWPMI module.
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_Transmit_IT(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SWPMI_StateTypeDef tmp_state;

  if((pData == NULL ) || (Size == 0U))
  {
    status =  HAL_ERROR;
  }
  else
  {
    /* Process Locked */
    __HAL_LOCK(hswpmi);

    tmp_state = hswpmi->State;
    if((tmp_state == HAL_SWPMI_STATE_READY) || (tmp_state == HAL_SWPMI_STATE_BUSY_RX))
    {
      /* Update handle */
      hswpmi->pTxBuffPtr = pData;
      hswpmi->TxXferSize = Size;
      hswpmi->TxXferCount = Size;
      hswpmi->ErrorCode = HAL_SWPMI_ERROR_NONE;

      /* Check if a receive process is ongoing or not */
      if(tmp_state == HAL_SWPMI_STATE_READY)
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX;

        /* Enable SWPMI peripheral if not */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);
      }
      else
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX_RX;
      }

      /* Enable the SWPMI transmit underrun error */
      __HAL_SWPMI_ENABLE_IT(hswpmi, SWPMI_IT_TXUNRIE);

      /* Process Unlocked */
      __HAL_UNLOCK(hswpmi);

      /* Enable the SWPMI interrupts:      */
      /* - Transmit data register empty    */
      /* - Transmit buffer empty           */
      /* - Transmit/Reception completion   */
      __HAL_SWPMI_ENABLE_IT(hswpmi, SWPMI_IT_TIE | SWPMI_IT_TXBEIE | SWPMI_IT_TCIE);
    }
    else
    {
      status =  HAL_BUSY;

      /* Process Unlocked */
      __HAL_UNLOCK(hswpmi);
    }
  }

  return status;
}

/**
  * @brief  Receive an amount of data in non-blocking mode with interrupt.
  * @param  hswpmi SWPMI handle
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be received
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_Receive_IT(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SWPMI_StateTypeDef tmp_state;

  if((pData == NULL ) || (Size == 0U))
  {
    status =  HAL_ERROR;
  }
  else
  {
    /* Process Locked */
    __HAL_LOCK(hswpmi);

    tmp_state = hswpmi->State;
    if((tmp_state == HAL_SWPMI_STATE_READY) || (tmp_state == HAL_SWPMI_STATE_BUSY_TX))
    {
      /* Update handle */
      hswpmi->pRxBuffPtr = pData;
      hswpmi->RxXferSize = Size;
      hswpmi->RxXferCount = Size;
      hswpmi->ErrorCode = HAL_SWPMI_ERROR_NONE;

      /* Check if a transmit process is ongoing or not */
      if(tmp_state == HAL_SWPMI_STATE_READY)
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_RX;

        /* Enable SWPMI peripheral if not */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);
      }
      else
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX_RX;
      }

      /* Process Unlocked */
      __HAL_UNLOCK(hswpmi);

      /* Enable the SWPMI slave resume */
      /* Enable the SWPMI Data Register not empty Interrupt, receive CRC Error, receive overrun and RxBuf Interrupt */
      /*  Enable the SWPMI Transmit/Reception completion   */
      __HAL_SWPMI_ENABLE_IT(hswpmi, SWPMI_IT_RIE | SWPMI_IT_RXBERIE | SWPMI_IT_RXOVRIE | SWPMI_IT_RXBFIE);
    }
    else
    {
      status = HAL_BUSY;

      /* Process Unlocked */
      __HAL_UNLOCK(hswpmi);
    }
  }

  return status;
}

/**
  * @brief  Transmit an amount of data in non-blocking mode with DMA interrupt.
  * @param  hswpmi SWPMI handle
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_Transmit_DMA(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SWPMI_StateTypeDef tmp_state;

  if((pData == NULL ) || (Size == 0U))
  {
    status =  HAL_ERROR;
  }
  else
  {
    /* Process Locked */
    __HAL_LOCK(hswpmi);

    tmp_state = hswpmi->State;
    if((tmp_state == HAL_SWPMI_STATE_READY) || (tmp_state == HAL_SWPMI_STATE_BUSY_RX))
    {
      /* Update handle */
      hswpmi->pTxBuffPtr = pData;
      hswpmi->TxXferSize = Size;
      hswpmi->TxXferCount = Size;
      hswpmi->ErrorCode = HAL_SWPMI_ERROR_NONE;

      /* Check if a receive process is ongoing or not */
      if(tmp_state == HAL_SWPMI_STATE_READY)
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX;

        /* Enable SWPMI peripheral if not */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);
      }
      else
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX_RX;
      }

      /* Set the SWPMI DMA transfer complete callback */
      hswpmi->hdmatx->XferCpltCallback = SWPMI_DMATransmitCplt;

      /* Set the SWPMI DMA Half transfer complete callback */
      hswpmi->hdmatx->XferHalfCpltCallback = SWPMI_DMATxHalfCplt;

      /* Set the DMA error callback */
      hswpmi->hdmatx->XferErrorCallback = SWPMI_DMAError;

      /* Enable the SWPMI transmit DMA channel */
      if(HAL_DMA_Start_IT(hswpmi->hdmatx, (uint32_t)hswpmi->pTxBuffPtr, (uint32_t)&hswpmi->Instance->TDR, Size) != HAL_OK)
      {
        hswpmi->State = tmp_state;    /* Back to previous state */
        hswpmi->ErrorCode = HAL_SWPMI_ERROR_DMA;
        status = HAL_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hswpmi);
      }
      else
      {
        /* Process Unlocked */
        __HAL_UNLOCK(hswpmi);

        /* Enable the SWPMI transmit underrun error */
        __HAL_SWPMI_ENABLE_IT(hswpmi, SWPMI_IT_TXUNRIE);

        /* Enable the DMA transfer for transmit request by setting the TXDMA bit
           in the SWPMI CR register */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_TXDMA);
      }
    }
    else
    {
      status = HAL_BUSY;

      /* Process Unlocked */
      __HAL_UNLOCK(hswpmi);
    }
  }

  return status;
}

/**
  * @brief  Receive an amount of data in non-blocking mode with DMA interrupt.
  * @param  hswpmi SWPMI handle
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be received
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_Receive_DMA(SWPMI_HandleTypeDef *hswpmi, uint32_t *pData, uint16_t Size)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_SWPMI_StateTypeDef tmp_state;

  if((pData == NULL ) || (Size == 0U))
  {
    status =  HAL_ERROR;
  }
  else
  {
    /* Process Locked */
    __HAL_LOCK(hswpmi);

    tmp_state = hswpmi->State;
    if((tmp_state == HAL_SWPMI_STATE_READY) || (tmp_state == HAL_SWPMI_STATE_BUSY_TX))
    {
      /* Update handle */
      hswpmi->pRxBuffPtr = pData;
      hswpmi->RxXferSize = Size;
      hswpmi->ErrorCode = HAL_SWPMI_ERROR_NONE;

      /* Check if a transmit process is ongoing or not */
      if(tmp_state == HAL_SWPMI_STATE_READY)
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_RX;

        /* Enable SWPMI peripheral if not */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);
      }
      else
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_TX_RX;
      }

      /* Set the SWPMI DMA transfer complete callback */
      hswpmi->hdmarx->XferCpltCallback = SWPMI_DMAReceiveCplt;

      /* Set the SWPMI DMA Half transfer complete callback */
      hswpmi->hdmarx->XferHalfCpltCallback = SWPMI_DMARxHalfCplt;

      /* Set the DMA error callback */
      hswpmi->hdmarx->XferErrorCallback = SWPMI_DMAError;

      /* Enable the DMA request */
      if(HAL_DMA_Start_IT(hswpmi->hdmarx, (uint32_t)&hswpmi->Instance->RDR, (uint32_t)hswpmi->pRxBuffPtr, Size) != HAL_OK)
      {
        hswpmi->State = tmp_state;    /* Back to previous state */
        hswpmi->ErrorCode = HAL_SWPMI_ERROR_DMA;
        status = HAL_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hswpmi);
      }
      else
      {
        /* Process Unlocked */
        __HAL_UNLOCK(hswpmi);

        /* Enable the SWPMI receive CRC Error and receive overrun interrupts */
        __HAL_SWPMI_ENABLE_IT(hswpmi, SWPMI_IT_RXBERIE | SWPMI_IT_RXOVRIE);

        /* Enable the DMA transfer for the receiver request by setting the RXDMA bit
           in the SWPMI CR register */
        SET_BIT(hswpmi->Instance->CR, SWPMI_CR_RXDMA);
      }
    }
    else
    {
      status = HAL_BUSY;

      /* Process Unlocked */
      __HAL_UNLOCK(hswpmi);
    }
  }

  return status;
}

/**
  * @brief Stop all DMA transfers.
  * @param hswpmi SWPMI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SWPMI_DMAStop(SWPMI_HandleTypeDef *hswpmi)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process Locked */
  __HAL_LOCK(hswpmi);

  /* Disable the SWPMI Tx/Rx DMA requests */
  CLEAR_BIT(hswpmi->Instance->CR, (SWPMI_CR_TXDMA | SWPMI_CR_RXDMA));

  /* Abort the SWPMI DMA tx channel */
  if(hswpmi->hdmatx != NULL)
  {
    if(HAL_DMA_Abort(hswpmi->hdmatx) != HAL_OK)
    {
      hswpmi->ErrorCode |= HAL_SWPMI_ERROR_DMA;
      status = HAL_ERROR;
    }
  }
  /* Abort the SWPMI DMA rx channel */
  if(hswpmi->hdmarx != NULL)
  {
    if(HAL_DMA_Abort(hswpmi->hdmarx) != HAL_OK)
    {
      hswpmi->ErrorCode |= HAL_SWPMI_ERROR_DMA;
      status = HAL_ERROR;
    }
  }

  /* Disable SWPMI interface */
  CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);

  hswpmi->State = HAL_SWPMI_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hswpmi);

  return status;
}


/**
  * @brief Enable the Loopback mode.
  * @param hswpmi SWPMI handle
  * @note  Loopback mode is to be used only for test purposes
  * @retval HAL_OK / HAL_BUSY
  */
HAL_StatusTypeDef HAL_SWPMI_EnableLoopback(SWPMI_HandleTypeDef *hswpmi)
{
  HAL_StatusTypeDef  status = HAL_OK;

  /* Process Locked */
  __HAL_LOCK(hswpmi);

  /* Make sure the SWPMI interface is not enabled to set the loopback mode */
  CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);

  /* Set Loopback */
  SET_BIT(hswpmi->Instance->CR, SWPMI_CR_LPBK);

  /* Enable SWPMI interface in loopback mode */
  SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);

  /* Process Unlocked */
  __HAL_UNLOCK(hswpmi);

  return status;
}

/**
  * @brief Disable the Loopback mode.
  * @param hswpmi SWPMI handle
  * @note  Loopback mode is to be used only for test purposes
  * @retval HAL_OK / HAL_BUSY
  */
HAL_StatusTypeDef HAL_SWPMI_DisableLoopback(SWPMI_HandleTypeDef *hswpmi)
{
  HAL_StatusTypeDef  status = HAL_OK;

  /* Process Locked */
  __HAL_LOCK(hswpmi);

  /* Make sure the SWPMI interface is not enabled to reset the loopback mode */
  CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);

  /* Reset Loopback */
  CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_LPBK);

  /* Re-enable SWPMI interface in normal mode */
  SET_BIT(hswpmi->Instance->CR, SWPMI_CR_SWPACT);

  /* Process Unlocked */
  __HAL_UNLOCK(hswpmi);

  return status;
}

/**
  * @}
  */

/** @defgroup SWPMI_Exported_Group3 SWPMI IRQ handler and callbacks
 *  @brief  SWPMI  IRQ handler.
 *
@verbatim
  ==============================================================================
                      ##### SWPMI IRQ handler and callbacks  #####
  ==============================================================================
[..]  This section provides SWPMI IRQ handler and callback functions called within
      the IRQ handler.

@endverbatim
  * @{
  */

/**
  * @brief Handle SWPMI interrupt request.
  * @param hswpmi SWPMI handle
  * @retval None
  */
void HAL_SWPMI_IRQHandler(SWPMI_HandleTypeDef *hswpmi)
{
  uint32_t regisr = READ_REG(hswpmi->Instance->ISR);
  uint32_t regier = READ_REG(hswpmi->Instance->IER);
  uint32_t errcode = HAL_SWPMI_ERROR_NONE;

  /* SWPMI CRC error interrupt occurred --------------------------------------*/
  if(((regisr & SWPMI_FLAG_RXBERF) != 0U) && ((regier & SWPMI_IT_RXBERIE) != 0U))
  {
    /* Disable Receive CRC interrupt */
    CLEAR_BIT(hswpmi->Instance->IER, SWPMI_IT_RXBERIE | SWPMI_IT_RXBFIE);
    /* Clear Receive CRC and Receive buffer full flag */
    WRITE_REG(hswpmi->Instance->ICR, SWPMI_FLAG_RXBERF | SWPMI_FLAG_RXBFF);

    errcode |= HAL_SWPMI_ERROR_CRC;
  }

  /* SWPMI Over-Run interrupt occurred -----------------------------------------*/
  if(((regisr & SWPMI_FLAG_RXOVRF) != 0U) && ((regier & SWPMI_IT_RXOVRIE) != 0U))
  {
    /* Disable Receive overrun interrupt */
    CLEAR_BIT(hswpmi->Instance->IER, SWPMI_IT_RXOVRIE);
    /* Clear Receive overrun flag */
    WRITE_REG(hswpmi->Instance->ICR, SWPMI_FLAG_RXOVRF);

    errcode |= HAL_SWPMI_ERROR_OVR;
  }

  /* SWPMI Under-Run interrupt occurred -----------------------------------------*/
  if(((regisr & SWPMI_FLAG_TXUNRF) != 0U) && ((regier & SWPMI_IT_TXUNRIE) != 0U))
  {
    /* Disable Transmit under run interrupt */
    CLEAR_BIT(hswpmi->Instance->IER, SWPMI_IT_TXUNRIE);
    /* Clear Transmit under run flag */
    WRITE_REG(hswpmi->Instance->ICR, SWPMI_FLAG_TXUNRF);

    errcode |= HAL_SWPMI_ERROR_UDR;
  }

   /* Call SWPMI Error Call back function if needed --------------------------*/
  if(errcode != HAL_SWPMI_ERROR_NONE)
  {
    hswpmi->ErrorCode |= errcode;

    if((errcode & HAL_SWPMI_ERROR_UDR) != 0U)
    {
      /* Check TXDMA transfer to abort */
      if(HAL_IS_BIT_SET(hswpmi->Instance->CR, SWPMI_CR_TXDMA))
      {
        /* Disable DMA TX at SWPMI level */
        CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_TXDMA);

        /* Abort the USART DMA Tx channel */
        if(hswpmi->hdmatx != NULL)
        {
          /* Set the SWPMI Tx DMA Abort callback :
             will lead to call HAL_SWPMI_ErrorCallback() at end of DMA abort procedure */
          hswpmi->hdmatx->XferAbortCallback = SWPMI_DMAAbortOnError;
          /* Abort DMA TX */
          if(HAL_DMA_Abort_IT(hswpmi->hdmatx) != HAL_OK)
          {
            /* Call Directly hswpmi->hdmatx->XferAbortCallback function in case of error */
            hswpmi->hdmatx->XferAbortCallback(hswpmi->hdmatx);
          }
        }
        else
        {
          /* Set the SWPMI state ready to be able to start again the process */
          hswpmi->State = HAL_SWPMI_STATE_READY;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
          hswpmi->ErrorCallback(hswpmi);
#else
          HAL_SWPMI_ErrorCallback(hswpmi);
#endif
        }
      }
      else
      {
        /* Set the SWPMI state ready to be able to start again the process */
        hswpmi->State = HAL_SWPMI_STATE_READY;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
        hswpmi->ErrorCallback(hswpmi);
#else
        HAL_SWPMI_ErrorCallback(hswpmi);
#endif
      }
    }
    else
    {
      /* Check RXDMA transfer to abort */
      if(HAL_IS_BIT_SET(hswpmi->Instance->CR, SWPMI_CR_RXDMA))
      {
        /* Disable DMA RX at SWPMI level */
        CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_RXDMA);

        /* Abort the USART DMA Rx channel */
        if(hswpmi->hdmarx != NULL)
        {
          /* Set the SWPMI Rx DMA Abort callback :
             will lead to call HAL_SWPMI_ErrorCallback() at end of DMA abort procedure */
          hswpmi->hdmarx->XferAbortCallback = SWPMI_DMAAbortOnError;
          /* Abort DMA RX */
          if(HAL_DMA_Abort_IT(hswpmi->hdmarx) != HAL_OK)
          {
            /* Call Directly hswpmi->hdmarx->XferAbortCallback function in case of error */
            hswpmi->hdmarx->XferAbortCallback(hswpmi->hdmarx);
          }
        }
        else
        {
          /* Set the SWPMI state ready to be able to start again the process */
          hswpmi->State = HAL_SWPMI_STATE_READY;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
          hswpmi->ErrorCallback(hswpmi);
#else
          HAL_SWPMI_ErrorCallback(hswpmi);
#endif
        }
      }
      else
      {
        /* Set the SWPMI state ready to be able to start again the process */
        hswpmi->State = HAL_SWPMI_STATE_READY;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
        hswpmi->ErrorCallback(hswpmi);
#else
        HAL_SWPMI_ErrorCallback(hswpmi);
#endif
      }
    }
  }

  /* SWPMI in mode Receiver ---------------------------------------------------*/
  if(((regisr & SWPMI_FLAG_RXNE) != 0U) && ((regier & SWPMI_IT_RIE)  != 0U))
  {
    SWPMI_Receive_IT(hswpmi);
  }

  /* SWPMI in mode Transmitter ------------------------------------------------*/
  if(((regisr & SWPMI_FLAG_TXE) != 0U) && ((regier & SWPMI_IT_TIE) != 0U))
  {
    SWPMI_Transmit_IT(hswpmi);
  }

  /* SWPMI in mode Transmitter (Transmit buffer empty) ------------------------*/
  if(((regisr & SWPMI_FLAG_TXBEF) != 0U) && ((regier & SWPMI_IT_TXBEIE) != 0U))
  {
    SWPMI_EndTransmit_IT(hswpmi);
  }

  /* SWPMI in mode Receiver (Receive buffer full) -----------------------------*/
  if(((regisr & SWPMI_FLAG_RXBFF) != 0U) && ((regier & SWPMI_IT_RXBFIE) != 0U))
  {
    SWPMI_EndReceive_IT(hswpmi);
  }

  /* Both Transmission and reception complete ---------------------------------*/
  if(((regisr & SWPMI_FLAG_TCF) != 0U) && ((regier & SWPMI_IT_TCIE) != 0U))
  {
    SWPMI_EndTransmitReceive_IT(hswpmi);
  }
}

/**
  * @brief Tx Transfer completed callback.
  * @param hswpmi SWPMI handle
  * @retval None
  */
__weak void HAL_SWPMI_TxCpltCallback(SWPMI_HandleTypeDef *hswpmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hswpmi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SWPMI_TxCpltCallback is to be implemented in the user file
   */
}

/**
  * @brief  Tx Half Transfer completed callback.
  * @param  hswpmi SWPMI handle
  * @retval None
  */
__weak void HAL_SWPMI_TxHalfCpltCallback(SWPMI_HandleTypeDef *hswpmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hswpmi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_SWPMI_TxHalfCpltCallback is to be implemented in the user file
   */
}

/**
  * @brief Rx Transfer completed callback.
  * @param hswpmi SWPMI handle
  * @retval None
  */
__weak void HAL_SWPMI_RxCpltCallback(SWPMI_HandleTypeDef *hswpmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hswpmi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SWPMI_RxCpltCallback is to be implemented in the user file
   */
}

/**
  * @brief  Rx Half Transfer completed callback.
  * @param  hswpmi SWPMI handle
  * @retval None
  */
__weak void HAL_SWPMI_RxHalfCpltCallback(SWPMI_HandleTypeDef *hswpmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hswpmi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_SWPMI_RxHalfCpltCallback is to be implemented in the user file
   */
}

/**
  * @brief SWPMI error callback.
  * @param hswpmi SWPMI handle
  * @retval None
  */
__weak void HAL_SWPMI_ErrorCallback(SWPMI_HandleTypeDef *hswpmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hswpmi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SWPMI_ErrorCallback is to be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup SWPMI_Exported_Group4 Peripheral Control methods
  *  @brief   SWPMI control functions
  *
@verbatim
 ===============================================================================
                      ##### Peripheral Control methods #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to control the SWPMI.
     (+) HAL_SWPMI_GetState() API is helpful to check in run-time the state of the SWPMI peripheral
     (+) HAL_SWPMI_GetError() API is helpful to check in run-time the error state of the SWPMI peripheral
@endverbatim
  * @{
  */

/**
  * @brief Return the SWPMI handle state.
  * @param hswpmi SWPMI handle
  * @retval HAL state
  */
HAL_SWPMI_StateTypeDef HAL_SWPMI_GetState(SWPMI_HandleTypeDef *hswpmi)
{
  /* Return SWPMI handle state */
  return hswpmi->State;
}

/**
* @brief  Return the SWPMI error code.
* @param  hswpmi : pointer to a SWPMI_HandleTypeDef structure that contains
  *              the configuration information for the specified SWPMI.
* @retval SWPMI Error Code
*/
uint32_t HAL_SWPMI_GetError(SWPMI_HandleTypeDef *hswpmi)
{
  return hswpmi->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/** @defgroup SWPMI_Private_Functions SWPMI Private Functions
  * @{
  */

/**
  * @brief Transmit an amount of data in interrupt mode.
  * @note  Function called under interruption only, once interruptions have been enabled by HAL_SWPMI_Transmit_IT()
  * @param  hswpmi SWPMI handle
  * @retval None
  */
static void SWPMI_Transmit_IT(SWPMI_HandleTypeDef *hswpmi)
{
  HAL_SWPMI_StateTypeDef tmp_state = hswpmi->State;

  if ((tmp_state == HAL_SWPMI_STATE_BUSY_TX) || (tmp_state == HAL_SWPMI_STATE_BUSY_TX_RX))
  {
    if(hswpmi->TxXferCount == 0U)
    {
      /* Disable the SWPMI TXE and Underrun Interrupts */
      CLEAR_BIT(hswpmi->Instance->IER, (SWPMI_IT_TIE | SWPMI_IT_TXUNRIE));
    }
    else
    {
      hswpmi->Instance->TDR = (uint32_t)*hswpmi->pTxBuffPtr;
      hswpmi->pTxBuffPtr++;
      hswpmi->TxXferCount--;
    }
  }
  else
  {
    /* nothing to do */
  }
}

/**
  * @brief  Wraps up transmission in non-blocking mode.
  * @param  hswpmi SWPMI handle
  * @retval None
  */
static void SWPMI_EndTransmit_IT(SWPMI_HandleTypeDef *hswpmi)
{
  /* Clear the SWPMI Transmit buffer empty Flag */
  WRITE_REG(hswpmi->Instance->ICR, SWPMI_FLAG_TXBEF);
  /* Disable the all SWPMI Transmit Interrupts  */
  CLEAR_BIT(hswpmi->Instance->IER, SWPMI_IT_TIE | SWPMI_IT_TXUNRIE | SWPMI_IT_TXBEIE);

  /* Check if a receive Process is ongoing or not */
  if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX_RX)
  {
    hswpmi->State = HAL_SWPMI_STATE_BUSY_RX;
  }
  else
  {
    hswpmi->State = HAL_SWPMI_STATE_READY;
  }

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
  hswpmi->TxCpltCallback(hswpmi);
#else
  HAL_SWPMI_TxCpltCallback(hswpmi);
#endif
}

/**
  * @brief Receive an amount of data in interrupt mode.
  * @note  Function called under interruption only, once interruptions have been enabled by HAL_SWPMI_Receive_IT()
  * @param  hswpmi SWPMI handle
  * @retval None
  */
static void SWPMI_Receive_IT(SWPMI_HandleTypeDef *hswpmi)
{
  HAL_SWPMI_StateTypeDef tmp_state = hswpmi->State;

  if((tmp_state == HAL_SWPMI_STATE_BUSY_RX) || (tmp_state == HAL_SWPMI_STATE_BUSY_TX_RX))
  {
    *hswpmi->pRxBuffPtr = (uint32_t)(hswpmi->Instance->RDR);
    hswpmi->pRxBuffPtr++;

    --hswpmi->RxXferCount;
    if(hswpmi->RxXferCount == 0U)
    {
      /* Wait for RXBFF flag to update state */
#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
      hswpmi->RxCpltCallback(hswpmi);
#else
      HAL_SWPMI_RxCpltCallback(hswpmi);
#endif
    }
  }
  else
  {
    /* nothing to do */
  }
}

/**
  * @brief  Wraps up reception in non-blocking mode.
  * @param  hswpmi SWPMI handle
  * @retval None
  */
static void SWPMI_EndReceive_IT(SWPMI_HandleTypeDef *hswpmi)
{
  /* Clear the SWPMI Receive buffer full Flag */
  WRITE_REG(hswpmi->Instance->ICR, SWPMI_FLAG_RXBFF);
  /* Disable the all SWPMI Receive Interrupts  */
  CLEAR_BIT(hswpmi->Instance->IER, SWPMI_IT_RIE | SWPMI_IT_RXBERIE | SWPMI_IT_RXOVRIE | SWPMI_IT_RXBFIE);

  /* Check if a transmit Process is ongoing or not */
  if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX_RX)
  {
    hswpmi->State = HAL_SWPMI_STATE_BUSY_TX;
  }
  else
  {
    hswpmi->State = HAL_SWPMI_STATE_READY;
  }
}

/**
  * @brief  Wraps up transmission and reception in non-blocking mode.
  * @param  hswpmi SWPMI handle
  * @retval None
  */
static void SWPMI_EndTransmitReceive_IT(SWPMI_HandleTypeDef *hswpmi)
{
  /* Clear the SWPMI Transmission Complete Flag */
  WRITE_REG(hswpmi->Instance->ICR, SWPMI_FLAG_TCF);
  /* Disable the SWPMI Transmission  Complete Interrupt */
  CLEAR_BIT(hswpmi->Instance->IER, SWPMI_IT_TCIE);

  /* Check if a receive Process is ongoing or not */
  if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX_RX)
  {
    hswpmi->State = HAL_SWPMI_STATE_BUSY_RX;
  }
  else if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX)
  {
    hswpmi->State = HAL_SWPMI_STATE_READY;
  }
  else
  {
    /* nothing to do */
  }
}

/**
  * @brief DMA SWPMI transmit process complete callback.
  * @param hdma DMA handle
  * @retval None
  */
static void SWPMI_DMATransmitCplt(DMA_HandleTypeDef *hdma)
{
  SWPMI_HandleTypeDef* hswpmi = ( SWPMI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  uint32_t tickstart;

  /* DMA Normal mode*/
  if((hdma->Instance->CCR & DMA_CCR_CIRC) == 0U)
  {
    hswpmi->TxXferCount = 0U;

    /* Disable the DMA transfer for transmit request by setting the TXDMA bit
    in the SWPMI CR register */
    CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_TXDMA);

    /* Init tickstart for timeout managment*/
    tickstart = HAL_GetTick();

    /* Wait the TXBEF */
    if(SWPMI_WaitOnFlagSetUntilTimeout(hswpmi, SWPMI_FLAG_TXBEF, tickstart, SWPMI_TIMEOUT_VALUE) != HAL_OK)
    {
      /* Timeout occurred */
      hswpmi->ErrorCode |= HAL_SWPMI_ERROR_TXBEF_TIMEOUT;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
      hswpmi->ErrorCallback(hswpmi);
#else
      HAL_SWPMI_ErrorCallback(hswpmi);
#endif
    }
    else
    {
      /* No Timeout */
      /* Check if a receive process is ongoing or not */
      if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX_RX)
      {
        hswpmi->State = HAL_SWPMI_STATE_BUSY_RX;
      }
      else
      {
        hswpmi->State = HAL_SWPMI_STATE_READY;
      }

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
      hswpmi->TxCpltCallback(hswpmi);
#else
      HAL_SWPMI_TxCpltCallback(hswpmi);
#endif
    }
  }
  /* DMA Circular mode */
  else
  {
#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
    hswpmi->TxCpltCallback(hswpmi);
#else
    HAL_SWPMI_TxCpltCallback(hswpmi);
#endif
  }
}

/**
  * @brief DMA SWPMI transmit process half complete callback.
  * @param hdma DMA handle
  * @retval None
  */
static void SWPMI_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
  SWPMI_HandleTypeDef* hswpmi = (SWPMI_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
  hswpmi->TxHalfCpltCallback(hswpmi);
#else
  HAL_SWPMI_TxHalfCpltCallback(hswpmi);
#endif
}


/**
  * @brief DMA SWPMI receive process complete callback.
  * @param hdma DMA handle
  * @retval None
  */
static void SWPMI_DMAReceiveCplt(DMA_HandleTypeDef *hdma)
{
  SWPMI_HandleTypeDef* hswpmi = ( SWPMI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

  /* DMA Normal mode*/
  if((hdma->Instance->CCR & DMA_CCR_CIRC) == 0U)
  {
    hswpmi->RxXferCount = 0U;

    /* Disable the DMA transfer for the receiver request by setting the RXDMA bit
    in the SWPMI CR register */
    CLEAR_BIT(hswpmi->Instance->CR, SWPMI_CR_RXDMA);

    /* Check if a transmit Process is ongoing or not */
    if(hswpmi->State == HAL_SWPMI_STATE_BUSY_TX_RX)
    {
      hswpmi->State = HAL_SWPMI_STATE_BUSY_TX;
    }
    else
    {
      hswpmi->State = HAL_SWPMI_STATE_READY;
    }
  }
#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
  hswpmi->RxCpltCallback(hswpmi);
#else
  HAL_SWPMI_RxCpltCallback(hswpmi);
#endif
}

/**
  * @brief DMA SWPMI receive process half complete callback.
  * @param hdma DMA handle
  * @retval None
  */
static void SWPMI_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
  SWPMI_HandleTypeDef* hswpmi = (SWPMI_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
  hswpmi->RxHalfCpltCallback(hswpmi);
#else
  HAL_SWPMI_RxHalfCpltCallback(hswpmi);
#endif
}

/**
  * @brief DMA SWPMI communication error callback.
  * @param hdma DMA handle
  * @retval None
  */
static void SWPMI_DMAError(DMA_HandleTypeDef *hdma)
{
  SWPMI_HandleTypeDef* hswpmi = ( SWPMI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

  /* Update handle */
  hswpmi->RxXferCount = 0U;
  hswpmi->TxXferCount = 0U;
  hswpmi->State= HAL_SWPMI_STATE_READY;
  hswpmi->ErrorCode |= HAL_SWPMI_ERROR_DMA;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
  hswpmi->ErrorCallback(hswpmi);
#else
  HAL_SWPMI_ErrorCallback(hswpmi);
#endif
}

/**
  * @brief DMA SWPMI communication abort callback.
  * @param hdma DMA handle
  * @retval None
  */
static void SWPMI_DMAAbortOnError(DMA_HandleTypeDef *hdma)
{
  SWPMI_HandleTypeDef* hswpmi = ( SWPMI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

  /* Update handle */
  hswpmi->RxXferCount = 0U;
  hswpmi->TxXferCount = 0U;
  hswpmi->State= HAL_SWPMI_STATE_READY;

#if (USE_HAL_SWPMI_REGISTER_CALLBACKS == 1)
  hswpmi->ErrorCallback(hswpmi);
#else
  HAL_SWPMI_ErrorCallback(hswpmi);
#endif
}

/**
  * @brief  Handle SWPMI Communication Timeout.
  * @param  hswpmi SWPMI handle
  * @param  Flag: specifies the SWPMI flag to check.
  * @param  Tickstart Tick start value
  * @param  Timeout timeout duration.
  * @retval HAL status
  */
static HAL_StatusTypeDef SWPMI_WaitOnFlagSetUntilTimeout(SWPMI_HandleTypeDef *hswpmi, uint32_t Flag, uint32_t Tickstart, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Wait until flag is set */
  while(!(HAL_IS_BIT_SET(hswpmi->Instance->ISR, Flag)))
  {
    /* Check for the Timeout */
    if ((((HAL_GetTick() - Tickstart) >  Timeout) && (Timeout != HAL_MAX_DELAY)) || (Timeout == 0U))
    {
      /* Set the SWPMI state ready to be able to start again the process */
      hswpmi->State = HAL_SWPMI_STATE_READY;

      status = HAL_TIMEOUT;
      break;
    }
  }

  return status;
}

/**
  * @}
  */

#endif /* HAL_SWPMI_MODULE_ENABLED */

/**
  * @}
  */

#endif /* SWPMI1 */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
