#include <furi_hal_version.h>
#include <furi_hal_rtc.h>

#include <furi.h>
#include <stm32wbxx.h>
#include <stm32wbxx_ll_rtc.h>

#include <stdio.h>
#include <ble/ble.h>

#define TAG "FuriHalVersion"

#define FURI_HAL_VERSION_OTP_HEADER_MAGIC 0xBABE
#define FURI_HAL_VERSION_OTP_ADDRESS      OTP_AREA_BASE

/** OTP V0 Structure: prototypes and early EVT */
typedef struct {
    uint8_t board_version;
    uint8_t board_target;
    uint8_t board_body;
    uint8_t board_connect;
    uint32_t header_timestamp;
    char name[FURI_HAL_VERSION_NAME_LENGTH];
} FuriHalVersionOTPv0;

/** OTP V1 Structure: late EVT, DVT */
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
    char name[FURI_HAL_VERSION_NAME_LENGTH]; /** Unique Device Name */
} FuriHalVersionOTPv1;

/** OTP V2 Structure: DVT2, PVT, Production */
typedef struct {
    /* Early First 64 bits: header */
    uint16_t header_magic;
    uint8_t header_version;
    uint8_t header_reserved;
    uint32_t header_timestamp;

    /* Early Second 64 bits: board info */
    uint8_t board_version; /** Board version */
    uint8_t board_target; /** Board target firmware */
    uint8_t board_body; /** Board body */
    uint8_t board_connect; /** Board interconnect */
    uint8_t board_display; /** Board display */
    uint8_t board_reserved2_0; /** Reserved for future use, 0x00 */
    uint16_t board_reserved2_1; /** Reserved for future use, 0x0000 */

    /* Late Third 64 bits: device info */
    uint8_t board_color; /** Board color */
    uint8_t board_region; /** Board region */
    uint16_t board_reserved3_0; /** Reserved for future use, 0x0000 */
    uint32_t board_reserved3_1; /** Reserved for future use, 0x00000000 */

    /* Late Fourth 64 bits: Unique Device Name */
    char name[FURI_HAL_VERSION_NAME_LENGTH]; /** Unique Device Name */
} FuriHalVersionOTPv2;

/** Represenation Model: */
typedef struct {
    uint32_t timestamp;

    uint8_t board_version; /** Board version */
    uint8_t board_target; /** Board target firmware */
    uint8_t board_body; /** Board body */
    uint8_t board_connect; /** Board interconnect */
    uint8_t board_color; /** Board color */
    uint8_t board_region; /** Board region */
    uint8_t board_display; /** Board display */

    char name[FURI_HAL_VERSION_ARRAY_NAME_LENGTH]; /** \0 terminated name */
    char device_name[FURI_HAL_VERSION_DEVICE_NAME_LENGTH]; /** device name for special needs */
    uint8_t ble_mac[6];
} FuriHalVersion;

static FuriHalVersion furi_hal_version = {0};

static void furi_hal_version_set_name(const char* name) {
    if(name != NULL) {
        strlcpy(furi_hal_version.name, name, FURI_HAL_VERSION_ARRAY_NAME_LENGTH);
        snprintf(
            furi_hal_version.device_name,
            FURI_HAL_VERSION_DEVICE_NAME_LENGTH,
            "xFlipper %s",
            furi_hal_version.name);
    } else {
        strlcpy(furi_hal_version.device_name, "xFlipper", FURI_HAL_VERSION_DEVICE_NAME_LENGTH);
    }

    furi_hal_version.device_name[0] = AD_TYPE_COMPLETE_LOCAL_NAME;

    // BLE Mac address
    uint32_t udn = LL_FLASH_GetUDN();
    uint32_t company_id = LL_FLASH_GetSTCompanyID();
    uint32_t device_id = LL_FLASH_GetDeviceID();
    furi_hal_version.ble_mac[0] = (uint8_t)(udn & 0x000000FF);
    furi_hal_version.ble_mac[1] = (uint8_t)((udn & 0x0000FF00) >> 8);
    furi_hal_version.ble_mac[2] = (uint8_t)((udn & 0x00FF0000) >> 16);
    furi_hal_version.ble_mac[3] = (uint8_t)device_id;
    furi_hal_version.ble_mac[4] = (uint8_t)(company_id & 0x000000FF);
    furi_hal_version.ble_mac[5] = (uint8_t)((company_id & 0x0000FF00) >> 8);
}

static void furi_hal_version_load_otp_default(void) {
    furi_hal_version_set_name(NULL);
}

static void furi_hal_version_load_otp_v0(void) {
    const FuriHalVersionOTPv0* otp = (FuriHalVersionOTPv0*)FURI_HAL_VERSION_OTP_ADDRESS;

    furi_hal_version.timestamp = otp->header_timestamp;
    furi_hal_version.board_version = otp->board_version;
    furi_hal_version.board_target = otp->board_target;
    furi_hal_version.board_body = otp->board_body;
    furi_hal_version.board_connect = otp->board_connect;

    furi_hal_version_set_name(otp->name);
}

static void furi_hal_version_load_otp_v1(void) {
    const FuriHalVersionOTPv1* otp = (FuriHalVersionOTPv1*)FURI_HAL_VERSION_OTP_ADDRESS;

    furi_hal_version.timestamp = otp->header_timestamp;
    furi_hal_version.board_version = otp->board_version;
    furi_hal_version.board_target = otp->board_target;
    furi_hal_version.board_body = otp->board_body;
    furi_hal_version.board_connect = otp->board_connect;
    furi_hal_version.board_color = otp->board_color;
    furi_hal_version.board_region = otp->board_region;

    furi_hal_version_set_name(otp->name);
}

static void furi_hal_version_load_otp_v2(void) {
    const FuriHalVersionOTPv2* otp = (FuriHalVersionOTPv2*)FURI_HAL_VERSION_OTP_ADDRESS;

    // 1st block, programmed afer baking
    furi_hal_version.timestamp = otp->header_timestamp;

    // 2nd block, programmed afer baking
    furi_hal_version.board_version = otp->board_version;
    furi_hal_version.board_target = otp->board_target;
    furi_hal_version.board_body = otp->board_body;
    furi_hal_version.board_connect = otp->board_connect;
    furi_hal_version.board_display = otp->board_display;

    // 3rd and 4th blocks, programmed on FATP stage
    if(otp->board_color != 0xFF) {
        furi_hal_version.board_color = otp->board_color;
        furi_hal_version.board_region = otp->board_region;
        furi_hal_version_set_name(otp->name);
    } else {
        furi_hal_version.board_color = 0;
        furi_hal_version.board_region = 0;
        furi_hal_version_set_name(NULL);
    }
}

void furi_hal_version_init(void) {
    switch(furi_hal_version_get_otp_version()) {
    case FuriHalVersionOtpVersionUnknown:
    case FuriHalVersionOtpVersionEmpty:
        furi_hal_version_load_otp_default();
        break;
    case FuriHalVersionOtpVersion0:
        furi_hal_version_load_otp_v0();
        break;
    case FuriHalVersionOtpVersion1:
        furi_hal_version_load_otp_v1();
        break;
    case FuriHalVersionOtpVersion2:
        furi_hal_version_load_otp_v2();
        break;
    default:
        furi_crash();
    }

    furi_hal_rtc_set_register(FuriHalRtcRegisterVersion, (uint32_t)version_get());

    FURI_LOG_I(TAG, "Init OK");
}

FuriHalVersionOtpVersion furi_hal_version_get_otp_version(void) {
    if(*(uint64_t*)FURI_HAL_VERSION_OTP_ADDRESS == 0xFFFFFFFF) {
        return FuriHalVersionOtpVersionEmpty;
    } else {
        if(((FuriHalVersionOTPv1*)FURI_HAL_VERSION_OTP_ADDRESS)->header_magic ==
           FURI_HAL_VERSION_OTP_HEADER_MAGIC) {
            // Version 1+
            uint8_t version = ((FuriHalVersionOTPv1*)FURI_HAL_VERSION_OTP_ADDRESS)->header_version;
            if(version >= FuriHalVersionOtpVersion1 && version <= FuriHalVersionOtpVersion2) {
                return version;
            } else {
                return FuriHalVersionOtpVersionUnknown;
            }
        } else if(((FuriHalVersionOTPv0*)FURI_HAL_VERSION_OTP_ADDRESS)->board_version <= 10) {
            // Version 0
            return FuriHalVersionOtpVersion0;
        } else {
            // Version Unknown
            return FuriHalVersionOtpVersionUnknown;
        }
    }
}

uint8_t furi_hal_version_get_hw_version(void) {
    return furi_hal_version.board_version;
}

uint8_t furi_hal_version_get_hw_target(void) {
    return furi_hal_version.board_target;
}

uint8_t furi_hal_version_get_hw_body(void) {
    return furi_hal_version.board_body;
}

FuriHalVersionColor furi_hal_version_get_hw_color(void) {
    return furi_hal_version.board_color;
}

uint8_t furi_hal_version_get_hw_connect(void) {
    return furi_hal_version.board_connect;
}

FuriHalVersionRegion furi_hal_version_get_hw_region(void) {
    return furi_hal_version.board_region;
}

const char* furi_hal_version_get_hw_region_name(void) {
    switch(furi_hal_version_get_hw_region()) {
    case FuriHalVersionRegionUnknown:
        return "R00";
    case FuriHalVersionRegionEuRu:
        return "R01";
    case FuriHalVersionRegionUsCaAu:
        return "R02";
    case FuriHalVersionRegionJp:
        return "R03";
    case FuriHalVersionRegionWorld:
        return "R04";
    }
    return "R??";
}

FuriHalVersionDisplay furi_hal_version_get_hw_display(void) {
    return furi_hal_version.board_display;
}

uint32_t furi_hal_version_get_hw_timestamp(void) {
    return furi_hal_version.timestamp;
}

const char* furi_hal_version_get_name_ptr(void) {
    return *furi_hal_version.name == 0x00 ? NULL : furi_hal_version.name;
}

const char* furi_hal_version_get_device_name_ptr(void) {
    return furi_hal_version.device_name + 1;
}

const char* furi_hal_version_get_ble_local_device_name_ptr(void) {
    return furi_hal_version.device_name;
}

const uint8_t* furi_hal_version_get_ble_mac(void) {
    return furi_hal_version.ble_mac;
}

const struct Version* furi_hal_version_get_firmware_version(void) {
    return version_get();
}

size_t furi_hal_version_uid_size(void) {
    return 64 / 8;
}

const uint8_t* furi_hal_version_uid(void) {
    return (const uint8_t*)UID64_BASE;
}
