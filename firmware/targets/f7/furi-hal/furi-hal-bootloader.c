#include <furi-hal-bootloader.h>
#include <furi-hal-rtc.h>
#include <furi.h>

#define TAG "FuriHalBoot"

// Boot request enum
#define BOOT_REQUEST_TAINTED 0x00000000
#define BOOT_REQUEST_CLEAN 0xDADEDADE
#define BOOT_REQUEST_DFU 0xDF00B000

void furi_hal_bootloader_init() {
#ifndef DEBUG
    furi_hal_rtc_set_register(FuriHalRtcRegisterBoot, BOOT_REQUEST_TAINTED);
#endif
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_bootloader_set_mode(FuriHalBootloaderMode mode) {
    if (mode == FuriHalBootloaderModeNormal) {
        furi_hal_rtc_set_register(FuriHalRtcRegisterBoot, BOOT_REQUEST_CLEAN);
    } else if (mode == FuriHalBootloaderModeDFU) {
        furi_hal_rtc_set_register(FuriHalRtcRegisterBoot, BOOT_REQUEST_DFU);
    }
}
