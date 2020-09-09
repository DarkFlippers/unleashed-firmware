/**
  ******************************************************************************
  * @file    stm32l4xx_hal_uart.c
  * @author  MCD Application Team
  * @brief   UART HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Universal Asynchronous Receiver Transmitter Peripheral (UART).
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *
  *
  @verbatim
 ===============================================================================
                        ##### How to use this driver #####
 ===============================================================================
  [..]
    The UART HAL driver can be used as follows:

    (#) Declare a UART_HandleTypeDef handle structure (eg. UART_HandleTypeDef huart).
    (#) Initialize the UART low level resources by implementing the HAL_UART_MspInit() API:
        (++) Enable the USARTx interface clock.
        (++) UART pins configuration:
            (+++) Enable the clock for the UART GPIOs.
            (+++) Configure these UART pins as alternate function pull-up.
        (++) NVIC configuration if you need to use interrupt process (HAL_UART_Transmit_IT()
             and HAL_UART_Receive_IT() APIs):
            (+++) Configure the USARTx interrupt priority.
            (+++) Enable the NVIC USART IRQ handle.
        (++) UART interrupts handling:
              -@@-  The specific UART interrupts (Transmission complete interrupt,
                RXNE interrupt, RX/TX FIFOs related interrupts and Error Interrupts)
                are managed using the macros __HAL_UART_ENABLE_IT() and __HAL_UART_DISABLE_IT()
                inside the transmit and receive processes.
        (++) DMA Configuration if you need to use DMA process (HAL_UART_Transmit_DMA()
             and HAL_UART_Receive_DMA() APIs):
            (+++) Declare a DMA handle structure for the Tx/Rx channel.
            (+++) Enable the DMAx interface clock.
            (+++) Configure the declared DMA handle structure with the required Tx/Rx parameters.
            (+++) Configure the DMA Tx/Rx channel.
            (+++) Associate the initialized DMA handle to the UART DMA Tx/Rx handle.
            (+++) Configure the priority and enable the NVIC for the transfer complete interrupt on the DMA Tx/Rx channel.

    (#) Program the Baud Rate, Word Length, Stop Bit, Parity, Prescaler value , Hardware
        flow control and Mode (Receiver/Transmitter) in the huart handle Init structure.

    (#) If required, program UART advanced features (TX/RX pins swap, auto Baud rate detection,...)
        in the huart handle AdvancedInit structure.

    (#) For the UART asynchronous mode, initialize the UART registers by calling
        the HAL_UART_Init() API.

    (#) For the UART Half duplex mode, initialize the UART registers by calling
        the HAL_HalfDuplex_Init() API.

    (#) For the UART LIN (Local Interconnection Network) mode, initialize the UART registers
        by calling the HAL_LIN_Init() API.

    (#) For the UART Multiprocessor mode, initialize the UART registers
        by calling the HAL_MultiProcessor_Init() API.

    (#) For the UART RS485 Driver Enabled mode, initialize the UART registers
        by calling the HAL_RS485Ex_Init() API.

    [..]
    (@) These API's (HAL_UART_Init(), HAL_HalfDuplex_Init(), HAL_LIN_Init(), HAL_MultiProcessor_Init(),
        also configure the low level Hardware GPIO, CLOCK, CORTEX...etc) by
        calling the customized HAL_UART_MspInit() API.

    ##### Callback registration #####
    ==================================

    [..]
    The compilation define USE_HAL_UART_REGISTER_CALLBACKS when set to 1
    allows the user to configure dynamically the driver callbacks.

    [..]
    Use Function @ref HAL_UART_RegisterCallback() to register a user callback.
    Function @ref HAL_UART_RegisterCallback() allows to register following callbacks:
    (+) TxHalfCpltCallback        : Tx Half Complete Callback.
    (+) TxCpltCallback            : Tx Complete Callback.
    (+) RxHalfCpltCallback        : Rx Half Complete Callback.
    (+) RxCpltCallback            : Rx Complete Callback.
    (+) ErrorCallback             : Error Callback.
    (+) AbortCpltCallback         : Abort Complete Callback.
    (+) AbortTransmitCpltCallback : Abort Transmit Complete Callback.
    (+) AbortReceiveCpltCallback  : Abort Receive Complete Callback.
    (+) WakeupCallback            : Wakeup Callback.
    (+) RxFifoFullCallback        : Rx Fifo Full Callback.
    (+) TxFifoEmptyCallback       : Tx Fifo Empty Callback.
    (+) MspInitCallback           : UART MspInit.
    (+) MspDeInitCallback         : UART MspDeInit.
    This function takes as parameters the HAL peripheral handle, the Callback ID
    and a pointer to the user callback function.

    [..]
    Use function @ref HAL_UART_UnRegisterCallback() to reset a callback to the default
    weak (surcharged) function.
    @ref HAL_UART_UnRegisterCallback() takes as parameters the HAL peripheral handle,
    and the Callback ID.
    This function allows to reset following callbacks:
    (+) TxHalfCpltCallback        : Tx Half Complete Callback.
    (+) TxCpltCallback            : Tx Complete Callback.
    (+) RxHalfCpltCallback        : Rx Half Complete Callback.
    (+) RxCpltCallback            : Rx Complete Callback.
    (+) ErrorCallback             : Error Callback.
    (+) AbortCpltCallback         : Abort Complete Callback.
    (+) AbortTransmitCpltCallback : Abort Transmit Complete Callback.
    (+) AbortReceiveCpltCallback  : Abort Receive Complete Callback.
    (+) WakeupCallback            : Wakeup Callback.
    (+) RxFifoFullCallback        : Rx Fifo Full Callback.
    (+) TxFifoEmptyCallback       : Tx Fifo Empty Callback.
    (+) MspInitCallback           : UART MspInit.
    (+) MspDeInitCallback         : UART MspDeInit.

    [..]
    By default, after the @ref HAL_UART_Init() and when the state is HAL_UART_STATE_RESET
    all callbacks are set to the corresponding weak (surcharged) functions:
    examples @ref HAL_UART_TxCpltCallback(), @ref HAL_UART_RxHalfCpltCallback().
    Exception done for MspInit and MspDeInit functions that are respectively
    reset to the legacy weak (surcharged) functions in the @ref HAL_UART_Init()
    and @ref HAL_UART_DeInit() only when these callbacks are null (not registered beforehand).
    If not, MspInit or MspDeInit are not null, the @ref HAL_UART_Init() and @ref HAL_UART_DeInit()
    keep and use the user MspInit/MspDeInit callbacks (registered beforehand).

    [..]
    Callbacks can be registered/unregistered in HAL_UART_STATE_READY state only.
    Exception done MspInit/MspDeInit that can be registered/unregistered
    in HAL_UART_STATE_READY or HAL_UART_STATE_RESET state, thus registered (user)
    MspInit/DeInit callbacks can be used during the Init/DeInit.
    In that case first register the MspInit/MspDeInit user callbacks
    using @ref HAL_UART_RegisterCallback() before calling @ref HAL_UART_DeInit()
    or @ref HAL_UART_Init() function.

    [..]
    When The compilation define USE_HAL_UART_REGISTER_CALLBACKS is set to 0 or
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

/** @defgroup UART UART
  * @brief HAL UART module driver
  * @{
  */

#ifdef HAL_UART_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup UART_Private_Constants UART Private Constants
  * @{
  */
#if defined(USART_CR1_FIFOEN)
#define USART_CR1_FIELDS  ((uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | \
                                      USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8| \
                                      USART_CR1_FIFOEN ))                      /*!< UART or USART CR1 fields of parameters set by UART_SetConfig API */
#else
#define USART_CR1_FIELDS  ((uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | \
                                      USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8 )) /*!< UART or USART CR1 fields of parameters set by UART_SetConfig API */
#endif /* USART_CR1_FIFOEN */

#if defined(USART_CR1_FIFOEN)
#define USART_CR3_FIELDS  ((uint32_t)(USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_ONEBIT| \
                                      USART_CR3_TXFTCFG | USART_CR3_RXFTCFG ))  /*!< UART or USART CR3 fields of parameters set by UART_SetConfig API */
#else
#define USART_CR3_FIELDS  ((uint32_t)(USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_ONEBIT))  /*!< UART or USART CR3 fields of parameters set by UART_SetConfig API */
#endif /* USART_CR1_FIFOEN */

#define LPUART_BRR_MIN  0x00000300U  /* LPUART BRR minimum authorized value */
#define LPUART_BRR_MAX  0x000FFFFFU  /* LPUART BRR maximum authorized value */

#define UART_BRR_MIN    0x10U        /* UART BRR minimum authorized value */
#define UART_BRR_MAX    0x0000FFFFU  /* UART BRR maximum authorized value */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @addtogroup UART_Private_Functions
  * @{
  */
static void UART_EndTxTransfer(UART_HandleTypeDef *huart);
static void UART_EndRxTransfer(UART_HandleTypeDef *huart);
static void UART_DMATransmitCplt(DMA_HandleTypeDef *hdma);
static void UART_DMAReceiveCplt(DMA_HandleTypeDef *hdma);
static void UART_DMARxHalfCplt(DMA_HandleTypeDef *hdma);
static void UART_DMATxHalfCplt(DMA_HandleTypeDef *hdma);
static void UART_DMAError(DMA_HandleTypeDef *hdma);
static void UART_DMAAbortOnError(DMA_HandleTypeDef *hdma);
static void UART_DMATxAbortCallback(DMA_HandleTypeDef *hdma);
static void UART_DMARxAbortCallback(DMA_HandleTypeDef *hdma);
static void UART_DMATxOnlyAbortCallback(DMA_HandleTypeDef *hdma);
static void UART_DMARxOnlyAbortCallback(DMA_HandleTypeDef *hdma);
static void UART_TxISR_8BIT(UART_HandleTypeDef *huart);
static void UART_TxISR_16BIT(UART_HandleTypeDef *huart);
#if defined(USART_CR1_FIFOEN)
static void UART_TxISR_8BIT_FIFOEN(UART_HandleTypeDef *huart);
static void UART_TxISR_16BIT_FIFOEN(UART_HandleTypeDef *huart);
#endif /* USART_CR1_FIFOEN */
static void UART_EndTransmit_IT(UART_HandleTypeDef *huart);
static void UART_RxISR_8BIT(UART_HandleTypeDef *huart);
static void UART_RxISR_16BIT(UART_HandleTypeDef *huart);
#if defined(USART_CR1_FIFOEN)
static void UART_RxISR_8BIT_FIFOEN(UART_HandleTypeDef *huart);
static void UART_RxISR_16BIT_FIFOEN(UART_HandleTypeDef *huart);
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup UART_Exported_Functions UART Exported Functions
  * @{
  */

/** @defgroup UART_Exported_Functions_Group1 Initialization and de-initialization functions
  *  @brief    Initialization and Configuration functions
  *
@verbatim
===============================================================================
            ##### Initialization and Configuration functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to initialize the USARTx or the UARTy
    in asynchronous mode.
      (+) For the asynchronous mode the parameters below can be configured:
        (++) Baud Rate
        (++) Word Length
        (++) Stop Bit
        (++) Parity: If the parity is enabled, then the MSB bit of the data written
             in the data register is transmitted but is changed by the parity bit.
        (++) Hardware flow control
        (++) Receiver/transmitter modes
        (++) Over Sampling Method
        (++) One-Bit Sampling Method
      (+) For the asynchronous mode, the following advanced features can be configured as well:
        (++) TX and/or RX pin level inversion
        (++) data logical level inversion
        (++) RX and TX pins swap
        (++) RX overrun detection disabling
        (++) DMA disabling on RX error
        (++) MSB first on communication line
        (++) auto Baud rate detection
    [..]
    The HAL_UART_Init(), HAL_HalfDuplex_Init(), HAL_LIN_Init()and HAL_MultiProcessor_Init()API
    follow respectively the UART asynchronous, UART Half duplex, UART LIN mode
    and UART multiprocessor mode configuration procedures (details for the procedures
    are available in reference manual).

@endverbatim

  Depending on the frame length defined by the M1 and M0 bits (7-bit,
  8-bit or 9-bit), the possible UART formats are listed in the
  following table.

  Table 1. UART frame format.
    +-----------------------------------------------------------------------+
    |  M1 bit |  M0 bit |  PCE bit  |             UART frame                |
    |---------|---------|-----------|---------------------------------------|
    |    0    |    0    |    0      |    | SB |    8 bit data   | STB |     |
    |---------|---------|-----------|---------------------------------------|
    |    0    |    0    |    1      |    | SB | 7 bit data | PB | STB |     |
    |---------|---------|-----------|---------------------------------------|
    |    0    |    1    |    0      |    | SB |    9 bit data   | STB |     |
    |---------|---------|-----------|---------------------------------------|
    |    0    |    1    |    1      |    | SB | 8 bit data | PB | STB |     |
    |---------|---------|-----------|---------------------------------------|
    |    1    |    0    |    0      |    | SB |    7 bit data   | STB |     |
    |---------|---------|-----------|---------------------------------------|
    |    1    |    0    |    1      |    | SB | 6 bit data | PB | STB |     |
    +-----------------------------------------------------------------------+

  * @{
  */

/**
  * @brief Initialize the UART mode according to the specified
  *        parameters in the UART_InitTypeDef and initialize the associated handle.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart)
{
  /* Check the UART handle allocation */
  if (huart == NULL)
  {
    return HAL_ERROR;
  }

  if (huart->Init.HwFlowCtl != UART_HWCONTROL_NONE)
  {
    /* Check the parameters */
    assert_param(IS_UART_HWFLOW_INSTANCE(huart->Instance));
  }
  else
  {
    /* Check the parameters */
    assert_param((IS_UART_INSTANCE(huart->Instance)) || (IS_LPUART_INSTANCE(huart->Instance)));
  }

  if (huart->gState == HAL_UART_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    UART_InitCallbacksToDefault(huart);

    if (huart->MspInitCallback == NULL)
    {
      huart->MspInitCallback = HAL_UART_MspInit;
    }

    /* Init the low level hardware */
    huart->MspInitCallback(huart);
#else
    /* Init the low level hardware : GPIO, CLOCK */
    HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
  }

  huart->gState = HAL_UART_STATE_BUSY;

  __HAL_UART_DISABLE(huart);

  /* Set the UART Communication parameters */
  if (UART_SetConfig(huart) == HAL_ERROR)
  {
    return HAL_ERROR;
  }

  if (huart->AdvancedInit.AdvFeatureInit != UART_ADVFEATURE_NO_INIT)
  {
    UART_AdvFeatureConfig(huart);
  }

  /* In asynchronous mode, the following bits must be kept cleared:
  - LINEN and CLKEN bits in the USART_CR2 register,
  - SCEN, HDSEL and IREN  bits in the USART_CR3 register.*/
  CLEAR_BIT(huart->Instance->CR2, (USART_CR2_LINEN | USART_CR2_CLKEN));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN));

  __HAL_UART_ENABLE(huart);

  /* TEACK and/or REACK to check before moving huart->gState and huart->RxState to Ready */
  return (UART_CheckIdleState(huart));
}

/**
  * @brief Initialize the half-duplex mode according to the specified
  *        parameters in the UART_InitTypeDef and creates the associated handle.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_HalfDuplex_Init(UART_HandleTypeDef *huart)
{
  /* Check the UART handle allocation */
  if (huart == NULL)
  {
    return HAL_ERROR;
  }

  /* Check UART instance */
  assert_param(IS_UART_HALFDUPLEX_INSTANCE(huart->Instance));

  if (huart->gState == HAL_UART_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    UART_InitCallbacksToDefault(huart);

    if (huart->MspInitCallback == NULL)
    {
      huart->MspInitCallback = HAL_UART_MspInit;
    }

    /* Init the low level hardware */
    huart->MspInitCallback(huart);
#else
    /* Init the low level hardware : GPIO, CLOCK */
    HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
  }

  huart->gState = HAL_UART_STATE_BUSY;

  __HAL_UART_DISABLE(huart);

  /* Set the UART Communication parameters */
  if (UART_SetConfig(huart) == HAL_ERROR)
  {
    return HAL_ERROR;
  }

  if (huart->AdvancedInit.AdvFeatureInit != UART_ADVFEATURE_NO_INIT)
  {
    UART_AdvFeatureConfig(huart);
  }

  /* In half-duplex mode, the following bits must be kept cleared:
  - LINEN and CLKEN bits in the USART_CR2 register,
  - SCEN and IREN bits in the USART_CR3 register.*/
  CLEAR_BIT(huart->Instance->CR2, (USART_CR2_LINEN | USART_CR2_CLKEN));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_IREN | USART_CR3_SCEN));

  /* Enable the Half-Duplex mode by setting the HDSEL bit in the CR3 register */
  SET_BIT(huart->Instance->CR3, USART_CR3_HDSEL);

  __HAL_UART_ENABLE(huart);

  /* TEACK and/or REACK to check before moving huart->gState and huart->RxState to Ready */
  return (UART_CheckIdleState(huart));
}


/**
  * @brief Initialize the LIN mode according to the specified
  *        parameters in the UART_InitTypeDef and creates the associated handle.
  * @param huart             UART handle.
  * @param BreakDetectLength Specifies the LIN break detection length.
  *        This parameter can be one of the following values:
  *          @arg @ref UART_LINBREAKDETECTLENGTH_10B 10-bit break detection
  *          @arg @ref UART_LINBREAKDETECTLENGTH_11B 11-bit break detection
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LIN_Init(UART_HandleTypeDef *huart, uint32_t BreakDetectLength)
{
  /* Check the UART handle allocation */
  if (huart == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the LIN UART instance */
  assert_param(IS_UART_LIN_INSTANCE(huart->Instance));
  /* Check the Break detection length parameter */
  assert_param(IS_UART_LIN_BREAK_DETECT_LENGTH(BreakDetectLength));

  /* LIN mode limited to 16-bit oversampling only */
  if (huart->Init.OverSampling == UART_OVERSAMPLING_8)
  {
    return HAL_ERROR;
  }
  /* LIN mode limited to 8-bit data length */
  if (huart->Init.WordLength != UART_WORDLENGTH_8B)
  {
    return HAL_ERROR;
  }

  if (huart->gState == HAL_UART_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    UART_InitCallbacksToDefault(huart);

    if (huart->MspInitCallback == NULL)
    {
      huart->MspInitCallback = HAL_UART_MspInit;
    }

    /* Init the low level hardware */
    huart->MspInitCallback(huart);
#else
    /* Init the low level hardware : GPIO, CLOCK */
    HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
  }

  huart->gState = HAL_UART_STATE_BUSY;

  __HAL_UART_DISABLE(huart);

  /* Set the UART Communication parameters */
  if (UART_SetConfig(huart) == HAL_ERROR)
  {
    return HAL_ERROR;
  }

  if (huart->AdvancedInit.AdvFeatureInit != UART_ADVFEATURE_NO_INIT)
  {
    UART_AdvFeatureConfig(huart);
  }

  /* In LIN mode, the following bits must be kept cleared:
  - LINEN and CLKEN bits in the USART_CR2 register,
  - SCEN and IREN bits in the USART_CR3 register.*/
  CLEAR_BIT(huart->Instance->CR2, USART_CR2_CLKEN);
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_HDSEL | USART_CR3_IREN | USART_CR3_SCEN));

  /* Enable the LIN mode by setting the LINEN bit in the CR2 register */
  SET_BIT(huart->Instance->CR2, USART_CR2_LINEN);

  /* Set the USART LIN Break detection length. */
  MODIFY_REG(huart->Instance->CR2, USART_CR2_LBDL, BreakDetectLength);

  __HAL_UART_ENABLE(huart);

  /* TEACK and/or REACK to check before moving huart->gState and huart->RxState to Ready */
  return (UART_CheckIdleState(huart));
}


/**
  * @brief Initialize the multiprocessor mode according to the specified
  *        parameters in the UART_InitTypeDef and initialize the associated handle.
  * @param huart        UART handle.
  * @param Address      UART node address (4-, 6-, 7- or 8-bit long).
  * @param WakeUpMethod Specifies the UART wakeup method.
  *        This parameter can be one of the following values:
  *          @arg @ref UART_WAKEUPMETHOD_IDLELINE WakeUp by an idle line detection
  *          @arg @ref UART_WAKEUPMETHOD_ADDRESSMARK WakeUp by an address mark
  * @note  If the user resorts to idle line detection wake up, the Address parameter
  *        is useless and ignored by the initialization function.
  * @note  If the user resorts to address mark wake up, the address length detection
  *        is configured by default to 4 bits only. For the UART to be able to
  *        manage 6-, 7- or 8-bit long addresses detection, the API
  *        HAL_MultiProcessorEx_AddressLength_Set() must be called after
  *        HAL_MultiProcessor_Init().
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MultiProcessor_Init(UART_HandleTypeDef *huart, uint8_t Address, uint32_t WakeUpMethod)
{
  /* Check the UART handle allocation */
  if (huart == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the wake up method parameter */
  assert_param(IS_UART_WAKEUPMETHOD(WakeUpMethod));

  if (huart->gState == HAL_UART_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    UART_InitCallbacksToDefault(huart);

    if (huart->MspInitCallback == NULL)
    {
      huart->MspInitCallback = HAL_UART_MspInit;
    }

    /* Init the low level hardware */
    huart->MspInitCallback(huart);
#else
    /* Init the low level hardware : GPIO, CLOCK */
    HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
  }

  huart->gState = HAL_UART_STATE_BUSY;

  __HAL_UART_DISABLE(huart);

  /* Set the UART Communication parameters */
  if (UART_SetConfig(huart) == HAL_ERROR)
  {
    return HAL_ERROR;
  }

  if (huart->AdvancedInit.AdvFeatureInit != UART_ADVFEATURE_NO_INIT)
  {
    UART_AdvFeatureConfig(huart);
  }

  /* In multiprocessor mode, the following bits must be kept cleared:
  - LINEN and CLKEN bits in the USART_CR2 register,
  - SCEN, HDSEL and IREN  bits in the USART_CR3 register. */
  CLEAR_BIT(huart->Instance->CR2, (USART_CR2_LINEN | USART_CR2_CLKEN));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN));

  if (WakeUpMethod == UART_WAKEUPMETHOD_ADDRESSMARK)
  {
    /* If address mark wake up method is chosen, set the USART address node */
    MODIFY_REG(huart->Instance->CR2, USART_CR2_ADD, ((uint32_t)Address << UART_CR2_ADDRESS_LSB_POS));
  }

  /* Set the wake up method by setting the WAKE bit in the CR1 register */
  MODIFY_REG(huart->Instance->CR1, USART_CR1_WAKE, WakeUpMethod);

  __HAL_UART_ENABLE(huart);

  /* TEACK and/or REACK to check before moving huart->gState and huart->RxState to Ready */
  return (UART_CheckIdleState(huart));
}


/**
  * @brief DeInitialize the UART peripheral.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart)
{
  /* Check the UART handle allocation */
  if (huart == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param((IS_UART_INSTANCE(huart->Instance)) || (IS_LPUART_INSTANCE(huart->Instance)));

  huart->gState = HAL_UART_STATE_BUSY;

  __HAL_UART_DISABLE(huart);

  huart->Instance->CR1 = 0x0U;
  huart->Instance->CR2 = 0x0U;
  huart->Instance->CR3 = 0x0U;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  if (huart->MspDeInitCallback == NULL)
  {
    huart->MspDeInitCallback = HAL_UART_MspDeInit;
  }
  /* DeInit the low level hardware */
  huart->MspDeInitCallback(huart);
#else
  /* DeInit the low level hardware */
  HAL_UART_MspDeInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */

  huart->ErrorCode = HAL_UART_ERROR_NONE;
  huart->gState = HAL_UART_STATE_RESET;
  huart->RxState = HAL_UART_STATE_RESET;

  __HAL_UNLOCK(huart);

  return HAL_OK;
}

/**
  * @brief Initialize the UART MSP.
  * @param huart UART handle.
  * @retval None
  */
__weak void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_MspInit can be implemented in the user file
   */
}

/**
  * @brief DeInitialize the UART MSP.
  * @param huart UART handle.
  * @retval None
  */
__weak void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_MspDeInit can be implemented in the user file
   */
}

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User UART Callback
  *         To be used instead of the weak predefined callback
  * @param  huart uart handle
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_UART_TX_HALFCOMPLETE_CB_ID Tx Half Complete Callback ID
  *           @arg @ref HAL_UART_TX_COMPLETE_CB_ID Tx Complete Callback ID
  *           @arg @ref HAL_UART_RX_HALFCOMPLETE_CB_ID Rx Half Complete Callback ID
  *           @arg @ref HAL_UART_RX_COMPLETE_CB_ID Rx Complete Callback ID
  *           @arg @ref HAL_UART_ERROR_CB_ID Error Callback ID
  *           @arg @ref HAL_UART_ABORT_COMPLETE_CB_ID Abort Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID Abort Transmit Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID Abort Receive Complete Callback ID
  *           @arg @ref HAL_UART_WAKEUP_CB_ID Wakeup Callback ID
  *           @arg @ref HAL_UART_RX_FIFO_FULL_CB_ID Rx Fifo Full Callback ID
  *           @arg @ref HAL_UART_TX_FIFO_EMPTY_CB_ID Tx Fifo Empty Callback ID
  *           @arg @ref HAL_UART_MSPINIT_CB_ID MspInit Callback ID
  *           @arg @ref HAL_UART_MSPDEINIT_CB_ID MspDeInit Callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_RegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID,
                                            pUART_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }

  __HAL_LOCK(huart);

  if (huart->gState == HAL_UART_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_UART_TX_HALFCOMPLETE_CB_ID :
        huart->TxHalfCpltCallback = pCallback;
        break;

      case HAL_UART_TX_COMPLETE_CB_ID :
        huart->TxCpltCallback = pCallback;
        break;

      case HAL_UART_RX_HALFCOMPLETE_CB_ID :
        huart->RxHalfCpltCallback = pCallback;
        break;

      case HAL_UART_RX_COMPLETE_CB_ID :
        huart->RxCpltCallback = pCallback;
        break;

      case HAL_UART_ERROR_CB_ID :
        huart->ErrorCallback = pCallback;
        break;

      case HAL_UART_ABORT_COMPLETE_CB_ID :
        huart->AbortCpltCallback = pCallback;
        break;

      case HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID :
        huart->AbortTransmitCpltCallback = pCallback;
        break;

      case HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID :
        huart->AbortReceiveCpltCallback = pCallback;
        break;

      case HAL_UART_WAKEUP_CB_ID :
        huart->WakeupCallback = pCallback;
        break;

#if defined(USART_CR1_FIFOEN)
      case HAL_UART_RX_FIFO_FULL_CB_ID :
        huart->RxFifoFullCallback = pCallback;
        break;

      case HAL_UART_TX_FIFO_EMPTY_CB_ID :
        huart->TxFifoEmptyCallback = pCallback;
        break;
#endif /* USART_CR1_FIFOEN */

      case HAL_UART_MSPINIT_CB_ID :
        huart->MspInitCallback = pCallback;
        break;

      case HAL_UART_MSPDEINIT_CB_ID :
        huart->MspDeInitCallback = pCallback;
        break;

      default :
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        status =  HAL_ERROR;
        break;
    }
  }
  else if (huart->gState == HAL_UART_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_UART_MSPINIT_CB_ID :
        huart->MspInitCallback = pCallback;
        break;

      case HAL_UART_MSPDEINIT_CB_ID :
        huart->MspDeInitCallback = pCallback;
        break;

      default :
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

    status =  HAL_ERROR;
  }

  __HAL_UNLOCK(huart);

  return status;
}

/**
  * @brief  Unregister an UART Callback
  *         UART callaback is redirected to the weak predefined callback
  * @param  huart uart handle
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_UART_TX_HALFCOMPLETE_CB_ID Tx Half Complete Callback ID
  *           @arg @ref HAL_UART_TX_COMPLETE_CB_ID Tx Complete Callback ID
  *           @arg @ref HAL_UART_RX_HALFCOMPLETE_CB_ID Rx Half Complete Callback ID
  *           @arg @ref HAL_UART_RX_COMPLETE_CB_ID Rx Complete Callback ID
  *           @arg @ref HAL_UART_ERROR_CB_ID Error Callback ID
  *           @arg @ref HAL_UART_ABORT_COMPLETE_CB_ID Abort Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID Abort Transmit Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID Abort Receive Complete Callback ID
  *           @arg @ref HAL_UART_WAKEUP_CB_ID Wakeup Callback ID
  *           @arg @ref HAL_UART_RX_FIFO_FULL_CB_ID Rx Fifo Full Callback ID
  *           @arg @ref HAL_UART_TX_FIFO_EMPTY_CB_ID Tx Fifo Empty Callback ID
  *           @arg @ref HAL_UART_MSPINIT_CB_ID MspInit Callback ID
  *           @arg @ref HAL_UART_MSPDEINIT_CB_ID MspDeInit Callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_UnRegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  __HAL_LOCK(huart);

  if (HAL_UART_STATE_READY == huart->gState)
  {
    switch (CallbackID)
    {
      case HAL_UART_TX_HALFCOMPLETE_CB_ID :
        huart->TxHalfCpltCallback = HAL_UART_TxHalfCpltCallback;               /* Legacy weak  TxHalfCpltCallback       */
        break;

      case HAL_UART_TX_COMPLETE_CB_ID :
        huart->TxCpltCallback = HAL_UART_TxCpltCallback;                       /* Legacy weak TxCpltCallback            */
        break;

      case HAL_UART_RX_HALFCOMPLETE_CB_ID :
        huart->RxHalfCpltCallback = HAL_UART_RxHalfCpltCallback;               /* Legacy weak RxHalfCpltCallback        */
        break;

      case HAL_UART_RX_COMPLETE_CB_ID :
        huart->RxCpltCallback = HAL_UART_RxCpltCallback;                       /* Legacy weak RxCpltCallback            */
        break;

      case HAL_UART_ERROR_CB_ID :
        huart->ErrorCallback = HAL_UART_ErrorCallback;                         /* Legacy weak ErrorCallback             */
        break;

      case HAL_UART_ABORT_COMPLETE_CB_ID :
        huart->AbortCpltCallback = HAL_UART_AbortCpltCallback;                 /* Legacy weak AbortCpltCallback         */
        break;

      case HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID :
        huart->AbortTransmitCpltCallback = HAL_UART_AbortTransmitCpltCallback; /* Legacy weak AbortTransmitCpltCallback */
        break;

      case HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID :
        huart->AbortReceiveCpltCallback = HAL_UART_AbortReceiveCpltCallback;   /* Legacy weak AbortReceiveCpltCallback  */
        break;

      case HAL_UART_WAKEUP_CB_ID :
        huart->WakeupCallback = HAL_UARTEx_WakeupCallback;                     /* Legacy weak WakeupCallback            */
        break;

#if defined(USART_CR1_FIFOEN)
      case HAL_UART_RX_FIFO_FULL_CB_ID :
        huart->RxFifoFullCallback = HAL_UARTEx_RxFifoFullCallback;             /* Legacy weak RxFifoFullCallback        */
        break;

      case HAL_UART_TX_FIFO_EMPTY_CB_ID :
        huart->TxFifoEmptyCallback = HAL_UARTEx_TxFifoEmptyCallback;           /* Legacy weak TxFifoEmptyCallback       */
        break;

#endif /* USART_CR1_FIFOEN */
      case HAL_UART_MSPINIT_CB_ID :
        huart->MspInitCallback = HAL_UART_MspInit;                             /* Legacy weak MspInitCallback           */
        break;

      case HAL_UART_MSPDEINIT_CB_ID :
        huart->MspDeInitCallback = HAL_UART_MspDeInit;                         /* Legacy weak MspDeInitCallback         */
        break;

      default :
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_UART_STATE_RESET == huart->gState)
  {
    switch (CallbackID)
    {
      case HAL_UART_MSPINIT_CB_ID :
        huart->MspInitCallback = HAL_UART_MspInit;
        break;

      case HAL_UART_MSPDEINIT_CB_ID :
        huart->MspDeInitCallback = HAL_UART_MspDeInit;
        break;

      default :
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

    status =  HAL_ERROR;
  }

  __HAL_UNLOCK(huart);

  return status;
}
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup UART_Exported_Functions_Group2 IO operation functions
  * @brief UART Transmit/Receive functions
  *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
    This subsection provides a set of functions allowing to manage the UART asynchronous
    and Half duplex data transfers.

    (#) There are two mode of transfer:
       (+) Blocking mode: The communication is performed in polling mode.
           The HAL status of all data processing is returned by the same function
           after finishing transfer.
       (+) Non-Blocking mode: The communication is performed using Interrupts
           or DMA, These API's return the HAL status.
           The end of the data processing will be indicated through the
           dedicated UART IRQ when using Interrupt mode or the DMA IRQ when
           using DMA mode.
           The HAL_UART_TxCpltCallback(), HAL_UART_RxCpltCallback() user callbacks
           will be executed respectively at the end of the transmit or Receive process
           The HAL_UART_ErrorCallback()user callback will be executed when a communication error is detected

    (#) Blocking mode API's are :
        (+) HAL_UART_Transmit()
        (+) HAL_UART_Receive()

    (#) Non-Blocking mode API's with Interrupt are :
        (+) HAL_UART_Transmit_IT()
        (+) HAL_UART_Receive_IT()
        (+) HAL_UART_IRQHandler()

    (#) Non-Blocking mode API's with DMA are :
        (+) HAL_UART_Transmit_DMA()
        (+) HAL_UART_Receive_DMA()
        (+) HAL_UART_DMAPause()
        (+) HAL_UART_DMAResume()
        (+) HAL_UART_DMAStop()

    (#) A set of Transfer Complete Callbacks are provided in Non_Blocking mode:
        (+) HAL_UART_TxHalfCpltCallback()
        (+) HAL_UART_TxCpltCallback()
        (+) HAL_UART_RxHalfCpltCallback()
        (+) HAL_UART_RxCpltCallback()
        (+) HAL_UART_ErrorCallback()

    (#) Non-Blocking mode transfers could be aborted using Abort API's :
        (+) HAL_UART_Abort()
        (+) HAL_UART_AbortTransmit()
        (+) HAL_UART_AbortReceive()
        (+) HAL_UART_Abort_IT()
        (+) HAL_UART_AbortTransmit_IT()
        (+) HAL_UART_AbortReceive_IT()

    (#) For Abort services based on interrupts (HAL_UART_Abortxxx_IT), a set of Abort Complete Callbacks are provided:
        (+) HAL_UART_AbortCpltCallback()
        (+) HAL_UART_AbortTransmitCpltCallback()
        (+) HAL_UART_AbortReceiveCpltCallback()

    (#) In Non-Blocking mode transfers, possible errors are split into 2 categories.
        Errors are handled as follows :
       (+) Error is considered as Recoverable and non blocking : Transfer could go till end, but error severity is
           to be evaluated by user : this concerns Frame Error, Parity Error or Noise Error in Interrupt mode reception .
           Received character is then retrieved and stored in Rx buffer, Error code is set to allow user to identify error type,
           and HAL_UART_ErrorCallback() user callback is executed. Transfer is kept ongoing on UART side.
           If user wants to abort it, Abort services should be called by user.
       (+) Error is considered as Blocking : Transfer could not be completed properly and is aborted.
           This concerns Overrun Error In Interrupt mode reception and all errors in DMA mode.
           Error code is set to allow user to identify error type, and HAL_UART_ErrorCallback() user callback is executed.

    -@- In the Half duplex communication, it is forbidden to run the transmit
        and receive process in parallel, the UART state HAL_UART_STATE_BUSY_TX_RX can't be useful.

@endverbatim
  * @{
  */

/**
  * @brief Send an amount of data in blocking mode.
  * @note When FIFO mode is enabled, writing a data in the TDR register adds one
  *       data to the TXFIFO. Write operations to the TDR register are performed
  *       when TXFNF flag is set. From hardware perspective, TXFNF flag and
  *       TXE are mapped on the same bit-field.
  * @param huart   UART handle.
  * @param pData   Pointer to data buffer.
  * @param Size    Amount of data to be sent.
  * @param Timeout Timeout duration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint8_t  *pdata8bits;
  uint16_t *pdata16bits;
  uint32_t tickstart;

  /* Check that a Tx process is not already ongoing */
  if (huart->gState == HAL_UART_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return  HAL_ERROR;
    }

    __HAL_LOCK(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;

    /* Init tickstart for timeout managment*/
    tickstart = HAL_GetTick();

    huart->TxXferSize  = Size;
    huart->TxXferCount = Size;

    /* In case of 9bits/No Parity transfer, pData needs to be handled as a uint16_t pointer */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      pdata8bits  = NULL;
      pdata16bits = (uint16_t *) pData;
    }
    else
    {
      pdata8bits  = pData;
      pdata16bits = NULL;
    }

    while (huart->TxXferCount > 0U)
    {
      if (UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TXE, RESET, tickstart, Timeout) != HAL_OK)
      {
        return HAL_TIMEOUT;
      }
      if (pdata8bits == NULL)
      {
        huart->Instance->TDR = (uint16_t)(*pdata16bits & 0x01FFU);
        pdata16bits++;
      }
      else
      {
        huart->Instance->TDR = (uint8_t)(*pdata8bits & 0xFFU);
        pdata8bits++;
      }
      huart->TxXferCount--;
    }

    if (UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TC, RESET, tickstart, Timeout) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }

    /* At end of Tx process, restore huart->gState to Ready */
    huart->gState = HAL_UART_STATE_READY;

    __HAL_UNLOCK(huart);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief Receive an amount of data in blocking mode.
  * @note When FIFO mode is enabled, the RXFNE flag is set as long as the RXFIFO
  *       is not empty. Read operations from the RDR register are performed when
  *       RXFNE flag is set. From hardware perspective, RXFNE flag and
  *       RXNE are mapped on the same bit-field.
  * @param huart   UART handle.
  * @param pData   Pointer to data buffer.
  * @param Size    Amount of data to be received.
  * @param Timeout Timeout duration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint8_t  *pdata8bits;
  uint16_t *pdata16bits;
  uint16_t uhMask;
  uint32_t tickstart;

  /* Check that a Rx process is not already ongoing */
  if (huart->RxState == HAL_UART_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return  HAL_ERROR;
    }

    __HAL_LOCK(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    /* Init tickstart for timeout managment*/
    tickstart = HAL_GetTick();

    huart->RxXferSize  = Size;
    huart->RxXferCount = Size;

    /* Computation of UART mask to apply to RDR register */
    UART_MASK_COMPUTATION(huart);
    uhMask = huart->Mask;

    /* In case of 9bits/No Parity transfer, pRxData needs to be handled as a uint16_t pointer */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      pdata8bits  = NULL;
      pdata16bits = (uint16_t *) pData;
    }
    else
    {
      pdata8bits  = pData;
      pdata16bits = NULL;
    }

    /* as long as data have to be received */
    while (huart->RxXferCount > 0U)
    {
      if (UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_RXNE, RESET, tickstart, Timeout) != HAL_OK)
      {
        return HAL_TIMEOUT;
      }
      if (pdata8bits == NULL)
      {
        *pdata16bits = (uint16_t)(huart->Instance->RDR & uhMask);
        pdata16bits++;
      }
      else
      {
        *pdata8bits = (uint8_t)(huart->Instance->RDR & (uint8_t)uhMask);
        pdata8bits++;
      }
      huart->RxXferCount--;
    }

    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;

    __HAL_UNLOCK(huart);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief Send an amount of data in interrupt mode.
  * @param huart UART handle.
  * @param pData Pointer to data buffer.
  * @param Size  Amount of data to be sent.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  /* Check that a Tx process is not already ongoing */
  if (huart->gState == HAL_UART_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    __HAL_LOCK(huart);

    huart->pTxBuffPtr  = pData;
    huart->TxXferSize  = Size;
    huart->TxXferCount = Size;
    huart->TxISR       = NULL;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;

#if defined(USART_CR1_FIFOEN)
    /* Configure Tx interrupt processing */
    if (huart->FifoMode == UART_FIFOMODE_ENABLE)
    {
      /* Set the Tx ISR function pointer according to the data word length */
      if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
      {
        huart->TxISR = UART_TxISR_16BIT_FIFOEN;
      }
      else
      {
        huart->TxISR = UART_TxISR_8BIT_FIFOEN;
      }

      __HAL_UNLOCK(huart);

      /* Enable the TX FIFO threshold interrupt */
      SET_BIT(huart->Instance->CR3, USART_CR3_TXFTIE);
    }
    else
    {
      /* Set the Tx ISR function pointer according to the data word length */
      if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
      {
        huart->TxISR = UART_TxISR_16BIT;
      }
      else
      {
        huart->TxISR = UART_TxISR_8BIT;
      }

      __HAL_UNLOCK(huart);

      /* Enable the Transmit Data Register Empty interrupt */
      SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE_TXFNFIE);
    }
#else
    /* Set the Tx ISR function pointer according to the data word length */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      huart->TxISR = UART_TxISR_16BIT;
    }
    else
    {
      huart->TxISR = UART_TxISR_8BIT;
    }

    __HAL_UNLOCK(huart);

    /* Enable the Transmit Data Register Empty interrupt */
    SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
#endif /* USART_CR1_FIFOEN */

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief Receive an amount of data in interrupt mode.
  * @param huart UART handle.
  * @param pData Pointer to data buffer.
  * @param Size  Amount of data to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  /* Check that a Rx process is not already ongoing */
  if (huart->RxState == HAL_UART_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    __HAL_LOCK(huart);

    huart->pRxBuffPtr  = pData;
    huart->RxXferSize  = Size;
    huart->RxXferCount = Size;
    huart->RxISR       = NULL;

    /* Computation of UART mask to apply to RDR register */
    UART_MASK_COMPUTATION(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

#if defined(USART_CR1_FIFOEN)
    /* Configure Rx interrupt processing*/
    if ((huart->FifoMode == UART_FIFOMODE_ENABLE) && (Size >= huart->NbRxDataToProcess))
    {
      /* Set the Rx ISR function pointer according to the data word length */
      if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
      {
        huart->RxISR = UART_RxISR_16BIT_FIFOEN;
      }
      else
      {
        huart->RxISR = UART_RxISR_8BIT_FIFOEN;
      }

      __HAL_UNLOCK(huart);

      /* Enable the UART Parity Error interrupt and RX FIFO Threshold interrupt */
      SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);
      SET_BIT(huart->Instance->CR3, USART_CR3_RXFTIE);
    }
    else
    {
      /* Set the Rx ISR function pointer according to the data word length */
      if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
      {
        huart->RxISR = UART_RxISR_16BIT;
      }
      else
      {
        huart->RxISR = UART_RxISR_8BIT;
      }

      __HAL_UNLOCK(huart);

      /* Enable the UART Parity Error interrupt and Data Register Not Empty interrupt */
      SET_BIT(huart->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE_RXFNEIE);
    }
#else
    /* Set the Rx ISR function pointer according to the data word length */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      huart->RxISR = UART_RxISR_16BIT;
    }
    else
    {
      huart->RxISR = UART_RxISR_8BIT;
    }

    __HAL_UNLOCK(huart);

    /* Enable the UART Parity Error interrupt and Data Register Not Empty interrupt */
    SET_BIT(huart->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);
#endif /* USART_CR1_FIFOEN */

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief Send an amount of data in DMA mode.
  * @param huart UART handle.
  * @param pData Pointer to data buffer.
  * @param Size  Amount of data to be sent.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  /* Check that a Tx process is not already ongoing */
  if (huart->gState == HAL_UART_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    __HAL_LOCK(huart);

    huart->pTxBuffPtr  = pData;
    huart->TxXferSize  = Size;
    huart->TxXferCount = Size;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;

    if (huart->hdmatx != NULL)
    {
      /* Set the UART DMA transfer complete callback */
      huart->hdmatx->XferCpltCallback = UART_DMATransmitCplt;

      /* Set the UART DMA Half transfer complete callback */
      huart->hdmatx->XferHalfCpltCallback = UART_DMATxHalfCplt;

      /* Set the DMA error callback */
      huart->hdmatx->XferErrorCallback = UART_DMAError;

      /* Set the DMA abort callback */
      huart->hdmatx->XferAbortCallback = NULL;

      /* Enable the UART transmit DMA channel */
      if (HAL_DMA_Start_IT(huart->hdmatx, (uint32_t)huart->pTxBuffPtr, (uint32_t)&huart->Instance->TDR, Size) != HAL_OK)
      {
        /* Set error code to DMA */
        huart->ErrorCode = HAL_UART_ERROR_DMA;

        __HAL_UNLOCK(huart);

        /* Restore huart->gState to ready */
        huart->gState = HAL_UART_STATE_READY;

        return HAL_ERROR;
      }
    }
    /* Clear the TC flag in the ICR register */
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_TCF);

    __HAL_UNLOCK(huart);

    /* Enable the DMA transfer for transmit request by setting the DMAT bit
    in the UART CR3 register */
    SET_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief Receive an amount of data in DMA mode.
  * @note   When the UART parity is enabled (PCE = 1), the received data contain
  *         the parity bit (MSB position).
  * @param huart UART handle.
  * @param pData Pointer to data buffer.
  * @param Size  Amount of data to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  /* Check that a Rx process is not already ongoing */
  if (huart->RxState == HAL_UART_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    __HAL_LOCK(huart);

    huart->pRxBuffPtr = pData;
    huart->RxXferSize = Size;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    if (huart->hdmarx != NULL)
    {
      /* Set the UART DMA transfer complete callback */
      huart->hdmarx->XferCpltCallback = UART_DMAReceiveCplt;

      /* Set the UART DMA Half transfer complete callback */
      huart->hdmarx->XferHalfCpltCallback = UART_DMARxHalfCplt;

      /* Set the DMA error callback */
      huart->hdmarx->XferErrorCallback = UART_DMAError;

      /* Set the DMA abort callback */
      huart->hdmarx->XferAbortCallback = NULL;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(huart->hdmarx, (uint32_t)&huart->Instance->RDR, (uint32_t)huart->pRxBuffPtr, Size) != HAL_OK)
      {
        /* Set error code to DMA */
        huart->ErrorCode = HAL_UART_ERROR_DMA;

        __HAL_UNLOCK(huart);

        /* Restore huart->gState to ready */
        huart->gState = HAL_UART_STATE_READY;

        return HAL_ERROR;
      }
    }
    __HAL_UNLOCK(huart);

    /* Enable the UART Parity Error Interrupt */
    SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);

    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

    /* Enable the DMA transfer for the receiver request by setting the DMAR bit
    in the UART CR3 register */
    SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief Pause the DMA Transfer.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DMAPause(UART_HandleTypeDef *huart)
{
  const HAL_UART_StateTypeDef gstate = huart->gState;
  const HAL_UART_StateTypeDef rxstate = huart->RxState;

  __HAL_LOCK(huart);

  if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT)) &&
      (gstate == HAL_UART_STATE_BUSY_TX))
  {
    /* Disable the UART DMA Tx request */
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);
  }
  if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)) &&
      (rxstate == HAL_UART_STATE_BUSY_RX))
  {
    /* Disable PE and ERR (Frame error, noise error, overrun error) interrupts */
    CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

    /* Disable the UART DMA Rx request */
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);
  }

  __HAL_UNLOCK(huart);

  return HAL_OK;
}

/**
  * @brief Resume the DMA Transfer.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DMAResume(UART_HandleTypeDef *huart)
{
  __HAL_LOCK(huart);

  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {
    /* Enable the UART DMA Tx request */
    SET_BIT(huart->Instance->CR3, USART_CR3_DMAT);
  }
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
    /* Clear the Overrun flag before resuming the Rx transfer */
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF);

    /* Reenable PE and ERR (Frame error, noise error, overrun error) interrupts */
    SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);
    SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

    /* Enable the UART DMA Rx request */
    SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);
  }

  __HAL_UNLOCK(huart);

  return HAL_OK;
}

/**
  * @brief Stop the DMA Transfer.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DMAStop(UART_HandleTypeDef *huart)
{
  /* The Lock is not implemented on this API to allow the user application
     to call the HAL UART API under callbacks HAL_UART_TxCpltCallback() / HAL_UART_RxCpltCallback() /
     HAL_UART_TxHalfCpltCallback / HAL_UART_RxHalfCpltCallback:
     indeed, when HAL_DMA_Abort() API is called, the DMA TX/RX Transfer or Half Transfer complete
     interrupt is generated if the DMA transfer interruption occurs at the middle or at the end of
     the stream and the corresponding call back is executed. */

  const HAL_UART_StateTypeDef gstate = huart->gState;
  const HAL_UART_StateTypeDef rxstate = huart->RxState;

  /* Stop UART DMA Tx request if ongoing */
  if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT)) &&
      (gstate == HAL_UART_STATE_BUSY_TX))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    /* Abort the UART DMA Tx channel */
    if (huart->hdmatx != NULL)
    {
      if (HAL_DMA_Abort(huart->hdmatx) != HAL_OK)
      {
        if (HAL_DMA_GetError(huart->hdmatx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          huart->ErrorCode = HAL_UART_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }

    UART_EndTxTransfer(huart);
  }

  /* Stop UART DMA Rx request if ongoing */
  if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)) &&
      (rxstate == HAL_UART_STATE_BUSY_RX))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* Abort the UART DMA Rx channel */
    if (huart->hdmarx != NULL)
    {
      if (HAL_DMA_Abort(huart->hdmarx) != HAL_OK)
      {
        if (HAL_DMA_GetError(huart->hdmarx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          huart->ErrorCode = HAL_UART_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }

    UART_EndRxTransfer(huart);
  }

  return HAL_OK;
}

/**
  * @brief  Abort ongoing transfers (blocking mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx and Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *huart)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable TXE, TC, RXNE, PE, RXFT, TXFT and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE | USART_CR1_TXEIE_TXFNFIE | USART_CR1_TCIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE | USART_CR3_RXFTIE | USART_CR3_TXFTIE);
#else
  /* Disable TXEIE, TCIE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* Disable the UART DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    /* Abort the UART DMA Tx channel : use blocking DMA Abort API (no callback) */
    if (huart->hdmatx != NULL)
    {
      /* Set the UART DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      huart->hdmatx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(huart->hdmatx) != HAL_OK)
      {
        if (HAL_DMA_GetError(huart->hdmatx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          huart->ErrorCode = HAL_UART_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Disable the UART DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* Abort the UART DMA Rx channel : use blocking DMA Abort API (no callback) */
    if (huart->hdmarx != NULL)
    {
      /* Set the UART DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      huart->hdmarx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(huart->hdmarx) != HAL_OK)
      {
        if (HAL_DMA_GetError(huart->hdmarx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          huart->ErrorCode = HAL_UART_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Reset Tx and Rx transfer counters */
  huart->TxXferCount = 0U;
  huart->RxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

#if defined(USART_CR1_FIFOEN)
  /* Flush the whole TX FIFO (if needed) */
  if (huart->FifoMode == UART_FIFOMODE_ENABLE)
  {
    __HAL_UART_SEND_REQ(huart, UART_TXDATA_FLUSH_REQUEST);
  }
#endif /* USART_CR1_FIFOEN */

  /* Discard the received data */
  __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

  /* Restore huart->gState and huart->RxState to Ready */
  huart->gState  = HAL_UART_STATE_READY;
  huart->RxState = HAL_UART_STATE_READY;

  huart->ErrorCode = HAL_UART_ERROR_NONE;

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Transmit transfer (blocking mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Tx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortTransmit(UART_HandleTypeDef *huart)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable TCIE, TXEIE and TXFTIE interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TCIE | USART_CR1_TXEIE_TXFNFIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_TXFTIE);
#else
  /* Disable TXEIE and TCIE interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
#endif /* USART_CR1_FIFOEN */

  /* Disable the UART DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    /* Abort the UART DMA Tx channel : use blocking DMA Abort API (no callback) */
    if (huart->hdmatx != NULL)
    {
      /* Set the UART DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      huart->hdmatx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(huart->hdmatx) != HAL_OK)
      {
        if (HAL_DMA_GetError(huart->hdmatx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          huart->ErrorCode = HAL_UART_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Reset Tx transfer counter */
  huart->TxXferCount = 0U;

#if defined(USART_CR1_FIFOEN)
  /* Flush the whole TX FIFO (if needed) */
  if (huart->FifoMode == UART_FIFOMODE_ENABLE)
  {
    __HAL_UART_SEND_REQ(huart, UART_TXDATA_FLUSH_REQUEST);
  }
#endif /* USART_CR1_FIFOEN */

  /* Restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Receive transfer (blocking mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Rx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *huart)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable PEIE, EIE, RXNEIE and RXFTIE interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_PEIE | USART_CR1_RXNEIE_RXFNEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE | USART_CR3_RXFTIE);
#else
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* Disable the UART DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* Abort the UART DMA Rx channel : use blocking DMA Abort API (no callback) */
    if (huart->hdmarx != NULL)
    {
      /* Set the UART DMA Abort callback to Null.
         No call back execution at end of DMA abort procedure */
      huart->hdmarx->XferAbortCallback = NULL;

      if (HAL_DMA_Abort(huart->hdmarx) != HAL_OK)
      {
        if (HAL_DMA_GetError(huart->hdmarx) == HAL_DMA_ERROR_TIMEOUT)
        {
          /* Set error code to DMA */
          huart->ErrorCode = HAL_UART_ERROR_DMA;

          return HAL_TIMEOUT;
        }
      }
    }
  }

  /* Reset Rx transfer counter */
  huart->RxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

  /* Discard the received data */
  __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

  /* Restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Abort ongoing transfers (Interrupt mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx and Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Abort_IT(UART_HandleTypeDef *huart)
{
  uint32_t abortcplt = 1U;

  /* Disable interrupts */
#if defined(USART_CR1_FIFOEN)
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_PEIE | USART_CR1_TCIE | USART_CR1_RXNEIE_RXFNEIE | USART_CR1_TXEIE_TXFNFIE));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE | USART_CR3_TXFTIE));
#else
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* If DMA Tx and/or DMA Rx Handles are associated to UART Handle, DMA Abort complete callbacks should be initialised
     before any call to DMA Abort functions */
  /* DMA Tx Handle is valid */
  if (huart->hdmatx != NULL)
  {
    /* Set DMA Abort Complete callback if UART DMA Tx request if enabled.
       Otherwise, set it to NULL */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT))
    {
      huart->hdmatx->XferAbortCallback = UART_DMATxAbortCallback;
    }
    else
    {
      huart->hdmatx->XferAbortCallback = NULL;
    }
  }
  /* DMA Rx Handle is valid */
  if (huart->hdmarx != NULL)
  {
    /* Set DMA Abort Complete callback if UART DMA Rx request if enabled.
       Otherwise, set it to NULL */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
    {
      huart->hdmarx->XferAbortCallback = UART_DMARxAbortCallback;
    }
    else
    {
      huart->hdmarx->XferAbortCallback = NULL;
    }
  }

  /* Disable the UART DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT))
  {
    /* Disable DMA Tx at UART level */
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    /* Abort the UART DMA Tx channel : use non blocking DMA Abort API (callback) */
    if (huart->hdmatx != NULL)
    {
      /* UART Tx DMA Abort callback has already been initialised :
         will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */

      /* Abort DMA TX */
      if (HAL_DMA_Abort_IT(huart->hdmatx) != HAL_OK)
      {
        huart->hdmatx->XferAbortCallback = NULL;
      }
      else
      {
        abortcplt = 0U;
      }
    }
  }

  /* Disable the UART DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* Abort the UART DMA Rx channel : use non blocking DMA Abort API (callback) */
    if (huart->hdmarx != NULL)
    {
      /* UART Rx DMA Abort callback has already been initialised :
         will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */

      /* Abort DMA RX */
      if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
      {
        huart->hdmarx->XferAbortCallback = NULL;
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
    huart->TxXferCount = 0U;
    huart->RxXferCount = 0U;

    /* Clear ISR function pointers */
    huart->RxISR = NULL;
    huart->TxISR = NULL;

    /* Reset errorCode */
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    /* Clear the Error flags in the ICR register */
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

#if defined(USART_CR1_FIFOEN)
    /* Flush the whole TX FIFO (if needed) */
    if (huart->FifoMode == UART_FIFOMODE_ENABLE)
    {
      __HAL_UART_SEND_REQ(huart, UART_TXDATA_FLUSH_REQUEST);
    }
#endif /* USART_CR1_FIFOEN */

    /* Discard the received data */
    __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

    /* Restore huart->gState and huart->RxState to Ready */
    huart->gState  = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;

    /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Abort complete callback */
    huart->AbortCpltCallback(huart);
#else
    /* Call legacy weak Abort complete callback */
    HAL_UART_AbortCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Transmit transfer (Interrupt mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Tx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortTransmit_IT(UART_HandleTypeDef *huart)
{
  /* Disable interrupts */
#if defined(USART_CR1_FIFOEN)
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TCIE | USART_CR1_TXEIE_TXFNFIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_TXFTIE);
#else
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
#endif /* USART_CR1_FIFOEN */

  /* Disable the UART DMA Tx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    /* Abort the UART DMA Tx channel : use non blocking DMA Abort API (callback) */
    if (huart->hdmatx != NULL)
    {
      /* Set the UART DMA Abort callback :
         will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */
      huart->hdmatx->XferAbortCallback = UART_DMATxOnlyAbortCallback;

      /* Abort DMA TX */
      if (HAL_DMA_Abort_IT(huart->hdmatx) != HAL_OK)
      {
        /* Call Directly huart->hdmatx->XferAbortCallback function in case of error */
        huart->hdmatx->XferAbortCallback(huart->hdmatx);
      }
    }
    else
    {
      /* Reset Tx transfer counter */
      huart->TxXferCount = 0U;

      /* Clear TxISR function pointers */
      huart->TxISR = NULL;

      /* Restore huart->gState to Ready */
      huart->gState = HAL_UART_STATE_READY;

      /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
      /* Call registered Abort Transmit Complete Callback */
      huart->AbortTransmitCpltCallback(huart);
#else
      /* Call legacy weak Abort Transmit Complete Callback */
      HAL_UART_AbortTransmitCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
  }
  else
  {
    /* Reset Tx transfer counter */
    huart->TxXferCount = 0U;

    /* Clear TxISR function pointers */
    huart->TxISR = NULL;

#if defined(USART_CR1_FIFOEN)
    /* Flush the whole TX FIFO (if needed) */
    if (huart->FifoMode == UART_FIFOMODE_ENABLE)
    {
      __HAL_UART_SEND_REQ(huart, UART_TXDATA_FLUSH_REQUEST);
    }
#endif /* USART_CR1_FIFOEN */

    /* Restore huart->gState to Ready */
    huart->gState = HAL_UART_STATE_READY;

    /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Abort Transmit Complete Callback */
    huart->AbortTransmitCpltCallback(huart);
#else
    /* Call legacy weak Abort Transmit Complete Callback */
    HAL_UART_AbortTransmitCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }

  return HAL_OK;
}

/**
  * @brief  Abort ongoing Receive transfer (Interrupt mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Rx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortReceive_IT(UART_HandleTypeDef *huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
#if defined(USART_CR1_FIFOEN)
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_PEIE | USART_CR1_RXNEIE_RXFNEIE));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE));
#else
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* Disable the UART DMA Rx request if enabled */
  if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* Abort the UART DMA Rx channel : use non blocking DMA Abort API (callback) */
    if (huart->hdmarx != NULL)
    {
      /* Set the UART DMA Abort callback :
         will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */
      huart->hdmarx->XferAbortCallback = UART_DMARxOnlyAbortCallback;

      /* Abort DMA RX */
      if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
      {
        /* Call Directly huart->hdmarx->XferAbortCallback function in case of error */
        huart->hdmarx->XferAbortCallback(huart->hdmarx);
      }
    }
    else
    {
      /* Reset Rx transfer counter */
      huart->RxXferCount = 0U;

      /* Clear RxISR function pointer */
      huart->pRxBuffPtr = NULL;

      /* Clear the Error flags in the ICR register */
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

      /* Discard the received data */
      __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

      /* Restore huart->RxState to Ready */
      huart->RxState = HAL_UART_STATE_READY;

      /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
      /* Call registered Abort Receive Complete Callback */
      huart->AbortReceiveCpltCallback(huart);
#else
      /* Call legacy weak Abort Receive Complete Callback */
      HAL_UART_AbortReceiveCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
  }
  else
  {
    /* Reset Rx transfer counter */
    huart->RxXferCount = 0U;

    /* Clear RxISR function pointer */
    huart->pRxBuffPtr = NULL;

    /* Clear the Error flags in the ICR register */
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

    /* Restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;

    /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Abort Receive Complete Callback */
    huart->AbortReceiveCpltCallback(huart);
#else
    /* Call legacy weak Abort Receive Complete Callback */
    HAL_UART_AbortReceiveCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }

  return HAL_OK;
}

/**
  * @brief Handle UART interrupt request.
  * @param huart UART handle.
  * @retval None
  */
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
  uint32_t isrflags   = READ_REG(huart->Instance->ISR);
  uint32_t cr1its     = READ_REG(huart->Instance->CR1);
  uint32_t cr3its     = READ_REG(huart->Instance->CR3);

  uint32_t errorflags;
  uint32_t errorcode;

  /* If no error occurs */
  errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
  if (errorflags == 0U)
  {
    /* UART in mode Receiver ---------------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
    if (((isrflags & USART_ISR_RXNE_RXFNE) != 0U)
        && (((cr1its & USART_CR1_RXNEIE_RXFNEIE) != 0U)
            || ((cr3its & USART_CR3_RXFTIE) != 0U)))
#else
    if (((isrflags & USART_ISR_RXNE) != 0U)
        && ((cr1its & USART_CR1_RXNEIE) != 0U))
#endif /* USART_CR1_FIFOEN */
    {
      if (huart->RxISR != NULL)
      {
        huart->RxISR(huart);
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
    /* UART parity error interrupt occurred -------------------------------------*/
    if (((isrflags & USART_ISR_PE) != 0U) && ((cr1its & USART_CR1_PEIE) != 0U))
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_PEF);

      huart->ErrorCode |= HAL_UART_ERROR_PE;
    }

    /* UART frame error interrupt occurred --------------------------------------*/
    if (((isrflags & USART_ISR_FE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_FEF);

      huart->ErrorCode |= HAL_UART_ERROR_FE;
    }

    /* UART noise error interrupt occurred --------------------------------------*/
    if (((isrflags & USART_ISR_NE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_NEF);

      huart->ErrorCode |= HAL_UART_ERROR_NE;
    }

    /* UART Over-Run interrupt occurred -----------------------------------------*/
#if defined(USART_CR1_FIFOEN)
    if (((isrflags & USART_ISR_ORE) != 0U)
        && (((cr1its & USART_CR1_RXNEIE_RXFNEIE) != 0U) ||
            ((cr3its & (USART_CR3_RXFTIE | USART_CR3_EIE)) != 0U)))
#else
    if (((isrflags & USART_ISR_ORE) != 0U)
        && (((cr1its & USART_CR1_RXNEIE) != 0U) ||
            ((cr3its & USART_CR3_EIE) != 0U)))
#endif /* USART_CR1_FIFOEN */
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF);

      huart->ErrorCode |= HAL_UART_ERROR_ORE;
    }

    /* Call UART Error Call back function if need be --------------------------*/
    if (huart->ErrorCode != HAL_UART_ERROR_NONE)
    {
      /* UART in mode Receiver ---------------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
      if (((isrflags & USART_ISR_RXNE_RXFNE) != 0U)
          && (((cr1its & USART_CR1_RXNEIE_RXFNEIE) != 0U)
              || ((cr3its & USART_CR3_RXFTIE) != 0U)))
#else
      if (((isrflags & USART_ISR_RXNE) != 0U)
          && ((cr1its & USART_CR1_RXNEIE) != 0U))
#endif /* USART_CR1_FIFOEN */
      {
        if (huart->RxISR != NULL)
        {
          huart->RxISR(huart);
        }
      }

      /* If Overrun error occurs, or if any error occurs in DMA mode reception,
         consider error as blocking */
      errorcode = huart->ErrorCode;
      if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)) ||
          ((errorcode & HAL_UART_ERROR_ORE) != 0U))
      {
        /* Blocking error : transfer is aborted
           Set the UART state ready to be able to start again the process,
           Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
        UART_EndRxTransfer(huart);

        /* Disable the UART DMA Rx request if enabled */
        if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
        {
          CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

          /* Abort the UART DMA Rx channel */
          if (huart->hdmarx != NULL)
          {
            /* Set the UART DMA Abort callback :
               will lead to call HAL_UART_ErrorCallback() at end of DMA abort procedure */
            huart->hdmarx->XferAbortCallback = UART_DMAAbortOnError;

            /* Abort DMA RX */
            if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
            {
              /* Call Directly huart->hdmarx->XferAbortCallback function in case of error */
              huart->hdmarx->XferAbortCallback(huart->hdmarx);
            }
          }
          else
          {
            /* Call user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
            /*Call registered error callback*/
            huart->ErrorCallback(huart);
#else
            /*Call legacy weak error callback*/
            HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

          }
        }
        else
        {
          /* Call user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
          /*Call registered error callback*/
          huart->ErrorCallback(huart);
#else
          /*Call legacy weak error callback*/
          HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
        }
      }
      else
      {
        /* Non Blocking error : transfer could go on.
           Error is notified to user through user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered error callback*/
        huart->ErrorCallback(huart);
#else
        /*Call legacy weak error callback*/
        HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
        huart->ErrorCode = HAL_UART_ERROR_NONE;
      }
    }
    return;

  } /* End if some error occurs */

  /* UART wakeup from Stop mode interrupt occurred ---------------------------*/
  if (((isrflags & USART_ISR_WUF) != 0U) && ((cr3its & USART_CR3_WUFIE) != 0U))
  {
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_WUF);

    /* UART Rx state is not reset as a reception process might be ongoing.
       If UART handle state fields need to be reset to READY, this could be done in Wakeup callback */

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Wakeup Callback */
    huart->WakeupCallback(huart);
#else
    /* Call legacy weak Wakeup Callback */
    HAL_UARTEx_WakeupCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    return;
  }

  /* UART in mode Transmitter ------------------------------------------------*/
#if defined(USART_CR1_FIFOEN)
  if (((isrflags & USART_ISR_TXE_TXFNF) != 0U)
      && (((cr1its & USART_CR1_TXEIE_TXFNFIE) != 0U)
          || ((cr3its & USART_CR3_TXFTIE) != 0U)))
#else
  if (((isrflags & USART_ISR_TXE) != 0U)
      && ((cr1its & USART_CR1_TXEIE) != 0U))
#endif /* USART_CR1_FIFOEN */
  {
    if (huart->TxISR != NULL)
    {
      huart->TxISR(huart);
    }
    return;
  }

  /* UART in mode Transmitter (transmission end) -----------------------------*/
  if (((isrflags & USART_ISR_TC) != 0U) && ((cr1its & USART_CR1_TCIE) != 0U))
  {
    UART_EndTransmit_IT(huart);
    return;
  }

#if defined(USART_CR1_FIFOEN)
  /* UART TX Fifo Empty occurred ----------------------------------------------*/
  if (((isrflags & USART_ISR_TXFE) != 0U) && ((cr1its & USART_CR1_TXFEIE) != 0U))
  {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Tx Fifo Empty Callback */
    huart->TxFifoEmptyCallback(huart);
#else
    /* Call legacy weak Tx Fifo Empty Callback */
    HAL_UARTEx_TxFifoEmptyCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    return;
  }

  /* UART RX Fifo Full occurred ----------------------------------------------*/
  if (((isrflags & USART_ISR_RXFF) != 0U) && ((cr1its & USART_CR1_RXFFIE) != 0U))
  {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Rx Fifo Full Callback */
    huart->RxFifoFullCallback(huart);
#else
    /* Call legacy weak Rx Fifo Full Callback */
    HAL_UARTEx_RxFifoFullCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    return;
  }
#endif /* USART_CR1_FIFOEN */
}

/**
  * @brief Tx Transfer completed callback.
  * @param huart UART handle.
  * @retval None
  */
__weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_TxCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  Tx Half Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_TxHalfCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  Rx Half Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxHalfCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  UART error callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_ErrorCallback can be implemented in the user file.
   */
}

/**
  * @brief  UART Abort Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_AbortCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  UART Abort Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_AbortTransmitCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief  UART Abort Receive Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_AbortReceiveCpltCallback can be implemented in the user file.
   */
}

/**
  * @}
  */

/** @defgroup UART_Exported_Functions_Group3 Peripheral Control functions
  *  @brief   UART control functions
  *
@verbatim
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to control the UART.
     (+) HAL_MultiProcessor_EnableMuteMode() API enables mute mode
     (+) HAL_MultiProcessor_DisableMuteMode() API disables mute mode
     (+) HAL_MultiProcessor_EnterMuteMode() API enters mute mode
     (+) UART_SetConfig() API configures the UART peripheral
     (+) UART_AdvFeatureConfig() API optionally configures the UART advanced features
     (+) UART_CheckIdleState() API ensures that TEACK and/or REACK are set after initialization
     (+) HAL_HalfDuplex_EnableTransmitter() API disables receiver and enables transmitter
     (+) HAL_HalfDuplex_EnableReceiver() API disables transmitter and enables receiver
     (+) HAL_LIN_SendBreak() API transmits the break characters
@endverbatim
  * @{
  */

/**
  * @brief  Enable UART in mute mode (does not mean UART enters mute mode;
  *         to enter mute mode, HAL_MultiProcessor_EnterMuteMode() API must be called).
  * @param  huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MultiProcessor_EnableMuteMode(UART_HandleTypeDef *huart)
{
  __HAL_LOCK(huart);

  huart->gState = HAL_UART_STATE_BUSY;

  /* Enable USART mute mode by setting the MME bit in the CR1 register */
  SET_BIT(huart->Instance->CR1, USART_CR1_MME);

  huart->gState = HAL_UART_STATE_READY;

  return (UART_CheckIdleState(huart));
}

/**
  * @brief  Disable UART mute mode (does not mean the UART actually exits mute mode
  *         as it may not have been in mute mode at this very moment).
  * @param  huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MultiProcessor_DisableMuteMode(UART_HandleTypeDef *huart)
{
  __HAL_LOCK(huart);

  huart->gState = HAL_UART_STATE_BUSY;

  /* Disable USART mute mode by clearing the MME bit in the CR1 register */
  CLEAR_BIT(huart->Instance->CR1, USART_CR1_MME);

  huart->gState = HAL_UART_STATE_READY;

  return (UART_CheckIdleState(huart));
}

/**
  * @brief Enter UART mute mode (means UART actually enters mute mode).
  * @note  To exit from mute mode, HAL_MultiProcessor_DisableMuteMode() API must be called.
  * @param huart UART handle.
  * @retval None
  */
void HAL_MultiProcessor_EnterMuteMode(UART_HandleTypeDef *huart)
{
  __HAL_UART_SEND_REQ(huart, UART_MUTE_MODE_REQUEST);
}

/**
  * @brief  Enable the UART transmitter and disable the UART receiver.
  * @param  huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_HalfDuplex_EnableTransmitter(UART_HandleTypeDef *huart)
{
  __HAL_LOCK(huart);
  huart->gState = HAL_UART_STATE_BUSY;

  /* Clear TE and RE bits */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TE | USART_CR1_RE));

  /* Enable the USART's transmit interface by setting the TE bit in the USART CR1 register */
  SET_BIT(huart->Instance->CR1, USART_CR1_TE);

  huart->gState = HAL_UART_STATE_READY;

  __HAL_UNLOCK(huart);

  return HAL_OK;
}

/**
  * @brief  Enable the UART receiver and disable the UART transmitter.
  * @param  huart UART handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_HalfDuplex_EnableReceiver(UART_HandleTypeDef *huart)
{
  __HAL_LOCK(huart);
  huart->gState = HAL_UART_STATE_BUSY;

  /* Clear TE and RE bits */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TE | USART_CR1_RE));

  /* Enable the USART's receive interface by setting the RE bit in the USART CR1 register */
  SET_BIT(huart->Instance->CR1, USART_CR1_RE);

  huart->gState = HAL_UART_STATE_READY;

  __HAL_UNLOCK(huart);

  return HAL_OK;
}


/**
  * @brief  Transmit break characters.
  * @param  huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LIN_SendBreak(UART_HandleTypeDef *huart)
{
  /* Check the parameters */
  assert_param(IS_UART_LIN_INSTANCE(huart->Instance));

  __HAL_LOCK(huart);

  huart->gState = HAL_UART_STATE_BUSY;

  /* Send break characters */
  __HAL_UART_SEND_REQ(huart, UART_SENDBREAK_REQUEST);

  huart->gState = HAL_UART_STATE_READY;

  __HAL_UNLOCK(huart);

  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup UART_Exported_Functions_Group4 Peripheral State and Error functions
  *  @brief   UART Peripheral State functions
  *
@verbatim
  ==============================================================================
            ##### Peripheral State and Error functions #####
  ==============================================================================
    [..]
    This subsection provides functions allowing to :
      (+) Return the UART handle state.
      (+) Return the UART handle error code

@endverbatim
  * @{
  */

/**
  * @brief Return the UART handle state.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART.
  * @retval HAL state
  */
HAL_UART_StateTypeDef HAL_UART_GetState(UART_HandleTypeDef *huart)
{
  uint32_t temp1;
  uint32_t temp2;
  temp1 = huart->gState;
  temp2 = huart->RxState;

  return (HAL_UART_StateTypeDef)(temp1 | temp2);
}

/**
  * @brief  Return the UART handle error code.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART.
  * @retval UART Error Code
  */
uint32_t HAL_UART_GetError(UART_HandleTypeDef *huart)
{
  return huart->ErrorCode;
}
/**
  * @}
  */

/**
  * @}
  */

/** @defgroup UART_Private_Functions UART Private Functions
  * @{
  */

/**
  * @brief  Initialize the callbacks to their default values.
  * @param  huart UART handle.
  * @retval none
  */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
void UART_InitCallbacksToDefault(UART_HandleTypeDef *huart)
{
  /* Init the UART Callback settings */
  huart->TxHalfCpltCallback        = HAL_UART_TxHalfCpltCallback;        /* Legacy weak TxHalfCpltCallback        */
  huart->TxCpltCallback            = HAL_UART_TxCpltCallback;            /* Legacy weak TxCpltCallback            */
  huart->RxHalfCpltCallback        = HAL_UART_RxHalfCpltCallback;        /* Legacy weak RxHalfCpltCallback        */
  huart->RxCpltCallback            = HAL_UART_RxCpltCallback;            /* Legacy weak RxCpltCallback            */
  huart->ErrorCallback             = HAL_UART_ErrorCallback;             /* Legacy weak ErrorCallback             */
  huart->AbortCpltCallback         = HAL_UART_AbortCpltCallback;         /* Legacy weak AbortCpltCallback         */
  huart->AbortTransmitCpltCallback = HAL_UART_AbortTransmitCpltCallback; /* Legacy weak AbortTransmitCpltCallback */
  huart->AbortReceiveCpltCallback  = HAL_UART_AbortReceiveCpltCallback;  /* Legacy weak AbortReceiveCpltCallback  */
  huart->WakeupCallback            = HAL_UARTEx_WakeupCallback;          /* Legacy weak WakeupCallback            */
#if defined(USART_CR1_FIFOEN)
  huart->RxFifoFullCallback        = HAL_UARTEx_RxFifoFullCallback;      /* Legacy weak RxFifoFullCallback        */
  huart->TxFifoEmptyCallback       = HAL_UARTEx_TxFifoEmptyCallback;     /* Legacy weak TxFifoEmptyCallback       */
#endif /* USART_CR1_FIFOEN */

}
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

/**
  * @brief Configure the UART peripheral.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef UART_SetConfig(UART_HandleTypeDef *huart)
{
  uint32_t tmpreg;
  uint16_t brrtemp;
  UART_ClockSourceTypeDef clocksource;
  uint32_t usartdiv                   = 0x00000000U;
  HAL_StatusTypeDef ret               = HAL_OK;
  uint32_t lpuart_ker_ck_pres         = 0x00000000U;

  /* Check the parameters */
  assert_param(IS_UART_BAUDRATE(huart->Init.BaudRate));
  assert_param(IS_UART_WORD_LENGTH(huart->Init.WordLength));
  if (UART_INSTANCE_LOWPOWER(huart))
  {
    assert_param(IS_LPUART_STOPBITS(huart->Init.StopBits));
  }
  else
  {
    assert_param(IS_UART_STOPBITS(huart->Init.StopBits));
    assert_param(IS_UART_ONE_BIT_SAMPLE(huart->Init.OneBitSampling));
  }

  assert_param(IS_UART_PARITY(huart->Init.Parity));
  assert_param(IS_UART_MODE(huart->Init.Mode));
  assert_param(IS_UART_HARDWARE_FLOW_CONTROL(huart->Init.HwFlowCtl));
  assert_param(IS_UART_OVERSAMPLING(huart->Init.OverSampling));
#if defined(USART_PRESC_PRESCALER)
  assert_param(IS_UART_PRESCALER(huart->Init.ClockPrescaler));
#endif /* USART_PRESC_PRESCALER */

  /*-------------------------- USART CR1 Configuration -----------------------*/
  /* Clear M, PCE, PS, TE, RE and OVER8 bits and configure
  *  the UART Word Length, Parity, Mode and oversampling:
  *  set the M bits according to huart->Init.WordLength value
  *  set PCE and PS bits according to huart->Init.Parity value
  *  set TE and RE bits according to huart->Init.Mode value
  *  set OVER8 bit according to huart->Init.OverSampling value */
  tmpreg = (uint32_t)huart->Init.WordLength | huart->Init.Parity | huart->Init.Mode | huart->Init.OverSampling ;
#if defined(USART_CR1_FIFOEN)
  tmpreg |= (uint32_t)huart->FifoMode;
#endif /* USART_CR1_FIFOEN */
  MODIFY_REG(huart->Instance->CR1, USART_CR1_FIELDS, tmpreg);

  /*-------------------------- USART CR2 Configuration -----------------------*/
  /* Configure the UART Stop Bits: Set STOP[13:12] bits according
  * to huart->Init.StopBits value */
  MODIFY_REG(huart->Instance->CR2, USART_CR2_STOP, huart->Init.StopBits);

  /*-------------------------- USART CR3 Configuration -----------------------*/
  /* Configure
  * - UART HardWare Flow Control: set CTSE and RTSE bits according
  *   to huart->Init.HwFlowCtl value
  * - one-bit sampling method versus three samples' majority rule according
  *   to huart->Init.OneBitSampling (not applicable to LPUART) */
  tmpreg = (uint32_t)huart->Init.HwFlowCtl;

  if (!(UART_INSTANCE_LOWPOWER(huart)))
  {
    tmpreg |= huart->Init.OneBitSampling;
  }
  MODIFY_REG(huart->Instance->CR3, USART_CR3_FIELDS, tmpreg);

#if defined(USART_PRESC_PRESCALER)
  /*-------------------------- USART PRESC Configuration -----------------------*/
  /* Configure
  * - UART Clock Prescaler : set PRESCALER according to huart->Init.ClockPrescaler value */
  MODIFY_REG(huart->Instance->PRESC, USART_PRESC_PRESCALER, huart->Init.ClockPrescaler);
#endif /* USART_PRESC_PRESCALER */

  /*-------------------------- USART BRR Configuration -----------------------*/
  UART_GETCLOCKSOURCE(huart, clocksource);

  /* Check LPUART instance */
  if (UART_INSTANCE_LOWPOWER(huart))
  {
    /* Retrieve frequency clock */
    switch (clocksource)
    {
      case UART_CLOCKSOURCE_PCLK1:
#if defined(USART_PRESC_PRESCALER)
        lpuart_ker_ck_pres = (HAL_RCC_GetPCLK1Freq() / UART_GET_DIV_FACTOR(huart->Init.ClockPrescaler));
#else
        lpuart_ker_ck_pres = HAL_RCC_GetPCLK1Freq();
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_HSI:
#if defined(USART_PRESC_PRESCALER)
        lpuart_ker_ck_pres = ((uint32_t)HSI_VALUE / UART_GET_DIV_FACTOR(huart->Init.ClockPrescaler));
#else
        lpuart_ker_ck_pres = (uint32_t)HSI_VALUE;
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_SYSCLK:
#if defined(USART_PRESC_PRESCALER)
        lpuart_ker_ck_pres = (HAL_RCC_GetSysClockFreq() / UART_GET_DIV_FACTOR(huart->Init.ClockPrescaler));
#else
        lpuart_ker_ck_pres = HAL_RCC_GetSysClockFreq();
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_LSE:
#if defined(USART_PRESC_PRESCALER)
        lpuart_ker_ck_pres = ((uint32_t)LSE_VALUE / UART_GET_DIV_FACTOR(huart->Init.ClockPrescaler));
#else
        lpuart_ker_ck_pres = (uint32_t)LSE_VALUE;
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_UNDEFINED:
      default:
        ret = HAL_ERROR;
        break;
    }

    /* if proper clock source reported */
    if (lpuart_ker_ck_pres != 0U)
    {
      /* ensure that Frequency clock is in the range [3 * baudrate, 4096 * baudrate] */
      if ((lpuart_ker_ck_pres < (3U * huart->Init.BaudRate)) ||
          (lpuart_ker_ck_pres > (4096U * huart->Init.BaudRate)))
      {
        ret = HAL_ERROR;
      }
      else
      {
        switch (clocksource)
        {
          case UART_CLOCKSOURCE_PCLK1:
#if defined(USART_PRESC_PRESCALER)
            usartdiv = (uint32_t)(UART_DIV_LPUART(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
            usartdiv = (uint32_t)(UART_DIV_LPUART(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
            break;
          case UART_CLOCKSOURCE_HSI:
#if defined(USART_PRESC_PRESCALER)
            usartdiv = (uint32_t)(UART_DIV_LPUART(HSI_VALUE, huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
            usartdiv = (uint32_t)(UART_DIV_LPUART(HSI_VALUE, huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
            break;
          case UART_CLOCKSOURCE_SYSCLK:
#if defined(USART_PRESC_PRESCALER)
            usartdiv = (uint32_t)(UART_DIV_LPUART(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
            usartdiv = (uint32_t)(UART_DIV_LPUART(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
            break;
          case UART_CLOCKSOURCE_LSE:
#if defined(USART_PRESC_PRESCALER)
            usartdiv = (uint32_t)(UART_DIV_LPUART(LSE_VALUE, huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
            usartdiv = (uint32_t)(UART_DIV_LPUART(LSE_VALUE, huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
            break;
          case UART_CLOCKSOURCE_UNDEFINED:
          default:
            ret = HAL_ERROR;
            break;
        }

        /* It is forbidden to write values lower than 0x300 in the LPUART_BRR register */
        if ((usartdiv >= LPUART_BRR_MIN) && (usartdiv <= LPUART_BRR_MAX))
        {
          huart->Instance->BRR = usartdiv;
        }
        else
        {
          ret = HAL_ERROR;
        }
      } /*   if ( (lpuart_ker_ck_pres < (3 * huart->Init.BaudRate) ) || (lpuart_ker_ck_pres > (4096 * huart->Init.BaudRate) )) */
    } /* if (lpuart_ker_ck_pres != 0) */
  }
  /* Check UART Over Sampling to set Baud Rate Register */
  else if (huart->Init.OverSampling == UART_OVERSAMPLING_8)
  {
    switch (clocksource)
    {
      case UART_CLOCKSOURCE_PCLK1:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_PCLK2:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HAL_RCC_GetPCLK2Freq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HAL_RCC_GetPCLK2Freq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_HSI:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HSI_VALUE, huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HSI_VALUE, huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_SYSCLK:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_LSE:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8((uint32_t)LSE_VALUE, huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING8(LSE_VALUE, huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_UNDEFINED:
      default:
        ret = HAL_ERROR;
        break;
    }

    /* USARTDIV must be greater than or equal to 0d16 */
    if ((usartdiv >= UART_BRR_MIN) && (usartdiv <= UART_BRR_MAX))
    {
      brrtemp = (uint16_t)(usartdiv & 0xFFF0U);
      brrtemp |= (uint16_t)((usartdiv & (uint16_t)0x000FU) >> 1U);
      huart->Instance->BRR = brrtemp;
    }
    else
    {
      ret = HAL_ERROR;
    }
  }
  else
  {
    switch (clocksource)
    {
      case UART_CLOCKSOURCE_PCLK1:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_PCLK2:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetPCLK2Freq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetPCLK2Freq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_HSI:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HSI_VALUE, huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HSI_VALUE, huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_SYSCLK:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_LSE:
#if defined(USART_PRESC_PRESCALER)
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16((uint32_t)LSE_VALUE, huart->Init.BaudRate, huart->Init.ClockPrescaler));
#else
        usartdiv = (uint16_t)(UART_DIV_SAMPLING16(LSE_VALUE, huart->Init.BaudRate));
#endif /* USART_PRESC_PRESCALER */
        break;
      case UART_CLOCKSOURCE_UNDEFINED:
      default:
        ret = HAL_ERROR;
        break;
    }

    /* USARTDIV must be greater than or equal to 0d16 */
    if ((usartdiv >= UART_BRR_MIN) && (usartdiv <= UART_BRR_MAX))
    {
      huart->Instance->BRR = usartdiv;
    }
    else
    {
      ret = HAL_ERROR;
    }
  }

#if defined(USART_CR1_FIFOEN)
  /* Initialize the number of data to process during RX/TX ISR execution */
  huart->NbTxDataToProcess = 1;
  huart->NbRxDataToProcess = 1;
#endif /* USART_CR1_FIFOEN */

  /* Clear ISR function pointers */
  huart->RxISR = NULL;
  huart->TxISR = NULL;

  return ret;
}

/**
  * @brief Configure the UART peripheral advanced features.
  * @param huart UART handle.
  * @retval None
  */
void UART_AdvFeatureConfig(UART_HandleTypeDef *huart)
{
  /* Check whether the set of advanced features to configure is properly set */
  assert_param(IS_UART_ADVFEATURE_INIT(huart->AdvancedInit.AdvFeatureInit));

  /* if required, configure TX pin active level inversion */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_TXINVERT_INIT))
  {
    assert_param(IS_UART_ADVFEATURE_TXINV(huart->AdvancedInit.TxPinLevelInvert));
    MODIFY_REG(huart->Instance->CR2, USART_CR2_TXINV, huart->AdvancedInit.TxPinLevelInvert);
  }

  /* if required, configure RX pin active level inversion */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_RXINVERT_INIT))
  {
    assert_param(IS_UART_ADVFEATURE_RXINV(huart->AdvancedInit.RxPinLevelInvert));
    MODIFY_REG(huart->Instance->CR2, USART_CR2_RXINV, huart->AdvancedInit.RxPinLevelInvert);
  }

  /* if required, configure data inversion */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_DATAINVERT_INIT))
  {
    assert_param(IS_UART_ADVFEATURE_DATAINV(huart->AdvancedInit.DataInvert));
    MODIFY_REG(huart->Instance->CR2, USART_CR2_DATAINV, huart->AdvancedInit.DataInvert);
  }

  /* if required, configure RX/TX pins swap */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_SWAP_INIT))
  {
    assert_param(IS_UART_ADVFEATURE_SWAP(huart->AdvancedInit.Swap));
    MODIFY_REG(huart->Instance->CR2, USART_CR2_SWAP, huart->AdvancedInit.Swap);
  }

  /* if required, configure RX overrun detection disabling */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_RXOVERRUNDISABLE_INIT))
  {
    assert_param(IS_UART_OVERRUN(huart->AdvancedInit.OverrunDisable));
    MODIFY_REG(huart->Instance->CR3, USART_CR3_OVRDIS, huart->AdvancedInit.OverrunDisable);
  }

  /* if required, configure DMA disabling on reception error */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_DMADISABLEONERROR_INIT))
  {
    assert_param(IS_UART_ADVFEATURE_DMAONRXERROR(huart->AdvancedInit.DMADisableonRxError));
    MODIFY_REG(huart->Instance->CR3, USART_CR3_DDRE, huart->AdvancedInit.DMADisableonRxError);
  }

  /* if required, configure auto Baud rate detection scheme */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_AUTOBAUDRATE_INIT))
  {
    assert_param(IS_USART_AUTOBAUDRATE_DETECTION_INSTANCE(huart->Instance));
    assert_param(IS_UART_ADVFEATURE_AUTOBAUDRATE(huart->AdvancedInit.AutoBaudRateEnable));
    MODIFY_REG(huart->Instance->CR2, USART_CR2_ABREN, huart->AdvancedInit.AutoBaudRateEnable);
    /* set auto Baudrate detection parameters if detection is enabled */
    if (huart->AdvancedInit.AutoBaudRateEnable == UART_ADVFEATURE_AUTOBAUDRATE_ENABLE)
    {
      assert_param(IS_UART_ADVFEATURE_AUTOBAUDRATEMODE(huart->AdvancedInit.AutoBaudRateMode));
      MODIFY_REG(huart->Instance->CR2, USART_CR2_ABRMODE, huart->AdvancedInit.AutoBaudRateMode);
    }
  }

  /* if required, configure MSB first on communication line */
  if (HAL_IS_BIT_SET(huart->AdvancedInit.AdvFeatureInit, UART_ADVFEATURE_MSBFIRST_INIT))
  {
    assert_param(IS_UART_ADVFEATURE_MSBFIRST(huart->AdvancedInit.MSBFirst));
    MODIFY_REG(huart->Instance->CR2, USART_CR2_MSBFIRST, huart->AdvancedInit.MSBFirst);
  }
}

/**
  * @brief Check the UART Idle State.
  * @param huart UART handle.
  * @retval HAL status
  */
HAL_StatusTypeDef UART_CheckIdleState(UART_HandleTypeDef *huart)
{
  uint32_t tickstart;

  /* Initialize the UART ErrorCode */
  huart->ErrorCode = HAL_UART_ERROR_NONE;

  /* Init tickstart for timeout managment*/
  tickstart = HAL_GetTick();

  /* Check if the Transmitter is enabled */
  if ((huart->Instance->CR1 & USART_CR1_TE) == USART_CR1_TE)
  {
    /* Wait until TEACK flag is set */
    if (UART_WaitOnFlagUntilTimeout(huart, USART_ISR_TEACK, RESET, tickstart, HAL_UART_TIMEOUT_VALUE) != HAL_OK)
    {
      /* Timeout occurred */
      return HAL_TIMEOUT;
    }
  }

  /* Check if the Receiver is enabled */
  if ((huart->Instance->CR1 & USART_CR1_RE) == USART_CR1_RE)
  {
    /* Wait until REACK flag is set */
    if (UART_WaitOnFlagUntilTimeout(huart, USART_ISR_REACK, RESET, tickstart, HAL_UART_TIMEOUT_VALUE) != HAL_OK)
    {
      /* Timeout occurred */
      return HAL_TIMEOUT;
    }
  }

  /* Initialize the UART State */
  huart->gState = HAL_UART_STATE_READY;
  huart->RxState = HAL_UART_STATE_READY;

  __HAL_UNLOCK(huart);

  return HAL_OK;
}

/**
  * @brief  Handle UART Communication Timeout.
  * @param huart     UART handle.
  * @param Flag      Specifies the UART flag to check
  * @param Status    Flag status (SET or RESET)
  * @param Tickstart Tick start value
  * @param Timeout   Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status,
                                              uint32_t Tickstart, uint32_t Timeout)
{
  /* Wait until flag is set */
  while ((__HAL_UART_GET_FLAG(huart, Flag) ? SET : RESET) == Status)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        /* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
#if defined(USART_CR1_FIFOEN)
        CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE | USART_CR1_TXEIE_TXFNFIE));
#else
        CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_TXEIE));
#endif /* USART_CR1_FIFOEN */
        CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

        huart->gState = HAL_UART_STATE_READY;
        huart->RxState = HAL_UART_STATE_READY;

        __HAL_UNLOCK(huart);

        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}


/**
  * @brief  End ongoing Tx transfer on UART peripheral (following error detection or Transmit completion).
  * @param  huart UART handle.
  * @retval None
  */
static void UART_EndTxTransfer(UART_HandleTypeDef *huart)
{
#if defined(USART_CR1_FIFOEN)
  /* Disable TXEIE, TCIE, TXFT interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE_TXFNFIE | USART_CR1_TCIE));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_TXFTIE));
#else
  /* Disable TXEIE and TCIE interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
#endif /* USART_CR1_FIFOEN */

  /* At end of Tx process, restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;
}


/**
  * @brief  End ongoing Rx transfer on UART peripheral (following error detection or Reception completion).
  * @param  huart UART handle.
  * @retval None
  */
static void UART_EndRxTransfer(UART_HandleTypeDef *huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
#if defined(USART_CR1_FIFOEN)
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE));
#else
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);
#endif /* USART_CR1_FIFOEN */

  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;

  /* Reset RxIsr function pointer */
  huart->RxISR = NULL;
}


/**
  * @brief DMA UART transmit process complete callback.
  * @param hdma DMA handle.
  * @retval None
  */
static void UART_DMATransmitCplt(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

  /* DMA Normal mode */
  if (HAL_IS_BIT_CLR(hdma->Instance->CCR, DMA_CCR_CIRC))
  {
    huart->TxXferCount = 0U;

    /* Disable the DMA transfer for transmit request by resetting the DMAT bit
       in the UART CR3 register */
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    /* Enable the UART Transmit Complete Interrupt */
    SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
  }
  /* DMA Circular mode */
  else
  {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered Tx complete callback*/
    huart->TxCpltCallback(huart);
#else
    /*Call legacy weak Tx complete callback*/
    HAL_UART_TxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }
}

/**
  * @brief DMA UART transmit process half complete callback.
  * @param hdma DMA handle.
  * @retval None
  */
static void UART_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered Tx Half complete callback*/
  huart->TxHalfCpltCallback(huart);
#else
  /*Call legacy weak Tx Half complete callback*/
  HAL_UART_TxHalfCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief DMA UART receive process complete callback.
  * @param hdma DMA handle.
  * @retval None
  */
static void UART_DMAReceiveCplt(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

  /* DMA Normal mode */
  if (HAL_IS_BIT_CLR(hdma->Instance->CCR, DMA_CCR_CIRC))
  {
    huart->RxXferCount = 0U;

    /* Disable PE and ERR (Frame error, noise error, overrun error) interrupts */
    CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

    /* Disable the DMA transfer for the receiver request by resetting the DMAR bit
       in the UART CR3 register */
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;
  }

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered Rx complete callback*/
  huart->RxCpltCallback(huart);
#else
  /*Call legacy weak Rx complete callback*/
  HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief DMA UART receive process half complete callback.
  * @param hdma DMA handle.
  * @retval None
  */
static void UART_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered Rx Half complete callback*/
  huart->RxHalfCpltCallback(huart);
#else
  /*Call legacy weak Rx Half complete callback*/
  HAL_UART_RxHalfCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief DMA UART communication error callback.
  * @param hdma DMA handle.
  * @retval None
  */
static void UART_DMAError(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

  const HAL_UART_StateTypeDef gstate = huart->gState;
  const HAL_UART_StateTypeDef rxstate = huart->RxState;

  /* Stop UART DMA Tx request if ongoing */
  if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT)) &&
      (gstate == HAL_UART_STATE_BUSY_TX))
  {
    huart->TxXferCount = 0U;
    UART_EndTxTransfer(huart);
  }

  /* Stop UART DMA Rx request if ongoing */
  if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)) &&
      (rxstate == HAL_UART_STATE_BUSY_RX))
  {
    huart->RxXferCount = 0U;
    UART_EndRxTransfer(huart);
  }

  huart->ErrorCode |= HAL_UART_ERROR_DMA;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered error callback*/
  huart->ErrorCallback(huart);
#else
  /*Call legacy weak error callback*/
  HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART communication abort callback, when initiated by HAL services on Error
  *         (To be called at end of DMA Abort procedure following error occurrence).
  * @param  hdma DMA handle.
  * @retval None
  */
static void UART_DMAAbortOnError(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);
  huart->RxXferCount = 0U;
  huart->TxXferCount = 0U;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered error callback*/
  huart->ErrorCallback(huart);
#else
  /*Call legacy weak error callback*/
  HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART Tx communication abort callback, when initiated by user
  *         (To be called at end of DMA Tx Abort procedure following user abort request).
  * @note   When this callback is executed, User Abort complete call back is called only if no
  *         Abort still ongoing for Rx DMA Handle.
  * @param  hdma DMA handle.
  * @retval None
  */
static void UART_DMATxAbortCallback(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

  huart->hdmatx->XferAbortCallback = NULL;

  /* Check if an Abort process is still ongoing */
  if (huart->hdmarx != NULL)
  {
    if (huart->hdmarx->XferAbortCallback != NULL)
    {
      return;
    }
  }

  /* No Abort process still ongoing : All DMA channels are aborted, call user Abort Complete callback */
  huart->TxXferCount = 0U;
  huart->RxXferCount = 0U;

  /* Reset errorCode */
  huart->ErrorCode = HAL_UART_ERROR_NONE;

  /* Clear the Error flags in the ICR register */
  __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

#if defined(USART_CR1_FIFOEN)
  /* Flush the whole TX FIFO (if needed) */
  if (huart->FifoMode == UART_FIFOMODE_ENABLE)
  {
    __HAL_UART_SEND_REQ(huart, UART_TXDATA_FLUSH_REQUEST);
  }
#endif /* USART_CR1_FIFOEN */

  /* Restore huart->gState and huart->RxState to Ready */
  huart->gState  = HAL_UART_STATE_READY;
  huart->RxState = HAL_UART_STATE_READY;

  /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /* Call registered Abort complete callback */
  huart->AbortCpltCallback(huart);
#else
  /* Call legacy weak Abort complete callback */
  HAL_UART_AbortCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}


/**
  * @brief  DMA UART Rx communication abort callback, when initiated by user
  *         (To be called at end of DMA Rx Abort procedure following user abort request).
  * @note   When this callback is executed, User Abort complete call back is called only if no
  *         Abort still ongoing for Tx DMA Handle.
  * @param  hdma DMA handle.
  * @retval None
  */
static void UART_DMARxAbortCallback(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

  huart->hdmarx->XferAbortCallback = NULL;

  /* Check if an Abort process is still ongoing */
  if (huart->hdmatx != NULL)
  {
    if (huart->hdmatx->XferAbortCallback != NULL)
    {
      return;
    }
  }

  /* No Abort process still ongoing : All DMA channels are aborted, call user Abort Complete callback */
  huart->TxXferCount = 0U;
  huart->RxXferCount = 0U;

  /* Reset errorCode */
  huart->ErrorCode = HAL_UART_ERROR_NONE;

  /* Clear the Error flags in the ICR register */
  __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

  /* Discard the received data */
  __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

  /* Restore huart->gState and huart->RxState to Ready */
  huart->gState  = HAL_UART_STATE_READY;
  huart->RxState = HAL_UART_STATE_READY;

  /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /* Call registered Abort complete callback */
  huart->AbortCpltCallback(huart);
#else
  /* Call legacy weak Abort complete callback */
  HAL_UART_AbortCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}


/**
  * @brief  DMA UART Tx communication abort callback, when initiated by user by a call to
  *         HAL_UART_AbortTransmit_IT API (Abort only Tx transfer)
  *         (This callback is executed at end of DMA Tx Abort procedure following user abort request,
  *         and leads to user Tx Abort Complete callback execution).
  * @param  hdma DMA handle.
  * @retval None
  */
static void UART_DMATxOnlyAbortCallback(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

  huart->TxXferCount = 0U;

#if defined(USART_CR1_FIFOEN)
  /* Flush the whole TX FIFO (if needed) */
  if (huart->FifoMode == UART_FIFOMODE_ENABLE)
  {
    __HAL_UART_SEND_REQ(huart, UART_TXDATA_FLUSH_REQUEST);
  }
#endif /* USART_CR1_FIFOEN */

  /* Restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;

  /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /* Call registered Abort Transmit Complete Callback */
  huart->AbortTransmitCpltCallback(huart);
#else
  /* Call legacy weak Abort Transmit Complete Callback */
  HAL_UART_AbortTransmitCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART Rx communication abort callback, when initiated by user by a call to
  *         HAL_UART_AbortReceive_IT API (Abort only Rx transfer)
  *         (This callback is executed at end of DMA Rx Abort procedure following user abort request,
  *         and leads to user Rx Abort Complete callback execution).
  * @param  hdma DMA handle.
  * @retval None
  */
static void UART_DMARxOnlyAbortCallback(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  huart->RxXferCount = 0U;

  /* Clear the Error flags in the ICR register */
  __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

  /* Discard the received data */
  __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

  /* Restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;

  /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /* Call registered Abort Receive Complete Callback */
  huart->AbortReceiveCpltCallback(huart);
#else
  /* Call legacy weak Abort Receive Complete Callback */
  HAL_UART_AbortReceiveCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief TX interrrupt handler for 7 or 8 bits data word length .
  * @note   Function is called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Transmit_IT().
  * @param huart UART handle.
  * @retval None
  */
static void UART_TxISR_8BIT(UART_HandleTypeDef *huart)
{
  /* Check that a Tx process is ongoing */
  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {
    if (huart->TxXferCount == 0U)
    {
      /* Disable the UART Transmit Data Register Empty Interrupt */
#if defined(USART_CR1_FIFOEN)
      CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE_TXFNFIE);
#else
      CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
#endif /* USART_CR1_FIFOEN */

      /* Enable the UART Transmit Complete Interrupt */
      SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
    }
    else
    {
      huart->Instance->TDR = (uint8_t)(*huart->pTxBuffPtr & (uint8_t)0xFF);
      huart->pTxBuffPtr++;
      huart->TxXferCount--;
    }
  }
}

/**
  * @brief TX interrrupt handler for 9 bits data word length.
  * @note   Function is called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Transmit_IT().
  * @param huart UART handle.
  * @retval None
  */
static void UART_TxISR_16BIT(UART_HandleTypeDef *huart)
{
  uint16_t *tmp;

  /* Check that a Tx process is ongoing */
  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {
    if (huart->TxXferCount == 0U)
    {
      /* Disable the UART Transmit Data Register Empty Interrupt */
#if defined(USART_CR1_FIFOEN)
      CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE_TXFNFIE);
#else
      CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
#endif /* USART_CR1_FIFOEN */

      /* Enable the UART Transmit Complete Interrupt */
      SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
    }
    else
    {
      tmp = (uint16_t *) huart->pTxBuffPtr;
      huart->Instance->TDR = (((uint32_t)(*tmp)) & 0x01FFUL);
      huart->pTxBuffPtr += 2U;
      huart->TxXferCount--;
    }
  }
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief TX interrrupt handler for 7 or 8 bits data word length and FIFO mode is enabled.
  * @note   Function is called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Transmit_IT().
  * @param huart UART handle.
  * @retval None
  */
static void UART_TxISR_8BIT_FIFOEN(UART_HandleTypeDef *huart)
{
  uint16_t  nb_tx_data;

  /* Check that a Tx process is ongoing */
  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {
    for (nb_tx_data = huart->NbTxDataToProcess ; nb_tx_data > 0U ; nb_tx_data--)
    {
      if (huart->TxXferCount == 0U)
      {
        /* Disable the TX FIFO threshold interrupt */
        CLEAR_BIT(huart->Instance->CR3, USART_CR3_TXFTIE);

        /* Enable the UART Transmit Complete Interrupt */
        SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);

        break; /* force exit loop */
      }
      else if (READ_BIT(huart->Instance->ISR, USART_ISR_TXE_TXFNF) != 0U)
      {
        huart->Instance->TDR = (uint8_t)(*huart->pTxBuffPtr & (uint8_t)0xFF);
        huart->pTxBuffPtr++;
        huart->TxXferCount--;
      }
      else
      {
        /* Nothing to do */
      }
    }
  }
}

/**
  * @brief TX interrrupt handler for 9 bits data word length and FIFO mode is enabled.
  * @note   Function is called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Transmit_IT().
  * @param huart UART handle.
  * @retval None
  */
static void UART_TxISR_16BIT_FIFOEN(UART_HandleTypeDef *huart)
{
  uint16_t *tmp;
  uint16_t  nb_tx_data;

  /* Check that a Tx process is ongoing */
  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {
    for (nb_tx_data = huart->NbTxDataToProcess ; nb_tx_data > 0U ; nb_tx_data--)
    {
      if (huart->TxXferCount == 0U)
      {
        /* Disable the TX FIFO threshold interrupt */
        CLEAR_BIT(huart->Instance->CR3, USART_CR3_TXFTIE);

        /* Enable the UART Transmit Complete Interrupt */
        SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);

        break; /* force exit loop */
      }
      else if (READ_BIT(huart->Instance->ISR, USART_ISR_TXE_TXFNF) != 0U)
      {
        tmp = (uint16_t *) huart->pTxBuffPtr;
        huart->Instance->TDR = (((uint32_t)(*tmp)) & 0x01FFUL);
        huart->pTxBuffPtr += 2U;
        huart->TxXferCount--;
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
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
static void UART_EndTransmit_IT(UART_HandleTypeDef *huart)
{
  /* Disable the UART Transmit Complete Interrupt */
  CLEAR_BIT(huart->Instance->CR1, USART_CR1_TCIE);

  /* Tx process is ended, restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;

  /* Cleat TxISR function pointer */
  huart->TxISR = NULL;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered Tx complete callback*/
  huart->TxCpltCallback(huart);
#else
  /*Call legacy weak Tx complete callback*/
  HAL_UART_TxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief RX interrrupt handler for 7 or 8 bits data word length .
  * @param huart UART handle.
  * @retval None
  */
static void UART_RxISR_8BIT(UART_HandleTypeDef *huart)
{
  uint16_t uhMask = huart->Mask;
  uint16_t  uhdata;

  /* Check that a Rx process is ongoing */
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
    uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
    *huart->pRxBuffPtr = (uint8_t)(uhdata & (uint8_t)uhMask);
    huart->pRxBuffPtr++;
    huart->RxXferCount--;

    if (huart->RxXferCount == 0U)
    {
      /* Disable the UART Parity Error Interrupt and RXNE interrupts */
#if defined(USART_CR1_FIFOEN)
      CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE));
#else
      CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
#endif /* USART_CR1_FIFOEN */

      /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
      CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

      /* Rx process is completed, restore huart->RxState to Ready */
      huart->RxState = HAL_UART_STATE_READY;

      /* Clear RxISR function pointer */
      huart->RxISR = NULL;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
      /*Call registered Rx complete callback*/
      huart->RxCpltCallback(huart);
#else
      /*Call legacy weak Rx complete callback*/
      HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
  }
  else
  {
    /* Clear RXNE interrupt flag */
    __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
  }
}

/**
  * @brief RX interrrupt handler for 9 bits data word length .
  * @note   Function is called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Receive_IT()
  * @param huart UART handle.
  * @retval None
  */
static void UART_RxISR_16BIT(UART_HandleTypeDef *huart)
{
  uint16_t *tmp;
  uint16_t uhMask = huart->Mask;
  uint16_t  uhdata;

  /* Check that a Rx process is ongoing */
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
    uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
    tmp = (uint16_t *) huart->pRxBuffPtr ;
    *tmp = (uint16_t)(uhdata & uhMask);
    huart->pRxBuffPtr += 2U;
    huart->RxXferCount--;

    if (huart->RxXferCount == 0U)
    {
      /* Disable the UART Parity Error Interrupt and RXNE interrupt*/
#if defined(USART_CR1_FIFOEN)
      CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE));
#else
      CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
#endif /* USART_CR1_FIFOEN */

      /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
      CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

      /* Rx process is completed, restore huart->RxState to Ready */
      huart->RxState = HAL_UART_STATE_READY;

      /* Clear RxISR function pointer */
      huart->RxISR = NULL;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
      /*Call registered Rx complete callback*/
      huart->RxCpltCallback(huart);
#else
      /*Call legacy weak Rx complete callback*/
      HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
  }
  else
  {
    /* Clear RXNE interrupt flag */
    __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
  }
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief RX interrrupt handler for 7 or 8  bits data word length and FIFO mode is enabled.
  * @note   Function is called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Receive_IT()
  * @param huart UART handle.
  * @retval None
  */
static void UART_RxISR_8BIT_FIFOEN(UART_HandleTypeDef *huart)
{
  uint16_t  uhMask = huart->Mask;
  uint16_t  uhdata;
  uint16_t   nb_rx_data;
  uint16_t  rxdatacount;

  /* Check that a Rx process is ongoing */
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
    for (nb_rx_data = huart->NbRxDataToProcess ; nb_rx_data > 0U ; nb_rx_data--)
    {
      uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
      *huart->pRxBuffPtr = (uint8_t)(uhdata & (uint8_t)uhMask);
      huart->pRxBuffPtr++;
      huart->RxXferCount--;

      if (huart->RxXferCount == 0U)
      {
        /* Disable the UART Parity Error Interrupt and RXFT interrupt*/
        CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);

        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) and RX FIFO Threshold interrupt */
        CLEAR_BIT(huart->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE));

        /* Rx process is completed, restore huart->RxState to Ready */
        huart->RxState = HAL_UART_STATE_READY;

        /* Clear RxISR function pointer */
        huart->RxISR = NULL;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx complete callback*/
        huart->RxCpltCallback(huart);
#else
        /*Call legacy weak Rx complete callback*/
        HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
      }
    }

    /* When remaining number of bytes to receive is less than the RX FIFO
    threshold, next incoming frames are processed as if FIFO mode was
    disabled (i.e. one interrupt per received frame).
    */
    rxdatacount = huart->RxXferCount;
    if ((rxdatacount != 0U) && (rxdatacount < huart->NbRxDataToProcess))
    {
      /* Disable the UART RXFT interrupt*/
      CLEAR_BIT(huart->Instance->CR3, USART_CR3_RXFTIE);

      /* Update the RxISR function pointer */
      huart->RxISR = UART_RxISR_8BIT;

      /* Enable the UART Data Register Not Empty interrupt */
      SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE_RXFNEIE);
    }
  }
  else
  {
    /* Clear RXNE interrupt flag */
    __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
  }
}

/**
  * @brief RX interrrupt handler for 9 bits data word length and FIFO mode is enabled.
  * @note   Function is called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Receive_IT()
  * @param huart UART handle.
  * @retval None
  */
static void UART_RxISR_16BIT_FIFOEN(UART_HandleTypeDef *huart)
{
  uint16_t *tmp;
  uint16_t  uhMask = huart->Mask;
  uint16_t  uhdata;
  uint16_t   nb_rx_data;
  uint16_t  rxdatacount;

  /* Check that a Rx process is ongoing */
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
    for (nb_rx_data = huart->NbRxDataToProcess ; nb_rx_data > 0U ; nb_rx_data--)
    {
      uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
      tmp = (uint16_t *) huart->pRxBuffPtr ;
      *tmp = (uint16_t)(uhdata & uhMask);
      huart->pRxBuffPtr += 2U;
      huart->RxXferCount--;

      if (huart->RxXferCount == 0U)
      {
        /* Disable the UART Parity Error Interrupt and RXFT interrupt*/
        CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);

        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) and RX FIFO Threshold interrupt */
        CLEAR_BIT(huart->Instance->CR3, (USART_CR3_EIE | USART_CR3_RXFTIE));

        /* Rx process is completed, restore huart->RxState to Ready */
        huart->RxState = HAL_UART_STATE_READY;

        /* Clear RxISR function pointer */
        huart->RxISR = NULL;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx complete callback*/
        huart->RxCpltCallback(huart);
#else
        /*Call legacy weak Rx complete callback*/
        HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
      }
    }

    /* When remaining number of bytes to receive is less than the RX FIFO
    threshold, next incoming frames are processed as if FIFO mode was
    disabled (i.e. one interrupt per received frame).
    */
    rxdatacount = huart->RxXferCount;
    if ((rxdatacount != 0U) && (rxdatacount < huart->NbRxDataToProcess))
    {
      /* Disable the UART RXFT interrupt*/
      CLEAR_BIT(huart->Instance->CR3, USART_CR3_RXFTIE);

      /* Update the RxISR function pointer */
      huart->RxISR = UART_RxISR_16BIT;

      /* Enable the UART Data Register Not Empty interrupt */
      SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE_RXFNEIE);
    }
  }
  else
  {
    /* Clear RXNE interrupt flag */
    __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
  }
}
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

#endif /* HAL_UART_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
