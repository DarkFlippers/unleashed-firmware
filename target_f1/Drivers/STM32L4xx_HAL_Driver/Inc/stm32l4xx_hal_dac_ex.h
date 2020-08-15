/**
  ******************************************************************************
  * @file    stm32l4xx_hal_dac_ex.h
  * @author  MCD Application Team
  * @brief   Header file of DAC HAL Extended module.
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
#ifndef STM32L4xx_HAL_DAC_EX_H
#define STM32L4xx_HAL_DAC_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

#if defined(DAC1)

/** @addtogroup DACEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  HAL State structures definition
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup DACEx_Exported_Constants DACEx Exported Constants
  * @{
  */

/** @defgroup DACEx_lfsrunmask_triangleamplitude DACEx lfsrunmask triangle amplitude
  * @{
  */
#define DAC_LFSRUNMASK_BIT0                0x00000000U                                                         /*!< Unmask DAC channel LFSR bit0 for noise wave generation */
#define DAC_LFSRUNMASK_BITS1_0             (                                                   DAC_CR_MAMP1_0) /*!< Unmask DAC channel LFSR bit[1:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS2_0             (                                  DAC_CR_MAMP1_1                 ) /*!< Unmask DAC channel LFSR bit[2:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS3_0             (                                  DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Unmask DAC channel LFSR bit[3:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS4_0             (                 DAC_CR_MAMP1_2                                  ) /*!< Unmask DAC channel LFSR bit[4:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS5_0             (                 DAC_CR_MAMP1_2                  | DAC_CR_MAMP1_0) /*!< Unmask DAC channel LFSR bit[5:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS6_0             (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1                 ) /*!< Unmask DAC channel LFSR bit[6:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS7_0             (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Unmask DAC channel LFSR bit[7:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS8_0             (DAC_CR_MAMP1_3                                                   ) /*!< Unmask DAC channel LFSR bit[8:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS9_0             (DAC_CR_MAMP1_3                                   | DAC_CR_MAMP1_0) /*!< Unmask DAC channel LFSR bit[9:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS10_0            (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1                 ) /*!< Unmask DAC channel LFSR bit[10:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS11_0            (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Unmask DAC channel LFSR bit[11:0] for noise wave generation */
#define DAC_TRIANGLEAMPLITUDE_1            0x00000000U                                                         /*!< Select max triangle amplitude of 1 */
#define DAC_TRIANGLEAMPLITUDE_3            (                                                   DAC_CR_MAMP1_0) /*!< Select max triangle amplitude of 3 */
#define DAC_TRIANGLEAMPLITUDE_7            (                                  DAC_CR_MAMP1_1                 ) /*!< Select max triangle amplitude of 7 */
#define DAC_TRIANGLEAMPLITUDE_15           (                                  DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Select max triangle amplitude of 15 */
#define DAC_TRIANGLEAMPLITUDE_31           (                 DAC_CR_MAMP1_2                                  ) /*!< Select max triangle amplitude of 31 */
#define DAC_TRIANGLEAMPLITUDE_63           (                 DAC_CR_MAMP1_2                  | DAC_CR_MAMP1_0) /*!< Select max triangle amplitude of 63 */
#define DAC_TRIANGLEAMPLITUDE_127          (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1                 ) /*!< Select max triangle amplitude of 127 */
#define DAC_TRIANGLEAMPLITUDE_255          (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Select max triangle amplitude of 255 */
#define DAC_TRIANGLEAMPLITUDE_511          (DAC_CR_MAMP1_3                                                   ) /*!< Select max triangle amplitude of 511 */
#define DAC_TRIANGLEAMPLITUDE_1023         (DAC_CR_MAMP1_3                                   | DAC_CR_MAMP1_0) /*!< Select max triangle amplitude of 1023 */
#define DAC_TRIANGLEAMPLITUDE_2047         (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1                 ) /*!< Select max triangle amplitude of 2047 */
#define DAC_TRIANGLEAMPLITUDE_4095         (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Select max triangle amplitude of 4095 */

/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/

/** @defgroup DACEx_Private_Macros DACEx Private Macros
  * @{
  */
#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx)
#define IS_DAC_TRIGGER(TRIGGER) (((TRIGGER) == DAC_TRIGGER_NONE)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T2_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T6_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T7_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_EXT_IT9) || \
                                 ((TRIGGER) == DAC_TRIGGER_SOFTWARE))
#endif     /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx */

#if defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx)
#define IS_DAC_TRIGGER(TRIGGER) (((TRIGGER) == DAC_TRIGGER_NONE)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T2_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T6_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_EXT_IT9) || \
                                 ((TRIGGER) == DAC_TRIGGER_SOFTWARE))
#endif     /* STM32L451xx STM32L452xx STM32L462xx                         */

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx)
#define IS_DAC_TRIGGER(TRIGGER) (((TRIGGER) == DAC_TRIGGER_NONE)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T2_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T4_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T5_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T6_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T7_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_T8_TRGO) || \
                                 ((TRIGGER) == DAC_TRIGGER_EXT_IT9) || \
                                 ((TRIGGER) == DAC_TRIGGER_SOFTWARE))
#endif     /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
#define IS_DAC_TRIGGER(TRIGGER) (((TRIGGER) == DAC_TRIGGER_NONE)       || \
                                 ((TRIGGER) == DAC_TRIGGER_T1_TRGO)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T2_TRGO)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T4_TRGO)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T5_TRGO)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T6_TRGO)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T7_TRGO)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T8_TRGO)    || \
                                 ((TRIGGER) == DAC_TRIGGER_T15_TRGO)   || \
                                 ((TRIGGER) == DAC_TRIGGER_LPTIM1_OUT) || \
                                 ((TRIGGER) == DAC_TRIGGER_LPTIM2_OUT) || \
                                 ((TRIGGER) == DAC_TRIGGER_EXT_IT9)    || \
                                 ((TRIGGER) == DAC_TRIGGER_SOFTWARE))

#define  IS_DAC_HIGH_FREQUENCY_MODE(MODE) (((MODE) == DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE)         || \
                                           ((MODE) == DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ)     || \
                                           ((MODE) == DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC))

#endif  /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx */

#define IS_DAC_SAMPLETIME(TIME) ((TIME) <= 0x000003FFU)

#define IS_DAC_HOLDTIME(TIME)   ((TIME) <= 0x000003FFU)

#define IS_DAC_SAMPLEANDHOLD(MODE) (((MODE) == DAC_SAMPLEANDHOLD_DISABLE) || \
                                    ((MODE) == DAC_SAMPLEANDHOLD_ENABLE))

#define IS_DAC_TRIMMINGVALUE(TRIMMINGVALUE) ((TRIMMINGVALUE) <= 0x1FU)

#define IS_DAC_NEWTRIMMINGVALUE(TRIMMINGVALUE) ((TRIMMINGVALUE) <= 0x1FU)

#define IS_DAC_CHIP_CONNECTION(CONNECT) (((CONNECT) == DAC_CHIPCONNECT_DISABLE) || \
                                         ((CONNECT) == DAC_CHIPCONNECT_ENABLE))

#define IS_DAC_TRIMMING(TRIMMING) (((TRIMMING) == DAC_TRIMMING_FACTORY) || \
                                     ((TRIMMING) == DAC_TRIMMING_USER))

#define IS_DAC_LFSR_UNMASK_TRIANGLE_AMPLITUDE(VALUE) (((VALUE) == DAC_LFSRUNMASK_BIT0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS1_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS2_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS3_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS4_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS5_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS6_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS7_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS8_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS9_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS10_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS11_0) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_1) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_3) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_7) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_15) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_31) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_63) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_127) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_255) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_511) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_1023) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_2047) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_4095))
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/* Extended features functions ***********************************************/

/** @addtogroup DACEx_Exported_Functions
  * @{
  */

/** @addtogroup DACEx_Exported_Functions_Group2
 * @{
 */
/* IO operation functions *****************************************************/

HAL_StatusTypeDef HAL_DACEx_TriangleWaveGenerate(DAC_HandleTypeDef* hdac, uint32_t Channel, uint32_t Amplitude);
HAL_StatusTypeDef HAL_DACEx_NoiseWaveGenerate(DAC_HandleTypeDef* hdac, uint32_t Channel, uint32_t Amplitude);

#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
HAL_StatusTypeDef HAL_DACEx_DualSetValue(DAC_HandleTypeDef* hdac, uint32_t Alignment, uint32_t Data1, uint32_t Data2);
uint32_t HAL_DACEx_DualGetValue(DAC_HandleTypeDef* hdac);
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */
#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)
void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac);
void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac);
void HAL_DACEx_ErrorCallbackCh2(DAC_HandleTypeDef* hdac);
void HAL_DACEx_DMAUnderrunCallbackCh2(DAC_HandleTypeDef* hdac);
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */


/**
  * @}
  */

/** @addtogroup DACEx_Exported_Functions_Group3
  * @{
  */
/* Peripheral Control functions ***********************************************/

HAL_StatusTypeDef HAL_DACEx_SelfCalibrate(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel);
HAL_StatusTypeDef HAL_DACEx_SetUserTrimming(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel, uint32_t NewTrimmingValue);
uint32_t HAL_DACEx_GetTrimOffset (DAC_HandleTypeDef *hdac, uint32_t Channel);

/**
  * @}
  */

/**
  * @}
  */

#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined(STM32L4S9xx)

/** @addtogroup DACEx_Private_Functions
  * @{
  */

/* DAC_DMAConvCpltCh2 / DAC_DMAErrorCh2 / DAC_DMAHalfConvCpltCh2 */
/* are called by HAL_DAC_Start_DMA */
void DAC_DMAConvCpltCh2(DMA_HandleTypeDef *hdma);
void DAC_DMAErrorCh2(DMA_HandleTypeDef *hdma);
void DAC_DMAHalfConvCpltCh2(DMA_HandleTypeDef *hdma);

/**
 * @}
 */
#endif  /* STM32L431xx STM32L432xx STM32L433xx STM32L442xx STM32L443xx                         */
        /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx STM32L496xx STM32L4A6xx */
        /* STM32L4R5xx STM32L4R7xx STM32L4R9xx STM32L4S5xx STM32L4S7xx STM32L4S9xx             */
/**
  * @}
  */

#endif /* DAC1 */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /*STM32L4xx_HAL_DAC_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
