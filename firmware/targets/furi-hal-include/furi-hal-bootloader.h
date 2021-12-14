/**
 * @file furi-hal-bootloader.h
 * Bootloader HAL API
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Boot modes */
typedef enum {
    FuriHalBootloaderModeNormal,
    FuriHalBootloaderModeDFU
} FuriHalBootloaderMode;

/** Initialize boot subsystem
 */
void furi_hal_bootloader_init();

/** Set bootloader mode
 *
 * @param[in]  mode  FuriHalBootloaderMode
 */
void furi_hal_bootloader_set_mode(FuriHalBootloaderMode mode);

#ifdef __cplusplus
}
#endif
