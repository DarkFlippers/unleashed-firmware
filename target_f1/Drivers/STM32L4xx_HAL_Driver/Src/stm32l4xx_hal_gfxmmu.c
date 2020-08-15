/**
  ******************************************************************************
  * @file    stm32l4xx_hal_gfxmmu.c
  * @author  MCD Application Team
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the Graphic MMU (GFXMMU) peripheral:
  *           + Initialization and De-initialization.
  *           + LUT configuration.
  *           + Modify physical buffer adresses.
  *           + Error management.
  *         
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
  [..]
    *** Initialization ***
    ======================
    [..]
      (#) As prerequisite, fill in the HAL_GFXMMU_MspInit() :
        (++) Enable GFXMMU clock interface with __HAL_RCC_GFXMMU_CLK_ENABLE().
        (++) If interrupts are used, enable and configure GFXMMU global
            interrupt with HAL_NVIC_SetPriority() and HAL_NVIC_EnableIRQ().
      (#) Configure the number of blocks per line, default value, physical 
          buffer addresses and interrupts using the HAL_GFXMMU_Init() function.

    *** LUT configuration ***
    =========================
    [..]
      (#) Use HAL_GFXMMU_DisableLutLines() to deactivate all LUT lines (or a
          range of lines).
      (#) Use HAL_GFXMMU_ConfigLut() to copy LUT from flash to look up RAM.
      (#) Use HAL_GFXMMU_ConfigLutLine() to configure one line of LUT.

    *** Modify physical buffer adresses ***
    =======================================
    [..]    
      (#) Use HAL_GFXMMU_ModifyBuffers() to modify physical buffer addresses.

    *** Error management ***
    ========================
    [..]
      (#) If interrupts are used, HAL_GFXMMU_IRQHandler() will be called when
          an error occurs. This function will call HAL_GFXMMU_ErrorCallback().
          Use HAL_GFXMMU_GetError() to get the error code.

    *** De-initialization ***
    =========================
    [..]    
      (#) As prerequisite, fill in the HAL_GFXMMU_MspDeInit() :
        (++) Disable GFXMMU clock interface with __HAL_RCC_GFXMMU_CLK_ENABLE().
        (++) If interrupts has been used, disable GFXMMU global interrupt with
             HAL_NVIC_DisableIRQ().
      (#) De-initialize GFXMMU using the HAL_GFXMMU_DeInit() function.

    *** Callback registration ***
    =============================
    [..]
    The compilation define USE_HAL_GFXMMU_REGISTER_CALLBACKS when set to 1
    allows the user to configure dynamically the driver callbacks.
    Use functions HAL_GFXMMU_RegisterCallback() to register a user callback.

    [..]
    Function HAL_GFXMMU_RegisterCallback() allows to register following callbacks:
      (+) ErrorCallback      : GFXMMU error.
      (+) MspInitCallback    : GFXMMU MspInit.
      (+) MspDeInitCallback  : GFXMMU MspDeInit.
    [..]
    This function takes as parameters the HAL peripheral handle, the callback ID
    and a pointer to the user callback function.

    [..]
    Use function HAL_GFXMMU_UnRegisterCallback() to reset a callback to the default
    weak (surcharged) function.
    HAL_GFXMMU_UnRegisterCallback() takes as parameters the HAL peripheral handle,
    and the callback ID.
    [..]
    This function allows to reset following callbacks:
      (+) ErrorCallback      : GFXMMU error.
      (+) MspInitCallback    : GFXMMU MspInit.
      (+) MspDeInitCallback  : GFXMMU MspDeInit.

    [..]
    By default, after the HAL_GFXMMU_Init and if the state is HAL_GFXMMU_STATE_RESET
    all callbacks are reset to the corresponding legacy weak (surcharged) functions:
    examples HAL_GFXMMU_ErrorCallback().
    Exception done for MspInit and MspDeInit callbacks that are respectively
    reset to the legacy weak (surcharged) functions in the HAL_GFXMMU_Init
    and HAL_GFXMMU_DeInit only when these callbacks are null (not registered beforehand).
    If not, MspInit or MspDeInit are not null, the HAL_GFXMMU_Init and HAL_GFXMMU_DeInit
    keep and use the user MspInit/MspDeInit callbacks (registered beforehand).

    [..]
    Callbacks can be registered/unregistered in READY state only.
    Exception done for MspInit/MspDeInit callbacks that can be registered/unregistered
    in READY or RESET state, thus registered (user) MspInit/DeInit callbacks can be used
    during the Init/DeInit.
    In that case first register the MspInit/MspDeInit user callbacks
    using HAL_GFXMMU_RegisterCallback before calling HAL_GFXMMU_DeInit
    or HAL_GFXMMU_Init function.

    [..]
    When the compilation define USE_HAL_GFXMMU_REGISTER_CALLBACKS is set to 0 or
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
#ifdef HAL_GFXMMU_MODULE_ENABLED
#if defined(GFXMMU)
/** @defgroup GFXMMU GFXMMU
  * @brief GFXMMU HAL driver module
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define GFXMMU_LUTXL_FVB_OFFSET     8U
#define GFXMMU_LUTXL_LVB_OFFSET     16U
#define GFXMMU_CR_ITS_MASK          0x1FU
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup GFXMMU_Exported_Functions GFXMMU Exported Functions
  * @{
  */

/** @defgroup GFXMMU_Exported_Functions_Group1 Initialization and de-initialization functions
 *  @brief    Initialization and de-initialization functions 
 *
@verbatim
  ==============================================================================
          ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the GFXMMU.
      (+) De-initialize the GFXMMU.
@endverbatim
  * @{
  */

/**
  * @brief  Initialize the GFXMMU according to the specified parameters in the
  *         GFXMMU_InitTypeDef structure and initialize the associated handle.
  * @param  hgfxmmu GFXMMU handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_Init(GFXMMU_HandleTypeDef *hgfxmmu)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check GFXMMU handle */
  if(hgfxmmu == NULL)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check parameters */
    assert_param(IS_GFXMMU_ALL_INSTANCE(hgfxmmu->Instance));
    assert_param(IS_GFXMMU_BLOCKS_PER_LINE(hgfxmmu->Init.BlocksPerLine));
    assert_param(IS_GFXMMU_BUFFER_ADDRESS(hgfxmmu->Init.Buffers.Buf0Address));
    assert_param(IS_GFXMMU_BUFFER_ADDRESS(hgfxmmu->Init.Buffers.Buf1Address));
    assert_param(IS_GFXMMU_BUFFER_ADDRESS(hgfxmmu->Init.Buffers.Buf2Address));
    assert_param(IS_GFXMMU_BUFFER_ADDRESS(hgfxmmu->Init.Buffers.Buf3Address));
    assert_param(IS_FUNCTIONAL_STATE(hgfxmmu->Init.Interrupts.Activation));
    
#if (USE_HAL_GFXMMU_REGISTER_CALLBACKS == 1)
    /* Reset callback pointers to the weak predefined callbacks */
    hgfxmmu->ErrorCallback = HAL_GFXMMU_ErrorCallback;

    /* Call GFXMMU MSP init function */
    if(hgfxmmu->MspInitCallback == NULL)
    {
      hgfxmmu->MspInitCallback = HAL_GFXMMU_MspInit;
    }
    hgfxmmu->MspInitCallback(hgfxmmu);
#else
    /* Call GFXMMU MSP init function */
    HAL_GFXMMU_MspInit(hgfxmmu);
#endif
    
    /* Configure blocks per line and interrupts parameters on GFXMMU_CR register */
    hgfxmmu->Instance->CR &= ~(GFXMMU_CR_B0OIE | GFXMMU_CR_B1OIE | GFXMMU_CR_B2OIE | GFXMMU_CR_B3OIE |
                               GFXMMU_CR_AMEIE | GFXMMU_CR_192BM);
    hgfxmmu->Instance->CR |= (hgfxmmu->Init.BlocksPerLine);
    if(hgfxmmu->Init.Interrupts.Activation == ENABLE)
    {
      assert_param(IS_GFXMMU_INTERRUPTS(hgfxmmu->Init.Interrupts.UsedInterrupts));
      hgfxmmu->Instance->CR |= hgfxmmu->Init.Interrupts.UsedInterrupts;
    }
    
    /* Configure default value on GFXMMU_DVR register */
    hgfxmmu->Instance->DVR = hgfxmmu->Init.DefaultValue;
    
    /* Configure physical buffer adresses on GFXMMU_BxCR registers */
    hgfxmmu->Instance->B0CR = hgfxmmu->Init.Buffers.Buf0Address;
    hgfxmmu->Instance->B1CR = hgfxmmu->Init.Buffers.Buf1Address;
    hgfxmmu->Instance->B2CR = hgfxmmu->Init.Buffers.Buf2Address;
    hgfxmmu->Instance->B3CR = hgfxmmu->Init.Buffers.Buf3Address;
    
    /* Reset GFXMMU error code */
    hgfxmmu->ErrorCode = GFXMMU_ERROR_NONE;
    
    /* Set GFXMMU to ready state */
    hgfxmmu->State = HAL_GFXMMU_STATE_READY;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  De-initialize the GFXMMU.
  * @param  hgfxmmu GFXMMU handle.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_DeInit(GFXMMU_HandleTypeDef *hgfxmmu)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check GFXMMU handle */
  if(hgfxmmu == NULL)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check parameters */
    assert_param(IS_GFXMMU_ALL_INSTANCE(hgfxmmu->Instance));
    
    /* Disable all interrupts on GFXMMU_CR register */
    hgfxmmu->Instance->CR &= ~(GFXMMU_CR_B0OIE | GFXMMU_CR_B1OIE | GFXMMU_CR_B2OIE | GFXMMU_CR_B3OIE |
                               GFXMMU_CR_AMEIE);
    
    /* Call GFXMMU MSP de-init function */
#if (USE_HAL_GFXMMU_REGISTER_CALLBACKS == 1)
    if(hgfxmmu->MspDeInitCallback == NULL)
    {
      hgfxmmu->MspDeInitCallback = HAL_GFXMMU_MspDeInit;
    }
    hgfxmmu->MspDeInitCallback(hgfxmmu);
#else
    HAL_GFXMMU_MspDeInit(hgfxmmu);
#endif
    
    /* Set GFXMMU to reset state */
    hgfxmmu->State = HAL_GFXMMU_STATE_RESET;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  Initialize the GFXMMU MSP.
  * @param  hgfxmmu GFXMMU handle.
  * @retval None.
  */
__weak void HAL_GFXMMU_MspInit(GFXMMU_HandleTypeDef *hgfxmmu)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hgfxmmu);
  
  /* NOTE : This function should not be modified, when the function is needed,
            the HAL_GFXMMU_MspInit could be implemented in the user file.
   */
}

/**
  * @brief  De-initialize the GFXMMU MSP.
  * @param  hgfxmmu GFXMMU handle.
  * @retval None.
  */
__weak void HAL_GFXMMU_MspDeInit(GFXMMU_HandleTypeDef *hgfxmmu)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hgfxmmu);
  
  /* NOTE : This function should not be modified, when the function is needed,
            the HAL_GFXMMU_MspDeInit could be implemented in the user file.
   */
}

#if (USE_HAL_GFXMMU_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a user GFXMMU callback
  *         to be used instead of the weak predefined callback.
  * @param  hgfxmmu GFXMMU handle.
  * @param  CallbackID ID of the callback to be registered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_GFXMMU_ERROR_CB_ID error callback ID.
  *           @arg @ref HAL_GFXMMU_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_GFXMMU_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @param  pCallback pointer to the callback function.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_RegisterCallback(GFXMMU_HandleTypeDef        *hgfxmmu,
                                              HAL_GFXMMU_CallbackIDTypeDef CallbackID,
                                              pGFXMMU_CallbackTypeDef      pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    /* update the error code */
    hgfxmmu->ErrorCode |= GFXMMU_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  else
  {
    if(HAL_GFXMMU_STATE_READY == hgfxmmu->State)
    {
      switch (CallbackID)
      {
      case HAL_GFXMMU_ERROR_CB_ID :
        hgfxmmu->ErrorCallback = pCallback;
        break;
      case HAL_GFXMMU_MSPINIT_CB_ID :
        hgfxmmu->MspInitCallback = pCallback;
        break;
      case HAL_GFXMMU_MSPDEINIT_CB_ID :
        hgfxmmu->MspDeInitCallback = pCallback;
        break;
      default :
        /* update the error code */
        hgfxmmu->ErrorCode |= GFXMMU_ERROR_INVALID_CALLBACK;
        /* update return status */
        status = HAL_ERROR;
        break;
      }
    }
    else if(HAL_GFXMMU_STATE_RESET == hgfxmmu->State)
    {
      switch (CallbackID)
      {
      case HAL_GFXMMU_MSPINIT_CB_ID :
        hgfxmmu->MspInitCallback = pCallback;
        break;
      case HAL_GFXMMU_MSPDEINIT_CB_ID :
        hgfxmmu->MspDeInitCallback = pCallback;
        break;
      default :
        /* update the error code */
        hgfxmmu->ErrorCode |= GFXMMU_ERROR_INVALID_CALLBACK;
        /* update return status */
        status = HAL_ERROR;
        break;
      }
    }
    else
    {
      /* update the error code */
      hgfxmmu->ErrorCode |= GFXMMU_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
    }
  }
  return status;
}

/**
  * @brief  Unregister a user GFXMMU callback.
  *         GFXMMU callback is redirected to the weak predefined callback.
  * @param  hgfxmmu GFXMMU handle.
  * @param  CallbackID ID of the callback to be unregistered.
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_GFXMMU_ERROR_CB_ID error callback ID.
  *           @arg @ref HAL_GFXMMU_MSPINIT_CB_ID MSP init callback ID.
  *           @arg @ref HAL_GFXMMU_MSPDEINIT_CB_ID MSP de-init callback ID.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_UnRegisterCallback(GFXMMU_HandleTypeDef        *hgfxmmu,
                                                HAL_GFXMMU_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(HAL_GFXMMU_STATE_READY == hgfxmmu->State)
  {
    switch (CallbackID)
    {
    case HAL_GFXMMU_ERROR_CB_ID :
      hgfxmmu->ErrorCallback = HAL_GFXMMU_ErrorCallback;
      break;
    case HAL_GFXMMU_MSPINIT_CB_ID :
      hgfxmmu->MspInitCallback = HAL_GFXMMU_MspInit;
      break;
    case HAL_GFXMMU_MSPDEINIT_CB_ID :
      hgfxmmu->MspDeInitCallback = HAL_GFXMMU_MspDeInit;
      break;
    default :
      /* update the error code */
      hgfxmmu->ErrorCode |= GFXMMU_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
      break;
    }
  }
  else if(HAL_GFXMMU_STATE_RESET == hgfxmmu->State)
  {
    switch (CallbackID)
    {
    case HAL_GFXMMU_MSPINIT_CB_ID :
      hgfxmmu->MspInitCallback = HAL_GFXMMU_MspInit;
      break;
    case HAL_GFXMMU_MSPDEINIT_CB_ID :
      hgfxmmu->MspDeInitCallback = HAL_GFXMMU_MspDeInit;
      break;
    default :
      /* update the error code */
      hgfxmmu->ErrorCode |= GFXMMU_ERROR_INVALID_CALLBACK;
      /* update return status */
      status = HAL_ERROR;
      break;
    }
  }
  else
  {
    /* update the error code */
    hgfxmmu->ErrorCode |= GFXMMU_ERROR_INVALID_CALLBACK;
    /* update return status */
    status = HAL_ERROR;
  }
  return status;
}
#endif /* USE_HAL_GFXMMU_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup GFXMMU_Exported_Functions_Group2 Operations functions
 *  @brief    GFXMMU operation functions
 *
@verbatim
  ==============================================================================
                      ##### Operation functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Configure LUT.
      (+) Modify physical buffer adresses.
      (+) Manage error.
@endverbatim
  * @{
  */

/**
  * @brief  This function allows to copy LUT from flash to look up RAM.
  * @param  hgfxmmu GFXMMU handle.
  * @param  FirstLine First line enabled on LUT.
  *         This parameter must be a number between Min_Data = 0 and Max_Data = 1023.
  * @param  LinesNumber Number of lines enabled on LUT.
  *         This parameter must be a number between Min_Data = 1 and Max_Data = 1024.
  * @param  Address Start address of LUT in flash.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_ConfigLut(GFXMMU_HandleTypeDef *hgfxmmu,
                                       uint32_t FirstLine,
                                       uint32_t LinesNumber,
                                       uint32_t Address)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check parameters */
  assert_param(IS_GFXMMU_ALL_INSTANCE(hgfxmmu->Instance));
  assert_param(IS_GFXMMU_LUT_LINE(FirstLine));
  assert_param(IS_GFXMMU_LUT_LINES_NUMBER(LinesNumber));
  
  /* Check GFXMMU state and coherent parameters */
  if((hgfxmmu->State != HAL_GFXMMU_STATE_READY) || ((FirstLine + LinesNumber) > 1024U))
  {
    status = HAL_ERROR;
  }
  else
  {
    uint32_t current_address, current_line, lutxl_address, lutxh_address;
    
    /* Initialize local variables */
    current_address = Address;
    current_line    = 0U;
    lutxl_address   = (uint32_t) &(hgfxmmu->Instance->LUT[2U * FirstLine]);
    lutxh_address   = (uint32_t) &(hgfxmmu->Instance->LUT[(2U * FirstLine) + 1U]);
    
    /* Copy LUT from flash to look up RAM */
    while(current_line < LinesNumber)
    {
      *((uint32_t *)lutxl_address) = *((uint32_t *)current_address);
      current_address += 4U;
      *((uint32_t *)lutxh_address) = *((uint32_t *)current_address);
      current_address += 4U;
      lutxl_address += 8U;
      lutxh_address += 8U;
      current_line++;
    }
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to disable a range of LUT lines.
  * @param  hgfxmmu GFXMMU handle.
  * @param  FirstLine First line to disable on LUT.
  *         This parameter must be a number between Min_Data = 0 and Max_Data = 1023.
  * @param  LinesNumber Number of lines to disable on LUT.
  *         This parameter must be a number between Min_Data = 1 and Max_Data = 1024.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_DisableLutLines(GFXMMU_HandleTypeDef *hgfxmmu,
                                             uint32_t FirstLine,
                                             uint32_t LinesNumber)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check parameters */
  assert_param(IS_GFXMMU_ALL_INSTANCE(hgfxmmu->Instance));
  assert_param(IS_GFXMMU_LUT_LINE(FirstLine));
  assert_param(IS_GFXMMU_LUT_LINES_NUMBER(LinesNumber));
  
  /* Check GFXMMU state and coherent parameters */
  if((hgfxmmu->State != HAL_GFXMMU_STATE_READY) || ((FirstLine + LinesNumber) > 1024U))
  {
    status = HAL_ERROR;
  }
  else
  {
    uint32_t current_line, lutxl_address, lutxh_address;
    
    /* Initialize local variables */
    current_line    = 0U;
    lutxl_address   = (uint32_t) &(hgfxmmu->Instance->LUT[2U * FirstLine]);
    lutxh_address   = (uint32_t) &(hgfxmmu->Instance->LUT[(2U * FirstLine) + 1U]);
    
    /* Disable LUT lines */
    while(current_line < LinesNumber)
    {
      *((uint32_t *)lutxl_address) = 0U;
      *((uint32_t *)lutxh_address) = 0U;
      lutxl_address += 8U;
      lutxh_address += 8U;
      current_line++;
    }
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to configure one line of LUT.
  * @param  hgfxmmu GFXMMU handle.
  * @param  lutLine LUT line parameters.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_ConfigLutLine(GFXMMU_HandleTypeDef *hgfxmmu, GFXMMU_LutLineTypeDef *lutLine)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check parameters */
  assert_param(IS_GFXMMU_ALL_INSTANCE(hgfxmmu->Instance));
  assert_param(IS_GFXMMU_LUT_LINE(lutLine->LineNumber));
  assert_param(IS_GFXMMU_LUT_LINE_STATUS(lutLine->LineStatus));
  assert_param(IS_GFXMMU_LUT_BLOCK(lutLine->FirstVisibleBlock));
  assert_param(IS_GFXMMU_LUT_BLOCK(lutLine->LastVisibleBlock));
  assert_param(IS_GFXMMU_LUT_LINE_OFFSET(lutLine->LineOffset));
  
  /* Check GFXMMU state */
  if(hgfxmmu->State != HAL_GFXMMU_STATE_READY)
  {
    status = HAL_ERROR;
  }
  else
  {
    uint32_t lutxl_address, lutxh_address;
    
    /* Initialize local variables */
    lutxl_address   = (uint32_t) &(hgfxmmu->Instance->LUT[2U * lutLine->LineNumber]);
    lutxh_address   = (uint32_t) &(hgfxmmu->Instance->LUT[(2U * lutLine->LineNumber) + 1U]);
    
    /* Configure LUT line */
    if(lutLine->LineStatus == GFXMMU_LUT_LINE_ENABLE)
    {
      /* Enable and configure LUT line */
      *((uint32_t *)lutxl_address) = (lutLine->LineStatus | 
                                     (lutLine->FirstVisibleBlock << GFXMMU_LUTXL_FVB_OFFSET) | 
                                     (lutLine->LastVisibleBlock << GFXMMU_LUTXL_LVB_OFFSET));
      *((uint32_t *)lutxh_address) = (uint32_t) lutLine->LineOffset;
    }
    else
    {
      /* Disable LUT line */
      *((uint32_t *)lutxl_address) = 0U;
      *((uint32_t *)lutxh_address) = 0U;
    }
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function allows to modify physical buffer addresses.
  * @param  hgfxmmu GFXMMU handle.
  * @param  Buffers Buffers parameters.
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_GFXMMU_ModifyBuffers(GFXMMU_HandleTypeDef *hgfxmmu, GFXMMU_BuffersTypeDef *Buffers)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check parameters */
  assert_param(IS_GFXMMU_ALL_INSTANCE(hgfxmmu->Instance));
  assert_param(IS_GFXMMU_BUFFER_ADDRESS(Buffers->Buf0Address));
  assert_param(IS_GFXMMU_BUFFER_ADDRESS(Buffers->Buf1Address));
  assert_param(IS_GFXMMU_BUFFER_ADDRESS(Buffers->Buf2Address));
  assert_param(IS_GFXMMU_BUFFER_ADDRESS(Buffers->Buf3Address));
  
  /* Check GFXMMU state */
  if(hgfxmmu->State != HAL_GFXMMU_STATE_READY)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Modify physical buffer adresses on GFXMMU_BxCR registers */
    hgfxmmu->Instance->B0CR = Buffers->Buf0Address;
    hgfxmmu->Instance->B1CR = Buffers->Buf1Address;
    hgfxmmu->Instance->B2CR = Buffers->Buf2Address;
    hgfxmmu->Instance->B3CR = Buffers->Buf3Address;
  }
  /* Return function status */
  return status;
}

/**
  * @brief  This function handles the GFXMMU interrupts.
  * @param  hgfxmmu GFXMMU handle.
  * @retval None.
  */
void HAL_GFXMMU_IRQHandler(GFXMMU_HandleTypeDef *hgfxmmu)
{
  uint32_t flags, interrupts, error;
  
  /* Read current flags and interrupts and determine which error occurs */
  flags = hgfxmmu->Instance->SR;
  interrupts = (hgfxmmu->Instance->CR & GFXMMU_CR_ITS_MASK);
  error = (flags & interrupts);
  
  if(error != 0U)
  {
    /* Clear flags on GFXMMU_FCR register */
    hgfxmmu->Instance->FCR = error;
    
    /* Update GFXMMU error code */
    hgfxmmu->ErrorCode |= error;
    
    /* Call GFXMMU error callback */
#if (USE_HAL_GFXMMU_REGISTER_CALLBACKS == 1)
    hgfxmmu->ErrorCallback(hgfxmmu);
#else
    HAL_GFXMMU_ErrorCallback(hgfxmmu);
#endif
  }
}

/**
  * @brief  Error callback. 
  * @param  hgfxmmu GFXMMU handle.
  * @retval None.
  */
__weak void HAL_GFXMMU_ErrorCallback(GFXMMU_HandleTypeDef *hgfxmmu)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hgfxmmu);
  
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_GFXMMU_ErrorCallback could be implemented in the user file.
   */
}

/**
  * @}
  */

/** @defgroup GFXMMU_Exported_Functions_Group3 State functions
 *  @brief    GFXMMU state functions
 *
@verbatim
  ==============================================================================
                         ##### State functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Get GFXMMU handle state.
      (+) Get GFXMMU error code.
@endverbatim
  * @{
  */

/**
  * @brief  This function allows to get the current GFXMMU handle state.
  * @param  hgfxmmu GFXMMU handle.
  * @retval GFXMMU state.
  */
HAL_GFXMMU_StateTypeDef HAL_GFXMMU_GetState(GFXMMU_HandleTypeDef *hgfxmmu)
{
  /* Return GFXMMU handle state */
  return hgfxmmu->State;
}

/**
  * @brief  This function allows to get the current GFXMMU error code.
  * @param  hgfxmmu GFXMMU handle.
  * @retval GFXMMU error code.
  */
uint32_t HAL_GFXMMU_GetError(GFXMMU_HandleTypeDef *hgfxmmu)
{
  uint32_t error_code;
  
  /* Enter in critical section */
  __disable_irq();  
  
  /* Store and reset GFXMMU error code */
  error_code = hgfxmmu->ErrorCode;
  hgfxmmu->ErrorCode = GFXMMU_ERROR_NONE;
  
  /* Exit from critical section */
  __enable_irq();
  
  /* Return GFXMMU error code */
  return error_code;
}

/**
  * @}
  */

/**
  * @}
  */
/* End of exported functions -------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* End of private functions --------------------------------------------------*/

/**
  * @}
  */
#endif /* GFXMMU */
#endif /* HAL_GFXMMU_MODULE_ENABLED */
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
