#include <furi_hal_clock.h>
#include <furi_hal_resources.h>
#include <furi.h>

#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_cortex.h>
#include <stm32wbxx_ll_bus.h>

#define TAG "FuriHalClock"

#define CPU_CLOCK_HZ_EARLY 4000000
#define CPU_CLOCK_HZ_MAIN 64000000
#define TICK_INT_PRIORITY 15U
#define HS_CLOCK_IS_READY() (LL_RCC_HSE_IsReady() && LL_RCC_HSI_IsReady())
#define LS_CLOCK_IS_READY() (LL_RCC_LSE_IsReady() && LL_RCC_LSI1_IsReady())

void furi_hal_clock_init_early() {
    LL_SetSystemCoreClock(CPU_CLOCK_HZ_EARLY);
    LL_Init1msTick(SystemCoreClock);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOH);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPTIM2);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C3);
}

void furi_hal_clock_deinit_early() {
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C3);

    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI2);

    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOE);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOH);
}

void furi_hal_clock_init() {
    /* Prepare Flash memory for 64MHz system clock */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_3)
        ;

    /* HSE and HSI configuration and activation */
    LL_RCC_HSE_SetCapacitorTuning(0x26);
    LL_RCC_HSE_Enable();
    LL_RCC_HSI_Enable();
    while(!HS_CLOCK_IS_READY())
        ;
    LL_RCC_HSE_EnableCSS();

    /* LSE and LSI1 configuration and activation */
    LL_PWR_EnableBkUpAccess();
    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_HIGH);
    LL_RCC_LSE_Enable();
    LL_RCC_LSI1_Enable();
    while(!LS_CLOCK_IS_READY())
        ;
    LL_EXTI_EnableIT_0_31(
        LL_EXTI_LINE_18); /* Why? Because that's why. See RM0434, Table 61. CPU1 vector table. */
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_18);
    LL_RCC_EnableIT_LSECSS();
    /* ES0394, extended case of 2.2.2 */
    if(!LL_RCC_IsActiveFlag_BORRST()) {
        LL_RCC_LSE_EnableCSS();
    }

    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 8, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while(LL_RCC_PLL_IsReady() != 1)
        ;

    LL_RCC_PLLSAI1_ConfigDomain_48M(
        LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 6, LL_RCC_PLLSAI1Q_DIV_2);
    LL_RCC_PLLSAI1_ConfigDomain_ADC(
        LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 6, LL_RCC_PLLSAI1R_DIV_2);
    LL_RCC_PLLSAI1_Enable();
    LL_RCC_PLLSAI1_EnableDomain_48M();
    LL_RCC_PLLSAI1_EnableDomain_ADC();
    while(LL_RCC_PLLSAI1_IsReady() != 1)
        ;

    /* Sysclk activation on the main PLL */
    /* Set CPU1 prescaler*/
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    /* Set CPU2 prescaler*/
    LL_C2_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
        ;

    /* Set AHB SHARED prescaler*/
    LL_RCC_SetAHB4Prescaler(LL_RCC_SYSCLK_DIV_1);

    /* Set APB1 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    /* Set APB2 prescaler*/
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Disable MSI */
    LL_RCC_MSI_Disable();
    while(LL_RCC_MSI_IsReady() != 0)
        ;

    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(CPU_CLOCK_HZ_MAIN);

    /* Update the time base */
    LL_Init1msTick(SystemCoreClock);
    LL_SYSTICK_EnableIT();
    NVIC_SetPriority(
        SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), TICK_INT_PRIORITY, 0));
    NVIC_EnableIRQ(SysTick_IRQn);

    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);
    LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_PLLSAI1);
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
    LL_RCC_SetRNGClockSource(LL_RCC_RNG_CLKSOURCE_CLK48);
    LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLLSAI1);
    LL_RCC_SetCLK48ClockSource(LL_RCC_CLK48_CLKSOURCE_PLLSAI1);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSE);
    LL_RCC_SetSMPSPrescaler(LL_RCC_SMPS_DIV_1);
    LL_RCC_SetRFWKPClockSource(LL_RCC_RFWKP_CLKSOURCE_LSE);

    // AHB1 GRP1
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);
    // LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_TSC);

    // AHB2 GRP1
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOH);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_AES1);

    // AHB3 GRP1
    // LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_QUADSPI);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_PKA);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_AES2);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_RNG);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_HSEM);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_IPCC);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_FLASH);

    // APB1 GRP1
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    // LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LCD);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);
    // LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_WWDG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C3);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_CRS);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USB);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPTIM1);

    // APB1 GRP2
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);

    // APB2
    // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM16);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM17);
    // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SAI1);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_clock_switch_to_hsi() {
    LL_RCC_HSI_Enable();

    while(!LL_RCC_HSI_IsReady())
        ;

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSI);

    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
        ;

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
}

void furi_hal_clock_switch_to_pll() {
    LL_RCC_HSE_Enable();
    LL_RCC_PLL_Enable();

    while(!LL_RCC_HSE_IsReady())
        ;
    while(!LL_RCC_PLL_IsReady())
        ;

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSE);

    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
        ;
}

void furi_hal_clock_suspend_tick() {
    CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}

void furi_hal_clock_resume_tick() {
    SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}

void furi_hal_clock_mco_enable(FuriHalClockMcoSourceId source, FuriHalClockMcoDivisorId div) {
    if(source == FuriHalClockMcoLse) {
        LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_LSE, div);
    } else if(source == FuriHalClockMcoSysclk) {
        LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_SYSCLK, div);
    } else {
        LL_RCC_MSI_Enable();
        while(LL_RCC_MSI_IsReady() != 1)
            ;
        switch(source) {
        case FuriHalClockMcoMsi100k:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_0);
            break;
        case FuriHalClockMcoMsi200k:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_1);
            break;
        case FuriHalClockMcoMsi400k:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_2);
            break;
        case FuriHalClockMcoMsi800k:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_3);
            break;
        case FuriHalClockMcoMsi1m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_4);
            break;
        case FuriHalClockMcoMsi2m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_5);
            break;
        case FuriHalClockMcoMsi4m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
            break;
        case FuriHalClockMcoMsi8m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_7);
            break;
        case FuriHalClockMcoMsi16m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_8);
            break;
        case FuriHalClockMcoMsi24m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
            break;
        case FuriHalClockMcoMsi32m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_10);
            break;
        case FuriHalClockMcoMsi48m:
            LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_11);
            break;
        default:
            break;
        }
        LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_MSI, div);
    }
}

void furi_hal_clock_mco_disable() {
    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_NOCLOCK, FuriHalClockMcoDiv1);
    LL_RCC_MSI_Disable();
    while(LL_RCC_MSI_IsReady() != 0)
        ;
}