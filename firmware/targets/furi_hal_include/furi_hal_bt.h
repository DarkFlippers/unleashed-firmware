/**
 * @file furi_hal_bt.h
 * BT/BLE HAL API
 */

#pragma once

#include <furi.h>
#include <stdbool.h>
#include <gap.h>
#include <serial_service.h>
#include <ble_glue.h>
#include <ble_app.h>

#include <furi_hal_bt_serial.h>

#define FURI_HAL_BT_STACK_VERSION_MAJOR (1)
#define FURI_HAL_BT_STACK_VERSION_MINOR (12)
#define FURI_HAL_BT_C2_START_TIMEOUT 1000

#define FURI_HAL_BT_EMPTY_MAC_ADDR \
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

#define FURI_HAL_BT_DEFAULT_MAC_ADDR \
    { 0x6c, 0x7a, 0xd8, 0xac, 0x57, 0x72 }

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriHalBtStackUnknown,
    FuriHalBtStackLight,
    FuriHalBtStackFull,
} FuriHalBtStack;

typedef enum {
    FuriHalBtProfileSerial,
    FuriHalBtProfileHidKeyboard,

    // Keep last for Profiles number calculation
    FuriHalBtProfileNumber,
} FuriHalBtProfile;

/** Initialize
 */
void furi_hal_bt_init();

/** Lock core2 state transition */
void furi_hal_bt_lock_core2();

/** Lock core2 state transition */
void furi_hal_bt_unlock_core2();

/** Start radio stack
 *
 * @return  true on successfull radio stack start
 */
bool furi_hal_bt_start_radio_stack();

/** Get radio stack type
 *
 * @return  FuriHalBtStack instance
 */
FuriHalBtStack furi_hal_bt_get_radio_stack();

/** Check if radio stack supports BLE GAT/GAP
 *
 * @return  true if supported
 */
bool furi_hal_bt_is_ble_gatt_gap_supported();

/** Check if radio stack supports testing
 *
 * @return  true if supported
 */
bool furi_hal_bt_is_testing_supported();

/** Start BLE app
 *
 * @param profile   FuriHalBtProfile instance
 * @param event_cb  GapEventCallback instance
 * @param context   pointer to context
 *
 * @return          true on success
*/
bool furi_hal_bt_start_app(FuriHalBtProfile profile, GapEventCallback event_cb, void* context);

/** Reinitialize core2
 * 
 * Also can be used to prepare core2 for stop modes
 */
void furi_hal_bt_reinit();

/** Change BLE app
 * Restarts 2nd core
 *
 * @param profile   FuriHalBtProfile instance
 * @param event_cb  GapEventCallback instance
 * @param context   pointer to context
 *
 * @return          true on success
*/
bool furi_hal_bt_change_app(FuriHalBtProfile profile, GapEventCallback event_cb, void* context);

/** Update battery level
 *
 * @param battery_level battery level
 */
void furi_hal_bt_update_battery_level(uint8_t battery_level);

/** Update battery power state */
void furi_hal_bt_update_power_state();

/** Checks if BLE state is active
 *
 * @return          true if device is connected or advertising, false otherwise
 */
bool furi_hal_bt_is_active();

/** Start advertising
 */
void furi_hal_bt_start_advertising();

/** Stop advertising
 */
void furi_hal_bt_stop_advertising();

/** Get BT/BLE system component state
 *
 * @param[in]  buffer  FuriString* buffer to write to
 */
void furi_hal_bt_dump_state(FuriString* buffer);

/** Get BT/BLE system component state
 *
 * @return     true if core2 is alive
 */
bool furi_hal_bt_is_alive();

/** Get key storage buffer address and size
 *
 * @param      key_buff_addr  pointer to store buffer address
 * @param      key_buff_size  pointer to store buffer size
 */
void furi_hal_bt_get_key_storage_buff(uint8_t** key_buff_addr, uint16_t* key_buff_size);

/** Get SRAM2 hardware semaphore
 * @note Must be called before SRAM2 read/write operations
 */
void furi_hal_bt_nvm_sram_sem_acquire();

/** Release SRAM2 hardware semaphore
 * @note Must be called after SRAM2 read/write operations
 */
void furi_hal_bt_nvm_sram_sem_release();

/** Clear key storage
 *
 * @return      true on success
*/
bool furi_hal_bt_clear_white_list();

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
void furi_hal_bt_stop_tone_tx();

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
uint16_t furi_hal_bt_stop_packet_test();

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
void furi_hal_bt_stop_rx();

/** Get RSSI
 *
 * @return     RSSI in dBm
 */
float furi_hal_bt_get_rssi();

/** Get number of transmitted packets
 *
 * @return     packet count
 */
uint32_t furi_hal_bt_get_transmitted_packets();

// BadBT Stuff
/** Reverse a MAC address byte order in-place
 * @param[in] mac       mac address to reverse
*/
void furi_hal_bt_reverse_mac_addr(uint8_t mac_addr[GAP_MAC_ADDR_SIZE]);

/** Modify profile advertisement name and restart bluetooth
 * @param[in] profile   profile type
 * @param[in] name      new adv name
*/
void furi_hal_bt_set_profile_adv_name(
    FuriHalBtProfile profile,
    const char name[FURI_HAL_BT_ADV_NAME_LENGTH]);

const char* furi_hal_bt_get_profile_adv_name(FuriHalBtProfile profile);

/** Modify profile mac address and restart bluetooth
 * @param[in] profile   profile type
 * @param[in] mac       new mac address
*/
void furi_hal_bt_set_profile_mac_addr(
    FuriHalBtProfile profile,
    const uint8_t mac_addr[GAP_MAC_ADDR_SIZE]);

const uint8_t* furi_hal_bt_get_profile_mac_addr(FuriHalBtProfile profile);

uint32_t furi_hal_bt_get_conn_rssi(uint8_t* rssi);

void furi_hal_bt_set_profile_pairing_method(FuriHalBtProfile profile, GapPairing pairing_method);

GapPairing furi_hal_bt_get_profile_pairing_method(FuriHalBtProfile profile);

bool furi_hal_bt_is_connected(void);

/** Check & switch C2 to given mode
 *
 * @param[in]  mode  mode to switch into
 */
bool furi_hal_bt_ensure_c2_mode(BleGlueC2Mode mode);

typedef struct {
    uint32_t magic;
    uint32_t source_pc;
    uint32_t source_lr;
    uint32_t source_sp;
} FuriHalBtHardfaultInfo;

/** Get hardfault info
 *
 * @return     hardfault info. NULL if no hardfault
 */
const FuriHalBtHardfaultInfo* furi_hal_bt_get_hardfault_info();

#ifdef __cplusplus
}
#endif
