#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Check target firmware version */
bool api_hal_version_do_i_belong_here();

/** Get hardware version */
const uint8_t api_hal_version_get_hw_version();

/** Get hardware target */
const uint8_t api_hal_version_get_hw_target();

/** Get hardware body */
const uint8_t api_hal_version_get_hw_body();

/** Get hardware connect */
const uint8_t api_hal_version_get_hw_connect();

/** Get hardware timestamp */
const uint32_t api_hal_version_get_hw_timestamp();

/** Get pointer to target name */
const char * api_hal_version_get_name_ptr();

#ifdef __cplusplus
}
#endif
