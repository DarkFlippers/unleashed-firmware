/**
 * @file furi_hal_subghz.h
 * SubGhz HAL API
 */

#pragma once

#include <lib/subghz/devices/preset.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <toolbox/level_duration.h>
#include <furi_hal_gpio.h>
// #include <furi_hal_spi_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Low level buffer dimensions and guard times */
#define API_HAL_SUBGHZ_ASYNC_TX_BUFFER_FULL (256)
#define API_HAL_SUBGHZ_ASYNC_TX_BUFFER_HALF (API_HAL_SUBGHZ_ASYNC_TX_BUFFER_FULL / 2)
#define API_HAL_SUBGHZ_ASYNC_TX_GUARD_TIME 999

/** Switchable Radio Paths */
typedef enum {
    FuriHalSubGhzPathIsolate, /**< Isolate Radio from antenna */
    FuriHalSubGhzPath433, /**< Center Frequency: 433MHz. Path 1: SW1RF1-SW2RF2, LCLCL */
    FuriHalSubGhzPath315, /**< Center Frequency: 315MHz. Path 2: SW1RF2-SW2RF1, LCLCLCL */
    FuriHalSubGhzPath868, /**< Center Frequency: 868MHz. Path 3: SW1RF3-SW2RF3, LCLC */
} FuriHalSubGhzPath;

/* Mirror RX/TX async modulation signal to specified pin
 *
 * @warning    Configures pin to output mode. Make sure it is not connected
 *             directly to power or ground.
 *
 * @param[in]  pin   pointer to the gpio pin structure or NULL to disable
 */
void furi_hal_subghz_set_async_mirror_pin(const GpioPin* pin);

/** Get data GPIO
 *
 * @return     pointer to the gpio pin structure
 */
const GpioPin* furi_hal_subghz_get_data_gpio();

/** Initialize and switch to power save mode Used by internal API-HAL
 * initialization routine Can be used to reinitialize device to safe state and
 * send it to sleep
 */
void furi_hal_subghz_init();

/** Send device to sleep mode
 */
void furi_hal_subghz_sleep();

/** Dump info to stdout
 */
void furi_hal_subghz_dump_state();

/** Load custom registers from preset
 *
 * @param      preset_data   registers to load
 */
void furi_hal_subghz_load_custom_preset(const uint8_t* preset_data);

/** Load registers
 *
 * @param      data  Registers data
 */
void furi_hal_subghz_load_registers(const uint8_t* data);

/** Load PATABLE
 *
 * @param      data  8 uint8_t values
 */
void furi_hal_subghz_load_patable(const uint8_t data[8]);

/** Write packet to FIFO
 *
 * @param      data  bytes array
 * @param      size  size
 */
void furi_hal_subghz_write_packet(const uint8_t* data, uint8_t size);

/** Check if receive pipe is not empty
 *
 * @return     true if not empty
 */
bool furi_hal_subghz_rx_pipe_not_empty();

/** Check if received data crc is valid
 *
 * @return     true if valid
 */
bool furi_hal_subghz_is_rx_data_crc_valid();

/** Read packet from FIFO
 *
 * @param      data  pointer
 * @param      size  size
 */
void furi_hal_subghz_read_packet(uint8_t* data, uint8_t* size);

/** Flush rx FIFO buffer
 */
void furi_hal_subghz_flush_rx();

/** Flush tx FIFO buffer
 */
void furi_hal_subghz_flush_tx();

/** Shutdown Issue SPWD command
 * @warning    registers content will be lost
 */
void furi_hal_subghz_shutdown();

/** Reset Issue reset command
 * @warning    registers content will be lost
 */
void furi_hal_subghz_reset();

/** Switch to Idle
 */
void furi_hal_subghz_idle();

/** Switch to Receive
 */
void furi_hal_subghz_rx();

/** Switch to Transmit
 *
 * @return     true if the transfer is allowed by belonging to the region
 */
bool furi_hal_subghz_tx();

/** Get RSSI value in dBm
 *
 * @return     RSSI value
 */
float furi_hal_subghz_get_rssi();

/** Get LQI
 *
 * @return     LQI value
 */
uint8_t furi_hal_subghz_get_lqi();

/** Check if frequency is in valid range
 *
 * @param      value  frequency in Hz
 *
 * @return     true if frequency is valid, otherwise false
 */
bool furi_hal_subghz_is_frequency_valid(uint32_t value);

/** Set frequency and path This function automatically selects antenna matching
 * network
 *
 * @param      value  frequency in Hz
 *
 * @return     real frequency in Hz
 */
uint32_t furi_hal_subghz_set_frequency_and_path(uint32_t value);

/** Сheck if transmission is allowed on this frequency with your current config
 *
 * @param      value  frequency in Hz
 *
 * @return     true if allowed
 */
bool furi_hal_subghz_is_tx_allowed(uint32_t value);

/** Get the current rolling protocols counter ++/-- value
 * @return    int8_t current value
 */
int8_t furi_hal_subghz_get_rolling_counter_mult(void);

/** Set the current rolling protocols counter ++/-- value
 * @param      mult int8_t = -1, -10, -100, 0, 1, 10, 100 
 */
void furi_hal_subghz_set_rolling_counter_mult(int8_t mult);

/** Set frequency
 *
 * @param      value  frequency in Hz
 *
 * @return     real frequency in Hz
 */
uint32_t furi_hal_subghz_set_frequency(uint32_t value);

/** Set path
 *
 * @param      path  path to use
 */
void furi_hal_subghz_set_path(FuriHalSubGhzPath path);

/* High Level API */

/** Signal Timings Capture callback */
typedef void (*FuriHalSubGhzCaptureCallback)(bool level, uint32_t duration, void* context);

/** Enable signal timings capture Initializes GPIO and TIM2 for timings capture
 *
 * @param      callback  FuriHalSubGhzCaptureCallback
 * @param      context   callback context
 */
void furi_hal_subghz_start_async_rx(FuriHalSubGhzCaptureCallback callback, void* context);

/** Disable signal timings capture Resets GPIO and TIM2
 */
void furi_hal_subghz_stop_async_rx();

/** Async TX callback type
 * @param      context  callback context
 * @return     LevelDuration
 */
typedef LevelDuration (*FuriHalSubGhzAsyncTxCallback)(void* context);

/** Start async TX Initializes GPIO, TIM2 and DMA1 for signal output
 *
 * @param      callback  FuriHalSubGhzAsyncTxCallback
 * @param      context   callback context
 *
 * @return     true if the transfer is allowed by belonging to the region
 */
bool furi_hal_subghz_start_async_tx(FuriHalSubGhzAsyncTxCallback callback, void* context);

/** Wait for async transmission to complete
 *
 * @return     true if TX complete
 */
bool furi_hal_subghz_is_async_tx_complete();

/** Stop async transmission and cleanup resources Resets GPIO, TIM2, and DMA1
 */
void furi_hal_subghz_stop_async_tx();

// /** Initialize and switch to power save mode Used by internal API-HAL
//  * initialization routine Can be used to reinitialize device to safe state and
//  * send it to sleep
//  * @return     true if initialisation is successfully
//  */
// bool furi_hal_subghz_init_check(void);

// /** Switching between internal and external radio
//  * @param      state SubGhzRadioInternal or SubGhzRadioExternal
//  * @return     true if switching is successful
//  */
// bool furi_hal_subghz_init_radio_type(SubGhzRadioType state);

// /** Get current radio
//  * @return     SubGhzRadioInternal or SubGhzRadioExternal
//  */
// SubGhzRadioType furi_hal_subghz_get_radio_type(void);

// /** Check for a radio module
//  * @return     true if check is successful
//  */
// bool furi_hal_subghz_check_radio(void);

// /** Turn on the power of the external radio module
//  * @return     true if power-up is successful
//  */
// bool furi_hal_subghz_enable_ext_power(void);

// /** Turn off the power of the external radio module
//  */
// void furi_hal_subghz_disable_ext_power(void);

// /** If true - disable 5v power of the external radio module
//  */
// void furi_hal_subghz_set_external_power_disable(bool state);

// /** Get the current state of the external power disable flag
//  */
// bool furi_hal_subghz_get_external_power_disable(void);

// /** Set what radio module we will be using
//  */
// void furi_hal_subghz_select_radio_type(SubGhzRadioType state);

// External CC1101 Ebytes power amplifier control
void furi_hal_subghz_set_ext_power_amp(bool enabled);

bool furi_hal_subghz_get_ext_power_amp();

#ifdef __cplusplus
}
#endif
