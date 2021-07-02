#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Boot modes */
typedef enum {
    ApiHalBootModeNormal,
    ApiHalBootModeDFU
} ApiHalBootMode;

/** Boot flags */
typedef enum {
    ApiHalBootFlagDefault=0,
    ApiHalBootFlagFactoryReset=1,
} ApiHalBootFlag;

/** Initialize boot subsystem */
void api_hal_boot_init();

/** Set boot mode */
void api_hal_boot_set_mode(ApiHalBootMode mode);

/** Set boot flags */
void api_hal_boot_set_flags(ApiHalBootFlag flags);

/** Get boot flag */
ApiHalBootFlag api_hal_boot_get_flags();

#ifdef __cplusplus
}
#endif
