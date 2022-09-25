#pragma once

#include "nfc_supported_card.h"

bool troika_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool troika_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool troika_parser_parse(NfcDeviceData* dev_data);
