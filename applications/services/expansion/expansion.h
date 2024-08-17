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
 * @brief Enable support for expansion modules.
 *
 * Calling this function will load user settings and enable
 * expansion module support on the serial port specified in said settings.
 *
 * If expansion module support was disabled in settings, this function
 * does nothing.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 */
void expansion_enable(Expansion* instance);

/**
 * @brief Disable support for expansion modules.
 *
 * Calling this function will cease all communications with the
 * expansion module (if any), release the serial handle and
 * reset the respective pins to the default state.
 *
 * @note Applications requiring serial port access MUST call
 * this function BEFORE calling furi_hal_serial_control_acquire().
 * Similarly, an expansion_enable() call MUST be made right AFTER
 * a call to furi_hal_serial_control_release() to ensure that
 * the user settings are properly restored.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 */
void expansion_disable(Expansion* instance);

/**
 * @brief Enable support for expansion modules on designated serial port.
 *
 * Only one serial port can be used to communicate with an expansion
 * module at a time.
 *
 * Calling this function when expansion module support is already enabled
 * will first disable the previous setting, then enable the current one.
 *
 * @warning This function does not respect user settings for expansion modules,
 * so calling it might leave the system in inconsistent state. Avoid using it
 * unless absolutely necessary.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 * @param[in] serial_id numerical identifier of the serial.
 */
void expansion_set_listen_serial(Expansion* instance, FuriHalSerialId serial_id);

#ifdef __cplusplus
}
#endif
