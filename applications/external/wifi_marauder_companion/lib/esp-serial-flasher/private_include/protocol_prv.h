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
#include <stdbool.h>
#include <stddef.h>
#include "esp_loader.h"
#include "protocol.h"

void log_loader_internal_error(error_code_t error);

esp_loader_error_t send_cmd(const void *cmd_data, uint32_t size, uint32_t *reg_value);

esp_loader_error_t send_cmd_with_data(const void *cmd_data, size_t cmd_size,
                                      const void *data, size_t data_size);

esp_loader_error_t send_cmd_md5(const void *cmd_data, size_t cmd_size, uint8_t md5_out[MD5_SIZE]);
