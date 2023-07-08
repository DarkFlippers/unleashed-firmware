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

#include "esp_loader.h"
#include <stdint.h>
#include <stdlib.h>

esp_loader_error_t SLIP_receive_data(uint8_t *buff, size_t size);

esp_loader_error_t SLIP_receive_packet(uint8_t *buff, size_t size);

esp_loader_error_t SLIP_send(const uint8_t *data, size_t size);

esp_loader_error_t SLIP_send_delimiter(void);
