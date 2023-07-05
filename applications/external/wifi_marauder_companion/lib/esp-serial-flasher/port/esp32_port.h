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
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  uint32_t baud_rate;     /*!< Initial baud rate, can be changed later */
  uint32_t uart_port;     /*!< UART port */
  uint32_t uart_rx_pin;   /*!< This pin will be configured as UART Rx pin */
  uint32_t uart_tx_pin;   /*!< This pin will be configured as UART Tx pin */
  uint32_t reset_trigger_pin; /*!< This pin will be used to reset target chip */
  uint32_t gpio0_trigger_pin; /*!< This pin will be used to toggle set IO0 of target chip */
  uint32_t rx_buffer_size;    /*!< Set to zero for default RX buffer size */
  uint32_t tx_buffer_size;    /*!< Set to zero for default TX buffer size */
  uint32_t queue_size;        /*!< Set to zero for default UART queue size */
  QueueHandle_t *uart_queue;  /*!< Set to NULL, if UART queue handle is not
                                   necessary. Otherwise, it will be assigned here */
} loader_esp32_config_t;

/**
  * @brief Initializes serial interface.
  *
  * @param baud_rate[in]       Communication speed.
  *
  * @return
  *     - ESP_LOADER_SUCCESS Success
  *     - ESP_LOADER_ERROR_FAIL Initialization failure
  */
esp_loader_error_t loader_port_esp32_init(const loader_esp32_config_t *config);

/**
  * @brief Deinitialize serial interface.
  */
void loader_port_esp32_deinit(void);

#ifdef __cplusplus
}
#endif
