#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <shci/shci.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*BleGlueKeyStorageChangedCallback)(uint8_t* change_addr_start, uint16_t size, void* context);


/** Initialize start core2 and initialize transport */
void ble_glue_init();

/** Start Core2 Radio stack
 *
 * @return     true on success
 */
bool ble_glue_start();

/** Is core2 alive and at least FUS is running
 * 
 * @return     true if core2 is alive
 */
bool ble_glue_is_alive();

bool ble_glue_wait_for_fus_start(WirelessFwInfo_t* info);

/** Is core2 radio stack present and ready
 *
 * @return     true if present and ready
 */
bool ble_glue_is_radio_stack_ready();

/** Set callback for NVM in RAM changes
 *
 * @param[in]  callback  The callback to call on NVM change
 * @param      context   The context for callback
 */
void ble_glue_set_key_storage_changed_callback(BleGlueKeyStorageChangedCallback callback, void* context);

void ble_glue_thread_stop();

#ifdef __cplusplus
}
#endif
