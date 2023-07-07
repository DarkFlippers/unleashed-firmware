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

#include "esp32_port.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_idf_version.h"
#include <unistd.h>

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

static int64_t s_time_end;
static int32_t s_uart_port;
static int32_t s_reset_trigger_pin;
static int32_t s_gpio0_trigger_pin;

esp_loader_error_t loader_port_esp32_init(const loader_esp32_config_t *config)
{
    s_uart_port = config->uart_port;
    s_reset_trigger_pin = config->reset_trigger_pin;
    s_gpio0_trigger_pin = config->gpio0_trigger_pin;

    // Initialize UART
    uart_config_t uart_config = {
        .baud_rate = config->baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        .source_clk = UART_SCLK_DEFAULT,
#endif
    };

    int rx_buffer_size = config->rx_buffer_size ? config->rx_buffer_size : 400;
    int tx_buffer_size = config->tx_buffer_size ? config->tx_buffer_size : 400;
    QueueHandle_t *uart_queue = config->uart_queue ? config->uart_queue : NULL;
    int queue_size = config->queue_size ? config->queue_size : 0;

    if ( uart_param_config(s_uart_port, &uart_config) != ESP_OK ) {
        return ESP_LOADER_ERROR_FAIL;
    }
    if ( uart_set_pin(s_uart_port, config->uart_tx_pin, config->uart_rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK ) {
        return ESP_LOADER_ERROR_FAIL;
    }
    if ( uart_driver_install(s_uart_port, rx_buffer_size, tx_buffer_size, queue_size, uart_queue, 0) != ESP_OK ) {
        return ESP_LOADER_ERROR_FAIL;
    }

    // Initialize boot pin selection pins
    gpio_reset_pin(s_reset_trigger_pin);
    gpio_set_pull_mode(s_reset_trigger_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_reset_trigger_pin, GPIO_MODE_OUTPUT);

    gpio_reset_pin(s_gpio0_trigger_pin);
    gpio_set_pull_mode(s_gpio0_trigger_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_gpio0_trigger_pin, GPIO_MODE_OUTPUT);

    return ESP_LOADER_SUCCESS;
}

void loader_port_esp32_deinit(void)
{
    uart_driver_delete(s_uart_port);
}


esp_loader_error_t loader_port_write(const uint8_t *data, uint16_t size, uint32_t timeout)
{
    uart_write_bytes(s_uart_port, (const char *)data, size);
    esp_err_t err = uart_wait_tx_done(s_uart_port, pdMS_TO_TICKS(timeout));

    if (err == ESP_OK) {
#ifdef SERIAL_FLASHER_DEBUG_TRACE
        transfer_debug_print(data, size, true);
#endif
        return ESP_LOADER_SUCCESS;
    } else if (err == ESP_ERR_TIMEOUT) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_ERROR_FAIL;
    }
}


esp_loader_error_t loader_port_read(uint8_t *data, uint16_t size, uint32_t timeout)
{
    int read = uart_read_bytes(s_uart_port, data, size, pdMS_TO_TICKS(timeout));

    if (read < 0) {
        return ESP_LOADER_ERROR_FAIL;
    } else if (read < size) {
#ifdef SERIAL_FLASHER_DEBUG_TRACE
        transfer_debug_print(data, read, false);
#endif
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
#ifdef SERIAL_FLASHER_DEBUG_TRACE
        transfer_debug_print(data, read, false);
#endif
        return ESP_LOADER_SUCCESS;
    }
}


// Set GPIO0 LOW, then
// assert reset pin for 50 milliseconds.
void loader_port_enter_bootloader(void)
{
    gpio_set_level(s_gpio0_trigger_pin, 0);
    loader_port_reset_target();
    loader_port_delay_ms(SERIAL_FLASHER_BOOT_HOLD_TIME_MS);
    gpio_set_level(s_gpio0_trigger_pin, 1);
}


void loader_port_reset_target(void)
{
    gpio_set_level(s_reset_trigger_pin, 0);
    loader_port_delay_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
    gpio_set_level(s_reset_trigger_pin, 1);
}


void loader_port_delay_ms(uint32_t ms)
{
    usleep(ms * 1000);
}


void loader_port_start_timer(uint32_t ms)
{
    s_time_end = esp_timer_get_time() + ms * 1000;
}


uint32_t loader_port_remaining_time(void)
{
    int64_t remaining = (s_time_end - esp_timer_get_time()) / 1000;
    return (remaining > 0) ? (uint32_t)remaining : 0;
}


void loader_port_debug_print(const char *str)
{
    printf("DEBUG: %s\n", str);
}

esp_loader_error_t loader_port_change_transmission_rate(uint32_t baudrate)
{
    esp_err_t err = uart_set_baudrate(s_uart_port, baudrate);
    return (err == ESP_OK) ? ESP_LOADER_SUCCESS : ESP_LOADER_ERROR_FAIL;
}