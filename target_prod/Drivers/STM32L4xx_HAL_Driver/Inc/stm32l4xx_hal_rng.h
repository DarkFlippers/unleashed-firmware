/**
  ******************************************************************************
  * @file    stm32l4xx_hal_rng.h
  * @author  MCD Application Team
  * @brief   Header file of RNG HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L4xx_HAL_RNG_H
#define __STM32L4xx_HAL_RNG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup RNG
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/ 
/** @defgroup RNG_Exported_Types RNG Exported Types
  * @{
  */

#if defined(RNG_CR_CED)
/**
  * @brief  RNG Configuration Structure definition
  */
typedef struct
{
  uint32_t                    ClockErrorDetection; /*!< Clock error detection */
}RNG_InitTypeDef;
#endif /* defined(RNG_CR_CED) */

/** 
  * @brief  RNG HAL State Structure definition  
  */ 
typedef enum
{
  HAL_RNG_STATE_RESET     = 0x00,  /*!< RNG not yet initialized or disabled */
  HAL_RNG_STATE_READY     = 0x01,  /*!< RNG initialized and ready for use   */
  HAL_RNG_STATE_BUSY      = 0x02,  /*!< RNG internal process is ongoing     */ 
  HAL_RNG_STATE_TIMEOUT   = 0x03,  /*!< RNG timeout state                   */
  HAL_RNG_STATE_ERROR     = 0x04   /*!< RNG error state                     */

}HAL_RNG_StateTypeDef;

/** 
  * @brief  RNG Handle Structure definition  
  */ 
typedef struct __RNG_HandleTypeDef
{
  RNG_TypeDef                 *Instance;     /*!< Register base address        */

#if defined(RNG_CR_CED)
  RNG_InitTypeDef             Init;          /*!< RNG configuration parameters */
#endif /* defined(RNG_CR_CED) */

  HAL_LockTypeDef             Lock;          /*!< RNG locking object           */

  __IO HAL_RNG_StateTypeDef   State;         /*!< RNG communication state      */

  __IO  uint32_t              ErrorCode;     /*!< RNG Error code               */

  uint32_t                    RandomNumber;  /*!< Last Generated RNG Data      */

#if (USE_HAL_RNG_REGISTER_CALLBACKS == 1)
  void (* ReadyDataCallback)(struct __RNG_HandleTypeDef *hrng, uint32_t random32bit);  /*!< RNG Data Ready Callback    */
  void (* ErrorCallback)(struct __RNG_HandleTypeDef *hrng);                            /*!< RNG Error Callback         */

  void (* MspInitCallback)(struct __RNG_HandleTypeDef *hrng);                          /*!< RNG Msp Init callback      */
  void (* MspDeInitCallback)(struct __RNG_HandleTypeDef *hrng);                        /*!< RNG Msp DeInit callback    */
#endif  /* USE_HAL_RNG_REGISTER_CALLBACKS */

}RNG_HandleTypeDef;

#if (USE_HAL_RNG_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL RNG Callback ID enumeration definition
  */
typedef enum
{
  HAL_RNG_ERROR_CB_ID                   = 0x00U,     /*!< RNG Error Callback ID          */

  HAL_RNG_MSPINIT_CB_ID                 = 0x01U,     /*!< RNG MspInit callback ID        */
  HAL_RNG_MSPDEINIT_CB_ID               = 0x02U      /*!< RNG MspDeInit callback ID      */

} HAL_RNG_CallbackIDTypeDef;

/**
  * @brief  HAL RNG Callback pointer definition
  */
typedef  void (*pRNG_CallbackTypeDef)(RNG_HandleTypeDef *hrng);                                  /*!< pointer to a common RNG callback function */
typedef  void (*pRNG_ReadyDataCallbackTypeDef)(RNG_HandleTypeDef * hrng, uint32_t random32bit);  /*!< pointer to an RNG Data Ready specific callback function */

#endif /* USE_HAL_RNG_REGISTER_CALLBACKS */

/** 
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup RNG_Exported_Constants RNG Exported Constants
  * @{
  */

/** @defgroup RNG_Interrupt_definition  RNG Interrupts Definition
  * @{
  */
#define RNG_IT_DRDY  RNG_SR_DRDY  /*!< Data Ready interrupt  */
#define RNG_IT_CEI   RNG_SR_CEIS  /*!< Clock error interrupt */
#define RNG_IT_SEI   RNG_SR_SEIS  /*!< Seed error interrupt  */
/**
  * @}
  */

/** @defgroup RNG_Flag_definition  RNG Flags Definition
  * @{
  */ 
#define RNG_FLAG_DRDY   RNG_SR_DRDY  /*!< Data ready                 */
#define RNG_FLAG_CECS   RNG_SR_CECS  /*!< Clock error current status */
#define RNG_FLAG_SECS   RNG_SR_SECS  /*!< Seed error current status  */
/**
  * @}
  */

#if defined(RNG_CR_CED)
/** @defgroup RNG_Clock_Error_Detection RNG Clock Error Detection
  * @{
  */
#define RNG_CED_ENABLE         ((uint32_t)0x00000000) /*!< Clock error detection enabled  */
#define RNG_CED_DISABLE        RNG_CR_CED             /*!< Clock error detection disabled */
/**
  * @}
  */
#endif /* defined(RNG_CR_CED) */

/** @defgroup RNG_Error_Definition   RNG Error Definition
  * @{
  */
#define  HAL_RNG_ERROR_NONE             ((uint32_t)0x00000000U)    /*!< No error                */
#if (USE_HAL_RNG_REGISTER_CALLBACKS == 1)
#define  HAL_RNG_ERROR_INVALID_CALLBACK ((uint32_t)0x00000001U)    /*!< Invalid Callback error  */
#endif /* USE_HAL_RNG_REGISTER_CALLBACKS */
/**
  * @}
  */

/**
  * @}
  */ 
  
/* Exported macros -----------------------------------------------------------*/
/** @defgroup RNG_Exported_Macros RNG Exported Macros
  * @{
  */

/** @brief Reset RNG handle state.
  * @param  __HANDLE__: RNG Handle
  * @retval None
  */
#if (USE_HAL_RNG_REGISTER_CALLBACKS == 1)
#define __HAL_RNG_RESET_HANDLE_STATE(__HANDLE__)  do{                                                   \
                                                       (__HANDLE__)->State = HAL_RNG_STATE_RESET;       \
                                                       (__HANDLE__)->MspInitCallback = NULL;            \
                                                       (__HANDLE__)->MspDeInitCallback = NULL;          \
                                                    } while(0U)
#else
#define __HAL_RNG_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_RNG_STATE_RESET)
#endif /*USE_HAL_RNG_REGISTER_CALLBACKS */

/**
  * @brief  Enable the RNG peripheral.
  * @param  __HANDLE__: RNG Handle
  * @retval None
  */
#define __HAL_RNG_ENABLE(__HANDLE__) ((__HANDLE__)->Instance->CR |=  RNG_CR_RNGEN)

/**
  * @brief  Disable the RNG peripheral.
  * @param  __HANDLE__: RNG Handle
  * @retval None
  */
#define __HAL_RNG_DISABLE(__HANDLE__) ((__HANDLE__)->Instance->CR &= ~RNG_CR_RNGEN)

/**
  * @brief  Check whether the specified RNG flag is set or not.
  * @param  __HANDLE__: RNG Handle
  * @param  __FLAG__: RNG flag
  *          This parameter can be one of the following values:
  *            @arg RNG_FLAG_DRDY:  Data ready                
  *            @arg RNG_FLAG_CECS:  Clock error current status
  *            @arg RNG_FLAG_SECS:  Seed error current status 
  * @retval The new state of __FLAG__ (SET or RESET).
  */
#define __HAL_RNG_GET_FLAG(__HANDLE__, __FLAG__) (((__HANDLE__)->Instance->SR & (__FLAG__)) == (__FLAG__))

/**
  * @brief  Clear the selected RNG flag status.
  * @param  __HANDLE__: RNG handle
  * @param  __FLAG__: RNG flag to clear  
  * @note   WARNING: This is a dummy macro for HAL code alignment,
  *         flags RNG_FLAG_DRDY, RNG_FLAG_CECS and RNG_FLAG_SECS are read-only.
  * @retval None
  */
#define __HAL_RNG_CLEAR_FLAG(__HANDLE__, __FLAG__)                      /* dummy  macro */

/**
  * @brief  Enable the RNG interrupt.
  * @param  __HANDLE__: RNG Handle
  * @retval None
  */
#define __HAL_RNG_ENABLE_IT(__HANDLE__) ((__HANDLE__)->Instance->CR |=  RNG_CR_IE)

/**
  * @brief  Disable the RNG interrupt.
  * @param  __HANDLE__: RNG Handle
  * @retval None
  */
#define __HAL_RNG_DISABLE_IT(__HANDLE__) ((__HANDLE__)->Instance->CR &= ~RNG_CR_IE)

/**
  * @brief  Check whether the specified RNG interrupt has occurred or not.
  * @param  __HANDLE__: RNG Handle
  * @param  __INTERRUPT__: specifies the RNG interrupt status flag to check.
  *         This parameter can be one of the following values:
  *            @arg RNG_IT_DRDY: Data ready interrupt              
  *            @arg RNG_IT_CEI: Clock error interrupt
  *            @arg RNG_IT_SEI: Seed error interrupt
  * @retval The new state of __INTERRUPT__ (SET or RESET).
  */
#define __HAL_RNG_GET_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->SR & (__INTERRUPT__)) == (__INTERRUPT__))   

/**
  * @brief  Clear the RNG interrupt status flags.
  * @param  __HANDLE__: RNG Handle
  * @param  __INTERRUPT__: specifies the RNG interrupt status flag to clear.
  *          This parameter can be one of the following values:            
  *            @arg RNG_IT_CEI: Clock error interrupt
  *            @arg RNG_IT_SEI: Seed error interrupt
  * @note   RNG_IT_DRDY flag is read-only, reading RNG_DR register automatically clears RNG_IT_DRDY.          
  * @retval None
  */
#define __HAL_RNG_CLEAR_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->SR) = ~(__INTERRUPT__))

/**
  * @}
  */ 


/* Exported functions --------------------------------------------------------*/
/** @defgroup RNG_Exported_Functions RNG Exported Functions
  * @{
  */

/* Initialization and de-initialization functions  ******************************/
/** @defgroup RNG_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */  
HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng);
HAL_StatusTypeDef HAL_RNG_DeInit (RNG_HandleTypeDef *hrng);
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng);
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng);

/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_RNG_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef HAL_RNG_RegisterCallback(RNG_HandleTypeDef *hrng, HAL_RNG_CallbackIDTypeDef CallbackID, pRNG_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_RNG_UnRegisterCallback(RNG_HandleTypeDef *hrng, HAL_RNG_CallbackIDTypeDef CallbackID);

HAL_StatusTypeDef HAL_RNG_RegisterReadyDataCallback(RNG_HandleTypeDef *hrng, pRNG_ReadyDataCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_RNG_UnRegisterReadyDataCallback(RNG_HandleTypeDef *hrng);
#endif /* USE_HAL_RNG_REGISTER_CALLBACKS */

/**
  * @}
  */ 

/* Peripheral Control functions  ************************************************/
/** @defgroup RNG_Exported_Functions_Group2 Peripheral Control functions
  * @{
  */
uint32_t HAL_RNG_GetRandomNumber(RNG_HandleTypeDef *hrng);    /* Obsolete, use HAL_RNG_GenerateRandomNumber() instead    */
uint32_t HAL_RNG_GetRandomNumber_IT(RNG_HandleTypeDef *hrng); /* Obsolete, use HAL_RNG_GenerateRandomNumber_IT() instead */

HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef *hrng, uint32_t *random32bit);
HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber_IT(RNG_HandleTypeDef *hrng);
uint32_t HAL_RNG_ReadLastRandomNumber(RNG_HandleTypeDef *hrng);

void HAL_RNG_IRQHandler(RNG_HandleTypeDef *hrng);
void HAL_RNG_ErrorCallback(RNG_HandleTypeDef *hrng);
void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef* hrng, uint32_t random32bit);
/**
  * @}
  */ 

/* Peripheral State functions  **************************************************/
/** @defgroup RNG_Exported_Functions_Group3 Peripheral State and Error functions
  * @{
  */
HAL_RNG_StateTypeDef HAL_RNG_GetState(RNG_HandleTypeDef *hrng);
uint32_t             HAL_RNG_GetError(RNG_HandleTypeDef *hrng);
/**
  * @}
  */
  
/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/** @addtogroup  RNG_Private_Macros   RNG Private Macros
  * @{
  */

#if defined(RNG_CR_CED)
/**
  * @brief Verify the RNG Clock Error Detection mode.
  * @param __MODE__: RNG Clock Error Detection mode
  * @retval SET (__MODE__ is valid) or RESET (__MODE__ is invalid)
  */
#define IS_RNG_CED(__MODE__) (((__MODE__) == RNG_CED_ENABLE) || \
                              ((__MODE__) == RNG_CED_DISABLE))
#endif /* defined(RNG_CR_CED) */

/**
  * @}
  */
/* Private functions prototypes ----------------------------------------------*/

/**
  * @}
  */ 

/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_HAL_RNG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
