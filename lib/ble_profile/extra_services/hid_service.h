#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BleServiceHid BleServiceHid;

BleServiceHid* ble_svc_hid_start(void);

void ble_svc_hid_stop(BleServiceHid* service);

bool ble_svc_hid_update_report_map(BleServiceHid* service, const uint8_t* data, uint16_t len);

bool ble_svc_hid_update_input_report(
    BleServiceHid* service,
    uint8_t input_report_num,
    uint8_t* data,
    uint16_t len);

// Expects data to be of length BLE_SVC_HID_INFO_LEN (4 bytes)
bool ble_svc_hid_update_info(BleServiceHid* service, uint8_t* data);

#ifdef __cplusplus
}
#endif