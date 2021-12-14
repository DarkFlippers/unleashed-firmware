#include <furi-hal-bootloader.h>
#include <stm32wbxx_ll_rtc.h>
#include <furi.h>

#define TAG "FuriHalBoot"

// Boot request enum
#define BOOT_REQUEST_TAINTED 0x00000000
#define BOOT_REQUEST_CLEAN 0xDADEDADE
#define BOOT_REQUEST_DFU 0xDF00B000

void furi_hal_bootloader_init() {
#ifndef DEBUG
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_TAINTED);
#endif
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_bootloader_set_mode(FuriHalBootloaderMode mode) {
    if (mode == FuriHalBootloaderModeNormal) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_CLEAN);
    } else if (mode == FuriHalBootloaderModeDFU) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_DFU);
    }
}
