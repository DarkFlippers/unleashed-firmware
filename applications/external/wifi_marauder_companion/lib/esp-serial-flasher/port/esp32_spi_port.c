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

#include "esp32_spi_port.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_idf_version.h"
#include <unistd.h>

// #define SERIAL_DEBUG_ENABLE

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
#define DMA_CHAN SPI_DMA_CH_AUTO
#else
#define DMA_CHAN 1
#endif

#define WORD_ALIGNED(ptr) ((size_t)ptr % sizeof(size_t) == 0)

#ifdef SERIAL_DEBUG_ENABLE

static void dec_to_hex_str(const uint8_t dec, uint8_t hex_str[3])
{
    static const uint8_t dec_to_hex[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    hex_str[0] = dec_to_hex[dec >> 4];
    hex_str[1] = dec_to_hex[dec & 0xF];
    hex_str[2] = '\0';
}

static void serial_debug_print(const uint8_t *data, uint16_t size, bool write)
{
    static bool write_prev = false;
    uint8_t hex_str[3];

    if(write_prev != write) {
        write_prev = write;
        printf("\n--- %s ---\n", write ? "WRITE" : "READ");
    }

    for(uint32_t i = 0; i < size; i++) {
        dec_to_hex_str(data[i], hex_str);
        printf("%s ", hex_str);
    }
}

#else
static void serial_debug_print(const uint8_t *data, uint16_t size, bool write) { }
#endif

static spi_host_device_t s_spi_bus;
static spi_bus_config_t s_spi_config;
static spi_device_handle_t s_device_h;
static spi_device_interface_config_t s_device_config;
static int64_t s_time_end;
static uint32_t s_reset_trigger_pin;
static uint32_t s_strap_bit0_pin;
static uint32_t s_strap_bit1_pin;
static uint32_t s_strap_bit2_pin;
static uint32_t s_strap_bit3_pin;
static uint32_t s_spi_cs_pin;

esp_loader_error_t loader_port_esp32_spi_init(const loader_esp32_spi_config_t *config)
{
    /* Initialize the global static variables*/
    s_spi_bus = config->spi_bus;
    s_reset_trigger_pin = config->reset_trigger_pin;
    s_strap_bit0_pin = config->strap_bit0_pin;
    s_strap_bit1_pin = config->strap_bit1_pin;
    s_strap_bit2_pin = config->strap_bit2_pin;
    s_strap_bit3_pin = config->strap_bit3_pin;
    s_spi_cs_pin = config->spi_cs_pin;

    /* Configure and initialize the SPI bus*/
    s_spi_config.mosi_io_num = config->spi_mosi_pin;
    s_spi_config.miso_io_num = config->spi_miso_pin;
    s_spi_config.sclk_io_num = config->spi_clk_pin;
    s_spi_config.quadwp_io_num = config->spi_quadwp_pin;
    s_spi_config.quadhd_io_num = config->spi_quadhd_pin;
    s_spi_config.max_transfer_sz = 4096 * 4;

    if (spi_bus_initialize(s_spi_bus, &s_spi_config, DMA_CHAN) != ESP_OK) {
        return ESP_LOADER_ERROR_FAIL;
    }

    /* Configure and add the device */
    s_device_config.clock_speed_hz = config->frequency;
    s_device_config.spics_io_num = -1; /* We're using the chip select pin as GPIO as we need to
                                          chain multiple transactions with CS pulled down */
    s_device_config.flags = SPI_DEVICE_HALFDUPLEX;
    s_device_config.queue_size = 16;

    if (spi_bus_add_device(s_spi_bus, &s_device_config, &s_device_h) != ESP_OK) {
        return ESP_LOADER_ERROR_FAIL;
    }

    /* Initialize the pins except for the strapping ones */
    gpio_reset_pin(s_reset_trigger_pin);
    gpio_set_pull_mode(s_reset_trigger_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_reset_trigger_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_reset_trigger_pin, 1);

    gpio_reset_pin(s_spi_cs_pin);
    gpio_set_pull_mode(s_spi_cs_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_spi_cs_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_spi_cs_pin, 1);

    return ESP_LOADER_SUCCESS;
}


void loader_port_esp32_spi_deinit(void)
{
    gpio_reset_pin(s_reset_trigger_pin);
    gpio_reset_pin(s_spi_cs_pin);
    spi_bus_remove_device(s_device_h);
    spi_bus_free(s_spi_bus);
}


void loader_port_spi_set_cs(const uint32_t level) {
    gpio_set_level(s_spi_cs_pin, level);
}


esp_loader_error_t loader_port_write(const uint8_t *data, const uint16_t size, const uint32_t timeout)
{
    /* Due to the fact that the SPI driver uses DMA for larger transfers,
       and the DMA requirements, the buffer must be word aligned */
    if (data == NULL || !WORD_ALIGNED(data)) {
        return ESP_LOADER_ERROR_INVALID_PARAM;
    }

    serial_debug_print(data, size, true);

    spi_transaction_t transaction = {
        .tx_buffer = data,
        .rx_buffer = NULL,
        .length = size * 8U,
        .rxlength = 0,
    };

    esp_err_t err = spi_device_transmit(s_device_h, &transaction);

    if (err == ESP_OK) {
        serial_debug_print(data, size, false);
        return ESP_LOADER_SUCCESS;
    } else if (err == ESP_ERR_TIMEOUT) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_ERROR_FAIL;
    }
}


esp_loader_error_t loader_port_read(uint8_t *data, const uint16_t size, const uint32_t timeout)
{
    /* Due to the fact that the SPI driver uses DMA for larger transfers,
       and the DMA requirements, the buffer must be word aligned */
    if (data == NULL || !WORD_ALIGNED(data)) {
        return ESP_LOADER_ERROR_INVALID_PARAM;
    }

    serial_debug_print(data, size, true);

    spi_transaction_t transaction = {
        .tx_buffer = NULL,
        .rx_buffer = data,
        .rxlength = size * 8,
    };

    esp_err_t err = spi_device_transmit(s_device_h, &transaction);

    if (err == ESP_OK) {
        serial_debug_print(data, size, false);
        return ESP_LOADER_SUCCESS;
    } else if (err == ESP_ERR_TIMEOUT) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_ERROR_FAIL;
    }
}


void loader_port_enter_bootloader(void)
{
    /*
        We have to initialize the GPIO pins for the target strapping pins here,
        as they may overlap with target SPI pins.
        For instance in the case of ESP32C3 MISO and strapping bit 0 pins overlap.
    */
    spi_bus_remove_device(s_device_h);
    spi_bus_free(s_spi_bus);

    gpio_reset_pin(s_strap_bit0_pin);
    gpio_set_pull_mode(s_strap_bit0_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_strap_bit0_pin, GPIO_MODE_OUTPUT);

    gpio_reset_pin(s_strap_bit1_pin);
    gpio_set_pull_mode(s_strap_bit1_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_strap_bit1_pin, GPIO_MODE_OUTPUT);

    gpio_reset_pin(s_strap_bit2_pin);
    gpio_set_pull_mode(s_strap_bit2_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_strap_bit2_pin, GPIO_MODE_OUTPUT);

    gpio_reset_pin(s_strap_bit3_pin);
    gpio_set_pull_mode(s_strap_bit3_pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(s_strap_bit3_pin, GPIO_MODE_OUTPUT);

    /* Set the strapping pins and perform the reset sequence */
    gpio_set_level(s_strap_bit0_pin, 1);
    gpio_set_level(s_strap_bit1_pin, 0);
    gpio_set_level(s_strap_bit2_pin, 0);
    gpio_set_level(s_strap_bit3_pin, 0);
    loader_port_reset_target();
    loader_port_delay_ms(SERIAL_FLASHER_BOOT_HOLD_TIME_MS);
    gpio_set_level(s_strap_bit3_pin, 1);
    gpio_set_level(s_strap_bit0_pin, 0);

    /* Disable the strapping pins so they can be used by the slave later */
    gpio_reset_pin(s_strap_bit0_pin);
    gpio_reset_pin(s_strap_bit1_pin);
    gpio_reset_pin(s_strap_bit2_pin);
    gpio_reset_pin(s_strap_bit3_pin);

    /* Restore the SPI bus pins */
    spi_bus_initialize(s_spi_bus, &s_spi_config, DMA_CHAN);
    spi_bus_add_device(s_spi_bus, &s_device_config, &s_device_h);
}


void loader_port_reset_target(void)
{
    gpio_set_level(s_reset_trigger_pin, 0);
    loader_port_delay_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
    gpio_set_level(s_reset_trigger_pin, 1);
}


void loader_port_delay_ms(const uint32_t ms)
{
    usleep(ms * 1000);
}


void loader_port_start_timer(const uint32_t ms)
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


esp_loader_error_t loader_port_change_transmission_rate(const uint32_t frequency)
{
    if (spi_bus_remove_device(s_device_h) != ESP_OK) {
        return ESP_LOADER_ERROR_FAIL;
    }

    uint32_t old_frequency = s_device_config.clock_speed_hz;
    s_device_config.clock_speed_hz = frequency;

    if (spi_bus_add_device(s_spi_bus, &s_device_config, &s_device_h) != ESP_OK) {
        s_device_config.clock_speed_hz = old_frequency;
        return ESP_LOADER_ERROR_FAIL;
    }

    return ESP_LOADER_SUCCESS;
}
