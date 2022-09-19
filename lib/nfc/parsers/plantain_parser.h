#pragma once

#include "nfc_supported_card.h"

bool plantain_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool plantain_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool plantain_parser_parse(NfcDeviceData* dev_data);

void string_push_uint64(uint64_t input, string_t output);

uint8_t plantain_calculate_luhn(uint64_t number);
