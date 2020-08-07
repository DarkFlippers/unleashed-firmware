/**
  ******************************************************************************
  * @file    stm32l4xx_hal_tim_ex.h
  * @author  MCD Application Team
  * @brief   Header file of TIM HAL Extended module.
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
#ifndef STM32L4xx_HAL_TIM_EX_H
#define STM32L4xx_HAL_TIM_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup TIMEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup TIMEx_Exported_Types TIM Extended Exported Types
  * @{
  */

/**
  * @brief  TIM Hall sensor Configuration Structure definition
  */

typedef struct
{
  uint32_t IC1Polarity;         /*!< Specifies the active edge of the input signal.
                                     This parameter can be a value of @ref TIM_Input_Capture_Polarity */

  uint32_t IC1Prescaler;        /*!< Specifies the Input Capture Prescaler.
                                     This parameter can be a value of @ref TIM_Input_Capture_Prescaler */

  uint32_t IC1Filter;           /*!< Specifies the input capture filter.
                                     This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */

  uint32_t Commutation_Delay;   /*!< Specifies the pulse value to be loaded into the Capture Compare Register.
                                     This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF */
} TIM_HallSensor_InitTypeDef;

/**
  * @brief  TIM Break/Break2 input configuration
  */
typedef struct
{
  uint32_t Source;         /*!< Specifies the source of the timer break input.
                                This parameter can be a value of @ref TIMEx_Break_Input_Source */
  uint32_t Enable;         /*!< Specifies whether or not the break input source is enabled.
                                This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
  uint32_t Polarity;       /*!< Specifies the break input source polarity.
                                This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity
                                Not relevant when analog watchdog output of the DFSDM1 used as break input source */
}
TIMEx_BreakInputConfigTypeDef;

/**
  * @}
  */
/* End of exported types -----------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/** @defgroup TIMEx_Exported_Constants TIM Extended Exported Constants
  * @{
  */

/** @defgroup TIMEx_Remap TIM Extended Remapping
  * @{
  */
#define TIM_TIM1_ETR_ADC1_NONE      0x00000000U                                           /* !< TIM1_ETR is not connected to any AWD (analog watchdog)*/
#define TIM_TIM1_ETR_ADC1_AWD1      TIM1_OR1_ETR_ADC1_RMP_0                               /* !< TIM1_ETR is connected to ADC1 AWD1 */
#define TIM_TIM1_ETR_ADC1_AWD2      TIM1_OR1_ETR_ADC1_RMP_1                               /* !< TIM1_ETR is connected to ADC1 AWD2 */
#define TIM_TIM1_ETR_ADC1_AWD3      (TIM1_OR1_ETR_ADC1_RMP_1 | TIM1_OR1_ETR_ADC1_RMP_0)   /* !< TIM1_ETR is connected to ADC1 AWD3 */
#if defined (ADC3)
#define TIM_TIM1_ETR_ADC3_NONE      0x00000000U                                           /* !< TIM1_ETR is not connected to any AWD (analog watchdog)*/
#define TIM_TIM1_ETR_ADC3_AWD1      TIM1_OR1_ETR_ADC3_RMP_0                               /* !< TIM1_ETR is connected to ADC3 AWD1 */
#define TIM_TIM1_ETR_ADC3_AWD2      TIM1_OR1_ETR_ADC3_RMP_1                               /* !< TIM1_ETR is connected to ADC3 AWD2 */
#define TIM_TIM1_ETR_ADC3_AWD3      (TIM1_OR1_ETR_ADC3_RMP_1 | TIM1_OR1_ETR_ADC3_RMP_0)   /* !< TIM1_ETR is connected to ADC3 AWD3 */
#endif /* ADC3 */
#define TIM_TIM1_TI1_GPIO           0x00000000U                                           /* !< TIM1 TI1 is connected to GPIO */
#define TIM_TIM1_TI1_COMP1          TIM1_OR1_TI1_RMP                                      /* !< TIM1 TI1 is connected to COMP1 */
#define TIM_TIM1_ETR_GPIO           0x00000000U                                           /* !< TIM1_ETR is connected to GPIO */
#define TIM_TIM1_ETR_COMP1          TIM1_OR2_ETRSEL_0                                     /* !< TIM1_ETR is connected to COMP1 output */
#if defined(COMP2)
#define TIM_TIM1_ETR_COMP2          TIM1_OR2_ETRSEL_1                                     /* !< TIM1_ETR is connected to COMP2 output */
#endif /* COMP2 */

#if defined (USB_OTG_FS)
#define TIM_TIM2_ITR1_TIM8_TRGO     0x00000000U                                           /* !< TIM2_ITR1 is connected to TIM8_TRGO */
#define TIM_TIM2_ITR1_OTG_FS_SOF    TIM2_OR1_ITR1_RMP                                     /* !< TIM2_ITR1 is connected to OTG_FS SOF */
#else
#if defined(STM32L471xx)
#define TIM_TIM2_ITR1_TIM8_TRGO     0x00000000U                                           /* !< TIM2_ITR1 is connected to TIM8_TRGO */
#define TIM_TIM2_ITR1_NONE          TIM2_OR1_ITR1_RMP                                     /* !< No internal trigger on TIM2_ITR1 */
#else
#define TIM_TIM2_ITR1_NONE          0x00000000U                                           /* !< No internal trigger on TIM2_ITR1 */
#define TIM_TIM2_ITR1_USB_SOF       TIM2_OR1_ITR1_RMP                                     /* !< TIM2_ITR1 is connected to USB SOF */
#endif /* STM32L471xx */
#endif /* USB_OTG_FS */
#define TIM_TIM2_ETR_GPIO           0x00000000U                                           /* !< TIM2_ETR is connected to GPIO */
#define TIM_TIM2_ETR_LSE            TIM2_OR1_ETR1_RMP                                     /* !< TIM2_ETR is connected to LSE */
#define TIM_TIM2_ETR_COMP1          TIM2_OR2_ETRSEL_0                                     /* !< TIM2_ETR is connected to COMP1 output */
#if defined(COMP2)
#define TIM_TIM2_ETR_COMP2          TIM2_OR2_ETRSEL_1                                     /* !< TIM2_ETR is connected to COMP2 output */
#endif /* COMP2 */
#define TIM_TIM2_TI4_GPIO           0x00000000U                                           /* !< TIM2 TI4 is connected to GPIO */
#define TIM_TIM2_TI4_COMP1          TIM2_OR1_TI4_RMP_0                                    /* !< TIM2 TI4 is connected to COMP1 output */
#if defined(COMP2)
#define TIM_TIM2_TI4_COMP2          TIM2_OR1_TI4_RMP_1                                    /* !< TIM2 TI4 is connected to COMP2 output */
#define TIM_TIM2_TI4_COMP1_COMP2    (TIM2_OR1_TI4_RMP_1| TIM2_OR1_TI4_RMP_0)              /* !< TIM2 TI4 is connected to logical OR between COMP1 and COMP2 output2 */
#endif /* COMP2 */

#if defined (TIM3)
#define TIM_TIM3_TI1_GPIO           0x00000000U                                           /* !< TIM3 TI1 is connected to GPIO */
#define TIM_TIM3_TI1_COMP1          TIM3_OR1_TI1_RMP_0                                    /* !< TIM3 TI1 is connected to COMP1 output */
#define TIM_TIM3_TI1_COMP2          TIM3_OR1_TI1_RMP_1                                    /* !< TIM3 TI1 is connected to COMP2 output */
#define TIM_TIM3_TI1_COMP1_COMP2    (TIM3_OR1_TI1_RMP_1 | TIM3_OR1_TI1_RMP_0)             /* !< TIM3 TI1 is connected to logical OR between COMP1 and COMP2 output2 */
#define TIM_TIM3_ETR_GPIO           0x00000000U                                           /* !< TIM3_ETR is connected to GPIO */
#define TIM_TIM3_ETR_COMP1          TIM3_OR2_ETRSEL_0                                     /* !< TIM3_ETR is connected to COMP1 output */
#endif /* TIM3 */

#if defined (TIM8)
#if defined(ADC2) && defined(ADC3)
#define TIM_TIM8_ETR_ADC2_NONE      0x00000000U                                           /* !< TIM8_ETR is not connected to any AWD (analog watchdog)*/
#define TIM_TIM8_ETR_ADC2_AWD1      TIM8_OR1_ETR_ADC2_RMP_0                               /* !< TIM8_ETR is connected to ADC2 AWD1 */
#define TIM_TIM8_ETR_ADC2_AWD2      TIM8_OR1_ETR_ADC2_RMP_1                               /* !< TIM8_ETR is connected to ADC2 AWD2 */
#define TIM_TIM8_ETR_ADC2_AWD3      (TIM8_OR1_ETR_ADC2_RMP_1 | TIM8_OR1_ETR_ADC2_RMP_0)   /* !< TIM8_ETR is connected to ADC2 AWD3 */
#define TIM_TIM8_ETR_ADC3_NONE      0x00000000U                                           /* !< TIM8_ETR is not connected to any AWD (analog watchdog)*/
#define TIM_TIM8_ETR_ADC3_AWD1      TIM8_OR1_ETR_ADC3_RMP_0                               /* !< TIM8_ETR is connected to ADC3 AWD1 */
#define TIM_TIM8_ETR_ADC3_AWD2      TIM8_OR1_ETR_ADC3_RMP_1                               /* !< TIM8_ETR is connected to ADC3 AWD2 */
#define TIM_TIM8_ETR_ADC3_AWD3      (TIM8_OR1_ETR_ADC3_RMP_1 | TIM8_OR1_ETR_ADC3_RMP_0)   /* !< TIM8_ETR is connected to ADC3 AWD3 */
#endif /* ADC2 && ADC3 */

#define TIM_TIM8_TI1_GPIO           0x00000000U                                           /* !< TIM8 TI1 is connected to GPIO */
#define TIM_TIM8_TI1_COMP2          TIM8_OR1_TI1_RMP                                      /* !< TIM8 TI1 is connected to COMP1 */
#define TIM_TIM8_ETR_GPIO           0x00000000U                                           /* !< TIM8_ETR is connected to GPIO */
#define TIM_TIM8_ETR_COMP1          TIM8_OR2_ETRSEL_0                                     /* !< TIM8_ETR is connected to COMP1 output */
#define TIM_TIM8_ETR_COMP2          TIM8_OR2_ETRSEL_1                                     /* !< TIM8_ETR is connected to COMP2 output */
#endif /* TIM8 */

#define TIM_TIM15_TI1_GPIO          0x00000000U                                           /* !< TIM15 TI1 is connected to GPIO */
#define TIM_TIM15_TI1_LSE           TIM15_OR1_TI1_RMP                                     /* !< TIM15 TI1 is connected to LSE */
#define TIM_TIM15_ENCODERMODE_NONE  0x00000000U                                           /* !< No redirection */
#define TIM_TIM15_ENCODERMODE_TIM2  TIM15_OR1_ENCODER_MODE_0                              /* !< TIM2 IC1 and TIM2 IC2 are connected to TIM15 IC1 and TIM15 IC2 respectively */
#if defined (TIM3)
#define TIM_TIM15_ENCODERMODE_TIM3  TIM15_OR1_ENCODER_MODE_1                              /* !< TIM3 IC1 and TIM3 IC2 are connected to TIM15 IC1 and TIM15 IC2 respectively */
#endif /* TIM3 */
#if defined (TIM4)
#define TIM_TIM15_ENCODERMODE_TIM4  (TIM15_OR1_ENCODER_MODE_1 | TIM15_OR1_ENCODER_MODE_0) /* !< TIM4 IC1 and TIM4 IC2 are connected to TIM15 IC1 and TIM15 IC2 respectively */
#endif /* TIM4 */

#define TIM_TIM16_TI1_GPIO          0x00000000U                                           /* !< TIM16 TI1 is connected to GPIO */
#define TIM_TIM16_TI1_LSI           TIM16_OR1_TI1_RMP_0                                   /* !< TIM16 TI1 is connected to LSI */
#define TIM_TIM16_TI1_LSE           TIM16_OR1_TI1_RMP_1                                   /* !< TIM16 TI1 is connected to LSE */
#define TIM_TIM16_TI1_RTC           (TIM16_OR1_TI1_RMP_1 | TIM16_OR1_TI1_RMP_0)           /* !< TIM16 TI1 is connected to RTC wakeup interrupt */
#if defined (TIM16_OR1_TI1_RMP_2)
#define TIM_TIM16_TI1_MSI           TIM16_OR1_TI1_RMP_2                                   /* !< TIM16 TI1 is connected to MSI */
#define TIM_TIM16_TI1_HSE_32        (TIM16_OR1_TI1_RMP_2 | TIM16_OR1_TI1_RMP_0)           /* !< TIM16 TI1 is connected to HSE div 32 */
#define TIM_TIM16_TI1_MCO           (TIM16_OR1_TI1_RMP_2 | TIM16_OR1_TI1_RMP_1)           /* !< TIM16 TI1 is connected to MCO */
#endif /* TIM16_OR1_TI1_RMP_2 */

#if defined (TIM17)
#define TIM_TIM17_TI1_GPIO          0x00000000U                                           /* !< TIM17 TI1 is connected to GPIO */
#define TIM_TIM17_TI1_MSI           TIM17_OR1_TI1_RMP_0                                   /* !< TIM17 TI1 is connected to MSI */
#define TIM_TIM17_TI1_HSE_32        TIM17_OR1_TI1_RMP_1                                   /* !< TIM17 TI1 is connected to HSE div 32 */
#define TIM_TIM17_TI1_MCO           (TIM17_OR1_TI1_RMP_1 | TIM17_OR1_TI1_RMP_0)           /* !< TIM17 TI1 is connected to MCO */
#endif /* TIM17 */
/**
  * @}
  */

/** @defgroup TIMEx_Break_Input TIM Extended Break input
  * @{
  */
#define TIM_BREAKINPUT_BRK     0x00000001U                                      /* !< Timer break input  */
#define TIM_BREAKINPUT_BRK2    0x00000002U                                      /* !< Timer break2 input */
/**
  * @}
  */

/** @defgroup TIMEx_Break_Input_Source TIM Extended Break input source
  * @{
  */
#define TIM_BREAKINPUTSOURCE_BKIN     0x00000001U                               /* !< An external source (GPIO) is connected to the BKIN pin  */
#define TIM_BREAKINPUTSOURCE_COMP1    0x00000002U                               /* !< The COMP1 output is connected to the break input */
#define TIM_BREAKINPUTSOURCE_COMP2    0x00000004U                               /* !< The COMP2 output is connected to the break input */
#if defined (DFSDM1_Channel0)
#define TIM_BREAKINPUTSOURCE_DFSDM1   0x00000008U                               /* !< The analog watchdog output of the DFSDM1 peripheral is connected to the break input */
#endif /* DFSDM1_Channel0 */
/**
  * @}
  */

/** @defgroup TIMEx_Break_Input_Source_Enable TIM Extended Break input source enabling
  * @{
  */
#define TIM_BREAKINPUTSOURCE_DISABLE     0x00000000U                            /* !< Break input source is disabled */
#define TIM_BREAKINPUTSOURCE_ENABLE      0x00000001U                            /* !< Break input source is enabled */
/**
  * @}
  */

/** @defgroup TIMEx_Break_Input_Source_Polarity TIM Extended Break input polarity
  * @{
  */
#define TIM_BREAKINPUTSOURCE_POLARITY_LOW     0x00000001U                       /* !< Break input source is active low */
#define TIM_BREAKINPUTSOURCE_POLARITY_HIGH    0x00000000U                       /* !< Break input source is active_high */
/**
  * @}
  */

/**
  * @}
  */
/* End of exported constants -------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/** @defgroup TIMEx_Exported_Macros TIM Extended Exported Macros
  * @{
  */

/**
  * @}
  */
/* End of exported macro -----------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/** @defgroup TIMEx_Private_Macros TIM Extended Private Macros
  * @{
  */
#define IS_TIM_REMAP(__REMAP__)    (((__REMAP__) <= (uint32_t)0x0001C01F))

#define IS_TIM_BREAKINPUT(__BREAKINPUT__)  (((__BREAKINPUT__) == TIM_BREAKINPUT_BRK)  || \
                                            ((__BREAKINPUT__) == TIM_BREAKINPUT_BRK2))

#if defined (DFSDM1_Channel0)
#define IS_TIM_BREAKINPUTSOURCE(__SOURCE__)  (((__SOURCE__) == TIM_BREAKINPUTSOURCE_BKIN)  || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP1) || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP2) || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_DFSDM1))
#else
#define IS_TIM_BREAKINPUTSOURCE(__SOURCE__)  (((__SOURCE__) == TIM_BREAKINPUTSOURCE_BKIN)  || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP1) || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP2))
#endif /* DFSDM1_Channel0 */

#define IS_TIM_BREAKINPUTSOURCE_STATE(__STATE__)  (((__STATE__) == TIM_BREAKINPUTSOURCE_DISABLE)  || \
                                                   ((__STATE__) == TIM_BREAKINPUTSOURCE_ENABLE))

#define IS_TIM_BREAKINPUTSOURCE_POLARITY(__POLARITY__)  (((__POLARITY__) == TIM_BREAKINPUTSOURCE_POLARITY_LOW)  || \
                                                         ((__POLARITY__) == TIM_BREAKINPUTSOURCE_POLARITY_HIGH))

/**
  * @}
  */
/* End of private macro ------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup TIMEx_Exported_Functions TIM Extended Exported Functions
  * @{
  */

/** @addtogroup TIMEx_Exported_Functions_Group1 Extended Timer Hall Sensor functions
  *  @brief    Timer Hall Sensor functions
  * @{
  */
/*  Timer Hall Sensor functions  **********************************************/
HAL_StatusTypeDef HAL_TIMEx_HallSensor_Init(TIM_HandleTypeDef *htim, TIM_HallSensor_InitTypeDef *sConfig);
HAL_StatusTypeDef HAL_TIMEx_HallSensor_DeInit(TIM_HandleTypeDef *htim);

void HAL_TIMEx_HallSensor_MspInit(TIM_HandleTypeDef *htim);
void HAL_TIMEx_HallSensor_MspDeInit(TIM_HandleTypeDef *htim);

/* Blocking mode: Polling */
HAL_StatusTypeDef HAL_TIMEx_HallSensor_Start(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIMEx_HallSensor_Stop(TIM_HandleTypeDef *htim);
/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_TIMEx_HallSensor_Start_IT(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIMEx_HallSensor_Stop_IT(TIM_HandleTypeDef *htim);
/* Non-Blocking mode: DMA */
HAL_StatusTypeDef HAL_TIMEx_HallSensor_Start_DMA(TIM_HandleTypeDef *htim, uint32_t *pData, uint16_t Length);
HAL_StatusTypeDef HAL_TIMEx_HallSensor_Stop_DMA(TIM_HandleTypeDef *htim);
/**
  * @}
  */

/** @addtogroup TIMEx_Exported_Functions_Group2 Extended Timer Complementary Output Compare functions
  *  @brief   Timer Complementary Output Compare functions
  * @{
  */
/*  Timer Complementary Output Compare functions  *****************************/
/* Blocking mode: Polling */
HAL_StatusTypeDef HAL_TIMEx_OCN_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIMEx_OCN_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);

/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_TIMEx_OCN_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIMEx_OCN_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel);

/* Non-Blocking mode: DMA */
HAL_StatusTypeDef HAL_TIMEx_OCN_Start_DMA(TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t *pData, uint16_t Length);
HAL_StatusTypeDef HAL_TIMEx_OCN_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);
/**
  * @}
  */

/** @addtogroup TIMEx_Exported_Functions_Group3 Extended Timer Complementary PWM functions
  *  @brief    Timer Complementary PWM functions
  * @{
  */
/*  Timer Complementary PWM functions  ****************************************/
/* Blocking mode: Polling */
HAL_StatusTypeDef HAL_TIMEx_PWMN_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIMEx_PWMN_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);

/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_TIMEx_PWMN_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIMEx_PWMN_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
/* Non-Blocking mode: DMA */
HAL_StatusTypeDef HAL_TIMEx_PWMN_Start_DMA(TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t *pData, uint16_t Length);
HAL_StatusTypeDef HAL_TIMEx_PWMN_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);
/**
  * @}
  */

/** @addtogroup TIMEx_Exported_Functions_Group4 Extended Timer Complementary One Pulse functions
  *  @brief    Timer Complementary One Pulse functions
  * @{
  */
/*  Timer Complementary One Pulse functions  **********************************/
/* Blocking mode: Polling */
HAL_StatusTypeDef HAL_TIMEx_OnePulseN_Start(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
HAL_StatusTypeDef HAL_TIMEx_OnePulseN_Stop(TIM_HandleTypeDef *htim, uint32_t OutputChannel);

/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_TIMEx_OnePulseN_Start_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
HAL_StatusTypeDef HAL_TIMEx_OnePulseN_Stop_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
/**
  * @}
  */

/** @addtogroup TIMEx_Exported_Functions_Group5 Extended Peripheral Control functions
  *  @brief    Peripheral Control functions
  * @{
  */
/* Extended Control functions  ************************************************/
HAL_StatusTypeDef HAL_TIMEx_ConfigCommutEvent(TIM_HandleTypeDef *htim, uint32_t  InputTrigger,
                                              uint32_t  CommutationSource);
HAL_StatusTypeDef HAL_TIMEx_ConfigCommutEvent_IT(TIM_HandleTypeDef *htim, uint32_t  InputTrigger,
                                                 uint32_t  CommutationSource);
HAL_StatusTypeDef HAL_TIMEx_ConfigCommutEvent_DMA(TIM_HandleTypeDef *htim, uint32_t  InputTrigger,
                                                  uint32_t  CommutationSource);
HAL_StatusTypeDef HAL_TIMEx_MasterConfigSynchronization(TIM_HandleTypeDef *htim,
                                                        TIM_MasterConfigTypeDef *sMasterConfig);
HAL_StatusTypeDef HAL_TIMEx_ConfigBreakDeadTime(TIM_HandleTypeDef *htim,
                                                TIM_BreakDeadTimeConfigTypeDef *sBreakDeadTimeConfig);
HAL_StatusTypeDef HAL_TIMEx_ConfigBreakInput(TIM_HandleTypeDef *htim, uint32_t BreakInput,
                                             TIMEx_BreakInputConfigTypeDef *sBreakInputConfig);
HAL_StatusTypeDef HAL_TIMEx_GroupChannel5(TIM_HandleTypeDef *htim, uint32_t Channels);
HAL_StatusTypeDef HAL_TIMEx_RemapConfig(TIM_HandleTypeDef *htim, uint32_t Remap);
/**
  * @}
  */

/** @addtogroup TIMEx_Exported_Functions_Group6 Extended Callbacks functions
  * @brief    Extended Callbacks functions
  * @{
  */
/* Extended Callback **********************************************************/
void HAL_TIMEx_CommutCallback(TIM_HandleTypeDef *htim);
void HAL_TIMEx_CommutHalfCpltCallback(TIM_HandleTypeDef *htim);
void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim);
void HAL_TIMEx_Break2Callback(TIM_HandleTypeDef *htim);
/**
  * @}
  */

/** @addtogroup TIMEx_Exported_Functions_Group7 Extended Peripheral State functions
  * @brief    Extended Peripheral State functions
  * @{
  */
/* Extended Peripheral State functions  ***************************************/
HAL_TIM_StateTypeDef HAL_TIMEx_HallSensor_GetState(TIM_HandleTypeDef *htim);
/**
  * @}
  */

/**
  * @}
  */
/* End of exported functions -------------------------------------------------*/

/* Private functions----------------------------------------------------------*/
/** @addtogroup TIMEx_Private_Functions TIMEx Private Functions
  * @{
  */
void TIMEx_DMACommutationCplt(DMA_HandleTypeDef *hdma);
void TIMEx_DMACommutationHalfCplt(DMA_HandleTypeDef *hdma);
/**
  * @}
  */
/* End of private functions --------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif


#endif /* STM32L4xx_HAL_TIM_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
