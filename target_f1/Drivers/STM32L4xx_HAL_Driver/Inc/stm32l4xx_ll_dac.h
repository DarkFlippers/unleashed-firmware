/**
  ******************************************************************************
  * @file    stm32l4xx_ll_dac.h
  * @author  MCD Application Team
  * @brief   Header file of DAC LL module.
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
#ifndef STM32L4xx_LL_DAC_H
#define STM32L4xx_LL_DAC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined (DAC1)

/** @defgroup DAC_LL DAC
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/
/** @defgroup DAC_LL_Private_Constants DAC Private Constants
  * @{
  */

/* Internal masks for DAC channels definition */
/* To select into literal LL_DAC_CHANNEL_x the relevant bits for:             */
/* - channel bits position into registers CR, MCR, CCR, SHHR, SHRR            */
/* - channel bits position into register SWTRIG                               */
/* - channel register offset of data holding register DHRx                    */
/* - channel register offset of data output register DORx                     */
/* - channel register offset of sample-and-hold sample time register SHSRx    */
#define DAC_CR_CH1_BITOFFSET           0U    /* Position of channel bits into registers CR, MCR, CCR, SHHR, SHRR of channel 1 */
#define DAC_CR_CH2_BITOFFSET           16U   /* Position of channel bits into registers CR, MCR, CCR, SHHR, SHRR of channel 2 */
#define DAC_CR_CHX_BITOFFSET_MASK      (DAC_CR_CH1_BITOFFSET | DAC_CR_CH2_BITOFFSET)

#define DAC_SWTR_CH1                   (DAC_SWTRIGR_SWTRIG1) /* Channel bit into register SWTRIGR of channel 1. */
#if defined(DAC_CHANNEL2_SUPPORT)
#define DAC_SWTR_CH2                   (DAC_SWTRIGR_SWTRIG2) /* Channel bit into register SWTRIGR of channel 2. */
#define DAC_SWTR_CHX_MASK              (DAC_SWTR_CH1 | DAC_SWTR_CH2)
#else
#define DAC_SWTR_CHX_MASK              (DAC_SWTR_CH1)
#endif /* DAC_CHANNEL2_SUPPORT */

#define DAC_REG_DHR12R1_REGOFFSET      0x00000000U             /* Register DHR12Rx channel 1 taken as reference */
#define DAC_REG_DHR12L1_REGOFFSET      0x00100000U             /* Register offset of DHR12Lx channel 1 versus DHR12Rx channel 1 (shifted left of 20 bits) */
#define DAC_REG_DHR8R1_REGOFFSET       0x02000000U             /* Register offset of DHR8Rx  channel 1 versus DHR12Rx channel 1 (shifted left of 24 bits) */
#if defined(DAC_CHANNEL2_SUPPORT)
#define DAC_REG_DHR12R2_REGOFFSET      0x30000000U             /* Register offset of DHR12Rx channel 2 versus DHR12Rx channel 1 (shifted left of 28 bits) */
#define DAC_REG_DHR12L2_REGOFFSET      0x00400000U             /* Register offset of DHR12Lx channel 2 versus DHR12Rx channel 1 (shifted left of 20 bits) */
#define DAC_REG_DHR8R2_REGOFFSET       0x05000000U             /* Register offset of DHR8Rx  channel 2 versus DHR12Rx channel 1 (shifted left of 24 bits) */
#endif /* DAC_CHANNEL2_SUPPORT */
#define DAC_REG_DHR12RX_REGOFFSET_MASK 0xF0000000U
#define DAC_REG_DHR12LX_REGOFFSET_MASK 0x00F00000U
#define DAC_REG_DHR8RX_REGOFFSET_MASK  0x0F000000U
#define DAC_REG_DHRX_REGOFFSET_MASK    (DAC_REG_DHR12RX_REGOFFSET_MASK | DAC_REG_DHR12LX_REGOFFSET_MASK | DAC_REG_DHR8RX_REGOFFSET_MASK)

#define DAC_REG_DOR1_REGOFFSET         0x00000000U             /* Register DORx channel 1 taken as reference */
#if defined(DAC_CHANNEL2_SUPPORT)
#define DAC_REG_DOR2_REGOFFSET         0x00000020U             /* Register offset of DORx channel 1 versus DORx channel 2 (shifted left of 5 bits) */
#define DAC_REG_DORX_REGOFFSET_MASK    (DAC_REG_DOR1_REGOFFSET | DAC_REG_DOR2_REGOFFSET)
#else
#define DAC_REG_DORX_REGOFFSET_MASK    (DAC_REG_DOR1_REGOFFSET)
#endif /* DAC_CHANNEL2_SUPPORT */

#define DAC_REG_SHSR1_REGOFFSET        0x00000000U             /* Register SHSRx channel 1 taken as reference */
#if defined(DAC_CHANNEL2_SUPPORT)
#define DAC_REG_SHSR2_REGOFFSET        0x00000040U             /* Register offset of SHSRx channel 1 versus SHSRx channel 2 (shifted left of 6 bits) */
#define DAC_REG_SHSRX_REGOFFSET_MASK   (DAC_REG_SHSR1_REGOFFSET | DAC_REG_SHSR2_REGOFFSET)
#else
#define DAC_REG_SHSRX_REGOFFSET_MASK   (DAC_REG_SHSR1_REGOFFSET)
#endif /* DAC_CHANNEL2_SUPPORT */

#define DAC_REG_DHR_REGOFFSET_MASK_POSBIT0         0x0000000FU  /* Mask of data hold registers offset (DHR12Rx, DHR12Lx, DHR8Rx, ...) when shifted to position 0 */
#define DAC_REG_DORX_REGOFFSET_MASK_POSBIT0        0x00000001U  /* Mask of DORx registers offset when shifted to position 0 */
#define DAC_REG_SHSRX_REGOFFSET_MASK_POSBIT0       0x00000001U  /* Mask of SHSRx registers offset when shifted to position 0 */

#define DAC_REG_DHR12RX_REGOFFSET_BITOFFSET_POS           28U   /* Position of bits register offset of DHR12Rx channel 1 or 2 versus DHR12Rx channel 1 (shifted left of 28 bits) */
#define DAC_REG_DHR12LX_REGOFFSET_BITOFFSET_POS           20U   /* Position of bits register offset of DHR12Lx channel 1 or 2 versus DHR12Rx channel 1 (shifted left of 20 bits) */
#define DAC_REG_DHR8RX_REGOFFSET_BITOFFSET_POS            24U   /* Position of bits register offset of DHR8Rx  channel 1 or 2 versus DHR12Rx channel 1 (shifted left of 24 bits) */
#define DAC_REG_DORX_REGOFFSET_BITOFFSET_POS               5U   /* Position of bits register offset of DORx channel 1 or 2 versus DORx channel 1 (shifted left of 5 bits) */
#define DAC_REG_SHSRX_REGOFFSET_BITOFFSET_POS              6U   /* Position of bits register offset of SHSRx channel 1 or 2 versus SHSRx channel 1 (shifted left of 6 bits) */

/* DAC registers bits positions */
#if defined(DAC_CHANNEL2_SUPPORT)
#define DAC_DHR12RD_DACC2DHR_BITOFFSET_POS                DAC_DHR12RD_DACC2DHR_Pos
#define DAC_DHR12LD_DACC2DHR_BITOFFSET_POS                DAC_DHR12LD_DACC2DHR_Pos
#define DAC_DHR8RD_DACC2DHR_BITOFFSET_POS                 DAC_DHR8RD_DACC2DHR_Pos
#endif /* DAC_CHANNEL2_SUPPORT */

/* Miscellaneous data */
#define DAC_DIGITAL_SCALE_12BITS                        4095U  /* Full-scale digital value with a resolution of 12 bits (voltage range determined by analog voltage references Vref+ and Vref-, refer to reference manual) */

/**
  * @}
  */


/* Private macros ------------------------------------------------------------*/
/** @defgroup DAC_LL_Private_Macros DAC Private Macros
  * @{
  */

/**
  * @brief  Driver macro reserved for internal use: set a pointer to
  *         a register from a register basis from which an offset
  *         is applied.
  * @param  __REG__ Register basis from which the offset is applied.
  * @param  __REG_OFFFSET__ Offset to be applied (unit: number of registers).
  * @retval Pointer to register address
*/
#define __DAC_PTR_REG_OFFSET(__REG__, __REG_OFFFSET__)                         \
 ((uint32_t *)((uint32_t) ((uint32_t)(&(__REG__)) + ((__REG_OFFFSET__) << 2U))))

/**
  * @}
  */


/* Exported types ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup DAC_LL_ES_INIT DAC Exported Init structure
  * @{
  */

/**
  * @brief  Structure definition of some features of DAC instance.
  */
typedef struct
{
  uint32_t TriggerSource;               /*!< Set the conversion trigger source for the selected DAC channel: internal (SW start) or from external peripheral (timer event, external interrupt line).
                                             This parameter can be a value of @ref DAC_LL_EC_TRIGGER_SOURCE

                                             This feature can be modified afterwards using unitary function @ref LL_DAC_SetTriggerSource(). */

  uint32_t WaveAutoGeneration;          /*!< Set the waveform automatic generation mode for the selected DAC channel.
                                             This parameter can be a value of @ref DAC_LL_EC_WAVE_AUTO_GENERATION_MODE

                                             This feature can be modified afterwards using unitary function @ref LL_DAC_SetWaveAutoGeneration(). */

  uint32_t WaveAutoGenerationConfig;    /*!< Set the waveform automatic generation mode for the selected DAC channel.
                                             If waveform automatic generation mode is set to noise, this parameter can be a value of @ref DAC_LL_EC_WAVE_NOISE_LFSR_UNMASK_BITS
                                             If waveform automatic generation mode is set to triangle, this parameter can be a value of @ref DAC_LL_EC_WAVE_TRIANGLE_AMPLITUDE
                                             @note If waveform automatic generation mode is disabled, this parameter is discarded.

                                             This feature can be modified afterwards using unitary function @ref LL_DAC_SetWaveNoiseLFSR(), @ref LL_DAC_SetWaveTriangleAmplitude()
                                             depending on the wave automatic generation selected. */

  uint32_t OutputBuffer;                /*!< Set the output buffer for the selected DAC channel.
                                             This parameter can be a value of @ref DAC_LL_EC_OUTPUT_BUFFER

                                             This feature can be modified afterwards using unitary function @ref LL_DAC_SetOutputBuffer(). */

  uint32_t OutputConnection;            /*!< Set the output connection for the selected DAC channel.
                                             This parameter can be a value of @ref DAC_LL_EC_OUTPUT_CONNECTION

                                             This feature can be modified afterwards using unitary function @ref LL_DAC_SetOutputConnection(). */

  uint32_t OutputMode;                  /*!< Set the output mode normal or sample-and-hold for the selected DAC channel.
                                             This parameter can be a value of @ref DAC_LL_EC_OUTPUT_MODE

                                             This feature can be modified afterwards using unitary function @ref LL_DAC_SetOutputMode(). */
} LL_DAC_InitTypeDef;

/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/* Exported constants --------------------------------------------------------*/
/** @defgroup DAC_LL_Exported_Constants DAC Exported Constants
  * @{
  */

/** @defgroup DAC_LL_EC_GET_FLAG DAC flags
  * @brief    Flags defines which can be used with LL_DAC_ReadReg function
  * @{
  */
/* DAC channel 1 flags */
#define LL_DAC_FLAG_DMAUDR1                (DAC_SR_DMAUDR1)   /*!< DAC channel 1 flag DMA underrun */
#define LL_DAC_FLAG_CAL1                   (DAC_SR_CAL_FLAG1) /*!< DAC channel 1 flag offset calibration status */
#define LL_DAC_FLAG_BWST1                  (DAC_SR_BWST1)     /*!< DAC channel 1 flag busy writing sample time */

#if defined(DAC_CHANNEL2_SUPPORT)
/* DAC channel 2 flags */
#define LL_DAC_FLAG_DMAUDR2                (DAC_SR_DMAUDR2)   /*!< DAC channel 2 flag DMA underrun */
#define LL_DAC_FLAG_CAL2                   (DAC_SR_CAL_FLAG2) /*!< DAC channel 2 flag offset calibration status */
#define LL_DAC_FLAG_BWST2                  (DAC_SR_BWST2)     /*!< DAC channel 2 flag busy writing sample time */
#endif /* DAC_CHANNEL2_SUPPORT */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_IT DAC interruptions
  * @brief    IT defines which can be used with LL_DAC_ReadReg and  LL_DAC_WriteReg functions
  * @{
  */
#define LL_DAC_IT_DMAUDRIE1                (DAC_CR_DMAUDRIE1) /*!< DAC channel 1 interruption DMA underrun */
#if defined(DAC_CHANNEL2_SUPPORT)
#define LL_DAC_IT_DMAUDRIE2                (DAC_CR_DMAUDRIE2) /*!< DAC channel 2 interruption DMA underrun */
#endif /* DAC_CHANNEL2_SUPPORT */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_CHANNEL DAC channels
  * @{
  */
#define LL_DAC_CHANNEL_1                   (DAC_REG_SHSR1_REGOFFSET | DAC_REG_DOR1_REGOFFSET | DAC_REG_DHR12R1_REGOFFSET | DAC_REG_DHR12L1_REGOFFSET | DAC_REG_DHR8R1_REGOFFSET | DAC_CR_CH1_BITOFFSET | DAC_SWTR_CH1) /*!< DAC channel 1 */
#if defined(DAC_CHANNEL2_SUPPORT)
#define LL_DAC_CHANNEL_2                   (DAC_REG_SHSR2_REGOFFSET | DAC_REG_DOR2_REGOFFSET | DAC_REG_DHR12R2_REGOFFSET | DAC_REG_DHR12L2_REGOFFSET | DAC_REG_DHR8R2_REGOFFSET | DAC_CR_CH2_BITOFFSET | DAC_SWTR_CH2) /*!< DAC channel 2 */
#endif /* DAC_CHANNEL2_SUPPORT */
/**
  * @}
  */
#if defined (DAC_CR_HFSEL) /* High frequency interface mode */

/** @defgroup DAC_LL_EC_HIGH_FREQUENCY_MODE DAC high frequency interface mode
  * @brief    High frequency interface mode defines that can be used with LL_DAC_SetHighFrequencyMode and LL_DAC_GetHighFrequencyMode
  * @{
  */
#define LL_DAC_HIGH_FREQ_MODE_DISABLE         0x00000000U        /*!< High frequency interface mode disabled */
#define LL_DAC_HIGH_FREQ_MODE_ABOVE_80MHZ     (DAC_CR_HFSEL)     /*!< High frequency interface mode compatible to AHB>80MHz enabled */
/**
  * @}
  */
#endif /* High frequency interface mode */

/** @defgroup DAC_LL_EC_OPERATING_MODE DAC operating mode
  * @{
  */
#define LL_DAC_MODE_NORMAL_OPERATION       0x00000000U             /*!< DAC channel in mode normal operation */
#define LL_DAC_MODE_CALIBRATION            (DAC_CR_CEN1)           /*!< DAC channel in mode calibration */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_TRIGGER_SOURCE DAC trigger source
  * @{
  */
#if defined (DAC_CR_TSEL1_3)
#define LL_DAC_TRIG_EXT_TIM1_TRGO          (                                                   DAC_CR_TSEL1_0) /*!< DAC channel conversion trigger from external IP: TIM1 TRGO. */
#define LL_DAC_TRIG_EXT_TIM2_TRGO          (                                  DAC_CR_TSEL1_1                 ) /*!< DAC channel conversion trigger from external IP: TIM2 TRGO. */
#define LL_DAC_TRIG_EXT_TIM4_TRGO          (                                  DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0) /*!< DAC channel conversion trigger from external IP: TIM4 TRGO. */
#define LL_DAC_TRIG_EXT_TIM5_TRGO          (                 DAC_CR_TSEL1_2                                  ) /*!< DAC channel conversion trigger from external IP: TIM5 TRGO. */
#define LL_DAC_TRIG_EXT_TIM6_TRGO          (                 DAC_CR_TSEL1_2 |                  DAC_CR_TSEL1_0) /*!< DAC channel conversion trigger from external IP: TIM6 TRGO. */
#define LL_DAC_TRIG_EXT_TIM7_TRGO          (                 DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1                 ) /*!< DAC channel conversion trigger from external IP: TIM7 TRGO. */
#define LL_DAC_TRIG_EXT_TIM8_TRGO          (                 DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0) /*!< DAC channel conversion trigger from external IP: TIM8 TRGO. */
#define LL_DAC_TRIG_EXT_TIM15_TRGO         (DAC_CR_TSEL1_3                                                   ) /*!< DAC channel conversion trigger from external IP: TIM15 TRGO. */
#define LL_DAC_TRIG_EXT_LPTIM1_OUT         (DAC_CR_TSEL1_3                  | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0) /*!< DAC channel conversion trigger from external IP: LPTIM1 TRGO. */
#define LL_DAC_TRIG_EXT_LPTIM2_OUT         (DAC_CR_TSEL1_3 | DAC_CR_TSEL1_2                                  ) /*!< DAC channel conversion trigger from external IP: LPTIM2 TRGO. */
#define LL_DAC_TRIG_EXT_EXTI_LINE9         (DAC_CR_TSEL1_3 | DAC_CR_TSEL1_2                  | DAC_CR_TSEL1_0) /*!< DAC channel conversion trigger from external IP: external interrupt line 9.  */
#define LL_DAC_TRIG_SOFTWARE               0x00000000U                                                         /*!< DAC channel conversion trigger internal (SW start) */
#else
#define LL_DAC_TRIG_SOFTWARE               (DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0)                  /*!< DAC channel conversion trigger internal (SW start) */
#define LL_DAC_TRIG_EXT_TIM2_TRGO          (DAC_CR_TSEL1_2                                  )                  /*!< DAC channel conversion trigger from external IP: TIM2 TRGO. */
#define LL_DAC_TRIG_EXT_TIM4_TRGO          (DAC_CR_TSEL1_2                  | DAC_CR_TSEL1_0)                  /*!< DAC channel conversion trigger from external IP: TIM4 TRGO. */
#define LL_DAC_TRIG_EXT_TIM5_TRGO          (                 DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0)                  /*!< DAC channel conversion trigger from external IP: TIM5 TRGO. */
#define LL_DAC_TRIG_EXT_TIM6_TRGO          0x00000000U                                                         /*!< DAC channel conversion trigger from external IP: TIM6 TRGO. */
#define LL_DAC_TRIG_EXT_TIM7_TRGO          (                 DAC_CR_TSEL1_1                 )                  /*!< DAC channel conversion trigger from external IP: TIM7 TRGO. */
#define LL_DAC_TRIG_EXT_TIM8_TRGO          (                                  DAC_CR_TSEL1_0)                  /*!< DAC channel conversion trigger from external IP: TIM8 TRGO. */
#define LL_DAC_TRIG_EXT_EXTI_LINE9         (DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1                 )                  /*!< DAC channel conversion trigger from external IP: external interrupt line 9. */
#endif

/**
  * @}
  */

/** @defgroup DAC_LL_EC_WAVE_AUTO_GENERATION_MODE DAC waveform automatic generation mode
  * @{
  */
#define LL_DAC_WAVE_AUTO_GENERATION_NONE     0x00000000U                     /*!< DAC channel wave auto generation mode disabled. */
#define LL_DAC_WAVE_AUTO_GENERATION_NOISE    (               DAC_CR_WAVE1_0) /*!< DAC channel wave auto generation mode enabled, set generated noise waveform. */
#define LL_DAC_WAVE_AUTO_GENERATION_TRIANGLE (DAC_CR_WAVE1_1               ) /*!< DAC channel wave auto generation mode enabled, set generated triangle waveform. */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_WAVE_NOISE_LFSR_UNMASK_BITS DAC wave generation - Noise LFSR unmask bits
  * @{
  */
#define LL_DAC_NOISE_LFSR_UNMASK_BIT0      0x00000000U                                                         /*!< Noise wave generation, unmask LFSR bit0, for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS1_0   (                                                   DAC_CR_MAMP1_0) /*!< Noise wave generation, unmask LFSR bits[1:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS2_0   (                                  DAC_CR_MAMP1_1                 ) /*!< Noise wave generation, unmask LFSR bits[2:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS3_0   (                                  DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Noise wave generation, unmask LFSR bits[3:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS4_0   (                 DAC_CR_MAMP1_2                                  ) /*!< Noise wave generation, unmask LFSR bits[4:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS5_0   (                 DAC_CR_MAMP1_2                  | DAC_CR_MAMP1_0) /*!< Noise wave generation, unmask LFSR bits[5:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS6_0   (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1                 ) /*!< Noise wave generation, unmask LFSR bits[6:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS7_0   (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Noise wave generation, unmask LFSR bits[7:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS8_0   (DAC_CR_MAMP1_3                                                   ) /*!< Noise wave generation, unmask LFSR bits[8:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS9_0   (DAC_CR_MAMP1_3                                   | DAC_CR_MAMP1_0) /*!< Noise wave generation, unmask LFSR bits[9:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS10_0  (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1                 ) /*!< Noise wave generation, unmask LFSR bits[10:0], for the selected DAC channel */
#define LL_DAC_NOISE_LFSR_UNMASK_BITS11_0  (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Noise wave generation, unmask LFSR bits[11:0], for the selected DAC channel */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_WAVE_TRIANGLE_AMPLITUDE DAC wave generation - Triangle amplitude
  * @{
  */
#define LL_DAC_TRIANGLE_AMPLITUDE_1        0x00000000U                                                         /*!< Triangle wave generation, amplitude of 1 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_3        (                                                   DAC_CR_MAMP1_0) /*!< Triangle wave generation, amplitude of 3 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_7        (                                  DAC_CR_MAMP1_1                 ) /*!< Triangle wave generation, amplitude of 7 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_15       (                                  DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Triangle wave generation, amplitude of 15 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_31       (                 DAC_CR_MAMP1_2                                  ) /*!< Triangle wave generation, amplitude of 31 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_63       (                 DAC_CR_MAMP1_2                  | DAC_CR_MAMP1_0) /*!< Triangle wave generation, amplitude of 63 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_127      (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1                 ) /*!< Triangle wave generation, amplitude of 127 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_255      (                 DAC_CR_MAMP1_2 | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Triangle wave generation, amplitude of 255 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_511      (DAC_CR_MAMP1_3                                                   ) /*!< Triangle wave generation, amplitude of 512 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_1023     (DAC_CR_MAMP1_3                                   | DAC_CR_MAMP1_0) /*!< Triangle wave generation, amplitude of 1023 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_2047     (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1                 ) /*!< Triangle wave generation, amplitude of 2047 LSB of DAC output range, for the selected DAC channel */
#define LL_DAC_TRIANGLE_AMPLITUDE_4095     (DAC_CR_MAMP1_3                  | DAC_CR_MAMP1_1 | DAC_CR_MAMP1_0) /*!< Triangle wave generation, amplitude of 4095 LSB of DAC output range, for the selected DAC channel */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_OUTPUT_MODE DAC channel output mode
  * @{
  */
#define LL_DAC_OUTPUT_MODE_NORMAL          0x00000000U             /*!< The selected DAC channel output is on mode normal. */
#define LL_DAC_OUTPUT_MODE_SAMPLE_AND_HOLD (DAC_MCR_MODE1_2)       /*!< The selected DAC channel output is on mode sample-and-hold. Mode sample-and-hold requires an external capacitor, refer to description of function @ref LL_DAC_ConfigOutput() or @ref LL_DAC_SetOutputMode(). */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_OUTPUT_BUFFER DAC channel output buffer
  * @{
  */
#define LL_DAC_OUTPUT_BUFFER_ENABLE        0x00000000U             /*!< The selected DAC channel output is buffered: higher drive current capability, but also higher current consumption */
#define LL_DAC_OUTPUT_BUFFER_DISABLE       (DAC_MCR_MODE1_1)       /*!< The selected DAC channel output is not buffered: lower drive current capability, but also lower current consumption */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_OUTPUT_CONNECTION DAC channel output connection
  * @{
  */
#define LL_DAC_OUTPUT_CONNECT_GPIO         0x00000000U             /*!< The selected DAC channel output is connected to external pin */
#define LL_DAC_OUTPUT_CONNECT_INTERNAL     (DAC_MCR_MODE1_0)       /*!< The selected DAC channel output is connected to on-chip peripherals via internal paths. On this STM32 serie, output connection depends on output mode (normal or sample and hold) and output buffer state. Refer to comments of function @ref LL_DAC_SetOutputConnection(). */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_RESOLUTION  DAC channel output resolution
  * @{
  */
#define LL_DAC_RESOLUTION_12B              0x00000000U             /*!< DAC channel resolution 12 bits */
#define LL_DAC_RESOLUTION_8B               0x00000002U             /*!< DAC channel resolution 8 bits */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_REGISTERS  DAC registers compliant with specific purpose
  * @{
  */
/* List of DAC registers intended to be used (most commonly) with             */
/* DMA transfer.                                                              */
/* Refer to function @ref LL_DAC_DMA_GetRegAddr().                            */
#define LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED  DAC_REG_DHR12RX_REGOFFSET_BITOFFSET_POS /*!< DAC channel data holding register 12 bits right aligned */
#define LL_DAC_DMA_REG_DATA_12BITS_LEFT_ALIGNED   DAC_REG_DHR12LX_REGOFFSET_BITOFFSET_POS /*!< DAC channel data holding register 12 bits left aligned */
#define LL_DAC_DMA_REG_DATA_8BITS_RIGHT_ALIGNED   DAC_REG_DHR8RX_REGOFFSET_BITOFFSET_POS  /*!< DAC channel data holding register 8 bits right aligned */
/**
  * @}
  */

/** @defgroup DAC_LL_EC_HW_DELAYS  Definitions of DAC hardware constraints delays
  * @note   Only DAC peripheral HW delays are defined in DAC LL driver driver,
  *         not timeout values.
  *         For details on delays values, refer to descriptions in source code
  *         above each literal definition.
  * @{
  */

/* Delay for DAC channel voltage settling time from DAC channel startup       */
/* (transition from disable to enable).                                       */
/* Note: DAC channel startup time depends on board application environment:   */
/*       impedance connected to DAC channel output.                           */
/*       The delay below is specified under conditions:                       */
/*        - voltage maximum transition (lowest to highest value)              */
/*        - until voltage reaches final value +-1LSB                          */
/*        - DAC channel output buffer enabled                                 */
/*        - load impedance of 5kOhm (min), 50pF (max)                         */
/* Literal set to maximum value (refer to device datasheet,                   */
/* parameter "tWAKEUP").                                                      */
/* Unit: us                                                                   */
#define LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US             8U  /*!< Delay for DAC channel voltage settling time from DAC channel startup (transition from disable to enable) */


/* Delay for DAC channel voltage settling time.                               */
/* Note: DAC channel startup time depends on board application environment:   */
/*       impedance connected to DAC channel output.                           */
/*       The delay below is specified under conditions:                       */
/*        - voltage maximum transition (lowest to highest value)              */
/*        - until voltage reaches final value +-1LSB                          */
/*        - DAC channel output buffer enabled                                 */
/*        - load impedance of 5kOhm min, 50pF max                             */
/* Literal set to maximum value (refer to device datasheet,                   */
/* parameter "tSETTLING").                                                    */
/* Unit: us                                                                   */
#define LL_DAC_DELAY_VOLTAGE_SETTLING_US                     3U  /*!< Delay for DAC channel voltage settling time */

/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup DAC_LL_Exported_Macros DAC Exported Macros
  * @{
  */

/** @defgroup DAC_LL_EM_WRITE_READ Common write and read registers macros
  * @{
  */

/**
  * @brief  Write a value in DAC register
  * @param  __INSTANCE__ DAC Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_DAC_WriteReg(__INSTANCE__, __REG__, __VALUE__) WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
  * @brief  Read a value in DAC register
  * @param  __INSTANCE__ DAC Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_DAC_ReadReg(__INSTANCE__, __REG__) READ_REG(__INSTANCE__->__REG__)

/**
  * @}
  */

/** @defgroup DAC_LL_EM_HELPER_MACRO DAC helper macro
  * @{
  */

/**
  * @brief  Helper macro to get DAC channel number in decimal format
  *         from literals LL_DAC_CHANNEL_x.
  *         Example:
  *            __LL_DAC_CHANNEL_TO_DECIMAL_NB(LL_DAC_CHANNEL_1)
  *            will return decimal number "1".
  * @note   The input can be a value from functions where a channel
  *         number is returned.
  * @param  __CHANNEL__ This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2
  * @retval 1...2
  */
#define __LL_DAC_CHANNEL_TO_DECIMAL_NB(__CHANNEL__)                            \
  ((__CHANNEL__) & DAC_SWTR_CHX_MASK)

/**
  * @brief  Helper macro to get DAC channel in literal format LL_DAC_CHANNEL_x
  *         from number in decimal format.
  *         Example:
  *           __LL_DAC_DECIMAL_NB_TO_CHANNEL(1)
  *           will return a data equivalent to "LL_DAC_CHANNEL_1".
  * @note  If the input parameter does not correspond to a DAC channel,
  *        this macro returns value '0'.
  * @param  __DECIMAL_NB__ 1...2
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2
  */
#if defined(DAC_CHANNEL2_SUPPORT)
#define __LL_DAC_DECIMAL_NB_TO_CHANNEL(__DECIMAL_NB__)                         \
  (((__DECIMAL_NB__) == 1U)                                                    \
    ? (                                                                        \
       LL_DAC_CHANNEL_1                                                        \
      )                                                                        \
      :                                                                        \
      (((__DECIMAL_NB__) == 2U)                                                \
        ? (                                                                    \
           LL_DAC_CHANNEL_2                                                    \
          )                                                                    \
          :                                                                    \
          (                                                                    \
           0U                                                                  \
          )                                                                    \
      )                                                                        \
  )
#else
#define __LL_DAC_DECIMAL_NB_TO_CHANNEL(__DECIMAL_NB__)                         \
  (((__DECIMAL_NB__) == 1U)                                                    \
    ? (                                                                        \
       LL_DAC_CHANNEL_1                                                        \
      )                                                                        \
      :                                                                        \
      (                                                                        \
       0U                                                                      \
      )                                                                        \
  )
#endif  /* DAC_CHANNEL2_SUPPORT */

/**
  * @brief  Helper macro to define the DAC conversion data full-scale digital
  *         value corresponding to the selected DAC resolution.
  * @note   DAC conversion data full-scale corresponds to voltage range
  *         determined by analog voltage references Vref+ and Vref-
  *         (refer to reference manual).
  * @param  __DAC_RESOLUTION__ This parameter can be one of the following values:
  *         @arg @ref LL_DAC_RESOLUTION_12B
  *         @arg @ref LL_DAC_RESOLUTION_8B
  * @retval ADC conversion data equivalent voltage value (unit: mVolt)
  */
#define __LL_DAC_DIGITAL_SCALE(__DAC_RESOLUTION__)                             \
  ((0x00000FFFU) >> ((__DAC_RESOLUTION__) << 1U))

/**
  * @brief  Helper macro to calculate the DAC conversion data (unit: digital
  *         value) corresponding to a voltage (unit: mVolt).
  * @note   This helper macro is intended to provide input data in voltage
  *         rather than digital value,
  *         to be used with LL DAC functions such as
  *         @ref LL_DAC_ConvertData12RightAligned().
  * @note   Analog reference voltage (Vref+) must be either known from
  *         user board environment or can be calculated using ADC measurement
  *         and ADC helper macro @ref __LL_ADC_CALC_VREFANALOG_VOLTAGE().
  * @param  __VREFANALOG_VOLTAGE__ Analog reference voltage (unit: mV)
  * @param  __DAC_VOLTAGE__ Voltage to be generated by DAC channel
  *                         (unit: mVolt).
  * @param  __DAC_RESOLUTION__ This parameter can be one of the following values:
  *         @arg @ref LL_DAC_RESOLUTION_12B
  *         @arg @ref LL_DAC_RESOLUTION_8B
  * @retval DAC conversion data (unit: digital value)
  */
#define __LL_DAC_CALC_VOLTAGE_TO_DATA(__VREFANALOG_VOLTAGE__,\
                                      __DAC_VOLTAGE__,\
                                      __DAC_RESOLUTION__)                      \
  ((__DAC_VOLTAGE__) * __LL_DAC_DIGITAL_SCALE(__DAC_RESOLUTION__)              \
   / (__VREFANALOG_VOLTAGE__)                                                  \
  )

/**
  * @}
  */

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup DAC_LL_Exported_Functions DAC Exported Functions
  * @{
  */

#if defined (DAC_CR_HFSEL) /* High frequency interface mode */

/** @defgroup DAC_LL_EF_Configuration Configuration of DAC instance
  * @{
  */
/**
  * @brief  Set the high frequency interface mode for the selected DAC instance
  * @rmtoll CR       HFSEL          LL_DAC_SetHighFrequencyMode
  * @param  DACx DAC instance
  * @param  HighFreqMode This parameter can be one of the following values:
  *         @arg @ref LL_DAC_HIGH_FREQ_MODE_DISABLE
  *         @arg @ref LL_DAC_HIGH_FREQ_MODE_ABOVE_80MHZ
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetHighFrequencyMode(DAC_TypeDef *DACx, uint32_t HighFreqMode)
{
  MODIFY_REG(DACx->CR, DAC_CR_HFSEL, HighFreqMode);
}

/**
  * @brief  Get the high frequency interface mode for the selected DAC instance
  * @rmtoll CR       HFSEL          LL_DAC_GetHighFrequencyMode
  * @param  DACx DAC instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_HIGH_FREQ_MODE_DISABLE
  *         @arg @ref LL_DAC_HIGH_FREQ_MODE_ABOVE_80MHZ
  */
__STATIC_INLINE uint32_t LL_DAC_GetHighFrequencyMode(DAC_TypeDef *DACx)
{
  return (uint32_t)(READ_BIT(DACx->CR, DAC_CR_HFSEL));
}
/**
  * @}
  */

#endif /* High frequency interface mode */

/** @defgroup DAC_LL_EF_Configuration Configuration of DAC channels
  * @{
  */

/**
  * @brief  Set the operating mode for the selected DAC channel:
  *         calibration or normal operating mode.
  * @rmtoll CR       CEN1           LL_DAC_SetMode\n
  *         CR       CEN2           LL_DAC_SetMode
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2
  * @param  ChannelMode This parameter can be one of the following values:
  *         @arg @ref LL_DAC_MODE_NORMAL_OPERATION
  *         @arg @ref LL_DAC_MODE_CALIBRATION
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetMode(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t ChannelMode)
{
  MODIFY_REG(DACx->CR,
             DAC_CR_CEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             ChannelMode << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the operating mode for the selected DAC channel:
  *         calibration or normal operating mode.
  * @rmtoll CR       CEN1           LL_DAC_GetMode\n
  *         CR       CEN2           LL_DAC_GetMode
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_MODE_NORMAL_OPERATION
  *         @arg @ref LL_DAC_MODE_CALIBRATION
  */
__STATIC_INLINE uint32_t LL_DAC_GetMode(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->CR, DAC_CR_CEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the offset trimming value for the selected DAC channel.
  *         Trimming has an impact when output buffer is enabled
  *         and is intended to replace factory calibration default values.
  * @rmtoll CCR      OTRIM1         LL_DAC_SetTrimmingValue\n
  *         CCR      OTRIM2         LL_DAC_SetTrimmingValue
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2
  * @param  TrimmingValue Value between Min_Data=0x00 and Max_Data=0x1F
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetTrimmingValue(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t TrimmingValue)
{
  MODIFY_REG(DACx->CCR,
             DAC_CCR_OTRIM1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             TrimmingValue << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the offset trimming value for the selected DAC channel.
  *         Trimming has an impact when output buffer is enabled
  *         and is intended to replace factory calibration default values.
  * @rmtoll CCR      OTRIM1         LL_DAC_GetTrimmingValue\n
  *         CCR      OTRIM2         LL_DAC_GetTrimmingValue
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2
  * @retval TrimmingValue Value between Min_Data=0x00 and Max_Data=0x1F
  */
__STATIC_INLINE uint32_t LL_DAC_GetTrimmingValue(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->CCR, DAC_CCR_OTRIM1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the conversion trigger source for the selected DAC channel.
  * @note   For conversion trigger source to be effective, DAC trigger
  *         must be enabled using function @ref LL_DAC_EnableTrigger().
  * @note   To set conversion trigger source, DAC channel must be disabled.
  *         Otherwise, the setting is discarded.
  * @note   Availability of parameters of trigger sources from timer
  *         depends on timers availability on the selected device.
  * @rmtoll CR       TSEL1          LL_DAC_SetTriggerSource\n
  *         CR       TSEL2          LL_DAC_SetTriggerSource
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2
  * @param  TriggerSource This parameter can be one of the following values:
  *         @arg @ref LL_DAC_TRIG_SOFTWARE
  *         @arg @ref LL_DAC_TRIG_EXT_TIM1_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM2_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM4_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM5_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM6_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM7_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM8_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM15_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_LPTIM1_OUT
  *         @arg @ref LL_DAC_TRIG_EXT_LPTIM2_OUT
  *         @arg @ref LL_DAC_TRIG_EXT_EXTI_LINE9
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetTriggerSource(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t TriggerSource)
{
  MODIFY_REG(DACx->CR,
             DAC_CR_TSEL1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             TriggerSource << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the conversion trigger source for the selected DAC channel.
  * @note   For conversion trigger source to be effective, DAC trigger
  *         must be enabled using function @ref LL_DAC_EnableTrigger().
  * @note   Availability of parameters of trigger sources from timer
  *         depends on timers availability on the selected device.
  * @rmtoll CR       TSEL1          LL_DAC_GetTriggerSource\n
  *         CR       TSEL2          LL_DAC_GetTriggerSource
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_TRIG_SOFTWARE
  *         @arg @ref LL_DAC_TRIG_EXT_TIM1_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM2_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM4_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM5_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM6_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM7_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM8_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_TIM15_TRGO
  *         @arg @ref LL_DAC_TRIG_EXT_LPTIM1_OUT
  *         @arg @ref LL_DAC_TRIG_EXT_LPTIM2_OUT
  *         @arg @ref LL_DAC_TRIG_EXT_EXTI_LINE9
  */
__STATIC_INLINE uint32_t LL_DAC_GetTriggerSource(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->CR, DAC_CR_TSEL1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the waveform automatic generation mode
  *         for the selected DAC channel.
  * @rmtoll CR       WAVE1          LL_DAC_SetWaveAutoGeneration\n
  *         CR       WAVE2          LL_DAC_SetWaveAutoGeneration
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  WaveAutoGeneration This parameter can be one of the following values:
  *         @arg @ref LL_DAC_WAVE_AUTO_GENERATION_NONE
  *         @arg @ref LL_DAC_WAVE_AUTO_GENERATION_NOISE
  *         @arg @ref LL_DAC_WAVE_AUTO_GENERATION_TRIANGLE
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetWaveAutoGeneration(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t WaveAutoGeneration)
{
  MODIFY_REG(DACx->CR,
             DAC_CR_WAVE1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             WaveAutoGeneration << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the waveform automatic generation mode
  *         for the selected DAC channel.
  * @rmtoll CR       WAVE1          LL_DAC_GetWaveAutoGeneration\n
  *         CR       WAVE2          LL_DAC_GetWaveAutoGeneration
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_WAVE_AUTO_GENERATION_NONE
  *         @arg @ref LL_DAC_WAVE_AUTO_GENERATION_NOISE
  *         @arg @ref LL_DAC_WAVE_AUTO_GENERATION_TRIANGLE
  */
__STATIC_INLINE uint32_t LL_DAC_GetWaveAutoGeneration(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->CR, DAC_CR_WAVE1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the noise waveform generation for the selected DAC channel:
  *         Noise mode and parameters LFSR (linear feedback shift register).
  * @note   For wave generation to be effective, DAC channel
  *         wave generation mode must be enabled using
  *         function @ref LL_DAC_SetWaveAutoGeneration().
  * @note   This setting can be set when the selected DAC channel is disabled
  *         (otherwise, the setting operation is ignored).
  * @rmtoll CR       MAMP1          LL_DAC_SetWaveNoiseLFSR\n
  *         CR       MAMP2          LL_DAC_SetWaveNoiseLFSR
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  NoiseLFSRMask This parameter can be one of the following values:
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BIT0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS1_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS2_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS3_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS4_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS5_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS6_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS7_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS8_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS9_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS10_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS11_0
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetWaveNoiseLFSR(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t NoiseLFSRMask)
{
  MODIFY_REG(DACx->CR,
             DAC_CR_MAMP1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             NoiseLFSRMask << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the noise waveform generation for the selected DAC channel:
  *         Noise mode and parameters LFSR (linear feedback shift register).
  * @rmtoll CR       MAMP1          LL_DAC_GetWaveNoiseLFSR\n
  *         CR       MAMP2          LL_DAC_GetWaveNoiseLFSR
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BIT0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS1_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS2_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS3_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS4_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS5_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS6_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS7_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS8_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS9_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS10_0
  *         @arg @ref LL_DAC_NOISE_LFSR_UNMASK_BITS11_0
  */
__STATIC_INLINE uint32_t LL_DAC_GetWaveNoiseLFSR(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->CR, DAC_CR_MAMP1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the triangle waveform generation for the selected DAC channel:
  *         triangle mode and amplitude.
  * @note   For wave generation to be effective, DAC channel
  *         wave generation mode must be enabled using
  *         function @ref LL_DAC_SetWaveAutoGeneration().
  * @note   This setting can be set when the selected DAC channel is disabled
  *         (otherwise, the setting operation is ignored).
  * @rmtoll CR       MAMP1          LL_DAC_SetWaveTriangleAmplitude\n
  *         CR       MAMP2          LL_DAC_SetWaveTriangleAmplitude
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  TriangleAmplitude This parameter can be one of the following values:
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_1
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_3
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_7
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_15
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_31
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_63
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_127
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_255
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_511
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_1023
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_2047
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_4095
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetWaveTriangleAmplitude(DAC_TypeDef *DACx, uint32_t DAC_Channel,
                                                     uint32_t TriangleAmplitude)
{
  MODIFY_REG(DACx->CR,
             DAC_CR_MAMP1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             TriangleAmplitude << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the triangle waveform generation for the selected DAC channel:
  *         triangle mode and amplitude.
  * @rmtoll CR       MAMP1          LL_DAC_GetWaveTriangleAmplitude\n
  *         CR       MAMP2          LL_DAC_GetWaveTriangleAmplitude
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_1
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_3
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_7
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_15
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_31
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_63
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_127
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_255
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_511
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_1023
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_2047
  *         @arg @ref LL_DAC_TRIANGLE_AMPLITUDE_4095
  */
__STATIC_INLINE uint32_t LL_DAC_GetWaveTriangleAmplitude(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->CR, DAC_CR_MAMP1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the output for the selected DAC channel.
  * @note   This function set several features:
  *         - mode normal or sample-and-hold
  *         - buffer
  *         - connection to GPIO or internal path.
  *         These features can also be set individually using
  *         dedicated functions:
  *         - @ref LL_DAC_SetOutputBuffer()
  *         - @ref LL_DAC_SetOutputMode()
  *         - @ref LL_DAC_SetOutputConnection()
  * @note   On this STM32 serie, output connection depends on output mode
  *         (normal or sample and hold) and output buffer state.
  *         - if output connection is set to internal path and output buffer
  *           is enabled (whatever output mode):
  *           output connection is also connected to GPIO pin
  *           (both connections to GPIO pin and internal path).
  *         - if output connection is set to GPIO pin, output buffer
  *           is disabled, output mode set to sample and hold:
  *           output connection is also connected to internal path
  *           (both connections to GPIO pin and internal path).
  * @note   Mode sample-and-hold requires an external capacitor
  *         to be connected between DAC channel output and ground.
  *         Capacitor value depends on load on DAC channel output and
  *         sample-and-hold timings configured.
  *         As indication, capacitor typical value is 100nF
  *         (refer to device datasheet, parameter "CSH").
  * @rmtoll CR       MODE1          LL_DAC_ConfigOutput\n
  *         CR       MODE2          LL_DAC_ConfigOutput
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  OutputMode This parameter can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_MODE_NORMAL
  *         @arg @ref LL_DAC_OUTPUT_MODE_SAMPLE_AND_HOLD
  * @param  OutputBuffer This parameter can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_BUFFER_ENABLE
  *         @arg @ref LL_DAC_OUTPUT_BUFFER_DISABLE
  * @param  OutputConnection This parameter can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_CONNECT_GPIO
  *         @arg @ref LL_DAC_OUTPUT_CONNECT_INTERNAL
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ConfigOutput(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t OutputMode,
                                         uint32_t OutputBuffer, uint32_t OutputConnection)
{
  MODIFY_REG(DACx->MCR,
             (DAC_MCR_MODE1_2 | DAC_MCR_MODE1_1 | DAC_MCR_MODE1_0) << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             (OutputMode | OutputBuffer | OutputConnection) << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Set the output mode normal or sample-and-hold
  *         for the selected DAC channel.
  * @note   Mode sample-and-hold requires an external capacitor
  *         to be connected between DAC channel output and ground.
  *         Capacitor value depends on load on DAC channel output and
  *         sample-and-hold timings configured.
  *         As indication, capacitor typical value is 100nF
  *         (refer to device datasheet, parameter "CSH").
  * @rmtoll CR       MODE1          LL_DAC_SetOutputMode\n
  *         CR       MODE2          LL_DAC_SetOutputMode
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  OutputMode This parameter can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_MODE_NORMAL
  *         @arg @ref LL_DAC_OUTPUT_MODE_SAMPLE_AND_HOLD
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetOutputMode(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t OutputMode)
{
  MODIFY_REG(DACx->MCR,
             (uint32_t)DAC_MCR_MODE1_2 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             OutputMode << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the output mode normal or sample-and-hold for the selected DAC channel.
  * @rmtoll CR       MODE1          LL_DAC_GetOutputMode\n
  *         CR       MODE2          LL_DAC_GetOutputMode
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_MODE_NORMAL
  *         @arg @ref LL_DAC_OUTPUT_MODE_SAMPLE_AND_HOLD
  */
__STATIC_INLINE uint32_t LL_DAC_GetOutputMode(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->MCR, (uint32_t)DAC_MCR_MODE1_2 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the output buffer for the selected DAC channel.
  * @note   On this STM32 serie, when buffer is enabled, its offset can be
  *         trimmed: factory calibration default values can be
  *         replaced by user trimming values, using function
  *         @ref LL_DAC_SetTrimmingValue().
  * @rmtoll CR       MODE1          LL_DAC_SetOutputBuffer\n
  *         CR       MODE2          LL_DAC_SetOutputBuffer
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  OutputBuffer This parameter can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_BUFFER_ENABLE
  *         @arg @ref LL_DAC_OUTPUT_BUFFER_DISABLE
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetOutputBuffer(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t OutputBuffer)
{
  MODIFY_REG(DACx->MCR,
             (uint32_t)DAC_MCR_MODE1_1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             OutputBuffer << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the output buffer state for the selected DAC channel.
  * @rmtoll CR       MODE1          LL_DAC_GetOutputBuffer\n
  *         CR       MODE2          LL_DAC_GetOutputBuffer
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_BUFFER_ENABLE
  *         @arg @ref LL_DAC_OUTPUT_BUFFER_DISABLE
  */
__STATIC_INLINE uint32_t LL_DAC_GetOutputBuffer(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->MCR, (uint32_t)DAC_MCR_MODE1_1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the output connection for the selected DAC channel.
  * @note   On this STM32 serie, output connection depends on output mode (normal or
  *         sample and hold) and output buffer state.
  *         - if output connection is set to internal path and output buffer
  *           is enabled (whatever output mode):
  *           output connection is also connected to GPIO pin
  *           (both connections to GPIO pin and internal path).
  *         - if output connection is set to GPIO pin, output buffer
  *           is disabled, output mode set to sample and hold:
  *           output connection is also connected to internal path
  *           (both connections to GPIO pin and internal path).
  * @rmtoll CR       MODE1          LL_DAC_SetOutputConnection\n
  *         CR       MODE2          LL_DAC_SetOutputConnection
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  OutputConnection This parameter can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_CONNECT_GPIO
  *         @arg @ref LL_DAC_OUTPUT_CONNECT_INTERNAL
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetOutputConnection(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t OutputConnection)
{
  MODIFY_REG(DACx->MCR,
             (uint32_t)DAC_MCR_MODE1_0 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             OutputConnection << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the output connection for the selected DAC channel.
  * @note   On this STM32 serie, output connection depends on output mode (normal or
  *         sample and hold) and output buffer state.
  *         - if output connection is set to internal path and output buffer
  *           is enabled (whatever output mode):
  *           output connection is also connected to GPIO pin
  *           (both connections to GPIO pin and internal path).
  *         - if output connection is set to GPIO pin, output buffer
  *           is disabled, output mode set to sample and hold:
  *           output connection is also connected to internal path
  *           (both connections to GPIO pin and internal path).
  * @rmtoll CR       MODE1          LL_DAC_GetOutputConnection\n
  *         CR       MODE2          LL_DAC_GetOutputConnection
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DAC_OUTPUT_CONNECT_GPIO
  *         @arg @ref LL_DAC_OUTPUT_CONNECT_INTERNAL
  */
__STATIC_INLINE uint32_t LL_DAC_GetOutputConnection(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->MCR, (uint32_t)DAC_MCR_MODE1_0 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the sample-and-hold timing for the selected DAC channel:
  *         sample time
  * @note   Sample time must be set when DAC channel is disabled
  *         or during DAC operation when DAC channel flag BWSTx is reset,
  *         otherwise the setting is ignored.
  *         Check BWSTx flag state using function "LL_DAC_IsActiveFlag_BWSTx()".
  * @rmtoll SHSR1    TSAMPLE1       LL_DAC_SetSampleAndHoldSampleTime\n
  *         SHSR2    TSAMPLE2       LL_DAC_SetSampleAndHoldSampleTime
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  SampleTime Value between Min_Data=0x000 and Max_Data=0x3FF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetSampleAndHoldSampleTime(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t SampleTime)
{
  __IO uint32_t *preg = __DAC_PTR_REG_OFFSET(DACx->SHSR1, (DAC_Channel >> DAC_REG_SHSRX_REGOFFSET_BITOFFSET_POS) & DAC_REG_SHSRX_REGOFFSET_MASK_POSBIT0);

  MODIFY_REG(*preg,
             DAC_SHSR1_TSAMPLE1,
             SampleTime);
}

/**
  * @brief  Get the sample-and-hold timing for the selected DAC channel:
  *         sample time
  * @rmtoll SHSR1    TSAMPLE1       LL_DAC_GetSampleAndHoldSampleTime\n
  *         SHSR2    TSAMPLE2       LL_DAC_GetSampleAndHoldSampleTime
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Value between Min_Data=0x000 and Max_Data=0x3FF
  */
__STATIC_INLINE uint32_t LL_DAC_GetSampleAndHoldSampleTime(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  __IO uint32_t const *preg = __DAC_PTR_REG_OFFSET(DACx->SHSR1, (DAC_Channel >> DAC_REG_SHSRX_REGOFFSET_BITOFFSET_POS) & DAC_REG_SHSRX_REGOFFSET_MASK_POSBIT0);

  return (uint32_t) READ_BIT(*preg, DAC_SHSR1_TSAMPLE1);
}

/**
  * @brief  Set the sample-and-hold timing for the selected DAC channel:
  *         hold time
  * @rmtoll SHHR     THOLD1         LL_DAC_SetSampleAndHoldHoldTime\n
  *         SHHR     THOLD2         LL_DAC_SetSampleAndHoldHoldTime
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  HoldTime Value between Min_Data=0x000 and Max_Data=0x3FF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetSampleAndHoldHoldTime(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t HoldTime)
{
  MODIFY_REG(DACx->SHHR,
             DAC_SHHR_THOLD1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             HoldTime << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the sample-and-hold timing for the selected DAC channel:
  *         hold time
  * @rmtoll SHHR     THOLD1         LL_DAC_GetSampleAndHoldHoldTime\n
  *         SHHR     THOLD2         LL_DAC_GetSampleAndHoldHoldTime
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Value between Min_Data=0x000 and Max_Data=0x3FF
  */
__STATIC_INLINE uint32_t LL_DAC_GetSampleAndHoldHoldTime(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->SHHR, DAC_SHHR_THOLD1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @brief  Set the sample-and-hold timing for the selected DAC channel:
  *         refresh time
  * @rmtoll SHRR     TREFRESH1      LL_DAC_SetSampleAndHoldRefreshTime\n
  *         SHRR     TREFRESH2      LL_DAC_SetSampleAndHoldRefreshTime
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  RefreshTime Value between Min_Data=0x00 and Max_Data=0xFF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_SetSampleAndHoldRefreshTime(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t RefreshTime)
{
  MODIFY_REG(DACx->SHRR,
             DAC_SHRR_TREFRESH1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK),
             RefreshTime << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get the sample-and-hold timing for the selected DAC channel:
  *         refresh time
  * @rmtoll SHRR     TREFRESH1      LL_DAC_GetSampleAndHoldRefreshTime\n
  *         SHRR     TREFRESH2      LL_DAC_GetSampleAndHoldRefreshTime
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Value between Min_Data=0x00 and Max_Data=0xFF
  */
__STATIC_INLINE uint32_t LL_DAC_GetSampleAndHoldRefreshTime(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return (uint32_t)(READ_BIT(DACx->SHRR, DAC_SHRR_TREFRESH1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
                    >> (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                   );
}

/**
  * @}
  */

/** @defgroup DAC_LL_EF_DMA_Management DMA Management
  * @{
  */

/**
  * @brief  Enable DAC DMA transfer request of the selected channel.
  * @note   To configure DMA source address (peripheral address),
  *         use function @ref LL_DAC_DMA_GetRegAddr().
  * @rmtoll CR       DMAEN1         LL_DAC_EnableDMAReq\n
  *         CR       DMAEN2         LL_DAC_EnableDMAReq
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval None
  */
__STATIC_INLINE void LL_DAC_EnableDMAReq(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  SET_BIT(DACx->CR,
          DAC_CR_DMAEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Disable DAC DMA transfer request of the selected channel.
  * @note   To configure DMA source address (peripheral address),
  *         use function @ref LL_DAC_DMA_GetRegAddr().
  * @rmtoll CR       DMAEN1         LL_DAC_DisableDMAReq\n
  *         CR       DMAEN2         LL_DAC_DisableDMAReq
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval None
  */
__STATIC_INLINE void LL_DAC_DisableDMAReq(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  CLEAR_BIT(DACx->CR,
            DAC_CR_DMAEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get DAC DMA transfer request state of the selected channel.
  *         (0: DAC DMA transfer request is disabled, 1: DAC DMA transfer request is enabled)
  * @rmtoll CR       DMAEN1         LL_DAC_IsDMAReqEnabled\n
  *         CR       DMAEN2         LL_DAC_IsDMAReqEnabled
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsDMAReqEnabled(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return ((READ_BIT(DACx->CR,
                   DAC_CR_DMAEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
           == (DAC_CR_DMAEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))) ? 1UL : 0UL);
}

/**
  * @brief  Function to help to configure DMA transfer to DAC: retrieve the
  *         DAC register address from DAC instance and a list of DAC registers
  *         intended to be used (most commonly) with DMA transfer.
  * @note   These DAC registers are data holding registers:
  *         when DAC conversion is requested, DAC generates a DMA transfer
  *         request to have data available in DAC data holding registers.
  * @note   This macro is intended to be used with LL DMA driver, refer to
  *         function "LL_DMA_ConfigAddresses()".
  *         Example:
  *           LL_DMA_ConfigAddresses(DMA1,
  *                                  LL_DMA_CHANNEL_1,
  *                                  (uint32_t)&< array or variable >,
  *                                  LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED),
  *                                  LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  * @rmtoll DHR12R1  DACC1DHR       LL_DAC_DMA_GetRegAddr\n
  *         DHR12L1  DACC1DHR       LL_DAC_DMA_GetRegAddr\n
  *         DHR8R1   DACC1DHR       LL_DAC_DMA_GetRegAddr\n
  *         DHR12R2  DACC2DHR       LL_DAC_DMA_GetRegAddr\n
  *         DHR12L2  DACC2DHR       LL_DAC_DMA_GetRegAddr\n
  *         DHR8R2   DACC2DHR       LL_DAC_DMA_GetRegAddr
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  Register This parameter can be one of the following values:
  *         @arg @ref LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED
  *         @arg @ref LL_DAC_DMA_REG_DATA_12BITS_LEFT_ALIGNED
  *         @arg @ref LL_DAC_DMA_REG_DATA_8BITS_RIGHT_ALIGNED
  * @retval DAC register address
  */
__STATIC_INLINE uint32_t LL_DAC_DMA_GetRegAddr(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t Register)
{
  /* Retrieve address of register DHR12Rx, DHR12Lx or DHR8Rx depending on     */
  /* DAC channel selected.                                                    */
  return ((uint32_t)(__DAC_PTR_REG_OFFSET((DACx)->DHR12R1,
                                          ((DAC_Channel >> (Register & 0x1FUL)) & DAC_REG_DHR_REGOFFSET_MASK_POSBIT0))));
}
/**
  * @}
  */

/** @defgroup DAC_LL_EF_Operation Operation on DAC channels
  * @{
  */

/**
  * @brief  Enable DAC selected channel.
  * @rmtoll CR       EN1            LL_DAC_Enable\n
  *         CR       EN2            LL_DAC_Enable
  * @note   After enable from off state, DAC channel requires a delay
  *         for output voltage to reach accuracy +/- 1 LSB.
  *         Refer to device datasheet, parameter "tWAKEUP".
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval None
  */
__STATIC_INLINE void LL_DAC_Enable(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  SET_BIT(DACx->CR,
          DAC_CR_EN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Disable DAC selected channel.
  * @rmtoll CR       EN1            LL_DAC_Disable\n
  *         CR       EN2            LL_DAC_Disable
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval None
  */
__STATIC_INLINE void LL_DAC_Disable(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  CLEAR_BIT(DACx->CR,
            DAC_CR_EN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get DAC enable state of the selected channel.
  *         (0: DAC channel is disabled, 1: DAC channel is enabled)
  * @rmtoll CR       EN1            LL_DAC_IsEnabled\n
  *         CR       EN2            LL_DAC_IsEnabled
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsEnabled(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return ((READ_BIT(DACx->CR,
                   DAC_CR_EN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
           == (DAC_CR_EN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))) ? 1UL : 0UL);
}

/**
  * @brief  Enable DAC trigger of the selected channel.
  * @note   - If DAC trigger is disabled, DAC conversion is performed
  *           automatically once the data holding register is updated,
  *           using functions "LL_DAC_ConvertData{8; 12}{Right; Left} Aligned()":
  *           @ref LL_DAC_ConvertData12RightAligned(), ...
  *         - If DAC trigger is enabled, DAC conversion is performed
  *           only when a hardware of software trigger event is occurring.
  *           Select trigger source using
  *           function @ref LL_DAC_SetTriggerSource().
  * @rmtoll CR       TEN1           LL_DAC_EnableTrigger\n
  *         CR       TEN2           LL_DAC_EnableTrigger
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval None
  */
__STATIC_INLINE void LL_DAC_EnableTrigger(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  SET_BIT(DACx->CR,
          DAC_CR_TEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Disable DAC trigger of the selected channel.
  * @rmtoll CR       TEN1           LL_DAC_DisableTrigger\n
  *         CR       TEN2           LL_DAC_DisableTrigger
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval None
  */
__STATIC_INLINE void LL_DAC_DisableTrigger(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  CLEAR_BIT(DACx->CR,
            DAC_CR_TEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK));
}

/**
  * @brief  Get DAC trigger state of the selected channel.
  *         (0: DAC trigger is disabled, 1: DAC trigger is enabled)
  * @rmtoll CR       TEN1           LL_DAC_IsTriggerEnabled\n
  *         CR       TEN2           LL_DAC_IsTriggerEnabled
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsTriggerEnabled(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  return ((READ_BIT(DACx->CR,
                   DAC_CR_TEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))
           == (DAC_CR_TEN1 << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK))) ? 1UL : 0UL);
}

/**
  * @brief  Trig DAC conversion by software for the selected DAC channel.
  * @note   Preliminarily, DAC trigger must be set to software trigger
  *         using function
  *           @ref LL_DAC_Init()
  *           @ref LL_DAC_SetTriggerSource()
  *         with parameter "LL_DAC_TRIGGER_SOFTWARE".
  *         and DAC trigger must be enabled using
  *         function @ref LL_DAC_EnableTrigger().
  * @note   For devices featuring DAC with 2 channels: this function
  *         can perform a SW start of both DAC channels simultaneously.
  *         Two channels can be selected as parameter.
  *         Example: (LL_DAC_CHANNEL_1 | LL_DAC_CHANNEL_2)
  * @rmtoll SWTRIGR  SWTRIG1        LL_DAC_TrigSWConversion\n
  *         SWTRIGR  SWTRIG2        LL_DAC_TrigSWConversion
  * @param  DACx DAC instance
  * @param  DAC_Channel  This parameter can a combination of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval None
  */
__STATIC_INLINE void LL_DAC_TrigSWConversion(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  SET_BIT(DACx->SWTRIGR,
          (DAC_Channel & DAC_SWTR_CHX_MASK));
}

/**
  * @brief  Set the data to be loaded in the data holding register
  *         in format 12 bits left alignment (LSB aligned on bit 0),
  *         for the selected DAC channel.
  * @rmtoll DHR12R1  DACC1DHR       LL_DAC_ConvertData12RightAligned\n
  *         DHR12R2  DACC2DHR       LL_DAC_ConvertData12RightAligned
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  Data Value between Min_Data=0x000 and Max_Data=0xFFF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ConvertData12RightAligned(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t Data)
{
  __IO uint32_t *preg = __DAC_PTR_REG_OFFSET(DACx->DHR12R1, (DAC_Channel >> DAC_REG_DHR12RX_REGOFFSET_BITOFFSET_POS) & DAC_REG_DHR_REGOFFSET_MASK_POSBIT0);

  MODIFY_REG(*preg,
             DAC_DHR12R1_DACC1DHR,
             Data);
}

/**
  * @brief  Set the data to be loaded in the data holding register
  *         in format 12 bits left alignment (MSB aligned on bit 15),
  *         for the selected DAC channel.
  * @rmtoll DHR12L1  DACC1DHR       LL_DAC_ConvertData12LeftAligned\n
  *         DHR12L2  DACC2DHR       LL_DAC_ConvertData12LeftAligned
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  Data Value between Min_Data=0x000 and Max_Data=0xFFF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ConvertData12LeftAligned(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t Data)
{
  __IO uint32_t *preg = __DAC_PTR_REG_OFFSET(DACx->DHR12R1, (DAC_Channel >> DAC_REG_DHR12LX_REGOFFSET_BITOFFSET_POS) & DAC_REG_DHR_REGOFFSET_MASK_POSBIT0);

  MODIFY_REG(*preg,
             DAC_DHR12L1_DACC1DHR,
             Data);
}

/**
  * @brief  Set the data to be loaded in the data holding register
  *         in format 8 bits left alignment (LSB aligned on bit 0),
  *         for the selected DAC channel.
  * @rmtoll DHR8R1   DACC1DHR       LL_DAC_ConvertData8RightAligned\n
  *         DHR8R2   DACC2DHR       LL_DAC_ConvertData8RightAligned
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @param  Data Value between Min_Data=0x00 and Max_Data=0xFF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ConvertData8RightAligned(DAC_TypeDef *DACx, uint32_t DAC_Channel, uint32_t Data)
{
  __IO uint32_t *preg = __DAC_PTR_REG_OFFSET(DACx->DHR12R1, (DAC_Channel >> DAC_REG_DHR8RX_REGOFFSET_BITOFFSET_POS) & DAC_REG_DHR_REGOFFSET_MASK_POSBIT0);

  MODIFY_REG(*preg,
             DAC_DHR8R1_DACC1DHR,
             Data);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Set the data to be loaded in the data holding register
  *         in format 12 bits left alignment (LSB aligned on bit 0),
  *         for both DAC channels.
  * @rmtoll DHR12RD  DACC1DHR       LL_DAC_ConvertDualData12RightAligned\n
  *         DHR12RD  DACC2DHR       LL_DAC_ConvertDualData12RightAligned
  * @param  DACx DAC instance
  * @param  DataChannel1 Value between Min_Data=0x000 and Max_Data=0xFFF
  * @param  DataChannel2 Value between Min_Data=0x000 and Max_Data=0xFFF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ConvertDualData12RightAligned(DAC_TypeDef *DACx, uint32_t DataChannel1,
                                                          uint32_t DataChannel2)
{
  MODIFY_REG(DACx->DHR12RD,
             (DAC_DHR12RD_DACC2DHR | DAC_DHR12RD_DACC1DHR),
             ((DataChannel2 << DAC_DHR12RD_DACC2DHR_BITOFFSET_POS) | DataChannel1));
}

/**
  * @brief  Set the data to be loaded in the data holding register
  *         in format 12 bits left alignment (MSB aligned on bit 15),
  *         for both DAC channels.
  * @rmtoll DHR12LD  DACC1DHR       LL_DAC_ConvertDualData12LeftAligned\n
  *         DHR12LD  DACC2DHR       LL_DAC_ConvertDualData12LeftAligned
  * @param  DACx DAC instance
  * @param  DataChannel1 Value between Min_Data=0x000 and Max_Data=0xFFF
  * @param  DataChannel2 Value between Min_Data=0x000 and Max_Data=0xFFF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ConvertDualData12LeftAligned(DAC_TypeDef *DACx, uint32_t DataChannel1, uint32_t DataChannel2)
{
  /* Note: Data of DAC channel 2 shift value subtracted of 4 because          */
  /*       data on 16 bits and DAC channel 2 bits field is on the 12 MSB,     */
  /*       the 4 LSB must be taken into account for the shift value.          */
  MODIFY_REG(DACx->DHR12LD,
             (DAC_DHR12LD_DACC2DHR | DAC_DHR12LD_DACC1DHR),
             ((DataChannel2 << (DAC_DHR12LD_DACC2DHR_BITOFFSET_POS - 4U)) | DataChannel1));
}

/**
  * @brief  Set the data to be loaded in the data holding register
  *         in format 8 bits left alignment (LSB aligned on bit 0),
  *         for both DAC channels.
  * @rmtoll DHR8RD  DACC1DHR       LL_DAC_ConvertDualData8RightAligned\n
  *         DHR8RD  DACC2DHR       LL_DAC_ConvertDualData8RightAligned
  * @param  DACx DAC instance
  * @param  DataChannel1 Value between Min_Data=0x00 and Max_Data=0xFF
  * @param  DataChannel2 Value between Min_Data=0x00 and Max_Data=0xFF
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ConvertDualData8RightAligned(DAC_TypeDef *DACx, uint32_t DataChannel1, uint32_t DataChannel2)
{
  MODIFY_REG(DACx->DHR8RD,
             (DAC_DHR8RD_DACC2DHR | DAC_DHR8RD_DACC1DHR),
             ((DataChannel2 << DAC_DHR8RD_DACC2DHR_BITOFFSET_POS) | DataChannel1));
}

#endif /* DAC_CHANNEL2_SUPPORT */
/**
  * @brief  Retrieve output data currently generated for the selected DAC channel.
  * @note   Whatever alignment and resolution settings
  *         (using functions "LL_DAC_ConvertData{8; 12}{Right; Left} Aligned()":
  *         @ref LL_DAC_ConvertData12RightAligned(), ...),
  *         output data format is 12 bits right aligned (LSB aligned on bit 0).
  * @rmtoll DOR1     DACC1DOR       LL_DAC_RetrieveOutputData\n
  *         DOR2     DACC2DOR       LL_DAC_RetrieveOutputData
  * @param  DACx DAC instance
  * @param  DAC_Channel This parameter can be one of the following values:
  *         @arg @ref LL_DAC_CHANNEL_1
  *         @arg @ref LL_DAC_CHANNEL_2 (1)
  *
  *         (1) On this STM32 serie, parameter not available on all devices.
  *             Refer to device datasheet for channels availability.
  * @retval Value between Min_Data=0x000 and Max_Data=0xFFF
  */
__STATIC_INLINE uint32_t LL_DAC_RetrieveOutputData(DAC_TypeDef *DACx, uint32_t DAC_Channel)
{
  __IO uint32_t const *preg = __DAC_PTR_REG_OFFSET(DACx->DOR1, (DAC_Channel >> DAC_REG_DORX_REGOFFSET_BITOFFSET_POS) & DAC_REG_DORX_REGOFFSET_MASK_POSBIT0);

  return (uint16_t) READ_BIT(*preg, DAC_DOR1_DACC1DOR);
}

/**
  * @}
  */

/** @defgroup DAC_LL_EF_FLAG_Management FLAG Management
  * @{
  */
/**
  * @brief  Get DAC calibration offset flag for DAC channel 1
  * @rmtoll SR       CAL_FLAG1      LL_DAC_IsActiveFlag_CAL1
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsActiveFlag_CAL1(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->SR, LL_DAC_FLAG_CAL1) == (LL_DAC_FLAG_CAL1)) ? 1UL : 0UL);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Get DAC calibration offset flag for DAC channel 2
  * @rmtoll SR       CAL_FLAG2      LL_DAC_IsActiveFlag_CAL2
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsActiveFlag_CAL2(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->SR, LL_DAC_FLAG_CAL2) == (LL_DAC_FLAG_CAL2)) ? 1UL : 0UL);
}

#endif /* DAC_CHANNEL2_SUPPORT */
/**
  * @brief  Get DAC busy writing sample time flag for DAC channel 1
  * @rmtoll SR       BWST1          LL_DAC_IsActiveFlag_BWST1
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsActiveFlag_BWST1(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->SR, LL_DAC_FLAG_BWST1) == (LL_DAC_FLAG_BWST1)) ? 1UL : 0UL);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Get DAC busy writing sample time flag for DAC channel 2
  * @rmtoll SR       BWST2          LL_DAC_IsActiveFlag_BWST2
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsActiveFlag_BWST2(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->SR, LL_DAC_FLAG_BWST2) == (LL_DAC_FLAG_BWST2)) ? 1UL : 0UL);
}

#endif /* DAC_CHANNEL2_SUPPORT */
/**
  * @brief  Get DAC underrun flag for DAC channel 1
  * @rmtoll SR       DMAUDR1        LL_DAC_IsActiveFlag_DMAUDR1
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsActiveFlag_DMAUDR1(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->SR, LL_DAC_FLAG_DMAUDR1) == (LL_DAC_FLAG_DMAUDR1)) ? 1UL : 0UL);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Get DAC underrun flag for DAC channel 2
  * @rmtoll SR       DMAUDR2        LL_DAC_IsActiveFlag_DMAUDR2
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsActiveFlag_DMAUDR2(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->SR, LL_DAC_FLAG_DMAUDR2) == (LL_DAC_FLAG_DMAUDR2)) ? 1UL : 0UL);
}
#endif /* DAC_CHANNEL2_SUPPORT */

/**
  * @brief  Clear DAC underrun flag for DAC channel 1
  * @rmtoll SR       DMAUDR1        LL_DAC_ClearFlag_DMAUDR1
  * @param  DACx DAC instance
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ClearFlag_DMAUDR1(DAC_TypeDef *DACx)
{
  WRITE_REG(DACx->SR, LL_DAC_FLAG_DMAUDR1);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Clear DAC underrun flag for DAC channel 2
  * @rmtoll SR       DMAUDR2        LL_DAC_ClearFlag_DMAUDR2
  * @param  DACx DAC instance
  * @retval None
  */
__STATIC_INLINE void LL_DAC_ClearFlag_DMAUDR2(DAC_TypeDef *DACx)
{
  WRITE_REG(DACx->SR, LL_DAC_FLAG_DMAUDR2);
}
#endif /* DAC_CHANNEL2_SUPPORT */

/**
  * @}
  */

/** @defgroup DAC_LL_EF_IT_Management IT management
  * @{
  */

/**
  * @brief  Enable DMA underrun interrupt for DAC channel 1
  * @rmtoll CR       DMAUDRIE1      LL_DAC_EnableIT_DMAUDR1
  * @param  DACx DAC instance
  * @retval None
  */
__STATIC_INLINE void LL_DAC_EnableIT_DMAUDR1(DAC_TypeDef *DACx)
{
  SET_BIT(DACx->CR, LL_DAC_IT_DMAUDRIE1);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Enable DMA underrun interrupt for DAC channel 2
  * @rmtoll CR       DMAUDRIE2      LL_DAC_EnableIT_DMAUDR2
  * @param  DACx DAC instance
  * @retval None
  */
__STATIC_INLINE void LL_DAC_EnableIT_DMAUDR2(DAC_TypeDef *DACx)
{
  SET_BIT(DACx->CR, LL_DAC_IT_DMAUDRIE2);
}
#endif /* DAC_CHANNEL2_SUPPORT */

/**
  * @brief  Disable DMA underrun interrupt for DAC channel 1
  * @rmtoll CR       DMAUDRIE1      LL_DAC_DisableIT_DMAUDR1
  * @param  DACx DAC instance
  * @retval None
  */
__STATIC_INLINE void LL_DAC_DisableIT_DMAUDR1(DAC_TypeDef *DACx)
{
  CLEAR_BIT(DACx->CR, LL_DAC_IT_DMAUDRIE1);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Disable DMA underrun interrupt for DAC channel 2
  * @rmtoll CR       DMAUDRIE2      LL_DAC_DisableIT_DMAUDR2
  * @param  DACx DAC instance
  * @retval None
  */
__STATIC_INLINE void LL_DAC_DisableIT_DMAUDR2(DAC_TypeDef *DACx)
{
  CLEAR_BIT(DACx->CR, LL_DAC_IT_DMAUDRIE2);
}
#endif /* DAC_CHANNEL2_SUPPORT */

/**
  * @brief  Get DMA underrun interrupt for DAC channel 1
  * @rmtoll CR       DMAUDRIE1      LL_DAC_IsEnabledIT_DMAUDR1
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsEnabledIT_DMAUDR1(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->CR, LL_DAC_IT_DMAUDRIE1) == (LL_DAC_IT_DMAUDRIE1)) ? 1UL : 0UL);
}

#if defined(DAC_CHANNEL2_SUPPORT)
/**
  * @brief  Get DMA underrun interrupt for DAC channel 2
  * @rmtoll CR       DMAUDRIE2      LL_DAC_IsEnabledIT_DMAUDR2
  * @param  DACx DAC instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DAC_IsEnabledIT_DMAUDR2(DAC_TypeDef *DACx)
{
  return ((READ_BIT(DACx->CR, LL_DAC_IT_DMAUDRIE2) == (LL_DAC_IT_DMAUDRIE2)) ? 1UL : 0UL);
}
#endif /* DAC_CHANNEL2_SUPPORT */

/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup DAC_LL_EF_Init Initialization and de-initialization functions
  * @{
  */

ErrorStatus LL_DAC_DeInit(DAC_TypeDef *DACx);
ErrorStatus LL_DAC_Init(DAC_TypeDef *DACx, uint32_t DAC_Channel, LL_DAC_InitTypeDef *DAC_InitStruct);
void        LL_DAC_StructInit(LL_DAC_InitTypeDef *DAC_InitStruct);

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

#endif /* DAC1 */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_LL_DAC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
