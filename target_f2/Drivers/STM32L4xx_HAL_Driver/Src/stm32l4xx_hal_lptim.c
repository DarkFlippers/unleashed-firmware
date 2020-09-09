/**
  ******************************************************************************
  * @file    stm32l4xx_hal_lptim.c
  * @author  MCD Application Team
  * @brief   LPTIM HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Low Power Timer (LPTIM) peripheral:
  *           + Initialization and de-initialization functions.
  *           + Start/Stop operation functions in polling mode.
  *           + Start/Stop operation functions in interrupt mode.
  *           + Reading operation functions.
  *           + Peripheral State functions.
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
      The LPTIM HAL driver can be used as follows:

      (#)Initialize the LPTIM low level resources by implementing the
        HAL_LPTIM_MspInit():
         (++) Enable the LPTIM interface clock using __HAL_RCC_LPTIMx_CLK_ENABLE().
         (++) In case of using interrupts (e.g. HAL_LPTIM_PWM_Start_IT()):
             (+++) Configure the LPTIM interrupt priority using HAL_NVIC_SetPriority().
             (+++) Enable the LPTIM IRQ handler using HAL_NVIC_EnableIRQ().
             (+++) In LPTIM IRQ handler, call HAL_LPTIM_IRQHandler().

      (#)Initialize the LPTIM HAL using HAL_LPTIM_Init(). This function
         configures mainly:
         (++) The instance: LPTIM1 or LPTIM2.
         (++) Clock: the counter clock.
             (+++) Source   : it can be either the ULPTIM input (IN1) or one of
                              the internal clock; (APB, LSE, LSI or MSI).
             (+++) Prescaler: select the clock divider.
         (++)  UltraLowPowerClock : To be used only if the ULPTIM is selected
               as counter clock source.
             (+++) Polarity:   polarity of the active edge for the counter unit
                               if the ULPTIM input is selected.
             (+++) SampleTime: clock sampling time to configure the clock glitch
                               filter.
         (++) Trigger: How the counter start.
             (+++) Source: trigger can be software or one of the hardware triggers.
             (+++) ActiveEdge : only for hardware trigger.
             (+++) SampleTime : trigger sampling time to configure the trigger
                                glitch filter.
         (++) OutputPolarity : 2 opposite polarities are possible.
         (++) UpdateMode: specifies whether the update of the autoreload and
              the compare values is done immediately or after the end of current
              period.
         (++) Input1Source: Source selected for input1 (GPIO or comparator output).
         (++) Input2Source: Source selected for input2 (GPIO or comparator output).
              Input2 is used only for encoder feature so is used only for LPTIM1 instance.

      (#)Six modes are available:

         (++) PWM Mode: To generate a PWM signal with specified period and pulse,
         call HAL_LPTIM_PWM_Start() or HAL_LPTIM_PWM_Start_IT() for interruption
         mode.

         (++) One Pulse Mode: To generate pulse with specified width in response
         to a stimulus, call HAL_LPTIM_OnePulse_Start() or
         HAL_LPTIM_OnePulse_Start_IT() for interruption mode.

         (++) Set once Mode: In this mode, the output changes the level (from
         low level to high level if the output polarity is configured high, else
         the opposite) when a compare match occurs. To start this mode, call
         HAL_LPTIM_SetOnce_Start() or HAL_LPTIM_SetOnce_Start_IT() for
         interruption mode.

         (++) Encoder Mode: To use the encoder interface call
         HAL_LPTIM_Encoder_Start() or HAL_LPTIM_Encoder_Start_IT() for
         interruption mode. Only available for LPTIM1 instance.

         (++) Time out Mode: an active edge on one selected trigger input rests
         the counter. The first trigger event will start the timer, any
         successive trigger event will reset the counter and the timer will
         restart. To start this mode call HAL_LPTIM_TimeOut_Start_IT() or
         HAL_LPTIM_TimeOut_Start_IT() for interruption mode.

         (++) Counter Mode: counter can be used to count external events on
         the LPTIM Input1 or it can be used to count internal clock cycles.
         To start this mode, call HAL_LPTIM_Counter_Start() or
         HAL_LPTIM_Counter_Start_IT() for interruption mode.


      (#) User can stop any process by calling the corresponding API:
          HAL_LPTIM_Xxx_Stop() or HAL_LPTIM_Xxx_Stop_IT() if the process is
          already started in interruption mode.

      (#) De-initialize the LPTIM peripheral using HAL_LPTIM_DeInit().

    *** Callback registration ***
  =============================================
  [..]
  The compilation define  USE_HAL_LPTIM_REGISTER_CALLBACKS when set to 1
  allows the user to configure dynamically the driver callbacks.
  [..]
  Use Function @ref HAL_LPTIM_RegisterCallback() to register a callback.
  @ref HAL_LPTIM_RegisterCallback() takes as parameters the HAL peripheral handle,
  the Callback ID and a pointer to the user callback function.
  [..]
  Use function @ref HAL_LPTIM_UnRegisterCallback() to reset a callback to the
  default weak function.
  @ref HAL_LPTIM_UnRegisterCallback takes as parameters the HAL peripheral handle,
  and the Callback ID.
  [..]
  These functions allow to register/unregister following callbacks:

    (+) MspInitCallback         : LPTIM Base Msp Init Callback.
    (+) MspDeInitCallback       : LPTIM Base Msp DeInit Callback.
    (+) CompareMatchCallback    : Compare match Callback.
    (+) AutoReloadMatchCallback : Auto-reload match Callback.
    (+) TriggerCallback         : External trigger event detection Callback.
    (+) CompareWriteCallback    : Compare register write complete Callback.
    (+) AutoReloadWriteCallback : Auto-reload register write complete Callback.
    (+) DirectionUpCallback     : Up-counting direction change Callback.
    (+) DirectionDownCallback   : Down-counting direction change Callback.

  [..]
  By default, after the Init and when the state is HAL_LPTIM_STATE_RESET
  all interrupt callbacks are set to the corresponding weak functions:
  examples @ref HAL_LPTIM_TriggerCallback(), @ref HAL_LPTIM_CompareMatchCallback().

  [..]
  Exception done for MspInit and MspDeInit functions that are reset to the legacy weak
  functionalities in the Init/DeInit only when these callbacks are null
  (not registered beforehand). If not, MspInit or MspDeInit are not null, the Init/DeInit
  keep and use the user MspInit/MspDeInit callbacks (registered beforehand)

  [..]
  Callbacks can be registered/unregistered in HAL_LPTIM_STATE_READY state only.
  Exception done MspInit/MspDeInit that can be registered/unregistered
  in HAL_LPTIM_STATE_READY or HAL_LPTIM_STATE_RESET state,
  thus registered (user) MspInit/DeInit callbacks can be used during the Init/DeInit.
  In that case first register the MspInit/MspDeInit user callbacks
  using @ref HAL_LPTIM_RegisterCallback() before calling DeInit or Init function.

  [..]
  When The compilation define USE_HAL_LPTIM_REGISTER_CALLBACKS is set to 0 or
  not defined, the callback registration feature is not available and all callbacks
  are set to the corresponding weak functions.

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

/** @defgroup LPTIM LPTIM
  * @brief LPTIM HAL module driver.
  * @{
  */

#ifdef HAL_LPTIM_MODULE_ENABLED

#if defined (LPTIM1) || defined (LPTIM2)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define TIMEOUT                                     1000UL /* Timeout is 1s */
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
static void LPTIM_ResetCallback(LPTIM_HandleTypeDef *lptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/* Exported functions --------------------------------------------------------*/

/** @defgroup LPTIM_Exported_Functions LPTIM Exported Functions
  * @{
  */

/** @defgroup LPTIM_Exported_Functions_Group1 Initialization/de-initialization functions
 *  @brief    Initialization and Configuration functions.
 *
@verbatim
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the LPTIM according to the specified parameters in the
          LPTIM_InitTypeDef and initialize the associated handle.
      (+) DeInitialize the LPTIM peripheral.
      (+) Initialize the LPTIM MSP.
      (+) DeInitialize the LPTIM MSP.

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the LPTIM according to the specified parameters in the
  *         LPTIM_InitTypeDef and initialize the associated handle.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Init(LPTIM_HandleTypeDef *hlptim)
{
  uint32_t tmpcfgr;

  /* Check the LPTIM handle allocation */
  if (hlptim == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  assert_param(IS_LPTIM_CLOCK_SOURCE(hlptim->Init.Clock.Source));
  assert_param(IS_LPTIM_CLOCK_PRESCALER(hlptim->Init.Clock.Prescaler));
  if ((hlptim->Init.Clock.Source) == LPTIM_CLOCKSOURCE_ULPTIM)
  {
    assert_param(IS_LPTIM_CLOCK_POLARITY(hlptim->Init.UltraLowPowerClock.Polarity));
    assert_param(IS_LPTIM_CLOCK_SAMPLE_TIME(hlptim->Init.UltraLowPowerClock.SampleTime));
  }
  assert_param(IS_LPTIM_TRG_SOURCE(hlptim->Init.Trigger.Source));
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    assert_param(IS_LPTIM_TRIG_SAMPLE_TIME(hlptim->Init.Trigger.SampleTime));
    assert_param(IS_LPTIM_EXT_TRG_POLARITY(hlptim->Init.Trigger.ActiveEdge));
  }
  assert_param(IS_LPTIM_OUTPUT_POLARITY(hlptim->Init.OutputPolarity));
  assert_param(IS_LPTIM_UPDATE_MODE(hlptim->Init.UpdateMode));
  assert_param(IS_LPTIM_COUNTER_SOURCE(hlptim->Init.CounterSource));

#if defined(LPTIM_RCR_REP)
  assert_param(IS_LPTIM_REPETITION(hlptim->Init.RepetitionCounter));
#endif

  if (hlptim->State == HAL_LPTIM_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hlptim->Lock = HAL_UNLOCKED;

#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
    /* Reset interrupt callbacks to legacy weak callbacks */
    LPTIM_ResetCallback(hlptim);

    if (hlptim->MspInitCallback == NULL)
    {
      hlptim->MspInitCallback = HAL_LPTIM_MspInit;
    }

    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    hlptim->MspInitCallback(hlptim);
#else
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    HAL_LPTIM_MspInit(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
  }

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Get the LPTIMx CFGR value */
  tmpcfgr = hlptim->Instance->CFGR;

  if (((hlptim->Init.Clock.Source) == LPTIM_CLOCKSOURCE_ULPTIM) || ((hlptim->Init.CounterSource) == LPTIM_COUNTERSOURCE_EXTERNAL))
  {
    tmpcfgr &= (uint32_t)(~(LPTIM_CFGR_CKPOL | LPTIM_CFGR_CKFLT));
  }
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    tmpcfgr &= (uint32_t)(~(LPTIM_CFGR_TRGFLT | LPTIM_CFGR_TRIGSEL));
  }

  /* Clear CKSEL, CKPOL, PRESC, TRIGEN, TRGFLT, WAVPOL, PRELOAD & COUNTMODE bits */
  tmpcfgr &= (uint32_t)(~(LPTIM_CFGR_CKSEL | LPTIM_CFGR_CKPOL | LPTIM_CFGR_TRIGEN | LPTIM_CFGR_PRELOAD |
                          LPTIM_CFGR_WAVPOL | LPTIM_CFGR_PRESC | LPTIM_CFGR_COUNTMODE));

  /* Set initialization parameters */
  tmpcfgr |= (hlptim->Init.Clock.Source    |
              hlptim->Init.Clock.Prescaler |
              hlptim->Init.OutputPolarity  |
              hlptim->Init.UpdateMode      |
              hlptim->Init.CounterSource);

  if (((hlptim->Init.Clock.Source) == LPTIM_CLOCKSOURCE_ULPTIM) || ((hlptim->Init.CounterSource) == LPTIM_COUNTERSOURCE_EXTERNAL))
  {
    tmpcfgr |= (hlptim->Init.UltraLowPowerClock.Polarity |
                hlptim->Init.UltraLowPowerClock.SampleTime);
  }

  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable External trigger and set the trigger source */
    tmpcfgr |= (hlptim->Init.Trigger.Source     |
                hlptim->Init.Trigger.ActiveEdge |
                hlptim->Init.Trigger.SampleTime);
  }

  /* Write to LPTIMx CFGR */
  hlptim->Instance->CFGR = tmpcfgr;

  /* Configure LPTIM input sources */
  if (hlptim->Instance == LPTIM1)
  {
    /* Check LPTIM Input1 and Input2 sources */
    assert_param(IS_LPTIM_INPUT1_SOURCE(hlptim->Instance, hlptim->Init.Input1Source));
    assert_param(IS_LPTIM_INPUT2_SOURCE(hlptim->Instance, hlptim->Init.Input2Source));

    /* Configure LPTIM Input1 and Input2 sources */
    hlptim->Instance->OR = (hlptim->Init.Input1Source | hlptim->Init.Input2Source);
  }
  else
  {
    /* Check LPTIM2 Input1 source */
    assert_param(IS_LPTIM_INPUT1_SOURCE(hlptim->Instance, hlptim->Init.Input1Source));

    /* Configure LPTIM2 Input1 source */
    hlptim->Instance->OR = hlptim->Init.Input1Source;
  }

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  DeInitialize the LPTIM peripheral.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_DeInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the LPTIM handle allocation */
  if (hlptim == NULL)
  {
    return HAL_ERROR;
  }

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the LPTIM Peripheral Clock */
  __HAL_LPTIM_DISABLE(hlptim);

#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
  if (hlptim->MspDeInitCallback == NULL)
  {
    hlptim->MspDeInitCallback = HAL_LPTIM_MspDeInit;
  }

  /* DeInit the low level hardware: CLOCK, NVIC.*/
  hlptim->MspDeInitCallback(hlptim);
#else
  /* DeInit the low level hardware: CLOCK, NVIC.*/
  HAL_LPTIM_MspDeInit(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hlptim);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the LPTIM MSP.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitialize LPTIM MSP.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_MspDeInit could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup LPTIM_Exported_Functions_Group2 LPTIM Start-Stop operation functions
 *  @brief   Start-Stop operation functions.
 *
@verbatim
  ==============================================================================
                ##### LPTIM Start Stop operation functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Start the PWM mode.
      (+) Stop the PWM mode.
      (+) Start the One pulse mode.
      (+) Stop the One pulse mode.
      (+) Start the Set once mode.
      (+) Stop the Set once mode.
      (+) Start the Encoder mode.
      (+) Stop the Encoder mode.
      (+) Start the Timeout mode.
      (+) Stop the Timeout mode.
      (+) Start the Counter mode.
      (+) Stop the Counter mode.


@endverbatim
  * @{
  */

/**
  * @brief  Start the LPTIM PWM generation.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Reset WAVE bit to set PWM mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);

#if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM PWM generation.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM PWM generation in interrupt mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF
  * @param  Pulse Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Reset WAVE bit to set PWM mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;

  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Enable Compare write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPOK);

  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* If external trigger source is used, then enable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable external trigger interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }

#if defined(LPTIM_RCR_REP)
  /* Enable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_ENABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);

#if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM PWM generation in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Disable Compare write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPOK);

  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* If external trigger source is used, then disable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Disable external trigger interrupt */
    __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }

#if defined(LPTIM_RCR_REP)
  /* Disable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_DISABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM One pulse generation.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Reset WAVE bit to set one pulse mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);

  /* Start timer in single (one shot) mode */
  __HAL_LPTIM_START_SINGLE(hlptim);

#if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM One pulse generation.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM One pulse generation in interrupt mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Reset WAVE bit to set one pulse mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;

  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Enable Compare write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPOK);

  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* If external trigger source is used, then enable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable external trigger interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }

#if defined(LPTIM_RCR_REP)
  /* Enable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_ENABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);

#if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_SINGLE(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM One pulse generation in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Disable Compare write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPOK);

  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* If external trigger source is used, then disable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Disable external trigger interrupt */
    __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }

#if defined(LPTIM_RCR_REP)
  /* Disable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_DISABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM in Set once mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Set WAVE bit to enable the set once mode */
  hlptim->Instance->CFGR |= LPTIM_CFGR_WAVE;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);

#if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_SINGLE(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set once mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM Set once mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Set WAVE bit to enable the set once mode */
  hlptim->Instance->CFGR |= LPTIM_CFGR_WAVE;

  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Enable Compare write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPOK);

  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* If external trigger source is used, then enable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable external trigger interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }

#if defined(LPTIM_RCR_REP)
  /* Enable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_ENABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);

#if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_SINGLE(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set once mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Disable Compare write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPOK);

  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* If external trigger source is used, then disable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) != LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Disable external trigger interrupt */
    __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }

#if defined(LPTIM_RCR_REP)
  /* Disable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_DISABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Encoder interface.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  uint32_t          tmpcfgr;

  /* Check the parameters */
  assert_param(IS_LPTIM_ENCODER_INTERFACE_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(hlptim->Init.Clock.Source == LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC);
  assert_param(hlptim->Init.Clock.Prescaler == LPTIM_PRESCALER_DIV1);
  assert_param(IS_LPTIM_CLOCK_POLARITY(hlptim->Init.UltraLowPowerClock.Polarity));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Get the LPTIMx CFGR value */
  tmpcfgr = hlptim->Instance->CFGR;

  /* Clear CKPOL bits */
  tmpcfgr &= (uint32_t)(~LPTIM_CFGR_CKPOL);

  /* Set Input polarity */
  tmpcfgr |=  hlptim->Init.UltraLowPowerClock.Polarity;

  /* Write to LPTIMx CFGR */
  hlptim->Instance->CFGR = tmpcfgr;

  /* Set ENC bit to enable the encoder interface */
  hlptim->Instance->CFGR |= LPTIM_CFGR_ENC;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Encoder interface.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_ENCODER_INTERFACE_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Reset ENC bit to disable the encoder interface */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_ENC;

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Encoder interface in interrupt mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  uint32_t          tmpcfgr;

  /* Check the parameters */
  assert_param(IS_LPTIM_ENCODER_INTERFACE_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(hlptim->Init.Clock.Source == LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC);
  assert_param(hlptim->Init.Clock.Prescaler == LPTIM_PRESCALER_DIV1);
  assert_param(IS_LPTIM_CLOCK_POLARITY(hlptim->Init.UltraLowPowerClock.Polarity));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Configure edge sensitivity for encoder mode */
  /* Get the LPTIMx CFGR value */
  tmpcfgr = hlptim->Instance->CFGR;

  /* Clear CKPOL bits */
  tmpcfgr &= (uint32_t)(~LPTIM_CFGR_CKPOL);

  /* Set Input polarity */
  tmpcfgr |=  hlptim->Init.UltraLowPowerClock.Polarity;

  /* Write to LPTIMx CFGR */
  hlptim->Instance->CFGR = tmpcfgr;

  /* Set ENC bit to enable the encoder interface */
  hlptim->Instance->CFGR |= LPTIM_CFGR_ENC;

  /* Enable "switch to down direction" interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_DOWN);

  /* Enable "switch to up direction" interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_UP);

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Encoder interface in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_ENCODER_INTERFACE_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Reset ENC bit to disable the encoder interface */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_ENC;

  /* Disable "switch to down direction" interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_DOWN);

  /* Disable "switch to up direction" interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_UP);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Timeout function.
  * @note   The first trigger event will start the timer, any successive
  *         trigger event will reset the counter and the timer restarts.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Timeout Specifies the TimeOut value to reset the counter.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Timeout)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Timeout));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Set TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR |= LPTIM_CFGR_TIMOUT;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the Timeout value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Timeout);

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Timeout function.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Reset TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_TIMOUT;

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Timeout function in interrupt mode.
  * @note   The first trigger event will start the timer, any successive
  *         trigger event will reset the counter and the timer restarts.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Timeout Specifies the TimeOut value to reset the counter.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Timeout)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Timeout));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Enable EXTI Line interrupt on the LPTIM Wake-up Timer */
  __HAL_LPTIM_WAKEUPTIMER_EXTI_ENABLE_IT();

  /* Set TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR |= LPTIM_CFGR_TIMOUT;

  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Load the Timeout value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Timeout);

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Timeout function in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable EXTI Line interrupt on the LPTIM Wake-up Timer */
  __HAL_LPTIM_WAKEUPTIMER_EXTI_DISABLE_IT();

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Reset TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_TIMOUT;

  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Counter mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* If clock source is not ULPTIM clock and counter source is external, then it must not be prescaled */
  if ((hlptim->Init.Clock.Source != LPTIM_CLOCKSOURCE_ULPTIM) && (hlptim->Init.CounterSource == LPTIM_COUNTERSOURCE_EXTERNAL))
  {
    /* Check if clock is prescaled */
    assert_param(IS_LPTIM_CLOCK_PRESCALERDIV1(hlptim->Init.Clock.Prescaler));
    /* Set clock prescaler to 0 */
    hlptim->Instance->CFGR &= ~LPTIM_CFGR_PRESC;
  }

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

#if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Counter mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Counter mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Enable EXTI Line interrupt on the LPTIM Wake-up Timer */
  __HAL_LPTIM_WAKEUPTIMER_EXTI_ENABLE_IT();

  /* If clock source is not ULPTIM clock and counter source is external, then it must not be prescaled */
  if ((hlptim->Init.Clock.Source != LPTIM_CLOCKSOURCE_ULPTIM) && (hlptim->Init.CounterSource == LPTIM_COUNTERSOURCE_EXTERNAL))
  {
    /* Check if clock is prescaled */
    assert_param(IS_LPTIM_CLOCK_PRESCALERDIV1(hlptim->Init.Clock.Prescaler));
    /* Set clock prescaler to 0 */
    hlptim->Instance->CFGR &= ~LPTIM_CFGR_PRESC;
  }

  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);

#if defined(LPTIM_RCR_REP)
  /* Enable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_ENABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

 #if defined(LPTIM_RCR_REP)
  /* Load the repetition value in the repetition counter */
  if (hlptim->Init.RepetitionCounter != 0)
  {
    __HAL_LPTIM_REPETITIONCOUNTER_SET(hlptim, hlptim->Init.RepetitionCounter);
  }
#endif

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Counter mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable EXTI Line interrupt on the LPTIM Wake-up Timer */
  __HAL_LPTIM_WAKEUPTIMER_EXTI_DISABLE_IT();

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);

  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);

#if defined(LPTIM_RCR_REP)
  /* Disable the update event and the repetition register update OK interrupts */
  if ((hlptim->Init.RepetitionCounter) !=  0)
  {
    __HAL_LPTIM_DISABLE_IT(hlptim, (LPTIM_IT_UPDATE | LPTIM_IT_REPOK));
  }
#endif

  /* Change the TIM state*/
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup LPTIM_Exported_Functions_Group3 LPTIM Read operation functions
 *  @brief  Read operation functions.
 *
@verbatim
  ==============================================================================
                  ##### LPTIM Read operation functions #####
  ==============================================================================
[..]  This section provides LPTIM Reading functions.
      (+) Read the counter value.
      (+) Read the period (Auto-reload) value.
      (+) Read the pulse (Compare)value.
@endverbatim
  * @{
  */

/**
  * @brief  Return the current counter value.
  * @param  hlptim LPTIM handle
  * @retval Counter value.
  */
uint32_t HAL_LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  return (hlptim->Instance->CNT);
}

/**
  * @brief  Return the current Autoreload (Period) value.
  * @param  hlptim LPTIM handle
  * @retval Autoreload value.
  */
uint32_t HAL_LPTIM_ReadAutoReload(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  return (hlptim->Instance->ARR);
}

/**
  * @brief  Return the current Compare (Pulse) value.
  * @param  hlptim LPTIM handle
  * @retval Compare value.
  */
uint32_t HAL_LPTIM_ReadCompare(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  return (hlptim->Instance->CMP);
}

/**
  * @}
  */

/** @defgroup LPTIM_Exported_Functions_Group4 LPTIM IRQ handler and callbacks
 *  @brief  LPTIM  IRQ handler.
 *
@verbatim
  ==============================================================================
                      ##### LPTIM IRQ handler and callbacks  #####
  ==============================================================================
[..]  This section provides LPTIM IRQ handler and callback functions called within
      the IRQ handler:
   (+) LPTIM interrupt request handler
   (+) Compare match Callback
   (+) Auto-reload match Callback
   (+) External trigger event detection Callback
   (+) Compare register write complete Callback
   (+) Auto-reload register write complete Callback
   (+) Up-counting direction change Callback
   (+) Down-counting direction change Callback

@endverbatim
  * @{
  */

/**
  * @brief  Handle LPTIM interrupt request.
  * @param  hlptim LPTIM handle
  * @retval None
  */
void HAL_LPTIM_IRQHandler(LPTIM_HandleTypeDef *hlptim)
{
  /* Compare match interrupt */
  if (__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_CMPM) != RESET)
  {
    if (__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_CMPM) != RESET)
    {
      /* Clear Compare match flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_CMPM);

      /* Compare match Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->CompareMatchCallback(hlptim);
#else
      HAL_LPTIM_CompareMatchCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }

  /* Autoreload match interrupt */
  if (__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_ARRM) != RESET)
  {
    if (__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_ARRM) != RESET)
    {
      /* Clear Autoreload match flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_ARRM);

      /* Autoreload match Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->AutoReloadMatchCallback(hlptim);
#else
      HAL_LPTIM_AutoReloadMatchCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }

  /* Trigger detected interrupt */
  if (__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_EXTTRIG) != RESET)
  {
    if (__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_EXTTRIG) != RESET)
    {
      /* Clear Trigger detected flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_EXTTRIG);

      /* Trigger detected callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->TriggerCallback(hlptim);
#else
      HAL_LPTIM_TriggerCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }

  /* Compare write interrupt */
  if (__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_CMPOK) != RESET)
  {
    if (__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_CMPOK) != RESET)
    {
      /* Clear Compare write flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_CMPOK);

      /* Compare write Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->CompareWriteCallback(hlptim);
#else
      HAL_LPTIM_CompareWriteCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }

  /* Autoreload write interrupt */
  if (__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_ARROK) != RESET)
  {
    if (__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_ARROK) != RESET)
    {
      /* Clear Autoreload write flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_ARROK);

      /* Autoreload write Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->AutoReloadWriteCallback(hlptim);
#else
      HAL_LPTIM_AutoReloadWriteCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }

  /* Direction counter changed from Down to Up interrupt */
  if (__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_UP) != RESET)
  {
    if (__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_UP) != RESET)
    {
      /* Clear Direction counter changed from Down to Up flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_UP);

      /* Direction counter changed from Down to Up Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->DirectionUpCallback(hlptim);
#else
      HAL_LPTIM_DirectionUpCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }

  /* Direction counter changed from Up to Down interrupt */
  if (__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_DOWN) != RESET)
  {
    if (__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_DOWN) != RESET)
    {
      /* Clear Direction counter changed from Up to Down flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_DOWN);

      /* Direction counter changed from Up to Down Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->DirectionDownCallback(hlptim);
#else
      HAL_LPTIM_DirectionDownCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }

#if defined(LPTIM_RCR_REP)
  /* Repetition counter underflowed (or contains zero) and the LPTIM counter
     overflowed */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_UPDATE) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_UPDATE) != RESET)
    {
      /* Clear update event flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_UPDATE);

      /* Update event Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->UpdateEventCallback(hlptim);
#else
      HAL_LPTIM_UpdateEventCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */      
    }
  }

  /* Successful APB bus write to repetition counter register */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_REPOK) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_REPOK) != RESET)
    {
      /* Clear successful APB bus write to repetition counter flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_REPOK);

      /* Successful APB bus write to repetition counter Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->RepCounterWriteCallback(hlptim);
#else
      HAL_LPTIM_RepCounterWriteCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */      
    }
  }
#endif
}

/**
  * @brief  Compare match callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_CompareMatchCallback could be implemented in the user file
   */
}

/**
  * @brief  Autoreload match callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_AutoReloadMatchCallback could be implemented in the user file
   */
}

/**
  * @brief  Trigger detected callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_TriggerCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_TriggerCallback could be implemented in the user file
   */
}

/**
  * @brief  Compare write callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_CompareWriteCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_CompareWriteCallback could be implemented in the user file
   */
}

/**
  * @brief  Autoreload write callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_AutoReloadWriteCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_AutoReloadWriteCallback could be implemented in the user file
   */
}

/**
  * @brief  Direction counter changed from Down to Up callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_DirectionUpCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_DirectionUpCallback could be implemented in the user file
   */
}

/**
  * @brief  Direction counter changed from Up to Down callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_DirectionDownCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_DirectionDownCallback could be implemented in the user file
   */
}

#if defined(LPTIM_RCR_REP)
/**
  * @brief Repetition counter underflowed (or contains zero) and LPTIM counter overflowed callback in non-blocking mode.
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_UpdateEventCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_UpdateEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Successful APB bus write to repetition counter register callback in non-blocking mode.
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_RepCounterWriteCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_RepCounterWriteCallback could be implemented in the user file
   */
}
#endif /* LPTIM_RCR_REP */

#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User LPTIM callback to be used instead of the weak predefined callback
  * @param hlptim LPTIM handle
  * @param CallbackID ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_LPTIM_MSPINIT_CB_ID          LPTIM Base Msp Init Callback ID
  *          @arg @ref HAL_LPTIM_MSPDEINIT_CB_ID        LPTIM Base Msp DeInit Callback ID
  *          @arg @ref HAL_LPTIM_COMPARE_MATCH_CB_ID    Compare match Callback ID
  *          @arg @ref HAL_LPTIM_AUTORELOAD_MATCH_CB_ID Auto-reload match Callback ID
  *          @arg @ref HAL_LPTIM_TRIGGER_CB_ID          External trigger event detection Callback ID
  *          @arg @ref HAL_LPTIM_COMPARE_WRITE_CB_ID    Compare register write complete Callback ID
  *          @arg @ref HAL_LPTIM_AUTORELOAD_WRITE_CB_ID Auto-reload register write complete Callback ID
  *          @arg @ref HAL_LPTIM_DIRECTION_UP_CB_ID     Up-counting direction change Callback ID
  *          @arg @ref HAL_LPTIM_DIRECTION_DOWN_CB_ID   Down-counting direction change Callback ID
  * @param pCallback pointer to the callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_LPTIM_RegisterCallback(LPTIM_HandleTypeDef        *hlptim,
                                             HAL_LPTIM_CallbackIDTypeDef CallbackID,
                                             pLPTIM_CallbackTypeDef      pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hlptim);

  if (hlptim->State == HAL_LPTIM_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_LPTIM_MSPINIT_CB_ID :
        hlptim->MspInitCallback = pCallback;
        break;

      case HAL_LPTIM_MSPDEINIT_CB_ID :
        hlptim->MspDeInitCallback = pCallback;
        break;

      case HAL_LPTIM_COMPARE_MATCH_CB_ID :
        hlptim->CompareMatchCallback = pCallback;
        break;

      case HAL_LPTIM_AUTORELOAD_MATCH_CB_ID :
        hlptim->AutoReloadMatchCallback = pCallback;
        break;

      case HAL_LPTIM_TRIGGER_CB_ID :
        hlptim->TriggerCallback = pCallback;
        break;

      case HAL_LPTIM_COMPARE_WRITE_CB_ID :
        hlptim->CompareWriteCallback = pCallback;
        break;

      case HAL_LPTIM_AUTORELOAD_WRITE_CB_ID :
        hlptim->AutoReloadWriteCallback = pCallback;
        break;

      case HAL_LPTIM_DIRECTION_UP_CB_ID :
        hlptim->DirectionUpCallback = pCallback;
        break;

      case HAL_LPTIM_DIRECTION_DOWN_CB_ID :
        hlptim->DirectionDownCallback = pCallback;
        break;

#if defined(LPTIM_RCR_REP)
      case HAL_LPTIM_UPDATE_EVENT_CB_ID :
        hlptim->UpdateEventCallback = pCallback;
        break;

      case HAL_LPTIM_REPETITION_WRITE_CB_ID :
        hlptim->RepCounterWriteCallback = pCallback;
        break;
#endif /* LPTIM_RCR_REP */

      default :
        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (hlptim->State == HAL_LPTIM_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_LPTIM_MSPINIT_CB_ID :
        hlptim->MspInitCallback = pCallback;
        break;

      case HAL_LPTIM_MSPDEINIT_CB_ID :
        hlptim->MspDeInitCallback = pCallback;
        break;

      default :
        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hlptim);

  return status;
}

/**
  * @brief  Unregister a LPTIM callback
  *         LLPTIM callback is redirected to the weak predefined callback
  * @param hlptim LPTIM handle
  * @param CallbackID ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_LPTIM_MSPINIT_CB_ID          LPTIM Base Msp Init Callback ID
  *          @arg @ref HAL_LPTIM_MSPDEINIT_CB_ID        LPTIM Base Msp DeInit Callback ID
  *          @arg @ref HAL_LPTIM_COMPARE_MATCH_CB_ID    Compare match Callback ID
  *          @arg @ref HAL_LPTIM_AUTORELOAD_MATCH_CB_ID Auto-reload match Callback ID
  *          @arg @ref HAL_LPTIM_TRIGGER_CB_ID          External trigger event detection Callback ID
  *          @arg @ref HAL_LPTIM_COMPARE_WRITE_CB_ID    Compare register write complete Callback ID
  *          @arg @ref HAL_LPTIM_AUTORELOAD_WRITE_CB_ID Auto-reload register write complete Callback ID
  *          @arg @ref HAL_LPTIM_DIRECTION_UP_CB_ID     Up-counting direction change Callback ID
  *          @arg @ref HAL_LPTIM_DIRECTION_DOWN_CB_ID   Down-counting direction change Callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_LPTIM_UnRegisterCallback(LPTIM_HandleTypeDef        *hlptim,
                                               HAL_LPTIM_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hlptim);

  if (hlptim->State == HAL_LPTIM_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_LPTIM_MSPINIT_CB_ID :
        hlptim->MspInitCallback = HAL_LPTIM_MspInit;                          /* Legacy weak MspInit Callback */
        break;

      case HAL_LPTIM_MSPDEINIT_CB_ID :
        hlptim->MspDeInitCallback = HAL_LPTIM_MspDeInit;                       /* Legacy weak Msp DeInit Callback */
        break;

      case HAL_LPTIM_COMPARE_MATCH_CB_ID :
        hlptim->CompareMatchCallback = HAL_LPTIM_CompareMatchCallback;         /* Legacy weak Compare match Callback */
        break;

      case HAL_LPTIM_AUTORELOAD_MATCH_CB_ID :
        hlptim->AutoReloadMatchCallback = HAL_LPTIM_AutoReloadMatchCallback;   /* Legacy weak Auto-reload match Callback */
        break;

      case HAL_LPTIM_TRIGGER_CB_ID :
        hlptim->TriggerCallback = HAL_LPTIM_TriggerCallback;                   /* Legacy weak External trigger event detection Callback */
        break;

      case HAL_LPTIM_COMPARE_WRITE_CB_ID :
        hlptim->CompareWriteCallback = HAL_LPTIM_CompareWriteCallback;         /* Legacy weak Compare register write complete Callback */
        break;

      case HAL_LPTIM_AUTORELOAD_WRITE_CB_ID :
        hlptim->AutoReloadWriteCallback = HAL_LPTIM_AutoReloadWriteCallback;   /* Legacy weak Auto-reload register write complete Callback */
        break;

      case HAL_LPTIM_DIRECTION_UP_CB_ID :
        hlptim->DirectionUpCallback = HAL_LPTIM_DirectionUpCallback;           /* Legacy weak Up-counting direction change Callback */
        break;

      case HAL_LPTIM_DIRECTION_DOWN_CB_ID :
        hlptim->DirectionDownCallback = HAL_LPTIM_DirectionDownCallback;       /* Legacy weak Down-counting direction change Callback */
        break;

#if defined(LPTIM_RCR_REP)
      case HAL_LPTIM_UPDATE_EVENT_CB_ID :
        hlptim->UpdateEventCallback = HAL_LPTIM_UpdateEventCallback;           /* Legacy weak Update Event Callback */
        break;

      case HAL_LPTIM_REPETITION_WRITE_CB_ID :
        hlptim->RepCounterWriteCallback = HAL_LPTIM_RepCounterWriteCallback;   /* Legacy weak Repetition counter successful write Callback */
        break;
#endif /* LPTIM_RCR_REP */

      default :
        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (hlptim->State == HAL_LPTIM_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_LPTIM_MSPINIT_CB_ID :
        hlptim->MspInitCallback = HAL_LPTIM_MspInit;                           /* Legacy weak MspInit Callback */
        break;

      case HAL_LPTIM_MSPDEINIT_CB_ID :
        hlptim->MspDeInitCallback = HAL_LPTIM_MspDeInit;                        /* Legacy weak Msp DeInit Callback */
        break;

      default :
        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hlptim);

  return status;
}
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup LPTIM_Group5 Peripheral State functions
 *  @brief   Peripheral State functions.
 *
@verbatim
  ==============================================================================
                      ##### Peripheral State functions #####
  ==============================================================================
    [..]
    This subsection permits to get in run-time the status of the peripheral.

@endverbatim
  * @{
  */

/**
  * @brief  Return the LPTIM handle state.
  * @param  hlptim LPTIM handle
  * @retval HAL state
  */
HAL_LPTIM_StateTypeDef HAL_LPTIM_GetState(LPTIM_HandleTypeDef *hlptim)
{
  /* Return LPTIM handle state */
  return hlptim->State;
}

/**
  * @}
  */


/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/** @defgroup LPTIM_Private_Functions LPTIM Private Functions
  * @{
  */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
/**
  * @brief  Reset interrupt callbacks to the legacy weak callbacks.
  * @param  lptim pointer to a LPTIM_HandleTypeDef structure that contains
  *                the configuration information for LPTIM module.
  * @retval None
  */
static void LPTIM_ResetCallback(LPTIM_HandleTypeDef *lptim)
{
  /* Reset the LPTIM callback to the legacy weak callbacks */
  lptim->CompareMatchCallback    = HAL_LPTIM_CompareMatchCallback;    /* Compare match Callback                       */
  lptim->AutoReloadMatchCallback = HAL_LPTIM_AutoReloadMatchCallback; /* Auto-reload match Callback                   */
  lptim->TriggerCallback         = HAL_LPTIM_TriggerCallback;         /* External trigger event detection Callback    */
  lptim->CompareWriteCallback    = HAL_LPTIM_CompareWriteCallback;    /* Compare register write complete Callback     */
  lptim->AutoReloadWriteCallback = HAL_LPTIM_AutoReloadWriteCallback; /* Auto-reload register write complete Callback */
  lptim->DirectionUpCallback     = HAL_LPTIM_DirectionUpCallback;     /* Up-counting direction change Callback        */
  lptim->DirectionDownCallback   = HAL_LPTIM_DirectionDownCallback;   /* Down-counting direction change Callback      */
#if defined(LPTIM_RCR_REP)
  lptim->UpdateEventCallback     = HAL_LPTIM_UpdateEventCallback;     /* Update Event Callback                        */
  lptim->RepCounterWriteCallback = HAL_LPTIM_RepCounterWriteCallback; /* Repetition counter successful write Callback */
#endif /* LPTIM_RCR_REP */
}
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/**
  * @brief  Disable LPTIM HW instance.
  * @param  lptim pointer to a LPTIM_HandleTypeDef structure that contains
  *                the configuration information for LPTIM module.
  * @note   The following sequence is required to solve LPTIM disable HW limitation.
  *         Please check Errata Sheet ES0335 for more details under "MCU may remain
  *         stuck in LPTIM interrupt when entering Stop mode" section.
  * @retval None
  */
void LPTIM_Disable(LPTIM_HandleTypeDef *lptim)
{
  uint32_t tmpclksource = 0;
  uint32_t tmpIER;
  uint32_t tmpCFGR;
  uint32_t tmpCMP;
  uint32_t tmpARR;
  uint32_t tmpOR;

  __disable_irq();

  /*********** Save LPTIM Config ***********/
  /* Save LPTIM source clock */
  switch ((uint32_t)lptim->Instance)
  {
     case LPTIM1_BASE:
       tmpclksource = __HAL_RCC_GET_LPTIM1_SOURCE();
       break;
#if defined(LPTIM2)
     case LPTIM2_BASE:
       tmpclksource = __HAL_RCC_GET_LPTIM2_SOURCE();
       break;
#endif /* LPTIM2 */
     default:
       break;
  }

  /* Save LPTIM configuration registers */
  tmpIER = lptim->Instance->IER;
  tmpCFGR = lptim->Instance->CFGR;
  tmpCMP = lptim->Instance->CMP;
  tmpARR = lptim->Instance->ARR;
  tmpOR = lptim->Instance->OR;

  /*********** Reset LPTIM ***********/
  switch ((uint32_t)lptim->Instance)
  {
     case LPTIM1_BASE:
       __HAL_RCC_LPTIM1_FORCE_RESET();
       __HAL_RCC_LPTIM1_RELEASE_RESET();
       break;
#if defined(LPTIM2)
     case LPTIM2_BASE:
       __HAL_RCC_LPTIM2_FORCE_RESET();
       __HAL_RCC_LPTIM2_RELEASE_RESET();
       break;
#endif /* LPTIM2 */
     default:
       break;
  }

  /*********** Restore LPTIM Config ***********/
  uint32_t Ref_Time;
  uint32_t Time_Elapsed;

  if ((tmpCMP != 0UL) || (tmpARR != 0UL))
  {
    /* Force LPTIM source kernel clock from APB */
    switch ((uint32_t)lptim->Instance)
    {
       case LPTIM1_BASE:
         __HAL_RCC_LPTIM1_CONFIG(RCC_LPTIM1CLKSOURCE_PCLK1);
         break;
#if defined(LPTIM2)
       case LPTIM2_BASE:
         __HAL_RCC_LPTIM2_CONFIG(RCC_LPTIM2CLKSOURCE_PCLK1);
         break;
#endif /* LPTIM2 */
       default:
         break;
    }

    if (tmpCMP != 0UL)
    {
      /* Restore CMP register (LPTIM should be enabled first) */
      lptim->Instance->CR |= LPTIM_CR_ENABLE;
      lptim->Instance->CMP = tmpCMP;
      /* Polling on CMP write ok status after above restore operation */
      Ref_Time = HAL_GetTick();
      do
      {
        Time_Elapsed = HAL_GetTick() - Ref_Time;
      } while ((!(__HAL_LPTIM_GET_FLAG(lptim, LPTIM_FLAG_CMPOK))) && (Time_Elapsed <= TIMEOUT));

      __HAL_LPTIM_CLEAR_FLAG(lptim, LPTIM_FLAG_CMPOK);
    }

    if (tmpARR != 0UL)
    {
      /* Restore ARR register (LPTIM should be enabled first) */
      lptim->Instance->CR |= LPTIM_CR_ENABLE;
      lptim->Instance->ARR = tmpARR;
      /* Polling on ARR write ok status after above restore operation */
      Ref_Time = HAL_GetTick();
      do
      {
        Time_Elapsed = HAL_GetTick() - Ref_Time;
      } while ((!(__HAL_LPTIM_GET_FLAG(lptim, LPTIM_FLAG_ARROK))) && (Time_Elapsed <= TIMEOUT));

      __HAL_LPTIM_CLEAR_FLAG(lptim, LPTIM_FLAG_ARROK);
    }

    /* Restore LPTIM source kernel clock */
    switch ((uint32_t)lptim->Instance)
    {
       case LPTIM1_BASE:
         __HAL_RCC_LPTIM1_CONFIG(tmpclksource);
         break;
#if defined(LPTIM2)
       case LPTIM2_BASE:
         __HAL_RCC_LPTIM2_CONFIG(tmpclksource);
         break;
#endif /* LPTIM2 */
       default:
         break;
    }
  }

  /* Restore configuration registers (LPTIM should be disabled first) */
  lptim->Instance->CR &= ~(LPTIM_CR_ENABLE);
  lptim->Instance->IER = tmpIER;
  lptim->Instance->CFGR = tmpCFGR;
  lptim->Instance->OR = tmpOR;

  __enable_irq();
}
/**
  * @}
  */
#endif /* LPTIM1 || LPTIM2 */

#endif /* HAL_LPTIM_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
