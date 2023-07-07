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

#include "esp_loader_io.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    spi_host_device_t spi_bus;
    uint32_t frequency;
    uint32_t spi_clk_pin;
    uint32_t spi_miso_pin;
    uint32_t spi_mosi_pin;
    uint32_t spi_cs_pin;
    uint32_t spi_quadwp_pin;
    uint32_t spi_quadhd_pin;
    uint32_t reset_trigger_pin;
    uint32_t strap_bit0_pin;
    uint32_t strap_bit1_pin;
    uint32_t strap_bit2_pin;
    uint32_t strap_bit3_pin;
} loader_esp32_spi_config_t;

/**
  * @brief Initializes the SPI interface.
  *
  * @param config[in] Configuration structure
  *
  * @return
  *     - ESP_LOADER_SUCCESS Success
  *     - ESP_LOADER_ERROR_FAIL Initialization failure
  */
esp_loader_error_t loader_port_esp32_spi_init(const loader_esp32_spi_config_t *config);

/**
  * @brief Deinitializes the SPI interface.
  */
void loader_port_esp32_spi_deinit(void);

#ifdef __cplusplus
}
#endif
