#include "furi_hal_bt_hid.h"
#include "dev_info_service.h"
#include "battery_service.h"
#include "hid_service.h"

#include <furi.h>

#define FURI_HAL_BT_INFO_BASE_USB_SPECIFICATION (0x0101)
#define FURI_HAL_BT_INFO_COUNTRY_CODE (0x00)
#define FURI_HAL_BT_HID_INFO_FLAG_REMOTE_WAKE_MSK (0x01)
#define FURI_HAL_BT_HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK (0x02)

#define FURI_HAL_BT_HID_KB_KEYS_MAX (6)

typedef struct {
    // uint8_t report_id;
    uint8_t mods;
    uint8_t reserved;
    uint8_t key[FURI_HAL_BT_HID_KB_KEYS_MAX];
} FuriHalBtHidKbReport;

typedef struct {
    uint8_t report_id;
    uint8_t key;
} FuriHalBtHidMediaReport;

// TODO make composite HID device
static uint8_t furi_hal_bt_hid_report_map_data[] = {
    0x05,
    0x01, // Usage Page (Generic Desktop)
    0x09,
    0x06, // Usage (Keyboard)
    0xA1,
    0x01, // Collection (Application)
    // 0x85, 0x01,       // Report ID (1)
    0x05,
    0x07, // Usage Page (Key Codes)
    0x19,
    0xe0, // Usage Minimum (224)
    0x29,
    0xe7, // Usage Maximum (231)
    0x15,
    0x00, // Logical Minimum (0)
    0x25,
    0x01, // Logical Maximum (1)
    0x75,
    0x01, // Report Size (1)
    0x95,
    0x08, // Report Count (8)
    0x81,
    0x02, // Input (Data, Variable, Absolute)

    0x95,
    0x01, // Report Count (1)
    0x75,
    0x08, // Report Size (8)
    0x81,
    0x01, // Input (Constant) reserved byte(1)

    0x95,
    0x05, // Report Count (5)
    0x75,
    0x01, // Report Size (1)
    0x05,
    0x08, // Usage Page (Page# for LEDs)
    0x19,
    0x01, // Usage Minimum (1)
    0x29,
    0x05, // Usage Maximum (5)
    0x91,
    0x02, // Output (Data, Variable, Absolute), Led report
    0x95,
    0x01, // Report Count (1)
    0x75,
    0x03, // Report Size (3)
    0x91,
    0x01, // Output (Data, Variable, Absolute), Led report padding

    0x95,
    0x06, // Report Count (6)
    0x75,
    0x08, // Report Size (8)
    0x15,
    0x00, // Logical Minimum (0)
    0x25,
    0x65, // Logical Maximum (101)
    0x05,
    0x07, // Usage Page (Key codes)
    0x19,
    0x00, // Usage Minimum (0)
    0x29,
    0x65, // Usage Maximum (101)
    0x81,
    0x00, // Input (Data, Array) Key array(6 bytes)

    0x09,
    0x05, // Usage (Vendor Defined)
    0x15,
    0x00, // Logical Minimum (0)
    0x26,
    0xFF,
    0x00, // Logical Maximum (255)
    0x75,
    0x08, // Report Size (8 bit)
    0x95,
    0x02, // Report Count (2)
    0xB1,
    0x02, // Feature (Data, Variable, Absolute)

    0xC0, // End Collection (Application)

    // 0x05, 0x0C,        // Usage Page (Consumer)
    // 0x09, 0x01,        // Usage (Consumer Control)
    // 0xA1, 0x01,        // Collection (Application)
    // 0x85, 0x02,        //   Report ID (2)
    // 0x05, 0x0C,        //   Usage Page (Consumer)
    // 0x15, 0x00,        //   Logical Minimum (0)
    // 0x25, 0x01,        //   Logical Maximum (1)
    // 0x75, 0x01,        //   Report Size (1)
    // 0x95, 0x07,        //   Report Count (7)
    // 0x09, 0xB5,        //   Usage (Scan Next Track)
    // 0x09, 0xB6,        //   Usage (Scan Previous Track)
    // 0x09, 0xB7,        //   Usage (Stop)
    // 0x09, 0xB8,        //   Usage (Eject)
    // 0x09, 0xCD,        //   Usage (Play/Pause)
    // 0x09, 0xE2,        //   Usage (Mute)
    // 0x09, 0xE9,        //   Usage (Volume Increment)
    // 0x09, 0xEA,        //   Usage (Volume Decrement)
    // 0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    // 0xC0,              // End Collection
};

FuriHalBtHidKbReport* kb_report = NULL;
FuriHalBtHidMediaReport* media_report = NULL;

void furi_hal_bt_hid_start() {
    // Start device info
    if(!dev_info_svc_is_started()) {
        dev_info_svc_start();
    }
    // Start battery service
    if(!battery_svc_is_started()) {
        battery_svc_start();
    }
    // Start HID service
    if(!hid_svc_is_started()) {
        hid_svc_start();
    }
    // Configure HID Keyboard
    kb_report = malloc(sizeof(FuriHalBtHidKbReport));
    media_report = malloc(sizeof(FuriHalBtHidMediaReport));
    // Configure Report Map characteristic
    hid_svc_update_report_map(
        furi_hal_bt_hid_report_map_data, sizeof(furi_hal_bt_hid_report_map_data));
    // Configure HID Information characteristic
    uint8_t hid_info_val[4] = {
        FURI_HAL_BT_INFO_BASE_USB_SPECIFICATION & 0x00ff,
        (FURI_HAL_BT_INFO_BASE_USB_SPECIFICATION & 0xff00) >> 8,
        FURI_HAL_BT_INFO_COUNTRY_CODE,
        FURI_HAL_BT_HID_INFO_FLAG_REMOTE_WAKE_MSK |
            FURI_HAL_BT_HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK,
    };
    hid_svc_update_info(hid_info_val, sizeof(hid_info_val));
}

void furi_hal_bt_hid_stop() {
    furi_assert(kb_report);
    // Stop all services
    if(dev_info_svc_is_started()) {
        dev_info_svc_stop();
    }
    if(battery_svc_is_started()) {
        battery_svc_stop();
    }
    if(hid_svc_is_started()) {
        hid_svc_stop();
    }
    free(kb_report);
    free(media_report);
    media_report = NULL;
    kb_report = NULL;
}

bool furi_hal_bt_hid_kb_press(uint16_t button) {
    furi_assert(kb_report);
    // kb_report->report_id = 0x01;
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_KB_KEYS_MAX; i++) {
        if(kb_report->key[i] == 0) {
            kb_report->key[i] = button & 0xFF;
            break;
        }
    }
    kb_report->mods |= (button >> 8);
    return hid_svc_update_input_report((uint8_t*)kb_report, sizeof(FuriHalBtHidKbReport));
}

bool furi_hal_bt_hid_kb_release(uint16_t button) {
    furi_assert(kb_report);
    // kb_report->report_id = 0x01;
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_KB_KEYS_MAX; i++) {
        if(kb_report->key[i] == (button & 0xFF)) {
            kb_report->key[i] = 0;
            break;
        }
    }
    kb_report->mods &= ~(button >> 8);
    return hid_svc_update_input_report((uint8_t*)kb_report, sizeof(FuriHalBtHidKbReport));
}

bool furi_hal_bt_hid_kb_release_all() {
    furi_assert(kb_report);
    // kb_report->report_id = 0x01;
    memset(kb_report, 0, sizeof(FuriHalBtHidKbReport));
    return hid_svc_update_input_report((uint8_t*)kb_report, sizeof(FuriHalBtHidKbReport));
}

bool furi_hal_bt_hid_media_press(uint8_t button) {
    furi_assert(media_report);
    media_report->report_id = 0x02;
    media_report->key |= (0x01 << button);
    return hid_svc_update_input_report((uint8_t*)media_report, sizeof(FuriHalBtHidMediaReport));
}

bool furi_hal_bt_hid_media_release(uint8_t button) {
    furi_assert(media_report);
    media_report->report_id = 0x02;
    media_report->key &= ~(0x01 << button);
    return hid_svc_update_input_report((uint8_t*)media_report, sizeof(FuriHalBtHidMediaReport));
}

bool furi_hal_bt_hid_media_release_all() {
    furi_assert(media_report);
    media_report->report_id = 0x02;
    media_report->key = 0x00;
    return hid_svc_update_input_report((uint8_t*)media_report, sizeof(FuriHalBtHidMediaReport));
}
