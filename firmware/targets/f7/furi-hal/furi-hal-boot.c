#include <furi-hal-boot.h>
#include <stm32wbxx_ll_rtc.h>
#include <furi.h>

// Boot request enum
#define BOOT_REQUEST_TAINTED 0x00000000
#define BOOT_REQUEST_CLEAN 0xDADEDADE
#define BOOT_REQUEST_DFU 0xDF00B000

void furi_hal_boot_init() {
#ifndef DEBUG
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_TAINTED);
#endif
    FURI_LOG_I("FuriHalBoot", "Init OK");
}

void furi_hal_boot_set_mode(FuriHalBootMode mode) {
    if (mode == FuriHalBootModeNormal) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_CLEAN);
    } else if (mode == FuriHalBootModeDFU) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_DFU);
    }
}

void furi_hal_boot_set_flags(FuriHalBootFlag flags) {
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR2, flags);
}

FuriHalBootFlag furi_hal_boot_get_flags() {
    return LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR2);
}