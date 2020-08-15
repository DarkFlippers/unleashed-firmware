/**
  ******************************************************************************
  * @file    stm32l4xx_hal_cryp.c
  * @author  MCD Application Team
  * @brief   CRYP HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Cryptography (CRYP) peripheral:
  *           + Initialization and de-initialization functions
  *           + Processing functions using polling mode
  *           + Processing functions using interrupt mode
  *           + Processing functions using DMA mode
  *           + Peripheral State functions
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
      The CRYP HAL driver can be used as follows:

      (#)Initialize the CRYP low level resources by implementing the HAL_CRYP_MspInit():
         (++) Enable the CRYP interface clock using __HAL_RCC_AES_CLK_ENABLE()
         (++) In case of using interrupts (e.g. HAL_CRYP_AES_IT())
             (+++) Configure the CRYP interrupt priority using HAL_NVIC_SetPriority()
             (+++) Enable the AES IRQ handler using HAL_NVIC_EnableIRQ()
             (+++) In AES IRQ handler, call HAL_CRYP_IRQHandler()
         (++) In case of using DMA to control data transfer (e.g. HAL_CRYPEx_AES_DMA())
             (+++) Enable the DMA2 interface clock using
                 __HAL_RCC_DMA2_CLK_ENABLE()
             (+++) Configure and enable two DMA channels one for managing data transfer from
                 memory to peripheral (input channel) and another channel for managing data
                 transfer from peripheral to memory (output channel)
             (+++) Associate the initialized DMA handle to the CRYP DMA handle
                 using __HAL_LINKDMA()
             (+++) Configure the priority and enable the NVIC for the transfer complete
                 interrupt on the two DMA channels. The output channel should have higher
                 priority than the input channel.
                 Resort to HAL_NVIC_SetPriority() and HAL_NVIC_EnableIRQ()

      (#)Initialize the CRYP HAL using HAL_CRYP_Init(). This function configures:
         (++) The data type: 1-bit, 8-bit, 16-bit and 32-bit
         (++) The AES operating mode (encryption, key derivation and/or decryption)
         (++) The AES chaining mode (ECB, CBC, CTR, GCM, GMAC, CMAC when applicable, CCM when applicable)
         (++) The encryption/decryption key if so required
         (++) The initialization vector or nonce if applicable (not used in ECB mode).

      (#)Three processing (encryption/decryption) functions are available:
         (++) Polling mode: encryption and decryption APIs are blocking functions
              i.e. they process the data and wait till the processing is finished
         (++) Interrupt mode: encryption and decryption APIs are not blocking functions
              i.e. they process the data under interrupt
         (++) DMA mode: encryption and decryption APIs are not blocking functions
              i.e. the data transfer is ensured by DMA

       (#)Call HAL_CRYP_DeInit() to deinitialize the CRYP peripheral.

     *** Callback registration ***
     ===================================
     [..]
      (#) The compilation define  USE_HAL_CRYP_REGISTER_CALLBACKS when set to 1
          allows the user to configure dynamically the driver callbacks.
          Use function @ref HAL_CRYP_RegisterCallback() to register a user callback.

      (#) Function @ref HAL_CRYP_RegisterCallback() allows to register following callbacks:
            (+) InCpltCallback : callback for input DMA transfer completion.
            (+) OutCpltCallback : callback for output DMA transfer completion.
            (+) CompCpltCallback : callback for computation completion.
            (+) ErrorCallback : callback for error.
            (+) MspInitCallback    : CRYP MspInit.
            (+) MspDeInitCallback  : CRYP MspDeInit.
          This function takes as parameters the HAL peripheral handle, the Callback ID
          and a pointer to the user callback function.

      (#) Use function @ref HAL_CRYP_UnRegisterCallback() to reset a callback to the default
          weak (surcharged) function.
          @ref HAL_CRYP_UnRegisterCallback() takes as parameters the HAL peripheral handle,
          and the Callback ID.
          This function allows to reset following callbacks:
            (+) InCpltCallback : callback for input DMA transfer completion.
            (+) OutCpltCallback : callback for output DMA transfer completion.
            (+) CompCpltCallback : callback for computation completion.
            (+) ErrorCallback : callback for error.
            (+) MspInitCallback    : CRYP MspInit.
            (+) MspDeInitCallback  : CRYP MspDeInit.

      (#) By default, after the @ref HAL_CRYP_Init and if the state is HAL_CRYP_STATE_RESET
          all callbacks are reset to the corresponding legacy weak (surcharged) functions:
          examples @ref HAL_CRYP_InCpltCallback(), @ref HAL_CRYP_ErrorCallback()
          Exception done for MspInit and MspDeInit callbacks that are respectively
          reset to the legacy weak (surcharged) functions in the @ref HAL_CRYP_Init
          and @ref HAL_CRYP_DeInit only when these callbacks are null (not registered beforehand)
          If not, MspInit or MspDeInit are not null, the @ref HAL_CRYP_Init and @ref HAL_CRYP_DeInit
          keep and use the user MspInit/MspDeInit callbacks (registered beforehand).

          Callbacks can be registered/unregistered in READY state only.
          Exception done for MspInit/MspDeInit callbacks that can be registered/unregistered
          in READY or RESET state, thus registered (user) MspInit/DeInit callbacks can be used
          during the Init/DeInit.
          In that case first register the MspInit/MspDeInit user callbacks
          using @ref HAL_CRYP_RegisterCallback before calling @ref HAL_CRYP_DeInit
          or @ref HAL_¨CRYP_Init function.

          When The compilation define USE_HAL_CRYP_REGISTER_CALLBACKS is set to 0 or
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

#ifdef HAL_CRYP_MODULE_ENABLED

#if defined(AES)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup CRYP CRYP
  * @brief CRYP HAL module driver.
  * @{
  */



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions --------------------------------------------------------*/

/** @defgroup CRYP_Private_Functions CRYP Private Functions
  * @{
  */

static HAL_StatusTypeDef CRYP_SetInitVector(CRYP_HandleTypeDef *hcryp);
static HAL_StatusTypeDef CRYP_SetKey(CRYP_HandleTypeDef *hcryp);
static HAL_StatusTypeDef CRYP_AES_IT(CRYP_HandleTypeDef *hcryp);

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup CRYP_Exported_Functions CRYP Exported Functions
  * @{
  */

/** @defgroup CRYP_Exported_Functions_Group1 Initialization and deinitialization functions
 *  @brief    Initialization and Configuration functions.
 *
@verbatim
  ==============================================================================
              ##### Initialization and deinitialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the CRYP according to the specified parameters
          in the CRYP_InitTypeDef and creates the associated handle
      (+) DeInitialize the CRYP peripheral
      (+) Initialize the CRYP MSP (MCU Specific Package)
      (+) De-Initialize the CRYP MSP

    [..]
    (@) Specific care must be taken to format the key and the Initialization Vector IV!

   [..] If the key is defined as a 128-bit long array key[127..0] = {b127 ... b0} where
        b127 is the MSB and b0 the LSB, the key must be stored in MCU memory
        (+) as a sequence of words where the MSB word comes first (occupies the
          lowest memory address)
        (+) where each word is byte-swapped:
         (++)   address n+0 : 0b b103 .. b96 b111 .. b104 b119 .. b112 b127 .. b120
         (++)   address n+4 : 0b b71 .. b64 b79 .. b72 b87 .. b80 b95 .. b88
         (++)   address n+8 : 0b b39 .. b32 b47 .. b40 b55 .. b48 b63 .. b56
         (++)   address n+C : 0b b7 .. b0 b15 .. b8 b23 .. b16 b31 .. b24
    [..] Hereafter, another illustration when considering a 128-bit long key made of 16 bytes {B15..B0}.
        The 4 32-bit words that make the key must be stored as follows in MCU memory:
         (+)    address n+0 : 0x B12 B13 B14 B15
         (+)    address n+4 : 0x B8 B9 B10 B11
         (+)    address n+8 : 0x B4 B5 B6 B7
         (+)    address n+C : 0x B0 B1 B2 B3
    [..]  which leads to the expected setting
      (+)       AES_KEYR3 = 0x B15 B14 B13 B12
      (+)       AES_KEYR2 = 0x B11 B10 B9 B8
      (+)       AES_KEYR1 = 0x B7 B6 B5 B4
      (+)       AES_KEYR0 = 0x B3 B2 B1 B0

   [..]  Same format must be applied for a 256-bit long key made of 32 bytes {B31..B0}.
         The 8 32-bit words that make the key must be stored as follows in MCU memory:
         (+)    address n+00 : 0x B28 B29 B30 B31
         (+)    address n+04 : 0x B24 B25 B26 B27
         (+)    address n+08 : 0x B20 B21 B22 B23
         (+)    address n+0C : 0x B16 B17 B18 B19
         (+)    address n+10 : 0x B12 B13 B14 B15
         (+)    address n+14 : 0x B8 B9 B10 B11
         (+)    address n+18 : 0x B4 B5 B6 B7
         (+)    address n+1C : 0x B0 B1 B2 B3
    [..]  which leads to the expected setting
      (+)       AES_KEYR7 = 0x B31 B30 B29 B28
      (+)       AES_KEYR6 = 0x B27 B26 B25 B24
      (+)       AES_KEYR5 = 0x B23 B22 B21 B20
      (+)       AES_KEYR4 = 0x B19 B18 B17 B16
      (+)       AES_KEYR3 = 0x B15 B14 B13 B12
      (+)       AES_KEYR2 = 0x B11 B10 B9 B8
      (+)       AES_KEYR1 = 0x B7 B6 B5 B4
      (+)       AES_KEYR0 = 0x B3 B2 B1 B0

   [..] Initialization Vector IV (4 32-bit words) format must follow the same as
        that of a 128-bit long key.

  [..]

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the CRYP according to the specified
  *         parameters in the CRYP_InitTypeDef and initialize the associated handle.
  * @note Specific care must be taken to format the key and the Initialization Vector IV
  *       stored in the MCU memory before calling HAL_CRYP_Init(). Refer to explanations
  *       hereabove.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_Init(CRYP_HandleTypeDef *hcryp)
{
  /* Check the CRYP handle allocation */
  if(hcryp == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the instance */
  assert_param(IS_AES_ALL_INSTANCE(hcryp->Instance));

  /* Check the parameters */
  assert_param(IS_CRYP_KEYSIZE(hcryp->Init.KeySize));
  assert_param(IS_CRYP_DATATYPE(hcryp->Init.DataType));
  assert_param(IS_CRYP_ALGOMODE(hcryp->Init.OperatingMode));
  /* ChainingMode parameter is irrelevant when mode is set to Key derivation */
  if (hcryp->Init.OperatingMode != CRYP_ALGOMODE_KEYDERIVATION)
  {
    assert_param(IS_CRYP_CHAINMODE(hcryp->Init.ChainingMode));
  }
  assert_param(IS_CRYP_WRITE(hcryp->Init.KeyWriteFlag));

  /*========================================================*/
  /* Check the proper operating/chaining modes combinations */
  /*========================================================*/
  /* Check the proper chaining when the operating mode is key derivation and decryption */
#if defined(AES_CR_NPBLB)
  if ((hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION_DECRYPT) &&\
         ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CTR)           \
       || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)      \
       || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM)))
#else
  if ((hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION_DECRYPT) &&\
         ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CTR)           \
       || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)      \
       || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)))
#endif
  {
    return HAL_ERROR;
  }
  /* Check that key derivation is not set in CMAC mode or CCM mode when applicable */
#if defined(AES_CR_NPBLB)
  if ((hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION)
   && (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM))
#else
  if ((hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION)
   && (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC))
#endif
  {
    return HAL_ERROR;
  }


  /*================*/
  /* Initialization */
  /*================*/
  /* Initialization start */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
  if (hcryp->State == HAL_CRYP_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hcryp->Lock = HAL_UNLOCKED;

    /* Reset Callback pointers in HAL_CRYP_STATE_RESET only */
    hcryp->InCpltCallback   =  HAL_CRYP_InCpltCallback;            /* Legacy weak (surcharged) input DMA transfer completion callback */
    hcryp->OutCpltCallback   =  HAL_CRYP_OutCpltCallback;           /* Legacy weak (surcharged) output DMA transfer completion callback */
    hcryp->CompCpltCallback =  HAL_CRYPEx_ComputationCpltCallback; /* Legacy weak (surcharged) computation completion callback */
    hcryp->ErrorCallback    =  HAL_CRYP_ErrorCallback;             /* Legacy weak (surcharged) error callback */
    if(hcryp->MspInitCallback == NULL)
    {
      hcryp->MspInitCallback = HAL_CRYP_MspInit;
    }

    /* Init the low level hardware */
    hcryp->MspInitCallback(hcryp);
  }
#else
  if(hcryp->State == HAL_CRYP_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hcryp->Lock = HAL_UNLOCKED;

    /* Init the low level hardware */
    HAL_CRYP_MspInit(hcryp);
  }
#endif /* (USE_HAL_CRYP_REGISTER_CALLBACKS) */

  /* Change the CRYP state */
  hcryp->State = HAL_CRYP_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_CRYP_DISABLE(hcryp);

  /*=============================================================*/
  /* AES initialization common to all operating modes            */
  /*=============================================================*/
  /* Set the Key size selection */
  MODIFY_REG(hcryp->Instance->CR, AES_CR_KEYSIZE, hcryp->Init.KeySize);

  /* Set the default CRYP phase when this parameter is not used.
     Phase is updated below in case of GCM/GMAC(/CMAC)(/CCM) setting. */
  hcryp->Phase = HAL_CRYP_PHASE_NOT_USED;



  /*=============================================================*/
  /* Carry on the initialization based on the AES operating mode */
  /*=============================================================*/
  /* Key derivation */
  if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION)
  {
    MODIFY_REG(hcryp->Instance->CR, AES_CR_MODE, CRYP_ALGOMODE_KEYDERIVATION);

    /* Configure the Key registers */
    if (CRYP_SetKey(hcryp) != HAL_OK)
    {
      return HAL_ERROR;
    }
  }
  else
  /* Encryption / Decryption (with or without key derivation) / authentication */
  {
#if !defined(AES_CR_NPBLB)
    /* Set data type, operating and chaining modes.
       In case of GCM or GMAC, data type is forced to 0b00 */
    if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
    {
      MODIFY_REG(hcryp->Instance->CR, AES_CR_DATATYPE|AES_CR_MODE|AES_CR_CHMOD, hcryp->Init.OperatingMode|hcryp->Init.ChainingMode);
    }
    else
#endif
    {
      MODIFY_REG(hcryp->Instance->CR, AES_CR_DATATYPE|AES_CR_MODE|AES_CR_CHMOD, hcryp->Init.DataType|hcryp->Init.OperatingMode|hcryp->Init.ChainingMode);
    }


   /* Specify the encryption/decryption phase in case of Galois counter mode (GCM),
      Galois message authentication code (GMAC), cipher message authentication code (CMAC) when applicable
      or Counter with Cipher Mode (CCM) when applicable */
#if defined(AES_CR_NPBLB)
   if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
    || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM))
#else
   if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
    || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC))
#endif
    {
      MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, hcryp->Init.GCMCMACPhase);
      hcryp->Phase = HAL_CRYP_PHASE_START;
    }


    /* Configure the Key registers if no need to bypass this step */
    if (hcryp->Init.KeyWriteFlag == CRYP_KEY_WRITE_ENABLE)
    {
      if (CRYP_SetKey(hcryp) != HAL_OK)
      {
        return HAL_ERROR;
      }
    }

    /* If applicable, configure the Initialization Vector */
    if (hcryp->Init.ChainingMode != CRYP_CHAINMODE_AES_ECB)
    {
      if (CRYP_SetInitVector(hcryp) != HAL_OK)
      {
        return HAL_ERROR;
      }
    }
  }

#if defined(AES_CR_NPBLB)
  /* Clear NPBLB field */
  CLEAR_BIT(hcryp->Instance->CR, AES_CR_NPBLB);
#endif

  /* Reset CrypInCount and CrypOutCount */
  hcryp->CrypInCount = 0;
  hcryp->CrypOutCount = 0;

  /* Reset ErrorCode field */
  hcryp->ErrorCode = HAL_CRYP_ERROR_NONE;

  /* Reset Mode suspension request */
  hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;

  /* Change the CRYP state */
  hcryp->State = HAL_CRYP_STATE_READY;

  /* Enable the Peripheral */
  __HAL_CRYP_ENABLE(hcryp);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  DeInitialize the CRYP peripheral.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_DeInit(CRYP_HandleTypeDef *hcryp)
{
  /* Check the CRYP handle allocation */
  if(hcryp == NULL)
  {
    return HAL_ERROR;
  }

  /* Change the CRYP state */
  hcryp->State = HAL_CRYP_STATE_BUSY;

  /* Set the default CRYP phase */
  hcryp->Phase = HAL_CRYP_PHASE_READY;

  /* Reset CrypInCount and CrypOutCount */
  hcryp->CrypInCount = 0;
  hcryp->CrypOutCount = 0;

  /* Disable the CRYP Peripheral Clock */
  __HAL_CRYP_DISABLE(hcryp);

#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
    if(hcryp->MspDeInitCallback == NULL)
    {
      hcryp->MspDeInitCallback = HAL_CRYP_MspDeInit;
    }

    /* DeInit the low level hardware */
    hcryp->MspDeInitCallback(hcryp);
#else
  /* DeInit the low level hardware: CLOCK, NVIC.*/
  HAL_CRYP_MspDeInit(hcryp);
#endif /* (USE_HAL_CRYP_REGISTER_CALLBACKS) */

  /* Change the CRYP state */
  hcryp->State = HAL_CRYP_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hcryp);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the CRYP MSP.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_MspInit(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CRYP_MspInit can be implemented in the user file
   */
}

/**
  * @brief  DeInitialize CRYP MSP.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CRYP_MspDeInit can be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup CRYP_Exported_Functions_Group2 AES processing functions
 *  @brief   Processing functions.
 *
@verbatim
  ==============================================================================
                      ##### AES processing functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Encrypt plaintext using AES algorithm in different chaining modes
      (+) Decrypt cyphertext using AES algorithm in different chaining modes
    [..]  Three processing functions are available:
      (+) Polling mode
      (+) Interrupt mode
      (+) DMA mode

@endverbatim
  * @{
  */


/**
  * @brief  Encrypt pPlainData in AES ECB encryption mode. The cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Timeout: Specify Timeout value
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Encrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData, uint32_t Timeout)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_ECB;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES(hcryp, pPlainData, Size, pCypherData, Timeout);
}


/**
  * @brief  Encrypt pPlainData in AES CBC encryption mode with key derivation. The cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Timeout: Specify Timeout value
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Encrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData, uint32_t Timeout)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CBC;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES(hcryp, pPlainData, Size, pCypherData, Timeout);
}


/**
  * @brief  Encrypt pPlainData in AES CTR encryption mode. The cypher data are available in pCypherData
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Timeout: Specify Timeout value
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Encrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData, uint32_t Timeout)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CTR;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES(hcryp, pPlainData, Size, pCypherData, Timeout);
}

/**
  * @brief  Decrypt pCypherData in AES ECB decryption mode with key derivation,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Timeout: Specify Timeout value
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Decrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData, uint32_t Timeout)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_KEYDERIVATION_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_ECB;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES(hcryp, pCypherData, Size, pPlainData, Timeout);
}

/**
  * @brief  Decrypt pCypherData in AES ECB decryption mode with key derivation,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Timeout: Specify Timeout value
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Decrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData, uint32_t Timeout)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_KEYDERIVATION_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CBC;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES(hcryp, pCypherData, Size, pPlainData, Timeout);
}

/**
  * @brief  Decrypt pCypherData in AES CTR decryption mode,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Timeout: Specify Timeout value
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Decrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData, uint32_t Timeout)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CTR;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES(hcryp, pCypherData, Size, pPlainData, Timeout);
}

/**
  * @brief  Encrypt pPlainData in AES ECB encryption mode using Interrupt,
  *         the cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_IT() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Encrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_ECB;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_IT(hcryp, pPlainData, Size, pCypherData);
}

/**
  * @brief  Encrypt pPlainData in AES CBC encryption mode using Interrupt,
  *         the cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_IT() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Encrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CBC;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_IT(hcryp, pPlainData, Size, pCypherData);
}


/**
  * @brief  Encrypt pPlainData in AES CTR encryption mode using Interrupt,
  *         the cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_IT() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Encrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CTR;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_IT(hcryp, pPlainData, Size, pCypherData);
}

/**
  * @brief  Decrypt pCypherData in AES ECB decryption mode using Interrupt,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer.
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_IT() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Decrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_KEYDERIVATION_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_ECB;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_IT(hcryp, pCypherData, Size, pPlainData);
}

/**
  * @brief  Decrypt pCypherData in AES CBC decryption mode using Interrupt,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_IT() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Decrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_KEYDERIVATION_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CBC;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_IT(hcryp, pCypherData, Size, pPlainData);
}

/**
  * @brief  Decrypt pCypherData in AES CTR decryption mode using Interrupt,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_IT() API instead (usage recommended).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Decrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CTR;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_IT(hcryp, pCypherData, Size, pPlainData);
}

/**
  * @brief  Encrypt pPlainData in AES ECB encryption mode using DMA,
  *         the cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_DMA() API instead (usage recommended).
  * @note   pPlainData and pCypherData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Encrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_ECB;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_DMA(hcryp, pPlainData, Size, pCypherData);
}



/**
  * @brief  Encrypt pPlainData in AES CBC encryption mode using DMA,
  *         the cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_DMA() API instead (usage recommended).
  * @note   pPlainData and pCypherData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Encrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CBC;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_DMA(hcryp, pPlainData, Size, pCypherData);
}

/**
  * @brief  Encrypt pPlainData in AES CTR encryption mode using DMA,
  *         the cypher data are available in pCypherData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData: Pointer to the plaintext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pCypherData: Pointer to the cyphertext buffer.
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_DMA() API instead (usage recommended).
  * @note   pPlainData and pCypherData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Encrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CTR;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_DMA(hcryp, pPlainData, Size, pCypherData);
}

/**
  * @brief  Decrypt pCypherData in AES ECB decryption mode using DMA,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_DMA() API instead (usage recommended).
  * @note   pPlainData and pCypherData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Decrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_KEYDERIVATION_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_ECB;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_DMA(hcryp, pCypherData, Size, pPlainData);
}

/**
  * @brief  Decrypt pCypherData in AES CBC decryption mode using DMA,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_DMA() API instead (usage recommended).
  * @note   pPlainData and pCypherData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Decrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_KEYDERIVATION_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CBC;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_DMA(hcryp, pCypherData, Size, pPlainData);
}

/**
  * @brief  Decrypt pCypherData in AES CTR decryption mode using DMA,
  *         the decyphered data are available in pPlainData.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData: Pointer to the cyphertext buffer
  * @param  Size: Length of the plaintext buffer in bytes, must be a multiple of 16.
  * @param  pPlainData: Pointer to the plaintext buffer
  * @note   This API is provided only to maintain compatibility with legacy software. Users should directly
  *         resort to generic HAL_CRYPEx_AES_DMA() API instead (usage recommended).
  * @note   pPlainData and pCypherData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Decrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  /* Re-initialize AES IP with proper parameters */
  if (HAL_CRYP_DeInit(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }
  hcryp->Init.OperatingMode = CRYP_ALGOMODE_DECRYPT;
  hcryp->Init.ChainingMode = CRYP_CHAINMODE_AES_CTR;
  hcryp->Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  if (HAL_CRYP_Init(hcryp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_CRYPEx_AES_DMA(hcryp, pCypherData, Size, pPlainData);
}


/**
  * @}
  */

/** @defgroup CRYP_Exported_Functions_Group3 Callback functions
 *  @brief   Callback functions.
 *
@verbatim
  ==============================================================================
                      ##### Callback functions  #####
  ==============================================================================
    [..]  This section provides Interruption and DMA callback functions:
      (+) DMA Input data transfer complete
      (+) DMA Output data transfer complete
      (+) DMA or Interrupt error

@endverbatim
  * @{
  */

/**
  * @brief  CRYP error callback.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_ErrorCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CRYP_ErrorCallback can be implemented in the user file
   */
}

/**
  * @brief  Input DMA transfer complete callback.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_InCpltCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CRYP_InCpltCallback can be implemented in the user file
   */
}

/**
  * @brief  Output DMA transfer complete callback.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_OutCpltCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CRYP_OutCpltCallback can be implemented in the user file
   */
}

#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User CRYP Callback
  *         To be used instead of the weak (surcharged) predefined callback
  * @param hcryp CRYP handle
  * @param CallbackID ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_CRYP_INPUTCPLT_CB_ID CRYP input DMA transfer completion Callback ID
  *          @arg @ref HAL_CRYP_OUTPUTCPLT_CB_ID CRYP output DMA transfer completion Callback ID
  *          @arg @ref HAL_CRYP_COMPCPLT_CB_ID CRYP computation completion Callback ID
  *          @arg @ref HAL_CRYP_ERROR_CB_ID CRYP error callback ID
  *          @arg @ref HAL_CRYP_MSPINIT_CB_ID CRYP MspDeInit callback ID
  *          @arg @ref HAL_CRYP_MSPDEINIT_CB_ID CRYP MspDeInit callback ID
  * @param pCallback pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_CRYP_RegisterCallback(CRYP_HandleTypeDef *hcryp, HAL_CRYP_CallbackIDTypeDef CallbackID, pCRYP_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    /* Update the error code */
    hcryp->ErrorCode |= HAL_CRYP_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }
  /* Process locked */
  __HAL_LOCK(hcryp);

  if(HAL_CRYP_STATE_READY == hcryp->State)
  {
    switch (CallbackID)
    {
    case HAL_CRYP_INPUTCPLT_CB_ID :
      hcryp->InCpltCallback = pCallback;
      break;

    case HAL_CRYP_OUTPUTCPLT_CB_ID :
      hcryp->OutCpltCallback = pCallback;
      break;

    case HAL_CRYP_COMPCPLT_CB_ID :
      hcryp->CompCpltCallback = pCallback;
      break;

    case HAL_CRYP_ERROR_CB_ID :
      hcryp->ErrorCallback = pCallback;
      break;

    case HAL_CRYP_MSPINIT_CB_ID :
      hcryp->MspInitCallback = pCallback;
      break;

    case HAL_CRYP_MSPDEINIT_CB_ID :
      hcryp->MspDeInitCallback = pCallback;
      break;

    default :
     /* Update the error code */
     hcryp->ErrorCode |= HAL_CRYP_ERROR_INVALID_CALLBACK;
     /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if(HAL_CRYP_STATE_RESET == hcryp->State)
  {
    switch (CallbackID)
    {
    case HAL_CRYP_MSPINIT_CB_ID :
      hcryp->MspInitCallback = pCallback;
      break;

    case HAL_CRYP_MSPDEINIT_CB_ID :
      hcryp->MspDeInitCallback = pCallback;
      break;

    default :
     /* Update the error code */
     hcryp->ErrorCode |= HAL_CRYP_ERROR_INVALID_CALLBACK;
     /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
     hcryp->ErrorCode |= HAL_CRYP_ERROR_INVALID_CALLBACK;
     /* update return status */
      status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hcryp);
  return status;
}

/**
  * @brief  Unregister a CRYP Callback
  *         CRYP Callback is redirected to the weak (surcharged) predefined callback
  * @param hcryp CRYP handle
  * @param CallbackID ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_CRYP_INPUTCPLT_CB_ID CRYP input DMA transfer completion Callback ID
  *          @arg @ref HAL_CRYP_OUTPUTCPLT_CB_ID CRYP output DMA transfer completion Callback ID
  *          @arg @ref HAL_CRYP_COMPCPLT_CB_ID CRYP computation completion Callback ID
  *          @arg @ref HAL_CRYP_ERROR_CB_ID CRYP error callback ID
  *          @arg @ref HAL_CRYP_MSPINIT_CB_ID CRYP MspDeInit callback ID
  *          @arg @ref HAL_CRYP_MSPDEINIT_CB_ID CRYP MspDeInit callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_CRYP_UnRegisterCallback(CRYP_HandleTypeDef *hcryp, HAL_CRYP_CallbackIDTypeDef CallbackID)
{
HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hcryp);

  if(HAL_CRYP_STATE_READY == hcryp->State)
  {
    switch (CallbackID)
    {
    case HAL_CRYP_INPUTCPLT_CB_ID :
      hcryp->InCpltCallback = HAL_CRYP_InCpltCallback;              /* Legacy weak (surcharged) input DMA transfer completion callback */
      break;

    case HAL_CRYP_OUTPUTCPLT_CB_ID :
      hcryp->OutCpltCallback = HAL_CRYP_OutCpltCallback;            /* Legacy weak (surcharged) output DMA transfer completion callback */
      break;

    case HAL_CRYP_COMPCPLT_CB_ID :
      hcryp->CompCpltCallback = HAL_CRYPEx_ComputationCpltCallback; /* Legacy weak (surcharged) computation completion callback */
      break;

    case HAL_CRYP_ERROR_CB_ID :
      hcryp->ErrorCallback = HAL_CRYP_ErrorCallback;                /* Legacy weak (surcharged) error callback */
      break;

    case HAL_CRYP_MSPINIT_CB_ID :
      hcryp->MspInitCallback = HAL_CRYP_MspInit;                    /* Legacy weak (surcharged) Msp DeInit */
      break;

    case HAL_CRYP_MSPDEINIT_CB_ID :
      hcryp->MspDeInitCallback = HAL_CRYP_MspDeInit;                /* Legacy weak (surcharged) Msp DeInit */
      break;

    default :
     /* Update the error code */
     hcryp->ErrorCode |= HAL_CRYP_ERROR_INVALID_CALLBACK;
     /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if(HAL_CRYP_STATE_RESET == hcryp->State)
  {
    switch (CallbackID)
    {
    case HAL_CRYP_MSPINIT_CB_ID :
      hcryp->MspInitCallback = HAL_CRYP_MspInit;           /* Legacy weak (surcharged) Msp Init */
      break;

    case HAL_CRYP_MSPDEINIT_CB_ID :
      hcryp->MspDeInitCallback = HAL_CRYP_MspDeInit;       /* Legacy weak (surcharged) Msp DeInit */
      break;

    default :
     /* Update the error code */
     hcryp->ErrorCode |= HAL_CRYP_ERROR_INVALID_CALLBACK;
     /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
     /* Update the error code */
     hcryp->ErrorCode |= HAL_CRYP_ERROR_INVALID_CALLBACK;
     /* update return status */
      status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hcryp);
  return status;
}
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup CRYP_Exported_Functions_Group4 CRYP IRQ handler
 *  @brief   AES IRQ handler.
 *
@verbatim
  ==============================================================================
                ##### AES IRQ handler management #####
  ==============================================================================
[..]  This section provides AES IRQ handler function.

@endverbatim
  * @{
  */

/**
  * @brief  Handle AES interrupt request.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
void HAL_CRYP_IRQHandler(CRYP_HandleTypeDef *hcryp)
{
  /* Check if error occurred */
  if (__HAL_CRYP_GET_IT_SOURCE(hcryp, CRYP_IT_ERRIE) != RESET)
  {
    /* If Write Error occurred */
    if (__HAL_CRYP_GET_FLAG(hcryp, CRYP_IT_WRERR) != RESET)
    {
      hcryp->ErrorCode |= HAL_CRYP_WRITE_ERROR;
      hcryp->State = HAL_CRYP_STATE_ERROR;
    }
    /* If Read Error occurred */
    if (__HAL_CRYP_GET_FLAG(hcryp, CRYP_IT_RDERR) != RESET)
    {
      hcryp->ErrorCode |= HAL_CRYP_READ_ERROR;
      hcryp->State = HAL_CRYP_STATE_ERROR;
    }

    /* If an error has been reported */
    if (hcryp->State == HAL_CRYP_STATE_ERROR)
    {
      /* Disable Error and Computation Complete Interrupts */
      __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
      /* Clear all Interrupt flags */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_ERR_CLEAR|CRYP_CCF_CLEAR);

      /* Process Unlocked */
      __HAL_UNLOCK(hcryp);

#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
      hcryp->ErrorCallback(hcryp);
#else
      HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */

      return;
    }

  }

  /* Check if computation complete interrupt is enabled
     and if the computation complete flag is raised */
  if (__HAL_CRYP_GET_FLAG(hcryp, CRYP_IT_CCF) != RESET)
  {
    if (__HAL_CRYP_GET_IT_SOURCE(hcryp, CRYP_IT_CCFIE) != RESET)
    {
#if defined(AES_CR_NPBLB)
      if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
       || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM))
#else
      if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
       || (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC))
#endif
      {
       /* To ensure proper suspension requests management, CCF flag
          is reset in CRYP_AES_Auth_IT() according to the current
          phase under handling */
        if (CRYP_AES_Auth_IT(hcryp) !=  HAL_OK)
        {
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
          hcryp->ErrorCallback(hcryp);
#else
          HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
        }
      }
      else
      {
        /* Clear Computation Complete Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
        if (CRYP_AES_IT(hcryp) !=  HAL_OK)
        {
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
          hcryp->ErrorCallback(hcryp);
#else
          HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
        }
      }
    }
  }
}

/**
  * @}
  */

/** @defgroup CRYP_Exported_Functions_Group5 Peripheral State functions
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
  * @brief  Return the CRYP handle state.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval HAL state
  */
HAL_CRYP_STATETypeDef HAL_CRYP_GetState(CRYP_HandleTypeDef *hcryp)
{
  /* Return CRYP handle state */
  return hcryp->State;
}

/**
  * @brief  Return the CRYP peripheral error.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @note   The returned error is a bit-map combination of possible errors
  * @retval Error bit-map
  */
uint32_t HAL_CRYP_GetError(CRYP_HandleTypeDef *hcryp)
{
  return hcryp->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup CRYP_Private_Functions
  * @{
  */


/**
  * @brief  Write the Key in KeyRx registers.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
static HAL_StatusTypeDef  CRYP_SetKey(CRYP_HandleTypeDef *hcryp)
{
  uint32_t keyaddr;

  if (hcryp->Init.pKey == NULL)
  {
    return HAL_ERROR;
  }


  keyaddr = (uint32_t)(hcryp->Init.pKey);

  if (hcryp->Init.KeySize == CRYP_KEYSIZE_256B)
  {
    hcryp->Instance->KEYR7 = __REV(*(uint32_t*)(keyaddr));
    keyaddr+=4U;
    hcryp->Instance->KEYR6 = __REV(*(uint32_t*)(keyaddr));
    keyaddr+=4U;
    hcryp->Instance->KEYR5 = __REV(*(uint32_t*)(keyaddr));
    keyaddr+=4U;
    hcryp->Instance->KEYR4 = __REV(*(uint32_t*)(keyaddr));
    keyaddr+=4U;
  }

  hcryp->Instance->KEYR3 = __REV(*(uint32_t*)(keyaddr));
  keyaddr+=4U;
  hcryp->Instance->KEYR2 = __REV(*(uint32_t*)(keyaddr));
  keyaddr+=4U;
  hcryp->Instance->KEYR1 = __REV(*(uint32_t*)(keyaddr));
  keyaddr+=4U;
  hcryp->Instance->KEYR0 = __REV(*(uint32_t*)(keyaddr));

  return HAL_OK;
}

/**
  * @brief  Write the InitVector/InitCounter in IVRx registers.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
static HAL_StatusTypeDef CRYP_SetInitVector(CRYP_HandleTypeDef *hcryp)
{
  uint32_t ivaddr;

#if !defined(AES_CR_NPBLB)
  if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
  {
    hcryp->Instance->IVR3 = 0;
    hcryp->Instance->IVR2 = 0;
    hcryp->Instance->IVR1 = 0;
    hcryp->Instance->IVR0 = 0;
  }
  else
#endif
  {
    if (hcryp->Init.pInitVect == NULL)
    {
      return HAL_ERROR;
    }

    ivaddr = (uint32_t)(hcryp->Init.pInitVect);

    hcryp->Instance->IVR3 = __REV(*(uint32_t*)(ivaddr));
    ivaddr+=4U;
    hcryp->Instance->IVR2 = __REV(*(uint32_t*)(ivaddr));
    ivaddr+=4U;
    hcryp->Instance->IVR1 = __REV(*(uint32_t*)(ivaddr));
    ivaddr+=4U;
    hcryp->Instance->IVR0 = __REV(*(uint32_t*)(ivaddr));
  }
  return HAL_OK;
}



/**
  * @brief  Handle CRYP block input/output data handling under interruption.
  * @note   The function is called under interruption only, once
  *         interruptions have been enabled by HAL_CRYPEx_AES_IT().
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @retval HAL status
  */
static HAL_StatusTypeDef CRYP_AES_IT(CRYP_HandleTypeDef *hcryp)
{
  uint32_t inputaddr;
  uint32_t outputaddr = (uint32_t)hcryp->pCrypOutBuffPtr;

  if(hcryp->State == HAL_CRYP_STATE_BUSY)
  {
    if (hcryp->Init.OperatingMode != CRYP_ALGOMODE_KEYDERIVATION)
    {
      /* Read the last available output block from the Data Output Register */
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
      outputaddr+=4U;
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
      outputaddr+=4U;
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
      outputaddr+=4U;
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
      hcryp->pCrypOutBuffPtr += 16;
      hcryp->CrypOutCount -= 16U;

    }
    else
    {
      /* Read the derived key from the Key registers */
      if (hcryp->Init.KeySize == CRYP_KEYSIZE_256B)
      {
        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR7);
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR6);
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR5);
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR4);
        outputaddr+=4U;
      }

        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR3);
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR2);
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR1);
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->KEYR0);
    }

    /* In case of ciphering or deciphering, check if all output text has been retrieved;
       In case of key derivation, stop right there */
    if ((hcryp->CrypOutCount == 0U) || (hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION))
    {
      /* Disable Computation Complete Flag and Errors Interrupts */
      __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
      /* Change the CRYP state */
      hcryp->State = HAL_CRYP_STATE_READY;

     /* Process Unlocked */
      __HAL_UNLOCK(hcryp);

      /* Call computation complete callback */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
      hcryp->CompCpltCallback(hcryp);
#else
      HAL_CRYPEx_ComputationCpltCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */

      return HAL_OK;
    }
    /* If suspension flag has been raised, suspend processing */
    else if (hcryp->SuspendRequest == HAL_CRYP_SUSPEND)
    {
      /* reset ModeSuspend */
      hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;

      /* Disable Computation Complete Flag and Errors Interrupts */
      __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
      /* Change the CRYP state */
      hcryp->State = HAL_CRYP_STATE_SUSPENDED;

     /* Process Unlocked */
      __HAL_UNLOCK(hcryp);

      return HAL_OK;
    }
    else /* Process the rest of input data */
    {
      /* Get the Intput data address */
      inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;

      /* Increment/decrement instance pointer/counter */
      hcryp->pCrypInBuffPtr += 16;
      hcryp->CrypInCount -= 16U;

      /* Write the next input block in the Data Input register */
      hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      inputaddr+=4U;
      hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      inputaddr+=4U;
      hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
      inputaddr+=4U;
      hcryp->Instance->DINR = *(uint32_t*)(inputaddr);

      return HAL_OK;
    }
  }
  else
  {
    return HAL_BUSY;
  }
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

#endif /* AES */

#endif /* HAL_CRYP_MODULE_ENABLED */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
