/* Copyright 2020-2023 Espressif Systems (Shanghai) CO LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>
#include "esp_loader_io.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *device;
    uint32_t baudrate;
    uint32_t reset_trigger_pin;
    uint32_t gpio0_trigger_pin;
} loader_raspberry_config_t;

esp_loader_error_t loader_port_raspberry_init(const loader_raspberry_config_t *config);

#ifdef __cplusplus
}
#endif
