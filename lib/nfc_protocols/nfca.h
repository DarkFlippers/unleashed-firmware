#pragma once

#include <stdint.h>
#include <stdbool.h>

uint16_t nfca_get_crc16(uint8_t* buff, uint16_t len);

void nfca_append_crc16(uint8_t* buff, uint16_t len);

bool nfca_emulation_handler(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len);
