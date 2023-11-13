/**
 * @file furi_hal_info.h
 * Device info HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <core/string.h>
#include <toolbox/property.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_info_get_api_version(uint16_t* major, uint16_t* minor);

/** Get device information
 *
 * @param[in]  callback     callback to provide with new data
 * @param[in]  sep          category separator character
 * @param[in]  context      context to pass to callback
 */
void furi_hal_info_get(PropertyValueCallback callback, char sep, void* context);

#ifdef __cplusplus
}
#endif
