#pragma once

#include <m-string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize */
void api_hal_bt_init();

/* Get BT/BLE system component state */
void api_hal_bt_dump_state(string_t buffer);

/* Get BT/BLE system component state */
bool api_hal_bt_is_alive();

/* Lock shared access to flash controller */
void api_hal_bt_lock_flash();

/* Unlock shared access to flash controller */
void api_hal_bt_unlock_flash();

#ifdef __cplusplus
}
#endif
