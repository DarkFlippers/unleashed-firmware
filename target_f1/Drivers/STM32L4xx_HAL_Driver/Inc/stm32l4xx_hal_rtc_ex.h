/**
  ******************************************************************************
  * @file    stm32l4xx_hal_rtc_ex.h
  * @author  MCD Application Team
  * @brief   Header file of RTC HAL Extended module.
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
#ifndef STM32L4xx_HAL_RTC_EX_H
#define STM32L4xx_HAL_RTC_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup RTCEx RTCEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup RTCEx_Exported_Types RTCEx Exported Types
  * @{
  */

/** @defgroup RTCEx_Tamper_structure_definition RTCEx Tamper structure definition
  * @{
  */
typedef struct
{
  uint32_t Tamper;                      /*!< Specifies the Tamper Pin.
                                             This parameter can be a value of @ref RTCEx_Tamper_Pins_Definitions */

  uint32_t Interrupt;                   /*!< Specifies the Tamper Interrupt.
                                             This parameter can be a value of @ref  RTCEx_Tamper_Interrupt_Definitions */

  uint32_t Trigger;                     /*!< Specifies the Tamper Trigger.
                                             This parameter can be a value of @ref RTCEx_Tamper_Trigger_Definitions */

  uint32_t NoErase;                     /*!< Specifies the Tamper no erase mode.
                                             This parameter can be a value of @ref RTCEx_Tamper_EraseBackUp_Definitions */

  uint32_t MaskFlag;                    /*!< Specifies the Tamper Flag masking.
                                             This parameter can be a value of @ref RTCEx_Tamper_MaskFlag_Definitions   */

  uint32_t Filter;                      /*!< Specifies the TAMP Filter Tamper.
                                             This parameter can be a value of @ref RTCEx_Tamper_Filter_Definitions */

  uint32_t SamplingFrequency;           /*!< Specifies the sampling frequency.
                                             This parameter can be a value of @ref RTCEx_Tamper_Sampling_Frequencies_Definitions */

  uint32_t PrechargeDuration;           /*!< Specifies the Precharge Duration .
                                             This parameter can be a value of @ref RTCEx_Tamper_Pin_Precharge_Duration_Definitions */

  uint32_t TamperPullUp;                /*!< Specifies the Tamper PullUp .
                                             This parameter can be a value of @ref RTCEx_Tamper_Pull_UP_Definitions */

  uint32_t TimeStampOnTamperDetection;  /*!< Specifies the TimeStampOnTamperDetection.
                                             This parameter can be a value of @ref RTCEx_Tamper_TimeStampOnTamperDetection_Definitions */
} RTC_TamperTypeDef;
/**
  * @}
  */

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup RTCEx_Exported_Constants RTCEx Exported Constants
  * @{
  */

/* ========================================================================== */
/*                 ##### RTC TimeStamp exported constants #####               */
/* ========================================================================== */

/** @defgroup RTCEx_Time_Stamp_Edges_definitions RTCEx Time Stamp Edges Definitions
  *
  * @{
  */
#define RTC_TIMESTAMPEDGE_RISING        0x00000000u
#define RTC_TIMESTAMPEDGE_FALLING       RTC_CR_TSEDGE
/**
  * @}
  */

/** @defgroup RTCEx_TimeStamp_Pin_Selection RTCEx TimeStamp Pin Selection
  * @{
  */
#define RTC_TIMESTAMPPIN_DEFAULT              0x00000000u
/**
  * @}
  */

/* ========================================================================== */
/*                   ##### RTC Wake-up exported constants #####               */
/* ========================================================================== */

/** @defgroup RTCEx_Wakeup_Timer_Definitions RTCEx Wakeup Timer Definitions
  * @{
  */
#define RTC_WAKEUPCLOCK_RTCCLK_DIV16        0x00000000u
#define RTC_WAKEUPCLOCK_RTCCLK_DIV8         RTC_CR_WUCKSEL_0
#define RTC_WAKEUPCLOCK_RTCCLK_DIV4         RTC_CR_WUCKSEL_1
#define RTC_WAKEUPCLOCK_RTCCLK_DIV2         (RTC_CR_WUCKSEL_0 | RTC_CR_WUCKSEL_1)
#define RTC_WAKEUPCLOCK_CK_SPRE_16BITS      RTC_CR_WUCKSEL_2
#define RTC_WAKEUPCLOCK_CK_SPRE_17BITS      (RTC_CR_WUCKSEL_1 | RTC_CR_WUCKSEL_2)
/**
  * @}
  */

/* ========================================================================== */
/*        ##### Extended RTC Peripheral Control exported constants #####      */
/* ========================================================================== */

/** @defgroup RTCEx_Smooth_calib_period_Definitions RTCEx Smooth Calib Period Definitions
  * @{
  */
#define RTC_SMOOTHCALIB_PERIOD_32SEC   0x00000000u              /*!< If RTCCLK = 32768 Hz, Smooth calibration
                                                                     period is 32s,  else 2exp20 RTCCLK pulses */
#define RTC_SMOOTHCALIB_PERIOD_16SEC   RTC_CALR_CALW16          /*!< If RTCCLK = 32768 Hz, Smooth calibration
                                                                     period is 16s, else 2exp19 RTCCLK pulses */
#define RTC_SMOOTHCALIB_PERIOD_8SEC    RTC_CALR_CALW8           /*!< If RTCCLK = 32768 Hz, Smooth calibration
                                                                     period is 8s, else 2exp18 RTCCLK pulses */
/**
  * @}
  */

/** @defgroup RTCEx_Smooth_calib_Plus_pulses_Definitions RTCEx Smooth calib Plus pulses Definitions
  * @{
  */
#define RTC_SMOOTHCALIB_PLUSPULSES_SET    RTC_CALR_CALP         /*!< The number of RTCCLK pulses added
                                                                     during a X -second window = Y - CALM[8:0]
                                                                     with Y = 512, 256, 128 when X = 32, 16, 8 */
#define RTC_SMOOTHCALIB_PLUSPULSES_RESET  0x00000000u           /*!< The number of RTCCLK pulses subbstited
                                                                     during a 32-second window = CALM[8:0] */
/**
  * @}
  */

#if defined(STM32L412xx) || defined(STM32L422xx)
/** @defgroup RTCEx_Smooth_Calib_Low_Power_Definitions RTCEx Smooth Calib Low Power Definitions
  * @{
  */
#define RTC_LPCAL_SET                     RTC_CALR_LPCAL        /*!< Calibration window is 220 ck_apre,
                                                                     which is the required configuration for
                                                                     ultra-low consumption mode. */
#define RTC_LPCAL_RESET                   0x00000000u           /*!< Calibration window is 220 RTCCLK,
                                                                     which is a high-consumption mode.
                                                                     This mode should be set only when less
                                                                     than 32s calibration window is required. */
/**
  * @}
  */
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */

/** @defgroup RTCEx_Calib_Output_selection_Definitions RTCEx Calib Output Selection Definitions
  * @{
  */
#define RTC_CALIBOUTPUT_512HZ            0x00000000u
#define RTC_CALIBOUTPUT_1HZ              RTC_CR_COSEL
/**
  * @}
  */

/** @defgroup RTCEx_Add_1_Second_Parameter_Definitions RTC Add 1 Second Parameter Definitions
  * @{
  */
#define RTC_SHIFTADD1S_RESET      0x00000000u
#define RTC_SHIFTADD1S_SET        RTC_SHIFTR_ADD1S
/**
  * @}
  */


/* ========================================================================== */
/*                    ##### RTC Tamper exported constants #####               */
/* ========================================================================== */

/** @defgroup RTCEx_Tamper_Pins_Definitions RTCEx Tamper Pins Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#if defined(RTC_TAMPER1_SUPPORT)
#define RTC_TAMPER_1                        TAMP_CR1_TAMP1E
#endif /* RTC_TAMPER1_SUPPORT */
#define RTC_TAMPER_2                        TAMP_CR1_TAMP2E
#if defined(RTC_TAMPER3_SUPPORT)
#define RTC_TAMPER_3                        TAMP_CR1_TAMP3E
#endif /* RTC_TAMPER3_SUPPORT */
#define RTC_TAMPER_ALL                      (TAMP_CR1_TAMP1E | TAMP_CR1_TAMP2E)
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */
#if defined(RTC_TAMPER1_SUPPORT)
#define RTC_TAMPER_1                        RTC_TAMPCR_TAMP1E
#endif /* RTC_TAMPER1_SUPPORT */
#define RTC_TAMPER_2                        RTC_TAMPCR_TAMP2E
#if defined(RTC_TAMPER3_SUPPORT)
#define RTC_TAMPER_3                        RTC_TAMPCR_TAMP3E
#endif /* RTC_TAMPER3_SUPPORT */
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */
/**
  * @}
  */

/** @defgroup RTCEx_Tamper_Trigger_Definitions RTCEx Tamper Triggers Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPERTRIGGER_RISINGEDGE        0x00u  /*!< Warning : Filter must be RTC_TAMPERFILTER_DISABLE */
#define RTC_TAMPERTRIGGER_FALLINGEDGE       0x01u  /*!< Warning : Filter must be RTC_TAMPERFILTER_DISABLE */
#define RTC_TAMPERTRIGGER_LOWLEVEL          0x02u  /*!< Warning : Filter must not be RTC_TAMPERFILTER_DISABLE */
#define RTC_TAMPERTRIGGER_HIGHLEVEL         0x03u  /*!< Warning : Filter must not be RTC_TAMPERFILTER_DISABLE */
#else
#define RTC_TAMPERTRIGGER_RISINGEDGE        ((uint32_t)0x00000000)
#define RTC_TAMPERTRIGGER_FALLINGEDGE       ((uint32_t)0x00000002)
#define RTC_TAMPERTRIGGER_LOWLEVEL          RTC_TAMPERTRIGGER_RISINGEDGE
#define RTC_TAMPERTRIGGER_HIGHLEVEL         RTC_TAMPERTRIGGER_FALLINGEDGE
#endif
/**
  * @}
  */

/** @defgroup RTCEx_Tamper_MaskFlag_Definitions RTCEx Tamper Mask Flag Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPERMASK_FLAG_DISABLE         0x00u
#define RTC_TAMPERMASK_FLAG_ENABLE          0x01u
#else
#define RTC_TAMPERMASK_FLAG_DISABLE         0x00000000u
#define RTC_TAMPERMASK_FLAG_ENABLE          0x00040000u
#endif
/**
  * @}
  */

/** @defgroup RTCEx_Tamper_EraseBackUp_Definitions RTCEx Tamper EraseBackUp Definitions
* @{
*/
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPER_ERASE_BACKUP_ENABLE      0x00u
#define RTC_TAMPER_ERASE_BACKUP_DISABLE     0x01u
#else
#define RTC_TAMPER_ERASE_BACKUP_ENABLE      0x00000000u
#define RTC_TAMPER_ERASE_BACKUP_DISABLE     0x00020000u
#endif
/**
  * @}
  */

/** @defgroup RTCEx_Tamper_Filter_Definitions RTCEx Tamper Filter Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPERFILTER_DISABLE           0x00000000U             /*!< Tamper filter is disabled */

#define RTC_TAMPERFILTER_2SAMPLE           TAMP_FLTCR_TAMPFLT_0    /*!< Tamper is activated after 2
                                                                         consecutive samples at the active level */
#define RTC_TAMPERFILTER_4SAMPLE           TAMP_FLTCR_TAMPFLT_1    /*!< Tamper is activated after 4
                                                                         consecutive samples at the active level */
#define RTC_TAMPERFILTER_8SAMPLE           TAMP_FLTCR_TAMPFLT      /*!< Tamper is activated after 8
                                                                         consecutive samples at the active level */
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */
#define RTC_TAMPERFILTER_DISABLE           0x00000000u             /*!< Tamper filter is disabled */

#define RTC_TAMPERFILTER_2SAMPLE           RTC_TAMPCR_TAMPFLT_0    /*!< Tamper is activated after 2
                                                                        consecutive samples at the active level */
#define RTC_TAMPERFILTER_4SAMPLE           RTC_TAMPCR_TAMPFLT_1    /*!< Tamper is activated after 4
                                                                         consecutive samples at the active level */
#define RTC_TAMPERFILTER_8SAMPLE           RTC_TAMPCR_TAMPFLT      /*!< Tamper is activated after 8
                                                                         consecutive samples at the active level. */
#endif /*#if defined(STM32L412xx) || defined(STM32L422xx) */

/**
  * @}
  */

/** @defgroup RTCEx_Tamper_Sampling_Frequencies_Definitions RTCEx Tamper Sampling Frequencies Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV32768  0x00000000U                                     /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 32768 */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV16384  TAMP_FLTCR_TAMPFREQ_0                           /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 16384 */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV8192   TAMP_FLTCR_TAMPFREQ_1                           /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 8192  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV4096   (TAMP_FLTCR_TAMPFREQ_0 | TAMP_FLTCR_TAMPFREQ_1) /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 4096  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV2048   TAMP_FLTCR_TAMPFREQ_2                           /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 2048  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV1024   (TAMP_FLTCR_TAMPFREQ_0 | TAMP_FLTCR_TAMPFREQ_2) /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 1024  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV512    (TAMP_FLTCR_TAMPFREQ_1 | TAMP_FLTCR_TAMPFREQ_2) /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 512   */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV256    TAMP_FLTCR_TAMPFREQ                             /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 256   */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_MASK      TAMP_FLTCR_TAMPFREQ                             /*!< Masking all bits except those of
                                                                                                      field TAMPFREQ[2:0]*/
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */

#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV32768  0x00000000u                                     /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 32768 */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV16384  RTC_TAMPCR_TAMPFREQ_0                           /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 16384 */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV8192   RTC_TAMPCR_TAMPFREQ_1                           /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 8192  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV4096   (RTC_TAMPCR_TAMPFREQ_0 | RTC_TAMPCR_TAMPFREQ_1) /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 4096  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV2048   RTC_TAMPCR_TAMPFREQ_2                           /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 2048  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV1024   (RTC_TAMPCR_TAMPFREQ_0 | RTC_TAMPCR_TAMPFREQ_2)  /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 1024  */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV512    (RTC_TAMPCR_TAMPFREQ_1 | RTC_TAMPCR_TAMPFREQ_2)  /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 512   */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV256    RTC_TAMPCR_TAMPFREQ                              /*!< Each of the tamper inputs are sampled
                                                                                                      with a frequency =  RTCCLK / 256   */
#define RTC_TAMPERSAMPLINGFREQ_RTCCLK_MASK      RTC_TAMPCR_TAMPFREQ                             /*!< Masking all bits except those of
                                                                                                      field TAMPFREQ[2:0]*/
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */
/**
  * @}
  */

/** @defgroup RTCEx_Tamper_Pin_Precharge_Duration_Definitions RTCEx Tamper Pin Precharge Duration Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPERPRECHARGEDURATION_1RTCCLK     0x00000000U                                       /*!< Tamper pins are pre-charged before
                                                                                                        sampling during 1 RTCCLK cycle  */
#define RTC_TAMPERPRECHARGEDURATION_2RTCCLK     TAMP_FLTCR_TAMPPRCH_0                             /*!< Tamper pins are pre-charged before
                                                                                                        sampling during 2 RTCCLK cycles */
#define RTC_TAMPERPRECHARGEDURATION_4RTCCLK     TAMP_FLTCR_TAMPPRCH_1                             /*!< Tamper pins are pre-charged before
                                                                                                        sampling during 4 RTCCLK cycles */
#define RTC_TAMPERPRECHARGEDURATION_8RTCCLK     TAMP_FLTCR_TAMPPRCH     /*!< Tamper pins are pre-charged before
                                                                         sampling during 8 RTCCLK cycles */
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */

#define RTC_TAMPERPRECHARGEDURATION_1RTCCLK     0x00000000u             /*!< Tamper pins are pre-charged before
                                                                              sampling during 1 RTCCLK cycle  */
#define RTC_TAMPERPRECHARGEDURATION_2RTCCLK     RTC_TAMPCR_TAMPPRCH_0   /*!< Tamper pins are pre-charged before
                                                                              sampling during 2 RTCCLK cycles */
#define RTC_TAMPERPRECHARGEDURATION_4RTCCLK     RTC_TAMPCR_TAMPPRCH_1   /*!< Tamper pins are pre-charged before
                                                                              sampling during 4 RTCCLK cycles */
#define RTC_TAMPERPRECHARGEDURATION_8RTCCLK     RTC_TAMPCR_TAMPPRCH     /*!< Tamper pins are pre-charged before
                                                                              sampling during 8 RTCCLK cycles */
#define RTC_TAMPERPRECHARGEDURATION_MASK        RTC_TAMPCR_TAMPPRCH     /*!< Masking all bits except those of
                                                                              field TAMPPRCH[1:0] */
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */
/**
  * @}
  */

/** @defgroup RTCEx_Tamper_Pull_UP_Definitions RTCEx Tamper Pull Up Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPER_PULLUP_ENABLE           0x00000000u           /*!< Tamper pins are pre-charged before sampling */
#define RTC_TAMPER_PULLUP_DISABLE          TAMP_FLTCR_TAMPPUDIS  /*!< Tamper pins pre-charge is disabled          */
#else
#define RTC_TAMPER_PULLUP_ENABLE           0x00000000u           /*!< TimeStamp on Tamper Detection event saved        */
#define RTC_TAMPER_PULLUP_DISABLE          RTC_TAMPCR_TAMPPUDIS  /*!< TimeStamp on Tamper Detection event is not saved */
#endif

/**
  * @}
  */

/** @defgroup RTCEx_Tamper_TimeStampOnTamperDetection_Definitions RTCEx Tamper TimeStamp On Tamper Detection Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TIMESTAMPONTAMPERDETECTION_DISABLE  0x00000000u    /*!< TimeStamp on Tamper Detection event is not saved */
#define RTC_TIMESTAMPONTAMPERDETECTION_ENABLE   RTC_CR_TAMPTS  /*!< TimeStamp on Tamper Detection event saved        */
#else
#define RTC_TIMESTAMPONTAMPERDETECTION_DISABLE  0x00000000u        /*!< TimeStamp on Tamper Detection event is not saved */
#define RTC_TIMESTAMPONTAMPERDETECTION_ENABLE   RTC_TAMPCR_TAMPTS  /*!< TimeStamp on Tamper Detection event saved        */
#endif
/**
  * @}
  */

/** @defgroup RTCEx_Tamper_Interrupt_Definitions RTC Tamper Interrupts Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_IT_TAMP                         (TAMP_IER_TAMP1IE | TAMP_IER_TAMP2IE)  /*!< Enable all Tamper Interrupt */
#define RTC_IT_TAMP1                        TAMP_IER_TAMP1IE   /*!< Tamper 1 Interrupt */
#define RTC_IT_TAMP2                        TAMP_IER_TAMP2IE   /*!< Tamper 2 Interrupt */
#define RTC_IT_TAMPALL                      RTC_IT_TAMP
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */
#define RTC_IT_TAMP                         RTC_TAMPCR_TAMPIE  /*!< Enable all Tamper Interrupt  */
#define RTC_IT_TAMP1                        RTC_TAMPCR_TAMP1IE /*!< Enable Tamper 1 Interrupt     */
#define RTC_IT_TAMP2                        RTC_TAMPCR_TAMP2IE /*!< Enable Tamper 2 Interrupt     */
#define RTC_IT_TAMP3                        RTC_TAMPCR_TAMP3IE /*!< Enable Tamper 3 Interrupt     */
#define RTC_IT_TAMPALL                      RTC_IT_TAMP
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */
/**
  * @}
  */

/** @defgroup RTCEx_Flags  RTCEx Flags
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_FLAG_TAMP1F                     TAMP_SR_TAMP1F
#define RTC_FLAG_TAMP2F                     TAMP_SR_TAMP2F
#define RTC_FLAG_TAMPALL                   (RTC_FLAG_TAMP1F | RTC_FLAG_TAMP2F)
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */
#define RTC_FLAG_TAMP1F                     RTC_ISR_TAMP1F
#define RTC_FLAG_TAMP2F                     RTC_ISR_TAMP2F
#define RTC_FLAG_TAMP3F                     RTC_ISR_TAMP3F
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */
/**
  * @}
  */

/* ========================================================================== */
/*         ##### Extended RTC Backup registers exported constants #####       */
/* ========================================================================== */

/** @defgroup RTCEx_Backup_Data_Registers_Number_Definitions RTC Backup Data Registers Number Definitions
  * @{
  */
#if defined(RTC_BKP_NUMBER)
#define BKP_REG_NUMBER                       RTC_BKP_NUMBER
#endif /* RTC_BKP_NUMBER */
#if defined(TAMP_BKP_NUMBER)
#define BKP_REG_NUMBER                       TAMP_BKP_NUMBER
#endif /* TAMP_BKP_NUMBER */
/**
  * @}
  */

/** @defgroup RTCEx_Backup_Data_Registers_Definitions RTCEx Backup Data Registers Definitions
  * @{
  */
#define RTC_BKP_DR0                       0x00u
#define RTC_BKP_DR1                       0x01u
#define RTC_BKP_DR2                       0x02u
#define RTC_BKP_DR3                       0x03u
#define RTC_BKP_DR4                       0x04u
#define RTC_BKP_DR5                       0x05u
#define RTC_BKP_DR6                       0x06u
#define RTC_BKP_DR7                       0x07u
#define RTC_BKP_DR8                       0x08u
#define RTC_BKP_DR9                       0x09u
#define RTC_BKP_DR10                      0x0Au
#define RTC_BKP_DR11                      0x0Bu
#define RTC_BKP_DR12                      0x0Cu
#define RTC_BKP_DR13                      0x0Du
#define RTC_BKP_DR14                      0x0Eu
#define RTC_BKP_DR15                      0x0Fu
#define RTC_BKP_DR16                      0x10u
#define RTC_BKP_DR17                      0x11u
#define RTC_BKP_DR18                      0x12u
#define RTC_BKP_DR19                      0x13u
#define RTC_BKP_DR20                      0x14u
#define RTC_BKP_DR21                      0x15u
#define RTC_BKP_DR22                      0x16u
#define RTC_BKP_DR23                      0x17u
#define RTC_BKP_DR24                      0x18u
#define RTC_BKP_DR25                      0x19u
#define RTC_BKP_DR26                      0x1Au
#define RTC_BKP_DR27                      0x1Bu
#define RTC_BKP_DR28                      0x1Cu
#define RTC_BKP_DR29                      0x1Du
#define RTC_BKP_DR30                      0x1Eu
#define RTC_BKP_DR31                      0x1Fu
/**
  * @}
  */




/** @defgroup RTCEx_Tamper_Interrupt_Definitions RTC Tamper Interrupts Definitions
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define RTC_TAMPER1_INTERRUPT              TAMP_IER_TAMP1IE
#define RTC_TAMPER2_INTERRUPT              TAMP_IER_TAMP2IE
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */
#if defined(RTC_TAMPER1_SUPPORT)
#define RTC_TAMPER1_INTERRUPT              RTC_TAMPCR_TAMP1IE
#endif /* RTC_TAMPER1_SUPPORT */
#define RTC_TAMPER2_INTERRUPT              RTC_TAMPCR_TAMP2IE
#if defined(RTC_TAMPER3_SUPPORT)
#define RTC_TAMPER3_INTERRUPT              RTC_TAMPCR_TAMP3IE
#endif /* RTC_TAMPER3_SUPPORT */
#define RTC_ALL_TAMPER_INTERRUPT           RTC_TAMPCR_TAMPIE
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */


/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup RTCEx_Exported_Macros RTCEx Exported Macros
  * @{
  */

#if defined(STM32L412xx) || defined(STM32L422xx)
/** @brief  Clear the specified RTC pending flag.
  * @param  __HANDLE__ specifies the RTC Handle.
  * @param  __FLAG__ specifies the flag to check.
  *          This parameter can be any combination of the following values:
  *            @arg @ref RTC_CLEAR_ITSF               Clear Internal Time-stamp flag
  *            @arg @ref RTC_CLEAR_TSOVF              Clear Time-stamp overflow flag
  *            @arg @ref RTC_CLEAR_TSF                Clear Time-stamp flag
  *            @arg @ref RTC_CLEAR_WUTF               Clear Wakeup timer flag
  *            @arg @ref RTC_CLEAR_ALRBF              Clear Alarm B flag
  *            @arg @ref RTC_CLEAR_ALRAF              Clear Alarm A flag
  * @retval None
  */
#define __HAL_RTC_CLEAR_FLAG(__HANDLE__, __FLAG__)   ((__HANDLE__)->Instance->SCR = (__FLAG__))

/** @brief  Check whether the specified RTC flag is set or not.
  * @param  __HANDLE__ specifies the RTC Handle.
  * @param  __FLAG__ specifies the flag to check.
  *          This parameter can be any combination of the following values:
  *            @arg @ref RTC_FLAG_RECALPF             Recalibration pending Flag
  *            @arg @ref RTC_FLAG_INITF               Initialization flag
  *            @arg @ref RTC_FLAG_RSF                 Registers synchronization flag
  *            @arg @ref RTC_FLAG_INITS               Initialization status flag
  *            @arg @ref RTC_FLAG_SHPF                Shift operation pending flag
  *            @arg @ref RTC_FLAG_WUTWF               Wakeup timer write flag
  *            @arg @ref RTC_FLAG_ALRBWF              Alarm B write flag
  *            @arg @ref RTC_FLAG_ALRAWF              Alarm A write flag
  *            @arg @ref RTC_FLAG_ITSF                Internal Time-stamp flag
  *            @arg @ref RTC_FLAG_TSOVF               Time-stamp overflow flag
  *            @arg @ref RTC_FLAG_TSF                 Time-stamp flag
  *            @arg @ref RTC_FLAG_WUTF                Wakeup timer flag
  *            @arg @ref RTC_FLAG_ALRBF               Alarm B flag
  *            @arg @ref RTC_FLAG_ALRAF               Alarm A flag
  * @retval None
  */
#define __HAL_RTC_GET_FLAG(__HANDLE__, __FLAG__)    (((((__FLAG__)) >> 8U) == 1U) ? ((__HANDLE__)->Instance->ICSR & (1U << (((uint16_t)(__FLAG__)) & RTC_FLAG_MASK))) : \
                                                     ((__HANDLE__)->Instance->SR & (1U << (((uint16_t)(__FLAG__)) & RTC_FLAG_MASK))))
#endif /*#if defined(STM32L412xx) || defined(STM32L422xx) */

/* ---------------------------------WAKEUPTIMER---------------------------------*/
/** @defgroup RTCEx_WakeUp_Timer RTC WakeUp Timer
  * @{
  */
/**
  * @brief  Enable the RTC WakeUp Timer peripheral.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_ENABLE(__HANDLE__)                      ((__HANDLE__)->Instance->CR |= (RTC_CR_WUTE))

/**
  * @brief  Disable the RTC WakeUp Timer peripheral.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_DISABLE(__HANDLE__)                     ((__HANDLE__)->Instance->CR &= ~(RTC_CR_WUTE))

/**
  * @brief  Enable the RTC WakeUpTimer interrupt.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC WakeUpTimer interrupt sources to be enabled.
  *         This parameter can be:
  *            @arg @ref RTC_IT_WUT WakeUpTimer interrupt
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_ENABLE_IT(__HANDLE__, __INTERRUPT__)    ((__HANDLE__)->Instance->CR |= (__INTERRUPT__))

/**
  * @brief  Disable the RTC WakeUpTimer interrupt.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC WakeUpTimer interrupt sources to be disabled.
  *         This parameter can be:
  *            @arg @ref RTC_IT_WUT WakeUpTimer interrupt
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_DISABLE_IT(__HANDLE__, __INTERRUPT__)   ((__HANDLE__)->Instance->CR &= ~(__INTERRUPT__))


/**
  * @brief  Check whether the specified RTC WakeUpTimer interrupt has occurred or not.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC WakeUpTimer interrupt sources to check.
  *         This parameter can be:
  *            @arg @ref RTC_IT_WUT  WakeUpTimer interrupt
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_WAKEUPTIMER_GET_IT(__HANDLE__, __INTERRUPT__)       (((((__HANDLE__)->Instance->MISR) & ((__INTERRUPT__) >> 12)) != 0U) ? 1U : 0U)
#else
#define __HAL_RTC_WAKEUPTIMER_GET_IT(__HANDLE__, __INTERRUPT__)       (((((__HANDLE__)->Instance->ISR) & ((__INTERRUPT__) >> 4)) != 0U) ? 1U : 0U)
#endif

/**
  * @brief  Check whether the specified RTC Wake Up timer interrupt has been enabled or not.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC Wake Up timer interrupt sources to check.
  *         This parameter can be:
  *            @arg @ref RTC_IT_WUT  WakeUpTimer interrupt
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)   (((((__HANDLE__)->Instance->CR) & (__INTERRUPT__)) != 0U) ? 1U : 0U)

/**
  * @brief  Get the selected RTC WakeUpTimer's flag status.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC WakeUpTimer Flag is pending or not.
  *          This parameter can be:
  *             @arg @ref RTC_FLAG_WUTF
  *             @arg @ref RTC_FLAG_WUTWF
  * @retval Flag status
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_WAKEUPTIMER_GET_FLAG(__HANDLE__, __FLAG__)   (__HAL_RTC_GET_FLAG((__HANDLE__), (__FLAG__)))
#else
#define __HAL_RTC_WAKEUPTIMER_GET_FLAG(__HANDLE__, __FLAG__)   (((((__HANDLE__)->Instance->ISR) & (__FLAG__)) != 0U) ? 1U : 0U)
#endif

/**
  * @brief  Clear the RTC Wake Up timer's pending flags.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC WakeUpTimer Flag to clear.
  *         This parameter can be:
  *            @arg @ref RTC_FLAG_WUTF
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(__HANDLE__, __FLAG__)     (__HAL_RTC_CLEAR_FLAG((__HANDLE__), RTC_CLEAR_WUTF))
#else
#define __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->Instance->ISR) = (~((__FLAG__) | RTC_ISR_INIT)|((__HANDLE__)->Instance->ISR & RTC_ISR_INIT))
#endif


/* WAKE-UP TIMER EXTI */
/* ------------------ */
/**
  * @brief  Enable interrupt on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_IT()       (EXTI->IMR1 |= RTC_EXTI_LINE_WAKEUPTIMER_EVENT)

/**
  * @brief  Disable interrupt on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_DISABLE_IT()      (EXTI->IMR1 &= ~(RTC_EXTI_LINE_WAKEUPTIMER_EVENT))

/**
  * @brief  Enable event on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_EVENT()    (EXTI->EMR1 |= RTC_EXTI_LINE_WAKEUPTIMER_EVENT)

/**
  * @brief  Disable event on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_DISABLE_EVENT()   (EXTI->EMR1 &= ~(RTC_EXTI_LINE_WAKEUPTIMER_EVENT))

/**
  * @brief  Enable falling edge trigger on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_FALLING_EDGE()   (EXTI->FTSR1 |= RTC_EXTI_LINE_WAKEUPTIMER_EVENT)

/**
  * @brief  Disable falling edge trigger on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_DISABLE_FALLING_EDGE()  (EXTI->FTSR1 &= ~(RTC_EXTI_LINE_WAKEUPTIMER_EVENT))

/**
  * @brief  Enable rising edge trigger on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_RISING_EDGE()    (EXTI->RTSR1 |= RTC_EXTI_LINE_WAKEUPTIMER_EVENT)

/**
  * @brief  Disable rising edge trigger on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_DISABLE_RISING_EDGE()   (EXTI->RTSR1 &= ~(RTC_EXTI_LINE_WAKEUPTIMER_EVENT))

/**
  * @brief  Enable rising & falling edge trigger on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_RISING_FALLING_EDGE()  do { \
                                                                   __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_RISING_EDGE();  \
                                                                   __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_FALLING_EDGE(); \
                                                                 } while(0)

/**
  * @brief  Disable rising & falling edge trigger on the RTC WakeUp Timer associated Exti line.
  * This parameter can be:
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_DISABLE_RISING_FALLING_EDGE()  do { \
                                                                   __HAL_RTC_WAKEUPTIMER_EXTI_DISABLE_RISING_EDGE();  \
                                                                   __HAL_RTC_WAKEUPTIMER_EXTI_DISABLE_FALLING_EDGE(); \
                                                                  } while(0)

/**
  * @brief Check whether the RTC WakeUp Timer associated Exti line interrupt flag is set or not.
  * @retval Line Status.
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_GET_FLAG()              (EXTI->PR1 & RTC_EXTI_LINE_WAKEUPTIMER_EVENT)

/**
  * @brief Clear the RTC WakeUp Timer associated Exti line flag.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG()            (EXTI->PR1 = RTC_EXTI_LINE_WAKEUPTIMER_EVENT)

/**
  * @brief Generate a Software interrupt on the RTC WakeUp Timer associated Exti line.
  * @retval None
  */
#define __HAL_RTC_WAKEUPTIMER_EXTI_GENERATE_SWIT()         (EXTI->SWIER1 |= RTC_EXTI_LINE_WAKEUPTIMER_EVENT)

/**
  * @}
  */

/* ---------------------------------TIMESTAMP---------------------------------*/
/** @defgroup RTCEx_Timestamp RTC Timestamp
  * @{
  */
/**
  * @brief  Enable the RTC TimeStamp peripheral.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TIMESTAMP_ENABLE(__HANDLE__)                       ((__HANDLE__)->Instance->CR |= (RTC_CR_TSE))

/**
  * @brief  Disable the RTC TimeStamp peripheral.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TIMESTAMP_DISABLE(__HANDLE__)                      ((__HANDLE__)->Instance->CR &= ~(RTC_CR_TSE))

/**
  * @brief  Enable the RTC TimeStamp interrupt.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC TimeStamp interrupt source to be enabled.
  *         This parameter can be:
  *            @arg @ref RTC_IT_TS TimeStamp interrupt
  * @retval None
  */
#define __HAL_RTC_TIMESTAMP_ENABLE_IT(__HANDLE__, __INTERRUPT__)     ((__HANDLE__)->Instance->CR |= (__INTERRUPT__))

/**
  * @brief  Disable the RTC TimeStamp interrupt.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC TimeStamp interrupt source to be disabled.
  *         This parameter can be:
  *            @arg @ref RTC_IT_TS TimeStamp interrupt
  * @retval None
  */
#define __HAL_RTC_TIMESTAMP_DISABLE_IT(__HANDLE__, __INTERRUPT__)    ((__HANDLE__)->Instance->CR &= ~(__INTERRUPT__))

/**
  * @brief  Check whether the specified RTC TimeStamp interrupt has occurred or not.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC TimeStamp interrupt source to check.
  *         This parameter can be:
  *            @arg @ref RTC_IT_TS TimeStamp interrupt
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TIMESTAMP_GET_IT(__HANDLE__, __INTERRUPT__)        (((((__HANDLE__)->Instance->MISR) & ((__INTERRUPT__) >> 12)) != 0U) ? 1U : 0U)
#else
#define __HAL_RTC_TIMESTAMP_GET_IT(__HANDLE__, __INTERRUPT__)        (((((__HANDLE__)->Instance->ISR) & ((__INTERRUPT__) >> 4)) != 0U) ? 1U : 0U)
#endif
/**
  * @brief  Check whether the specified RTC Time Stamp interrupt has been enabled or not.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC Time Stamp interrupt source to check.
  *         This parameter can be:
  *            @arg @ref RTC_IT_TS TimeStamp interrupt
  * @retval None
  */
#define __HAL_RTC_TIMESTAMP_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)     (((((__HANDLE__)->Instance->CR) & (__INTERRUPT__)) != 0U) ? 1U : 0U)

/**
  * @brief  Get the selected RTC TimeStamp's flag status.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC TimeStamp Flag is pending or not.
  *         This parameter can be:
  *            @arg @ref RTC_FLAG_TSF
  *            @arg @ref RTC_FLAG_TSOVF
  * @retval Flag status
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TIMESTAMP_GET_FLAG(__HANDLE__, __FLAG__)     (__HAL_RTC_GET_FLAG((__HANDLE__),(__FLAG__)))
#else
#define __HAL_RTC_TIMESTAMP_GET_FLAG(__HANDLE__, __FLAG__)     (((((__HANDLE__)->Instance->ISR) & (__FLAG__)) != 0U) ? 1U : 0U)
#endif

/**
  * @brief  Clear the RTC Time Stamp's pending flags.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC TimeStamp Flag to clear.
  *          This parameter can be:
  *             @arg @ref RTC_FLAG_TSF
  *             @arg @ref RTC_FLAG_TSOVF
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TIMESTAMP_CLEAR_FLAG(__HANDLE__, __FLAG__)   (__HAL_RTC_CLEAR_FLAG((__HANDLE__), (__FLAG__)))
#else
#define __HAL_RTC_TIMESTAMP_CLEAR_FLAG(__HANDLE__, __FLAG__)   ((__HANDLE__)->Instance->ISR) = (~((__FLAG__) | RTC_ISR_INIT)|((__HANDLE__)->Instance->ISR & RTC_ISR_INIT))
#endif

/**
  * @brief  Enable the RTC internal TimeStamp peripheral.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_INTERNAL_TIMESTAMP_ENABLE(__HANDLE__)                ((__HANDLE__)->Instance->CR |= (RTC_CR_ITSE))

/**
  * @brief  Disable the RTC internal TimeStamp peripheral.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_INTERNAL_TIMESTAMP_DISABLE(__HANDLE__)               ((__HANDLE__)->Instance->CR &= ~(RTC_CR_ITSE))

/**
  * @brief  Get the selected RTC Internal Time Stamp's flag status.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC Internal Time Stamp Flag is pending or not.
  *         This parameter can be:
  *            @arg @ref RTC_FLAG_ITSF
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_INTERNAL_TIMESTAMP_GET_FLAG(__HANDLE__, __FLAG__)     (__HAL_RTC_GET_FLAG((__HANDLE__),(__FLAG__)))
#else
#define __HAL_RTC_INTERNAL_TIMESTAMP_GET_FLAG(__HANDLE__, __FLAG__)    (((((__HANDLE__)->Instance->ISR) & (__FLAG__)) != 0U) ? 1U : 0U)
#endif

/**
  * @brief  Clear the RTC Internal Time Stamp's pending flags.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC Internal Time Stamp Flag source to clear.
  * This parameter can be:
  *             @arg @ref RTC_FLAG_ITSF
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_INTERNAL_TIMESTAMP_CLEAR_FLAG(__HANDLE__, __FLAG__)     (__HAL_RTC_CLEAR_FLAG((__HANDLE__), RTC_CLEAR_ITSF))
#else
#define __HAL_RTC_INTERNAL_TIMESTAMP_CLEAR_FLAG(__HANDLE__, __FLAG__)  ((__HANDLE__)->Instance->ISR) = (~((__FLAG__) | RTC_ISR_INIT)|((__HANDLE__)->Instance->ISR & RTC_ISR_INIT))
#endif


#if defined(STM32L412xx) || defined(STM32L422xx)
/**
  * @brief  Enable the RTC TimeStamp on Tamper detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TAMPTS_ENABLE(__HANDLE__)                       ((__HANDLE__)->Instance->CR |= (RTC_CR_TAMPTS))

/**
  * @brief  Disable the RTC TimeStamp on Tamper detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TAMPTS_DISABLE(__HANDLE__)                      ((__HANDLE__)->Instance->CR &= ~(RTC_CR_TAMPTS))

/**
  * @brief  Enable the RTC Tamper detection output.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TAMPOE_ENABLE(__HANDLE__)                       ((__HANDLE__)->Instance->CR |= (RTC_CR_TAMPOE))

/**
  * @brief  Disable the RTC Tamper detection output.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TAMPOE_DISABLE(__HANDLE__)                      ((__HANDLE__)->Instance->CR &= ~(RTC_CR_TAMPOE))


/**
  * @}
  */
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */

/* ------------------------------Calibration----------------------------------*/
/** @defgroup RTCEx_Calibration RTC Calibration
  * @{
  */

/**
  * @brief  Enable the RTC calibration output.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_CALIBRATION_OUTPUT_ENABLE(__HANDLE__)               ((__HANDLE__)->Instance->CR |= (RTC_CR_COE))

/**
  * @brief  Disable the calibration output.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_CALIBRATION_OUTPUT_DISABLE(__HANDLE__)              ((__HANDLE__)->Instance->CR &= ~(RTC_CR_COE))

/**
  * @brief  Enable the clock reference detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_CLOCKREF_DETECTION_ENABLE(__HANDLE__)               ((__HANDLE__)->Instance->CR |= (RTC_CR_REFCKON))

/**
  * @brief  Disable the clock reference detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_CLOCKREF_DETECTION_DISABLE(__HANDLE__)              ((__HANDLE__)->Instance->CR &= ~(RTC_CR_REFCKON))

/**
  * @brief  Get the selected RTC shift operation's flag status.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC shift operation Flag is pending or not.
  *          This parameter can be:
  *             @arg @ref RTC_FLAG_SHPF
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_SHIFT_GET_FLAG(__HANDLE__, __FLAG__)                (__HAL_RTC_GET_FLAG((__HANDLE__), (__FLAG__)))
#else
#define __HAL_RTC_SHIFT_GET_FLAG(__HANDLE__, __FLAG__)         (((((__HANDLE__)->Instance->ISR) & (__FLAG__)) != 0U) ? 1U : 0U)
#endif

/**
  * @}
  */


/* ------------------------------Tamper----------------------------------*/
/** @defgroup RTCEx_Tamper RTCEx tamper
  * @{
  */
#if defined(RTC_TAMPER1_SUPPORT)
/**
  * @brief  Enable the RTC Tamper1 input detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER1_ENABLE(__HANDLE__)           (((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->CR1 |= (TAMP_CR1_TAMP1E))
#else
#define __HAL_RTC_TAMPER1_ENABLE(__HANDLE__)           ((__HANDLE__)->Instance->TAMPCR |= (RTC_TAMPCR_TAMP1E))
#endif

/**
  * @brief  Disable the RTC Tamper1 input detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER1_DISABLE(__HANDLE__)          (((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->CR1 &= ~(RTC_TAMPCR_TAMP1E))
#else
#define __HAL_RTC_TAMPER1_DISABLE(__HANDLE__)          ((__HANDLE__)->Instance->TAMPCR &= ~(RTC_TAMPCR_TAMP1E))
#endif
#endif /* RTC_TAMPER1_SUPPORT */

/**
  * @brief  Enable the RTC Tamper2 input detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER2_ENABLE(__HANDLE__)           (((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->CR1 |= (TAMP_CR1_TAMP2E))
#else
#define __HAL_RTC_TAMPER2_ENABLE(__HANDLE__)           ((__HANDLE__)->Instance->TAMPCR |= (RTC_TAMPCR_TAMP2E))
#endif

/**
  * @brief  Disable the RTC Tamper2 input detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER2_DISABLE(__HANDLE__)          (((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->CR1 &= ~(RTC_TAMPCR_TAMP2E))
#else
#define __HAL_RTC_TAMPER2_DISABLE(__HANDLE__)          ((__HANDLE__)->Instance->TAMPCR &= ~(RTC_TAMPCR_TAMP2E))
#endif

#if defined(RTC_TAMPER3_SUPPORT)
/**
  * @brief  Enable the RTC Tamper3 input detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TAMPER3_ENABLE(__HANDLE__)                         ((__HANDLE__)->Instance->TAMPCR |= (RTC_TAMPCR_TAMP3E))

/**
  * @brief  Disable the RTC Tamper3 input detection.
  * @param  __HANDLE__ specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_TAMPER3_DISABLE(__HANDLE__)                        ((__HANDLE__)->Instance->TAMPCR &= ~(RTC_TAMPCR_TAMP3E))
#endif /* RTC_TAMPER3_SUPPORT */

/**************************************************************************************************/
/**
  * @brief  Enable the TAMP Tamper interrupt.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC Tamper interrupt sources to be enabled.
  *          This parameter can be any combination of the following values:
  *             @arg  RTC_IT_TAMPALL: All tampers interrupts
  *             @arg  RTC_IT_TAMP1: Tamper1 interrupt
  *             @arg  RTC_IT_TAMP2: Tamper2 interrupt
  *             @arg  RTC_IT_TAMP3: Tamper3 interrupt
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER_ENABLE_IT(__HANDLE__, __INTERRUPT__)        (((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->IER |= (__INTERRUPT__))
#else
#define __HAL_RTC_TAMPER_ENABLE_IT(__HANDLE__, __INTERRUPT__)        ((__HANDLE__)->Instance->TAMPCR |= (__INTERRUPT__))
#endif
/**
  * @brief  Disable the TAMP Tamper interrupt.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC Tamper interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *            @arg  RTC_IT_TAMPALL: All tampers interrupts
  *            @arg  RTC_IT_TAMP1: Tamper1 interrupt
  *            @arg  RTC_IT_TAMP2: Tamper2 interrupt
  *            @arg  RTC_IT_TAMP3: Tamper3 interrupt
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER_DISABLE_IT(__HANDLE__, __INTERRUPT__)       (((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->IER &= ~(__INTERRUPT__))
#else
#define __HAL_RTC_TAMPER_DISABLE_IT(__HANDLE__, __INTERRUPT__)       ((__HANDLE__)->Instance->TAMPCR &= ~(__INTERRUPT__))
#endif


/**************************************************************************************************/
/**
  * @brief  Check whether the specified RTC Tamper interrupt has occurred or not.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC Tamper interrupt to check.
  *         This parameter can be:
  *            @arg  RTC_IT_TAMPALL: All tampers interrupts
  *            @arg  RTC_IT_TAMP1: Tamper1 interrupt
  *            @arg  RTC_IT_TAMP2: Tamper2 interrupt
  *            @arg  RTC_IT_TAMP3: Tamper3 interrupt
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER_GET_IT(__HANDLE__, __INTERRUPT__)     ((((((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->MISR) & (__INTERRUPT__)) != 0U) ? 1U : 0U)
#else /* #if defined(STM32L412xx) || defined(STM32L422xx) */
#define __HAL_RTC_TAMPER_GET_IT(__HANDLE__, __INTERRUPT__)     (((((__HANDLE__)->Instance->ISR) & (__INTERRUPT__)) != 0U) ? 1U : 0U)
#endif /* #if defined(STM32L412xx) || defined(STM32L422xx) */
/**
  * @brief  Check whether the specified RTC Tamper interrupt has been enabled or not.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __INTERRUPT__ specifies the RTC Tamper interrupt source to check.
  *         This parameter can be:
  *            @arg  RTC_IT_TAMPALL: All tampers interrupts
  *            @arg  RTC_IT_TAMP1: Tamper1 interrupt
  *            @arg  RTC_IT_TAMP2: Tamper2 interrupt
  *            @arg  RTC_IT_TAMP3: Tamper3 interrupt
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)    ((((((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->IER) & (__INTERRUPT__)) != 0U) ? 1U : 0U)
#else
#define __HAL_RTC_TAMPER_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)    (((((__HANDLE__)->Instance->TAMPCR) & (__INTERRUPT__)) != 0U) ? 1U : 0U)
#endif

/**
  * @brief  Get the selected RTC Tamper's flag status.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC Tamper Flag is pending or not.
  *          This parameter can be:
  *             @arg RTC_FLAG_TAMP1F: Tamper1 flag
  *             @arg RTC_FLAG_TAMP2F: Tamper2 flag
  *             @arg RTC_FLAG_TAMP3F: Tamper3 flag
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER_GET_FLAG(__HANDLE__, __FLAG__)        (((((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->SR) & (__FLAG__)) != 0U)
#else
#define __HAL_RTC_TAMPER_GET_FLAG(__HANDLE__, __FLAG__)        (((((__HANDLE__)->Instance->ISR) & (__FLAG__)) != 0U) ? 1U : 0U)
#endif
/**
  * @brief  Clear the RTC Tamper's pending flags.
  * @param  __HANDLE__ specifies the RTC handle.
  * @param  __FLAG__ specifies the RTC Tamper Flag to clear.
  *          This parameter can be:
  *             @arg RTC_FLAG_TAMP1F: Tamper1 flag
  *             @arg RTC_FLAG_TAMP2F: Tamper2 flag
  *             @arg RTC_FLAG_TAMP3F: Tamper3 flag
  * @retval None
  */
#if defined(STM32L412xx) || defined(STM32L422xx)
#define __HAL_RTC_TAMPER_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((((TAMP_TypeDef *)((uint32_t)((__HANDLE__)->Instance) + (__HANDLE__)->TampOffset))->SCR) = (__FLAG__))
#else
#define __HAL_RTC_TAMPER_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->ISR) = (~((__FLAG__) | RTC_ISR_INIT)|((__HANDLE__)->Instance->ISR & RTC_ISR_INIT))
#endif

/**
  * @brief  Enable interrupt on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_IT()        (EXTI->IMR1 |= RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT)

/**
  * @brief  Disable interrupt on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_DISABLE_IT()       (EXTI->IMR1 &= ~(RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT))

/**
  * @brief  Enable event on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_EVENT()    (EXTI->EMR1 |= RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT)

/**
  * @brief  Disable event on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_DISABLE_EVENT()   (EXTI->EMR1 &= ~(RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT))

/**
  * @brief  Enable falling edge trigger on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_FALLING_EDGE()   (EXTI->FTSR1 |= RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT)

/**
  * @brief  Disable falling edge trigger on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_DISABLE_FALLING_EDGE()  (EXTI->FTSR1 &= ~(RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT))

/**
  * @brief  Enable rising edge trigger on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_RISING_EDGE()    (EXTI->RTSR1 |= RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT)

/**
  * @brief  Disable rising edge trigger on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_DISABLE_RISING_EDGE()   (EXTI->RTSR1 &= ~(RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT))

/**
  * @brief  Enable rising & falling edge trigger on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_RISING_FALLING_EDGE()  do { \
                                                                        __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_RISING_EDGE();  \
                                                                        __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_FALLING_EDGE(); \
                                                                      } while(0)

/**
  * @brief  Disable rising & falling edge trigger on the RTC Tamper and Timestamp associated Exti line.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_DISABLE_RISING_FALLING_EDGE()  do { \
                                                                        __HAL_RTC_TAMPER_TIMESTAMP_EXTI_DISABLE_RISING_EDGE();  \
                                                                        __HAL_RTC_TAMPER_TIMESTAMP_EXTI_DISABLE_FALLING_EDGE(); \
                                                                       } while(0)

/**
  * @brief Check whether the RTC Tamper and Timestamp associated Exti line interrupt flag is set or not.
  * @retval Line Status.
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_GET_FLAG()         (EXTI->PR1 & RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT)

/**
  * @brief Clear the RTC Tamper and Timestamp associated Exti line flag.
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_CLEAR_FLAG()       (EXTI->PR1 = RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT)

/**
  * @brief Generate a Software interrupt on the RTC Tamper and Timestamp associated Exti line
  * @retval None
  */
#define __HAL_RTC_TAMPER_TIMESTAMP_EXTI_GENERATE_SWIT()    (EXTI->SWIER1 |= RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT)

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup RTCEx_Exported_Functions RTCEx Exported Functions
  * @{
  */

/* ========================================================================== */
/*                  ##### RTC TimeStamp exported functions #####              */
/* ========================================================================== */

/* RTC TimeStamp functions ****************************************************/

/** @defgroup RTCEx_Exported_Functions_Group1 Extended RTC TimeStamp functions
  * @{
  */
HAL_StatusTypeDef HAL_RTCEx_SetTimeStamp(RTC_HandleTypeDef *hrtc, uint32_t TimeStampEdge, uint32_t RTC_TimeStampPin);
HAL_StatusTypeDef HAL_RTCEx_SetTimeStamp_IT(RTC_HandleTypeDef *hrtc, uint32_t TimeStampEdge, uint32_t RTC_TimeStampPin);
HAL_StatusTypeDef HAL_RTCEx_DeactivateTimeStamp(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_SetInternalTimeStamp(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_DeactivateInternalTimeStamp(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_GetTimeStamp(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTimeStamp, RTC_DateTypeDef *sTimeStampDate, uint32_t Format);
void              HAL_RTCEx_TamperTimeStampIRQHandler(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_TimeStampEventCallback(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_PollForTimeStampEvent(RTC_HandleTypeDef *hrtc, uint32_t Timeout);
/**
  * @}
  */

/* ========================================================================== */
/*                   ##### RTC Wake-up exported functions #####               */
/* ========================================================================== */

/* RTC Wake-up functions ******************************************************/

/** @defgroup RTCEx_Exported_Functions_Group2 Extended RTC Wake-up functions
 * @{
 */
HAL_StatusTypeDef HAL_RTCEx_SetWakeUpTimer(RTC_HandleTypeDef *hrtc, uint32_t WakeUpCounter, uint32_t WakeUpClock);
#if defined(STM32L412xx) || defined(STM32L422xx)
HAL_StatusTypeDef HAL_RTCEx_SetWakeUpTimer_IT(RTC_HandleTypeDef *hrtc, uint32_t WakeUpCounter, uint32_t WakeUpClock, uint32_t WakeUpAutoClr);
#else
HAL_StatusTypeDef HAL_RTCEx_SetWakeUpTimer_IT(RTC_HandleTypeDef *hrtc, uint32_t WakeUpCounter, uint32_t WakeUpClock);
#endif
HAL_StatusTypeDef HAL_RTCEx_DeactivateWakeUpTimer(RTC_HandleTypeDef *hrtc);
uint32_t          HAL_RTCEx_GetWakeUpTimer(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_WakeUpTimerIRQHandler(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_PollForWakeUpTimerEvent(RTC_HandleTypeDef *hrtc, uint32_t Timeout);
/**
  * @}
  */

/* ========================================================================== */
/*        ##### Extended RTC Peripheral Control exported functions #####      */
/* ========================================================================== */

/* Extended RTC Peripheral Control functions **********************************/

/** @defgroup RTCEx_Exported_Functions_Group3 Extended Peripheral Control functions
 * @{
 */
HAL_StatusTypeDef HAL_RTCEx_SetSmoothCalib(RTC_HandleTypeDef *hrtc, uint32_t SmoothCalibPeriod, uint32_t SmoothCalibPlusPulses, uint32_t SmoothCalibMinusPulsesValue);
#if defined(STM32L412xx) || defined(STM32L422xx)
HAL_StatusTypeDef HAL_RTCEx_SetLowPowerCalib(RTC_HandleTypeDef *hrtc, uint32_t LowPowerCalib);
#endif
HAL_StatusTypeDef HAL_RTCEx_SetSynchroShift(RTC_HandleTypeDef *hrtc, uint32_t ShiftAdd1S, uint32_t ShiftSubFS);
HAL_StatusTypeDef HAL_RTCEx_SetCalibrationOutPut(RTC_HandleTypeDef *hrtc, uint32_t CalibOutput);
HAL_StatusTypeDef HAL_RTCEx_DeactivateCalibrationOutPut(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_SetRefClock(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_DeactivateRefClock(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_EnableBypassShadow(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_DisableBypassShadow(RTC_HandleTypeDef *hrtc);
/**
  * @}
  */

/* Extended RTC features functions *******************************************/
/** @defgroup RTCEx_Exported_Functions_Group4 Extended features functions
  * @{
  */

void              HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_PollForAlarmBEvent(RTC_HandleTypeDef *hrtc, uint32_t Timeout);
/**
  * @}
  */

/** @defgroup RTCEx_Exported_Functions_Group5 Extended RTC Tamper functions
  * @{
  */
HAL_StatusTypeDef HAL_RTCEx_SetTamper(RTC_HandleTypeDef *hrtc, RTC_TamperTypeDef *sTamper);
HAL_StatusTypeDef HAL_RTCEx_SetTamper_IT(RTC_HandleTypeDef *hrtc, RTC_TamperTypeDef *sTamper);
HAL_StatusTypeDef HAL_RTCEx_DeactivateTamper(RTC_HandleTypeDef *hrtc, uint32_t Tamper);

#if defined(RTC_TAMPER1_SUPPORT)
HAL_StatusTypeDef HAL_RTCEx_PollForTamper1Event(RTC_HandleTypeDef *hrtc, uint32_t Timeout);
#endif /* RTC_TAMPER1_SUPPORT */
HAL_StatusTypeDef HAL_RTCEx_PollForTamper2Event(RTC_HandleTypeDef *hrtc, uint32_t Timeout);
#if defined(RTC_TAMPER3_SUPPORT)
HAL_StatusTypeDef HAL_RTCEx_PollForTamper3Event(RTC_HandleTypeDef *hrtc, uint32_t Timeout);
#endif /* RTC_TAMPER3_SUPPORT */

#if defined(RTC_TAMPER1_SUPPORT)
void              HAL_RTCEx_Tamper1EventCallback(RTC_HandleTypeDef *hrtc);
#endif /* RTC_TAMPER1_SUPPORT */
void              HAL_RTCEx_Tamper2EventCallback(RTC_HandleTypeDef *hrtc);
#if defined(RTC_TAMPER3_SUPPORT)
void              HAL_RTCEx_Tamper3EventCallback(RTC_HandleTypeDef *hrtc);
#endif /* RTC_TAMPER3_SUPPORT */


/**
  * @}
  */

/** @defgroup RTCEx_Exported_Functions_Group6 Extended RTC Backup register functions
 * @{
 */
void              HAL_RTCEx_BKUPWrite(RTC_HandleTypeDef *hrtc, uint32_t BackupRegister, uint32_t Data);
uint32_t          HAL_RTCEx_BKUPRead(RTC_HandleTypeDef *hrtc, uint32_t BackupRegister);
/**
  * @}
  */

/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup RTCEx_Private_Constants RTCEx Private Constants
  * @{
  */
#define RTC_EXTI_LINE_TAMPER_TIMESTAMP_EVENT  EXTI_IMR1_IM19  /*!< External interrupt line 19 Connected to the RTC Tamper and Time Stamp events */
#define RTC_EXTI_LINE_WAKEUPTIMER_EVENT       EXTI_IMR1_IM20  /*!< External interrupt line 20 Connected to the RTC Wakeup event */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup RTCEx_Private_Macros RTCEx Private Macros
  * @{
  */

/** @defgroup RTCEx_IS_RTC_Definitions Private macros to check input parameters
  * @{
  */
#define IS_TIMESTAMP_EDGE(EDGE) (((EDGE) == RTC_TIMESTAMPEDGE_RISING) || \
                                 ((EDGE) == RTC_TIMESTAMPEDGE_FALLING))

#define IS_RTC_TAMPER_INTERRUPT(INTERRUPT) ((((INTERRUPT) & (uint32_t)0xFFB6FFFB) == 0x00) && ((INTERRUPT) != 0U))

#define IS_RTC_TIMESTAMP_PIN(PIN)  (((PIN) == RTC_TIMESTAMPPIN_DEFAULT))

#define IS_RTC_WAKEUP_CLOCK(CLOCK) (((CLOCK) == RTC_WAKEUPCLOCK_RTCCLK_DIV16)   || \
                                    ((CLOCK) == RTC_WAKEUPCLOCK_RTCCLK_DIV8)    || \
                                    ((CLOCK) == RTC_WAKEUPCLOCK_RTCCLK_DIV4)    || \
                                    ((CLOCK) == RTC_WAKEUPCLOCK_RTCCLK_DIV2)    || \
                                    ((CLOCK) == RTC_WAKEUPCLOCK_CK_SPRE_16BITS) || \
                                    ((CLOCK) == RTC_WAKEUPCLOCK_CK_SPRE_17BITS))

#define IS_RTC_WAKEUP_COUNTER(COUNTER)  ((COUNTER) <= RTC_WUTR_WUT)

#define IS_RTC_SMOOTH_CALIB_PERIOD(PERIOD) (((PERIOD) == RTC_SMOOTHCALIB_PERIOD_32SEC) || \
                                            ((PERIOD) == RTC_SMOOTHCALIB_PERIOD_16SEC) || \
                                            ((PERIOD) == RTC_SMOOTHCALIB_PERIOD_8SEC))

#define IS_RTC_SMOOTH_CALIB_PLUS(PLUS) (((PLUS) == RTC_SMOOTHCALIB_PLUSPULSES_SET) || \
                                        ((PLUS) == RTC_SMOOTHCALIB_PLUSPULSES_RESET))

#define IS_RTC_SMOOTH_CALIB_MINUS(VALUE) ((VALUE) <= RTC_CALR_CALM)

#if defined(STM32L412xx) || defined(STM32L422xx)
#define IS_RTC_LOW_POWER_CALIB(LPCAL) (((LPCAL) == RTC_LPCAL_SET) || \
                                       ((LPCAL) == RTC_LPCAL_RESET))
#endif

#if defined(STM32L412xx) || defined(STM32L422xx)
#define IS_RTC_TAMPER(__TAMPER__)                ((((__TAMPER__) & RTC_TAMPER_ALL) != 0x00U) && \
                                                  (((__TAMPER__) & ~RTC_TAMPER_ALL) == 0x00U))
#else
#define IS_RTC_TAMPER(TAMPER) ((((TAMPER) & (uint32_t)0xFFFFFFD6) == 0x00) && ((TAMPER) != 0U))
#endif


#define IS_RTC_TAMPER_TRIGGER(__TRIGGER__)       (((__TRIGGER__) == RTC_TAMPERTRIGGER_RISINGEDGE)  || \
                                                  ((__TRIGGER__) == RTC_TAMPERTRIGGER_FALLINGEDGE) || \
                                                  ((__TRIGGER__) == RTC_TAMPERTRIGGER_LOWLEVEL)    || \
                                                  ((__TRIGGER__) == RTC_TAMPERTRIGGER_HIGHLEVEL))

#define IS_RTC_TAMPER_ERASE_MODE(__MODE__)       (((__MODE__) == RTC_TAMPER_ERASE_BACKUP_ENABLE) || \
                                                  ((__MODE__) == RTC_TAMPER_ERASE_BACKUP_DISABLE))

#define IS_RTC_TAMPER_MASKFLAG_STATE(__STATE__)  (((__STATE__) == RTC_TAMPERMASK_FLAG_ENABLE) || \
                                                  ((__STATE__) == RTC_TAMPERMASK_FLAG_DISABLE))

#define IS_RTC_TAMPER_FILTER(__FILTER__)         (((__FILTER__) == RTC_TAMPERFILTER_DISABLE)  || \
                                                  ((__FILTER__) == RTC_TAMPERFILTER_2SAMPLE) || \
                                                  ((__FILTER__) == RTC_TAMPERFILTER_4SAMPLE) || \
                                                  ((__FILTER__) == RTC_TAMPERFILTER_8SAMPLE))

#define IS_RTC_TAMPER_SAMPLING_FREQ(__FREQ__)    (((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV32768)|| \
                                                  ((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV16384)|| \
                                                  ((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV8192) || \
                                                  ((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV4096) || \
                                                  ((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV2048) || \
                                                  ((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV1024) || \
                                                  ((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV512)  || \
                                                  ((__FREQ__) == RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV256))

#define IS_RTC_TAMPER_PRECHARGE_DURATION(__DURATION__)   (((__DURATION__) == RTC_TAMPERPRECHARGEDURATION_1RTCCLK) || \
                                                          ((__DURATION__) == RTC_TAMPERPRECHARGEDURATION_2RTCCLK) || \
                                                          ((__DURATION__) == RTC_TAMPERPRECHARGEDURATION_4RTCCLK) || \
                                                          ((__DURATION__) == RTC_TAMPERPRECHARGEDURATION_8RTCCLK))

#define IS_RTC_TAMPER_PULLUP_STATE(__STATE__)    (((__STATE__) == RTC_TAMPER_PULLUP_ENABLE) || \
                                                  ((__STATE__) == RTC_TAMPER_PULLUP_DISABLE))

#define IS_RTC_TAMPER_TIMESTAMPONTAMPER_DETECTION(DETECTION) (((DETECTION) == RTC_TIMESTAMPONTAMPERDETECTION_ENABLE) || \
                                                              ((DETECTION) == RTC_TIMESTAMPONTAMPERDETECTION_DISABLE))

#define IS_RTC_BKP(__BKP__)   ((__BKP__) < RTC_BKP_NUMBER)

#define IS_RTC_SHIFT_ADD1S(SEL) (((SEL) == RTC_SHIFTADD1S_RESET) || \
                                 ((SEL) == RTC_SHIFTADD1S_SET))

#define IS_RTC_SHIFT_SUBFS(FS) ((FS) <= RTC_SHIFTR_SUBFS)

#define IS_RTC_CALIB_OUTPUT(OUTPUT)  (((OUTPUT) == RTC_CALIBOUTPUT_512HZ) || \
                                      ((OUTPUT) == RTC_CALIBOUTPUT_1HZ))

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
#endif /* __cplusplus */

#endif /* STM32L4xx_HAL_RTC_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
