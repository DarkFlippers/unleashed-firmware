/**
 * @file furi-hal-info.h
 * Device info HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Callback type called every time another key-value pair of device information is ready
 *
 * @param      key[in]      device information type identifier
 * @param      value[in]    device information value
 * @param      last[in]     whether the passed key-value pair is the last one
 * @param      context[in]  to pass to callback
 */
typedef void (*FuriHalInfoValueCallback) (const char* key, const char* value, bool last, void* context);

/** Get device information
 *
 * @param[in]  callback     callback to provide with new data
 * @param[in]  context      context to pass to callback
 */
void furi_hal_info_get(FuriHalInfoValueCallback callback, void* context);

#ifdef __cplusplus
}
#endif
