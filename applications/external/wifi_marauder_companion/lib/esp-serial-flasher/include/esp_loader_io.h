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
#include "esp_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Changes the transmission rate of the used peripheral.
  */
esp_loader_error_t loader_port_change_transmission_rate(uint32_t transmission_rate);

/**
  * @brief Writes data over the io interface.
  *
  * @param data[in]     Buffer with data to be written.
  * @param size[in]     Size of data in bytes.
  * @param timeout[in]  Timeout in milliseconds.
  *
  * @return
  *     - ESP_LOADER_SUCCESS Success
  *     - ESP_LOADER_ERROR_TIMEOUT Timeout elapsed
  */
esp_loader_error_t loader_port_write(const uint8_t *data, uint16_t size, uint32_t timeout);

/**
  * @brief Reads data from the io interface.
  *
  * @param data[out]    Buffer into which received data will be written.
  * @param size[in]     Number of bytes to read.
  * @param timeout[in]  Timeout in milliseconds.
  *
  * @return
  *     - ESP_LOADER_SUCCESS Success
  *     - ESP_LOADER_ERROR_TIMEOUT Timeout elapsed
  */
esp_loader_error_t loader_port_read(uint8_t *data, uint16_t size, uint32_t timeout);

/**
  * @brief Delay in milliseconds.
  *
  * @param ms[in]   Number of milliseconds.
  *
  */
void loader_port_delay_ms(uint32_t ms);

/**
  * @brief Starts timeout timer.
  *
  * @param ms[in]   Number of milliseconds.
  *
  */
void loader_port_start_timer(uint32_t ms);

/**
  * @brief Returns remaining time since timer was started by calling esp_loader_start_timer.
  *        0 if timer has elapsed.
  *
  * @return   Number of milliseconds.
  *
  */
uint32_t loader_port_remaining_time(void);

/**
  * @brief Asserts bootstrap pins to enter boot mode and toggles reset pin.
  *
  * @note  Reset pin should stay asserted for at least 20 milliseconds.
  */
void loader_port_enter_bootloader(void);

/**
  * @brief Toggles reset pin.
  *
  * @note  Reset pin should stay asserted for at least 20 milliseconds.
  */
void loader_port_reset_target(void);

/**
  * @brief Function can be defined by user to print debug message.
  *
  * @note  Empty weak function is used, otherwise.
  *
  */
void loader_port_debug_print(const char *str);

#ifdef SERIAL_FLASHER_INTERFACE_SPI
/**
  * @brief Sets the chip select to a defined level
  */
void loader_port_spi_set_cs(uint32_t level);
#endif /* SERIAL_FLASHER_INTERFACE_SPI */

#ifdef __cplusplus
}
#endif
