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

/** Set boot mode */
void api_hal_boot_set_mode(ApiHalBootMode mode);

#ifdef __cplusplus
}
#endif
