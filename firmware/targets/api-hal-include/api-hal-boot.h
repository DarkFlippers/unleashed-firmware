#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ApiHalBootModeNormal,
    ApiHalBootModeDFU
} ApiHalBootMode;

void api_hal_boot_set_mode(ApiHalBootMode mode);

#ifdef __cplusplus
}
#endif
