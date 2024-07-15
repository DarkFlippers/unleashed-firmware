/**
 * @file furi_hal_bt.h
 * BT/BLE HAL API
 */

#pragma once

#include <furi.h>
#include <stdbool.h>
#include <gap.h>
#include <extra_beacon.h>
#include <furi_ble/profile_interface.h>
#include <ble_glue.h>
#include <ble_app.h>
#include <stdint.h>

#define FURI_HAL_BT_STACK_VERSION_MAJOR (1)
#define FURI_HAL_BT_STACK_VERSION_MINOR (12)
#define FURI_HAL_BT_C2_START_TIMEOUT    (1000)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriHalBtStackUnknown,
    FuriHalBtStackLight,
    FuriHalBtStackFull,
} FuriHalBtStack;

/** Initialize
 */
void furi_hal_bt_init(void);

/** Lock core2 state transition */
void furi_hal_bt_lock_core2(void);

/** Lock core2 state transition */
void furi_hal_bt_unlock_core2(void);

/** Start radio stack
 *
 * @return  true on successfull radio stack start
 */
bool furi_hal_bt_start_radio_stack(void);

/** Get radio stack type
 *
 * @return  FuriHalBtStack instance
 */
FuriHalBtStack furi_hal_bt_get_radio_stack(void);

/** Check if radio stack supports BLE GAT/GAP
 *
 * @return  true if supported
 */
bool furi_hal_bt_is_gatt_gap_supported(void);

/** Check if radio stack supports testing
 *
 * @return  true if supported
 */
bool furi_hal_bt_is_testing_supported(void);

/** Check if particular instance of profile belongs to given type
 *
 * @param profile           FuriHalBtProfile instance. If NULL, uses current profile
 * @param profile_template  basic profile template to check against
 *
 * @return          true on success
*/
bool furi_hal_bt_check_profile_type(
    FuriHalBleProfileBase* profile,
    const FuriHalBleProfileTemplate* profile_template);

/** Start BLE app
 *
 * @param profile_template  FuriHalBleProfileTemplate instance
 * @param params            Parameters to pass to the profile. Can be NULL
 * @param event_cb          GapEventCallback instance
 * @param context           pointer to context
 *
 * @return                  instance of profile, NULL on failure
*/
FURI_WARN_UNUSED FuriHalBleProfileBase* furi_hal_bt_start_app(
    const FuriHalBleProfileTemplate* profile_template,
    FuriHalBleProfileParams params,
    GapEventCallback event_cb,
    void* context);

/** Reinitialize core2
 * 
 * Also can be used to prepare core2 for stop modes
 */
void furi_hal_bt_reinit(void);

/** Change BLE app
 * Restarts 2nd core
 *
 * @param profile_template FuriHalBleProfileTemplate instance
 * @param profile_params   Parameters to pass to the profile. Can be NULL
 * @param event_cb         GapEventCallback instance
 * @param context          pointer to context
 *
 * @return                 instance of profile, NULL on failure
*/
FURI_WARN_UNUSED FuriHalBleProfileBase* furi_hal_bt_change_app(
    const FuriHalBleProfileTemplate* profile_template,
    FuriHalBleProfileParams profile_params,
    GapEventCallback event_cb,
    void* context);

/** Update battery level
 *
 * @param battery_level battery level
 */
void furi_hal_bt_update_battery_level(uint8_t battery_level);

/** Update battery power state */
void furi_hal_bt_update_power_state(bool charging);

/** Checks if BLE state is active
 *
 * @return          true if device is connected or advertising, false otherwise
 */
bool furi_hal_bt_is_active(void);

/** Start advertising
 */
void furi_hal_bt_start_advertising(void);

/** Stop advertising
 */
void furi_hal_bt_stop_advertising(void);

/** Get BT/BLE system component state
 *
 * @param[in]  buffer  FuriString* buffer to write to
 */
void furi_hal_bt_dump_state(FuriString* buffer);

/** Get BT/BLE system component state
 *
 * @return     true if core2 is alive
 */
bool furi_hal_bt_is_alive(void);

/** Get key storage buffer address and size
 *
 * @param      key_buff_addr  pointer to store buffer address
 * @param      key_buff_size  pointer to store buffer size
 */
void furi_hal_bt_get_key_storage_buff(uint8_t** key_buff_addr, uint16_t* key_buff_size);

/** Get SRAM2 hardware semaphore
 * @note Must be called before SRAM2 read/write operations
 */
void furi_hal_bt_nvm_sram_sem_acquire(void);

/** Release SRAM2 hardware semaphore
 * @note Must be called after SRAM2 read/write operations
 */
void furi_hal_bt_nvm_sram_sem_release(void);

/** Clear key storage
 *
 * @return      true on success
*/
bool furi_hal_bt_clear_white_list(void);

/** Set key storage change callback
 *
 * @param       callback    BleGlueKeyStorageChangedCallback instance
 * @param       context     pointer to context
 */
void furi_hal_bt_set_key_storage_change_callback(
    BleGlueKeyStorageChangedCallback callback,
    void* context);

/** Start ble tone tx at given channel and power
 *
 * @param[in]  channel  The channel
 * @param[in]  power    The power
 */
void furi_hal_bt_start_tone_tx(uint8_t channel, uint8_t power);

/** Stop ble tone tx
 */
void furi_hal_bt_stop_tone_tx(void);

/** Start sending ble packets at a given frequency and datarate
 *
 * @param[in]  channel   The channel
 * @param[in]  pattern   The pattern
 * @param[in]  datarate  The datarate
 */
void furi_hal_bt_start_packet_tx(uint8_t channel, uint8_t pattern, uint8_t datarate);

/** Stop sending ble packets
 *
 * @return     sent packet count
 */
uint16_t furi_hal_bt_stop_packet_test(void);

/** Start receiving packets
 *
 * @param[in]  channel   RX channel
 * @param[in]  datarate  Datarate
 */
void furi_hal_bt_start_packet_rx(uint8_t channel, uint8_t datarate);

/** Set up the RF to listen to a given RF channel
 *
 * @param[in]  channel  RX channel
 */
void furi_hal_bt_start_rx(uint8_t channel);

/** Stop RF listenning
 */
void furi_hal_bt_stop_rx(void);

/** Get RSSI
 *
 * @return     RSSI in dBm
 */
float furi_hal_bt_get_rssi(void);

/** Get number of transmitted packets
 *
 * @return     packet count
 */
uint32_t furi_hal_bt_get_transmitted_packets(void);

/** Check & switch C2 to given mode
 *
 * @param[in]  mode  mode to switch into
 */
bool furi_hal_bt_ensure_c2_mode(BleGlueC2Mode mode);

/**
 * Extra BLE beacon API 
 */

/** Set extra beacon data. Can be called in any state
 *
 * @param[in]  data  data to set
 * @param[in]  len   data length. Must be <= EXTRA_BEACON_MAX_DATA_SIZE
 *
 * @return     true on success
 */
bool furi_hal_bt_extra_beacon_set_data(const uint8_t* data, uint8_t len);

/** Get last configured extra beacon data
 *
 * @param      data  data buffer to write to. Must be at least EXTRA_BEACON_MAX_DATA_SIZE bytes long
 *
 * @return     valid data length
 */
uint8_t furi_hal_bt_extra_beacon_get_data(uint8_t* data);

/** Configure extra beacon.
 *
 * @param[in]  config  extra beacon config: interval, power, address, etc.
 *
 * @return     true on success
 */
bool furi_hal_bt_extra_beacon_set_config(const GapExtraBeaconConfig* config);

/** Start extra beacon. 
 * Beacon must configured with furi_hal_bt_extra_beacon_set_config()
 * and in stopped state before calling this function.
 *
 * @return     true on success
 */
bool furi_hal_bt_extra_beacon_start(void);

/** Stop extra beacon
 *
 * @return     true on success
 */
bool furi_hal_bt_extra_beacon_stop(void);

/** Check if extra beacon is active.
 *
 * @return     extra beacon state
 */
bool furi_hal_bt_extra_beacon_is_active(void);

/** Get last configured extra beacon config
 *
 * @return     extra beacon config. NULL if beacon had never been configured.
 */
const GapExtraBeaconConfig* furi_hal_bt_extra_beacon_get_config(void);

#ifdef __cplusplus
}
#endif
