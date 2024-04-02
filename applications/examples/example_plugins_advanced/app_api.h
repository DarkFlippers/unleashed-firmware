/**
 * @file app_api.h
 * @brief Application API example.
 *
 * This file contains an API that is internally implemented by the application
 * It is also exposed to plugins to allow them to use the application's API.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void app_api_accumulator_set(uint32_t value);

uint32_t app_api_accumulator_get(void);

void app_api_accumulator_add(uint32_t value);

void app_api_accumulator_sub(uint32_t value);

void app_api_accumulator_mul(uint32_t value);

#ifdef __cplusplus
}
#endif
