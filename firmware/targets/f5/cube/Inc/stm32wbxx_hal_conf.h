/**
  ******************************************************************************
  * @file    stm32wbxx_hal_conf.h
  * @author  MCD Application Team
  * @brief   HAL configuration file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#ifndef __STM32WBxx_HAL_CONF_H
#define __STM32WBxx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
/**
  * @brief This is the list of modules to be used in the HAL driver
  */
#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CRYP_MODULE_ENABLED
#define HAL_COMP_MODULE_ENABLED
#define HAL_CRC_MODULE_ENABLED
#define HAL_HSEM_MODULE_ENABLED
/*#define HAL_I2C_MODULE_ENABLED   */
/*#define HAL_IPCC_MODULE_ENABLED   */
/*#define HAL_IRDA_MODULE_ENABLED   */
/*#define HAL_IWDG_MODULE_ENABLED   */
/*#define HAL_LCD_MODULE_ENABLED   */
/*#define HAL_LPTIM_MODULE_ENABLED   */
#define HAL_PCD_MODULE_ENABLED
#define HAL_PKA_MODULE_ENABLED
/*#define HAL_QSPI_MODULE_ENABLED   */
#define HAL_RNG_MODULE_ENABLED
#define HAL_RTC_MODULE_ENABLED
/*#define HAL_SAI_MODULE_ENABLED   */
/*#define HAL_SMBUS_MODULE_ENABLED   */
/*#define HAL_SMARTCARD_MODULE_ENABLED   */
#define HAL_SPI_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
/*#define HAL_TSC_MODULE_ENABLED   */
#define HAL_UART_MODULE_ENABLED
/*#define HAL_USART_MODULE_ENABLED   */
/*#define HAL_WWDG_MODULE_ENABLED   */
#define HAL_EXTI_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED

#define USE_HAL_ADC_REGISTER_CALLBACKS       0u
#define USE_HAL_COMP_REGISTER_CALLBACKS      0u
#define USE_HAL_CRYP_REGISTER_CALLBACKS      0u
#define USE_HAL_I2C_REGISTER_CALLBACKS       0u
#define USE_HAL_IRDA_REGISTER_CALLBACKS      0u
#define USE_HAL_LPTIM_REGISTER_CALLBACKS     0u
#define USE_HAL_PCD_REGISTER_CALLBACKS       0u
#define USE_HAL_PKA_REGISTER_CALLBACKS       0u
#define USE_HAL_QSPI_REGISTER_CALLBACKS      0u
#define USE_HAL_RNG_REGISTER_CALLBACKS       0u
#define USE_HAL_RTC_REGISTER_CALLBACKS       0u
#define USE_HAL_SAI_REGISTER_CALLBACKS       0u
#define USE_HAL_SMARTCARD_REGISTER_CALLBACKS 0u
#define USE_HAL_SMBUS_REGISTER_CALLBACKS     0u
#define USE_HAL_SPI_REGISTER_CALLBACKS       0u
#define USE_HAL_TIM_REGISTER_CALLBACKS       0u
#define USE_HAL_TSC_REGISTER_CALLBACKS       0u
#define USE_HAL_UART_REGISTER_CALLBACKS      0u
#define USE_HAL_USART_REGISTER_CALLBACKS     0u
#define USE_HAL_WWDG_REGISTER_CALLBACKS      0u

/* ########################## Oscillator Values adaptation ####################*/
/**
  * @brief Adjust the value of External High Speed oscillator (HSE) used in your application.
  *        This value is used by the RCC HAL module to compute the system frequency
  *        (when HSE is used as system clock source, directly or through the PLL).
  */
#if !defined  (HSE_VALUE)
#define HSE_VALUE    32000000U             /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    ((uint32_t)100)   /*!< Time out for HSE start up, in ms */
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief Internal Multiple Speed oscillator (MSI) default value.
  *        This value is the default MSI range value after Reset.
  */
#if !defined  (MSI_VALUE)
  #define MSI_VALUE    ((uint32_t)4000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* MSI_VALUE */

/**
  * @brief Internal High Speed oscillator (HSI) value.
  *        This value is used by the RCC HAL module to compute the system frequency
  *        (when HSI is used as system clock source, directly or through the PLL).
  */
#if !defined  (HSI_VALUE)
#define HSI_VALUE    16000000U            /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

/**
  * @brief Internal Low Speed oscillator (LSI1) value.
  */
#if !defined  (LSI1_VALUE)
 #define LSI1_VALUE  ((uint32_t)32000)       /*!< LSI1 Typical Value in Hz*/
#endif /* LSI1_VALUE */                      /*!< Value of the Internal Low Speed oscillator in Hz
                                             The real value may vary depending on the variations
                                             in voltage and temperature.*/
/**
  * @brief Internal Low Speed oscillator (LSI2) value.
  */
#if !defined  (LSI2_VALUE)
 #define LSI2_VALUE  ((uint32_t)32000)       /*!< LSI2 Typical Value in Hz*/
#endif /* LSI2_VALUE */                      /*!< Value of the Internal Low Speed oscillator in Hz
                                             The real value may vary depending on the variations
                                             in voltage and temperature.*/

/**
  * @brief External Low Speed oscillator (LSE) value.
  *        This value is used by the UART, RTC HAL module to compute the system frequency
  */
#if !defined  (LSE_VALUE)
#define LSE_VALUE    32768U               /*!< Value of the External oscillator in Hz*/
#endif /* LSE_VALUE */

/**
  * @brief Internal Multiple Speed oscillator (HSI48) default value.
  *        This value is the default HSI48 range value after Reset.
  */
#if !defined (HSI48_VALUE)
  #define HSI48_VALUE    ((uint32_t)48000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI48_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT    1000U      /*!< Time out for LSE start up, in ms */
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief External clock source for SAI1 peripheral
  *        This value is used by the RCC HAL module to compute the SAI1 & SAI2 clock source
  *        frequency.
  */
#if !defined (EXTERNAL_SAI1_CLOCK_VALUE)
  #define EXTERNAL_SAI1_CLOCK_VALUE    ((uint32_t)2097000) /*!< Value of the SAI1 External clock source in Hz*/
#endif /* EXTERNAL_SAI1_CLOCK_VALUE */

/* Tip: To avoid modifying this file each time you need to use different HSE,
   ===  you can define the HSE value in your toolchain compiler preprocessor. */

/* ########################### System Configuration ######################### */
/**
  * @brief This is the HAL system configuration section
  */

#define  VDD_VALUE				3300U                   /*!< Value of VDD in mv */
#define  TICK_INT_PRIORITY            15U      /*!< tick interrupt priority */
#define  USE_RTOS                     0U
#define  PREFETCH_ENABLE              1U
#define  INSTRUCTION_CACHE_ENABLE     1U
#define  DATA_CACHE_ENABLE            1U

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* ################## SPI peripheral configuration ########################## */

/* CRC FEATURE: Use to activate CRC feature inside HAL SPI Driver
 * Activated: CRC code is present inside driver
 * Deactivated: CRC code cleaned from driver
 */

#define USE_SPI_CRC                   0U

/* Includes ------------------------------------------------------------------*/
/**
  * @brief Include module's header file
  */
#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32wbxx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32wbxx_hal_adc.h"
#endif /* HAL_ADC_MODULE_ENABLED */

#ifdef HAL_COMP_MODULE_ENABLED
  #include "stm32wbxx_hal_comp.h"
#endif /* HAL_COMP_MODULE_ENABLED */

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32wbxx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef HAL_CRC_MODULE_ENABLED
  #include "stm32wbxx_hal_crc.h"
#endif /* HAL_CRC_MODULE_ENABLED */

#ifdef HAL_CRYP_MODULE_ENABLED
  #include "stm32wbxx_hal_cryp.h"
#endif /* HAL_CRYP_MODULE_ENABLED */

#ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32wbxx_hal_exti.h"
#endif /* HAL_EXTI_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32wbxx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32wbxx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_HSEM_MODULE_ENABLED
  #include "stm32wbxx_hal_hsem.h"
#endif /* HAL_HSEM_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
 #include "stm32wbxx_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_IPCC_MODULE_ENABLED
 #include "stm32wbxx_hal_ipcc.h"
#endif /* HAL_IPCC_MODULE_ENABLED */

#ifdef HAL_IRDA_MODULE_ENABLED
 #include "stm32wbxx_hal_irda.h"
#endif /* HAL_IRDA_MODULE_ENABLED */

#ifdef HAL_IWDG_MODULE_ENABLED
 #include "stm32wbxx_hal_iwdg.h"
#endif /* HAL_IWDG_MODULE_ENABLED */

#ifdef HAL_LCD_MODULE_ENABLED
 #include "stm32wbxx_hal_lcd.h"
#endif /* HAL_LCD_MODULE_ENABLED */

#ifdef HAL_LPTIM_MODULE_ENABLED
  #include "stm32wbxx_hal_lptim.h"
#endif /* HAL_LPTIM_MODULE_ENABLED */

#ifdef HAL_PCD_MODULE_ENABLED
  #include "stm32wbxx_hal_pcd.h"
#endif /* HAL_PCD_MODULE_ENABLED */

#ifdef HAL_PKA_MODULE_ENABLED
  #include "stm32wbxx_hal_pka.h"
#endif /* HAL_PKA_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
 #include "stm32wbxx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

#ifdef HAL_QSPI_MODULE_ENABLED
 #include "stm32wbxx_hal_qspi.h"
#endif /* HAL_QSPI_MODULE_ENABLED */

#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32wbxx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_RNG_MODULE_ENABLED
  #include "stm32wbxx_hal_rng.h"
#endif /* HAL_RNG_MODULE_ENABLED */

#ifdef HAL_RTC_MODULE_ENABLED
 #include "stm32wbxx_hal_rtc.h"
#endif /* HAL_RTC_MODULE_ENABLED */

#ifdef HAL_SAI_MODULE_ENABLED
 #include "stm32wbxx_hal_sai.h"
#endif /* HAL_SAI_MODULE_ENABLED */

#ifdef HAL_SMARTCARD_MODULE_ENABLED
 #include "stm32wbxx_hal_smartcard.h"
#endif /* HAL_SMARTCARD_MODULE_ENABLED */

#ifdef HAL_SMBUS_MODULE_ENABLED
 #include "stm32wbxx_hal_smbus.h"
#endif /* HAL_SMBUS_MODULE_ENABLED */

#ifdef HAL_SPI_MODULE_ENABLED
 #include "stm32wbxx_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_TIM_MODULE_ENABLED
 #include "stm32wbxx_hal_tim.h"
#endif /* HAL_TIM_MODULE_ENABLED */

#ifdef HAL_TSC_MODULE_ENABLED
  #include "stm32wbxx_hal_tsc.h"
#endif /* HAL_TSC_MODULE_ENABLED */

#ifdef HAL_UART_MODULE_ENABLED
 #include "stm32wbxx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

#ifdef HAL_USART_MODULE_ENABLED
 #include "stm32wbxx_hal_usart.h"
#endif /* HAL_USART_MODULE_ENABLED */

#ifdef HAL_WWDG_MODULE_ENABLED
 #include "stm32wbxx_hal_wwdg.h"
#endif /* HAL_WWDG_MODULE_ENABLED */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param expr If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __STM32WBxx_HAL_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
