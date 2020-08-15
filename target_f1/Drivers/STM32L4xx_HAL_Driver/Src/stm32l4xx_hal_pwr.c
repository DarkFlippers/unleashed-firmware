/**
  ******************************************************************************
  * @file    stm32l4xx_hal_pwr.c
  * @author  MCD Application Team
  * @brief   PWR HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Power Controller (PWR) peripheral:
  *           + Initialization/de-initialization functions
  *           + Peripheral Control functions
  *
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

/** @defgroup PWR PWR
  * @brief PWR HAL module driver
  * @{
  */

#ifdef HAL_PWR_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup PWR_Private_Defines PWR Private Defines
  * @{
  */

/** @defgroup PWR_PVD_Mode_Mask PWR PVD Mode Mask
  * @{
  */
#define PVD_MODE_IT               ((uint32_t)0x00010000)  /*!< Mask for interruption yielded by PVD threshold crossing */
#define PVD_MODE_EVT              ((uint32_t)0x00020000)  /*!< Mask for event yielded by PVD threshold crossing        */
#define PVD_RISING_EDGE           ((uint32_t)0x00000001)  /*!< Mask for rising edge set as PVD trigger                 */
#define PVD_FALLING_EDGE          ((uint32_t)0x00000002)  /*!< Mask for falling edge set as PVD trigger                */
/**
  * @}
  */

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup PWR_Exported_Functions PWR Exported Functions
  * @{
  */

/** @defgroup PWR_Exported_Functions_Group1 Initialization and de-initialization functions
  *  @brief    Initialization and de-initialization functions
  *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]

@endverbatim
  * @{
  */

/**
  * @brief Deinitialize the HAL PWR peripheral registers to their default reset values.
  * @retval None
  */
void HAL_PWR_DeInit(void)
{
  __HAL_RCC_PWR_FORCE_RESET();
  __HAL_RCC_PWR_RELEASE_RESET();
}

/**
  * @brief Enable access to the backup domain
  *        (RTC registers, RTC backup data registers).
  * @note  After reset, the backup domain is protected against
  *        possible unwanted write accesses.
  * @note  RTCSEL that sets the RTC clock source selection is in the RTC back-up domain.
  *        In order to set or modify the RTC clock, the backup domain access must be
  *        disabled.
  * @note  LSEON bit that switches on and off the LSE crystal belongs as well to the
  *        back-up domain.
  * @retval None
  */
void HAL_PWR_EnableBkUpAccess(void)
{
  SET_BIT(PWR->CR1, PWR_CR1_DBP);
}

/**
  * @brief Disable access to the backup domain
  *        (RTC registers, RTC backup data registers).
  * @retval None
  */
void HAL_PWR_DisableBkUpAccess(void)
{
  CLEAR_BIT(PWR->CR1, PWR_CR1_DBP);
}




/**
  * @}
  */



/** @defgroup PWR_Exported_Functions_Group2 Peripheral Control functions
  *  @brief Low Power modes configuration functions
  *
@verbatim

 ===============================================================================
                 ##### Peripheral Control functions #####
 ===============================================================================

     [..]
     *** PVD configuration ***
    =========================
    [..]
      (+) The PVD is used to monitor the VDD power supply by comparing it to a
          threshold selected by the PVD Level (PLS[2:0] bits in PWR_CR2 register).

      (+) PVDO flag is available to indicate if VDD/VDDA is higher or lower
          than the PVD threshold. This event is internally connected to the EXTI
          line16 and can generate an interrupt if enabled. This is done through
          __HAL_PVD_EXTI_ENABLE_IT() macro.
      (+) The PVD is stopped in Standby mode.


    *** WakeUp pin configuration ***
    ================================
    [..]
      (+) WakeUp pins are used to wakeup the system from Standby mode or Shutdown mode.
          The polarity of these pins can be set to configure event detection on high
          level (rising edge) or low level (falling edge).



    *** Low Power modes configuration ***
    =====================================
    [..]
      The devices feature 8 low-power modes:
      (+) Low-power Run mode: core and peripherals are running, main regulator off, low power regulator on.
      (+) Sleep mode: Cortex-M4 core stopped, peripherals kept running, main and low power regulators on.
      (+) Low-power Sleep mode: Cortex-M4 core stopped, peripherals kept running, main regulator off, low power regulator on.
      (+) Stop 0 mode: all clocks are stopped except LSI and LSE, main and low power regulators on.
      (+) Stop 1 mode: all clocks are stopped except LSI and LSE, main regulator off, low power regulator on.
      (+) Stop 2 mode: all clocks are stopped except LSI and LSE, main regulator off, low power regulator on, reduced set of waking up IPs compared to Stop 1 mode.
      (+) Standby mode with SRAM2: all clocks are stopped except LSI and LSE, SRAM2 content preserved, main regulator off, low power regulator on.
      (+) Standby mode without SRAM2: all clocks are stopped except LSI and LSE, main and low power regulators off.
      (+) Shutdown mode: all clocks are stopped except LSE, main and low power regulators off.


   *** Low-power run mode ***
   ==========================
    [..]
      (+) Entry: (from main run mode)
        (++) set LPR bit with HAL_PWREx_EnableLowPowerRunMode() API after having decreased the system clock below 2 MHz.

      (+) Exit:
        (++) clear LPR bit then wait for REGLP bit to be reset with HAL_PWREx_DisableLowPowerRunMode() API. Only
             then can the system clock frequency be increased above 2 MHz.


   *** Sleep mode / Low-power sleep mode ***
   =========================================
    [..]
      (+) Entry:
          The Sleep mode / Low-power Sleep mode is entered thru HAL_PWR_EnterSLEEPMode() API
          in specifying whether or not the regulator is forced to low-power mode and if exit is interrupt or event-triggered.
          (++) PWR_MAINREGULATOR_ON: Sleep mode (regulator in main mode).
          (++) PWR_LOWPOWERREGULATOR_ON: Low-power sleep (regulator in low power mode).
          In the latter case, the system clock frequency must have been decreased below 2 MHz beforehand.
          (++) PWR_SLEEPENTRY_WFI: enter SLEEP mode with WFI instruction
          (++) PWR_SLEEPENTRY_WFE: enter SLEEP mode with WFE instruction

      (+) WFI Exit:
        (++) Any peripheral interrupt acknowledged by the nested vectored interrupt
             controller (NVIC) or any wake-up event.

      (+) WFE Exit:
        (++) Any wake-up event such as an EXTI line configured in event mode.

         [..] When exiting the Low-power sleep mode by issuing an interrupt or a wakeup event,
             the MCU is in Low-power Run mode.

   *** Stop 0, Stop 1 and Stop 2 modes ***
   ===============================
    [..]
      (+) Entry:
          The Stop 0, Stop 1 or Stop 2 modes are entered thru the following API's:
          (++) HAL_PWREx_EnterSTOP0Mode() for mode 0 or HAL_PWREx_EnterSTOP1Mode() for mode 1 or for porting reasons HAL_PWR_EnterSTOPMode().
          (++) HAL_PWREx_EnterSTOP2Mode() for mode 2.
      (+) Regulator setting (applicable to HAL_PWR_EnterSTOPMode() only):
          (++) PWR_MAINREGULATOR_ON
          (++) PWR_LOWPOWERREGULATOR_ON
      (+) Exit (interrupt or event-triggered, specified when entering STOP mode):
          (++) PWR_STOPENTRY_WFI: enter Stop mode with WFI instruction
          (++) PWR_STOPENTRY_WFE: enter Stop mode with WFE instruction

      (+) WFI Exit:
          (++) Any EXTI Line (Internal or External) configured in Interrupt mode.
          (++) Some specific communication peripherals (USART, LPUART, I2C) interrupts
               when programmed in wakeup mode.
      (+) WFE Exit:
          (++) Any EXTI Line (Internal or External) configured in Event mode.

       [..]
          When exiting Stop 0 and Stop 1 modes, the MCU is either in Run mode or in Low-power Run mode
          depending on the LPR bit setting.
          When exiting Stop 2 mode, the MCU is in Run mode.

   *** Standby mode ***
   ====================
     [..]
      The Standby mode offers two options:
      (+) option a) all clocks off except LSI and LSE, RRS bit set (keeps voltage regulator in low power mode).
        SRAM and registers contents are lost except for the SRAM2 content, the RTC registers, RTC backup registers
        and Standby circuitry.
      (+) option b) all clocks off except LSI and LSE, RRS bit cleared (voltage regulator then disabled).
        SRAM and register contents are lost except for the RTC registers, RTC backup registers
        and Standby circuitry.

      (++) Entry:
          (+++) The Standby mode is entered thru HAL_PWR_EnterSTANDBYMode() API.
                SRAM1 and register contents are lost except for registers in the Backup domain and
                Standby circuitry. SRAM2 content can be preserved if the bit RRS is set in PWR_CR3 register.
                To enable this feature, the user can resort to HAL_PWREx_EnableSRAM2ContentRetention() API
                to set RRS bit.

      (++) Exit:
          (+++) WKUP pin rising edge, RTC alarm or wakeup, tamper event, time-stamp event,
                external reset in NRST pin, IWDG reset.

      [..]    After waking up from Standby mode, program execution restarts in the same way as after a Reset.


    *** Shutdown mode ***
   ======================
     [..]
      In Shutdown mode,
        voltage regulator is disabled, all clocks are off except LSE, RRS bit is cleared.
        SRAM and registers contents are lost except for backup domain registers.

      (+) Entry:
          The Shutdown mode is entered thru HAL_PWREx_EnterSHUTDOWNMode() API.

      (+) Exit:
          (++) WKUP pin rising edge, RTC alarm or wakeup, tamper event, time-stamp event,
               external reset in NRST pin.

         [..] After waking up from Shutdown mode, program execution restarts in the same way as after a Reset.


   *** Auto-wakeup (AWU) from low-power mode ***
   =============================================
    [..]
      The MCU can be woken up from low-power mode by an RTC Alarm event, an RTC
      Wakeup event, a tamper event or a time-stamp event, without depending on
      an external interrupt (Auto-wakeup mode).

      (+) RTC auto-wakeup (AWU) from the Stop, Standby and Shutdown modes


        (++) To wake up from the Stop mode with an RTC alarm event, it is necessary to
             configure the RTC to generate the RTC alarm using the HAL_RTC_SetAlarm_IT() function.

        (++) To wake up from the Stop mode with an RTC Tamper or time stamp event, it
             is necessary to configure the RTC to detect the tamper or time stamp event using the
             HAL_RTCEx_SetTimeStamp_IT() or HAL_RTCEx_SetTamper_IT() functions.

        (++) To wake up from the Stop mode with an RTC WakeUp event, it is necessary to
              configure the RTC to generate the RTC WakeUp event using the HAL_RTCEx_SetWakeUpTimer_IT() function.

@endverbatim
  * @{
  */



/**
  * @brief Configure the voltage threshold detected by the Power Voltage Detector (PVD).
  * @param sConfigPVD: pointer to a PWR_PVDTypeDef structure that contains the PVD
  *        configuration information.
  * @note Refer to the electrical characteristics of your device datasheet for
  *         more details about the voltage thresholds corresponding to each
  *         detection level.
  * @retval None
  */
HAL_StatusTypeDef HAL_PWR_ConfigPVD(PWR_PVDTypeDef *sConfigPVD)
{
  /* Check the parameters */
  assert_param(IS_PWR_PVD_LEVEL(sConfigPVD->PVDLevel));
  assert_param(IS_PWR_PVD_MODE(sConfigPVD->Mode));

  /* Set PLS bits according to PVDLevel value */
  MODIFY_REG(PWR->CR2, PWR_CR2_PLS, sConfigPVD->PVDLevel);

  /* Clear any previous config. Keep it clear if no event or IT mode is selected */
  __HAL_PWR_PVD_EXTI_DISABLE_EVENT();
  __HAL_PWR_PVD_EXTI_DISABLE_IT();
  __HAL_PWR_PVD_EXTI_DISABLE_FALLING_EDGE();
  __HAL_PWR_PVD_EXTI_DISABLE_RISING_EDGE();

  /* Configure interrupt mode */
  if((sConfigPVD->Mode & PVD_MODE_IT) == PVD_MODE_IT)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_IT();
  }

  /* Configure event mode */
  if((sConfigPVD->Mode & PVD_MODE_EVT) == PVD_MODE_EVT)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_EVENT();
  }

  /* Configure the edge */
  if((sConfigPVD->Mode & PVD_RISING_EDGE) == PVD_RISING_EDGE)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_RISING_EDGE();
  }

  if((sConfigPVD->Mode & PVD_FALLING_EDGE) == PVD_FALLING_EDGE)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_FALLING_EDGE();
  }

  return HAL_OK;
}


/**
  * @brief Enable the Power Voltage Detector (PVD).
  * @retval None
  */
void HAL_PWR_EnablePVD(void)
{
  SET_BIT(PWR->CR2, PWR_CR2_PVDE);
}

/**
  * @brief Disable the Power Voltage Detector (PVD).
  * @retval None
  */
void HAL_PWR_DisablePVD(void)
{
  CLEAR_BIT(PWR->CR2, PWR_CR2_PVDE);
}




/**
  * @brief Enable the WakeUp PINx functionality.
  * @param WakeUpPinPolarity: Specifies which Wake-Up pin to enable.
  *         This parameter can be one of the following legacy values which set the default polarity
  *         i.e. detection on high level (rising edge):
  *           @arg @ref PWR_WAKEUP_PIN1, PWR_WAKEUP_PIN2, PWR_WAKEUP_PIN3, PWR_WAKEUP_PIN4, PWR_WAKEUP_PIN5
  *
  *         or one of the following value where the user can explicitly specify the enabled pin and
  *         the chosen polarity:
  *           @arg @ref PWR_WAKEUP_PIN1_HIGH or PWR_WAKEUP_PIN1_LOW
  *           @arg @ref PWR_WAKEUP_PIN2_HIGH or PWR_WAKEUP_PIN2_LOW
  *           @arg @ref PWR_WAKEUP_PIN3_HIGH or PWR_WAKEUP_PIN3_LOW
  *           @arg @ref PWR_WAKEUP_PIN4_HIGH or PWR_WAKEUP_PIN4_LOW
  *           @arg @ref PWR_WAKEUP_PIN5_HIGH or PWR_WAKEUP_PIN5_LOW
  * @note  PWR_WAKEUP_PINx and PWR_WAKEUP_PINx_HIGH are equivalent.
  * @retval None
  */
void HAL_PWR_EnableWakeUpPin(uint32_t WakeUpPinPolarity)
{
  assert_param(IS_PWR_WAKEUP_PIN(WakeUpPinPolarity));

  /* Specifies the Wake-Up pin polarity for the event detection
    (rising or falling edge) */
  MODIFY_REG(PWR->CR4, (PWR_CR3_EWUP & WakeUpPinPolarity), (WakeUpPinPolarity >> PWR_WUP_POLARITY_SHIFT));

  /* Enable wake-up pin */
  SET_BIT(PWR->CR3, (PWR_CR3_EWUP & WakeUpPinPolarity));


}

/**
  * @brief Disable the WakeUp PINx functionality.
  * @param WakeUpPinx: Specifies the Power Wake-Up pin to disable.
  *         This parameter can be one of the following values:
  *           @arg @ref PWR_WAKEUP_PIN1, PWR_WAKEUP_PIN2, PWR_WAKEUP_PIN3, PWR_WAKEUP_PIN4, PWR_WAKEUP_PIN5
  * @retval None
  */
void HAL_PWR_DisableWakeUpPin(uint32_t WakeUpPinx)
{
  assert_param(IS_PWR_WAKEUP_PIN(WakeUpPinx));

  CLEAR_BIT(PWR->CR3, (PWR_CR3_EWUP & WakeUpPinx));
}


/**
  * @brief Enter Sleep or Low-power Sleep mode.
  * @note  In Sleep/Low-power Sleep mode, all I/O pins keep the same state as in Run mode.
  * @param Regulator: Specifies the regulator state in Sleep/Low-power Sleep mode.
  *          This parameter can be one of the following values:
  *            @arg @ref PWR_MAINREGULATOR_ON Sleep mode (regulator in main mode)
  *            @arg @ref PWR_LOWPOWERREGULATOR_ON Low-power Sleep mode (regulator in low-power mode)
  * @note  Low-power Sleep mode is entered from Low-power Run mode. Therefore, if not yet
  *        in Low-power Run mode before calling HAL_PWR_EnterSLEEPMode() with Regulator set
  *        to PWR_LOWPOWERREGULATOR_ON, the user can optionally configure the
  *        Flash in power-down monde in setting the SLEEP_PD bit in FLASH_ACR register.
  *        Additionally, the clock frequency must be reduced below 2 MHz.
  *        Setting SLEEP_PD in FLASH_ACR then appropriately reducing the clock frequency must
  *        be done before calling HAL_PWR_EnterSLEEPMode() API.
  * @note  When exiting Low-power Sleep mode, the MCU is in Low-power Run mode. To move in
  *        Run mode, the user must resort to HAL_PWREx_DisableLowPowerRunMode() API.
  * @param SLEEPEntry: Specifies if Sleep mode is entered with WFI or WFE instruction.
  *           This parameter can be one of the following values:
  *            @arg @ref PWR_SLEEPENTRY_WFI enter Sleep or Low-power Sleep mode with WFI instruction
  *            @arg @ref PWR_SLEEPENTRY_WFE enter Sleep or Low-power Sleep mode with WFE instruction
  * @note  When WFI entry is used, tick interrupt have to be disabled if not desired as
  *        the interrupt wake up source.
  * @retval None
  */
void HAL_PWR_EnterSLEEPMode(uint32_t Regulator, uint8_t SLEEPEntry)
{
  /* Check the parameters */
  assert_param(IS_PWR_REGULATOR(Regulator));
  assert_param(IS_PWR_SLEEP_ENTRY(SLEEPEntry));

  /* Set Regulator parameter */
  if (Regulator == PWR_MAINREGULATOR_ON)
  {
    /* If in low-power run mode at this point, exit it */
    if (HAL_IS_BIT_SET(PWR->SR2, PWR_SR2_REGLPF))
    {
      if (HAL_PWREx_DisableLowPowerRunMode() != HAL_OK)
      {
        return ;
      }
    }
    /* Regulator now in main mode. */
  }
  else
  {
    /* If in run mode, first move to low-power run mode.
       The system clock frequency must be below 2 MHz at this point. */
    if (HAL_IS_BIT_SET(PWR->SR2, PWR_SR2_REGLPF) == RESET)
    {
      HAL_PWREx_EnableLowPowerRunMode();
    }
  }

  /* Clear SLEEPDEEP bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  /* Select SLEEP mode entry -------------------------------------------------*/
  if(SLEEPEntry == PWR_SLEEPENTRY_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }

}


/**
  * @brief Enter Stop mode
  * @note  This API is named HAL_PWR_EnterSTOPMode to ensure compatibility with legacy code running
  *        on devices where only "Stop mode" is mentioned with main or low power regulator ON.
  * @note  In Stop mode, all I/O pins keep the same state as in Run mode.
  * @note  All clocks in the VCORE domain are stopped; the PLL, the MSI,
  *        the HSI and the HSE oscillators are disabled. Some peripherals with the wakeup capability
  *        (I2Cx, USARTx and LPUART) can switch on the HSI to receive a frame, and switch off the HSI
  *        after receiving the frame if it is not a wakeup frame. In this case, the HSI clock is propagated
  *        only to the peripheral requesting it.
  *        SRAM1, SRAM2 and register contents are preserved.
  *        The BOR is available.
  *        The voltage regulator can be configured either in normal (Stop 0) or low-power mode (Stop 1).
  * @note  When exiting Stop 0 or Stop 1 mode by issuing an interrupt or a wakeup event,
  *         the HSI RC oscillator is selected as system clock if STOPWUCK bit in RCC_CFGR register
  *         is set; the MSI oscillator is selected if STOPWUCK is cleared.
  * @note  When the voltage regulator operates in low power mode (Stop 1), an additional
  *         startup delay is incurred when waking up.
  *         By keeping the internal regulator ON during Stop mode (Stop 0), the consumption
  *         is higher although the startup time is reduced.
  * @param Regulator: Specifies the regulator state in Stop mode.
  *          This parameter can be one of the following values:
  *            @arg @ref PWR_MAINREGULATOR_ON  Stop 0 mode (main regulator ON)
  *            @arg @ref PWR_LOWPOWERREGULATOR_ON  Stop 1 mode (low power regulator ON)
  * @param STOPEntry: Specifies Stop 0 or Stop 1 mode is entered with WFI or WFE instruction.
  *          This parameter can be one of the following values:
  *            @arg @ref PWR_STOPENTRY_WFI  Enter Stop 0 or Stop 1 mode with WFI instruction.
  *            @arg @ref PWR_STOPENTRY_WFE  Enter Stop 0 or Stop 1 mode with WFE instruction.
  * @retval None
  */
void HAL_PWR_EnterSTOPMode(uint32_t Regulator, uint8_t STOPEntry)
{
  /* Check the parameters */
  assert_param(IS_PWR_REGULATOR(Regulator));

  if(Regulator == PWR_LOWPOWERREGULATOR_ON)
  {
    HAL_PWREx_EnterSTOP1Mode(STOPEntry);
  }
  else
  {
    HAL_PWREx_EnterSTOP0Mode(STOPEntry);
  }
}

/**
  * @brief Enter Standby mode.
  * @note  In Standby mode, the PLL, the HSI, the MSI and the HSE oscillators are switched
  *        off. The voltage regulator is disabled, except when SRAM2 content is preserved
  *        in which case the regulator is in low-power mode.
  *        SRAM1 and register contents are lost except for registers in the Backup domain and
  *        Standby circuitry. SRAM2 content can be preserved if the bit RRS is set in PWR_CR3 register.
  *        To enable this feature, the user can resort to HAL_PWREx_EnableSRAM2ContentRetention() API
  *        to set RRS bit.
  *        The BOR is available.
  * @note  The I/Os can be configured either with a pull-up or pull-down or can be kept in analog state.
  *        HAL_PWREx_EnableGPIOPullUp() and HAL_PWREx_EnableGPIOPullDown() respectively enable Pull Up and
  *        Pull Down state, HAL_PWREx_DisableGPIOPullUp() and HAL_PWREx_DisableGPIOPullDown() disable the
  *        same.
  *        These states are effective in Standby mode only if APC bit is set through
  *        HAL_PWREx_EnablePullUpPullDownConfig() API.
  * @retval None
  */
void HAL_PWR_EnterSTANDBYMode(void)
{
  /* Set Stand-by mode */
  MODIFY_REG(PWR->CR1, PWR_CR1_LPMS, PWR_CR1_LPMS_STANDBY);

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

/* This option is used to ensure that store operations are completed */
#if defined ( __CC_ARM)
  __force_stores();
#endif
  /* Request Wait For Interrupt */
  __WFI();
}



/**
  * @brief Indicate Sleep-On-Exit when returning from Handler mode to Thread mode.
  * @note Set SLEEPONEXIT bit of SCR register. When this bit is set, the processor
  *       re-enters SLEEP mode when an interruption handling is over.
  *       Setting this bit is useful when the processor is expected to run only on
  *       interruptions handling.
  * @retval None
  */
void HAL_PWR_EnableSleepOnExit(void)
{
  /* Set SLEEPONEXIT bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}


/**
  * @brief Disable Sleep-On-Exit feature when returning from Handler mode to Thread mode.
  * @note Clear SLEEPONEXIT bit of SCR register. When this bit is set, the processor
  *       re-enters SLEEP mode when an interruption handling is over.
  * @retval None
  */
void HAL_PWR_DisableSleepOnExit(void)
{
  /* Clear SLEEPONEXIT bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}



/**
  * @brief Enable CORTEX M4 SEVONPEND bit.
  * @note Set SEVONPEND bit of SCR register. When this bit is set, this causes
  *       WFE to wake up when an interrupt moves from inactive to pended.
  * @retval None
  */
void HAL_PWR_EnableSEVOnPend(void)
{
  /* Set SEVONPEND bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}


/**
  * @brief Disable CORTEX M4 SEVONPEND bit.
  * @note Clear SEVONPEND bit of SCR register. When this bit is set, this causes
  *       WFE to wake up when an interrupt moves from inactive to pended.
  * @retval None
  */
void HAL_PWR_DisableSEVOnPend(void)
{
  /* Clear SEVONPEND bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}





/**
  * @brief PWR PVD interrupt callback
  * @retval None
  */
__weak void HAL_PWR_PVDCallback(void)
{
  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_PWR_PVDCallback can be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_PWR_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
