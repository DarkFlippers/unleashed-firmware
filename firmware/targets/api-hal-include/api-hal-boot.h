#pragma once

typedef enum {
    ApiHalBootModeNormal,
    ApiHalBootModeDFU
} ApiHalBootMode;

void api_hal_boot_set_mode(ApiHalBootMode mode);
