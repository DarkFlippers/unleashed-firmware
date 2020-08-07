/**
  ******************************************************************************
  * @file    stm32l4xx_hal_dfsdm.c
  * @author  MCD Application Team
  * @brief   This file provides firmware functions to manage the following
  *          functionalities of the Digital Filter for Sigma-Delta Modulators
  *          (DFSDM) peripherals:
  *           + Initialization and configuration of channels and filters
  *           + Regular channels configuration
  *           + Injected channels configuration
  *           + Regular/Injected Channels DMA Configuration
  *           + Interrupts and flags management
  *           + Analog watchdog feature
  *           + Short-circuit detector feature
  *           + Extremes detector feature
  *           + Clock absence detector feature
  *           + Break generation on analog watchdog or short-circuit event
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
  [..]
    *** Channel initialization ***
    ==============================
    [..]
      (#) User has first to initialize channels (before filters initialization).
      (#) As prerequisite, fill in the HAL_DFSDM_ChannelMspInit() :
        (++) Enable DFSDMz clock interface with __HAL_RCC_DFSDMz_CLK_ENABLE().
        (++) Enable the clocks for the DFSDMz GPIOS with __HAL_RCC_GPIOx_CLK_ENABLE().
        (++) Configure these DFSDMz pins in alternate mode using HAL_GPIO_Init().
        (++) If interrupt mode is used, enable and configure DFSDMz_FLT0 global
            interrupt with HAL_NVIC_SetPriority() and HAL_NVIC_EnableIRQ().
      (#) Configure the output clock, input, serial interface, analog watchdog,
          offset and data right bit shift parameters for this channel using the
          HAL_DFSDM_ChannelInit() function.

    *** Channel clock absence detector ***
    ======================================
    [..]
      (#) Start clock absence detector using HAL_DFSDM_ChannelCkabStart() or
          HAL_DFSDM_ChannelCkabStart_IT().
      (#) In polling mode, use HAL_DFSDM_ChannelPollForCkab() to detect the clock
          absence.
      (#) In interrupt mode, HAL_DFSDM_ChannelCkabCallback() will be called if
          clock absence is detected.
      (#) Stop clock absence detector using HAL_DFSDM_ChannelCkabStop() or
          HAL_DFSDM_ChannelCkabStop_IT().
      (#) Please note that the same mode (polling or interrupt) has to be used
          for all channels because the channels are sharing the same interrupt.
      (#) Please note also that in interrupt mode, if clock absence detector is
          stopped for one channel, interrupt will be disabled for all channels.

    *** Channel short circuit detector ***
    ======================================
    [..]
      (#) Start short circuit detector using HAL_DFSDM_ChannelScdStart() or
          or HAL_DFSDM_ChannelScdStart_IT().
      (#) In polling mode, use HAL_DFSDM_ChannelPollForScd() to detect short
          circuit.
      (#) In interrupt mode, HAL_DFSDM_ChannelScdCallback() will be called if
          short circuit is detected.
      (#) Stop short circuit detector using HAL_DFSDM_ChannelScdStop() or
          or HAL_DFSDM_ChannelScdStop_IT().
      (#) Please note that the same mode (polling or interrupt) has to be used
          for all channels because the channels are sharing the same interrupt.
      (#) Please note also that in interrupt mode, if short circuit detector is
          stopped for one channel, interrupt will be disabled for all channels.

    *** Channel analog watchdog value ***
    =====================================
    [..]
      (#) Get analog watchdog filter value of a channel using
          HAL_DFSDM_ChannelGetAwdValue().

    *** Channel offset value ***
    =====================================
    [..]
      (#) Modify offset value of a channel using HAL_DFSDM_ChannelModifyOffset().

    *** Filter initialization ***
    =============================
    [..]
      (#) After channel initialization, user has to init filters.
      (#) As prerequisite, fill in the HAL_DFSDM_FilterMspInit() :
        (++) If interrupt mode is used , enable and configure DFSDMz_FLTx global
            interrupt with HAL_NVIC_SetPriority() and HAL_NVIC_EnableIRQ().
            Please note that DFSDMz_FLT0 global interrupt could be already
            enabled if interrupt is used for channel.
        (++) If DMA mode is used, configure DMA with HAL_DMA_Init() and link it
            with DFSDMz filter handle using __HAL_LINKDMA().
      (#) Configure the regular conversion, injected conversion and filter
          parameters for this filter using the HAL_DFSDM_FilterInit() function.

    *** Filter regular channel conversion ***
    =========================================
    [..]
      (#) Select regular channel and enable/disable continuous mode using
          HAL_DFSDM_FilterConfigRegChannel().
      (#) Start regular conversion using HAL_DFSDM_FilterRegularStart(),
          HAL_DFSDM_FilterRegularStart_IT(), HAL_DFSDM_FilterRegularStart_DMA() or
          HAL_DFSDM_FilterRegularMsbStart_DMA().
      (#) In polling mode, use HAL_DFSDM_FilterPollForRegConversion() to detect
          the end of regular conversion.
      (#) In interrupt mode, HAL_DFSDM_FilterRegConvCpltCallback() will be called
          at the end of regular conversion.
      (#) Get value of regular conversion and corresponding channel using
          HAL_DFSDM_FilterGetRegularValue().
      (#) In DMA mode, HAL_DFSDM_FilterRegConvHalfCpltCallback() and
          HAL_DFSDM_FilterRegConvCpltCallback() will be called respectively at the
          half transfer and at the transfer complete. Please note that
          HAL_DFSDM_FilterRegConvHalfCpltCallback() will be called only in DMA
          circular mode.
      (#) Stop regular conversion using HAL_DFSDM_FilterRegularStop(),
          HAL_DFSDM_FilterRegularStop_IT() or HAL_DFSDM_FilterRegularStop_DMA().

    *** Filter injected channels conversion ***
    ===========================================
    [..]
      (#) Select injected channels using HAL_DFSDM_FilterConfigInjChannel().
      (#) Start injected conversion using HAL_DFSDM_FilterInjectedStart(),
          HAL_DFSDM_FilterInjectedStart_IT(), HAL_DFSDM_FilterInjectedStart_DMA() or
          HAL_DFSDM_FilterInjectedMsbStart_DMA().
      (#) In polling mode, use HAL_DFSDM_FilterPollForInjConversion() to detect
          the end of injected conversion.
      (#) In interrupt mode, HAL_DFSDM_FilterInjConvCpltCallback() will be called
          at the end of injected conversion.
      (#) Get value of injected conversion and corresponding channel using
          HAL_DFSDM_FilterGetInjectedValue().
      (#) In DMA mode, HAL_DFSDM_FilterInjConvHalfCpltCallback() and
          HAL_DFSDM_FilterInjConvCpltCallback() will be called respectively at the
          half transfer and at the transfer complete. Please note that
          HAL_DFSDM_FilterInjConvCpltCallback() will be called only in DMA
          circular mode.
      (#) Stop injected conversion using HAL_DFSDM_FilterInjectedStop(),
          HAL_DFSDM_FilterInjectedStop_IT() or HAL_DFSDM_FilterInjectedStop_DMA().

    *** Filter analog watchdog ***
    ==============================
    [..]
      (#) Start filter analog watchdog using HAL_DFSDM_FilterAwdStart_IT().
      (#) HAL_DFSDM_FilterAwdCallback() will be called if analog watchdog occurs.
      (#) Stop filter analog watchdog using HAL_DFSDM_FilterAwdStop_IT().

    *** Filter extreme detector ***
    ===============================
    [..]
      (#) Start filter extreme detector using HAL_DFSDM_FilterExdStart().
      (#) Get extreme detector maximum value using HAL_DFSDM_FilterGetExdMaxValue().
      (#) Get extreme detector minimum value using HAL_DFSDM_FilterGetExdMinValue().
      (#) Start filter extreme detector using HAL_DFSDM_FilterExdStop().

    *** Filter conversion time ***
    ==============================
    [..]
      (#) Get conversion time value using HAL_DFSDM_FilterGetConvTimeValue().

    *** Callback registration ***
    =============================
    [..]
    The compilation define USE_HAL_DFSDM_REGISTER_CALLBACKS when set to 1
    allows the user to configure dynamically the driver callbacks.
    Use functions HAL_DFSDM_Channel_RegisterCallback(),
    HAL_DFSDM_Filter_RegisterCallback() or
    HAL_DFSDM_Filter_RegisterAwdCallback() to register a user callback.

    [..]
    Function HAL_DFSDM_Channel_RegisterCallback() allows to register
    following callbacks:
      (+) CkabCallback      : DFSDM channel clock absence detection callback.
      (+) ScdCallback       : DFSDM channel short circuit detection callback.
      (+) MspInitCallback   : DFSDM channel MSP init callback.
      (+) MspDeInitCallback : DFSDM channel MSP de-init callback.
    [..]
    This function takes as parameters the HAL peripheral handle, the Callback ID
    and a pointer to the user callback function.

    [..]
    Function HAL_DFSDM_Filter_RegisterCallback() allows to register
    following callbacks:
      (+) RegConvCpltCallback     : DFSDM filter regular conversion complete callback.
      (+) RegConvHalfCpltCallback : DFSDM filter half regular conversion complete callback.
      (+) InjConvCpltCallback     : DFSDM filter injected conversion complete callback.
      (+) InjConvHalfCpltCallback : DFSDM filter half injected conversion complete callback.
      (+) ErrorCallback           : DFSDM filter error callback.
      (+) MspInitCallback         : DFSDM filter MSP init callback.
      (+) MspDeInitCallback       : DFSDM filter MSP de-init callback.
    [..]
    This function takes as parameters the HAL peripheral handle, the Callback ID
    and a pointer to the user callback function.

    [..]
    For specific DFSDM filter analog watchdog callback use dedicated register callback:
    HAL_DFSDM_Filter_RegisterAwdCallback().

    [..]
    Use functions HAL_DFSDM_Channel_UnRegisterCallback() or
    HAL_DFSDM_Filter_UnRegisterCallback() to reset a callback to the default
    weak function.

    [..]
    HAL_DFSDM_Channel_UnRegisterCallback() takes as parameters the HAL peripheral handle,
    and the Callback ID.
    [..]
    This function allows to reset following callbacks:
      (+) CkabCallback      : DFSDM channel clock absence detection callback.
      (+) ScdCallback       : DFSDM channel short circuit detection callback.
      (+) MspInitCallback   : DFSDM channel MSP init callback.
      (+) MspDeInitCallback : DFSDM channel MSP de-init callback.

    [..]
    HAL_DFSDM_Filter_UnRegisterCallback() takes as parameters the HAL peripheral handle,
    and the Callback ID.
    [..]
    This function allows to reset following callbacks:
      (+) RegConvCpltCallback     : DFSDM filter regular conversion complete callback.
      (+) RegConvHalfCpltCallback : DFSDM filter half regular conversion complete callback.
      (+) InjConvCpltCallback     : DFSDM filter injected conversion complete callback.
      (+) InjConvHalfCpltCallback : DFSDM filter half injected conversion complete callback.
      (+) ErrorCallback           : DFSDM filter error callback.
      (+) MspInitCallback         : DFSDM filter MSP init callback.
      (+) MspDeInitCallback       : DFSDM filter MSP de-init callback.

    [..]
    For specific DFSDM filter analog watchdog callback use dedicated unregister callback:
    HAL_DFSDM_Filter_UnRegisterAwdCallback().

    [..]
    By default, after the call of init function and if the state is RESET
    all callbacks are reset to the corresponding legacy weak functions:
    examples HAL_DFSDM_ChannelScdCallback(), HAL_DFSDM_FilterErrorCallback().
    Exception done for MspInit and MspDeInit callbacks that are respectively
    reset to the legacy weak functions in the init and de-init only when these
    callbacks are null (not registered beforehand).
    If not, MspInit or MspDeInit are not null, the init and de-init keep and use
    the user MspInit/MspDeInit callbacks (registered beforehand)

    [..]
    Callbacks can be registered/unregistered in READY state only.
    Exception done for MspInit/MspDeInit callbacks that can be registered/unregistered
    in READY or RESET state, thus registered (user) MspInit/DeInit callbacks can be used
    during the init/de-init.
    In that case first register the MspInit/MspDeInit user callbacks using
    HAL_DFSDM_Channel_RegisterCallback() or
    HAL_DFSDM_Filter_RegisterCallback() before calling init or de-init function.

    [..]
    When The compilation define USE_HAL_DFSDM_REGISTER_CALLBACKS is set to 0 or
    not defined, the callback registering feature is not available
    and weak callbacks are used.

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
#ifdef HAL_DFSDM_MODULE_ENABLED

#if defined(STM32L451xx) || defined(STM32L452xx) || defined(STM32L462xx) || \
    defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || \
    defined(STM32L496xx) || defined(STM32L4A6xx) || \
    defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)

/** @defgroup DFSDM DFSDM
  * @brief DFSDM HAL driver module
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup DFSDM_Private_Define DFSDM Private Define
 * @{
 */
#define DFSDM_FLTCR1_MSB_RCH_OFFSET     8
#define DFSDM_MSB_MASK                  0xFFFF0000U
#define DFSDM_LSB_MASK                  0x0000FFFFU
#define DFSDM_CKAB_TIMEOUT              5000U
#if defined(STM32L451xx) || defined(STM32L452xx) || defined(STM32L462xx)
#define DFSDM1_CHANNEL_NUMBER           4U
#else /* STM32L451xx || STM32L452xx || STM32L462xx */
#define DFSDM1_CHANNEL_NUMBER           8U
#endif /* STM32L451xx || STM32L452xx || STM32L462xx */
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup DFSDM_Private_Variables DFSDM Private Variables
  * @{
  */
static __IO uint32_t                v_dfsdm1ChannelCounter = 0;
static DFSDM_Channel_HandleTypeDef *a_dfsdm1ChannelHandle[DFSDM1_CHANNEL_NUMBER] = {NULL};
/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup DFSDM_Private_Functions DFSDM Private Functions
  * @{
  */
static uint32_t DFSDM_GetInjChannelsNbr(uint32_t Channels);
static uint32_t DFSDM_GetChannelFromInstance(const DFSDM_Channel_TypeDef *Instance);
static void     DFSDM_RegConvStart(DFSDM_Filter_HandleTypeDef *hdfsdm_filter);
static void     DFSDM_RegConvStop(DFSDM_Filter_HandleTypeDef *hdfsdm_filter);
static void     DFSDM_InjConvStart(DFSDM_Filter_HandleTypeDef *hdfsdm_filter);
static void     DFSDM_InjConvStop(DFSDM_Filter_HandleTypeDef *hdfsdm_filter);
static void     DFSDM_DMARegularHalfConvCplt(DMA_HandleTypeDef *hdma);
static void     DFSDM_DMARegularConvCplt(DMA_HandleTypeDef *hdma);
static void     DFSDM_DMAInjectedHalfConvCplt(DMA_HandleTypeDef *hdma);
static void     DFSDM_DMAInjectedConvCplt(DMA_HandleTypeDef *hdma);
static void     DFSDM_DMAError(DMA_HandleTypeDef *hdma);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup DFSDM_Exported_Functions DFSDM Exported Functions
  * @{
  */

/** @defgroup DFSDM_Exported_Functions_Group1_Channel Channel initialization and de-initialization functions
 *  @brief    Channel initialization and de-initialization functions
 *
@verbatim
  ==============================================================================
        ##### Channel initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the DFSDM channel.
      (+) De-initialize the DFSDM channel.
@endverbatim
  * @{
  */

/**
  * @brief  Initialize the DFSDM channel according to the specified parameters
  *         in the DFSDM_ChannelInitTypeDef structure and initialize the associated handle.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelInit(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  /* Check DFSDM Channel handle */
  if (hdfsdm_channel == NULL)
  {
    return HAL_ERROR;
  }

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));
  assert_param(IS_FUNCTIONAL_STATE(hdfsdm_channel->Init.OutputClock.Activation));
  assert_param(IS_DFSDM_CHANNEL_INPUT(hdfsdm_channel->Init.Input.Multiplexer));
  assert_param(IS_DFSDM_CHANNEL_DATA_PACKING(hdfsdm_channel->Init.Input.DataPacking));
  assert_param(IS_DFSDM_CHANNEL_INPUT_PINS(hdfsdm_channel->Init.Input.Pins));
  assert_param(IS_DFSDM_CHANNEL_SERIAL_INTERFACE_TYPE(hdfsdm_channel->Init.SerialInterface.Type));
  assert_param(IS_DFSDM_CHANNEL_SPI_CLOCK(hdfsdm_channel->Init.SerialInterface.SpiClock));
  assert_param(IS_DFSDM_CHANNEL_FILTER_ORDER(hdfsdm_channel->Init.Awd.FilterOrder));
  assert_param(IS_DFSDM_CHANNEL_FILTER_OVS_RATIO(hdfsdm_channel->Init.Awd.Oversampling));
  assert_param(IS_DFSDM_CHANNEL_OFFSET(hdfsdm_channel->Init.Offset));
  assert_param(IS_DFSDM_CHANNEL_RIGHT_BIT_SHIFT(hdfsdm_channel->Init.RightBitShift));

  /* Check that channel has not been already initialized */
  if (a_dfsdm1ChannelHandle[DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance)] != NULL)
  {
    return HAL_ERROR;
  }

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  /* Reset callback pointers to the weak predefined callbacks */
  hdfsdm_channel->CkabCallback = HAL_DFSDM_ChannelCkabCallback;
  hdfsdm_channel->ScdCallback  = HAL_DFSDM_ChannelScdCallback;

  /* Call MSP init function */
  if (hdfsdm_channel->MspInitCallback == NULL)
  {
    hdfsdm_channel->MspInitCallback = HAL_DFSDM_ChannelMspInit;
  }
  hdfsdm_channel->MspInitCallback(hdfsdm_channel);
#else
  /* Call MSP init function */
  HAL_DFSDM_ChannelMspInit(hdfsdm_channel);
#endif

  /* Update the channel counter */
  v_dfsdm1ChannelCounter++;

  /* Configure output serial clock and enable global DFSDM interface only for first channel */
  if (v_dfsdm1ChannelCounter == 1U)
  {
    assert_param(IS_DFSDM_CHANNEL_OUTPUT_CLOCK(hdfsdm_channel->Init.OutputClock.Selection));
    /* Set the output serial clock source */
    DFSDM1_Channel0->CHCFGR1 &= ~(DFSDM_CHCFGR1_CKOUTSRC);
    DFSDM1_Channel0->CHCFGR1 |= hdfsdm_channel->Init.OutputClock.Selection;

    /* Reset clock divider */
    DFSDM1_Channel0->CHCFGR1 &= ~(DFSDM_CHCFGR1_CKOUTDIV);
    if (hdfsdm_channel->Init.OutputClock.Activation == ENABLE)
    {
      assert_param(IS_DFSDM_CHANNEL_OUTPUT_CLOCK_DIVIDER(hdfsdm_channel->Init.OutputClock.Divider));
      /* Set the output clock divider */
      DFSDM1_Channel0->CHCFGR1 |= (uint32_t)((hdfsdm_channel->Init.OutputClock.Divider - 1U) <<
                                             DFSDM_CHCFGR1_CKOUTDIV_Pos);
    }

    /* enable the DFSDM global interface */
    DFSDM1_Channel0->CHCFGR1 |= DFSDM_CHCFGR1_DFSDMEN;
  }

  /* Set channel input parameters */
  hdfsdm_channel->Instance->CHCFGR1 &= ~(DFSDM_CHCFGR1_DATPACK | DFSDM_CHCFGR1_DATMPX |
                                         DFSDM_CHCFGR1_CHINSEL);
  hdfsdm_channel->Instance->CHCFGR1 |= (hdfsdm_channel->Init.Input.Multiplexer |
                                        hdfsdm_channel->Init.Input.DataPacking |
                                        hdfsdm_channel->Init.Input.Pins);

  /* Set serial interface parameters */
  hdfsdm_channel->Instance->CHCFGR1 &= ~(DFSDM_CHCFGR1_SITP | DFSDM_CHCFGR1_SPICKSEL);
  hdfsdm_channel->Instance->CHCFGR1 |= (hdfsdm_channel->Init.SerialInterface.Type |
                                        hdfsdm_channel->Init.SerialInterface.SpiClock);

  /* Set analog watchdog parameters */
  hdfsdm_channel->Instance->CHAWSCDR &= ~(DFSDM_CHAWSCDR_AWFORD | DFSDM_CHAWSCDR_AWFOSR);
  hdfsdm_channel->Instance->CHAWSCDR |= (hdfsdm_channel->Init.Awd.FilterOrder |
                                         ((hdfsdm_channel->Init.Awd.Oversampling - 1U) << DFSDM_CHAWSCDR_AWFOSR_Pos));

  /* Set channel offset and right bit shift */
  hdfsdm_channel->Instance->CHCFGR2 &= ~(DFSDM_CHCFGR2_OFFSET | DFSDM_CHCFGR2_DTRBS);
  hdfsdm_channel->Instance->CHCFGR2 |= (((uint32_t) hdfsdm_channel->Init.Offset << DFSDM_CHCFGR2_OFFSET_Pos) |
                                        (hdfsdm_channel->Init.RightBitShift << DFSDM_CHCFGR2_DTRBS_Pos));

  /* Enable DFSDM channel */
  hdfsdm_channel->Instance->CHCFGR1 |= DFSDM_CHCFGR1_CHEN;

  /* Set DFSDM Channel to ready state */
  hdfsdm_channel->State = HAL_DFSDM_CHANNEL_STATE_READY;

  /* Store channel handle in DFSDM channel handle table */
  a_dfsdm1ChannelHandle[DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance)] = hdfsdm_channel;

  return HAL_OK;
}

/**
  * @brief  De-initialize the DFSDM channel.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelDeInit(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  /* Check DFSDM Channel handle */
  if (hdfsdm_channel == NULL)
  {
    return HAL_ERROR;
  }

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check that channel has not been already deinitialized */
  if (a_dfsdm1ChannelHandle[DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance)] == NULL)
  {
    return HAL_ERROR;
  }

  /* Disable the DFSDM channel */
  hdfsdm_channel->Instance->CHCFGR1 &= ~(DFSDM_CHCFGR1_CHEN);

  /* Update the channel counter */
  v_dfsdm1ChannelCounter--;

  /* Disable global DFSDM at deinit of last channel */
  if (v_dfsdm1ChannelCounter == 0U)
  {
    DFSDM1_Channel0->CHCFGR1 &= ~(DFSDM_CHCFGR1_DFSDMEN);
  }

  /* Call MSP deinit function */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  if (hdfsdm_channel->MspDeInitCallback == NULL)
  {
    hdfsdm_channel->MspDeInitCallback = HAL_DFSDM_ChannelMspDeInit;
  }
  hdfsdm_channel->MspDeInitCallback(hdfsdm_channel);
#else
  HAL_DFSDM_ChannelMspDeInit(hdfsdm_channel);
#endif

  /* Set DFSDM Channel in reset state */
  hdfsdm_channel->State = HAL_DFSDM_CHANNEL_STATE_RESET;

  /* Reset channel handle in DFSDM channel handle table */
  a_dfsdm1ChannelHandle[DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance)] = (DFSDM_Channel_HandleTypeDef *) NULL;

  return HAL_OK;
}

/**
  * @brief  Initialize the DFSDM channel MSP.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval None
  */
__weak void HAL_DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_channel);

  /* NOTE : This function should not be modified, when the function is needed,
            the HAL_DFSDM_ChannelMspInit could be implemented in the user file.
   */
}

/**
  * @brief  De-initialize the DFSDM channel MSP.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval None
  */
__weak void HAL_DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_channel);

  /* NOTE : This function should not be modified, when the function is needed,
            the HAL_DFSDM_ChannelMspDeInit could be implemented in the user file.
   */
}

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a user DFSDM channel callback
  *         to be used instead of the weak predefined callback.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @param  CallbackID ID of the callback to be registered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_DFSDM_CHANNEL_CKAB_CB_ID clock absence detection callback ID.
  *           @arg @ref HAL_DFSDM_CHANNEL_SCD_CB_ID short circuit detection callback ID.
  *           @arg @ref HAL_DFSDM_CHANNEL_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @param  pCallback pointer to the callback function.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_Channel_RegisterCallback(DFSDM_Channel_HandleTypeDef        *hdfsdm_channel,
                                                     HAL_DFSDM_Channel_CallbackIDTypeDef CallbackID,
                                                     pDFSDM_Channel_CallbackTypeDef      pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* update return status */
    status = HAL_ERROR;
  }
  else
  {
    if (HAL_DFSDM_CHANNEL_STATE_READY == hdfsdm_channel->State)
    {
      switch (CallbackID)
      {
        case HAL_DFSDM_CHANNEL_CKAB_CB_ID :
          hdfsdm_channel->CkabCallback = pCallback;
          break;
        case HAL_DFSDM_CHANNEL_SCD_CB_ID :
          hdfsdm_channel->ScdCallback = pCallback;
          break;
        case HAL_DFSDM_CHANNEL_MSPINIT_CB_ID :
          hdfsdm_channel->MspInitCallback = pCallback;
          break;
        case HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID :
          hdfsdm_channel->MspDeInitCallback = pCallback;
          break;
        default :
          /* update return status */
          status = HAL_ERROR;
          break;
      }
    }
    else if (HAL_DFSDM_CHANNEL_STATE_RESET == hdfsdm_channel->State)
    {
      switch (CallbackID)
      {
        case HAL_DFSDM_CHANNEL_MSPINIT_CB_ID :
          hdfsdm_channel->MspInitCallback = pCallback;
          break;
        case HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID :
          hdfsdm_channel->MspDeInitCallback = pCallback;
          break;
        default :
          /* update return status */
          status = HAL_ERROR;
          break;
      }
    }
    else
    {
      /* update return status */
      status = HAL_ERROR;
    }
  }
  return status;
}

/**
  * @brief  Unregister a user DFSDM channel callback.
  *         DFSDM channel callback is redirected to the weak predefined callback.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @param  CallbackID ID of the callback to be unregistered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_DFSDM_CHANNEL_CKAB_CB_ID clock absence detection callback ID.
  *           @arg @ref HAL_DFSDM_CHANNEL_SCD_CB_ID short circuit detection callback ID.
  *           @arg @ref HAL_DFSDM_CHANNEL_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_Channel_UnRegisterCallback(DFSDM_Channel_HandleTypeDef        *hdfsdm_channel,
                                                       HAL_DFSDM_Channel_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (HAL_DFSDM_CHANNEL_STATE_READY == hdfsdm_channel->State)
  {
    switch (CallbackID)
    {
      case HAL_DFSDM_CHANNEL_CKAB_CB_ID :
        hdfsdm_channel->CkabCallback = HAL_DFSDM_ChannelCkabCallback;
        break;
      case HAL_DFSDM_CHANNEL_SCD_CB_ID :
        hdfsdm_channel->ScdCallback = HAL_DFSDM_ChannelScdCallback;
        break;
      case HAL_DFSDM_CHANNEL_MSPINIT_CB_ID :
        hdfsdm_channel->MspInitCallback = HAL_DFSDM_ChannelMspInit;
        break;
      case HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID :
        hdfsdm_channel->MspDeInitCallback = HAL_DFSDM_ChannelMspDeInit;
        break;
      default :
        /* update return status */
        status = HAL_ERROR;
        break;
    }
  }
  else if (HAL_DFSDM_CHANNEL_STATE_RESET == hdfsdm_channel->State)
  {
    switch (CallbackID)
    {
      case HAL_DFSDM_CHANNEL_MSPINIT_CB_ID :
        hdfsdm_channel->MspInitCallback = HAL_DFSDM_ChannelMspInit;
        break;
      case HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID :
        hdfsdm_channel->MspDeInitCallback = HAL_DFSDM_ChannelMspDeInit;
        break;
      default :
        /* update return status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* update return status */
    status = HAL_ERROR;
  }
  return status;
}
#endif /* USE_HAL_DFSDM_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup DFSDM_Exported_Functions_Group2_Channel Channel operation functions
 *  @brief    Channel operation functions
 *
@verbatim
  ==============================================================================
                   ##### Channel operation functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Manage clock absence detector feature.
      (+) Manage short circuit detector feature.
      (+) Get analog watchdog value.
      (+) Modify offset value.
@endverbatim
  * @{
  */

/**
  * @brief  This function allows to start clock absence detection in polling mode.
  * @note   Same mode has to be used for all channels.
  * @note   If clock is not available on this channel during 5 seconds,
  *         clock absence detection will not be activated and function
  *         will return HAL_TIMEOUT error.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelCkabStart(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t channel;
  uint32_t tickstart;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Get channel number from channel instance */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);

    /* Get timeout */
    tickstart = HAL_GetTick();

    /* Clear clock absence flag */
    while ((((DFSDM1_Filter0->FLTISR & DFSDM_FLTISR_CKABF) >> (DFSDM_FLTISR_CKABF_Pos + channel)) & 1U) != 0U)
    {
      DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRCKABF_Pos + channel));

      /* Check the Timeout */
      if ((HAL_GetTick() - tickstart) > DFSDM_CKAB_TIMEOUT)
      {
        /* Set timeout status */
        status = HAL_TIMEOUT;
        break;
      }
    }

    if (status == HAL_OK)
    {
      /* Start clock absence detection */
      hdfsdm_channel->Instance->CHCFGR1 |= DFSDM_CHCFGR1_CKABEN;
    }
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to poll for the clock absence detection.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @param  Timeout Timeout value in milliseconds.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelPollForCkab(DFSDM_Channel_HandleTypeDef *hdfsdm_channel,
                                               uint32_t Timeout)
{
  uint32_t tickstart;
  uint32_t channel;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    return HAL_ERROR;
  }
  else
  {
    /* Get channel number from channel instance */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);

    /* Get timeout */
    tickstart = HAL_GetTick();

    /* Wait clock absence detection */
    while ((((DFSDM1_Filter0->FLTISR & DFSDM_FLTISR_CKABF) >> (DFSDM_FLTISR_CKABF_Pos + channel)) & 1U) == 0U)
    {
      /* Check the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U))
        {
          /* Return timeout status */
          return HAL_TIMEOUT;
        }
      }
    }

    /* Clear clock absence detection flag */
    DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRCKABF_Pos + channel));

    /* Return function status */
    return HAL_OK;
  }
}

/**
  * @brief  This function allows to stop clock absence detection in polling mode.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelCkabStop(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t channel;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop clock absence detection */
    hdfsdm_channel->Instance->CHCFGR1 &= ~(DFSDM_CHCFGR1_CKABEN);

    /* Clear clock absence flag */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);
    DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRCKABF_Pos + channel));
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start clock absence detection in interrupt mode.
  * @note   Same mode has to be used for all channels.
  * @note   If clock is not available on this channel during 5 seconds,
  *         clock absence detection will not be activated and function
  *         will return HAL_TIMEOUT error.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelCkabStart_IT(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t channel;
  uint32_t tickstart;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Get channel number from channel instance */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);

    /* Get timeout */
    tickstart = HAL_GetTick();

    /* Clear clock absence flag */
    while ((((DFSDM1_Filter0->FLTISR & DFSDM_FLTISR_CKABF) >> (DFSDM_FLTISR_CKABF_Pos + channel)) & 1U) != 0U)
    {
      DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRCKABF_Pos + channel));

      /* Check the Timeout */
      if ((HAL_GetTick() - tickstart) > DFSDM_CKAB_TIMEOUT)
      {
        /* Set timeout status */
        status = HAL_TIMEOUT;
        break;
      }
    }

    if (status == HAL_OK)
    {
      /* Activate clock absence detection interrupt */
      DFSDM1_Filter0->FLTCR2 |= DFSDM_FLTCR2_CKABIE;

      /* Start clock absence detection */
      hdfsdm_channel->Instance->CHCFGR1 |= DFSDM_CHCFGR1_CKABEN;
    }
  }
  /* Return function status */
  return status;
}

/**
  * @brief  Clock absence detection callback.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval None
  */
__weak void HAL_DFSDM_ChannelCkabCallback(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_channel);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_ChannelCkabCallback could be implemented in the user file
   */
}

/**
  * @brief  This function allows to stop clock absence detection in interrupt mode.
  * @note   Interrupt will be disabled for all channels
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelCkabStop_IT(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t channel;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop clock absence detection */
    hdfsdm_channel->Instance->CHCFGR1 &= ~(DFSDM_CHCFGR1_CKABEN);

    /* Clear clock absence flag */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);
    DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRCKABF_Pos + channel));

    /* Disable clock absence detection interrupt */
    DFSDM1_Filter0->FLTCR2 &= ~(DFSDM_FLTCR2_CKABIE);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start short circuit detection in polling mode.
  * @note   Same mode has to be used for all channels
  * @param  hdfsdm_channel DFSDM channel handle.
  * @param  Threshold Short circuit detector threshold.
  *         This parameter must be a number between Min_Data = 0 and Max_Data = 255.
  * @param  BreakSignal Break signals assigned to short circuit event.
  *         This parameter can be a values combination of @ref DFSDM_BreakSignals.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelScdStart(DFSDM_Channel_HandleTypeDef *hdfsdm_channel,
                                            uint32_t Threshold,
                                            uint32_t BreakSignal)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));
  assert_param(IS_DFSDM_CHANNEL_SCD_THRESHOLD(Threshold));
  assert_param(IS_DFSDM_BREAK_SIGNALS(BreakSignal));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Configure threshold and break signals */
    hdfsdm_channel->Instance->CHAWSCDR &= ~(DFSDM_CHAWSCDR_BKSCD | DFSDM_CHAWSCDR_SCDT);
    hdfsdm_channel->Instance->CHAWSCDR |= ((BreakSignal << DFSDM_CHAWSCDR_BKSCD_Pos) | \
                                           Threshold);

    /* Start short circuit detection */
    hdfsdm_channel->Instance->CHCFGR1 |= DFSDM_CHCFGR1_SCDEN;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to poll for the short circuit detection.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @param  Timeout Timeout value in milliseconds.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelPollForScd(DFSDM_Channel_HandleTypeDef *hdfsdm_channel,
                                              uint32_t Timeout)
{
  uint32_t tickstart;
  uint32_t channel;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    return HAL_ERROR;
  }
  else
  {
    /* Get channel number from channel instance */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);

    /* Get timeout */
    tickstart = HAL_GetTick();

    /* Wait short circuit detection */
    while (((DFSDM1_Filter0->FLTISR & DFSDM_FLTISR_SCDF) >> (DFSDM_FLTISR_SCDF_Pos + channel)) == 0U)
    {
      /* Check the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U))
        {
          /* Return timeout status */
          return HAL_TIMEOUT;
        }
      }
    }

    /* Clear short circuit detection flag */
    DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRSCDF_Pos + channel));

    /* Return function status */
    return HAL_OK;
  }
}

/**
  * @brief  This function allows to stop short circuit detection in polling mode.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelScdStop(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t channel;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop short circuit detection */
    hdfsdm_channel->Instance->CHCFGR1 &= ~(DFSDM_CHCFGR1_SCDEN);

    /* Clear short circuit detection flag */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);
    DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRSCDF_Pos + channel));
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start short circuit detection in interrupt mode.
  * @note   Same mode has to be used for all channels
  * @param  hdfsdm_channel DFSDM channel handle.
  * @param  Threshold Short circuit detector threshold.
  *         This parameter must be a number between Min_Data = 0 and Max_Data = 255.
  * @param  BreakSignal Break signals assigned to short circuit event.
  *         This parameter can be a values combination of @ref DFSDM_BreakSignals.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelScdStart_IT(DFSDM_Channel_HandleTypeDef *hdfsdm_channel,
                                               uint32_t Threshold,
                                               uint32_t BreakSignal)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));
  assert_param(IS_DFSDM_CHANNEL_SCD_THRESHOLD(Threshold));
  assert_param(IS_DFSDM_BREAK_SIGNALS(BreakSignal));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Activate short circuit detection interrupt */
    DFSDM1_Filter0->FLTCR2 |= DFSDM_FLTCR2_SCDIE;

    /* Configure threshold and break signals */
    hdfsdm_channel->Instance->CHAWSCDR &= ~(DFSDM_CHAWSCDR_BKSCD | DFSDM_CHAWSCDR_SCDT);
    hdfsdm_channel->Instance->CHAWSCDR |= ((BreakSignal << DFSDM_CHAWSCDR_BKSCD_Pos) | \
                                           Threshold);

    /* Start short circuit detection */
    hdfsdm_channel->Instance->CHCFGR1 |= DFSDM_CHCFGR1_SCDEN;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  Short circuit detection callback.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval None
  */
__weak void HAL_DFSDM_ChannelScdCallback(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_channel);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_ChannelScdCallback could be implemented in the user file
   */
}

/**
  * @brief  This function allows to stop short circuit detection in interrupt mode.
  * @note   Interrupt will be disabled for all channels
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelScdStop_IT(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t channel;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop short circuit detection */
    hdfsdm_channel->Instance->CHCFGR1 &= ~(DFSDM_CHCFGR1_SCDEN);

    /* Clear short circuit detection flag */
    channel = DFSDM_GetChannelFromInstance(hdfsdm_channel->Instance);
    DFSDM1_Filter0->FLTICR = (1UL << (DFSDM_FLTICR_CLRSCDF_Pos + channel));

    /* Disable short circuit detection interrupt */
    DFSDM1_Filter0->FLTCR2 &= ~(DFSDM_FLTCR2_SCDIE);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to get channel analog watchdog value.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval Channel analog watchdog value.
  */
int16_t HAL_DFSDM_ChannelGetAwdValue(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  return (int16_t) hdfsdm_channel->Instance->CHWDATAR;
}

/**
  * @brief  This function allows to modify channel offset value.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @param  Offset DFSDM channel offset.
  *         This parameter must be a number between Min_Data = -8388608 and Max_Data = 8388607.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_ChannelModifyOffset(DFSDM_Channel_HandleTypeDef *hdfsdm_channel,
                                                int32_t Offset)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_CHANNEL_ALL_INSTANCE(hdfsdm_channel->Instance));
  assert_param(IS_DFSDM_CHANNEL_OFFSET(Offset));

  /* Check DFSDM channel state */
  if (hdfsdm_channel->State != HAL_DFSDM_CHANNEL_STATE_READY)
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Modify channel offset */
    hdfsdm_channel->Instance->CHCFGR2 &= ~(DFSDM_CHCFGR2_OFFSET);
    hdfsdm_channel->Instance->CHCFGR2 |= ((uint32_t) Offset << DFSDM_CHCFGR2_OFFSET_Pos);
  }
  /* Return function status */
  return status;
}

/**
  * @}
  */

/** @defgroup DFSDM_Exported_Functions_Group3_Channel Channel state function
 *  @brief    Channel state function
 *
@verbatim
  ==============================================================================
                   ##### Channel state function #####
  ==============================================================================
    [..]  This section provides function allowing to:
      (+) Get channel handle state.
@endverbatim
  * @{
  */

/**
  * @brief  This function allows to get the current DFSDM channel handle state.
  * @param  hdfsdm_channel DFSDM channel handle.
  * @retval DFSDM channel state.
  */
HAL_DFSDM_Channel_StateTypeDef HAL_DFSDM_ChannelGetState(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
  /* Return DFSDM channel handle state */
  return hdfsdm_channel->State;
}

/**
  * @}
  */

/** @defgroup DFSDM_Exported_Functions_Group1_Filter Filter initialization and de-initialization functions
 *  @brief    Filter initialization and de-initialization functions
 *
@verbatim
  ==============================================================================
        ##### Filter initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the DFSDM filter.
      (+) De-initialize the DFSDM filter.
@endverbatim
  * @{
  */

/**
  * @brief  Initialize the DFSDM filter according to the specified parameters
  *         in the DFSDM_FilterInitTypeDef structure and initialize the associated handle.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInit(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Check DFSDM Channel handle */
  if (hdfsdm_filter == NULL)
  {
    return HAL_ERROR;
  }

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(IS_DFSDM_FILTER_REG_TRIGGER(hdfsdm_filter->Init.RegularParam.Trigger));
  assert_param(IS_FUNCTIONAL_STATE(hdfsdm_filter->Init.RegularParam.FastMode));
  assert_param(IS_FUNCTIONAL_STATE(hdfsdm_filter->Init.RegularParam.DmaMode));
  assert_param(IS_DFSDM_FILTER_INJ_TRIGGER(hdfsdm_filter->Init.InjectedParam.Trigger));
  assert_param(IS_FUNCTIONAL_STATE(hdfsdm_filter->Init.InjectedParam.ScanMode));
  assert_param(IS_FUNCTIONAL_STATE(hdfsdm_filter->Init.InjectedParam.DmaMode));
  assert_param(IS_DFSDM_FILTER_SINC_ORDER(hdfsdm_filter->Init.FilterParam.SincOrder));
  assert_param(IS_DFSDM_FILTER_OVS_RATIO(hdfsdm_filter->Init.FilterParam.Oversampling));
  assert_param(IS_DFSDM_FILTER_INTEGRATOR_OVS_RATIO(hdfsdm_filter->Init.FilterParam.IntOversampling));

  /* Check parameters compatibility */
  if ((hdfsdm_filter->Instance == DFSDM1_Filter0) &&
      ((hdfsdm_filter->Init.RegularParam.Trigger  == DFSDM_FILTER_SYNC_TRIGGER) ||
       (hdfsdm_filter->Init.InjectedParam.Trigger == DFSDM_FILTER_SYNC_TRIGGER)))
  {
    return HAL_ERROR;
  }

  /* Initialize DFSDM filter variables with default values */
  hdfsdm_filter->RegularContMode     = DFSDM_CONTINUOUS_CONV_OFF;
  hdfsdm_filter->InjectedChannelsNbr = 1;
  hdfsdm_filter->InjConvRemaining    = 1;
  hdfsdm_filter->ErrorCode           = DFSDM_FILTER_ERROR_NONE;

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  /* Reset callback pointers to the weak predefined callbacks */
  hdfsdm_filter->AwdCallback             = HAL_DFSDM_FilterAwdCallback;
  hdfsdm_filter->RegConvCpltCallback     = HAL_DFSDM_FilterRegConvCpltCallback;
  hdfsdm_filter->RegConvHalfCpltCallback = HAL_DFSDM_FilterRegConvHalfCpltCallback;
  hdfsdm_filter->InjConvCpltCallback     = HAL_DFSDM_FilterInjConvCpltCallback;
  hdfsdm_filter->InjConvHalfCpltCallback = HAL_DFSDM_FilterInjConvHalfCpltCallback;
  hdfsdm_filter->ErrorCallback           = HAL_DFSDM_FilterErrorCallback;

  /* Call MSP init function */
  if (hdfsdm_filter->MspInitCallback == NULL)
  {
    hdfsdm_filter->MspInitCallback = HAL_DFSDM_FilterMspInit;
  }
  hdfsdm_filter->MspInitCallback(hdfsdm_filter);
#else
  /* Call MSP init function */
  HAL_DFSDM_FilterMspInit(hdfsdm_filter);
#endif

  /* Set regular parameters */
  hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_RSYNC);
  if (hdfsdm_filter->Init.RegularParam.FastMode == ENABLE)
  {
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_FAST;
  }
  else
  {
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_FAST);
  }

  if (hdfsdm_filter->Init.RegularParam.DmaMode == ENABLE)
  {
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_RDMAEN;
  }
  else
  {
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_RDMAEN);
  }

  /* Set injected parameters */
  hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_JSYNC | DFSDM_FLTCR1_JEXTEN | DFSDM_FLTCR1_JEXTSEL);
  if (hdfsdm_filter->Init.InjectedParam.Trigger == DFSDM_FILTER_EXT_TRIGGER)
  {
    assert_param(IS_DFSDM_FILTER_EXT_TRIG(hdfsdm_filter->Init.InjectedParam.ExtTrigger));
    assert_param(IS_DFSDM_FILTER_EXT_TRIG_EDGE(hdfsdm_filter->Init.InjectedParam.ExtTriggerEdge));
    hdfsdm_filter->Instance->FLTCR1 |= (hdfsdm_filter->Init.InjectedParam.ExtTrigger);
  }

  if (hdfsdm_filter->Init.InjectedParam.ScanMode == ENABLE)
  {
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_JSCAN;
  }
  else
  {
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_JSCAN);
  }

  if (hdfsdm_filter->Init.InjectedParam.DmaMode == ENABLE)
  {
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_JDMAEN;
  }
  else
  {
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_JDMAEN);
  }

  /* Set filter parameters */
  hdfsdm_filter->Instance->FLTFCR &= ~(DFSDM_FLTFCR_FORD | DFSDM_FLTFCR_FOSR | DFSDM_FLTFCR_IOSR);
  hdfsdm_filter->Instance->FLTFCR |= (hdfsdm_filter->Init.FilterParam.SincOrder |
                                      ((hdfsdm_filter->Init.FilterParam.Oversampling - 1U) << DFSDM_FLTFCR_FOSR_Pos) |
                                      (hdfsdm_filter->Init.FilterParam.IntOversampling - 1U));

  /* Store regular and injected triggers and injected scan mode*/
  hdfsdm_filter->RegularTrigger   = hdfsdm_filter->Init.RegularParam.Trigger;
  hdfsdm_filter->InjectedTrigger  = hdfsdm_filter->Init.InjectedParam.Trigger;
  hdfsdm_filter->ExtTriggerEdge   = hdfsdm_filter->Init.InjectedParam.ExtTriggerEdge;
  hdfsdm_filter->InjectedScanMode = hdfsdm_filter->Init.InjectedParam.ScanMode;

  /* Enable DFSDM filter */
  hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_DFEN;

  /* Set DFSDM filter to ready state */
  hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  De-initializes the DFSDM filter.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_FilterDeInit(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Check DFSDM filter handle */
  if (hdfsdm_filter == NULL)
  {
    return HAL_ERROR;
  }

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Disable the DFSDM filter */
  hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_DFEN);

  /* Call MSP deinit function */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  if (hdfsdm_filter->MspDeInitCallback == NULL)
  {
    hdfsdm_filter->MspDeInitCallback = HAL_DFSDM_FilterMspDeInit;
  }
  hdfsdm_filter->MspDeInitCallback(hdfsdm_filter);
#else
  HAL_DFSDM_FilterMspDeInit(hdfsdm_filter);
#endif

  /* Set DFSDM filter in reset state */
  hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_RESET;

  return HAL_OK;
}

/**
  * @brief  Initializes the DFSDM filter MSP.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
__weak void HAL_DFSDM_FilterMspInit(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);

  /* NOTE : This function should not be modified, when the function is needed,
            the HAL_DFSDM_FilterMspInit could be implemented in the user file.
   */
}

/**
  * @brief  De-initializes the DFSDM filter MSP.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
__weak void HAL_DFSDM_FilterMspDeInit(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);

  /* NOTE : This function should not be modified, when the function is needed,
            the HAL_DFSDM_FilterMspDeInit could be implemented in the user file.
   */
}

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a user DFSDM filter callback
  *         to be used instead of the weak predefined callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  CallbackID ID of the callback to be registered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_DFSDM_FILTER_REGCONV_COMPLETE_CB_ID regular conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_REGCONV_HALFCOMPLETE_CB_ID half regular conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_INJCONV_COMPLETE_CB_ID injected conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_INJCONV_HALFCOMPLETE_CB_ID half injected conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_ERROR_CB_ID error callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @param  pCallback pointer to the callback function.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_Filter_RegisterCallback(DFSDM_Filter_HandleTypeDef        *hdfsdm_filter,
                                                    HAL_DFSDM_Filter_CallbackIDTypeDef CallbackID,
                                                    pDFSDM_Filter_CallbackTypeDef      pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* update the error code */
    hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  else
  {
    if (HAL_DFSDM_FILTER_STATE_READY == hdfsdm_filter->State)
    {
      switch (CallbackID)
      {
        case HAL_DFSDM_FILTER_REGCONV_COMPLETE_CB_ID :
          hdfsdm_filter->RegConvCpltCallback = pCallback;
          break;
        case HAL_DFSDM_FILTER_REGCONV_HALFCOMPLETE_CB_ID :
          hdfsdm_filter->RegConvHalfCpltCallback = pCallback;
          break;
        case HAL_DFSDM_FILTER_INJCONV_COMPLETE_CB_ID :
          hdfsdm_filter->InjConvCpltCallback = pCallback;
          break;
        case HAL_DFSDM_FILTER_INJCONV_HALFCOMPLETE_CB_ID :
          hdfsdm_filter->InjConvHalfCpltCallback = pCallback;
          break;
        case HAL_DFSDM_FILTER_ERROR_CB_ID :
          hdfsdm_filter->ErrorCallback = pCallback;
          break;
        case HAL_DFSDM_FILTER_MSPINIT_CB_ID :
          hdfsdm_filter->MspInitCallback = pCallback;
          break;
        case HAL_DFSDM_FILTER_MSPDEINIT_CB_ID :
          hdfsdm_filter->MspDeInitCallback = pCallback;
          break;
        default :
          /* update the error code */
          hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
          /* update return status */
          status = HAL_ERROR;
          break;
      }
    }
    else if (HAL_DFSDM_FILTER_STATE_RESET == hdfsdm_filter->State)
    {
      switch (CallbackID)
      {
        case HAL_DFSDM_FILTER_MSPINIT_CB_ID :
          hdfsdm_filter->MspInitCallback = pCallback;
          break;
        case HAL_DFSDM_FILTER_MSPDEINIT_CB_ID :
          hdfsdm_filter->MspDeInitCallback = pCallback;
          break;
        default :
          /* update the error code */
          hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
          /* update return status */
          status = HAL_ERROR;
          break;
      }
    }
    else
    {
      /* update the error code */
      hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
    }
  }
  return status;
}

/**
  * @brief  Unregister a user DFSDM filter callback.
  *         DFSDM filter callback is redirected to the weak predefined callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  CallbackID ID of the callback to be unregistered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_DFSDM_FILTER_REGCONV_COMPLETE_CB_ID regular conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_REGCONV_HALFCOMPLETE_CB_ID half regular conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_INJCONV_COMPLETE_CB_ID injected conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_INJCONV_HALFCOMPLETE_CB_ID half injected conversion complete callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_ERROR_CB_ID error callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_DFSDM_FILTER_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_Filter_UnRegisterCallback(DFSDM_Filter_HandleTypeDef        *hdfsdm_filter,
                                                      HAL_DFSDM_Filter_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (HAL_DFSDM_FILTER_STATE_READY == hdfsdm_filter->State)
  {
    switch (CallbackID)
    {
      case HAL_DFSDM_FILTER_REGCONV_COMPLETE_CB_ID :
        hdfsdm_filter->RegConvCpltCallback = HAL_DFSDM_FilterRegConvCpltCallback;
        break;
      case HAL_DFSDM_FILTER_REGCONV_HALFCOMPLETE_CB_ID :
        hdfsdm_filter->RegConvHalfCpltCallback = HAL_DFSDM_FilterRegConvHalfCpltCallback;
        break;
      case HAL_DFSDM_FILTER_INJCONV_COMPLETE_CB_ID :
        hdfsdm_filter->InjConvCpltCallback = HAL_DFSDM_FilterInjConvCpltCallback;
        break;
      case HAL_DFSDM_FILTER_INJCONV_HALFCOMPLETE_CB_ID :
        hdfsdm_filter->InjConvHalfCpltCallback = HAL_DFSDM_FilterInjConvHalfCpltCallback;
        break;
      case HAL_DFSDM_FILTER_ERROR_CB_ID :
        hdfsdm_filter->ErrorCallback = HAL_DFSDM_FilterErrorCallback;
        break;
      case HAL_DFSDM_FILTER_MSPINIT_CB_ID :
        hdfsdm_filter->MspInitCallback = HAL_DFSDM_FilterMspInit;
        break;
      case HAL_DFSDM_FILTER_MSPDEINIT_CB_ID :
        hdfsdm_filter->MspDeInitCallback = HAL_DFSDM_FilterMspDeInit;
        break;
      default :
        /* update the error code */
        hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
        /* update return status */
        status = HAL_ERROR;
        break;
    }
  }
  else if (HAL_DFSDM_FILTER_STATE_RESET == hdfsdm_filter->State)
  {
    switch (CallbackID)
    {
      case HAL_DFSDM_FILTER_MSPINIT_CB_ID :
        hdfsdm_filter->MspInitCallback = HAL_DFSDM_FilterMspInit;
        break;
      case HAL_DFSDM_FILTER_MSPDEINIT_CB_ID :
        hdfsdm_filter->MspDeInitCallback = HAL_DFSDM_FilterMspDeInit;
        break;
      default :
        /* update the error code */
        hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
        /* update return status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* update the error code */
    hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  return status;
}

/**
  * @brief  Register a user DFSDM filter analog watchdog callback
  *         to be used instead of the weak predefined callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  pCallback pointer to the DFSDM filter analog watchdog callback function.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_Filter_RegisterAwdCallback(DFSDM_Filter_HandleTypeDef      *hdfsdm_filter,
                                                       pDFSDM_Filter_AwdCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* update the error code */
    hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  else
  {
    if (HAL_DFSDM_FILTER_STATE_READY == hdfsdm_filter->State)
    {
      hdfsdm_filter->AwdCallback = pCallback;
    }
    else
    {
      /* update the error code */
      hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
    }
  }
  return status;
}

/**
  * @brief  Unregister a user DFSDM filter analog watchdog callback.
  *         DFSDM filter AWD callback is redirected to the weak predefined callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_DFSDM_Filter_UnRegisterAwdCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (HAL_DFSDM_FILTER_STATE_READY == hdfsdm_filter->State)
  {
    hdfsdm_filter->AwdCallback = HAL_DFSDM_FilterAwdCallback;
  }
  else
  {
    /* update the error code */
    hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  return status;
}
#endif /* USE_HAL_DFSDM_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup DFSDM_Exported_Functions_Group2_Filter Filter control functions
 *  @brief    Filter control functions
 *
@verbatim
  ==============================================================================
                    ##### Filter control functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Select channel and enable/disable continuous mode for regular conversion.
      (+) Select channels for injected conversion.
@endverbatim
  * @{
  */

/**
  * @brief  This function allows to select channel and to enable/disable
  *         continuous mode for regular conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Channel for regular conversion.
  *         This parameter can be a value of @ref DFSDM_Channel_Selection.
  * @param  ContinuousMode Enable/disable continuous mode for regular conversion.
  *         This parameter can be a value of @ref DFSDM_ContinuousMode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterConfigRegChannel(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                   uint32_t                    Channel,
                                                   uint32_t                    ContinuousMode)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(IS_DFSDM_REGULAR_CHANNEL(Channel));
  assert_param(IS_DFSDM_CONTINUOUS_MODE(ContinuousMode));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_RESET) &&
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_ERROR))
  {
    /* Configure channel and continuous mode for regular conversion */
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_RCH | DFSDM_FLTCR1_RCONT);
    if (ContinuousMode == DFSDM_CONTINUOUS_CONV_ON)
    {
      hdfsdm_filter->Instance->FLTCR1 |= (uint32_t)(((Channel & DFSDM_MSB_MASK) << DFSDM_FLTCR1_MSB_RCH_OFFSET) |
                                                    DFSDM_FLTCR1_RCONT);
    }
    else
    {
      hdfsdm_filter->Instance->FLTCR1 |= (uint32_t)((Channel & DFSDM_MSB_MASK) << DFSDM_FLTCR1_MSB_RCH_OFFSET);
    }
    /* Store continuous mode information */
    hdfsdm_filter->RegularContMode = ContinuousMode;
  }
  else
  {
    status = HAL_ERROR;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to select channels for injected conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Channels for injected conversion.
  *         This parameter can be a values combination of @ref DFSDM_Channel_Selection.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterConfigInjChannel(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                   uint32_t                    Channel)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(IS_DFSDM_INJECTED_CHANNEL(Channel));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_RESET) &&
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_ERROR))
  {
    /* Configure channel for injected conversion */
    hdfsdm_filter->Instance->FLTJCHGR = (uint32_t)(Channel & DFSDM_LSB_MASK);
    /* Store number of injected channels */
    hdfsdm_filter->InjectedChannelsNbr = DFSDM_GetInjChannelsNbr(Channel);
    /* Update number of injected channels remaining */
    hdfsdm_filter->InjConvRemaining = (hdfsdm_filter->InjectedScanMode == ENABLE) ? \
                                      hdfsdm_filter->InjectedChannelsNbr : 1U;
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @}
  */

/** @defgroup DFSDM_Exported_Functions_Group3_Filter Filter operation functions
 *  @brief    Filter operation functions
 *
@verbatim
  ==============================================================================
                    ##### Filter operation functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Start conversion of regular/injected channel.
      (+) Poll for the end of regular/injected conversion.
      (+) Stop conversion of regular/injected channel.
      (+) Start conversion of regular/injected channel and enable interrupt.
      (+) Call the callback functions at the end of regular/injected conversions.
      (+) Stop conversion of regular/injected channel and disable interrupt.
      (+) Start conversion of regular/injected channel and enable DMA transfer.
      (+) Stop conversion of regular/injected channel and disable DMA transfer.
      (+) Start analog watchdog and enable interrupt.
      (+) Call the callback function when analog watchdog occurs.
      (+) Stop analog watchdog and disable interrupt.
      (+) Start extreme detector.
      (+) Stop extreme detector.
      (+) Get result of regular channel conversion.
      (+) Get result of injected channel conversion.
      (+) Get extreme detector maximum and minimum values.
      (+) Get conversion time.
      (+) Handle DFSDM interrupt request.
@endverbatim
  * @{
  */

/**
  * @brief  This function allows to start regular conversion in polling mode.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if injected conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterRegularStart(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ))
  {
    /* Start regular conversion */
    DFSDM_RegConvStart(hdfsdm_filter);
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to poll for the end of regular conversion.
  * @note   This function should be called only if regular conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Timeout Timeout value in milliseconds.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterPollForRegConversion(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                       uint32_t                    Timeout)
{
  uint32_t tickstart;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    return HAL_ERROR;
  }
  else
  {
    /* Get timeout */
    tickstart = HAL_GetTick();

    /* Wait end of regular conversion */
    while ((hdfsdm_filter->Instance->FLTISR & DFSDM_FLTISR_REOCF) != DFSDM_FLTISR_REOCF)
    {
      /* Check the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U))
        {
          /* Return timeout status */
          return HAL_TIMEOUT;
        }
      }
    }
    /* Check if overrun occurs */
    if ((hdfsdm_filter->Instance->FLTISR & DFSDM_FLTISR_ROVRF) == DFSDM_FLTISR_ROVRF)
    {
      /* Update error code and call error callback */
      hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_REGULAR_OVERRUN;
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
      hdfsdm_filter->ErrorCallback(hdfsdm_filter);
#else
      HAL_DFSDM_FilterErrorCallback(hdfsdm_filter);
#endif

      /* Clear regular overrun flag */
      hdfsdm_filter->Instance->FLTICR = DFSDM_FLTICR_CLRROVRF;
    }
    /* Update DFSDM filter state only if not continuous conversion and SW trigger */
    if ((hdfsdm_filter->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
        (hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER))
    {
      hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG) ? \
                             HAL_DFSDM_FILTER_STATE_READY : HAL_DFSDM_FILTER_STATE_INJ;
    }
    /* Return function status */
    return HAL_OK;
  }
}

/**
  * @brief  This function allows to stop regular conversion in polling mode.
  * @note   This function should be called only if regular conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterRegularStop(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop regular conversion */
    DFSDM_RegConvStop(hdfsdm_filter);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start regular conversion in interrupt mode.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if injected conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterRegularStart_IT(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ))
  {
    /* Enable interrupts for regular conversions */
    hdfsdm_filter->Instance->FLTCR2 |= (DFSDM_FLTCR2_REOCIE | DFSDM_FLTCR2_ROVRIE);

    /* Start regular conversion */
    DFSDM_RegConvStart(hdfsdm_filter);
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to stop regular conversion in interrupt mode.
  * @note   This function should be called only if regular conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterRegularStop_IT(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Disable interrupts for regular conversions */
    hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_REOCIE | DFSDM_FLTCR2_ROVRIE);

    /* Stop regular conversion */
    DFSDM_RegConvStop(hdfsdm_filter);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start regular conversion in DMA mode.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if injected conversion is ongoing.
  *         Please note that data on buffer will contain signed regular conversion
  *         value on 24 most significant bits and corresponding channel on 3 least
  *         significant bits.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  pData The destination buffer address.
  * @param  Length The length of data to be transferred from DFSDM filter to memory.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterRegularStart_DMA(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                   int32_t                    *pData,
                                                   uint32_t                    Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check destination address and length */
  if ((pData == NULL) || (Length == 0U))
  {
    status = HAL_ERROR;
  }
  /* Check that DMA is enabled for regular conversion */
  else if ((hdfsdm_filter->Instance->FLTCR1 & DFSDM_FLTCR1_RDMAEN) != DFSDM_FLTCR1_RDMAEN)
  {
    status = HAL_ERROR;
  }
  /* Check parameters compatibility */
  else if ((hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
           (hdfsdm_filter->hdmaReg->Init.Mode == DMA_NORMAL) && \
           (Length != 1U))
  {
    status = HAL_ERROR;
  }
  else if ((hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
           (hdfsdm_filter->hdmaReg->Init.Mode == DMA_CIRCULAR))
  {
    status = HAL_ERROR;
  }
  /* Check DFSDM filter state */
  else if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
           (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ))
  {
    /* Set callbacks on DMA handler */
    hdfsdm_filter->hdmaReg->XferCpltCallback = DFSDM_DMARegularConvCplt;
    hdfsdm_filter->hdmaReg->XferErrorCallback = DFSDM_DMAError;
    hdfsdm_filter->hdmaReg->XferHalfCpltCallback = (hdfsdm_filter->hdmaReg->Init.Mode == DMA_CIRCULAR) ? \
                                                   DFSDM_DMARegularHalfConvCplt : NULL;

    /* Start DMA in interrupt mode */
    if (HAL_DMA_Start_IT(hdfsdm_filter->hdmaReg, (uint32_t)&hdfsdm_filter->Instance->FLTRDATAR, \
                         (uint32_t) pData, Length) != HAL_OK)
    {
      /* Set DFSDM filter in error state */
      hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_ERROR;
      status = HAL_ERROR;
    }
    else
    {
      /* Start regular conversion */
      DFSDM_RegConvStart(hdfsdm_filter);
    }
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start regular conversion in DMA mode and to get
  *         only the 16 most significant bits of conversion.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if injected conversion is ongoing.
  *         Please note that data on buffer will contain signed 16 most significant
  *         bits of regular conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  pData The destination buffer address.
  * @param  Length The length of data to be transferred from DFSDM filter to memory.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterRegularMsbStart_DMA(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                      int16_t                    *pData,
                                                      uint32_t                    Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check destination address and length */
  if ((pData == NULL) || (Length == 0U))
  {
    status = HAL_ERROR;
  }
  /* Check that DMA is enabled for regular conversion */
  else if ((hdfsdm_filter->Instance->FLTCR1 & DFSDM_FLTCR1_RDMAEN) != DFSDM_FLTCR1_RDMAEN)
  {
    status = HAL_ERROR;
  }
  /* Check parameters compatibility */
  else if ((hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
           (hdfsdm_filter->hdmaReg->Init.Mode == DMA_NORMAL) && \
           (Length != 1U))
  {
    status = HAL_ERROR;
  }
  else if ((hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
           (hdfsdm_filter->hdmaReg->Init.Mode == DMA_CIRCULAR))
  {
    status = HAL_ERROR;
  }
  /* Check DFSDM filter state */
  else if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
           (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ))
  {
    /* Set callbacks on DMA handler */
    hdfsdm_filter->hdmaReg->XferCpltCallback = DFSDM_DMARegularConvCplt;
    hdfsdm_filter->hdmaReg->XferErrorCallback = DFSDM_DMAError;
    hdfsdm_filter->hdmaReg->XferHalfCpltCallback = (hdfsdm_filter->hdmaReg->Init.Mode == DMA_CIRCULAR) ? \
                                                   DFSDM_DMARegularHalfConvCplt : NULL;

    /* Start DMA in interrupt mode */
    if (HAL_DMA_Start_IT(hdfsdm_filter->hdmaReg, (uint32_t)(&hdfsdm_filter->Instance->FLTRDATAR) + 2U, \
                         (uint32_t) pData, Length) != HAL_OK)
    {
      /* Set DFSDM filter in error state */
      hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_ERROR;
      status = HAL_ERROR;
    }
    else
    {
      /* Start regular conversion */
      DFSDM_RegConvStart(hdfsdm_filter);
    }
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to stop regular conversion in DMA mode.
  * @note   This function should be called only if regular conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterRegularStop_DMA(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop current DMA transfer */
    if (HAL_DMA_Abort(hdfsdm_filter->hdmaReg) != HAL_OK)
    {
      /* Set DFSDM filter in error state */
      hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_ERROR;
      status = HAL_ERROR;
    }
    else
    {
      /* Stop regular conversion */
      DFSDM_RegConvStop(hdfsdm_filter);
    }
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to get regular conversion value.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Corresponding channel of regular conversion.
  * @retval Regular conversion value
  */
int32_t HAL_DFSDM_FilterGetRegularValue(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                        uint32_t                   *Channel)
{
  uint32_t reg;
  int32_t  value;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(Channel != (void *)0);

  /* Get value of data register for regular channel */
  reg = hdfsdm_filter->Instance->FLTRDATAR;

  /* Extract channel and regular conversion value */
  *Channel = (reg & DFSDM_FLTRDATAR_RDATACH);
  /* Regular conversion value is a signed value located on 24 MSB of register */
  /* So after applying a mask on these bits we have to perform a division by 256 (2 raised to the power of 8) */
  reg &= DFSDM_FLTRDATAR_RDATA;
  value = ((int32_t)reg) / 256;

  /* return regular conversion value */
  return value;
}

/**
  * @brief  This function allows to start injected conversion in polling mode.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if regular conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInjectedStart(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG))
  {
    /* Start injected conversion */
    DFSDM_InjConvStart(hdfsdm_filter);
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to poll for the end of injected conversion.
  * @note   This function should be called only if injected conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Timeout Timeout value in milliseconds.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterPollForInjConversion(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                       uint32_t                    Timeout)
{
  uint32_t tickstart;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_INJ) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    return HAL_ERROR;
  }
  else
  {
    /* Get timeout */
    tickstart = HAL_GetTick();

    /* Wait end of injected conversions */
    while ((hdfsdm_filter->Instance->FLTISR & DFSDM_FLTISR_JEOCF) != DFSDM_FLTISR_JEOCF)
    {
      /* Check the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U))
        {
          /* Return timeout status */
          return HAL_TIMEOUT;
        }
      }
    }
    /* Check if overrun occurs */
    if ((hdfsdm_filter->Instance->FLTISR & DFSDM_FLTISR_JOVRF) == DFSDM_FLTISR_JOVRF)
    {
      /* Update error code and call error callback */
      hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INJECTED_OVERRUN;
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
      hdfsdm_filter->ErrorCallback(hdfsdm_filter);
#else
      HAL_DFSDM_FilterErrorCallback(hdfsdm_filter);
#endif

      /* Clear injected overrun flag */
      hdfsdm_filter->Instance->FLTICR = DFSDM_FLTICR_CLRJOVRF;
    }

    /* Update remaining injected conversions */
    hdfsdm_filter->InjConvRemaining--;
    if (hdfsdm_filter->InjConvRemaining == 0U)
    {
      /* Update DFSDM filter state only if trigger is software */
      if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER)
      {
        hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ) ? \
                               HAL_DFSDM_FILTER_STATE_READY : HAL_DFSDM_FILTER_STATE_REG;
      }

      /* end of injected sequence, reset the value */
      hdfsdm_filter->InjConvRemaining = (hdfsdm_filter->InjectedScanMode == ENABLE) ? \
                                        hdfsdm_filter->InjectedChannelsNbr : 1U;
    }

    /* Return function status */
    return HAL_OK;
  }
}

/**
  * @brief  This function allows to stop injected conversion in polling mode.
  * @note   This function should be called only if injected conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInjectedStop(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_INJ) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop injected conversion */
    DFSDM_InjConvStop(hdfsdm_filter);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start injected conversion in interrupt mode.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if regular conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInjectedStart_IT(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG))
  {
    /* Enable interrupts for injected conversions */
    hdfsdm_filter->Instance->FLTCR2 |= (DFSDM_FLTCR2_JEOCIE | DFSDM_FLTCR2_JOVRIE);

    /* Start injected conversion */
    DFSDM_InjConvStart(hdfsdm_filter);
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to stop injected conversion in interrupt mode.
  * @note   This function should be called only if injected conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInjectedStop_IT(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_INJ) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Disable interrupts for injected conversions */
    hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_JEOCIE | DFSDM_FLTCR2_JOVRIE);

    /* Stop injected conversion */
    DFSDM_InjConvStop(hdfsdm_filter);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start injected conversion in DMA mode.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if regular conversion is ongoing.
  *         Please note that data on buffer will contain signed injected conversion
  *         value on 24 most significant bits and corresponding channel on 3 least
  *         significant bits.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  pData The destination buffer address.
  * @param  Length The length of data to be transferred from DFSDM filter to memory.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInjectedStart_DMA(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                    int32_t                    *pData,
                                                    uint32_t                    Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check destination address and length */
  if ((pData == NULL) || (Length == 0U))
  {
    status = HAL_ERROR;
  }
  /* Check that DMA is enabled for injected conversion */
  else if ((hdfsdm_filter->Instance->FLTCR1 & DFSDM_FLTCR1_JDMAEN) != DFSDM_FLTCR1_JDMAEN)
  {
    status = HAL_ERROR;
  }
  /* Check parameters compatibility */
  else if ((hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->hdmaInj->Init.Mode == DMA_NORMAL) && \
           (Length > hdfsdm_filter->InjConvRemaining))
  {
    status = HAL_ERROR;
  }
  else if ((hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->hdmaInj->Init.Mode == DMA_CIRCULAR))
  {
    status = HAL_ERROR;
  }
  /* Check DFSDM filter state */
  else if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
           (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG))
  {
    /* Set callbacks on DMA handler */
    hdfsdm_filter->hdmaInj->XferCpltCallback = DFSDM_DMAInjectedConvCplt;
    hdfsdm_filter->hdmaInj->XferErrorCallback = DFSDM_DMAError;
    hdfsdm_filter->hdmaInj->XferHalfCpltCallback = (hdfsdm_filter->hdmaInj->Init.Mode == DMA_CIRCULAR) ? \
                                                   DFSDM_DMAInjectedHalfConvCplt : NULL;

    /* Start DMA in interrupt mode */
    if (HAL_DMA_Start_IT(hdfsdm_filter->hdmaInj, (uint32_t)&hdfsdm_filter->Instance->FLTJDATAR, \
                         (uint32_t) pData, Length) != HAL_OK)
    {
      /* Set DFSDM filter in error state */
      hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_ERROR;
      status = HAL_ERROR;
    }
    else
    {
      /* Start injected conversion */
      DFSDM_InjConvStart(hdfsdm_filter);
    }
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start injected conversion in DMA mode and to get
  *         only the 16 most significant bits of conversion.
  * @note   This function should be called only when DFSDM filter instance is
  *         in idle state or if regular conversion is ongoing.
  *         Please note that data on buffer will contain signed 16 most significant
  *         bits of injected conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  pData The destination buffer address.
  * @param  Length The length of data to be transferred from DFSDM filter to memory.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInjectedMsbStart_DMA(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                                       int16_t                    *pData,
                                                       uint32_t                    Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check destination address and length */
  if ((pData == NULL) || (Length == 0U))
  {
    status = HAL_ERROR;
  }
  /* Check that DMA is enabled for injected conversion */
  else if ((hdfsdm_filter->Instance->FLTCR1 & DFSDM_FLTCR1_JDMAEN) != DFSDM_FLTCR1_JDMAEN)
  {
    status = HAL_ERROR;
  }
  /* Check parameters compatibility */
  else if ((hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->hdmaInj->Init.Mode == DMA_NORMAL) && \
           (Length > hdfsdm_filter->InjConvRemaining))
  {
    status = HAL_ERROR;
  }
  else if ((hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER) && \
           (hdfsdm_filter->hdmaInj->Init.Mode == DMA_CIRCULAR))
  {
    status = HAL_ERROR;
  }
  /* Check DFSDM filter state */
  else if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) || \
           (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG))
  {
    /* Set callbacks on DMA handler */
    hdfsdm_filter->hdmaInj->XferCpltCallback = DFSDM_DMAInjectedConvCplt;
    hdfsdm_filter->hdmaInj->XferErrorCallback = DFSDM_DMAError;
    hdfsdm_filter->hdmaInj->XferHalfCpltCallback = (hdfsdm_filter->hdmaInj->Init.Mode == DMA_CIRCULAR) ? \
                                                   DFSDM_DMAInjectedHalfConvCplt : NULL;

    /* Start DMA in interrupt mode */
    if (HAL_DMA_Start_IT(hdfsdm_filter->hdmaInj, (uint32_t)(&hdfsdm_filter->Instance->FLTJDATAR) + 2U, \
                         (uint32_t) pData, Length) != HAL_OK)
    {
      /* Set DFSDM filter in error state */
      hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_ERROR;
      status = HAL_ERROR;
    }
    else
    {
      /* Start injected conversion */
      DFSDM_InjConvStart(hdfsdm_filter);
    }
  }
  else
  {
    status = HAL_ERROR;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to stop injected conversion in DMA mode.
  * @note   This function should be called only if injected conversion is ongoing.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterInjectedStop_DMA(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_INJ) && \
      (hdfsdm_filter->State != HAL_DFSDM_FILTER_STATE_REG_INJ))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Stop current DMA transfer */
    if (HAL_DMA_Abort(hdfsdm_filter->hdmaInj) != HAL_OK)
    {
      /* Set DFSDM filter in error state */
      hdfsdm_filter->State = HAL_DFSDM_FILTER_STATE_ERROR;
      status = HAL_ERROR;
    }
    else
    {
      /* Stop regular conversion */
      DFSDM_InjConvStop(hdfsdm_filter);
    }
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to get injected conversion value.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Corresponding channel of injected conversion.
  * @retval Injected conversion value
  */
int32_t HAL_DFSDM_FilterGetInjectedValue(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                         uint32_t                   *Channel)
{
  uint32_t reg;
  int32_t  value;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(Channel != (void *)0);

  /* Get value of data register for injected channel */
  reg = hdfsdm_filter->Instance->FLTJDATAR;

  /* Extract channel and injected conversion value */
  *Channel = (reg & DFSDM_FLTJDATAR_JDATACH);
  /* Injected conversion value is a signed value located on 24 MSB of register */
  /* So after applying a mask on these bits we have to perform a division by 256 (2 raised to the power of 8) */
  reg &= DFSDM_FLTJDATAR_JDATA;
  value = ((int32_t)reg) / 256;

  /* return regular conversion value */
  return value;
}

/**
  * @brief  This function allows to start filter analog watchdog in interrupt mode.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  awdParam DFSDM filter analog watchdog parameters.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterAwdStart_IT(DFSDM_Filter_HandleTypeDef   *hdfsdm_filter,
                                              DFSDM_Filter_AwdParamTypeDef *awdParam)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(IS_DFSDM_FILTER_AWD_DATA_SOURCE(awdParam->DataSource));
  assert_param(IS_DFSDM_INJECTED_CHANNEL(awdParam->Channel));
  assert_param(IS_DFSDM_FILTER_AWD_THRESHOLD(awdParam->HighThreshold));
  assert_param(IS_DFSDM_FILTER_AWD_THRESHOLD(awdParam->LowThreshold));
  assert_param(IS_DFSDM_BREAK_SIGNALS(awdParam->HighBreakSignal));
  assert_param(IS_DFSDM_BREAK_SIGNALS(awdParam->LowBreakSignal));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_RESET) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_ERROR))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Set analog watchdog data source */
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_AWFSEL);
    hdfsdm_filter->Instance->FLTCR1 |= awdParam->DataSource;

    /* Set thresholds and break signals */
    hdfsdm_filter->Instance->FLTAWHTR &= ~(DFSDM_FLTAWHTR_AWHT | DFSDM_FLTAWHTR_BKAWH);
    hdfsdm_filter->Instance->FLTAWHTR |= (((uint32_t) awdParam->HighThreshold << DFSDM_FLTAWHTR_AWHT_Pos) | \
                                          awdParam->HighBreakSignal);
    hdfsdm_filter->Instance->FLTAWLTR &= ~(DFSDM_FLTAWLTR_AWLT | DFSDM_FLTAWLTR_BKAWL);
    hdfsdm_filter->Instance->FLTAWLTR |= (((uint32_t) awdParam->LowThreshold << DFSDM_FLTAWLTR_AWLT_Pos) | \
                                          awdParam->LowBreakSignal);

    /* Set channels and interrupt for analog watchdog */
    hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_AWDCH);
    hdfsdm_filter->Instance->FLTCR2 |= (((awdParam->Channel & DFSDM_LSB_MASK) << DFSDM_FLTCR2_AWDCH_Pos) | \
                                        DFSDM_FLTCR2_AWDIE);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to stop filter analog watchdog in interrupt mode.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterAwdStop_IT(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_RESET) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_ERROR))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Reset channels for analog watchdog and deactivate interrupt */
    hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_AWDCH | DFSDM_FLTCR2_AWDIE);

    /* Clear all analog watchdog flags */
    hdfsdm_filter->Instance->FLTAWCFR = (DFSDM_FLTAWCFR_CLRAWHTF | DFSDM_FLTAWCFR_CLRAWLTF);

    /* Reset thresholds and break signals */
    hdfsdm_filter->Instance->FLTAWHTR &= ~(DFSDM_FLTAWHTR_AWHT | DFSDM_FLTAWHTR_BKAWH);
    hdfsdm_filter->Instance->FLTAWLTR &= ~(DFSDM_FLTAWLTR_AWLT | DFSDM_FLTAWLTR_BKAWL);

    /* Reset analog watchdog data source */
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_AWFSEL);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to start extreme detector feature.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Channels where extreme detector is enabled.
  *         This parameter can be a values combination of @ref DFSDM_Channel_Selection.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterExdStart(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                           uint32_t                    Channel)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(IS_DFSDM_INJECTED_CHANNEL(Channel));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_RESET) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_ERROR))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Set channels for extreme detector */
    hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_EXCH);
    hdfsdm_filter->Instance->FLTCR2 |= ((Channel & DFSDM_LSB_MASK) << DFSDM_FLTCR2_EXCH_Pos);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to stop extreme detector feature.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DFSDM_FilterExdStop(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  HAL_StatusTypeDef status = HAL_OK;
  __IO uint32_t     reg1;
  __IO uint32_t     reg2;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Check DFSDM filter state */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_RESET) || \
      (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_ERROR))
  {
    /* Return error status */
    status = HAL_ERROR;
  }
  else
  {
    /* Reset channels for extreme detector */
    hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_EXCH);

    /* Clear extreme detector values */
    reg1 = hdfsdm_filter->Instance->FLTEXMAX;
    reg2 = hdfsdm_filter->Instance->FLTEXMIN;
    UNUSED(reg1); /* To avoid GCC warning */
    UNUSED(reg2); /* To avoid GCC warning */
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to get extreme detector maximum value.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Corresponding channel.
  * @retval Extreme detector maximum value
  *         This value is between Min_Data = -8388608 and Max_Data = 8388607.
  */
int32_t HAL_DFSDM_FilterGetExdMaxValue(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                       uint32_t                   *Channel)
{
  uint32_t reg;
  int32_t  value;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(Channel != (void *)0);

  /* Get value of extreme detector maximum register */
  reg = hdfsdm_filter->Instance->FLTEXMAX;

  /* Extract channel and extreme detector maximum value */
  *Channel = (reg & DFSDM_FLTEXMAX_EXMAXCH);
  /* Extreme detector maximum value is a signed value located on 24 MSB of register */
  /* So after applying a mask on these bits we have to perform a division by 256 (2 raised to the power of 8) */
  reg &= DFSDM_FLTEXMAX_EXMAX;
  value = ((int32_t)reg) / 256;

  /* return extreme detector maximum value */
  return value;
}

/**
  * @brief  This function allows to get extreme detector minimum value.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Corresponding channel.
  * @retval Extreme detector minimum value
  *         This value is between Min_Data = -8388608 and Max_Data = 8388607.
  */
int32_t HAL_DFSDM_FilterGetExdMinValue(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                       uint32_t                   *Channel)
{
  uint32_t reg;
  int32_t  value;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));
  assert_param(Channel != (void *)0);

  /* Get value of extreme detector minimum register */
  reg = hdfsdm_filter->Instance->FLTEXMIN;

  /* Extract channel and extreme detector minimum value */
  *Channel = (reg & DFSDM_FLTEXMIN_EXMINCH);
  /* Extreme detector minimum value is a signed value located on 24 MSB of register */
  /* So after applying a mask on these bits we have to perform a division by 256 (2 raised to the power of 8) */
  reg &= DFSDM_FLTEXMIN_EXMIN;
  value = ((int32_t)reg) / 256;

  /* return extreme detector minimum value */
  return value;
}

/**
  * @brief  This function allows to get conversion time value.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval Conversion time value
  * @note   To get time in second, this value has to be divided by DFSDM clock frequency.
  */
uint32_t HAL_DFSDM_FilterGetConvTimeValue(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  uint32_t reg;
  uint32_t value;

  /* Check parameters */
  assert_param(IS_DFSDM_FILTER_ALL_INSTANCE(hdfsdm_filter->Instance));

  /* Get value of conversion timer register */
  reg = hdfsdm_filter->Instance->FLTCNVTIMR;

  /* Extract conversion time value */
  value = ((reg & DFSDM_FLTCNVTIMR_CNVCNT) >> DFSDM_FLTCNVTIMR_CNVCNT_Pos);

  /* return extreme detector minimum value */
  return value;
}

/**
  * @brief  This function handles the DFSDM interrupts.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_IRQHandler(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Get FTLISR and FLTCR2 register values */
  const uint32_t temp_fltisr = hdfsdm_filter->Instance->FLTISR;
  const uint32_t temp_fltcr2 = hdfsdm_filter->Instance->FLTCR2;

  /* Check if overrun occurs during regular conversion */
  if (((temp_fltisr & DFSDM_FLTISR_ROVRF) != 0U) && \
      ((temp_fltcr2 & DFSDM_FLTCR2_ROVRIE) != 0U))
  {
    /* Clear regular overrun flag */
    hdfsdm_filter->Instance->FLTICR = DFSDM_FLTICR_CLRROVRF;

    /* Update error code */
    hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_REGULAR_OVERRUN;

    /* Call error callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
    hdfsdm_filter->ErrorCallback(hdfsdm_filter);
#else
    HAL_DFSDM_FilterErrorCallback(hdfsdm_filter);
#endif
  }
  /* Check if overrun occurs during injected conversion */
  else if (((temp_fltisr & DFSDM_FLTISR_JOVRF) != 0U) && \
           ((temp_fltcr2 & DFSDM_FLTCR2_JOVRIE) != 0U))
  {
    /* Clear injected overrun flag */
    hdfsdm_filter->Instance->FLTICR = DFSDM_FLTICR_CLRJOVRF;

    /* Update error code */
    hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_INJECTED_OVERRUN;

    /* Call error callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
    hdfsdm_filter->ErrorCallback(hdfsdm_filter);
#else
    HAL_DFSDM_FilterErrorCallback(hdfsdm_filter);
#endif
  }
  /* Check if end of regular conversion */
  else if (((temp_fltisr & DFSDM_FLTISR_REOCF) != 0U) && \
           ((temp_fltcr2 & DFSDM_FLTCR2_REOCIE) != 0U))
  {
    /* Call regular conversion complete callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
    hdfsdm_filter->RegConvCpltCallback(hdfsdm_filter);
#else
    HAL_DFSDM_FilterRegConvCpltCallback(hdfsdm_filter);
#endif

    /* End of conversion if mode is not continuous and software trigger */
    if ((hdfsdm_filter->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
        (hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER))
    {
      /* Disable interrupts for regular conversions */
      hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_REOCIE);

      /* Update DFSDM filter state */
      hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG) ? \
                             HAL_DFSDM_FILTER_STATE_READY : HAL_DFSDM_FILTER_STATE_INJ;
    }
  }
  /* Check if end of injected conversion */
  else if (((temp_fltisr & DFSDM_FLTISR_JEOCF) != 0U) && \
           ((temp_fltcr2 & DFSDM_FLTCR2_JEOCIE) != 0U))
  {
    /* Call injected conversion complete callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
    hdfsdm_filter->InjConvCpltCallback(hdfsdm_filter);
#else
    HAL_DFSDM_FilterInjConvCpltCallback(hdfsdm_filter);
#endif

    /* Update remaining injected conversions */
    hdfsdm_filter->InjConvRemaining--;
    if (hdfsdm_filter->InjConvRemaining == 0U)
    {
      /* End of conversion if trigger is software */
      if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER)
      {
        /* Disable interrupts for injected conversions */
        hdfsdm_filter->Instance->FLTCR2 &= ~(DFSDM_FLTCR2_JEOCIE);

        /* Update DFSDM filter state */
        hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ) ? \
                               HAL_DFSDM_FILTER_STATE_READY : HAL_DFSDM_FILTER_STATE_REG;
      }
      /* end of injected sequence, reset the value */
      hdfsdm_filter->InjConvRemaining = (hdfsdm_filter->InjectedScanMode == ENABLE) ? \
                                        hdfsdm_filter->InjectedChannelsNbr : 1U;
    }
  }
  /* Check if analog watchdog occurs */
  else if (((temp_fltisr & DFSDM_FLTISR_AWDF) != 0U) && \
           ((temp_fltcr2 & DFSDM_FLTCR2_AWDIE) != 0U))
  {
    uint32_t reg;
    uint32_t threshold;
    uint32_t channel = 0;

    /* Get channel and threshold */
    reg = hdfsdm_filter->Instance->FLTAWSR;
    threshold = ((reg & DFSDM_FLTAWSR_AWLTF) != 0U) ? DFSDM_AWD_LOW_THRESHOLD : DFSDM_AWD_HIGH_THRESHOLD;
    if (threshold == DFSDM_AWD_HIGH_THRESHOLD)
    {
      reg = reg >> DFSDM_FLTAWSR_AWHTF_Pos;
    }
    while (((reg & 1U) == 0U) && (channel < (DFSDM1_CHANNEL_NUMBER - 1U)))
    {
      channel++;
      reg = reg >> 1;
    }
    /* Clear analog watchdog flag */
    hdfsdm_filter->Instance->FLTAWCFR = (threshold == DFSDM_AWD_HIGH_THRESHOLD) ? \
                                        (1UL << (DFSDM_FLTAWSR_AWHTF_Pos + channel)) : \
                                        (1UL << channel);

    /* Call analog watchdog callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
    hdfsdm_filter->AwdCallback(hdfsdm_filter, channel, threshold);
#else
    HAL_DFSDM_FilterAwdCallback(hdfsdm_filter, channel, threshold);
#endif
  }
  /* Check if clock absence occurs */
  else if ((hdfsdm_filter->Instance == DFSDM1_Filter0) && \
           ((temp_fltisr & DFSDM_FLTISR_CKABF) != 0U) && \
           ((temp_fltcr2 & DFSDM_FLTCR2_CKABIE) != 0U))
  {
    uint32_t reg;
    uint32_t channel = 0;

    reg = ((hdfsdm_filter->Instance->FLTISR & DFSDM_FLTISR_CKABF) >> DFSDM_FLTISR_CKABF_Pos);

    while (channel < DFSDM1_CHANNEL_NUMBER)
    {
      /* Check if flag is set and corresponding channel is enabled */
      if (((reg & 1U) != 0U) && (a_dfsdm1ChannelHandle[channel] != NULL))
      {
        /* Check clock absence has been enabled for this channel */
        if ((a_dfsdm1ChannelHandle[channel]->Instance->CHCFGR1 & DFSDM_CHCFGR1_CKABEN) != 0U)
        {
          /* Clear clock absence flag */
          hdfsdm_filter->Instance->FLTICR = (1UL << (DFSDM_FLTICR_CLRCKABF_Pos + channel));

          /* Call clock absence callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
          a_dfsdm1ChannelHandle[channel]->CkabCallback(a_dfsdm1ChannelHandle[channel]);
#else
          HAL_DFSDM_ChannelCkabCallback(a_dfsdm1ChannelHandle[channel]);
#endif
        }
      }
      channel++;
      reg = reg >> 1;
    }
  }
  /* Check if short circuit detection occurs */
  else if ((hdfsdm_filter->Instance == DFSDM1_Filter0) && \
           ((temp_fltisr & DFSDM_FLTISR_SCDF) != 0U) && \
           ((temp_fltcr2 & DFSDM_FLTCR2_SCDIE) != 0U))
  {
    uint32_t reg;
    uint32_t channel = 0;

    /* Get channel */
    reg = ((hdfsdm_filter->Instance->FLTISR & DFSDM_FLTISR_SCDF) >> DFSDM_FLTISR_SCDF_Pos);
    while (((reg & 1U) == 0U) && (channel < (DFSDM1_CHANNEL_NUMBER - 1U)))
    {
      channel++;
      reg = reg >> 1;
    }

    /* Clear short circuit detection flag */
    hdfsdm_filter->Instance->FLTICR = (1UL << (DFSDM_FLTICR_CLRSCDF_Pos + channel));

    /* Call short circuit detection callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
    a_dfsdm1ChannelHandle[channel]->ScdCallback(a_dfsdm1ChannelHandle[channel]);
#else
    HAL_DFSDM_ChannelScdCallback(a_dfsdm1ChannelHandle[channel]);
#endif
  }
}

/**
  * @brief  Regular conversion complete callback.
  * @note   In interrupt mode, user has to read conversion value in this function
  *         using HAL_DFSDM_FilterGetRegularValue.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
__weak void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_FilterRegConvCpltCallback could be implemented in the user file.
   */
}

/**
  * @brief  Half regular conversion complete callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
__weak void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_FilterRegConvHalfCpltCallback could be implemented in the user file.
   */
}

/**
  * @brief  Injected conversion complete callback.
  * @note   In interrupt mode, user has to read conversion value in this function
  *         using HAL_DFSDM_FilterGetInjectedValue.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
__weak void HAL_DFSDM_FilterInjConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_FilterInjConvCpltCallback could be implemented in the user file.
   */
}

/**
  * @brief  Half injected conversion complete callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
__weak void HAL_DFSDM_FilterInjConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_FilterInjConvHalfCpltCallback could be implemented in the user file.
   */
}

/**
  * @brief  Filter analog watchdog callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @param  Channel Corresponding channel.
  * @param  Threshold Low or high threshold has been reached.
  * @retval None
  */
__weak void HAL_DFSDM_FilterAwdCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter,
                                        uint32_t Channel, uint32_t Threshold)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);
  UNUSED(Channel);
  UNUSED(Threshold);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_FilterAwdCallback could be implemented in the user file.
   */
}

/**
  * @brief  Error callback.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
__weak void HAL_DFSDM_FilterErrorCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdfsdm_filter);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DFSDM_FilterErrorCallback could be implemented in the user file.
   */
}

/**
  * @}
  */

/** @defgroup DFSDM_Exported_Functions_Group4_Filter Filter state functions
 *  @brief    Filter state functions
 *
@verbatim
  ==============================================================================
                     ##### Filter state functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Get the DFSDM filter state.
      (+) Get the DFSDM filter error.
@endverbatim
  * @{
  */

/**
  * @brief  This function allows to get the current DFSDM filter handle state.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval DFSDM filter state.
  */
HAL_DFSDM_Filter_StateTypeDef HAL_DFSDM_FilterGetState(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Return DFSDM filter handle state */
  return hdfsdm_filter->State;
}

/**
  * @brief  This function allows to get the current DFSDM filter error.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval DFSDM filter error code.
  */
uint32_t HAL_DFSDM_FilterGetError(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  return hdfsdm_filter->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */
/* End of exported functions -------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/** @addtogroup DFSDM_Private_Functions DFSDM Private Functions
  * @{
  */

/**
  * @brief  DMA half transfer complete callback for regular conversion.
  * @param  hdma DMA handle.
  * @retval None
  */
static void DFSDM_DMARegularHalfConvCplt(DMA_HandleTypeDef *hdma)
{
  /* Get DFSDM filter handle */
  DFSDM_Filter_HandleTypeDef *hdfsdm_filter = (DFSDM_Filter_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Call regular half conversion complete callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  hdfsdm_filter->RegConvHalfCpltCallback(hdfsdm_filter);
#else
  HAL_DFSDM_FilterRegConvHalfCpltCallback(hdfsdm_filter);
#endif
}

/**
  * @brief  DMA transfer complete callback for regular conversion.
  * @param  hdma DMA handle.
  * @retval None
  */
static void DFSDM_DMARegularConvCplt(DMA_HandleTypeDef *hdma)
{
  /* Get DFSDM filter handle */
  DFSDM_Filter_HandleTypeDef *hdfsdm_filter = (DFSDM_Filter_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Call regular conversion complete callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  hdfsdm_filter->RegConvCpltCallback(hdfsdm_filter);
#else
  HAL_DFSDM_FilterRegConvCpltCallback(hdfsdm_filter);
#endif
}

/**
  * @brief  DMA half transfer complete callback for injected conversion.
  * @param  hdma DMA handle.
  * @retval None
  */
static void DFSDM_DMAInjectedHalfConvCplt(DMA_HandleTypeDef *hdma)
{
  /* Get DFSDM filter handle */
  DFSDM_Filter_HandleTypeDef *hdfsdm_filter = (DFSDM_Filter_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Call injected half conversion complete callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  hdfsdm_filter->InjConvHalfCpltCallback(hdfsdm_filter);
#else
  HAL_DFSDM_FilterInjConvHalfCpltCallback(hdfsdm_filter);
#endif
}

/**
  * @brief  DMA transfer complete callback for injected conversion.
  * @param  hdma DMA handle.
  * @retval None
  */
static void DFSDM_DMAInjectedConvCplt(DMA_HandleTypeDef *hdma)
{
  /* Get DFSDM filter handle */
  DFSDM_Filter_HandleTypeDef *hdfsdm_filter = (DFSDM_Filter_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Call injected conversion complete callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  hdfsdm_filter->InjConvCpltCallback(hdfsdm_filter);
#else
  HAL_DFSDM_FilterInjConvCpltCallback(hdfsdm_filter);
#endif
}

/**
  * @brief  DMA error callback.
  * @param  hdma DMA handle.
  * @retval None
  */
static void DFSDM_DMAError(DMA_HandleTypeDef *hdma)
{
  /* Get DFSDM filter handle */
  DFSDM_Filter_HandleTypeDef *hdfsdm_filter = (DFSDM_Filter_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Update error code */
  hdfsdm_filter->ErrorCode = DFSDM_FILTER_ERROR_DMA;

  /* Call error callback */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  hdfsdm_filter->ErrorCallback(hdfsdm_filter);
#else
  HAL_DFSDM_FilterErrorCallback(hdfsdm_filter);
#endif
}

/**
  * @brief  This function allows to get the number of injected channels.
  * @param  Channels bitfield of injected channels.
  * @retval Number of injected channels.
  */
static uint32_t DFSDM_GetInjChannelsNbr(uint32_t Channels)
{
  uint32_t nbChannels = 0;
  uint32_t tmp;

  /* Get the number of channels from bitfield */
  tmp = (uint32_t)(Channels & DFSDM_LSB_MASK);
  while (tmp != 0U)
  {
    if ((tmp & 1U) != 0U)
    {
      nbChannels++;
    }
    tmp = (uint32_t)(tmp >> 1);
  }
  return nbChannels;
}

/**
  * @brief  This function allows to get the channel number from channel instance.
  * @param  Instance DFSDM channel instance.
  * @retval Channel number.
  */
static uint32_t DFSDM_GetChannelFromInstance(const DFSDM_Channel_TypeDef *Instance)
{
  uint32_t channel;

  /* Get channel from instance */
  if (Instance == DFSDM1_Channel0)
  {
    channel = 0;
  }
  else if (Instance == DFSDM1_Channel1)
  {
    channel = 1;
  }
  else if (Instance == DFSDM1_Channel2)
  {
    channel = 2;
  }
  else if (Instance == DFSDM1_Channel3)
  {
    channel = 3;
  }
#if defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || \
    defined(STM32L496xx) || defined(STM32L4A6xx) || \
    defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  else if (Instance == DFSDM1_Channel4)
  {
    channel = 4;
  }
  else if (Instance == DFSDM1_Channel5)
  {
    channel = 5;
  }
  else if (Instance == DFSDM1_Channel6)
  {
    channel = 6;
  }
  else if (Instance == DFSDM1_Channel7)
  {
    channel = 7;
  }
#endif /* STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx || STM32L496xx || STM32L4A6xx || STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
  else
  {
    channel = 0;
  }

  return channel;
}

/**
  * @brief  This function allows to really start regular conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
static void DFSDM_RegConvStart(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Check regular trigger */
  if (hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER)
  {
    /* Software start of regular conversion */
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_RSWSTART;
  }
  else /* synchronous trigger */
  {
    /* Disable DFSDM filter */
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_DFEN);

    /* Set RSYNC bit in DFSDM_FLTCR1 register */
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_RSYNC;

    /* Enable DFSDM  filter */
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_DFEN;

    /* If injected conversion was in progress, restart it */
    if (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ)
    {
      if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER)
      {
        hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_JSWSTART;
      }
      /* Update remaining injected conversions */
      hdfsdm_filter->InjConvRemaining = (hdfsdm_filter->InjectedScanMode == ENABLE) ? \
                                        hdfsdm_filter->InjectedChannelsNbr : 1U;
    }
  }
  /* Update DFSDM filter state */
  hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) ? \
                         HAL_DFSDM_FILTER_STATE_REG : HAL_DFSDM_FILTER_STATE_REG_INJ;
}

/**
  * @brief  This function allows to really stop regular conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
static void DFSDM_RegConvStop(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Disable DFSDM filter */
  hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_DFEN);

  /* If regular trigger was synchronous, reset RSYNC bit in DFSDM_FLTCR1 register */
  if (hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SYNC_TRIGGER)
  {
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_RSYNC);
  }

  /* Enable DFSDM filter */
  hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_DFEN;

  /* If injected conversion was in progress, restart it */
  if (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG_INJ)
  {
    if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER)
    {
      hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_JSWSTART;
    }
    /* Update remaining injected conversions */
    hdfsdm_filter->InjConvRemaining = (hdfsdm_filter->InjectedScanMode == ENABLE) ? \
                                      hdfsdm_filter->InjectedChannelsNbr : 1U;
  }

  /* Update DFSDM filter state */
  hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG) ? \
                         HAL_DFSDM_FILTER_STATE_READY : HAL_DFSDM_FILTER_STATE_INJ;
}

/**
  * @brief  This function allows to really start injected conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
static void DFSDM_InjConvStart(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Check injected trigger */
  if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SW_TRIGGER)
  {
    /* Software start of injected conversion */
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_JSWSTART;
  }
  else /* external or synchronous trigger */
  {
    /* Disable DFSDM filter */
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_DFEN);

    if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SYNC_TRIGGER)
    {
      /* Set JSYNC bit in DFSDM_FLTCR1 register */
      hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_JSYNC;
    }
    else /* external trigger */
    {
      /* Set JEXTEN[1:0] bits in DFSDM_FLTCR1 register */
      hdfsdm_filter->Instance->FLTCR1 |= hdfsdm_filter->ExtTriggerEdge;
    }

    /* Enable DFSDM filter */
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_DFEN;

    /* If regular conversion was in progress, restart it */
    if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG) && \
        (hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER))
    {
      hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_RSWSTART;
    }
  }
  /* Update DFSDM filter state */
  hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_READY) ? \
                         HAL_DFSDM_FILTER_STATE_INJ : HAL_DFSDM_FILTER_STATE_REG_INJ;
}

/**
  * @brief  This function allows to really stop injected conversion.
  * @param  hdfsdm_filter DFSDM filter handle.
  * @retval None
  */
static void DFSDM_InjConvStop(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  /* Disable DFSDM filter */
  hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_DFEN);

  /* If injected trigger was synchronous, reset JSYNC bit in DFSDM_FLTCR1 register */
  if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_SYNC_TRIGGER)
  {
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_JSYNC);
  }
  else if (hdfsdm_filter->InjectedTrigger == DFSDM_FILTER_EXT_TRIGGER)
  {
    /* Reset JEXTEN[1:0] bits in DFSDM_FLTCR1 register */
    hdfsdm_filter->Instance->FLTCR1 &= ~(DFSDM_FLTCR1_JEXTEN);
  }
  else
  {
    /* Nothing to do */
  }

  /* Enable DFSDM filter */
  hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_DFEN;

  /* If regular conversion was in progress, restart it */
  if ((hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_REG_INJ) && \
      (hdfsdm_filter->RegularTrigger == DFSDM_FILTER_SW_TRIGGER))
  {
    hdfsdm_filter->Instance->FLTCR1 |= DFSDM_FLTCR1_RSWSTART;
  }

  /* Update remaining injected conversions */
  hdfsdm_filter->InjConvRemaining = (hdfsdm_filter->InjectedScanMode == ENABLE) ? \
                                    hdfsdm_filter->InjectedChannelsNbr : 1U;

  /* Update DFSDM filter state */
  hdfsdm_filter->State = (hdfsdm_filter->State == HAL_DFSDM_FILTER_STATE_INJ) ? \
                         HAL_DFSDM_FILTER_STATE_READY : HAL_DFSDM_FILTER_STATE_REG;
}

/**
  * @}
  */
/* End of private functions --------------------------------------------------*/

/**
  * @}
  */

#endif /* STM32L451xx || STM32L452xx || STM32L462xx || STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx || STM32L496xx || STM32L4A6xx || STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#endif /* HAL_DFSDM_MODULE_ENABLED */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
