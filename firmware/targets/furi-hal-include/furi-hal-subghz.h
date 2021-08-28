#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <toolbox/level_duration.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Radio Presets */
typedef enum {
    FuriHalSubGhzPresetOok270Async,     /** OOK, bandwidth 270kHz, asynchronous */
    FuriHalSubGhzPresetOok650Async,     /** OOK, bandwidth 650kHz, asynchronous */
    FuriHalSubGhzPreset2FSKAsync,       /** FM, asynchronous */
} FuriHalSubGhzPreset;

/**  Switchable Radio Paths */
typedef enum {
    FuriHalSubGhzPathIsolate,        /** Isolate Radio from antenna */
    FuriHalSubGhzPath433,            /** Center Frquency: 433MHz. Path 1: SW1RF1-SW2RF2, LCLCL */
    FuriHalSubGhzPath315,            /** Center Frquency: 315MHz. Path 2: SW1RF2-SW2RF1, LCLCLCL */
    FuriHalSubGhzPath868,            /** Center Frquency: 868MHz. Path 3: SW1RF3-SW2RF3, LCLC */
} FuriHalSubGhzPath;

/** SubGhz state */
typedef enum {
    SubGhzStateInit,        /** Init pending */

    SubGhzStateIdle,        /** Idle, energy save mode */

    SubGhzStateAsyncRx,   /** Async RX started */

    SubGhzStateAsyncTx,   /** Async TX started, DMA and timer is on */
    SubGhzStateAsyncTxLast, /** Async TX continue, DMA completed and timer got last value to go */
    SubGhzStateAsyncTxEnd,  /** Async TX complete, cleanup needed */
} SubGhzState;

/** Initialize and switch to power save mode
 * Used by internal API-HAL initalization routine
 * Can be used to reinitialize device to safe state and send it to sleep
 */
void furi_hal_subghz_init();

/** Send device to sleep mode */
void furi_hal_subghz_sleep();

/** Dump info to stdout */
void furi_hal_subghz_dump_state();

/** Load registers from preset by preset name 
 * @param preset to load
 */
void furi_hal_subghz_load_preset(FuriHalSubGhzPreset preset);

/** Get status */
uint8_t furi_hal_subghz_get_status();

/** Load registers
 * @param register-value pairs array, terminated with {0,0}
 */
void furi_hal_subghz_load_registers(const uint8_t data[][2]);

/** Load PATABLE
 * @param data, 8 uint8_t values
 */
void furi_hal_subghz_load_patable(const uint8_t data[8]);

/** Write packet to FIFO
 * @param data, bytes array
 * @param size, size
 */
void furi_hal_subghz_write_packet(const uint8_t* data, uint8_t size);

/** Read packet from FIFO
 * @param data, pointer
 * @param size, size
 */
void furi_hal_subghz_read_packet(uint8_t* data, uint8_t* size);

/** Flush rx FIFO buffer */
void furi_hal_subghz_flush_rx();

/** Shutdown
 * Issue spwd command
 * @warning registers content will be lost
 */
void furi_hal_subghz_shutdown();

/** Reset
 * Issue reset command
 * @warning registers content will be lost
 */
void furi_hal_subghz_reset();

/** Switch to Idle */
void furi_hal_subghz_idle();

/** Switch to Recieve */
void furi_hal_subghz_rx();

/** Switch to Transmit */
void furi_hal_subghz_tx();

/** Get RSSI value in dBm */
float furi_hal_subghz_get_rssi();

/** Check if frequency is in valid range
 * @return true if frequncy is valid, otherwise false
 */
bool furi_hal_subghz_is_frequency_valid(uint32_t value);

/** Set frequency and path
 * This function automatically selects antenna matching network
 * @param frequency in herz
 * @return real frequency in herz
 */
uint32_t furi_hal_subghz_set_frequency_and_path(uint32_t value);

/** Set frequency
 * @param frequency in herz
 * @return real frequency in herz
 */
uint32_t furi_hal_subghz_set_frequency(uint32_t value);

/** Set path
 * @param radio path to use
 */
void furi_hal_subghz_set_path(FuriHalSubGhzPath path);

/* High Level API */

/** Signal Timings Capture callback */
typedef void (*FuriHalSubGhzCaptureCallback)(bool level, uint32_t duration, void* context);

/** Enable signal timings capture 
 * Initializes GPIO and TIM2 for timings capture
 */
void furi_hal_subghz_start_async_rx(FuriHalSubGhzCaptureCallback callback, void* context);

/** Disable signal timings capture
 * Resets GPIO and TIM2
 */
void furi_hal_subghz_stop_async_rx();

/** Async TX callback type
 * @param context - callback context
 * @return LevelDuration
 */
typedef LevelDuration (*FuriHalSubGhzAsyncTxCallback)(void* context);

/** Start async TX
 * Initializes GPIO, TIM2 and DMA1 for signal output
 */
void furi_hal_subghz_start_async_tx(FuriHalSubGhzAsyncTxCallback callback, void* context);

/** Wait for async transmission to complete */
bool furi_hal_subghz_is_async_tx_complete();

/** Stop async transmission and cleanup resources
 * Resets GPIO, TIM2, and DMA1
 */
void furi_hal_subghz_stop_async_tx();

#ifdef __cplusplus
}
#endif
