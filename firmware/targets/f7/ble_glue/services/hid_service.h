#pragma once

#include <stdint.h>
#include <stdbool.h>

#define HID_SVC_REPORT_MAP_MAX_LEN (255)
#define HID_SVC_REPORT_MAX_LEN (255)
#define HID_SVC_REPORT_REF_LEN (2)
#define HID_SVC_INFO_LEN (4)
#define HID_SVC_CONTROL_POINT_LEN (1)

#define HID_SVC_INPUT_REPORT_COUNT (3)
#define HID_SVC_OUTPUT_REPORT_COUNT (0)
#define HID_SVC_FEATURE_REPORT_COUNT (0)
#define HID_SVC_REPORT_COUNT \
    (HID_SVC_INPUT_REPORT_COUNT + HID_SVC_OUTPUT_REPORT_COUNT + HID_SVC_FEATURE_REPORT_COUNT)

typedef uint16_t (*HidLedStateEventCallback)(uint8_t state, void* ctx);

void hid_svc_start();

void hid_svc_stop();

bool hid_svc_is_started();

bool hid_svc_update_report_map(const uint8_t* data, uint16_t len);

bool hid_svc_update_input_report(uint8_t input_report_num, uint8_t* data, uint16_t len);

// Expects data to be of length HID_SVC_INFO_LEN (4 bytes)
bool hid_svc_update_info(uint8_t* data);

void hid_svc_register_led_state_callback(HidLedStateEventCallback callback, void* context);
