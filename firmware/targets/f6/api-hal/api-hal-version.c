#include <api-hal-version.h>
#include <stm32wbxx.h>
#include <stm32wbxx_ll_rtc.h>
#include <stdio.h>
#include "ble.h"

#define FLIPPER_NAME_LENGTH 8

typedef struct {
    uint8_t version;
    uint8_t target;
    uint8_t body;
    uint8_t connect;
    uint32_t timestamp;
    char name[FLIPPER_NAME_LENGTH];
} ApiHalVersionOTP;

#define FLIPPER_ARRAY_NAME_LENGTH (FLIPPER_NAME_LENGTH + 1)
// BLE symbol + "Flipper " + name
#define FLIPPER_DEVICE_NAME_LENGTH (1 + 8 + FLIPPER_ARRAY_NAME_LENGTH)

// Initialiazed from OTP, used to guarantee zero terminated C string
static char flipper_name[FLIPPER_ARRAY_NAME_LENGTH];
static char flipper_device_name[FLIPPER_DEVICE_NAME_LENGTH];

void api_hal_version_init() {
    char* name = ((ApiHalVersionOTP*)OTP_AREA_BASE)->name;
    strlcpy(flipper_name, name, FLIPPER_ARRAY_NAME_LENGTH);

    if(api_hal_version_get_name_ptr() != NULL) {
        snprintf(
            flipper_device_name,
            FLIPPER_DEVICE_NAME_LENGTH,
            "xFlipper %s",
            flipper_name);
    } else {
        snprintf(
            flipper_device_name,
            FLIPPER_DEVICE_NAME_LENGTH,
            "xFlipper");
    }

    flipper_device_name[0] = AD_TYPE_COMPLETE_LOCAL_NAME;
}

bool api_hal_version_do_i_belong_here() {
    return api_hal_version_get_hw_target() == 6;
}

const uint8_t api_hal_version_get_hw_version() {
    return ((ApiHalVersionOTP*)OTP_AREA_BASE)->version;
}

const uint8_t api_hal_version_get_hw_target() {
    return ((ApiHalVersionOTP*)OTP_AREA_BASE)->target;
}

const uint8_t api_hal_version_get_hw_body() {
    return ((ApiHalVersionOTP*)OTP_AREA_BASE)->body;
}

const uint8_t api_hal_version_get_hw_connect() {
    return ((ApiHalVersionOTP*)OTP_AREA_BASE)->connect;
}

const uint32_t api_hal_version_get_hw_timestamp() {
    return ((ApiHalVersionOTP*)OTP_AREA_BASE)->timestamp;
}

const char* api_hal_version_get_name_ptr() {
    return *flipper_name == 0xFFU ? NULL : flipper_name;
}

const char* api_hal_version_get_device_name_ptr() {
    return flipper_device_name + 1;
}

const char* api_hal_version_get_ble_local_device_name_ptr() {
    return flipper_device_name;
}

const struct Version* api_hal_version_get_fw_version(void) {
    return version_get();
}

const struct Version* api_hal_version_get_boot_version(void) {
#ifdef NO_BOOTLOADER
    return 0;
#else
    /* Backup register which points to structure in flash memory */
    return (const struct Version*)LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR1);
#endif
}
