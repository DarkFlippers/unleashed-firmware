/**
  ******************************************************************************
  * @file    stm32l4xx_hal_rcc.h
  * @author  MCD Application Team
  * @brief   Header file of RCC HAL module.
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
#ifndef __STM32L4xx_HAL_RCC_H
#define __STM32L4xx_HAL_RCC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup RCC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup RCC_Exported_Types RCC Exported Types
  * @{
  */

/**
  * @brief  RCC PLL configuration structure definition
  */
typedef struct
{
  uint32_t PLLState;   /*!< The new state of the PLL.
                            This parameter can be a value of @ref RCC_PLL_Config                      */

  uint32_t PLLSource;  /*!< RCC_PLLSource: PLL entry clock source.
                            This parameter must be a value of @ref RCC_PLL_Clock_Source               */

  uint32_t PLLM;       /*!< PLLM: Division factor for PLL VCO input clock.
                            This parameter must be a number between Min_Data = 1 and Max_Data = 16 on STM32L4Rx/STM32L4Sx devices.
                            This parameter must be a number between Min_Data = 1 and Max_Data = 8 on the other devices */

  uint32_t PLLN;       /*!< PLLN: Multiplication factor for PLL VCO output clock.
                            This parameter must be a number between Min_Data = 8 and Max_Data = 86    */

#if defined(RCC_PLLP_SUPPORT)
  uint32_t PLLP;       /*!< PLLP: Division factor for SAI clock.
                            This parameter must be a value of @ref RCC_PLLP_Clock_Divider             */
#endif /* RCC_PLLP_SUPPORT */

  uint32_t PLLQ;       /*!< PLLQ: Division factor for SDMMC1, RNG and USB clocks.
                            This parameter must be a value of @ref RCC_PLLQ_Clock_Divider             */

  uint32_t PLLR;       /*!< PLLR: Division for the main system clock.
                            User have to set the PLLR parameter correctly to not exceed max frequency 120MHZ
                            on STM32L4Rx/STM32L4Sx devices else 80MHz on the other devices.
                            This parameter must be a value of @ref RCC_PLLR_Clock_Divider             */

}RCC_PLLInitTypeDef;

/**
  * @brief  RCC Internal/External Oscillator (HSE, HSI, MSI, LSE and LSI) configuration structure definition
  */
typedef struct
{
  uint32_t OscillatorType;       /*!< The oscillators to be configured.
                                      This parameter can be a value of @ref RCC_Oscillator_Type                   */

  uint32_t HSEState;             /*!< The new state of the HSE.
                                      This parameter can be a value of @ref RCC_HSE_Config                        */

  uint32_t LSEState;             /*!< The new state of the LSE.
                                      This parameter can be a value of @ref RCC_LSE_Config                        */

  uint32_t HSIState;             /*!< The new state of the HSI.
                                      This parameter can be a value of @ref RCC_HSI_Config                        */

  uint32_t HSICalibrationValue;  /*!< The calibration trimming value (default is RCC_HSICALIBRATION_DEFAULT).
                                      This parameter must be a number between Min_Data = 0x00 and Max_Data = 0x1F on STM32L43x/STM32L44x/STM32L47x/STM32L48x devices.
                                      This parameter must be a number between Min_Data = 0x00 and Max_Data = 0x7F on the other devices */

  uint32_t LSIState;             /*!< The new state of the LSI.
                                      This parameter can be a value of @ref RCC_LSI_Config                        */
#if defined(RCC_CSR_LSIPREDIV)

  uint32_t LSIDiv;               /*!< The division factor of the LSI.
                                      This parameter can be a value of @ref RCC_LSI_Div                           */
#endif /* RCC_CSR_LSIPREDIV */

  uint32_t MSIState;             /*!< The new state of the MSI.
                                      This parameter can be a value of @ref RCC_MSI_Config */

  uint32_t MSICalibrationValue;  /*!< The calibration trimming value (default is RCC_MSICALIBRATION_DEFAULT).
                                      This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF */

  uint32_t MSIClockRange;        /*!< The MSI frequency range.
                                      This parameter can be a value of @ref RCC_MSI_Clock_Range  */

  uint32_t HSI48State;             /*!< The new state of the HSI48 (only applicable to STM32L43x/STM32L44x/STM32L49x/STM32L4Ax devices).
                                        This parameter can be a value of @ref RCC_HSI48_Config */

  RCC_PLLInitTypeDef PLL;        /*!< Main PLL structure parameters                                               */

}RCC_OscInitTypeDef;

/**
  * @brief  RCC System, AHB and APB busses clock configuration structure definition
  */
typedef struct
{
  uint32_t ClockType;             /*!< The clock to be configured.
                                       This parameter can be a value of @ref RCC_System_Clock_Type      */

  uint32_t SYSCLKSource;          /*!< The clock source used as system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_System_Clock_Source    */

  uint32_t AHBCLKDivider;         /*!< The AHB clock (HCLK) divider. This clock is derived from the system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_AHB_Clock_Source       */

  uint32_t APB1CLKDivider;        /*!< The APB1 clock (PCLK1) divider. This clock is derived from the AHB clock (HCLK).
                                       This parameter can be a value of @ref RCC_APB1_APB2_Clock_Source */

  uint32_t APB2CLKDivider;        /*!< The APB2 clock (PCLK2) divider. This clock is derived from the AHB clock (HCLK).
                                       This parameter can be a value of @ref RCC_APB1_APB2_Clock_Source */

}RCC_ClkInitTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup RCC_Exported_Constants RCC Exported Constants
  * @{
  */

/** @defgroup RCC_Timeout_Value Timeout Values
  * @{
  */
#define RCC_DBP_TIMEOUT_VALUE          2U            /* 2 ms (minimum Tick + 1) */
#define RCC_LSE_TIMEOUT_VALUE          LSE_STARTUP_TIMEOUT
/**
  * @}
  */

/** @defgroup RCC_Oscillator_Type Oscillator Type
  * @{
  */
#define RCC_OSCILLATORTYPE_NONE        0x00000000U   /*!< Oscillator configuration unchanged */
#define RCC_OSCILLATORTYPE_HSE         0x00000001U   /*!< HSE to configure */
#define RCC_OSCILLATORTYPE_HSI         0x00000002U   /*!< HSI to configure */
#define RCC_OSCILLATORTYPE_LSE         0x00000004U   /*!< LSE to configure */
#define RCC_OSCILLATORTYPE_LSI         0x00000008U   /*!< LSI to configure */
#define RCC_OSCILLATORTYPE_MSI         0x00000010U   /*!< MSI to configure */
#if defined(RCC_HSI48_SUPPORT)
#define RCC_OSCILLATORTYPE_HSI48       0x00000020U   /*!< HSI48 to configure */
#endif /* RCC_HSI48_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_HSE_Config HSE Config
  * @{
  */
#define RCC_HSE_OFF                    0x00000000U                    /*!< HSE clock deactivation */
#define RCC_HSE_ON                     RCC_CR_HSEON                   /*!< HSE clock activation */
#define RCC_HSE_BYPASS                 (RCC_CR_HSEBYP | RCC_CR_HSEON) /*!< External clock source for HSE clock */
/**
  * @}
  */

/** @defgroup RCC_LSE_Config LSE Config
  * @{
  */
#define RCC_LSE_OFF                    0x00000000U                           /*!< LSE clock deactivation */
#define RCC_LSE_ON                     RCC_BDCR_LSEON                        /*!< LSE clock activation */
#define RCC_LSE_BYPASS                 (RCC_BDCR_LSEBYP | RCC_BDCR_LSEON)    /*!< External clock source for LSE clock */
#if defined(RCC_BDCR_LSESYSDIS)
#define RCC_LSE_ON_RTC_ONLY            (RCC_BDCR_LSESYSDIS | RCC_BDCR_LSEON) /*!< LSE clock activation without propagation to system */
#define RCC_LSE_BYPASS_RTC_ONLY        (RCC_BDCR_LSEBYP | RCC_BDCR_LSESYSDIS | RCC_BDCR_LSEON) /*!< External clock source for LSE clock without propagation to system */
#endif /* RCC_BDCR_LSESYSDIS */
/**
  * @}
  */

/** @defgroup RCC_HSI_Config HSI Config
  * @{
  */
#define RCC_HSI_OFF                    0x00000000U   /*!< HSI clock deactivation */
#define RCC_HSI_ON                     RCC_CR_HSION  /*!< HSI clock activation */

#if defined(STM32L431xx) || defined(STM32L432xx) || defined(STM32L433xx) || defined(STM32L442xx) || defined(STM32L443xx) || \
    defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx)
#define RCC_HSICALIBRATION_DEFAULT     0x10U         /* Default HSI calibration trimming value */
#else
#define RCC_HSICALIBRATION_DEFAULT     0x40U         /* Default HSI calibration trimming value */
#endif /* STM32L431xx || STM32L432xx || STM32L433xx || STM32L442xx || STM32L443xx || */
       /* STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx    */
/**
  * @}
  */

/** @defgroup RCC_LSI_Config LSI Config
  * @{
  */
#define RCC_LSI_OFF                    0x00000000U   /*!< LSI clock deactivation */
#define RCC_LSI_ON                     RCC_CSR_LSION /*!< LSI clock activation */
/**
  * @}
  */
#if defined(RCC_CSR_LSIPREDIV)

/** @defgroup RCC_LSI_Div LSI Div
  * @{
  */
#define RCC_LSI_DIV1                   0x00000000U       /*!< LSI clock not divided    */
#define RCC_LSI_DIV128                 RCC_CSR_LSIPREDIV /*!< LSI clock divided by 128 */
/**
  * @}
  */
#endif /* RCC_CSR_LSIPREDIV */

/** @defgroup RCC_MSI_Config MSI Config
  * @{
  */
#define RCC_MSI_OFF                    0x00000000U   /*!< MSI clock deactivation */
#define RCC_MSI_ON                     RCC_CR_MSION  /*!< MSI clock activation */

#define RCC_MSICALIBRATION_DEFAULT     0U            /*!< Default MSI calibration trimming value */
/**
  * @}
  */

#if defined(RCC_HSI48_SUPPORT)
/** @defgroup RCC_HSI48_Config HSI48 Config
  * @{
  */
#define RCC_HSI48_OFF                  0x00000000U       /*!< HSI48 clock deactivation */
#define RCC_HSI48_ON                   RCC_CRRCR_HSI48ON /*!< HSI48 clock activation */
/**
  * @}
  */
#else
/** @defgroup RCC_HSI48_Config HSI48 Config
  * @{
  */
#define RCC_HSI48_OFF                  0x00000000U   /*!< HSI48 clock deactivation */
/**
  * @}
  */
#endif /* RCC_HSI48_SUPPORT */

/** @defgroup RCC_PLL_Config PLL Config
  * @{
  */
#define RCC_PLL_NONE                   0x00000000U   /*!< PLL configuration unchanged */
#define RCC_PLL_OFF                    0x00000001U   /*!< PLL deactivation */
#define RCC_PLL_ON                     0x00000002U   /*!< PLL activation */
/**
  * @}
  */

#if defined(RCC_PLLP_SUPPORT)
/** @defgroup RCC_PLLP_Clock_Divider PLLP Clock Divider
  * @{
  */
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
#define RCC_PLLP_DIV2                  0x00000002U   /*!< PLLP division factor = 2  */
#define RCC_PLLP_DIV3                  0x00000003U   /*!< PLLP division factor = 3  */
#define RCC_PLLP_DIV4                  0x00000004U   /*!< PLLP division factor = 4  */
#define RCC_PLLP_DIV5                  0x00000005U   /*!< PLLP division factor = 5  */
#define RCC_PLLP_DIV6                  0x00000006U   /*!< PLLP division factor = 6  */
#define RCC_PLLP_DIV7                  0x00000007U   /*!< PLLP division factor = 7  */
#define RCC_PLLP_DIV8                  0x00000008U   /*!< PLLP division factor = 8  */
#define RCC_PLLP_DIV9                  0x00000009U   /*!< PLLP division factor = 9  */
#define RCC_PLLP_DIV10                 0x0000000AU   /*!< PLLP division factor = 10 */
#define RCC_PLLP_DIV11                 0x0000000BU   /*!< PLLP division factor = 11 */
#define RCC_PLLP_DIV12                 0x0000000CU   /*!< PLLP division factor = 12 */
#define RCC_PLLP_DIV13                 0x0000000DU   /*!< PLLP division factor = 13 */
#define RCC_PLLP_DIV14                 0x0000000EU   /*!< PLLP division factor = 14 */
#define RCC_PLLP_DIV15                 0x0000000FU   /*!< PLLP division factor = 15 */
#define RCC_PLLP_DIV16                 0x00000010U   /*!< PLLP division factor = 16 */
#define RCC_PLLP_DIV17                 0x00000011U   /*!< PLLP division factor = 17 */
#define RCC_PLLP_DIV18                 0x00000012U   /*!< PLLP division factor = 18 */
#define RCC_PLLP_DIV19                 0x00000013U   /*!< PLLP division factor = 19 */
#define RCC_PLLP_DIV20                 0x00000014U   /*!< PLLP division factor = 20 */
#define RCC_PLLP_DIV21                 0x00000015U   /*!< PLLP division factor = 21 */
#define RCC_PLLP_DIV22                 0x00000016U   /*!< PLLP division factor = 22 */
#define RCC_PLLP_DIV23                 0x00000017U   /*!< PLLP division factor = 23 */
#define RCC_PLLP_DIV24                 0x00000018U   /*!< PLLP division factor = 24 */
#define RCC_PLLP_DIV25                 0x00000019U   /*!< PLLP division factor = 25 */
#define RCC_PLLP_DIV26                 0x0000001AU   /*!< PLLP division factor = 26 */
#define RCC_PLLP_DIV27                 0x0000001BU   /*!< PLLP division factor = 27 */
#define RCC_PLLP_DIV28                 0x0000001CU   /*!< PLLP division factor = 28 */
#define RCC_PLLP_DIV29                 0x0000001DU   /*!< PLLP division factor = 29 */
#define RCC_PLLP_DIV30                 0x0000001EU   /*!< PLLP division factor = 30 */
#define RCC_PLLP_DIV31                 0x0000001FU   /*!< PLLP division factor = 31 */
#else
#define RCC_PLLP_DIV7                  0x00000007U   /*!< PLLP division factor = 7  */
#define RCC_PLLP_DIV17                 0x00000011U   /*!< PLLP division factor = 17 */
#endif /* RCC_PLLP_DIV_2_31_SUPPORT */
/**
  * @}
  */
#endif /* RCC_PLLP_SUPPORT */

/** @defgroup RCC_PLLQ_Clock_Divider PLLQ Clock Divider
  * @{
  */
#define RCC_PLLQ_DIV2                  0x00000002U   /*!< PLLQ division factor = 2 */
#define RCC_PLLQ_DIV4                  0x00000004U   /*!< PLLQ division factor = 4 */
#define RCC_PLLQ_DIV6                  0x00000006U   /*!< PLLQ division factor = 6 */
#define RCC_PLLQ_DIV8                  0x00000008U   /*!< PLLQ division factor = 8 */
/**
  * @}
  */

/** @defgroup RCC_PLLR_Clock_Divider PLLR Clock Divider
  * @{
  */
#define RCC_PLLR_DIV2                  0x00000002U   /*!< PLLR division factor = 2 */
#define RCC_PLLR_DIV4                  0x00000004U   /*!< PLLR division factor = 4 */
#define RCC_PLLR_DIV6                  0x00000006U   /*!< PLLR division factor = 6 */
#define RCC_PLLR_DIV8                  0x00000008U   /*!< PLLR division factor = 8 */
/**
  * @}
  */

/** @defgroup RCC_PLL_Clock_Source PLL Clock Source
  * @{
  */
#define RCC_PLLSOURCE_NONE             0x00000000U             /*!< No clock selected as PLL entry clock source  */
#define RCC_PLLSOURCE_MSI              RCC_PLLCFGR_PLLSRC_MSI  /*!< MSI clock selected as PLL entry clock source */
#define RCC_PLLSOURCE_HSI              RCC_PLLCFGR_PLLSRC_HSI  /*!< HSI clock selected as PLL entry clock source */
#define RCC_PLLSOURCE_HSE              RCC_PLLCFGR_PLLSRC_HSE  /*!< HSE clock selected as PLL entry clock source */
/**
  * @}
  */

/** @defgroup RCC_PLL_Clock_Output PLL Clock Output
  * @{
  */
#if defined(RCC_PLLSAI2_SUPPORT)
#define RCC_PLL_SAI3CLK                RCC_PLLCFGR_PLLPEN      /*!< PLLSAI3CLK selection from main PLL (for devices with PLLSAI2) */
#elif defined(RCC_PLLSAI1_SUPPORT)
#define RCC_PLL_SAI2CLK                RCC_PLLCFGR_PLLPEN      /*!< PLLSAI2CLK selection from main PLL (for devices without PLLSAI2) */
#endif /* RCC_PLLSAI2_SUPPORT */
#define RCC_PLL_48M1CLK                RCC_PLLCFGR_PLLQEN      /*!< PLL48M1CLK selection from main PLL */
#define RCC_PLL_SYSCLK                 RCC_PLLCFGR_PLLREN      /*!< PLLCLK selection from main PLL */
/**
  * @}
  */
#if defined(RCC_PLLSAI1_SUPPORT)

/** @defgroup RCC_PLLSAI1_Clock_Output PLLSAI1 Clock Output
  * @{
  */
#define RCC_PLLSAI1_SAI1CLK            RCC_PLLSAI1CFGR_PLLSAI1PEN /*!< PLLSAI1CLK selection from PLLSAI1 */
#define RCC_PLLSAI1_48M2CLK            RCC_PLLSAI1CFGR_PLLSAI1QEN /*!< PLL48M2CLK selection from PLLSAI1 */
#define RCC_PLLSAI1_ADC1CLK            RCC_PLLSAI1CFGR_PLLSAI1REN /*!< PLLADC1CLK selection from PLLSAI1 */
/**
  * @}
  */
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)

/** @defgroup RCC_PLLSAI2_Clock_Output PLLSAI2 Clock Output
  * @{
  */
#define RCC_PLLSAI2_SAI2CLK            RCC_PLLSAI2CFGR_PLLSAI2PEN /*!< PLLSAI2CLK selection from PLLSAI2 */
#if defined(RCC_PLLSAI2Q_DIV_SUPPORT)
#define RCC_PLLSAI2_DSICLK             RCC_PLLSAI2CFGR_PLLSAI2QEN /*!< PLLDSICLK selection from PLLSAI2  */
#endif /* RCC_PLLSAI2Q_DIV_SUPPORT */
#if defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || defined(STM32L496xx) || defined(STM32L4A6xx)
#define RCC_PLLSAI2_ADC2CLK            RCC_PLLSAI2CFGR_PLLSAI2REN /*!< PLLADC2CLK selection from PLLSAI2 */
#else
#define RCC_PLLSAI2_LTDCCLK            RCC_PLLSAI2CFGR_PLLSAI2REN /*!< PLLLTDCCLK selection from PLLSAI2 */
#endif /* STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx || STM32L496xx || STM32L4A6xx */
/**
  * @}
  */

#endif /* RCC_PLLSAI2_SUPPORT */

/** @defgroup RCC_MSI_Clock_Range MSI Clock Range
  * @{
  */
#define RCC_MSIRANGE_0                 RCC_CR_MSIRANGE_0  /*!< MSI = 100 KHz  */
#define RCC_MSIRANGE_1                 RCC_CR_MSIRANGE_1  /*!< MSI = 200 KHz  */
#define RCC_MSIRANGE_2                 RCC_CR_MSIRANGE_2  /*!< MSI = 400 KHz  */
#define RCC_MSIRANGE_3                 RCC_CR_MSIRANGE_3  /*!< MSI = 800 KHz  */
#define RCC_MSIRANGE_4                 RCC_CR_MSIRANGE_4  /*!< MSI = 1 MHz    */
#define RCC_MSIRANGE_5                 RCC_CR_MSIRANGE_5  /*!< MSI = 2 MHz    */
#define RCC_MSIRANGE_6                 RCC_CR_MSIRANGE_6  /*!< MSI = 4 MHz    */
#define RCC_MSIRANGE_7                 RCC_CR_MSIRANGE_7  /*!< MSI = 8 MHz    */
#define RCC_MSIRANGE_8                 RCC_CR_MSIRANGE_8  /*!< MSI = 16 MHz   */
#define RCC_MSIRANGE_9                 RCC_CR_MSIRANGE_9  /*!< MSI = 24 MHz   */
#define RCC_MSIRANGE_10                RCC_CR_MSIRANGE_10 /*!< MSI = 32 MHz   */
#define RCC_MSIRANGE_11                RCC_CR_MSIRANGE_11 /*!< MSI = 48 MHz   */
/**
  * @}
  */

/** @defgroup RCC_System_Clock_Type System Clock Type
  * @{
  */
#define RCC_CLOCKTYPE_SYSCLK           0x00000001U   /*!< SYSCLK to configure */
#define RCC_CLOCKTYPE_HCLK             0x00000002U   /*!< HCLK to configure */
#define RCC_CLOCKTYPE_PCLK1            0x00000004U   /*!< PCLK1 to configure */
#define RCC_CLOCKTYPE_PCLK2            0x00000008U   /*!< PCLK2 to configure */
/**
  * @}
  */

/** @defgroup RCC_System_Clock_Source System Clock Source
  * @{
  */
#define RCC_SYSCLKSOURCE_MSI           RCC_CFGR_SW_MSI    /*!< MSI selection as system clock */
#define RCC_SYSCLKSOURCE_HSI           RCC_CFGR_SW_HSI    /*!< HSI selection as system clock */
#define RCC_SYSCLKSOURCE_HSE           RCC_CFGR_SW_HSE    /*!< HSE selection as system clock */
#define RCC_SYSCLKSOURCE_PLLCLK        RCC_CFGR_SW_PLL    /*!< PLL selection as system clock */
/**
  * @}
  */

/** @defgroup RCC_System_Clock_Source_Status System Clock Source Status
  * @{
  */
#define RCC_SYSCLKSOURCE_STATUS_MSI    RCC_CFGR_SWS_MSI   /*!< MSI used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_HSI    RCC_CFGR_SWS_HSI   /*!< HSI used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_HSE    RCC_CFGR_SWS_HSE   /*!< HSE used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_PLLCLK RCC_CFGR_SWS_PLL   /*!< PLL used as system clock */
/**
  * @}
  */

/** @defgroup RCC_AHB_Clock_Source AHB Clock Source
  * @{
  */
#define RCC_SYSCLK_DIV1                RCC_CFGR_HPRE_DIV1   /*!< SYSCLK not divided */
#define RCC_SYSCLK_DIV2                RCC_CFGR_HPRE_DIV2   /*!< SYSCLK divided by 2 */
#define RCC_SYSCLK_DIV4                RCC_CFGR_HPRE_DIV4   /*!< SYSCLK divided by 4 */
#define RCC_SYSCLK_DIV8                RCC_CFGR_HPRE_DIV8   /*!< SYSCLK divided by 8 */
#define RCC_SYSCLK_DIV16               RCC_CFGR_HPRE_DIV16  /*!< SYSCLK divided by 16 */
#define RCC_SYSCLK_DIV64               RCC_CFGR_HPRE_DIV64  /*!< SYSCLK divided by 64 */
#define RCC_SYSCLK_DIV128              RCC_CFGR_HPRE_DIV128 /*!< SYSCLK divided by 128 */
#define RCC_SYSCLK_DIV256              RCC_CFGR_HPRE_DIV256 /*!< SYSCLK divided by 256 */
#define RCC_SYSCLK_DIV512              RCC_CFGR_HPRE_DIV512 /*!< SYSCLK divided by 512 */
/**
  * @}
  */

/** @defgroup RCC_APB1_APB2_Clock_Source APB1 APB2 Clock Source
  * @{
  */
#define RCC_HCLK_DIV1                  RCC_CFGR_PPRE1_DIV1  /*!< HCLK not divided */
#define RCC_HCLK_DIV2                  RCC_CFGR_PPRE1_DIV2  /*!< HCLK divided by 2 */
#define RCC_HCLK_DIV4                  RCC_CFGR_PPRE1_DIV4  /*!< HCLK divided by 4 */
#define RCC_HCLK_DIV8                  RCC_CFGR_PPRE1_DIV8  /*!< HCLK divided by 8 */
#define RCC_HCLK_DIV16                 RCC_CFGR_PPRE1_DIV16 /*!< HCLK divided by 16 */
/**
  * @}
  */

/** @defgroup RCC_RTC_Clock_Source RTC Clock Source
  * @{
  */
#define RCC_RTCCLKSOURCE_NONE          0x00000000U          /*!< No clock used as RTC clock */
#define RCC_RTCCLKSOURCE_LSE           RCC_BDCR_RTCSEL_0    /*!< LSE oscillator clock used as RTC clock */
#define RCC_RTCCLKSOURCE_LSI           RCC_BDCR_RTCSEL_1    /*!< LSI oscillator clock used as RTC clock */
#define RCC_RTCCLKSOURCE_HSE_DIV32     RCC_BDCR_RTCSEL      /*!< HSE oscillator clock divided by 32 used as RTC clock */
/**
  * @}
  */

/** @defgroup RCC_MCO_Index MCO Index
  * @{
  */
#define RCC_MCO1                       0x00000000U
#define RCC_MCO                        RCC_MCO1      /*!< MCO1 to be compliant with other families with 2 MCOs*/
/**
  * @}
  */

/** @defgroup RCC_MCO1_Clock_Source MCO1 Clock Source
  * @{
  */
#define RCC_MCO1SOURCE_NOCLOCK         0x00000000U                            /*!< MCO1 output disabled, no clock on MCO1 */
#define RCC_MCO1SOURCE_SYSCLK          RCC_CFGR_MCOSEL_0                      /*!< SYSCLK selection as MCO1 source */
#define RCC_MCO1SOURCE_MSI             RCC_CFGR_MCOSEL_1                      /*!< MSI selection as MCO1 source */
#define RCC_MCO1SOURCE_HSI             (RCC_CFGR_MCOSEL_0| RCC_CFGR_MCOSEL_1) /*!< HSI selection as MCO1 source */
#define RCC_MCO1SOURCE_HSE             RCC_CFGR_MCOSEL_2                      /*!< HSE selection as MCO1 source */
#define RCC_MCO1SOURCE_PLLCLK          (RCC_CFGR_MCOSEL_0|RCC_CFGR_MCOSEL_2)  /*!< PLLCLK selection as MCO1 source */
#define RCC_MCO1SOURCE_LSI             (RCC_CFGR_MCOSEL_1|RCC_CFGR_MCOSEL_2)  /*!< LSI selection as MCO1 source */
#define RCC_MCO1SOURCE_LSE             (RCC_CFGR_MCOSEL_0|RCC_CFGR_MCOSEL_1|RCC_CFGR_MCOSEL_2) /*!< LSE selection as MCO1 source */
#if defined(RCC_HSI48_SUPPORT)
#define RCC_MCO1SOURCE_HSI48           RCC_CFGR_MCOSEL_3                      /*!< HSI48 selection as MCO1 source (STM32L43x/STM32L44x devices) */
#endif /* RCC_HSI48_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_MCOx_Clock_Prescaler MCO1 Clock Prescaler
  * @{
  */
#define RCC_MCODIV_1                   RCC_CFGR_MCOPRE_DIV1     /*!< MCO not divided  */
#define RCC_MCODIV_2                   RCC_CFGR_MCOPRE_DIV2     /*!< MCO divided by 2 */
#define RCC_MCODIV_4                   RCC_CFGR_MCOPRE_DIV4     /*!< MCO divided by 4 */
#define RCC_MCODIV_8                   RCC_CFGR_MCOPRE_DIV8     /*!< MCO divided by 8 */
#define RCC_MCODIV_16                  RCC_CFGR_MCOPRE_DIV16    /*!< MCO divided by 16 */
/**
  * @}
  */

/** @defgroup RCC_Interrupt Interrupts
  * @{
  */
#define RCC_IT_LSIRDY                  RCC_CIFR_LSIRDYF      /*!< LSI Ready Interrupt flag */
#define RCC_IT_LSERDY                  RCC_CIFR_LSERDYF      /*!< LSE Ready Interrupt flag */
#define RCC_IT_MSIRDY                  RCC_CIFR_MSIRDYF      /*!< MSI Ready Interrupt flag */
#define RCC_IT_HSIRDY                  RCC_CIFR_HSIRDYF      /*!< HSI16 Ready Interrupt flag */
#define RCC_IT_HSERDY                  RCC_CIFR_HSERDYF      /*!< HSE Ready Interrupt flag */
#define RCC_IT_PLLRDY                  RCC_CIFR_PLLRDYF      /*!< PLL Ready Interrupt flag */
#if defined(RCC_PLLSAI1_SUPPORT)
#define RCC_IT_PLLSAI1RDY              RCC_CIFR_PLLSAI1RDYF  /*!< PLLSAI1 Ready Interrupt flag */
#endif /* RCC_PLLSAI1_SUPPORT */
#if defined(RCC_PLLSAI2_SUPPORT)
#define RCC_IT_PLLSAI2RDY              RCC_CIFR_PLLSAI2RDYF  /*!< PLLSAI2 Ready Interrupt flag */
#endif /* RCC_PLLSAI2_SUPPORT */
#define RCC_IT_CSS                     RCC_CIFR_CSSF        /*!< Clock Security System Interrupt flag */
#define RCC_IT_LSECSS                  RCC_CIFR_LSECSSF     /*!< LSE Clock Security System Interrupt flag */
#if defined(RCC_HSI48_SUPPORT)
#define RCC_IT_HSI48RDY                RCC_CIFR_HSI48RDYF   /*!< HSI48 Ready Interrupt flag */
#endif /* RCC_HSI48_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_Flag Flags
  *        Elements values convention: XXXYYYYYb
  *           - YYYYY  : Flag position in the register
  *           - XXX  : Register index
  *                 - 001: CR register
  *                 - 010: BDCR register
  *                 - 011: CSR register
  *                 - 100: CRRCR register
  * @{
  */
/* Flags in the CR register */
#define RCC_FLAG_MSIRDY                ((CR_REG_INDEX << 5U) | RCC_CR_MSIRDY_Pos) /*!< MSI Ready flag */
#define RCC_FLAG_HSIRDY                ((CR_REG_INDEX << 5U) | RCC_CR_HSIRDY_Pos) /*!< HSI Ready flag */
#define RCC_FLAG_HSERDY                ((CR_REG_INDEX << 5U) | RCC_CR_HSERDY_Pos) /*!< HSE Ready flag */
#define RCC_FLAG_PLLRDY                ((CR_REG_INDEX << 5U) | RCC_CR_PLLRDY_Pos) /*!< PLL Ready flag */
#if defined(RCC_PLLSAI1_SUPPORT)
#define RCC_FLAG_PLLSAI1RDY            ((CR_REG_INDEX << 5U) | RCC_CR_PLLSAI1RDY_Pos) /*!< PLLSAI1 Ready flag */
#endif /* RCC_PLLSAI1_SUPPORT */
#if defined(RCC_PLLSAI2_SUPPORT)
#define RCC_FLAG_PLLSAI2RDY            ((CR_REG_INDEX << 5U) | RCC_CR_PLLSAI2RDY_Pos) /*!< PLLSAI2 Ready flag */
#endif /* RCC_PLLSAI2_SUPPORT */

/* Flags in the BDCR register */
#define RCC_FLAG_LSERDY                ((BDCR_REG_INDEX << 5U) | RCC_BDCR_LSERDY_Pos)  /*!< LSE Ready flag */
#define RCC_FLAG_LSECSSD               ((BDCR_REG_INDEX << 5U) | RCC_BDCR_LSECSSD_Pos) /*!< LSE Clock Security System Interrupt flag */

/* Flags in the CSR register */
#define RCC_FLAG_LSIRDY                ((CSR_REG_INDEX << 5U) | RCC_CSR_LSIRDY_Pos)    /*!< LSI Ready flag */
#define RCC_FLAG_FWRST                 ((CSR_REG_INDEX << 5U) | RCC_CSR_FWRSTF_Pos)    /*!< Firewall reset flag */
#define RCC_FLAG_OBLRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_OBLRSTF_Pos)   /*!< Option Byte Loader reset flag */
#define RCC_FLAG_PINRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_PINRSTF_Pos)   /*!< PIN reset flag */
#define RCC_FLAG_BORRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_BORRSTF_Pos)   /*!< BOR reset flag */
#define RCC_FLAG_SFTRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_SFTRSTF_Pos)   /*!< Software Reset flag */
#define RCC_FLAG_IWDGRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_IWDGRSTF_Pos)  /*!< Independent Watchdog reset flag */
#define RCC_FLAG_WWDGRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_WWDGRSTF_Pos)  /*!< Window watchdog reset flag */
#define RCC_FLAG_LPWRRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_LPWRRSTF_Pos)  /*!< Low-Power reset flag */

#if defined(RCC_HSI48_SUPPORT)
/* Flags in the CRRCR register */
#define RCC_FLAG_HSI48RDY              ((CRRCR_REG_INDEX << 5U) | RCC_CRRCR_HSI48RDY_Pos) /*!< HSI48 Ready flag */
#endif /* RCC_HSI48_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_LSEDrive_Config LSE Drive Config
  * @{
  */
#define RCC_LSEDRIVE_LOW                 0x00000000U         /*!< LSE low drive capability */
#define RCC_LSEDRIVE_MEDIUMLOW           RCC_BDCR_LSEDRV_0   /*!< LSE medium low drive capability */
#define RCC_LSEDRIVE_MEDIUMHIGH          RCC_BDCR_LSEDRV_1   /*!< LSE medium high drive capability */
#define RCC_LSEDRIVE_HIGH                RCC_BDCR_LSEDRV     /*!< LSE high drive capability */
/**
  * @}
  */

/** @defgroup RCC_Stop_WakeUpClock Wake-Up from STOP Clock
  * @{
  */
#define RCC_STOP_WAKEUPCLOCK_MSI       0x00000000U           /*!< MSI selection after wake-up from STOP */
#define RCC_STOP_WAKEUPCLOCK_HSI       RCC_CFGR_STOPWUCK     /*!< HSI selection after wake-up from STOP */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/

/** @defgroup RCC_Exported_Macros RCC Exported Macros
  * @{
  */

/** @defgroup RCC_AHB1_Peripheral_Clock_Enable_Disable AHB1 Peripheral Clock Enable Disable
  * @brief  Enable or disable the AHB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_DMA1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_DMA2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_CLK_ENABLE()         do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_CRC_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TSC_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_TSCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_TSCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2DEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2DEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GFXMMUEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GFXMMUEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* GFXMMU */


#define __HAL_RCC_DMA1_CLK_DISABLE()           CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN)

#define __HAL_RCC_DMA2_CLK_DISABLE()           CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_CLK_DISABLE()        CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_CLK_DISABLE()          CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN)

#define __HAL_RCC_CRC_CLK_DISABLE()            CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN)

#define __HAL_RCC_TSC_CLK_DISABLE()            CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_TSCEN)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_CLK_DISABLE()          CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2DEN)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_CLK_DISABLE()         CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GFXMMUEN)
#endif /* GFXMMU */

/**
  * @}
  */

/** @defgroup RCC_AHB2_Peripheral_Clock_Enable_Disable AHB2 Peripheral Clock Enable Disable
  * @brief  Enable or disable the AHB2 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_GPIOA_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_GPIOB_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_GPIOC_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOHEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOHEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOIEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOIEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* GPIOI */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_CLK_ENABLE()      do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(DCMI)
#define __HAL_RCC_DCMI_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DCMIEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DCMIEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_AESEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_AESEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_HASHEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_HASHEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* HASH */

#define __HAL_RCC_RNG_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_RNGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_RNGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OSPIMEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OSPIMEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2ENR_SDMMC1EN)
#define __HAL_RCC_SDMMC1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_SDMMC1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_SDMMC1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* SDMMC1 && RCC_AHB2ENR_SDMMC1EN */


#define __HAL_RCC_GPIOA_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN)

#define __HAL_RCC_GPIOB_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN)

#define __HAL_RCC_GPIOC_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOGEN)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOHEN)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOIEN)
#endif /* GPIOI */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_CLK_DISABLE()     CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_CLK_DISABLE()            CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADCEN)

#if defined(DCMI)
#define __HAL_RCC_DCMI_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DCMIEN)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_CLK_DISABLE()            CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_AESEN);
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_HASHEN)
#endif /* HASH */

#define __HAL_RCC_RNG_CLK_DISABLE()            CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_RNGEN)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OSPIMEN)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2ENR_SDMMC1EN)
#define __HAL_RCC_SDMMC1_CLK_DISABLE()         CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_SDMMC1EN)
#endif /* SDMMC1 && RCC_AHB2ENR_SDMMC1EN */

/**
  * @}
  */

/** @defgroup RCC_AHB3_Clock_Enable_Disable AHB3 Peripheral Clock Enable Disable
  * @brief  Enable or disable the AHB3 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* FMC_BANK1 */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB3ENR, RCC_AHB3ENR_QSPIEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_QSPIEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB3ENR, RCC_AHB3ENR_OSPI1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_OSPI1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB3ENR, RCC_AHB3ENR_OSPI2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_OSPI2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* OCTOSPI2 */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_CLK_DISABLE()            CLEAR_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN)
#endif /* FMC_BANK1 */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_CLK_DISABLE()           CLEAR_BIT(RCC->AHB3ENR, RCC_AHB3ENR_QSPIEN)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_CLK_DISABLE()          CLEAR_BIT(RCC->AHB3ENR, RCC_AHB3ENR_OSPI1EN)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_CLK_DISABLE()          CLEAR_BIT(RCC->AHB3ENR, RCC_AHB3ENR_OSPI2EN)
#endif /* OCTOSPI2 */

/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Enable_Disable APB1 Peripheral Clock Enable Disable
  * @brief  Enable or disable the APB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_TIM2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(TIM3)
#define __HAL_RCC_TIM3_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(TIM7)
#define __HAL_RCC_TIM7_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LCDEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LCDEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* LCD */

#if defined(RCC_APB1ENR1_RTCAPBEN)
#define __HAL_RCC_RTCAPB_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_RTCAPBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_RTCAPBEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* RCC_APB1ENR1_RTCAPBEN */

#define __HAL_RCC_WWDG_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(SPI2)
#define __HAL_RCC_SPI2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* SPI3 */

#define __HAL_RCC_USART2_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(USART3)
#define __HAL_RCC_USART3_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* UART5 */

#define __HAL_RCC_I2C1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(I2C2)
#define __HAL_RCC_I2C2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(I2C4)
#define __HAL_RCC_I2C4_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR2, RCC_APB1ENR2_I2C4EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_I2C4EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CRSEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CRSEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBFSEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBFSEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* USB */

#define __HAL_RCC_PWR_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(DAC1)
#define __HAL_RCC_DAC1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_DAC1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_DAC1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_OPAMPEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_OPAMPEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_LPTIM1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LPTIM1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LPTIM1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_LPUART1_CLK_ENABLE()         do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR2, RCC_APB1ENR2_SWPMI1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_SWPMI1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPTIM2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPTIM2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)


#define __HAL_RCC_TIM2_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN)

#if defined(TIM3)
#define __HAL_RCC_TIM3_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN)

#if defined(TIM7)
#define __HAL_RCC_TIM7_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LCDEN);
#endif /* LCD */

#if defined(RCC_APB1ENR1_RTCAPBEN)
#define __HAL_RCC_RTCAPB_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_RTCAPBEN);
#endif /* RCC_APB1ENR1_RTCAPBEN */

#if defined(SPI2)
#define __HAL_RCC_SPI2_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI3EN)
#endif /* SPI3 */

#define __HAL_RCC_USART2_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN)

#if defined(USART3)
#define __HAL_RCC_USART3_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_CLK_DISABLE()          CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_CLK_DISABLE()          CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN)
#endif /* UART5 */

#define __HAL_RCC_I2C1_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN)

#if defined(I2C2)
#define __HAL_RCC_I2C2_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN)

#if defined(I2C4)
#define __HAL_RCC_I2C4_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR2, RCC_APB1ENR2_I2C4EN)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CRSEN);
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN1EN)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN2EN)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBFSEN);
#endif /* USB */

#define __HAL_RCC_PWR_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN)

#if defined(DAC1)
#define __HAL_RCC_DAC1_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_DAC1EN)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_CLK_DISABLE()          CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_OPAMPEN)

#define __HAL_RCC_LPTIM1_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LPTIM1EN)

#define __HAL_RCC_LPUART1_CLK_DISABLE()        CLEAR_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR2, RCC_APB1ENR2_SWPMI1EN)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPTIM2EN)

/**
  * @}
  */

/** @defgroup RCC_APB2_Clock_Enable_Disable APB2 Peripheral Clock Enable Disable
  * @brief  Enable or disable the APB2 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_SYSCFG_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_FIREWALL_CLK_ENABLE()        do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_FWEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_FWEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(SDMMC1) && defined(RCC_APB2ENR_SDMMC1EN)
#define __HAL_RCC_SDMMC1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SDMMC1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SDMMC1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* SDMMC1 && RCC_APB2ENR_SDMMC1EN */

#define __HAL_RCC_TIM1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_SPI1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(TIM8)
#define __HAL_RCC_TIM8_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* TIM8 */

#define __HAL_RCC_USART1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)


#define __HAL_RCC_TIM15_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM16_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#if defined(TIM17)
#define __HAL_RCC_TIM17_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_LTDCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_LTDCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_DSIEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DSIEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)
#endif /* DSI */


#define __HAL_RCC_SYSCFG_CLK_DISABLE()         CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN)

#if defined(SDMMC1) && defined(RCC_APB2ENR_SDMMC1EN)
#define __HAL_RCC_SDMMC1_CLK_DISABLE()         CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_SDMMC1EN)
#endif /* SDMMC1 && RCC_APB2ENR_SDMMC1EN */

#define __HAL_RCC_TIM1_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN)

#define __HAL_RCC_SPI1_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN)

#if defined(TIM8)
#define __HAL_RCC_TIM8_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN)
#endif /* TIM8 */

#define __HAL_RCC_USART1_CLK_DISABLE()         CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN)

#define __HAL_RCC_TIM15_CLK_DISABLE()          CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN)

#define __HAL_RCC_TIM16_CLK_DISABLE()          CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN)

#if defined(TIM17)
#define __HAL_RCC_TIM17_CLK_DISABLE()          CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI2EN)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_CLK_DISABLE()         CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_LTDCEN)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_CLK_DISABLE()            CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_DSIEN)
#endif /* DSI */

/**
  * @}
  */

/** @defgroup RCC_AHB1_Peripheral_Clock_Enable_Disable_Status AHB1 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the AHB1 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_DMA1_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN) != 0U)

#define __HAL_RCC_DMA2_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN) != 0U)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_IS_CLK_ENABLED()     (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN) != 0U)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN) != 0U)

#define __HAL_RCC_CRC_IS_CLK_ENABLED()         (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN) != 0U)

#define __HAL_RCC_TSC_IS_CLK_ENABLED()         (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_TSCEN) != 0U)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2DEN) != 0U)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_IS_CLK_ENABLED()      (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GFXMMUEN) != 0U)
#endif /* GFXMMU */


#define __HAL_RCC_DMA1_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN) == 0U)

#define __HAL_RCC_DMA2_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN) == 0U)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_IS_CLK_DISABLED()    (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN) == 0U)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN) == 0U)

#define __HAL_RCC_CRC_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN) == 0U)

#define __HAL_RCC_TSC_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_TSCEN) == 0U)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2DEN) == 0U)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_IS_CLK_DISABLED()     (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GFXMMUEN) == 0U)
#endif /* GFXMMU */

/**
  * @}
  */

/** @defgroup RCC_AHB2_Clock_Enable_Disable_Status AHB2 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the AHB2 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_GPIOA_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN) != 0U)

#define __HAL_RCC_GPIOB_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN) != 0U)

#define __HAL_RCC_GPIOC_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN) != 0U)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN) != 0U)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN) != 0U)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN) != 0U)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOGEN) != 0U)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOHEN) != 0U)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOIEN) != 0U)
#endif /* GPIOI */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_IS_CLK_ENABLED()  (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN) != 0U)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_IS_CLK_ENABLED()         (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADCEN) != 0U)

#if defined(DCMI)
#define __HAL_RCC_DCMI_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DCMIEN) != 0U)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_IS_CLK_ENABLED()         (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_AESEN) != 0U)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_HASHEN) != 0U)
#endif /* HASH */

#define __HAL_RCC_RNG_IS_CLK_ENABLED()         (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_RNGEN) != 0U)


#define __HAL_RCC_GPIOA_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN) == 0U)

#define __HAL_RCC_GPIOB_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN) == 0U)

#define __HAL_RCC_GPIOC_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN) == 0U)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN) == 0U)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN) == 0U)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN) == 0U)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOGEN) == 0U)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOHEN) == 0U)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOIEN) == 0U)
#endif /* GPIOI */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_IS_CLK_DISABLED() (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN) == 0U)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADCEN) == 0U)

#if defined(DCMI)
#define __HAL_RCC_DCMI_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DCMIEN) == 0U)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_AESEN) == 0U)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_HASHEN) == 0U)
#endif /* HASH */

#define __HAL_RCC_RNG_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_RNGEN) == 0U)

/**
  * @}
  */

/** @defgroup RCC_AHB3_Clock_Enable_Disable_Status AHB3 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the AHB3 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_IS_CLK_ENABLED()         (READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN) != 0U)
#endif /* FMC_BANK1 */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_QSPIEN) != 0U)
#endif /* QUADSPI */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN) == 0U)
#endif /* FMC_BANK1 */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_QSPIEN) == 0U)
#endif /* QUADSPI */

/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Enable_Disable_Status APB1 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the APB1 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_TIM2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN) != 0U)

#if defined(TIM3)
#define __HAL_RCC_TIM3_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN) != 0U)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN) != 0U)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN) != 0U)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN) != 0U)

#if defined(TIM7)
#define __HAL_RCC_TIM7_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN) != 0U)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LCDEN) != 0U)
#endif /* LCD */

#if defined(RCC_APB1ENR1_RTCAPBEN)
#define __HAL_RCC_RTCAPB_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_RTCAPBEN) != 0U)
#endif /* RCC_APB1ENR1_RTCAPBEN */

#define __HAL_RCC_WWDG_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN) != 0U)

#if defined(SPI2)
#define __HAL_RCC_SPI2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN) != 0U)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI3EN) != 0U)
#endif /* SPI3 */

#define __HAL_RCC_USART2_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN) != 0U)

#if defined(USART3)
#define __HAL_RCC_USART3_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN) != 0U)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_IS_CLK_ENABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN) != 0U)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_IS_CLK_ENABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN) != 0U)
#endif /* UART5 */

#define __HAL_RCC_I2C1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN) != 0U)

#if defined(I2C2)
#define __HAL_RCC_I2C2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN) != 0U)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN) != 0U)

#if defined(I2C4)
#define __HAL_RCC_I2C4_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_I2C4EN) != 0U)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CRSEN) != 0U)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN1EN) != 0U)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN2EN) != 0U)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBFSEN) != 0U)
#endif /* USB */

#define __HAL_RCC_PWR_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN) != 0U)

#if defined(DAC1)
#define __HAL_RCC_DAC1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_DAC1EN) != 0U)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_IS_CLK_ENABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_OPAMPEN) != 0U)

#define __HAL_RCC_LPTIM1_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LPTIM1EN) != 0U)

#define __HAL_RCC_LPUART1_IS_CLK_ENABLED()     (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN) != 0U)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_SWPMI1EN) != 0U)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPTIM2EN) != 0U)


#define __HAL_RCC_TIM2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN) == 0U)

#if defined(TIM3)
#define __HAL_RCC_TIM3_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN) == 0U)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN) == 0U)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN) == 0U)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN) == 0U)

#if defined(TIM7)
#define __HAL_RCC_TIM7_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN) == 0U)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LCDEN) == 0U)
#endif /* LCD */

#if defined(RCC_APB1ENR1_RTCAPBEN)
#define __HAL_RCC_RTCAPB_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_RTCAPBEN) == 0U)
#endif /* RCC_APB1ENR1_RTCAPBEN */

#define __HAL_RCC_WWDG_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN) == 0U)

#if defined(SPI2)
#define __HAL_RCC_SPI2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN) == 0U)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI3EN) == 0U)
#endif /* SPI3 */

#define __HAL_RCC_USART2_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN) == 0U)

#if defined(USART3)
#define __HAL_RCC_USART3_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN) == 0U)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_IS_CLK_DISABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN) == 0U)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_IS_CLK_DISABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN) == 0U)
#endif /* UART5 */

#define __HAL_RCC_I2C1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN) == 0U)

#if defined(I2C2)
#define __HAL_RCC_I2C2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN) == 0U)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN) == 0U)

#if defined(I2C4)
#define __HAL_RCC_I2C4_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_I2C4EN) == 0U)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CRSEN) == 0U)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN1EN) == 0U)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CAN2EN) == 0U)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBFSEN) == 0U)
#endif /* USB */

#define __HAL_RCC_PWR_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN) == 0U)

#if defined(DAC1)
#define __HAL_RCC_DAC1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_DAC1EN) == 0U)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_IS_CLK_DISABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_OPAMPEN) == 0U)

#define __HAL_RCC_LPTIM1_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LPTIM1EN) == 0U)

#define __HAL_RCC_LPUART1_IS_CLK_DISABLED()    (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN) == 0U)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_SWPMI1EN) == 0U)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR2, RCC_APB1ENR2_LPTIM2EN) == 0U)

/**
  * @}
  */

/** @defgroup RCC_APB2_Clock_Enable_Disable_Status APB2 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the APB2 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_SYSCFG_IS_CLK_ENABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN) != 0U)

#define __HAL_RCC_FIREWALL_IS_CLK_ENABLED()    (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_FWEN) != 0U)

#if defined(SDMMC1) && defined(RCC_APB2ENR_SDMMC1EN)
#define __HAL_RCC_SDMMC1_IS_CLK_ENABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SDMMC1EN) != 0U)
#endif /* SDMMC1 && RCC_APB2ENR_SDMMC1EN */

#define __HAL_RCC_TIM1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN) != 0U)

#define __HAL_RCC_SPI1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN) != 0U)

#if defined(TIM8)
#define __HAL_RCC_TIM8_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN) != 0U)
#endif /* TIM8 */

#define __HAL_RCC_USART1_IS_CLK_ENABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN) != 0U)

#define __HAL_RCC_TIM15_IS_CLK_ENABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN) != 0U)

#define __HAL_RCC_TIM16_IS_CLK_ENABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN) != 0U)

#if defined(TIM17)
#define __HAL_RCC_TIM17_IS_CLK_ENABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN) != 0U)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN) != 0U)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI2EN) != 0U)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_IS_CLK_ENABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN) != 0U)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_LTDCEN) != 0U)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_IS_CLK_ENABLED()         (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DSIEN) != 0U)
#endif /* DSI */


#define __HAL_RCC_SYSCFG_IS_CLK_DISABLED()     (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN) == 0U)

#if defined(SDMMC1) && defined(RCC_APB2ENR_SDMMC1EN)
#define __HAL_RCC_SDMMC1_IS_CLK_DISABLED()     (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SDMMC1EN) == 0U)
#endif /* SDMMC1 && RCC_APB2ENR_SDMMC1EN */

#define __HAL_RCC_TIM1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN) == 0U)

#define __HAL_RCC_SPI1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN) == 0U)

#if defined(TIM8)
#define __HAL_RCC_TIM8_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN) == 0U)
#endif /* TIM8 */

#define __HAL_RCC_USART1_IS_CLK_DISABLED()     (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN) == 0U)

#define __HAL_RCC_TIM15_IS_CLK_DISABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN) == 0U)

#define __HAL_RCC_TIM16_IS_CLK_DISABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN) == 0U)

#if defined(TIM17)
#define __HAL_RCC_TIM17_IS_CLK_DISABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN) == 0U)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN) == 0U)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI2EN) == 0U)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_IS_CLK_DISABLED()     (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN) == 0U)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_LTDCEN) == 0U)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_IS_CLK_DISABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DSIEN) == 0U)
#endif /* DSI */

/**
  * @}
  */

/** @defgroup RCC_AHB1_Force_Release_Reset AHB1 Peripheral Force Release Reset
  * @brief  Force or release AHB1 peripheral reset.
  * @{
  */
#define __HAL_RCC_AHB1_FORCE_RESET()           WRITE_REG(RCC->AHB1RSTR, 0xFFFFFFFFU)

#define __HAL_RCC_DMA1_FORCE_RESET()           SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA1RST)

#define __HAL_RCC_DMA2_FORCE_RESET()           SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA2RST)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_FORCE_RESET()        SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMAMUX1RST)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_FORCE_RESET()          SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_FLASHRST)

#define __HAL_RCC_CRC_FORCE_RESET()            SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_CRCRST)

#define __HAL_RCC_TSC_FORCE_RESET()            SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_TSCRST)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_FORCE_RESET()          SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA2DRST)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_FORCE_RESET()         SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_GFXMMURST)
#endif /* GFXMMU */


#define __HAL_RCC_AHB1_RELEASE_RESET()         WRITE_REG(RCC->AHB1RSTR, 0x00000000U)

#define __HAL_RCC_DMA1_RELEASE_RESET()         CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA1RST)

#define __HAL_RCC_DMA2_RELEASE_RESET()         CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA2RST)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_RELEASE_RESET()      CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMAMUX1RST)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_RELEASE_RESET()        CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_FLASHRST)

#define __HAL_RCC_CRC_RELEASE_RESET()          CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_CRCRST)

#define __HAL_RCC_TSC_RELEASE_RESET()          CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_TSCRST)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_RELEASE_RESET()        CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA2DRST)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_RELEASE_RESET()       CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_GFXMMURST)
#endif /* GFXMMU */

/**
  * @}
  */

/** @defgroup RCC_AHB2_Force_Release_Reset AHB2 Peripheral Force Release Reset
  * @brief  Force or release AHB2 peripheral reset.
  * @{
  */
#define __HAL_RCC_AHB2_FORCE_RESET()           WRITE_REG(RCC->AHB2RSTR, 0xFFFFFFFFU)

#define __HAL_RCC_GPIOA_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOARST)

#define __HAL_RCC_GPIOB_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOBRST)

#define __HAL_RCC_GPIOC_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOCRST)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIODRST)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOERST)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOFRST)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOGRST)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOHRST)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOIRST)
#endif /* GPIOI */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_FORCE_RESET()     SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_OTGFSRST)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_FORCE_RESET()            SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADCRST)

#if defined(DCMI)
#define __HAL_RCC_DCMI_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DCMIRST)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_FORCE_RESET()            SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_AESRST)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_HASHRST)
#endif /* HASH */

#define __HAL_RCC_RNG_FORCE_RESET()            SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_RNGRST)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_OSPIMRST)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2RSTR_SDMMC1RST)
#define __HAL_RCC_SDMMC1_FORCE_RESET()         SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_SDMMC1RST)
#endif /* SDMMC1 && RCC_AHB2RSTR_SDMMC1RST */


#define __HAL_RCC_AHB2_RELEASE_RESET()         WRITE_REG(RCC->AHB2RSTR, 0x00000000U)

#define __HAL_RCC_GPIOA_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOARST)

#define __HAL_RCC_GPIOB_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOBRST)

#define __HAL_RCC_GPIOC_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOCRST)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIODRST)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOERST)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOFRST)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOGRST)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOHRST)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOIRST)
#endif /* GPIOI */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_RELEASE_RESET()   CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_OTGFSRST)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_RELEASE_RESET()          CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADCRST)

#if defined(DCMI)
#define __HAL_RCC_DCMI_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DCMIRST)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_RELEASE_RESET()          CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_AESRST)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_HASHRST)
#endif /* HASH */

#define __HAL_RCC_RNG_RELEASE_RESET()          CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_RNGRST)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_OSPIMRST)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2RSTR_SDMMC1RST)
#define __HAL_RCC_SDMMC1_RELEASE_RESET()       CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_SDMMC1RST)
#endif /* SDMMC1 && RCC_AHB2RSTR_SDMMC1RST */

/**
  * @}
  */

/** @defgroup RCC_AHB3_Force_Release_Reset AHB3 Peripheral Force Release Reset
  * @brief  Force or release AHB3 peripheral reset.
  * @{
  */
#define __HAL_RCC_AHB3_FORCE_RESET()           WRITE_REG(RCC->AHB3RSTR, 0xFFFFFFFFU)

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_FORCE_RESET()            SET_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_FMCRST)
#endif /* FMC_BANK1 */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_FORCE_RESET()           SET_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_QSPIRST)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_FORCE_RESET()          SET_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_OSPI1RST)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_FORCE_RESET()          SET_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_OSPI2RST)
#endif /* OCTOSPI2 */

#define __HAL_RCC_AHB3_RELEASE_RESET()         WRITE_REG(RCC->AHB3RSTR, 0x00000000U)

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_RELEASE_RESET()          CLEAR_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_FMCRST)
#endif /* FMC_BANK1 */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_RELEASE_RESET()         CLEAR_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_QSPIRST)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_RELEASE_RESET()        CLEAR_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_OSPI1RST)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_RELEASE_RESET()        CLEAR_BIT(RCC->AHB3RSTR, RCC_AHB3RSTR_OSPI2RST)
#endif /* OCTOSPI2 */

/**
  * @}
  */

/** @defgroup RCC_APB1_Force_Release_Reset APB1 Peripheral Force Release Reset
  * @brief  Force or release APB1 peripheral reset.
  * @{
  */
#define __HAL_RCC_APB1_FORCE_RESET()           WRITE_REG(RCC->APB1RSTR1, 0xFFFFFFFFU)

#define __HAL_RCC_TIM2_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM2RST)

#if defined(TIM3)
#define __HAL_RCC_TIM3_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM3RST)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM4RST)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM5RST)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM6RST)

#if defined(TIM7)
#define __HAL_RCC_TIM7_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM7RST)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_FORCE_RESET()            SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_LCDRST)
#endif /* LCD */

#if defined(SPI2)
#define __HAL_RCC_SPI2_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_SPI2RST)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_SPI3RST)
#endif /* SPI3 */

#define __HAL_RCC_USART2_FORCE_RESET()         SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST)

#if defined(USART3)
#define __HAL_RCC_USART3_FORCE_RESET()         SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_USART3RST)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_FORCE_RESET()          SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_UART4RST)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_FORCE_RESET()          SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_UART5RST)
#endif /* UART5 */

#define __HAL_RCC_I2C1_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_I2C1RST)

#if defined(I2C2)
#define __HAL_RCC_I2C2_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_I2C2RST)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_I2C3RST)

#if defined(I2C4)
#define __HAL_RCC_I2C4_FORCE_RESET()           SET_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_I2C4RST)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_FORCE_RESET()            SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_CRSRST)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_CAN1RST)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_CAN2RST)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_FORCE_RESET()            SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_USBFSRST)
#endif /* USB */

#define __HAL_RCC_PWR_FORCE_RESET()            SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_PWRRST)

#if defined(DAC1)
#define __HAL_RCC_DAC1_FORCE_RESET()           SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_DAC1RST)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_FORCE_RESET()          SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_OPAMPRST)

#define __HAL_RCC_LPTIM1_FORCE_RESET()         SET_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_LPTIM1RST)

#define __HAL_RCC_LPUART1_FORCE_RESET()        SET_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_LPUART1RST)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_FORCE_RESET()         SET_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_SWPMI1RST)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_FORCE_RESET()         SET_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_LPTIM2RST)


#define __HAL_RCC_APB1_RELEASE_RESET()         WRITE_REG(RCC->APB1RSTR1, 0x00000000U)

#define __HAL_RCC_TIM2_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM2RST)

#if defined(TIM3)
#define __HAL_RCC_TIM3_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM3RST)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM4RST)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM5RST)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM6RST)

#if defined(TIM7)
#define __HAL_RCC_TIM7_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_TIM7RST)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_LCDRST)
#endif /* LCD */

#if defined(SPI2)
#define __HAL_RCC_SPI2_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_SPI2RST)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_SPI3RST)
#endif /* SPI3 */

#define __HAL_RCC_USART2_RELEASE_RESET()       CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST)

#if defined(USART3)
#define __HAL_RCC_USART3_RELEASE_RESET()       CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_USART3RST)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_RELEASE_RESET()        CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_UART4RST)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_RELEASE_RESET()        CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_UART5RST)
#endif /* UART5 */

#define __HAL_RCC_I2C1_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_I2C1RST)

#if defined(I2C2)
#define __HAL_RCC_I2C2_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_I2C2RST)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_I2C3RST)

#if defined(I2C4)
#define __HAL_RCC_I2C4_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_I2C4RST)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_CRSRST)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_CAN1RST)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_CAN2RST)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_USBFSRST)
#endif /* USB */

#define __HAL_RCC_PWR_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_PWRRST)

#if defined(DAC1)
#define __HAL_RCC_DAC1_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_DAC1RST)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_RELEASE_RESET()        CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_OPAMPRST)

#define __HAL_RCC_LPTIM1_RELEASE_RESET()       CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_LPTIM1RST)

#define __HAL_RCC_LPUART1_RELEASE_RESET()      CLEAR_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_LPUART1RST)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_RELEASE_RESET()       CLEAR_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_SWPMI1RST)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_RELEASE_RESET()       CLEAR_BIT(RCC->APB1RSTR2, RCC_APB1RSTR2_LPTIM2RST)

/**
  * @}
  */

/** @defgroup RCC_APB2_Force_Release_Reset APB2 Peripheral Force Release Reset
  * @brief  Force or release APB2 peripheral reset.
  * @{
  */
#define __HAL_RCC_APB2_FORCE_RESET()           WRITE_REG(RCC->APB2RSTR, 0xFFFFFFFFU)

#define __HAL_RCC_SYSCFG_FORCE_RESET()         SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SYSCFGRST)

#if defined(SDMMC1) && defined(RCC_APB2RSTR_SDMMC1RST)
#define __HAL_RCC_SDMMC1_FORCE_RESET()         SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SDMMC1RST)
#endif /* SDMMC1 && RCC_APB2RSTR_SDMMC1RST */

#define __HAL_RCC_TIM1_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM1RST)

#define __HAL_RCC_SPI1_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SPI1RST)

#if defined(TIM8)
#define __HAL_RCC_TIM8_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM8RST)
#endif /* TIM8 */

#define __HAL_RCC_USART1_FORCE_RESET()         SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST)

#define __HAL_RCC_TIM15_FORCE_RESET()          SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM15RST)

#define __HAL_RCC_TIM16_FORCE_RESET()          SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM16RST)

#if defined(TIM17)
#define __HAL_RCC_TIM17_FORCE_RESET()          SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM17RST)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SAI1RST)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SAI2RST)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_FORCE_RESET()         SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_DFSDM1RST)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_LTDCRST)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_FORCE_RESET()            SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_DSIRST)
#endif /* DSI */


#define __HAL_RCC_APB2_RELEASE_RESET()         WRITE_REG(RCC->APB2RSTR, 0x00000000U)

#define __HAL_RCC_SYSCFG_RELEASE_RESET()       CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SYSCFGRST)

#if defined(SDMMC1) && defined(RCC_APB2RSTR_SDMMC1RST)
#define __HAL_RCC_SDMMC1_RELEASE_RESET()       CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SDMMC1RST)
#endif /* SDMMC1 && RCC_APB2RSTR_SDMMC1RST */

#define __HAL_RCC_TIM1_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM1RST)

#define __HAL_RCC_SPI1_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SPI1RST)

#if defined(TIM8)
#define __HAL_RCC_TIM8_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM8RST)
#endif /* TIM8 */

#define __HAL_RCC_USART1_RELEASE_RESET()       CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST)

#define __HAL_RCC_TIM15_RELEASE_RESET()        CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM15RST)

#define __HAL_RCC_TIM16_RELEASE_RESET()        CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM16RST)

#if defined(TIM17)
#define __HAL_RCC_TIM17_RELEASE_RESET()        CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM17RST)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SAI1RST)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SAI2RST)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_RELEASE_RESET()       CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_DFSDM1RST)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_LTDCRST)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_RELEASE_RESET()          CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_DSIRST)
#endif /* DSI */

/**
  * @}
  */

/** @defgroup RCC_AHB1_Clock_Sleep_Enable_Disable AHB1 Peripheral Clock Sleep Enable Disable
  * @brief  Enable or disable the AHB1 peripheral clock during Low Power (Sleep) mode.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_DMA1_CLK_SLEEP_ENABLE()      SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA1SMEN)

#define __HAL_RCC_DMA2_CLK_SLEEP_ENABLE()      SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2SMEN)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_CLK_SLEEP_ENABLE()   SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMAMUX1SMEN)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_FLASHSMEN)

#define __HAL_RCC_SRAM1_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_SRAM1SMEN)

#define __HAL_RCC_CRC_CLK_SLEEP_ENABLE()       SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_CRCSMEN)

#define __HAL_RCC_TSC_CLK_SLEEP_ENABLE()       SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_TSCSMEN)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2DSMEN)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_CLK_SLEEP_ENABLE()    SET_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_GFXMMUSMEN)
#endif /* GFXMMU */


#define __HAL_RCC_DMA1_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA1SMEN)

#define __HAL_RCC_DMA2_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2SMEN)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_CLK_SLEEP_DISABLE()  CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMAMUX1SMEN)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_FLASHSMEN)

#define __HAL_RCC_SRAM1_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_SRAM1SMEN)

#define __HAL_RCC_CRC_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_CRCSMEN)

#define __HAL_RCC_TSC_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_TSCSMEN)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2DSMEN)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_GFXMMUSMEN)
#endif /* GFXMMU */

/**
  * @}
  */

/** @defgroup RCC_AHB2_Clock_Sleep_Enable_Disable AHB2 Peripheral Clock Sleep Enable Disable
  * @brief  Enable or disable the AHB2 peripheral clock during Low Power (Sleep) mode.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_GPIOA_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOASMEN)

#define __HAL_RCC_GPIOB_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOBSMEN)

#define __HAL_RCC_GPIOC_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOCSMEN)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIODSMEN)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOESMEN)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOFSMEN)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOGSMEN)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOHSMEN)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOISMEN)
#endif /* GPIOI */

#define __HAL_RCC_SRAM2_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM2SMEN)

#if defined(SRAM3)
#define __HAL_RCC_SRAM3_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM3SMEN)
#endif /* SRAM3 */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_CLK_SLEEP_ENABLE()  SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OTGFSSMEN)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_CLK_SLEEP_ENABLE()       SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_ADCSMEN)

#if defined(DCMI)
#define __HAL_RCC_DCMI_CLK_SLEEP_ENABLE()      SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_DCMISMEN)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_CLK_SLEEP_ENABLE()       SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_AESSMEN)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_CLK_SLEEP_ENABLE()      SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_HASHSMEN)
#endif /* HASH */

#define __HAL_RCC_RNG_CLK_SLEEP_ENABLE()       SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_RNGSMEN)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OSPIMSMEN)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_CLK_SLEEP_ENABLE()    SET_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SDMMC1SMEN)
#endif /* SDMMC1 && RCC_AHB2SMENR_SDMMC1SMEN */


#define __HAL_RCC_GPIOA_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOASMEN)

#define __HAL_RCC_GPIOB_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOBSMEN)

#define __HAL_RCC_GPIOC_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOCSMEN)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIODSMEN)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOESMEN)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOFSMEN)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOGSMEN)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOHSMEN)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOISMEN)
#endif /* GPIOI */

#define __HAL_RCC_SRAM2_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM2SMEN)

#if defined(SRAM3)
#define __HAL_RCC_SRAM3_IS_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM3SMEN)
#endif /* SRAM3 */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_CLK_SLEEP_DISABLE() CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OTGFSSMEN)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_ADCSMEN)

#if defined(DCMI)
#define __HAL_RCC_DCMI_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_DCMISMEN)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_AESSMEN)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_HASHSMEN)
#endif /* HASH */

#define __HAL_RCC_RNG_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_RNGSMEN)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OSPIMSMEN)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SDMMC1SMEN)
#endif /* SDMMC1 && RCC_AHB2SMENR_SDMMC1SMEN */

/**
  * @}
  */

/** @defgroup RCC_AHB3_Clock_Sleep_Enable_Disable AHB3 Peripheral Clock Sleep Enable Disable
  * @brief  Enable or disable the AHB3 peripheral clock during Low Power (Sleep) mode.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_CLK_SLEEP_ENABLE()      SET_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_QSPISMEN)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI1SMEN)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_CLK_SLEEP_ENABLE()     SET_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI2SMEN)
#endif /* OCTOSPI2 */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_CLK_SLEEP_ENABLE()       SET_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_FMCSMEN)
#endif /* FMC_BANK1 */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_QSPISMEN)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI1SMEN)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI2SMEN)
#endif /* OCTOSPI2 */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_FMCSMEN)
#endif /* FMC_BANK1 */

/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Sleep_Enable_Disable APB1 Peripheral Clock Sleep Enable Disable
  * @brief  Enable or disable the APB1 peripheral clock during Low Power (Sleep) mode.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_TIM2_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM2SMEN)

#if defined(TIM3)
#define __HAL_RCC_TIM3_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM3SMEN)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM4SMEN)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM5SMEN)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM6SMEN)

#if defined(TIM7)
#define __HAL_RCC_TIM7_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM7SMEN)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_CLK_SLEEP_ENABLE()       SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LCDSMEN)
#endif /* LCD */

#if defined(RCC_APB1SMENR1_RTCAPBSMEN)
#define __HAL_RCC_RTCAPB_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_RTCAPBSMEN)
#endif /* RCC_APB1SMENR1_RTCAPBSMEN */

#define __HAL_RCC_WWDG_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_WWDGSMEN)

#if defined(SPI2)
#define __HAL_RCC_SPI2_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI2SMEN)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI3SMEN)
#endif /* SPI3 */

#define __HAL_RCC_USART2_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART2SMEN)

#if defined(USART3)
#define __HAL_RCC_USART3_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART3SMEN)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_CLK_SLEEP_ENABLE()     SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART4SMEN)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_CLK_SLEEP_ENABLE()     SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART5SMEN)
#endif /* UART5 */

#define __HAL_RCC_I2C1_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C1SMEN)

#if defined(I2C2)
#define __HAL_RCC_I2C2_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C2SMEN)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C3SMEN)

#if defined(I2C4)
#define __HAL_RCC_I2C4_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_I2C4SMEN)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_CLK_SLEEP_ENABLE()       SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CRSSMEN)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN1SMEN)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN2SMEN)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_CLK_SLEEP_ENABLE()       SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USBFSSMEN)
#endif /* USB */

#define __HAL_RCC_PWR_CLK_SLEEP_ENABLE()       SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_PWRSMEN)

#if defined(DAC1)
#define __HAL_RCC_DAC1_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_DAC1SMEN)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_CLK_SLEEP_ENABLE()     SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_OPAMPSMEN)

#define __HAL_RCC_LPTIM1_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LPTIM1SMEN)

#define __HAL_RCC_LPUART1_CLK_SLEEP_ENABLE()   SET_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPUART1SMEN)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_SWPMI1SMEN)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPTIM2SMEN)


#define __HAL_RCC_TIM2_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM2SMEN)

#if defined(TIM3)
#define __HAL_RCC_TIM3_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM3SMEN)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM4SMEN)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM5SMEN)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM6SMEN)

#if defined(TIM7)
#define __HAL_RCC_TIM7_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM7SMEN)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LCDSMEN)
#endif /* LCD */

#if defined(RCC_APB1SMENR1_RTCAPBSMEN)
#define __HAL_RCC_RTCAPB_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_RTCAPBSMEN)
#endif /* RCC_APB1SMENR1_RTCAPBSMEN */

#define __HAL_RCC_WWDG_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_WWDGSMEN)

#if defined(SPI2)
#define __HAL_RCC_SPI2_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI2SMEN)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI3SMEN)
#endif /* SPI3 */

#define __HAL_RCC_USART2_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART2SMEN)

#if defined(USART3)
#define __HAL_RCC_USART3_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART3SMEN)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART4SMEN)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART5SMEN)
#endif /* UART5 */

#define __HAL_RCC_I2C1_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C1SMEN)

#if defined(I2C2)
#define __HAL_RCC_I2C2_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C2SMEN)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C3SMEN)

#if defined(I2C4)
#define __HAL_RCC_I2C4_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_I2C4SMEN)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CRSSMEN)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN1SMEN)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN2SMEN)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USBFSSMEN)
#endif /* USB */

#define __HAL_RCC_PWR_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_PWRSMEN)

#if defined(DAC1)
#define __HAL_RCC_DAC1_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_DAC1SMEN)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_OPAMPSMEN)

#define __HAL_RCC_LPTIM1_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LPTIM1SMEN)

#define __HAL_RCC_LPUART1_CLK_SLEEP_DISABLE()  CLEAR_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPUART1SMEN)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_SWPMI1SMEN)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPTIM2SMEN)

/**
  * @}
  */

/** @defgroup RCC_APB2_Clock_Sleep_Enable_Disable APB2 Peripheral Clock Sleep Enable Disable
  * @brief  Enable or disable the APB2 peripheral clock during Low Power (Sleep) mode.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_SYSCFG_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SYSCFGSMEN)

#if defined(SDMMC1) && defined(RCC_APB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SDMMC1SMEN)
#endif /* SDMMC1 && RCC_APB2SMENR_SDMMC1SMEN */

#define __HAL_RCC_TIM1_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM1SMEN)

#define __HAL_RCC_SPI1_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SPI1SMEN)

#if defined(TIM8)
#define __HAL_RCC_TIM8_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM8SMEN)
#endif /* TIM8 */

#define __HAL_RCC_USART1_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_USART1SMEN)

#define __HAL_RCC_TIM15_CLK_SLEEP_ENABLE()     SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM15SMEN)

#define __HAL_RCC_TIM16_CLK_SLEEP_ENABLE()     SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM16SMEN)

#if defined(TIM17)
#define __HAL_RCC_TIM17_CLK_SLEEP_ENABLE()     SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM17SMEN)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI1SMEN)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI2SMEN)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_CLK_SLEEP_ENABLE()    SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DFSDM1SMEN)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_CLK_SLEEP_ENABLE()      SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_LTDCSMEN)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_CLK_SLEEP_ENABLE()       SET_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DSISMEN)
#endif /* DSI */


#define __HAL_RCC_SYSCFG_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SYSCFGSMEN)

#if defined(SDMMC1) && defined(RCC_APB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SDMMC1SMEN)
#endif /* SDMMC1 && RCC_APB2SMENR_SDMMC1SMEN */

#define __HAL_RCC_TIM1_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM1SMEN)

#define __HAL_RCC_SPI1_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SPI1SMEN)

#if defined(TIM8)
#define __HAL_RCC_TIM8_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM8SMEN)
#endif /* TIM8 */

#define __HAL_RCC_USART1_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_USART1SMEN)

#define __HAL_RCC_TIM15_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM15SMEN)

#define __HAL_RCC_TIM16_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM16SMEN)

#if defined(TIM17)
#define __HAL_RCC_TIM17_CLK_SLEEP_DISABLE()    CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM17SMEN)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI1SMEN)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI2SMEN)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_CLK_SLEEP_DISABLE()   CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DFSDM1SMEN)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_CLK_SLEEP_DISABLE()     CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_LTDCSMEN)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_CLK_SLEEP_DISABLE()      CLEAR_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DSISMEN)
#endif /* DSI */

/**
  * @}
  */

/** @defgroup RCC_AHB1_Clock_Sleep_Enable_Disable_Status AHB1 Peripheral Clock Sleep Enabled or Disabled Status
  * @brief  Check whether the AHB1 peripheral clock during Low Power (Sleep) mode is enabled or not.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_DMA1_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA1SMEN) != 0U)

#define __HAL_RCC_DMA2_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2SMEN) != 0U)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_IS_CLK_SLEEP_ENABLED() (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMAMUX1SMEN) != 0U)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_FLASHSMEN) != 0U)

#define __HAL_RCC_SRAM1_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_SRAM1SMEN) != 0U)

#define __HAL_RCC_CRC_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_CRCSMEN) != 0U)

#define __HAL_RCC_TSC_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_TSCSMEN) != 0U)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2DSMEN) != 0U)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_IS_CLK_SLEEP_ENABLED()  (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_GFXMMUSMEN) != 0U)
#endif /* GFXMMU */


#define __HAL_RCC_DMA1_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA1SMEN) == 0U)

#define __HAL_RCC_DMA2_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2SMEN) == 0U)

#if defined(DMAMUX1)
#define __HAL_RCC_DMAMUX1_IS_CLK_SLEEP_DISABLED() (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMAMUX1SMEN) == 0U)
#endif /* DMAMUX1 */

#define __HAL_RCC_FLASH_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_FLASHSMEN) == 0U)

#define __HAL_RCC_SRAM1_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_SRAM1SMEN) == 0U)

#define __HAL_RCC_CRC_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_CRCSMEN) == 0U)

#define __HAL_RCC_TSC_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_TSCSMEN) == 0U)

#if defined(DMA2D)
#define __HAL_RCC_DMA2D_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_DMA2DSMEN) == 0U)
#endif /* DMA2D */

#if defined(GFXMMU)
#define __HAL_RCC_GFXMMU_IS_CLK_SLEEP_DISABLED() (READ_BIT(RCC->AHB1SMENR, RCC_AHB1SMENR_GFXMMUSMEN) == 0U)
#endif /* GFXMMU */

/**
  * @}
  */

/** @defgroup RCC_AHB2_Clock_Sleep_Enable_Disable_Status AHB2 Peripheral Clock Sleep Enabled or Disabled Status
  * @brief  Check whether the AHB2 peripheral clock during Low Power (Sleep) mode is enabled or not.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_GPIOA_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOASMEN) != 0U)

#define __HAL_RCC_GPIOB_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOBSMEN) != 0U)

#define __HAL_RCC_GPIOC_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOCSMEN) != 0U)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIODSMEN) != 0U)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOESMEN) != 0U)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOFSMEN) != 0U)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOGSMEN) != 0U)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOHSMEN) != 0U)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOISMEN) != 0U)
#endif /* GPIOI */

#define __HAL_RCC_SRAM2_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM2SMEN) != 0U)

#if defined(SRAM3)
#define __HAL_RCC_SRAM3_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM3SMEN) != 0U)
#endif /* SRAM3 */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_IS_CLK_SLEEP_ENABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OTGFSSMEN) != 0U)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_ADCSMEN) != 0U)

#if defined(DCMI)
#define __HAL_RCC_DCMI_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_DCMISMEN) != 0U)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_AESSMEN) != 0U)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_HASHSMEN) != 0U)
#endif /* HASH */

#define __HAL_RCC_RNG_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_RNGSMEN) != 0U)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OSPIMSMEN) != 0U)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_IS_CLK_SLEEP_ENABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SDMMC1SMEN) != 0U)
#endif /* SDMMC1 && RCC_AHB2SMENR_SDMMC1SMEN */


#define __HAL_RCC_GPIOA_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOASMEN) == 0U)

#define __HAL_RCC_GPIOB_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOBSMEN) == 0U)

#define __HAL_RCC_GPIOC_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOCSMEN) == 0U)

#if defined(GPIOD)
#define __HAL_RCC_GPIOD_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIODSMEN) == 0U)
#endif /* GPIOD */

#if defined(GPIOE)
#define __HAL_RCC_GPIOE_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOESMEN) == 0U)
#endif /* GPIOE */

#if defined(GPIOF)
#define __HAL_RCC_GPIOF_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOFSMEN) == 0U)
#endif /* GPIOF */

#if defined(GPIOG)
#define __HAL_RCC_GPIOG_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOGSMEN) == 0U)
#endif /* GPIOG */

#define __HAL_RCC_GPIOH_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOHSMEN) == 0U)

#if defined(GPIOI)
#define __HAL_RCC_GPIOI_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_GPIOISMEN) == 0U)
#endif /* GPIOI */

#define __HAL_RCC_SRAM2_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM2SMEN) == 0U)

#if defined(SRAM3)
#define __HAL_RCC_SRAM3_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SRAM3SMEN) == 0U)
#endif /* SRAM3 */

#if defined(USB_OTG_FS)
#define __HAL_RCC_USB_OTG_FS_IS_CLK_SLEEP_DISABLED() (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OTGFSSMEN) == 0U)
#endif /* USB_OTG_FS */

#define __HAL_RCC_ADC_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_ADCSMEN) == 0U)

#if defined(DCMI)
#define __HAL_RCC_DCMI_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_DCMISMEN) == 0U)
#endif /* DCMI */

#if defined(AES)
#define __HAL_RCC_AES_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_AESSMEN) == 0U)
#endif /* AES */

#if defined(HASH)
#define __HAL_RCC_HASH_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_HASHSMEN) == 0U)
#endif /* HASH */

#define __HAL_RCC_RNG_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_RNGSMEN) == 0U)

#if defined(OCTOSPIM)
#define __HAL_RCC_OSPIM_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_OSPIMSMEN) == 0U)
#endif /* OCTOSPIM */

#if defined(SDMMC1) && defined(RCC_AHB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_IS_CLK_SLEEP_DISABLED() (READ_BIT(RCC->AHB2SMENR, RCC_AHB2SMENR_SDMMC1SMEN) == 0U)
#endif /* SDMMC1 && RCC_AHB2SMENR_SDMMC1SMEN */

/**
  * @}
  */

/** @defgroup RCC_AHB3_Clock_Sleep_Enable_Disable_Status AHB3 Peripheral Clock Sleep Enabled or Disabled Status
  * @brief  Check whether the AHB3 peripheral clock during Low Power (Sleep) mode is enabled or not.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#if defined(QUADSPI)
#define __HAL_RCC_QSPI_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_QSPISMEN) != 0U)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI1SMEN) != 0U)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI2SMEN) != 0U)
#endif /* OCTOSPI2 */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_FMCSMEN) != 0U)
#endif /* FMC_BANK1 */


#if defined(QUADSPI)
#define __HAL_RCC_QSPI_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_QSPISMEN) == 0U)
#endif /* QUADSPI */

#if defined(OCTOSPI1)
#define __HAL_RCC_OSPI1_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI1SMEN) == 0U)
#endif /* OCTOSPI1 */

#if defined(OCTOSPI2)
#define __HAL_RCC_OSPI2_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_OSPI2SMEN) == 0U)
#endif /* OCTOSPI2 */

#if defined(FMC_BANK1)
#define __HAL_RCC_FMC_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->AHB3SMENR, RCC_AHB3SMENR_FMCSMEN) == 0U)
#endif /* FMC_BANK1 */

/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Sleep_Enable_Disable_Status APB1 Peripheral Clock Sleep Enabled or Disabled Status
  * @brief  Check whether the APB1 peripheral clock during Low Power (Sleep) mode is enabled or not.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_TIM2_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM2SMEN) != 0U)

#if defined(TIM3)
#define __HAL_RCC_TIM3_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM3SMEN) != 0U)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM4SMEN) != 0U)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM5SMEN) != 0U)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM6SMEN) != 0U)

#if defined(TIM7)
#define __HAL_RCC_TIM7_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM7SMEN) != 0U)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_IS_CLK_SLEEP_ENABLED()       (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LCDSMEN) != 0U)
#endif /* LCD */

#if defined(RCC_APB1SMENR1_RTCAPBSMEN)
#define __HAL_RCC_RTCAPB_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_RTCAPBSMEN) != 0U)
#endif /* RCC_APB1SMENR1_RTCAPBSMEN */

#define __HAL_RCC_WWDG_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_WWDGSMEN) != 0U)

#if defined(SPI2)
#define __HAL_RCC_SPI2_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI2SMEN) != 0U)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI3SMEN) != 0U)
#endif /* SPI3 */

#define __HAL_RCC_USART2_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART2SMEN) != 0U)

#if defined(USART3)
#define __HAL_RCC_USART3_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART3SMEN) != 0U)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART4SMEN) != 0U)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART5SMEN) != 0U)
#endif /* UART5 */

#define __HAL_RCC_I2C1_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C1SMEN) != 0U)

#if defined(I2C2)
#define __HAL_RCC_I2C2_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C2SMEN) != 0U)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C3SMEN) != 0U)

#if defined(I2C4)
#define __HAL_RCC_I2C4_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_I2C4SMEN) != 0U)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_IS_CLK_SLEEP_ENABLED()       (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CRSSMEN) != 0U)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN1SMEN) != 0U)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN2SMEN) != 0U)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_IS_CLK_SLEEP_ENABLED()       (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USBFSSMEN) != 0U)
#endif /* USB */

#define __HAL_RCC_PWR_IS_CLK_SLEEP_ENABLED()       (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_PWRSMEN) != 0U)

#if defined(DAC1)
#define __HAL_RCC_DAC1_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_DAC1SMEN) != 0U)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_OPAMPSMEN) != 0U)

#define __HAL_RCC_LPTIM1_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LPTIM1SMEN) != 0U)

#define __HAL_RCC_LPUART1_IS_CLK_SLEEP_ENABLED()   (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPUART1SMEN) != 0U)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_SWPMI1SMEN) != 0U)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPTIM2SMEN) != 0U)


#define __HAL_RCC_TIM2_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM2SMEN) == 0U)

#if defined(TIM3)
#define __HAL_RCC_TIM3_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM3SMEN) == 0U)
#endif /* TIM3 */

#if defined(TIM4)
#define __HAL_RCC_TIM4_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM4SMEN) == 0U)
#endif /* TIM4 */

#if defined(TIM5)
#define __HAL_RCC_TIM5_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM5SMEN) == 0U)
#endif /* TIM5 */

#define __HAL_RCC_TIM6_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM6SMEN) == 0U)

#if defined(TIM7)
#define __HAL_RCC_TIM7_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_TIM7SMEN) == 0U)
#endif /* TIM7 */

#if defined(LCD)
#define __HAL_RCC_LCD_IS_CLK_SLEEP_DISABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LCDSMEN) == 0U)
#endif /* LCD */

#if defined(RCC_APB1SMENR1_RTCAPBSMEN)
#define __HAL_RCC_RTCAPB_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_RTCAPBSMEN) == 0U)
#endif /* RCC_APB1SMENR1_RTCAPBSMEN */

#define __HAL_RCC_WWDG_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_WWDGSMEN) == 0U)

#if defined(SPI2)
#define __HAL_RCC_SPI2_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI2SMEN) == 0U)
#endif /* SPI2 */

#if defined(SPI3)
#define __HAL_RCC_SPI3_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_SPI3SMEN) == 0U)
#endif /* SPI3 */

#define __HAL_RCC_USART2_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART2SMEN) == 0U)

#if defined(USART3)
#define __HAL_RCC_USART3_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USART3SMEN) == 0U)
#endif /* USART3 */

#if defined(UART4)
#define __HAL_RCC_UART4_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART4SMEN) == 0U)
#endif /* UART4 */

#if defined(UART5)
#define __HAL_RCC_UART5_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_UART5SMEN) == 0U)
#endif /* UART5 */

#define __HAL_RCC_I2C1_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C1SMEN) == 0U)

#if defined(I2C2)
#define __HAL_RCC_I2C2_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C2SMEN) == 0U)
#endif /* I2C2 */

#define __HAL_RCC_I2C3_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_I2C3SMEN) == 0U)

#if defined(I2C4)
#define __HAL_RCC_I2C4_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_I2C4SMEN) == 0U)
#endif /* I2C4 */

#if defined(CRS)
#define __HAL_RCC_CRS_IS_CLK_SLEEP_DISABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CRSSMEN) == 0U)
#endif /* CRS */

#if defined(CAN1)
#define __HAL_RCC_CAN1_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN1SMEN) == 0U)
#endif /* CAN1 */

#if defined(CAN2)
#define __HAL_RCC_CAN2_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_CAN2SMEN) == 0U)
#endif /* CAN2 */

#if defined(USB)
#define __HAL_RCC_USB_IS_CLK_SLEEP_DISABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_USBFSSMEN) == 0U)
#endif /* USB */

#define __HAL_RCC_PWR_IS_CLK_SLEEP_DISABLED()      (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_PWRSMEN) == 0U)

#if defined(DAC1)
#define __HAL_RCC_DAC1_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_DAC1SMEN) == 0U)
#endif /* DAC1 */

#define __HAL_RCC_OPAMP_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_OPAMPSMEN) == 0U)

#define __HAL_RCC_LPTIM1_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB1SMENR1, RCC_APB1SMENR1_LPTIM1SMEN) == 0U)

#define __HAL_RCC_LPUART1_IS_CLK_SLEEP_DISABLED()  (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPUART1SMEN) == 0U)

#if defined(SWPMI1)
#define __HAL_RCC_SWPMI1_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_SWPMI1SMEN) == 0U)
#endif /* SWPMI1 */

#define __HAL_RCC_LPTIM2_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB1SMENR2, RCC_APB1SMENR2_LPTIM2SMEN) == 0U)

/**
  * @}
  */

/** @defgroup RCC_APB2_Clock_Sleep_Enable_Disable_Status APB2 Peripheral Clock Sleep Enabled or Disabled Status
  * @brief  Check whether the APB2 peripheral clock during Low Power (Sleep) mode is enabled or not.
  * @note   Peripheral clock gating in SLEEP mode can be used to further reduce
  *         power consumption.
  * @note   After wakeup from SLEEP mode, the peripheral clock is enabled again.
  * @note   By default, all peripheral clocks are enabled during SLEEP mode.
  * @{
  */

#define __HAL_RCC_SYSCFG_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SYSCFGSMEN) != 0U)

#if defined(SDMMC1) && defined(RCC_APB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SDMMC1SMEN) != 0U)
#endif /* SDMMC1 && RCC_APB2SMENR_SDMMC1SMEN */

#define __HAL_RCC_TIM1_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM1SMEN) != 0U)

#define __HAL_RCC_SPI1_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SPI1SMEN) != 0U)

#if defined(TIM8)
#define __HAL_RCC_TIM8_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM8SMEN) != 0U)
#endif /* TIM8 */

#define __HAL_RCC_USART1_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_USART1SMEN) != 0U)

#define __HAL_RCC_TIM15_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM15SMEN) != 0U)

#define __HAL_RCC_TIM16_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM16SMEN) != 0U)

#if defined(TIM17)
#define __HAL_RCC_TIM17_IS_CLK_SLEEP_ENABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM17SMEN) != 0U)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI1SMEN) != 0U)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI2SMEN) != 0U)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_IS_CLK_SLEEP_ENABLED()    (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DFSDM1SMEN) != 0U)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_IS_CLK_SLEEP_ENABLED()      (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_LTDCSMEN) != 0U)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_IS_CLK_SLEEP_ENABLED()       (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DSISMEN) != 0U)
#endif /* DSI */


#define __HAL_RCC_SYSCFG_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SYSCFGSMEN) == 0U)

#if defined(SDMMC1) && defined(RCC_APB2SMENR_SDMMC1SMEN)
#define __HAL_RCC_SDMMC1_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SDMMC1SMEN) == 0U)
#endif /* SDMMC1 && RCC_APB2SMENR_SDMMC1SMEN */

#define __HAL_RCC_TIM1_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM1SMEN) == 0U)

#define __HAL_RCC_SPI1_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SPI1SMEN) == 0U)

#if defined(TIM8)
#define __HAL_RCC_TIM8_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM8SMEN) == 0U)
#endif /* TIM8 */

#define __HAL_RCC_USART1_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_USART1SMEN) == 0U)

#define __HAL_RCC_TIM15_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM15SMEN) == 0U)

#define __HAL_RCC_TIM16_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM16SMEN) == 0U)

#if defined(TIM17)
#define __HAL_RCC_TIM17_IS_CLK_SLEEP_DISABLED()    (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_TIM17SMEN) == 0U)
#endif /* TIM17 */

#if defined(SAI1)
#define __HAL_RCC_SAI1_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI1SMEN) == 0U)
#endif /* SAI1 */

#if defined(SAI2)
#define __HAL_RCC_SAI2_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_SAI2SMEN) == 0U)
#endif /* SAI2 */

#if defined(DFSDM1_Filter0)
#define __HAL_RCC_DFSDM1_IS_CLK_SLEEP_DISABLED()   (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DFSDM1SMEN) == 0U)
#endif /* DFSDM1_Filter0 */

#if defined(LTDC)
#define __HAL_RCC_LTDC_IS_CLK_SLEEP_DISABLED()     (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_LTDCSMEN) == 0U)
#endif /* LTDC */

#if defined(DSI)
#define __HAL_RCC_DSI_IS_CLK_SLEEP_DISABLED()      (READ_BIT(RCC->APB2SMENR, RCC_APB2SMENR_DSISMEN) == 0U)
#endif /* DSI */

/**
  * @}
  */

/** @defgroup RCC_Backup_Domain_Reset RCC Backup Domain Reset
  * @{
  */

/** @brief  Macros to force or release the Backup domain reset.
  * @note   This function resets the RTC peripheral (including the backup registers)
  *         and the RTC clock source selection in RCC_CSR register.
  * @note   The BKPSRAM is not affected by this reset.
  * @retval None
  */
#define __HAL_RCC_BACKUPRESET_FORCE()   SET_BIT(RCC->BDCR, RCC_BDCR_BDRST)

#define __HAL_RCC_BACKUPRESET_RELEASE() CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST)

/**
  * @}
  */

/** @defgroup RCC_RTC_Clock_Configuration RCC RTC Clock Configuration
  * @{
  */

/** @brief  Macros to enable or disable the RTC clock.
  * @note   As the RTC is in the Backup domain and write access is denied to
  *         this domain after reset, you have to enable write access using
  *         HAL_PWR_EnableBkUpAccess() function before to configure the RTC
  *         (to be done once after reset).
  * @note   These macros must be used after the RTC clock source was selected.
  * @retval None
  */
#define __HAL_RCC_RTC_ENABLE()         SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN)

#define __HAL_RCC_RTC_DISABLE()        CLEAR_BIT(RCC->BDCR, RCC_BDCR_RTCEN)

/**
  * @}
  */

/** @brief  Macros to enable or disable the Internal High Speed 16MHz oscillator (HSI).
  * @note   The HSI is stopped by hardware when entering STOP and STANDBY modes.
  *         It is used (enabled by hardware) as system clock source after startup
  *         from Reset, wakeup from STOP and STANDBY mode, or in case of failure
  *         of the HSE used directly or indirectly as system clock (if the Clock
  *         Security System CSS is enabled).
  * @note   HSI can not be stopped if it is used as system clock source. In this case,
  *         you have to select another source of the system clock then stop the HSI.
  * @note   After enabling the HSI, the application software should wait on HSIRDY
  *         flag to be set indicating that HSI clock is stable and can be used as
  *         system clock source.
  *         This parameter can be: ENABLE or DISABLE.
  * @note   When the HSI is stopped, HSIRDY flag goes low after 6 HSI oscillator
  *         clock cycles.
  * @retval None
  */
#define __HAL_RCC_HSI_ENABLE()  SET_BIT(RCC->CR, RCC_CR_HSION)

#define __HAL_RCC_HSI_DISABLE() CLEAR_BIT(RCC->CR, RCC_CR_HSION)

/** @brief  Macro to adjust the Internal High Speed 16MHz oscillator (HSI) calibration value.
  * @note   The calibration is used to compensate for the variations in voltage
  *         and temperature that influence the frequency of the internal HSI RC.
  * @param  __HSICALIBRATIONVALUE__ specifies the calibration trimming value
  *         (default is RCC_HSICALIBRATION_DEFAULT).
  *         This parameter must be a number between 0 and 0x1F (STM32L43x/STM32L44x/STM32L47x/STM32L48x) or 0x7F (for other devices).
  * @retval None
  */
#define __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(__HSICALIBRATIONVALUE__) \
                  MODIFY_REG(RCC->ICSCR, RCC_ICSCR_HSITRIM, (__HSICALIBRATIONVALUE__) << RCC_ICSCR_HSITRIM_Pos)

/**
  * @brief    Macros to enable or disable the wakeup the Internal High Speed oscillator (HSI)
  *           in parallel to the Internal Multi Speed oscillator (MSI) used at system wakeup.
  * @note     The enable of this function has not effect on the HSION bit.
  *           This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
#define __HAL_RCC_HSIAUTOMATIC_START_ENABLE()   SET_BIT(RCC->CR, RCC_CR_HSIASFS)

#define __HAL_RCC_HSIAUTOMATIC_START_DISABLE()  CLEAR_BIT(RCC->CR, RCC_CR_HSIASFS)

/**
  * @brief    Macros to enable or disable the force of the Internal High Speed oscillator (HSI)
  *           in STOP mode to be quickly available as kernel clock for USARTs and I2Cs.
  * @note     Keeping the HSI ON in STOP mode allows to avoid slowing down the communication
  *           speed because of the HSI startup time.
  * @note     The enable of this function has not effect on the HSION bit.
  *           This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
#define __HAL_RCC_HSISTOP_ENABLE()     SET_BIT(RCC->CR, RCC_CR_HSIKERON)

#define __HAL_RCC_HSISTOP_DISABLE()    CLEAR_BIT(RCC->CR, RCC_CR_HSIKERON)

/**
  * @brief  Macros to enable or disable the Internal Multi Speed oscillator (MSI).
  * @note     The MSI is stopped by hardware when entering STOP and STANDBY modes.
  *           It is used (enabled by hardware) as system clock source after
  *           startup from Reset, wakeup from STOP and STANDBY mode, or in case
  *           of failure of the HSE used directly or indirectly as system clock
  *           (if the Clock Security System CSS is enabled).
  * @note     MSI can not be stopped if it is used as system clock source.
  *           In this case, you have to select another source of the system
  *           clock then stop the MSI.
  * @note     After enabling the MSI, the application software should wait on
  *           MSIRDY flag to be set indicating that MSI clock is stable and can
  *           be used as system clock source.
  * @note   When the MSI is stopped, MSIRDY flag goes low after 6 MSI oscillator
  *         clock cycles.
  * @retval None
  */
#define __HAL_RCC_MSI_ENABLE()  SET_BIT(RCC->CR, RCC_CR_MSION)

#define __HAL_RCC_MSI_DISABLE() CLEAR_BIT(RCC->CR, RCC_CR_MSION)

/** @brief  Macro Adjusts the Internal Multi Speed oscillator (MSI) calibration value.
  * @note   The calibration is used to compensate for the variations in voltage
  *         and temperature that influence the frequency of the internal MSI RC.
  *         Refer to the Application Note AN3300 for more details on how to
  *         calibrate the MSI.
  * @param  __MSICALIBRATIONVALUE__ specifies the calibration trimming value
  *         (default is RCC_MSICALIBRATION_DEFAULT).
  *         This parameter must be a number between 0 and 255.
  * @retval None
  */
#define __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(__MSICALIBRATIONVALUE__) \
                  MODIFY_REG(RCC->ICSCR, RCC_ICSCR_MSITRIM, (__MSICALIBRATIONVALUE__) << RCC_ICSCR_MSITRIM_Pos)

/**
  * @brief  Macro configures the Internal Multi Speed oscillator (MSI) clock range in run mode
  * @note     After restart from Reset , the MSI clock is around 4 MHz.
  *           After stop the startup clock can be MSI (at any of its possible
  *           frequencies, the one that was used before entering stop mode) or HSI.
  *          After Standby its frequency can be selected between 4 possible values
  *          (1, 2, 4 or 8 MHz).
  * @note     MSIRANGE can be modified when MSI is OFF (MSION=0) or when MSI is ready
  *          (MSIRDY=1).
  * @note    The MSI clock range after reset can be modified on the fly.
  * @param  __MSIRANGEVALUE__ specifies the MSI clock range.
  *         This parameter must be one of the following values:
  *            @arg @ref RCC_MSIRANGE_0  MSI clock is around 100 KHz
  *            @arg @ref RCC_MSIRANGE_1  MSI clock is around 200 KHz
  *            @arg @ref RCC_MSIRANGE_2  MSI clock is around 400 KHz
  *            @arg @ref RCC_MSIRANGE_3  MSI clock is around 800 KHz
  *            @arg @ref RCC_MSIRANGE_4  MSI clock is around 1 MHz
  *            @arg @ref RCC_MSIRANGE_5  MSI clock is around 2 MHz
  *            @arg @ref RCC_MSIRANGE_6  MSI clock is around 4 MHz (default after Reset)
  *            @arg @ref RCC_MSIRANGE_7  MSI clock is around 8 MHz
  *            @arg @ref RCC_MSIRANGE_8  MSI clock is around 16 MHz
  *            @arg @ref RCC_MSIRANGE_9  MSI clock is around 24 MHz
  *            @arg @ref RCC_MSIRANGE_10  MSI clock is around 32 MHz
  *            @arg @ref RCC_MSIRANGE_11  MSI clock is around 48 MHz
  * @retval None
  */
#define __HAL_RCC_MSI_RANGE_CONFIG(__MSIRANGEVALUE__) \
                  do {                                                         \
                    SET_BIT(RCC->CR, RCC_CR_MSIRGSEL);                         \
                    MODIFY_REG(RCC->CR, RCC_CR_MSIRANGE, (__MSIRANGEVALUE__)); \
                  } while(0)

/**
  * @brief  Macro configures the Internal Multi Speed oscillator (MSI) clock range after Standby mode
  *         After Standby its frequency can be selected between 4 possible values (1, 2, 4 or 8 MHz).
  * @param  __MSIRANGEVALUE__ specifies the MSI clock range.
  *         This parameter must be one of the following values:
  *            @arg @ref RCC_MSIRANGE_4  MSI clock is around 1 MHz
  *            @arg @ref RCC_MSIRANGE_5  MSI clock is around 2 MHz
  *            @arg @ref RCC_MSIRANGE_6  MSI clock is around 4 MHz (default after Reset)
  *            @arg @ref RCC_MSIRANGE_7  MSI clock is around 8 MHz
  * @retval None
  */
#define __HAL_RCC_MSI_STANDBY_RANGE_CONFIG(__MSIRANGEVALUE__) \
                  MODIFY_REG(RCC->CSR, RCC_CSR_MSISRANGE, (__MSIRANGEVALUE__) << 4U)

/** @brief  Macro to get the Internal Multi Speed oscillator (MSI) clock range in run mode
  * @retval MSI clock range.
  *         This parameter must be one of the following values:
  *            @arg @ref RCC_MSIRANGE_0  MSI clock is around 100 KHz
  *            @arg @ref RCC_MSIRANGE_1  MSI clock is around 200 KHz
  *            @arg @ref RCC_MSIRANGE_2  MSI clock is around 400 KHz
  *            @arg @ref RCC_MSIRANGE_3  MSI clock is around 800 KHz
  *            @arg @ref RCC_MSIRANGE_4  MSI clock is around 1 MHz
  *            @arg @ref RCC_MSIRANGE_5  MSI clock is around 2 MHz
  *            @arg @ref RCC_MSIRANGE_6  MSI clock is around 4 MHz (default after Reset)
  *            @arg @ref RCC_MSIRANGE_7  MSI clock is around 8 MHz
  *            @arg @ref RCC_MSIRANGE_8  MSI clock is around 16 MHz
  *            @arg @ref RCC_MSIRANGE_9  MSI clock is around 24 MHz
  *            @arg @ref RCC_MSIRANGE_10  MSI clock is around 32 MHz
  *            @arg @ref RCC_MSIRANGE_11  MSI clock is around 48 MHz
  */
#define __HAL_RCC_GET_MSI_RANGE()                                              \
                  ((READ_BIT(RCC->CR, RCC_CR_MSIRGSEL) != 0U) ?             \
                   READ_BIT(RCC->CR, RCC_CR_MSIRANGE) :                        \
                   (READ_BIT(RCC->CSR, RCC_CSR_MSISRANGE) >> 4U))

/** @brief  Macros to enable or disable the Internal Low Speed oscillator (LSI).
  * @note   After enabling the LSI, the application software should wait on
  *         LSIRDY flag to be set indicating that LSI clock is stable and can
  *         be used to clock the IWDG and/or the RTC.
  * @note   LSI can not be disabled if the IWDG is running.
  * @note   When the LSI is stopped, LSIRDY flag goes low after 6 LSI oscillator
  *         clock cycles.
  * @retval None
  */
#define __HAL_RCC_LSI_ENABLE()         SET_BIT(RCC->CSR, RCC_CSR_LSION)

#define __HAL_RCC_LSI_DISABLE()        CLEAR_BIT(RCC->CSR, RCC_CSR_LSION)

/**
  * @brief  Macro to configure the External High Speed oscillator (HSE).
  * @note   Transition HSE Bypass to HSE On and HSE On to HSE Bypass are not
  *         supported by this macro. User should request a transition to HSE Off
  *         first and then HSE On or HSE Bypass.
  * @note   After enabling the HSE (RCC_HSE_ON or RCC_HSE_Bypass), the application
  *         software should wait on HSERDY flag to be set indicating that HSE clock
  *         is stable and can be used to clock the PLL and/or system clock.
  * @note   HSE state can not be changed if it is used directly or through the
  *         PLL as system clock. In this case, you have to select another source
  *         of the system clock then change the HSE state (ex. disable it).
  * @note   The HSE is stopped by hardware when entering STOP and STANDBY modes.
  * @note   This function reset the CSSON bit, so if the clock security system(CSS)
  *         was previously enabled you have to enable it again after calling this
  *         function.
  * @param  __STATE__ specifies the new state of the HSE.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_HSE_OFF  Turn OFF the HSE oscillator, HSERDY flag goes low after
  *                              6 HSE oscillator clock cycles.
  *            @arg @ref RCC_HSE_ON  Turn ON the HSE oscillator.
  *            @arg @ref RCC_HSE_BYPASS  HSE oscillator bypassed with external clock.
  * @retval None
  */
#define __HAL_RCC_HSE_CONFIG(__STATE__)                      \
                    do {                                     \
                      if((__STATE__) == RCC_HSE_ON)          \
                      {                                      \
                        SET_BIT(RCC->CR, RCC_CR_HSEON);      \
                      }                                      \
                      else if((__STATE__) == RCC_HSE_BYPASS) \
                      {                                      \
                        SET_BIT(RCC->CR, RCC_CR_HSEBYP);     \
                        SET_BIT(RCC->CR, RCC_CR_HSEON);      \
                      }                                      \
                      else                                   \
                      {                                      \
                        CLEAR_BIT(RCC->CR, RCC_CR_HSEON);    \
                        CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);   \
                      }                                      \
                    } while(0)

/**
  * @brief  Macro to configure the External Low Speed oscillator (LSE).
  * @note   Transitions LSE Bypass to LSE On and LSE On to LSE Bypass are not
  *         supported by this macro. User should request a transition to LSE Off
  *         first and then LSE On or LSE Bypass.
  * @note   As the LSE is in the Backup domain and write access is denied to
  *         this domain after reset, you have to enable write access using
  *         HAL_PWR_EnableBkUpAccess() function before to configure the LSE
  *         (to be done once after reset).
  * @note   After enabling the LSE (RCC_LSE_ON or RCC_LSE_BYPASS), the application
  *         software should wait on LSERDY flag to be set indicating that LSE clock
  *         is stable and can be used to clock the RTC.
  * @param  __STATE__ specifies the new state of the LSE.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_LSE_OFF  Turn OFF the LSE oscillator, LSERDY flag goes low after
  *                              6 LSE oscillator clock cycles.
  *            @arg @ref RCC_LSE_ON  Turn ON the LSE oscillator.
  *            @arg @ref RCC_LSE_BYPASS  LSE oscillator bypassed with external clock.
  * @retval None
  */
#define __HAL_RCC_LSE_CONFIG(__STATE__)                        \
                    do {                                       \
                      if((__STATE__) == RCC_LSE_ON)            \
                      {                                        \
                        SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);    \
                      }                                        \
                      else if((__STATE__) == RCC_LSE_BYPASS)   \
                      {                                        \
                        SET_BIT(RCC->BDCR, RCC_BDCR_LSEBYP);   \
                        SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);    \
                      }                                        \
                      else                                     \
                      {                                        \
                        CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEON);  \
                        CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEBYP); \
                      }                                        \
                    } while(0)

#if defined(RCC_HSI48_SUPPORT)

/** @brief  Macros to enable or disable the Internal High Speed 48MHz oscillator (HSI48).
  * @note   The HSI48 is stopped by hardware when entering STOP and STANDBY modes.
  * @note   After enabling the HSI48, the application software should wait on HSI48RDY
  *         flag to be set indicating that HSI48 clock is stable.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
#define __HAL_RCC_HSI48_ENABLE()  SET_BIT(RCC->CRRCR, RCC_CRRCR_HSI48ON)

#define __HAL_RCC_HSI48_DISABLE() CLEAR_BIT(RCC->CRRCR, RCC_CRRCR_HSI48ON)

#endif /* RCC_HSI48_SUPPORT */

/** @brief  Macros to configure the RTC clock (RTCCLK).
  * @note   As the RTC clock configuration bits are in the Backup domain and write
  *         access is denied to this domain after reset, you have to enable write
  *         access using the Power Backup Access macro before to configure
  *         the RTC clock source (to be done once after reset).
  * @note   Once the RTC clock is configured it cannot be changed unless the
  *         Backup domain is reset using __HAL_RCC_BACKUPRESET_FORCE() macro, or by
  *         a Power On Reset (POR).
  *
  * @param  __RTC_CLKSOURCE__ specifies the RTC clock source.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_RTCCLKSOURCE_NONE  No clock selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSE  LSE selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSI  LSI selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_HSE_DIV32  HSE clock divided by 32 selected
  *
  * @note   If the LSE or LSI is used as RTC clock source, the RTC continues to
  *         work in STOP and STANDBY modes, and can be used as wakeup source.
  *         However, when the HSE clock is used as RTC clock source, the RTC
  *         cannot be used in STOP and STANDBY modes.
  * @note   The maximum input clock frequency for RTC is 1MHz (when using HSE as
  *         RTC clock source).
  * @retval None
  */
#define __HAL_RCC_RTC_CONFIG(__RTC_CLKSOURCE__)  \
                  MODIFY_REG( RCC->BDCR, RCC_BDCR_RTCSEL, (__RTC_CLKSOURCE__))


/** @brief  Macro to get the RTC clock source.
  * @retval The returned value can be one of the following:
  *            @arg @ref RCC_RTCCLKSOURCE_NONE  No clock selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSE  LSE selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSI  LSI selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_HSE_DIV32  HSE clock divided by 32 selected
  */
#define  __HAL_RCC_GET_RTC_SOURCE() (READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL))

/** @brief  Macros to enable or disable the main PLL.
  * @note   After enabling the main PLL, the application software should wait on
  *         PLLRDY flag to be set indicating that PLL clock is stable and can
  *         be used as system clock source.
  * @note   The main PLL can not be disabled if it is used as system clock source
  * @note   The main PLL is disabled by hardware when entering STOP and STANDBY modes.
  * @retval None
  */
#define __HAL_RCC_PLL_ENABLE()         SET_BIT(RCC->CR, RCC_CR_PLLON)

#define __HAL_RCC_PLL_DISABLE()        CLEAR_BIT(RCC->CR, RCC_CR_PLLON)

/** @brief  Macro to configure the PLL clock source.
  * @note   This function must be used only when the main PLL is disabled.
  * @param  __PLLSOURCE__ specifies the PLL entry clock source.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_PLLSOURCE_NONE  No clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_MSI  MSI oscillator clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_HSI  HSI oscillator clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_HSE  HSE oscillator clock selected as PLL clock entry
  * @note   This clock source is common for the main PLL and audio PLL (PLLSAI1 and PLLSAI2).
  * @retval None
  *
  */
#define __HAL_RCC_PLL_PLLSOURCE_CONFIG(__PLLSOURCE__) \
                  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, (__PLLSOURCE__))

/** @brief  Macro to configure the PLL source division factor M.
  * @note   This function must be used only when the main PLL is disabled.
  * @param  __PLLM__ specifies the division factor for PLL VCO input clock
  *         This parameter must be a number between Min_Data = 1 and Max_Data = 16 on STM32L4Rx/STM32L4Sx devices.
  *         This parameter must be a number between Min_Data = 1 and Max_Data = 8 on other devices.
  * @note   You have to set the PLLM parameter correctly to ensure that the VCO input
  *         frequency ranges from 4 to 16 MHz. It is recommended to select a frequency
  *         of 16 MHz to limit PLL jitter.
  * @retval None
  *
  */
#define __HAL_RCC_PLL_PLLM_CONFIG(__PLLM__) \
                  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLM, ((__PLLM__) - 1) << 4U)

/**
  * @brief  Macro to configure the main PLL clock source, multiplication and division factors.
  * @note   This function must be used only when the main PLL is disabled.
  *
  * @param  __PLLSOURCE__ specifies the PLL entry clock source.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_PLLSOURCE_NONE  No clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_MSI  MSI oscillator clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_HSI  HSI oscillator clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_HSE  HSE oscillator clock selected as PLL clock entry
  * @note   This clock source is common for the main PLL and audio PLL (PLLSAI1 and PLLSAI2).
  *
  * @param  __PLLM__ specifies the division factor for PLL VCO input clock.
  *          This parameter must be a number between Min_Data = 1 and Max_Data = 16 on STM32L4Rx/STM32L4Sx devices.
  *          This parameter must be a number between Min_Data = 1 and Max_Data = 8 on other devices.
  * @note   You have to set the PLLM parameter correctly to ensure that the VCO input
  *         frequency ranges from 4 to 16 MHz. It is recommended to select a frequency
  *         of 16 MHz to limit PLL jitter.
  *
  * @param  __PLLN__ specifies the multiplication factor for PLL VCO output clock.
  *          This parameter must be a number between 8 and 86.
  * @note   You have to set the PLLN parameter correctly to ensure that the VCO
  *         output frequency is between 64 and 344 MHz.
  *
  * @param  __PLLP__ specifies the division factor for SAI clock when SAI available on device.
  *          This parameter must be a number in the range (7 or 17) for STM32L47x/STM32L48x
  *          else (2 to 31).
  *
  * @param  __PLLQ__ specifies the division factor for OTG FS, SDMMC1 and RNG clocks.
  *          This parameter must be in the range (2, 4, 6 or 8).
  * @note   If the USB OTG FS is used in your application, you have to set the
  *         PLLQ parameter correctly to have 48 MHz clock for the USB. However,
  *         the SDMMC1 and RNG need a frequency lower than or equal to 48 MHz to work
  *         correctly.
  * @param  __PLLR__ specifies the division factor for the main system clock.
  * @note   You have to set the PLLR parameter correctly to not exceed 80MHZ.
  *          This parameter must be in the range (2, 4, 6 or 8).
  * @retval None
  */
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)

#define __HAL_RCC_PLL_CONFIG(__PLLSOURCE__, __PLLM__, __PLLN__, __PLLP__, __PLLQ__,__PLLR__ ) \
                  MODIFY_REG(RCC->PLLCFGR, \
                             (RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | \
                              RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLR | RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLPDIV), \
                             ((__PLLSOURCE__) | \
                              (((__PLLM__) - 1U) << RCC_PLLCFGR_PLLM_Pos) | \
                              ((__PLLN__) << RCC_PLLCFGR_PLLN_Pos) | \
                              ((((__PLLQ__) >> 1U) - 1U) << RCC_PLLCFGR_PLLQ_Pos) | \
                              ((((__PLLR__) >> 1U) - 1U) << RCC_PLLCFGR_PLLR_Pos) | \
                              ((uint32_t)(__PLLP__) << RCC_PLLCFGR_PLLPDIV_Pos)))

#elif defined(RCC_PLLP_SUPPORT)

#define __HAL_RCC_PLL_CONFIG(__PLLSOURCE__, __PLLM__, __PLLN__, __PLLP__, __PLLQ__,__PLLR__ ) \
                  MODIFY_REG(RCC->PLLCFGR, \
                             (RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | \
                              RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLR | RCC_PLLCFGR_PLLP), \
                             ((__PLLSOURCE__) | \
                              (((__PLLM__) - 1U) << RCC_PLLCFGR_PLLM_Pos) | \
                              ((__PLLN__) << RCC_PLLCFGR_PLLN_Pos) | \
                              ((((__PLLQ__) >> 1U) - 1U) << RCC_PLLCFGR_PLLQ_Pos) | \
                              ((((__PLLR__) >> 1U) - 1U) << RCC_PLLCFGR_PLLR_Pos) | \
                              (((__PLLP__) >> 4U) << RCC_PLLCFGR_PLLP_Pos)))

#else

#define __HAL_RCC_PLL_CONFIG(__PLLSOURCE__, __PLLM__, __PLLN__, __PLLQ__,__PLLR__ ) \
                  MODIFY_REG(RCC->PLLCFGR, \
                             (RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | \
                              RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLR), \
                             ((__PLLSOURCE__) | \
                              (((__PLLM__) - 1U) << RCC_PLLCFGR_PLLM_Pos) | \
                              ((__PLLN__) << RCC_PLLCFGR_PLLN_Pos) | \
                              ((((__PLLQ__) >> 1U) - 1U) << RCC_PLLCFGR_PLLQ_Pos) | \
                              ((((__PLLR__) >> 1U) - 1U) << RCC_PLLCFGR_PLLR_Pos)))

#endif /* RCC_PLLP_DIV_2_31_SUPPORT */

/** @brief  Macro to get the oscillator used as PLL clock source.
  * @retval The oscillator used as PLL clock source. The returned value can be one
  *         of the following:
  *              - RCC_PLLSOURCE_NONE: No oscillator is used as PLL clock source.
  *              - RCC_PLLSOURCE_MSI: MSI oscillator is used as PLL clock source.
  *              - RCC_PLLSOURCE_HSI: HSI oscillator is used as PLL clock source.
  *              - RCC_PLLSOURCE_HSE: HSE oscillator is used as PLL clock source.
  */
#define __HAL_RCC_GET_PLL_OSCSOURCE() (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC))

/**
  * @brief  Enable or disable each clock output (RCC_PLL_SYSCLK, RCC_PLL_48M1CLK, RCC_PLL_SAI3CLK)
  * @note   Enabling/disabling clock outputs RCC_PLL_SAI3CLK and RCC_PLL_48M1CLK can be done at anytime
  *         without the need to stop the PLL in order to save power. But RCC_PLL_SYSCLK cannot
  *         be stopped if used as System Clock.
  * @param  __PLLCLOCKOUT__ specifies the PLL clock to be output.
  *          This parameter can be one or a combination of the following values:
  *            @arg @ref RCC_PLL_SAI3CLK  This clock is used to generate an accurate clock to achieve
  *                                   high-quality audio performance on SAI interface in case.
  *            @arg @ref RCC_PLL_48M1CLK  This Clock is used to generate the clock for the USB OTG FS (48 MHz),
  *                                   the random analog generator (<=48 MHz) and the SDMMC1 (<= 48 MHz).
  *            @arg @ref RCC_PLL_SYSCLK  This Clock is used to generate the high speed system clock (up to 80MHz)
  * @retval None
  */
#define __HAL_RCC_PLLCLKOUT_ENABLE(__PLLCLOCKOUT__)   SET_BIT(RCC->PLLCFGR, (__PLLCLOCKOUT__))

#define __HAL_RCC_PLLCLKOUT_DISABLE(__PLLCLOCKOUT__)  CLEAR_BIT(RCC->PLLCFGR, (__PLLCLOCKOUT__))

/**
  * @brief  Get clock output enable status (RCC_PLL_SYSCLK, RCC_PLL_48M1CLK, RCC_PLL_SAI3CLK)
  * @param  __PLLCLOCKOUT__ specifies the output PLL clock to be checked.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_PLL_SAI3CLK  This clock is used to generate an accurate clock to achieve
  *                                   high-quality audio performance on SAI interface in case.
  *            @arg @ref RCC_PLL_48M1CLK  This Clock is used to generate the clock for the USB OTG FS (48 MHz),
  *                                   the random analog generator (<=48 MHz) and the SDMMC1 (<= 48 MHz).
  *            @arg @ref RCC_PLL_SYSCLK  This Clock is used to generate the high speed system clock (up to 80MHz)
  * @retval SET / RESET
  */
#define __HAL_RCC_GET_PLLCLKOUT_CONFIG(__PLLCLOCKOUT__)  READ_BIT(RCC->PLLCFGR, (__PLLCLOCKOUT__))

/**
  * @brief  Macro to configure the system clock source.
  * @param  __SYSCLKSOURCE__ specifies the system clock source.
  *          This parameter can be one of the following values:
  *              - RCC_SYSCLKSOURCE_MSI: MSI oscillator is used as system clock source.
  *              - RCC_SYSCLKSOURCE_HSI: HSI oscillator is used as system clock source.
  *              - RCC_SYSCLKSOURCE_HSE: HSE oscillator is used as system clock source.
  *              - RCC_SYSCLKSOURCE_PLLCLK: PLL output is used as system clock source.
  * @retval None
  */
#define __HAL_RCC_SYSCLK_CONFIG(__SYSCLKSOURCE__) \
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, (__SYSCLKSOURCE__))

/** @brief  Macro to get the clock source used as system clock.
  * @retval The clock source used as system clock. The returned value can be one
  *         of the following:
  *              - RCC_SYSCLKSOURCE_STATUS_MSI: MSI used as system clock.
  *              - RCC_SYSCLKSOURCE_STATUS_HSI: HSI used as system clock.
  *              - RCC_SYSCLKSOURCE_STATUS_HSE: HSE used as system clock.
  *              - RCC_SYSCLKSOURCE_STATUS_PLLCLK: PLL used as system clock.
  */
#define __HAL_RCC_GET_SYSCLK_SOURCE() (READ_BIT(RCC->CFGR, RCC_CFGR_SWS))

/**
  * @brief  Macro to configure the External Low Speed oscillator (LSE) drive capability.
  * @note   As the LSE is in the Backup domain and write access is denied to
  *         this domain after reset, you have to enable write access using
  *         HAL_PWR_EnableBkUpAccess() function before to configure the LSE
  *         (to be done once after reset).
  * @param  __LSEDRIVE__ specifies the new state of the LSE drive capability.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_LSEDRIVE_LOW  LSE oscillator low drive capability.
  *            @arg @ref RCC_LSEDRIVE_MEDIUMLOW  LSE oscillator medium low drive capability.
  *            @arg @ref RCC_LSEDRIVE_MEDIUMHIGH  LSE oscillator medium high drive capability.
  *            @arg @ref RCC_LSEDRIVE_HIGH  LSE oscillator high drive capability.
  * @retval None
  */
#define __HAL_RCC_LSEDRIVE_CONFIG(__LSEDRIVE__) \
                  MODIFY_REG(RCC->BDCR, RCC_BDCR_LSEDRV, (__LSEDRIVE__))

/**
  * @brief  Macro to configure the wake up from stop clock.
  * @param  __STOPWUCLK__ specifies the clock source used after wake up from stop.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_STOP_WAKEUPCLOCK_MSI  MSI selected as system clock source
  *            @arg @ref RCC_STOP_WAKEUPCLOCK_HSI  HSI selected as system clock source
  * @retval None
  */
#define __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(__STOPWUCLK__) \
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_STOPWUCK, (__STOPWUCLK__))


/** @brief  Macro to configure the MCO clock.
  * @param  __MCOCLKSOURCE__ specifies the MCO clock source.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCO1SOURCE_NOCLOCK  MCO output disabled
  *            @arg @ref RCC_MCO1SOURCE_SYSCLK  System  clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_MSI  MSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_HSI  HSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_HSE  HSE clock selected as MCO sourcee
  *            @arg @ref RCC_MCO1SOURCE_PLLCLK  Main PLL clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_LSI  LSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_LSE  LSE clock selected as MCO source
  @if STM32L443xx
  *            @arg @ref RCC_MCO1SOURCE_HSI48  HSI48 clock selected as MCO source for devices with HSI48
  @endif
  @if STM32L4A6xx
  *            @arg @ref RCC_MCO1SOURCE_HSI48  HSI48 clock selected as MCO source for devices with HSI48
  @endif
  * @param  __MCODIV__ specifies the MCO clock prescaler.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCODIV_1   MCO clock source is divided by 1
  *            @arg @ref RCC_MCODIV_2   MCO clock source is divided by 2
  *            @arg @ref RCC_MCODIV_4   MCO clock source is divided by 4
  *            @arg @ref RCC_MCODIV_8   MCO clock source is divided by 8
  *            @arg @ref RCC_MCODIV_16  MCO clock source is divided by 16
  */
#define __HAL_RCC_MCO1_CONFIG(__MCOCLKSOURCE__, __MCODIV__) \
                 MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE), ((__MCOCLKSOURCE__) | (__MCODIV__)))

/** @defgroup RCC_Flags_Interrupts_Management Flags Interrupts Management
  * @brief macros to manage the specified RCC Flags and interrupts.
  * @{
  */

/** @brief  Enable RCC interrupt(s).
  * @param  __INTERRUPT__ specifies the RCC interrupt source(s) to be enabled.
  *         This parameter can be any combination of the following values:
  *            @arg @ref RCC_IT_LSIRDY  LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY  LSE ready interrupt
  *            @arg @ref RCC_IT_MSIRDY  HSI ready interrupt
  *            @arg @ref RCC_IT_HSIRDY  HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY  HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  *            @arg @ref RCC_IT_PLLSAI1RDY  PLLSAI1 ready interrupt for devices with PLLSAI1
  *            @arg @ref RCC_IT_PLLSAI2RDY  PLLSAI2 ready interrupt for devices with PLLSAI2
  *            @arg @ref RCC_IT_LSECSS  LSE Clock security system interrupt
  @if STM32L443xx
  *            @arg @ref RCC_IT_HSI48RDY  HSI48 ready interrupt for devices with HSI48
  @endif
  @if STM32L4A6xx
  *            @arg @ref RCC_IT_HSI48RDY  HSI48 ready interrupt for devices with HSI48
  @endif
  * @retval None
  */
#define __HAL_RCC_ENABLE_IT(__INTERRUPT__) SET_BIT(RCC->CIER, (__INTERRUPT__))

/** @brief Disable RCC interrupt(s).
  * @param  __INTERRUPT__ specifies the RCC interrupt source(s) to be disabled.
  *         This parameter can be any combination of the following values:
  *            @arg @ref RCC_IT_LSIRDY  LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY  LSE ready interrupt
  *            @arg @ref RCC_IT_MSIRDY  HSI ready interrupt
  *            @arg @ref RCC_IT_HSIRDY  HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY  HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  *            @arg @ref RCC_IT_PLLSAI1RDY  PLLSAI1 ready interrupt for devices with PLLSAI1
  *            @arg @ref RCC_IT_PLLSAI2RDY  PLLSAI2 ready interrupt for devices with PLLSAI2
  *            @arg @ref RCC_IT_LSECSS  LSE Clock security system interrupt
  @if STM32L443xx
  *            @arg @ref RCC_IT_HSI48RDY  HSI48 ready interrupt for devices with HSI48
  @endif
  @if STM32L4A6xx
  *            @arg @ref RCC_IT_HSI48RDY  HSI48 ready interrupt for devices with HSI48
  @endif
  * @retval None
  */
#define __HAL_RCC_DISABLE_IT(__INTERRUPT__) CLEAR_BIT(RCC->CIER, (__INTERRUPT__))

/** @brief  Clear the RCC's interrupt pending bits.
  * @param  __INTERRUPT__ specifies the interrupt pending bit to clear.
  *         This parameter can be any combination of the following values:
  *            @arg @ref RCC_IT_LSIRDY  LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY  LSE ready interrupt
  *            @arg @ref RCC_IT_MSIRDY  MSI ready interrupt
  *            @arg @ref RCC_IT_HSIRDY  HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY  HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  *            @arg @ref RCC_IT_PLLSAI1RDY  PLLSAI1 ready interrupt for devices with PLLSAI1
  *            @arg @ref RCC_IT_PLLSAI2RDY  PLLSAI2 ready interrupt for devices with PLLSAI2
  *            @arg @ref RCC_IT_CSS  HSE Clock security system interrupt
  *            @arg @ref RCC_IT_LSECSS  LSE Clock security system interrupt
  @if STM32L443xx
  *            @arg @ref RCC_IT_HSI48RDY  HSI48 ready interrupt for devices with HSI48
  @endif
  @if STM32L4A6xx
  *            @arg @ref RCC_IT_HSI48RDY  HSI48 ready interrupt for devices with HSI48
  @endif
  * @retval None
  */
#define __HAL_RCC_CLEAR_IT(__INTERRUPT__) WRITE_REG(RCC->CICR, (__INTERRUPT__))

/** @brief  Check whether the RCC interrupt has occurred or not.
  * @param  __INTERRUPT__ specifies the RCC interrupt source to check.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_IT_LSIRDY  LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY  LSE ready interrupt
  *            @arg @ref RCC_IT_MSIRDY  MSI ready interrupt
  *            @arg @ref RCC_IT_HSIRDY  HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY  HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  *            @arg @ref RCC_IT_PLLSAI1RDY  PLLSAI1 ready interrupt for devices with PLLSAI1
  *            @arg @ref RCC_IT_PLLSAI2RDY  PLLSAI2 ready interrupt for devices with PLLSAI2
  *            @arg @ref RCC_IT_CSS  HSE Clock security system interrupt
  *            @arg @ref RCC_IT_LSECSS  LSE Clock security system interrupt
  @if STM32L443xx
  *            @arg @ref RCC_IT_HSI48RDY HSI48 ready interrupt for devices with HSI48
  @endif
  @if STM32L4A6xx
  *            @arg @ref RCC_IT_HSI48RDY HSI48 ready interrupt for devices with HSI48
  @endif
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_RCC_GET_IT(__INTERRUPT__) (READ_BIT(RCC->CIFR, (__INTERRUPT__)) == (__INTERRUPT__))

/** @brief Set RMVF bit to clear the reset flags.
  *        The reset flags are: RCC_FLAG_FWRRST, RCC_FLAG_OBLRST, RCC_FLAG_PINRST, RCC_FLAG_BORRST,
  *        RCC_FLAG_SFTRST, RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST and RCC_FLAG_LPWRRST.
  * @retval None
 */
#define __HAL_RCC_CLEAR_RESET_FLAGS() SET_BIT(RCC->CSR, RCC_CSR_RMVF)

/** @brief  Check whether the selected RCC flag is set or not.
  * @param  __FLAG__ specifies the flag to check.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_FLAG_MSIRDY  MSI oscillator clock ready
  *            @arg @ref RCC_FLAG_HSIRDY  HSI oscillator clock ready
  *            @arg @ref RCC_FLAG_HSERDY  HSE oscillator clock ready
  *            @arg @ref RCC_FLAG_PLLRDY  Main PLL clock ready
  *            @arg @ref RCC_FLAG_PLLSAI1RDY  PLLSAI1 clock ready for devices with PLLSAI1
  *            @arg @ref RCC_FLAG_PLLSAI2RDY  PLLSAI2 clock ready for devices with PLLSAI2
  @if STM32L443xx
  *            @arg @ref RCC_FLAG_HSI48RDY  HSI48 clock ready for devices with HSI48
  @endif
  @if STM32L4A6xx
  *            @arg @ref RCC_FLAG_HSI48RDY  HSI48 clock ready for devices with HSI48
  @endif
  *            @arg @ref RCC_FLAG_LSERDY  LSE oscillator clock ready
  *            @arg @ref RCC_FLAG_LSECSSD  Clock security system failure on LSE oscillator detection
  *            @arg @ref RCC_FLAG_LSIRDY  LSI oscillator clock ready
  *            @arg @ref RCC_FLAG_BORRST  BOR reset
  *            @arg @ref RCC_FLAG_OBLRST  OBLRST reset
  *            @arg @ref RCC_FLAG_PINRST  Pin reset
  *            @arg @ref RCC_FLAG_FWRST  FIREWALL reset
  *            @arg @ref RCC_FLAG_SFTRST  Software reset
  *            @arg @ref RCC_FLAG_IWDGRST  Independent Watchdog reset
  *            @arg @ref RCC_FLAG_WWDGRST  Window Watchdog reset
  *            @arg @ref RCC_FLAG_LPWRRST  Low Power reset
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#if defined(RCC_HSI48_SUPPORT)
#define __HAL_RCC_GET_FLAG(__FLAG__) (((((((__FLAG__) >> 5U) == 1U) ? RCC->CR :                    \
                                        ((((__FLAG__) >> 5U) == 4U) ? RCC->CRRCR :                 \
                                        ((((__FLAG__) >> 5U) == 2U) ? RCC->BDCR :                  \
                                        ((((__FLAG__) >> 5U) == 3U) ? RCC->CSR : RCC->CIFR)))) &   \
                                          (1U << ((__FLAG__) & RCC_FLAG_MASK))) != 0U) ? 1U : 0U)
#else
#define __HAL_RCC_GET_FLAG(__FLAG__) (((((((__FLAG__) >> 5U) == 1U) ? RCC->CR :                    \
                                        ((((__FLAG__) >> 5U) == 2U) ? RCC->BDCR :                  \
                                        ((((__FLAG__) >> 5U) == 3U) ? RCC->CSR : RCC->CIFR))) &    \
                                          (1U << ((__FLAG__) & RCC_FLAG_MASK))) != 0U) ? 1U : 0U)
#endif /* RCC_HSI48_SUPPORT */

/**
  * @}
  */

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup RCC_Private_Constants RCC Private Constants
  * @{
  */
/* Defines used for Flags */
#define CR_REG_INDEX              1U
#define BDCR_REG_INDEX            2U
#define CSR_REG_INDEX             3U
#if defined(RCC_HSI48_SUPPORT)
#define CRRCR_REG_INDEX           4U
#endif /* RCC_HSI48_SUPPORT */

#define RCC_FLAG_MASK             0x1FU
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @addtogroup RCC_Private_Macros
  * @{
  */

#if defined(RCC_HSI48_SUPPORT)
#define IS_RCC_OSCILLATORTYPE(__OSCILLATOR__) (((__OSCILLATOR__) == RCC_OSCILLATORTYPE_NONE)                               || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSE)   == RCC_OSCILLATORTYPE_HSE)   || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSI)   == RCC_OSCILLATORTYPE_HSI)   || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSI48) == RCC_OSCILLATORTYPE_HSI48) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_MSI)   == RCC_OSCILLATORTYPE_MSI)   || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSI)   == RCC_OSCILLATORTYPE_LSI)   || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSE)   == RCC_OSCILLATORTYPE_LSE))
#else
#define IS_RCC_OSCILLATORTYPE(__OSCILLATOR__) (((__OSCILLATOR__) == RCC_OSCILLATORTYPE_NONE)                           || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSE) == RCC_OSCILLATORTYPE_HSE) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSI) == RCC_OSCILLATORTYPE_HSI) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_MSI) == RCC_OSCILLATORTYPE_MSI) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSI) == RCC_OSCILLATORTYPE_LSI) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSE) == RCC_OSCILLATORTYPE_LSE))
#endif /* RCC_HSI48_SUPPORT */

#define IS_RCC_HSE(__HSE__)  (((__HSE__) == RCC_HSE_OFF) || ((__HSE__) == RCC_HSE_ON) || \
                              ((__HSE__) == RCC_HSE_BYPASS))

#if defined(RCC_BDCR_LSESYSDIS)
#define IS_RCC_LSE(__LSE__)  (((__LSE__) == RCC_LSE_OFF) || ((__LSE__) == RCC_LSE_ON) || ((__LSE__) == RCC_LSE_BYPASS_RTC_ONLY) || \
                              ((__LSE__) == RCC_LSE_ON_RTC_ONLY) || ((__LSE__) == RCC_LSE_BYPASS))
#else
#define IS_RCC_LSE(__LSE__)  (((__LSE__) == RCC_LSE_OFF) || ((__LSE__) == RCC_LSE_ON) || \
                              ((__LSE__) == RCC_LSE_BYPASS))
#endif /* RCC_BDCR_LSESYSDIS */

#define IS_RCC_HSI(__HSI__)  (((__HSI__) == RCC_HSI_OFF) || ((__HSI__) == RCC_HSI_ON))

#define IS_RCC_HSI_CALIBRATION_VALUE(__VALUE__) ((__VALUE__) <= (RCC_ICSCR_HSITRIM >> RCC_ICSCR_HSITRIM_Pos))

#define IS_RCC_LSI(__LSI__)  (((__LSI__) == RCC_LSI_OFF) || ((__LSI__) == RCC_LSI_ON))

#if defined(RCC_CSR_LSIPREDIV)
#define IS_RCC_LSIDIV(__LSIDIV__)  (((__LSIDIV__) == RCC_LSI_DIV1) || ((__LSIDIV__) == RCC_LSI_DIV128))
#endif /* RCC_CSR_LSIPREDIV */

#define IS_RCC_MSI(__MSI__)  (((__MSI__) == RCC_MSI_OFF) || ((__MSI__) == RCC_MSI_ON))

#define IS_RCC_MSICALIBRATION_VALUE(__VALUE__) ((__VALUE__) <= 255U)

#if defined(RCC_HSI48_SUPPORT)
#define IS_RCC_HSI48(__HSI48__)  (((__HSI48__) == RCC_HSI48_OFF) || ((__HSI48__) == RCC_HSI48_ON))
#endif /* RCC_HSI48_SUPPORT */

#define IS_RCC_PLL(__PLL__) (((__PLL__) == RCC_PLL_NONE) ||((__PLL__) == RCC_PLL_OFF) || \
                             ((__PLL__) == RCC_PLL_ON))

#define IS_RCC_PLLSOURCE(__SOURCE__) (((__SOURCE__) == RCC_PLLSOURCE_NONE) || \
                                      ((__SOURCE__) == RCC_PLLSOURCE_MSI)  || \
                                      ((__SOURCE__) == RCC_PLLSOURCE_HSI)  || \
                                      ((__SOURCE__) == RCC_PLLSOURCE_HSE))

#if defined(RCC_PLLM_DIV_1_16_SUPPORT)
#define IS_RCC_PLLM_VALUE(__VALUE__) ((1U <= (__VALUE__)) && ((__VALUE__) <= 16U))
#else
#define IS_RCC_PLLM_VALUE(__VALUE__) ((1U <= (__VALUE__)) && ((__VALUE__) <= 8U))
#endif /*RCC_PLLM_DIV_1_16_SUPPORT */

#define IS_RCC_PLLN_VALUE(__VALUE__) ((8U <= (__VALUE__)) && ((__VALUE__) <= 86U))

#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
#define IS_RCC_PLLP_VALUE(__VALUE__) (((__VALUE__) >= 2U) && ((__VALUE__) <= 31U))
#else
#define IS_RCC_PLLP_VALUE(__VALUE__) (((__VALUE__) == 7U) || ((__VALUE__) == 17U))
#endif /*RCC_PLLP_DIV_2_31_SUPPORT */

#define IS_RCC_PLLQ_VALUE(__VALUE__) (((__VALUE__) == 2U) || ((__VALUE__) == 4U) || \
                                      ((__VALUE__) == 6U) || ((__VALUE__) == 8U))

#define IS_RCC_PLLR_VALUE(__VALUE__) (((__VALUE__) == 2U) || ((__VALUE__) == 4U) || \
                                      ((__VALUE__) == 6U) || ((__VALUE__) == 8U))

#if defined(RCC_PLLSAI1_SUPPORT)
#define IS_RCC_PLLSAI1CLOCKOUT_VALUE(__VALUE__) (((((__VALUE__) & RCC_PLLSAI1_SAI1CLK) == RCC_PLLSAI1_SAI1CLK)  || \
                                                  (((__VALUE__) & RCC_PLLSAI1_48M2CLK) == RCC_PLLSAI1_48M2CLK)  || \
                                                  (((__VALUE__) & RCC_PLLSAI1_ADC1CLK) == RCC_PLLSAI1_ADC1CLK)) && \
                                                 (((__VALUE__) & ~(RCC_PLLSAI1_SAI1CLK|RCC_PLLSAI1_48M2CLK|RCC_PLLSAI1_ADC1CLK)) == 0U))
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
#if defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || defined(STM32L496xx) || defined(STM32L4A6xx)
#define IS_RCC_PLLSAI2CLOCKOUT_VALUE(__VALUE__) (((((__VALUE__) & RCC_PLLSAI2_SAI2CLK) == RCC_PLLSAI2_SAI2CLK)  || \
                                                  (((__VALUE__) & RCC_PLLSAI2_ADC2CLK) == RCC_PLLSAI2_ADC2CLK)) && \
                                                 (((__VALUE__) & ~(RCC_PLLSAI2_SAI2CLK|RCC_PLLSAI2_ADC2CLK)) == 0U))
#elif defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define IS_RCC_PLLSAI2CLOCKOUT_VALUE(__VALUE__) (((((__VALUE__) & RCC_PLLSAI2_SAI2CLK) == RCC_PLLSAI2_SAI2CLK)  || \
                                                  (((__VALUE__) & RCC_PLLSAI2_DSICLK)  == RCC_PLLSAI2_DSICLK)   || \
                                                  (((__VALUE__) & RCC_PLLSAI2_LTDCCLK) == RCC_PLLSAI2_LTDCCLK)) && \
                                                 (((__VALUE__) & ~(RCC_PLLSAI2_SAI2CLK|RCC_PLLSAI2_DSICLK|RCC_PLLSAI2_LTDCCLK)) == 0U))
#endif /* STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx || STM32L496xx || STM32L4A6xx */
#endif /* RCC_PLLSAI2_SUPPORT */

#define IS_RCC_MSI_CLOCK_RANGE(__RANGE__) (((__RANGE__) == RCC_MSIRANGE_0)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_1)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_2)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_3)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_4)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_5)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_6)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_7)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_8)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_9)  || \
                                           ((__RANGE__) == RCC_MSIRANGE_10) || \
                                           ((__RANGE__) == RCC_MSIRANGE_11))

#define IS_RCC_MSI_STANDBY_CLOCK_RANGE(__RANGE__) (((__RANGE__) == RCC_MSIRANGE_4)  || \
                                                   ((__RANGE__) == RCC_MSIRANGE_5)  || \
                                                   ((__RANGE__) == RCC_MSIRANGE_6)  || \
                                                   ((__RANGE__) == RCC_MSIRANGE_7))

#define IS_RCC_CLOCKTYPE(__CLK__)  ((1U <= (__CLK__)) && ((__CLK__) <= 15U))

#define IS_RCC_SYSCLKSOURCE(__SOURCE__) (((__SOURCE__) == RCC_SYSCLKSOURCE_MSI) || \
                                         ((__SOURCE__) == RCC_SYSCLKSOURCE_HSI) || \
                                         ((__SOURCE__) == RCC_SYSCLKSOURCE_HSE) || \
                                         ((__SOURCE__) == RCC_SYSCLKSOURCE_PLLCLK))

#define IS_RCC_HCLK(__HCLK__) (((__HCLK__) == RCC_SYSCLK_DIV1)   || ((__HCLK__) == RCC_SYSCLK_DIV2)   || \
                               ((__HCLK__) == RCC_SYSCLK_DIV4)   || ((__HCLK__) == RCC_SYSCLK_DIV8)   || \
                               ((__HCLK__) == RCC_SYSCLK_DIV16)  || ((__HCLK__) == RCC_SYSCLK_DIV64)  || \
                               ((__HCLK__) == RCC_SYSCLK_DIV128) || ((__HCLK__) == RCC_SYSCLK_DIV256) || \
                               ((__HCLK__) == RCC_SYSCLK_DIV512))

#define IS_RCC_PCLK(__PCLK__) (((__PCLK__) == RCC_HCLK_DIV1) || ((__PCLK__) == RCC_HCLK_DIV2) || \
                               ((__PCLK__) == RCC_HCLK_DIV4) || ((__PCLK__) == RCC_HCLK_DIV8) || \
                               ((__PCLK__) == RCC_HCLK_DIV16))

#define IS_RCC_RTCCLKSOURCE(__SOURCE__) (((__SOURCE__) == RCC_RTCCLKSOURCE_NONE)   || \
                                         ((__SOURCE__) == RCC_RTCCLKSOURCE_LSE)    || \
                                         ((__SOURCE__) == RCC_RTCCLKSOURCE_LSI)    || \
                                         ((__SOURCE__) == RCC_RTCCLKSOURCE_HSE_DIV32))

#define IS_RCC_MCO(__MCOX__) ((__MCOX__) == RCC_MCO1)

#if defined(RCC_HSI48_SUPPORT)
#define IS_RCC_MCO1SOURCE(__SOURCE__) (((__SOURCE__) == RCC_MCO1SOURCE_NOCLOCK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_SYSCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_MSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSE) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_PLLCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSE) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSI48))
#else
#define IS_RCC_MCO1SOURCE(__SOURCE__) (((__SOURCE__) == RCC_MCO1SOURCE_NOCLOCK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_SYSCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_MSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSE) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_PLLCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSE))
#endif /* RCC_HSI48_SUPPORT */

#define IS_RCC_MCODIV(__DIV__) (((__DIV__) == RCC_MCODIV_1) || ((__DIV__) == RCC_MCODIV_2) || \
                                ((__DIV__) == RCC_MCODIV_4) || ((__DIV__) == RCC_MCODIV_8) || \
                                ((__DIV__) == RCC_MCODIV_16))

#define IS_RCC_LSE_DRIVE(__DRIVE__) (((__DRIVE__) == RCC_LSEDRIVE_LOW)        || \
                                     ((__DRIVE__) == RCC_LSEDRIVE_MEDIUMLOW)  || \
                                     ((__DRIVE__) == RCC_LSEDRIVE_MEDIUMHIGH) || \
                                     ((__DRIVE__) == RCC_LSEDRIVE_HIGH))

#define IS_RCC_STOP_WAKEUPCLOCK(__SOURCE__) (((__SOURCE__) == RCC_STOP_WAKEUPCLOCK_MSI) || \
                                             ((__SOURCE__) == RCC_STOP_WAKEUPCLOCK_HSI))
/**
  * @}
  */

/* Include RCC HAL Extended module */
#include "stm32l4xx_hal_rcc_ex.h"

/* Exported functions --------------------------------------------------------*/
/** @addtogroup RCC_Exported_Functions
  * @{
  */


/** @addtogroup RCC_Exported_Functions_Group1
  * @{
  */

/* Initialization and de-initialization functions  ******************************/
HAL_StatusTypeDef HAL_RCC_DeInit(void);
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);

/**
  * @}
  */

/** @addtogroup RCC_Exported_Functions_Group2
  * @{
  */

/* Peripheral Control functions  ************************************************/
void              HAL_RCC_MCOConfig(uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv);
void              HAL_RCC_EnableCSS(void);
uint32_t          HAL_RCC_GetSysClockFreq(void);
uint32_t          HAL_RCC_GetHCLKFreq(void);
uint32_t          HAL_RCC_GetPCLK1Freq(void);
uint32_t          HAL_RCC_GetPCLK2Freq(void);
void              HAL_RCC_GetOscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
void              HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t *pFLatency);
/* CSS NMI IRQ handler */
void              HAL_RCC_NMI_IRQHandler(void);
/* User Callbacks in non blocking mode (IT mode) */
void              HAL_RCC_CSSCallback(void);

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

#endif /* __STM32L4xx_HAL_RCC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
