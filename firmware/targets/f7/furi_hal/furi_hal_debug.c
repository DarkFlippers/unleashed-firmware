#include <furi_hal_debug.h>

#include <stm32wbxx_ll_exti.h>
#include <stm32wbxx_ll_system.h>

void furi_hal_debug_enable() {
    // Low power mode debug
    LL_DBGMCU_EnableDBGSleepMode();
    LL_DBGMCU_EnableDBGStopMode();
    LL_DBGMCU_EnableDBGStandbyMode();
    LL_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);
}

void furi_hal_debug_disable() {
    // Low power mode debug
    LL_DBGMCU_DisableDBGSleepMode();
    LL_DBGMCU_DisableDBGStopMode();
    LL_DBGMCU_DisableDBGStandbyMode();
    LL_EXTI_DisableIT_32_63(LL_EXTI_LINE_48);
}
