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

#include "slip.h"
#include "esp_loader_io.h"

static const uint8_t DELIMITER = 0xC0;
static const uint8_t C0_REPLACEMENT[2] = {0xDB, 0xDC};
static const uint8_t DB_REPLACEMENT[2] = {0xDB, 0xDD};

static inline esp_loader_error_t peripheral_read(uint8_t *buff, const size_t size)
{
    return loader_port_read(buff, size, loader_port_remaining_time());
}

static inline esp_loader_error_t peripheral_write(const uint8_t *buff, const size_t size)
{
    return loader_port_write(buff, size, loader_port_remaining_time());
}

esp_loader_error_t SLIP_receive_data(uint8_t *buff, const size_t size)
{
    uint8_t ch;

    for (uint32_t i = 0; i < size; i++) {
        RETURN_ON_ERROR( peripheral_read(&ch, 1) );

        if (ch == 0xDB) {
            RETURN_ON_ERROR( peripheral_read(&ch, 1) );
            if (ch == 0xDC) {
                buff[i] = 0xC0;
            } else if (ch == 0xDD) {
                buff[i] = 0xDB;
            } else {
                return ESP_LOADER_ERROR_INVALID_RESPONSE;
            }
        } else {
            buff[i] = ch;
        }
    }

    return ESP_LOADER_SUCCESS;
}


esp_loader_error_t SLIP_receive_packet(uint8_t *buff, const size_t size)
{
    uint8_t ch;

    // Wait for delimiter
    do {
        RETURN_ON_ERROR( peripheral_read(&ch, 1) );
    } while (ch != DELIMITER);

    // Workaround: bootloader sends two dummy(0xC0) bytes after response when baud rate is changed.
    do {
        RETURN_ON_ERROR( peripheral_read(&ch, 1) );
    } while (ch == DELIMITER);

    buff[0] = ch;

    RETURN_ON_ERROR( SLIP_receive_data(&buff[1], size - 1) );

    // Wait for delimiter
    do {
        RETURN_ON_ERROR( peripheral_read(&ch, 1) );
    } while (ch != DELIMITER);

    return ESP_LOADER_SUCCESS;
}


esp_loader_error_t SLIP_send(const uint8_t *data, const size_t size)
{
    uint32_t to_write = 0;  // Bytes ready to write as they are
    uint32_t written = 0;   // Bytes already written

    for (uint32_t i = 0; i < size; i++) {
        if (data[i] != 0xC0 && data[i] != 0xDB) {
            to_write++; // Queue this byte for writing
            continue;
        }

        // We have a byte that needs encoding, write the queue first
        if (to_write > 0) {
            RETURN_ON_ERROR( peripheral_write(&data[written], to_write) );
        }

        // Write the encoded byte
        if (data[i] == 0xC0) {
            RETURN_ON_ERROR( peripheral_write(C0_REPLACEMENT, 2) );
        } else {
            RETURN_ON_ERROR( peripheral_write(DB_REPLACEMENT, 2) );
        }

        // Update to start again after the encoded byte
        written = i + 1;
        to_write = 0;
    }

    // Write the rest of the bytes that didn't need encoding
    if (to_write > 0) {
        RETURN_ON_ERROR( peripheral_write(&data[written], to_write) );
    }

    return ESP_LOADER_SUCCESS;
}


esp_loader_error_t SLIP_send_delimiter(void)
{
    return peripheral_write(&DELIMITER, 1);
}
