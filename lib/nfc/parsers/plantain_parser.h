#pragma once

#include "nfc_supported_card.h"

bool plantain_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool plantain_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool plantain_parser_parse(NfcDeviceData* dev_data);

uint8_t plantain_calculate_luhn(uint64_t number);
