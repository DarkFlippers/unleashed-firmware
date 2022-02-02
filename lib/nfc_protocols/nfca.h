#pragma once

#include <stdint.h>
#include <stdbool.h>

bool nfca_emulation_handler(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len);
