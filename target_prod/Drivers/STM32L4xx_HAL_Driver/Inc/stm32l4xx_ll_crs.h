/**
  ******************************************************************************
  * @file    stm32l4xx_ll_crs.h
  * @author  MCD Application Team
  * @brief   Header file of CRS LL module.
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
#ifndef __STM32L4xx_LL_CRS_H
#define __STM32L4xx_LL_CRS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(CRS)

/** @defgroup CRS_LL CRS
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup CRS_LL_Exported_Constants CRS Exported Constants
  * @{
  */

/** @defgroup CRS_LL_EC_GET_FLAG Get Flags Defines
  * @brief    Flags defines which can be used with LL_CRS_ReadReg function
  * @{
  */
#define LL_CRS_ISR_SYNCOKF                 CRS_ISR_SYNCOKF
#define LL_CRS_ISR_SYNCWARNF               CRS_ISR_SYNCWARNF
#define LL_CRS_ISR_ERRF                    CRS_ISR_ERRF
#define LL_CRS_ISR_ESYNCF                  CRS_ISR_ESYNCF
#define LL_CRS_ISR_SYNCERR                 CRS_ISR_SYNCERR
#define LL_CRS_ISR_SYNCMISS                CRS_ISR_SYNCMISS
#define LL_CRS_ISR_TRIMOVF                 CRS_ISR_TRIMOVF
/**
  * @}
  */

/** @defgroup CRS_LL_EC_IT IT Defines
  * @brief    IT defines which can be used with LL_CRS_ReadReg and  LL_CRS_WriteReg functions
  * @{
  */
#define LL_CRS_CR_SYNCOKIE                 CRS_CR_SYNCOKIE
#define LL_CRS_CR_SYNCWARNIE               CRS_CR_SYNCWARNIE
#define LL_CRS_CR_ERRIE                    CRS_CR_ERRIE
#define LL_CRS_CR_ESYNCIE                  CRS_CR_ESYNCIE
/**
  * @}
  */

/** @defgroup CRS_LL_EC_SYNC_DIV Synchronization Signal Divider
  * @{
  */
#define LL_CRS_SYNC_DIV_1                  ((uint32_t)0x00U)                         /*!< Synchro Signal not divided (default) */
#define LL_CRS_SYNC_DIV_2                  CRS_CFGR_SYNCDIV_0                        /*!< Synchro Signal divided by 2 */
#define LL_CRS_SYNC_DIV_4                  CRS_CFGR_SYNCDIV_1                        /*!< Synchro Signal divided by 4 */
#define LL_CRS_SYNC_DIV_8                  (CRS_CFGR_SYNCDIV_1 | CRS_CFGR_SYNCDIV_0) /*!< Synchro Signal divided by 8 */
#define LL_CRS_SYNC_DIV_16                 CRS_CFGR_SYNCDIV_2                        /*!< Synchro Signal divided by 16 */
#define LL_CRS_SYNC_DIV_32                 (CRS_CFGR_SYNCDIV_2 | CRS_CFGR_SYNCDIV_0) /*!< Synchro Signal divided by 32 */
#define LL_CRS_SYNC_DIV_64                 (CRS_CFGR_SYNCDIV_2 | CRS_CFGR_SYNCDIV_1) /*!< Synchro Signal divided by 64 */
#define LL_CRS_SYNC_DIV_128                CRS_CFGR_SYNCDIV                          /*!< Synchro Signal divided by 128 */
/**
  * @}
  */

/** @defgroup CRS_LL_EC_SYNC_SOURCE Synchronization Signal Source
  * @{
  */
#define LL_CRS_SYNC_SOURCE_GPIO            ((uint32_t)0x00U)       /*!< Synchro Signal soucre GPIO */
#define LL_CRS_SYNC_SOURCE_LSE             CRS_CFGR_SYNCSRC_0      /*!< Synchro Signal source LSE */
#define LL_CRS_SYNC_SOURCE_USB             CRS_CFGR_SYNCSRC_1      /*!< Synchro Signal source USB SOF (default)*/
/**
  * @}
  */

/** @defgroup CRS_LL_EC_SYNC_POLARITY Synchronization Signal Polarity
  * @{
  */
#define LL_CRS_SYNC_POLARITY_RISING        ((uint32_t)0x00U)     /*!< Synchro Active on rising edge (default) */
#define LL_CRS_SYNC_POLARITY_FALLING       CRS_CFGR_SYNCPOL      /*!< Synchro Active on falling edge */
/**
  * @}
  */

/** @defgroup CRS_LL_EC_FREQERRORDIR Frequency Error Direction
  * @{
  */
#define LL_CRS_FREQ_ERROR_DIR_UP             ((uint32_t)0x00U)         /*!< Upcounting direction, the actual frequency is above the target */
#define LL_CRS_FREQ_ERROR_DIR_DOWN           ((uint32_t)CRS_ISR_FEDIR) /*!< Downcounting direction, the actual frequency is below the target */
/**
  * @}
  */

/** @defgroup CRS_LL_EC_DEFAULTVALUES Default Values
  * @{
  */
/**
  * @brief Reset value of the RELOAD field
  * @note The reset value of the RELOAD field corresponds to a target frequency of 48 MHz
  *       and a synchronization signal frequency of 1 kHz (SOF signal from USB)
  */
#define LL_CRS_RELOADVALUE_DEFAULT         ((uint32_t)0xBB7FU)

/**
  * @brief Reset value of Frequency error limit.
  */
#define LL_CRS_ERRORLIMIT_DEFAULT          ((uint32_t)0x22U)

/**
  * @brief Reset value of the HSI48 Calibration field
  * @note The default value is 64 for STM32L412xx/L422xx, 32 otherwise, which corresponds
  *       to the middle of the trimming interval.
  *       The trimming step is around 67 kHz between two consecutive TRIM steps.
  *       A higher TRIM value corresponds to a higher output frequency
  */
#if defined (STM32L412xx) || defined (STM32L422xx)
#define LL_CRS_HSI48CALIBRATION_DEFAULT    ((uint32_t)64U)
#else
#define LL_CRS_HSI48CALIBRATION_DEFAULT    ((uint32_t)32U)
#endif
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup CRS_LL_Exported_Macros CRS Exported Macros
  * @{
  */

/** @defgroup CRS_LL_EM_WRITE_READ Common Write and read registers Macros
  * @{
  */

/**
  * @brief  Write a value in CRS register
  * @param  __INSTANCE__ CRS Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_CRS_WriteReg(__INSTANCE__, __REG__, __VALUE__) WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
  * @brief  Read a value in CRS register
  * @param  __INSTANCE__ CRS Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_CRS_ReadReg(__INSTANCE__, __REG__) READ_REG(__INSTANCE__->__REG__)
/**
  * @}
  */

/** @defgroup CRS_LL_EM_Exported_Macros_Calculate_Reload Exported_Macros_Calculate_Reload
  * @{
  */

/**
  * @brief  Macro to calculate reload value to be set in CRS register according to target and sync frequencies
  * @note   The RELOAD value should be selected according to the ratio between
  *         the target frequency and the frequency of the synchronization source after
  *         prescaling. It is then decreased by one in order to reach the expected
  *         synchronization on the zero value. The formula is the following:
  *              RELOAD = (fTARGET / fSYNC) -1
  * @param  __FTARGET__ Target frequency (value in Hz)
  * @param  __FSYNC__ Synchronization signal frequency (value in Hz)
  * @retval Reload value (in Hz)
  */
#define __LL_CRS_CALC_CALCULATE_RELOADVALUE(__FTARGET__, __FSYNC__) (((__FTARGET__) / (__FSYNC__)) - 1U)

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup CRS_LL_Exported_Functions CRS Exported Functions
  * @{
  */

/** @defgroup CRS_LL_EF_Configuration Configuration
  * @{
  */

/**
  * @brief  Enable Frequency error counter
  * @note When this bit is set, the CRS_CFGR register is write-protected and cannot be modified
  * @rmtoll CR           CEN           LL_CRS_EnableFreqErrorCounter
  * @retval None
  */
__STATIC_INLINE void LL_CRS_EnableFreqErrorCounter(void)
{
  SET_BIT(CRS->CR, CRS_CR_CEN);
}

/**
  * @brief  Disable Frequency error counter
  * @rmtoll CR           CEN           LL_CRS_DisableFreqErrorCounter
  * @retval None
  */
__STATIC_INLINE void LL_CRS_DisableFreqErrorCounter(void)
{
  CLEAR_BIT(CRS->CR, CRS_CR_CEN);
}

/**
  * @brief  Check if Frequency error counter is enabled or not
  * @rmtoll CR           CEN           LL_CRS_IsEnabledFreqErrorCounter
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsEnabledFreqErrorCounter(void)
{
  return (READ_BIT(CRS->CR, CRS_CR_CEN) == (CRS_CR_CEN));
}

/**
  * @brief  Enable Automatic trimming counter
  * @rmtoll CR           AUTOTRIMEN    LL_CRS_EnableAutoTrimming
  * @retval None
  */
__STATIC_INLINE void LL_CRS_EnableAutoTrimming(void)
{
  SET_BIT(CRS->CR, CRS_CR_AUTOTRIMEN);
}

/**
  * @brief  Disable Automatic trimming counter
  * @rmtoll CR           AUTOTRIMEN    LL_CRS_DisableAutoTrimming
  * @retval None
  */
__STATIC_INLINE void LL_CRS_DisableAutoTrimming(void)
{
  CLEAR_BIT(CRS->CR, CRS_CR_AUTOTRIMEN);
}

/**
  * @brief  Check if Automatic trimming is enabled or not
  * @rmtoll CR           AUTOTRIMEN    LL_CRS_IsEnabledAutoTrimming
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsEnabledAutoTrimming(void)
{
  return (READ_BIT(CRS->CR, CRS_CR_AUTOTRIMEN) == (CRS_CR_AUTOTRIMEN));
}

/**
  * @brief  Set HSI48 oscillator smooth trimming
  * @note   When the AUTOTRIMEN bit is set, this field is controlled by hardware and is read-only
  * @rmtoll CR           TRIM          LL_CRS_SetHSI48SmoothTrimming
  * @param  Value a number between Min_Data = 0 and Max_Data = 127 for STM32L412xx/L422xx or 63 otherwise
  * @note   Default value can be set thanks to @ref LL_CRS_HSI48CALIBRATION_DEFAULT
  * @retval None
  */
__STATIC_INLINE void LL_CRS_SetHSI48SmoothTrimming(uint32_t Value)
{
  MODIFY_REG(CRS->CR, CRS_CR_TRIM, Value << CRS_CR_TRIM_Pos);
}

/**
  * @brief  Get HSI48 oscillator smooth trimming
  * @rmtoll CR           TRIM          LL_CRS_GetHSI48SmoothTrimming
  * @retval a number between Min_Data = 0 and Max_Data = 127 for STM32L412xx/L422xx or 63 otherwise
  */
__STATIC_INLINE uint32_t LL_CRS_GetHSI48SmoothTrimming(void)
{
  return (uint32_t)(READ_BIT(CRS->CR, CRS_CR_TRIM) >> CRS_CR_TRIM_Pos);
}

/**
  * @brief  Set counter reload value
  * @rmtoll CFGR         RELOAD        LL_CRS_SetReloadCounter
  * @param  Value a number between Min_Data = 0 and Max_Data = 0xFFFF
  * @note   Default value can be set thanks to @ref LL_CRS_RELOADVALUE_DEFAULT
  *         Otherwise it can be calculated in using macro @ref __LL_CRS_CALC_CALCULATE_RELOADVALUE (_FTARGET_, _FSYNC_)
  * @retval None
  */
__STATIC_INLINE void LL_CRS_SetReloadCounter(uint32_t Value)
{
  MODIFY_REG(CRS->CFGR, CRS_CFGR_RELOAD, Value);
}

/**
  * @brief  Get counter reload value
  * @rmtoll CFGR         RELOAD        LL_CRS_GetReloadCounter
  * @retval a number between Min_Data = 0 and Max_Data = 0xFFFF
  */
__STATIC_INLINE uint32_t LL_CRS_GetReloadCounter(void)
{
  return (uint32_t)(READ_BIT(CRS->CFGR, CRS_CFGR_RELOAD));
}

/**
  * @brief  Set frequency error limit
  * @rmtoll CFGR         FELIM         LL_CRS_SetFreqErrorLimit
  * @param  Value a number between Min_Data = 0 and Max_Data = 255
  * @note   Default value can be set thanks to @ref LL_CRS_ERRORLIMIT_DEFAULT
  * @retval None
  */
__STATIC_INLINE void LL_CRS_SetFreqErrorLimit(uint32_t Value)
{
  MODIFY_REG(CRS->CFGR, CRS_CFGR_FELIM, Value << CRS_CFGR_FELIM_Pos);
}

/**
  * @brief  Get frequency error limit
  * @rmtoll CFGR         FELIM         LL_CRS_GetFreqErrorLimit
  * @retval A number between Min_Data = 0 and Max_Data = 255
  */
__STATIC_INLINE uint32_t LL_CRS_GetFreqErrorLimit(void)
{
  return (uint32_t)(READ_BIT(CRS->CFGR, CRS_CFGR_FELIM) >> CRS_CFGR_FELIM_Pos);
}

/**
  * @brief  Set division factor for SYNC signal
  * @rmtoll CFGR         SYNCDIV       LL_CRS_SetSyncDivider
  * @param  Divider This parameter can be one of the following values:
  *         @arg @ref LL_CRS_SYNC_DIV_1
  *         @arg @ref LL_CRS_SYNC_DIV_2
  *         @arg @ref LL_CRS_SYNC_DIV_4
  *         @arg @ref LL_CRS_SYNC_DIV_8
  *         @arg @ref LL_CRS_SYNC_DIV_16
  *         @arg @ref LL_CRS_SYNC_DIV_32
  *         @arg @ref LL_CRS_SYNC_DIV_64
  *         @arg @ref LL_CRS_SYNC_DIV_128
  * @retval None
  */
__STATIC_INLINE void LL_CRS_SetSyncDivider(uint32_t Divider)
{
  MODIFY_REG(CRS->CFGR, CRS_CFGR_SYNCDIV, Divider);
}

/**
  * @brief  Get division factor for SYNC signal
  * @rmtoll CFGR         SYNCDIV       LL_CRS_GetSyncDivider
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_CRS_SYNC_DIV_1
  *         @arg @ref LL_CRS_SYNC_DIV_2
  *         @arg @ref LL_CRS_SYNC_DIV_4
  *         @arg @ref LL_CRS_SYNC_DIV_8
  *         @arg @ref LL_CRS_SYNC_DIV_16
  *         @arg @ref LL_CRS_SYNC_DIV_32
  *         @arg @ref LL_CRS_SYNC_DIV_64
  *         @arg @ref LL_CRS_SYNC_DIV_128
  */
__STATIC_INLINE uint32_t LL_CRS_GetSyncDivider(void)
{
  return (uint32_t)(READ_BIT(CRS->CFGR, CRS_CFGR_SYNCDIV));
}

/**
  * @brief  Set SYNC signal source
  * @rmtoll CFGR         SYNCSRC       LL_CRS_SetSyncSignalSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_CRS_SYNC_SOURCE_GPIO
  *         @arg @ref LL_CRS_SYNC_SOURCE_LSE
  *         @arg @ref LL_CRS_SYNC_SOURCE_USB
  * @retval None
  */
__STATIC_INLINE void LL_CRS_SetSyncSignalSource(uint32_t Source)
{
  MODIFY_REG(CRS->CFGR, CRS_CFGR_SYNCSRC, Source);
}

/**
  * @brief  Get SYNC signal source
  * @rmtoll CFGR         SYNCSRC       LL_CRS_GetSyncSignalSource
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_CRS_SYNC_SOURCE_GPIO
  *         @arg @ref LL_CRS_SYNC_SOURCE_LSE
  *         @arg @ref LL_CRS_SYNC_SOURCE_USB
  */
__STATIC_INLINE uint32_t LL_CRS_GetSyncSignalSource(void)
{
  return (uint32_t)(READ_BIT(CRS->CFGR, CRS_CFGR_SYNCSRC));
}

/**
  * @brief  Set input polarity for the SYNC signal source
  * @rmtoll CFGR         SYNCPOL       LL_CRS_SetSyncPolarity
  * @param  Polarity This parameter can be one of the following values:
  *         @arg @ref LL_CRS_SYNC_POLARITY_RISING
  *         @arg @ref LL_CRS_SYNC_POLARITY_FALLING
  * @retval None
  */
__STATIC_INLINE void LL_CRS_SetSyncPolarity(uint32_t Polarity)
{
  MODIFY_REG(CRS->CFGR, CRS_CFGR_SYNCPOL, Polarity);
}

/**
  * @brief  Get input polarity for the SYNC signal source
  * @rmtoll CFGR         SYNCPOL       LL_CRS_GetSyncPolarity
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_CRS_SYNC_POLARITY_RISING
  *         @arg @ref LL_CRS_SYNC_POLARITY_FALLING
  */
__STATIC_INLINE uint32_t LL_CRS_GetSyncPolarity(void)
{
  return (uint32_t)(READ_BIT(CRS->CFGR, CRS_CFGR_SYNCPOL));
}

/**
  * @brief  Configure CRS for the synchronization
  * @rmtoll CR           TRIM          LL_CRS_ConfigSynchronization\n
  *         CFGR         RELOAD        LL_CRS_ConfigSynchronization\n
  *         CFGR         FELIM         LL_CRS_ConfigSynchronization\n
  *         CFGR         SYNCDIV       LL_CRS_ConfigSynchronization\n
  *         CFGR         SYNCSRC       LL_CRS_ConfigSynchronization\n
  *         CFGR         SYNCPOL       LL_CRS_ConfigSynchronization
  * @param  HSI48CalibrationValue a number between Min_Data = 0 and Max_Data = 63
  * @param  ErrorLimitValue a number between Min_Data = 0 and Max_Data = 0xFFFF
  * @param  ReloadValue a number between Min_Data = 0 and Max_Data = 255
  * @param  Settings This parameter can be a combination of the following values:
  *         @arg @ref LL_CRS_SYNC_DIV_1 or @ref LL_CRS_SYNC_DIV_2 or @ref LL_CRS_SYNC_DIV_4 or @ref LL_CRS_SYNC_DIV_8
  *              or @ref LL_CRS_SYNC_DIV_16 or @ref LL_CRS_SYNC_DIV_32 or @ref LL_CRS_SYNC_DIV_64 or @ref LL_CRS_SYNC_DIV_128
  *         @arg @ref LL_CRS_SYNC_SOURCE_GPIO or @ref LL_CRS_SYNC_SOURCE_LSE or @ref LL_CRS_SYNC_SOURCE_USB
  *         @arg @ref LL_CRS_SYNC_POLARITY_RISING or @ref LL_CRS_SYNC_POLARITY_FALLING
  * @retval None
  */
__STATIC_INLINE void LL_CRS_ConfigSynchronization(uint32_t HSI48CalibrationValue, uint32_t ErrorLimitValue, uint32_t ReloadValue, uint32_t Settings)
{
  MODIFY_REG(CRS->CR, CRS_CR_TRIM, HSI48CalibrationValue);
  MODIFY_REG(CRS->CFGR,
             CRS_CFGR_RELOAD | CRS_CFGR_FELIM | CRS_CFGR_SYNCDIV | CRS_CFGR_SYNCSRC | CRS_CFGR_SYNCPOL,
             ReloadValue | (ErrorLimitValue << CRS_CFGR_FELIM_Pos) | Settings);
}

/**
  * @}
  */

/** @defgroup CRS_LL_EF_CRS_Management CRS_Management
  * @{
  */

/**
  * @brief  Generate software SYNC event
  * @rmtoll CR           SWSYNC        LL_CRS_GenerateEvent_SWSYNC
  * @retval None
  */
__STATIC_INLINE void LL_CRS_GenerateEvent_SWSYNC(void)
{
  SET_BIT(CRS->CR, CRS_CR_SWSYNC);
}

/**
  * @brief  Get the frequency error direction latched in the time of the last
  * SYNC event
  * @rmtoll ISR          FEDIR         LL_CRS_GetFreqErrorDirection
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_CRS_FREQ_ERROR_DIR_UP
  *         @arg @ref LL_CRS_FREQ_ERROR_DIR_DOWN
  */
__STATIC_INLINE uint32_t LL_CRS_GetFreqErrorDirection(void)
{
  return (uint32_t)(READ_BIT(CRS->ISR, CRS_ISR_FEDIR));
}

/**
  * @brief  Get the frequency error counter value latched in the time of the last SYNC event
  * @rmtoll ISR          FECAP         LL_CRS_GetFreqErrorCapture
  * @retval A number between Min_Data = 0x0000 and Max_Data = 0xFFFF
  */
__STATIC_INLINE uint32_t LL_CRS_GetFreqErrorCapture(void)
{
  return (uint32_t)(READ_BIT(CRS->ISR, CRS_ISR_FECAP) >> CRS_ISR_FECAP_Pos);
}

/**
  * @}
  */

/** @defgroup CRS_LL_EF_FLAG_Management FLAG_Management
  * @{
  */

/**
  * @brief  Check if SYNC event OK signal occurred or not
  * @rmtoll ISR          SYNCOKF       LL_CRS_IsActiveFlag_SYNCOK
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsActiveFlag_SYNCOK(void)
{
  return (READ_BIT(CRS->ISR, CRS_ISR_SYNCOKF) == (CRS_ISR_SYNCOKF));
}

/**
  * @brief  Check if SYNC warning signal occurred or not
  * @rmtoll ISR          SYNCWARNF     LL_CRS_IsActiveFlag_SYNCWARN
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsActiveFlag_SYNCWARN(void)
{
  return (READ_BIT(CRS->ISR, CRS_ISR_SYNCWARNF) == (CRS_ISR_SYNCWARNF));
}

/**
  * @brief  Check if Synchronization or trimming error signal occurred or not
  * @rmtoll ISR          ERRF          LL_CRS_IsActiveFlag_ERR
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsActiveFlag_ERR(void)
{
  return (READ_BIT(CRS->ISR, CRS_ISR_ERRF) == (CRS_ISR_ERRF));
}

/**
  * @brief  Check if Expected SYNC signal occurred or not
  * @rmtoll ISR          ESYNCF        LL_CRS_IsActiveFlag_ESYNC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsActiveFlag_ESYNC(void)
{
  return (READ_BIT(CRS->ISR, CRS_ISR_ESYNCF) == (CRS_ISR_ESYNCF));
}

/**
  * @brief  Check if SYNC error signal occurred or not
  * @rmtoll ISR          SYNCERR       LL_CRS_IsActiveFlag_SYNCERR
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsActiveFlag_SYNCERR(void)
{
  return (READ_BIT(CRS->ISR, CRS_ISR_SYNCERR) == (CRS_ISR_SYNCERR));
}

/**
  * @brief  Check if SYNC missed error signal occurred or not
  * @rmtoll ISR          SYNCMISS      LL_CRS_IsActiveFlag_SYNCMISS
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsActiveFlag_SYNCMISS(void)
{
  return (READ_BIT(CRS->ISR, CRS_ISR_SYNCMISS) == (CRS_ISR_SYNCMISS));
}

/**
  * @brief  Check if Trimming overflow or underflow occurred or not
  * @rmtoll ISR          TRIMOVF       LL_CRS_IsActiveFlag_TRIMOVF
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsActiveFlag_TRIMOVF(void)
{
  return (READ_BIT(CRS->ISR, CRS_ISR_TRIMOVF) == (CRS_ISR_TRIMOVF));
}

/**
  * @brief  Clear the SYNC event OK flag
  * @rmtoll ICR          SYNCOKC       LL_CRS_ClearFlag_SYNCOK
  * @retval None
  */
__STATIC_INLINE void LL_CRS_ClearFlag_SYNCOK(void)
{
  WRITE_REG(CRS->ICR, CRS_ICR_SYNCOKC);
}

/**
  * @brief  Clear the  SYNC warning flag
  * @rmtoll ICR          SYNCWARNC     LL_CRS_ClearFlag_SYNCWARN
  * @retval None
  */
__STATIC_INLINE void LL_CRS_ClearFlag_SYNCWARN(void)
{
  WRITE_REG(CRS->ICR, CRS_ICR_SYNCWARNC);
}

/**
  * @brief  Clear TRIMOVF, SYNCMISS and SYNCERR bits and consequently also
  * the ERR flag
  * @rmtoll ICR          ERRC          LL_CRS_ClearFlag_ERR
  * @retval None
  */
__STATIC_INLINE void LL_CRS_ClearFlag_ERR(void)
{
  WRITE_REG(CRS->ICR, CRS_ICR_ERRC);
}

/**
  * @brief  Clear Expected SYNC flag
  * @rmtoll ICR          ESYNCC        LL_CRS_ClearFlag_ESYNC
  * @retval None
  */
__STATIC_INLINE void LL_CRS_ClearFlag_ESYNC(void)
{
  WRITE_REG(CRS->ICR, CRS_ICR_ESYNCC);
}

/**
  * @}
  */

/** @defgroup CRS_LL_EF_IT_Management IT_Management
  * @{
  */

/**
  * @brief  Enable SYNC event OK interrupt
  * @rmtoll CR           SYNCOKIE      LL_CRS_EnableIT_SYNCOK
  * @retval None
  */
__STATIC_INLINE void LL_CRS_EnableIT_SYNCOK(void)
{
  SET_BIT(CRS->CR, CRS_CR_SYNCOKIE);
}

/**
  * @brief  Disable SYNC event OK interrupt
  * @rmtoll CR           SYNCOKIE      LL_CRS_DisableIT_SYNCOK
  * @retval None
  */
__STATIC_INLINE void LL_CRS_DisableIT_SYNCOK(void)
{
  CLEAR_BIT(CRS->CR, CRS_CR_SYNCOKIE);
}

/**
  * @brief  Check if SYNC event OK interrupt is enabled or not
  * @rmtoll CR           SYNCOKIE      LL_CRS_IsEnabledIT_SYNCOK
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsEnabledIT_SYNCOK(void)
{
  return (READ_BIT(CRS->CR, CRS_CR_SYNCOKIE) == (CRS_CR_SYNCOKIE));
}

/**
  * @brief  Enable SYNC warning interrupt
  * @rmtoll CR           SYNCWARNIE    LL_CRS_EnableIT_SYNCWARN
  * @retval None
  */
__STATIC_INLINE void LL_CRS_EnableIT_SYNCWARN(void)
{
  SET_BIT(CRS->CR, CRS_CR_SYNCWARNIE);
}

/**
  * @brief  Disable SYNC warning interrupt
  * @rmtoll CR           SYNCWARNIE    LL_CRS_DisableIT_SYNCWARN
  * @retval None
  */
__STATIC_INLINE void LL_CRS_DisableIT_SYNCWARN(void)
{
  CLEAR_BIT(CRS->CR, CRS_CR_SYNCWARNIE);
}

/**
  * @brief  Check if SYNC warning interrupt is enabled or not
  * @rmtoll CR           SYNCWARNIE    LL_CRS_IsEnabledIT_SYNCWARN
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsEnabledIT_SYNCWARN(void)
{
  return (READ_BIT(CRS->CR, CRS_CR_SYNCWARNIE) == (CRS_CR_SYNCWARNIE));
}

/**
  * @brief  Enable Synchronization or trimming error interrupt
  * @rmtoll CR           ERRIE         LL_CRS_EnableIT_ERR
  * @retval None
  */
__STATIC_INLINE void LL_CRS_EnableIT_ERR(void)
{
  SET_BIT(CRS->CR, CRS_CR_ERRIE);
}

/**
  * @brief  Disable Synchronization or trimming error interrupt
  * @rmtoll CR           ERRIE         LL_CRS_DisableIT_ERR
  * @retval None
  */
__STATIC_INLINE void LL_CRS_DisableIT_ERR(void)
{
  CLEAR_BIT(CRS->CR, CRS_CR_ERRIE);
}

/**
  * @brief  Check if Synchronization or trimming error interrupt is enabled or not
  * @rmtoll CR           ERRIE         LL_CRS_IsEnabledIT_ERR
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsEnabledIT_ERR(void)
{
  return (READ_BIT(CRS->CR, CRS_CR_ERRIE) == (CRS_CR_ERRIE));
}

/**
  * @brief  Enable Expected SYNC interrupt
  * @rmtoll CR           ESYNCIE       LL_CRS_EnableIT_ESYNC
  * @retval None
  */
__STATIC_INLINE void LL_CRS_EnableIT_ESYNC(void)
{
  SET_BIT(CRS->CR, CRS_CR_ESYNCIE);
}

/**
  * @brief  Disable Expected SYNC interrupt
  * @rmtoll CR           ESYNCIE       LL_CRS_DisableIT_ESYNC
  * @retval None
  */
__STATIC_INLINE void LL_CRS_DisableIT_ESYNC(void)
{
  CLEAR_BIT(CRS->CR, CRS_CR_ESYNCIE);
}

/**
  * @brief  Check if Expected SYNC interrupt is enabled or not
  * @rmtoll CR           ESYNCIE       LL_CRS_IsEnabledIT_ESYNC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_CRS_IsEnabledIT_ESYNC(void)
{
  return (READ_BIT(CRS->CR, CRS_CR_ESYNCIE) == (CRS_CR_ESYNCIE));
}

/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup CRS_LL_EF_Init Initialization and de-initialization functions
  * @{
  */

ErrorStatus LL_CRS_DeInit(void);

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

#endif /* defined(CRS) */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_LL_CRS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
