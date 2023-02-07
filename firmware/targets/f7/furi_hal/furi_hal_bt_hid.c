#include <furi_hal_bt_hid.h>
#include <furi_hal_usb_hid.h>
#include "usb_hid.h"
#include "dev_info_service.h"
#include "battery_service.h"
#include "hid_service.h"

#include <furi.h>

#define FURI_HAL_BT_INFO_BASE_USB_SPECIFICATION (0x0101)
#define FURI_HAL_BT_INFO_COUNTRY_CODE (0x00)
#define FURI_HAL_BT_HID_INFO_FLAG_REMOTE_WAKE_MSK (0x01)
#define FURI_HAL_BT_HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK (0x02)

#define FURI_HAL_BT_HID_KB_MAX_KEYS 6
#define FURI_HAL_BT_HID_CONSUMER_MAX_KEYS 1

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
    uint8_t key[FURI_HAL_BT_HID_KB_MAX_KEYS];
} __attribute__((__packed__)) FuriHalBtHidKbReport;

typedef struct {
    uint8_t btn;
    int8_t x;
    int8_t y;
    int8_t wheel;
} __attribute__((__packed__)) FuriHalBtHidMouseReport;

typedef struct {
    uint16_t key[FURI_HAL_BT_HID_CONSUMER_MAX_KEYS];
} __attribute__((__packed__)) FuriHalBtHidConsumerReport;

// keyboard+mouse+consumer hid report
static const uint8_t furi_hal_bt_hid_report_map_data[] = {
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
    HID_REPORT_COUNT(FURI_HAL_BT_HID_KB_MAX_KEYS),
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
    HID_REPORT_COUNT(FURI_HAL_BT_HID_CONSUMER_MAX_KEYS),
    HID_REPORT_SIZE(16),
    HID_INPUT(HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
    HID_END_COLLECTION,
};
FuriHalBtHidKbReport* kb_report = NULL;
FuriHalBtHidMouseReport* mouse_report = NULL;
FuriHalBtHidConsumerReport* consumer_report = NULL;

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
    mouse_report = malloc(sizeof(FuriHalBtHidMouseReport));
    consumer_report = malloc(sizeof(FuriHalBtHidConsumerReport));
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
    furi_assert(mouse_report);
    furi_assert(consumer_report);
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
    free(mouse_report);
    free(consumer_report);
    kb_report = NULL;
    mouse_report = NULL;
    consumer_report = NULL;
}

bool furi_hal_bt_hid_kb_press(uint16_t button) {
    furi_assert(kb_report);
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_KB_MAX_KEYS; i++) {
        if(kb_report->key[i] == 0) {
            kb_report->key[i] = button & 0xFF;
            break;
        }
    }
    kb_report->mods |= (button >> 8);
    return hid_svc_update_input_report(
        ReportNumberKeyboard, (uint8_t*)kb_report, sizeof(FuriHalBtHidKbReport));
}

bool furi_hal_bt_hid_kb_release(uint16_t button) {
    furi_assert(kb_report);
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_KB_MAX_KEYS; i++) {
        if(kb_report->key[i] == (button & 0xFF)) {
            kb_report->key[i] = 0;
            break;
        }
    }
    kb_report->mods &= ~(button >> 8);
    return hid_svc_update_input_report(
        ReportNumberKeyboard, (uint8_t*)kb_report, sizeof(FuriHalBtHidKbReport));
}

bool furi_hal_bt_hid_kb_release_all() {
    furi_assert(kb_report);
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_KB_MAX_KEYS; i++) {
        kb_report->key[i] = 0;
    }
    kb_report->mods = 0;
    return hid_svc_update_input_report(
        ReportNumberKeyboard, (uint8_t*)kb_report, sizeof(FuriHalBtHidKbReport));
}

bool furi_hal_bt_hid_consumer_key_press(uint16_t button) {
    furi_assert(consumer_report);
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_CONSUMER_MAX_KEYS; i++) { //-V1008
        if(consumer_report->key[i] == 0) {
            consumer_report->key[i] = button;
            break;
        }
    }
    return hid_svc_update_input_report(
        ReportNumberConsumer, (uint8_t*)consumer_report, sizeof(FuriHalBtHidConsumerReport));
}

bool furi_hal_bt_hid_consumer_key_release(uint16_t button) {
    furi_assert(consumer_report);
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_CONSUMER_MAX_KEYS; i++) { //-V1008
        if(consumer_report->key[i] == button) {
            consumer_report->key[i] = 0;
            break;
        }
    }
    return hid_svc_update_input_report(
        ReportNumberConsumer, (uint8_t*)consumer_report, sizeof(FuriHalBtHidConsumerReport));
}

bool furi_hal_bt_hid_consumer_key_release_all() {
    furi_assert(consumer_report);
    for(uint8_t i = 0; i < FURI_HAL_BT_HID_CONSUMER_MAX_KEYS; i++) { //-V1008
        consumer_report->key[i] = 0;
    }
    return hid_svc_update_input_report(
        ReportNumberConsumer, (uint8_t*)consumer_report, sizeof(FuriHalBtHidConsumerReport));
}

bool furi_hal_bt_hid_mouse_move(int8_t dx, int8_t dy) {
    furi_assert(mouse_report);
    mouse_report->x = dx;
    mouse_report->y = dy;
    bool state = hid_svc_update_input_report(
        ReportNumberMouse, (uint8_t*)mouse_report, sizeof(FuriHalBtHidMouseReport));
    mouse_report->x = 0;
    mouse_report->y = 0;
    return state;
}

bool furi_hal_bt_hid_mouse_press(uint8_t button) {
    furi_assert(mouse_report);
    mouse_report->btn |= button;
    return hid_svc_update_input_report(
        ReportNumberMouse, (uint8_t*)mouse_report, sizeof(FuriHalBtHidMouseReport));
}

bool furi_hal_bt_hid_mouse_release(uint8_t button) {
    furi_assert(mouse_report);
    mouse_report->btn &= ~button;
    return hid_svc_update_input_report(
        ReportNumberMouse, (uint8_t*)mouse_report, sizeof(FuriHalBtHidMouseReport));
}

bool furi_hal_bt_hid_mouse_release_all() {
    furi_assert(mouse_report);
    mouse_report->btn = 0;
    return hid_svc_update_input_report(
        ReportNumberMouse, (uint8_t*)mouse_report, sizeof(FuriHalBtHidMouseReport));
}

bool furi_hal_bt_hid_mouse_scroll(int8_t delta) {
    furi_assert(mouse_report);
    mouse_report->wheel = delta;
    bool state = hid_svc_update_input_report(
        ReportNumberMouse, (uint8_t*)mouse_report, sizeof(FuriHalBtHidMouseReport));
    mouse_report->wheel = 0;
    return state;
}
