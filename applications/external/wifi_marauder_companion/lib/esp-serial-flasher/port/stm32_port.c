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

#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/param.h>
#include <stdio.h>
#include "stm32_port.h"

static UART_HandleTypeDef *uart;
static GPIO_TypeDef* gpio_port_io0, *gpio_port_rst;
static uint16_t gpio_num_io0, gpio_num_rst;

#ifdef SERIAL_FLASHER_DEBUG_TRACE
static void transfer_debug_print(const uint8_t *data, uint16_t size, bool write)
{
    static bool write_prev = false;

    if (write_prev != write) {
        write_prev = write;
        printf("\n--- %s ---\n", write ? "WRITE" : "READ");
    }

    for (uint32_t i = 0; i < size; i++) {
        printf("%02x ", data[i]);
    }
}
#endif

static uint32_t s_time_end;

esp_loader_error_t loader_port_write(const uint8_t *data, uint16_t size, uint32_t timeout)
{
    HAL_StatusTypeDef err = HAL_UART_Transmit(uart, (uint8_t *)data, size, timeout);

    if (err == HAL_OK) {
#ifdef SERIAL_FLASHER_DEBUG_TRACE
        transfer_debug_print(data, size, true);
#endif
        return ESP_LOADER_SUCCESS;
    } else if (err == HAL_TIMEOUT) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_ERROR_FAIL;
    }
}


esp_loader_error_t loader_port_read(uint8_t *data, uint16_t size, uint32_t timeout)
{
    HAL_StatusTypeDef err = HAL_UART_Receive(uart, data, size, timeout);

    if (err == HAL_OK) {
#ifdef SERIAL_FLASHER_DEBUG_TRACE
        transfer_debug_print(data, size, false);
#endif
        return ESP_LOADER_SUCCESS;
    } else if (err == HAL_TIMEOUT) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_ERROR_FAIL;
    }
}

void loader_port_stm32_init(loader_stm32_config_t *config)

{
    uart = config->huart;
    gpio_port_io0 = config->port_io0;
    gpio_port_rst = config->port_rst;
    gpio_num_io0 = config->pin_num_io0;
    gpio_num_rst = config->pin_num_rst;
}

// Set GPIO0 LOW, then
// assert reset pin for 100 milliseconds.
void loader_port_enter_bootloader(void)
{
    HAL_GPIO_WritePin(gpio_port_io0, gpio_num_io0, GPIO_PIN_RESET);
    loader_port_reset_target();
    HAL_Delay(SERIAL_FLASHER_BOOT_HOLD_TIME_MS);
    HAL_GPIO_WritePin(gpio_port_io0, gpio_num_io0, GPIO_PIN_SET);
}


void loader_port_reset_target(void)
{
    HAL_GPIO_WritePin(gpio_port_rst, gpio_num_rst, GPIO_PIN_RESET);
    HAL_Delay(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
    HAL_GPIO_WritePin(gpio_port_rst, gpio_num_rst, GPIO_PIN_SET);
}


void loader_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}


void loader_port_start_timer(uint32_t ms)
{
    s_time_end = HAL_GetTick() + ms;
}


uint32_t loader_port_remaining_time(void)
{
    int32_t remaining = s_time_end - HAL_GetTick();
    return (remaining > 0) ? (uint32_t)remaining : 0;
}


void loader_port_debug_print(const char *str)
{
    printf("DEBUG: %s", str);
}

esp_loader_error_t loader_port_change_transmission_rate(uint32_t baudrate)
{
    uart->Init.BaudRate = baudrate;

    if( HAL_UART_Init(uart) != HAL_OK ) {
        return ESP_LOADER_ERROR_FAIL;
    }

    return ESP_LOADER_SUCCESS;
}
