/**
 * @file cc1101_ext.h
 * @brief External CC1101 transceiver access API.
 */

#pragma once
#include <lib/subghz/devices/preset.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <toolbox/level_duration.h>
#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mirror RX/TX async modulation signal to specified pin
 *
 * @warning    Configures pin to output mode. Make sure it is not connected
 *             directly to power or ground.
 *
 * @param[in]  pin   pointer to the gpio pin structure or NULL to disable
 */
void subghz_device_cc1101_ext_set_async_mirror_pin(const GpioPin* pin);

/** Get data GPIO
 *
 * @return     pointer to the gpio pin structure
 */
const GpioPin* subghz_device_cc1101_ext_get_data_gpio(void);

/** Initialize device
 *
 * @return     true if success
 */
bool subghz_device_cc1101_ext_alloc(void);

/** Deinitialize device
 */
void subghz_device_cc1101_ext_free(void);

/** Check and switch to power save mode Used by internal API-HAL
 * initialization routine Can be used to reinitialize device to safe state and
 * send it to sleep
 */
bool subghz_device_cc1101_ext_is_connect(void);

/** Send device to sleep mode
 */
void subghz_device_cc1101_ext_sleep(void);

/** Dump info to stdout
 */
void subghz_device_cc1101_ext_dump_state(void);

/** Load custom registers from preset
 *
 * @param      preset_data   registers to load
 */
void subghz_device_cc1101_ext_load_custom_preset(const uint8_t* preset_data);

/** Load registers
 *
 * @param      data  Registers data
 */
void subghz_device_cc1101_ext_load_registers(const uint8_t* data);

/** Load PATABLE
 *
 * @param      data  8 uint8_t values
 */
void subghz_device_cc1101_ext_load_patable(const uint8_t data[8]);

/** Write packet to FIFO
 *
 * @param      data  bytes array
 * @param      size  size
 */
void subghz_device_cc1101_ext_write_packet(const uint8_t* data, uint8_t size);

/** Check if receive pipe is not empty
 *
 * @return     true if not empty
 */
bool subghz_device_cc1101_ext_rx_pipe_not_empty(void);

/** Check if received data crc is valid
 *
 * @return     true if valid
 */
bool subghz_device_cc1101_ext_is_rx_data_crc_valid(void);

/** Read packet from FIFO
 *
 * @param      data  pointer
 * @param      size  size
 */
void subghz_device_cc1101_ext_read_packet(uint8_t* data, uint8_t* size);

/** Flush rx FIFO buffer
 */
void subghz_device_cc1101_ext_flush_rx(void);

/** Flush tx FIFO buffer
 */
void subghz_device_cc1101_ext_flush_tx(void);

/** Shutdown Issue SPWD command
 * @warning    registers content will be lost
 */
void subghz_device_cc1101_ext_shutdown(void);

/** Reset Issue reset command
 * @warning    registers content will be lost
 */
void subghz_device_cc1101_ext_reset(void);

/** Switch to Idle
 */
void subghz_device_cc1101_ext_idle(void);

/** Switch to Receive
 */
void subghz_device_cc1101_ext_rx(void);

/** Switch to Transmit
 *
 * @return     true if the transfer is allowed by belonging to the region
 */
bool subghz_device_cc1101_ext_tx(void);

/** Get RSSI value in dBm
 *
 * @return     RSSI value
 */
float subghz_device_cc1101_ext_get_rssi(void);

/** Get LQI
 *
 * @return     LQI value
 */
uint8_t subghz_device_cc1101_ext_get_lqi(void);

/** Check if frequency is in valid range
 *
 * @param      value  frequency in Hz
 *
 * @return     true if frequency is valid, otherwise false
 */
bool subghz_device_cc1101_ext_is_frequency_valid(uint32_t value);

/** Set frequency
 *
 * @param      value  frequency in Hz
 *
 * @return     real frequency in Hz
 */
uint32_t subghz_device_cc1101_ext_set_frequency(uint32_t value);

/* High Level API */

/** Signal Timings Capture callback */
typedef void (*SubGhzDeviceCC1101ExtCaptureCallback)(bool level, uint32_t duration, void* context);

/** Enable signal timings capture Initializes GPIO and TIM2 for timings capture
 *
 * @param      callback  SubGhzDeviceCC1101ExtCaptureCallback
 * @param      context   callback context
 */
void subghz_device_cc1101_ext_start_async_rx(
    SubGhzDeviceCC1101ExtCaptureCallback callback,
    void* context);

/** Disable signal timings capture Resets GPIO and TIM2
 */
void subghz_device_cc1101_ext_stop_async_rx(void);

/** Async TX callback type
 * @param      context  callback context
 * @return     LevelDuration
 */
typedef LevelDuration (*SubGhzDeviceCC1101ExtCallback)(void* context);

/** Start async TX Initializes GPIO, TIM2 and DMA1 for signal output
 *
 * @param      callback  SubGhzDeviceCC1101ExtCallback
 * @param      context   callback context
 *
 * @return     true if the transfer is allowed by belonging to the region
 */
bool subghz_device_cc1101_ext_start_async_tx(SubGhzDeviceCC1101ExtCallback callback, void* context);

/** Wait for async transmission to complete
 *
 * @return     true if TX complete
 */
bool subghz_device_cc1101_ext_is_async_tx_complete(void);

/** Stop async transmission and cleanup resources Resets GPIO, TIM2, and DMA1
 */
void subghz_device_cc1101_ext_stop_async_tx(void);

#ifdef __cplusplus
}
#endif
