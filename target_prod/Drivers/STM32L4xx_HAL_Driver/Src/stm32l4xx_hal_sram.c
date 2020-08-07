/**
  ******************************************************************************
  * @file    stm32l4xx_hal_sram.c
  * @author  MCD Application Team
  * @brief   SRAM HAL module driver.
  *          This file provides a generic firmware to drive SRAM memories  
  *          mounted as external device.
  *         
  @verbatim
  ==============================================================================
                          ##### How to use this driver #####
  ==============================================================================  
  [..]
    This driver is a generic layered driver which contains a set of APIs used to 
    control SRAM memories. It uses the FMC layer functions to interface 
    with SRAM devices.  
    The following sequence should be followed to configure the FMC to interface
    with SRAM/PSRAM memories: 
      
   (#) Declare a SRAM_HandleTypeDef handle structure, for example:
          SRAM_HandleTypeDef  hsram; and: 
          
       (++) Fill the SRAM_HandleTypeDef handle "Init" field with the allowed 
            values of the structure member.
            
       (++) Fill the SRAM_HandleTypeDef handle "Instance" field with a predefined 
            base register instance for NOR or SRAM device 
                         
       (++) Fill the SRAM_HandleTypeDef handle "Extended" field with a predefined
            base register instance for NOR or SRAM extended mode 
             
   (#) Declare two FMC_NORSRAM_TimingTypeDef structures, for both normal and extended 
       mode timings; for example:
          FMC_NORSRAM_TimingTypeDef  Timing and FMC_NORSRAM_TimingTypeDef  ExTiming;
      and fill its fields with the allowed values of the structure member.
      
   (#) Initialize the SRAM Controller by calling the function HAL_SRAM_Init(). This function
       performs the following sequence:
          
       (##) MSP hardware layer configuration using the function HAL_SRAM_MspInit()
       (##) Control register configuration using the FMC NORSRAM interface function 
            FMC_NORSRAM_Init()
       (##) Timing register configuration using the FMC NORSRAM interface function 
            FMC_NORSRAM_Timing_Init()
       (##) Extended mode Timing register configuration using the FMC NORSRAM interface function 
            FMC_NORSRAM_Extended_Timing_Init()
       (##) Enable the SRAM device using the macro __FMC_NORSRAM_ENABLE()    

   (#) At this stage you can perform read/write accesses from/to the memory connected 
       to the NOR/SRAM Bank. You can perform either polling or DMA transfer using the
       following APIs:
       (++) HAL_SRAM_Read()/HAL_SRAM_Write() for polling read/write access
       (++) HAL_SRAM_Read_DMA()/HAL_SRAM_Write_DMA() for DMA read/write transfer
       
   (#) You can also control the SRAM device by calling the control APIs HAL_SRAM_WriteOperation_Enable()/
       HAL_SRAM_WriteOperation_Disable() to respectively enable/disable the SRAM write operation  
       
   (#) You can continuously monitor the SRAM device HAL state by calling the function
       HAL_SRAM_GetState()              
                             
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

#if defined(FMC_BANK1)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

#ifdef HAL_SRAM_MODULE_ENABLED

/** @defgroup SRAM SRAM
  * @brief SRAM driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/    
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup SRAM_Exported_Functions SRAM Exported Functions
  * @{
  */

/** @defgroup SRAM_Exported_Functions_Group1 Initialization and de-initialization functions
  * @brief    Initialization and Configuration functions.
  *
  @verbatim    
  ==============================================================================
           ##### SRAM Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to initialize/de-initialize
          the SRAM memory.
  
@endverbatim
  * @{
  */

/**
  * @brief  Perform the SRAM device initialization sequence
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  Timing Pointer to SRAM control timing structure
  * @param  ExtTiming Pointer to SRAM extended mode timing structure
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Init(SRAM_HandleTypeDef *hsram, FMC_NORSRAM_TimingTypeDef *Timing, FMC_NORSRAM_TimingTypeDef *ExtTiming)
{ 
  /* Check the SRAM handle parameter */
  if(hsram == NULL)
  {
     return HAL_ERROR;
  }
  
  if(hsram->State == HAL_SRAM_STATE_RESET)
  {  
    /* Allocate lock resource and initialize it */
    hsram->Lock = HAL_UNLOCKED;
    /* Initialize the low level hardware (MSP) */
    HAL_SRAM_MspInit(hsram);
  }
  
  /* Initialize SRAM control Interface */
  FMC_NORSRAM_Init(hsram->Instance, &(hsram->Init));

  /* Initialize SRAM timing Interface */
  FMC_NORSRAM_Timing_Init(hsram->Instance, Timing, hsram->Init.NSBank); 

  /* Initialize SRAM extended mode timing Interface */
  FMC_NORSRAM_Extended_Timing_Init(hsram->Extended, ExtTiming, hsram->Init.NSBank,  hsram->Init.ExtendedMode);  
  
  /* Enable the NORSRAM device */
  __FMC_NORSRAM_ENABLE(hsram->Instance, hsram->Init.NSBank); 
  
  return HAL_OK;
}

/**
  * @brief  Perform the SRAM device de-initialization sequence.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL status
  */
HAL_StatusTypeDef  HAL_SRAM_DeInit(SRAM_HandleTypeDef *hsram)
{ 
  /* De-Initialize the low level hardware (MSP) */
  HAL_SRAM_MspDeInit(hsram);
   
  /* Configure the SRAM registers with their reset values */
  FMC_NORSRAM_DeInit(hsram->Instance, hsram->Extended, hsram->Init.NSBank);

  hsram->State = HAL_SRAM_STATE_RESET;
  
  /* Release Lock */
  __HAL_UNLOCK(hsram);

  return HAL_OK;
}

/**
  * @brief  Initialize the SRAM MSP.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsram);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SRAM_MspInit could be implemented in the user file
   */ 
}

/**
  * @brief  DeInitialize the SRAM MSP.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef *hsram)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsram);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SRAM_MspDeInit could be implemented in the user file
   */ 
}

/**
  * @brief  DMA transfer complete callback.
  * @param  hdma pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdma);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SRAM_DMA_XferCpltCallback could be implemented in the user file
   */ 
}

/**
  * @brief  DMA transfer complete error callback.
  * @param  hdma pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval None
  */
__weak void HAL_SRAM_DMA_XferErrorCallback(DMA_HandleTypeDef *hdma)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdma);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SRAM_DMA_XferErrorCallback could be implemented in the user file
   */ 
}

/**
  * @}
  */

/** @defgroup SRAM_Exported_Functions_Group2 Input Output and memory control functions 
  * @brief    Input Output and memory control functions 
  *
  @verbatim    
  ==============================================================================
                  ##### SRAM Input and Output functions #####
  ==============================================================================
  [..]  
    This section provides functions allowing to use and control the SRAM memory
  
@endverbatim
  * @{
  */

/**
  * @brief  Read 8-bit buffer from SRAM memory. 
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_8b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint8_t *pDstBuffer, uint32_t BufferSize)
{
  __IO uint8_t * psramaddress = (uint8_t *)pAddress;
  
  /* Process Locked */
  __HAL_LOCK(hsram);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY;  
  
  /* Read data from memory */
  for(; BufferSize != 0; BufferSize--)
  {
    *pDstBuffer = *(__IO uint8_t *)psramaddress;
    pDstBuffer++;
    psramaddress++;
  }
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY;    

  /* Process unlocked */
  __HAL_UNLOCK(hsram); 
    
  return HAL_OK;   
}

/**
  * @brief  Write 8-bit buffer to SRAM memory. 
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_8b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint8_t *pSrcBuffer, uint32_t BufferSize)
{
  __IO uint8_t * psramaddress = (uint8_t *)pAddress;
  
  /* Check the SRAM controller state */
  if(hsram->State == HAL_SRAM_STATE_PROTECTED)
  {
    return  HAL_ERROR; 
  }
  
  /* Process Locked */
  __HAL_LOCK(hsram);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY; 

  /* Write data to memory */
  for(; BufferSize != 0; BufferSize--)
  {
    *(__IO uint8_t *)psramaddress = *pSrcBuffer; 
    pSrcBuffer++;
    psramaddress++;    
  }    

  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY; 
  
  /* Process unlocked */
  __HAL_UNLOCK(hsram);
    
  return HAL_OK;   
}

/**
  * @brief  Read 16-bit buffer from SRAM memory. 
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_16b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint16_t *pDstBuffer, uint32_t BufferSize)
{
  __IO uint16_t * psramaddress = (uint16_t *)pAddress;
  
  /* Process Locked */
  __HAL_LOCK(hsram);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY;  
  
  /* Read data from memory */
  for(; BufferSize != 0; BufferSize--)
  {
    *pDstBuffer = *(__IO uint16_t *)psramaddress;
    pDstBuffer++;
    psramaddress++;
  }
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY;    

  /* Process unlocked */
  __HAL_UNLOCK(hsram); 
    
  return HAL_OK;  
}

/**
  * @brief  Write 16-bit buffer to SRAM memory. 
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_16b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint16_t *pSrcBuffer, uint32_t BufferSize)
{
  __IO uint16_t * psramaddress = (uint16_t *)pAddress; 
  
  /* Check the SRAM controller state */
  if(hsram->State == HAL_SRAM_STATE_PROTECTED)
  {
    return  HAL_ERROR; 
  }
  
  /* Process Locked */
  __HAL_LOCK(hsram);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY; 

  /* Write data to memory */
  for(; BufferSize != 0; BufferSize--)
  {
    *(__IO uint16_t *)psramaddress = *pSrcBuffer; 
    pSrcBuffer++;
    psramaddress++;    
  }    

  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY; 
  
  /* Process unlocked */
  __HAL_UNLOCK(hsram);
    
  return HAL_OK;  
}

/**
  * @brief  Read 32-bit buffer from SRAM memory. 
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_32b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
  /* Process Locked */
  __HAL_LOCK(hsram);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY;  
  
  /* Read data from memory */
  for(; BufferSize != 0; BufferSize--)
  {
    *pDstBuffer = *(__IO uint32_t *)pAddress;
    pDstBuffer++;
    pAddress++;
  }
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY;    

  /* Process unlocked */
  __HAL_UNLOCK(hsram); 
    
  return HAL_OK;  
}

/**
  * @brief  Write 32-bit buffer to SRAM memory. 
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_32b(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pSrcBuffer, uint32_t BufferSize)
{
  /* Check the SRAM controller state */
  if(hsram->State == HAL_SRAM_STATE_PROTECTED)
  {
    return  HAL_ERROR; 
  }
  
  /* Process Locked */
  __HAL_LOCK(hsram);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY; 

  /* Write data to memory */
  for(; BufferSize != 0; BufferSize--)
  {
    *(__IO uint32_t *)pAddress = *pSrcBuffer; 
    pSrcBuffer++;
    pAddress++;    
  }    

  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY; 
  
  /* Process unlocked */
  __HAL_UNLOCK(hsram);
    
  return HAL_OK;   
}

/**
  * @brief  Read a Word data buffer from the SRAM memory using DMA transfer.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to read start address
  * @param  pDstBuffer Pointer to destination buffer
  * @param  BufferSize Size of the buffer to read from memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Read_DMA(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
  /* Process Locked */
  __HAL_LOCK(hsram);  
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY;   
  
  /* Configure DMA user callbacks */
  hsram->hdma->XferCpltCallback  = HAL_SRAM_DMA_XferCpltCallback;
  hsram->hdma->XferErrorCallback = HAL_SRAM_DMA_XferErrorCallback;

  /* Enable the DMA Channel */
  HAL_DMA_Start_IT(hsram->hdma, (uint32_t)pAddress, (uint32_t)pDstBuffer, (uint32_t)BufferSize);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY; 
  
  /* Process unlocked */
  __HAL_UNLOCK(hsram);  
  
  return HAL_OK; 
}

/**
  * @brief  Write a Word data buffer to SRAM memory using DMA transfer.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @param  pAddress Pointer to write start address
  * @param  pSrcBuffer Pointer to source buffer to write
  * @param  BufferSize Size of the buffer to write to memory
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_Write_DMA(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint32_t *pSrcBuffer, uint32_t BufferSize)
{
  /* Check the SRAM controller state */
  if(hsram->State == HAL_SRAM_STATE_PROTECTED)
  {
    return  HAL_ERROR; 
  }
  
  /* Process Locked */
  __HAL_LOCK(hsram);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY; 
  
  /* Configure DMA user callbacks */
  hsram->hdma->XferCpltCallback  = HAL_SRAM_DMA_XferCpltCallback;
  hsram->hdma->XferErrorCallback = HAL_SRAM_DMA_XferErrorCallback;

  /* Enable the DMA Channel */
  HAL_DMA_Start_IT(hsram->hdma, (uint32_t)pSrcBuffer, (uint32_t)pAddress, (uint32_t)BufferSize);
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY;  
  
  /* Process unlocked */
  __HAL_UNLOCK(hsram);  
  
  return HAL_OK;
}

/**
  * @}
  */
  
/** @defgroup SRAM_Exported_Functions_Group3 Control functions 
 *  @brief   Control functions 
 *
@verbatim   
  ==============================================================================
                        ##### SRAM Control functions #####
  ==============================================================================  
  [..]
    This subsection provides a set of functions allowing to control dynamically
    the SRAM interface.

@endverbatim
  * @{
  */
    
/**
  * @brief  Enable dynamically SRAM write operation.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_WriteOperation_Enable(SRAM_HandleTypeDef *hsram)
{
  /* Process Locked */
  __HAL_LOCK(hsram);

  /* Enable write operation */
  FMC_NORSRAM_WriteOperation_Enable(hsram->Instance, hsram->Init.NSBank); 
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_READY;
  
  /* Process unlocked */
  __HAL_UNLOCK(hsram); 
  
  return HAL_OK;  
}

/**
  * @brief  Disable dynamically SRAM write operation.
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SRAM_WriteOperation_Disable(SRAM_HandleTypeDef *hsram)
{
  /* Process Locked */
  __HAL_LOCK(hsram);

  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_BUSY;
    
  /* Disable write operation */
  FMC_NORSRAM_WriteOperation_Disable(hsram->Instance, hsram->Init.NSBank); 
  
  /* Update the SRAM controller state */
  hsram->State = HAL_SRAM_STATE_PROTECTED;
  
  /* Process unlocked */
  __HAL_UNLOCK(hsram); 
  
  return HAL_OK;  
}

/**
  * @}
  */

/** @defgroup SRAM_Exported_Functions_Group4 Peripheral State functions 
 *  @brief   Peripheral State functions 
 *
@verbatim   
  ==============================================================================
                      ##### SRAM State functions #####
  ==============================================================================  
  [..]
    This subsection permits to get in run-time the status of the SRAM controller 
    and the data flow.

@endverbatim
  * @{
  */
  
/**
  * @brief  Return the SRAM controller state
  * @param  hsram pointer to a SRAM_HandleTypeDef structure that contains
  *                the configuration information for SRAM module.
  * @retval HAL state
  */
HAL_SRAM_StateTypeDef HAL_SRAM_GetState(SRAM_HandleTypeDef *hsram)
{
  /* Return SRAM handle state */
  return hsram->State;
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

#endif /* HAL_SRAM_MODULE_ENABLED */

/**
  * @}
  */

#endif /* FMC_BANK1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
