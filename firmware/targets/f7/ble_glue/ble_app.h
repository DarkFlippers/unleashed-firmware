#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool ble_app_init();
void ble_app_get_key_storage_buff(uint8_t** addr, uint16_t* size);
void ble_app_thread_stop();

#ifdef __cplusplus
}
#endif
