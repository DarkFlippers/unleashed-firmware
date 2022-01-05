#pragma once

#include <stdint.h>
#include <stdbool.h>

#define HID_SVC_REPORT_MAP_MAX_LEN (120)
#define HID_SVC_REPORT_MAX_LEN (9)
#define HID_SVC_BOOT_KEYBOARD_INPUT_REPORT_MAX_LEN (8)
#define HID_SVC_REPORT_REF_LEN (2)
#define HID_SVC_INFO_LEN (4)
#define HID_SVC_CONTROL_POINT_LEN (1)

void hid_svc_start();

void hid_svc_stop();

bool hid_svc_is_started();

bool hid_svc_update_report_map(uint8_t* data, uint16_t len);

bool hid_svc_update_input_report(uint8_t* data, uint16_t len);

bool hid_svc_update_info(uint8_t* data, uint16_t len);
