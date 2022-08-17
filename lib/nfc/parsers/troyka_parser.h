#pragma once

#include "nfc_supported_card.h"

bool troyka_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool troyka_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool troyka_parser_parse(NfcDeviceData* dev_data);
