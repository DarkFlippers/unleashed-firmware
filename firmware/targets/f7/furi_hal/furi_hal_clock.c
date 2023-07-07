#include <furi_hal_clock.h>
#include <furi_hal_resources.h>
#include <furi.h>

#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_cortex.h>

#define TAG "FuriHalClock"

#define CPU_CLOCK_HZ_EARLY 4000000
#define CPU_CLOCK_HZ_MAIN 64000000
#define TICK_INT_PRIORITY 15U
#define HS_CLOCK_IS_READY() (LL_RCC_HSE_IsReady() && LL_RCC_HSI_IsReady())
#define LS_CLOCK_IS_READY() (LL_RCC_LSE_IsReady() && LL_RCC_LSI1_IsReady())

void furi_hal_clock_init_early() {
    LL_SetSystemCoreClock(CPU_CLOCK_HZ_EARLY);
    LL_Init1msTick(SystemCoreClock);
}

void furi_hal_clock_deinit_early() {
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
    /* Select HSI as system clock source after Wake Up from Stop mode
     * Must be set before enabling CSS */
    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);

    LL_RCC_HSE_EnableCSS();

    /* LSE and LSI1 configuration and activation */
    LL_PWR_EnableBkUpAccess();
    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_HIGH);
    LL_RCC_LSE_Enable();
    LL_RCC_LSI1_Enable();
    while(!LS_CLOCK_IS_READY())
        ;

    /* RF wakeup */
    LL_RCC_SetRFWKPClockSource(LL_RCC_RFWKP_CLKSOURCE_LSE);

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

    LL_RCC_SetCLK48ClockSource(LL_RCC_CLK48_CLKSOURCE_PLLSAI1);
    LL_RCC_HSI_EnableInStopMode(); // Ensure that MR is capable of work in STOP0
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSI);
    LL_RCC_SetSMPSPrescaler(LL_RCC_SMPS_DIV_1);
    LL_RCC_SetRFWKPClockSource(LL_RCC_RFWKP_CLKSOURCE_LSE);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_clock_switch_to_hsi() {
    LL_RCC_HSI_Enable();

    while(!LL_RCC_HSI_IsReady())
        ;

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    furi_assert(LL_RCC_GetSMPSClockSource() == LL_RCC_SMPS_CLKSOURCE_HSI);

    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
        ;

    LL_C2_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
        ;
}

void furi_hal_clock_switch_to_pll() {
#ifdef FURI_HAL_CLOCK_TRACK_STARTUP
    uint32_t clock_start_time = DWT->CYCCNT;
#endif

    LL_RCC_HSE_Enable();
    LL_RCC_PLL_Enable();
    LL_RCC_PLLSAI1_Enable();

    while(!LL_RCC_HSE_IsReady())
        ;
    while(!LL_RCC_PLL_IsReady())
        ;
    while(!LL_RCC_PLLSAI1_IsReady())
        ;

    LL_C2_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_3)
        ;

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
        ;

#ifdef FURI_HAL_CLOCK_TRACK_STARTUP
    uint32_t total = DWT->CYCCNT - clock_start_time;
    if(total > (20 * 0x148)) {
        furi_crash("Slow HSE/PLL startup");
    }
#endif
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
