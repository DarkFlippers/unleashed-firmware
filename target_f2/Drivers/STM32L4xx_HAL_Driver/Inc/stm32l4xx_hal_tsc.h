/**
  ******************************************************************************
  * @file    stm32l4xx_hal_tsc.h
  * @author  MCD Application Team
  * @brief   Header file of TSC HAL module.
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
#ifndef STM32L4xx_HAL_TSC_H
#define STM32L4xx_HAL_TSC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup TSC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup TSC_Exported_Types TSC Exported Types
  * @{
  */

/**
  * @brief TSC state structure definition
  */
typedef enum
{
  HAL_TSC_STATE_RESET  = 0x00UL, /*!< TSC registers have their reset value */
  HAL_TSC_STATE_READY  = 0x01UL, /*!< TSC registers are initialized or acquisition is completed with success */
  HAL_TSC_STATE_BUSY   = 0x02UL, /*!< TSC initialization or acquisition is on-going */
  HAL_TSC_STATE_ERROR  = 0x03UL  /*!< Acquisition is completed with max count error */
} HAL_TSC_StateTypeDef;

/**
  * @brief TSC group status structure definition
  */
typedef enum
{
  TSC_GROUP_ONGOING   = 0x00UL, /*!< Acquisition on group is on-going or not started */
  TSC_GROUP_COMPLETED = 0x01UL /*!< Acquisition on group is completed with success (no max count error) */
} TSC_GroupStatusTypeDef;

/**
  * @brief TSC init structure definition
  */
typedef struct
{
  uint32_t CTPulseHighLength;       /*!< Charge-transfer high pulse length
                                         This parameter can be a value of @ref TSC_CTPulseHL_Config  */
  uint32_t CTPulseLowLength;        /*!< Charge-transfer low pulse length
                                         This parameter can be a value of @ref TSC_CTPulseLL_Config  */
  FunctionalState SpreadSpectrum;   /*!< Spread spectrum activation
                                         This parameter can be set to ENABLE or DISABLE. */
  uint32_t SpreadSpectrumDeviation; /*!< Spread spectrum deviation
                                         This parameter must be a number between Min_Data = 0 and Max_Data = 127 */
  uint32_t SpreadSpectrumPrescaler; /*!< Spread spectrum prescaler
                                         This parameter can be a value of @ref TSC_SpreadSpec_Prescaler */
  uint32_t PulseGeneratorPrescaler; /*!< Pulse generator prescaler
                                         This parameter can be a value of @ref TSC_PulseGenerator_Prescaler */
  uint32_t MaxCountValue;           /*!< Max count value
                                         This parameter can be a value of @ref TSC_MaxCount_Value  */
  uint32_t IODefaultMode;           /*!< IO default mode
                                         This parameter can be a value of @ref TSC_IO_Default_Mode  */
  uint32_t SynchroPinPolarity;      /*!< Synchro pin polarity
                                         This parameter can be a value of @ref TSC_Synchro_Pin_Polarity */
  uint32_t AcquisitionMode;         /*!< Acquisition mode
                                         This parameter can be a value of @ref TSC_Acquisition_Mode  */
  FunctionalState MaxCountInterrupt;/*!< Max count interrupt activation
                                         This parameter can be set to ENABLE or DISABLE. */
  uint32_t ChannelIOs;              /*!< Channel IOs mask */
  uint32_t ShieldIOs;               /*!< Shield IOs mask */
  uint32_t SamplingIOs;             /*!< Sampling IOs mask */
} TSC_InitTypeDef;

/**
  * @brief TSC IOs configuration structure definition
  */
typedef struct
{
  uint32_t ChannelIOs;  /*!< Channel IOs mask */
  uint32_t ShieldIOs;   /*!< Shield IOs mask */
  uint32_t SamplingIOs; /*!< Sampling IOs mask */
} TSC_IOConfigTypeDef;

/**
  * @brief  TSC handle Structure definition
  */
typedef struct __TSC_HandleTypeDef
{
  TSC_TypeDef               *Instance;  /*!< Register base address      */
  TSC_InitTypeDef           Init;       /*!< Initialization parameters  */
  __IO HAL_TSC_StateTypeDef State;      /*!< Peripheral state           */
  HAL_LockTypeDef           Lock;       /*!< Lock feature               */
  __IO uint32_t             ErrorCode;  /*!< I2C Error code             */

#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
  void (* ConvCpltCallback)(struct __TSC_HandleTypeDef *htsc);   /*!< TSC Conversion complete callback  */
  void (* ErrorCallback)(struct __TSC_HandleTypeDef *htsc);      /*!< TSC Error callback                */

  void (* MspInitCallback)(struct __TSC_HandleTypeDef *htsc);    /*!< TSC Msp Init callback             */
  void (* MspDeInitCallback)(struct __TSC_HandleTypeDef *htsc);  /*!< TSC Msp DeInit callback           */

#endif  /* USE_HAL_TSC_REGISTER_CALLBACKS */
} TSC_HandleTypeDef;

enum
{
  TSC_GROUP1_IDX = 0x00UL,
  TSC_GROUP2_IDX,
  TSC_GROUP3_IDX,
  TSC_GROUP4_IDX,
#if defined(TSC_IOCCR_G5_IO1)
  TSC_GROUP5_IDX,
#endif
#if defined(TSC_IOCCR_G6_IO1)
  TSC_GROUP6_IDX,
#endif
#if defined(TSC_IOCCR_G7_IO1)
  TSC_GROUP7_IDX,
#endif
#if defined(TSC_IOCCR_G8_IO1)
  TSC_GROUP8_IDX,
#endif
  TSC_NB_OF_GROUPS
};

#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL TSC Callback ID enumeration definition
  */
typedef enum
{
  HAL_TSC_CONV_COMPLETE_CB_ID           = 0x00UL,  /*!< TSC Conversion completed callback ID  */
  HAL_TSC_ERROR_CB_ID                   = 0x01UL,  /*!< TSC Error callback ID                 */

  HAL_TSC_MSPINIT_CB_ID                 = 0x02UL,  /*!< TSC Msp Init callback ID              */
  HAL_TSC_MSPDEINIT_CB_ID               = 0x03UL   /*!< TSC Msp DeInit callback ID            */

} HAL_TSC_CallbackIDTypeDef;

/**
  * @brief  HAL TSC Callback pointer definition
  */
typedef  void (*pTSC_CallbackTypeDef)(TSC_HandleTypeDef *htsc); /*!< pointer to an TSC callback function */

#endif  /* USE_HAL_TSC_REGISTER_CALLBACKS */

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup TSC_Exported_Constants TSC Exported Constants
  * @{
  */

/** @defgroup TSC_Error_Code_definition TSC Error Code definition
  * @brief  TSC Error Code definition
  * @{
  */
#define HAL_TSC_ERROR_NONE      0x00000000UL    /*!< No error              */
#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
#define HAL_TSC_ERROR_INVALID_CALLBACK  0x00000001UL    /*!< Invalid Callback error */
#endif /* USE_HAL_TSC_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @defgroup TSC_CTPulseHL_Config CTPulse High Length
  * @{
  */
#define TSC_CTPH_1CYCLE         0x00000000UL                                                    /*!< Charge transfer pulse high during 1 cycle (PGCLK)   */
#define TSC_CTPH_2CYCLES        TSC_CR_CTPH_0                                                   /*!< Charge transfer pulse high during 2 cycles (PGCLK)  */
#define TSC_CTPH_3CYCLES        TSC_CR_CTPH_1                                                   /*!< Charge transfer pulse high during 3 cycles (PGCLK)  */
#define TSC_CTPH_4CYCLES        (TSC_CR_CTPH_1 | TSC_CR_CTPH_0)                                 /*!< Charge transfer pulse high during 4 cycles (PGCLK)  */
#define TSC_CTPH_5CYCLES        TSC_CR_CTPH_2                                                   /*!< Charge transfer pulse high during 5 cycles (PGCLK)  */
#define TSC_CTPH_6CYCLES        (TSC_CR_CTPH_2 | TSC_CR_CTPH_0)                                 /*!< Charge transfer pulse high during 6 cycles (PGCLK)  */
#define TSC_CTPH_7CYCLES        (TSC_CR_CTPH_2 | TSC_CR_CTPH_1)                                 /*!< Charge transfer pulse high during 7 cycles (PGCLK)  */
#define TSC_CTPH_8CYCLES        (TSC_CR_CTPH_2 | TSC_CR_CTPH_1 | TSC_CR_CTPH_0)                 /*!< Charge transfer pulse high during 8 cycles (PGCLK)  */
#define TSC_CTPH_9CYCLES        TSC_CR_CTPH_3                                                   /*!< Charge transfer pulse high during 9 cycles (PGCLK)  */
#define TSC_CTPH_10CYCLES       (TSC_CR_CTPH_3 | TSC_CR_CTPH_0)                                 /*!< Charge transfer pulse high during 10 cycles (PGCLK) */
#define TSC_CTPH_11CYCLES       (TSC_CR_CTPH_3 | TSC_CR_CTPH_1)                                 /*!< Charge transfer pulse high during 11 cycles (PGCLK) */
#define TSC_CTPH_12CYCLES       (TSC_CR_CTPH_3 | TSC_CR_CTPH_1 | TSC_CR_CTPH_0)                 /*!< Charge transfer pulse high during 12 cycles (PGCLK) */
#define TSC_CTPH_13CYCLES       (TSC_CR_CTPH_3 | TSC_CR_CTPH_2)                                 /*!< Charge transfer pulse high during 13 cycles (PGCLK) */
#define TSC_CTPH_14CYCLES       (TSC_CR_CTPH_3 | TSC_CR_CTPH_2 | TSC_CR_CTPH_0)                 /*!< Charge transfer pulse high during 14 cycles (PGCLK) */
#define TSC_CTPH_15CYCLES       (TSC_CR_CTPH_3 | TSC_CR_CTPH_2 | TSC_CR_CTPH_1)                 /*!< Charge transfer pulse high during 15 cycles (PGCLK) */
#define TSC_CTPH_16CYCLES       (TSC_CR_CTPH_3 | TSC_CR_CTPH_2 | TSC_CR_CTPH_1 | TSC_CR_CTPH_0) /*!< Charge transfer pulse high during 16 cycles (PGCLK) */
/**
  * @}
  */

/** @defgroup TSC_CTPulseLL_Config CTPulse Low Length
  * @{
  */
#define TSC_CTPL_1CYCLE         0x00000000UL                                                     /*!< Charge transfer pulse low during 1 cycle (PGCLK)   */
#define TSC_CTPL_2CYCLES        TSC_CR_CTPL_0                                                    /*!< Charge transfer pulse low during 2 cycles (PGCLK)  */
#define TSC_CTPL_3CYCLES        TSC_CR_CTPL_1                                                    /*!< Charge transfer pulse low during 3 cycles (PGCLK)  */
#define TSC_CTPL_4CYCLES        (TSC_CR_CTPL_1 | TSC_CR_CTPL_0)                                  /*!< Charge transfer pulse low during 4 cycles (PGCLK)  */
#define TSC_CTPL_5CYCLES        TSC_CR_CTPL_2                                                    /*!< Charge transfer pulse low during 5 cycles (PGCLK)  */
#define TSC_CTPL_6CYCLES        (TSC_CR_CTPL_2 | TSC_CR_CTPL_0)                                  /*!< Charge transfer pulse low during 6 cycles (PGCLK)  */
#define TSC_CTPL_7CYCLES        (TSC_CR_CTPL_2 | TSC_CR_CTPL_1)                                  /*!< Charge transfer pulse low during 7 cycles (PGCLK)  */
#define TSC_CTPL_8CYCLES        (TSC_CR_CTPL_2 | TSC_CR_CTPL_1 | TSC_CR_CTPL_0)                  /*!< Charge transfer pulse low during 8 cycles (PGCLK)  */
#define TSC_CTPL_9CYCLES        TSC_CR_CTPL_3                                                    /*!< Charge transfer pulse low during 9 cycles (PGCLK)  */
#define TSC_CTPL_10CYCLES       (TSC_CR_CTPL_3 | TSC_CR_CTPL_0)                                  /*!< Charge transfer pulse low during 10 cycles (PGCLK) */
#define TSC_CTPL_11CYCLES       (TSC_CR_CTPL_3 | TSC_CR_CTPL_1)                                  /*!< Charge transfer pulse low during 11 cycles (PGCLK) */
#define TSC_CTPL_12CYCLES       (TSC_CR_CTPL_3 | TSC_CR_CTPL_1 | TSC_CR_CTPL_0)                  /*!< Charge transfer pulse low during 12 cycles (PGCLK) */
#define TSC_CTPL_13CYCLES       (TSC_CR_CTPL_3 | TSC_CR_CTPL_2)                                  /*!< Charge transfer pulse low during 13 cycles (PGCLK) */
#define TSC_CTPL_14CYCLES       (TSC_CR_CTPL_3 | TSC_CR_CTPL_2 | TSC_CR_CTPL_0)                  /*!< Charge transfer pulse low during 14 cycles (PGCLK) */
#define TSC_CTPL_15CYCLES       (TSC_CR_CTPL_3 | TSC_CR_CTPL_2 | TSC_CR_CTPL_1)                  /*!< Charge transfer pulse low during 15 cycles (PGCLK) */
#define TSC_CTPL_16CYCLES       (TSC_CR_CTPL_3 | TSC_CR_CTPL_2 | TSC_CR_CTPL_1 | TSC_CR_CTPL_0)  /*!< Charge transfer pulse low during 16 cycles (PGCLK) */
/**
  * @}
  */

/** @defgroup TSC_SpreadSpec_Prescaler Spread Spectrum Prescaler
  * @{
  */
#define TSC_SS_PRESC_DIV1       0x00000000UL  /*!< Spread Spectrum Prescaler Div1 */
#define TSC_SS_PRESC_DIV2       TSC_CR_SSPSC  /*!< Spread Spectrum Prescaler Div2 */
/**
  * @}
  */

/** @defgroup TSC_PulseGenerator_Prescaler Pulse Generator Prescaler
  * @{
  */
#define TSC_PG_PRESC_DIV1       0x00000000UL                                        /*!< Pulse Generator HCLK Div1   */
#define TSC_PG_PRESC_DIV2       TSC_CR_PGPSC_0                                      /*!< Pulse Generator HCLK Div2   */
#define TSC_PG_PRESC_DIV4       TSC_CR_PGPSC_1                                      /*!< Pulse Generator HCLK Div4   */
#define TSC_PG_PRESC_DIV8       (TSC_CR_PGPSC_1 | TSC_CR_PGPSC_0)                   /*!< Pulse Generator HCLK Div8   */
#define TSC_PG_PRESC_DIV16      TSC_CR_PGPSC_2                                      /*!< Pulse Generator HCLK Div16  */
#define TSC_PG_PRESC_DIV32      (TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0)                   /*!< Pulse Generator HCLK Div32  */
#define TSC_PG_PRESC_DIV64      (TSC_CR_PGPSC_2 | TSC_CR_PGPSC_1)                   /*!< Pulse Generator HCLK Div64  */
#define TSC_PG_PRESC_DIV128     (TSC_CR_PGPSC_2 | TSC_CR_PGPSC_1 | TSC_CR_PGPSC_0)  /*!< Pulse Generator HCLK Div128 */
/**
  * @}
  */

/** @defgroup TSC_MaxCount_Value Max Count Value
  * @{
  */
#define TSC_MCV_255             0x00000000UL                   /*!< 255 maximum number of charge transfer pulses   */
#define TSC_MCV_511             TSC_CR_MCV_0                   /*!< 511 maximum number of charge transfer pulses   */
#define TSC_MCV_1023            TSC_CR_MCV_1                   /*!< 1023 maximum number of charge transfer pulses  */
#define TSC_MCV_2047            (TSC_CR_MCV_1 | TSC_CR_MCV_0)  /*!< 2047 maximum number of charge transfer pulses  */
#define TSC_MCV_4095            TSC_CR_MCV_2                   /*!< 4095 maximum number of charge transfer pulses  */
#define TSC_MCV_8191            (TSC_CR_MCV_2 | TSC_CR_MCV_0)  /*!< 8191 maximum number of charge transfer pulses  */
#define TSC_MCV_16383           (TSC_CR_MCV_2 | TSC_CR_MCV_1)  /*!< 16383 maximum number of charge transfer pulses */
/**
  * @}
  */

/** @defgroup TSC_IO_Default_Mode IO Default Mode
  * @{
  */
#define TSC_IODEF_OUT_PP_LOW    0x00000000UL /*!< I/Os are forced to output push-pull low */
#define TSC_IODEF_IN_FLOAT      TSC_CR_IODEF /*!< I/Os are in input floating              */
/**
  * @}
  */

/** @defgroup TSC_Synchro_Pin_Polarity Synchro Pin Polarity
  * @{
  */
#define TSC_SYNC_POLARITY_FALLING  0x00000000UL   /*!< Falling edge only           */
#define TSC_SYNC_POLARITY_RISING   TSC_CR_SYNCPOL /*!< Rising edge and high level  */
/**
  * @}
  */

/** @defgroup TSC_Acquisition_Mode Acquisition Mode
  * @{
  */
#define TSC_ACQ_MODE_NORMAL     0x00000000UL  /*!< Normal acquisition mode (acquisition starts as soon as START bit is set)                                                              */
#define TSC_ACQ_MODE_SYNCHRO    TSC_CR_AM     /*!< Synchronized acquisition mode (acquisition starts if START bit is set and when the selected signal is detected on the SYNC input pin) */
/**
  * @}
  */

/** @defgroup TSC_interrupts_definition Interrupts definition
  * @{
  */
#define TSC_IT_EOA              TSC_IER_EOAIE /*!< End of acquisition interrupt enable */
#define TSC_IT_MCE              TSC_IER_MCEIE /*!< Max count error interrupt enable    */
/**
  * @}
  */

/** @defgroup TSC_flags_definition Flags definition
  * @{
  */
#define TSC_FLAG_EOA            TSC_ISR_EOAF /*!< End of acquisition flag */
#define TSC_FLAG_MCE            TSC_ISR_MCEF /*!< Max count error flag    */
/**
  * @}
  */

/** @defgroup TSC_Group_definition Group definition
  * @{
  */
#define TSC_GROUP1              (0x1UL << TSC_GROUP1_IDX)
#define TSC_GROUP2              (0x1UL << TSC_GROUP2_IDX)
#define TSC_GROUP3              (0x1UL << TSC_GROUP3_IDX)
#define TSC_GROUP4              (0x1UL << TSC_GROUP4_IDX)
#if defined(TSC_IOCCR_G5_IO1)
#define TSC_GROUP5              (0x1UL << TSC_GROUP5_IDX)
#endif
#if defined(TSC_IOCCR_G6_IO1)
#define TSC_GROUP6              (0x1UL << TSC_GROUP6_IDX)
#endif
#if defined(TSC_IOCCR_G7_IO1)
#define TSC_GROUP7              (0x1UL << TSC_GROUP7_IDX)
#endif
#if defined(TSC_IOCCR_G8_IO1)
#define TSC_GROUP8              (0x1UL << TSC_GROUP8_IDX)
#endif

#define TSC_GROUPX_NOT_SUPPORTED        0xFF000000UL    /*!< TSC GroupX not supported       */

#define TSC_GROUP1_IO1          TSC_IOCCR_G1_IO1 /*!< TSC Group1 IO1 */
#define TSC_GROUP1_IO2          TSC_IOCCR_G1_IO2 /*!< TSC Group1 IO2 */
#define TSC_GROUP1_IO3          TSC_IOCCR_G1_IO3 /*!< TSC Group1 IO3 */
#define TSC_GROUP1_IO4          TSC_IOCCR_G1_IO4 /*!< TSC Group1 IO4 */

#define TSC_GROUP2_IO1          TSC_IOCCR_G2_IO1 /*!< TSC Group2 IO1 */
#define TSC_GROUP2_IO2          TSC_IOCCR_G2_IO2 /*!< TSC Group2 IO2 */
#define TSC_GROUP2_IO3          TSC_IOCCR_G2_IO3 /*!< TSC Group2 IO3 */
#define TSC_GROUP2_IO4          TSC_IOCCR_G2_IO4 /*!< TSC Group2 IO4 */

#define TSC_GROUP3_IO1          TSC_IOCCR_G3_IO1 /*!< TSC Group3 IO1 */
#define TSC_GROUP3_IO2          TSC_IOCCR_G3_IO2 /*!< TSC Group3 IO2 */
#define TSC_GROUP3_IO3          TSC_IOCCR_G3_IO3 /*!< TSC Group3 IO3 */
#define TSC_GROUP3_IO4          TSC_IOCCR_G3_IO4 /*!< TSC Group3 IO4 */

#define TSC_GROUP4_IO1          TSC_IOCCR_G4_IO1 /*!< TSC Group4 IO1 */
#define TSC_GROUP4_IO2          TSC_IOCCR_G4_IO2 /*!< TSC Group4 IO2 */
#define TSC_GROUP4_IO3          TSC_IOCCR_G4_IO3 /*!< TSC Group4 IO3 */
#define TSC_GROUP4_IO4          TSC_IOCCR_G4_IO4 /*!< TSC Group4 IO4 */
#if defined(TSC_IOCCR_G5_IO1)

#define TSC_GROUP5_IO1          TSC_IOCCR_G5_IO1 /*!< TSC Group5 IO1 */
#define TSC_GROUP5_IO2          TSC_IOCCR_G5_IO2 /*!< TSC Group5 IO2 */
#define TSC_GROUP5_IO3          TSC_IOCCR_G5_IO3 /*!< TSC Group5 IO3 */
#define TSC_GROUP5_IO4          TSC_IOCCR_G5_IO4 /*!< TSC Group5 IO4 */
#else

#define TSC_GROUP5_IO1          (uint32_t)(0x00000010UL | TSC_GROUPX_NOT_SUPPORTED)     /*!< TSC Group5 IO1 not supported   */
#define TSC_GROUP5_IO2          TSC_GROUP5_IO1                                          /*!< TSC Group5 IO2 not supported   */
#define TSC_GROUP5_IO3          TSC_GROUP5_IO1                                          /*!< TSC Group5 IO3 not supported   */
#define TSC_GROUP5_IO4          TSC_GROUP5_IO1                                          /*!< TSC Group5 IO4 not supported   */
#endif
#if defined(TSC_IOCCR_G6_IO1)

#define TSC_GROUP6_IO1          TSC_IOCCR_G6_IO1 /*!< TSC Group6 IO1 */
#define TSC_GROUP6_IO2          TSC_IOCCR_G6_IO2 /*!< TSC Group6 IO2 */
#define TSC_GROUP6_IO3          TSC_IOCCR_G6_IO3 /*!< TSC Group6 IO3 */
#define TSC_GROUP6_IO4          TSC_IOCCR_G6_IO4 /*!< TSC Group6 IO4 */
#else

#define TSC_GROUP6_IO1          (uint32_t)(0x00000020UL | TSC_GROUPX_NOT_SUPPORTED)     /*!< TSC Group6 IO1 not supported   */
#define TSC_GROUP6_IO2          TSC_GROUP6_IO1                                          /*!< TSC Group6 IO2 not supported   */
#define TSC_GROUP6_IO3          TSC_GROUP6_IO1                                          /*!< TSC Group6 IO3 not supported   */
#define TSC_GROUP6_IO4          TSC_GROUP6_IO1                                          /*!< TSC Group6 IO4 not supported   */
#endif
#if defined(TSC_IOCCR_G7_IO1)

#define TSC_GROUP7_IO1          TSC_IOCCR_G7_IO1 /*!< TSC Group7 IO1 */
#define TSC_GROUP7_IO2          TSC_IOCCR_G7_IO2 /*!< TSC Group7 IO2 */
#define TSC_GROUP7_IO3          TSC_IOCCR_G7_IO3 /*!< TSC Group7 IO3 */
#define TSC_GROUP7_IO4          TSC_IOCCR_G7_IO4 /*!< TSC Group7 IO4 */
#else

#define TSC_GROUP7_IO1          (uint32_t)(0x00000040UL | TSC_GROUPX_NOT_SUPPORTED)     /*!< TSC Group7 IO1 not supported   */
#define TSC_GROUP7_IO2          TSC_GROUP7_IO1                                          /*!< TSC Group7 IO2 not supported   */
#define TSC_GROUP7_IO3          TSC_GROUP7_IO1                                          /*!< TSC Group7 IO3 not supported   */
#define TSC_GROUP7_IO4          TSC_GROUP7_IO1                                          /*!< TSC Group7 IO4 not supported   */
#endif
#if defined(TSC_IOCCR_G8_IO1)

#define TSC_GROUP8_IO1          TSC_IOCCR_G8_IO1 /*!< TSC Group8 IO1 */
#define TSC_GROUP8_IO2          TSC_IOCCR_G8_IO2 /*!< TSC Group8 IO2 */
#define TSC_GROUP8_IO3          TSC_IOCCR_G8_IO3 /*!< TSC Group8 IO3 */
#define TSC_GROUP8_IO4          TSC_IOCCR_G8_IO4 /*!< TSC Group8 IO4 */
#else

#define TSC_GROUP8_IO1          (uint32_t)(0x00000080UL | TSC_GROUPX_NOT_SUPPORTED)     /*!< TSC Group8 IO1 not supported   */
#define TSC_GROUP8_IO2          TSC_GROUP8_IO1                                          /*!< TSC Group8 IO2 not supported   */
#define TSC_GROUP8_IO3          TSC_GROUP8_IO1                                          /*!< TSC Group8 IO3 not supported   */
#define TSC_GROUP8_IO4          TSC_GROUP8_IO1                                          /*!< TSC Group8 IO4 not supported   */
#endif
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/

/** @defgroup TSC_Exported_Macros TSC Exported Macros
  * @{
  */

/** @brief Reset TSC handle state.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
#define __HAL_TSC_RESET_HANDLE_STATE(__HANDLE__)                   do{                                                   \
                                                                       (__HANDLE__)->State = HAL_TSC_STATE_RESET;       \
                                                                       (__HANDLE__)->MspInitCallback = NULL;            \
                                                                       (__HANDLE__)->MspDeInitCallback = NULL;          \
                                                                     } while(0)
#else
#define __HAL_TSC_RESET_HANDLE_STATE(__HANDLE__)                   ((__HANDLE__)->State = HAL_TSC_STATE_RESET)
#endif

/**
  * @brief Enable the TSC peripheral.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_ENABLE(__HANDLE__)                               ((__HANDLE__)->Instance->CR |= TSC_CR_TSCE)

/**
  * @brief Disable the TSC peripheral.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_DISABLE(__HANDLE__)                              ((__HANDLE__)->Instance->CR &= (~TSC_CR_TSCE))

/**
  * @brief Start acquisition.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_START_ACQ(__HANDLE__)                            ((__HANDLE__)->Instance->CR |= TSC_CR_START)

/**
  * @brief Stop acquisition.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_STOP_ACQ(__HANDLE__)                             ((__HANDLE__)->Instance->CR &= (~TSC_CR_START))

/**
  * @brief Set IO default mode to output push-pull low.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_SET_IODEF_OUTPPLOW(__HANDLE__)                   ((__HANDLE__)->Instance->CR &= (~TSC_CR_IODEF))

/**
  * @brief Set IO default mode to input floating.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_SET_IODEF_INFLOAT(__HANDLE__)                    ((__HANDLE__)->Instance->CR |= TSC_CR_IODEF)

/**
  * @brief Set synchronization polarity to falling edge.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_SET_SYNC_POL_FALL(__HANDLE__)                    ((__HANDLE__)->Instance->CR &= (~TSC_CR_SYNCPOL))

/**
  * @brief Set synchronization polarity to rising edge and high level.
  * @param  __HANDLE__ TSC handle
  * @retval None
  */
#define __HAL_TSC_SET_SYNC_POL_RISE_HIGH(__HANDLE__)               ((__HANDLE__)->Instance->CR |= TSC_CR_SYNCPOL)

/**
  * @brief Enable TSC interrupt.
  * @param  __HANDLE__ TSC handle
  * @param  __INTERRUPT__ TSC interrupt
  * @retval None
  */
#define __HAL_TSC_ENABLE_IT(__HANDLE__, __INTERRUPT__)             ((__HANDLE__)->Instance->IER |= (__INTERRUPT__))

/**
  * @brief Disable TSC interrupt.
  * @param  __HANDLE__ TSC handle
  * @param  __INTERRUPT__ TSC interrupt
  * @retval None
  */
#define __HAL_TSC_DISABLE_IT(__HANDLE__, __INTERRUPT__)            ((__HANDLE__)->Instance->IER &= (~(__INTERRUPT__)))

/** @brief Check whether the specified TSC interrupt source is enabled or not.
  * @param  __HANDLE__ TSC Handle
  * @param  __INTERRUPT__ TSC interrupt
  * @retval SET or RESET
  */
#define __HAL_TSC_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)         ((((__HANDLE__)->Instance->IER & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)

/**
  * @brief Check whether the specified TSC flag is set or not.
  * @param  __HANDLE__ TSC handle
  * @param  __FLAG__ TSC flag
  * @retval SET or RESET
  */
#define __HAL_TSC_GET_FLAG(__HANDLE__, __FLAG__)                   ((((__HANDLE__)->Instance->ISR & (__FLAG__)) == (__FLAG__)) ? SET : RESET)

/**
  * @brief Clear the TSC's pending flag.
  * @param  __HANDLE__ TSC handle
  * @param  __FLAG__ TSC flag
  * @retval None
  */
#define __HAL_TSC_CLEAR_FLAG(__HANDLE__, __FLAG__)                 ((__HANDLE__)->Instance->ICR = (__FLAG__))

/**
  * @brief Enable schmitt trigger hysteresis on a group of IOs.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_ENABLE_HYSTERESIS(__HANDLE__, __GX_IOY_MASK__)   ((__HANDLE__)->Instance->IOHCR |= (__GX_IOY_MASK__))

/**
  * @brief Disable schmitt trigger hysteresis on a group of IOs.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_DISABLE_HYSTERESIS(__HANDLE__, __GX_IOY_MASK__)  ((__HANDLE__)->Instance->IOHCR &= (~(__GX_IOY_MASK__)))

/**
  * @brief Open analog switch on a group of IOs.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_OPEN_ANALOG_SWITCH(__HANDLE__, __GX_IOY_MASK__)  ((__HANDLE__)->Instance->IOASCR &= (~(__GX_IOY_MASK__)))

/**
  * @brief Close analog switch on a group of IOs.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_CLOSE_ANALOG_SWITCH(__HANDLE__, __GX_IOY_MASK__) ((__HANDLE__)->Instance->IOASCR |= (__GX_IOY_MASK__))

/**
  * @brief Enable a group of IOs in channel mode.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_ENABLE_CHANNEL(__HANDLE__, __GX_IOY_MASK__)      ((__HANDLE__)->Instance->IOCCR |= (__GX_IOY_MASK__))

/**
  * @brief Disable a group of channel IOs.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_DISABLE_CHANNEL(__HANDLE__, __GX_IOY_MASK__)     ((__HANDLE__)->Instance->IOCCR &= (~(__GX_IOY_MASK__)))

/**
  * @brief Enable a group of IOs in sampling mode.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_ENABLE_SAMPLING(__HANDLE__, __GX_IOY_MASK__)     ((__HANDLE__)->Instance->IOSCR |= (__GX_IOY_MASK__))

/**
  * @brief Disable a group of sampling IOs.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_IOY_MASK__ IOs mask
  * @retval None
  */
#define __HAL_TSC_DISABLE_SAMPLING(__HANDLE__, __GX_IOY_MASK__) ((__HANDLE__)->Instance->IOSCR &= (~(__GX_IOY_MASK__)))

/**
  * @brief Enable acquisition groups.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_MASK__ Groups mask
  * @retval None
  */
#define __HAL_TSC_ENABLE_GROUP(__HANDLE__, __GX_MASK__) ((__HANDLE__)->Instance->IOGCSR |= (__GX_MASK__))

/**
  * @brief Disable acquisition groups.
  * @param  __HANDLE__ TSC handle
  * @param  __GX_MASK__ Groups mask
  * @retval None
  */
#define __HAL_TSC_DISABLE_GROUP(__HANDLE__, __GX_MASK__) ((__HANDLE__)->Instance->IOGCSR &= (~(__GX_MASK__)))

/** @brief Gets acquisition group status.
  * @param  __HANDLE__ TSC Handle
  * @param  __GX_INDEX__ Group index
  * @retval SET or RESET
  */
#define __HAL_TSC_GET_GROUP_STATUS(__HANDLE__, __GX_INDEX__) \
((((__HANDLE__)->Instance->IOGCSR & (uint32_t)(1UL << (((__GX_INDEX__) & 0xFUL) + 16UL))) == (uint32_t)(1UL << (((__GX_INDEX__) & 0xFUL) + 16UL))) ? TSC_GROUP_COMPLETED : TSC_GROUP_ONGOING)

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/

/** @defgroup TSC_Private_Macros TSC Private Macros
  * @{
  */

#define IS_TSC_CTPH(__VALUE__)          (((__VALUE__) == TSC_CTPH_1CYCLE)   || \
                                         ((__VALUE__) == TSC_CTPH_2CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_3CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_4CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_5CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_6CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_7CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_8CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_9CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPH_10CYCLES) || \
                                         ((__VALUE__) == TSC_CTPH_11CYCLES) || \
                                         ((__VALUE__) == TSC_CTPH_12CYCLES) || \
                                         ((__VALUE__) == TSC_CTPH_13CYCLES) || \
                                         ((__VALUE__) == TSC_CTPH_14CYCLES) || \
                                         ((__VALUE__) == TSC_CTPH_15CYCLES) || \
                                         ((__VALUE__) == TSC_CTPH_16CYCLES))

#define IS_TSC_CTPL(__VALUE__)          (((__VALUE__) == TSC_CTPL_1CYCLE)   || \
                                         ((__VALUE__) == TSC_CTPL_2CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_3CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_4CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_5CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_6CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_7CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_8CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_9CYCLES)  || \
                                         ((__VALUE__) == TSC_CTPL_10CYCLES) || \
                                         ((__VALUE__) == TSC_CTPL_11CYCLES) || \
                                         ((__VALUE__) == TSC_CTPL_12CYCLES) || \
                                         ((__VALUE__) == TSC_CTPL_13CYCLES) || \
                                         ((__VALUE__) == TSC_CTPL_14CYCLES) || \
                                         ((__VALUE__) == TSC_CTPL_15CYCLES) || \
                                         ((__VALUE__) == TSC_CTPL_16CYCLES))

#define IS_TSC_SS(__VALUE__)            (((FunctionalState)(__VALUE__) == DISABLE) || ((FunctionalState)(__VALUE__) == ENABLE))

#define IS_TSC_SSD(__VALUE__)           (((__VALUE__) == 0UL) || (((__VALUE__) > 0UL) && ((__VALUE__) < 128UL)))

#define IS_TSC_SS_PRESC(__VALUE__)      (((__VALUE__) == TSC_SS_PRESC_DIV1) || ((__VALUE__) == TSC_SS_PRESC_DIV2))

#define IS_TSC_PG_PRESC(__VALUE__)      (((__VALUE__) == TSC_PG_PRESC_DIV1)  || \
                                         ((__VALUE__) == TSC_PG_PRESC_DIV2)  || \
                                         ((__VALUE__) == TSC_PG_PRESC_DIV4)  || \
                                         ((__VALUE__) == TSC_PG_PRESC_DIV8)  || \
                                         ((__VALUE__) == TSC_PG_PRESC_DIV16) || \
                                         ((__VALUE__) == TSC_PG_PRESC_DIV32) || \
                                         ((__VALUE__) == TSC_PG_PRESC_DIV64) || \
                                         ((__VALUE__) == TSC_PG_PRESC_DIV128))

#define IS_TSC_MCV(__VALUE__)           (((__VALUE__) == TSC_MCV_255)  || \
                                         ((__VALUE__) == TSC_MCV_511)  || \
                                         ((__VALUE__) == TSC_MCV_1023) || \
                                         ((__VALUE__) == TSC_MCV_2047) || \
                                         ((__VALUE__) == TSC_MCV_4095) || \
                                         ((__VALUE__) == TSC_MCV_8191) || \
                                          ((__VALUE__) == TSC_MCV_16383))

#define IS_TSC_IODEF(__VALUE__)         (((__VALUE__) == TSC_IODEF_OUT_PP_LOW) || ((__VALUE__) == TSC_IODEF_IN_FLOAT))

#define IS_TSC_SYNC_POL(__VALUE__)      (((__VALUE__) == TSC_SYNC_POLARITY_FALLING) || ((__VALUE__) == TSC_SYNC_POLARITY_RISING))

#define IS_TSC_ACQ_MODE(__VALUE__)      (((__VALUE__) == TSC_ACQ_MODE_NORMAL) || ((__VALUE__) == TSC_ACQ_MODE_SYNCHRO))

#define IS_TSC_MCE_IT(__VALUE__)        (((FunctionalState)(__VALUE__) == DISABLE) || ((FunctionalState)(__VALUE__) == ENABLE))

#define IS_TSC_GROUP_INDEX(__VALUE__)   (((__VALUE__) == 0UL) || (((__VALUE__) > 0UL) && ((__VALUE__) < (uint32_t)TSC_NB_OF_GROUPS)))


#define IS_TSC_GROUP(__VALUE__)        ((((__VALUE__) & TSC_GROUPX_NOT_SUPPORTED) != TSC_GROUPX_NOT_SUPPORTED) && \
                                        ((((__VALUE__) & TSC_GROUP1_IO1) == TSC_GROUP1_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP1_IO2) == TSC_GROUP1_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP1_IO3) == TSC_GROUP1_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP1_IO4) == TSC_GROUP1_IO4) ||\
                                         (((__VALUE__) & TSC_GROUP2_IO1) == TSC_GROUP2_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP2_IO2) == TSC_GROUP2_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP2_IO3) == TSC_GROUP2_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP2_IO4) == TSC_GROUP2_IO4) ||\
                                         (((__VALUE__) & TSC_GROUP3_IO1) == TSC_GROUP3_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP3_IO2) == TSC_GROUP3_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP3_IO3) == TSC_GROUP3_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP3_IO4) == TSC_GROUP3_IO4) ||\
                                         (((__VALUE__) & TSC_GROUP4_IO1) == TSC_GROUP4_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP4_IO2) == TSC_GROUP4_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP4_IO3) == TSC_GROUP4_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP4_IO4) == TSC_GROUP4_IO4) ||\
                                         (((__VALUE__) & TSC_GROUP5_IO1) == TSC_GROUP5_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP5_IO2) == TSC_GROUP5_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP5_IO3) == TSC_GROUP5_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP5_IO4) == TSC_GROUP5_IO4) ||\
                                         (((__VALUE__) & TSC_GROUP6_IO1) == TSC_GROUP6_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP6_IO2) == TSC_GROUP6_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP6_IO3) == TSC_GROUP6_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP6_IO4) == TSC_GROUP6_IO4) ||\
                                         (((__VALUE__) & TSC_GROUP7_IO1) == TSC_GROUP7_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP7_IO2) == TSC_GROUP7_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP7_IO3) == TSC_GROUP7_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP7_IO4) == TSC_GROUP7_IO4) ||\
                                         (((__VALUE__) & TSC_GROUP8_IO1) == TSC_GROUP8_IO1) ||\
                                         (((__VALUE__) & TSC_GROUP8_IO2) == TSC_GROUP8_IO2) ||\
                                         (((__VALUE__) & TSC_GROUP8_IO3) == TSC_GROUP8_IO3) ||\
                                         (((__VALUE__) & TSC_GROUP8_IO4) == TSC_GROUP8_IO4)))

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup TSC_Exported_Functions
  * @{
  */

/** @addtogroup TSC_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
/* Initialization and de-initialization functions *****************************/
HAL_StatusTypeDef HAL_TSC_Init(TSC_HandleTypeDef *htsc);
HAL_StatusTypeDef HAL_TSC_DeInit(TSC_HandleTypeDef *htsc);
void HAL_TSC_MspInit(TSC_HandleTypeDef *htsc);
void HAL_TSC_MspDeInit(TSC_HandleTypeDef *htsc);

/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_TSC_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef HAL_TSC_RegisterCallback(TSC_HandleTypeDef *htsc, HAL_TSC_CallbackIDTypeDef CallbackID, pTSC_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_TSC_UnRegisterCallback(TSC_HandleTypeDef *htsc, HAL_TSC_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_TSC_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @addtogroup TSC_Exported_Functions_Group2 Input and Output operation functions
  * @{
  */
/* IO operation functions *****************************************************/
HAL_StatusTypeDef HAL_TSC_Start(TSC_HandleTypeDef *htsc);
HAL_StatusTypeDef HAL_TSC_Start_IT(TSC_HandleTypeDef *htsc);
HAL_StatusTypeDef HAL_TSC_Stop(TSC_HandleTypeDef *htsc);
HAL_StatusTypeDef HAL_TSC_Stop_IT(TSC_HandleTypeDef *htsc);
HAL_StatusTypeDef HAL_TSC_PollForAcquisition(TSC_HandleTypeDef *htsc);
TSC_GroupStatusTypeDef HAL_TSC_GroupGetStatus(TSC_HandleTypeDef *htsc, uint32_t gx_index);
uint32_t HAL_TSC_GroupGetValue(TSC_HandleTypeDef *htsc, uint32_t gx_index);
/**
  * @}
  */

/** @addtogroup TSC_Exported_Functions_Group3 Peripheral Control functions
  * @{
  */
/* Peripheral Control functions ***********************************************/
HAL_StatusTypeDef HAL_TSC_IOConfig(TSC_HandleTypeDef *htsc, TSC_IOConfigTypeDef *config);
HAL_StatusTypeDef HAL_TSC_IODischarge(TSC_HandleTypeDef *htsc, FunctionalState choice);
/**
  * @}
  */

/** @addtogroup TSC_Exported_Functions_Group4 Peripheral State and Errors functions
  * @{
  */
/* Peripheral State and Error functions ***************************************/
HAL_TSC_StateTypeDef HAL_TSC_GetState(TSC_HandleTypeDef *htsc);
/**
  * @}
  */

/** @addtogroup TSC_IRQ_Handler_and_Callbacks IRQ Handler and Callbacks
 * @{
 */
/******* TSC IRQHandler and Callbacks used in Interrupt mode */
void HAL_TSC_IRQHandler(TSC_HandleTypeDef *htsc);
void HAL_TSC_ConvCpltCallback(TSC_HandleTypeDef *htsc);
void HAL_TSC_ErrorCallback(TSC_HandleTypeDef *htsc);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_TSC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
