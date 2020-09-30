#include "boot.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_pwr.h"

#define BOOT_REQUEST_DFU 0xDF00B000

void boot_restart_in_dfu() {
    // Request DFU on boot
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_EnableBkUpAccess();
    // Enable RTC
    LL_RCC_EnableRTC();
    // Write backup registry
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_DFU);
    // Reset
    NVIC_SystemReset();
}
