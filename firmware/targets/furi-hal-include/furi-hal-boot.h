/**
 * @file furi-hal-boot.h
 * Bootloader HAL API
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Boot modes */
typedef enum {
    FuriHalBootModeNormal,
    FuriHalBootModeDFU
} FuriHalBootMode;

/** Boot flags */
typedef enum {
    FuriHalBootFlagDefault=0,
    FuriHalBootFlagFactoryReset=1,
} FuriHalBootFlag;

/** Initialize boot subsystem
 */
void furi_hal_boot_init();

/** Set boot mode
 *
 * @param[in]  mode  FuriHalBootMode
 */
void furi_hal_boot_set_mode(FuriHalBootMode mode);

/** Set boot flags
 *
 * @param[in]  flags  FuriHalBootFlag
 */
void furi_hal_boot_set_flags(FuriHalBootFlag flags);

/** Get boot flag
 *
 * @return     FuriHalBootFlag
 */
FuriHalBootFlag furi_hal_boot_get_flags();

#ifdef __cplusplus
}
#endif
