#include <furi_hal_debug.h>

#include <stm32wbxx_ll_exti.h>
#include <stm32wbxx_ll_system.h>

#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>

volatile bool furi_hal_debug_gdb_session_active = false;

void furi_hal_debug_enable(void) {
    // Low power mode debug
    LL_DBGMCU_EnableDBGSleepMode();
    LL_DBGMCU_EnableDBGStopMode();
    LL_DBGMCU_EnableDBGStandbyMode();
    LL_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);
    // SWD GPIO
    furi_hal_gpio_init_ex(
        &gpio_swdio,
        GpioModeAltFunctionPushPull,
        GpioPullUp,
        GpioSpeedVeryHigh,
        GpioAltFn0JTMS_SWDIO);
    furi_hal_gpio_init_ex(
        &gpio_swclk, GpioModeAltFunctionPushPull, GpioPullDown, GpioSpeedLow, GpioAltFn0JTCK_SWCLK);
}

void furi_hal_debug_disable(void) {
    // Low power mode debug
    LL_DBGMCU_DisableDBGSleepMode();
    LL_DBGMCU_DisableDBGStopMode();
    LL_DBGMCU_DisableDBGStandbyMode();
    LL_EXTI_DisableIT_32_63(LL_EXTI_LINE_48);
    // SWD GPIO
    furi_hal_gpio_init_simple(&gpio_swdio, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_swclk, GpioModeAnalog);
}

bool furi_hal_debug_is_gdb_session_active(void) {
    return furi_hal_debug_gdb_session_active;
}
