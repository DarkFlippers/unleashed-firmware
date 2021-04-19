#include <api-hal-version.h>
#include <stm32wbxx.h>
#include <stm32wbxx_ll_rtc.h>

typedef struct {
    uint8_t version;
    uint8_t target;
    uint8_t body;
    uint8_t connect;
    uint32_t timestamp;
    char name[8];
} ApiHalVersionOTP;

bool api_hal_version_do_i_belong_here() {
    return api_hal_version_get_hw_target() == 5;
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

const char * api_hal_version_get_name_ptr() {
    char * name = ((ApiHalVersionOTP*)OTP_AREA_BASE)->name;
    return *name == 0xFFU ? NULL : name; 
}

const struct Version* api_hal_version_get_fw_version(void) {
    return version_get();
}

const struct Version* api_hal_version_get_boot_version(void) {
#ifdef NO_BOOTLOADER
    return 0;
#else
    /* Backup register which points to structure in flash memory */
    return (const struct Version*) LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR1);
#endif
}

