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

#include "protocol.h"
#include "protocol_prv.h"
#include "esp_loader_io.h"
#include "slip.h"
#include <stddef.h>
#include <string.h>

static esp_loader_error_t check_response(command_t cmd, uint32_t *reg_value, void* resp, uint32_t resp_size);

esp_loader_error_t loader_initialize_conn(esp_loader_connect_args_t *connect_args) {
    esp_loader_error_t err;
    int32_t trials = connect_args->trials;

    do {
        loader_port_start_timer(connect_args->sync_timeout);
        err = loader_sync_cmd();
        if (err == ESP_LOADER_ERROR_TIMEOUT) {
            if (--trials == 0) {
                return ESP_LOADER_ERROR_TIMEOUT;
            }
            loader_port_delay_ms(100);
        } else if (err != ESP_LOADER_SUCCESS) {
            return err;
        }
    } while (err != ESP_LOADER_SUCCESS);

    return err;
}

esp_loader_error_t send_cmd(const void *cmd_data, uint32_t size, uint32_t *reg_value)
{
    response_t response;
    command_t command = ((const command_common_t *)cmd_data)->command;

    RETURN_ON_ERROR( SLIP_send_delimiter() );
    RETURN_ON_ERROR( SLIP_send((const uint8_t *)cmd_data, size) );
    RETURN_ON_ERROR( SLIP_send_delimiter() );

    return check_response(command, reg_value, &response, sizeof(response));
}


esp_loader_error_t send_cmd_with_data(const void *cmd_data, size_t cmd_size,
                                      const void *data, size_t data_size)
{
    response_t response;
    command_t command = ((const command_common_t *)cmd_data)->command;

    RETURN_ON_ERROR( SLIP_send_delimiter() );
    RETURN_ON_ERROR( SLIP_send((const uint8_t *)cmd_data, cmd_size) );
    RETURN_ON_ERROR( SLIP_send(data, data_size) );
    RETURN_ON_ERROR( SLIP_send_delimiter() );

    return check_response(command, NULL, &response, sizeof(response));
}


esp_loader_error_t send_cmd_md5(const void *cmd_data, size_t cmd_size, uint8_t md5_out[MD5_SIZE])
{
    rom_md5_response_t response;
    command_t command = ((const command_common_t *)cmd_data)->command;

    RETURN_ON_ERROR( SLIP_send_delimiter() );
    RETURN_ON_ERROR( SLIP_send((const uint8_t *)cmd_data, cmd_size) );
    RETURN_ON_ERROR( SLIP_send_delimiter() );

    RETURN_ON_ERROR( check_response(command, NULL, &response, sizeof(response)) );

    memcpy(md5_out, response.md5, MD5_SIZE);

    return ESP_LOADER_SUCCESS;
}


static esp_loader_error_t check_response(command_t cmd, uint32_t *reg_value, void* resp, uint32_t resp_size)
{
    esp_loader_error_t err;
    common_response_t *response = (common_response_t *)resp;

    do {
        err = SLIP_receive_packet(resp, resp_size);
        if (err != ESP_LOADER_SUCCESS) {
            return err;
        }
    } while ((response->direction != READ_DIRECTION) || (response->command != cmd));

    response_status_t *status = (response_status_t *)((uint8_t *)resp + resp_size - sizeof(response_status_t));

    if (status->failed) {
        log_loader_internal_error(status->error);
        return ESP_LOADER_ERROR_INVALID_RESPONSE;
    }

    if (reg_value != NULL) {
        *reg_value = response->value;
    }

    return ESP_LOADER_SUCCESS;
}
