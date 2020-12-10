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

#ifdef __cplusplus
}
#endif
