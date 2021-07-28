#include <api-hal-version.h>

#include <furi.h>
#include <stm32wbxx.h>
#include <stm32wbxx_ll_rtc.h>

#include <stdio.h>
#include "ble.h"

#define API_HAL_VERSION_OTP_HEADER_MAGIC 0xBABE
#define API_HAL_VERSION_NAME_LENGTH 8
#define API_HAL_VERSION_ARRAY_NAME_LENGTH (API_HAL_VERSION_NAME_LENGTH + 1)
/** BLE symbol + "Flipper " + name */
#define API_HAL_VERSION_DEVICE_NAME_LENGTH (1 + 8 + API_HAL_VERSION_ARRAY_NAME_LENGTH)
#define API_HAL_VERSION_OTP_ADDRESS OTP_AREA_BASE

/** OTP Versions enum */
typedef enum {
    ApiHalVersionOtpVersion0=0x00,
    ApiHalVersionOtpVersion1=0x01,
    ApiHalVersionOtpVersionEmpty=0xFFFFFFFE,
    ApiHalVersionOtpVersionUnknown=0xFFFFFFFF,
} ApiHalVersionOtpVersion;

/** OTP V0 Structure: prototypes and early EVT */
typedef struct {
    uint8_t board_version;
    uint8_t board_target;
    uint8_t board_body;
    uint8_t board_connect;
    uint32_t header_timestamp;
    char name[API_HAL_VERSION_NAME_LENGTH];
} ApiHalVersionOTPv0;

/** OTP V1 Structure: late EVT, DVT, PVT, Production */
typedef struct {
    /* First 64 bits: header */
    uint16_t header_magic;
    uint8_t header_version;
    uint8_t header_reserved;
    uint32_t header_timestamp;

    /* Second 64 bits: board info */
    uint8_t board_version; /** Board version */
    uint8_t board_target; /** Board target firmware */
    uint8_t board_body; /** Board body */
    uint8_t board_connect; /** Board interconnect */
    uint8_t board_color; /** Board color */
    uint8_t board_region; /** Board region */
    uint16_t board_reserved; /** Reserved for future use, 0x0000 */

    /* Third 64 bits: Unique Device Name */
    char name[API_HAL_VERSION_NAME_LENGTH]; /** Unique Device Name */
} ApiHalVersionOTPv1;

/** Represenation Model: */
typedef struct {
    ApiHalVersionOtpVersion otp_version;

    uint32_t timestamp;

    uint8_t board_version; /** Board version */
    uint8_t board_target; /** Board target firmware */
    uint8_t board_body; /** Board body */
    uint8_t board_connect; /** Board interconnect */
    uint8_t board_color; /** Board color */
    uint8_t board_region; /** Board region */

    char name[API_HAL_VERSION_ARRAY_NAME_LENGTH]; /** \0 terminated name */
    char device_name[API_HAL_VERSION_DEVICE_NAME_LENGTH];  /** device name for special needs */
    uint8_t ble_mac[6];
} ApiHalVersion;

static ApiHalVersion api_hal_version = {0};

static ApiHalVersionOtpVersion api_hal_version_get_otp_version() {
    if (*(uint64_t*)API_HAL_VERSION_OTP_ADDRESS == 0xFFFFFFFF) {
        return ApiHalVersionOtpVersionEmpty;
    } else {
        if (((ApiHalVersionOTPv1*)API_HAL_VERSION_OTP_ADDRESS)->header_magic == API_HAL_VERSION_OTP_HEADER_MAGIC) {
            return ApiHalVersionOtpVersion1;
        } else if (((ApiHalVersionOTPv0*)API_HAL_VERSION_OTP_ADDRESS)->board_version <= 10) {
            return ApiHalVersionOtpVersion0;
        } else {
            return ApiHalVersionOtpVersionUnknown;
        }
    }
}

static void api_hal_version_set_name(const char* name) {
    if(name != NULL) {
        strlcpy(api_hal_version.name, name, API_HAL_VERSION_ARRAY_NAME_LENGTH);
        snprintf(
            api_hal_version.device_name,
            API_HAL_VERSION_DEVICE_NAME_LENGTH,
            "xFlipper %s",
            api_hal_version.name);
    } else {
        snprintf(
            api_hal_version.device_name,
            API_HAL_VERSION_DEVICE_NAME_LENGTH,
            "xFlipper");
    }

    api_hal_version.device_name[0] = AD_TYPE_COMPLETE_LOCAL_NAME;

    // BLE Mac address
    uint32_t udn = LL_FLASH_GetUDN();
    uint32_t company_id = LL_FLASH_GetSTCompanyID();
    uint32_t device_id = LL_FLASH_GetDeviceID();
    api_hal_version.ble_mac[0] = (uint8_t)(udn & 0x000000FF);
    api_hal_version.ble_mac[1] = (uint8_t)( (udn & 0x0000FF00) >> 8 );
    api_hal_version.ble_mac[2] = (uint8_t)( (udn & 0x00FF0000) >> 16 );
    api_hal_version.ble_mac[3] = (uint8_t)device_id;
    api_hal_version.ble_mac[4] = (uint8_t)(company_id & 0x000000FF);
    api_hal_version.ble_mac[5] = (uint8_t)( (company_id & 0x0000FF00) >> 8 );
}

static void api_hal_version_load_otp_default() {
    api_hal_version_set_name(NULL);
}

static void api_hal_version_load_otp_v0() {
    const ApiHalVersionOTPv0* otp = (ApiHalVersionOTPv0*)API_HAL_VERSION_OTP_ADDRESS;

    api_hal_version.timestamp = otp->header_timestamp;
    api_hal_version.board_version = otp->board_version;
    api_hal_version.board_target = otp->board_target;
    api_hal_version.board_body = otp->board_body;
    api_hal_version.board_connect = otp->board_connect;
    api_hal_version.board_color = 0;
    api_hal_version.board_region = 0;

    api_hal_version_set_name(otp->name);
}

static void api_hal_version_load_otp_v1() {
    const ApiHalVersionOTPv1* otp = (ApiHalVersionOTPv1*)API_HAL_VERSION_OTP_ADDRESS;

    api_hal_version.timestamp = otp->header_timestamp;
    api_hal_version.board_version = otp->board_version;
    api_hal_version.board_target = otp->board_target;
    api_hal_version.board_body = otp->board_body;
    api_hal_version.board_connect = otp->board_connect;
    api_hal_version.board_color = otp->board_color;
    api_hal_version.board_region = otp->board_region;

    api_hal_version_set_name(otp->name);
}

void api_hal_version_init() {
    api_hal_version.otp_version = api_hal_version_get_otp_version();
    switch(api_hal_version.otp_version) {
        case ApiHalVersionOtpVersionUnknown:
            api_hal_version_load_otp_default();
        break;
        case ApiHalVersionOtpVersionEmpty:
            api_hal_version_load_otp_default();
        break;
        case ApiHalVersionOtpVersion0:
            api_hal_version_load_otp_v0();
        break;
        case ApiHalVersionOtpVersion1:
            api_hal_version_load_otp_v1();
        break;
        default: furi_check(0);
    }
    FURI_LOG_I("FuriHalVersion", "Init OK");
}

bool api_hal_version_do_i_belong_here() {
    return api_hal_version_get_hw_target() == 6;
}

const char* api_hal_version_get_model_name() {
    return "Flipper Zero";
}

const uint8_t api_hal_version_get_hw_version() {
    return api_hal_version.board_version;
}

const uint8_t api_hal_version_get_hw_target() {
    return api_hal_version.board_target;
}

const uint8_t api_hal_version_get_hw_body() {
    return api_hal_version.board_body;
}

const ApiHalVersionColor api_hal_version_get_hw_color() {
    return api_hal_version.board_color;
}

const uint8_t api_hal_version_get_hw_connect() {
    return api_hal_version.board_connect;
}

const ApiHalVersionRegion api_hal_version_get_hw_region() {
    return api_hal_version.board_region;
}

const uint32_t api_hal_version_get_hw_timestamp() {
    return api_hal_version.timestamp;
}

const char* api_hal_version_get_name_ptr() {
    return *api_hal_version.name == 0x00 ? NULL : api_hal_version.name;
}

const char* api_hal_version_get_device_name_ptr() {
    return api_hal_version.device_name + 1;
}

const char* api_hal_version_get_ble_local_device_name_ptr() {
    return api_hal_version.device_name;
}

const uint8_t* api_hal_version_get_ble_mac() {
    return api_hal_version.ble_mac;
}

const struct Version* api_hal_version_get_firmware_version(void) {
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

size_t api_hal_version_uid_size() {
    return 64/8;
}

const uint8_t* api_hal_version_uid() {
    return (const uint8_t *)UID64_BASE;
}
