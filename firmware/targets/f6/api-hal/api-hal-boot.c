#include <api-hal-boot.h>
#include <stm32wbxx_ll_rtc.h>

// Boot request enum
#define BOOT_REQUEST_TAINTED 0x00000000
#define BOOT_REQUEST_CLEAN 0xDADEDADE
#define BOOT_REQUEST_DFU 0xDF00B000

void api_hal_boot_init() {
#ifndef DEBUG
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_TAINTED);
#endif
}

void api_hal_boot_set_mode(ApiHalBootMode mode) {
    if (mode == ApiHalBootModeNormal) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_CLEAN);
    } else if (mode == ApiHalBootModeDFU) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_DFU);
    }
}

void api_hal_boot_set_flags(ApiHalBootFlag flags) {
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR2, flags);
}

ApiHalBootFlag api_hal_boot_get_flags() {
    return LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR2);
}