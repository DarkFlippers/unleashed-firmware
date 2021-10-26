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

/** Boot flags */
typedef enum {
    FuriHalBootloaderFlagDefault=0,
    FuriHalBootloaderFlagFactoryReset=1,
} FuriHalBootloaderFlag;

/** Initialize boot subsystem
 */
void furi_hal_bootloader_init();

/** Set bootloader mode
 *
 * @param[in]  mode  FuriHalBootloaderMode
 */
void furi_hal_bootloader_set_mode(FuriHalBootloaderMode mode);

/** Set bootloader flags
 *
 * @param[in]  flags  FuriHalBootloaderFlag
 */
void furi_hal_bootloader_set_flags(FuriHalBootloaderFlag flags);

/** Get boot flag
 *
 * @return     FuriHalBootloaderFlag
 */
FuriHalBootloaderFlag furi_hal_bootloader_get_flags();

#ifdef __cplusplus
}
#endif
