/**
 * @file furi-hal-bt.h
 * BT/BLE HAL API
 */

#pragma once

#include <m-string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize
 */
void furi_hal_bt_init();

/** Start BLE app
 *
 * @return     true if app inited
 */
bool furi_hal_bt_init_app();

/** Start advertising
 */
void furi_hal_bt_start_advertising();

/** Stop advertising
 */
void furi_hal_bt_stop_advertising();

/** Returns true if BLE is advertising
 *
 * @return     true if BLE advertising
 */
bool furi_hal_bt_is_active();

/** Get BT/BLE system component state
 *
 * @param[in]  buffer  string_t buffer to write to
 */
void furi_hal_bt_dump_state(string_t buffer);

/** Get BT/BLE system component state
 *
 * @return     true if core2 is alive
 */
bool furi_hal_bt_is_alive();

/** Wait for Core2 startup
 *
 * @return     true if success, otherwise timeouted
 */
bool furi_hal_bt_wait_startup();

/** Lock shared access to flash controller
 *
 * @param[in]  erase_flag  true if erase operation
 *
 * @return     true if lock was successful, false if not
 */
bool furi_hal_bt_lock_flash(bool erase_flag);

/** Unlock shared access to flash controller
 *
 * @param[in]  erase_flag  true if erase operation
 */
void furi_hal_bt_unlock_flash(bool erase_flag);

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

#ifdef __cplusplus
}
#endif
