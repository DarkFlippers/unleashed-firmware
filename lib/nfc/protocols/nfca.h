#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <lib/digital_signal/digital_signal.h>

#define NFCA_CMD_RATS (0xE0U)
#define NFCA_CMD_HALT (0x50U)

typedef struct {
    DigitalSignal* one;
    DigitalSignal* zero;
    DigitalSignal* tx_signal;
} NfcaSignal;

uint16_t nfca_get_crc16(uint8_t* buff, uint16_t len);

void nfca_append_crc16(uint8_t* buff, uint16_t len);

bool nfca_emulation_handler(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len);

NfcaSignal* nfca_signal_alloc();

void nfca_signal_free(NfcaSignal* nfca_signal);

void nfca_signal_encode(NfcaSignal* nfca_signal, uint8_t* data, uint16_t bits, uint8_t* parity);
