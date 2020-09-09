/**
  ******************************************************************************
  * @file    stm32l4xx_hal_cryp_ex.c
  * @author  MCD Application Team
  * @brief   CRYPEx HAL module driver.
  *          This file provides firmware functions to manage the extended
  *          functionalities of the Cryptography (CRYP) peripheral.
  *
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

/** @defgroup CRYPEx CRYPEx
  * @brief CRYP Extended HAL module driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup CRYPEx_Private_Constants CRYPEx Private Constants
  * @{
  */
#define CRYP_CCF_TIMEOUTVALUE                      22000  /*!< CCF flag raising time-out value */
#define CRYP_BUSY_TIMEOUTVALUE                     22000  /*!< BUSY flag reset time-out value  */

#define CRYP_POLLING_OFF                             0x0  /*!< No polling when padding */
#define CRYP_POLLING_ON                              0x1  /*!< Polling when padding    */

#if defined(AES_CR_NPBLB)
#define AES_POSITION_CR_NPBLB     (uint32_t)POSITION_VAL(AES_CR_NPBLB)    /*!< Required left shift to set background CLUT size */
#endif
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @defgroup CRYPEx_Private_Functions CRYPEx Private Functions
 * @{
 */
static HAL_StatusTypeDef CRYP_ProcessData(CRYP_HandleTypeDef *hcryp, uint8_t* Input, uint16_t Ilength, uint8_t* Output, uint32_t Timeout);
static HAL_StatusTypeDef CRYP_ReadKey(CRYP_HandleTypeDef *hcryp, uint8_t* Output, uint32_t Timeout);
static void CRYP_SetDMAConfig(CRYP_HandleTypeDef *hcryp, uint32_t inputaddr, uint16_t Size, uint32_t outputaddr);
static void CRYP_Authentication_SetDMAConfig(CRYP_HandleTypeDef *hcryp, uint32_t inputaddr, uint16_t Size, uint32_t outputaddr);
static void CRYP_Authentication_DMAInCplt(DMA_HandleTypeDef *hdma);
static void CRYP_Authentication_DMAError(DMA_HandleTypeDef *hdma);
static void CRYP_Authentication_DMAOutCplt(DMA_HandleTypeDef *hdma);
static HAL_StatusTypeDef CRYP_WaitOnCCFlag(CRYP_HandleTypeDef const * const hcryp, uint32_t Timeout);
static HAL_StatusTypeDef CRYP_WaitOnBusyFlagReset(CRYP_HandleTypeDef const * const hcryp, uint32_t Timeout);
static void CRYP_DMAInCplt(DMA_HandleTypeDef *hdma);
static void CRYP_DMAOutCplt(DMA_HandleTypeDef *hdma);
static void CRYP_DMAError(DMA_HandleTypeDef *hdma);
static void CRYP_Padding(CRYP_HandleTypeDef *hcryp, uint32_t difflength, uint32_t polling);
/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup CRYPEx_Exported_Functions CRYPEx Exported Functions
  * @{
  */


/** @defgroup CRYPEx_Exported_Functions_Group1 Extended callback function
 *  @brief    Extended callback functions.
 *
@verbatim
 ===============================================================================
                 ##### Extended callback functions #####
 ===============================================================================
    [..]  This section provides callback function:
      (+) Computation completed.

@endverbatim
  * @{
  */


/**
  * @brief  Computation completed callbacks.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYPEx_ComputationCpltCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CRYPEx_ComputationCpltCallback can be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup CRYPEx_Exported_Functions_Group2 AES extended processing functions
 *  @brief   Extended processing functions.
 *
@verbatim
  ==============================================================================
                      ##### AES extended processing functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Encrypt plaintext or decrypt cipher text using AES algorithm in different chaining modes.
          Functions are generic (handles ECB, CBC and CTR and all modes) and are only differentiated
          based on the processing type. Three processing types are available:
          (++) Polling mode
          (++) Interrupt mode
          (++) DMA mode
      (+) Generate and authentication tag in addition to encrypt/decrypt a plain/cipher text using AES
          algorithm in different chaining modes.
          Functions are generic (handles GCM, GMAC, CMAC and CCM when applicable) and process only one phase
          so that steps can be skipped if so required. Functions are only differentiated based on the processing type.
          Three processing types are available:
          (++) Polling mode
          (++) Interrupt mode
          (++) DMA mode

@endverbatim
  * @{
  */

/**
  * @brief  Carry out in polling mode the ciphering or deciphering operation according to
  *         hcryp->Init structure fields, all operating modes (encryption, key derivation and/or decryption) and
  *         chaining modes ECB, CBC and CTR are managed by this function in polling mode.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pInputData: Pointer to the plain text in case of encryption or cipher text in case of decryption
  *                     or key derivation+decryption.
  *                     Parameter is meaningless in case of key derivation.
  * @param  Size: Length of the input data buffer in bytes, must be a multiple of 16.
  *               Parameter is meaningless in case of key derivation.
  * @param  pOutputData: Pointer to the cipher text in case of encryption or plain text in case of
  *                     decryption/key derivation+decryption, or pointer to the derivative keys in
  *                     case of key derivation only.
  * @param  Timeout: Specify Timeout value
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYPEx_AES(CRYP_HandleTypeDef *hcryp, uint8_t *pInputData, uint16_t Size, uint8_t *pOutputData, uint32_t Timeout)
{

  if (hcryp->State == HAL_CRYP_STATE_READY)
  {
    /* Check parameters setting */
    if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION)
    {
      if (pOutputData == NULL)
      {
        return  HAL_ERROR;
      }
    }
    else
    {
      if ((pInputData == NULL) || (pOutputData == NULL) || (Size == 0U))
      {
        return  HAL_ERROR;
      }
    }

    /* Process Locked */
    __HAL_LOCK(hcryp);

    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;

    /* Call CRYP_ReadKey() API if the operating mode is set to
       key derivation, CRYP_ProcessData() otherwise  */
    if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION)
    {
      if(CRYP_ReadKey(hcryp, pOutputData, Timeout) != HAL_OK)
      {
        return HAL_TIMEOUT;
      }
    }
    else
    {
      if(CRYP_ProcessData(hcryp, pInputData, Size, pOutputData, Timeout) != HAL_OK)
      {
        return HAL_TIMEOUT;
      }
    }

    /* If the state has not been set to SUSPENDED, set it to
       READY, otherwise keep it as it is */
    if (hcryp->State != HAL_CRYP_STATE_SUSPENDED)
    {
      hcryp->State = HAL_CRYP_STATE_READY;
    }

    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}



/**
  * @brief  Carry out in interrupt mode the ciphering or deciphering operation according to
  *         hcryp->Init structure fields, all operating modes (encryption, key derivation and/or decryption) and
  *         chaining modes ECB, CBC and CTR are managed by this function in interrupt mode.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pInputData: Pointer to the plain text in case of encryption or cipher text in case of decryption
  *                     or key derivation+decryption.
  *                     Parameter is meaningless in case of key derivation.
  * @param  Size: Length of the input data buffer in bytes, must be a multiple of 16.
  *               Parameter is meaningless in case of key derivation.
  * @param  pOutputData: Pointer to the cipher text in case of encryption or plain text in case of
  *                     decryption/key derivation+decryption, or pointer to the derivative keys in
  *                     case of key derivation only.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYPEx_AES_IT(CRYP_HandleTypeDef *hcryp,  uint8_t *pInputData, uint16_t Size, uint8_t *pOutputData)
{
  uint32_t inputaddr;

  if(hcryp->State == HAL_CRYP_STATE_READY)
  {
    /* Check parameters setting */
    if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION)
    {
      if (pOutputData == NULL)
      {
        return  HAL_ERROR;
      }
    }
    else
    {
      if ((pInputData == NULL) || (pOutputData == NULL) || (Size == 0U))
      {
        return  HAL_ERROR;
      }
    }
    /* Process Locked */
    __HAL_LOCK(hcryp);

    /* If operating mode is not limited to key derivation only,
       get the buffers addresses and sizes */
    if (hcryp->Init.OperatingMode != CRYP_ALGOMODE_KEYDERIVATION)
    {

      hcryp->CrypInCount = Size;
      hcryp->pCrypInBuffPtr = pInputData;
      hcryp->pCrypOutBuffPtr = pOutputData;
      hcryp->CrypOutCount = Size;
    }
    else
    {
      /* For key derivation, set output buffer only
        (will point at derivated key) */
      hcryp->pCrypOutBuffPtr = pOutputData;
    }

    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;

      /* Process Unlocked */
    __HAL_UNLOCK(hcryp);

    /* Enable Computation Complete Flag and Error Interrupts */
    __HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);


    /* If operating mode is key derivation only, the input data have
       already been entered during the initialization process. For
       the other operating modes, they are fed to the CRYP hardware
       block at this point. */
    if (hcryp->Init.OperatingMode != CRYP_ALGOMODE_KEYDERIVATION)
    {
      /* Initiate the processing under interrupt in entering
         the first input data */
      inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
      /* Increment/decrement instance pointer/counter */
      hcryp->pCrypInBuffPtr += 16;
      hcryp->CrypInCount -= 16U;
      /* Write the first input block in the Data Input register */
      hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      inputaddr+=4U;
      hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      inputaddr+=4U;
      hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
      inputaddr+=4U;
      hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}





/**
  * @brief  Carry out in DMA mode the ciphering or deciphering operation according to
  *         hcryp->Init structure fields.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pInputData: Pointer to the plain text in case of encryption or cipher text in case of decryption
  *                     or key derivation+decryption.
  * @param  Size: Length of the input data buffer in bytes, must be a multiple of 16.
  * @param  pOutputData: Pointer to the cipher text in case of encryption or plain text in case of
  *                     decryption/key derivation+decryption.
  * @note   Chaining modes ECB, CBC and CTR are managed by this function in DMA mode.
  * @note   Supported operating modes are encryption, decryption and key derivation with decryption.
  * @note   No DMA channel is provided for key derivation only and therefore, access to AES_KEYRx
  *         registers must be done by software.
  * @note   This API is not applicable to key derivation only; for such a mode, access to AES_KEYRx
  *         registers must be done by software thru HAL_CRYPEx_AES() or HAL_CRYPEx_AES_IT() APIs.
  * @note   pInputData and pOutputData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYPEx_AES_DMA(CRYP_HandleTypeDef *hcryp,  uint8_t *pInputData, uint16_t Size, uint8_t *pOutputData)
{
  uint32_t inputaddr;
  uint32_t outputaddr;

  if (hcryp->State == HAL_CRYP_STATE_READY)
  {
    /* Check parameters setting */
    if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_KEYDERIVATION)
    {
      /* no DMA channel is provided for key derivation operating mode,
         access to AES_KEYRx registers must be done by software */
      return  HAL_ERROR;
    }
    else
    {
      if ((pInputData == NULL) || (pOutputData == NULL) || (Size == 0U))
      {
        return  HAL_ERROR;
      }
    }


    /* Process Locked */
    __HAL_LOCK(hcryp);

    inputaddr  = (uint32_t)pInputData;
    outputaddr = (uint32_t)pOutputData;

    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;

    /* Set the input and output addresses and start DMA transfer */
    CRYP_SetDMAConfig(hcryp, inputaddr, Size, outputaddr);

    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}






/**
  * @brief  Carry out in polling mode the authentication tag generation as well as the ciphering or deciphering
  *         operation according to hcryp->Init structure fields.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pInputData:
  *         - pointer to payload data in GCM or CCM payload phase,
  *         - pointer to B0 block in CMAC header phase,
  *         - pointer to C block in CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init, header and final phases.
  * @param  Size:
  *         - length of the input payload data buffer in bytes in GCM or CCM payload phase,
  *         - length of B0 block (in bytes) in CMAC header phase,
  *         - length of C block (in bytes) in CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init and header phases.
  *         - Parameter is meaningless in case of CCM final phase.
  *         - Parameter is message length in bytes in case of GCM final phase.
  *         - Parameter must be set to zero in case of GMAC final phase.
  * @param  pOutputData:
  *         - pointer to plain or cipher text in GCM/CCM payload phase,
  *         - pointer to authentication tag in GCM/GMAC/CCM/CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init and header phases.
  *         - Parameter is meaningless in case of CMAC header phase.
  * @param  Timeout: Specify Timeout value
  * @note   Supported operating modes are encryption and decryption, supported chaining modes are GCM, GMAC, CMAC and CCM when the latter is applicable.
  * @note   Phases are singly processed according to hcryp->Init.GCMCMACPhase so that steps in these specific chaining modes
  *         can be skipped by the user if so required.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYPEx_AES_Auth(CRYP_HandleTypeDef *hcryp, uint8_t *pInputData, uint64_t Size, uint8_t *pOutputData, uint32_t Timeout)
{
  uint32_t index             ;
  uint32_t inputaddr         ;
  uint32_t outputaddr        ;
  uint32_t tagaddr           ;
  uint64_t headerlength      ;
  uint64_t inputlength       ;
  uint64_t payloadlength     ;
  uint32_t difflength     = 0;
  uint32_t addhoc_process = 0;

  if (hcryp->State == HAL_CRYP_STATE_READY)
  {
    /* input/output parameters check */
    if (hcryp->Init.GCMCMACPhase == CRYP_INIT_PHASE)
    {
       /* No processing required */
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
    {
      if (((hcryp->Init.Header != NULL) && (hcryp->Init.HeaderSize == 0U)) ||
          ((hcryp->Init.Header == NULL) && (hcryp->Init.HeaderSize != 0U)))
      {
        return  HAL_ERROR;
      }
#if defined(AES_CR_NPBLB)
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM)
#else
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
#endif
      {
        /* In case of CMAC or CCM (when applicable) header phase resumption, we can have pInputData = NULL and  Size = 0 */
        if (((pInputData != NULL) && (Size == 0U)) || ((pInputData == NULL) && (Size != 0U)))
        {
          return  HAL_ERROR;
        }
      }
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
    {
      if (((pInputData == NULL) && (Size != 0U)) || \
          ((pInputData != NULL) && (Size == 0U)) || \
          ((pInputData != NULL) && (Size != 0U) && (pOutputData == NULL)))
      {
        return  HAL_ERROR;
      }
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_FINAL_PHASE)
    {
      if (pOutputData == NULL)
      {
        return  HAL_ERROR;
      }
#if !defined(AES_CR_NPBLB)
      if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC) && (pInputData == NULL))
      {
        return  HAL_ERROR;
      }
#endif
    }
    else
    {
      /* Unspecified Phase */
      return  HAL_ERROR;
    }


    /* Process Locked */
    __HAL_LOCK(hcryp);

    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;

    /*==============================================*/
    /* GCM/GMAC (or CCM when applicable) init phase */
    /*==============================================*/
    /* In case of init phase, the input data (Key and Initialization Vector) have
       already been entered during the initialization process. Therefore, the
       API just waits for the CCF flag to be set. */
    if (hcryp->Init.GCMCMACPhase == CRYP_INIT_PHASE)
    {
      /* just wait for hash computation */
      if(CRYP_WaitOnCCFlag(hcryp, Timeout) != HAL_OK)
      {
        hcryp->State = HAL_CRYP_STATE_READY;
        __HAL_UNLOCK(hcryp);
        return HAL_TIMEOUT;
      }

      /* Clear CCF Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      /* Mark that the initialization phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_INIT_OVER;
    }
    /*=======================================================*/
    /* GCM/GMAC or (CCM / CMAC when applicable) header phase */
    /*=======================================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
    {
#if !defined(AES_CR_NPBLB)
      /* Set header phase; for GCM or GMAC, set data-byte at this point */
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
      {
        MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH|AES_CR_DATATYPE, CRYP_HEADER_PHASE|hcryp->Init.DataType);
      }
      else
#endif
      {
        MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_HEADER_PHASE);
      }

      /* Enable the Peripheral */
      __HAL_CRYP_ENABLE(hcryp);

#if !defined(AES_CR_NPBLB)
      /* in case of CMAC, enter B0 block in header phase, before the header itself. */
      /* If Size = 0 (possible case of resumption after CMAC header phase suspension),
         skip these steps and go directly to header buffer feeding to the HW */
      if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC) && (Size != 0U))
      {
        uint64_t index_test;
        inputaddr = (uint32_t)pInputData;

        for(index=0U ; (index < Size); index += 16U)
        {
          /* Write the Input block in the Data Input register */
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;

          if(CRYP_WaitOnCCFlag(hcryp, Timeout) != HAL_OK)
          {
            hcryp->State = HAL_CRYP_STATE_READY;
            __HAL_UNLOCK(hcryp);
            return HAL_TIMEOUT;
          }
          /* Clear CCF Flag */
          __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

          /* If the suspension flag has been raised and if the processing is not about
           to end, suspend processing */
          index_test = (uint64_t)index + 16U;
          if ((hcryp->SuspendRequest == HAL_CRYP_SUSPEND) && (index_test < Size))
          {
            /* reset SuspendRequest */
            hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;
            /* Change the CRYP state */
            hcryp->State = HAL_CRYP_STATE_SUSPENDED;
            /* Mark that the header phase is over */
            hcryp->Phase = HAL_CRYP_PHASE_HEADER_SUSPENDED;

           /* Save current reading and writing locations of Input and Output buffers */
           hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
           /* Save the total number of bytes (B blocks + header) that remain to be
              processed at this point */
           hcryp->CrypInCount     =  (uint32_t) (hcryp->Init.HeaderSize + Size - index_test);

           /* Process Unlocked */
            __HAL_UNLOCK(hcryp);

            return HAL_OK;
          }
        } /* for(index=0; (index < Size); index += 16) */
      }
#endif /* !defined(AES_CR_NPBLB) */

      /* Enter header */
      inputaddr = (uint32_t)hcryp->Init.Header;
      /* Local variable headerlength is a number of bytes multiple of 128 bits,
         remaining header data (if any) are handled after this loop */
      headerlength =  (((hcryp->Init.HeaderSize)/16U)*16U) ;
      if ((hcryp->Init.HeaderSize % 16U) != 0U)
      {
        difflength = (uint32_t) (hcryp->Init.HeaderSize - headerlength);
      }
      for(index=0U ; index < headerlength; index += 16U)
      {
        uint64_t index_temp;
        /* Write the Input block in the Data Input register */
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;

        if(CRYP_WaitOnCCFlag(hcryp, Timeout) != HAL_OK)
        {
          hcryp->State = HAL_CRYP_STATE_READY;
          __HAL_UNLOCK(hcryp);
          return HAL_TIMEOUT;
        }
        /* Clear CCF Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

        /* If the suspension flag has been raised and if the processing is not about
         to end, suspend processing */
        index_temp = (uint64_t)index + 16U;
        if ((hcryp->SuspendRequest == HAL_CRYP_SUSPEND) && (index_temp < headerlength))
        {
          /* reset SuspendRequest */
          hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;
          /* Change the CRYP state */
          hcryp->State = HAL_CRYP_STATE_SUSPENDED;
          /* Mark that the header phase is over */
          hcryp->Phase = HAL_CRYP_PHASE_HEADER_SUSPENDED;

         /* Save current reading and writing locations of Input and Output buffers */
         hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
         /* Save the total number of bytes that remain to be processed at this point */
          hcryp->CrypInCount =  (uint32_t) (hcryp->Init.HeaderSize - index_temp);

         /* Process Unlocked */
          __HAL_UNLOCK(hcryp);

          return HAL_OK;
        }
      }

      /* Case header length is not a multiple of 16 bytes */
      if (difflength != 0U)
      {
        hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
        CRYP_Padding(hcryp, difflength, CRYP_POLLING_ON);
      }

      /* Mark that the header phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_HEADER_OVER;
    }
    /*============================================*/
    /* GCM (or CCM when applicable) payload phase */
    /*============================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
    {

      MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_PAYLOAD_PHASE);

      /* if the header phase has been bypassed, AES must be enabled again */
      if (hcryp->Phase == HAL_CRYP_PHASE_INIT_OVER)
      {
        __HAL_CRYP_ENABLE(hcryp);
      }

      inputaddr  = (uint32_t)pInputData;
      outputaddr = (uint32_t)pOutputData;

      /* Enter payload */
      /* Specific handling to manage payload last block size less than 128 bits */
      if ((Size % 16U) != 0U)
      {
        payloadlength = (Size/16U) * 16U;
        difflength = (uint32_t) (Size - payloadlength);
        addhoc_process = 1;
      }
      else
      {
        payloadlength = Size;
      }

      /* Feed payload */
      for(index=0U ; index < payloadlength; index += 16U)
      {
        uint64_t index_temp;
        /* Write the Input block in the Data Input register */
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;

        if(CRYP_WaitOnCCFlag(hcryp, Timeout) != HAL_OK)
        {
          hcryp->State = HAL_CRYP_STATE_READY;
          __HAL_UNLOCK(hcryp);
          return HAL_TIMEOUT;
        }

        /* Clear CCF Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

        /* Retrieve output data: read the output block
           from the Data Output Register */
        *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
        outputaddr+=4U;

        /* If the suspension flag has been raised and if the processing is not about
         to end, suspend processing */
        index_temp = (uint64_t)index + 16U;
        if ((hcryp->SuspendRequest == HAL_CRYP_SUSPEND) && (index_temp < payloadlength))
        {
          /* no flag waiting under IRQ handling */
          if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_ENCRYPT)
          {
            /* Ensure that Busy flag is reset */
            if(CRYP_WaitOnBusyFlagReset(hcryp, CRYP_BUSY_TIMEOUTVALUE) != HAL_OK)
            {
              hcryp->State = HAL_CRYP_STATE_READY;
              __HAL_UNLOCK(hcryp);
              return HAL_TIMEOUT;
            }
          }
          /* reset SuspendRequest */
          hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;
          /* Change the CRYP state */
          hcryp->State = HAL_CRYP_STATE_SUSPENDED;
          /* Mark that the header phase is over */
          hcryp->Phase = HAL_CRYP_PHASE_HEADER_SUSPENDED;

          /* Save current reading and writing locations of Input and Output buffers */
          hcryp->pCrypOutBuffPtr =  (uint8_t *)outputaddr;
          hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
          /* Save the number of bytes that remain to be processed at this point */
          hcryp->CrypInCount     =  (uint32_t) (Size - index_temp);

          /* Process Unlocked */
          __HAL_UNLOCK(hcryp);

          return HAL_OK;
        }

      }

      /* Additional processing to manage GCM(/CCM) encryption and decryption cases when
         payload last block size less than 128 bits */
      if (addhoc_process == 1U)
      {

        hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
        hcryp->pCrypOutBuffPtr =  (uint8_t *)outputaddr;
        CRYP_Padding(hcryp, difflength, CRYP_POLLING_ON);

      } /* (addhoc_process == 1) */

      /* Mark that the payload phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_OVER;
    }
    /*==================================*/
    /* GCM/GMAC/CCM or CMAC final phase */
    /*==================================*/
    else
    {
      tagaddr = (uint32_t)pOutputData;

#if defined(AES_CR_NPBLB)
     /* By default, clear NPBLB field */
      CLEAR_BIT(hcryp->Instance->CR, AES_CR_NPBLB);
#endif

      MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_FINAL_PHASE);

      /* if the header and payload phases have been bypassed, AES must be enabled again */
      if (hcryp->Phase == HAL_CRYP_PHASE_INIT_OVER)
      {
        __HAL_CRYP_ENABLE(hcryp);
      }

      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
      {
        headerlength = hcryp->Init.HeaderSize * 8U; /* Header length in bits */
        inputlength = Size * 8U;                    /* input length in bits */

#if !defined(AES_CR_NPBLB)
        if(hcryp->Init.DataType == CRYP_DATATYPE_1B)
        {
          hcryp->Instance->DINR = __RBIT((uint32_t)(headerlength>>32));
          hcryp->Instance->DINR = __RBIT((uint32_t)headerlength);
          hcryp->Instance->DINR = __RBIT((uint32_t)(inputlength>>32));
          hcryp->Instance->DINR = __RBIT((uint32_t)inputlength);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_8B)
        {
          hcryp->Instance->DINR = __REV((uint32_t)(headerlength>>32));
          hcryp->Instance->DINR = __REV((uint32_t)headerlength);
          hcryp->Instance->DINR = __REV((uint32_t)(inputlength>>32));
          hcryp->Instance->DINR = __REV((uint32_t)inputlength);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_16B)
        {
          hcryp->Instance->DINR = __ROR((uint32_t)(headerlength>>32), 16);
          hcryp->Instance->DINR = __ROR((uint32_t)headerlength, 16);
          hcryp->Instance->DINR = __ROR((uint32_t)(inputlength>>32), 16);
          hcryp->Instance->DINR = __ROR((uint32_t)inputlength, 16);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_32B)
        {
          hcryp->Instance->DINR = (uint32_t)(headerlength>>32);
          hcryp->Instance->DINR = (uint32_t)(headerlength);
          hcryp->Instance->DINR = (uint32_t)(inputlength>>32);
          hcryp->Instance->DINR = (uint32_t)(inputlength);
        }
        else
        {
          /* Unspecified Data Type */
         return  HAL_ERROR;
        }
#else
        hcryp->Instance->DINR = (uint32_t)(headerlength>>32);
        hcryp->Instance->DINR = (uint32_t)(headerlength);
        hcryp->Instance->DINR = (uint32_t)(inputlength>>32);
        hcryp->Instance->DINR = (uint32_t)(inputlength);
#endif
      }
#if !defined(AES_CR_NPBLB)
      else if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
      {
        inputaddr  = (uint32_t)pInputData;
        /* Enter the last block made of a 128-bit value formatted
           from the original B0 packet. */
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      }
      else
      {
         /* Unspecified Chaining Mode */
         return  HAL_ERROR;
       }
#endif


      if(CRYP_WaitOnCCFlag(hcryp, Timeout) != HAL_OK)
      {
          hcryp->State = HAL_CRYP_STATE_READY;
          __HAL_UNLOCK(hcryp);
          return HAL_TIMEOUT;
      }

      /* Read the Auth TAG in the Data Out register */
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;
      tagaddr+=4U;
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;
      tagaddr+=4U;
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;
      tagaddr+=4U;
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;


      /* Clear CCF Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      /* Mark that the final phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_FINAL_OVER;
      /* Disable the Peripheral */
      __HAL_CRYP_DISABLE(hcryp);
    }

    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}




/**
  * @brief  Carry out in interrupt mode the authentication tag generation as well as the ciphering or deciphering
  *         operation according to hcryp->Init structure fields.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pInputData:
  *         - pointer to payload data in GCM or CCM payload phase,
  *         - pointer to B0 block in CMAC header phase,
  *         - pointer to C block in CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init, header and final phases.
  * @param  Size:
  *         - length of the input payload data buffer in bytes in GCM or CCM payload phase,
  *         - length of B0 block (in bytes) in CMAC header phase,
  *         - length of C block (in bytes) in CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init and header phases.
  *         - Parameter is meaningless in case of CCM final phase.
  *         - Parameter is message length in bytes in case of GCM final phase.
  *         - Parameter must be set to zero in case of GMAC final phase.
  * @param  pOutputData:
  *         - pointer to plain or cipher text in GCM/CCM payload phase,
  *         - pointer to authentication tag in GCM/GMAC/CCM/CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init and header phases.
  *         - Parameter is meaningless in case of CMAC header phase.
  * @note   Supported operating modes are encryption and decryption, supported chaining modes are GCM, GMAC and CMAC.
  * @note   Phases are singly processed according to hcryp->Init.GCMCMACPhase so that steps in these specific chaining modes
  *         can be skipped by the user if so required.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYPEx_AES_Auth_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pInputData, uint64_t Size, uint8_t *pOutputData)
{

  uint32_t inputaddr         ;
  uint64_t headerlength      ;
  uint64_t inputlength       ;
  uint32_t index             ;
  uint32_t addhoc_process = 0;
  uint32_t difflength     = 0;
  uint32_t difflengthmod4 = 0;
  uint32_t mask[4][3];

  uint32_t mask_index = hcryp->Init.DataType >> AES_CR_DATATYPE_Pos;

  mask[0][0] = 0xFF000000U;  mask[0][1] = 0xFFFF0000U;  mask[0][2] = 0xFFFFFF00U;  /* 32-bit data */
  mask[1][0] = 0x0000FF00U;  mask[1][1] = 0x0000FFFFU;  mask[1][2] = 0xFF00FFFFU;  /* 16-bit data */
  mask[2][0] = 0x000000FFU;  mask[2][1] = 0x0000FFFFU;  mask[2][2] = 0x00FFFFFFU;  /* 8-bit data  */
  mask[3][0] = 0x000000FFU;  mask[3][1] = 0x0000FFFFU;  mask[3][2] = 0x00FFFFFFU;  /* Bit data    */

  if (hcryp->State == HAL_CRYP_STATE_READY)
  {
    /* input/output parameters check */
    if (hcryp->Init.GCMCMACPhase == CRYP_INIT_PHASE)
    {
       /* No processing required */
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
    {
      if (((hcryp->Init.Header != NULL) && (hcryp->Init.HeaderSize == 0U)) ||
          ((hcryp->Init.Header == NULL) && (hcryp->Init.HeaderSize != 0U)))
      {
        return  HAL_ERROR;
      }
#if defined(AES_CR_NPBLB)
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM)
#else
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
#endif
      {
        /* In case of CMAC or CCM header phase resumption, we can have pInputData = NULL and  Size = 0 */
        if (((pInputData != NULL) && (Size == 0U)) || ((pInputData == NULL) && (Size != 0U)))
        {
          return  HAL_ERROR;
        }
      }
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
    {
      if ((pInputData != NULL) && (Size != 0U) && (pOutputData == NULL))
      {
        return  HAL_ERROR;
      }
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_FINAL_PHASE)
    {
      if (pOutputData == NULL)
      {
        return  HAL_ERROR;
      }
#if !defined(AES_CR_NPBLB)
      if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC) && (pInputData == NULL))
      {
        return  HAL_ERROR;
      }
#endif
    }
    else
    {
      /* Unspecified Phase */
      return  HAL_ERROR;
     }


    /* Process Locked */
    __HAL_LOCK(hcryp);

    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;

    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);

    /* Enable Computation Complete Flag and Error Interrupts */
    __HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);



    /*==============================================*/
    /* GCM/GMAC (or CCM when applicable) init phase */
    /*==============================================*/
    if (hcryp->Init.GCMCMACPhase == CRYP_INIT_PHASE)
    {
    /* In case of init phase, the input data (Key and Initialization Vector) have
       already been entered during the initialization process. Therefore, the
       software just waits for the CCF interrupt to be raised and which will
       be handled by CRYP_AES_Auth_IT() API. */
    }
    /*===================================*/
    /* GCM/GMAC/CCM or CMAC header phase */
    /*===================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
    {

#if defined(AES_CR_NPBLB)
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM)
#else
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
#endif
      {
        /* In case of CMAC, B blocks are first entered, before the header.
           Therefore, B blocks and the header are entered back-to-back
           as if it was only one single block.
           However, in case of resumption after suspension, if all the
           B blocks have been entered (in that case, Size = 0), only the
           remainder of the non-processed header bytes are entered. */
          if (Size != 0U)
          {
            hcryp->CrypInCount = (uint32_t)(Size + hcryp->Init.HeaderSize);
            hcryp->pCrypInBuffPtr = pInputData;
          }
          else
          {
            hcryp->CrypInCount = (uint32_t)hcryp->Init.HeaderSize;
            hcryp->pCrypInBuffPtr = hcryp->Init.Header;
          }
      }
      else
      {
        /* Get the header addresses and sizes */
        hcryp->CrypInCount = (uint32_t)hcryp->Init.HeaderSize;
        hcryp->pCrypInBuffPtr = hcryp->Init.Header;
      }

      inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;


#if !defined(AES_CR_NPBLB)
      /* Set header phase; for GCM or GMAC, set data-byte at this point */
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
      {
        MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH|AES_CR_DATATYPE, CRYP_HEADER_PHASE|hcryp->Init.DataType);
      }
      else
#endif
      {
        MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_HEADER_PHASE);
      }

      /* Enable the Peripheral */
      __HAL_CRYP_ENABLE(hcryp);

      /* Increment/decrement instance pointer/counter */
      if (hcryp->CrypInCount == 0U)
      {
        /* Case of no header */
        hcryp->State = HAL_CRYP_STATE_READY;
        /* Mark that the header phase is over */
        hcryp->Phase = HAL_CRYP_PHASE_HEADER_OVER;
        return HAL_OK;
      }
      else if (hcryp->CrypInCount < 16U)
      {
        hcryp->CrypInCount = 0;
        addhoc_process = 1;
        difflength = (uint32_t) (hcryp->Init.HeaderSize);
        difflengthmod4 = difflength%4U;
      }
      else
      {
        hcryp->pCrypInBuffPtr += 16;
        hcryp->CrypInCount -= 16U;
      }


#if defined(AES_CR_NPBLB)
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM)
#else
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
#endif
      {
        if (hcryp->CrypInCount == hcryp->Init.HeaderSize)
        {
          /* All B blocks will have been entered after the next
             four DINR writing, so point at header buffer for
             the next iteration */
          hcryp->pCrypInBuffPtr = hcryp->Init.Header;
        }
      }

      /* Enter header first block to initiate the process
         in the Data Input register */
      if (addhoc_process == 0U)
      {
        /* Header has size equal or larger than 128 bits */
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      }
      else
      {
        /* Header has size less than 128 bits */
        /* Enter complete words when possible */
        for(index=0U ; index < (difflength/4U); index ++)
        {
          /* Write the Input block in the Data Input register */
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
        }
        /* Enter incomplete word padded with zeroes if applicable
          (case of header length not a multiple of 32-bits) */
        if (difflengthmod4 != 0U)
        {
          hcryp->Instance->DINR = ((*(uint32_t*)(inputaddr)) & mask[mask_index][difflengthmod4-1U]);
        }
        /* Pad with zero-words to reach 128-bit long block and wrap-up header feeding to the IP */
        for(index=0U; index < (4U - ((difflength+3U)/4U)); index ++)
        {
          hcryp->Instance->DINR = 0;
        }

      }
    }
    /*============================================*/
    /* GCM (or CCM when applicable) payload phase */
    /*============================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
    {
      /* Get the buffer addresses and sizes */
      hcryp->CrypInCount = (uint32_t)Size;
      hcryp->pCrypInBuffPtr = pInputData;
      hcryp->pCrypOutBuffPtr = pOutputData;
      hcryp->CrypOutCount = (uint32_t)Size;

      inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;

      MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_PAYLOAD_PHASE);

      /* if the header phase has been bypassed, AES must be enabled again */
      if (hcryp->Phase == HAL_CRYP_PHASE_INIT_OVER)
      {
        __HAL_CRYP_ENABLE(hcryp);
      }

      /* No payload case */
      if (pInputData == NULL)
      {
        hcryp->State = HAL_CRYP_STATE_READY;
        /* Mark that the header phase is over */
        hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_OVER;
        /* Process Unlocked */
        __HAL_UNLOCK(hcryp);

        return HAL_OK;
      }

     /* Specific handling to manage payload size less than 128 bits */
      if (Size < 16U)
      {
        difflength = (uint32_t) (Size);
#if defined(AES_CR_NPBLB)
        /* In case of GCM encryption or CCM decryption, specify the number of padding
           bytes in last block of payload */
        if (READ_BIT(hcryp->Instance->CR, AES_CR_GCMPH) == CRYP_PAYLOAD_PHASE)
        {
          uint32_t cr_temp = hcryp->Instance->CR;

          if (((cr_temp & (AES_CR_CHMOD|AES_CR_MODE)) == (CRYP_CHAINMODE_AES_GCM_GMAC|CRYP_ALGOMODE_ENCRYPT))
           ||  ((cr_temp & (AES_CR_CHMOD|AES_CR_MODE)) == (CRYP_CHAINMODE_AES_CCM|CRYP_ALGOMODE_DECRYPT)))
          {
            /* Set NPBLB field in writing the number of padding bytes
               for the last block of payload */
            MODIFY_REG(hcryp->Instance->CR, AES_CR_NPBLB, (16U - difflength) << AES_POSITION_CR_NPBLB);
          }
        }
#else
        /* Software workaround applied to GCM encryption only */
        if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_ENCRYPT)
        {
          /* Change the mode configured in CHMOD bits of CR register to select CTR mode */
          __HAL_CRYP_SET_CHAININGMODE(hcryp, CRYP_CHAINMODE_AES_CTR);
        }
#endif


        /* Set hcryp->CrypInCount to 0 (no more data to enter) */
        hcryp->CrypInCount = 0;

        /*  Insert the last block (which size is inferior to 128 bits) padded with zeroes,
            to have a complete block of 128 bits */
        difflengthmod4 = difflength%4U;
        /*  Insert the last block (which size is inferior to 128 bits) padded with zeroes
            to have a complete block of 128 bits */
        for(index=0U; index < (difflength/4U); index ++)
        {
          /* Write the Input block in the Data Input register */
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
        }
        /* If required, manage input data size not multiple of 32 bits */
        if (difflengthmod4 != 0U)
        {
          hcryp->Instance->DINR = ((*(uint32_t*)(inputaddr)) & mask[mask_index][difflengthmod4-1U]);
        }
        /* Wrap-up in padding with zero-words if applicable */
        for(index=0U; index < (4U - ((difflength+3U)/4U)); index ++)
        {
          hcryp->Instance->DINR = 0;
        }
      }
      else
      {
        /* Increment/decrement instance pointer/counter */
        hcryp->pCrypInBuffPtr += 16;
        hcryp->CrypInCount -= 16U;

        /* Enter payload first block to initiate the process
           in the Data Input register */
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      }
    }
    /*==================================*/
    /* GCM/GMAC/CCM or CMAC final phase */
    /*==================================*/
    else
    {
       hcryp->pCrypOutBuffPtr = pOutputData;

#if defined(AES_CR_NPBLB)
     /* By default, clear NPBLB field */
      CLEAR_BIT(hcryp->Instance->CR, AES_CR_NPBLB);
#endif

       MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_FINAL_PHASE);

      /* if the header and payload phases have been bypassed, AES must be enabled again */
      if (hcryp->Phase == HAL_CRYP_PHASE_INIT_OVER)
      {
        __HAL_CRYP_ENABLE(hcryp);
      }

      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
      {
        headerlength = hcryp->Init.HeaderSize * 8U; /* Header length in bits */
        inputlength = Size * 8U;                    /* Input length in bits */
        /* Write the number of bits in the header on 64 bits followed by the number
           of bits in the payload on 64 bits as well */

#if !defined(AES_CR_NPBLB)
        if(hcryp->Init.DataType == CRYP_DATATYPE_1B)
        {
          hcryp->Instance->DINR = __RBIT((uint32_t)((headerlength)>>32));
          hcryp->Instance->DINR = __RBIT((uint32_t)headerlength);
          hcryp->Instance->DINR = __RBIT((uint32_t)((inputlength)>>32));
          hcryp->Instance->DINR = __RBIT((uint32_t)inputlength);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_8B)
        {
          hcryp->Instance->DINR = __REV((uint32_t)(headerlength>>32));
          hcryp->Instance->DINR = __REV((uint32_t)headerlength);
          hcryp->Instance->DINR = __REV((uint32_t)(inputlength>>32));
          hcryp->Instance->DINR = __REV((uint32_t)inputlength);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_16B)
        {
          hcryp->Instance->DINR = __ROR((uint32_t)((headerlength)>>32), 16);
          hcryp->Instance->DINR = __ROR((uint32_t)headerlength, 16);
          hcryp->Instance->DINR = __ROR((uint32_t)((inputlength)>>32), 16);
          hcryp->Instance->DINR = __ROR((uint32_t)inputlength, 16);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_32B)
        {
          hcryp->Instance->DINR = (uint32_t)(headerlength>>32);
          hcryp->Instance->DINR = (uint32_t)(headerlength);
          hcryp->Instance->DINR = (uint32_t)(inputlength>>32);
          hcryp->Instance->DINR = (uint32_t)(inputlength);
        }
        else
        {
          /* Unspecified Data Type */
          return  HAL_ERROR;
        }
#else
        hcryp->Instance->DINR = (uint32_t)(headerlength>>32);
        hcryp->Instance->DINR = (uint32_t)(headerlength);
        hcryp->Instance->DINR = (uint32_t)(inputlength>>32);
        hcryp->Instance->DINR = (uint32_t)(inputlength);
#endif
      }
#if !defined(AES_CR_NPBLB)
      else if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
      {
        inputaddr  = (uint32_t)pInputData;
        /* Enter the last block made of a 128-bit value formatted
           from the original B0 packet. */
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      }
      else
      {
        /* Unspecified Chaining Mode */
        return  HAL_ERROR;
      }
#endif
    }

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}




/**
  * @brief  Carry out in DMA mode the authentication tag generation as well as the ciphering or deciphering
  *         operation according to hcryp->Init structure fields.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pInputData:
  *         - pointer to payload data in GCM or CCM payload phase,
  *         - pointer to B0 block in CMAC header phase,
  *         - pointer to C block in CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init, header and final phases.
  * @param  Size:
  *         - length of the input payload data buffer in bytes in GCM or CCM payload phase,
  *         - length of B0 block (in bytes) in CMAC header phase,
  *         - length of C block (in bytes) in CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init and header phases.
  *         - Parameter is meaningless in case of CCM final phase.
  *         - Parameter is message length in bytes in case of GCM final phase.
  *         - Parameter must be set to zero in case of GMAC final phase.
  * @param  pOutputData:
  *         - pointer to plain or cipher text in GCM/CCM payload phase,
  *         - pointer to authentication tag in GCM/GMAC/CCM/CMAC final phase.
  *         - Parameter is meaningless in case of GCM/GMAC/CCM init and header phases.
  *         - Parameter is meaningless in case of CMAC header phase.
  * @note   Supported operating modes are encryption and decryption, supported chaining modes are GCM, GMAC and CMAC.
  * @note   Phases are singly processed according to hcryp->Init.GCMCMACPhase so that steps in these specific chaining modes
  *         can be skipped by the user if so required.
  * @note   pInputData and pOutputData buffers must be 32-bit aligned to ensure a correct DMA transfer to and from the IP.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYPEx_AES_Auth_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pInputData, uint64_t Size, uint8_t *pOutputData)
{
  uint32_t inputaddr      ;
  uint32_t outputaddr     ;
  uint32_t tagaddr        ;
  uint64_t headerlength   ;
  uint64_t inputlength    ;
  uint64_t payloadlength  ;


  if (hcryp->State == HAL_CRYP_STATE_READY)
  {
    /* input/output parameters check */
    if (hcryp->Init.GCMCMACPhase == CRYP_INIT_PHASE)
    {
       /* No processing required */
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
    {
      if ((hcryp->Init.Header != NULL) && (hcryp->Init.HeaderSize == 0U))
      {
        return  HAL_ERROR;
      }
#if defined(AES_CR_NPBLB)
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM)
      {
        /* In case of CMAC or CCM header phase resumption, we can have pInputData = NULL and  Size = 0 */
        if (((pInputData != NULL) && (Size == 0U)) || ((pInputData == NULL) && (Size != 0U)))
        {
          return  HAL_ERROR;
        }
      }
#else
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
      {
        if ((pInputData == NULL) || (Size == 0U))
        {
          return  HAL_ERROR;
        }
      }
#endif
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
    {
      if ((pInputData != NULL) && (Size != 0U) && (pOutputData == NULL))
      {
        return  HAL_ERROR;
      }
    }
    else if (hcryp->Init.GCMCMACPhase == CRYP_FINAL_PHASE)
    {
      if (pOutputData == NULL)
      {
        return  HAL_ERROR;
      }
#if !defined(AES_CR_NPBLB)
      if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC) && (pInputData == NULL))
      {
        return  HAL_ERROR;
      }
#endif
    }
    else
    {
      /* Unspecified Phase */
      return  HAL_ERROR;
    }


    /* Process Locked */
    __HAL_LOCK(hcryp);

    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;

    /*==============================================*/
    /* GCM/GMAC (or CCM when applicable) init phase */
    /*==============================================*/
    /* In case of init phase, the input data (Key and Initialization Vector) have
       already been entered during the initialization process. No DMA transfer is
       required at that point therefore, the software just waits for the CCF flag
       to be raised. */
    if (hcryp->Init.GCMCMACPhase == CRYP_INIT_PHASE)
    {
      /* just wait for hash computation */
      if(CRYP_WaitOnCCFlag(hcryp, CRYP_CCF_TIMEOUTVALUE) != HAL_OK)
      {
        hcryp->State = HAL_CRYP_STATE_READY;
        __HAL_UNLOCK(hcryp);
        return HAL_TIMEOUT;
      }

      /* Clear CCF Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      /* Mark that the initialization phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_INIT_OVER;
      hcryp->State = HAL_CRYP_STATE_READY;
    }
    /*====================================*/
    /* GCM/GMAC/ CCM or CMAC header phase */
    /*====================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
    {
#if !defined(AES_CR_NPBLB)
      /* Set header phase; for GCM or GMAC, set data-byte at this point */
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
      {
        MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH|AES_CR_DATATYPE, CRYP_HEADER_PHASE|hcryp->Init.DataType);
      }
      else
#endif
      {
        MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_HEADER_PHASE);
      }

      /* Enable the CRYP peripheral */
      __HAL_CRYP_ENABLE(hcryp);

#if !defined(AES_CR_NPBLB)
      /* enter first B0 block in polling mode (no DMA transfer for B0) */
      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
      {
        inputaddr  = (uint32_t)pInputData;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);

        if(CRYP_WaitOnCCFlag(hcryp, CRYP_CCF_TIMEOUTVALUE) != HAL_OK)
        {
          hcryp->State = HAL_CRYP_STATE_READY;
          __HAL_UNLOCK(hcryp);
          return HAL_TIMEOUT;
        }
        /* Clear CCF Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      }
#endif

      /* No header case */
      if (hcryp->Init.Header == NULL)
      {
        hcryp->State = HAL_CRYP_STATE_READY;
        /* Mark that the header phase is over */
        hcryp->Phase = HAL_CRYP_PHASE_HEADER_OVER;
        /* Process Unlocked */
        __HAL_UNLOCK(hcryp);

        return HAL_OK;
      }

      inputaddr = (uint32_t)hcryp->Init.Header;
      if ((hcryp->Init.HeaderSize % 16U) != 0U)
      {

        if (hcryp->Init.HeaderSize < 16U)
        {
          hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
          CRYP_Padding(hcryp, (uint32_t) (hcryp->Init.HeaderSize), CRYP_POLLING_OFF);

          hcryp->State = HAL_CRYP_STATE_READY;
          /* Mark that the header phase is over */
          hcryp->Phase = HAL_CRYP_PHASE_HEADER_OVER;

          /* CCF flag indicating header phase AES processing completion
             will be checked at the start of the next phase:
            - payload phase (GCM / CCM when applicable)
            - final phase (GMAC or CMAC when applicable).  */
        }
        else
        {
          /* Local variable headerlength is a number of bytes multiple of 128 bits,
            remaining header data (if any) are handled after this loop */
          headerlength =  (((hcryp->Init.HeaderSize)/16U)*16U) ;
          /* Store the ending transfer point */
          hcryp->pCrypInBuffPtr = hcryp->Init.Header + headerlength;
          hcryp->CrypInCount = (uint32_t)(hcryp->Init.HeaderSize - headerlength); /* remainder */

          /* Set the input and output addresses and start DMA transfer */
          /* (incomplete DMA transfer, will be wrapped up after completion of
             the first one (initiated here) with data padding */
          CRYP_Authentication_SetDMAConfig(hcryp, inputaddr, (uint16_t)headerlength, 0);
        }
      }
      else
      {
        hcryp->CrypInCount = 0;
        /* Set the input address and start DMA transfer */
        CRYP_Authentication_SetDMAConfig(hcryp, inputaddr, (uint16_t)hcryp->Init.HeaderSize, 0);
      }
    }
    /*============================================*/
    /* GCM (or CCM when applicable) payload phase */
    /*============================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
    {
      /* Coming from header phase, wait for CCF flag to be raised
          if header present and fed to the IP in the previous phase */
      if (hcryp->Init.Header != NULL)
      {
        if(CRYP_WaitOnCCFlag(hcryp, CRYP_CCF_TIMEOUTVALUE) != HAL_OK)
        {
          hcryp->State = HAL_CRYP_STATE_READY;
          __HAL_UNLOCK(hcryp);
          return HAL_TIMEOUT;
        }
      }
      else
      {
        /* Enable the Peripheral since wasn't in header phase (no header case) */
        __HAL_CRYP_ENABLE(hcryp);
      }
      /* Clear CCF Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

      MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_PAYLOAD_PHASE);

      /* No payload case */
      if (pInputData == NULL)
      {
        hcryp->State = HAL_CRYP_STATE_READY;
        /* Mark that the header phase is over */
        hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_OVER;
        /* Process Unlocked */
        __HAL_UNLOCK(hcryp);

        return HAL_OK;
      }


      /* Specific handling to manage payload size less than 128 bits */
      if ((Size % 16U) != 0U)
      {
        inputaddr  = (uint32_t)pInputData;
        outputaddr = (uint32_t)pOutputData;
        if (Size < 16U)
        {
          /* Block is now entered in polling mode, no actual gain in resorting to DMA */
          hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
          hcryp->pCrypOutBuffPtr =  (uint8_t *)outputaddr;

          CRYP_Padding(hcryp, (uint32_t)Size, CRYP_POLLING_ON);

          /* Change the CRYP state to ready */
          hcryp->State = HAL_CRYP_STATE_READY;
          /* Mark that the payload phase is over */
          hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_OVER;

          /* Call output data transfer complete callback */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
          hcryp->OutCpltCallback(hcryp);
#else
          HAL_CRYP_OutCpltCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
        }
        else
        {
          payloadlength = (Size/16U) * 16U;

          /* Store the ending transfer points */
          hcryp->pCrypInBuffPtr = pInputData;
          hcryp->pCrypInBuffPtr += payloadlength;
          hcryp->pCrypOutBuffPtr = pOutputData;
          hcryp->pCrypOutBuffPtr += payloadlength;
          hcryp->CrypInCount = (uint32_t)(Size - payloadlength); /* remainder */

          /* Set the input and output addresses and start DMA transfer */
          /* (incomplete DMA transfer, will be wrapped up with data padding
             after completion of the one initiated here) */
          CRYP_Authentication_SetDMAConfig(hcryp, inputaddr, (uint16_t)payloadlength, outputaddr);
        }
      }
      else
      {
        hcryp->CrypInCount = 0;
        inputaddr  = (uint32_t)pInputData;
        outputaddr = (uint32_t)pOutputData;

        /* Set the input and output addresses and start DMA transfer */
        CRYP_Authentication_SetDMAConfig(hcryp, inputaddr, (uint16_t)Size, outputaddr);
      }
    }
    /*==================================*/
    /* GCM/GMAC/CCM or CMAC final phase */
    /*==================================*/
    else
    {
      /* If coming from header phase (GMAC or CMAC case when applicable),
         wait for CCF flag to be raised */
      if (READ_BIT(hcryp->Instance->CR, AES_CR_GCMPH) == CRYP_HEADER_PHASE)
      {
        if(CRYP_WaitOnCCFlag(hcryp, CRYP_CCF_TIMEOUTVALUE) != HAL_OK)
        {
          hcryp->State = HAL_CRYP_STATE_READY;
          __HAL_UNLOCK(hcryp);
          return HAL_TIMEOUT;
        }
        /* Clear CCF Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      }

      tagaddr = (uint32_t)pOutputData;

#if defined(AES_CR_NPBLB)
     /* By default, clear NPBLB field */
      CLEAR_BIT(hcryp->Instance->CR, AES_CR_NPBLB);
#endif
      MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_FINAL_PHASE);

      /* if the header and payload phases have been bypassed, AES must be enabled again */
      if (hcryp->Phase == HAL_CRYP_PHASE_INIT_OVER)
      {
        __HAL_CRYP_ENABLE(hcryp);
      }

      if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC)
      {
        headerlength = hcryp->Init.HeaderSize * 8U; /* Header length in bits */
        inputlength = Size * 8U;  /* input length in bits */
        /* Write the number of bits in the header on 64 bits followed by the number
           of bits in the payload on 64 bits as well */
#if !defined(AES_CR_NPBLB)
        if(hcryp->Init.DataType == CRYP_DATATYPE_1B)
        {
          hcryp->Instance->DINR = __RBIT((uint32_t)(headerlength>>32));
          hcryp->Instance->DINR = __RBIT((uint32_t)headerlength);
          hcryp->Instance->DINR = __RBIT((uint32_t)(inputlength>>32));
          hcryp->Instance->DINR = __RBIT((uint32_t)inputlength);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_8B)
        {
          hcryp->Instance->DINR = __REV((uint32_t)(headerlength>>32));
          hcryp->Instance->DINR = __REV((uint32_t)headerlength);
          hcryp->Instance->DINR = __REV((uint32_t)(inputlength>>32));
          hcryp->Instance->DINR = __REV((uint32_t)inputlength);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_16B)
        {
          hcryp->Instance->DINR = __ROR((uint32_t)(headerlength>>32), 16);
          hcryp->Instance->DINR = __ROR((uint32_t)headerlength, 16);
          hcryp->Instance->DINR = __ROR((uint32_t)(inputlength>>32), 16);
          hcryp->Instance->DINR = __ROR((uint32_t)inputlength, 16);
        }
        else if(hcryp->Init.DataType == CRYP_DATATYPE_32B)
        {
          hcryp->Instance->DINR = (uint32_t)(headerlength>>32);
          hcryp->Instance->DINR = (uint32_t)(headerlength);
          hcryp->Instance->DINR = (uint32_t)(inputlength>>32);
          hcryp->Instance->DINR = (uint32_t)(inputlength);
        }
        else
        {
          /* Unspecified Data Type */
          return  HAL_ERROR;
        }
#else
        hcryp->Instance->DINR = (uint32_t)(headerlength>>32);
        hcryp->Instance->DINR = (uint32_t)(headerlength);
        hcryp->Instance->DINR = (uint32_t)(inputlength>>32);
        hcryp->Instance->DINR = (uint32_t)(inputlength);
#endif
      }
#if !defined(AES_CR_NPBLB)
      else if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
      {
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

        inputaddr  = (uint32_t)pInputData;
        /* Enter the last block made of a 128-bit value formatted
           from the original B0 packet. */
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
        inputaddr+=4U;
        hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
      }
      else
      {
        /* Unspecified Chaining Mode */
        return  HAL_ERROR;
      }
#endif

      /* No DMA transfer is required at that point therefore, the software
         just waits for the CCF flag to be raised. */
      if(CRYP_WaitOnCCFlag(hcryp, CRYP_CCF_TIMEOUTVALUE) != HAL_OK)
      {
          hcryp->State = HAL_CRYP_STATE_READY;
          __HAL_UNLOCK(hcryp);
          return HAL_TIMEOUT;
      }

      /* Read the Auth TAG in the IN FIFO */
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;
      tagaddr+=4U;
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;
      tagaddr+=4U;
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;
      tagaddr+=4U;
      *(uint32_t*)(tagaddr) = hcryp->Instance->DOUTR;

      /* Clear CCF Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

      /* Mark that the final phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_FINAL_OVER;
      hcryp->State = HAL_CRYP_STATE_READY;
      /* Disable the Peripheral */
      __HAL_CRYP_DISABLE(hcryp);

    }

    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @}
  */

/** @defgroup CRYPEx_Exported_Functions_Group3 AES suspension/resumption functions
 *  @brief   Extended processing functions.
 *
@verbatim
  ==============================================================================
                    ##### AES extended suspension and resumption functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) save in memory the Initialization Vector, the Key registers, the Control register or
          the Suspend registers when a process is suspended by a higher priority message
      (+) write back in CRYP hardware block the saved values listed above when the suspended
          lower priority message processing is resumed.

@endverbatim
  * @{
  */


/**
  * @brief  In case of message processing suspension, read the Initialization Vector.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Output: Pointer to the buffer containing the saved Initialization Vector.
  * @note   This value has to be stored for reuse by writing the AES_IVRx registers
  *         as soon as the interrupted processing has to be resumed.
  *         Applicable to all chaining modes.
  * @note   AES must be disabled when reading or resetting the IV values.
  * @retval None
  */
void HAL_CRYPEx_Read_IVRegisters(CRYP_HandleTypeDef *hcryp, uint8_t* Output)
{
  uint32_t outputaddr = (uint32_t)Output;

  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->IVR3);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->IVR2);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->IVR1);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->IVR0);
}

/**
  * @brief  In case of message processing resumption, rewrite the Initialization
  *         Vector in the AES_IVRx registers.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Input: Pointer to the buffer containing the saved Initialization Vector to
  *         write back in the CRYP hardware block.
  * @note   Applicable to all chaining modes.
  * @note   AES must be disabled when reading or resetting the IV values.
  * @retval None
  */
void HAL_CRYPEx_Write_IVRegisters(CRYP_HandleTypeDef *hcryp, uint8_t* Input)
{
  uint32_t ivaddr = (uint32_t)Input;

  hcryp->Instance->IVR3 = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->IVR2 = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->IVR1 = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->IVR0 = __REV(*(uint32_t*)(ivaddr));
}


/**
  * @brief  In case of message GCM/GMAC (CCM/CMAC when applicable) processing suspension,
  *         read the Suspend Registers.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Output: Pointer to the buffer containing the saved Suspend Registers.
  * @note   These values have to be stored for reuse by writing back the AES_SUSPxR registers
  *         as soon as the interrupted processing has to be resumed.
  * @retval None
  */
void HAL_CRYPEx_Read_SuspendRegisters(CRYP_HandleTypeDef *hcryp, uint8_t* Output)
{
  uint32_t outputaddr = (uint32_t)Output;

  /* In case of GCM payload phase encryption, check that suspension can be carried out */
  if (READ_BIT(hcryp->Instance->CR, (AES_CR_CHMOD|AES_CR_GCMPH|AES_CR_MODE)) == (CRYP_CHAINMODE_AES_GCM_GMAC|CRYP_PAYLOAD_PHASE|CRYP_ALGOMODE_ENCRYPT))
  {
    /* Ensure that Busy flag is reset */
    if(CRYP_WaitOnBusyFlagReset(hcryp, CRYP_BUSY_TIMEOUTVALUE) != HAL_OK)
    {
      hcryp->ErrorCode |= HAL_CRYP_BUSY_ERROR;
      hcryp->State = HAL_CRYP_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hcryp);

#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
      hcryp->ErrorCallback(hcryp);
#else
      HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
      return ;
    }
  }

  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP7R);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP6R);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP5R);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP4R);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP3R);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP2R);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP1R);
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = __REV(hcryp->Instance->SUSP0R);
}

/**
  * @brief  In case of message GCM/GMAC (CCM/CMAC when applicable) processing resumption, rewrite the Suspend
  *         Registers in the AES_SUSPxR registers.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Input: Pointer to the buffer containing the saved suspend registers to
  *         write back in the CRYP hardware block.
  * @retval None
  */
void HAL_CRYPEx_Write_SuspendRegisters(CRYP_HandleTypeDef *hcryp, uint8_t* Input)
{
  uint32_t ivaddr = (uint32_t)Input;

  hcryp->Instance->SUSP7R = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->SUSP6R = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->SUSP5R = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->SUSP4R = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->SUSP3R = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->SUSP2R = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->SUSP1R = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->SUSP0R = __REV(*(uint32_t*)(ivaddr));
}


/**
  * @brief  In case of message GCM/GMAC (CCM/CMAC when applicable) processing suspension, read the Key Registers.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Output: Pointer to the buffer containing the saved Key Registers.
  * @param  KeySize: Indicates the key size (128 or 256 bits).
  * @note   These values have to be stored for reuse by writing back the AES_KEYRx registers
  *         as soon as the interrupted processing has to be resumed.
  * @retval None
  */
void HAL_CRYPEx_Read_KeyRegisters(CRYP_HandleTypeDef *hcryp, uint8_t* Output, uint32_t KeySize)
{
  uint32_t keyaddr = (uint32_t)Output;

  if (KeySize == CRYP_KEYSIZE_256B)
  {
    *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR7);
    keyaddr+=4U;
    *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR6);
    keyaddr+=4U;
    *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR5);
    keyaddr+=4U;
    *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR4);
    keyaddr+=4U;
  }

  *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR3);
  keyaddr+=4U;
  *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR2);
  keyaddr+=4U;
  *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR1);
  keyaddr+=4U;
  *(uint32_t*)(keyaddr) = __REV(hcryp->Instance->KEYR0);
}

/**
  * @brief  In case of message GCM/GMAC (CCM/CMAC when applicable) processing resumption, rewrite the Key
  *         Registers in the AES_KEYRx registers.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Input: Pointer to the buffer containing the saved key registers to
  *         write back in the CRYP hardware block.
  * @param  KeySize: Indicates the key size (128 or 256 bits)
  * @retval None
  */
void HAL_CRYPEx_Write_KeyRegisters(CRYP_HandleTypeDef *hcryp, uint8_t* Input, uint32_t KeySize)
{
  uint32_t keyaddr = (uint32_t)Input;

  if (KeySize == CRYP_KEYSIZE_256B)
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
}


/**
  * @brief  In case of message GCM/GMAC (CCM/CMAC when applicable) processing suspension, read the Control Register.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Output: Pointer to the buffer containing the saved Control Register.
  * @note   This values has to be stored for reuse by writing back the AES_CR register
  *         as soon as the interrupted processing has to be resumed.
  * @retval None
  */
void HAL_CRYPEx_Read_ControlRegister(CRYP_HandleTypeDef *hcryp, uint8_t* Output)
{
  *(uint32_t*)(void *)(Output) = hcryp->Instance->CR;                           /* Derogation MisraC2012 R.11.5 */
}

/**
  * @brief  In case of message GCM/GMAC (CCM/CMAC when applicable) processing resumption, rewrite the Control
  *         Registers in the AES_CR register.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Input: Pointer to the buffer containing the saved Control Register to
  *         write back in the CRYP hardware block.
  * @retval None
  */
void HAL_CRYPEx_Write_ControlRegister(CRYP_HandleTypeDef *hcryp, uint8_t* Input)
{
  hcryp->Instance->CR = *(uint32_t*)(void *)(Input);                            /* Derogation MisraC2012 R.11.5 */
  /* At the same time, set handle state back to READY to be able to resume the AES calculations
     without the processing APIs returning HAL_BUSY when called. */
  hcryp->State        = HAL_CRYP_STATE_READY;
}

/**
  * @brief  Request CRYP processing suspension when in polling or interruption mode.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @note   Set the handle field SuspendRequest to the appropriate value so that
  *         the on-going CRYP processing is suspended as soon as the required
  *         conditions are met.
  * @note   It is advised not to suspend the CRYP processing when the DMA controller
  *         is managing the data transfer
  * @retval None
  */
void HAL_CRYPEx_ProcessSuspend(CRYP_HandleTypeDef *hcryp)
{
  /* Set Handle Suspend Request field */
  hcryp->SuspendRequest = HAL_CRYP_SUSPEND;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup CRYPEx_Private_Functions
  * @{
  */

/**
  * @brief  DMA CRYP Input Data process complete callback
  *         for GCM, GMAC, CCM or CMAC chaining modes.
  * @note   Specific setting of hcryp fields are required only
  *         in the case of header phase where no output data DMA
  *         transfer is on-going (only input data transfer is enabled
  *         in such a case).
  * @param  hdma: DMA handle.
  * @retval None
  */
static void CRYP_Authentication_DMAInCplt(DMA_HandleTypeDef *hdma)
{
  uint32_t difflength;
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;   /* Derogation MisraC2012 R.11.5 */

  /* Disable the DMA transfer for input request  */
  CLEAR_BIT(hcryp->Instance->CR, AES_CR_DMAINEN);

  if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
  {

    if (hcryp->CrypInCount != 0U)
    {
      /* Last block is now entered in polling mode, no actual gain in resorting to DMA */
      difflength = hcryp->CrypInCount;
      hcryp->CrypInCount = 0;

      CRYP_Padding(hcryp, difflength, CRYP_POLLING_OFF);
    }
    hcryp->State = HAL_CRYP_STATE_READY;
    /* Mark that the header phase is over */
    hcryp->Phase = HAL_CRYP_PHASE_HEADER_OVER;
  }
  /* CCF flag indicating header phase AES processing completion
     will be checked at the start of the next phase:
     - payload phase (GCM or CCM when applicable)
     - final phase (GMAC or CMAC).
    This allows to avoid the Wait on Flag within the IRQ handling.  */

  /* Call input data transfer complete callback */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
  hcryp->InCpltCallback(hcryp);
#else
  HAL_CRYP_InCpltCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA CRYP Output Data process complete callback
  *         for GCM, GMAC, CCM or CMAC chaining modes.
  * @note   This callback is called only in the payload phase.
  * @param  hdma: DMA handle.
  * @retval None
  */
static void CRYP_Authentication_DMAOutCplt(DMA_HandleTypeDef *hdma)
{
  uint32_t difflength;
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;   /* Derogation MisraC2012 R.11.5 */

  /* Disable the DMA transfer for output request */
  CLEAR_BIT(hcryp->Instance->CR, AES_CR_DMAOUTEN);

  /* Clear CCF Flag */
  __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

  /* Initiate additional transfer to wrap-up data feeding to the IP */
  if (hcryp->CrypInCount != 0U)
  {
    /* Last block is now entered in polling mode, no actual gain in resorting to DMA */
    difflength = hcryp->CrypInCount;
    hcryp->CrypInCount = 0;

    CRYP_Padding(hcryp, difflength, CRYP_POLLING_ON);
  }

  /* Change the CRYP state to ready */
  hcryp->State = HAL_CRYP_STATE_READY;
  /* Mark that the payload phase is over */
  hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_OVER;

  /* Call output data transfer complete callback */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
  hcryp->OutCpltCallback(hcryp);
#else
  HAL_CRYP_OutCpltCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA CRYP communication error callback
  *         for GCM, GMAC, CCM or CMAC chaining modes.
  * @param  hdma: DMA handle
  * @retval None
  */
static void CRYP_Authentication_DMAError(DMA_HandleTypeDef *hdma)
{
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;   /* Derogation MisraC2012 R.11.5 */

  hcryp->State= HAL_CRYP_STATE_ERROR;
  hcryp->ErrorCode |= HAL_CRYP_DMA_ERROR;
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
  hcryp->ErrorCallback(hcryp);
#else
  HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
  /* Clear Error Flag */
  __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_ERR_CLEAR);
}



/**
  * @brief  Handle CRYP block input/output data handling under interruption
  *         for GCM, GMAC, CCM  or CMAC chaining modes.
  * @note   The function is called under interruption only, once
  *         interruptions have been enabled by HAL_CRYPEx_AES_Auth_IT().
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval HAL status
  */
HAL_StatusTypeDef CRYP_AES_Auth_IT(CRYP_HandleTypeDef *hcryp)
{
  uint32_t inputaddr         ;
  uint32_t outputaddr        ;
  uint32_t index             ;
  uint32_t addhoc_process = 0;
  uint32_t difflength     = 0;
  uint32_t difflengthmod4 = 0;
  uint32_t mask[4][3]        ;
  uint32_t mask_index = hcryp->Init.DataType >> AES_CR_DATATYPE_Pos;
  uint32_t intermediate_data[4] = {0};

  mask[0][0] = 0xFF000000U;  mask[0][1] = 0xFFFF0000U;  mask[0][2] = 0xFFFFFF00U;  /* 32-bit data */
  mask[1][0] = 0x0000FF00U;  mask[1][1] = 0x0000FFFFU;  mask[1][2] = 0xFF00FFFFU;  /* 16-bit data */
  mask[2][0] = 0x000000FFU;  mask[2][1] = 0x0000FFFFU;  mask[2][2] = 0x00FFFFFFU;  /* 8-bit data  */
  mask[3][0] = 0x000000FFU;  mask[3][1] = 0x0000FFFFU;  mask[3][2] = 0x00FFFFFFU;  /* Bit data    */

  if(hcryp->State == HAL_CRYP_STATE_BUSY)
  {
    /*===========================*/
    /* GCM/GMAC(/CCM) init phase */
    /*===========================*/
    if (hcryp->Init.GCMCMACPhase == CRYP_INIT_PHASE)
    {
      /* Clear Computation Complete Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      /* Disable Computation Complete Flag and Errors Interrupts */
      __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
      /* Change the CRYP state */
      hcryp->State = HAL_CRYP_STATE_READY;

      /* Mark that the initialization phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_INIT_OVER;

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
    /*========================================*/
    /* GCM/GMAC (or CCM or CMAC) header phase */
    /*========================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_HEADER_PHASE)
    {
      /* Check if all input header data have been entered */
      if (hcryp->CrypInCount == 0U)
      {
        /* Clear Computation Complete Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
        /* Disable Computation Complete Flag and Errors Interrupts */
        __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
        /* Change the CRYP state */
        hcryp->State = HAL_CRYP_STATE_READY;
       /* Mark that the header phase is over */
        hcryp->Phase = HAL_CRYP_PHASE_HEADER_OVER;

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
        /* Clear CCF Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

        /* reset SuspendRequest */
        hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;
        /* Disable Computation Complete Flag and Errors Interrupts */
        __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
        /* Change the CRYP state */
        hcryp->State = HAL_CRYP_STATE_SUSPENDED;
        /* Mark that the header phase is suspended */
        hcryp->Phase = HAL_CRYP_PHASE_HEADER_SUSPENDED;

       /* Process Unlocked */
        __HAL_UNLOCK(hcryp);

        return HAL_OK;
      }
      else /* Carry on feeding input data to the CRYP hardware block */
      {
        /* Clear Computation Complete Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
        /* Get the last Input data address */
        inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;

        /* Increment/decrement instance pointer/counter */
        if (hcryp->CrypInCount < 16U)
        {
          difflength = hcryp->CrypInCount;
          hcryp->CrypInCount = 0;
          addhoc_process = 1;
          difflengthmod4 = difflength%4U;
        }
        else
        {
          hcryp->pCrypInBuffPtr += 16;
          hcryp->CrypInCount -= 16U;
        }

#if defined(AES_CR_NPBLB)
        if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CCM)
#else
        if (hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_CMAC)
#endif
        {
          if (hcryp->CrypInCount == hcryp->Init.HeaderSize)
          {
            /* All B blocks will have been entered after the next
              four DINR writing, so point at header buffer for
              the next iteration */
            hcryp->pCrypInBuffPtr = hcryp->Init.Header;
          }
        }

        /* Write the Input block in the Data Input register */
        if (addhoc_process == 0U)
        {
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        }
        else
        {
          /* Header remainder has size less than 128 bits */
          /* Enter complete words when possible */
          for(index=0U ; index < (difflength/4U); index ++)
          {
            /* Write the Input block in the Data Input register */
            hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
            inputaddr+=4U;
          }
          /* Enter incomplete word padded with zeroes if applicable
            (case of header length not a multiple of 32-bits) */
          if (difflengthmod4 != 0U)
          {
            hcryp->Instance->DINR = ((*(uint32_t*)(inputaddr)) & mask[mask_index][difflengthmod4-1U]);
          }
          /* Pad with zero-words to reach 128-bit long block and wrap-up header feeding to the IP */
          for(index=0U; index < (4U - ((difflength+3U)/4U)); index ++)
          {
            hcryp->Instance->DINR = 0;
          }
        }

        return HAL_OK;
      }
    }
    /*=======================*/
    /* GCM/CCM payload phase */
    /*=======================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
    {
      /* Get the last output data address */
      outputaddr = (uint32_t)hcryp->pCrypOutBuffPtr;

     /* Specific handling to manage payload size less than 128 bits
        when GCM (or CCM when applicable) encryption or decryption is selected.
        Check here if the last block output data are read */
#if defined(AES_CR_NPBLB)
      if ((hcryp->CrypOutCount < 16U)                                && \
          (hcryp->CrypOutCount > 0U))
#else
      if ((hcryp->Init.ChainingMode == CRYP_CHAINMODE_AES_GCM_GMAC) && \
          (hcryp->CrypOutCount < 16U)                                && \
          (hcryp->CrypOutCount > 0U))
#endif
      {
        difflength = hcryp->CrypOutCount;
        difflengthmod4 = difflength%4U;
        hcryp->CrypOutCount = 0;   /* mark that no more output data will be needed */
        /* Retrieve intermediate data */
        for(index=0U ; index < 4U; index ++)
        {
          intermediate_data[index] = hcryp->Instance->DOUTR;
        }
        /* Retrieve last words of cyphered data */
        /* First, retrieve complete output words */
        for(index=0U ; index < (difflength/4U); index ++)
        {
          *(uint32_t*)(outputaddr) = intermediate_data[index];
          outputaddr+=4U;
        }
        /* Next, retrieve partial output word if applicable;
           at the same time, start masking intermediate data
           with a mask of zeros of same size than the padding
           applied to the last block of payload */
        if (difflengthmod4 != 0U)
        {
          intermediate_data[difflength/4U] &= mask[mask_index][difflengthmod4-1U];
          *(uint32_t*)(outputaddr) = intermediate_data[difflength/4U];
        }

#if !defined(AES_CR_NPBLB)
        if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_ENCRYPT)
        {
          /* Change again CHMOD configuration to GCM mode */
          __HAL_CRYP_SET_CHAININGMODE(hcryp, CRYP_CHAINMODE_AES_GCM_GMAC);

          /* Select FINAL phase */
          MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_FINAL_PHASE);

          /* Before inserting the intermediate data, carry on masking operation
             with a mask of zeros of same size than the padding applied to the last block of payload */
          for(index=0U ; index < (4U - ((difflength+3U)/4U)); index ++)
          {
            intermediate_data[((difflength+3U)/4U)+index] = 0;
          }

          /* Insert intermediate data to trigger an additional DOUTR reading round */
          /* Clear Computation Complete Flag before entering new block */
          __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
          for(index=0U ; index < 4U; index ++)
          {
            hcryp->Instance->DINR = intermediate_data[index];
          }
        }
        else
#endif
        {
          /* Payload phase is now over */
          /* Clear Computation Complete Flag */
          __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
          /* Disable Computation Complete Flag and Errors Interrupts */
          __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
          /* Change the CRYP state */
          hcryp->State = HAL_CRYP_STATE_READY;
          /* Mark that the payload phase is over */
          hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_OVER;

          /* Process Unlocked */
          __HAL_UNLOCK(hcryp);

          /* Call computation complete callback */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
          hcryp->CompCpltCallback(hcryp);
#else
          HAL_CRYPEx_ComputationCpltCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
        }
        return HAL_OK;
      }
      else
      {
        if (hcryp->CrypOutCount != 0U)
        {
          /* Usual case (different than GCM/CCM last block < 128 bits ciphering) */
          /* Retrieve the last block available from the CRYP hardware block:
            read the output block from the Data Output Register */
          *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
          outputaddr+=4U;
          *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
          outputaddr+=4U;
          *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
          outputaddr+=4U;
          *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;

          /* Increment/decrement instance pointer/counter */
          hcryp->pCrypOutBuffPtr += 16;
          hcryp->CrypOutCount -= 16U;
        }
#if !defined(AES_CR_NPBLB)
        else
        {
          /* Software work-around: additional DOUTR reading round to discard the data */
          for(index=0U ; index < 4U; index ++)
          {
            intermediate_data[index] = hcryp->Instance->DOUTR;
          }
        }
#endif
      }

      /* Check if all output text has been retrieved */
      if (hcryp->CrypOutCount == 0U)
      {
        /* Clear Computation Complete Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
        /* Disable Computation Complete Flag and Errors Interrupts */
        __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
        /* Change the CRYP state */
        hcryp->State = HAL_CRYP_STATE_READY;
       /* Mark that the payload phase is over */
        hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_OVER;

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
        /* Clear CCF Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

        /* reset SuspendRequest */
        hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;
        /* Disable Computation Complete Flag and Errors Interrupts */
        __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
        /* Change the CRYP state */
        hcryp->State = HAL_CRYP_STATE_SUSPENDED;
        /* Mark that the payload phase is suspended */
        hcryp->Phase = HAL_CRYP_PHASE_PAYLOAD_SUSPENDED;

       /* Process Unlocked */
        __HAL_UNLOCK(hcryp);

        return HAL_OK;
      }
      else /* Output data are still expected, carry on feeding the CRYP
               hardware block with input data */
      {
        /* Clear Computation Complete Flag */
        __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
        /* Get the last Input data address */
        inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;

        /* Usual input data feeding case */
        if (hcryp->CrypInCount < 16U)
        {
          difflength = (uint32_t) (hcryp->CrypInCount);
          difflengthmod4 = difflength%4U;
          hcryp->CrypInCount = 0;

#if defined(AES_CR_NPBLB)
          /* In case of GCM encryption or CCM decryption, specify the number of padding
             bytes in last block of payload */
          {
            uint32_t cr_temp = hcryp->Instance->CR;

            if (((cr_temp & (AES_CR_CHMOD|AES_CR_MODE)) == (CRYP_CHAINMODE_AES_GCM_GMAC|CRYP_ALGOMODE_ENCRYPT))
             ||  ((cr_temp & (AES_CR_CHMOD|AES_CR_MODE)) == (CRYP_CHAINMODE_AES_CCM|CRYP_ALGOMODE_DECRYPT)))
            {
              /* Set NPBLB field in writing the number of padding bytes
              for the last block of payload */
              MODIFY_REG(hcryp->Instance->CR, AES_CR_NPBLB, (16U - difflength) << AES_POSITION_CR_NPBLB);
             }
          }
#else
          /* Software workaround applied to GCM encryption only */
          if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_ENCRYPT)
          {
            /* Change the mode configured in CHMOD bits of CR register to select CTR mode */
            __HAL_CRYP_SET_CHAININGMODE(hcryp, CRYP_CHAINMODE_AES_CTR);
          }
#endif

          /*  Insert the last block (which size is inferior to 128 bits) padded with zeroes
              to have a complete block of 128 bits */
          for(index=0U ; index < (difflength/4U); index ++)
          {
            /* Write the Input block in the Data Input register */
            hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
            inputaddr+=4U;
          }
          /* If required, manage input data size not multiple of 32 bits */
          if (difflengthmod4 != 0U)
          {
            hcryp->Instance->DINR = ((*(uint32_t*)(inputaddr)) & mask[mask_index][difflengthmod4-1U]);
          }
          /* Wrap-up in padding with zero-words if applicable */
          for(index=0U ; index < (4U - ((difflength+3U)/4U)); index ++)
          {
            hcryp->Instance->DINR = 0;
          }

        }
        else
        {
          hcryp->pCrypInBuffPtr += 16;
          hcryp->CrypInCount -= 16U;

          /* Write the Input block in the Data Input register */
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
          inputaddr+=4U;
          hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
        }


        return HAL_OK;
      }
    }
    /*=======================================*/
    /* GCM/GMAC (or CCM or CMAC) final phase */
    /*=======================================*/
    else if (hcryp->Init.GCMCMACPhase == CRYP_FINAL_PHASE)
    {
      /* Clear Computation Complete Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

      /* Get the last output data address */
      outputaddr = (uint32_t)hcryp->pCrypOutBuffPtr;

      /* Retrieve the last expected data from the CRYP hardware block:
         read the output block from the Data Output Register */
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
      outputaddr+=4U;
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
      outputaddr+=4U;
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
      outputaddr+=4U;
      *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;

      /* Disable Computation Complete Flag and Errors Interrupts */
      __HAL_CRYP_DISABLE_IT(hcryp, CRYP_IT_CCFIE|CRYP_IT_ERRIE);
      /* Change the CRYP state */
      hcryp->State = HAL_CRYP_STATE_READY;
      /* Mark that the header phase is over */
      hcryp->Phase = HAL_CRYP_PHASE_FINAL_OVER;

      /* Disable the Peripheral */
      __HAL_CRYP_DISABLE(hcryp);
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
    else
    {
      /* Clear Computation Complete Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      hcryp->State = HAL_CRYP_STATE_ERROR;
      __HAL_UNLOCK(hcryp);
      return HAL_ERROR;
    }
  }
  else
  {
    return HAL_BUSY;
  }
}



/**
  * @brief  Set the DMA configuration and start the DMA transfer
  *         for GCM, GMAC, CCM  or CMAC chaining modes.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  inputaddr: Address of the Input buffer.
  * @param  Size: Size of the Input buffer un bytes, must be a multiple of 16.
  * @param  outputaddr: Address of the Output buffer, null pointer when no output DMA stream
  *         has to be configured.
  * @retval None
  */
static void CRYP_Authentication_SetDMAConfig(CRYP_HandleTypeDef *hcryp, uint32_t inputaddr, uint16_t Size, uint32_t outputaddr)
{

  /* Set the input CRYP DMA transfer complete callback */
  hcryp->hdmain->XferCpltCallback = CRYP_Authentication_DMAInCplt;
  /* Set the DMA error callback */
  hcryp->hdmain->XferErrorCallback = CRYP_Authentication_DMAError;

  if (outputaddr != 0U)
  {
    /* Set the output CRYP DMA transfer complete callback */
    hcryp->hdmaout->XferCpltCallback = CRYP_Authentication_DMAOutCplt;
    /* Set the DMA error callback */
    hcryp->hdmaout->XferErrorCallback = CRYP_Authentication_DMAError;
  }

  /* Enable the CRYP peripheral */
  __HAL_CRYP_ENABLE(hcryp);

  /* Enable the DMA input stream */
  if (HAL_DMA_Start_IT(hcryp->hdmain, inputaddr, (uint32_t)&hcryp->Instance->DINR, ((uint32_t)Size)/4U) != HAL_OK)
  {
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
    hcryp->ErrorCallback(hcryp);
#else
    HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
  }

  /* Enable the DMA input request */
  SET_BIT(hcryp->Instance->CR, AES_CR_DMAINEN);


  if (outputaddr != 0U)
  {
    /* Enable the DMA output stream */
    if (HAL_DMA_Start_IT(hcryp->hdmaout, (uint32_t)&hcryp->Instance->DOUTR, outputaddr, ((uint32_t)Size)/4U) != HAL_OK)
    {
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
      hcryp->ErrorCallback(hcryp);
#else
      HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
    }

    /* Enable the DMA output request */
    SET_BIT(hcryp->Instance->CR, AES_CR_DMAOUTEN);
  }
}



/**
  * @brief  Write/read input/output data in polling mode.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Input: Pointer to the Input buffer.
  * @param  Ilength: Length of the Input buffer in bytes, must be a multiple of 16.
  * @param  Output: Pointer to the returned buffer.
  * @param  Timeout: Specify Timeout value.
  * @retval HAL status
  */
static HAL_StatusTypeDef CRYP_ProcessData(CRYP_HandleTypeDef *hcryp, uint8_t* Input, uint16_t Ilength, uint8_t* Output, uint32_t Timeout)
{
  uint32_t index;
  uint32_t inputaddr  = (uint32_t)Input;
  uint32_t outputaddr = (uint32_t)Output;


  for(index=0U ; (index < Ilength); index += 16U)
  {
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;

    /* Wait for CCF flag to be raised */
    if(CRYP_WaitOnCCFlag(hcryp, Timeout) != HAL_OK)
    {
      hcryp->State = HAL_CRYP_STATE_READY;
      __HAL_UNLOCK(hcryp);
      return HAL_TIMEOUT;
    }

    /* Clear CCF Flag */
    __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

    /* Read the Output block from the Data Output Register */
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;

    /* If the suspension flag has been raised and if the processing is not about
       to end, suspend processing */
    if ((hcryp->SuspendRequest == HAL_CRYP_SUSPEND) && ((index+16U) < Ilength))
    {
      /* Reset SuspendRequest */
      hcryp->SuspendRequest = HAL_CRYP_SUSPEND_NONE;

      /* Save current reading and writing locations of Input and Output buffers */
      hcryp->pCrypOutBuffPtr =  (uint8_t *)outputaddr;
      hcryp->pCrypInBuffPtr  =  (uint8_t *)inputaddr;
      /* Save the number of bytes that remain to be processed at this point */
      hcryp->CrypInCount     =  Ilength - (index+16U);

      /* Change the CRYP state */
      hcryp->State = HAL_CRYP_STATE_SUSPENDED;

      return HAL_OK;
    }


  }
  /* Return function status */
  return HAL_OK;

}





/**
  * @brief  Read derivative key in polling mode when CRYP hardware block is set
  *         in key derivation operating mode (mode 2).
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Output: Pointer to the returned buffer.
  * @param  Timeout: Specify Timeout value.
  * @retval HAL status
  */
static HAL_StatusTypeDef CRYP_ReadKey(CRYP_HandleTypeDef *hcryp, uint8_t* Output, uint32_t Timeout)
{
  uint32_t outputaddr = (uint32_t)Output;

  /* Wait for CCF flag to be raised */
  if(CRYP_WaitOnCCFlag(hcryp, Timeout) != HAL_OK)
  {
    hcryp->State = HAL_CRYP_STATE_READY;
    __HAL_UNLOCK(hcryp);
    return HAL_TIMEOUT;
  }
  /* Clear CCF Flag */
  __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

    /* Read the derivative key from the AES_KEYRx registers */
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


  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Set the DMA configuration and start the DMA transfer.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  inputaddr: Address of the Input buffer.
  * @param  Size: Size of the Input buffer in bytes, must be a multiple of 16.
  * @param  outputaddr: Address of the Output buffer.
  * @retval None
  */
static void CRYP_SetDMAConfig(CRYP_HandleTypeDef *hcryp, uint32_t inputaddr, uint16_t Size, uint32_t outputaddr)
{
  /* Set the CRYP DMA transfer complete callback */
  hcryp->hdmain->XferCpltCallback = CRYP_DMAInCplt;
  /* Set the DMA error callback */
  hcryp->hdmain->XferErrorCallback = CRYP_DMAError;

  /* Set the CRYP DMA transfer complete callback */
  hcryp->hdmaout->XferCpltCallback = CRYP_DMAOutCplt;
  /* Set the DMA error callback */
  hcryp->hdmaout->XferErrorCallback = CRYP_DMAError;

  /* Enable the DMA input stream */
  if (HAL_DMA_Start_IT(hcryp->hdmain, inputaddr, (uint32_t)&hcryp->Instance->DINR, ((uint32_t)Size)/4U) != HAL_OK)
  {
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
    hcryp->ErrorCallback(hcryp);
#else
    HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
  }

  /* Enable the DMA output stream */
  if (HAL_DMA_Start_IT(hcryp->hdmaout, (uint32_t)&hcryp->Instance->DOUTR, outputaddr, ((uint32_t)Size)/4U) != HAL_OK)
  {
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
    hcryp->ErrorCallback(hcryp);
#else
    HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
  }

  /* Enable In and Out DMA requests */
  SET_BIT(hcryp->Instance->CR, (AES_CR_DMAINEN | AES_CR_DMAOUTEN));

  /* Enable the CRYP peripheral */
  __HAL_CRYP_ENABLE(hcryp);
}


/**
  * @brief  Handle CRYP hardware block Timeout when waiting for CCF flag to be raised.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Timeout: Timeout duration.
  * @retval HAL status
  */
static HAL_StatusTypeDef CRYP_WaitOnCCFlag(CRYP_HandleTypeDef const * const hcryp, uint32_t Timeout)
{
  uint32_t tickstart;

  /* Get timeout */
  tickstart = HAL_GetTick();

  while(HAL_IS_BIT_CLR(hcryp->Instance->SR, AES_SR_CCF))
  {
    /* Check for the Timeout */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((HAL_GetTick() - tickstart ) > Timeout)
      {
        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}

/**
  * @brief  Wait for Busy Flag to be reset during a GCM payload encryption process suspension.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  Timeout: Timeout duration.
  * @retval HAL status
  */
static HAL_StatusTypeDef CRYP_WaitOnBusyFlagReset(CRYP_HandleTypeDef const * const hcryp, uint32_t Timeout)
{
  uint32_t tickstart;

  /* Get timeout */
  tickstart = HAL_GetTick();

  while(HAL_IS_BIT_SET(hcryp->Instance->SR, AES_SR_BUSY))
  {
    /* Check for the Timeout */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((HAL_GetTick() - tickstart ) > Timeout)
      {
        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}


/**
  * @brief  DMA CRYP Input Data process complete callback.
  * @param  hdma: DMA handle.
  * @retval None
  */
static void CRYP_DMAInCplt(DMA_HandleTypeDef *hdma)
{
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;   /* Derogation MisraC2012 R.11.5 */

  /* Disable the DMA transfer for input request  */
  CLEAR_BIT(hcryp->Instance->CR, AES_CR_DMAINEN);

  /* Call input data transfer complete callback */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
  hcryp->InCpltCallback(hcryp);
#else
  HAL_CRYP_InCpltCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA CRYP Output Data process complete callback.
  * @param  hdma: DMA handle.
  * @retval None
  */
static void CRYP_DMAOutCplt(DMA_HandleTypeDef *hdma)
{
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;   /* Derogation MisraC2012 R.11.5 */

  /* Disable the DMA transfer for output request */
  CLEAR_BIT(hcryp->Instance->CR, AES_CR_DMAOUTEN);

  /* Clear CCF Flag */
  __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);

  /* Disable CRYP */
  __HAL_CRYP_DISABLE(hcryp);

  /* Change the CRYP state to ready */
  hcryp->State = HAL_CRYP_STATE_READY;

  /* Call output data transfer complete callback */
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
  hcryp->OutCpltCallback(hcryp);
#else
  HAL_CRYP_OutCpltCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA CRYP communication error callback.
  * @param  hdma: DMA handle.
  * @retval None
  */
static void CRYP_DMAError(DMA_HandleTypeDef *hdma)
{
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;   /* Derogation MisraC2012 R.11.5 */

  hcryp->State= HAL_CRYP_STATE_ERROR;
  hcryp->ErrorCode |= HAL_CRYP_DMA_ERROR;
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
  hcryp->ErrorCallback(hcryp);
#else
  HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
  /* Clear Error Flag */
  __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_ERR_CLEAR);
}

/**
  * @brief  Last header or payload block padding when size is not a multiple of 128 bits.
  * @param  hcryp: pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module.
  * @param  difflength: size remainder after having fed all complete 128-bit blocks.
  * @param  polling: specifies whether or not polling on CCF must be done after having
  *                  entered a complete block.
  * @retval None
  */
static void CRYP_Padding(CRYP_HandleTypeDef *hcryp, uint32_t difflength, uint32_t polling)
{
  uint32_t index;
  uint32_t difflengthmod4 = difflength%4U;
  uint32_t inputaddr      = (uint32_t)hcryp->pCrypInBuffPtr;
  uint32_t outputaddr     = (uint32_t)hcryp->pCrypOutBuffPtr;
  uint32_t mask[4][3];
  uint32_t mask_index = hcryp->Init.DataType >> AES_CR_DATATYPE_Pos;

  uint32_t intermediate_data[4] = {0};

  mask[0][0] = 0xFF000000U;  mask[0][1] = 0xFFFF0000U;  mask[0][2] = 0xFFFFFF00U;  /* 32-bit data */
  mask[1][0] = 0x0000FF00U;  mask[1][1] = 0x0000FFFFU;  mask[1][2] = 0xFF00FFFFU;  /* 16-bit data */
  mask[2][0] = 0x000000FFU;  mask[2][1] = 0x0000FFFFU;  mask[2][2] = 0x00FFFFFFU;  /* 8-bit data  */
  mask[3][0] = 0x000000FFU;  mask[3][1] = 0x0000FFFFU;  mask[3][2] = 0x00FFFFFFU;  /* Bit data    */

#if defined(AES_CR_NPBLB)
  /* In case of GCM encryption or CCM decryption, specify the number of padding
     bytes in last block of payload */
     if (READ_BIT(hcryp->Instance->CR,AES_CR_GCMPH) == CRYP_PAYLOAD_PHASE)
     {
       uint32_t cr_temp = hcryp->Instance->CR;

       if (((cr_temp & (AES_CR_CHMOD|AES_CR_MODE)) == (CRYP_CHAINMODE_AES_GCM_GMAC|CRYP_ALGOMODE_ENCRYPT))
       ||  ((cr_temp & (AES_CR_CHMOD|AES_CR_MODE)) == (CRYP_CHAINMODE_AES_CCM|CRYP_ALGOMODE_DECRYPT)))
       {
         /* Set NPBLB field in writing the number of padding bytes
            for the last block of payload */
         MODIFY_REG(hcryp->Instance->CR, AES_CR_NPBLB, (16U - difflength) << AES_POSITION_CR_NPBLB);
       }
     }
#else
  /* Software workaround applied to GCM encryption only */
  if ((hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE) &&
      (hcryp->Init.OperatingMode == CRYP_ALGOMODE_ENCRYPT))
  {
    /* Change the mode configured in CHMOD bits of CR register to select CTR mode */
    __HAL_CRYP_SET_CHAININGMODE(hcryp, CRYP_CHAINMODE_AES_CTR);
  }
#endif

  /* Wrap-up entering header or payload data */
  /* Enter complete words when possible */
  for(index=0U ; index < (difflength/4U); index ++)
  {
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
  }
  /* Enter incomplete word padded with zeroes if applicable
    (case of header length not a multiple of 32-bits) */
  if (difflengthmod4 != 0U)
  {
    hcryp->Instance->DINR = ((*(uint32_t*)(inputaddr)) & mask[mask_index][difflengthmod4-1U]);
  }
  /* Pad with zero-words to reach 128-bit long block and wrap-up header feeding to the IP */
  for(index=0U ; index < (4U - ((difflength+3U)/4U)); index ++)
  {
    hcryp->Instance->DINR = 0;
  }

  if (polling == (uint32_t)CRYP_POLLING_ON)
  {
    if(CRYP_WaitOnCCFlag(hcryp, CRYP_CCF_TIMEOUTVALUE) != HAL_OK)
    {
        hcryp->State = HAL_CRYP_STATE_READY;
        __HAL_UNLOCK(hcryp);
#if (USE_HAL_CRYP_REGISTER_CALLBACKS == 1)
        hcryp->ErrorCallback(hcryp);
#else
       HAL_CRYP_ErrorCallback(hcryp);
#endif /* USE_HAL_CRYP_REGISTER_CALLBACKS */
      }

    /* Clear CCF Flag */
    __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
  }

  /* if payload */
  if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE)
  {

    /* Retrieve intermediate data */
    for(index=0U ; index < 4U; index ++)
    {
      intermediate_data[index] = hcryp->Instance->DOUTR;
    }
    /* Retrieve last words of cyphered data */
    /* First, retrieve complete output words */
    for(index=0U ; index < (difflength/4U); index ++)
    {
      *(uint32_t*)(outputaddr) = intermediate_data[index];
      outputaddr+=4U;
    }
    /* Next, retrieve partial output word if applicable;
       at the same time, start masking intermediate data
       with a mask of zeros of same size than the padding
       applied to the last block of payload */
    if (difflengthmod4 != 0U)
    {
      intermediate_data[difflength/4U] &= mask[mask_index][difflengthmod4-1U];
      *(uint32_t*)(outputaddr) = intermediate_data[difflength/4U];
    }


#if !defined(AES_CR_NPBLB)
    /* Software workaround applied to GCM encryption only,
       applicable for AES IP v2 version (where NPBLB is not defined) */
    if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_ENCRYPT)
    {
      /* Change again CHMOD configuration to GCM mode */
      __HAL_CRYP_SET_CHAININGMODE(hcryp, CRYP_CHAINMODE_AES_GCM_GMAC);

      /* Select FINAL phase */
      MODIFY_REG(hcryp->Instance->CR, AES_CR_GCMPH, CRYP_FINAL_PHASE);

      /* Before inserting the intermediate data, carry on masking operation
         with a mask of zeros of same size than the padding applied to the last block of payload */
      for(index=0U ; index < (4U - ((difflength+3U)/4U)); index ++)
      {
        intermediate_data[((difflength+3U)/4U)+index] = 0;
      }
      /* Insert intermediate data */
      for(index=0U ; index < 4U; index ++)
      {
        hcryp->Instance->DINR = intermediate_data[index];
      }

      /*  Wait for completion, and read data on DOUT. This data is to discard. */
      if(CRYP_WaitOnCCFlag(hcryp, CRYP_CCF_TIMEOUTVALUE) != HAL_OK)
      {
        hcryp->State = HAL_CRYP_STATE_READY;
        __HAL_UNLOCK(hcryp);
        HAL_CRYP_ErrorCallback(hcryp);
      }

      /* Read data to discard */
      /* Clear CCF Flag */
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CCF_CLEAR);
      for(index=0U ; index < 4U; index ++)
      {
        intermediate_data[index] = hcryp->Instance->DOUTR;
      }

    } /* if (hcryp->Init.OperatingMode == CRYP_ALGOMODE_ENCRYPT) */
#endif  /* !defined(AES_CR_NPBLB) */
  }   /* if (hcryp->Init.GCMCMACPhase == CRYP_PAYLOAD_PHASE) */

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
