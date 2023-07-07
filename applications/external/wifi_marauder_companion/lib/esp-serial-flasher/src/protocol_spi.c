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
#include <stddef.h>
#include <assert.h>

typedef struct __attribute__((packed)) {
    uint8_t cmd;
    uint8_t addr;
    uint8_t dummy;
} transaction_preamble_t;

typedef enum {
    TRANS_CMD_WRBUF = 0x01,
    TRANS_CMD_RDBUF = 0x02,
    TRANS_CMD_WRDMA = 0x03,
    TRANS_CMD_RDDMA = 0x04,
    TRANS_CMD_SEG_DONE = 0x05,
    TRANS_CMD_ENQPI = 0x06,
    TRANS_CMD_WR_DONE = 0x07,
    TRANS_CMD_CMD8 = 0x08,
    TRANS_CMD_CMD9 = 0x09,
    TRANS_CMD_CMDA = 0x0A,
    TRANS_CMD_EXQPI = 0xDD,
} transaction_cmd_t;

/* Slave protocol registers */
typedef enum {
    SLAVE_REGISTER_VER = 0,
    SLAVE_REGISTER_RXSTA = 4,
    SLAVE_REGISTER_TXSTA = 8,
    SLAVE_REGISTER_CMD = 12,
} slave_register_addr_t;

#define SLAVE_STA_TOGGLE_BIT (0x01U << 0)
#define SLAVE_STA_INIT_BIT (0x01U << 1)
#define SLAVE_STA_BUF_LENGTH_POS 2U

typedef enum {
    SLAVE_STATE_INIT = SLAVE_STA_TOGGLE_BIT | SLAVE_STA_INIT_BIT,
    SLAVE_STATE_FIRST_PACKET = SLAVE_STA_INIT_BIT,
} slave_state_t;

typedef enum {
    /* Target to host */
    SLAVE_CMD_IDLE = 0xAA,
    SLAVE_CMD_READY = 0xA5,
    /* Host to target */
    SLAVE_CMD_REBOOT = 0xFE,
    SLAVE_CMD_COMM_REINIT = 0x5A,
    SLAVE_CMD_DONE = 0x55,
} slave_cmd_t;

static uint8_t s_slave_seq_tx;
static uint8_t s_slave_seq_rx;

static esp_loader_error_t write_slave_reg(const uint8_t *data, const uint32_t addr,
                                                 const uint8_t size);
static esp_loader_error_t read_slave_reg(uint8_t *out_data, const uint32_t addr,
                                                const uint8_t size);
static esp_loader_error_t handle_slave_state(const uint32_t status_reg_addr, uint8_t *seq_state,
                                             bool *slave_ready, uint32_t *buf_size);
static esp_loader_error_t check_response(command_t cmd, uint32_t *reg_value);

esp_loader_error_t loader_initialize_conn(esp_loader_connect_args_t *connect_args)
{
    for (uint8_t trial = 0; trial < connect_args->trials; trial++) {
        uint8_t slave_ready_flag;
        RETURN_ON_ERROR(read_slave_reg(&slave_ready_flag, SLAVE_REGISTER_CMD,
                                              sizeof(slave_ready_flag)));

        if (slave_ready_flag != SLAVE_CMD_IDLE) {
            loader_port_debug_print("Waiting for Slave to be idle...\n");
            loader_port_delay_ms(100);
        } else {
            break;
        }
    }

    const uint8_t reg_val = SLAVE_CMD_READY;
    RETURN_ON_ERROR(write_slave_reg(&reg_val, SLAVE_REGISTER_CMD, sizeof(reg_val)));

    for (uint8_t trial = 0; trial < connect_args->trials; trial++) {
        uint8_t slave_ready_flag;
        RETURN_ON_ERROR(read_slave_reg(&slave_ready_flag, SLAVE_REGISTER_CMD,
                                              sizeof(slave_ready_flag)));

        if (slave_ready_flag != SLAVE_CMD_READY) {
            loader_port_debug_print("Waiting for Slave to be ready...\n");
            loader_port_delay_ms(100);
        } else {
            break;
        }
    }

    return ESP_LOADER_SUCCESS;
}


esp_loader_error_t send_cmd(const void *cmd_data, uint32_t size, uint32_t *reg_value)
{
    command_t command = ((const command_common_t *)cmd_data)->command;

    uint32_t buf_size;
    bool slave_ready = false;
    while (!slave_ready) {
        RETURN_ON_ERROR(handle_slave_state(SLAVE_REGISTER_RXSTA, &s_slave_seq_rx, &slave_ready,
                                           &buf_size));
    }

    if (size > buf_size) {
        return ESP_LOADER_ERROR_INVALID_PARAM;
    }

    /* Start and write the command */
    transaction_preamble_t preamble = {.cmd = TRANS_CMD_WRDMA};

    loader_port_spi_set_cs(0);
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)cmd_data, size,
                                      loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    /* Terminate the write */
    loader_port_spi_set_cs(0);
    preamble.cmd = TRANS_CMD_WR_DONE;
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    return check_response(command, reg_value);
}


esp_loader_error_t send_cmd_with_data(const void *cmd_data, size_t cmd_size,
                                      const void *data, size_t data_size)
{
    uint32_t buf_size;
    bool slave_ready = false;
    while (!slave_ready) {
        RETURN_ON_ERROR(handle_slave_state(SLAVE_REGISTER_RXSTA, &s_slave_seq_rx, &slave_ready,
                                           &buf_size));
    }

    if (cmd_size + data_size > buf_size) {
        return ESP_LOADER_ERROR_INVALID_PARAM;
    }

    /* Start and write the command and the data */
    transaction_preamble_t preamble = {.cmd = TRANS_CMD_WRDMA};

    loader_port_spi_set_cs(0);
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)cmd_data, cmd_size,
                                      loader_port_remaining_time()));
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)data, data_size,
                                      loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    /* Terminate the write */
    loader_port_spi_set_cs(0);
    preamble.cmd = TRANS_CMD_WR_DONE;
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    command_t command = ((const command_common_t *)cmd_data)->command;
    return check_response(command, NULL);
}


static esp_loader_error_t read_slave_reg(uint8_t *out_data, const uint32_t addr,
                                         const uint8_t size)
{
    transaction_preamble_t preamble = {
        .cmd = TRANS_CMD_RDBUF,
        .addr = addr,
    };

    loader_port_spi_set_cs(0);
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    RETURN_ON_ERROR(loader_port_read(out_data, size, loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    return ESP_LOADER_SUCCESS;
}


static esp_loader_error_t write_slave_reg(const uint8_t *data, const uint32_t addr,
                                          const uint8_t size)
{
    transaction_preamble_t preamble = {
        .cmd = TRANS_CMD_WRBUF,
        .addr = addr,
    };

    loader_port_spi_set_cs(0);
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    RETURN_ON_ERROR(loader_port_write(data, size, loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    return ESP_LOADER_SUCCESS;
}


static esp_loader_error_t handle_slave_state(const uint32_t status_reg_addr, uint8_t *seq_state,
                                             bool *slave_ready, uint32_t *buf_size)
{
    uint32_t status_reg;
    RETURN_ON_ERROR(read_slave_reg((uint8_t *)&status_reg, status_reg_addr,
                                   sizeof(status_reg)));
    const slave_state_t state = status_reg & (SLAVE_STA_TOGGLE_BIT | SLAVE_STA_INIT_BIT);

    switch(state) {
        case SLAVE_STATE_INIT: {
            const uint32_t initial = 0U;
            RETURN_ON_ERROR(write_slave_reg((uint8_t *)&initial, status_reg_addr, sizeof(initial)));
            break;
        }

        case SLAVE_STATE_FIRST_PACKET: {
            *seq_state = state & SLAVE_STA_TOGGLE_BIT;
            *buf_size = status_reg >> SLAVE_STA_BUF_LENGTH_POS;
            *slave_ready = true;
            break;
        }

        default: {
            const uint8_t new_seq = state & SLAVE_STA_TOGGLE_BIT;
            if (new_seq != *seq_state) {
                *seq_state = new_seq;
                *buf_size = status_reg >> SLAVE_STA_BUF_LENGTH_POS;
                *slave_ready = true;
            }
            break;
        }
    }

    return ESP_LOADER_SUCCESS;
}


static esp_loader_error_t check_response(command_t cmd, uint32_t *reg_value)
{
    response_t resp;

    uint32_t buf_size;
    bool slave_ready = false;
    while (!slave_ready) {
        RETURN_ON_ERROR(handle_slave_state(SLAVE_REGISTER_TXSTA, &s_slave_seq_tx, &slave_ready,
                                           &buf_size));
    }

    if (sizeof(resp) > buf_size) {
        return ESP_LOADER_ERROR_INVALID_PARAM;
    }

    transaction_preamble_t preamble = {
        .cmd = TRANS_CMD_RDDMA,
    };

    loader_port_spi_set_cs(0);
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    RETURN_ON_ERROR(loader_port_read((uint8_t *)&resp, sizeof(resp),
                                     loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    /* Terminate the read */
    loader_port_spi_set_cs(0);
    preamble.cmd = TRANS_CMD_CMD8;
    RETURN_ON_ERROR(loader_port_write((const uint8_t *)&preamble, sizeof(preamble),
                                      loader_port_remaining_time()));
    loader_port_spi_set_cs(1);

    common_response_t *common = (common_response_t *)&resp;
    if ((common->direction != READ_DIRECTION) || (common->command != cmd)) {
        return ESP_LOADER_ERROR_INVALID_RESPONSE;
    }

    response_status_t *status =
        (response_status_t *)((uint8_t *)&resp + sizeof(resp) - sizeof(response_status_t));
    if (status->failed) {
        log_loader_internal_error(status->error);
        return ESP_LOADER_ERROR_INVALID_RESPONSE;
    }

    if (reg_value != NULL) {
        *reg_value = common->value;
    }

    return ESP_LOADER_SUCCESS;
}
