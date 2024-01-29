#pragma once

#include "furi_hal_serial_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize Serial Control */
void furi_hal_serial_control_init(void);

/** De-Initialize Serial Control */
void furi_hal_serial_control_deinit(void);

/** Suspend All Serial Interfaces
 *
 * @warning    this is internal method, can only be used in suppress tick
 *             callback
 */
void furi_hal_serial_control_suspend(void);

/** Resume All Serial Interfaces
 *
 * @warning    this is internal method, can only be used in suppress tick
 *             callback
 */
void furi_hal_serial_control_resume(void);

/** Acquire Serial Interface Handler
 *
 * @param[in]  serial_id  The serial transceiver identifier
 *
 * @return     The Serial Interface Handle or null if interfaces is in use
 */
FuriHalSerialHandle* furi_hal_serial_control_acquire(FuriHalSerialId serial_id);

/** Release Serial Interface Handler
 *
 * @param      handle  The handle
 */
void furi_hal_serial_control_release(FuriHalSerialHandle* handle);

/** Acquire Serial Interface Handler
 *
 * @param[in]  serial_id  The serial transceiver identifier
 *
 * @return     true if handle is acquired by someone
 */
bool furi_hal_serial_control_is_busy(FuriHalSerialId serial_id);

/** Acquire Serial Interface Handler
 *
 * @param[in]  serial_id  The serial transceiver identifier. Use FuriHalSerialIdMax to disable logging.
 * @param[in]  baud_rate  The baud rate
 *
 * @return     The Serial Interface Handle or null if interfaces is in use
 */
void furi_hal_serial_control_set_logging_config(FuriHalSerialId serial_id, uint32_t baud_rate);

/**
 * @brief Expansion module detection callback type.
 *
 * @param[in,out] context Pointer to the user-defined context object.
 */
typedef void (*FuriHalSerialControlExpansionCallback)(void* context);

/**
 * @brief Enable expansion module detection for a given serial interface.
 *
 * Passing NULL as the callback parameter disables external module detection.
 *
 * @param[in] serial_id Identifier of the serial interface to be used.
 * @param[in] callback Pointer to the callback function to be called upon module detection.
 * @param[in,out] context Pointer to the user-defined context object. Will be passed to the callback function.
 */
void furi_hal_serial_control_set_expansion_callback(
    FuriHalSerialId serial_id,
    FuriHalSerialControlExpansionCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
