/**
  ******************************************************************************
  * @file    stm32l4xx_hal_rcc.c
  * @author  MCD Application Team
  * @brief   RCC HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Reset and Clock Control (RCC) peripheral:
  *           + Initialization and de-initialization functions
  *           + Peripheral Control functions
  *
  @verbatim
  ==============================================================================
                      ##### RCC specific features #####
  ==============================================================================
    [..]
      After reset the device is running from Multiple Speed Internal oscillator
      (4 MHz) with Flash 0 wait state. Flash prefetch buffer, D-Cache
      and I-Cache are disabled, and all peripherals are off except internal
      SRAM, Flash and JTAG.

      (+) There is no prescaler on High speed (AHBs) and Low speed (APBs) busses:
          all peripherals mapped on these busses are running at MSI speed.
      (+) The clock for all peripherals is switched off, except the SRAM and FLASH.
      (+) All GPIOs are in analog mode, except the JTAG pins which
          are assigned to be used for debug purpose.

    [..]
      Once the device started from reset, the user application has to:
      (+) Configure the clock source to be used to drive the System clock
          (if the application needs higher frequency/performance)
      (+) Configure the System clock frequency and Flash settings
      (+) Configure the AHB and APB busses prescalers
      (+) Enable the clock for the peripheral(s) to be used
      (+) Configure the clock source(s) for peripherals which clocks are not
          derived from the System clock (SAIx, RTC, ADC, USB OTG FS/SDMMC1/RNG)

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

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup RCC RCC
  * @brief RCC HAL module driver
  * @{
  */

#ifdef HAL_RCC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup RCC_Private_Constants RCC Private Constants
 * @{
 */
#define HSE_TIMEOUT_VALUE          HSE_STARTUP_TIMEOUT
#define HSI_TIMEOUT_VALUE          2U    /* 2 ms (minimum Tick + 1) */
#define MSI_TIMEOUT_VALUE          2U    /* 2 ms (minimum Tick + 1) */
#if defined(RCC_CSR_LSIPREDIV)
#define LSI_TIMEOUT_VALUE          17U   /* 17 ms (16 ms starting time + 1) */
#else
#define LSI_TIMEOUT_VALUE          2U    /* 2 ms (minimum Tick + 1) */
#endif /* RCC_CSR_LSIPREDIV */
#define HSI48_TIMEOUT_VALUE        2U    /* 2 ms (minimum Tick + 1) */
#define PLL_TIMEOUT_VALUE          2U    /* 2 ms (minimum Tick + 1) */
#define CLOCKSWITCH_TIMEOUT_VALUE  5000U /* 5 s    */
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/** @defgroup RCC_Private_Macros RCC Private Macros
  * @{
  */
#define __MCO1_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define MCO1_GPIO_PORT        GPIOA
#define MCO1_PIN              GPIO_PIN_8

#define RCC_PLL_OSCSOURCE_CONFIG(__HAL_RCC_PLLSOURCE__) \
            (MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, (__HAL_RCC_PLLSOURCE__)))
/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/** @defgroup RCC_Private_Functions RCC Private Functions
  * @{
  */
static HAL_StatusTypeDef RCC_SetFlashLatencyFromMSIRange(uint32_t msirange);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
static uint32_t          RCC_GetSysClockFreqFromPLLSource(void);
#endif
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup RCC_Exported_Functions RCC Exported Functions
  * @{
  */

/** @defgroup RCC_Exported_Functions_Group1 Initialization and de-initialization functions
  *  @brief    Initialization and Configuration functions
  *
  @verbatim
 ===============================================================================
           ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]
      This section provides functions allowing to configure the internal and external oscillators
      (HSE, HSI, LSE, MSI, LSI, PLL, CSS and MCO) and the System busses clocks (SYSCLK, AHB, APB1
       and APB2).

    [..] Internal/external clock and PLL configuration
         (+) HSI (high-speed internal): 16 MHz factory-trimmed RC used directly or through
             the PLL as System clock source.

         (+) MSI (Mutiple Speed Internal): Its frequency is software trimmable from 100KHZ to 48MHZ.
             It can be used to generate the clock for the USB OTG FS (48 MHz).
             The number of flash wait states is automatically adjusted when MSI range is updated with
             HAL_RCC_OscConfig() and the MSI is used as System clock source.

         (+) LSI (low-speed internal): 32 KHz low consumption RC used as IWDG and/or RTC
             clock source.

         (+) HSE (high-speed external): 4 to 48 MHz crystal oscillator used directly or
             through the PLL as System clock source. Can be used also optionally as RTC clock source.

         (+) LSE (low-speed external): 32.768 KHz oscillator used optionally as RTC clock source.

         (+) PLL (clocked by HSI, HSE or MSI) providing up to three independent output clocks:
           (++) The first output is used to generate the high speed system clock (up to 80MHz).
           (++) The second output is used to generate the clock for the USB OTG FS (48 MHz),
                the random analog generator (<=48 MHz) and the SDMMC1 (<= 48 MHz).
           (++) The third output is used to generate an accurate clock to achieve
                high-quality audio performance on SAI interface.

         (+) PLLSAI1 (clocked by HSI, HSE or MSI) providing up to three independent output clocks:
           (++) The first output is used to generate SAR ADC1 clock.
           (++) The second output is used to generate the clock for the USB OTG FS (48 MHz),
                the random analog generator (<=48 MHz) and the SDMMC1 (<= 48 MHz).
           (++) The Third output is used to generate an accurate clock to achieve
                high-quality audio performance on SAI interface.

         (+) PLLSAI2 (clocked by HSI, HSE or MSI) providing up to two independent output clocks:
           (++) The first output is used to generate SAR ADC2 clock.
           (++) The second  output is used to generate an accurate clock to achieve
                high-quality audio performance on SAI interface.

         (+) CSS (Clock security system): once enabled, if a HSE clock failure occurs
            (HSE used directly or through PLL as System clock source), the System clock
             is automatically switched to HSI and an interrupt is generated if enabled.
             The interrupt is linked to the Cortex-M4 NMI (Non-Maskable Interrupt)
             exception vector.

         (+) MCO (microcontroller clock output): used to output MSI, LSI, HSI, LSE, HSE or
             main PLL clock (through a configurable prescaler) on PA8 pin.

    [..] System, AHB and APB busses clocks configuration
         (+) Several clock sources can be used to drive the System clock (SYSCLK): MSI, HSI,
             HSE and main PLL.
             The AHB clock (HCLK) is derived from System clock through configurable
             prescaler and used to clock the CPU, memory and peripherals mapped
             on AHB bus (DMA, GPIO...). APB1 (PCLK1) and APB2 (PCLK2) clocks are derived
             from AHB clock through configurable prescalers and used to clock
             the peripherals mapped on these busses. You can use
             "HAL_RCC_GetSysClockFreq()" function to retrieve the frequencies of these clocks.

         -@- All the peripheral clocks are derived from the System clock (SYSCLK) except:

           (+@) SAI: the SAI clock can be derived either from a specific PLL (PLLSAI1) or (PLLSAI2) or
                from an external clock mapped on the SAI_CKIN pin.
                You have to use HAL_RCCEx_PeriphCLKConfig() function to configure this clock.
           (+@) RTC: the RTC clock can be derived either from the LSI, LSE or HSE clock
                divided by 2 to 31.
                You have to use __HAL_RCC_RTC_ENABLE() and HAL_RCCEx_PeriphCLKConfig() function
                to configure this clock.
           (+@) USB OTG FS, SDMMC1 and RNG: USB OTG FS requires a frequency equal to 48 MHz
                to work correctly, while the SDMMC1 and RNG peripherals require a frequency
                equal or lower than to 48 MHz. This clock is derived of the main PLL or PLLSAI1
                through PLLQ divider. You have to enable the peripheral clock and use
                HAL_RCCEx_PeriphCLKConfig() function to configure this clock.
           (+@) IWDG clock which is always the LSI clock.


         (+) The maximum frequency of the SYSCLK, HCLK, PCLK1 and PCLK2 is 80 MHz.
             The clock source frequency should be adapted depending on the device voltage range
             as listed in the Reference Manual "Clock source frequency versus voltage scaling" chapter.

  @endverbatim

           Table 1. HCLK clock frequency for STM32L4Rx/STM32L4Sx devices
           +--------------------------------------------------------+
           | Latency         |     HCLK clock frequency (MHz)       |
           |                 |--------------------------------------|
           |                 |  voltage range 1  | voltage range 2  |
           |                 |       1.2 V       |     1.0 V        |
           |-----------------|-------------------|------------------|
           |0WS(1 CPU cycles)|   0 < HCLK <= 20  |  0 < HCLK <= 8   |
           |-----------------|-------------------|------------------|
           |1WS(2 CPU cycles)|  20 < HCLK <= 40  |  8 < HCLK <= 16  |
           |-----------------|-------------------|------------------|
           |2WS(3 CPU cycles)|  40 < HCLK <= 60  | 16 < HCLK <= 26  |
           |-----------------|-------------------|------------------|
           |3WS(4 CPU cycles)|  60 < HCLK <= 80  | 16 < HCLK <= 26  |
           |-----------------|-------------------|------------------|
           |4WS(5 CPU cycles)|  80 < HCLK <= 100 | 16 < HCLK <= 26  |
           |-----------------|-------------------|------------------|
           |5WS(6 CPU cycles)| 100 < HCLK <= 120 | 16 < HCLK <= 26  |
           +--------------------------------------------------------+

           Table 2. HCLK clock frequency for other STM32L4 devices
           +-------------------------------------------------------+
           | Latency         |    HCLK clock frequency (MHz)       |
           |                 |-------------------------------------|
           |                 | voltage range 1  | voltage range 2  |
           |                 |      1.2 V       |     1.0 V        |
           |-----------------|------------------|------------------|
           |0WS(1 CPU cycles)|  0 < HCLK <= 16  |  0 < HCLK <= 6   |
           |-----------------|------------------|------------------|
           |1WS(2 CPU cycles)| 16 < HCLK <= 32  |  6 < HCLK <= 12  |
           |-----------------|------------------|------------------|
           |2WS(3 CPU cycles)| 32 < HCLK <= 48  | 12 < HCLK <= 18  |
           |-----------------|------------------|------------------|
           |3WS(4 CPU cycles)| 48 < HCLK <= 64  | 18 < HCLK <= 26  |
           |-----------------|------------------|------------------|
           |4WS(5 CPU cycles)| 64 < HCLK <= 80  | 18 < HCLK <= 26  |
           +-------------------------------------------------------+
  * @{
  */

/**
  * @brief  Reset the RCC clock configuration to the default reset state.
  * @note   The default reset state of the clock configuration is given below:
  *            - MSI ON and used as system clock source
  *            - HSE, HSI, PLL, PLLSAI1 and PLLSAI2 OFF
  *            - AHB, APB1 and APB2 prescalers set to 1.
  *            - CSS, MCO1 OFF
  *            - All interrupts disabled
  *            - All interrupt and reset flags cleared
  * @note   This function does not modify the configuration of the
  *            - Peripheral clock sources
  *            - LSI, LSE and RTC clocks (Backup domain)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCC_DeInit(void)
{
  uint32_t tickstart;

  /* Reset to default System clock */
  /* Set MSION bit */
  SET_BIT(RCC->CR, RCC_CR_MSION);

  /* Insure MSIRDY bit is set before writing default MSIRANGE value */
  /* Get start tick */
  tickstart = HAL_GetTick();

  /* Wait till MSI is ready */
  while(READ_BIT(RCC->CR, RCC_CR_MSIRDY) == 0U)
  {
    if((HAL_GetTick() - tickstart) > MSI_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Set MSIRANGE default value */
  MODIFY_REG(RCC->CR, RCC_CR_MSIRANGE, RCC_MSIRANGE_6);

  /* Reset CFGR register (MSI is selected as system clock source) */
  CLEAR_REG(RCC->CFGR);

  /* Update the SystemCoreClock global variable for MSI as system clock source */
  SystemCoreClock = MSI_VALUE;

  /* Configure the source of time base considering new system clock settings  */
  if(HAL_InitTick(uwTickPrio) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Insure MSI selected as system clock source */
  /* Get start tick */
  tickstart = HAL_GetTick();

  /* Wait till system clock source is ready */
  while(READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_MSI)
  {
    if((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Reset HSION, HSIKERON, HSIASFS, HSEON, HSECSSON, PLLON, PLLSAIxON bits */
#if defined(RCC_PLLSAI2_SUPPORT)

  CLEAR_BIT(RCC->CR, RCC_CR_HSEON | RCC_CR_HSION | RCC_CR_HSIKERON| RCC_CR_HSIASFS | RCC_CR_PLLON | RCC_CR_PLLSAI1ON | RCC_CR_PLLSAI2ON);

#elif defined(RCC_PLLSAI1_SUPPORT)

  CLEAR_BIT(RCC->CR, RCC_CR_HSEON | RCC_CR_HSION | RCC_CR_HSIKERON| RCC_CR_HSIASFS | RCC_CR_PLLON | RCC_CR_PLLSAI1ON);

#else

  CLEAR_BIT(RCC->CR, RCC_CR_HSEON | RCC_CR_HSION | RCC_CR_HSIKERON| RCC_CR_HSIASFS | RCC_CR_PLLON);

#endif /* RCC_PLLSAI2_SUPPORT */

  /* Insure PLLRDY, PLLSAI1RDY and PLLSAI2RDY (if present) are reset */
  /* Get start tick */
  tickstart = HAL_GetTick();

#if defined(RCC_PLLSAI2_SUPPORT)

  while(READ_BIT(RCC->CR, RCC_CR_PLLRDY | RCC_CR_PLLSAI1RDY | RCC_CR_PLLSAI2RDY) != 0U)

#elif defined(RCC_PLLSAI1_SUPPORT)

  while(READ_BIT(RCC->CR, RCC_CR_PLLRDY | RCC_CR_PLLSAI1RDY) != 0U)

#else

  while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)

#endif
  {
    if((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Reset PLLCFGR register */
  CLEAR_REG(RCC->PLLCFGR);
  SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN_4 );

#if defined(RCC_PLLSAI1_SUPPORT)

  /* Reset PLLSAI1CFGR register */
  CLEAR_REG(RCC->PLLSAI1CFGR);
  SET_BIT(RCC->PLLSAI1CFGR,  RCC_PLLSAI1CFGR_PLLSAI1N_4 );

#endif /* RCC_PLLSAI1_SUPPORT */

#if defined(RCC_PLLSAI2_SUPPORT)

  /* Reset PLLSAI2CFGR register */
  CLEAR_REG(RCC->PLLSAI2CFGR);
  SET_BIT(RCC->PLLSAI2CFGR,  RCC_PLLSAI2CFGR_PLLSAI2N_4 );

#endif /* RCC_PLLSAI2_SUPPORT */

  /* Reset HSEBYP bit */
  CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);

  /* Disable all interrupts */
  CLEAR_REG(RCC->CIER);

  /* Clear all interrupt flags */
  WRITE_REG(RCC->CICR, 0xFFFFFFFFU);

  /* Clear all reset flags */
  SET_BIT(RCC->CSR, RCC_CSR_RMVF);

  return HAL_OK;
}

/**
  * @brief  Initialize the RCC Oscillators according to the specified parameters in the
  *         RCC_OscInitTypeDef.
  * @param  RCC_OscInitStruct  pointer to an RCC_OscInitTypeDef structure that
  *         contains the configuration information for the RCC Oscillators.
  * @note   The PLL is not disabled when used as system clock.
  * @note   Transitions LSE Bypass to LSE On and LSE On to LSE Bypass are not
  *         supported by this macro. User should request a transition to LSE Off
  *         first and then LSE On or LSE Bypass.
  * @note   Transition HSE Bypass to HSE On and HSE On to HSE Bypass are not
  *         supported by this macro. User should request a transition to HSE Off
  *         first and then HSE On or HSE Bypass.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef  *RCC_OscInitStruct)
{
  uint32_t tickstart;
  HAL_StatusTypeDef status;
  uint32_t sysclk_source, pll_config;

  /* Check Null pointer */
  if(RCC_OscInitStruct == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RCC_OSCILLATORTYPE(RCC_OscInitStruct->OscillatorType));

  sysclk_source = __HAL_RCC_GET_SYSCLK_SOURCE();
  pll_config = __HAL_RCC_GET_PLL_OSCSOURCE();

  /*----------------------------- MSI Configuration --------------------------*/
  if(((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_MSI) == RCC_OSCILLATORTYPE_MSI)
  {
    /* Check the parameters */
    assert_param(IS_RCC_MSI(RCC_OscInitStruct->MSIState));
    assert_param(IS_RCC_MSICALIBRATION_VALUE(RCC_OscInitStruct->MSICalibrationValue));
    assert_param(IS_RCC_MSI_CLOCK_RANGE(RCC_OscInitStruct->MSIClockRange));

    /* Check if MSI is used as system clock or as PLL source when PLL is selected as system clock */
    if((sysclk_source == RCC_CFGR_SWS_MSI) ||
       ((sysclk_source == RCC_CFGR_SWS_PLL) && (pll_config == RCC_PLLSOURCE_MSI)))
    {
      if((READ_BIT(RCC->CR, RCC_CR_MSIRDY) != 0U) && (RCC_OscInitStruct->MSIState == RCC_MSI_OFF))
      {
        return HAL_ERROR;
      }

       /* Otherwise, just the calibration and MSI range change are allowed */
      else
      {
        /* To correctly read data from FLASH memory, the number of wait states (LATENCY)
           must be correctly programmed according to the frequency of the CPU clock
           (HCLK) and the supply voltage of the device. */
        if(RCC_OscInitStruct->MSIClockRange > __HAL_RCC_GET_MSI_RANGE())
        {
          /* First increase number of wait states update if necessary */
          if(RCC_SetFlashLatencyFromMSIRange(RCC_OscInitStruct->MSIClockRange) != HAL_OK)
          {
            return HAL_ERROR;
          }

          /* Selects the Multiple Speed oscillator (MSI) clock range .*/
          __HAL_RCC_MSI_RANGE_CONFIG(RCC_OscInitStruct->MSIClockRange);
          /* Adjusts the Multiple Speed oscillator (MSI) calibration value.*/
          __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->MSICalibrationValue);
        }
        else
        {
          /* Else, keep current flash latency while decreasing applies */
          /* Selects the Multiple Speed oscillator (MSI) clock range .*/
          __HAL_RCC_MSI_RANGE_CONFIG(RCC_OscInitStruct->MSIClockRange);
          /* Adjusts the Multiple Speed oscillator (MSI) calibration value.*/
          __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->MSICalibrationValue);

          /* Decrease number of wait states update if necessary */
          if(RCC_SetFlashLatencyFromMSIRange(RCC_OscInitStruct->MSIClockRange) != HAL_OK)
          {
            return HAL_ERROR;
          }
        }

        /* Update the SystemCoreClock global variable */
        SystemCoreClock = HAL_RCC_GetSysClockFreq() >> (AHBPrescTable[READ_BIT(RCC->CFGR, RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos] & 0x1FU);

        /* Configure the source of time base considering new system clocks settings*/
        status = HAL_InitTick(uwTickPrio);
        if(status != HAL_OK)
        {
          return status;
        }
      }
    }
    else
    {
      /* Check the MSI State */
      if(RCC_OscInitStruct->MSIState != RCC_MSI_OFF)
      {
        /* Enable the Internal High Speed oscillator (MSI). */
        __HAL_RCC_MSI_ENABLE();

        /* Get timeout */
        tickstart = HAL_GetTick();

        /* Wait till MSI is ready */
        while(READ_BIT(RCC->CR, RCC_CR_MSIRDY) == 0U)
        {
          if((HAL_GetTick() - tickstart) > MSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
         /* Selects the Multiple Speed oscillator (MSI) clock range .*/
        __HAL_RCC_MSI_RANGE_CONFIG(RCC_OscInitStruct->MSIClockRange);
         /* Adjusts the Multiple Speed oscillator (MSI) calibration value.*/
        __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->MSICalibrationValue);

      }
      else
      {
        /* Disable the Internal High Speed oscillator (MSI). */
        __HAL_RCC_MSI_DISABLE();

        /* Get timeout */
        tickstart = HAL_GetTick();

        /* Wait till MSI is ready */
        while(READ_BIT(RCC->CR, RCC_CR_MSIRDY) != 0U)
        {
          if((HAL_GetTick() - tickstart) > MSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }
  /*------------------------------- HSE Configuration ------------------------*/
  if(((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_HSE) == RCC_OSCILLATORTYPE_HSE)
  {
    /* Check the parameters */
    assert_param(IS_RCC_HSE(RCC_OscInitStruct->HSEState));

    /* When the HSE is used as system clock or clock source for PLL in these cases it is not allowed to be disabled */
    if((sysclk_source == RCC_CFGR_SWS_HSE) ||
       ((sysclk_source == RCC_CFGR_SWS_PLL) && (pll_config == RCC_PLLSOURCE_HSE)))
    {
      if((READ_BIT(RCC->CR, RCC_CR_HSERDY) != 0U) && (RCC_OscInitStruct->HSEState == RCC_HSE_OFF))
      {
        return HAL_ERROR;
      }
    }
    else
    {
      /* Set the new HSE configuration ---------------------------------------*/
      __HAL_RCC_HSE_CONFIG(RCC_OscInitStruct->HSEState);

      /* Check the HSE State */
      if(RCC_OscInitStruct->HSEState != RCC_HSE_OFF)
      {
        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSE is ready */
        while(READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0U)
        {
          if((HAL_GetTick() - tickstart) > HSE_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
      else
      {
        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSE is disabled */
        while(READ_BIT(RCC->CR, RCC_CR_HSERDY) != 0U)
        {
          if((HAL_GetTick() - tickstart) > HSE_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }
  /*----------------------------- HSI Configuration --------------------------*/
  if(((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_HSI) == RCC_OSCILLATORTYPE_HSI)
  {
    /* Check the parameters */
    assert_param(IS_RCC_HSI(RCC_OscInitStruct->HSIState));
    assert_param(IS_RCC_HSI_CALIBRATION_VALUE(RCC_OscInitStruct->HSICalibrationValue));

    /* Check if HSI is used as system clock or as PLL source when PLL is selected as system clock */
    if((sysclk_source == RCC_CFGR_SWS_HSI) ||
       ((sysclk_source == RCC_CFGR_SWS_PLL) && (pll_config == RCC_PLLSOURCE_HSI)))
    {
      /* When HSI is used as system clock it will not be disabled */
      if((READ_BIT(RCC->CR, RCC_CR_HSIRDY) != 0U) && (RCC_OscInitStruct->HSIState == RCC_HSI_OFF))
      {
        return HAL_ERROR;
      }
      /* Otherwise, just the calibration is allowed */
      else
      {
        /* Adjusts the Internal High Speed oscillator (HSI) calibration value.*/
        __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->HSICalibrationValue);
      }
    }
    else
    {
      /* Check the HSI State */
      if(RCC_OscInitStruct->HSIState != RCC_HSI_OFF)
      {
        /* Enable the Internal High Speed oscillator (HSI). */
        __HAL_RCC_HSI_ENABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSI is ready */
        while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
        {
          if((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }

        /* Adjusts the Internal High Speed oscillator (HSI) calibration value.*/
        __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->HSICalibrationValue);
      }
      else
      {
        /* Disable the Internal High Speed oscillator (HSI). */
        __HAL_RCC_HSI_DISABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSI is disabled */
        while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) != 0U)
        {
          if((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }
  /*------------------------------ LSI Configuration -------------------------*/
  if(((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_LSI) == RCC_OSCILLATORTYPE_LSI)
  {
    /* Check the parameters */
    assert_param(IS_RCC_LSI(RCC_OscInitStruct->LSIState));

    /* Check the LSI State */
    if(RCC_OscInitStruct->LSIState != RCC_LSI_OFF)
    {
#if defined(RCC_CSR_LSIPREDIV)
      uint32_t csr_temp = RCC->CSR;

      /* Check LSI division factor */
      assert_param(IS_RCC_LSIDIV(RCC_OscInitStruct->LSIDiv));

      if (RCC_OscInitStruct->LSIDiv != (csr_temp & RCC_CSR_LSIPREDIV))
      {
        if (((csr_temp & RCC_CSR_LSIRDY) == RCC_CSR_LSIRDY) && \
            ((csr_temp & RCC_CSR_LSION) != RCC_CSR_LSION))
        {
           /* If LSIRDY is set while LSION is not enabled,
              LSIPREDIV can't be updated  */
          return HAL_ERROR;
        }

        /* Turn off LSI before changing RCC_CSR_LSIPREDIV */
        if ((csr_temp & RCC_CSR_LSION) == RCC_CSR_LSION)
        {
          __HAL_RCC_LSI_DISABLE();

          /* Get Start Tick*/
          tickstart = HAL_GetTick();

          /* Wait till LSI is disabled */
          while(READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) != 0U)
          {
            if((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
            {
              return HAL_TIMEOUT;
            }
          }
        }

        /* Set LSI division factor */
        MODIFY_REG(RCC->CSR, RCC_CSR_LSIPREDIV, RCC_OscInitStruct->LSIDiv);
      }
#endif /* RCC_CSR_LSIPREDIV */

      /* Enable the Internal Low Speed oscillator (LSI). */
      __HAL_RCC_LSI_ENABLE();

      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till LSI is ready */
      while(READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0U)
      {
        if((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }
    else
    {
      /* Disable the Internal Low Speed oscillator (LSI). */
      __HAL_RCC_LSI_DISABLE();

      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till LSI is disabled */
      while(READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) != 0U)
      {
        if((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }
  }
  /*------------------------------ LSE Configuration -------------------------*/
  if(((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_LSE) == RCC_OSCILLATORTYPE_LSE)
  {
    FlagStatus       pwrclkchanged = RESET;

    /* Check the parameters */
    assert_param(IS_RCC_LSE(RCC_OscInitStruct->LSEState));

    /* Update LSE configuration in Backup Domain control register    */
    /* Requires to enable write access to Backup Domain of necessary */
    if(HAL_IS_BIT_CLR(RCC->APB1ENR1, RCC_APB1ENR1_PWREN))
    {
      __HAL_RCC_PWR_CLK_ENABLE();
      pwrclkchanged = SET;
    }

    if(HAL_IS_BIT_CLR(PWR->CR1, PWR_CR1_DBP))
    {
      /* Enable write access to Backup domain */
      SET_BIT(PWR->CR1, PWR_CR1_DBP);

      /* Wait for Backup domain Write protection disable */
      tickstart = HAL_GetTick();

      while(HAL_IS_BIT_CLR(PWR->CR1, PWR_CR1_DBP))
      {
        if((HAL_GetTick() - tickstart) > RCC_DBP_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }

    /* Set the new LSE configuration -----------------------------------------*/
#if defined(RCC_BDCR_LSESYSDIS)
    if((RCC_OscInitStruct->LSEState & RCC_BDCR_LSEON) != 0U)
    {
      /* Set LSESYSDIS bit according to LSE propagation option (enabled or disabled) */
      MODIFY_REG(RCC->BDCR, RCC_BDCR_LSESYSDIS, (RCC_OscInitStruct->LSEState & RCC_BDCR_LSESYSDIS));

      if((RCC_OscInitStruct->LSEState & RCC_BDCR_LSEBYP) != 0U)
      {
        /* LSE oscillator bypass enable */
        SET_BIT(RCC->BDCR, RCC_BDCR_LSEBYP);
        SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
      }
      else
      {
        /* LSE oscillator enable */
        SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
      }
    }
    else
    {
      CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEON);
      CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEBYP);
    }
#else
    __HAL_RCC_LSE_CONFIG(RCC_OscInitStruct->LSEState);
#endif /* RCC_BDCR_LSESYSDIS */

    /* Check the LSE State */
    if(RCC_OscInitStruct->LSEState != RCC_LSE_OFF)
    {
      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till LSE is ready */
      while(READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) == 0U)
      {
        if((HAL_GetTick() - tickstart) > RCC_LSE_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }
    else
    {
      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till LSE is disabled */
      while(READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) != 0U)
      {
        if((HAL_GetTick() - tickstart) > RCC_LSE_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }

#if defined(RCC_BDCR_LSESYSDIS)
      /* By default, stop disabling LSE propagation */
      CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSESYSDIS);
#endif /* RCC_BDCR_LSESYSDIS */
    }

    /* Restore clock configuration if changed */
    if(pwrclkchanged == SET)
    {
      __HAL_RCC_PWR_CLK_DISABLE();
    }
  }
#if defined(RCC_HSI48_SUPPORT)
  /*------------------------------ HSI48 Configuration -----------------------*/
  if(((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_HSI48) == RCC_OSCILLATORTYPE_HSI48)
  {
    /* Check the parameters */
    assert_param(IS_RCC_HSI48(RCC_OscInitStruct->HSI48State));

    /* Check the LSI State */
    if(RCC_OscInitStruct->HSI48State != RCC_HSI48_OFF)
    {
      /* Enable the Internal Low Speed oscillator (HSI48). */
      __HAL_RCC_HSI48_ENABLE();

      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till HSI48 is ready */
      while(READ_BIT(RCC->CRRCR, RCC_CRRCR_HSI48RDY) == 0U)
      {
        if((HAL_GetTick() - tickstart) > HSI48_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }
    else
    {
      /* Disable the Internal Low Speed oscillator (HSI48). */
      __HAL_RCC_HSI48_DISABLE();

      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till HSI48 is disabled */
      while(READ_BIT(RCC->CRRCR, RCC_CRRCR_HSI48RDY) != 0U)
      {
        if((HAL_GetTick() - tickstart) > HSI48_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }
  }
#endif /* RCC_HSI48_SUPPORT */
  /*-------------------------------- PLL Configuration -----------------------*/
  /* Check the parameters */
  assert_param(IS_RCC_PLL(RCC_OscInitStruct->PLL.PLLState));

  if(RCC_OscInitStruct->PLL.PLLState != RCC_PLL_NONE)
  {
    /* Check if the PLL is used as system clock or not */
    if(sysclk_source != RCC_CFGR_SWS_PLL)
    {
      if(RCC_OscInitStruct->PLL.PLLState == RCC_PLL_ON)
      {
        /* Check the parameters */
        assert_param(IS_RCC_PLLSOURCE(RCC_OscInitStruct->PLL.PLLSource));
        assert_param(IS_RCC_PLLM_VALUE(RCC_OscInitStruct->PLL.PLLM));
        assert_param(IS_RCC_PLLN_VALUE(RCC_OscInitStruct->PLL.PLLN));
#if defined(RCC_PLLP_SUPPORT)
        assert_param(IS_RCC_PLLP_VALUE(RCC_OscInitStruct->PLL.PLLP));
#endif /* RCC_PLLP_SUPPORT */
        assert_param(IS_RCC_PLLQ_VALUE(RCC_OscInitStruct->PLL.PLLQ));
        assert_param(IS_RCC_PLLR_VALUE(RCC_OscInitStruct->PLL.PLLR));

        /* Disable the main PLL. */
        __HAL_RCC_PLL_DISABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till PLL is ready */
        while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)
        {
          if((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }

        /* Configure the main PLL clock source, multiplication and division factors. */
        __HAL_RCC_PLL_CONFIG(RCC_OscInitStruct->PLL.PLLSource,
                             RCC_OscInitStruct->PLL.PLLM,
                             RCC_OscInitStruct->PLL.PLLN,
#if defined(RCC_PLLP_SUPPORT)
                             RCC_OscInitStruct->PLL.PLLP,
#endif
                             RCC_OscInitStruct->PLL.PLLQ,
                             RCC_OscInitStruct->PLL.PLLR);

        /* Enable the main PLL. */
        __HAL_RCC_PLL_ENABLE();

        /* Enable PLL System Clock output. */
        __HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL_SYSCLK);

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till PLL is ready */
        while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0U)
        {
          if((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
      else
      {
        /* Disable the main PLL. */
        __HAL_RCC_PLL_DISABLE();

        /* Disable all PLL outputs to save power if no PLLs on */
#if defined(RCC_PLLSAI1_SUPPORT) && defined(RCC_CR_PLLSAI2RDY)
        if(READ_BIT(RCC->CR, (RCC_CR_PLLSAI1RDY | RCC_CR_PLLSAI2RDY)) == 0U)
        {
          MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, RCC_PLLSOURCE_NONE);
        }
#elif defined(RCC_PLLSAI1_SUPPORT)
        if(READ_BIT(RCC->CR, RCC_CR_PLLSAI1RDY) == 0U)
        {
          MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, RCC_PLLSOURCE_NONE);
        }
#else
        MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, RCC_PLLSOURCE_NONE);
#endif /* RCC_PLLSAI1_SUPPORT && RCC_CR_PLLSAI2RDY */

#if defined(RCC_PLLSAI2_SUPPORT)
        __HAL_RCC_PLLCLKOUT_DISABLE(RCC_PLL_SYSCLK | RCC_PLL_48M1CLK | RCC_PLL_SAI3CLK);
#elif defined(RCC_PLLSAI1_SUPPORT)
        __HAL_RCC_PLLCLKOUT_DISABLE(RCC_PLL_SYSCLK | RCC_PLL_48M1CLK | RCC_PLL_SAI2CLK);
#else
        __HAL_RCC_PLLCLKOUT_DISABLE(RCC_PLL_SYSCLK | RCC_PLL_48M1CLK);
#endif /* RCC_PLLSAI2_SUPPORT */

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till PLL is disabled */
        while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)
        {
          if((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
    else
    {
      /* Check if there is a request to disable the PLL used as System clock source */
      if((RCC_OscInitStruct->PLL.PLLState) == RCC_PLL_OFF)
      {
        return HAL_ERROR;
      }
      else
      {
        pll_config = RCC->PLLCFGR;
        /* Do not return HAL_ERROR if request repeats the current configuration */
        if((READ_BIT(pll_config, RCC_PLLCFGR_PLLSRC)  != RCC_OscInitStruct->PLL.PLLSource) ||
           (READ_BIT(pll_config, RCC_PLLCFGR_PLLM)    != ((RCC_OscInitStruct->PLL.PLLM - 1U) << RCC_PLLCFGR_PLLM_Pos)) ||
           (READ_BIT(pll_config, RCC_PLLCFGR_PLLN)    != (RCC_OscInitStruct->PLL.PLLN << RCC_PLLCFGR_PLLN_Pos)) ||
#if defined(RCC_PLLP_SUPPORT)
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
           (READ_BIT(pll_config, RCC_PLLCFGR_PLLPDIV) != (RCC_OscInitStruct->PLL.PLLP << RCC_PLLCFGR_PLLPDIV_Pos)) ||
#else
           (READ_BIT(pll_config, RCC_PLLCFGR_PLLP)    != ((RCC_OscInitStruct->PLL.PLLP == RCC_PLLP_DIV7) ? 0U : 1U)) ||
#endif
#endif
           (READ_BIT(pll_config, RCC_PLLCFGR_PLLQ)    != ((((RCC_OscInitStruct->PLL.PLLQ) >> 1U) - 1U) << RCC_PLLCFGR_PLLQ_Pos)) ||
           (READ_BIT(pll_config, RCC_PLLCFGR_PLLR)    != ((((RCC_OscInitStruct->PLL.PLLR) >> 1U) - 1U) << RCC_PLLCFGR_PLLR_Pos)))
        {
          return HAL_ERROR;
        }
      }
    }
  }
  return HAL_OK;
}

/**
  * @brief  Initialize the CPU, AHB and APB busses clocks according to the specified
  *         parameters in the RCC_ClkInitStruct.
  * @param  RCC_ClkInitStruct  pointer to an RCC_OscInitTypeDef structure that
  *         contains the configuration information for the RCC peripheral.
  * @param  FLatency  FLASH Latency
  *          This parameter can be one of the following values:
  *            @arg FLASH_LATENCY_0   FLASH 0 Latency cycle
  *            @arg FLASH_LATENCY_1   FLASH 1 Latency cycle
  *            @arg FLASH_LATENCY_2   FLASH 2 Latency cycles
  *            @arg FLASH_LATENCY_3   FLASH 3 Latency cycles
  *            @arg FLASH_LATENCY_4   FLASH 4 Latency cycles
  @if STM32L4S9xx
  *            @arg FLASH_LATENCY_5   FLASH 5 Latency cycles
  *            @arg FLASH_LATENCY_6   FLASH 6 Latency cycles
  *            @arg FLASH_LATENCY_7   FLASH 7 Latency cycles
  *            @arg FLASH_LATENCY_8   FLASH 8 Latency cycles
  *            @arg FLASH_LATENCY_9   FLASH 9 Latency cycles
  *            @arg FLASH_LATENCY_10  FLASH 10 Latency cycles
  *            @arg FLASH_LATENCY_11  FLASH 11 Latency cycles
  *            @arg FLASH_LATENCY_12  FLASH 12 Latency cycles
  *            @arg FLASH_LATENCY_13  FLASH 13 Latency cycles
  *            @arg FLASH_LATENCY_14  FLASH 14 Latency cycles
  *            @arg FLASH_LATENCY_15  FLASH 15 Latency cycles
  @endif
  *
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency
  *         and updated by HAL_RCC_GetHCLKFreq() function called within this function
  *
  * @note   The MSI is used by default as system clock source after
  *         startup from Reset, wake-up from STANDBY mode. After restart from Reset,
  *         the MSI frequency is set to its default value 4 MHz.
  *
  * @note   The HSI can be selected as system clock source after
  *         from STOP modes or in case of failure of the HSE used directly or indirectly
  *         as system clock (if the Clock Security System CSS is enabled).
  *
  * @note   A switch from one clock source to another occurs only if the target
  *         clock source is ready (clock stable after startup delay or PLL locked).
  *         If a clock source which is not yet ready is selected, the switch will
  *         occur when the clock source is ready.
  *
  * @note   You can use HAL_RCC_GetClockConfig() function to know which clock is
  *         currently used as system clock source.
  *
  * @note   Depending on the device voltage range, the software has to set correctly
  *         HPRE[3:0] bits to ensure that HCLK not exceed the maximum allowed frequency
  *         (for more details refer to section above "Initialization/de-initialization functions")
  * @retval None
  */
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef  *RCC_ClkInitStruct, uint32_t FLatency)
{
  uint32_t tickstart;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  uint32_t hpre = RCC_SYSCLK_DIV1;
#endif
  HAL_StatusTypeDef status;

  /* Check Null pointer */
  if(RCC_ClkInitStruct == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RCC_CLOCKTYPE(RCC_ClkInitStruct->ClockType));
  assert_param(IS_FLASH_LATENCY(FLatency));

  /* To correctly read data from FLASH memory, the number of wait states (LATENCY)
    must be correctly programmed according to the frequency of the CPU clock
    (HCLK) and the supply voltage of the device. */

  /* Increasing the number of wait states because of higher CPU frequency */
  if(FLatency > __HAL_FLASH_GET_LATENCY())
  {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    __HAL_FLASH_SET_LATENCY(FLatency);

    /* Check that the new number of wait states is taken into account to access the Flash
    memory by reading the FLASH_ACR register */
    if(__HAL_FLASH_GET_LATENCY() != FLatency)
    {
      return HAL_ERROR;
    }
  }

  /*------------------------- SYSCLK Configuration ---------------------------*/
  if(((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_SYSCLK) == RCC_CLOCKTYPE_SYSCLK)
  {
    assert_param(IS_RCC_SYSCLKSOURCE(RCC_ClkInitStruct->SYSCLKSource));

    /* PLL is selected as System Clock Source */
    if(RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_PLLCLK)
    {
      /* Check the PLL ready flag */
      if(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0U)
      {
        return HAL_ERROR;
      }
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
      /* Undershoot management when selection PLL as SYSCLK source and frequency above 80Mhz */
      /* Compute target PLL output frequency */
      if(RCC_GetSysClockFreqFromPLLSource() > 80000000U)
      {
        if(READ_BIT(RCC->CFGR, RCC_CFGR_HPRE) == RCC_SYSCLK_DIV1)
        {
          /* Intermediate step with HCLK prescaler 2 necessary before to go over 80Mhz */
          MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV2);
          hpre = RCC_SYSCLK_DIV2;
        }
        else if((((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_HCLK) == RCC_CLOCKTYPE_HCLK) && (RCC_ClkInitStruct->AHBCLKDivider == RCC_SYSCLK_DIV1))
        {
          /* Intermediate step with HCLK prescaler 2 necessary before to go over 80Mhz */
          MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV2);
          hpre = RCC_SYSCLK_DIV2;
        }
        else
        {
          /* nothing to do */
        }
      }
#endif
    }
    else
    {
      /* HSE is selected as System Clock Source */
      if(RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_HSE)
      {
        /* Check the HSE ready flag */
        if(READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0U)
        {
          return HAL_ERROR;
        }
      }
      /* MSI is selected as System Clock Source */
      else if(RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_MSI)
      {
        /* Check the MSI ready flag */
        if(READ_BIT(RCC->CR, RCC_CR_MSIRDY) == 0U)
        {
          return HAL_ERROR;
        }
      }
      /* HSI is selected as System Clock Source */
      else
      {
        /* Check the HSI ready flag */
        if(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
        {
          return HAL_ERROR;
        }
      }
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
      /* Overshoot management when going down from PLL as SYSCLK source and frequency above 80Mhz */
      if(HAL_RCC_GetSysClockFreq() > 80000000U)
      {
        /* Intermediate step with HCLK prescaler 2 necessary before to go under 80Mhz */
        MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV2);
        hpre = RCC_SYSCLK_DIV2;
      }
#endif

    }

    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_ClkInitStruct->SYSCLKSource);

    /* Get Start Tick*/
    tickstart = HAL_GetTick();

    while(__HAL_RCC_GET_SYSCLK_SOURCE() != (RCC_ClkInitStruct->SYSCLKSource << RCC_CFGR_SWS_Pos))
    {
      if((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  /*-------------------------- HCLK Configuration --------------------------*/
  if(((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_HCLK) == RCC_CLOCKTYPE_HCLK)
  {
    assert_param(IS_RCC_HCLK(RCC_ClkInitStruct->AHBCLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_ClkInitStruct->AHBCLKDivider);
  }
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  else
  {
    /* Is intermediate HCLK prescaler 2 applied internally, complete with HCLK prescaler 1 */
    if(hpre == RCC_SYSCLK_DIV2)
    {
      MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV1);
    }
  }
#endif

  /* Decreasing the number of wait states because of lower CPU frequency */
  if(FLatency < __HAL_FLASH_GET_LATENCY())
  {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    __HAL_FLASH_SET_LATENCY(FLatency);

    /* Check that the new number of wait states is taken into account to access the Flash
    memory by reading the FLASH_ACR register */
    if(__HAL_FLASH_GET_LATENCY() != FLatency)
    {
      return HAL_ERROR;
    }
  }

  /*-------------------------- PCLK1 Configuration ---------------------------*/
  if(((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK1) == RCC_CLOCKTYPE_PCLK1)
  {
    assert_param(IS_RCC_PCLK(RCC_ClkInitStruct->APB1CLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_ClkInitStruct->APB1CLKDivider);
  }

  /*-------------------------- PCLK2 Configuration ---------------------------*/
  if(((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK2) == RCC_CLOCKTYPE_PCLK2)
  {
    assert_param(IS_RCC_PCLK(RCC_ClkInitStruct->APB2CLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, ((RCC_ClkInitStruct->APB2CLKDivider) << 3U));
  }

  /* Update the SystemCoreClock global variable */
  SystemCoreClock = HAL_RCC_GetSysClockFreq() >> (AHBPrescTable[READ_BIT(RCC->CFGR, RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos] & 0x1FU);

  /* Configure the source of time base considering new system clocks settings*/
  status = HAL_InitTick(uwTickPrio);

  return status;
}

/**
  * @}
  */

/** @defgroup RCC_Exported_Functions_Group2 Peripheral Control functions
 *  @brief   RCC clocks control functions
 *
@verbatim
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to:

    (+) Ouput clock to MCO pin.
    (+) Retrieve current clock frequencies.
    (+) Enable the Clock Security System.

@endverbatim
  * @{
  */

/**
  * @brief  Select the clock source to output on MCO pin(PA8).
  * @note   PA8 should be configured in alternate function mode.
  * @param  RCC_MCOx  specifies the output direction for the clock source.
  *          For STM32L4xx family this parameter can have only one value:
  *            @arg @ref RCC_MCO1  Clock source to output on MCO1 pin(PA8).
  * @param  RCC_MCOSource  specifies the clock source to output.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCO1SOURCE_NOCLOCK  MCO output disabled, no clock on MCO
  *            @arg @ref RCC_MCO1SOURCE_SYSCLK  system  clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_MSI  MSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_HSI  HSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_HSE  HSE clock selected as MCO sourcee
  *            @arg @ref RCC_MCO1SOURCE_PLLCLK  main PLL clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_LSI  LSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_LSE  LSE clock selected as MCO source
  @if STM32L443xx
  *            @arg @ref RCC_MCO1SOURCE_HSI48  HSI48 clock selected as MCO source for devices with HSI48
  @endif
  * @param  RCC_MCODiv  specifies the MCO prescaler.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCODIV_1  no division applied to MCO clock
  *            @arg @ref RCC_MCODIV_2  division by 2 applied to MCO clock
  *            @arg @ref RCC_MCODIV_4  division by 4 applied to MCO clock
  *            @arg @ref RCC_MCODIV_8  division by 8 applied to MCO clock
  *            @arg @ref RCC_MCODIV_16  division by 16 applied to MCO clock
  * @retval None
  */
void HAL_RCC_MCOConfig( uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Check the parameters */
  assert_param(IS_RCC_MCO(RCC_MCOx));
  assert_param(IS_RCC_MCODIV(RCC_MCODiv));
  assert_param(IS_RCC_MCO1SOURCE(RCC_MCOSource));

  /* Prevent unused argument(s) compilation warning if no assert_param check */
  UNUSED(RCC_MCOx);

  /* MCO Clock Enable */
  __MCO1_CLK_ENABLE();

  /* Configue the MCO1 pin in alternate function mode */
  GPIO_InitStruct.Pin = MCO1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(MCO1_GPIO_PORT, &GPIO_InitStruct);

  /* Mask MCOSEL[] and MCOPRE[] bits then set MCO1 clock source and prescaler */
  MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE), (RCC_MCOSource | RCC_MCODiv ));
}

/**
  * @brief  Return the SYSCLK frequency.
  *
  * @note   The system frequency computed by this function is not the real
  *         frequency in the chip. It is calculated based on the predefined
  *         constant and the selected clock source:
  * @note     If SYSCLK source is MSI, function returns values based on MSI
  *             Value as defined by the MSI range.
  * @note     If SYSCLK source is HSI, function returns values based on HSI_VALUE(*)
  * @note     If SYSCLK source is HSE, function returns values based on HSE_VALUE(**)
  * @note     If SYSCLK source is PLL, function returns values based on HSE_VALUE(**),
  *           HSI_VALUE(*) or MSI Value multiplied/divided by the PLL factors.
  * @note     (*) HSI_VALUE is a constant defined in stm32l4xx_hal_conf.h file (default value
  *               16 MHz) but the real value may vary depending on the variations
  *               in voltage and temperature.
  * @note     (**) HSE_VALUE is a constant defined in stm32l4xx_hal_conf.h file (default value
  *                8 MHz), user has to ensure that HSE_VALUE is same as the real
  *                frequency of the crystal used. Otherwise, this function may
  *                have wrong result.
  *
  * @note   The result of this function could be not correct when using fractional
  *         value for HSE crystal.
  *
  * @note   This function can be used by the user application to compute the
  *         baudrate for the communication peripherals or configure other parameters.
  *
  * @note   Each time SYSCLK changes, this function must be called to update the
  *         right SYSCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  *
  * @retval SYSCLK frequency
  */
uint32_t HAL_RCC_GetSysClockFreq(void)
{
  uint32_t msirange = 0U, sysclockfreq = 0U;
  uint32_t pllvco, pllsource, pllr, pllm;    /* no init needed */
  uint32_t sysclk_source, pll_oscsource;

  sysclk_source = __HAL_RCC_GET_SYSCLK_SOURCE();
  pll_oscsource = __HAL_RCC_GET_PLL_OSCSOURCE();

  if((sysclk_source == RCC_CFGR_SWS_MSI) ||
     ((sysclk_source == RCC_CFGR_SWS_PLL) && (pll_oscsource == RCC_PLLSOURCE_MSI)))
  {
    /* MSI or PLL with MSI source used as system clock source */

    /* Get SYSCLK source */
    if(READ_BIT(RCC->CR, RCC_CR_MSIRGSEL) == 0U)
    { /* MSISRANGE from RCC_CSR applies */
      msirange = READ_BIT(RCC->CSR, RCC_CSR_MSISRANGE) >> RCC_CSR_MSISRANGE_Pos;
    }
    else
    { /* MSIRANGE from RCC_CR applies */
      msirange = READ_BIT(RCC->CR, RCC_CR_MSIRANGE) >> RCC_CR_MSIRANGE_Pos;
    }
    /*MSI frequency range in HZ*/
    msirange = MSIRangeTable[msirange];

    if(sysclk_source == RCC_CFGR_SWS_MSI)
    {
      /* MSI used as system clock source */
      sysclockfreq = msirange;
    }
  }
  else if(sysclk_source == RCC_CFGR_SWS_HSI)
  {
    /* HSI used as system clock source */
    sysclockfreq = HSI_VALUE;
  }
  else if(sysclk_source == RCC_CFGR_SWS_HSE)
  {
    /* HSE used as system clock source */
    sysclockfreq = HSE_VALUE;
  }
  else
  {
    /* unexpected case: sysclockfreq at 0 */
  }

  if(sysclk_source == RCC_CFGR_SWS_PLL)
  {
    /* PLL used as system clock  source */

    /* PLL_VCO = (HSE_VALUE or HSI_VALUE or MSI_VALUE) * PLLN / PLLM
    SYSCLK = PLL_VCO / PLLR
    */
    pllsource = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);

    switch (pllsource)
    {
    case RCC_PLLSOURCE_HSI:  /* HSI used as PLL clock source */
      pllvco = HSI_VALUE;
      break;

    case RCC_PLLSOURCE_HSE:  /* HSE used as PLL clock source */
      pllvco = HSE_VALUE;
      break;

    case RCC_PLLSOURCE_MSI:  /* MSI used as PLL clock source */
    default:
      pllvco = msirange;
      break;
    }
    pllm = (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos) + 1U ;
    pllvco = (pllvco * (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)) / pllm;
    pllr = ((READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLR) >> RCC_PLLCFGR_PLLR_Pos) + 1U ) * 2U;
    sysclockfreq = pllvco / pllr;
  }

  return sysclockfreq;
}

/**
  * @brief  Return the HCLK frequency.
  * @note   Each time HCLK changes, this function must be called to update the
  *         right HCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency.
  * @retval HCLK frequency in Hz
  */
uint32_t HAL_RCC_GetHCLKFreq(void)
{
  return SystemCoreClock;
}

/**
  * @brief  Return the PCLK1 frequency.
  * @note   Each time PCLK1 changes, this function must be called to update the
  *         right PCLK1 value. Otherwise, any configuration based on this function will be incorrect.
  * @retval PCLK1 frequency in Hz
  */
uint32_t HAL_RCC_GetPCLK1Freq(void)
{
  /* Get HCLK source and Compute PCLK1 frequency ---------------------------*/
  return (HAL_RCC_GetHCLKFreq() >> (APBPrescTable[READ_BIT(RCC->CFGR, RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos] & 0x1FU));
}

/**
  * @brief  Return the PCLK2 frequency.
  * @note   Each time PCLK2 changes, this function must be called to update the
  *         right PCLK2 value. Otherwise, any configuration based on this function will be incorrect.
  * @retval PCLK2 frequency in Hz
  */
uint32_t HAL_RCC_GetPCLK2Freq(void)
{
  /* Get HCLK source and Compute PCLK2 frequency ---------------------------*/
  return (HAL_RCC_GetHCLKFreq()>> (APBPrescTable[READ_BIT(RCC->CFGR, RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos] & 0x1FU));
}

/**
  * @brief  Configure the RCC_OscInitStruct according to the internal
  *         RCC configuration registers.
  * @param  RCC_OscInitStruct  pointer to an RCC_OscInitTypeDef structure that
  *         will be configured.
  * @retval None
  */
void HAL_RCC_GetOscConfig(RCC_OscInitTypeDef  *RCC_OscInitStruct)
{
  /* Check the parameters */
  assert_param(RCC_OscInitStruct != (void *)NULL);

  /* Set all possible values for the Oscillator type parameter ---------------*/
#if defined(RCC_HSI48_SUPPORT)
  RCC_OscInitStruct->OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_MSI | \
                                      RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSI48;
#else
  RCC_OscInitStruct->OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_MSI | \
                                      RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_LSI;
#endif /* RCC_HSI48_SUPPORT */

  /* Get the HSE configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CR, RCC_CR_HSEBYP) == RCC_CR_HSEBYP)
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_BYPASS;
  }
  else if(READ_BIT(RCC->CR, RCC_CR_HSEON) == RCC_CR_HSEON)
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_ON;
  }
  else
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_OFF;
  }

   /* Get the MSI configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CR, RCC_CR_MSION) == RCC_CR_MSION)
  {
    RCC_OscInitStruct->MSIState = RCC_MSI_ON;
  }
  else
  {
    RCC_OscInitStruct->MSIState = RCC_MSI_OFF;
  }

  RCC_OscInitStruct->MSICalibrationValue = READ_BIT(RCC->ICSCR, RCC_ICSCR_MSITRIM) >> RCC_ICSCR_MSITRIM_Pos;
  RCC_OscInitStruct->MSIClockRange = READ_BIT(RCC->CR, RCC_CR_MSIRANGE);

  /* Get the HSI configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CR, RCC_CR_HSION) == RCC_CR_HSION)
  {
    RCC_OscInitStruct->HSIState = RCC_HSI_ON;
  }
  else
  {
    RCC_OscInitStruct->HSIState = RCC_HSI_OFF;
  }

  RCC_OscInitStruct->HSICalibrationValue = READ_BIT(RCC->ICSCR, RCC_ICSCR_HSITRIM) >> RCC_ICSCR_HSITRIM_Pos;

  /* Get the LSE configuration -----------------------------------------------*/
  if(READ_BIT(RCC->BDCR, RCC_BDCR_LSEBYP) == RCC_BDCR_LSEBYP)
  {
#if defined(RCC_BDCR_LSESYSDIS)
    if((RCC->BDCR & RCC_BDCR_LSESYSDIS) == RCC_BDCR_LSESYSDIS)
    {
      RCC_OscInitStruct->LSEState = RCC_LSE_BYPASS_RTC_ONLY;
    }
    else
#endif /* RCC_BDCR_LSESYSDIS */
    {
      RCC_OscInitStruct->LSEState = RCC_LSE_BYPASS;
    }
  }
  else if(READ_BIT(RCC->BDCR, RCC_BDCR_LSEON) == RCC_BDCR_LSEON)
  {
#if defined(RCC_BDCR_LSESYSDIS)
    if((RCC->BDCR & RCC_BDCR_LSESYSDIS) == RCC_BDCR_LSESYSDIS)
    {
      RCC_OscInitStruct->LSEState = RCC_LSE_ON_RTC_ONLY;
    }
    else
#endif /* RCC_BDCR_LSESYSDIS */
    {
      RCC_OscInitStruct->LSEState = RCC_LSE_ON;
    }
  }
  else
  {
    RCC_OscInitStruct->LSEState = RCC_LSE_OFF;
  }

  /* Get the LSI configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CSR, RCC_CSR_LSION) == RCC_CSR_LSION)
  {
    RCC_OscInitStruct->LSIState = RCC_LSI_ON;
  }
  else
  {
    RCC_OscInitStruct->LSIState = RCC_LSI_OFF;
  }
#if defined(RCC_CSR_LSIPREDIV)

  /* Get the LSI configuration -----------------------------------------------*/
  if((RCC->CSR & RCC_CSR_LSIPREDIV) == RCC_CSR_LSIPREDIV)
  {
    RCC_OscInitStruct->LSIDiv = RCC_LSI_DIV128;
  }
  else
  {
    RCC_OscInitStruct->LSIDiv = RCC_LSI_DIV1;
  }
#endif /* RCC_CSR_LSIPREDIV */

#if defined(RCC_HSI48_SUPPORT)
  /* Get the HSI48 configuration ---------------------------------------------*/
  if(READ_BIT(RCC->CRRCR, RCC_CRRCR_HSI48ON) == RCC_CRRCR_HSI48ON)
  {
    RCC_OscInitStruct->HSI48State = RCC_HSI48_ON;
  }
  else
  {
    RCC_OscInitStruct->HSI48State = RCC_HSI48_OFF;
  }
#else
  RCC_OscInitStruct->HSI48State = RCC_HSI48_OFF;
#endif /* RCC_HSI48_SUPPORT */

  /* Get the PLL configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CR, RCC_CR_PLLON) == RCC_CR_PLLON)
  {
    RCC_OscInitStruct->PLL.PLLState = RCC_PLL_ON;
  }
  else
  {
    RCC_OscInitStruct->PLL.PLLState = RCC_PLL_OFF;
  }
  RCC_OscInitStruct->PLL.PLLSource = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);
  RCC_OscInitStruct->PLL.PLLM = (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos) + 1U;
  RCC_OscInitStruct->PLL.PLLN = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos;
  RCC_OscInitStruct->PLL.PLLQ = (((READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLQ) >> RCC_PLLCFGR_PLLQ_Pos) + 1U) << 1U);
  RCC_OscInitStruct->PLL.PLLR = (((READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLR) >> RCC_PLLCFGR_PLLR_Pos) + 1U) << 1U);
#if defined(RCC_PLLP_SUPPORT)
#if defined(RCC_PLLP_DIV_2_31_SUPPORT)
  RCC_OscInitStruct->PLL.PLLP = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLPDIV) >> RCC_PLLCFGR_PLLPDIV_Pos;
#else
  if(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLP) != 0U)
  {
    RCC_OscInitStruct->PLL.PLLP = RCC_PLLP_DIV17;
  }
  else
  {
    RCC_OscInitStruct->PLL.PLLP = RCC_PLLP_DIV7;
  }
#endif /* RCC_PLLP_DIV_2_31_SUPPORT */
#endif /* RCC_PLLP_SUPPORT */
}

/**
  * @brief  Configure the RCC_ClkInitStruct according to the internal
  *         RCC configuration registers.
  * @param  RCC_ClkInitStruct  pointer to an RCC_ClkInitTypeDef structure that
  *         will be configured.
  * @param  pFLatency  Pointer on the Flash Latency.
  * @retval None
  */
void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef  *RCC_ClkInitStruct, uint32_t *pFLatency)
{
  /* Check the parameters */
  assert_param(RCC_ClkInitStruct != (void  *)NULL);
  assert_param(pFLatency != (void *)NULL);

  /* Set all possible values for the Clock type parameter --------------------*/
  RCC_ClkInitStruct->ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

  /* Get the SYSCLK configuration --------------------------------------------*/
  RCC_ClkInitStruct->SYSCLKSource = READ_BIT(RCC->CFGR, RCC_CFGR_SW);

  /* Get the HCLK configuration ----------------------------------------------*/
  RCC_ClkInitStruct->AHBCLKDivider = READ_BIT(RCC->CFGR, RCC_CFGR_HPRE);

  /* Get the APB1 configuration ----------------------------------------------*/
  RCC_ClkInitStruct->APB1CLKDivider = READ_BIT(RCC->CFGR, RCC_CFGR_PPRE1);

  /* Get the APB2 configuration ----------------------------------------------*/
  RCC_ClkInitStruct->APB2CLKDivider = (READ_BIT(RCC->CFGR, RCC_CFGR_PPRE2) >> 3U);

  /* Get the Flash Wait State (Latency) configuration ------------------------*/
  *pFLatency = __HAL_FLASH_GET_LATENCY();
}

/**
  * @brief  Enable the Clock Security System.
  * @note   If a failure is detected on the HSE oscillator clock, this oscillator
  *         is automatically disabled and an interrupt is generated to inform the
  *         software about the failure (Clock Security System Interrupt, CSSI),
  *         allowing the MCU to perform rescue operations. The CSSI is linked to
  *         the Cortex-M4 NMI (Non-Maskable Interrupt) exception vector.
  * @note   The Clock Security System can only be cleared by reset.
  * @retval None
  */
void HAL_RCC_EnableCSS(void)
{
  SET_BIT(RCC->CR, RCC_CR_CSSON) ;
}

/**
  * @brief Handle the RCC Clock Security System interrupt request.
  * @note This API should be called under the NMI_Handler().
  * @retval None
  */
void HAL_RCC_NMI_IRQHandler(void)
{
  /* Check RCC CSSF interrupt flag  */
  if(__HAL_RCC_GET_IT(RCC_IT_CSS))
  {
    /* RCC Clock Security System interrupt user callback */
    HAL_RCC_CSSCallback();

    /* Clear RCC CSS pending bit */
    __HAL_RCC_CLEAR_IT(RCC_IT_CSS);
  }
}

/**
  * @brief  RCC Clock Security System interrupt callback.
  * @retval none
  */
__weak void HAL_RCC_CSSCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_RCC_CSSCallback should be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup RCC_Private_Functions
  * @{
  */
/**
  * @brief  Update number of Flash wait states in line with MSI range and current
            voltage range.
  * @param  msirange  MSI range value from RCC_MSIRANGE_0 to RCC_MSIRANGE_11
  * @retval HAL status
  */
static HAL_StatusTypeDef RCC_SetFlashLatencyFromMSIRange(uint32_t msirange)
{
  uint32_t vos;
  uint32_t latency = FLASH_LATENCY_0;  /* default value 0WS */

  if(__HAL_RCC_PWR_IS_CLK_ENABLED())
  {
    vos = HAL_PWREx_GetVoltageRange();
  }
  else
  {
    __HAL_RCC_PWR_CLK_ENABLE();
    vos = HAL_PWREx_GetVoltageRange();
    __HAL_RCC_PWR_CLK_DISABLE();
  }

  if(vos == PWR_REGULATOR_VOLTAGE_SCALE1)
  {
    if(msirange > RCC_MSIRANGE_8)
    {
      /* MSI > 16Mhz */
      if(msirange > RCC_MSIRANGE_10)
      {
        /* MSI 48Mhz */
        latency = FLASH_LATENCY_2; /* 2WS */
      }
      else
      {
        /* MSI 24Mhz or 32Mhz */
        latency = FLASH_LATENCY_1; /* 1WS */
      }
    }
    /* else MSI <= 16Mhz default FLASH_LATENCY_0 0WS */
  }
  else
  {
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    if(msirange >= RCC_MSIRANGE_8)
    {
      /* MSI >= 16Mhz */
      latency = FLASH_LATENCY_2; /* 2WS */
    }
    else
    {
      if(msirange == RCC_MSIRANGE_7)
      {
        /* MSI 8Mhz */
        latency = FLASH_LATENCY_1; /* 1WS */
      }
      /* else MSI < 8Mhz default FLASH_LATENCY_0 0WS */
    }
#else
    if(msirange > RCC_MSIRANGE_8)
    {
      /* MSI > 16Mhz */
      latency = FLASH_LATENCY_3; /* 3WS */
    }
    else
    {
      if(msirange == RCC_MSIRANGE_8)
      {
        /* MSI 16Mhz */
        latency = FLASH_LATENCY_2; /* 2WS */
      }
      else if(msirange == RCC_MSIRANGE_7)
      {
        /* MSI 8Mhz */
        latency = FLASH_LATENCY_1; /* 1WS */
      }
      /* else MSI < 8Mhz default FLASH_LATENCY_0 0WS */
    }
#endif
  }

  __HAL_FLASH_SET_LATENCY(latency);

  /* Check that the new number of wait states is taken into account to access the Flash
     memory by reading the FLASH_ACR register */
  if(__HAL_FLASH_GET_LATENCY() != latency)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/**
  * @brief  Compute SYSCLK frequency based on PLL SYSCLK source.
  * @retval SYSCLK frequency
  */
static uint32_t RCC_GetSysClockFreqFromPLLSource(void)
{
  uint32_t msirange = 0U;
  uint32_t pllvco, pllsource, pllr, pllm, sysclockfreq;  /* no init needed */

  if(__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_MSI)
  {
    /* Get MSI range source */
    if(READ_BIT(RCC->CR, RCC_CR_MSIRGSEL) == 0U)
    { /* MSISRANGE from RCC_CSR applies */
      msirange = READ_BIT(RCC->CSR, RCC_CSR_MSISRANGE) >> RCC_CSR_MSISRANGE_Pos;
    }
    else
    { /* MSIRANGE from RCC_CR applies */
      msirange = READ_BIT(RCC->CR, RCC_CR_MSIRANGE) >> RCC_CR_MSIRANGE_Pos;
    }
    /*MSI frequency range in HZ*/
    msirange = MSIRangeTable[msirange];
  }

  /* PLL_VCO = (HSE_VALUE or HSI_VALUE or MSI_VALUE) * PLLN / PLLM
     SYSCLK = PLL_VCO / PLLR
   */
  pllsource = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);

  switch (pllsource)
  {
  case RCC_PLLSOURCE_HSI:  /* HSI used as PLL clock source */
    pllvco = HSI_VALUE;
    break;

  case RCC_PLLSOURCE_HSE:  /* HSE used as PLL clock source */
    pllvco = HSE_VALUE;
    break;

  case RCC_PLLSOURCE_MSI:  /* MSI used as PLL clock source */
  default:
    pllvco = msirange;
    break;
  }
  pllm = (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos) + 1U ;
  pllvco = (pllvco * (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)) / pllm;
  pllr = ((READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLR) >> RCC_PLLCFGR_PLLR_Pos) + 1U ) * 2U;
  sysclockfreq = pllvco / pllr;

  return sysclockfreq;
}
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

#endif /* HAL_RCC_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
