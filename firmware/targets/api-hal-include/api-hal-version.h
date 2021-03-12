#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

bool api_hal_version_do_i_belong_here();

const uint8_t api_hal_version_get_hw_version();

const uint8_t api_hal_version_get_hw_target();

const uint8_t api_hal_version_get_hw_body();

const uint8_t api_hal_version_get_hw_connect();

const uint32_t api_hal_version_get_hw_timestamp();

const char * api_hal_version_get_name_ptr();

#ifdef __cplusplus
}
#endif
