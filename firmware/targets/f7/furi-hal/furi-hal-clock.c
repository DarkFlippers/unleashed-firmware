#include <furi-hal-clock.h>

#include <main.h>
#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_utils.h>

#define HS_CLOCK_IS_READY() (LL_RCC_HSE_IsReady() && LL_RCC_HSI_IsReady())
#define LS_CLOCK_IS_READY() (LL_RCC_LSE_IsReady() && LL_RCC_LSI1_IsReady())

void furi_hal_clock_init() {
    /* Prepare Flash memory for 64mHz system clock */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_3);

    /* HSE and HSI configuration and activation */
    LL_RCC_HSE_SetCapacitorTuning(0x26);
    LL_RCC_HSE_Enable();
    LL_RCC_HSI_Enable();
    while(!HS_CLOCK_IS_READY());
    LL_RCC_HSE_EnableCSS();

    /* LSE and LSI1 configuration and activation */
    LL_PWR_EnableBkUpAccess();
    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_HIGH);
    LL_RCC_LSE_Enable();
    LL_RCC_LSI1_Enable();
    while(!LS_CLOCK_IS_READY());
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_18); /* Why? Because that's why. See RM0434, Table 61. CPU1 vector table. */
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_18);
    LL_RCC_EnableIT_LSECSS();
    LL_RCC_LSE_EnableCSS();

    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 8, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while(LL_RCC_PLL_IsReady() != 1);

    LL_RCC_PLLSAI1_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 6, LL_RCC_PLLSAI1Q_DIV_2);
    LL_RCC_PLLSAI1_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 6, LL_RCC_PLLSAI1R_DIV_2);
    LL_RCC_PLLSAI1_Enable();
    LL_RCC_PLLSAI1_EnableDomain_48M();
    LL_RCC_PLLSAI1_EnableDomain_ADC();
    while(LL_RCC_PLLSAI1_IsReady() != 1);

    /* Sysclk activation on the main PLL */
    /* Set CPU1 prescaler*/
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    /* Set CPU2 prescaler*/
    LL_C2_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    /* Set AHB SHARED prescaler*/
    LL_RCC_SetAHB4Prescaler(LL_RCC_SYSCLK_DIV_1);

    /* Set APB1 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    /* Set APB2 prescaler*/
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Disable MSI */
    LL_RCC_MSI_Disable();
    while(LL_RCC_MSI_IsReady() != 0);

    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(64000000);

    /* Update the time base */
    if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK) {
        Error_Handler();
    }

    if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    }

    LL_RCC_EnableRTC();

    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
    LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_PLLSAI1);
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
    LL_RCC_SetRNGClockSource(LL_RCC_RNG_CLKSOURCE_CLK48);
    LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLLSAI1);
    LL_RCC_SetCLK48ClockSource(LL_RCC_CLK48_CLKSOURCE_PLLSAI1);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSE);
    LL_RCC_SetSMPSPrescaler(LL_RCC_SMPS_DIV_1);
    LL_RCC_SetRFWKPClockSource(LL_RCC_RFWKP_CLKSOURCE_LSE);

    // AHB1
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

    // AHB2
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOH);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

    // APB1
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    // APB2
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
}

void furi_hal_clock_switch_to_hsi() {
    LL_RCC_HSI_Enable( );

    while(!LL_RCC_HSI_IsReady());

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSI);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI);
}

void furi_hal_clock_switch_to_pll() {
    LL_RCC_HSE_Enable();
    LL_RCC_PLL_Enable();

    while(!LL_RCC_HSE_IsReady());
    while(!LL_RCC_PLL_IsReady());

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSE);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
}
