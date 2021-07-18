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
    ApiHalSubGhzPresetOokAsync,     /** OOK, asynchronous */
} ApiHalSubGhzPreset;

/**  Switchable Radio Paths */
typedef enum {
    ApiHalSubGhzPathIsolate,        /** Isolate Radio from antenna */
    ApiHalSubGhzPath433,            /** Center Frquency: 433MHz. Path 1: SW1RF1-SW2RF2, LCLCL */
    ApiHalSubGhzPath315,            /** Center Frquency: 315MHz. Path 2: SW1RF2-SW2RF1, LCLCLCL */
    ApiHalSubGhzPath868,            /** Center Frquency: 868MHz. Path 3: SW1RF3-SW2RF3, LCLC */
} ApiHalSubGhzPath;

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
void api_hal_subghz_init();

/** Send device to sleep mode */
void api_hal_subghz_sleep();

/** Dump info to stdout */
void api_hal_subghz_dump_state();

/** Load registers from preset by preset name 
 * @param preset to load
 */
void api_hal_subghz_load_preset(ApiHalSubGhzPreset preset);

/** Get status */
uint8_t api_hal_subghz_get_status();

/** Load registers
 * @param register-value pairs array, terminated with {0,0}
 */
void api_hal_subghz_load_registers(const uint8_t data[][2]);

/** Load PATABLE
 * @param data, 8 uint8_t values
 */
void api_hal_subghz_load_patable(const uint8_t data[8]);

/** Write packet to FIFO
 * @param data, bytes array
 * @param size, size
 */
void api_hal_subghz_write_packet(const uint8_t* data, uint8_t size);

/** Read packet from FIFO
 * @param data, pointer
 * @param size, size
 */
void api_hal_subghz_read_packet(uint8_t* data, uint8_t* size);

/** Flush rx FIFO buffer */
void api_hal_subghz_flush_rx();

/** Shutdown
 * Issue spwd command
 * @warning registers content will be lost
 */
void api_hal_subghz_shutdown();

/** Reset
 * Issue reset command
 * @warning registers content will be lost
 */
void api_hal_subghz_reset();

/** Switch to Idle */
void api_hal_subghz_idle();

/** Switch to Recieve */
void api_hal_subghz_rx();

/** Switch to Transmit */
void api_hal_subghz_tx();

/** Get RSSI value in dBm */
float api_hal_subghz_get_rssi();

/** Set frequency and path
 * This function automatically selects antenna matching network
 * @param frequency in herz
 * @return real frequency in herz
 */
uint32_t api_hal_subghz_set_frequency_and_path(uint32_t value);

/** Set frequency
 * @param frequency in herz
 * @return real frequency in herz
 */
uint32_t api_hal_subghz_set_frequency(uint32_t value);

/** Set path
 * @param radio path to use
 */
void api_hal_subghz_set_path(ApiHalSubGhzPath path);

/* High Level API */

/** Signal Timings Capture callback */
typedef void (*ApiHalSubGhzCaptureCallback)(bool level, uint32_t duration, void* context);

/** Set signal timings capture callback
 * @param callback - your callback for front capture
 */
void api_hal_subghz_set_async_rx_callback(ApiHalSubGhzCaptureCallback callback, void* context);

/** Enable signal timings capture 
 * Initializes GPIO and TIM2 for timings capture
 */
void api_hal_subghz_start_async_rx();

/** Disable signal timings capture
 * Resets GPIO and TIM2
 */
void api_hal_subghz_stop_async_rx();

/** Send buffer
 * Initializes GPIO, TIM2 and DMA1 for signal output
 * @param buffer - pointer to data buffer
 * @param buffer_size - buffer size in bytes
 */
void api_hal_subghz_start_async_tx(uint32_t* buffer, size_t buffer_size, size_t repeat);

/** Get repeats left count for async tx
 * @return packets left to send
 */
size_t api_hal_subghz_get_async_tx_repeat_left();

/** Wait for async transmission to complete */
void api_hal_subghz_wait_async_tx();

/** Stop async transmission and cleanup resources
 * Resets GPIO, TIM2, and DMA1
 */
void api_hal_subghz_stop_async_tx();

#ifdef __cplusplus
}
#endif
