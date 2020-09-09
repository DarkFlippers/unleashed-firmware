/**
  ******************************************************************************
  * @file    stm32l4xx_hal_gpio_ex.h
  * @author  MCD Application Team
  * @brief   Header file of GPIO HAL Extended module.
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
#ifndef __STM32L4xx_HAL_GPIO_EX_H
#define __STM32L4xx_HAL_GPIO_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup GPIOEx GPIOEx
  * @brief GPIO Extended HAL module driver
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup GPIOEx_Exported_Constants GPIOEx Exported Constants
  * @{
  */

/** @defgroup GPIOEx_Alternate_function_selection GPIOEx Alternate function selection
  * @{
  */

#if defined(STM32L412xx) || defined(STM32L422xx)
/*--------------STM32L412xx/STM32L422xx---*/
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_RTC_50Hz      ((uint8_t)0x00)  /* RTC_50Hz Alternate Function mapping                       */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /* MCO (MCO1 and MCO2) Alternate Function mapping            */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping             */
#define GPIO_AF0_TRACE         ((uint8_t)0x00)  /* TRACE Alternate Function mapping                          */

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_TIM1          ((uint8_t)0x01)  /* TIM1 Alternate Function mapping */
#define GPIO_AF1_TIM2          ((uint8_t)0x01)  /* TIM2 Alternate Function mapping */
#define GPIO_AF1_LPTIM1        ((uint8_t)0x01)  /* LPTIM1 Alternate Function mapping */
#define GPIO_AF1_IR            ((uint8_t)0x01)  /* IR Alternate Function mapping */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_TIM1          ((uint8_t)0x02)  /* TIM1 Alternate Function mapping */
#define GPIO_AF2_TIM2          ((uint8_t)0x02)  /* TIM2 Alternate Function mapping */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_USART2        ((uint8_t)0x03)  /* USART1 Alternate Function mapping */
#define GPIO_AF3_TIM1_COMP1    ((uint8_t)0x03)  /* TIM1/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 4 selection
  */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping   */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping   */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping   */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping   */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping   */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_COMP1         ((uint8_t)0x06)  /* COMP1 Alternate Function mapping   */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_USART1        ((uint8_t)0x07)  /* USART1 Alternate Function mapping     */
#define GPIO_AF7_USART2        ((uint8_t)0x07)  /* USART2 Alternate Function mapping     */
#define GPIO_AF7_USART3        ((uint8_t)0x07)  /* USART3 Alternate Function mapping     */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_LPUART1       ((uint8_t)0x08)  /* LPUART1 Alternate Function mapping */

/**
  * @brief   AF 9 selection
  */
#define GPIO_AF9_TSC           ((uint8_t)0x09)  /* TSC Alternate Function mapping   */

/**
  * @brief   AF 10 selection
  */
#define GPIO_AF10_USB_FS       ((uint8_t)0x0A)  /* USB_FS Alternate Function mapping */
#define GPIO_AF10_QUADSPI      ((uint8_t)0x0A)  /* QUADSPI Alternate Function mapping */

/**
  * @brief   AF 12 selection
  */
#define GPIO_AF12_COMP1        ((uint8_t)0x0C)  /* COMP1 Alternate Function mapping   */


/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_TIM2         ((uint8_t)0x0E)  /* TIM2 Alternate Function mapping */
#define GPIO_AF14_TIM15        ((uint8_t)0x0E)  /* TIM15 Alternate Function mapping */
#define GPIO_AF14_TIM16        ((uint8_t)0x0E)  /* TIM16 Alternate Function mapping */
#define GPIO_AF14_LPTIM2       ((uint8_t)0x0E)  /* LPTIM2 Alternate Function mapping */

/**
  * @brief   AF 15 selection
  */
#define GPIO_AF15_EVENTOUT     ((uint8_t)0x0F)  /* EVENTOUT Alternate Function mapping */

#define IS_GPIO_AF(AF)   ((AF) <= (uint8_t)0x0F)

#endif /* STM32L412xx || STM32L422xx */

#if defined(STM32L431xx) || defined(STM32L432xx) || defined(STM32L433xx) || defined(STM32L442xx) || defined(STM32L443xx)
/*--------------STM32L431xx/STM32L432xx/STM32L433xx/STM32L442xx/STM32L443xx---*/
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_RTC_50Hz      ((uint8_t)0x00)  /* RTC_50Hz Alternate Function mapping                       */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /* MCO (MCO1 and MCO2) Alternate Function mapping            */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping             */
#if defined(STM32L433xx) || defined(STM32L443xx)
#define GPIO_AF0_LCDBIAS       ((uint8_t)0x00)  /* LCDBIAS Alternate Function mapping                          */
#endif /* STM32L433xx || STM32L443xx */
#define GPIO_AF0_TRACE         ((uint8_t)0x00)  /* TRACE Alternate Function mapping                          */

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_TIM1          ((uint8_t)0x01)  /* TIM1 Alternate Function mapping */
#define GPIO_AF1_TIM2          ((uint8_t)0x01)  /* TIM2 Alternate Function mapping */
#define GPIO_AF1_LPTIM1        ((uint8_t)0x01)  /* LPTIM1 Alternate Function mapping */
#define GPIO_AF1_IR            ((uint8_t)0x01)  /* IR Alternate Function mapping */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_TIM1          ((uint8_t)0x02)  /* TIM1 Alternate Function mapping */
#define GPIO_AF2_TIM2          ((uint8_t)0x02)  /* TIM2 Alternate Function mapping */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_USART2        ((uint8_t)0x03)  /* USART1 Alternate Function mapping */
#define GPIO_AF3_TIM1_COMP2    ((uint8_t)0x03)  /* TIM1/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF3_TIM1_COMP1    ((uint8_t)0x03)  /* TIM1/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 4 selection
  */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping   */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping   */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping   */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping   */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping   */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_SPI3          ((uint8_t)0x06)  /* SPI3 Alternate Function mapping   */
#define GPIO_AF6_COMP1         ((uint8_t)0x06)  /* COMP1 Alternate Function mapping   */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_USART1        ((uint8_t)0x07)  /* USART1 Alternate Function mapping     */
#define GPIO_AF7_USART2        ((uint8_t)0x07)  /* USART2 Alternate Function mapping     */
#define GPIO_AF7_USART3        ((uint8_t)0x07)  /* USART3 Alternate Function mapping     */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_LPUART1       ((uint8_t)0x08)  /* LPUART1 Alternate Function mapping */

/**
  * @brief   AF 9 selection
  */
#define GPIO_AF9_CAN1          ((uint8_t)0x09)  /* CAN1 Alternate Function mapping    */
#define GPIO_AF9_TSC           ((uint8_t)0x09)  /* TSC Alternate Function mapping   */

/**
  * @brief   AF 10 selection
  */
#if defined(STM32L432xx) || defined(STM32L433xx) || defined(STM32L442xx) || defined(STM32L443xx)
#define GPIO_AF10_USB_FS       ((uint8_t)0x0A)  /* USB_FS Alternate Function mapping */
#endif /* STM32L432xx || STM32L433xx || STM32L442xx || STM32L443xx */
#define GPIO_AF10_QUADSPI      ((uint8_t)0x0A)  /* QUADSPI Alternate Function mapping */

#if defined(STM32L433xx) || defined(STM32L443xx)
/**
  * @brief   AF 11 selection
  */
#define GPIO_AF11_LCD          ((uint8_t)0x0B)  /* LCD Alternate Function mapping */
#endif /* STM32L433xx || STM32L443xx */

/**
  * @brief   AF 12 selection
  */
#define GPIO_AF12_SWPMI1       ((uint8_t)0x0C)  /* SWPMI1 Alternate Function mapping  */
#define GPIO_AF12_COMP1        ((uint8_t)0x0C)  /* COMP1 Alternate Function mapping   */
#define GPIO_AF12_COMP2        ((uint8_t)0x0C)  /* COMP2 Alternate Function mapping   */
#define GPIO_AF12_SDMMC1       ((uint8_t)0x0C)  /* SDMMC1 Alternate Function mapping  */

/**
  * @brief   AF 13 selection
  */
#define GPIO_AF13_SAI1         ((uint8_t)0x0D)  /* SAI1 Alternate Function mapping */

/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_TIM2         ((uint8_t)0x0E)  /* TIM2 Alternate Function mapping */
#define GPIO_AF14_TIM15        ((uint8_t)0x0E)  /* TIM15 Alternate Function mapping */
#define GPIO_AF14_TIM16        ((uint8_t)0x0E)  /* TIM16 Alternate Function mapping */
#define GPIO_AF14_LPTIM2       ((uint8_t)0x0E)  /* LPTIM2 Alternate Function mapping */

/**
  * @brief   AF 15 selection
  */
#define GPIO_AF15_EVENTOUT     ((uint8_t)0x0F)  /* EVENTOUT Alternate Function mapping */

#define IS_GPIO_AF(AF)   ((AF) <= (uint8_t)0x0F)

#endif /* STM32L431xx || STM32L432xx || STM32L433xx || STM32L442xx || STM32L443xx */

#if defined(STM32L451xx) || defined(STM32L452xx) || defined(STM32L462xx)
/*--------------STM32L451xx/STM32L452xx/STM32L462xx---------------------------*/
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_RTC_50Hz      ((uint8_t)0x00)  /* RTC_50Hz Alternate Function mapping                       */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /* MCO (MCO1 and MCO2) Alternate Function mapping            */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping             */
#define GPIO_AF0_TRACE         ((uint8_t)0x00)  /* TRACE Alternate Function mapping                          */

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_TIM1          ((uint8_t)0x01)  /* TIM1 Alternate Function mapping */
#define GPIO_AF1_TIM2          ((uint8_t)0x01)  /* TIM2 Alternate Function mapping */
#define GPIO_AF1_LPTIM1        ((uint8_t)0x01)  /* LPTIM1 Alternate Function mapping */
#define GPIO_AF1_IR            ((uint8_t)0x01)  /* IR Alternate Function mapping */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_TIM1          ((uint8_t)0x02)  /* TIM1 Alternate Function mapping */
#define GPIO_AF2_TIM2          ((uint8_t)0x02)  /* TIM2 Alternate Function mapping */
#define GPIO_AF2_TIM3          ((uint8_t)0x02)  /* TIM3 Alternate Function mapping */
#define GPIO_AF2_I2C4          ((uint8_t)0x02)  /* I2C4 Alternate Function mapping */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_TIM1_COMP2    ((uint8_t)0x03)  /* TIM1/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF3_TIM1_COMP1    ((uint8_t)0x03)  /* TIM1/COMP1 Break in Alternate Function mapping  */
#define GPIO_AF3_USART2        ((uint8_t)0x03)  /* USART2 Alternate Function mapping     */
#define GPIO_AF3_CAN1          ((uint8_t)0x03)  /* CAN1 Alternate Function mapping  */
#define GPIO_AF3_I2C4          ((uint8_t)0x03)  /* I2C4 Alternate Function mapping */

/**
  * @brief   AF 4 selection
  */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping   */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping   */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping   */
#define GPIO_AF4_I2C4          ((uint8_t)0x04)  /* I2C4 Alternate Function mapping   */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping   */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping   */
#define GPIO_AF5_I2C4          ((uint8_t)0x05)  /* I2C4 Alternate Function mapping   */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_SPI3          ((uint8_t)0x06)  /* SPI3 Alternate Function mapping   */
#define GPIO_AF6_DFSDM1        ((uint8_t)0x06)  /* DFSDM1 Alternate Function mapping */
#define GPIO_AF6_COMP1         ((uint8_t)0x06)  /* COMP1 Alternate Function mapping   */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_USART1        ((uint8_t)0x07)  /* USART1 Alternate Function mapping     */
#define GPIO_AF7_USART2        ((uint8_t)0x07)  /* USART2 Alternate Function mapping     */
#define GPIO_AF7_USART3        ((uint8_t)0x07)  /* USART3 Alternate Function mapping     */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_UART4         ((uint8_t)0x08)  /* UART4 Alternate Function mapping   */
#define GPIO_AF8_LPUART1       ((uint8_t)0x08)  /* LPUART1 Alternate Function mapping */
#define GPIO_AF8_CAN1          ((uint8_t)0x08)  /* CAN1 Alternate Function mapping  */


/**
  * @brief   AF 9 selection
  */
#define GPIO_AF9_CAN1          ((uint8_t)0x09)  /* CAN1 Alternate Function mapping  */
#define GPIO_AF9_TSC           ((uint8_t)0x09)  /* TSC Alternate Function mapping   */

/**
  * @brief   AF 10 selection
  */
#if defined(STM32L452xx) || defined(STM32L462xx)
#define GPIO_AF10_USB_FS       ((uint8_t)0x0A)  /* USB_FS Alternate Function mapping */
#endif /* STM32L452xx || STM32L462xx */
#define GPIO_AF10_QUADSPI      ((uint8_t)0x0A)  /* QUADSPI Alternate Function mapping */
#define GPIO_AF10_CAN1         ((uint8_t)0x0A)  /* CAN1 Alternate Function mapping  */

/**
  * @brief   AF 11 selection
  */

/**
  * @brief   AF 12 selection
  */
#define GPIO_AF12_COMP1        ((uint8_t)0x0C)  /* COMP1 Alternate Function mapping   */
#define GPIO_AF12_COMP2        ((uint8_t)0x0C)  /* COMP2 Alternate Function mapping   */
#define GPIO_AF12_SDMMC1       ((uint8_t)0x0C)  /* SDMMC1 Alternate Function mapping  */

/**
  * @brief   AF 13 selection
  */
#define GPIO_AF13_SAI1         ((uint8_t)0x0D)  /* SAI1 Alternate Function mapping */

/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_TIM2         ((uint8_t)0x0E)  /* TIM2 Alternate Function mapping */
#define GPIO_AF14_TIM15        ((uint8_t)0x0E)  /* TIM15 Alternate Function mapping */
#define GPIO_AF14_TIM16        ((uint8_t)0x0E)  /* TIM16 Alternate Function mapping */
#define GPIO_AF14_TIM17        ((uint8_t)0x0E)  /* TIM17 Alternate Function mapping */
#define GPIO_AF14_LPTIM2       ((uint8_t)0x0E)  /* LPTIM2 Alternate Function mapping */

/**
  * @brief   AF 15 selection
  */
#define GPIO_AF15_EVENTOUT     ((uint8_t)0x0F)  /* EVENTOUT Alternate Function mapping */

#define IS_GPIO_AF(AF)   ((AF) <= (uint8_t)0x0F)

#endif /* STM32L451xx || STM32L452xx || STM32L462xx */

#if defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx)
/*--------------STM32L471xx/STM32L475xx/STM32L476xx/STM32L485xx/STM32L486xx---*/
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_RTC_50Hz      ((uint8_t)0x00)  /* RTC_50Hz Alternate Function mapping                       */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /* MCO (MCO1 and MCO2) Alternate Function mapping            */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping             */
#if defined(STM32L476xx) || defined(STM32L486xx)
#define GPIO_AF0_LCDBIAS       ((uint8_t)0x00)  /* LCDBIAS Alternate Function mapping                        */
#endif /* STM32L476xx || STM32L486xx */
#define GPIO_AF0_TRACE         ((uint8_t)0x00)  /* TRACE Alternate Function mapping                          */

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_TIM1          ((uint8_t)0x01)  /* TIM1 Alternate Function mapping */
#define GPIO_AF1_TIM2          ((uint8_t)0x01)  /* TIM2 Alternate Function mapping */
#define GPIO_AF1_TIM5          ((uint8_t)0x01)  /* TIM5 Alternate Function mapping */
#define GPIO_AF1_TIM8          ((uint8_t)0x01)  /* TIM8 Alternate Function mapping */
#define GPIO_AF1_LPTIM1        ((uint8_t)0x01)  /* LPTIM1 Alternate Function mapping */
#define GPIO_AF1_IR            ((uint8_t)0x01)  /* IR Alternate Function mapping */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_TIM1          ((uint8_t)0x02)  /* TIM1 Alternate Function mapping */
#define GPIO_AF2_TIM2          ((uint8_t)0x02)  /* TIM2 Alternate Function mapping */
#define GPIO_AF2_TIM3          ((uint8_t)0x02)  /* TIM3 Alternate Function mapping */
#define GPIO_AF2_TIM4          ((uint8_t)0x02)  /* TIM4 Alternate Function mapping */
#define GPIO_AF2_TIM5          ((uint8_t)0x02)  /* TIM5 Alternate Function mapping */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_TIM8          ((uint8_t)0x03)  /* TIM8 Alternate Function mapping  */
#define GPIO_AF3_TIM1_COMP2    ((uint8_t)0x03)  /* TIM1/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF3_TIM1_COMP1    ((uint8_t)0x03)  /* TIM1/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 4 selection
  */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping   */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping   */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping   */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping   */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping   */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_SPI3          ((uint8_t)0x06)  /* SPI3 Alternate Function mapping   */
#define GPIO_AF6_DFSDM1        ((uint8_t)0x06)  /* DFSDM1 Alternate Function mapping */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_USART1        ((uint8_t)0x07)  /* USART1 Alternate Function mapping     */
#define GPIO_AF7_USART2        ((uint8_t)0x07)  /* USART2 Alternate Function mapping     */
#define GPIO_AF7_USART3        ((uint8_t)0x07)  /* USART3 Alternate Function mapping     */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_UART4         ((uint8_t)0x08)  /* UART4 Alternate Function mapping   */
#define GPIO_AF8_UART5         ((uint8_t)0x08)  /* UART5 Alternate Function mapping   */
#define GPIO_AF8_LPUART1       ((uint8_t)0x08)  /* LPUART1 Alternate Function mapping */


/**
  * @brief   AF 9 selection
  */
#define GPIO_AF9_CAN1          ((uint8_t)0x09)  /* CAN1 Alternate Function mapping  */
#define GPIO_AF9_TSC           ((uint8_t)0x09)  /* TSC Alternate Function mapping   */

/**
  * @brief   AF 10 selection
  */
#if defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx)
#define GPIO_AF10_OTG_FS       ((uint8_t)0x0A)  /* OTG_FS Alternate Function mapping */
#endif /* STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx */
#define GPIO_AF10_QUADSPI      ((uint8_t)0x0A)  /* QUADSPI Alternate Function mapping */

#if defined(STM32L476xx) || defined(STM32L486xx)
/**
  * @brief   AF 11 selection
  */
#define GPIO_AF11_LCD          ((uint8_t)0x0B)  /* LCD Alternate Function mapping */
#endif /* STM32L476xx || STM32L486xx */

/**
  * @brief   AF 12 selection
  */
#define GPIO_AF12_FMC          ((uint8_t)0x0C)  /* FMC Alternate Function mapping     */
#define GPIO_AF12_SWPMI1       ((uint8_t)0x0C)  /* SWPMI1 Alternate Function mapping  */
#define GPIO_AF12_COMP1        ((uint8_t)0x0C)  /* COMP1 Alternate Function mapping   */
#define GPIO_AF12_COMP2        ((uint8_t)0x0C)  /* COMP2 Alternate Function mapping   */
#define GPIO_AF12_SDMMC1       ((uint8_t)0x0C)  /* SDMMC1 Alternate Function mapping  */

/**
  * @brief   AF 13 selection
  */
#define GPIO_AF13_SAI1         ((uint8_t)0x0D)  /* SAI1 Alternate Function mapping */
#define GPIO_AF13_SAI2         ((uint8_t)0x0D)  /* SAI2 Alternate Function mapping */
#define GPIO_AF13_TIM8_COMP2   ((uint8_t)0x0D)  /* TIM8/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF13_TIM8_COMP1   ((uint8_t)0x0D)  /* TIM8/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_TIM2         ((uint8_t)0x0E)  /* TIM2 Alternate Function mapping */
#define GPIO_AF14_TIM15        ((uint8_t)0x0E)  /* TIM15 Alternate Function mapping */
#define GPIO_AF14_TIM16        ((uint8_t)0x0E)  /* TIM16 Alternate Function mapping */
#define GPIO_AF14_TIM17        ((uint8_t)0x0E)  /* TIM17 Alternate Function mapping */
#define GPIO_AF14_LPTIM2       ((uint8_t)0x0E)  /* LPTIM2 Alternate Function mapping */
#define GPIO_AF14_TIM8_COMP1   ((uint8_t)0x0E)  /* TIM8/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 15 selection
  */
#define GPIO_AF15_EVENTOUT     ((uint8_t)0x0F)  /* EVENTOUT Alternate Function mapping */

#define IS_GPIO_AF(AF)   ((AF) <= (uint8_t)0x0F)

#endif /* STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx */

#if defined(STM32L496xx) || defined(STM32L4A6xx)
/*--------------------------------STM32L496xx/STM32L4A6xx---------------------*/
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_RTC_50Hz      ((uint8_t)0x00)  /* RTC_50Hz Alternate Function mapping                       */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /* MCO (MCO1 and MCO2) Alternate Function mapping            */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping             */
#define GPIO_AF0_TRACE         ((uint8_t)0x00)  /* TRACE Alternate Function mapping                          */

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_TIM1          ((uint8_t)0x01)  /* TIM1 Alternate Function mapping */
#define GPIO_AF1_TIM2          ((uint8_t)0x01)  /* TIM2 Alternate Function mapping */
#define GPIO_AF1_TIM5          ((uint8_t)0x01)  /* TIM5 Alternate Function mapping */
#define GPIO_AF1_TIM8          ((uint8_t)0x01)  /* TIM8 Alternate Function mapping */
#define GPIO_AF1_LPTIM1        ((uint8_t)0x01)  /* LPTIM1 Alternate Function mapping */
#define GPIO_AF1_IR            ((uint8_t)0x01)  /* IR Alternate Function mapping */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_TIM1          ((uint8_t)0x02)  /* TIM1 Alternate Function mapping */
#define GPIO_AF2_TIM2          ((uint8_t)0x02)  /* TIM2 Alternate Function mapping */
#define GPIO_AF2_TIM3          ((uint8_t)0x02)  /* TIM3 Alternate Function mapping */
#define GPIO_AF2_TIM4          ((uint8_t)0x02)  /* TIM4 Alternate Function mapping */
#define GPIO_AF2_TIM5          ((uint8_t)0x02)  /* TIM5 Alternate Function mapping */
#define GPIO_AF2_I2C4          ((uint8_t)0x02)  /* I2C4 Alternate Function mapping */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_TIM8          ((uint8_t)0x03)  /* TIM8 Alternate Function mapping  */
#define GPIO_AF3_TIM1_COMP2    ((uint8_t)0x03)  /* TIM1/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF3_TIM1_COMP1    ((uint8_t)0x03)  /* TIM1/COMP1 Break in Alternate Function mapping  */
#define GPIO_AF3_CAN2          ((uint8_t)0x03)  /* CAN2 Alternate Function mapping    */
#define GPIO_AF3_I2C4          ((uint8_t)0x03)  /* I2C4 Alternate Function mapping */
#define GPIO_AF3_QUADSPI       ((uint8_t)0x03)  /* QUADSPI Alternate Function mapping */
#define GPIO_AF3_SPI2          ((uint8_t)0x03)  /* SPI2 Alternate Function mapping */
#define GPIO_AF3_USART2        ((uint8_t)0x03)  /* USART2 Alternate Function mapping */

/**
  * @brief   AF 4 selection
  */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping   */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping   */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping   */
#define GPIO_AF4_I2C4          ((uint8_t)0x04)  /* I2C4 Alternate Function mapping   */
#define GPIO_AF4_DCMI          ((uint8_t)0x04)  /* DCMI Alternate Function mapping   */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping   */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping   */
#define GPIO_AF5_DCMI          ((uint8_t)0x05)  /* DCMI Alternate Function mapping   */
#define GPIO_AF5_I2C4          ((uint8_t)0x05)  /* I2C4 Alternate Function mapping */
#define GPIO_AF5_QUADSPI       ((uint8_t)0x05)  /* QUADSPI Alternate Function mapping */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_SPI3          ((uint8_t)0x06)  /* SPI3 Alternate Function mapping   */
#define GPIO_AF6_DFSDM1        ((uint8_t)0x06)  /* DFSDM1 Alternate Function mapping */
#define GPIO_AF6_I2C3          ((uint8_t)0x06)  /* I2C3 Alternate Function mapping */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_USART1        ((uint8_t)0x07)  /* USART1 Alternate Function mapping     */
#define GPIO_AF7_USART2        ((uint8_t)0x07)  /* USART2 Alternate Function mapping     */
#define GPIO_AF7_USART3        ((uint8_t)0x07)  /* USART3 Alternate Function mapping     */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_UART4         ((uint8_t)0x08)  /* UART4 Alternate Function mapping   */
#define GPIO_AF8_UART5         ((uint8_t)0x08)  /* UART5 Alternate Function mapping   */
#define GPIO_AF8_LPUART1       ((uint8_t)0x08)  /* LPUART1 Alternate Function mapping */
#define GPIO_AF8_CAN2          ((uint8_t)0x08)  /* CAN2 Alternate Function mapping    */

/**
  * @brief   AF 9 selection
  */
#define GPIO_AF9_CAN1          ((uint8_t)0x09)  /* CAN1 Alternate Function mapping  */
#define GPIO_AF9_TSC           ((uint8_t)0x09)  /* TSC Alternate Function mapping   */

/**
  * @brief   AF 10 selection
  */
#define GPIO_AF10_OTG_FS       ((uint8_t)0x0A)  /* OTG_FS Alternate Function mapping */
#define GPIO_AF10_QUADSPI      ((uint8_t)0x0A)  /* QUADSPI Alternate Function mapping */
#define GPIO_AF10_CAN2         ((uint8_t)0x0A)  /* CAN2 Alternate Function mapping */
#define GPIO_AF10_DCMI         ((uint8_t)0x0A)  /* DCMI Alternate Function mapping */

/**
  * @brief   AF 11 selection
  */
#define GPIO_AF11_LCD          ((uint8_t)0x0B)  /* LCD Alternate Function mapping */

/**
  * @brief   AF 12 selection
  */
#define GPIO_AF12_FMC          ((uint8_t)0x0C)  /* FMC Alternate Function mapping     */
#define GPIO_AF12_SWPMI1       ((uint8_t)0x0C)  /* SWPMI1 Alternate Function mapping  */
#define GPIO_AF12_COMP1        ((uint8_t)0x0C)  /* COMP1 Alternate Function mapping   */
#define GPIO_AF12_COMP2        ((uint8_t)0x0C)  /* COMP2 Alternate Function mapping   */
#define GPIO_AF12_SDMMC1       ((uint8_t)0x0C)  /* SDMMC1 Alternate Function mapping  */
#define GPIO_AF12_TIM1_COMP2   ((uint8_t)0x0C)  /* TIM1/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF12_TIM1_COMP1   ((uint8_t)0x0C)  /* TIM1/COMP1 Break in Alternate Function mapping  */
#define GPIO_AF12_TIM8_COMP2   ((uint8_t)0x0C)  /* TIM8/COMP2 Break in Alternate Function mapping  */

/**
  * @brief   AF 13 selection
  */
#define GPIO_AF13_SAI1         ((uint8_t)0x0D)  /* SAI1 Alternate Function mapping */
#define GPIO_AF13_SAI2         ((uint8_t)0x0D)  /* SAI2 Alternate Function mapping */
#define GPIO_AF13_TIM8_COMP2   ((uint8_t)0x0D)  /* TIM8/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF13_TIM8_COMP1   ((uint8_t)0x0D)  /* TIM8/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_TIM2         ((uint8_t)0x0E)  /* TIM2 Alternate Function mapping */
#define GPIO_AF14_TIM15        ((uint8_t)0x0E)  /* TIM15 Alternate Function mapping */
#define GPIO_AF14_TIM16        ((uint8_t)0x0E)  /* TIM16 Alternate Function mapping */
#define GPIO_AF14_TIM17        ((uint8_t)0x0E)  /* TIM17 Alternate Function mapping */
#define GPIO_AF14_LPTIM2       ((uint8_t)0x0E)  /* LPTIM2 Alternate Function mapping */
#define GPIO_AF14_TIM8_COMP1   ((uint8_t)0x0E)  /* TIM8/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 15 selection
  */
#define GPIO_AF15_EVENTOUT     ((uint8_t)0x0F)  /* EVENTOUT Alternate Function mapping */

#define IS_GPIO_AF(AF)   ((AF) <= (uint8_t)0x0F)

#endif /* STM32L496xx || STM32L4A6xx */

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
/*---STM32L4R5xx/STM32L4R7xx/STM32L4R9xx/STM32L4S5xx/STM32L4S7xx/STM32L4S9xx--*/
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_RTC_50Hz      ((uint8_t)0x00)  /* RTC_50Hz Alternate Function mapping                       */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /* MCO (MCO1 and MCO2) Alternate Function mapping            */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping             */
#define GPIO_AF0_TRACE         ((uint8_t)0x00)  /* TRACE Alternate Function mapping                          */

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_TIM1          ((uint8_t)0x01)  /* TIM1 Alternate Function mapping */
#define GPIO_AF1_TIM2          ((uint8_t)0x01)  /* TIM2 Alternate Function mapping */
#define GPIO_AF1_TIM5          ((uint8_t)0x01)  /* TIM5 Alternate Function mapping */
#define GPIO_AF1_TIM8          ((uint8_t)0x01)  /* TIM8 Alternate Function mapping */
#define GPIO_AF1_LPTIM1        ((uint8_t)0x01)  /* LPTIM1 Alternate Function mapping */
#define GPIO_AF1_IR            ((uint8_t)0x01)  /* IR Alternate Function mapping   */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_TIM1          ((uint8_t)0x02)  /* TIM1 Alternate Function mapping */
#define GPIO_AF2_TIM2          ((uint8_t)0x02)  /* TIM2 Alternate Function mapping */
#define GPIO_AF2_TIM3          ((uint8_t)0x02)  /* TIM3 Alternate Function mapping */
#define GPIO_AF2_TIM4          ((uint8_t)0x02)  /* TIM4 Alternate Function mapping */
#define GPIO_AF2_TIM5          ((uint8_t)0x02)  /* TIM5 Alternate Function mapping */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_I2C4          ((uint8_t)0x03)  /* I2C4 Alternate Function mapping   */
#define GPIO_AF3_OCTOSPIM_P1   ((uint8_t)0x03)  /* OctoSPI Manager Port 1 Alternate Function mapping */
#define GPIO_AF3_SAI1          ((uint8_t)0x03)  /* SAI1 Alternate Function mapping */
#define GPIO_AF3_SPI2          ((uint8_t)0x03)  /* SPI2 Alternate Function mapping   */
#define GPIO_AF3_TIM1_COMP1    ((uint8_t)0x03)  /* TIM1/COMP1 Break in Alternate Function mapping  */
#define GPIO_AF3_TIM1_COMP2    ((uint8_t)0x03)  /* TIM1/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF3_TIM8          ((uint8_t)0x03)  /* TIM8 Alternate Function mapping   */
#define GPIO_AF3_TIM8_COMP1    ((uint8_t)0x03)  /* TIM8/COMP1 Break in Alternate Function mapping  */
#define GPIO_AF3_TIM8_COMP2    ((uint8_t)0x03)  /* TIM8/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF3_USART2        ((uint8_t)0x03)  /* USART2 Alternate Function mapping   */

/**
  * @brief   AF 4 selection
  */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping   */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping   */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping   */
#define GPIO_AF4_I2C4          ((uint8_t)0x04)  /* I2C4 Alternate Function mapping   */
#define GPIO_AF4_DCMI          ((uint8_t)0x04)  /* DCMI Alternate Function mapping   */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_DCMI          ((uint8_t)0x05)  /* DCMI Alternate Function mapping     */
#define GPIO_AF5_DFSDM1        ((uint8_t)0x05)  /* DFSDM1 Alternate Function mapping   */
#define GPIO_AF5_I2C4          ((uint8_t)0x05)  /* I2C4 Alternate Function mapping     */
#define GPIO_AF5_OCTOSPIM_P1   ((uint8_t)0x05)  /* OctoSPI Manager Port 1 Alternate Function mapping */
#define GPIO_AF5_OCTOSPIM_P2   ((uint8_t)0x05)  /* OctoSPI Manager Port 2 Alternate Function mapping */
#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping     */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping     */
#define GPIO_AF5_SPI3          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping     */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_DFSDM1        ((uint8_t)0x06)  /* DFSDM1 Alternate Function mapping */
#define GPIO_AF6_I2C3          ((uint8_t)0x06)  /* I2C3 Alternate Function mapping   */
#define GPIO_AF6_SPI3          ((uint8_t)0x06)  /* SPI3 Alternate Function mapping   */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_USART1        ((uint8_t)0x07)  /* USART1 Alternate Function mapping */
#define GPIO_AF7_USART2        ((uint8_t)0x07)  /* USART2 Alternate Function mapping */
#define GPIO_AF7_USART3        ((uint8_t)0x07)  /* USART3 Alternate Function mapping */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_LPUART1       ((uint8_t)0x08)  /* LPUART1 Alternate Function mapping */
#define GPIO_AF8_SDMMC1        ((uint8_t)0x08)  /* SDMMC1 Alternate Function mapping  */
#define GPIO_AF8_UART4         ((uint8_t)0x08)  /* UART4 Alternate Function mapping   */
#define GPIO_AF8_UART5         ((uint8_t)0x08)  /* UART5 Alternate Function mapping   */

/**
  * @brief   AF 9 selection
  */
#define GPIO_AF9_CAN1          ((uint8_t)0x09)  /* CAN1 Alternate Function mapping  */
#define GPIO_AF9_LTDC          ((uint8_t)0x09)  /* LTDC Alternate Function mapping  */
#define GPIO_AF9_TSC           ((uint8_t)0x09)  /* TSC Alternate Function mapping   */

/**
  * @brief   AF 10 selection
  */
#define GPIO_AF10_DCMI         ((uint8_t)0x0A)  /* DCMI Alternate Function mapping     */
#define GPIO_AF10_OCTOSPIM_P1  ((uint8_t)0x0A)  /* OctoSPI Manager Port 1 Alternate Function mapping */
#define GPIO_AF10_OCTOSPIM_P2  ((uint8_t)0x0A)  /* OctoSPI Manager Port 2 Alternate Function mapping */
#define GPIO_AF10_OTG_FS       ((uint8_t)0x0A)  /* OTG_FS Alternate Function mapping   */

/**
  * @brief   AF 11 selection
  */
#define GPIO_AF11_DSI          ((uint8_t)0x0B)  /* DSI Alternate Function mapping  */
#define GPIO_AF11_LTDC         ((uint8_t)0x0B)  /* LTDC Alternate Function mapping */

/**
  * @brief   AF 12 selection
  */
#define GPIO_AF12_COMP1        ((uint8_t)0x0C)  /* COMP1 Alternate Function mapping   */
#define GPIO_AF12_COMP2        ((uint8_t)0x0C)  /* COMP2 Alternate Function mapping   */
#define GPIO_AF12_DSI          ((uint8_t)0x0C)  /* DSI Alternate Function mapping     */
#define GPIO_AF12_FMC          ((uint8_t)0x0C)  /* FMC Alternate Function mapping     */
#define GPIO_AF12_SDMMC1       ((uint8_t)0x0C)  /* SDMMC1 Alternate Function mapping  */
#define GPIO_AF12_TIM1_COMP1   ((uint8_t)0x0C)  /* TIM1/COMP1 Break in Alternate Function mapping  */
#define GPIO_AF12_TIM1_COMP2   ((uint8_t)0x0C)  /* TIM1/COMP2 Break in Alternate Function mapping  */
#define GPIO_AF12_TIM8_COMP2   ((uint8_t)0x0C)  /* TIM8/COMP2 Break in Alternate Function mapping  */

/**
  * @brief   AF 13 selection
  */
#define GPIO_AF13_SAI1         ((uint8_t)0x0D)  /* SAI1 Alternate Function mapping */
#define GPIO_AF13_SAI2         ((uint8_t)0x0D)  /* SAI2 Alternate Function mapping */
#define GPIO_AF13_TIM8_COMP1   ((uint8_t)0x0D)  /* TIM8/COMP1 Break in Alternate Function mapping  */

/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_TIM15        ((uint8_t)0x0E)  /* TIM15 Alternate Function mapping  */
#define GPIO_AF14_TIM16        ((uint8_t)0x0E)  /* TIM16 Alternate Function mapping  */
#define GPIO_AF14_TIM17        ((uint8_t)0x0E)  /* TIM17 Alternate Function mapping  */
#define GPIO_AF14_LPTIM2       ((uint8_t)0x0E)  /* LPTIM2 Alternate Function mapping */
#define GPIO_AF14_TIM8_COMP2   ((uint8_t)0x0E)  /* TIM8/COMP2 Break in Alternate Function mapping   */

/**
  * @brief   AF 15 selection
  */
#define GPIO_AF15_EVENTOUT     ((uint8_t)0x0F)  /* EVENTOUT Alternate Function mapping */

#define IS_GPIO_AF(AF)   ((AF) <= (uint8_t)0x0F)

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup GPIOEx_Exported_Macros GPIOEx Exported Macros
  * @{
  */

/** @defgroup GPIOEx_Get_Port_Index GPIOEx_Get Port Index
* @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)

#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL :\
                                      ((__GPIOx__) == (GPIOD))? 3uL : 7uL)

#endif /* STM32L412xx || STM32L422xx */

#if defined(STM32L431xx) || defined(STM32L433xx) || defined(STM32L443xx)

#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL :\
                                      ((__GPIOx__) == (GPIOD))? 3uL :\
                                      ((__GPIOx__) == (GPIOE))? 4uL : 7uL)

#endif /* STM32L431xx || STM32L433xx || STM32L443xx */

#if defined(STM32L432xx) || defined(STM32L442xx)

#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL : 7uL)

#endif /* STM32L432xx || STM32L442xx */

#if defined(STM32L451xx) || defined(STM32L452xx) || defined(STM32L462xx)

#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL :\
                                      ((__GPIOx__) == (GPIOD))? 3uL :\
                                      ((__GPIOx__) == (GPIOE))? 4uL : 7uL)

#endif /* STM32L451xx || STM32L452xx || STM32L462xx */

#if defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx)

#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL :\
                                      ((__GPIOx__) == (GPIOD))? 3uL :\
                                      ((__GPIOx__) == (GPIOE))? 4uL :\
                                      ((__GPIOx__) == (GPIOF))? 5uL :\
                                      ((__GPIOx__) == (GPIOG))? 6uL : 7uL)

#endif /* STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx */

#if defined(STM32L496xx) || defined(STM32L4A6xx)

#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL :\
                                      ((__GPIOx__) == (GPIOD))? 3uL :\
                                      ((__GPIOx__) == (GPIOE))? 4uL :\
                                      ((__GPIOx__) == (GPIOF))? 5uL :\
                                      ((__GPIOx__) == (GPIOG))? 6uL :\
                                      ((__GPIOx__) == (GPIOH))? 7uL : 8uL)

#endif /* STM32L496xx || STM32L4A6xx */

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)

#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL :\
                                      ((__GPIOx__) == (GPIOD))? 3uL :\
                                      ((__GPIOx__) == (GPIOE))? 4uL :\
                                      ((__GPIOx__) == (GPIOF))? 5uL :\
                                      ((__GPIOx__) == (GPIOG))? 6uL :\
                                      ((__GPIOx__) == (GPIOH))? 7uL : 8uL)

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_HAL_GPIO_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
