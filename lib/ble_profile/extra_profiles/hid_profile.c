#include "hid_profile.h"

#include <furi_hal_usb_hid.h>
#include <services/dev_info_service.h>
#include <services/battery_service.h>
#include <extra_services/hid_service.h>

#include <furi.h>
#include <usb_hid.h>
#include <ble/ble.h>

#define HID_INFO_BASE_USB_SPECIFICATION (0x0101)
#define HID_INFO_COUNTRY_CODE (0x00)
#define BLE_PROFILE_HID_INFO_FLAG_REMOTE_WAKE_MSK (0x01)
#define BLE_PROFILE_HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK (0x02)

#define BLE_PROFILE_HID_KB_MAX_KEYS (6)
#define BLE_PROFILE_CONSUMER_MAX_KEYS (1)

// Report ids cant be 0
enum HidReportId {
    ReportIdKeyboard = 1,
    ReportIdMouse = 2,
    ReportIdConsumer = 3,
};
// Report numbers corresponded to the report id with an offset of 1
enum HidInputNumber {
    ReportNumberKeyboard = 0,
    ReportNumberMouse = 1,
    ReportNumberConsumer = 2,
};

typedef struct {
    uint8_t mods;
    uint8_t reserved;
    uint8_t key[BLE_PROFILE_HID_KB_MAX_KEYS];
} FURI_PACKED FuriHalBtHidKbReport;

typedef struct {
    uint8_t btn;
    int8_t x;
    int8_t y;
    int8_t wheel;
} FURI_PACKED FuriHalBtHidMouseReport;

typedef struct {
    uint16_t key[BLE_PROFILE_CONSUMER_MAX_KEYS];
} FURI_PACKED FuriHalBtHidConsumerReport;

// keyboard+mouse+consumer hid report
static const uint8_t ble_profile_hid_report_map_data[] = {
    // Keyboard Report
    HID_USAGE_PAGE(HID_PAGE_DESKTOP),
    HID_USAGE(HID_DESKTOP_KEYBOARD),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
    HID_REPORT_ID(ReportIdKeyboard),
    HID_USAGE_PAGE(HID_DESKTOP_KEYPAD),
    HID_USAGE_MINIMUM(HID_KEYBOARD_L_CTRL),
    HID_USAGE_MAXIMUM(HID_KEYBOARD_R_GUI),
    HID_LOGICAL_MINIMUM(0),
    HID_LOGICAL_MAXIMUM(1),
    HID_REPORT_SIZE(1),
    HID_REPORT_COUNT(8),
    HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_REPORT_COUNT(1),
    HID_REPORT_SIZE(8),
    HID_INPUT(HID_IOF_CONSTANT | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_USAGE_PAGE(HID_PAGE_LED),
    HID_REPORT_COUNT(8),
    HID_REPORT_SIZE(1),
    HID_USAGE_MINIMUM(1),
    HID_USAGE_MAXIMUM(8),
    HID_OUTPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_REPORT_COUNT(BLE_PROFILE_HID_KB_MAX_KEYS),
    HID_REPORT_SIZE(8),
    HID_LOGICAL_MINIMUM(0),
    HID_LOGICAL_MAXIMUM(101),
    HID_USAGE_PAGE(HID_DESKTOP_KEYPAD),
    HID_USAGE_MINIMUM(0),
    HID_USAGE_MAXIMUM(101),
    HID_INPUT(HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
    HID_END_COLLECTION,
    // Mouse Report
    HID_USAGE_PAGE(HID_PAGE_DESKTOP),
    HID_USAGE(HID_DESKTOP_MOUSE),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
    HID_USAGE(HID_DESKTOP_POINTER),
    HID_COLLECTION(HID_PHYSICAL_COLLECTION),
    HID_REPORT_ID(ReportIdMouse),
    HID_USAGE_PAGE(HID_PAGE_BUTTON),
    HID_USAGE_MINIMUM(1),
    HID_USAGE_MAXIMUM(3),
    HID_LOGICAL_MINIMUM(0),
    HID_LOGICAL_MAXIMUM(1),
    HID_REPORT_COUNT(3),
    HID_REPORT_SIZE(1),
    HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_REPORT_SIZE(1),
    HID_REPORT_COUNT(5),
    HID_INPUT(HID_IOF_CONSTANT | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_USAGE_PAGE(HID_PAGE_DESKTOP),
    HID_USAGE(HID_DESKTOP_X),
    HID_USAGE(HID_DESKTOP_Y),
    HID_USAGE(HID_DESKTOP_WHEEL),
    HID_LOGICAL_MINIMUM(-127),
    HID_LOGICAL_MAXIMUM(127),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(3),
    HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),
    HID_END_COLLECTION,
    HID_END_COLLECTION,
    // Consumer Report
    HID_USAGE_PAGE(HID_PAGE_CONSUMER),
    HID_USAGE(HID_CONSUMER_CONTROL),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
    HID_REPORT_ID(ReportIdConsumer),
    HID_LOGICAL_MINIMUM(0),
    HID_RI_LOGICAL_MAXIMUM(16, 0x3FF),
    HID_USAGE_MINIMUM(0),
    HID_RI_USAGE_MAXIMUM(16, 0x3FF),
    HID_REPORT_COUNT(BLE_PROFILE_CONSUMER_MAX_KEYS),
    HID_REPORT_SIZE(16),
    HID_INPUT(HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
    HID_END_COLLECTION,
};

typedef struct {
    FuriHalBleProfileBase base;

    FuriHalBtHidKbReport* kb_report;
    FuriHalBtHidMouseReport* mouse_report;
    FuriHalBtHidConsumerReport* consumer_report;

    BleServiceBattery* battery_svc;
    BleServiceDevInfo* dev_info_svc;
    BleServiceHid* hid_svc;
} BleProfileHid;
_Static_assert(offsetof(BleProfileHid, base) == 0, "Wrong layout");

static FuriHalBleProfileBase* ble_profile_hid_start(FuriHalBleProfileParams profile_params) {
    UNUSED(profile_params);

    BleProfileHid* profile = malloc(sizeof(BleProfileHid));

    profile->base.config = ble_profile_hid;

    profile->battery_svc = ble_svc_battery_start(true);
    profile->dev_info_svc = ble_svc_dev_info_start();
    profile->hid_svc = ble_svc_hid_start();

    // Configure HID Keyboard
    profile->kb_report = malloc(sizeof(FuriHalBtHidKbReport));
    profile->mouse_report = malloc(sizeof(FuriHalBtHidMouseReport));
    profile->consumer_report = malloc(sizeof(FuriHalBtHidConsumerReport));

    // Configure Report Map characteristic
    ble_svc_hid_update_report_map(
        profile->hid_svc,
        ble_profile_hid_report_map_data,
        sizeof(ble_profile_hid_report_map_data));
    // Configure HID Information characteristic
    uint8_t hid_info_val[4] = {
        HID_INFO_BASE_USB_SPECIFICATION & 0x00ff,
        (HID_INFO_BASE_USB_SPECIFICATION & 0xff00) >> 8,
        HID_INFO_COUNTRY_CODE,
        BLE_PROFILE_HID_INFO_FLAG_REMOTE_WAKE_MSK |
            BLE_PROFILE_HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK,
    };
    ble_svc_hid_update_info(profile->hid_svc, hid_info_val);

    return &profile->base;
}

static void ble_profile_hid_stop(FuriHalBleProfileBase* profile) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    ble_svc_battery_stop(hid_profile->battery_svc);
    ble_svc_dev_info_stop(hid_profile->dev_info_svc);
    ble_svc_hid_stop(hid_profile->hid_svc);

    free(hid_profile->kb_report);
    free(hid_profile->mouse_report);
    free(hid_profile->consumer_report);
}

bool ble_profile_hid_kb_press(FuriHalBleProfileBase* profile, uint16_t button) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidKbReport* kb_report = hid_profile->kb_report;
    for(uint8_t i = 0; i < BLE_PROFILE_HID_KB_MAX_KEYS; i++) {
        if(kb_report->key[i] == 0) {
            kb_report->key[i] = button & 0xFF;
            break;
        }
    }
    kb_report->mods |= (button >> 8);
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberKeyboard,
        (uint8_t*)kb_report,
        sizeof(FuriHalBtHidKbReport));
}

bool ble_profile_hid_kb_release(FuriHalBleProfileBase* profile, uint16_t button) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;

    FuriHalBtHidKbReport* kb_report = hid_profile->kb_report;
    for(uint8_t i = 0; i < BLE_PROFILE_HID_KB_MAX_KEYS; i++) {
        if(kb_report->key[i] == (button & 0xFF)) {
            kb_report->key[i] = 0;
            break;
        }
    }
    kb_report->mods &= ~(button >> 8);
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberKeyboard,
        (uint8_t*)kb_report,
        sizeof(FuriHalBtHidKbReport));
}

bool ble_profile_hid_kb_release_all(FuriHalBleProfileBase* profile) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidKbReport* kb_report = hid_profile->kb_report;
    for(uint8_t i = 0; i < BLE_PROFILE_HID_KB_MAX_KEYS; i++) {
        kb_report->key[i] = 0;
    }
    kb_report->mods = 0;
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberKeyboard,
        (uint8_t*)kb_report,
        sizeof(FuriHalBtHidKbReport));
}

bool ble_profile_hid_consumer_key_press(FuriHalBleProfileBase* profile, uint16_t button) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidConsumerReport* consumer_report = hid_profile->consumer_report;
    for(uint8_t i = 0; i < BLE_PROFILE_CONSUMER_MAX_KEYS; i++) { //-V1008
        if(consumer_report->key[i] == 0) {
            consumer_report->key[i] = button;
            break;
        }
    }
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberConsumer,
        (uint8_t*)consumer_report,
        sizeof(FuriHalBtHidConsumerReport));
}

bool ble_profile_hid_consumer_key_release(FuriHalBleProfileBase* profile, uint16_t button) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidConsumerReport* consumer_report = hid_profile->consumer_report;
    for(uint8_t i = 0; i < BLE_PROFILE_CONSUMER_MAX_KEYS; i++) { //-V1008
        if(consumer_report->key[i] == button) {
            consumer_report->key[i] = 0;
            break;
        }
    }
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberConsumer,
        (uint8_t*)consumer_report,
        sizeof(FuriHalBtHidConsumerReport));
}

bool ble_profile_hid_consumer_key_release_all(FuriHalBleProfileBase* profile) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidConsumerReport* consumer_report = hid_profile->consumer_report;
    for(uint8_t i = 0; i < BLE_PROFILE_CONSUMER_MAX_KEYS; i++) { //-V1008
        consumer_report->key[i] = 0;
    }
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberConsumer,
        (uint8_t*)consumer_report,
        sizeof(FuriHalBtHidConsumerReport));
}

bool ble_profile_hid_mouse_move(FuriHalBleProfileBase* profile, int8_t dx, int8_t dy) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidMouseReport* mouse_report = hid_profile->mouse_report;
    mouse_report->x = dx;
    mouse_report->y = dy;
    bool state = ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberMouse,
        (uint8_t*)mouse_report,
        sizeof(FuriHalBtHidMouseReport));
    mouse_report->x = 0;
    mouse_report->y = 0;
    return state;
}

bool ble_profile_hid_mouse_press(FuriHalBleProfileBase* profile, uint8_t button) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidMouseReport* mouse_report = hid_profile->mouse_report;
    mouse_report->btn |= button;
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberMouse,
        (uint8_t*)mouse_report,
        sizeof(FuriHalBtHidMouseReport));
}

bool ble_profile_hid_mouse_release(FuriHalBleProfileBase* profile, uint8_t button) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidMouseReport* mouse_report = hid_profile->mouse_report;
    mouse_report->btn &= ~button;
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberMouse,
        (uint8_t*)mouse_report,
        sizeof(FuriHalBtHidMouseReport));
}

bool ble_profile_hid_mouse_release_all(FuriHalBleProfileBase* profile) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidMouseReport* mouse_report = hid_profile->mouse_report;
    mouse_report->btn = 0;
    return ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberMouse,
        (uint8_t*)mouse_report,
        sizeof(FuriHalBtHidMouseReport));
}

bool ble_profile_hid_mouse_scroll(FuriHalBleProfileBase* profile, int8_t delta) {
    furi_check(profile);
    furi_check(profile->config == ble_profile_hid);

    BleProfileHid* hid_profile = (BleProfileHid*)profile;
    FuriHalBtHidMouseReport* mouse_report = hid_profile->mouse_report;
    mouse_report->wheel = delta;
    bool state = ble_svc_hid_update_input_report(
        hid_profile->hid_svc,
        ReportNumberMouse,
        (uint8_t*)mouse_report,
        sizeof(FuriHalBtHidMouseReport));
    mouse_report->wheel = 0;
    return state;
}

// AN5289: 4.7, in order to use flash controller interval must be at least 25ms + advertisement, which is 30 ms
// Since we don't use flash controller anymore interval can be lowered to 7.5ms
#define CONNECTION_INTERVAL_MIN (0x0006)
// Up to 45 ms
#define CONNECTION_INTERVAL_MAX (0x24)

static GapConfig template_config = {
    .adv_service_uuid = HUMAN_INTERFACE_DEVICE_SERVICE_UUID,
    .appearance_char = GAP_APPEARANCE_KEYBOARD,
    .bonding_mode = true,
    .pairing_method = GapPairingPinCodeVerifyYesNo,
    .conn_param =
        {
            .conn_int_min = CONNECTION_INTERVAL_MIN,
            .conn_int_max = CONNECTION_INTERVAL_MAX,
            .slave_latency = 0,
            .supervisor_timeout = 0,
        },
};

static void ble_profile_hid_get_config(GapConfig* config, FuriHalBleProfileParams profile_params) {
    BleProfileHidParams* hid_profile_params = profile_params;

    furi_check(config);
    memcpy(config, &template_config, sizeof(GapConfig));
    // Set mac address
    memcpy(config->mac_address, furi_hal_version_get_ble_mac(), sizeof(config->mac_address));

    // Change MAC address for HID profile
    config->mac_address[2]++;
    if(hid_profile_params) {
        config->mac_address[0] ^= hid_profile_params->mac_xor;
        config->mac_address[1] ^= hid_profile_params->mac_xor >> 8;
    }

    // Set advertise name
    memset(config->adv_name, 0, sizeof(config->adv_name));
    FuriString* name = furi_string_alloc_set(furi_hal_version_get_ble_local_device_name_ptr());

    const char* clicker_str = "Control";
    if(hid_profile_params && hid_profile_params->device_name_prefix) {
        clicker_str = hid_profile_params->device_name_prefix;
    }
    furi_string_replace_str(name, "Flipper", clicker_str);
    if(furi_string_size(name) >= sizeof(config->adv_name)) {
        furi_string_left(name, sizeof(config->adv_name) - 1);
    }
    memcpy(config->adv_name, furi_string_get_cstr(name), furi_string_size(name));
    furi_string_free(name);
}

static const FuriHalBleProfileTemplate profile_callbacks = {
    .start = ble_profile_hid_start,
    .stop = ble_profile_hid_stop,
    .get_gap_config = ble_profile_hid_get_config,
};

const FuriHalBleProfileTemplate* ble_profile_hid = &profile_callbacks;
