/**
 * @file expansion.h
 * @brief Expansion module support library.
 */
#pragma once

#include <furi_hal_serial_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FURI record key to access the expansion object.
 */
#define RECORD_EXPANSION "expansion"

/**
 * @brief Expansion opaque type declaration.
 */
typedef struct Expansion Expansion;

/**
 * @brief Enable support for expansion modules on designated serial port.
 *
 * Only one serial port can be used to communicate with an expansion
 * module at a time.
 *
 * Calling this function when expansion module support is already enabled
 * will first disable the previous setting, then enable the current one.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 * @param[in] serial_id numerical identifier of the serial.
 */
void expansion_enable(Expansion* instance, FuriHalSerialId serial_id);

/**
 * @brief Disable support for expansion modules.
 *
 * Calling this function will cease all communications with the
 * expansion module (if any), release the serial handle and
 * reset the respective pins to the default state.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 */
void expansion_disable(Expansion* instance);

#ifdef __cplusplus
}
#endif
