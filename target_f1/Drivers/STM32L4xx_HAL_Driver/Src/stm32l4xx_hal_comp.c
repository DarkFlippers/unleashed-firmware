/**
  ******************************************************************************
  * @file    stm32l4xx_hal_comp.c
  * @author  MCD Application Team
  * @brief   COMP HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the COMP peripheral:
  *           + Initialization and de-initialization functions
  *           + Start/Stop operation functions in polling mode
  *           + Start/Stop operation functions in interrupt mode (through EXTI interrupt)
  *           + Peripheral control functions
  *           + Peripheral state functions
  *
  @verbatim
================================================================================
          ##### COMP Peripheral features #####
================================================================================

  [..]
      The STM32L4xx device family integrates two analog comparators instances:
      COMP1, COMP2 except for the STM32L412xx/STM32L422xx products that embed only
      one: COMP1.
      In the rest of the file, all comments related to a pair of comparators are not
      applicable to STM32L412xx or STM32L422xx.
      (#) Comparators input minus (inverting input) and input plus (non inverting input)
          can be set to internal references or to GPIO pins
          (refer to GPIO list in reference manual).

      (#) Comparators output level is available using HAL_COMP_GetOutputLevel()
          and can be redirected to other peripherals: GPIO pins (in mode
          alternate functions for comparator), timers.
          (refer to GPIO list in reference manual).

      (#) The comparators have interrupt capability through the EXTI controller
          with wake-up from sleep and stop modes.

      (#) Pairs of comparators instances can be combined in window mode
          (2 consecutive instances odd and even COMP<x> and COMP<x+1>).

          From the corresponding IRQ handler, the right interrupt source can be retrieved
          using macro __HAL_COMP_COMPx_EXTI_GET_FLAG().

            ##### How to use this driver #####
================================================================================
  [..]
      This driver provides functions to configure and program the comparator instances
      of STM32L4xx devices.

      To use the comparator, perform the following steps:

      (#)  Initialize the COMP low level resources by implementing the HAL_COMP_MspInit():
      (++) Configure the GPIO connected to comparator inputs plus and minus in analog mode
           using HAL_GPIO_Init().
      (++) If needed, configure the GPIO connected to comparator output in alternate function mode
           using HAL_GPIO_Init().
      (++) If required enable the COMP interrupt by configuring and enabling EXTI line in Interrupt mode and
           selecting the desired sensitivity level using HAL_GPIO_Init() function. After that enable the comparator
           interrupt vector using HAL_NVIC_EnableIRQ() function.

      (#) Configure the comparator using HAL_COMP_Init() function:
      (++) Select the input minus (inverting input)
      (++) Select the input plus (non-inverting input)
      (++) Select the hysteresis
      (++) Select the blanking source
      (++) Select the output polarity
      (++) Select the power mode
      (++) Select the window mode

      -@@- HAL_COMP_Init() calls internally __HAL_RCC_SYSCFG_CLK_ENABLE()
          to enable internal control clock of the comparators.
          However, this is a legacy strategy. In future STM32 families,
          COMP clock enable must be implemented by user in "HAL_COMP_MspInit()".
          Therefore, for compatibility anticipation, it is recommended to
          implement __HAL_RCC_SYSCFG_CLK_ENABLE() in "HAL_COMP_MspInit()".

      (#) Reconfiguration on-the-fly of comparator can be done by calling again
          function HAL_COMP_Init() with new input structure parameters values.

      (#) Enable the comparator using HAL_COMP_Start() function.

      (#) Use HAL_COMP_TriggerCallback() or HAL_COMP_GetOutputLevel() functions
          to manage comparator outputs (events and output level).

      (#) Disable the comparator using HAL_COMP_Stop() function.

      (#) De-initialize the comparator using HAL_COMP_DeInit() function.

      (#) For safety purpose, comparator configuration can be locked using HAL_COMP_Lock() function.
          The only way to unlock the comparator is a device hardware reset.

    *** Callback registration ***
    =============================================
    [..]

     The compilation flag USE_HAL_COMP_REGISTER_CALLBACKS, when set to 1,
     allows the user to configure dynamically the driver callbacks.
     Use Functions @ref HAL_COMP_RegisterCallback()
     to register an interrupt callback.
    [..]

     Function @ref HAL_COMP_RegisterCallback() allows to register following callbacks:
       (+) TriggerCallback       : callback for COMP trigger.
       (+) MspInitCallback       : callback for Msp Init.
       (+) MspDeInitCallback     : callback for Msp DeInit.
     This function takes as parameters the HAL peripheral handle, the Callback ID
     and a pointer to the user callback function.
    [..]

     Use function @ref HAL_COMP_UnRegisterCallback to reset a callback to the default
     weak function.
    [..]

     @ref HAL_COMP_UnRegisterCallback takes as parameters the HAL peripheral handle,
     and the Callback ID.
     This function allows to reset following callbacks:
       (+) TriggerCallback       : callback for COMP trigger.
       (+) MspInitCallback       : callback for Msp Init.
       (+) MspDeInitCallback     : callback for Msp DeInit.
     [..]

     By default, after the @ref HAL_COMP_Init() and when the state is @ref HAL_COMP_STATE_RESET
     all callbacks are set to the corresponding weak functions:
     example @ref HAL_COMP_TriggerCallback().
     Exception done for MspInit and MspDeInit functions that are
     reset to the legacy weak functions in the @ref HAL_COMP_Init()/ @ref HAL_COMP_DeInit() only when
     these callbacks are null (not registered beforehand).
    [..]

     If MspInit or MspDeInit are not null, the @ref HAL_COMP_Init()/ @ref HAL_COMP_DeInit()
     keep and use the user MspInit/MspDeInit callbacks (registered beforehand) whatever the state.
     [..]

     Callbacks can be registered/unregistered in @ref HAL_COMP_STATE_READY state only.
     Exception done MspInit/MspDeInit functions that can be registered/unregistered
     in @ref HAL_COMP_STATE_READY or @ref HAL_COMP_STATE_RESET state,
     thus registered (user) MspInit/DeInit callbacks can be used during the Init/DeInit.
    [..]

     Then, the user first registers the MspInit/MspDeInit user callbacks
     using @ref HAL_COMP_RegisterCallback() before calling @ref HAL_COMP_DeInit()
     or @ref HAL_COMP_Init() function.
     [..]

     When the compilation flag USE_HAL_COMP_REGISTER_CALLBACKS is set to 0 or
     not defined, the callback registration feature is not available and all callbacks
     are set to the corresponding weak functions.

  @endverbatim
  ******************************************************************************

  Table 1. COMP inputs and output for STM32L4xx devices
  +-----------------------------------------------------------------+
  |                |                |     COMP1     |   COMP2 (4)   |
  |----------------|----------------|---------------|---------------+
  |                | IO1            |      PC5      |      PB4      |
  | Input plus     | IO2            |      PB2      |      PB6      |
  |                | IO3 (3)        |      PA1      |      PA3      |
  |----------------|----------------|---------------|---------------+
  |                | 1/4 VrefInt    |   Available   |   Available   |
  |                | 1/2 VrefInt    |   Available   |   Available   |
  |                | 3/4 VrefInt    |   Available   |   Available   |
  | Input minus    | VrefInt        |   Available   |   Available   |
  |                | DAC1 channel 1 |   Available   | Available (4) |
  |                | DAC1 channel 2 |   Available   | Available (4) |
  |                | IO1            |      PB1      |      PB3      |
  |                | IO2            |      PC4      |      PB7      |
  |                | IO3 (3)        |      PA0      |      PA2      |
  |                | IO4 (3)        |      PA4      |      PA4      |
  |                | IO5 (3)        |      PA5      |      PA5      |
  +----------------|----------------|---------------|---------------+
  | Output         |                |    PB0  (1)   |    PB5  (1)   |
  |                |                |    PB10 (1)   |    PB11 (1)   |
  |                |                |    TIM  (2)   |    TIM  (2)   |
  +-----------------------------------------------------------------+
  (1) GPIO must be set to alternate function for comparator
  (2) Comparators output to timers is set in timers instances.
  (3) Only STM32L43x/L44x
  (4) Not applicable to STM32L412x/L422x

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

#ifdef HAL_COMP_MODULE_ENABLED

#if defined (COMP1) || defined (COMP2)

/** @defgroup COMP COMP
  * @brief COMP HAL module driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @addtogroup COMP_Private_Constants
  * @{
  */

/* Delay for COMP startup time.                                               */
/* Note: Delay required to reach propagation delay specification.             */
/* Literal set to maximum value (refer to device datasheet,                   */
/* parameter "tSTART").                                                       */
/* Unit: us                                                                   */
#define COMP_DELAY_STARTUP_US          (80UL) /*!< Delay for COMP startup time */

/* Delay for COMP voltage scaler stabilization time.                          */
/* Literal set to maximum value (refer to device datasheet,                   */
/* parameter "tSTART_SCALER").                                                */
/* Unit: us                                                                   */
#define COMP_DELAY_VOLTAGE_SCALER_STAB_US (200UL)  /*!< Delay for COMP voltage scaler stabilization time */

#define COMP_OUTPUT_LEVEL_BITOFFSET_POS    (30UL)

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup COMP_Exported_Functions COMP Exported Functions
  * @{
  */

/** @defgroup COMP_Exported_Functions_Group1 Initialization/de-initialization functions
  *  @brief    Initialization and de-initialization functions.
  *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]  This section provides functions to initialize and de-initialize comparators

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the COMP according to the specified
  *         parameters in the COMP_InitTypeDef and initialize the associated handle.
  * @note   If the selected comparator is locked, initialization can't be performed.
  *         To unlock the configuration, perform a system reset.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Init(COMP_HandleTypeDef *hcomp)
{
  uint32_t tmp_csr;
  uint32_t exti_line;
  uint32_t comp_voltage_scaler_initialized; /* Value "0" if comparator voltage scaler is not initialized */
  __IO uint32_t wait_loop_index = 0UL;
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the COMP handle allocation and lock status */
  if(hcomp == NULL)
  {
    status = HAL_ERROR;
  }
  else if(__HAL_COMP_IS_LOCKED(hcomp))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameters */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    assert_param(IS_COMP_INPUT_PLUS(hcomp->Instance, hcomp->Init.NonInvertingInput));
    assert_param(IS_COMP_INPUT_MINUS(hcomp->Instance, hcomp->Init.InvertingInput));
    assert_param(IS_COMP_OUTPUTPOL(hcomp->Init.OutputPol));
    assert_param(IS_COMP_POWERMODE(hcomp->Init.Mode));
    assert_param(IS_COMP_HYSTERESIS(hcomp->Init.Hysteresis));
    assert_param(IS_COMP_BLANKINGSRC_INSTANCE(hcomp->Instance, hcomp->Init.BlankingSrce));
    assert_param(IS_COMP_TRIGGERMODE(hcomp->Init.TriggerMode));
#if defined(COMP2)
    assert_param(IS_COMP_WINDOWMODE(hcomp->Init.WindowMode));
#endif

    if(hcomp->State == HAL_COMP_STATE_RESET)
    {
      /* Allocate lock resource and initialize it */
      hcomp->Lock = HAL_UNLOCKED;

      /* Set COMP error code to none */
      COMP_CLEAR_ERRORCODE(hcomp);

      /* Init SYSCFG and the low level hardware to access comparators */
      /* Note: HAL_COMP_Init() calls __HAL_RCC_SYSCFG_CLK_ENABLE()            */
      /*       to enable internal control clock of the comparators.           */
      /*       However, this is a legacy strategy. In future STM32 families,  */
      /*       COMP clock enable must be implemented by user                  */
      /*       in "HAL_COMP_MspInit()".                                       */
      /*       Therefore, for compatibility anticipation, it is recommended   */
      /*       to implement __HAL_RCC_SYSCFG_CLK_ENABLE()                     */
      /*       in "HAL_COMP_MspInit()".                                       */
      __HAL_RCC_SYSCFG_CLK_ENABLE();

#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
      /* Init the COMP Callback settings */
      hcomp->TriggerCallback = HAL_COMP_TriggerCallback; /* Legacy weak callback */

      if (hcomp->MspInitCallback == NULL)
      {
        hcomp->MspInitCallback = HAL_COMP_MspInit; /* Legacy weak MspInit  */
      }

      /* Init the low level hardware */
      hcomp->MspInitCallback(hcomp);
#else
      /* Init the low level hardware */
      HAL_COMP_MspInit(hcomp);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
    }

    /* Memorize voltage scaler state before initialization */
    comp_voltage_scaler_initialized = READ_BIT(hcomp->Instance->CSR, COMP_CSR_SCALEN);

    /* Set COMP parameters */
    tmp_csr = (  hcomp->Init.NonInvertingInput
               | hcomp->Init.InvertingInput
               | hcomp->Init.BlankingSrce
               | hcomp->Init.Hysteresis
               | hcomp->Init.OutputPol
               | hcomp->Init.Mode
              );

    /* Set parameters in COMP register */
    /* Note: Update all bits except read-only, lock and enable bits */
#if defined (COMP_CSR_INMESEL)
#if defined (COMP_CSR_WINMODE)
    MODIFY_REG(hcomp->Instance->CSR,
               COMP_CSR_PWRMODE  | COMP_CSR_INMSEL   | COMP_CSR_INPSEL  |
               COMP_CSR_WINMODE  | COMP_CSR_POLARITY | COMP_CSR_HYST    |
               COMP_CSR_BLANKING | COMP_CSR_BRGEN    | COMP_CSR_SCALEN  | COMP_CSR_INMESEL,
               tmp_csr
              );
#else
    MODIFY_REG(hcomp->Instance->CSR,
               COMP_CSR_PWRMODE  | COMP_CSR_INMSEL   | COMP_CSR_INPSEL  |
                                   COMP_CSR_POLARITY | COMP_CSR_HYST    |
               COMP_CSR_BLANKING | COMP_CSR_BRGEN    | COMP_CSR_SCALEN  | COMP_CSR_INMESEL,
               tmp_csr
              );
#endif
#else
    MODIFY_REG(hcomp->Instance->CSR,
               COMP_CSR_PWRMODE  | COMP_CSR_INMSEL   | COMP_CSR_INPSEL  |
               COMP_CSR_WINMODE  | COMP_CSR_POLARITY | COMP_CSR_HYST    |
               COMP_CSR_BLANKING | COMP_CSR_BRGEN    | COMP_CSR_SCALEN,
               tmp_csr
              );
#endif

#if defined(COMP2)
    /* Set window mode */
    /* Note: Window mode bit is located into 1 out of the 2 pairs of COMP     */
    /*       instances. Therefore, this function can update another COMP      */
    /*       instance that the one currently selected.                        */
    if(hcomp->Init.WindowMode == COMP_WINDOWMODE_COMP1_INPUT_PLUS_COMMON)
    {
      SET_BIT(COMP12_COMMON->CSR, COMP_CSR_WINMODE);
    }
    else
    {
      CLEAR_BIT(COMP12_COMMON->CSR, COMP_CSR_WINMODE);
    }
#endif /* COMP2 */

    /* Delay for COMP scaler bridge voltage stabilization */
    /* Apply the delay if voltage scaler bridge is required and not already enabled */
    if ((READ_BIT(hcomp->Instance->CSR, COMP_CSR_SCALEN) != 0UL) &&
        (comp_voltage_scaler_initialized == 0UL)               )
    {
      /* Wait loop initialization and execution */
      /* Note: Variable divided by 2 to compensate partially              */
      /*       CPU processing cycles, scaling in us split to not          */
      /*       exceed 32 bits register capacity and handle low frequency. */
      wait_loop_index = ((COMP_DELAY_VOLTAGE_SCALER_STAB_US / 10UL) * (SystemCoreClock / (100000UL * 2UL)));
      while(wait_loop_index != 0UL)
      {
        wait_loop_index--;
      }
    }

    /* Get the EXTI line corresponding to the selected COMP instance */
    exti_line = COMP_GET_EXTI_LINE(hcomp->Instance);

    /* Manage EXTI settings */
    if((hcomp->Init.TriggerMode & (COMP_EXTI_IT | COMP_EXTI_EVENT)) != 0UL)
    {
      /* Configure EXTI rising edge */
      if((hcomp->Init.TriggerMode & COMP_EXTI_RISING) != 0UL)
      {
        LL_EXTI_EnableRisingTrig_0_31(exti_line);
      }
      else
      {
        LL_EXTI_DisableRisingTrig_0_31(exti_line);
      }

      /* Configure EXTI falling edge */
      if((hcomp->Init.TriggerMode & COMP_EXTI_FALLING) != 0UL)
      {
        LL_EXTI_EnableFallingTrig_0_31(exti_line);
      }
      else
      {
        LL_EXTI_DisableFallingTrig_0_31(exti_line);
      }

      /* Clear COMP EXTI pending bit (if any) */
      LL_EXTI_ClearFlag_0_31(exti_line);

      /* Configure EXTI event mode */
      if((hcomp->Init.TriggerMode & COMP_EXTI_EVENT) != 0UL)
      {
        LL_EXTI_EnableEvent_0_31(exti_line);
      }
      else
      {
        LL_EXTI_DisableEvent_0_31(exti_line);
      }

      /* Configure EXTI interrupt mode */
      if((hcomp->Init.TriggerMode & COMP_EXTI_IT) != 0UL)
      {
        LL_EXTI_EnableIT_0_31(exti_line);
      }
      else
      {
        LL_EXTI_DisableIT_0_31(exti_line);
      }
    }
    else
    {
      /* Disable EXTI event mode */
      LL_EXTI_DisableEvent_0_31(exti_line);

      /* Disable EXTI interrupt mode */
      LL_EXTI_DisableIT_0_31(exti_line);
    }

    /* Set HAL COMP handle state */
    /* Note: Transition from state reset to state ready,                      */
    /*       otherwise (coming from state ready or busy) no state update.     */
    if (hcomp->State == HAL_COMP_STATE_RESET)
    {
      hcomp->State = HAL_COMP_STATE_READY;
    }
  }

  return status;
}

/**
  * @brief  DeInitialize the COMP peripheral.
  * @note   Deinitialization cannot be performed if the COMP configuration is locked.
  *         To unlock the configuration, perform a system reset.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_DeInit(COMP_HandleTypeDef *hcomp)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the COMP handle allocation and lock status */
  if(hcomp == NULL)
  {
    status = HAL_ERROR;
  }
  else if(__HAL_COMP_IS_LOCKED(hcomp))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));

    /* Set COMP_CSR register to reset value */
    WRITE_REG(hcomp->Instance->CSR, 0x00000000UL);

#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
    if (hcomp->MspDeInitCallback == NULL)
    {
      hcomp->MspDeInitCallback = HAL_COMP_MspDeInit; /* Legacy weak MspDeInit  */
    }

    /* DeInit the low level hardware: GPIO, RCC clock, NVIC */
    hcomp->MspDeInitCallback(hcomp);
#else
    /* DeInit the low level hardware: GPIO, RCC clock, NVIC */
    HAL_COMP_MspDeInit(hcomp);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */

    /* Set HAL COMP handle state */
    hcomp->State = HAL_COMP_STATE_RESET;

    /* Release Lock */
    __HAL_UNLOCK(hcomp);
  }

  return status;
}

/**
  * @brief  Initialize the COMP MSP.
  * @param  hcomp  COMP handle
  * @retval None
  */
__weak void HAL_COMP_MspInit(COMP_HandleTypeDef *hcomp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcomp);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_COMP_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitialize the COMP MSP.
  * @param  hcomp  COMP handle
  * @retval None
  */
__weak void HAL_COMP_MspDeInit(COMP_HandleTypeDef *hcomp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcomp);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_COMP_MspDeInit could be implemented in the user file
   */
}

#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User COMP Callback
  *         To be used instead of the weak predefined callback
  * @param  hcomp Pointer to a COMP_HandleTypeDef structure that contains
  *                the configuration information for the specified COMP.
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_COMP_TRIGGER_CB_ID Trigger callback ID
  *          @arg @ref HAL_COMP_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_COMP_MSPDEINIT_CB_ID MspDeInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_RegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID, pCOMP_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }

  if (HAL_COMP_STATE_READY == hcomp->State)
  {
    switch (CallbackID)
    {
      case HAL_COMP_TRIGGER_CB_ID :
        hcomp->TriggerCallback = pCallback;
        break;

      case HAL_COMP_MSPINIT_CB_ID :
        hcomp->MspInitCallback = pCallback;
        break;

      case HAL_COMP_MSPDEINIT_CB_ID :
        hcomp->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else if (HAL_COMP_STATE_RESET == hcomp->State)
  {
    switch (CallbackID)
    {
      case HAL_COMP_MSPINIT_CB_ID :
        hcomp->MspInitCallback = pCallback;
        break;

      case HAL_COMP_MSPDEINIT_CB_ID :
        hcomp->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Unregister a COMP Callback
  *         COMP callback is redirected to the weak predefined callback
  * @param  hcomp Pointer to a COMP_HandleTypeDef structure that contains
  *                the configuration information for the specified COMP.
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_COMP_TRIGGER_CB_ID Trigger callback ID
  *          @arg @ref HAL_COMP_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_COMP_MSPDEINIT_CB_ID MspDeInit callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_UnRegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (HAL_COMP_STATE_READY == hcomp->State)
  {
    switch (CallbackID)
    {
      case HAL_COMP_TRIGGER_CB_ID :
        hcomp->TriggerCallback = HAL_COMP_TriggerCallback;         /* Legacy weak callback */
        break;

      case HAL_COMP_MSPINIT_CB_ID :
        hcomp->MspInitCallback = HAL_COMP_MspInit;                 /* Legacy weak MspInit */
        break;

      case HAL_COMP_MSPDEINIT_CB_ID :
        hcomp->MspDeInitCallback = HAL_COMP_MspDeInit;             /* Legacy weak MspDeInit */
        break;

      default :
        /* Update the error code */
        hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_COMP_STATE_RESET == hcomp->State)
  {
    switch (CallbackID)
    {
      case HAL_COMP_MSPINIT_CB_ID :
        hcomp->MspInitCallback = HAL_COMP_MspInit;                 /* Legacy weak MspInit */
        break;

      case HAL_COMP_MSPDEINIT_CB_ID :
        hcomp->MspDeInitCallback = HAL_COMP_MspDeInit;             /* Legacy weak MspDeInit */
        break;

      default :
        /* Update the error code */
        hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  return status;
}

#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup COMP_Exported_Functions_Group2 Start-Stop operation functions
  *  @brief   Start-Stop operation functions.
  *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Start a comparator instance.
      (+) Stop a comparator instance.

@endverbatim
  * @{
  */

/**
  * @brief  Start the comparator.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Start(COMP_HandleTypeDef *hcomp)
{
  __IO uint32_t wait_loop_index = 0UL;
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the COMP handle allocation and lock status */
  if(hcomp == NULL)
  {
    status = HAL_ERROR;
  }
  else if(__HAL_COMP_IS_LOCKED(hcomp))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));

    if(hcomp->State == HAL_COMP_STATE_READY)
    {
      /* Enable the selected comparator */
      SET_BIT(hcomp->Instance->CSR, COMP_CSR_EN);

      /* Set HAL COMP handle state */
      hcomp->State = HAL_COMP_STATE_BUSY;

      /* Delay for COMP startup time */
      /* Wait loop initialization and execution */
      /* Note: Variable divided by 2 to compensate partially              */
      /*       CPU processing cycles, scaling in us split to not          */
      /*       exceed 32 bits register capacity and handle low frequency. */
      wait_loop_index = ((COMP_DELAY_STARTUP_US / 10UL) * (SystemCoreClock / (100000UL * 2UL)));
      while(wait_loop_index != 0UL)
      {
        wait_loop_index--;
      }
    }
    else
    {
      status = HAL_ERROR;
    }
  }

  return status;
}

/**
  * @brief  Stop the comparator.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Stop(COMP_HandleTypeDef *hcomp)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the COMP handle allocation and lock status */
  if(hcomp == NULL)
  {
    status = HAL_ERROR;
  }
  else if(__HAL_COMP_IS_LOCKED(hcomp))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));

    /* Check compliant states: HAL_COMP_STATE_READY or HAL_COMP_STATE_BUSY    */
    /* (all states except HAL_COMP_STATE_RESET and except locked status.      */
    if(hcomp->State != HAL_COMP_STATE_RESET)
    {
      /* Disable the selected comparator */
      CLEAR_BIT(hcomp->Instance->CSR, COMP_CSR_EN);

      /* Set HAL COMP handle state */
      hcomp->State = HAL_COMP_STATE_READY;
    }
    else
    {
      status = HAL_ERROR;
    }
  }

  return status;
}

/**
  * @brief  Comparator IRQ handler.
  * @param  hcomp  COMP handle
  * @retval None
  */
void HAL_COMP_IRQHandler(COMP_HandleTypeDef *hcomp)
{
  /* Get the EXTI line corresponding to the selected COMP instance */
  uint32_t exti_line = COMP_GET_EXTI_LINE(hcomp->Instance);

  /* Check COMP EXTI flag */
  if(LL_EXTI_IsActiveFlag_0_31(exti_line) != 0UL)
  {
#if defined(COMP2)
    /* Check whether comparator is in independent or window mode */
    if(READ_BIT(COMP12_COMMON->CSR, COMP_CSR_WINMODE) != RESET)
    {
      /* Clear COMP EXTI line pending bit of the pair of comparators          */
      /* in window mode.                                                      */
      /* Note: Pair of comparators in window mode can both trig IRQ when      */
      /*       input voltage is changing from "out of window" area            */
      /*       (low or high ) to the other "out of window" area (high or low).*/
      /*       Both flags must be cleared to call comparator trigger          */
      /*       callback is called once.                                       */
      LL_EXTI_ClearFlag_0_31((COMP_EXTI_LINE_COMP1 | COMP_EXTI_LINE_COMP2));
    }
    else
#endif /* COMP2 */
    {
      /* Clear COMP EXTI line pending bit */
      LL_EXTI_ClearFlag_0_31(exti_line);
    }

    /* COMP trigger user callback */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
    hcomp->TriggerCallback(hcomp);
#else
    HAL_COMP_TriggerCallback(hcomp);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
  }
}

/**
  * @}
  */

/** @defgroup COMP_Exported_Functions_Group3 Peripheral Control functions
  *  @brief   Management functions.
  *
@verbatim
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to control the comparators.

@endverbatim
  * @{
  */

/**
  * @brief  Lock the selected comparator configuration.
  * @note   A system reset is required to unlock the comparator configuration.
  * @note   Locking the comparator from reset state is possible
  *         if __HAL_RCC_SYSCFG_CLK_ENABLE() is being called before.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Lock(COMP_HandleTypeDef *hcomp)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the COMP handle allocation and lock status */
  if(hcomp == NULL)
  {
    status = HAL_ERROR;
  }
  else if(__HAL_COMP_IS_LOCKED(hcomp))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));

    /* Set HAL COMP handle state */
    switch(hcomp->State)
    {
      case HAL_COMP_STATE_RESET:
        hcomp->State = HAL_COMP_STATE_RESET_LOCKED;
        break;
      case HAL_COMP_STATE_READY:
        hcomp->State = HAL_COMP_STATE_READY_LOCKED;
        break;
      default: /* HAL_COMP_STATE_BUSY */
        hcomp->State = HAL_COMP_STATE_BUSY_LOCKED;
        break;
    }
  }

  if(status == HAL_OK)
  {
    /* Set the lock bit corresponding to selected comparator */
    __HAL_COMP_LOCK(hcomp);
  }

  return status;
}

/**
  * @brief  Return the output level (high or low) of the selected comparator.
  *         The output level depends on the selected polarity.
  *         If the polarity is not inverted:
  *           - Comparator output is low when the input plus is at a lower
  *             voltage than the input minus
  *           - Comparator output is high when the input plus is at a higher
  *             voltage than the input minus
  *         If the polarity is inverted:
  *           - Comparator output is high when the input plus is at a lower
  *             voltage than the input minus
  *           - Comparator output is low when the input plus is at a higher
  *             voltage than the input minus
  * @param  hcomp  COMP handle
  * @retval Returns the selected comparator output level:
  *         @arg COMP_OUTPUT_LEVEL_LOW
  *         @arg COMP_OUTPUT_LEVEL_HIGH
  *
  */
uint32_t HAL_COMP_GetOutputLevel(COMP_HandleTypeDef *hcomp)
{
  /* Check the parameter */
  assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));

  return (uint32_t)(READ_BIT(hcomp->Instance->CSR, COMP_CSR_VALUE)
                    >> COMP_OUTPUT_LEVEL_BITOFFSET_POS);
}

/**
  * @brief  Comparator trigger callback.
  * @param  hcomp  COMP handle
  * @retval None
  */
__weak void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcomp);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_COMP_TriggerCallback should be implemented in the user file
   */
}


/**
  * @}
  */

/** @defgroup COMP_Exported_Functions_Group4 Peripheral State functions
  *  @brief   Peripheral State functions.
  *
@verbatim
 ===============================================================================
                      ##### Peripheral State functions #####
 ===============================================================================
    [..]
    This subsection permit to get in run-time the status of the peripheral.

@endverbatim
  * @{
  */

/**
  * @brief  Return the COMP handle state.
  * @param  hcomp  COMP handle
  * @retval HAL state
  */
HAL_COMP_StateTypeDef HAL_COMP_GetState(COMP_HandleTypeDef *hcomp)
{
  /* Check the COMP handle allocation */
  if(hcomp == NULL)
  {
    return HAL_COMP_STATE_RESET;
  }

  /* Check the parameter */
  assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));

  /* Return HAL COMP handle state */
  return hcomp->State;
}

/**
  * @brief  Return the COMP error code.
  * @param hcomp COMP handle
  * @retval COMP error code
  */
uint32_t HAL_COMP_GetError(COMP_HandleTypeDef *hcomp)
{
  /* Check the parameters */
  assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));

  return hcomp->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* COMP1 || COMP2 */

#endif /* HAL_COMP_MODULE_ENABLED */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
