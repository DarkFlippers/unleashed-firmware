#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * BLE stack init and cleanup
 */

#ifdef __cplusplus
extern "C" {
#endif

bool ble_app_init(void);

void ble_app_get_key_storage_buff(uint8_t** addr, uint16_t* size);

void ble_app_deinit(void);

#ifdef __cplusplus
}
#endif
