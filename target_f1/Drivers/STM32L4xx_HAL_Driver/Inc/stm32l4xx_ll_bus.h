/**
  ******************************************************************************
  * @file    stm32l4xx_ll_bus.h
  * @author  MCD Application Team
  * @brief   Header file of BUS LL module.

  @verbatim
                      ##### RCC Limitations #####
  ==============================================================================
    [..]
      A delay between an RCC peripheral clock enable and the effective peripheral
      enabling should be taken into account in order to manage the peripheral read/write
      from/to registers.
      (+) This delay depends on the peripheral mapping.
        (++) AHB & APB peripherals, 1 dummy read is necessary

    [..]
      Workarounds:
      (#) For AHB & APB peripherals, a dummy read to the peripheral register has been
          inserted in each LL_{BUS}_GRP{x}_EnableClock() function.

  @endverbatim
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
#ifndef STM32L4xx_LL_BUS_H
#define STM32L4xx_LL_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined(RCC)

/** @defgroup BUS_LL BUS
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup BUS_LL_Exported_Constants BUS Exported Constants
  * @{
  */

/** @defgroup BUS_LL_EC_AHB1_GRP1_PERIPH  AHB1 GRP1 PERIPH
  * @{
  */
#define LL_AHB1_GRP1_PERIPH_ALL            0xFFFFFFFFU
#define LL_AHB1_GRP1_PERIPH_DMA1           RCC_AHB1ENR_DMA1EN
#define LL_AHB1_GRP1_PERIPH_DMA2           RCC_AHB1ENR_DMA2EN
#if defined(DMAMUX1)
#define LL_AHB1_GRP1_PERIPH_DMAMUX1        RCC_AHB1ENR_DMAMUX1EN
#endif /* DMAMUX1 */
#define LL_AHB1_GRP1_PERIPH_FLASH          RCC_AHB1ENR_FLASHEN
#define LL_AHB1_GRP1_PERIPH_CRC            RCC_AHB1ENR_CRCEN
#define LL_AHB1_GRP1_PERIPH_TSC            RCC_AHB1ENR_TSCEN
#if defined(DMA2D)
#define LL_AHB1_GRP1_PERIPH_DMA2D          RCC_AHB1ENR_DMA2DEN
#endif /* DMA2D */
#if defined(GFXMMU)
#define LL_AHB1_GRP1_PERIPH_GFXMMU         RCC_AHB1ENR_GFXMMUEN
#endif /* GFXMMU */
#define LL_AHB1_GRP1_PERIPH_SRAM1          RCC_AHB1SMENR_SRAM1SMEN
/**
  * @}
  */

/** @defgroup BUS_LL_EC_AHB2_GRP1_PERIPH  AHB2 GRP1 PERIPH
  * @{
  */
#define LL_AHB2_GRP1_PERIPH_ALL            0xFFFFFFFFU
#define LL_AHB2_GRP1_PERIPH_GPIOA          RCC_AHB2ENR_GPIOAEN
#define LL_AHB2_GRP1_PERIPH_GPIOB          RCC_AHB2ENR_GPIOBEN
#define LL_AHB2_GRP1_PERIPH_GPIOC          RCC_AHB2ENR_GPIOCEN
#if defined(GPIOD)
#define LL_AHB2_GRP1_PERIPH_GPIOD          RCC_AHB2ENR_GPIODEN
#endif /*GPIOD*/
#if defined(GPIOE)
#define LL_AHB2_GRP1_PERIPH_GPIOE          RCC_AHB2ENR_GPIOEEN
#endif /*GPIOE*/
#if defined(GPIOF)
#define LL_AHB2_GRP1_PERIPH_GPIOF          RCC_AHB2ENR_GPIOFEN
#endif /* GPIOF */
#if defined(GPIOG)
#define LL_AHB2_GRP1_PERIPH_GPIOG          RCC_AHB2ENR_GPIOGEN
#endif /* GPIOG */
#define LL_AHB2_GRP1_PERIPH_GPIOH          RCC_AHB2ENR_GPIOHEN
#if defined(GPIOI)
#define LL_AHB2_GRP1_PERIPH_GPIOI          RCC_AHB2ENR_GPIOIEN
#endif /* GPIOI */
#if defined(USB_OTG_FS)
#define LL_AHB2_GRP1_PERIPH_OTGFS          RCC_AHB2ENR_OTGFSEN
#endif /* USB_OTG_FS */
#define LL_AHB2_GRP1_PERIPH_ADC            RCC_AHB2ENR_ADCEN
#if defined(DCMI)
#define LL_AHB2_GRP1_PERIPH_DCMI           RCC_AHB2ENR_DCMIEN
#endif /* DCMI */
#if defined(AES)
#define LL_AHB2_GRP1_PERIPH_AES            RCC_AHB2ENR_AESEN
#endif /* AES */
#if defined(HASH)
#define LL_AHB2_GRP1_PERIPH_HASH           RCC_AHB2ENR_HASHEN
#endif /* HASH */
#define LL_AHB2_GRP1_PERIPH_RNG            RCC_AHB2ENR_RNGEN
#if defined(OCTOSPIM)
#define LL_AHB2_GRP1_PERIPH_OSPIM          RCC_AHB2ENR_OSPIMEN
#endif /* OCTOSPIM */
#if defined(SDMMC1) && defined(RCC_AHB2ENR_SDMMC1EN)
#define LL_AHB2_GRP1_PERIPH_SDMMC1         RCC_AHB2ENR_SDMMC1EN
#endif /* SDMMC1 && RCC_AHB2ENR_SDMMC1EN */
#define LL_AHB2_GRP1_PERIPH_SRAM2          RCC_AHB2SMENR_SRAM2SMEN
#if defined(SRAM3_BASE)
#define LL_AHB2_GRP1_PERIPH_SRAM3          RCC_AHB2SMENR_SRAM3SMEN
#endif /* SRAM3_BASE */
/**
  * @}
  */

/** @defgroup BUS_LL_EC_AHB3_GRP1_PERIPH  AHB3 GRP1 PERIPH
  * @{
  */
#define LL_AHB3_GRP1_PERIPH_ALL            0xFFFFFFFFU
#if defined(FMC_Bank1_R)
#define LL_AHB3_GRP1_PERIPH_FMC            RCC_AHB3ENR_FMCEN
#endif /* FMC_Bank1_R */
#if defined(QUADSPI)
#define LL_AHB3_GRP1_PERIPH_QSPI           RCC_AHB3ENR_QSPIEN
#endif /* QUADSPI */
#if defined(OCTOSPI1)
#define LL_AHB3_GRP1_PERIPH_OSPI1          RCC_AHB3ENR_OSPI1EN
#endif /* OCTOSPI1 */
#if defined(OCTOSPI2)
#define LL_AHB3_GRP1_PERIPH_OSPI2          RCC_AHB3ENR_OSPI2EN
#endif /* OCTOSPI2 */
/**
  * @}
  */

/** @defgroup BUS_LL_EC_APB1_GRP1_PERIPH  APB1 GRP1 PERIPH
  * @{
  */
#define LL_APB1_GRP1_PERIPH_ALL            0xFFFFFFFFU
#define LL_APB1_GRP1_PERIPH_TIM2           RCC_APB1ENR1_TIM2EN
#if defined(TIM3)
#define LL_APB1_GRP1_PERIPH_TIM3           RCC_APB1ENR1_TIM3EN
#endif /* TIM3 */
#if defined(TIM4)
#define LL_APB1_GRP1_PERIPH_TIM4           RCC_APB1ENR1_TIM4EN
#endif /* TIM4 */
#if defined(TIM5)
#define LL_APB1_GRP1_PERIPH_TIM5           RCC_APB1ENR1_TIM5EN
#endif /* TIM5 */
#define LL_APB1_GRP1_PERIPH_TIM6           RCC_APB1ENR1_TIM6EN
#define LL_APB1_GRP1_PERIPH_TIM7           RCC_APB1ENR1_TIM7EN
#if defined(LCD)
#define LL_APB1_GRP1_PERIPH_LCD            RCC_APB1ENR1_LCDEN
#endif /* LCD */
#if defined(RCC_APB1ENR1_RTCAPBEN)
#define LL_APB1_GRP1_PERIPH_RTCAPB         RCC_APB1ENR1_RTCAPBEN
#endif /* RCC_APB1ENR1_RTCAPBEN */
#define LL_APB1_GRP1_PERIPH_WWDG           RCC_APB1ENR1_WWDGEN
#if defined(SPI2)
#define LL_APB1_GRP1_PERIPH_SPI2           RCC_APB1ENR1_SPI2EN
#endif /* SPI2 */
#define LL_APB1_GRP1_PERIPH_SPI3           RCC_APB1ENR1_SPI3EN
#define LL_APB1_GRP1_PERIPH_USART2         RCC_APB1ENR1_USART2EN
#if defined(USART3)
#define LL_APB1_GRP1_PERIPH_USART3         RCC_APB1ENR1_USART3EN
#endif /* USART3 */
#if defined(UART4)
#define LL_APB1_GRP1_PERIPH_UART4          RCC_APB1ENR1_UART4EN
#endif /* UART4 */
#if defined(UART5)
#define LL_APB1_GRP1_PERIPH_UART5          RCC_APB1ENR1_UART5EN
#endif /* UART5 */
#define LL_APB1_GRP1_PERIPH_I2C1           RCC_APB1ENR1_I2C1EN
#if defined(I2C2)
#define LL_APB1_GRP1_PERIPH_I2C2           RCC_APB1ENR1_I2C2EN
#endif /* I2C2 */
#define LL_APB1_GRP1_PERIPH_I2C3           RCC_APB1ENR1_I2C3EN
#if defined(CRS)
#define LL_APB1_GRP1_PERIPH_CRS            RCC_APB1ENR1_CRSEN
#endif /* CRS */
#define LL_APB1_GRP1_PERIPH_CAN1           RCC_APB1ENR1_CAN1EN
#if defined(CAN2)
#define LL_APB1_GRP1_PERIPH_CAN2           RCC_APB1ENR1_CAN2EN
#endif /* CAN2 */
#if defined(USB)
#define LL_APB1_GRP1_PERIPH_USB            RCC_APB1ENR1_USBFSEN
#endif /* USB */
#define LL_APB1_GRP1_PERIPH_PWR            RCC_APB1ENR1_PWREN
#define LL_APB1_GRP1_PERIPH_DAC1           RCC_APB1ENR1_DAC1EN
#define LL_APB1_GRP1_PERIPH_OPAMP          RCC_APB1ENR1_OPAMPEN
#define LL_APB1_GRP1_PERIPH_LPTIM1         RCC_APB1ENR1_LPTIM1EN
/**
  * @}
  */


/** @defgroup BUS_LL_EC_APB1_GRP2_PERIPH  APB1 GRP2 PERIPH
  * @{
  */
#define LL_APB1_GRP2_PERIPH_ALL            0xFFFFFFFFU
#define LL_APB1_GRP2_PERIPH_LPUART1        RCC_APB1ENR2_LPUART1EN
#if defined(I2C4)
#define LL_APB1_GRP2_PERIPH_I2C4           RCC_APB1ENR2_I2C4EN
#endif /* I2C4 */
#if defined(SWPMI1)
#define LL_APB1_GRP2_PERIPH_SWPMI1         RCC_APB1ENR2_SWPMI1EN
#endif /* SWPMI1 */
#define LL_APB1_GRP2_PERIPH_LPTIM2         RCC_APB1ENR2_LPTIM2EN
/**
  * @}
  */

/** @defgroup BUS_LL_EC_APB2_GRP1_PERIPH  APB2 GRP1 PERIPH
  * @{
  */
#define LL_APB2_GRP1_PERIPH_ALL            0xFFFFFFFFU
#define LL_APB2_GRP1_PERIPH_SYSCFG         RCC_APB2ENR_SYSCFGEN
#define LL_APB2_GRP1_PERIPH_FW             RCC_APB2ENR_FWEN
#if defined(SDMMC1) && defined(RCC_APB2ENR_SDMMC1EN)
#define LL_APB2_GRP1_PERIPH_SDMMC1         RCC_APB2ENR_SDMMC1EN
#endif /* SDMMC1 && RCC_APB2ENR_SDMMC1EN */
#define LL_APB2_GRP1_PERIPH_TIM1           RCC_APB2ENR_TIM1EN
#define LL_APB2_GRP1_PERIPH_SPI1           RCC_APB2ENR_SPI1EN
#if defined(TIM8)
#define LL_APB2_GRP1_PERIPH_TIM8           RCC_APB2ENR_TIM8EN
#endif /* TIM8 */
#define LL_APB2_GRP1_PERIPH_USART1         RCC_APB2ENR_USART1EN
#define LL_APB2_GRP1_PERIPH_TIM15          RCC_APB2ENR_TIM15EN
#define LL_APB2_GRP1_PERIPH_TIM16          RCC_APB2ENR_TIM16EN
#if defined(TIM17)
#define LL_APB2_GRP1_PERIPH_TIM17          RCC_APB2ENR_TIM17EN
#endif /* TIM17 */
#define LL_APB2_GRP1_PERIPH_SAI1           RCC_APB2ENR_SAI1EN
#if defined(SAI2)
#define LL_APB2_GRP1_PERIPH_SAI2           RCC_APB2ENR_SAI2EN
#endif /* SAI2 */
#if defined(DFSDM1_Channel0)
#define LL_APB2_GRP1_PERIPH_DFSDM1         RCC_APB2ENR_DFSDM1EN
#endif /* DFSDM1_Channel0 */
#if defined(LTDC)
#define LL_APB2_GRP1_PERIPH_LTDC           RCC_APB2ENR_LTDCEN
#endif /* LTDC */
#if defined(DSI)
#define LL_APB2_GRP1_PERIPH_DSI            RCC_APB2ENR_DSIEN
#endif /* DSI */
/**
  * @}
  */

/** Legacy definitions for compatibility purpose
@cond 0
*/
#if defined(DFSDM1_Channel0)
#define LL_APB2_GRP1_PERIPH_DFSDM          LL_APB2_GRP1_PERIPH_DFSDM1
#endif /* DFSDM1_Channel0 */
/**
@endcond
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup BUS_LL_Exported_Functions BUS Exported Functions
  * @{
  */

/** @defgroup BUS_LL_EF_AHB1 AHB1
  * @{
  */

/**
  * @brief  Enable AHB1 peripherals clock.
  * @rmtoll AHB1ENR      DMA1EN        LL_AHB1_GRP1_EnableClock\n
  *         AHB1ENR      DMA2EN        LL_AHB1_GRP1_EnableClock\n
  *         AHB1ENR      DMAMUX1EN     LL_AHB1_GRP1_EnableClock\n
  *         AHB1ENR      FLASHEN       LL_AHB1_GRP1_EnableClock\n
  *         AHB1ENR      CRCEN         LL_AHB1_GRP1_EnableClock\n
  *         AHB1ENR      TSCEN         LL_AHB1_GRP1_EnableClock\n
  *         AHB1ENR      DMA2DEN       LL_AHB1_GRP1_EnableClock\n
  *         AHB1ENR      GFXMMUEN      LL_AHB1_GRP1_EnableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMAMUX1 (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_FLASH
  *         @arg @ref LL_AHB1_GRP1_PERIPH_CRC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_TSC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2D (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_GFXMMU (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB1_GRP1_EnableClock(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->AHB1ENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->AHB1ENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Check if AHB1 peripheral clock is enabled or not
  * @rmtoll AHB1ENR      DMA1EN        LL_AHB1_GRP1_IsEnabledClock\n
  *         AHB1ENR      DMA2EN        LL_AHB1_GRP1_IsEnabledClock\n
  *         AHB1ENR      DMAMUX1EN     LL_AHB1_GRP1_IsEnabledClock\n
  *         AHB1ENR      FLASHEN       LL_AHB1_GRP1_IsEnabledClock\n
  *         AHB1ENR      CRCEN         LL_AHB1_GRP1_IsEnabledClock\n
  *         AHB1ENR      TSCEN         LL_AHB1_GRP1_IsEnabledClock\n
  *         AHB1ENR      DMA2DEN       LL_AHB1_GRP1_IsEnabledClock\n
  *         AHB1ENR      GFXMMUEN      LL_AHB1_GRP1_IsEnabledClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMAMUX1 (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_FLASH
  *         @arg @ref LL_AHB1_GRP1_PERIPH_CRC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_TSC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2D (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_GFXMMU (*)
  *
  *         (*) value not defined in all devices.
  * @retval State of Periphs (1 or 0).
*/
__STATIC_INLINE uint32_t LL_AHB1_GRP1_IsEnabledClock(uint32_t Periphs)
{
  return ((READ_BIT(RCC->AHB1ENR, Periphs) == Periphs) ? 1UL : 0UL);
}

/**
  * @brief  Disable AHB1 peripherals clock.
  * @rmtoll AHB1ENR      DMA1EN        LL_AHB1_GRP1_DisableClock\n
  *         AHB1ENR      DMA2EN        LL_AHB1_GRP1_DisableClock\n
  *         AHB1ENR      DMAMUX1EN     LL_AHB1_GRP1_DisableClock\n
  *         AHB1ENR      FLASHEN       LL_AHB1_GRP1_DisableClock\n
  *         AHB1ENR      CRCEN         LL_AHB1_GRP1_DisableClock\n
  *         AHB1ENR      TSCEN         LL_AHB1_GRP1_DisableClock\n
  *         AHB1ENR      DMA2DEN       LL_AHB1_GRP1_DisableClock\n
  *         AHB1ENR      GFXMMUEN      LL_AHB1_GRP1_DisableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMAMUX1 (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_FLASH
  *         @arg @ref LL_AHB1_GRP1_PERIPH_CRC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_TSC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2D (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_GFXMMU (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB1_GRP1_DisableClock(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB1ENR, Periphs);
}

/**
  * @brief  Force AHB1 peripherals reset.
  * @rmtoll AHB1RSTR     DMA1RST       LL_AHB1_GRP1_ForceReset\n
  *         AHB1RSTR     DMA2RST       LL_AHB1_GRP1_ForceReset\n
  *         AHB1RSTR     DMAMUX1RST     LL_AHB1_GRP1_ForceReset\n
  *         AHB1RSTR     FLASHRST      LL_AHB1_GRP1_ForceReset\n
  *         AHB1RSTR     CRCRST        LL_AHB1_GRP1_ForceReset\n
  *         AHB1RSTR     TSCRST        LL_AHB1_GRP1_ForceReset\n
  *         AHB1RSTR     DMA2DRST      LL_AHB1_GRP1_ForceReset\n
  *         AHB1RSTR     GFXMMURST     LL_AHB1_GRP1_ForceReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB1_GRP1_PERIPH_ALL
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMAMUX1 (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_FLASH
  *         @arg @ref LL_AHB1_GRP1_PERIPH_CRC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_TSC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2D (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_GFXMMU (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB1_GRP1_ForceReset(uint32_t Periphs)
{
  SET_BIT(RCC->AHB1RSTR, Periphs);
}

/**
  * @brief  Release AHB1 peripherals reset.
  * @rmtoll AHB1RSTR     DMA1RST       LL_AHB1_GRP1_ReleaseReset\n
  *         AHB1RSTR     DMA2RST       LL_AHB1_GRP1_ReleaseReset\n
  *         AHB1RSTR     DMAMUX1RST     LL_AHB1_GRP1_ReleaseReset\n
  *         AHB1RSTR     FLASHRST      LL_AHB1_GRP1_ReleaseReset\n
  *         AHB1RSTR     CRCRST        LL_AHB1_GRP1_ReleaseReset\n
  *         AHB1RSTR     TSCRST        LL_AHB1_GRP1_ReleaseReset\n
  *         AHB1RSTR     DMA2DRST      LL_AHB1_GRP1_ReleaseReset\n
  *         AHB1RSTR     GFXMMURST     LL_AHB1_GRP1_ReleaseReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB1_GRP1_PERIPH_ALL
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMAMUX1 (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_FLASH
  *         @arg @ref LL_AHB1_GRP1_PERIPH_CRC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_TSC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2D (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_GFXMMU (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB1_GRP1_ReleaseReset(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB1RSTR, Periphs);
}

/**
  * @brief  Enable AHB1 peripheral clocks in Sleep and Stop modes
  * @rmtoll AHB1SMENR    DMA1SMEN      LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    DMA2SMEN      LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    DMAMUX1SMEN   LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    FLASHSMEN     LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    SRAM1SMEN     LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    CRCSMEN       LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    TSCSMEN       LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    DMA2DSMEN     LL_AHB1_GRP1_EnableClockStopSleep\n
  *         AHB1SMENR    GFXMMUSMEN    LL_AHB1_GRP1_EnableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMAMUX1 (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_FLASH
  *         @arg @ref LL_AHB1_GRP1_PERIPH_SRAM1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_CRC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_TSC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2D (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_GFXMMU (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB1_GRP1_EnableClockStopSleep(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->AHB1SMENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->AHB1SMENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Disable AHB1 peripheral clocks in Sleep and Stop modes
  * @rmtoll AHB1SMENR    DMA1SMEN      LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    DMA2SMEN      LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    DMAMUX1SMEN   LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    FLASHSMEN     LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    SRAM1SMEN     LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    CRCSMEN       LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    TSCSMEN       LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    DMA2DSMEN     LL_AHB1_GRP1_DisableClockStopSleep\n
  *         AHB1SMENR    GFXMMUSMEN    LL_AHB1_GRP1_DisableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMAMUX1 (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_FLASH
  *         @arg @ref LL_AHB1_GRP1_PERIPH_SRAM1
  *         @arg @ref LL_AHB1_GRP1_PERIPH_CRC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_TSC
  *         @arg @ref LL_AHB1_GRP1_PERIPH_DMA2D (*)
  *         @arg @ref LL_AHB1_GRP1_PERIPH_GFXMMU (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB1_GRP1_DisableClockStopSleep(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB1SMENR, Periphs);
}

/**
  * @}
  */

/** @defgroup BUS_LL_EF_AHB2 AHB2
  * @{
  */

/**
  * @brief  Enable AHB2 peripherals clock.
  * @rmtoll AHB2ENR      GPIOAEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIOBEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIOCEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIODEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIOEEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIOFEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIOGEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIOHEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      GPIOIEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      OTGFSEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      ADCEN         LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      DCMIEN        LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      AESEN         LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      HASHEN        LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      RNGEN         LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      OSPIMEN       LL_AHB2_GRP1_EnableClock\n
  *         AHB2ENR      SDMMC1EN      LL_AHB2_GRP1_EnableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOA
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOB
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOD (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOE (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOF (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOG (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOH
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OTGFS (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ADC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_DCMI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_AES (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_HASH (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_RNG
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OSPIM (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SDMMC1 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB2_GRP1_EnableClock(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->AHB2ENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->AHB2ENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Check if AHB2 peripheral clock is enabled or not
  * @rmtoll AHB2ENR      GPIOAEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIOBEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIOCEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIODEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIOEEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIOFEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIOGEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIOHEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      GPIOIEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      OTGFSEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      ADCEN         LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      DCMIEN        LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      AESEN         LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      HASHEN        LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      RNGEN         LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      OSPIMEN       LL_AHB2_GRP1_IsEnabledClock\n
  *         AHB2ENR      SDMMC1EN      LL_AHB2_GRP1_IsEnabledClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOA
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOB
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOD (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOE (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOF (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOG (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOH
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OTGFS (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ADC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_DCMI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_AES (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_HASH (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_RNG
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OSPIM (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SDMMC1 (*)
  *
  *         (*) value not defined in all devices.
  * @retval State of Periphs (1 or 0).
*/
__STATIC_INLINE uint32_t LL_AHB2_GRP1_IsEnabledClock(uint32_t Periphs)
{
  return ((READ_BIT(RCC->AHB2ENR, Periphs) == Periphs) ? 1UL : 0UL);
}

/**
  * @brief  Disable AHB2 peripherals clock.
  * @rmtoll AHB2ENR      GPIOAEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIOBEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIOCEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIODEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIOEEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIOFEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIOGEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIOHEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      GPIOIEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      OTGFSEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      ADCEN         LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      DCMIEN        LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      AESEN         LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      HASHEN        LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      RNGEN         LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      OSPIMEN       LL_AHB2_GRP1_DisableClock\n
  *         AHB2ENR      SDMMC1EN      LL_AHB2_GRP1_DisableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOA
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOB
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOD (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOE (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOF (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOG (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOH
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OTGFS (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ADC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_DCMI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_AES (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_HASH (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_RNG
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OSPIM (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SDMMC1 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB2_GRP1_DisableClock(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB2ENR, Periphs);
}

/**
  * @brief  Force AHB2 peripherals reset.
  * @rmtoll AHB2RSTR     GPIOARST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIOBRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIOCRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIODRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIOERST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIOFRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIOGRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIOHRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     GPIOIRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     OTGFSRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     ADCRST        LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     DCMIRST       LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     AESRST        LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     HASHRST       LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     RNGRST        LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     OSPIMRST      LL_AHB2_GRP1_ForceReset\n
  *         AHB2RSTR     SDMMC1RST     LL_AHB2_GRP1_ForceReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ALL
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOA
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOB
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOD (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOE (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOF (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOG (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOH
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OTGFS (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ADC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_DCMI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_AES (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_HASH (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_RNG
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OSPIM (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SDMMC1 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB2_GRP1_ForceReset(uint32_t Periphs)
{
  SET_BIT(RCC->AHB2RSTR, Periphs);
}

/**
  * @brief  Release AHB2 peripherals reset.
  * @rmtoll AHB2RSTR     GPIOARST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIOBRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIOCRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIODRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIOERST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIOFRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIOGRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIOHRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     GPIOIRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     OTGFSRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     ADCRST        LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     DCMIRST       LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     AESRST        LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     HASHRST       LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     RNGRST        LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     OSPIMRST      LL_AHB2_GRP1_ReleaseReset\n
  *         AHB2RSTR     SDMMC1RST     LL_AHB2_GRP1_ReleaseReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ALL
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOA
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOB
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOD (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOE (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOF (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOG (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOH
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OTGFS (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ADC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_DCMI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_AES (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_HASH (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_RNG
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OSPIM (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SDMMC1 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB2_GRP1_ReleaseReset(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB2RSTR, Periphs);
}

/**
  * @brief  Enable AHB2 peripheral clocks in Sleep and Stop modes
  * @rmtoll AHB2SMENR    GPIOASMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIOBSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIOCSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIODSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIOESMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIOFSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIOGSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIOHSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    GPIOISMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    SRAM2SMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    SRAM3SMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    OTGFSSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    ADCSMEN       LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    DCMISMEN      LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    AESSMEN       LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    HASHSMEN      LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    RNGSMEN       LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    OSPIMSMEN     LL_AHB2_GRP1_EnableClockStopSleep\n
  *         AHB2SMENR    SDMMC1SMEN    LL_AHB2_GRP1_EnableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOA
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOB
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOD (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOE (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOF (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOG (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOH
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SRAM2
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SRAM3 (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OTGFS (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ADC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_DCMI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_AES (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_HASH (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_RNG
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OSPIM (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SDMMC1 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB2_GRP1_EnableClockStopSleep(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->AHB2SMENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->AHB2SMENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Disable AHB2 peripheral clocks in Sleep and Stop modes
  * @rmtoll AHB2SMENR    GPIOASMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIOBSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIOCSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIODSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIOESMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIOFSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIOGSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIOHSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    GPIOISMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    SRAM2SMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    SRAM3SMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    OTGFSSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    ADCSMEN       LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    DCMISMEN      LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    AESSMEN       LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    HASHSMEN      LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    RNGSMEN       LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    OSPIMSMEN     LL_AHB2_GRP1_DisableClockStopSleep\n
  *         AHB2SMENR    SDMMC1SMEN    LL_AHB2_GRP1_DisableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOA
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOB
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOD (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOE (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOF (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOG (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOH
  *         @arg @ref LL_AHB2_GRP1_PERIPH_GPIOI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SRAM2
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SRAM3 (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OTGFS (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ADC
  *         @arg @ref LL_AHB2_GRP1_PERIPH_DCMI (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_AES (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_HASH (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_RNG
  *         @arg @ref LL_AHB2_GRP1_PERIPH_OSPIM (*)
  *         @arg @ref LL_AHB2_GRP1_PERIPH_SDMMC1 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB2_GRP1_DisableClockStopSleep(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB2SMENR, Periphs);
}

/**
  * @}
  */

/** @defgroup BUS_LL_EF_AHB3 AHB3
  * @{
  */

/**
  * @brief  Enable AHB3 peripherals clock.
  * @rmtoll AHB3ENR      FMCEN         LL_AHB3_GRP1_EnableClock\n
  *         AHB3ENR      QSPIEN        LL_AHB3_GRP1_EnableClock\n
  *         AHB3ENR      OSPI1EN       LL_AHB3_GRP1_EnableClock\n
  *         AHB3ENR      OSPI2EN       LL_AHB3_GRP1_EnableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB3_GRP1_PERIPH_FMC (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_QSPI (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI1 (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI2 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB3_GRP1_EnableClock(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->AHB3ENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->AHB3ENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Check if AHB3 peripheral clock is enabled or not
  * @rmtoll AHB3ENR      FMCEN         LL_AHB3_GRP1_IsEnabledClock\n
  *         AHB3ENR      QSPIEN        LL_AHB3_GRP1_IsEnabledClock\n
  *         AHB3ENR      OSPI1EN       LL_AHB3_GRP1_IsEnabledClock\n
  *         AHB3ENR      OSPI2EN       LL_AHB3_GRP1_IsEnabledClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB3_GRP1_PERIPH_FMC (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_QSPI (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI1 (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI2 (*)
  *
  *         (*) value not defined in all devices.
  * @retval State of Periphs (1 or 0).
*/
__STATIC_INLINE uint32_t LL_AHB3_GRP1_IsEnabledClock(uint32_t Periphs)
{
  return ((READ_BIT(RCC->AHB3ENR, Periphs) == Periphs) ? 1UL : 0UL);
}

/**
  * @brief  Disable AHB3 peripherals clock.
  * @rmtoll AHB3ENR      FMCEN         LL_AHB3_GRP1_DisableClock\n
  *         AHB3ENR      QSPIEN        LL_AHB3_GRP1_DisableClock\n
  *         AHB3ENR      OSPI1EN       LL_AHB3_GRP1_DisableClock\n
  *         AHB3ENR      OSPI2EN       LL_AHB3_GRP1_DisableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB3_GRP1_PERIPH_FMC (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_QSPI (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI1 (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI2 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB3_GRP1_DisableClock(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB3ENR, Periphs);
}

/**
  * @brief  Force AHB3 peripherals reset.
  * @rmtoll AHB3RSTR     FMCRST        LL_AHB3_GRP1_ForceReset\n
  *         AHB3RSTR     QSPIRST       LL_AHB3_GRP1_ForceReset\n
  *         AHB3RSTR     OSPI1RST      LL_AHB3_GRP1_ForceReset\n
  *         AHB3RSTR     OSPI2RST      LL_AHB3_GRP1_ForceReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB3_GRP1_PERIPH_ALL
  *         @arg @ref LL_AHB3_GRP1_PERIPH_FMC (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_QSPI (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI1 (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI2 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB3_GRP1_ForceReset(uint32_t Periphs)
{
  SET_BIT(RCC->AHB3RSTR, Periphs);
}

/**
  * @brief  Release AHB3 peripherals reset.
  * @rmtoll AHB3RSTR     FMCRST        LL_AHB3_GRP1_ReleaseReset\n
  *         AHB3RSTR     QSPIRST       LL_AHB3_GRP1_ReleaseReset\n
  *         AHB3RSTR     OSPI1RST      LL_AHB3_GRP1_ReleaseReset\n
  *         AHB3RSTR     OSPI2RST      LL_AHB3_GRP1_ReleaseReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB2_GRP1_PERIPH_ALL
  *         @arg @ref LL_AHB3_GRP1_PERIPH_FMC (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_QSPI (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI1 (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI2 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB3_GRP1_ReleaseReset(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB3RSTR, Periphs);
}

/**
  * @brief  Enable AHB3 peripheral clocks in Sleep and Stop modes
  * @rmtoll AHB3SMENR    FMCSMEN       LL_AHB3_GRP1_EnableClockStopSleep\n
  *         AHB3SMENR    QSPISMEN      LL_AHB3_GRP1_EnableClockStopSleep\n
  *         AHB3SMENR    OSPI1SMEN     LL_AHB3_GRP1_EnableClockStopSleep\n
  *         AHB3SMENR    OSPI2SMEN     LL_AHB3_GRP1_EnableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB3_GRP1_PERIPH_FMC (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_QSPI (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI1 (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI2 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB3_GRP1_EnableClockStopSleep(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->AHB3SMENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->AHB3SMENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Disable AHB3 peripheral clocks in Sleep and Stop modes
  * @rmtoll AHB3SMENR    FMCSMEN       LL_AHB3_GRP1_DisableClockStopSleep\n
  *         AHB3SMENR    QSPISMEN      LL_AHB3_GRP1_DisableClockStopSleep\n
  *         AHB3SMENR    OSPI1SMEN     LL_AHB3_GRP1_DisableClockStopSleep\n
  *         AHB3SMENR    OSPI2SMEN     LL_AHB3_GRP1_DisableClockStopSleep\n
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_AHB3_GRP1_PERIPH_FMC (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_QSPI (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI1 (*)
  *         @arg @ref LL_AHB3_GRP1_PERIPH_OSPI2 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_AHB3_GRP1_DisableClockStopSleep(uint32_t Periphs)
{
  CLEAR_BIT(RCC->AHB3SMENR, Periphs);
}

/**
  * @}
  */

/** @defgroup BUS_LL_EF_APB1 APB1
  * @{
  */

/**
  * @brief  Enable APB1 peripherals clock.
  * @rmtoll APB1ENR1     TIM2EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     TIM3EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     TIM4EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     TIM5EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     TIM6EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     TIM7EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     LCDEN         LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     RTCAPBEN      LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     WWDGEN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     SPI2EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     SPI3EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     USART2EN      LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     USART3EN      LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     UART4EN       LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     UART5EN       LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     I2C1EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     I2C2EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     I2C3EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     CRSEN         LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     CAN1EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     USBFSEN       LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     CAN2EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     PWREN         LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     DAC1EN        LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     OPAMPEN       LL_APB1_GRP1_EnableClock\n
  *         APB1ENR1     LPTIM1EN      LL_APB1_GRP1_EnableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM2
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM6
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM7
  *         @arg @ref LL_APB1_GRP1_PERIPH_LCD (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_RTCAPB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_WWDG
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI3
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART2
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C1
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C3
  *         @arg @ref LL_APB1_GRP1_PERIPH_CRS (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN1
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_USB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_PWR
  *         @arg @ref LL_APB1_GRP1_PERIPH_DAC1
  *         @arg @ref LL_APB1_GRP1_PERIPH_OPAMP
  *         @arg @ref LL_APB1_GRP1_PERIPH_LPTIM1
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP1_EnableClock(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->APB1ENR1, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->APB1ENR1, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Enable APB1 peripherals clock.
  * @rmtoll APB1ENR2     LPUART1EN     LL_APB1_GRP2_EnableClock\n
  *         APB1ENR2     I2C4EN        LL_APB1_GRP2_EnableClock\n
  *         APB1ENR2     SWPMI1EN      LL_APB1_GRP2_EnableClock\n
  *         APB1ENR2     LPTIM2EN      LL_APB1_GRP2_EnableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPUART1
  *         @arg @ref LL_APB1_GRP2_PERIPH_I2C4 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_SWPMI1 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPTIM2
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP2_EnableClock(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->APB1ENR2, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->APB1ENR2, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Check if APB1 peripheral clock is enabled or not
  * @rmtoll APB1ENR1     TIM2EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     TIM3EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     TIM4EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     TIM5EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     TIM6EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     TIM7EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     LCDEN         LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     RTCAPBEN      LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     WWDGEN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     SPI2EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     SPI3EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     USART2EN      LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     USART3EN      LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     UART4EN       LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     UART5EN       LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     I2C1EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     I2C2EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     I2C3EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     CRSEN         LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     CAN1EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     USBFSEN       LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     CAN2EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     PWREN         LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     DAC1EN        LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     OPAMPEN       LL_APB1_GRP1_IsEnabledClock\n
  *         APB1ENR1     LPTIM1EN      LL_APB1_GRP1_IsEnabledClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM2
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM6
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM7
  *         @arg @ref LL_APB1_GRP1_PERIPH_LCD (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_RTCAPB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_WWDG
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI3
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART2
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C1
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C3
  *         @arg @ref LL_APB1_GRP1_PERIPH_CRS (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN1
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_USB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_PWR
  *         @arg @ref LL_APB1_GRP1_PERIPH_DAC1
  *         @arg @ref LL_APB1_GRP1_PERIPH_OPAMP
  *         @arg @ref LL_APB1_GRP1_PERIPH_LPTIM1
  *
  *         (*) value not defined in all devices.
  * @retval State of Periphs (1 or 0).
*/
__STATIC_INLINE uint32_t LL_APB1_GRP1_IsEnabledClock(uint32_t Periphs)
{
  return ((READ_BIT(RCC->APB1ENR1, Periphs) == Periphs) ? 1UL : 0UL);
}

/**
  * @brief  Check if APB1 peripheral clock is enabled or not
  * @rmtoll APB1ENR2     LPUART1EN     LL_APB1_GRP2_IsEnabledClock\n
  *         APB1ENR2     I2C4EN        LL_APB1_GRP2_IsEnabledClock\n
  *         APB1ENR2     SWPMI1EN      LL_APB1_GRP2_IsEnabledClock\n
  *         APB1ENR2     LPTIM2EN      LL_APB1_GRP2_IsEnabledClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPUART1
  *         @arg @ref LL_APB1_GRP2_PERIPH_I2C4 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_SWPMI1 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPTIM2
  *
  *         (*) value not defined in all devices.
  * @retval State of Periphs (1 or 0).
*/
__STATIC_INLINE uint32_t LL_APB1_GRP2_IsEnabledClock(uint32_t Periphs)
{
  return ((READ_BIT(RCC->APB1ENR2, Periphs) == Periphs) ? 1UL : 0UL);
}

/**
  * @brief  Disable APB1 peripherals clock.
  * @rmtoll APB1ENR1     TIM2EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     TIM3EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     TIM4EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     TIM5EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     TIM6EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     TIM7EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     LCDEN         LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     RTCAPBEN      LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     WWDGEN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     SPI2EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     SPI3EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     USART2EN      LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     USART3EN      LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     UART4EN       LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     UART5EN       LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     I2C1EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     I2C2EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     I2C3EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     CRSEN         LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     CAN1EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     USBFSEN       LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     CAN2EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     PWREN         LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     DAC1EN        LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     OPAMPEN       LL_APB1_GRP1_DisableClock\n
  *         APB1ENR1     LPTIM1EN      LL_APB1_GRP1_DisableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM2
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM6
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM7
  *         @arg @ref LL_APB1_GRP1_PERIPH_LCD (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_RTCAPB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_WWDG
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI3
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART2
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C1
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C3
  *         @arg @ref LL_APB1_GRP1_PERIPH_CRS (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN1
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_USB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_PWR
  *         @arg @ref LL_APB1_GRP1_PERIPH_DAC1
  *         @arg @ref LL_APB1_GRP1_PERIPH_OPAMP
  *         @arg @ref LL_APB1_GRP1_PERIPH_LPTIM1
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP1_DisableClock(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB1ENR1, Periphs);
}

/**
  * @brief  Disable APB1 peripherals clock.
  * @rmtoll APB1ENR2     LPUART1EN     LL_APB1_GRP2_DisableClock\n
  *         APB1ENR2     I2C4EN        LL_APB1_GRP2_DisableClock\n
  *         APB1ENR2     SWPMI1EN      LL_APB1_GRP2_DisableClock\n
  *         APB1ENR2     LPTIM2EN      LL_APB1_GRP2_DisableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPUART1
  *         @arg @ref LL_APB1_GRP2_PERIPH_I2C4 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_SWPMI1 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPTIM2
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP2_DisableClock(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB1ENR2, Periphs);
}

/**
  * @brief  Force APB1 peripherals reset.
  * @rmtoll APB1RSTR1    TIM2RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    TIM3RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    TIM4RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    TIM5RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    TIM6RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    TIM7RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    LCDRST        LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    SPI2RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    SPI3RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    USART2RST     LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    USART3RST     LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    UART4RST      LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    UART5RST      LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    I2C1RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    I2C2RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    I2C3RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    CRSRST        LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    CAN1RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    USBFSRST      LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    CAN2RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    PWRRST        LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    DAC1RST       LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    OPAMPRST      LL_APB1_GRP1_ForceReset\n
  *         APB1RSTR1    LPTIM1RST     LL_APB1_GRP1_ForceReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP1_PERIPH_ALL
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM2
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM6
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM7
  *         @arg @ref LL_APB1_GRP1_PERIPH_LCD (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI3
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART2
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C1
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C3
  *         @arg @ref LL_APB1_GRP1_PERIPH_CRS (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN1
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_USB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_PWR
  *         @arg @ref LL_APB1_GRP1_PERIPH_DAC1
  *         @arg @ref LL_APB1_GRP1_PERIPH_OPAMP
  *         @arg @ref LL_APB1_GRP1_PERIPH_LPTIM1
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP1_ForceReset(uint32_t Periphs)
{
  SET_BIT(RCC->APB1RSTR1, Periphs);
}

/**
  * @brief  Force APB1 peripherals reset.
  * @rmtoll APB1RSTR2    LPUART1RST    LL_APB1_GRP2_ForceReset\n
  *         APB1RSTR2    I2C4RST       LL_APB1_GRP2_ForceReset\n
  *         APB1RSTR2    SWPMI1RST     LL_APB1_GRP2_ForceReset\n
  *         APB1RSTR2    LPTIM2RST     LL_APB1_GRP2_ForceReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP2_PERIPH_ALL
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPUART1
  *         @arg @ref LL_APB1_GRP2_PERIPH_I2C4 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_SWPMI1 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPTIM2
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP2_ForceReset(uint32_t Periphs)
{
  SET_BIT(RCC->APB1RSTR2, Periphs);
}

/**
  * @brief  Release APB1 peripherals reset.
  * @rmtoll APB1RSTR1    TIM2RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    TIM3RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    TIM4RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    TIM5RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    TIM6RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    TIM7RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    LCDRST        LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    SPI2RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    SPI3RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    USART2RST     LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    USART3RST     LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    UART4RST      LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    UART5RST      LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    I2C1RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    I2C2RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    I2C3RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    CRSRST        LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    CAN1RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    USBFSRST      LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    CAN2RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    PWRRST        LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    DAC1RST       LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    OPAMPRST      LL_APB1_GRP1_ReleaseReset\n
  *         APB1RSTR1    LPTIM1RST     LL_APB1_GRP1_ReleaseReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP1_PERIPH_ALL
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM2
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM6
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM7
  *         @arg @ref LL_APB1_GRP1_PERIPH_LCD (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI3
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART2
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C1
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C3
  *         @arg @ref LL_APB1_GRP1_PERIPH_CRS (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN1
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_USB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_PWR
  *         @arg @ref LL_APB1_GRP1_PERIPH_DAC1
  *         @arg @ref LL_APB1_GRP1_PERIPH_OPAMP
  *         @arg @ref LL_APB1_GRP1_PERIPH_LPTIM1
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP1_ReleaseReset(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB1RSTR1, Periphs);
}

/**
  * @brief  Release APB1 peripherals reset.
  * @rmtoll APB1RSTR2    LPUART1RST    LL_APB1_GRP2_ReleaseReset\n
  *         APB1RSTR2    I2C4RST       LL_APB1_GRP2_ReleaseReset\n
  *         APB1RSTR2    SWPMI1RST     LL_APB1_GRP2_ReleaseReset\n
  *         APB1RSTR2    LPTIM2RST     LL_APB1_GRP2_ReleaseReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP2_PERIPH_ALL
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPUART1
  *         @arg @ref LL_APB1_GRP2_PERIPH_I2C4 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_SWPMI1 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPTIM2
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP2_ReleaseReset(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB1RSTR2, Periphs);
}

/**
  * @brief  Enable APB1 peripheral clocks in Sleep and Stop modes
  * @rmtoll APB1SMENR1   TIM2SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   TIM3SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   TIM4SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   TIM5SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   TIM6SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   TIM7SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   LCDSMEN       LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   RTCAPBSMEN    LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   WWDGSMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   SPI2SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   SPI3SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   USART2SMEN    LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   USART3SMEN    LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   UART4SMEN     LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   UART5SMEN     LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   I2C1SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   I2C2SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   I2C3SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   CRSSMEN       LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   CAN1SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   USBFSSMEN     LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   CAN2SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   PWRSMEN       LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   DAC1SMEN      LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   OPAMPSMEN     LL_APB1_GRP1_EnableClockStopSleep\n
  *         APB1SMENR1   LPTIM1SMEN    LL_APB1_GRP1_EnableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM2
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM6
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM7
  *         @arg @ref LL_APB1_GRP1_PERIPH_LCD (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_RTCAPB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_WWDG
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI3
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART2
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C1
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C3
  *         @arg @ref LL_APB1_GRP1_PERIPH_CRS (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN1
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_USB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_PWR
  *         @arg @ref LL_APB1_GRP1_PERIPH_DAC1
  *         @arg @ref LL_APB1_GRP1_PERIPH_OPAMP
  *         @arg @ref LL_APB1_GRP1_PERIPH_LPTIM1
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP1_EnableClockStopSleep(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->APB1SMENR1, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->APB1SMENR1, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Enable APB1 peripheral clocks in Sleep and Stop modes
  * @rmtoll APB1SMENR2   LPUART1SMEN   LL_APB1_GRP2_EnableClockStopSleep\n
  *         APB1SMENR2   I2C4SMEN      LL_APB1_GRP2_EnableClockStopSleep\n
  *         APB1SMENR2   SWPMI1SMEN    LL_APB1_GRP2_EnableClockStopSleep\n
  *         APB1SMENR2   LPTIM2SMEN    LL_APB1_GRP2_EnableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPUART1
  *         @arg @ref LL_APB1_GRP2_PERIPH_I2C4 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_SWPMI1 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPTIM2
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP2_EnableClockStopSleep(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->APB1SMENR2, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->APB1SMENR2, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Disable APB1 peripheral clocks in Sleep and Stop modes
  * @rmtoll APB1SMENR1   TIM2SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   TIM3SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   TIM4SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   TIM5SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   TIM6SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   TIM7SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   LCDSMEN       LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   RTCAPBSMEN    LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   WWDGSMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   SPI2SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   SPI3SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   USART2SMEN    LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   USART3SMEN    LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   UART4SMEN     LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   UART5SMEN     LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   I2C1SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   I2C2SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   I2C3SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   CRSSMEN       LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   CAN1SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   USBFSSMEN     LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   CAN2SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   PWRSMEN       LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   DAC1SMEN      LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   OPAMPSMEN     LL_APB1_GRP1_DisableClockStopSleep\n
  *         APB1SMENR1   LPTIM1SMEN    LL_APB1_GRP1_DisableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM2
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM6
  *         @arg @ref LL_APB1_GRP1_PERIPH_TIM7
  *         @arg @ref LL_APB1_GRP1_PERIPH_LCD (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_RTCAPB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_WWDG
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_SPI3
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART2
  *         @arg @ref LL_APB1_GRP1_PERIPH_USART3 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART4 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_UART5 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C1
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_I2C3
  *         @arg @ref LL_APB1_GRP1_PERIPH_CRS (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN1
  *         @arg @ref LL_APB1_GRP1_PERIPH_CAN2 (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_USB (*)
  *         @arg @ref LL_APB1_GRP1_PERIPH_PWR
  *         @arg @ref LL_APB1_GRP1_PERIPH_DAC1
  *         @arg @ref LL_APB1_GRP1_PERIPH_OPAMP
  *         @arg @ref LL_APB1_GRP1_PERIPH_LPTIM1
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP1_DisableClockStopSleep(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB1SMENR1, Periphs);
}

/**
  * @brief  Disable APB1 peripheral clocks in Sleep and Stop modes
  * @rmtoll APB1SMENR2   LPUART1SMEN   LL_APB1_GRP2_DisableClockStopSleep\n
  *         APB1SMENR2   I2C4SMEN      LL_APB1_GRP2_DisableClockStopSleep\n
  *         APB1SMENR2   SWPMI1SMEN    LL_APB1_GRP2_DisableClockStopSleep\n
  *         APB1SMENR2   LPTIM2SMEN    LL_APB1_GRP2_DisableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPUART1
  *         @arg @ref LL_APB1_GRP2_PERIPH_I2C4 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_SWPMI1 (*)
  *         @arg @ref LL_APB1_GRP2_PERIPH_LPTIM2
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB1_GRP2_DisableClockStopSleep(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB1SMENR2, Periphs);
}

/**
  * @}
  */

/** @defgroup BUS_LL_EF_APB2 APB2
  * @{
  */

/**
  * @brief  Enable APB2 peripherals clock.
  * @rmtoll APB2ENR      SYSCFGEN      LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      FWEN          LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      SDMMC1EN      LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      TIM1EN        LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      SPI1EN        LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      TIM8EN        LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      USART1EN      LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      TIM15EN       LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      TIM16EN       LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      TIM17EN       LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      SAI1EN        LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      SAI2EN        LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      DFSDM1EN      LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      LTDCEN        LL_APB2_GRP1_EnableClock\n
  *         APB2ENR      DSIEN         LL_APB2_GRP1_EnableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB2_GRP1_PERIPH_SYSCFG
  *         @arg @ref LL_APB2_GRP1_PERIPH_FW
  *         @arg @ref LL_APB2_GRP1_PERIPH_SDMMC1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SPI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM8 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_USART1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM15
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM16
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM17 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI2 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DFSDM1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_LTDC (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB2_GRP1_EnableClock(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->APB2ENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->APB2ENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Check if APB2 peripheral clock is enabled or not
  * @rmtoll APB2ENR      SYSCFGEN      LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      FWEN          LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      SDMMC1EN      LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      TIM1EN        LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      SPI1EN        LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      TIM8EN        LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      USART1EN      LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      TIM15EN       LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      TIM16EN       LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      TIM17EN       LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      SAI1EN        LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      SAI2EN        LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      DFSDM1EN      LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      LTDCEN        LL_APB2_GRP1_IsEnabledClock\n
  *         APB2ENR      DSIEN         LL_APB2_GRP1_IsEnabledClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB2_GRP1_PERIPH_SYSCFG
  *         @arg @ref LL_APB2_GRP1_PERIPH_FW
  *         @arg @ref LL_APB2_GRP1_PERIPH_SDMMC1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SPI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM8 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_USART1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM15
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM16
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM17 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI2 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DFSDM1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_LTDC (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval State of Periphs (1 or 0).
*/
__STATIC_INLINE uint32_t LL_APB2_GRP1_IsEnabledClock(uint32_t Periphs)
{
  return ((READ_BIT(RCC->APB2ENR, Periphs) == Periphs) ? 1UL : 0UL);
}

/**
  * @brief  Disable APB2 peripherals clock.
  * @rmtoll APB2ENR      SYSCFGEN      LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      SDMMC1EN      LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      TIM1EN        LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      SPI1EN        LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      TIM8EN        LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      USART1EN      LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      TIM15EN       LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      TIM16EN       LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      TIM17EN       LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      SAI1EN        LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      SAI2EN        LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      DFSDM1EN      LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      LTDCEN        LL_APB2_GRP1_DisableClock\n
  *         APB2ENR      DSIEN         LL_APB2_GRP1_DisableClock
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB2_GRP1_PERIPH_SYSCFG
  *         @arg @ref LL_APB2_GRP1_PERIPH_SDMMC1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SPI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM8 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_USART1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM15
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM16
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM17 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI2 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DFSDM1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_LTDC (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB2_GRP1_DisableClock(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB2ENR, Periphs);
}

/**
  * @brief  Force APB2 peripherals reset.
  * @rmtoll APB2RSTR     SYSCFGRST     LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     SDMMC1RST     LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     TIM1RST       LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     SPI1RST       LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     TIM8RST       LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     USART1RST     LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     TIM15RST      LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     TIM16RST      LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     TIM17RST      LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     SAI1RST       LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     SAI2RST       LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     DFSDM1RST     LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     LTDCRST       LL_APB2_GRP1_ForceReset\n
  *         APB2RSTR     DSIRST        LL_APB2_GRP1_ForceReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB2_GRP1_PERIPH_ALL
  *         @arg @ref LL_APB2_GRP1_PERIPH_SYSCFG
  *         @arg @ref LL_APB2_GRP1_PERIPH_SDMMC1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SPI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM8 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_USART1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM15
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM16
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM17 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI2 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DFSDM1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_LTDC (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB2_GRP1_ForceReset(uint32_t Periphs)
{
  SET_BIT(RCC->APB2RSTR, Periphs);
}

/**
  * @brief  Release APB2 peripherals reset.
  * @rmtoll APB2RSTR     SYSCFGRST     LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     SDMMC1RST     LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     TIM1RST       LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     SPI1RST       LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     TIM8RST       LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     USART1RST     LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     TIM15RST      LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     TIM16RST      LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     TIM17RST      LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     SAI1RST       LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     SAI2RST       LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     DFSDM1RST     LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     LTDCRST       LL_APB2_GRP1_ReleaseReset\n
  *         APB2RSTR     DSIRST        LL_APB2_GRP1_ReleaseReset
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB2_GRP1_PERIPH_ALL
  *         @arg @ref LL_APB2_GRP1_PERIPH_SYSCFG
  *         @arg @ref LL_APB2_GRP1_PERIPH_SDMMC1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SPI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM8 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_USART1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM15
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM16
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM17 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI2 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DFSDM1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_LTDC (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB2_GRP1_ReleaseReset(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB2RSTR, Periphs);
}

/**
  * @brief  Enable APB2 peripheral clocks in Sleep and Stop modes
  * @rmtoll APB2SMENR    SYSCFGSMEN    LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    SDMMC1SMEN    LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    TIM1SMEN      LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    SPI1SMEN      LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    TIM8SMEN      LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    USART1SMEN    LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    TIM15SMEN     LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    TIM16SMEN     LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    TIM17SMEN     LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    SAI1SMEN      LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    SAI2SMEN      LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    DFSDM1SMEN    LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    LTDCSMEN      LL_APB2_GRP1_EnableClockStopSleep\n
  *         APB2SMENR    DSISMEN       LL_APB2_GRP1_EnableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB2_GRP1_PERIPH_SYSCFG
  *         @arg @ref LL_APB2_GRP1_PERIPH_SDMMC1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SPI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM8 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_USART1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM15
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM16
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM17 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI2 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DFSDM1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_LTDC (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB2_GRP1_EnableClockStopSleep(uint32_t Periphs)
{
  __IO uint32_t tmpreg;
  SET_BIT(RCC->APB2SMENR, Periphs);
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->APB2SMENR, Periphs);
  (void)tmpreg;
}

/**
  * @brief  Disable APB2 peripheral clocks in Sleep and Stop modes
  * @rmtoll APB2SMENR    SYSCFGSMEN    LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    SDMMC1SMEN    LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    TIM1SMEN      LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    SPI1SMEN      LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    TIM8SMEN      LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    USART1SMEN    LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    TIM15SMEN     LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    TIM16SMEN     LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    TIM17SMEN     LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    SAI1SMEN      LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    SAI2SMEN      LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    DFSDM1SMEN    LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    LTDCSMEN      LL_APB2_GRP1_DisableClockStopSleep\n
  *         APB2SMENR    DSISMEN       LL_APB2_GRP1_DisableClockStopSleep
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_APB2_GRP1_PERIPH_SYSCFG
  *         @arg @ref LL_APB2_GRP1_PERIPH_SDMMC1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SPI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM8 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_USART1
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM15
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM16
  *         @arg @ref LL_APB2_GRP1_PERIPH_TIM17 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI1
  *         @arg @ref LL_APB2_GRP1_PERIPH_SAI2 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DFSDM1 (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_LTDC (*)
  *         @arg @ref LL_APB2_GRP1_PERIPH_DSI (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
*/
__STATIC_INLINE void LL_APB2_GRP1_DisableClockStopSleep(uint32_t Periphs)
{
  CLEAR_BIT(RCC->APB2SMENR, Periphs);
}

/**
  * @}
  */


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

#endif /* STM32L4xx_LL_BUS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
