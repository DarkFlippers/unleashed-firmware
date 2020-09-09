/**
  ******************************************************************************
  * @file    stm32l4xx_ll_rcc.h
  * @author  MCD Application Team
  * @brief   Header file of RCC LL module.
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
#ifndef STM32L4xx_LL_RCC_H
#define STM32L4xx_LL_RCC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(RCC)

/** @defgroup RCC_LL RCC
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup RCC_LL_Private_Constants RCC Private Constants
  * @{
  */
/* Defines used to perform offsets*/
/* Offset used to access to RCC_CCIPR and RCC_CCIPR2 registers */
#define RCC_OFFSET_CCIPR        0U
#define RCC_OFFSET_CCIPR2       0x14U

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup RCC_LL_Private_Macros RCC Private Macros
  * @{
  */
/**
  * @}
  */
#endif /*USE_FULL_LL_DRIVER*/

/* Exported types ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup RCC_LL_Exported_Types RCC Exported Types
  * @{
  */

/** @defgroup LL_ES_CLOCK_FREQ Clocks Frequency Structure
  * @{
  */

/**
  * @brief  RCC Clocks Frequency Structure
  */
typedef struct
{
  uint32_t SYSCLK_Frequency;        /*!< SYSCLK clock frequency */
  uint32_t HCLK_Frequency;          /*!< HCLK clock frequency */
  uint32_t PCLK1_Frequency;         /*!< PCLK1 clock frequency */
  uint32_t PCLK2_Frequency;         /*!< PCLK2 clock frequency */
} LL_RCC_ClocksTypeDef;

/**
  * @}
  */

/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/* Exported constants --------------------------------------------------------*/
/** @defgroup RCC_LL_Exported_Constants RCC Exported Constants
  * @{
  */

/** @defgroup RCC_LL_EC_OSC_VALUES Oscillator Values adaptation
  * @brief    Defines used to adapt values of different oscillators
  * @note     These values could be modified in the user environment according to
  *           HW set-up.
  * @{
  */
#if !defined  (HSE_VALUE)
#define HSE_VALUE    8000000U   /*!< Value of the HSE oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
#define HSI_VALUE    16000000U  /*!< Value of the HSI oscillator in Hz */
#endif /* HSI_VALUE */

#if !defined  (LSE_VALUE)
#define LSE_VALUE    32768U     /*!< Value of the LSE oscillator in Hz */
#endif /* LSE_VALUE */

#if !defined  (LSI_VALUE)
#define LSI_VALUE    32000U     /*!< Value of the LSI oscillator in Hz */
#endif /* LSI_VALUE */
#if defined(RCC_HSI48_SUPPORT)

#if !defined  (HSI48_VALUE)
#define HSI48_VALUE  48000000U  /*!< Value of the HSI48 oscillator in Hz */
#endif /* HSI48_VALUE */
#endif /* RCC_HSI48_SUPPORT */

#if !defined  (EXTERNAL_SAI1_CLOCK_VALUE)
#define EXTERNAL_SAI1_CLOCK_VALUE    48000U /*!< Value of the SAI1_EXTCLK external oscillator in Hz */
#endif /* EXTERNAL_SAI1_CLOCK_VALUE */

#if !defined  (EXTERNAL_SAI2_CLOCK_VALUE)
#define EXTERNAL_SAI2_CLOCK_VALUE    48000U /*!< Value of the SAI2_EXTCLK external oscillator in Hz */
#endif /* EXTERNAL_SAI2_CLOCK_VALUE */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_CLEAR_FLAG Clear Flags Defines
  * @brief    Flags defines which can be used with LL_RCC_WriteReg function
  * @{
  */
#define LL_RCC_CICR_LSIRDYC                RCC_CICR_LSIRDYC     /*!< LSI Ready Interrupt Clear */
#define LL_RCC_CICR_LSERDYC                RCC_CICR_LSERDYC     /*!< LSE Ready Interrupt Clear */
#define LL_RCC_CICR_MSIRDYC                RCC_CICR_MSIRDYC     /*!< MSI Ready Interrupt Clear */
#define LL_RCC_CICR_HSIRDYC                RCC_CICR_HSIRDYC     /*!< HSI Ready Interrupt Clear */
#define LL_RCC_CICR_HSERDYC                RCC_CICR_HSERDYC     /*!< HSE Ready Interrupt Clear */
#define LL_RCC_CICR_PLLRDYC                RCC_CICR_PLLRDYC     /*!< PLL Ready Interrupt Clear */
#if defined(RCC_HSI48_SUPPORT)
#define LL_RCC_CICR_HSI48RDYC              RCC_CICR_HSI48RDYC   /*!< HSI48 Ready Interrupt Clear */
#endif /* RCC_HSI48_SUPPORT */
#if defined(RCC_PLLSAI1_SUPPORT)
#define LL_RCC_CICR_PLLSAI1RDYC            RCC_CICR_PLLSAI1RDYC /*!< PLLSAI1 Ready Interrupt Clear */
#endif /* RCC_PLLSAI1_SUPPORT */
#if defined(RCC_PLLSAI2_SUPPORT)
#define LL_RCC_CICR_PLLSAI2RDYC            RCC_CICR_PLLSAI2RDYC /*!< PLLSAI2 Ready Interrupt Clear */
#endif /* RCC_PLLSAI2_SUPPORT */
#define LL_RCC_CICR_LSECSSC                RCC_CICR_LSECSSC     /*!< LSE Clock Security System Interrupt Clear */
#define LL_RCC_CICR_CSSC                   RCC_CICR_CSSC        /*!< Clock Security System Interrupt Clear */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_GET_FLAG Get Flags Defines
  * @brief    Flags defines which can be used with LL_RCC_ReadReg function
  * @{
  */
#define LL_RCC_CIFR_LSIRDYF                RCC_CIFR_LSIRDYF     /*!< LSI Ready Interrupt flag */
#define LL_RCC_CIFR_LSERDYF                RCC_CIFR_LSERDYF     /*!< LSE Ready Interrupt flag */
#define LL_RCC_CIFR_MSIRDYF                RCC_CIFR_MSIRDYF     /*!< MSI Ready Interrupt flag */
#define LL_RCC_CIFR_HSIRDYF                RCC_CIFR_HSIRDYF     /*!< HSI Ready Interrupt flag */
#define LL_RCC_CIFR_HSERDYF                RCC_CIFR_HSERDYF     /*!< HSE Ready Interrupt flag */
#define LL_RCC_CIFR_PLLRDYF                RCC_CIFR_PLLRDYF     /*!< PLL Ready Interrupt flag */
#if defined(RCC_HSI48_SUPPORT)
#define LL_RCC_CIFR_HSI48RDYF              RCC_CIFR_HSI48RDYF   /*!< HSI48 Ready Interrupt flag */
#endif /* RCC_HSI48_SUPPORT */
#if defined(RCC_PLLSAI1_SUPPORT)
#define LL_RCC_CIFR_PLLSAI1RDYF            RCC_CIFR_PLLSAI1RDYF /*!< PLLSAI1 Ready Interrupt flag */
#endif /* RCC_PLLSAI1_SUPPORT */
#if defined(RCC_PLLSAI2_SUPPORT)
#define LL_RCC_CIFR_PLLSAI2RDYF            RCC_CIFR_PLLSAI2RDYF /*!< PLLSAI2 Ready Interrupt flag */
#endif /* RCC_PLLSAI2_SUPPORT */
#define LL_RCC_CIFR_LSECSSF                RCC_CIFR_LSECSSF     /*!< LSE Clock Security System Interrupt flag */
#define LL_RCC_CIFR_CSSF                   RCC_CIFR_CSSF        /*!< Clock Security System Interrupt flag */
#define LL_RCC_CSR_FWRSTF                  RCC_CSR_FWRSTF     /*!< Firewall reset flag */
#define LL_RCC_CSR_LPWRRSTF                RCC_CSR_LPWRRSTF   /*!< Low-Power reset flag */
#define LL_RCC_CSR_OBLRSTF                 RCC_CSR_OBLRSTF    /*!< OBL reset flag */
#define LL_RCC_CSR_PINRSTF                 RCC_CSR_PINRSTF    /*!< PIN reset flag */
#define LL_RCC_CSR_SFTRSTF                 RCC_CSR_SFTRSTF    /*!< Software Reset flag */
#define LL_RCC_CSR_IWDGRSTF                RCC_CSR_IWDGRSTF   /*!< Independent Watchdog reset flag */
#define LL_RCC_CSR_WWDGRSTF                RCC_CSR_WWDGRSTF   /*!< Window watchdog reset flag */
#define LL_RCC_CSR_BORRSTF                 RCC_CSR_BORRSTF    /*!< BOR reset flag */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_IT IT Defines
  * @brief    IT defines which can be used with LL_RCC_ReadReg and  LL_RCC_WriteReg functions
  * @{
  */
#define LL_RCC_CIER_LSIRDYIE               RCC_CIER_LSIRDYIE      /*!< LSI Ready Interrupt Enable */
#define LL_RCC_CIER_LSERDYIE               RCC_CIER_LSERDYIE      /*!< LSE Ready Interrupt Enable */
#define LL_RCC_CIER_MSIRDYIE               RCC_CIER_MSIRDYIE      /*!< MSI Ready Interrupt Enable */
#define LL_RCC_CIER_HSIRDYIE               RCC_CIER_HSIRDYIE      /*!< HSI Ready Interrupt Enable */
#define LL_RCC_CIER_HSERDYIE               RCC_CIER_HSERDYIE      /*!< HSE Ready Interrupt Enable */
#define LL_RCC_CIER_PLLRDYIE               RCC_CIER_PLLRDYIE      /*!< PLL Ready Interrupt Enable */
#if defined(RCC_HSI48_SUPPORT)
#define LL_RCC_CIER_HSI48RDYIE             RCC_CIER_HSI48RDYIE    /*!< HSI48 Ready Interrupt Enable */
#endif /* RCC_HSI48_SUPPORT */
#if defined(RCC_PLLSAI1_SUPPORT)
#define LL_RCC_CIER_PLLSAI1RDYIE           RCC_CIER_PLLSAI1RDYIE  /*!< PLLSAI1 Ready Interrupt Enable */
#endif /* RCC_PLLSAI1_SUPPORT */
#if defined(RCC_PLLSAI2_SUPPORT)
#define LL_RCC_CIER_PLLSAI2RDYIE           RCC_CIER_PLLSAI2RDYIE  /*!< PLLSAI2 Ready Interrupt Enable */
#endif /* RCC_PLLSAI2_SUPPORT */
#define LL_RCC_CIER_LSECSSIE               RCC_CIER_LSECSSIE      /*!< LSE CSS Interrupt Enable */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_LSEDRIVE  LSE oscillator drive capability
  * @{
  */
#define LL_RCC_LSEDRIVE_LOW                0x00000000U             /*!< Xtal mode lower driving capability */
#define LL_RCC_LSEDRIVE_MEDIUMLOW          RCC_BDCR_LSEDRV_0       /*!< Xtal mode medium low driving capability */
#define LL_RCC_LSEDRIVE_MEDIUMHIGH         RCC_BDCR_LSEDRV_1       /*!< Xtal mode medium high driving capability */
#define LL_RCC_LSEDRIVE_HIGH               RCC_BDCR_LSEDRV         /*!< Xtal mode higher driving capability */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_MSIRANGE  MSI clock ranges
  * @{
  */
#define LL_RCC_MSIRANGE_0                  RCC_CR_MSIRANGE_0  /*!< MSI = 100 KHz  */
#define LL_RCC_MSIRANGE_1                  RCC_CR_MSIRANGE_1  /*!< MSI = 200 KHz  */
#define LL_RCC_MSIRANGE_2                  RCC_CR_MSIRANGE_2  /*!< MSI = 400 KHz  */
#define LL_RCC_MSIRANGE_3                  RCC_CR_MSIRANGE_3  /*!< MSI = 800 KHz  */
#define LL_RCC_MSIRANGE_4                  RCC_CR_MSIRANGE_4  /*!< MSI = 1 MHz    */
#define LL_RCC_MSIRANGE_5                  RCC_CR_MSIRANGE_5  /*!< MSI = 2 MHz    */
#define LL_RCC_MSIRANGE_6                  RCC_CR_MSIRANGE_6  /*!< MSI = 4 MHz    */
#define LL_RCC_MSIRANGE_7                  RCC_CR_MSIRANGE_7  /*!< MSI = 8 MHz    */
#define LL_RCC_MSIRANGE_8                  RCC_CR_MSIRANGE_8  /*!< MSI = 16 MHz   */
#define LL_RCC_MSIRANGE_9                  RCC_CR_MSIRANGE_9  /*!< MSI = 24 MHz   */
#define LL_RCC_MSIRANGE_10                 RCC_CR_MSIRANGE_10 /*!< MSI = 32 MHz   */
#define LL_RCC_MSIRANGE_11                 RCC_CR_MSIRANGE_11 /*!< MSI = 48 MHz   */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_MSISRANGE  MSI range after Standby mode
  * @{
  */
#define LL_RCC_MSISRANGE_4                 RCC_CSR_MSISRANGE_1  /*!< MSI = 1 MHz    */
#define LL_RCC_MSISRANGE_5                 RCC_CSR_MSISRANGE_2  /*!< MSI = 2 MHz    */
#define LL_RCC_MSISRANGE_6                 RCC_CSR_MSISRANGE_4  /*!< MSI = 4 MHz    */
#define LL_RCC_MSISRANGE_7                 RCC_CSR_MSISRANGE_8  /*!< MSI = 8 MHz    */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_LSCO_CLKSOURCE  LSCO Selection
  * @{
  */
#define LL_RCC_LSCO_CLKSOURCE_LSI          0x00000000U                 /*!< LSI selection for low speed clock  */
#define LL_RCC_LSCO_CLKSOURCE_LSE          RCC_BDCR_LSCOSEL      /*!< LSE selection for low speed clock  */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_SYS_CLKSOURCE  System clock switch
  * @{
  */
#define LL_RCC_SYS_CLKSOURCE_MSI           RCC_CFGR_SW_MSI    /*!< MSI selection as system clock */
#define LL_RCC_SYS_CLKSOURCE_HSI           RCC_CFGR_SW_HSI    /*!< HSI selection as system clock */
#define LL_RCC_SYS_CLKSOURCE_HSE           RCC_CFGR_SW_HSE    /*!< HSE selection as system clock */
#define LL_RCC_SYS_CLKSOURCE_PLL           RCC_CFGR_SW_PLL    /*!< PLL selection as system clock */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_SYS_CLKSOURCE_STATUS  System clock switch status
  * @{
  */
#define LL_RCC_SYS_CLKSOURCE_STATUS_MSI    RCC_CFGR_SWS_MSI   /*!< MSI used as system clock */
#define LL_RCC_SYS_CLKSOURCE_STATUS_HSI    RCC_CFGR_SWS_HSI   /*!< HSI used as system clock */
#define LL_RCC_SYS_CLKSOURCE_STATUS_HSE    RCC_CFGR_SWS_HSE   /*!< HSE used as system clock */
#define LL_RCC_SYS_CLKSOURCE_STATUS_PLL    RCC_CFGR_SWS_PLL   /*!< PLL used as system clock */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_SYSCLK_DIV  AHB prescaler
  * @{
  */
#define LL_RCC_SYSCLK_DIV_1                RCC_CFGR_HPRE_DIV1   /*!< SYSCLK not divided */
#define LL_RCC_SYSCLK_DIV_2                RCC_CFGR_HPRE_DIV2   /*!< SYSCLK divided by 2 */
#define LL_RCC_SYSCLK_DIV_4                RCC_CFGR_HPRE_DIV4   /*!< SYSCLK divided by 4 */
#define LL_RCC_SYSCLK_DIV_8                RCC_CFGR_HPRE_DIV8   /*!< SYSCLK divided by 8 */
#define LL_RCC_SYSCLK_DIV_16               RCC_CFGR_HPRE_DIV16  /*!< SYSCLK divided by 16 */
#define LL_RCC_SYSCLK_DIV_64               RCC_CFGR_HPRE_DIV64  /*!< SYSCLK divided by 64 */
#define LL_RCC_SYSCLK_DIV_128              RCC_CFGR_HPRE_DIV128 /*!< SYSCLK divided by 128 */
#define LL_RCC_SYSCLK_DIV_256              RCC_CFGR_HPRE_DIV256 /*!< SYSCLK divided by 256 */
#define LL_RCC_SYSCLK_DIV_512              RCC_CFGR_HPRE_DIV512 /*!< SYSCLK divided by 512 */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_APB1_DIV  APB low-speed prescaler (APB1)
  * @{
  */
#define LL_RCC_APB1_DIV_1                  RCC_CFGR_PPRE1_DIV1  /*!< HCLK not divided */
#define LL_RCC_APB1_DIV_2                  RCC_CFGR_PPRE1_DIV2  /*!< HCLK divided by 2 */
#define LL_RCC_APB1_DIV_4                  RCC_CFGR_PPRE1_DIV4  /*!< HCLK divided by 4 */
#define LL_RCC_APB1_DIV_8                  RCC_CFGR_PPRE1_DIV8  /*!< HCLK divided by 8 */
#define LL_RCC_APB1_DIV_16                 RCC_CFGR_PPRE1_DIV16 /*!< HCLK divided by 16 */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_APB2_DIV  APB high-speed prescaler (APB2)
  * @{
  */
#define LL_RCC_APB2_DIV_1                  RCC_CFGR_PPRE2_DIV1  /*!< HCLK not divided */
#define LL_RCC_APB2_DIV_2                  RCC_CFGR_PPRE2_DIV2  /*!< HCLK divided by 2 */
#define LL_RCC_APB2_DIV_4                  RCC_CFGR_PPRE2_DIV4  /*!< HCLK divided by 4 */
#define LL_RCC_APB2_DIV_8                  RCC_CFGR_PPRE2_DIV8  /*!< HCLK divided by 8 */
#define LL_RCC_APB2_DIV_16                 RCC_CFGR_PPRE2_DIV16 /*!< HCLK divided by 16 */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_STOP_WAKEUPCLOCK  Wakeup from Stop and CSS backup clock selection
  * @{
  */
#define LL_RCC_STOP_WAKEUPCLOCK_MSI        0x00000000U             /*!< MSI selection after wake-up from STOP */
#define LL_RCC_STOP_WAKEUPCLOCK_HSI        RCC_CFGR_STOPWUCK       /*!< HSI selection after wake-up from STOP */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_MCO1SOURCE  MCO1 SOURCE selection
  * @{
  */
#define LL_RCC_MCO1SOURCE_NOCLOCK          0x00000000U                            /*!< MCO output disabled, no clock on MCO */
#define LL_RCC_MCO1SOURCE_SYSCLK           RCC_CFGR_MCOSEL_0                      /*!< SYSCLK selection as MCO1 source */
#define LL_RCC_MCO1SOURCE_MSI              RCC_CFGR_MCOSEL_1                      /*!< MSI selection as MCO1 source */
#define LL_RCC_MCO1SOURCE_HSI              (RCC_CFGR_MCOSEL_0| RCC_CFGR_MCOSEL_1) /*!< HSI16 selection as MCO1 source */
#define LL_RCC_MCO1SOURCE_HSE              RCC_CFGR_MCOSEL_2                      /*!< HSE selection as MCO1 source */
#define LL_RCC_MCO1SOURCE_PLLCLK           (RCC_CFGR_MCOSEL_0|RCC_CFGR_MCOSEL_2)  /*!< Main PLL selection as MCO1 source */
#define LL_RCC_MCO1SOURCE_LSI              (RCC_CFGR_MCOSEL_1|RCC_CFGR_MCOSEL_2)  /*!< LSI selection as MCO1 source */
#define LL_RCC_MCO1SOURCE_LSE              (RCC_CFGR_MCOSEL_0|RCC_CFGR_MCOSEL_1|RCC_CFGR_MCOSEL_2) /*!< LSE selection as MCO1 source */
#if defined(RCC_HSI48_SUPPORT)
#define LL_RCC_MCO1SOURCE_HSI48            RCC_CFGR_MCOSEL_3                      /*!< HSI48 selection as MCO1 source */
#endif /* RCC_HSI48_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_MCO1_DIV  MCO1 prescaler
  * @{
  */
#define LL_RCC_MCO1_DIV_1                  RCC_CFGR_MCOPRE_DIV1       /*!< MCO not divided */
#define LL_RCC_MCO1_DIV_2                  RCC_CFGR_MCOPRE_DIV2       /*!< MCO divided by 2 */
#define LL_RCC_MCO1_DIV_4                  RCC_CFGR_MCOPRE_DIV4       /*!< MCO divided by 4 */
#define LL_RCC_MCO1_DIV_8                  RCC_CFGR_MCOPRE_DIV8       /*!< MCO divided by 8 */
#define LL_RCC_MCO1_DIV_16                 RCC_CFGR_MCOPRE_DIV16      /*!< MCO divided by 16 */
/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup RCC_LL_EC_PERIPH_FREQUENCY Peripheral clock frequency
  * @{
  */
#define LL_RCC_PERIPH_FREQUENCY_NO         0x00000000U                 /*!< No clock enabled for the peripheral            */
#define LL_RCC_PERIPH_FREQUENCY_NA         0xFFFFFFFFU                 /*!< Frequency cannot be provided as external clock */
/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/** @defgroup RCC_LL_EC_USART1_CLKSOURCE  Peripheral USART clock source selection
  * @{
  */
#define LL_RCC_USART1_CLKSOURCE_PCLK2      (RCC_CCIPR_USART1SEL << 16U)                           /*!< PCLK2 clock used as USART1 clock source */
#define LL_RCC_USART1_CLKSOURCE_SYSCLK     ((RCC_CCIPR_USART1SEL << 16U) | RCC_CCIPR_USART1SEL_0) /*!< SYSCLK clock used as USART1 clock source */
#define LL_RCC_USART1_CLKSOURCE_HSI        ((RCC_CCIPR_USART1SEL << 16U) | RCC_CCIPR_USART1SEL_1) /*!< HSI clock used as USART1 clock source */
#define LL_RCC_USART1_CLKSOURCE_LSE        ((RCC_CCIPR_USART1SEL << 16U) | RCC_CCIPR_USART1SEL)   /*!< LSE clock used as USART1 clock source */
#define LL_RCC_USART2_CLKSOURCE_PCLK1      (RCC_CCIPR_USART2SEL << 16U)                           /*!< PCLK1 clock used as USART2 clock source */
#define LL_RCC_USART2_CLKSOURCE_SYSCLK     ((RCC_CCIPR_USART2SEL << 16U) | RCC_CCIPR_USART2SEL_0) /*!< SYSCLK clock used as USART2 clock source */
#define LL_RCC_USART2_CLKSOURCE_HSI        ((RCC_CCIPR_USART2SEL << 16U) | RCC_CCIPR_USART2SEL_1) /*!< HSI clock used as USART2 clock source */
#define LL_RCC_USART2_CLKSOURCE_LSE        ((RCC_CCIPR_USART2SEL << 16U) | RCC_CCIPR_USART2SEL)   /*!< LSE clock used as USART2 clock source */
#if defined(RCC_CCIPR_USART3SEL)
#define LL_RCC_USART3_CLKSOURCE_PCLK1      (RCC_CCIPR_USART3SEL << 16U)                           /*!< PCLK1 clock used as USART3 clock source */
#define LL_RCC_USART3_CLKSOURCE_SYSCLK     ((RCC_CCIPR_USART3SEL << 16U) | RCC_CCIPR_USART3SEL_0) /*!< SYSCLK clock used as USART3 clock source */
#define LL_RCC_USART3_CLKSOURCE_HSI        ((RCC_CCIPR_USART3SEL << 16U) | RCC_CCIPR_USART3SEL_1) /*!< HSI clock used as USART3 clock source */
#define LL_RCC_USART3_CLKSOURCE_LSE        ((RCC_CCIPR_USART3SEL << 16U) | RCC_CCIPR_USART3SEL)   /*!< LSE clock used as USART3 clock source */
#endif /* RCC_CCIPR_USART3SEL */
/**
  * @}
  */

#if defined(RCC_CCIPR_UART4SEL) || defined(RCC_CCIPR_UART5SEL)
/** @defgroup RCC_LL_EC_UART4_CLKSOURCE  Peripheral UART clock source selection
  * @{
  */
#if defined(RCC_CCIPR_UART4SEL)
#define LL_RCC_UART4_CLKSOURCE_PCLK1       (RCC_CCIPR_UART4SEL << 16U)                           /*!< PCLK1 clock used as UART4 clock source */
#define LL_RCC_UART4_CLKSOURCE_SYSCLK      ((RCC_CCIPR_UART4SEL << 16U) | RCC_CCIPR_UART4SEL_0)  /*!< SYSCLK clock used as UART4 clock source */
#define LL_RCC_UART4_CLKSOURCE_HSI         ((RCC_CCIPR_UART4SEL << 16U) | RCC_CCIPR_UART4SEL_1)  /*!< HSI clock used as UART4 clock source */
#define LL_RCC_UART4_CLKSOURCE_LSE         ((RCC_CCIPR_UART4SEL << 16U) | RCC_CCIPR_UART4SEL)    /*!< LSE clock used as UART4 clock source */
#endif /* RCC_CCIPR_UART4SEL */
#if defined(RCC_CCIPR_UART5SEL)
#define LL_RCC_UART5_CLKSOURCE_PCLK1       (RCC_CCIPR_UART5SEL << 16U)                           /*!< PCLK1 clock used as UART5 clock source */
#define LL_RCC_UART5_CLKSOURCE_SYSCLK      ((RCC_CCIPR_UART5SEL << 16U) | RCC_CCIPR_UART5SEL_0)  /*!< SYSCLK clock used as UART5 clock source */
#define LL_RCC_UART5_CLKSOURCE_HSI         ((RCC_CCIPR_UART5SEL << 16U) | RCC_CCIPR_UART5SEL_1)  /*!< HSI clock used as UART5 clock source */
#define LL_RCC_UART5_CLKSOURCE_LSE         ((RCC_CCIPR_UART5SEL << 16U) | RCC_CCIPR_UART5SEL)    /*!< LSE clock used as UART5 clock source */
#endif /* RCC_CCIPR_UART5SEL */
/**
  * @}
  */
#endif /* RCC_CCIPR_UART4SEL || RCC_CCIPR_UART5SEL */

/** @defgroup RCC_LL_EC_LPUART1_CLKSOURCE  Peripheral LPUART clock source selection
  * @{
  */
#define LL_RCC_LPUART1_CLKSOURCE_PCLK1     0x00000000U                     /*!< PCLK1 clock used as LPUART1 clock source */
#define LL_RCC_LPUART1_CLKSOURCE_SYSCLK    RCC_CCIPR_LPUART1SEL_0          /*!< SYSCLK clock used as LPUART1 clock source */
#define LL_RCC_LPUART1_CLKSOURCE_HSI       RCC_CCIPR_LPUART1SEL_1          /*!< HSI clock used as LPUART1 clock source */
#define LL_RCC_LPUART1_CLKSOURCE_LSE       RCC_CCIPR_LPUART1SEL            /*!< LSE clock used as LPUART1 clock source */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_I2C1_CLKSOURCE  Peripheral I2C clock source selection
  * @{
  */
#define LL_RCC_I2C1_CLKSOURCE_PCLK1        ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C1SEL_Pos << 16U))                                                  /*!< PCLK1 clock used as I2C1 clock source */
#define LL_RCC_I2C1_CLKSOURCE_SYSCLK       ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C1SEL_Pos << 16U) | (RCC_CCIPR_I2C1SEL_0 >> RCC_CCIPR_I2C1SEL_Pos)) /*!< SYSCLK clock used as I2C1 clock source */
#define LL_RCC_I2C1_CLKSOURCE_HSI          ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C1SEL_Pos << 16U) | (RCC_CCIPR_I2C1SEL_1 >> RCC_CCIPR_I2C1SEL_Pos)) /*!< HSI clock used as I2C1 clock source */
#if defined(RCC_CCIPR_I2C2SEL)
#define LL_RCC_I2C2_CLKSOURCE_PCLK1        ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C2SEL_Pos << 16U))                                                  /*!< PCLK1 clock used as I2C2 clock source */
#define LL_RCC_I2C2_CLKSOURCE_SYSCLK       ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C2SEL_Pos << 16U) | (RCC_CCIPR_I2C2SEL_0 >> RCC_CCIPR_I2C2SEL_Pos)) /*!< SYSCLK clock used as I2C2 clock source */
#define LL_RCC_I2C2_CLKSOURCE_HSI          ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C2SEL_Pos << 16U) | (RCC_CCIPR_I2C2SEL_1 >> RCC_CCIPR_I2C2SEL_Pos)) /*!< HSI clock used as I2C2 clock source */
#endif /* RCC_CCIPR_I2C2SEL */
#define LL_RCC_I2C3_CLKSOURCE_PCLK1        ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C3SEL_Pos << 16U))                                                  /*!< PCLK1 clock used as I2C3 clock source */
#define LL_RCC_I2C3_CLKSOURCE_SYSCLK       ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C3SEL_Pos << 16U) | (RCC_CCIPR_I2C3SEL_0 >> RCC_CCIPR_I2C3SEL_Pos)) /*!< SYSCLK clock used as I2C3 clock source */
#define LL_RCC_I2C3_CLKSOURCE_HSI          ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C3SEL_Pos << 16U) | (RCC_CCIPR_I2C3SEL_1 >> RCC_CCIPR_I2C3SEL_Pos)) /*!< HSI clock used as I2C3 clock source */
#if defined(RCC_CCIPR2_I2C4SEL)
#define LL_RCC_I2C4_CLKSOURCE_PCLK1        ((RCC_OFFSET_CCIPR2 << 24U) | (RCC_CCIPR2_I2C4SEL_Pos << 16U))                                                    /*!< PCLK1 clock used as I2C4 clock source */
#define LL_RCC_I2C4_CLKSOURCE_SYSCLK       ((RCC_OFFSET_CCIPR2 << 24U) | (RCC_CCIPR2_I2C4SEL_Pos << 16U) | (RCC_CCIPR2_I2C4SEL_0 >> RCC_CCIPR2_I2C4SEL_Pos)) /*!< SYSCLK clock used as I2C4 clock source */
#define LL_RCC_I2C4_CLKSOURCE_HSI          ((RCC_OFFSET_CCIPR2 << 24U) | (RCC_CCIPR2_I2C4SEL_Pos << 16U) | (RCC_CCIPR2_I2C4SEL_1 >> RCC_CCIPR2_I2C4SEL_Pos)) /*!< HSI clock used as I2C4 clock source */
#endif /* RCC_CCIPR2_I2C4SEL */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_LPTIM1_CLKSOURCE  Peripheral LPTIM clock source selection
  * @{
  */
#define LL_RCC_LPTIM1_CLKSOURCE_PCLK1      RCC_CCIPR_LPTIM1SEL                                    /*!< PCLK1 clock used as LPTIM1 clock source */
#define LL_RCC_LPTIM1_CLKSOURCE_LSI        (RCC_CCIPR_LPTIM1SEL | (RCC_CCIPR_LPTIM1SEL_0 >> 16U)) /*!< LSI clock used as LPTIM1 clock source */
#define LL_RCC_LPTIM1_CLKSOURCE_HSI        (RCC_CCIPR_LPTIM1SEL | (RCC_CCIPR_LPTIM1SEL_1 >> 16U)) /*!< HSI clock used as LPTIM1 clock source */
#define LL_RCC_LPTIM1_CLKSOURCE_LSE        (RCC_CCIPR_LPTIM1SEL | (RCC_CCIPR_LPTIM1SEL >> 16U))   /*!< LSE clock used as LPTIM1 clock source */
#define LL_RCC_LPTIM2_CLKSOURCE_PCLK1      RCC_CCIPR_LPTIM2SEL                                    /*!< PCLK1 clock used as LPTIM2 clock source */
#define LL_RCC_LPTIM2_CLKSOURCE_LSI        (RCC_CCIPR_LPTIM2SEL | (RCC_CCIPR_LPTIM2SEL_0 >> 16U)) /*!< LSI clock used as LPTIM2 clock source */
#define LL_RCC_LPTIM2_CLKSOURCE_HSI        (RCC_CCIPR_LPTIM2SEL | (RCC_CCIPR_LPTIM2SEL_1 >> 16U)) /*!< HSI clock used as LPTIM2 clock source */
#define LL_RCC_LPTIM2_CLKSOURCE_LSE        (RCC_CCIPR_LPTIM2SEL | (RCC_CCIPR_LPTIM2SEL >> 16U))   /*!< LSE clock used as LPTIM2 clock source */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_SAI1_CLKSOURCE  Peripheral SAI clock source selection
  * @{
  */
#if defined(RCC_CCIPR2_SAI1SEL)
#define LL_RCC_SAI1_CLKSOURCE_PLL          (RCC_CCIPR2_SAI1SEL << 16U)                          /*!< PLL clock used as SAI1 clock source */
#define LL_RCC_SAI1_CLKSOURCE_PLLSAI1      ((RCC_CCIPR2_SAI1SEL << 16U) | RCC_CCIPR2_SAI1SEL_0) /*!< PLLSAI1 clock used as SAI1 clock source */
#define LL_RCC_SAI1_CLKSOURCE_PLLSAI2      ((RCC_CCIPR2_SAI1SEL << 16U) | RCC_CCIPR2_SAI1SEL_1) /*!< PLLSAI2 clock used as SAI1 clock source */
#define LL_RCC_SAI1_CLKSOURCE_HSI          ((RCC_CCIPR2_SAI1SEL << 16U) | RCC_CCIPR2_SAI1SEL_2) /*!< HSI clock used as SAI1 clock source */
#define LL_RCC_SAI1_CLKSOURCE_PIN          ((RCC_CCIPR2_SAI1SEL << 16U) | (RCC_CCIPR2_SAI1SEL_1 | RCC_CCIPR2_SAI1SEL_0))  /*!< External input clock used as SAI1 clock source */
#elif defined(RCC_CCIPR_SAI1SEL)
#define LL_RCC_SAI1_CLKSOURCE_PLLSAI1      RCC_CCIPR_SAI1SEL                                    /*!< PLLSAI1 clock used as SAI1 clock source */
#if defined(RCC_PLLSAI2_SUPPORT)
#define LL_RCC_SAI1_CLKSOURCE_PLLSAI2      (RCC_CCIPR_SAI1SEL | (RCC_CCIPR_SAI1SEL_0 >> 16U))   /*!< PLLSAI2 clock used as SAI1 clock source */
#endif /* RCC_PLLSAI2_SUPPORT */
#define LL_RCC_SAI1_CLKSOURCE_PLL          (RCC_CCIPR_SAI1SEL | (RCC_CCIPR_SAI1SEL_1 >> 16U))   /*!< PLL clock used as SAI1 clock source */
#define LL_RCC_SAI1_CLKSOURCE_PIN          (RCC_CCIPR_SAI1SEL | (RCC_CCIPR_SAI1SEL >> 16U))     /*!< External input clock used as SAI1 clock source */
#endif /* RCC_CCIPR2_SAI1SEL */

#if defined(RCC_CCIPR2_SAI2SEL)
#define LL_RCC_SAI2_CLKSOURCE_PLL          (RCC_CCIPR2_SAI2SEL << 16U)                          /*!< PLL clock used as SAI2 clock source */
#define LL_RCC_SAI2_CLKSOURCE_PLLSAI1      ((RCC_CCIPR2_SAI2SEL << 16U) | RCC_CCIPR2_SAI2SEL_0) /*!< PLLSAI1 clock used as SAI2 clock source */
#define LL_RCC_SAI2_CLKSOURCE_PLLSAI2      ((RCC_CCIPR2_SAI2SEL << 16U) | RCC_CCIPR2_SAI2SEL_1) /*!< PLLSAI2 clock used as SAI2 clock source */
#define LL_RCC_SAI2_CLKSOURCE_HSI          ((RCC_CCIPR2_SAI2SEL << 16U) | RCC_CCIPR2_SAI2SEL_2) /*!< HSI clock used as SAI2 clock source */
#define LL_RCC_SAI2_CLKSOURCE_PIN          ((RCC_CCIPR2_SAI2SEL << 16U) | (RCC_CCIPR2_SAI2SEL_1 | RCC_CCIPR2_SAI2SEL_0))  /*!< External input clock used as SAI2 clock source */
#elif defined(RCC_CCIPR_SAI2SEL)
#define LL_RCC_SAI2_CLKSOURCE_PLLSAI1      RCC_CCIPR_SAI2SEL                                    /*!< PLLSAI1 clock used as SAI2 clock source */
#if defined(RCC_PLLSAI2_SUPPORT)
#define LL_RCC_SAI2_CLKSOURCE_PLLSAI2      (RCC_CCIPR_SAI2SEL | (RCC_CCIPR_SAI2SEL_0 >> 16U))   /*!< PLLSAI2 clock used as SAI2 clock source */
#endif /* RCC_PLLSAI2_SUPPORT */
#define LL_RCC_SAI2_CLKSOURCE_PLL          (RCC_CCIPR_SAI2SEL | (RCC_CCIPR_SAI2SEL_1 >> 16U))   /*!< PLL clock used as SAI2 clock source */
#define LL_RCC_SAI2_CLKSOURCE_PIN          (RCC_CCIPR_SAI2SEL | (RCC_CCIPR_SAI2SEL >> 16U))     /*!< External input clock used as SAI2 clock source */
#endif /* RCC_CCIPR2_SAI2SEL */
/**
  * @}
  */

#if defined(RCC_CCIPR2_SDMMCSEL)
/** @defgroup RCC_LL_EC_SDMMC1_KERNELCLKSOURCE  Peripheral SDMMC kernel clock source selection
  * @{
  */
#define LL_RCC_SDMMC1_KERNELCLKSOURCE_48CLK  0x00000000U          /*!< 48MHz clock from internal multiplexor used as SDMMC1 clock source */
#define LL_RCC_SDMMC1_KERNELCLKSOURCE_PLLP   RCC_CCIPR2_SDMMCSEL  /*!< PLLSAI3CLK clock used as SDMMC1 clock source */
/**
  * @}
  */
#endif /* RCC_CCIPR2_SDMMCSEL */

#if defined(SDMMC1)
/** @defgroup RCC_LL_EC_SDMMC1_CLKSOURCE  Peripheral SDMMC clock source selection
  * @{
  */
#if defined(RCC_HSI48_SUPPORT)
#define LL_RCC_SDMMC1_CLKSOURCE_HSI48      0x00000000U          /*!< HSI48 clock used as SDMMC1 clock source */
#else
#define LL_RCC_SDMMC1_CLKSOURCE_NONE       0x00000000U          /*!< No clock used as SDMMC1 clock source */
#endif
#if defined(RCC_PLLSAI1_SUPPORT)
#define LL_RCC_SDMMC1_CLKSOURCE_PLLSAI1    RCC_CCIPR_CLK48SEL_0 /*!< PLLSAI1 clock used as SDMMC1 clock source */
#endif /* RCC_PLLSAI1_SUPPORT */
#define LL_RCC_SDMMC1_CLKSOURCE_PLL        RCC_CCIPR_CLK48SEL_1 /*!< PLL clock used as SDMMC1 clock source */
#define LL_RCC_SDMMC1_CLKSOURCE_MSI        RCC_CCIPR_CLK48SEL   /*!< MSI clock used as SDMMC1 clock source */
/**
  * @}
  */
#endif /* SDMMC1 */

/** @defgroup RCC_LL_EC_RNG_CLKSOURCE  Peripheral RNG clock source selection
  * @{
  */
#if defined(RCC_HSI48_SUPPORT)
#define LL_RCC_RNG_CLKSOURCE_HSI48         0x00000000U          /*!< HSI48 clock used as RNG clock source */
#else
#define LL_RCC_RNG_CLKSOURCE_NONE          0x00000000U          /*!< No clock used as RNG clock source */
#endif
#if defined(RCC_PLLSAI1_SUPPORT)
#define LL_RCC_RNG_CLKSOURCE_PLLSAI1       RCC_CCIPR_CLK48SEL_0 /*!< PLLSAI1 clock used as RNG clock source */
#endif /* RCC_PLLSAI1_SUPPORT */
#define LL_RCC_RNG_CLKSOURCE_PLL           RCC_CCIPR_CLK48SEL_1 /*!< PLL clock used as RNG clock source */
#define LL_RCC_RNG_CLKSOURCE_MSI           RCC_CCIPR_CLK48SEL   /*!< MSI clock used as RNG clock source */
/**
  * @}
  */

#if defined(USB_OTG_FS) || defined(USB)
/** @defgroup RCC_LL_EC_USB_CLKSOURCE  Peripheral USB clock source selection
  * @{
  */
#if defined(RCC_HSI48_SUPPORT)
#define LL_RCC_USB_CLKSOURCE_HSI48         0x00000000U          /*!< HSI48 clock used as USB clock source */
#else
#define LL_RCC_USB_CLKSOURCE_NONE          0x00000000U          /*!< No clock used as USB clock source */
#endif
#if defined(RCC_PLLSAI1_SUPPORT)
#define LL_RCC_USB_CLKSOURCE_PLLSAI1       RCC_CCIPR_CLK48SEL_0 /*!< PLLSAI1 clock used as USB clock source */
#endif /* RCC_PLLSAI1_SUPPORT */
#define LL_RCC_USB_CLKSOURCE_PLL           RCC_CCIPR_CLK48SEL_1 /*!< PLL clock used as USB clock source */
#define LL_RCC_USB_CLKSOURCE_MSI           RCC_CCIPR_CLK48SEL   /*!< MSI clock used as USB clock source */
/**
  * @}
  */

#endif /* USB_OTG_FS || USB */

/** @defgroup RCC_LL_EC_ADC_CLKSOURCE  Peripheral ADC clock source selection
  * @{
  */
#define LL_RCC_ADC_CLKSOURCE_NONE          0x00000000U          /*!< No clock used as ADC clock source */
#if defined(RCC_PLLSAI1_SUPPORT)
#define LL_RCC_ADC_CLKSOURCE_PLLSAI1       RCC_CCIPR_ADCSEL_0   /*!< PLLSAI1 clock used as ADC clock source */
#endif /* RCC_PLLSAI1_SUPPORT */
#if defined(RCC_PLLSAI2_SUPPORT) && !defined(LTDC)
#define LL_RCC_ADC_CLKSOURCE_PLLSAI2       RCC_CCIPR_ADCSEL_1   /*!< PLLSAI2 clock used as ADC clock source */
#endif /* RCC_PLLSAI2_SUPPORT */
#if defined(RCC_CCIPR_ADCSEL)
#define LL_RCC_ADC_CLKSOURCE_SYSCLK        RCC_CCIPR_ADCSEL     /*!< SYSCLK clock used as ADC clock source */
#else
#define LL_RCC_ADC_CLKSOURCE_SYSCLK        0x30000000U          /*!< SYSCLK clock used as ADC clock source */
#endif
/**
  * @}
  */

#if defined(SWPMI1)
/** @defgroup RCC_LL_EC_SWPMI1_CLKSOURCE  Peripheral SWPMI1 clock source selection
  * @{
  */
#define LL_RCC_SWPMI1_CLKSOURCE_PCLK1      0x00000000U          /*!< PCLK1 used as SWPMI1 clock source */
#define LL_RCC_SWPMI1_CLKSOURCE_HSI        RCC_CCIPR_SWPMI1SEL  /*!< HSI used as SWPMI1 clock source */
/**
  * @}
  */
#endif /* SWPMI1 */

#if defined(DFSDM1_Channel0)
#if defined(RCC_CCIPR2_ADFSDM1SEL)
/** @defgroup RCC_LL_EC_DFSDM1_AUDIO_CLKSOURCE  Peripheral DFSDM1 Audio clock source selection
  * @{
  */
#define LL_RCC_DFSDM1_AUDIO_CLKSOURCE_SAI1 0x00000000U             /*!< SAI1 clock used as DFSDM1 Audio clock */
#define LL_RCC_DFSDM1_AUDIO_CLKSOURCE_HSI  RCC_CCIPR2_ADFSDM1SEL_0 /*!< HSI clock used as DFSDM1 Audio clock */
#define LL_RCC_DFSDM1_AUDIO_CLKSOURCE_MSI  RCC_CCIPR2_ADFSDM1SEL_1 /*!< MSI clock used as DFSDM1 Audio clock */
/**
  * @}
  */
#endif /* RCC_CCIPR2_ADFSDM1SEL */

/** @defgroup RCC_LL_EC_DFSDM1_CLKSOURCE  Peripheral DFSDM1 clock source selection
  * @{
  */
#if defined(RCC_CCIPR2_DFSDM1SEL)
#define LL_RCC_DFSDM1_CLKSOURCE_PCLK2      0x00000000U          /*!< PCLK2 used as DFSDM1 clock source */
#define LL_RCC_DFSDM1_CLKSOURCE_SYSCLK     RCC_CCIPR2_DFSDM1SEL /*!< SYSCLK used as DFSDM1 clock source */
#else
#define LL_RCC_DFSDM1_CLKSOURCE_PCLK2      0x00000000U          /*!< PCLK2 used as DFSDM1 clock source */
#define LL_RCC_DFSDM1_CLKSOURCE_SYSCLK     RCC_CCIPR_DFSDM1SEL  /*!< SYSCLK used as DFSDM1 clock source */
#endif /* RCC_CCIPR2_DFSDM1SEL */
/**
  * @}
  */
#endif /* DFSDM1_Channel0 */

#if defined(DSI)
/** @defgroup RCC_LL_EC_DSI_CLKSOURCE  Peripheral DSI clock source selection
  * @{
  */
#define LL_RCC_DSI_CLKSOURCE_PHY          0x00000000U           /*!< DSI-PHY clock used as DSI byte lane clock source */
#define LL_RCC_DSI_CLKSOURCE_PLL          RCC_CCIPR2_DSISEL     /*!< PLL clock used as DSI byte lane clock source */
/**
  * @}
  */
#endif /* DSI */

#if defined(LTDC)
/** @defgroup RCC_LL_EC_LTDC_CLKSOURCE  Peripheral LTDC clock source selection
  * @{
  */
#define LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV2  0x00000000U              /*!< PLLSAI2DIVR divided by 2 used as LTDC clock source */
#define LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV4  RCC_CCIPR2_PLLSAI2DIVR_0 /*!< PLLSAI2DIVR divided by 4 used as LTDC clock source */
#define LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV8  RCC_CCIPR2_PLLSAI2DIVR_1 /*!< PLLSAI2DIVR divided by 8 used as LTDC clock source */
#define LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV16 RCC_CCIPR2_PLLSAI2DIVR   /*!< PLLSAI2DIVR divided by 16 used as LTDC clock source */
/**
  * @}
  */
#endif /* LTDC */

#if defined(OCTOSPI1)
/** @defgroup RCC_LL_EC_OCTOSPI  Peripheral OCTOSPI get clock source
  * @{
  */
#define LL_RCC_OCTOSPI_CLKSOURCE_SYSCLK    0x00000000U           /*!< SYSCLK used as OctoSPI clock source */
#define LL_RCC_OCTOSPI_CLKSOURCE_MSI       RCC_CCIPR2_OSPISEL_0  /*!< MSI used as OctoSPI clock source */
#define LL_RCC_OCTOSPI_CLKSOURCE_PLL       RCC_CCIPR2_OSPISEL_1  /*!< PLL used as OctoSPI clock source */
/**
  * @}
  */
#endif /* OCTOSPI1 */

/** @defgroup RCC_LL_EC_USART1 Peripheral USART get clock source
  * @{
  */
#define LL_RCC_USART1_CLKSOURCE            RCC_CCIPR_USART1SEL /*!< USART1 Clock source selection */
#define LL_RCC_USART2_CLKSOURCE            RCC_CCIPR_USART2SEL /*!< USART2 Clock source selection */
#if defined(RCC_CCIPR_USART3SEL)
#define LL_RCC_USART3_CLKSOURCE            RCC_CCIPR_USART3SEL /*!< USART3 Clock source selection */
#endif /* RCC_CCIPR_USART3SEL */
/**
  * @}
  */

#if defined(RCC_CCIPR_UART4SEL) || defined(RCC_CCIPR_UART5SEL)
/** @defgroup RCC_LL_EC_UART4 Peripheral UART get clock source
  * @{
  */
#if defined(RCC_CCIPR_UART4SEL)
#define LL_RCC_UART4_CLKSOURCE             RCC_CCIPR_UART4SEL /*!< UART4 Clock source selection */
#endif /* RCC_CCIPR_UART4SEL */
#if defined(RCC_CCIPR_UART5SEL)
#define LL_RCC_UART5_CLKSOURCE             RCC_CCIPR_UART5SEL /*!< UART5 Clock source selection */
#endif /* RCC_CCIPR_UART5SEL */
/**
  * @}
  */
#endif /* RCC_CCIPR_UART4SEL || RCC_CCIPR_UART5SEL */

/** @defgroup RCC_LL_EC_LPUART1 Peripheral LPUART get clock source
  * @{
  */
#define LL_RCC_LPUART1_CLKSOURCE           RCC_CCIPR_LPUART1SEL /*!< LPUART1 Clock source selection */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_I2C1 Peripheral I2C get clock source
  * @{
  */
#define LL_RCC_I2C1_CLKSOURCE              ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C1SEL_Pos << 16U) | (RCC_CCIPR_I2C1SEL >> RCC_CCIPR_I2C1SEL_Pos)) /*!< I2C1 Clock source selection */
#if defined(RCC_CCIPR_I2C2SEL)
#define LL_RCC_I2C2_CLKSOURCE              ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C2SEL_Pos << 16U) | (RCC_CCIPR_I2C2SEL >> RCC_CCIPR_I2C2SEL_Pos)) /*!< I2C2 Clock source selection */
#endif /* RCC_CCIPR_I2C2SEL */
#define LL_RCC_I2C3_CLKSOURCE              ((RCC_OFFSET_CCIPR << 24U) | (RCC_CCIPR_I2C3SEL_Pos << 16U) | (RCC_CCIPR_I2C3SEL >> RCC_CCIPR_I2C3SEL_Pos)) /*!< I2C3 Clock source selection */
#if defined(RCC_CCIPR2_I2C4SEL)
#define LL_RCC_I2C4_CLKSOURCE              ((RCC_OFFSET_CCIPR2 << 24U) | (RCC_CCIPR2_I2C4SEL_Pos << 16U) | (RCC_CCIPR2_I2C4SEL >> RCC_CCIPR2_I2C4SEL_Pos)) /*!< I2C4 Clock source selection */
#endif /* RCC_CCIPR2_I2C4SEL */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_LPTIM1 Peripheral LPTIM get clock source
  * @{
  */
#define LL_RCC_LPTIM1_CLKSOURCE            RCC_CCIPR_LPTIM1SEL /*!< LPTIM1 Clock source selection */
#define LL_RCC_LPTIM2_CLKSOURCE            RCC_CCIPR_LPTIM2SEL /*!< LPTIM2 Clock source selection */
/**
  * @}
  */

#if defined(RCC_CCIPR_SAI1SEL) || defined(RCC_CCIPR2_SAI1SEL)
/** @defgroup RCC_LL_EC_SAI1  Peripheral SAI get clock source
  * @{
  */
#if defined(RCC_CCIPR2_SAI1SEL)
#define LL_RCC_SAI1_CLKSOURCE              RCC_CCIPR2_SAI1SEL /*!< SAI1 Clock source selection */
#else
#define LL_RCC_SAI1_CLKSOURCE              RCC_CCIPR_SAI1SEL /*!< SAI1 Clock source selection */
#endif /* RCC_CCIPR2_SAI1SEL */
#if defined(RCC_CCIPR2_SAI2SEL)
#define LL_RCC_SAI2_CLKSOURCE              RCC_CCIPR2_SAI2SEL /*!< SAI2 Clock source selection */
#elif defined(RCC_CCIPR_SAI2SEL)
#define LL_RCC_SAI2_CLKSOURCE              RCC_CCIPR_SAI2SEL /*!< SAI2 Clock source selection */
#endif /* RCC_CCIPR2_SAI2SEL */
/**
  * @}
  */
#endif /* RCC_CCIPR_SAI1SEL || RCC_CCIPR2_SAI1SEL */

#if defined(SDMMC1)
#if defined(RCC_CCIPR2_SDMMCSEL)
/** @defgroup RCC_LL_EC_SDMMC1_KERNEL  Peripheral SDMMC get kernel clock source
  * @{
  */
#define LL_RCC_SDMMC1_KERNELCLKSOURCE      RCC_CCIPR2_SDMMCSEL /*!< SDMMC1 Kernel Clock source selection */
/**
  * @}
  */
#endif /* RCC_CCIPR2_SDMMCSEL */

/** @defgroup RCC_LL_EC_SDMMC1  Peripheral SDMMC get clock source
  * @{
  */
#define LL_RCC_SDMMC1_CLKSOURCE            RCC_CCIPR_CLK48SEL /*!< SDMMC1 Clock source selection */
/**
  * @}
  */
#endif /* SDMMC1 */

/** @defgroup RCC_LL_EC_RNG  Peripheral RNG get clock source
  * @{
  */
#define LL_RCC_RNG_CLKSOURCE               RCC_CCIPR_CLK48SEL /*!< RNG Clock source selection */
/**
  * @}
  */

#if defined(USB_OTG_FS) || defined(USB)
/** @defgroup RCC_LL_EC_USB  Peripheral USB get clock source
  * @{
  */
#define LL_RCC_USB_CLKSOURCE               RCC_CCIPR_CLK48SEL /*!< USB Clock source selection */
/**
  * @}
  */
#endif /* USB_OTG_FS || USB */

/** @defgroup RCC_LL_EC_ADC  Peripheral ADC get clock source
  * @{
  */
#if defined(RCC_CCIPR_ADCSEL)
#define LL_RCC_ADC_CLKSOURCE               RCC_CCIPR_ADCSEL /*!< ADC Clock source selection */
#else
#define LL_RCC_ADC_CLKSOURCE               0x30000000U /*!< ADC Clock source selection */
#endif
/**
  * @}
  */

#if defined(SWPMI1)
/** @defgroup RCC_LL_EC_SWPMI1  Peripheral SWPMI1 get clock source
  * @{
  */
#define LL_RCC_SWPMI1_CLKSOURCE            RCC_CCIPR_SWPMI1SEL /*!< SWPMI1 Clock source selection */
/**
  * @}
  */
#endif /* SWPMI1 */

#if defined(DFSDM1_Channel0)
#if defined(RCC_CCIPR2_ADFSDM1SEL)
/** @defgroup RCC_LL_EC_DFSDM1_AUDIO  Peripheral DFSDM1 Audio get clock source
  * @{
  */
#define LL_RCC_DFSDM1_AUDIO_CLKSOURCE      RCC_CCIPR2_ADFSDM1SEL /* DFSDM1 Audio Clock source selection */
/**
  * @}
  */

#endif /* RCC_CCIPR2_ADFSDM1SEL */
/** @defgroup RCC_LL_EC_DFSDM1  Peripheral DFSDM1 get clock source
  * @{
  */
#if defined(RCC_CCIPR2_DFSDM1SEL)
#define LL_RCC_DFSDM1_CLKSOURCE            RCC_CCIPR2_DFSDM1SEL /*!< DFSDM1 Clock source selection */
#else
#define LL_RCC_DFSDM1_CLKSOURCE            RCC_CCIPR_DFSDM1SEL /*!< DFSDM1 Clock source selection */
#endif /* RCC_CCIPR2_DFSDM1SEL */
/**
  * @}
  */
#endif /* DFSDM1_Channel0 */

#if defined(DSI)
/** @defgroup RCC_LL_EC_DSI  Peripheral DSI get clock source
  * @{
  */
#define LL_RCC_DSI_CLKSOURCE               RCC_CCIPR2_DSISEL      /*!< DSI Clock source selection */
/**
  * @}
  */
#endif /* DSI */

#if defined(LTDC)
/** @defgroup RCC_LL_EC_LTDC  Peripheral LTDC get clock source
  * @{
  */
#define LL_RCC_LTDC_CLKSOURCE              RCC_CCIPR2_PLLSAI2DIVR /*!< LTDC Clock source selection */
/**
  * @}
  */
#endif /* LTDC */

#if defined(OCTOSPI1)
/** @defgroup RCC_LL_EC_OCTOSPI  Peripheral OCTOSPI get clock source
  * @{
  */
#define LL_RCC_OCTOSPI_CLKSOURCE           RCC_CCIPR2_OSPISEL    /*!< OctoSPI Clock source selection */
/**
  * @}
  */
#endif /* OCTOSPI1 */


/** @defgroup RCC_LL_EC_RTC_CLKSOURCE  RTC clock source selection
  * @{
  */
#define LL_RCC_RTC_CLKSOURCE_NONE          0x00000000U                   /*!< No clock used as RTC clock */
#define LL_RCC_RTC_CLKSOURCE_LSE           RCC_BDCR_RTCSEL_0       /*!< LSE oscillator clock used as RTC clock */
#define LL_RCC_RTC_CLKSOURCE_LSI           RCC_BDCR_RTCSEL_1       /*!< LSI oscillator clock used as RTC clock */
#define LL_RCC_RTC_CLKSOURCE_HSE_DIV32     RCC_BDCR_RTCSEL         /*!< HSE oscillator clock divided by 32 used as RTC clock */
/**
  * @}
  */


/** @defgroup RCC_LL_EC_PLLSOURCE  PLL, PLLSAI1 and PLLSAI2 entry clock source
  * @{
  */
#define LL_RCC_PLLSOURCE_NONE              0x00000000U             /*!< No clock */
#define LL_RCC_PLLSOURCE_MSI               RCC_PLLCFGR_PLLSRC_MSI  /*!< MSI clock selected as PLL entry clock source */
#define LL_RCC_PLLSOURCE_HSI               RCC_PLLCFGR_PLLSRC_HSI  /*!< HSI16 clock selected as PLL entry clock source */
#define LL_RCC_PLLSOURCE_HSE               RCC_PLLCFGR_PLLSRC_HSE  /*!< HSE clock selected as PLL entry clock source */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_PLLM_DIV  PLL division factor
  * @{
  */
#define LL_RCC_PLLM_DIV_1                  0x00000000U                                                    /*!< Main PLL division factor for PLLM input by 1 */
#define LL_RCC_PLLM_DIV_2                  (RCC_PLLCFGR_PLLM_0)                                           /*!< Main PLL division factor for PLLM input by 2 */
#define LL_RCC_PLLM_DIV_3                  (RCC_PLLCFGR_PLLM_1)                                           /*!< Main PLL division factor for PLLM input by 3 */
#define LL_RCC_PLLM_DIV_4                  (RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_0)                      /*!< Main PLL division factor for PLLM input by 4 */
#define LL_RCC_PLLM_DIV_5                  (RCC_PLLCFGR_PLLM_2)                                           /*!< Main PLL division factor for PLLM input by 5 */
#define LL_RCC_PLLM_DIV_6                  (RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_0)                      /*!< Main PLL division factor for PLLM input by 6 */
#define LL_RCC_PLLM_DIV_7                  (RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_1)                      /*!< Main PLL division factor for PLLM input by 7 */
#define LL_RCC_PLLM_DIV_8                  (RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_0) /*!< Main PLL division factor for PLLM input by 8 */
#if defined(RCC_PLLM_DIV_1_16_SUPPORT)
#define LL_RCC_PLLM_DIV_9                  (RCC_PLLCFGR_PLLM_3)                                           /*!< Main PLL division factor for PLLM input by 9 */
#define LL_RCC_PLLM_DIV_10                 (RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_0)                      /*!< Main PLL division factor for PLLM input by 10 */
#define LL_RCC_PLLM_DIV_11                 (RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_1)                      /*!< Main PLL division factor for PLLM input by 11 */
#define LL_RCC_PLLM_DIV_12                 (RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_0) /*!< Main PLL division factor for PLLM input by 12 */
#define LL_RCC_PLLM_DIV_13                 (RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_2)                      /*!< Main PLL division factor for PLLM input by 13 */
#define LL_RCC_PLLM_DIV_14                 (RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_0) /*!< Main PLL division factor for PLLM input by 14 */
#define LL_RCC_PLLM_DIV_15                 (RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_1) /*!< Main PLL division factor for PLLM input by 15 */
#define LL_RCC_PLLM_DIV_16                 (RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_0) /*!< Main PLL division factor for PLLM input by 16 */
#endif /* RCC_PLLM_DIV_1_16_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_PLLR_DIV  PLL division factor (PLLR)
  * @{
  */
#define LL_RCC_PLLR_DIV_2                  0x00000000U            /*!< Main PLL division factor for PLLCLK (system clock) by 2 */
#define LL_RCC_PLLR_DIV_4                  (RCC_PLLCFGR_PLLR_0)   /*!< Main PLL division factor for PLLCLK (system clock) by 4 */
#define LL_RCC_PLLR_DIV_6                  (RCC_PLLCFGR_PLLR_1)   /*!< Main PLL division factor for PLLCLK (system clock) by 6 */
#define LL_RCC_PLLR_DIV_8                  (RCC_PLLCFGR_PLLR)     /*!< Main PLL division factor for PLLCLK (system clock) by 8 */
/**
  * @}
  */

#if defined(RCC_PLLP_SUPPORT)
/** @defgroup RCC_LL_EC_PLLP_DIV  PLL division factor (PLLP)
  * @{
  */
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
#define LL_RCC_PLLP_DIV_2                  (RCC_PLLCFGR_PLLPDIV_1)                                              /*!< Main PLL division factor for PLLP output by 2 */
#define LL_RCC_PLLP_DIV_3                  (RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0)                        /*!< Main PLL division factor for PLLP output by 3 */
#define LL_RCC_PLLP_DIV_4                  (RCC_PLLCFGR_PLLPDIV_2)                                              /*!< Main PLL division factor for PLLP output by 4 */
#define LL_RCC_PLLP_DIV_5                  (RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_0)                        /*!< Main PLL division factor for PLLP output by 5 */
#define LL_RCC_PLLP_DIV_6                  (RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1)                        /*!< Main PLL division factor for PLLP output by 6 */
#define LL_RCC_PLLP_DIV_7                  (RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0)  /*!< Main PLL division factor for PLLP output by 7 */
#define LL_RCC_PLLP_DIV_8                  (RCC_PLLCFGR_PLLPDIV_3)                                              /*!< Main PLL division factor for PLLP output by 8 */
#define LL_RCC_PLLP_DIV_9                  (RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_0)                        /*!< Main PLL division factor for PLLP output by 9 */
#define LL_RCC_PLLP_DIV_10                 (RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_1)                        /*!< Main PLL division factor for PLLP output by 10 */
#define LL_RCC_PLLP_DIV_11                 (RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0)  /*!< Main PLL division factor for PLLP output by 11 */
#define LL_RCC_PLLP_DIV_12                 (RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2)                        /*!< Main PLL division factor for PLLP output by 12 */
#define LL_RCC_PLLP_DIV_13                 (RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_0)  /*!< Main PLL division factor for PLLP output by 13 */
#define LL_RCC_PLLP_DIV_14                 (RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1)  /*!< Main PLL division factor for PLLP output by 14 */
#define LL_RCC_PLLP_DIV_15                 (RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0) /*!< Main PLL division factor for PLLP output by 15 */
#define LL_RCC_PLLP_DIV_16                 (RCC_PLLCFGR_PLLPDIV_4)                                              /*!< Main PLL division factor for PLLP output by 16 */
#define LL_RCC_PLLP_DIV_17                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_0)                        /*!< Main PLL division factor for PLLP output by 17 */
#define LL_RCC_PLLP_DIV_18                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_1)                        /*!< Main PLL division factor for PLLP output by 18 */
#define LL_RCC_PLLP_DIV_19                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0)  /*!< Main PLL division factor for PLLP output by 19 */
#define LL_RCC_PLLP_DIV_20                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_2)                        /*!< Main PLL division factor for PLLP output by 20 */
#define LL_RCC_PLLP_DIV_21                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_0)  /*!< Main PLL division factor for PLLP output by 21 */
#define LL_RCC_PLLP_DIV_22                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1)  /*!< Main PLL division factor for PLLP output by 22 */
#define LL_RCC_PLLP_DIV_23                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0) /*!< Main PLL division factor for PLLP output by 23 */
#define LL_RCC_PLLP_DIV_24                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3)                        /*!< Main PLL division factor for PLLP output by 24 */
#define LL_RCC_PLLP_DIV_25                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_0)  /*!< Main PLL division factor for PLLP output by 25 */
#define LL_RCC_PLLP_DIV_26                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_1)  /*!< Main PLL division factor for PLLP output by 26 */
#define LL_RCC_PLLP_DIV_27                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0) /*!< Main PLL division factor for PLLP output by 27 */
#define LL_RCC_PLLP_DIV_28                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2)  /*!< Main PLL division factor for PLLP output by 28 */
#define LL_RCC_PLLP_DIV_29                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_0) /*!< Main PLL division factor for PLLP output by 29 */
#define LL_RCC_PLLP_DIV_30                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1) /*!< Main PLL division factor for PLLP output by 30 */
#define LL_RCC_PLLP_DIV_31                 (RCC_PLLCFGR_PLLPDIV_4|RCC_PLLCFGR_PLLPDIV_3|RCC_PLLCFGR_PLLPDIV_2|RCC_PLLCFGR_PLLPDIV_1|RCC_PLLCFGR_PLLPDIV_0) /*!< Main PLL division factor for PLLP output by 31 */
#else
#define LL_RCC_PLLP_DIV_7                  0x00000000U            /*!< Main PLL division factor for PLLP output by 7 */
#define LL_RCC_PLLP_DIV_17                 (RCC_PLLCFGR_PLLP)     /*!< Main PLL division factor for PLLP output by 17 */
#endif /* RCC_PLLP_DIV_2_31_SUPPORT */
/**
  * @}
  */
#endif /* RCC_PLLP_SUPPORT */

/** @defgroup RCC_LL_EC_PLLQ_DIV  PLL division factor (PLLQ)
  * @{
  */
#define LL_RCC_PLLQ_DIV_2                  0x00000000U             /*!< Main PLL division factor for PLLQ output by 2 */
#define LL_RCC_PLLQ_DIV_4                  (RCC_PLLCFGR_PLLQ_0)    /*!< Main PLL division factor for PLLQ output by 4 */
#define LL_RCC_PLLQ_DIV_6                  (RCC_PLLCFGR_PLLQ_1)    /*!< Main PLL division factor for PLLQ output by 6 */
#define LL_RCC_PLLQ_DIV_8                  (RCC_PLLCFGR_PLLQ)      /*!< Main PLL division factor for PLLQ output by 8 */
/**
  * @}
  */

#if defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT)
/** @defgroup RCC_LL_EC_PLLSAI1M  PLLSAI1 division factor (PLLSAI1M)
  * @{
  */
#define LL_RCC_PLLSAI1M_DIV_1              0x00000000U                                             /*!< PLLSAI1 division factor for PLLSAI1M input by 1 */
#define LL_RCC_PLLSAI1M_DIV_2              (RCC_PLLSAI1CFGR_PLLSAI1M_0)                            /*!< PLLSAI1 division factor for PLLSAI1M input by 2 */
#define LL_RCC_PLLSAI1M_DIV_3              (RCC_PLLSAI1CFGR_PLLSAI1M_1)                            /*!< PLLSAI1 division factor for PLLSAI1M input by 3 */
#define LL_RCC_PLLSAI1M_DIV_4              (RCC_PLLSAI1CFGR_PLLSAI1M_1|RCC_PLLSAI1CFGR_PLLSAI1M_0) /*!< PLLSAI1 division factor for PLLSAI1M input by 4 */
#define LL_RCC_PLLSAI1M_DIV_5              (RCC_PLLSAI1CFGR_PLLSAI1M_2)                            /*!< PLLSAI1 division factor for PLLSAI1M input by 5 */
#define LL_RCC_PLLSAI1M_DIV_6              (RCC_PLLSAI1CFGR_PLLSAI1M_2|RCC_PLLSAI1CFGR_PLLSAI1M_0) /*!< PLLSAI1 division factor for PLLSAI1M input by 6 */
#define LL_RCC_PLLSAI1M_DIV_7              (RCC_PLLSAI1CFGR_PLLSAI1M_2|RCC_PLLSAI1CFGR_PLLSAI1M_1) /*!< PLLSAI1 division factor for PLLSAI1M input by 7 */
#define LL_RCC_PLLSAI1M_DIV_8              (RCC_PLLSAI1CFGR_PLLSAI1M_2|RCC_PLLSAI1CFGR_PLLSAI1M_1|RCC_PLLSAI1CFGR_PLLSAI1M_0) /*!< PLLSAI1 division factor for PLLSAI1M input by 8 */
#define LL_RCC_PLLSAI1M_DIV_9              (RCC_PLLSAI1CFGR_PLLSAI1M_3)                            /*!< PLLSAI1 division factor for PLLSAI1M input by 9 */
#define LL_RCC_PLLSAI1M_DIV_10             (RCC_PLLSAI1CFGR_PLLSAI1M_3|RCC_PLLSAI1CFGR_PLLSAI1M_0) /*!< PLLSAI1 division factor for PLLSAI1M input by 10 */
#define LL_RCC_PLLSAI1M_DIV_11             (RCC_PLLSAI1CFGR_PLLSAI1M_3|RCC_PLLSAI1CFGR_PLLSAI1M_1) /*!< PLLSAI1 division factor for PLLSAI1M input by 11 */
#define LL_RCC_PLLSAI1M_DIV_12             (RCC_PLLSAI1CFGR_PLLSAI1M_3|RCC_PLLSAI1CFGR_PLLSAI1M_1|RCC_PLLSAI1CFGR_PLLSAI1M_0) /*!< PLLSAI1 division factor for PLLSAI1M input by 12 */
#define LL_RCC_PLLSAI1M_DIV_13             (RCC_PLLSAI1CFGR_PLLSAI1M_3|RCC_PLLSAI1CFGR_PLLSAI1M_2) /*!< PLLSAI1 division factor for PLLSAI1M input by 13 */
#define LL_RCC_PLLSAI1M_DIV_14             (RCC_PLLSAI1CFGR_PLLSAI1M_3|RCC_PLLSAI1CFGR_PLLSAI1M_2|RCC_PLLSAI1CFGR_PLLSAI1M_0) /*!< PLLSAI1 division factor for PLLSAI1M input by 14 */
#define LL_RCC_PLLSAI1M_DIV_15             (RCC_PLLSAI1CFGR_PLLSAI1M_3|RCC_PLLSAI1CFGR_PLLSAI1M_2|RCC_PLLSAI1CFGR_PLLSAI1M_1) /*!< PLLSAI1 division factor for PLLSAI1M input by 15 */
#define LL_RCC_PLLSAI1M_DIV_16             (RCC_PLLSAI1CFGR_PLLSAI1M_3|RCC_PLLSAI1CFGR_PLLSAI1M_2|RCC_PLLSAI1CFGR_PLLSAI1M_1|RCC_PLLSAI1CFGR_PLLSAI1M_0) /*!< PLLSAI1 division factor for PLLSAI1M input by 16 */
/**
  * @}
  */
#endif /* RCC_PLLSAI1M_DIV_1_16_SUPPORT */

#if defined(RCC_PLLSAI1_SUPPORT)
/** @defgroup RCC_LL_EC_PLLSAI1Q  PLLSAI1 division factor (PLLSAI1Q)
  * @{
  */
#define LL_RCC_PLLSAI1Q_DIV_2              0x00000000U                  /*!< PLLSAI1 division factor for PLLSAI1Q output by 2 */
#define LL_RCC_PLLSAI1Q_DIV_4              (RCC_PLLSAI1CFGR_PLLSAI1Q_0) /*!< PLLSAI1 division factor for PLLSAI1Q output by 4 */
#define LL_RCC_PLLSAI1Q_DIV_6              (RCC_PLLSAI1CFGR_PLLSAI1Q_1) /*!< PLLSAI1 division factor for PLLSAI1Q output by 6 */
#define LL_RCC_PLLSAI1Q_DIV_8              (RCC_PLLSAI1CFGR_PLLSAI1Q)   /*!< PLLSAI1 division factor for PLLSAI1Q output by 8 */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_PLLSAI1P  PLLSAI1 division factor (PLLSAI1P)
  * @{
  */
#if defined(RCC_PLLSAI1P_DIV_2_31_SUPPORT)
#define LL_RCC_PLLSAI1P_DIV_2              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_1)                               /*!< PLLSAI1 division factor for PLLSAI1P output by 2 */
#define LL_RCC_PLLSAI1P_DIV_3              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 3 */
#define LL_RCC_PLLSAI1P_DIV_4              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_2)                               /*!< PLLSAI1 division factor for PLLSAI1P output by 4 */
#define LL_RCC_PLLSAI1P_DIV_5              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 5 */
#define LL_RCC_PLLSAI1P_DIV_6              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1) /*!< PLLSAI1 division factor for PLLSAI1P output by 6 */
#define LL_RCC_PLLSAI1P_DIV_7              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 7 */
#define LL_RCC_PLLSAI1P_DIV_8              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3)                               /*!< PLLSAI1 division factor for PLLSAI1P output by 8 */
#define LL_RCC_PLLSAI1P_DIV_9              (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 9 */
#define LL_RCC_PLLSAI1P_DIV_10             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1) /*!< PLLSAI1 division factor for PLLSAI1P output by 10 */
#define LL_RCC_PLLSAI1P_DIV_11             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 1 */
#define LL_RCC_PLLSAI1P_DIV_12             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2) /*!< PLLSAI1 division factor for PLLSAI1P output by 12 */
#define LL_RCC_PLLSAI1P_DIV_13             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 13 */
#define LL_RCC_PLLSAI1P_DIV_14             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1) /*!< PLLSAI1 division factor for PLLSAI1P output by 14 */
#define LL_RCC_PLLSAI1P_DIV_15             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 15 */
#define LL_RCC_PLLSAI1P_DIV_16             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4)                               /*!< PLLSAI1 division factor for PLLSAI1P output by 16 */
#define LL_RCC_PLLSAI1P_DIV_17             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 17 */
#define LL_RCC_PLLSAI1P_DIV_18             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1) /*!< PLLSAI1 division factor for PLLSAI1P output by 18 */
#define LL_RCC_PLLSAI1P_DIV_19             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 19 */
#define LL_RCC_PLLSAI1P_DIV_20             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2) /*!< PLLSAI1 division factor for PLLSAI1P output by 20 */
#define LL_RCC_PLLSAI1P_DIV_21             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division fctor for PLLSAI1P output by 21 */
#define LL_RCC_PLLSAI1P_DIV_22             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1) /*!< PLLSAI1 division factor for PLLSAI1P output by 22 */
#define LL_RCC_PLLSAI1P_DIV_23             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 23 */
#define LL_RCC_PLLSAI1P_DIV_24             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3) /*!< PLLSAI1 division factor for PLLSAI1P output by 24 */
#define LL_RCC_PLLSAI1P_DIV_25             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 25 */
#define LL_RCC_PLLSAI1P_DIV_26             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1) /*!< PLLSAI1 division factor for PLLSAI1P output by 26 */
#define LL_RCC_PLLSAI1P_DIV_27             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 27 */
#define LL_RCC_PLLSAI1P_DIV_28             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2) /*!< PLLSAI1 division factor for PLLSAI1P output by 28 */
#define LL_RCC_PLLSAI1P_DIV_29             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 29 */
#define LL_RCC_PLLSAI1P_DIV_30             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1) /*!< PLLSAI1 division factor for PLLSAI1P output by 30 */
#define LL_RCC_PLLSAI1P_DIV_31             (RCC_PLLSAI1CFGR_PLLSAI1PDIV_4|RCC_PLLSAI1CFGR_PLLSAI1PDIV_3|RCC_PLLSAI1CFGR_PLLSAI1PDIV_2|RCC_PLLSAI1CFGR_PLLSAI1PDIV_1|RCC_PLLSAI1CFGR_PLLSAI1PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 31 */
#else
#define LL_RCC_PLLSAI1P_DIV_7              0x00000000U                /*!< PLLSAI1 division factor for PLLSAI1P output by 7 */
#define LL_RCC_PLLSAI1P_DIV_17             (RCC_PLLSAI1CFGR_PLLSAI1P) /*!< PLLSAI1 division factor for PLLSAI1P output by 17 */
#endif /* RCC_PLLSAI1P_DIV_2_31_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_PLLSAI1R  PLLSAI1 division factor (PLLSAI1R)
  * @{
  */
#define LL_RCC_PLLSAI1R_DIV_2              0x00000000U                  /*!< PLLSAI1 division factor for PLLSAI1R output by 2 */
#define LL_RCC_PLLSAI1R_DIV_4              (RCC_PLLSAI1CFGR_PLLSAI1R_0) /*!< PLLSAI1 division factor for PLLSAI1R output by 4 */
#define LL_RCC_PLLSAI1R_DIV_6              (RCC_PLLSAI1CFGR_PLLSAI1R_1) /*!< PLLSAI1 division factor for PLLSAI1R output by 6 */
#define LL_RCC_PLLSAI1R_DIV_8              (RCC_PLLSAI1CFGR_PLLSAI1R)   /*!< PLLSAI1 division factor for PLLSAI1R output by 8 */
/**
  * @}
  */
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
#if defined(RCC_PLLSAI2M_DIV_1_16_SUPPORT)
/** @defgroup RCC_LL_EC_PLLSAI2M  PLLSAI1 division factor (PLLSAI2M)
  * @{
  */
#define LL_RCC_PLLSAI2M_DIV_1              0x00000000U                                             /*!< PLLSAI2 division factor for PLLSAI2M input by 1 */
#define LL_RCC_PLLSAI2M_DIV_2              (RCC_PLLSAI2CFGR_PLLSAI2M_0)                            /*!< PLLSAI2 division factor for PLLSAI2M input by 2 */
#define LL_RCC_PLLSAI2M_DIV_3              (RCC_PLLSAI2CFGR_PLLSAI2M_1)                            /*!< PLLSAI2 division factor for PLLSAI2M input by 3 */
#define LL_RCC_PLLSAI2M_DIV_4              (RCC_PLLSAI2CFGR_PLLSAI2M_1|RCC_PLLSAI2CFGR_PLLSAI2M_0) /*!< PLLSAI2 division factor for PLLSAI2M input by 4 */
#define LL_RCC_PLLSAI2M_DIV_5              (RCC_PLLSAI2CFGR_PLLSAI2M_2)                            /*!< PLLSAI2 division factor for PLLSAI2M input by 5 */
#define LL_RCC_PLLSAI2M_DIV_6              (RCC_PLLSAI2CFGR_PLLSAI2M_2|RCC_PLLSAI2CFGR_PLLSAI2M_0) /*!< PLLSAI2 division factor for PLLSAI2M input by 6 */
#define LL_RCC_PLLSAI2M_DIV_7              (RCC_PLLSAI2CFGR_PLLSAI2M_2|RCC_PLLSAI2CFGR_PLLSAI2M_1) /*!< PLLSAI2 division factor for PLLSAI2M input by 7 */
#define LL_RCC_PLLSAI2M_DIV_8              (RCC_PLLSAI2CFGR_PLLSAI2M_2|RCC_PLLSAI2CFGR_PLLSAI2M_1|RCC_PLLSAI2CFGR_PLLSAI2M_0) /*!< PLLSAI2 division factor for PLLSAI2M input by 8 */
#define LL_RCC_PLLSAI2M_DIV_9              (RCC_PLLSAI2CFGR_PLLSAI2M_3)                            /*!< PLLSAI2 division factor for PLLSAI2M input by 9 */
#define LL_RCC_PLLSAI2M_DIV_10             (RCC_PLLSAI2CFGR_PLLSAI2M_3|RCC_PLLSAI2CFGR_PLLSAI2M_0) /*!< PLLSAI2 division factor for PLLSAI2M input by 10 */
#define LL_RCC_PLLSAI2M_DIV_11             (RCC_PLLSAI2CFGR_PLLSAI2M_3|RCC_PLLSAI2CFGR_PLLSAI2M_1) /*!< PLLSAI2 division factor for PLLSAI2M input by 11 */
#define LL_RCC_PLLSAI2M_DIV_12             (RCC_PLLSAI2CFGR_PLLSAI2M_3|RCC_PLLSAI2CFGR_PLLSAI2M_1|RCC_PLLSAI2CFGR_PLLSAI2M_0) /*!< PLLSAI2 division factor for PLLSAI2M input by 12 */
#define LL_RCC_PLLSAI2M_DIV_13             (RCC_PLLSAI2CFGR_PLLSAI2M_3|RCC_PLLSAI2CFGR_PLLSAI2M_2) /*!< PLLSAI2 division factor for PLLSAI2M input by 13 */
#define LL_RCC_PLLSAI2M_DIV_14             (RCC_PLLSAI2CFGR_PLLSAI2M_3|RCC_PLLSAI2CFGR_PLLSAI2M_2|RCC_PLLSAI2CFGR_PLLSAI2M_0) /*!< PLLSAI2 division factor for PLLSAI2M input by 14 */
#define LL_RCC_PLLSAI2M_DIV_15             (RCC_PLLSAI2CFGR_PLLSAI2M_3|RCC_PLLSAI2CFGR_PLLSAI2M_2|RCC_PLLSAI2CFGR_PLLSAI2M_1) /*!< PLLSAI2 division factor for PLLSAI2M input by 15 */
#define LL_RCC_PLLSAI2M_DIV_16             (RCC_PLLSAI2CFGR_PLLSAI2M_3|RCC_PLLSAI2CFGR_PLLSAI2M_2|RCC_PLLSAI2CFGR_PLLSAI2M_1|RCC_PLLSAI2CFGR_PLLSAI2M_0) /*!< PLLSAI2 division factor for PLLSAI2M input by 16 */
/**
  * @}
  */
#endif /* RCC_PLLSAI2M_DIV_1_16_SUPPORT */

#if defined(RCC_PLLSAI2Q_DIV_SUPPORT)
/** @defgroup RCC_LL_EC_PLLSAI2Q  PLLSAI2 division factor (PLLSAI2Q)
  * @{
  */
#define LL_RCC_PLLSAI2Q_DIV_2              0x00000000U                  /*!< PLLSAI2 division factor for PLLSAI2Q output by 2 */
#define LL_RCC_PLLSAI2Q_DIV_4              (RCC_PLLSAI2CFGR_PLLSAI2Q_0) /*!< PLLSAI2 division factor for PLLSAI2Q output by 4 */
#define LL_RCC_PLLSAI2Q_DIV_6              (RCC_PLLSAI2CFGR_PLLSAI2Q_1) /*!< PLLSAI2 division factor for PLLSAI2Q output by 6 */
#define LL_RCC_PLLSAI2Q_DIV_8              (RCC_PLLSAI2CFGR_PLLSAI2Q)   /*!< PLLSAI2 division factor for PLLSAI2Q output by 8 */
/**
  * @}
  */
#endif /* RCC_PLLSAI2Q_DIV_SUPPORT */

/** @defgroup RCC_LL_EC_PLLSAI2P  PLLSAI2 division factor (PLLSAI2P)
  * @{
  */
#if defined(RCC_PLLSAI2P_DIV_2_31_SUPPORT)
#define LL_RCC_PLLSAI2P_DIV_2              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_1)                               /*!< PLLSAI2 division factor for PLLSAI2P output by 2 */
#define LL_RCC_PLLSAI2P_DIV_3              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 3 */
#define LL_RCC_PLLSAI2P_DIV_4              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_2)                               /*!< PLLSAI2 division factor for PLLSAI2P output by 4 */
#define LL_RCC_PLLSAI2P_DIV_5              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 5 */
#define LL_RCC_PLLSAI2P_DIV_6              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1) /*!< PLLSAI2 division factor for PLLSAI2P output by 6 */
#define LL_RCC_PLLSAI2P_DIV_7              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 7 */
#define LL_RCC_PLLSAI2P_DIV_8              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3)                               /*!< PLLSAI2 division factor for PLLSAI2P output by 8 */
#define LL_RCC_PLLSAI2P_DIV_9              (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 9 */
#define LL_RCC_PLLSAI2P_DIV_10             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1) /*!< PLLSAI2 division factor for PLLSAI2P output by 10 */
#define LL_RCC_PLLSAI2P_DIV_11             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 1 */
#define LL_RCC_PLLSAI2P_DIV_12             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2) /*!< PLLSAI2 division factor for PLLSAI2P output by 12 */
#define LL_RCC_PLLSAI2P_DIV_13             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 13 */
#define LL_RCC_PLLSAI2P_DIV_14             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1) /*!< PLLSAI2 division factor for PLLSAI2P output by 14 */
#define LL_RCC_PLLSAI2P_DIV_15             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 15 */
#define LL_RCC_PLLSAI2P_DIV_16             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4)                               /*!< PLLSAI2 division factor for PLLSAI2P output by 16 */
#define LL_RCC_PLLSAI2P_DIV_17             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 17 */
#define LL_RCC_PLLSAI2P_DIV_18             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1) /*!< PLLSAI2 division factor for PLLSAI2P output by 18 */
#define LL_RCC_PLLSAI2P_DIV_19             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 19 */
#define LL_RCC_PLLSAI2P_DIV_20             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2) /*!< PLLSAI2 division factor for PLLSAI2P output by 20 */
#define LL_RCC_PLLSAI2P_DIV_21             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division fctor for PLLSAI2P output by 21 */
#define LL_RCC_PLLSAI2P_DIV_22             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1) /*!< PLLSAI2 division factor for PLLSAI2P output by 22 */
#define LL_RCC_PLLSAI2P_DIV_23             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 23 */
#define LL_RCC_PLLSAI2P_DIV_24             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3) /*!< PLLSAI2 division factor for PLLSAI2P output by 24 */
#define LL_RCC_PLLSAI2P_DIV_25             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 25 */
#define LL_RCC_PLLSAI2P_DIV_26             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1) /*!< PLLSAI2 division factor for PLLSAI2P output by 26 */
#define LL_RCC_PLLSAI2P_DIV_27             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 27 */
#define LL_RCC_PLLSAI2P_DIV_28             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2) /*!< PLLSAI2 division factor for PLLSAI2P output by 28 */
#define LL_RCC_PLLSAI2P_DIV_29             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI2 division factor for PLLSAI2P output by 29 */
#define LL_RCC_PLLSAI2P_DIV_30             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1) /*!< PLLSAI2 division factor for PLLSAI2P output by 30 */
#define LL_RCC_PLLSAI2P_DIV_31             (RCC_PLLSAI2CFGR_PLLSAI2PDIV_4|RCC_PLLSAI2CFGR_PLLSAI2PDIV_3|RCC_PLLSAI2CFGR_PLLSAI2PDIV_2|RCC_PLLSAI2CFGR_PLLSAI2PDIV_1|RCC_PLLSAI2CFGR_PLLSAI2PDIV_0) /*!< PLLSAI1 division factor for PLLSAI1P output by 31 */
#else
#define LL_RCC_PLLSAI2P_DIV_7              0x00000000U                /*!< PLLSAI2 division factor for PLLSAI2P output by 7 */
#define LL_RCC_PLLSAI2P_DIV_17             (RCC_PLLSAI2CFGR_PLLSAI2P) /*!< PLLSAI2 division factor for PLLSAI2P output by 17 */
#endif /* RCC_PLLSAI2P_DIV_2_31_SUPPORT */
/**
  * @}
  */

/** @defgroup RCC_LL_EC_PLLSAI2R  PLLSAI2 division factor (PLLSAI2R)
  * @{
  */
#define LL_RCC_PLLSAI2R_DIV_2              0x00000000U                  /*!< PLLSAI2 division factor for PLLSAI2R output by 2 */
#define LL_RCC_PLLSAI2R_DIV_4              (RCC_PLLSAI2CFGR_PLLSAI2R_0) /*!< PLLSAI2 division factor for PLLSAI2R output by 4 */
#define LL_RCC_PLLSAI2R_DIV_6              (RCC_PLLSAI2CFGR_PLLSAI2R_1) /*!< PLLSAI2 division factor for PLLSAI2R output by 6 */
#define LL_RCC_PLLSAI2R_DIV_8              (RCC_PLLSAI2CFGR_PLLSAI2R)   /*!< PLLSAI2 division factor for PLLSAI2R output by 8 */
/**
  * @}
  */

#if defined(RCC_CCIPR2_PLLSAI2DIVR)
/** @defgroup RCC_LL_EC_PLLSAI2DIVR  PLLSAI2DIVR division factor (PLLSAI2DIVR)
  * @{
  */
#define LL_RCC_PLLSAI2DIVR_DIV_2           0x00000000U                     /*!< PLLSAI2 division factor for PLLSAI2DIVR output by 2 */
#define LL_RCC_PLLSAI2DIVR_DIV_4           RCC_CCIPR2_PLLSAI2DIVR_0        /*!< PLLSAI2 division factor for PLLSAI2DIVR output by 4 */
#define LL_RCC_PLLSAI2DIVR_DIV_8           RCC_CCIPR2_PLLSAI2DIVR_1        /*!< PLLSAI2 division factor for PLLSAI2DIVR output by 8 */
#define LL_RCC_PLLSAI2DIVR_DIV_16          (RCC_CCIPR2_PLLSAI2DIVR_1 | RCC_CCIPR2_PLLSAI2DIVR_0) /*!< PLLSAI2 division factor for PLLSAI2DIVR output by 16 */
/**
  * @}
  */
#endif /* RCC_CCIPR2_PLLSAI2DIVR */
#endif /* RCC_PLLSAI2_SUPPORT */

/** @defgroup RCC_LL_EC_MSIRANGESEL  MSI clock range selection
  * @{
  */
#define LL_RCC_MSIRANGESEL_STANDBY         0U                  /*!< MSI Range is provided by MSISRANGE */
#define LL_RCC_MSIRANGESEL_RUN             1U                  /*!< MSI Range is provided by MSIRANGE */
/**
  * @}
  */

#if defined(RCC_CSR_LSIPREDIV)
/** @defgroup RCC_LL_EC_LSIPREDIV  LSI division factor
  * @{
  */
#define LL_RCC_LSI_PREDIV_1                0x00000000U         /*!< LSI division factor by 1   */
#define LL_RCC_LSI_PREDIV_128              RCC_CSR_LSIPREDIV   /*!< LSI division factor by 128 */
/**
  * @}
  */
#endif /* RCC_CSR_LSIPREDIV */

/** Legacy definitions for compatibility purpose
@cond 0
*/
#if defined(DFSDM1_Channel0)
#define LL_RCC_DFSDM1_CLKSOURCE_PCLK       LL_RCC_DFSDM1_CLKSOURCE_PCLK2
#define LL_RCC_DFSDM_CLKSOURCE_PCLK        LL_RCC_DFSDM1_CLKSOURCE_PCLK2
#define LL_RCC_DFSDM_CLKSOURCE_SYSCLK      LL_RCC_DFSDM1_CLKSOURCE_SYSCLK
#define LL_RCC_DFSDM_CLKSOURCE             LL_RCC_DFSDM1_CLKSOURCE
#endif /* DFSDM1_Channel0 */
#if defined(SWPMI1)
#define LL_RCC_SWPMI1_CLKSOURCE_PCLK       LL_RCC_SWPMI1_CLKSOURCE_PCLK1
#endif /* SWPMI1 */
/**
@endcond
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup RCC_LL_Exported_Macros RCC Exported Macros
  * @{
  */

/** @defgroup RCC_LL_EM_WRITE_READ Common Write and read registers Macros
  * @{
  */

/**
  * @brief  Write a value in RCC register
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_RCC_WriteReg(__REG__, __VALUE__) WRITE_REG(RCC->__REG__, (__VALUE__))

/**
  * @brief  Read a value in RCC register
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_RCC_ReadReg(__REG__) READ_REG(RCC->__REG__)
/**
  * @}
  */

/** @defgroup RCC_LL_EM_CALC_FREQ Calculate frequencies
  * @{
  */

/**
  * @brief  Helper macro to calculate the PLLCLK frequency on system domain
  * @note ex: @ref __LL_RCC_CALC_PLLCLK_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLL_GetN (), @ref LL_RCC_PLL_GetR ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  *         @arg @ref LL_RCC_PLLM_DIV_9 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_10 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_11 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_12 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_13 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_14 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_15 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_16 (*)
  *
  *         (*) value not defined in all devices.
  * @param  __PLLN__ Between 8 and 86
  * @param  __PLLR__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLR_DIV_2
  *         @arg @ref LL_RCC_PLLR_DIV_4
  *         @arg @ref LL_RCC_PLLR_DIV_6
  *         @arg @ref LL_RCC_PLLR_DIV_8
  * @retval PLL clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLCLK_FREQ(__INPUTFREQ__, __PLLM__, __PLLN__, __PLLR__) ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLN__) / \
                   ((((__PLLR__) >> RCC_PLLCFGR_PLLR_Pos) + 1U) * 2U))

#if defined(RCC_PLLSAI1_SUPPORT)
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLCLK frequency used on SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLCLK_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLL_GetN (), @ref LL_RCC_PLL_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  *         @arg @ref LL_RCC_PLLM_DIV_9 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_10 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_11 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_12 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_13 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_14 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_15 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_16 (*)
  *
  *         (*) value not defined in all devices.
  * @param  __PLLN__ Between 8 and 86
  * @param  __PLLP__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLP_DIV_2
  *         @arg @ref LL_RCC_PLLP_DIV_3
  *         @arg @ref LL_RCC_PLLP_DIV_4
  *         @arg @ref LL_RCC_PLLP_DIV_5
  *         @arg @ref LL_RCC_PLLP_DIV_6
  *         @arg @ref LL_RCC_PLLP_DIV_7
  *         @arg @ref LL_RCC_PLLP_DIV_8
  *         @arg @ref LL_RCC_PLLP_DIV_9
  *         @arg @ref LL_RCC_PLLP_DIV_10
  *         @arg @ref LL_RCC_PLLP_DIV_11
  *         @arg @ref LL_RCC_PLLP_DIV_12
  *         @arg @ref LL_RCC_PLLP_DIV_13
  *         @arg @ref LL_RCC_PLLP_DIV_14
  *         @arg @ref LL_RCC_PLLP_DIV_15
  *         @arg @ref LL_RCC_PLLP_DIV_16
  *         @arg @ref LL_RCC_PLLP_DIV_17
  *         @arg @ref LL_RCC_PLLP_DIV_18
  *         @arg @ref LL_RCC_PLLP_DIV_19
  *         @arg @ref LL_RCC_PLLP_DIV_20
  *         @arg @ref LL_RCC_PLLP_DIV_21
  *         @arg @ref LL_RCC_PLLP_DIV_22
  *         @arg @ref LL_RCC_PLLP_DIV_23
  *         @arg @ref LL_RCC_PLLP_DIV_24
  *         @arg @ref LL_RCC_PLLP_DIV_25
  *         @arg @ref LL_RCC_PLLP_DIV_26
  *         @arg @ref LL_RCC_PLLP_DIV_27
  *         @arg @ref LL_RCC_PLLP_DIV_28
  *         @arg @ref LL_RCC_PLLP_DIV_29
  *         @arg @ref LL_RCC_PLLP_DIV_30
  *         @arg @ref LL_RCC_PLLP_DIV_31
  * @retval PLL clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLCLK_SAI_FREQ(__INPUTFREQ__, __PLLM__, __PLLN__, __PLLP__) ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLN__) / \
                   ((__PLLP__) >> RCC_PLLCFGR_PLLPDIV_Pos))

#else
/**
  * @brief  Helper macro to calculate the PLLCLK frequency used on SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLCLK_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLL_GetN (), @ref LL_RCC_PLL_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLN__ Between 8 and 86
  * @param  __PLLP__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLP_DIV_7
  *         @arg @ref LL_RCC_PLLP_DIV_17
  * @retval PLL clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLCLK_SAI_FREQ(__INPUTFREQ__, __PLLM__, __PLLN__, __PLLP__) ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLN__) / \
                   (((__PLLP__) == LL_RCC_PLLP_DIV_7) ? 7U : 17U))

#endif /* RCC_PLLP_DIV_2_31_SUPPORT */
#endif /* RCC_PLLSAI1_SUPPORT */

/**
  * @brief  Helper macro to calculate the PLLCLK frequency used on 48M domain
  * @note ex: @ref __LL_RCC_CALC_PLLCLK_48M_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLL_GetN (), @ref LL_RCC_PLL_GetQ ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  *         @arg @ref LL_RCC_PLLM_DIV_9 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_10 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_11 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_12 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_13 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_14 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_15 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_16 (*)
  *
  *         (*) value not defined in all devices.
  * @param  __PLLN__ Between 8 and 86
  * @param  __PLLQ__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLQ_DIV_2
  *         @arg @ref LL_RCC_PLLQ_DIV_4
  *         @arg @ref LL_RCC_PLLQ_DIV_6
  *         @arg @ref LL_RCC_PLLQ_DIV_8
  * @retval PLL clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLCLK_48M_FREQ(__INPUTFREQ__, __PLLM__, __PLLN__, __PLLQ__) ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLN__) / \
                   ((((__PLLQ__) >> RCC_PLLCFGR_PLLQ_Pos) + 1U) << 1U))

#if defined(RCC_PLLSAI1_SUPPORT)
#if defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT) && defined(RCC_PLLSAI1P_DIV_2_31_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLSAI1 frequency used for SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI1_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLLSAI1_GetDivider (),
  *             @ref LL_RCC_PLLSAI1_GetN (), @ref LL_RCC_PLLSAI1_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLSAI1M__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_16
  * @param  __PLLSAI1N__ Between 8 and 86
  * @param  __PLLSAI1P__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_31
  * @retval PLLSAI1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI1_SAI_FREQ(__INPUTFREQ__, __PLLSAI1M__, __PLLSAI1N__, __PLLSAI1P__) \
                   ((__INPUTFREQ__) / ((((__PLLSAI1M__) >> RCC_PLLSAI1CFGR_PLLSAI1M_Pos) + 1U)) * (__PLLSAI1N__) / \
                    ((__PLLSAI1P__) >> RCC_PLLSAI1CFGR_PLLSAI1PDIV_Pos))

#elif defined(RCC_PLLSAI1P_DIV_2_31_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLSAI1 frequency used for SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI1_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLLSAI1_GetN (), @ref LL_RCC_PLLSAI1_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLSAI1N__ Between 8 and 86
  * @param  __PLLSAI1P__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_31
  * @retval PLLSAI1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI1_SAI_FREQ(__INPUTFREQ__, __PLLM__, __PLLSAI1N__, __PLLSAI1P__) \
                   ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLSAI1N__) / \
                    ((__PLLSAI1P__) >> RCC_PLLSAI1CFGR_PLLSAI1PDIV_Pos))

#else
/**
  * @brief  Helper macro to calculate the PLLSAI1 frequency used for SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI1_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLLSAI1_GetN (), @ref LL_RCC_PLLSAI1_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLSAI1N__ Between 8 and 86
  * @param  __PLLSAI1P__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  * @retval PLLSAI1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI1_SAI_FREQ(__INPUTFREQ__, __PLLM__, __PLLSAI1N__, __PLLSAI1P__) \
                   ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLSAI1N__) / \
                    (((__PLLSAI1P__) == LL_RCC_PLLSAI1P_DIV_7) ? 7U : 17U))

#endif /* RCC_PLLSAI1P_DIV_2_31_SUPPORT */

#if defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLSAI1 frequency used on 48M domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI1_48M_FREQ (HSE_VALUE,@ref LL_RCC_PLLSAI1_GetDivider (),
  *             @ref LL_RCC_PLLSAI1_GetN (), @ref LL_RCC_PLLSAI1_GetQ ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLSAI1M__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_16
  * @param  __PLLSAI1N__ Between 8 and 86
  * @param  __PLLSAI1Q__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_8
  * @retval PLLSAI1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI1_48M_FREQ(__INPUTFREQ__, __PLLSAI1M__, __PLLSAI1N__, __PLLSAI1Q__) \
                   ((__INPUTFREQ__) / ((((__PLLSAI1M__) >> RCC_PLLSAI1CFGR_PLLSAI1M_Pos) + 1U)) * (__PLLSAI1N__) / \
                    ((((__PLLSAI1Q__) >> RCC_PLLSAI1CFGR_PLLSAI1Q_Pos) + 1U) << 1U))

#else
/**
  * @brief  Helper macro to calculate the PLLSAI1 frequency used on 48M domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI1_48M_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLLSAI1_GetN (), @ref LL_RCC_PLLSAI1_GetQ ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLSAI1N__ Between 8 and 86
  * @param  __PLLSAI1Q__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_8
  * @retval PLLSAI1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI1_48M_FREQ(__INPUTFREQ__, __PLLM__, __PLLSAI1N__, __PLLSAI1Q__) \
                   ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLSAI1N__) / \
                    ((((__PLLSAI1Q__) >> RCC_PLLSAI1CFGR_PLLSAI1Q_Pos) + 1U) << 1U))

#endif /* RCC_PLLSAI1M_DIV_1_16_SUPPORT */

#if defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLSAI1 frequency used on ADC domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI1_ADC_FREQ (HSE_VALUE,@ref LL_RCC_PLLSAI1_GetDivider (),
  *             @ref LL_RCC_PLLSAI1_GetN (), @ref LL_RCC_PLLSAI1_GetR ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLSAI1M__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_16
  * @param  __PLLSAI1N__ Between 8 and 86
  * @param  __PLLSAI1R__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_8
  * @retval PLLSAI1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI1_ADC_FREQ(__INPUTFREQ__, __PLLSAI1M__, __PLLSAI1N__, __PLLSAI1R__) \
                   ((__INPUTFREQ__) / ((((__PLLSAI1M__) >> RCC_PLLSAI1CFGR_PLLSAI1M_Pos) + 1U)) * (__PLLSAI1N__) / \
                    ((((__PLLSAI1R__) >> RCC_PLLSAI1CFGR_PLLSAI1R_Pos) + 1U) << 1U))

#else
/**
  * @brief  Helper macro to calculate the PLLSAI1 frequency used on ADC domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI1_ADC_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLLSAI1_GetN (), @ref LL_RCC_PLLSAI1_GetR ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLSAI1N__ Between 8 and 86
  * @param  __PLLSAI1R__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_8
  * @retval PLLSAI1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI1_ADC_FREQ(__INPUTFREQ__, __PLLM__, __PLLSAI1N__, __PLLSAI1R__) \
                   ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLSAI1N__) / \
                    ((((__PLLSAI1R__) >> RCC_PLLSAI1CFGR_PLLSAI1R_Pos) + 1U) << 1U))

#endif /* RCC_PLLSAI1M_DIV_1_16_SUPPORT */
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2M_DIV_1_16_SUPPORT) && defined(RCC_PLLSAI2P_DIV_2_31_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLSAI2 frequency used for SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI2_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLLSAI2_GetDivider (),
  *             @ref LL_RCC_PLLSAI2_GetN (), @ref LL_RCC_PLLSAI2_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLSAI2M__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_16
  * @param  __PLLSAI2N__ Between 8 and 86
  * @param  __PLLSAI2P__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_31
  * @retval PLLSAI2 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI2_SAI_FREQ(__INPUTFREQ__, __PLLSAI2M__, __PLLSAI2N__, __PLLSAI2P__) \
                   ((__INPUTFREQ__) / ((((__PLLSAI2M__) >> RCC_PLLSAI2CFGR_PLLSAI2M_Pos) + 1U)) * (__PLLSAI2N__) / \
                    ((__PLLSAI2P__) >> RCC_PLLSAI2CFGR_PLLSAI2PDIV_Pos))

#elif defined(RCC_PLLSAI2P_DIV_2_31_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLSAI2 frequency used for SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI2_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLLSAI2_GetN (), @ref LL_RCC_PLLSAI2_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLSAI2N__ Between 8 and 86
  * @param  __PLLSAI2P__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_31
  * @retval PLLSAI2 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI2_SAI_FREQ(__INPUTFREQ__, __PLLM__, __PLLSAI2N__, __PLLSAI2P__) \
                   ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLSAI2N__) / \
                    ((__PLLSAI2P__) >> RCC_PLLSAI2CFGR_PLLSAI2PDIV_Pos))

#else
/**
  * @brief  Helper macro to calculate the PLLSAI2 frequency used for SAI domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI2_SAI_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLLSAI2_GetN (), @ref LL_RCC_PLLSAI2_GetP ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLSAI2N__ Between 8 and 86
  * @param  __PLLSAI2P__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  * @retval PLLSAI2 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI2_SAI_FREQ(__INPUTFREQ__, __PLLM__, __PLLSAI2N__, __PLLSAI2P__) \
                   ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1)) * (__PLLSAI2N__) / \
                    (((__PLLSAI2P__) == LL_RCC_PLLSAI2P_DIV_7) ? 7U : 17U))

#endif /* RCC_PLLSAI2P_DIV_2_31_SUPPORT */

#if defined(LTDC)
/**
  * @brief  Helper macro to calculate the PLLSAI2 frequency used for LTDC domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI2_LTDC_FREQ (HSE_VALUE,@ref LL_RCC_PLLSAI2_GetDivider (),
  *             @ref LL_RCC_PLLSAI2_GetN (), @ref LL_RCC_PLLSAI2_GetR (), @ref LL_RCC_PLLSAI2_GetDIVR ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on HSE/HSI/MSI)
  * @param  __PLLSAI2M__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_16
  * @param  __PLLSAI2N__ Between 8 and 86
  * @param  __PLLSAI2R__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_8
  * @param  __PLLSAI2DIVR__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_16
  * @retval PLLSAI2 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI2_LTDC_FREQ(__INPUTFREQ__, __PLLSAI2M__, __PLLSAI2N__, __PLLSAI2R__, __PLLSAI2DIVR__) \
                   (((__INPUTFREQ__) / (((__PLLSAI2M__)>> RCC_PLLSAI2CFGR_PLLSAI2M_Pos) + 1U)) * (__PLLSAI2N__) / \
                    (((((__PLLSAI2R__) >> RCC_PLLSAI2CFGR_PLLSAI2R_Pos ) + 1U) << 1U) * (2UL << ((__PLLSAI2DIVR__) >> RCC_CCIPR2_PLLSAI2DIVR_Pos))))
#elif defined(RCC_PLLSAI2_SUPPORT)
/**
  * @brief  Helper macro to calculate the PLLSAI2 frequency used on ADC domain
  * @note ex: @ref __LL_RCC_CALC_PLLSAI2_ADC_FREQ (HSE_VALUE,@ref LL_RCC_PLL_GetDivider (),
  *             @ref LL_RCC_PLLSAI2_GetN (), @ref LL_RCC_PLLSAI2_GetR ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on MSI/HSE/HSI)
  * @param  __PLLM__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  __PLLSAI2N__ Between 8 and 86
  * @param  __PLLSAI2R__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_8
  * @retval PLLSAI2 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI2_ADC_FREQ(__INPUTFREQ__, __PLLM__, __PLLSAI2N__, __PLLSAI2R__) \
                   ((__INPUTFREQ__) / ((((__PLLM__)>> RCC_PLLCFGR_PLLM_Pos) + 1U)) * (__PLLSAI2N__) / \
                    ((((__PLLSAI2R__) >> RCC_PLLSAI2CFGR_PLLSAI2R_Pos ) + 1U) << 1U))

#endif /* LTDC */

#if defined(DSI)
/**
  * @brief  Helper macro to calculate the PLLDSICLK frequency used on DSI
  * @note ex: @ref __LL_RCC_CALC_PLLSAI2_DSI_FREQ (HSE_VALUE,@ref LL_RCC_PLLSAI2_GetDivider (),
  *             @ref LL_RCC_PLLSAI2_GetN (), @ref LL_RCC_PLLSAI2_GetQ ());
  * @param  __INPUTFREQ__ PLL Input frequency (based on HSE/HSI/MSI)
  * @param  __PLLSAI2M__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_16
  * @param  __PLLSAI2N__ Between 8 and 86
  * @param  __PLLSAI2Q__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_8
  * @retval PLL clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PLLSAI2_DSI_FREQ(__INPUTFREQ__, __PLLSAI2M__, __PLLSAI2N__, __PLLSAI2Q__) \
                   ((__INPUTFREQ__) / ((((__PLLSAI2M__) >> RCC_PLLSAI2CFGR_PLLSAI2M_Pos) + 1U)) * (__PLLSAI2N__) / \
                    ((((__PLLSAI2Q__) >> RCC_PLLSAI2CFGR_PLLSAI2Q_Pos) + 1U) << 1U))
#endif /* DSI */



/**
  * @brief  Helper macro to calculate the HCLK frequency
  * @param  __SYSCLKFREQ__ SYSCLK frequency (based on MSI/HSE/HSI/PLLCLK)
  * @param  __AHBPRESCALER__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SYSCLK_DIV_1
  *         @arg @ref LL_RCC_SYSCLK_DIV_2
  *         @arg @ref LL_RCC_SYSCLK_DIV_4
  *         @arg @ref LL_RCC_SYSCLK_DIV_8
  *         @arg @ref LL_RCC_SYSCLK_DIV_16
  *         @arg @ref LL_RCC_SYSCLK_DIV_64
  *         @arg @ref LL_RCC_SYSCLK_DIV_128
  *         @arg @ref LL_RCC_SYSCLK_DIV_256
  *         @arg @ref LL_RCC_SYSCLK_DIV_512
  * @retval HCLK clock frequency (in Hz)
  */
#define __LL_RCC_CALC_HCLK_FREQ(__SYSCLKFREQ__, __AHBPRESCALER__) ((__SYSCLKFREQ__) >> AHBPrescTable[((__AHBPRESCALER__) & RCC_CFGR_HPRE) >>  RCC_CFGR_HPRE_Pos])

/**
  * @brief  Helper macro to calculate the PCLK1 frequency (ABP1)
  * @param  __HCLKFREQ__ HCLK frequency
  * @param  __APB1PRESCALER__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_APB1_DIV_1
  *         @arg @ref LL_RCC_APB1_DIV_2
  *         @arg @ref LL_RCC_APB1_DIV_4
  *         @arg @ref LL_RCC_APB1_DIV_8
  *         @arg @ref LL_RCC_APB1_DIV_16
  * @retval PCLK1 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PCLK1_FREQ(__HCLKFREQ__, __APB1PRESCALER__) ((__HCLKFREQ__) >> APBPrescTable[(__APB1PRESCALER__) >>  RCC_CFGR_PPRE1_Pos])

/**
  * @brief  Helper macro to calculate the PCLK2 frequency (ABP2)
  * @param  __HCLKFREQ__ HCLK frequency
  * @param  __APB2PRESCALER__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_APB2_DIV_1
  *         @arg @ref LL_RCC_APB2_DIV_2
  *         @arg @ref LL_RCC_APB2_DIV_4
  *         @arg @ref LL_RCC_APB2_DIV_8
  *         @arg @ref LL_RCC_APB2_DIV_16
  * @retval PCLK2 clock frequency (in Hz)
  */
#define __LL_RCC_CALC_PCLK2_FREQ(__HCLKFREQ__, __APB2PRESCALER__) ((__HCLKFREQ__) >> APBPrescTable[(__APB2PRESCALER__) >>  RCC_CFGR_PPRE2_Pos])

/**
  * @brief  Helper macro to calculate the MSI frequency (in Hz)
  * @note __MSISEL__ can be retrieved thanks to function LL_RCC_MSI_IsEnabledRangeSelect()
  * @note if __MSISEL__ is equal to LL_RCC_MSIRANGESEL_STANDBY,
  *        __MSIRANGE__can be retrieved by LL_RCC_MSI_GetRangeAfterStandby()
  *        else by LL_RCC_MSI_GetRange()
  *        ex: __LL_RCC_CALC_MSI_FREQ(LL_RCC_MSI_IsEnabledRangeSelect(),
  *              (LL_RCC_MSI_IsEnabledRangeSelect()?
  *               LL_RCC_MSI_GetRange():
  *               LL_RCC_MSI_GetRangeAfterStandby()))
  * @param  __MSISEL__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_MSIRANGESEL_STANDBY
  *         @arg @ref LL_RCC_MSIRANGESEL_RUN
  * @param  __MSIRANGE__ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_MSIRANGE_0
  *         @arg @ref LL_RCC_MSIRANGE_1
  *         @arg @ref LL_RCC_MSIRANGE_2
  *         @arg @ref LL_RCC_MSIRANGE_3
  *         @arg @ref LL_RCC_MSIRANGE_4
  *         @arg @ref LL_RCC_MSIRANGE_5
  *         @arg @ref LL_RCC_MSIRANGE_6
  *         @arg @ref LL_RCC_MSIRANGE_7
  *         @arg @ref LL_RCC_MSIRANGE_8
  *         @arg @ref LL_RCC_MSIRANGE_9
  *         @arg @ref LL_RCC_MSIRANGE_10
  *         @arg @ref LL_RCC_MSIRANGE_11
  *         @arg @ref LL_RCC_MSISRANGE_4
  *         @arg @ref LL_RCC_MSISRANGE_5
  *         @arg @ref LL_RCC_MSISRANGE_6
  *         @arg @ref LL_RCC_MSISRANGE_7
  * @retval MSI clock frequency (in Hz)
  */
#define __LL_RCC_CALC_MSI_FREQ(__MSISEL__, __MSIRANGE__)   (((__MSISEL__) == LL_RCC_MSIRANGESEL_STANDBY) ? \
                           (MSIRangeTable[(__MSIRANGE__) >> 8U]) : \
                           (MSIRangeTable[(__MSIRANGE__) >> 4U]))

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup RCC_LL_Exported_Functions RCC Exported Functions
  * @{
  */

/** @defgroup RCC_LL_EF_HSE HSE
  * @{
  */

/**
  * @brief  Enable the Clock Security System.
  * @rmtoll CR           CSSON         LL_RCC_HSE_EnableCSS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSE_EnableCSS(void)
{
  SET_BIT(RCC->CR, RCC_CR_CSSON);
}

/**
  * @brief  Enable HSE external oscillator (HSE Bypass)
  * @rmtoll CR           HSEBYP        LL_RCC_HSE_EnableBypass
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSE_EnableBypass(void)
{
  SET_BIT(RCC->CR, RCC_CR_HSEBYP);
}

/**
  * @brief  Disable HSE external oscillator (HSE Bypass)
  * @rmtoll CR           HSEBYP        LL_RCC_HSE_DisableBypass
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSE_DisableBypass(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);
}

/**
  * @brief  Enable HSE crystal oscillator (HSE ON)
  * @rmtoll CR           HSEON         LL_RCC_HSE_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSE_Enable(void)
{
  SET_BIT(RCC->CR, RCC_CR_HSEON);
}

/**
  * @brief  Disable HSE crystal oscillator (HSE ON)
  * @rmtoll CR           HSEON         LL_RCC_HSE_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSE_Disable(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_HSEON);
}

/**
  * @brief  Check if HSE oscillator Ready
  * @rmtoll CR           HSERDY        LL_RCC_HSE_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_HSE_IsReady(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_HSERDY) == RCC_CR_HSERDY) ? 1UL : 0UL);
}

/**
  * @}
  */

/** @defgroup RCC_LL_EF_HSI HSI
  * @{
  */

/**
  * @brief  Enable HSI even in stop mode
  * @note HSI oscillator is forced ON even in Stop mode
  * @rmtoll CR           HSIKERON      LL_RCC_HSI_EnableInStopMode
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI_EnableInStopMode(void)
{
  SET_BIT(RCC->CR, RCC_CR_HSIKERON);
}

/**
  * @brief  Disable HSI in stop mode
  * @rmtoll CR           HSIKERON      LL_RCC_HSI_DisableInStopMode
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI_DisableInStopMode(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_HSIKERON);
}

/**
  * @brief  Check if HSI is enabled in stop mode
  * @rmtoll CR           HSIKERON        LL_RCC_HSI_IsEnabledInStopMode
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_HSI_IsEnabledInStopMode(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_HSIKERON) == RCC_CR_HSIKERON) ? 1UL : 0UL);
}

/**
  * @brief  Enable HSI oscillator
  * @rmtoll CR           HSION         LL_RCC_HSI_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI_Enable(void)
{
  SET_BIT(RCC->CR, RCC_CR_HSION);
}

/**
  * @brief  Disable HSI oscillator
  * @rmtoll CR           HSION         LL_RCC_HSI_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI_Disable(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_HSION);
}

/**
  * @brief  Check if HSI clock is ready
  * @rmtoll CR           HSIRDY        LL_RCC_HSI_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_HSI_IsReady(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_HSIRDY) == RCC_CR_HSIRDY) ? 1UL : 0UL);
}

/**
  * @brief  Enable HSI Automatic from stop mode
  * @rmtoll CR           HSIASFS       LL_RCC_HSI_EnableAutoFromStop
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI_EnableAutoFromStop(void)
{
  SET_BIT(RCC->CR, RCC_CR_HSIASFS);
}

/**
  * @brief  Disable HSI Automatic from stop mode
  * @rmtoll CR           HSIASFS       LL_RCC_HSI_DisableAutoFromStop
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI_DisableAutoFromStop(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_HSIASFS);
}
/**
  * @brief  Get HSI Calibration value
  * @note When HSITRIM is written, HSICAL is updated with the sum of
  *       HSITRIM and the factory trim value
  * @rmtoll ICSCR        HSICAL        LL_RCC_HSI_GetCalibration
  * @retval Between Min_Data = 0x00 and Max_Data = 0xFF
  */
__STATIC_INLINE uint32_t LL_RCC_HSI_GetCalibration(void)
{
  return (uint32_t)(READ_BIT(RCC->ICSCR, RCC_ICSCR_HSICAL) >> RCC_ICSCR_HSICAL_Pos);
}

/**
  * @brief  Set HSI Calibration trimming
  * @note user-programmable trimming value that is added to the HSICAL
  * @note Default value is 16, which, when added to the HSICAL value,
  *       should trim the HSI to 16 MHz +/- 1 %
  * @rmtoll ICSCR        HSITRIM       LL_RCC_HSI_SetCalibTrimming
  * @param  Value Between Min_Data = 0 and Max_Data = 31
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI_SetCalibTrimming(uint32_t Value)
{
  MODIFY_REG(RCC->ICSCR, RCC_ICSCR_HSITRIM, Value << RCC_ICSCR_HSITRIM_Pos);
}

/**
  * @brief  Get HSI Calibration trimming
  * @rmtoll ICSCR        HSITRIM       LL_RCC_HSI_GetCalibTrimming
  * @retval Between Min_Data = 0 and Max_Data = 31
  */
__STATIC_INLINE uint32_t LL_RCC_HSI_GetCalibTrimming(void)
{
  return (uint32_t)(READ_BIT(RCC->ICSCR, RCC_ICSCR_HSITRIM) >> RCC_ICSCR_HSITRIM_Pos);
}

/**
  * @}
  */

#if defined(RCC_HSI48_SUPPORT)
/** @defgroup RCC_LL_EF_HSI48 HSI48
  * @{
  */

/**
  * @brief  Enable HSI48
  * @rmtoll CRRCR          HSI48ON       LL_RCC_HSI48_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI48_Enable(void)
{
  SET_BIT(RCC->CRRCR, RCC_CRRCR_HSI48ON);
}

/**
  * @brief  Disable HSI48
  * @rmtoll CRRCR          HSI48ON       LL_RCC_HSI48_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_HSI48_Disable(void)
{
  CLEAR_BIT(RCC->CRRCR, RCC_CRRCR_HSI48ON);
}

/**
  * @brief  Check if HSI48 oscillator Ready
  * @rmtoll CRRCR          HSI48RDY      LL_RCC_HSI48_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_HSI48_IsReady(void)
{
  return ((READ_BIT(RCC->CRRCR, RCC_CRRCR_HSI48RDY) == RCC_CRRCR_HSI48RDY) ? 1UL : 0UL);
}

/**
  * @brief  Get HSI48 Calibration value
  * @rmtoll CRRCR          HSI48CAL      LL_RCC_HSI48_GetCalibration
  * @retval Between Min_Data = 0x00 and Max_Data = 0x1FF
  */
__STATIC_INLINE uint32_t LL_RCC_HSI48_GetCalibration(void)
{
  return (uint32_t)(READ_BIT(RCC->CRRCR, RCC_CRRCR_HSI48CAL) >> RCC_CRRCR_HSI48CAL_Pos);
}

/**
  * @}
  */
#endif /* RCC_HSI48_SUPPORT */

/** @defgroup RCC_LL_EF_LSE LSE
  * @{
  */

/**
  * @brief  Enable  Low Speed External (LSE) crystal.
  * @rmtoll BDCR         LSEON         LL_RCC_LSE_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_Enable(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
}

/**
  * @brief  Disable  Low Speed External (LSE) crystal.
  * @rmtoll BDCR         LSEON         LL_RCC_LSE_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_Disable(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEON);
}

/**
  * @brief  Enable external clock source (LSE bypass).
  * @rmtoll BDCR         LSEBYP        LL_RCC_LSE_EnableBypass
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_EnableBypass(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_LSEBYP);
}

/**
  * @brief  Disable external clock source (LSE bypass).
  * @rmtoll BDCR         LSEBYP        LL_RCC_LSE_DisableBypass
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_DisableBypass(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEBYP);
}

/**
  * @brief  Set LSE oscillator drive capability
  * @note The oscillator is in Xtal mode when it is not in bypass mode.
  * @rmtoll BDCR         LSEDRV        LL_RCC_LSE_SetDriveCapability
  * @param  LSEDrive This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LSEDRIVE_LOW
  *         @arg @ref LL_RCC_LSEDRIVE_MEDIUMLOW
  *         @arg @ref LL_RCC_LSEDRIVE_MEDIUMHIGH
  *         @arg @ref LL_RCC_LSEDRIVE_HIGH
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_SetDriveCapability(uint32_t LSEDrive)
{
  MODIFY_REG(RCC->BDCR, RCC_BDCR_LSEDRV, LSEDrive);
}

/**
  * @brief  Get LSE oscillator drive capability
  * @rmtoll BDCR         LSEDRV        LL_RCC_LSE_GetDriveCapability
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_LSEDRIVE_LOW
  *         @arg @ref LL_RCC_LSEDRIVE_MEDIUMLOW
  *         @arg @ref LL_RCC_LSEDRIVE_MEDIUMHIGH
  *         @arg @ref LL_RCC_LSEDRIVE_HIGH
  */
__STATIC_INLINE uint32_t LL_RCC_LSE_GetDriveCapability(void)
{
  return (uint32_t)(READ_BIT(RCC->BDCR, RCC_BDCR_LSEDRV));
}

/**
  * @brief  Enable Clock security system on LSE.
  * @rmtoll BDCR         LSECSSON      LL_RCC_LSE_EnableCSS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_EnableCSS(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_LSECSSON);
}

/**
  * @brief  Disable Clock security system on LSE.
  * @note Clock security system can be disabled only after a LSE
  *       failure detection. In that case it MUST be disabled by software.
  * @rmtoll BDCR         LSECSSON      LL_RCC_LSE_DisableCSS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_DisableCSS(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSECSSON);
}

/**
  * @brief  Check if LSE oscillator Ready
  * @rmtoll BDCR         LSERDY        LL_RCC_LSE_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_LSE_IsReady(void)
{
  return ((READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) == RCC_BDCR_LSERDY) ? 1UL : 0UL);
}

/**
  * @brief  Check if CSS on LSE failure Detection
  * @rmtoll BDCR         LSECSSD       LL_RCC_LSE_IsCSSDetected
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_LSE_IsCSSDetected(void)
{
  return ((READ_BIT(RCC->BDCR, RCC_BDCR_LSECSSD) == RCC_BDCR_LSECSSD) ? 1UL : 0UL);
}

#if defined(RCC_BDCR_LSESYSDIS)
/**
  * @brief  Disable LSE oscillator propagation
  * @note LSE clock is not propagated to any peripheral except to RTC which remains clocked
  * @note A 2 LSE-clock delay is needed for LSESYSDIS setting to be taken into account
  * @rmtoll BDCR         LSESYSDIS     LL_RCC_LSE_DisablePropagation
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_DisablePropagation(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_LSESYSDIS);
}

/**
  * @brief  Enable LSE oscillator propagation
  * @note A 2 LSE-clock delay is needed for LSESYSDIS resetting to be taken into account
  * @rmtoll BDCR         LSESYSDIS     LL_RCC_LSE_EnablePropagation
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSE_EnablePropagation(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSESYSDIS);
}

/**
  * @brief  Check if LSE oscillator propagation is enabled
  * @rmtoll BDCR         LSESYSDIS     LL_RCC_LSE_IsPropagationEnabled
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_LSE_IsPropagationEnabled(void)
{
  return ((READ_BIT(RCC->BDCR, RCC_BDCR_LSESYSDIS) == 0U) ? 1UL : 0UL);
}
#endif /* RCC_BDCR_LSESYSDIS */
/**
  * @}
  */

/** @defgroup RCC_LL_EF_LSI LSI
  * @{
  */

/**
  * @brief  Enable LSI Oscillator
  * @rmtoll CSR          LSION         LL_RCC_LSI_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSI_Enable(void)
{
  SET_BIT(RCC->CSR, RCC_CSR_LSION);
}

/**
  * @brief  Disable LSI Oscillator
  * @rmtoll CSR          LSION         LL_RCC_LSI_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSI_Disable(void)
{
  CLEAR_BIT(RCC->CSR, RCC_CSR_LSION);
}

/**
  * @brief  Check if LSI is Ready
  * @rmtoll CSR          LSIRDY        LL_RCC_LSI_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_LSI_IsReady(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == RCC_CSR_LSIRDY) ? 1UL : 0UL);
}

#if defined(RCC_CSR_LSIPREDIV)
/**
  * @brief  Set LSI division factor
  * @rmtoll CSR          LSIPREDIV     LL_RCC_LSI_SetPrediv
  * @param  LSI_PREDIV This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LSI_PREDIV_1
  *         @arg @ref LL_RCC_LSI_PREDIV_128
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSI_SetPrediv(uint32_t LSI_PREDIV)
{
  MODIFY_REG(RCC->CSR, RCC_CSR_LSIPREDIV, LSI_PREDIV);
}

/**
  * @brief  Get LSI division factor
  * @rmtoll CSR          LSIPREDIV     LL_RCC_LSI_GetPrediv
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_LSI_PREDIV_1
  *         @arg @ref LL_RCC_LSI_PREDIV_128
  */
__STATIC_INLINE uint32_t LL_RCC_LSI_GetPrediv(void)
{
  return (READ_BIT(RCC->CSR, RCC_CSR_LSIPREDIV));
}
#endif /* RCC_CSR_LSIPREDIV */

/**
  * @}
  */

/** @defgroup RCC_LL_EF_MSI MSI
  * @{
  */

/**
  * @brief  Enable MSI oscillator
  * @rmtoll CR           MSION         LL_RCC_MSI_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_Enable(void)
{
  SET_BIT(RCC->CR, RCC_CR_MSION);
}

/**
  * @brief  Disable MSI oscillator
  * @rmtoll CR           MSION         LL_RCC_MSI_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_Disable(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_MSION);
}

/**
  * @brief  Check if MSI oscillator Ready
  * @rmtoll CR           MSIRDY        LL_RCC_MSI_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_MSI_IsReady(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_MSIRDY) == RCC_CR_MSIRDY) ? 1UL : 0UL);
}

/**
  * @brief  Enable MSI PLL-mode (Hardware auto calibration with LSE)
  * @note MSIPLLEN must be enabled after LSE is enabled (LSEON enabled)
  *       and ready (LSERDY set by hardware)
  * @note hardware protection to avoid enabling MSIPLLEN if LSE is not
  *       ready
  * @rmtoll CR           MSIPLLEN      LL_RCC_MSI_EnablePLLMode
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_EnablePLLMode(void)
{
  SET_BIT(RCC->CR, RCC_CR_MSIPLLEN);
}

/**
  * @brief  Disable MSI-PLL mode
  * @note cleared by hardware when LSE is disabled (LSEON = 0) or when
  *       the Clock Security System on LSE detects a LSE failure
  * @rmtoll CR           MSIPLLEN      LL_RCC_MSI_DisablePLLMode
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_DisablePLLMode(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_MSIPLLEN);
}

/**
  * @brief  Enable MSI clock range selection with MSIRANGE register
  * @note Write 0 has no effect. After a standby or a reset
  *       MSIRGSEL is at 0 and the MSI range value is provided by
  *       MSISRANGE
  * @rmtoll CR           MSIRGSEL      LL_RCC_MSI_EnableRangeSelection
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_EnableRangeSelection(void)
{
  SET_BIT(RCC->CR, RCC_CR_MSIRGSEL);
}

/**
  * @brief  Check if MSI clock range is selected with MSIRANGE register
  * @rmtoll CR           MSIRGSEL      LL_RCC_MSI_IsEnabledRangeSelect
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_MSI_IsEnabledRangeSelect(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_MSIRGSEL) == RCC_CR_MSIRGSEL) ? 1UL : 0UL);
}

/**
  * @brief  Configure the Internal Multi Speed oscillator (MSI) clock range in run mode.
  * @rmtoll CR           MSIRANGE      LL_RCC_MSI_SetRange
  * @param  Range This parameter can be one of the following values:
  *         @arg @ref LL_RCC_MSIRANGE_0
  *         @arg @ref LL_RCC_MSIRANGE_1
  *         @arg @ref LL_RCC_MSIRANGE_2
  *         @arg @ref LL_RCC_MSIRANGE_3
  *         @arg @ref LL_RCC_MSIRANGE_4
  *         @arg @ref LL_RCC_MSIRANGE_5
  *         @arg @ref LL_RCC_MSIRANGE_6
  *         @arg @ref LL_RCC_MSIRANGE_7
  *         @arg @ref LL_RCC_MSIRANGE_8
  *         @arg @ref LL_RCC_MSIRANGE_9
  *         @arg @ref LL_RCC_MSIRANGE_10
  *         @arg @ref LL_RCC_MSIRANGE_11
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_SetRange(uint32_t Range)
{
  MODIFY_REG(RCC->CR, RCC_CR_MSIRANGE, Range);
}

/**
  * @brief  Get the Internal Multi Speed oscillator (MSI) clock range in run mode.
  * @rmtoll CR           MSIRANGE      LL_RCC_MSI_GetRange
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_MSIRANGE_0
  *         @arg @ref LL_RCC_MSIRANGE_1
  *         @arg @ref LL_RCC_MSIRANGE_2
  *         @arg @ref LL_RCC_MSIRANGE_3
  *         @arg @ref LL_RCC_MSIRANGE_4
  *         @arg @ref LL_RCC_MSIRANGE_5
  *         @arg @ref LL_RCC_MSIRANGE_6
  *         @arg @ref LL_RCC_MSIRANGE_7
  *         @arg @ref LL_RCC_MSIRANGE_8
  *         @arg @ref LL_RCC_MSIRANGE_9
  *         @arg @ref LL_RCC_MSIRANGE_10
  *         @arg @ref LL_RCC_MSIRANGE_11
  */
__STATIC_INLINE uint32_t LL_RCC_MSI_GetRange(void)
{
  return (uint32_t)(READ_BIT(RCC->CR, RCC_CR_MSIRANGE));
}

/**
  * @brief  Configure MSI range used after standby
  * @rmtoll CSR          MSISRANGE     LL_RCC_MSI_SetRangeAfterStandby
  * @param  Range This parameter can be one of the following values:
  *         @arg @ref LL_RCC_MSISRANGE_4
  *         @arg @ref LL_RCC_MSISRANGE_5
  *         @arg @ref LL_RCC_MSISRANGE_6
  *         @arg @ref LL_RCC_MSISRANGE_7
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_SetRangeAfterStandby(uint32_t Range)
{
  MODIFY_REG(RCC->CSR, RCC_CSR_MSISRANGE, Range);
}

/**
  * @brief  Get MSI range used after standby
  * @rmtoll CSR          MSISRANGE     LL_RCC_MSI_GetRangeAfterStandby
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_MSISRANGE_4
  *         @arg @ref LL_RCC_MSISRANGE_5
  *         @arg @ref LL_RCC_MSISRANGE_6
  *         @arg @ref LL_RCC_MSISRANGE_7
  */
__STATIC_INLINE uint32_t LL_RCC_MSI_GetRangeAfterStandby(void)
{
  return (uint32_t)(READ_BIT(RCC->CSR, RCC_CSR_MSISRANGE));
}

/**
  * @brief  Get MSI Calibration value
  * @note When MSITRIM is written, MSICAL is updated with the sum of
  *       MSITRIM and the factory trim value
  * @rmtoll ICSCR        MSICAL        LL_RCC_MSI_GetCalibration
  * @retval Between Min_Data = 0 and Max_Data = 255
  */
__STATIC_INLINE uint32_t LL_RCC_MSI_GetCalibration(void)
{
  return (uint32_t)(READ_BIT(RCC->ICSCR, RCC_ICSCR_MSICAL) >> RCC_ICSCR_MSICAL_Pos);
}

/**
  * @brief  Set MSI Calibration trimming
  * @note user-programmable trimming value that is added to the MSICAL
  * @rmtoll ICSCR        MSITRIM       LL_RCC_MSI_SetCalibTrimming
  * @param  Value Between Min_Data = 0 and Max_Data = 255
  * @retval None
  */
__STATIC_INLINE void LL_RCC_MSI_SetCalibTrimming(uint32_t Value)
{
  MODIFY_REG(RCC->ICSCR, RCC_ICSCR_MSITRIM, Value << RCC_ICSCR_MSITRIM_Pos);
}

/**
  * @brief  Get MSI Calibration trimming
  * @rmtoll ICSCR        MSITRIM       LL_RCC_MSI_GetCalibTrimming
  * @retval Between 0 and 255
  */
__STATIC_INLINE uint32_t LL_RCC_MSI_GetCalibTrimming(void)
{
  return (uint32_t)(READ_BIT(RCC->ICSCR, RCC_ICSCR_MSITRIM) >> RCC_ICSCR_MSITRIM_Pos);
}

/**
  * @}
  */

/** @defgroup RCC_LL_EF_LSCO LSCO
  * @{
  */

/**
  * @brief  Enable Low speed clock
  * @rmtoll BDCR         LSCOEN        LL_RCC_LSCO_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSCO_Enable(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_LSCOEN);
}

/**
  * @brief  Disable Low speed clock
  * @rmtoll BDCR         LSCOEN        LL_RCC_LSCO_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSCO_Disable(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSCOEN);
}

/**
  * @brief  Configure Low speed clock selection
  * @rmtoll BDCR         LSCOSEL       LL_RCC_LSCO_SetSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LSCO_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_LSCO_CLKSOURCE_LSE
  * @retval None
  */
__STATIC_INLINE void LL_RCC_LSCO_SetSource(uint32_t Source)
{
  MODIFY_REG(RCC->BDCR, RCC_BDCR_LSCOSEL, Source);
}

/**
  * @brief  Get Low speed clock selection
  * @rmtoll BDCR         LSCOSEL       LL_RCC_LSCO_GetSource
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_LSCO_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_LSCO_CLKSOURCE_LSE
  */
__STATIC_INLINE uint32_t LL_RCC_LSCO_GetSource(void)
{
  return (uint32_t)(READ_BIT(RCC->BDCR, RCC_BDCR_LSCOSEL));
}

/**
  * @}
  */

/** @defgroup RCC_LL_EF_System System
  * @{
  */

/**
  * @brief  Configure the system clock source
  * @rmtoll CFGR         SW            LL_RCC_SetSysClkSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_MSI
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_HSE
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_PLL
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetSysClkSource(uint32_t Source)
{
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, Source);
}

/**
  * @brief  Get the system clock source
  * @rmtoll CFGR         SWS           LL_RCC_GetSysClkSource
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_STATUS_MSI
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_STATUS_HSI
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_STATUS_HSE
  *         @arg @ref LL_RCC_SYS_CLKSOURCE_STATUS_PLL
  */
__STATIC_INLINE uint32_t LL_RCC_GetSysClkSource(void)
{
  return (uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_SWS));
}

/**
  * @brief  Set AHB prescaler
  * @rmtoll CFGR         HPRE          LL_RCC_SetAHBPrescaler
  * @param  Prescaler This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SYSCLK_DIV_1
  *         @arg @ref LL_RCC_SYSCLK_DIV_2
  *         @arg @ref LL_RCC_SYSCLK_DIV_4
  *         @arg @ref LL_RCC_SYSCLK_DIV_8
  *         @arg @ref LL_RCC_SYSCLK_DIV_16
  *         @arg @ref LL_RCC_SYSCLK_DIV_64
  *         @arg @ref LL_RCC_SYSCLK_DIV_128
  *         @arg @ref LL_RCC_SYSCLK_DIV_256
  *         @arg @ref LL_RCC_SYSCLK_DIV_512
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetAHBPrescaler(uint32_t Prescaler)
{
  MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, Prescaler);
}

/**
  * @brief  Set APB1 prescaler
  * @rmtoll CFGR         PPRE1         LL_RCC_SetAPB1Prescaler
  * @param  Prescaler This parameter can be one of the following values:
  *         @arg @ref LL_RCC_APB1_DIV_1
  *         @arg @ref LL_RCC_APB1_DIV_2
  *         @arg @ref LL_RCC_APB1_DIV_4
  *         @arg @ref LL_RCC_APB1_DIV_8
  *         @arg @ref LL_RCC_APB1_DIV_16
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetAPB1Prescaler(uint32_t Prescaler)
{
  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, Prescaler);
}

/**
  * @brief  Set APB2 prescaler
  * @rmtoll CFGR         PPRE2         LL_RCC_SetAPB2Prescaler
  * @param  Prescaler This parameter can be one of the following values:
  *         @arg @ref LL_RCC_APB2_DIV_1
  *         @arg @ref LL_RCC_APB2_DIV_2
  *         @arg @ref LL_RCC_APB2_DIV_4
  *         @arg @ref LL_RCC_APB2_DIV_8
  *         @arg @ref LL_RCC_APB2_DIV_16
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetAPB2Prescaler(uint32_t Prescaler)
{
  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, Prescaler);
}

/**
  * @brief  Get AHB prescaler
  * @rmtoll CFGR         HPRE          LL_RCC_GetAHBPrescaler
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_SYSCLK_DIV_1
  *         @arg @ref LL_RCC_SYSCLK_DIV_2
  *         @arg @ref LL_RCC_SYSCLK_DIV_4
  *         @arg @ref LL_RCC_SYSCLK_DIV_8
  *         @arg @ref LL_RCC_SYSCLK_DIV_16
  *         @arg @ref LL_RCC_SYSCLK_DIV_64
  *         @arg @ref LL_RCC_SYSCLK_DIV_128
  *         @arg @ref LL_RCC_SYSCLK_DIV_256
  *         @arg @ref LL_RCC_SYSCLK_DIV_512
  */
__STATIC_INLINE uint32_t LL_RCC_GetAHBPrescaler(void)
{
  return (uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_HPRE));
}

/**
  * @brief  Get APB1 prescaler
  * @rmtoll CFGR         PPRE1         LL_RCC_GetAPB1Prescaler
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_APB1_DIV_1
  *         @arg @ref LL_RCC_APB1_DIV_2
  *         @arg @ref LL_RCC_APB1_DIV_4
  *         @arg @ref LL_RCC_APB1_DIV_8
  *         @arg @ref LL_RCC_APB1_DIV_16
  */
__STATIC_INLINE uint32_t LL_RCC_GetAPB1Prescaler(void)
{
  return (uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_PPRE1));
}

/**
  * @brief  Get APB2 prescaler
  * @rmtoll CFGR         PPRE2         LL_RCC_GetAPB2Prescaler
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_APB2_DIV_1
  *         @arg @ref LL_RCC_APB2_DIV_2
  *         @arg @ref LL_RCC_APB2_DIV_4
  *         @arg @ref LL_RCC_APB2_DIV_8
  *         @arg @ref LL_RCC_APB2_DIV_16
  */
__STATIC_INLINE uint32_t LL_RCC_GetAPB2Prescaler(void)
{
  return (uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_PPRE2));
}

/**
  * @brief  Set Clock After Wake-Up From Stop mode
  * @rmtoll CFGR         STOPWUCK      LL_RCC_SetClkAfterWakeFromStop
  * @param  Clock This parameter can be one of the following values:
  *         @arg @ref LL_RCC_STOP_WAKEUPCLOCK_MSI
  *         @arg @ref LL_RCC_STOP_WAKEUPCLOCK_HSI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetClkAfterWakeFromStop(uint32_t Clock)
{
  MODIFY_REG(RCC->CFGR, RCC_CFGR_STOPWUCK, Clock);
}

/**
  * @brief  Get Clock After Wake-Up From Stop mode
  * @rmtoll CFGR         STOPWUCK      LL_RCC_GetClkAfterWakeFromStop
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_STOP_WAKEUPCLOCK_MSI
  *         @arg @ref LL_RCC_STOP_WAKEUPCLOCK_HSI
  */
__STATIC_INLINE uint32_t LL_RCC_GetClkAfterWakeFromStop(void)
{
  return (uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_STOPWUCK));
}

/**
  * @}
  */

/** @defgroup RCC_LL_EF_MCO MCO
  * @{
  */

/**
  * @brief  Configure MCOx
  * @rmtoll CFGR         MCOSEL        LL_RCC_ConfigMCO\n
  *         CFGR         MCOPRE        LL_RCC_ConfigMCO
  * @param  MCOxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_MCO1SOURCE_NOCLOCK
  *         @arg @ref LL_RCC_MCO1SOURCE_SYSCLK
  *         @arg @ref LL_RCC_MCO1SOURCE_MSI
  *         @arg @ref LL_RCC_MCO1SOURCE_HSI
  *         @arg @ref LL_RCC_MCO1SOURCE_HSE
  *         @arg @ref LL_RCC_MCO1SOURCE_HSI48 (*)
  *         @arg @ref LL_RCC_MCO1SOURCE_PLLCLK
  *         @arg @ref LL_RCC_MCO1SOURCE_LSI
  *         @arg @ref LL_RCC_MCO1SOURCE_LSE
  *
  *         (*) value not defined in all devices.
  * @param  MCOxPrescaler This parameter can be one of the following values:
  *         @arg @ref LL_RCC_MCO1_DIV_1
  *         @arg @ref LL_RCC_MCO1_DIV_2
  *         @arg @ref LL_RCC_MCO1_DIV_4
  *         @arg @ref LL_RCC_MCO1_DIV_8
  *         @arg @ref LL_RCC_MCO1_DIV_16
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ConfigMCO(uint32_t MCOxSource, uint32_t MCOxPrescaler)
{
  MODIFY_REG(RCC->CFGR, RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE, MCOxSource | MCOxPrescaler);
}

/**
  * @}
  */

/** @defgroup RCC_LL_EF_Peripheral_Clock_Source Peripheral Clock Source
  * @{
  */

/**
  * @brief  Configure USARTx clock source
  * @rmtoll CCIPR        USARTxSEL     LL_RCC_SetUSARTClockSource
  * @param  USARTxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_PCLK2
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_PCLK1 (*)
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_SYSCLK (*)
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_HSI (*)
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_LSE (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetUSARTClockSource(uint32_t USARTxSource)
{
  MODIFY_REG(RCC->CCIPR, (USARTxSource >> 16U), (USARTxSource & 0x0000FFFFU));
}

#if defined(UART4) || defined(UART5)
/**
  * @brief  Configure UARTx clock source
  * @rmtoll CCIPR        UARTxSEL      LL_RCC_SetUARTClockSource
  * @param  UARTxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_LSE
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetUARTClockSource(uint32_t UARTxSource)
{
  MODIFY_REG(RCC->CCIPR, (UARTxSource >> 16U), (UARTxSource & 0x0000FFFFU));
}
#endif /* UART4 || UART5 */

/**
  * @brief  Configure LPUART1x clock source
  * @rmtoll CCIPR        LPUART1SEL    LL_RCC_SetLPUARTClockSource
  * @param  LPUARTxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_LSE
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetLPUARTClockSource(uint32_t LPUARTxSource)
{
  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_LPUART1SEL, LPUARTxSource);
}

/**
  * @brief  Configure I2Cx clock source
  * @rmtoll CCIPR        I2CxSEL       LL_RCC_SetI2CClockSource
  * @param  I2CxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_I2C1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_I2C1_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_I2C1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_I2C2_CLKSOURCE_PCLK1 (*)
  *         @arg @ref LL_RCC_I2C2_CLKSOURCE_SYSCLK (*)
  *         @arg @ref LL_RCC_I2C2_CLKSOURCE_HSI (*)
  *         @arg @ref LL_RCC_I2C3_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_I2C3_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_I2C3_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_I2C4_CLKSOURCE_PCLK1 (*)
  *         @arg @ref LL_RCC_I2C4_CLKSOURCE_SYSCLK (*)
  *         @arg @ref LL_RCC_I2C4_CLKSOURCE_HSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetI2CClockSource(uint32_t I2CxSource)
{
  __IO uint32_t *reg = (__IO uint32_t *)(uint32_t)(RCC_BASE + 0x88U + (I2CxSource >> 24U));
  MODIFY_REG(*reg, 3UL << ((I2CxSource & 0x001F0000U) >> 16U), ((I2CxSource & 0x000000FFU) << ((I2CxSource & 0x001F0000U) >> 16U)));
}

/**
  * @brief  Configure LPTIMx clock source
  * @rmtoll CCIPR        LPTIMxSEL     LL_RCC_SetLPTIMClockSource
  * @param  LPTIMxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_LSE
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetLPTIMClockSource(uint32_t LPTIMxSource)
{
  MODIFY_REG(RCC->CCIPR, (LPTIMxSource & 0xFFFF0000U), (LPTIMxSource << 16U));
}

#if defined(RCC_CCIPR_SAI1SEL) || defined(RCC_CCIPR2_SAI1SEL)
/**
  * @brief  Configure SAIx clock source
  @if STM32L4S9xx
  * @rmtoll CCIPR2       SAIxSEL       LL_RCC_SetSAIClockSource
  @else
  * @rmtoll CCIPR        SAIxSEL       LL_RCC_SetSAIClockSource
  @endif
  * @param  SAIxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PLLSAI1
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PLLSAI2 (*)
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PIN
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PLLSAI2 (*)
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PLL (*)
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PIN (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetSAIClockSource(uint32_t SAIxSource)
{
#if defined(RCC_CCIPR2_SAI1SEL)
  MODIFY_REG(RCC->CCIPR2, (SAIxSource >> 16U), (SAIxSource & 0x0000FFFFU));
#else
  MODIFY_REG(RCC->CCIPR, (SAIxSource & 0xFFFF0000U), (SAIxSource << 16U));
#endif /* RCC_CCIPR2_SAI1SEL */
}
#endif /* RCC_CCIPR_SAI1SEL || RCC_CCIPR2_SAI1SEL */

#if defined(RCC_CCIPR2_SDMMCSEL)
/**
  * @brief  Configure SDMMC1 kernel clock source
  * @rmtoll CCIPR2       SDMMCSEL      LL_RCC_SetSDMMCKernelClockSource
  * @param  SDMMCxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SDMMC1_KERNELCLKSOURCE_48CLK (*)
  *         @arg @ref LL_RCC_SDMMC1_KERNELCLKSOURCE_PLLP (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetSDMMCKernelClockSource(uint32_t SDMMCxSource)
{
  MODIFY_REG(RCC->CCIPR2, RCC_CCIPR2_SDMMCSEL, SDMMCxSource);
}
#endif /* RCC_CCIPR2_SDMMCSEL */

/**
  * @brief  Configure SDMMC1 clock source
  * @rmtoll CCIPR        CLK48SEL      LL_RCC_SetSDMMCClockSource
  * @param  SDMMCxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_NONE (*)
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_HSI48 (*)
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_MSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetSDMMCClockSource(uint32_t SDMMCxSource)
{
  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_CLK48SEL, SDMMCxSource);
}

/**
  * @brief  Configure RNG clock source
  * @rmtoll CCIPR        CLK48SEL      LL_RCC_SetRNGClockSource
  * @param  RNGxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_NONE (*)
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_HSI48 (*)
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_MSI
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetRNGClockSource(uint32_t RNGxSource)
{
  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_CLK48SEL, RNGxSource);
}

#if defined(USB_OTG_FS) || defined(USB)
/**
  * @brief  Configure USB clock source
  * @rmtoll CCIPR        CLK48SEL      LL_RCC_SetUSBClockSource
  * @param  USBxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_USB_CLKSOURCE_NONE (*)
  *         @arg @ref LL_RCC_USB_CLKSOURCE_HSI48 (*)
  *         @arg @ref LL_RCC_USB_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_USB_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_USB_CLKSOURCE_MSI
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetUSBClockSource(uint32_t USBxSource)
{
  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_CLK48SEL, USBxSource);
}
#endif /* USB_OTG_FS || USB */

#if defined(RCC_CCIPR_ADCSEL)
/**
  * @brief  Configure ADC clock source
  * @rmtoll CCIPR        ADCSEL        LL_RCC_SetADCClockSource
  * @param  ADCxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_NONE
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_PLLSAI2 (*)
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_SYSCLK
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetADCClockSource(uint32_t ADCxSource)
{
  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_ADCSEL, ADCxSource);
}
#endif /* RCC_CCIPR_ADCSEL */

#if defined(SWPMI1)
/**
  * @brief  Configure SWPMI clock source
  * @rmtoll CCIPR        SWPMI1SEL     LL_RCC_SetSWPMIClockSource
  * @param  SWPMIxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SWPMI1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_SWPMI1_CLKSOURCE_HSI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetSWPMIClockSource(uint32_t SWPMIxSource)
{
  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_SWPMI1SEL, SWPMIxSource);
}
#endif /* SWPMI1 */

#if defined(DFSDM1_Channel0)
#if defined(RCC_CCIPR2_ADFSDM1SEL)
/**
  * @brief  Configure DFSDM Audio clock source
  * @rmtoll CCIPR2        ADFSDM1SEL        LL_RCC_SetDFSDMAudioClockSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_DFSDM1_AUDIO_CLKSOURCE_SAI1
  *         @arg @ref LL_RCC_DFSDM1_AUDIO_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_DFSDM1_AUDIO_CLKSOURCE_MSI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetDFSDMAudioClockSource(uint32_t Source)
{
  MODIFY_REG(RCC->CCIPR2, RCC_CCIPR2_ADFSDM1SEL, Source);
}
#endif /* RCC_CCIPR2_ADFSDM1SEL */

/**
  * @brief  Configure DFSDM Kernel clock source
  @if STM32L4S9xx
  * @rmtoll CCIPR2       DFSDM1SEL     LL_RCC_SetDFSDMClockSource
  @else
  * @rmtoll CCIPR        DFSDM1SEL     LL_RCC_SetDFSDMClockSource
  @endif
  * @param  DFSDMxSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_DFSDM1_CLKSOURCE_PCLK2
  *         @arg @ref LL_RCC_DFSDM1_CLKSOURCE_SYSCLK
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetDFSDMClockSource(uint32_t DFSDMxSource)
{
#if defined(RCC_CCIPR2_DFSDM1SEL)
  MODIFY_REG(RCC->CCIPR2, RCC_CCIPR2_DFSDM1SEL, DFSDMxSource);
#else
  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_DFSDM1SEL, DFSDMxSource);
#endif /* RCC_CCIPR2_DFSDM1SEL */
}
#endif /* DFSDM1_Channel0 */

#if defined(DSI)
/**
  * @brief  Configure DSI clock source
  * @rmtoll CCIPR2         DSISEL        LL_RCC_SetDSIClockSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_DSI_CLKSOURCE_PHY
  *         @arg @ref LL_RCC_DSI_CLKSOURCE_PLL
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetDSIClockSource(uint32_t Source)
{
  MODIFY_REG(RCC->CCIPR2, RCC_CCIPR2_DSISEL, Source);
}
#endif /* DSI */

#if defined(LTDC)
/**
  * @brief  Configure LTDC Clock Source
  * @rmtoll CCIPR2         PLLSAI2DIVR        LL_RCC_SetLTDCClockSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV2
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV4
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV8
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV16
  * @retval None
 */
__STATIC_INLINE void LL_RCC_SetLTDCClockSource(uint32_t Source)
{
  MODIFY_REG(RCC->CCIPR2, RCC_CCIPR2_PLLSAI2DIVR, Source);
}
#endif /* LTDC */

#if defined(OCTOSPI1)
/**
  * @brief  Configure OCTOSPI clock source
  * @rmtoll CCIPR2         OSPISEL        LL_RCC_SetOCTOSPIClockSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_OCTOSPI_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_OCTOSPI_CLKSOURCE_MSI
  *         @arg @ref LL_RCC_OCTOSPI_CLKSOURCE_PLL
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetOCTOSPIClockSource(uint32_t Source)
{
  MODIFY_REG(RCC->CCIPR2, RCC_CCIPR2_OSPISEL, Source);
}
#endif /* OCTOSPI1 */

/**
  * @brief  Get USARTx clock source
  * @rmtoll CCIPR        USARTxSEL     LL_RCC_GetUSARTClockSource
  * @param  USARTx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_USART1_CLKSOURCE
  *         @arg @ref LL_RCC_USART2_CLKSOURCE
  *         @arg @ref LL_RCC_USART3_CLKSOURCE (*)
  *
  *         (*) value not defined in all devices.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_PCLK2
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_USART1_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_USART2_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_PCLK1 (*)
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_SYSCLK (*)
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_HSI (*)
  *         @arg @ref LL_RCC_USART3_CLKSOURCE_LSE (*)
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_GetUSARTClockSource(uint32_t USARTx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR, USARTx) | (USARTx << 16U));
}

#if defined(UART4) || defined(UART5)
/**
  * @brief  Get UARTx clock source
  * @rmtoll CCIPR        UARTxSEL      LL_RCC_GetUARTClockSource
  * @param  UARTx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_UART4_CLKSOURCE
  *         @arg @ref LL_RCC_UART5_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_UART4_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_UART5_CLKSOURCE_LSE
  */
__STATIC_INLINE uint32_t LL_RCC_GetUARTClockSource(uint32_t UARTx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR, UARTx) | (UARTx << 16U));
}
#endif /* UART4 || UART5 */

/**
  * @brief  Get LPUARTx clock source
  * @rmtoll CCIPR        LPUART1SEL    LL_RCC_GetLPUARTClockSource
  * @param  LPUARTx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_LPUART1_CLKSOURCE_LSE
  */
__STATIC_INLINE uint32_t LL_RCC_GetLPUARTClockSource(uint32_t LPUARTx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR, LPUARTx));
}

/**
  * @brief  Get I2Cx clock source
  * @rmtoll CCIPR        I2CxSEL       LL_RCC_GetI2CClockSource
  * @param  I2Cx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_I2C1_CLKSOURCE
  *         @arg @ref LL_RCC_I2C2_CLKSOURCE (*)
  *         @arg @ref LL_RCC_I2C3_CLKSOURCE
  *         @arg @ref LL_RCC_I2C4_CLKSOURCE (*)
  *
  *         (*) value not defined in all devices.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_I2C1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_I2C1_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_I2C1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_I2C2_CLKSOURCE_PCLK1 (*)
  *         @arg @ref LL_RCC_I2C2_CLKSOURCE_SYSCLK (*)
  *         @arg @ref LL_RCC_I2C2_CLKSOURCE_HSI (*)
  *         @arg @ref LL_RCC_I2C3_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_I2C3_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_I2C3_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_I2C4_CLKSOURCE_PCLK1 (*)
  *         @arg @ref LL_RCC_I2C4_CLKSOURCE_SYSCLK (*)
  *         @arg @ref LL_RCC_I2C4_CLKSOURCE_HSI (*)
  *
  *         (*) value not defined in all devices.
 */
__STATIC_INLINE uint32_t LL_RCC_GetI2CClockSource(uint32_t I2Cx)
{
  __IO const uint32_t *reg = (__IO uint32_t *)(uint32_t)(RCC_BASE + 0x88U + (I2Cx >> 24U));
  return (uint32_t)((READ_BIT(*reg, 3UL << ((I2Cx & 0x001F0000U) >> 16U)) >> ((I2Cx & 0x001F0000U) >> 16U)) | (I2Cx & 0xFFFF0000U));
}

/**
  * @brief  Get LPTIMx clock source
  * @rmtoll CCIPR        LPTIMxSEL     LL_RCC_GetLPTIMClockSource
  * @param  LPTIMx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_LPTIM1_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_LPTIM2_CLKSOURCE_LSE
  */
__STATIC_INLINE uint32_t LL_RCC_GetLPTIMClockSource(uint32_t LPTIMx)
{
  return (uint32_t)((READ_BIT(RCC->CCIPR, LPTIMx) >> 16U) | LPTIMx);
}

#if defined(RCC_CCIPR_SAI1SEL) || defined(RCC_CCIPR2_SAI1SEL)
/**
  * @brief  Get SAIx clock source
  @if STM32L4S9xx
  * @rmtoll CCIPR2       SAIxSEL       LL_RCC_GetSAIClockSource
  @else
  * @rmtoll CCIPR        SAIxSEL       LL_RCC_GetSAIClockSource
  @endif
  * @param  SAIx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE (*)
  *
  *         (*) value not defined in all devices.
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PLLSAI1
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PLLSAI2 (*)
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_SAI1_CLKSOURCE_PIN
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PLLSAI2 (*)
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PLL (*)
  *         @arg @ref LL_RCC_SAI2_CLKSOURCE_PIN (*)
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_GetSAIClockSource(uint32_t SAIx)
{
#if defined(RCC_CCIPR2_SAI1SEL)
  return (uint32_t)(READ_BIT(RCC->CCIPR2, SAIx) | (SAIx << 16U));
#else
  return (uint32_t)(READ_BIT(RCC->CCIPR, SAIx) >> 16U | SAIx);
#endif /* RCC_CCIPR2_SAI1SEL */
}
#endif /* RCC_CCIPR_SAI1SEL || RCC_CCIPR2_SAI1SEL */

#if defined(SDMMC1)
#if defined(RCC_CCIPR2_SDMMCSEL)
/**
  * @brief  Get SDMMCx kernel clock source
  * @rmtoll CCIPR2       SDMMCSEL      LL_RCC_GetSDMMCKernelClockSource
  * @param  SDMMCx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SDMMC1_KERNELCLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_SDMMC1_KERNELCLKSOURCE_48CLK (*)
  *         @arg @ref LL_RCC_SDMMC1_KERNELCLKSOURCE_PLL (*)
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_GetSDMMCKernelClockSource(uint32_t SDMMCx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR2, SDMMCx));
}
#endif /* RCC_CCIPR2_SDMMCSEL */

/**
  * @brief  Get SDMMCx clock source
  * @rmtoll CCIPR        CLK48SEL      LL_RCC_GetSDMMCClockSource
  * @param  SDMMCx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_NONE (*)
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_HSI48 (*)
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_SDMMC1_CLKSOURCE_MSI (*)
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_GetSDMMCClockSource(uint32_t SDMMCx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR, SDMMCx));
}
#endif /* SDMMC1 */

/**
  * @brief  Get RNGx clock source
  * @rmtoll CCIPR        CLK48SEL      LL_RCC_GetRNGClockSource
  * @param  RNGx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_RNG_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_NONE (*)
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_HSI48 (*)
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_RNG_CLKSOURCE_MSI
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_GetRNGClockSource(uint32_t RNGx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR, RNGx));
}

#if defined(USB_OTG_FS) || defined(USB)
/**
  * @brief  Get USBx clock source
  * @rmtoll CCIPR        CLK48SEL      LL_RCC_GetUSBClockSource
  * @param  USBx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_USB_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_USB_CLKSOURCE_NONE (*)
  *         @arg @ref LL_RCC_USB_CLKSOURCE_HSI48 (*)
  *         @arg @ref LL_RCC_USB_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_USB_CLKSOURCE_PLL
  *         @arg @ref LL_RCC_USB_CLKSOURCE_MSI
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_GetUSBClockSource(uint32_t USBx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR, USBx));
}
#endif /* USB_OTG_FS || USB */

/**
  * @brief  Get ADCx clock source
  * @rmtoll CCIPR        ADCSEL        LL_RCC_GetADCClockSource
  * @param  ADCx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_ADC_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_NONE
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_PLLSAI1 (*)
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_PLLSAI2 (*)
  *         @arg @ref LL_RCC_ADC_CLKSOURCE_SYSCLK
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_GetADCClockSource(uint32_t ADCx)
{
#if defined(RCC_CCIPR_ADCSEL)
  return (uint32_t)(READ_BIT(RCC->CCIPR, ADCx));
#else
  (void)ADCx;  /* unused */
  return ((READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADCEN) != 0U) ? LL_RCC_ADC_CLKSOURCE_SYSCLK : LL_RCC_ADC_CLKSOURCE_NONE);
#endif /* RCC_CCIPR_ADCSEL */
}

#if defined(SWPMI1)
/**
  * @brief  Get SWPMIx clock source
  * @rmtoll CCIPR        SWPMI1SEL     LL_RCC_GetSWPMIClockSource
  * @param  SPWMIx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_SWPMI1_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_SWPMI1_CLKSOURCE_PCLK1
  *         @arg @ref LL_RCC_SWPMI1_CLKSOURCE_HSI
  */
__STATIC_INLINE uint32_t LL_RCC_GetSWPMIClockSource(uint32_t SPWMIx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR, SPWMIx));
}
#endif /* SWPMI1 */

#if defined(DFSDM1_Channel0)
#if defined(RCC_CCIPR2_ADFSDM1SEL)
/**
  * @brief  Get DFSDM Audio Clock Source
  * @rmtoll CCIPR2         ADFSDM1SEL        LL_RCC_GetDFSDMAudioClockSource
  * @param  DFSDMx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_DFSDM1_AUDIO_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_DFSDM1_AUDIO_CLKSOURCE_SAI1
  *         @arg @ref LL_RCC_DFSDM1_AUDIO_CLKSOURCE_HSI
  *         @arg @ref LL_RCC_DFSDM1_AUDIO_CLKSOURCE_MSI
  */
__STATIC_INLINE uint32_t LL_RCC_GetDFSDMAudioClockSource(uint32_t DFSDMx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR2, DFSDMx));
}
#endif /* RCC_CCIPR2_ADFSDM1SEL */

/**
  * @brief  Get DFSDMx Kernel clock source
  @if STM32L4S9xx
  * @rmtoll CCIPR2       DFSDM1SEL     LL_RCC_GetDFSDMClockSource
  @else
  * @rmtoll CCIPR        DFSDM1SEL     LL_RCC_GetDFSDMClockSource
  @endif
  * @param  DFSDMx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_DFSDM1_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_DFSDM1_CLKSOURCE_PCLK2
  *         @arg @ref LL_RCC_DFSDM1_CLKSOURCE_SYSCLK
  */
__STATIC_INLINE uint32_t LL_RCC_GetDFSDMClockSource(uint32_t DFSDMx)
{
#if defined(RCC_CCIPR2_DFSDM1SEL)
  return (uint32_t)(READ_BIT(RCC->CCIPR2, DFSDMx));
#else
  return (uint32_t)(READ_BIT(RCC->CCIPR, DFSDMx));
#endif /* RCC_CCIPR2_DFSDM1SEL */
}
#endif /* DFSDM1_Channel0 */

#if defined(DSI)
/**
  * @brief  Get DSI Clock Source
  * @rmtoll CCIPR2         DSISEL        LL_RCC_GetDSIClockSource
  * @param  DSIx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_DSI_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_DSI_CLKSOURCE_PHY
  *         @arg @ref LL_RCC_DSI_CLKSOURCE_PLL
  */
__STATIC_INLINE uint32_t LL_RCC_GetDSIClockSource(uint32_t DSIx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR2, DSIx));
}
#endif /* DSI */

#if defined(LTDC)
/**
  * @brief  Get LTDC Clock Source
  * @rmtoll CCIPR2         PLLSAI2DIVR        LL_RCC_GetLTDCClockSource
  * @param  LTDCx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV2
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV4
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV8
  *         @arg @ref LL_RCC_LTDC_CLKSOURCE_PLLSAI2R_DIV16
  */
__STATIC_INLINE uint32_t LL_RCC_GetLTDCClockSource(uint32_t LTDCx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR2, LTDCx));
}
#endif /* LTDC */

#if defined(OCTOSPI1)
/**
  * @brief  Get OCTOSPI clock source
  * @rmtoll CCIPR2         OSPISEL        LL_RCC_GetOCTOSPIClockSource
  * @param  OCTOSPIx This parameter can be one of the following values:
  *         @arg @ref LL_RCC_OCTOSPI_CLKSOURCE
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_OCTOSPI_CLKSOURCE_SYSCLK
  *         @arg @ref LL_RCC_OCTOSPI_CLKSOURCE_MSI
  *         @arg @ref LL_RCC_OCTOSPI_CLKSOURCE_PLL
  */
__STATIC_INLINE uint32_t LL_RCC_GetOCTOSPIClockSource(uint32_t OCTOSPIx)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR2, OCTOSPIx));
}
#endif /* OCTOSPI1 */
/**
  * @}
  */

/** @defgroup RCC_LL_EF_RTC RTC
  * @{
  */

/**
  * @brief  Set RTC Clock Source
  * @note Once the RTC clock source has been selected, it cannot be changed anymore unless
  *       the Backup domain is reset, or unless a failure is detected on LSE (LSECSSD is
  *       set). The BDRST bit can be used to reset them.
  * @rmtoll BDCR         RTCSEL        LL_RCC_SetRTCClockSource
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_NONE
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_HSE_DIV32
  * @retval None
  */
__STATIC_INLINE void LL_RCC_SetRTCClockSource(uint32_t Source)
{
  MODIFY_REG(RCC->BDCR, RCC_BDCR_RTCSEL, Source);
}

/**
  * @brief  Get RTC Clock Source
  * @rmtoll BDCR         RTCSEL        LL_RCC_GetRTCClockSource
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_NONE
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_LSE
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_LSI
  *         @arg @ref LL_RCC_RTC_CLKSOURCE_HSE_DIV32
  */
__STATIC_INLINE uint32_t LL_RCC_GetRTCClockSource(void)
{
  return (uint32_t)(READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL));
}

/**
  * @brief  Enable RTC
  * @rmtoll BDCR         RTCEN         LL_RCC_EnableRTC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableRTC(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
}

/**
  * @brief  Disable RTC
  * @rmtoll BDCR         RTCEN         LL_RCC_DisableRTC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableRTC(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
}

/**
  * @brief  Check if RTC has been enabled or not
  * @rmtoll BDCR         RTCEN         LL_RCC_IsEnabledRTC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledRTC(void)
{
  return ((READ_BIT(RCC->BDCR, RCC_BDCR_RTCEN) == RCC_BDCR_RTCEN) ? 1UL : 0UL);
}

/**
  * @brief  Force the Backup domain reset
  * @rmtoll BDCR         BDRST         LL_RCC_ForceBackupDomainReset
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ForceBackupDomainReset(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
}

/**
  * @brief  Release the Backup domain reset
  * @rmtoll BDCR         BDRST         LL_RCC_ReleaseBackupDomainReset
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ReleaseBackupDomainReset(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);
}

/**
  * @}
  */


/** @defgroup RCC_LL_EF_PLL PLL
  * @{
  */

/**
  * @brief  Enable PLL
  * @rmtoll CR           PLLON         LL_RCC_PLL_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_Enable(void)
{
  SET_BIT(RCC->CR, RCC_CR_PLLON);
}

/**
  * @brief  Disable PLL
  * @note Cannot be disabled if the PLL clock is used as the system clock
  * @rmtoll CR           PLLON         LL_RCC_PLL_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_Disable(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
}

/**
  * @brief  Check if PLL Ready
  * @rmtoll CR           PLLRDY        LL_RCC_PLL_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_IsReady(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_PLLRDY) == RCC_CR_PLLRDY) ? 1UL : 0UL);
}

/**
  * @brief  Configure PLL used for SYSCLK Domain
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLN/PLLR can be written only when PLL is disabled.
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLL_ConfigDomain_SYS\n
  *         PLLCFGR      PLLM          LL_RCC_PLL_ConfigDomain_SYS\n
  *         PLLCFGR      PLLN          LL_RCC_PLL_ConfigDomain_SYS\n
  *         PLLCFGR      PLLR          LL_RCC_PLL_ConfigDomain_SYS
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  *         @arg @ref LL_RCC_PLLM_DIV_9 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_10 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_11 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_12 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_13 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_14 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_15 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_16 (*)
  *
  *         (*) value not defined in all devices.
  * @param  PLLN Between 8 and 86
  * @param  PLLR This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLR_DIV_2
  *         @arg @ref LL_RCC_PLLR_DIV_4
  *         @arg @ref LL_RCC_PLLR_DIV_6
  *         @arg @ref LL_RCC_PLLR_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_ConfigDomain_SYS(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLR)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLR,
             Source | PLLM | (PLLN << RCC_PLLCFGR_PLLN_Pos) | PLLR);
}

#if defined(RCC_PLLP_SUPPORT)
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
/**
  * @brief  Configure PLL used for SAI domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLN/PLLP can be written only when PLL is disabled.
  * @note This  can be selected for SAI1 or SAI2 (*)
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLL_ConfigDomain_SAI\n
  *         PLLCFGR      PLLM          LL_RCC_PLL_ConfigDomain_SAI\n
  *         PLLCFGR      PLLN          LL_RCC_PLL_ConfigDomain_SAI\n
  *         PLLCFGR      PLLPDIV       LL_RCC_PLL_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  *         @arg @ref LL_RCC_PLLM_DIV_9 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_10 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_11 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_12 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_13 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_14 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_15 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_16 (*)
  *
  *         (*) value not defined in all devices.
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLP_DIV_2
  *         @arg @ref LL_RCC_PLLP_DIV_3
  *         @arg @ref LL_RCC_PLLP_DIV_4
  *         @arg @ref LL_RCC_PLLP_DIV_5
  *         @arg @ref LL_RCC_PLLP_DIV_6
  *         @arg @ref LL_RCC_PLLP_DIV_7
  *         @arg @ref LL_RCC_PLLP_DIV_8
  *         @arg @ref LL_RCC_PLLP_DIV_9
  *         @arg @ref LL_RCC_PLLP_DIV_10
  *         @arg @ref LL_RCC_PLLP_DIV_11
  *         @arg @ref LL_RCC_PLLP_DIV_12
  *         @arg @ref LL_RCC_PLLP_DIV_13
  *         @arg @ref LL_RCC_PLLP_DIV_14
  *         @arg @ref LL_RCC_PLLP_DIV_15
  *         @arg @ref LL_RCC_PLLP_DIV_16
  *         @arg @ref LL_RCC_PLLP_DIV_17
  *         @arg @ref LL_RCC_PLLP_DIV_18
  *         @arg @ref LL_RCC_PLLP_DIV_19
  *         @arg @ref LL_RCC_PLLP_DIV_20
  *         @arg @ref LL_RCC_PLLP_DIV_21
  *         @arg @ref LL_RCC_PLLP_DIV_22
  *         @arg @ref LL_RCC_PLLP_DIV_23
  *         @arg @ref LL_RCC_PLLP_DIV_24
  *         @arg @ref LL_RCC_PLLP_DIV_25
  *         @arg @ref LL_RCC_PLLP_DIV_26
  *         @arg @ref LL_RCC_PLLP_DIV_27
  *         @arg @ref LL_RCC_PLLP_DIV_28
  *         @arg @ref LL_RCC_PLLP_DIV_29
  *         @arg @ref LL_RCC_PLLP_DIV_30
  *         @arg @ref LL_RCC_PLLP_DIV_31
  * @retval None
  */
#else
/**
  * @brief  Configure PLL used for SAI domain clock
  * @note   PLL Source and PLLM Divider can be written only when PLL,
  *         PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note   PLLN/PLLP can be written only when PLL is disabled.
  * @note   This  can be selected for SAI1 or SAI2 (*)
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLL_ConfigDomain_SAI\n
  *         PLLCFGR      PLLM          LL_RCC_PLL_ConfigDomain_SAI\n
  *         PLLCFGR      PLLN          LL_RCC_PLL_ConfigDomain_SAI\n
  *         PLLCFGR      PLLP          LL_RCC_PLL_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLP_DIV_7
  *         @arg @ref LL_RCC_PLLP_DIV_17
  * @retval None
  */
#endif /* RCC_PLLP_DIV_2_31_SUPPORT */
__STATIC_INLINE void LL_RCC_PLL_ConfigDomain_SAI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLP)
{
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLPDIV,
             Source | PLLM | (PLLN << RCC_PLLCFGR_PLLN_Pos) | PLLP);
#else
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP,
             Source | PLLM | (PLLN << RCC_PLLCFGR_PLLN_Pos) | PLLP);
#endif /* RCC_PLLP_DIV_2_31_SUPPORT */
}
#endif /* RCC_PLLP_SUPPORT */

/**
  * @brief  Configure PLL used for 48Mhz domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLN/PLLQ can be written only when PLL is disabled.
  * @note This  can be selected for USB, RNG, SDMMC
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLL_ConfigDomain_48M\n
  *         PLLCFGR      PLLM          LL_RCC_PLL_ConfigDomain_48M\n
  *         PLLCFGR      PLLN          LL_RCC_PLL_ConfigDomain_48M\n
  *         PLLCFGR      PLLQ          LL_RCC_PLL_ConfigDomain_48M
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  *         @arg @ref LL_RCC_PLLM_DIV_9 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_10 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_11 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_12 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_13 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_14 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_15 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_16 (*)
  *
  *         (*) value not defined in all devices.
  * @param  PLLN Between 8 and 86
  * @param  PLLQ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLQ_DIV_2
  *         @arg @ref LL_RCC_PLLQ_DIV_4
  *         @arg @ref LL_RCC_PLLQ_DIV_6
  *         @arg @ref LL_RCC_PLLQ_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_ConfigDomain_48M(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLQ)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLQ,
             Source | PLLM | (PLLN << RCC_PLLCFGR_PLLN_Pos) | PLLQ);
}

/**
  * @brief  Configure PLL clock source
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLL_SetMainSource
  * @param  PLLSource This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_SetMainSource(uint32_t PLLSource)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, PLLSource);
}

/**
  * @brief  Get the oscillator used as PLL clock source.
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLL_GetMainSource
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_GetMainSource(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC));
}

/**
  * @brief  Get Main PLL multiplication factor for VCO
  * @rmtoll PLLCFGR      PLLN          LL_RCC_PLL_GetN
  * @retval Between 8 and 86
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_GetN(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >>  RCC_PLLCFGR_PLLN_Pos);
}

#if defined(RCC_PLLP_SUPPORT)
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
/**
  * @brief  Get Main PLL division factor for PLLP
  * @note Used for PLLSAI3CLK (SAI1 and SAI2 clock)
  * @rmtoll PLLCFGR      PLLPDIV       LL_RCC_PLL_GetP
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLP_DIV_2
  *         @arg @ref LL_RCC_PLLP_DIV_3
  *         @arg @ref LL_RCC_PLLP_DIV_4
  *         @arg @ref LL_RCC_PLLP_DIV_5
  *         @arg @ref LL_RCC_PLLP_DIV_6
  *         @arg @ref LL_RCC_PLLP_DIV_7
  *         @arg @ref LL_RCC_PLLP_DIV_8
  *         @arg @ref LL_RCC_PLLP_DIV_9
  *         @arg @ref LL_RCC_PLLP_DIV_10
  *         @arg @ref LL_RCC_PLLP_DIV_11
  *         @arg @ref LL_RCC_PLLP_DIV_12
  *         @arg @ref LL_RCC_PLLP_DIV_13
  *         @arg @ref LL_RCC_PLLP_DIV_14
  *         @arg @ref LL_RCC_PLLP_DIV_15
  *         @arg @ref LL_RCC_PLLP_DIV_16
  *         @arg @ref LL_RCC_PLLP_DIV_17
  *         @arg @ref LL_RCC_PLLP_DIV_18
  *         @arg @ref LL_RCC_PLLP_DIV_19
  *         @arg @ref LL_RCC_PLLP_DIV_20
  *         @arg @ref LL_RCC_PLLP_DIV_21
  *         @arg @ref LL_RCC_PLLP_DIV_22
  *         @arg @ref LL_RCC_PLLP_DIV_23
  *         @arg @ref LL_RCC_PLLP_DIV_24
  *         @arg @ref LL_RCC_PLLP_DIV_25
  *         @arg @ref LL_RCC_PLLP_DIV_26
  *         @arg @ref LL_RCC_PLLP_DIV_27
  *         @arg @ref LL_RCC_PLLP_DIV_28
  *         @arg @ref LL_RCC_PLLP_DIV_29
  *         @arg @ref LL_RCC_PLLP_DIV_30
  *         @arg @ref LL_RCC_PLLP_DIV_31
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_GetP(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLPDIV));
}
#else
/**
  * @brief  Get Main PLL division factor for PLLP
  * @note Used for PLLSAI3CLK (SAI1 and SAI2 clock)
  * @rmtoll PLLCFGR      PLLP          LL_RCC_PLL_GetP
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLP_DIV_7
  *         @arg @ref LL_RCC_PLLP_DIV_17
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_GetP(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLP));
}
#endif /* RCC_PLLP_DIV_2_31_SUPPORT */
#endif /* RCC_PLLP_SUPPORT */

/**
  * @brief  Get Main PLL division factor for PLLQ
  * @note Used for PLL48M1CLK selected for USB, RNG, SDMMC (48 MHz clock)
  * @rmtoll PLLCFGR      PLLQ          LL_RCC_PLL_GetQ
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLQ_DIV_2
  *         @arg @ref LL_RCC_PLLQ_DIV_4
  *         @arg @ref LL_RCC_PLLQ_DIV_6
  *         @arg @ref LL_RCC_PLLQ_DIV_8
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_GetQ(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLQ));
}

/**
  * @brief  Get Main PLL division factor for PLLR
  * @note Used for PLLCLK (system clock)
  * @rmtoll PLLCFGR      PLLR          LL_RCC_PLL_GetR
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLR_DIV_2
  *         @arg @ref LL_RCC_PLLR_DIV_4
  *         @arg @ref LL_RCC_PLLR_DIV_6
  *         @arg @ref LL_RCC_PLLR_DIV_8
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_GetR(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLR));
}

/**
  * @brief  Get Division factor for the main PLL and other PLL
  * @rmtoll PLLCFGR      PLLM          LL_RCC_PLL_GetDivider
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  *         @arg @ref LL_RCC_PLLM_DIV_9 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_10 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_11 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_12 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_13 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_14 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_15 (*)
  *         @arg @ref LL_RCC_PLLM_DIV_16 (*)
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_RCC_PLL_GetDivider(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLM));
}

#if defined(RCC_PLLP_SUPPORT)
/**
  * @brief  Enable PLL output mapped on SAI domain clock
  * @rmtoll PLLCFGR      PLLPEN        LL_RCC_PLL_EnableDomain_SAI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_EnableDomain_SAI(void)
{
  SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLPEN);
}

/**
  * @brief  Disable PLL output mapped on SAI domain clock
  * @note Cannot be disabled if the PLL clock is used as the system
  *       clock
  * @note In order to save power, when the PLLCLK  of the PLL is
  *       not used,  should be 0
  * @rmtoll PLLCFGR      PLLPEN        LL_RCC_PLL_DisableDomain_SAI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_DisableDomain_SAI(void)
{
  CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLPEN);
}
#endif /* RCC_PLLP_SUPPORT */

/**
  * @brief  Enable PLL output mapped on 48MHz domain clock
  * @rmtoll PLLCFGR      PLLQEN        LL_RCC_PLL_EnableDomain_48M
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_EnableDomain_48M(void)
{
  SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLQEN);
}

/**
  * @brief  Disable PLL output mapped on 48MHz domain clock
  * @note Cannot be disabled if the PLL clock is used as the system
  *       clock
  * @note In order to save power, when the PLLCLK  of the PLL is
  *       not used,  should be 0
  * @rmtoll PLLCFGR      PLLQEN        LL_RCC_PLL_DisableDomain_48M
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_DisableDomain_48M(void)
{
  CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLQEN);
}

/**
  * @brief  Enable PLL output mapped on SYSCLK domain
  * @rmtoll PLLCFGR      PLLREN        LL_RCC_PLL_EnableDomain_SYS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_EnableDomain_SYS(void)
{
  SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN);
}

/**
  * @brief  Disable PLL output mapped on SYSCLK domain
  * @note Cannot be disabled if the PLL clock is used as the system
  *       clock
  * @note In order to save power, when the PLLCLK  of the PLL is
  *       not used, Main PLL  should be 0
  * @rmtoll PLLCFGR      PLLREN        LL_RCC_PLL_DisableDomain_SYS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLL_DisableDomain_SYS(void)
{
  CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN);
}

/**
  * @}
  */

#if defined(RCC_PLLSAI1_SUPPORT)
/** @defgroup RCC_LL_EF_PLLSAI1 PLLSAI1
  * @{
  */

/**
  * @brief  Enable PLLSAI1
  * @rmtoll CR           PLLSAI1ON     LL_RCC_PLLSAI1_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_Enable(void)
{
  SET_BIT(RCC->CR, RCC_CR_PLLSAI1ON);
}

/**
  * @brief  Disable PLLSAI1
  * @rmtoll CR           PLLSAI1ON     LL_RCC_PLLSAI1_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_Disable(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_PLLSAI1ON);
}

/**
  * @brief  Check if PLLSAI1 Ready
  * @rmtoll CR           PLLSAI1RDY    LL_RCC_PLLSAI1_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI1_IsReady(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_PLLSAI1RDY) == RCC_CR_PLLSAI1RDY) ? 1UL : 0UL);
}

#if defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT)
/**
  * @brief  Configure PLLSAI1 used for 48Mhz domain clock
  * @note PLL Source can be written only when PLL, PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI1M/PLLSAI1N/PLLSAI1Q can be written only when PLLSAI1 is disabled.
  * @note This  can be selected for USB, RNG, SDMMC
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI1_ConfigDomain_48M\n
  *         PLLSAI1CFGR  PLLSAI1M      LL_RCC_PLLSAI1_ConfigDomain_48M\n
  *         PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_ConfigDomain_48M\n
  *         PLLSAI1CFGR  PLLSAI1Q      LL_RCC_PLLSAI1_ConfigDomain_48M
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_16
  * @param  PLLN Between 8 and 86
  * @param  PLLQ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_ConfigDomain_48M(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLQ)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, Source);
  MODIFY_REG(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1M | RCC_PLLSAI1CFGR_PLLSAI1N | RCC_PLLSAI1CFGR_PLLSAI1Q,
             PLLM | (PLLN << RCC_PLLSAI1CFGR_PLLSAI1N_Pos) | PLLQ);
}
#else
/**
  * @brief  Configure PLLSAI1 used for 48Mhz domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI1N/PLLSAI1Q can be written only when PLLSAI1 is disabled.
  * @note This  can be selected for USB, RNG, SDMMC
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI1_ConfigDomain_48M\n
  *         PLLCFGR      PLLM          LL_RCC_PLLSAI1_ConfigDomain_48M\n
  *         PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_ConfigDomain_48M\n
  *         PLLSAI1CFGR  PLLSAI1Q      LL_RCC_PLLSAI1_ConfigDomain_48M
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLQ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_ConfigDomain_48M(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLQ)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM, Source | PLLM);
  MODIFY_REG(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1N | RCC_PLLSAI1CFGR_PLLSAI1Q, PLLN << RCC_PLLSAI1CFGR_PLLSAI1N_Pos | PLLQ);
}
#endif /* RCC_PLLSAI1M_DIV_1_16_SUPPORT */

#if defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT) && defined(RCC_PLLSAI1P_DIV_2_31_SUPPORT)
/**
  * @brief  Configure PLLSAI1 used for SAI domain clock
  * @note PLL Source can be written only when PLL, PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI1M/PLLSAI1N/PLLSAI1PDIV can be written only when PLLSAI1 is disabled.
  * @note This  can be selected for SAI1 or SAI2
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLSAI1CFGR  PLLSAI1M      LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLSAI1CFGR  PLLSAI1PDIV   LL_RCC_PLLSAI1_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_16
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_31
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_ConfigDomain_SAI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLP)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, Source);
  MODIFY_REG(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1M | RCC_PLLSAI1CFGR_PLLSAI1N | RCC_PLLSAI1CFGR_PLLSAI1PDIV,
             PLLM | (PLLN << RCC_PLLSAI1CFGR_PLLSAI1N_Pos) | PLLP);
}
#elif defined(RCC_PLLSAI1P_DIV_2_31_SUPPORT)
/**
  * @brief  Configure PLLSAI1 used for SAI domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI1N/PLLSAI1PDIV can be written only when PLLSAI1 is disabled.
  * @note This  can be selected for SAI1 or SAI2 (*)
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLCFGR      PLLM          LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLSAI1CFGR  PLLSAI1PDIV   LL_RCC_PLLSAI1_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_31
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_ConfigDomain_SAI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLP)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM, Source | PLLM);
  MODIFY_REG(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1N | RCC_PLLSAI1CFGR_PLLSAI1PDIV,
             PLLN << RCC_PLLSAI1CFGR_PLLSAI1N_Pos | PLLP);
}
#else
/**
  * @brief  Configure PLLSAI1 used for SAI domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI1N/PLLSAI1P can be written only when PLLSAI1 is disabled.
  * @note This  can be selected for SAI1 or SAI2 (*)
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLCFGR      PLLM          LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_ConfigDomain_SAI\n
  *         PLLSAI1CFGR  PLLSAI1P      LL_RCC_PLLSAI1_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_ConfigDomain_SAI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLP)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM, Source | PLLM);
  MODIFY_REG(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1N | RCC_PLLSAI1CFGR_PLLSAI1P, PLLN << RCC_PLLSAI1CFGR_PLLSAI1N_Pos | PLLP);
}
#endif /* RCC_PLLSAI1M_DIV_1_16_SUPPORT && RCC_PLLSAI1P_DIV_2_31_SUPPORT */

#if defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT)
/**
  * @brief  Configure PLLSAI1 used for ADC domain clock
  * @note PLL Source can be written only when PLL, PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI1M/PLLSAI1N/PLLSAI1R can be written only when PLLSAI1 is disabled.
  * @note This  can be selected for ADC
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI1_ConfigDomain_ADC\n
  *         PLLSAI1CFGR  PLLSAI1M      LL_RCC_PLLSAI1_ConfigDomain_ADC\n
  *         PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_ConfigDomain_ADC\n
  *         PLLSAI1CFGR  PLLSAI1R      LL_RCC_PLLSAI1_ConfigDomain_ADC
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_16
  * @param  PLLN Between 8 and 86
  * @param  PLLR This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_ConfigDomain_ADC(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLR)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, Source);
  MODIFY_REG(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1M | RCC_PLLSAI1CFGR_PLLSAI1N | RCC_PLLSAI1CFGR_PLLSAI1R,
             PLLM | (PLLN << RCC_PLLSAI1CFGR_PLLSAI1N_Pos) | PLLR);
}
#else
/**
  * @brief  Configure PLLSAI1 used for ADC domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLN/PLLR can be written only when PLLSAI1 is disabled.
  * @note This  can be selected for ADC
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI1_ConfigDomain_ADC\n
  *         PLLCFGR      PLLM          LL_RCC_PLLSAI1_ConfigDomain_ADC\n
  *         PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_ConfigDomain_ADC\n
  *         PLLSAI1CFGR  PLLSAI1R      LL_RCC_PLLSAI1_ConfigDomain_ADC
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLR This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_ConfigDomain_ADC(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLR)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM, Source | PLLM);
  MODIFY_REG(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1N | RCC_PLLSAI1CFGR_PLLSAI1R, PLLN << RCC_PLLSAI1CFGR_PLLSAI1N_Pos | PLLR);
}
#endif /* RCC_PLLSAI1M_DIV_1_16_SUPPORT */

/**
  * @brief  Get SAI1PLL multiplication factor for VCO
  * @rmtoll PLLSAI1CFGR  PLLSAI1N      LL_RCC_PLLSAI1_GetN
  * @retval Between 8 and 86
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI1_GetN(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1N) >> RCC_PLLSAI1CFGR_PLLSAI1N_Pos);
}

#if defined(RCC_PLLSAI1P_DIV_2_31_SUPPORT)
/**
  * @brief  Get SAI1PLL division factor for PLLSAI1P
  * @note Used for PLLSAI1CLK (SAI1 or SAI2 (*) clock).
  * @rmtoll PLLSAI1CFGR  PLLSAI1PDIV      LL_RCC_PLLSAI1_GetP
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_31
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI1_GetP(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1PDIV));
}
#else
/**
  * @brief  Get SAI1PLL division factor for PLLSAI1P
  * @note Used for PLLSAI1CLK (SAI1 or SAI2 (*) clock).
  * @rmtoll PLLSAI1CFGR  PLLSAI1P      LL_RCC_PLLSAI1_GetP
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1P_DIV_17
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI1_GetP(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1P));
}
#endif /* RCC_PLLSAI1P_DIV_2_31_SUPPORT */

/**
  * @brief  Get SAI1PLL division factor for PLLSAI1Q
  * @note Used PLL48M2CLK selected for USB, RNG, SDMMC (48 MHz clock)
  * @rmtoll PLLSAI1CFGR  PLLSAI1Q      LL_RCC_PLLSAI1_GetQ
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1Q_DIV_8
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI1_GetQ(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1Q));
}

/**
  * @brief  Get PLLSAI1 division factor for PLLSAIR
  * @note Used for PLLADC1CLK (ADC clock)
  * @rmtoll PLLSAI1CFGR  PLLSAI1R      LL_RCC_PLLSAI1_GetR
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1R_DIV_8
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI1_GetR(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1R));
}

#if  defined(RCC_PLLSAI1M_DIV_1_16_SUPPORT)
/**
  * @brief  Get Division factor for the PLLSAI1
  * @rmtoll PLLSAI1CFGR  PLLSAI1M      LL_RCC_PLLSAI1_GetDivider
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI1M_DIV_16
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI1_GetDivider(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1M));
}
#endif /* RCC_PLLSAI1M_DIV_1_16_SUPPORT */

/**
  * @brief  Enable PLLSAI1 output mapped on SAI domain clock
  * @rmtoll PLLSAI1CFGR  PLLSAI1PEN    LL_RCC_PLLSAI1_EnableDomain_SAI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_EnableDomain_SAI(void)
{
  SET_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1PEN);
}

/**
  * @brief  Disable PLLSAI1 output mapped on SAI domain clock
  * @note In order to save power, when  of the PLLSAI1 is
  *       not used,  should be 0
  * @rmtoll PLLSAI1CFGR  PLLSAI1PEN    LL_RCC_PLLSAI1_DisableDomain_SAI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_DisableDomain_SAI(void)
{
  CLEAR_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1PEN);
}

/**
  * @brief  Enable PLLSAI1 output mapped on 48MHz domain clock
  * @rmtoll PLLSAI1CFGR  PLLSAI1QEN    LL_RCC_PLLSAI1_EnableDomain_48M
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_EnableDomain_48M(void)
{
  SET_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1QEN);
}

/**
  * @brief  Disable PLLSAI1 output mapped on 48MHz domain clock
  * @note In order to save power, when  of the PLLSAI1 is
  *       not used,  should be 0
  * @rmtoll PLLSAI1CFGR  PLLSAI1QEN    LL_RCC_PLLSAI1_DisableDomain_48M
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_DisableDomain_48M(void)
{
  CLEAR_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1QEN);
}

/**
  * @brief  Enable PLLSAI1 output mapped on ADC domain clock
  * @rmtoll PLLSAI1CFGR  PLLSAI1REN    LL_RCC_PLLSAI1_EnableDomain_ADC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_EnableDomain_ADC(void)
{
  SET_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1REN);
}

/**
  * @brief  Disable PLLSAI1 output mapped on ADC domain clock
  * @note In order to save power, when  of the PLLSAI1 is
  *       not used, Main PLLSAI1  should be 0
  * @rmtoll PLLSAI1CFGR  PLLSAI1REN    LL_RCC_PLLSAI1_DisableDomain_ADC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI1_DisableDomain_ADC(void)
{
  CLEAR_BIT(RCC->PLLSAI1CFGR, RCC_PLLSAI1CFGR_PLLSAI1REN);
}

/**
  * @}
  */
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
/** @defgroup RCC_LL_EF_PLLSAI2 PLLSAI2
  * @{
  */

/**
  * @brief  Enable PLLSAI2
  * @rmtoll CR           PLLSAI2ON     LL_RCC_PLLSAI2_Enable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_Enable(void)
{
  SET_BIT(RCC->CR, RCC_CR_PLLSAI2ON);
}

/**
  * @brief  Disable PLLSAI2
  * @rmtoll CR           PLLSAI2ON     LL_RCC_PLLSAI2_Disable
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_Disable(void)
{
  CLEAR_BIT(RCC->CR, RCC_CR_PLLSAI2ON);
}

/**
  * @brief  Check if PLLSAI2 Ready
  * @rmtoll CR           PLLSAI2RDY    LL_RCC_PLLSAI2_IsReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_IsReady(void)
{
  return ((READ_BIT(RCC->CR, RCC_CR_PLLSAI2RDY) == RCC_CR_PLLSAI2RDY) ? 1UL : 0UL);
}

#if defined(RCC_PLLSAI2M_DIV_1_16_SUPPORT) && defined(RCC_PLLSAI2P_DIV_2_31_SUPPORT)
/**
  * @brief  Configure PLLSAI2 used for SAI domain clock
  * @note PLL Source can be written only when PLL, PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI2M/PLLSAI2N/PLLSAI2PDIV can be written only when PLLSAI2 is disabled.
  * @note This  can be selected for SAI1 or SAI2
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLSAI2CFGR  PLLSAI2M      LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLSAI2CFGR  PLLSAI2N      LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLSAI2CFGR  PLLSAI2PDIV   LL_RCC_PLLSAI2_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_16
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_31
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_ConfigDomain_SAI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLP)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, Source);
  MODIFY_REG(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2M | RCC_PLLSAI2CFGR_PLLSAI2N | RCC_PLLSAI2CFGR_PLLSAI2PDIV,
             PLLM | (PLLN << RCC_PLLSAI2CFGR_PLLSAI2N_Pos) | PLLP);
}
#elif defined(RCC_PLLSAI2P_DIV_2_31_SUPPORT)
/**
  * @brief  Configure PLLSAI2 used for SAI domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI1 and PLLSAI2 are disabled.
  * @note PLLSAI2N/PLLSAI2PDIV can be written only when PLLSAI2 is disabled.
  * @note This  can be selected for SAI1 or SAI2
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLCFGR      PLLM          LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLSAI2CFGR  PLLSAI2N      LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLSAI2CFGR  PLLSAI2PDIV   LL_RCC_PLLSAI2_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_31
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_ConfigDomain_SAI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLP)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM, Source | PLLM);
  MODIFY_REG(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2N | RCC_PLLSAI2CFGR_PLLSAI2PDIV, PLLN << RCC_PLLSAI2CFGR_PLLSAI2N_Pos | PLLP);
}
#else
/**
  * @brief  Configure PLLSAI2 used for SAI domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI2 and PLLSAI2 are disabled.
  * @note PLLSAI2N/PLLSAI2P can be written only when PLLSAI2 is disabled.
  * @note This  can be selected for SAI1 or SAI2
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLCFGR      PLLM          LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLSAI2CFGR  PLLSAI2N      LL_RCC_PLLSAI2_ConfigDomain_SAI\n
  *         PLLSAI2CFGR  PLLSAI2P      LL_RCC_PLLSAI2_ConfigDomain_SAI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLP This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_ConfigDomain_SAI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLP)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM, Source | PLLM);
  MODIFY_REG(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2N | RCC_PLLSAI2CFGR_PLLSAI2P, PLLN << RCC_PLLSAI2CFGR_PLLSAI2N_Pos | PLLP);
}
#endif /* RCC_PLLSAI2M_DIV_1_16_SUPPORT && RCC_PLLSAI2P_DIV_2_31_SUPPORT */

#if defined(DSI)
/**
  * @brief  Configure PLLSAI2 used for DSI domain clock
  * @note PLL Source can be written only when PLL, PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI2M/PLLSAI2N/PLLSAI2Q can be written only when PLLSAI2 is disabled.
  * @note This  can be selected for DSI
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI2_ConfigDomain_DSI\n
  *         PLLSAI2CFGR  PLLSAI2M      LL_RCC_PLLSAI2_ConfigDomain_DSI\n
  *         PLLSAI2CFGR  PLLSAI2N      LL_RCC_PLLSAI2_ConfigDomain_DSI\n
  *         PLLSAI2CFGR  PLLSAI2Q      LL_RCC_PLLSAI2_ConfigDomain_DSI
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_16
  * @param  PLLN Between 8 and 86
  * @param  PLLQ This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_ConfigDomain_DSI(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLQ)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, Source);
  MODIFY_REG(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2M | RCC_PLLSAI2CFGR_PLLSAI2N | RCC_PLLSAI2CFGR_PLLSAI2Q,
             (PLLN << RCC_PLLSAI2CFGR_PLLSAI2N_Pos) | PLLQ | PLLM);
}
#endif /* DSI */

#if defined(LTDC)
/**
  * @brief  Configure PLLSAI2 used for LTDC domain clock
  * @note PLL Source can be written only when PLL, PLLSAI1 and PLLSAI2 (*) are disabled.
  * @note PLLSAI2M/PLLSAI2N/PLLSAI2R can be written only when PLLSAI2 is disabled.
  * @note This  can be selected for LTDC
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI2_ConfigDomain_LTDC\n
  *         PLLSAI2CFGR  PLLSAI2M      LL_RCC_PLLSAI2_ConfigDomain_LTDC\n
  *         PLLSAI2CFGR  PLLSAI2N      LL_RCC_PLLSAI2_ConfigDomain_LTDC\n
  *         PLLSAI2CFGR  PLLSAI2R      LL_RCC_PLLSAI2_ConfigDomain_LTDC\n
  *         CCIPR2       PLLSAI2DIVR   LL_RCC_PLLSAI2_ConfigDomain_LTDC
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_16
  * @param  PLLN Between 8 and 86
  * @param  PLLR This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_8
  * @param  PLLDIVR This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_16
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_ConfigDomain_LTDC(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLR, uint32_t PLLDIVR)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, Source);
  MODIFY_REG(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2M | RCC_PLLSAI2CFGR_PLLSAI2N | RCC_PLLSAI2CFGR_PLLSAI2R,
             (PLLN << RCC_PLLSAI2CFGR_PLLSAI2N_Pos) | PLLR | PLLM);
  MODIFY_REG(RCC->CCIPR2, RCC_CCIPR2_PLLSAI2DIVR, PLLDIVR);
}
#else
/**
  * @brief  Configure PLLSAI2 used for ADC domain clock
  * @note PLL Source and PLLM Divider can be written only when PLL,
  *       PLLSAI2 and PLLSAI2 are disabled.
  * @note PLLSAI2N/PLLSAI2R can be written only when PLLSAI2 is disabled.
  * @note This  can be selected for ADC
  * @rmtoll PLLCFGR      PLLSRC        LL_RCC_PLLSAI2_ConfigDomain_ADC\n
  *         PLLCFGR      PLLM          LL_RCC_PLLSAI2_ConfigDomain_ADC\n
  *         PLLSAI2CFGR  PLLSAI2N      LL_RCC_PLLSAI2_ConfigDomain_ADC\n
  *         PLLSAI2CFGR  PLLSAI2R      LL_RCC_PLLSAI2_ConfigDomain_ADC
  * @param  Source This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSOURCE_NONE
  *         @arg @ref LL_RCC_PLLSOURCE_MSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSI
  *         @arg @ref LL_RCC_PLLSOURCE_HSE
  * @param  PLLM This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLM_DIV_1
  *         @arg @ref LL_RCC_PLLM_DIV_2
  *         @arg @ref LL_RCC_PLLM_DIV_3
  *         @arg @ref LL_RCC_PLLM_DIV_4
  *         @arg @ref LL_RCC_PLLM_DIV_5
  *         @arg @ref LL_RCC_PLLM_DIV_6
  *         @arg @ref LL_RCC_PLLM_DIV_7
  *         @arg @ref LL_RCC_PLLM_DIV_8
  * @param  PLLN Between 8 and 86
  * @param  PLLR This parameter can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_8
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_ConfigDomain_ADC(uint32_t Source, uint32_t PLLM, uint32_t PLLN, uint32_t PLLR)
{
  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM, Source | PLLM);
  MODIFY_REG(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2N | RCC_PLLSAI2CFGR_PLLSAI2R, PLLN << RCC_PLLSAI2CFGR_PLLSAI2N_Pos | PLLR);
}
#endif /* LTDC */

/**
  * @brief  Get SAI2PLL multiplication factor for VCO
  * @rmtoll PLLSAI2CFGR  PLLSAI2N      LL_RCC_PLLSAI2_GetN
  * @retval Between 8 and 86
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_GetN(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2N) >> RCC_PLLSAI2CFGR_PLLSAI2N_Pos);
}

#if defined(RCC_PLLSAI2P_DIV_2_31_SUPPORT)
/**
  * @brief  Get SAI2PLL division factor for PLLSAI2P
  * @note Used for PLLSAI2CLK (SAI1 or SAI2 clock).
  * @rmtoll PLLSAI2CFGR  PLLSAI2PDIV    LL_RCC_PLLSAI2_GetP
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_16
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_18
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_19
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_20
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_21
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_22
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_23
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_24
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_25
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_26
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_27
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_28
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_29
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_30
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_31
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_GetP(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2PDIV));
}
#else
/**
  * @brief  Get SAI2PLL division factor for PLLSAI2P
  * @note Used for PLLSAI2CLK (SAI1 or SAI2 clock).
  * @rmtoll PLLSAI2CFGR  PLLSAI2P      LL_RCC_PLLSAI2_GetP
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2P_DIV_17
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_GetP(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2P));
}
#endif /* RCC_PLLSAI2P_DIV_2_31_SUPPORT */

#if defined(RCC_PLLSAI2Q_DIV_SUPPORT)
/**
  * @brief  Get division factor for PLLSAI2Q
  * @note Used for PLLDSICLK (DSI clock)
  * @rmtoll PLLSAI2CFGR  PLLSAI2Q      LL_RCC_PLLSAI2_GetQ
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2Q_DIV_8
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_GetQ(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2Q));
}
#endif /* RCC_PLLSAI2Q_DIV_SUPPORT */

/**
  * @brief  Get SAI2PLL division factor for PLLSAI2R
  * @note Used for PLLADC2CLK (ADC clock) or PLLLCDCLK (LTDC clock) depending on devices
  * @rmtoll PLLSAI2CFGR  PLLSAI2R      LL_RCC_PLLSAI2_GetR
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2R_DIV_8
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_GetR(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2R));
}

#if  defined(RCC_PLLSAI2M_DIV_1_16_SUPPORT)
/**
  * @brief  Get Division factor for the PLLSAI2
  * @rmtoll PLLSAI2CFGR  PLLSAI2M      LL_RCC_PLLSAI2_GetDivider
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_1
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_3
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_5
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_6
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_7
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_9
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_10
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_11
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_12
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_13
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_14
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_15
  *         @arg @ref LL_RCC_PLLSAI2M_DIV_16
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_GetDivider(void)
{
  return (uint32_t)(READ_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2M));
}
#endif /* RCC_PLLSAI2M_DIV_1_16_SUPPORT */

#if defined(RCC_CCIPR2_PLLSAI2DIVR)
/**
  * @brief  Get PLLSAI2 division factor for PLLSAI2DIVR
  * @note Used for LTDC domain clock
  * @rmtoll CCIPR2  PLLSAI2DIVR      LL_RCC_PLLSAI2_GetDIVR
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_2
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_4
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_8
  *         @arg @ref LL_RCC_PLLSAI2DIVR_DIV_16
  */
__STATIC_INLINE uint32_t LL_RCC_PLLSAI2_GetDIVR(void)
{
  return (uint32_t)(READ_BIT(RCC->CCIPR2, RCC_CCIPR2_PLLSAI2DIVR));
}
#endif /* RCC_CCIPR2_PLLSAI2DIVR */

/**
  * @brief  Enable PLLSAI2 output mapped on SAI domain clock
  * @rmtoll PLLSAI2CFGR  PLLSAI2PEN    LL_RCC_PLLSAI2_EnableDomain_SAI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_EnableDomain_SAI(void)
{
  SET_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2PEN);
}

/**
  * @brief  Disable PLLSAI2 output mapped on SAI domain clock
  * @note In order to save power, when  of the PLLSAI2 is
  *       not used,  should be 0
  * @rmtoll PLLSAI2CFGR  PLLSAI2PEN    LL_RCC_PLLSAI2_DisableDomain_SAI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_DisableDomain_SAI(void)
{
  CLEAR_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2PEN);
}

#if defined(DSI)
/**
  * @brief  Enable PLLSAI2 output mapped on DSI domain clock
  * @rmtoll PLLSAI2CFGR  PLLSAI2QEN    LL_RCC_PLLSAI2_EnableDomain_DSI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_EnableDomain_DSI(void)
{
  SET_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2QEN);
}

/**
  * @brief  Disable PLLSAI2 output mapped on DSI domain clock
  * @note In order to save power, when  of the PLLSAI2 is
  *       not used, Main PLLSAI2  should be 0
  * @rmtoll PLLSAI2CFGR  PLLSAI2QEN    LL_RCC_PLLSAI2_DisableDomain_DSI
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_DisableDomain_DSI(void)
{
  CLEAR_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2QEN);
}
#endif /* DSI */

#if defined(LTDC)
/**
  * @brief  Enable PLLSAI2 output mapped on LTDC domain clock
  * @rmtoll PLLSAI2CFGR  PLLSAI2REN    LL_RCC_PLLSAI2_EnableDomain_LTDC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_EnableDomain_LTDC(void)
{
  SET_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2REN);
}

/**
  * @brief  Disable PLLSAI2 output mapped on LTDC domain clock
  * @note In order to save power, when  of the PLLSAI2 is
  *       not used, Main PLLSAI2  should be 0
  * @rmtoll PLLSAI2CFGR  PLLSAI2REN    LL_RCC_PLLSAI2_DisableDomain_LTDC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_DisableDomain_LTDC(void)
{
  CLEAR_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2REN);
}
#else
/**
  * @brief  Enable PLLSAI2 output mapped on ADC domain clock
  * @rmtoll PLLSAI2CFGR  PLLSAI2REN    LL_RCC_PLLSAI2_EnableDomain_ADC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_EnableDomain_ADC(void)
{
  SET_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2REN);
}

/**
  * @brief  Disable PLLSAI2 output mapped on ADC domain clock
  * @note In order to save power, when  of the PLLSAI2 is
  *       not used, Main PLLSAI2  should be 0
  * @rmtoll PLLSAI2CFGR  PLLSAI2REN    LL_RCC_PLLSAI2_DisableDomain_ADC
  * @retval None
  */
__STATIC_INLINE void LL_RCC_PLLSAI2_DisableDomain_ADC(void)
{
  CLEAR_BIT(RCC->PLLSAI2CFGR, RCC_PLLSAI2CFGR_PLLSAI2REN);
}
#endif /* LTDC */

/**
  * @}
  */
#endif /* RCC_PLLSAI2_SUPPORT */



/** @defgroup RCC_LL_EF_FLAG_Management FLAG Management
  * @{
  */

/**
  * @brief  Clear LSI ready interrupt flag
  * @rmtoll CICR         LSIRDYC       LL_RCC_ClearFlag_LSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_LSIRDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_LSIRDYC);
}

/**
  * @brief  Clear LSE ready interrupt flag
  * @rmtoll CICR         LSERDYC       LL_RCC_ClearFlag_LSERDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_LSERDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_LSERDYC);
}

/**
  * @brief  Clear MSI ready interrupt flag
  * @rmtoll CICR         MSIRDYC       LL_RCC_ClearFlag_MSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_MSIRDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_MSIRDYC);
}

/**
  * @brief  Clear HSI ready interrupt flag
  * @rmtoll CICR         HSIRDYC       LL_RCC_ClearFlag_HSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_HSIRDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_HSIRDYC);
}

/**
  * @brief  Clear HSE ready interrupt flag
  * @rmtoll CICR         HSERDYC       LL_RCC_ClearFlag_HSERDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_HSERDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_HSERDYC);
}

/**
  * @brief  Clear PLL ready interrupt flag
  * @rmtoll CICR         PLLRDYC       LL_RCC_ClearFlag_PLLRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_PLLRDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_PLLRDYC);
}

#if defined(RCC_HSI48_SUPPORT)
/**
  * @brief  Clear HSI48 ready interrupt flag
  * @rmtoll CICR          HSI48RDYC     LL_RCC_ClearFlag_HSI48RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_HSI48RDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_HSI48RDYC);
}
#endif /* RCC_HSI48_SUPPORT */

#if defined(RCC_PLLSAI1_SUPPORT)
/**
  * @brief  Clear PLLSAI1 ready interrupt flag
  * @rmtoll CICR         PLLSAI1RDYC   LL_RCC_ClearFlag_PLLSAI1RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_PLLSAI1RDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_PLLSAI1RDYC);
}
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
/**
  * @brief  Clear PLLSAI1 ready interrupt flag
  * @rmtoll CICR         PLLSAI2RDYC   LL_RCC_ClearFlag_PLLSAI2RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_PLLSAI2RDY(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_PLLSAI2RDYC);
}
#endif /* RCC_PLLSAI2_SUPPORT */

/**
  * @brief  Clear Clock security system interrupt flag
  * @rmtoll CICR         CSSC          LL_RCC_ClearFlag_HSECSS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_HSECSS(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_CSSC);
}

/**
  * @brief  Clear LSE Clock security system interrupt flag
  * @rmtoll CICR         LSECSSC       LL_RCC_ClearFlag_LSECSS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearFlag_LSECSS(void)
{
  SET_BIT(RCC->CICR, RCC_CICR_LSECSSC);
}

/**
  * @brief  Check if LSI ready interrupt occurred or not
  * @rmtoll CIFR         LSIRDYF       LL_RCC_IsActiveFlag_LSIRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_LSIRDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_LSIRDYF) == RCC_CIFR_LSIRDYF) ? 1UL : 0UL);
}

/**
  * @brief  Check if LSE ready interrupt occurred or not
  * @rmtoll CIFR         LSERDYF       LL_RCC_IsActiveFlag_LSERDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_LSERDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_LSERDYF) == RCC_CIFR_LSERDYF) ? 1UL : 0UL);
}

/**
  * @brief  Check if MSI ready interrupt occurred or not
  * @rmtoll CIFR         MSIRDYF       LL_RCC_IsActiveFlag_MSIRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_MSIRDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_MSIRDYF) == RCC_CIFR_MSIRDYF) ? 1UL : 0UL);
}

/**
  * @brief  Check if HSI ready interrupt occurred or not
  * @rmtoll CIFR         HSIRDYF       LL_RCC_IsActiveFlag_HSIRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_HSIRDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_HSIRDYF) == RCC_CIFR_HSIRDYF) ? 1UL : 0UL);
}

/**
  * @brief  Check if HSE ready interrupt occurred or not
  * @rmtoll CIFR         HSERDYF       LL_RCC_IsActiveFlag_HSERDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_HSERDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_HSERDYF) == RCC_CIFR_HSERDYF) ? 1UL : 0UL);
}

/**
  * @brief  Check if PLL ready interrupt occurred or not
  * @rmtoll CIFR         PLLRDYF       LL_RCC_IsActiveFlag_PLLRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_PLLRDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_PLLRDYF) == RCC_CIFR_PLLRDYF) ? 1UL : 0UL);
}

#if defined(RCC_HSI48_SUPPORT)
/**
  * @brief  Check if HSI48 ready interrupt occurred or not
  * @rmtoll CIR          HSI48RDYF     LL_RCC_IsActiveFlag_HSI48RDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_HSI48RDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_HSI48RDYF) == RCC_CIFR_HSI48RDYF) ? 1UL : 0UL);
}
#endif /* RCC_HSI48_SUPPORT */

#if defined(RCC_PLLSAI1_SUPPORT)
/**
  * @brief  Check if PLLSAI1 ready interrupt occurred or not
  * @rmtoll CIFR         PLLSAI1RDYF   LL_RCC_IsActiveFlag_PLLSAI1RDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_PLLSAI1RDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_PLLSAI1RDYF) == RCC_CIFR_PLLSAI1RDYF) ? 1UL : 0UL);
}
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
/**
  * @brief  Check if PLLSAI1 ready interrupt occurred or not
  * @rmtoll CIFR         PLLSAI2RDYF   LL_RCC_IsActiveFlag_PLLSAI2RDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_PLLSAI2RDY(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_PLLSAI2RDYF) == RCC_CIFR_PLLSAI2RDYF) ? 1UL : 0UL);
}
#endif /* RCC_PLLSAI2_SUPPORT */

/**
  * @brief  Check if Clock security system interrupt occurred or not
  * @rmtoll CIFR         CSSF          LL_RCC_IsActiveFlag_HSECSS
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_HSECSS(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_CSSF) == RCC_CIFR_CSSF) ? 1UL : 0UL);
}

/**
  * @brief  Check if LSE Clock security system interrupt occurred or not
  * @rmtoll CIFR         LSECSSF       LL_RCC_IsActiveFlag_LSECSS
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_LSECSS(void)
{
  return ((READ_BIT(RCC->CIFR, RCC_CIFR_LSECSSF) == RCC_CIFR_LSECSSF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag FW reset is set or not.
  * @rmtoll CSR          FWRSTF        LL_RCC_IsActiveFlag_FWRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_FWRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_FWRSTF) == RCC_CSR_FWRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag Independent Watchdog reset is set or not.
  * @rmtoll CSR          IWDGRSTF      LL_RCC_IsActiveFlag_IWDGRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_IWDGRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_IWDGRSTF) == RCC_CSR_IWDGRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag Low Power reset is set or not.
  * @rmtoll CSR          LPWRRSTF      LL_RCC_IsActiveFlag_LPWRRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_LPWRRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_LPWRRSTF) == RCC_CSR_LPWRRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag is set or not.
  * @rmtoll CSR          OBLRSTF       LL_RCC_IsActiveFlag_OBLRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_OBLRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_OBLRSTF) == RCC_CSR_OBLRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag Pin reset is set or not.
  * @rmtoll CSR          PINRSTF       LL_RCC_IsActiveFlag_PINRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_PINRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_PINRSTF) == RCC_CSR_PINRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag Software reset is set or not.
  * @rmtoll CSR          SFTRSTF       LL_RCC_IsActiveFlag_SFTRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_SFTRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_SFTRSTF) == RCC_CSR_SFTRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag Window Watchdog reset is set or not.
  * @rmtoll CSR          WWDGRSTF      LL_RCC_IsActiveFlag_WWDGRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_WWDGRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_WWDGRSTF) == RCC_CSR_WWDGRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Check if RCC flag BOR reset is set or not.
  * @rmtoll CSR          BORRSTF       LL_RCC_IsActiveFlag_BORRST
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsActiveFlag_BORRST(void)
{
  return ((READ_BIT(RCC->CSR, RCC_CSR_BORRSTF) == RCC_CSR_BORRSTF) ? 1UL : 0UL);
}

/**
  * @brief  Set RMVF bit to clear the reset flags.
  * @rmtoll CSR          RMVF          LL_RCC_ClearResetFlags
  * @retval None
  */
__STATIC_INLINE void LL_RCC_ClearResetFlags(void)
{
  SET_BIT(RCC->CSR, RCC_CSR_RMVF);
}

/**
  * @}
  */

/** @defgroup RCC_LL_EF_IT_Management IT Management
  * @{
  */

/**
  * @brief  Enable LSI ready interrupt
  * @rmtoll CIER         LSIRDYIE      LL_RCC_EnableIT_LSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_LSIRDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_LSIRDYIE);
}

/**
  * @brief  Enable LSE ready interrupt
  * @rmtoll CIER         LSERDYIE      LL_RCC_EnableIT_LSERDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_LSERDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_LSERDYIE);
}

/**
  * @brief  Enable MSI ready interrupt
  * @rmtoll CIER         MSIRDYIE      LL_RCC_EnableIT_MSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_MSIRDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_MSIRDYIE);
}

/**
  * @brief  Enable HSI ready interrupt
  * @rmtoll CIER         HSIRDYIE      LL_RCC_EnableIT_HSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_HSIRDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_HSIRDYIE);
}

/**
  * @brief  Enable HSE ready interrupt
  * @rmtoll CIER         HSERDYIE      LL_RCC_EnableIT_HSERDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_HSERDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_HSERDYIE);
}

/**
  * @brief  Enable PLL ready interrupt
  * @rmtoll CIER         PLLRDYIE      LL_RCC_EnableIT_PLLRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_PLLRDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_PLLRDYIE);
}

#if defined(RCC_HSI48_SUPPORT)
/**
  * @brief  Enable HSI48 ready interrupt
  * @rmtoll CIER          HSI48RDYIE    LL_RCC_EnableIT_HSI48RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_HSI48RDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_HSI48RDYIE);
}
#endif /* RCC_HSI48_SUPPORT */

#if defined(RCC_PLLSAI1_SUPPORT)
/**
  * @brief  Enable PLLSAI1 ready interrupt
  * @rmtoll CIER         PLLSAI1RDYIE  LL_RCC_EnableIT_PLLSAI1RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_PLLSAI1RDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_PLLSAI1RDYIE);
}
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
/**
  * @brief  Enable PLLSAI2 ready interrupt
  * @rmtoll CIER         PLLSAI2RDYIE  LL_RCC_EnableIT_PLLSAI2RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_PLLSAI2RDY(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_PLLSAI2RDYIE);
}
#endif /* RCC_PLLSAI2_SUPPORT */

/**
  * @brief  Enable LSE clock security system interrupt
  * @rmtoll CIER         LSECSSIE      LL_RCC_EnableIT_LSECSS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_EnableIT_LSECSS(void)
{
  SET_BIT(RCC->CIER, RCC_CIER_LSECSSIE);
}

/**
  * @brief  Disable LSI ready interrupt
  * @rmtoll CIER         LSIRDYIE      LL_RCC_DisableIT_LSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_LSIRDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_LSIRDYIE);
}

/**
  * @brief  Disable LSE ready interrupt
  * @rmtoll CIER         LSERDYIE      LL_RCC_DisableIT_LSERDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_LSERDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_LSERDYIE);
}

/**
  * @brief  Disable MSI ready interrupt
  * @rmtoll CIER         MSIRDYIE      LL_RCC_DisableIT_MSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_MSIRDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_MSIRDYIE);
}

/**
  * @brief  Disable HSI ready interrupt
  * @rmtoll CIER         HSIRDYIE      LL_RCC_DisableIT_HSIRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_HSIRDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_HSIRDYIE);
}

/**
  * @brief  Disable HSE ready interrupt
  * @rmtoll CIER         HSERDYIE      LL_RCC_DisableIT_HSERDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_HSERDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_HSERDYIE);
}

/**
  * @brief  Disable PLL ready interrupt
  * @rmtoll CIER         PLLRDYIE      LL_RCC_DisableIT_PLLRDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_PLLRDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_PLLRDYIE);
}

#if defined(RCC_HSI48_SUPPORT)
/**
  * @brief  Disable HSI48 ready interrupt
  * @rmtoll CIER          HSI48RDYIE    LL_RCC_DisableIT_HSI48RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_HSI48RDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_HSI48RDYIE);
}
#endif /* RCC_HSI48_SUPPORT */

#if defined(RCC_PLLSAI1_SUPPORT)
/**
  * @brief  Disable PLLSAI1 ready interrupt
  * @rmtoll CIER         PLLSAI1RDYIE  LL_RCC_DisableIT_PLLSAI1RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_PLLSAI1RDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_PLLSAI1RDYIE);
}
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
/**
  * @brief  Disable PLLSAI2 ready interrupt
  * @rmtoll CIER         PLLSAI2RDYIE  LL_RCC_DisableIT_PLLSAI2RDY
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_PLLSAI2RDY(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_PLLSAI2RDYIE);
}
#endif /* RCC_PLLSAI2_SUPPORT */

/**
  * @brief  Disable LSE clock security system interrupt
  * @rmtoll CIER         LSECSSIE      LL_RCC_DisableIT_LSECSS
  * @retval None
  */
__STATIC_INLINE void LL_RCC_DisableIT_LSECSS(void)
{
  CLEAR_BIT(RCC->CIER, RCC_CIER_LSECSSIE);
}

/**
  * @brief  Checks if LSI ready interrupt source is enabled or disabled.
  * @rmtoll CIER         LSIRDYIE      LL_RCC_IsEnabledIT_LSIRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_LSIRDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_LSIRDYIE) == RCC_CIER_LSIRDYIE) ? 1UL : 0UL);
}

/**
  * @brief  Checks if LSE ready interrupt source is enabled or disabled.
  * @rmtoll CIER         LSERDYIE      LL_RCC_IsEnabledIT_LSERDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_LSERDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_LSERDYIE) == RCC_CIER_LSERDYIE) ? 1UL : 0UL);
}

/**
  * @brief  Checks if MSI ready interrupt source is enabled or disabled.
  * @rmtoll CIER         MSIRDYIE      LL_RCC_IsEnabledIT_MSIRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_MSIRDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_MSIRDYIE) == RCC_CIER_MSIRDYIE) ? 1UL : 0UL);
}

/**
  * @brief  Checks if HSI ready interrupt source is enabled or disabled.
  * @rmtoll CIER         HSIRDYIE      LL_RCC_IsEnabledIT_HSIRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_HSIRDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_HSIRDYIE) == RCC_CIER_HSIRDYIE) ? 1UL : 0UL);
}

/**
  * @brief  Checks if HSE ready interrupt source is enabled or disabled.
  * @rmtoll CIER         HSERDYIE      LL_RCC_IsEnabledIT_HSERDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_HSERDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_HSERDYIE) == RCC_CIER_HSERDYIE) ? 1UL : 0UL);
}

/**
  * @brief  Checks if PLL ready interrupt source is enabled or disabled.
  * @rmtoll CIER         PLLRDYIE      LL_RCC_IsEnabledIT_PLLRDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_PLLRDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_PLLRDYIE) == RCC_CIER_PLLRDYIE) ? 1UL : 0UL);
}

#if defined(RCC_HSI48_SUPPORT)
/**
  * @brief  Checks if HSI48 ready interrupt source is enabled or disabled.
  * @rmtoll CIER          HSI48RDYIE    LL_RCC_IsEnabledIT_HSI48RDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_HSI48RDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_HSI48RDYIE) == RCC_CIER_HSI48RDYIE) ? 1UL : 0UL);
}
#endif /* RCC_HSI48_SUPPORT */

#if defined(RCC_PLLSAI1_SUPPORT)
/**
  * @brief  Checks if PLLSAI1 ready interrupt source is enabled or disabled.
  * @rmtoll CIER         PLLSAI1RDYIE  LL_RCC_IsEnabledIT_PLLSAI1RDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_PLLSAI1RDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_PLLSAI1RDYIE) == RCC_CIER_PLLSAI1RDYIE) ? 1UL : 0UL);
}
#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)
/**
  * @brief  Checks if PLLSAI2 ready interrupt source is enabled or disabled.
  * @rmtoll CIER         PLLSAI2RDYIE  LL_RCC_IsEnabledIT_PLLSAI2RDY
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_PLLSAI2RDY(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_PLLSAI2RDYIE) == RCC_CIER_PLLSAI2RDYIE) ? 1UL : 0UL);
}
#endif /* RCC_PLLSAI2_SUPPORT */

/**
  * @brief  Checks if LSECSS interrupt source is enabled or disabled.
  * @rmtoll CIER         LSECSSIE      LL_RCC_IsEnabledIT_LSECSS
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_RCC_IsEnabledIT_LSECSS(void)
{
  return ((READ_BIT(RCC->CIER, RCC_CIER_LSECSSIE) == RCC_CIER_LSECSSIE) ? 1UL : 0UL);
}

/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup RCC_LL_EF_Init De-initialization function
  * @{
  */
ErrorStatus LL_RCC_DeInit(void);
/**
  * @}
  */

/** @defgroup RCC_LL_EF_Get_Freq Get system and peripherals clocks frequency functions
  * @{
  */
void        LL_RCC_GetSystemClocksFreq(LL_RCC_ClocksTypeDef *RCC_Clocks);
uint32_t    LL_RCC_GetUSARTClockFreq(uint32_t USARTxSource);
#if defined(UART4) || defined(UART5)
uint32_t    LL_RCC_GetUARTClockFreq(uint32_t UARTxSource);
#endif /* UART4 || UART5 */
uint32_t    LL_RCC_GetI2CClockFreq(uint32_t I2CxSource);
uint32_t    LL_RCC_GetLPUARTClockFreq(uint32_t LPUARTxSource);
uint32_t    LL_RCC_GetLPTIMClockFreq(uint32_t LPTIMxSource);
#if defined(SAI1)
uint32_t    LL_RCC_GetSAIClockFreq(uint32_t SAIxSource);
#endif /* SAI1 */
#if defined(SDMMC1)
#if defined(RCC_CCIPR2_SDMMCSEL)
uint32_t    LL_RCC_GetSDMMCKernelClockFreq(uint32_t SDMMCxSource);
#endif
uint32_t    LL_RCC_GetSDMMCClockFreq(uint32_t SDMMCxSource);
#endif /* SDMMC1 */
uint32_t    LL_RCC_GetRNGClockFreq(uint32_t RNGxSource);
#if defined(USB_OTG_FS) || defined(USB)
uint32_t    LL_RCC_GetUSBClockFreq(uint32_t USBxSource);
#endif /* USB_OTG_FS || USB */
uint32_t    LL_RCC_GetADCClockFreq(uint32_t ADCxSource);
#if defined(SWPMI1)
uint32_t    LL_RCC_GetSWPMIClockFreq(uint32_t SWPMIxSource);
#endif /* SWPMI1 */
#if defined(DFSDM1_Channel0)
uint32_t    LL_RCC_GetDFSDMClockFreq(uint32_t DFSDMxSource);
#if defined(RCC_CCIPR2_DFSDM1SEL)
uint32_t    LL_RCC_GetDFSDMAudioClockFreq(uint32_t DFSDMxSource);
#endif /* RCC_CCIPR2_DFSDM1SEL */
#endif /* DFSDM1_Channel0 */
#if defined(LTDC)
uint32_t    LL_RCC_GetLTDCClockFreq(uint32_t LTDCxSource);
#endif /* LTDC */
#if defined(DSI)
uint32_t    LL_RCC_GetDSIClockFreq(uint32_t DSIxSource);
#endif /* DSI */
#if defined(OCTOSPI1)
uint32_t    LL_RCC_GetOCTOSPIClockFreq(uint32_t OCTOSPIxSource);
#endif /* OCTOSPI1 */
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

#endif /* defined(RCC) */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_LL_RCC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
