#include <api-hal-clock.h>

#include <stm32wbxx_ll_rcc.h>

void api_hal_clock_switch_to_hsi() {
    LL_RCC_HSI_Enable( );

    while(!LL_RCC_HSI_IsReady());

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSI);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI);
}

void api_hal_clock_switch_to_pll() {
    LL_RCC_HSE_Enable();
    LL_RCC_PLL_Enable();

    while(!LL_RCC_HSE_IsReady());
    while(!LL_RCC_PLL_IsReady());

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSE);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
}
