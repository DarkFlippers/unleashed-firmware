/**
  ******************************************************************************
  * @file    stm32l4xx_ll_rng.h
  * @author  MCD Application Team
  * @brief   Header file of RNG LL module.
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
#ifndef __STM32L4xx_LL_RNG_H
#define __STM32L4xx_LL_RNG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(RNG)

/** @defgroup RNG_LL RNG
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup RNG_LL_ES_Init_Struct RNG Exported Init structures
  * @{
  */
  

#if defined(RNG_CR_CED)
/**
  * @brief LL RNG Init Structure Definition
  */
typedef struct
{
  uint32_t         ClockErrorDetection; /*!< Clock error detection.
                                      This parameter can be one value of @ref RNG_LL_CED.
                                      
                                      This parameter can be modified using unitary functions @ref LL_RNG_EnableClkErrorDetect(). */
}LL_RNG_InitTypeDef;
#endif /* defined(RNG_CR_CED) */

/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */
  
/* Exported constants --------------------------------------------------------*/
/** @defgroup RNG_LL_Exported_Constants RNG Exported Constants
  * @{
  */
  
#if defined(RNG_CR_CED)
/** @defgroup RNG_LL_CED Clock Error Detection
  * @{
  */
#define LL_RNG_CED_ENABLE         0x00000000U              /*!< Clock error detection enabled  */
#define LL_RNG_CED_DISABLE        RNG_CR_CED               /*!< Clock error detection disabled */
/**
  * @}
  */
#endif /* defined(RNG_CR_CED) */

  
/** @defgroup RNG_LL_EC_GET_FLAG Get Flags Defines
  * @brief    Flags defines which can be used with LL_RNG_ReadReg function
  * @{
  */
#define LL_RNG_SR_DRDY RNG_SR_DRDY    /*!< Register contains valid random data */
#define LL_RNG_SR_CECS RNG_SR_CECS    /*!< Clock error current status */
#define LL_RNG_SR_SECS RNG_SR_SECS    /*!< Seed error current status */
#define LL_RNG_SR_CEIS RNG_SR_CEIS    /*!< Clock error interrupt status */
#define LL_RNG_SR_SEIS RNG_SR_SEIS    /*!< Seed error interrupt status */
/**
  * @}
  */

/** @defgroup RNG_LL_EC_IT IT Defines
  * @brief    IT defines which can be used with LL_RNG_ReadReg and  LL_RNG_WriteReg macros
  * @{
  */
#define LL_RNG_CR_IE   RNG_CR_IE      /*!< RNG Interrupt enable */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup RNG_LL_Exported_Macros RNG Exported Macros
  * @{
  */

/** @defgroup RNG_LL_EM_WRITE_READ Common Write and read registers Macros
  * @{
  */

/**
  * @brief  Write a value in RNG register
  * @param  __INSTANCE__ RNG Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_RNG_WriteReg(__INSTANCE__, __REG__, __VALUE__) WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
  * @brief  Read a value in RNG register
  * @param  __INSTANCE__ RNG Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_RNG_ReadReg(__INSTANCE__, __REG__) READ_REG(__INSTANCE__->__REG__)
/**
  * @}
  */

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup RNG_LL_Exported_Functions RNG Exported Functions
  * @{
  */
/** @defgroup RNG_LL_EF_Configuration RNG Configuration functions
  * @{
  */

/**
  * @brief  Enable Random Number Generation
  * @rmtoll CR           RNGEN         LL_RNG_Enable
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_Enable(RNG_TypeDef *RNGx)
{
  SET_BIT(RNGx->CR, RNG_CR_RNGEN);
}

/**
  * @brief  Disable Random Number Generation
  * @rmtoll CR           RNGEN         LL_RNG_Disable
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_Disable(RNG_TypeDef *RNGx)
{
  CLEAR_BIT(RNGx->CR, RNG_CR_RNGEN);
}

/**
  * @brief  Check if Random Number Generator is enabled
  * @rmtoll CR           RNGEN         LL_RNG_IsEnabled
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsEnabled(RNG_TypeDef *RNGx)
{
  return (READ_BIT(RNGx->CR, RNG_CR_RNGEN) == (RNG_CR_RNGEN));
}

#if defined(RNG_CR_CED)
/**
  * @brief  Enable RNG Clock Error Detection
  * @rmtoll CR           CED         LL_RNG_EnableClkErrorDetect
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_EnableClkErrorDetect(RNG_TypeDef *RNGx)
{
  CLEAR_BIT(RNGx->CR, RNG_CR_CED);
}

/**
  * @brief  Disable RNG Clock Error Detection
  * @rmtoll CR           CED         LL_RNG_DisableClkErrorDetect
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_DisableClkErrorDetect(RNG_TypeDef *RNGx)
{
  SET_BIT(RNGx->CR, RNG_CR_CED);
}

/**
  * @brief  Check if RNG Clock Error Detection is enabled
  * @rmtoll CR           CED         LL_RNG_IsEnabledClkErrorDetect
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsEnabledClkErrorDetect(RNG_TypeDef *RNGx)
{
  return (!(READ_BIT(RNGx->CR, RNG_CR_CED) == (RNG_CR_CED)));
}
#endif /* defined(RNG_CR_CED) */


/**
  * @}
  */

/** @defgroup RNG_LL_EF_FLAG_Management FLAG Management
  * @{
  */

/**
  * @brief  Indicate if the RNG Data ready Flag is set or not
  * @rmtoll SR           DRDY          LL_RNG_IsActiveFlag_DRDY
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsActiveFlag_DRDY(RNG_TypeDef *RNGx)
{
  return (READ_BIT(RNGx->SR, RNG_SR_DRDY) == (RNG_SR_DRDY));
}

/**
  * @brief  Indicate if the Clock Error Current Status Flag is set or not
  * @rmtoll SR           CECS          LL_RNG_IsActiveFlag_CECS
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsActiveFlag_CECS(RNG_TypeDef *RNGx)
{
  return (READ_BIT(RNGx->SR, RNG_SR_CECS) == (RNG_SR_CECS));
}

/**
  * @brief  Indicate if the Seed Error Current Status Flag is set or not
  * @rmtoll SR           SECS          LL_RNG_IsActiveFlag_SECS
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsActiveFlag_SECS(RNG_TypeDef *RNGx)
{
  return (READ_BIT(RNGx->SR, RNG_SR_SECS) == (RNG_SR_SECS));
}

/**
  * @brief  Indicate if the Clock Error Interrupt Status Flag is set or not
  * @rmtoll SR           CEIS          LL_RNG_IsActiveFlag_CEIS
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsActiveFlag_CEIS(RNG_TypeDef *RNGx)
{
  return (READ_BIT(RNGx->SR, RNG_SR_CEIS) == (RNG_SR_CEIS));
}

/**
  * @brief  Indicate if the Seed Error Interrupt Status Flag is set or not
  * @rmtoll SR           SEIS          LL_RNG_IsActiveFlag_SEIS
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsActiveFlag_SEIS(RNG_TypeDef *RNGx)
{
  return (READ_BIT(RNGx->SR, RNG_SR_SEIS) == (RNG_SR_SEIS));
}

/**
  * @brief  Clear Clock Error interrupt Status (CEIS) Flag
  * @rmtoll SR           CEIS          LL_RNG_ClearFlag_CEIS
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_ClearFlag_CEIS(RNG_TypeDef *RNGx)
{
  WRITE_REG(RNGx->SR, ~RNG_SR_CEIS);
}

/**
  * @brief  Clear Seed Error interrupt Status (SEIS) Flag
  * @rmtoll SR           SEIS          LL_RNG_ClearFlag_SEIS
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_ClearFlag_SEIS(RNG_TypeDef *RNGx)
{
  WRITE_REG(RNGx->SR, ~RNG_SR_SEIS);
}

/**
  * @}
  */

/** @defgroup RNG_LL_EF_IT_Management IT Management
  * @{
  */

/**
  * @brief  Enable Random Number Generator Interrupt
  *         (applies for either Seed error, Clock Error or Data ready interrupts)
  * @rmtoll CR           IE            LL_RNG_EnableIT
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_EnableIT(RNG_TypeDef *RNGx)
{
  SET_BIT(RNGx->CR, RNG_CR_IE);
}

/**
  * @brief  Disable Random Number Generator Interrupt
  *         (applies for either Seed error, Clock Error or Data ready interrupts)
  * @rmtoll CR           IE            LL_RNG_DisableIT
  * @param  RNGx RNG Instance
  * @retval None
  */
__STATIC_INLINE void LL_RNG_DisableIT(RNG_TypeDef *RNGx)
{
  CLEAR_BIT(RNGx->CR, RNG_CR_IE);
}

/**
  * @brief  Check if Random Number Generator Interrupt is enabled
  *         (applies for either Seed error, Clock Error or Data ready interrupts)
  * @rmtoll CR           IE            LL_RNG_IsEnabledIT
  * @param  RNGx RNG Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RNG_IsEnabledIT(RNG_TypeDef *RNGx)
{
  return (READ_BIT(RNGx->CR, RNG_CR_IE) == (RNG_CR_IE));
}

/**
  * @}
  */

/** @defgroup RNG_LL_EF_Data_Management Data Management
  * @{
  */

/**
  * @brief  Return32-bit Random Number value
  * @rmtoll DR           RNDATA        LL_RNG_ReadRandData32
  * @param  RNGx RNG Instance
  * @retval Generated 32-bit random value
  */
__STATIC_INLINE uint32_t LL_RNG_ReadRandData32(RNG_TypeDef *RNGx)
{
  return (uint32_t)(READ_REG(RNGx->DR));
}

/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup RNG_LL_EF_Init Initialization and de-initialization functions
  * @{
  */
#if defined(RNG_CR_CED)
ErrorStatus LL_RNG_Init(RNG_TypeDef *RNGx, LL_RNG_InitTypeDef *RNG_InitStruct);
void LL_RNG_StructInit(LL_RNG_InitTypeDef *RNG_InitStruct);
#endif /* defined(RNG_CR_CED) */
ErrorStatus LL_RNG_DeInit(RNG_TypeDef *RNGx);

/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/**
  * @}
  */

/**
  * @}
  */

#endif /* defined(RNG) */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_LL_RNG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
