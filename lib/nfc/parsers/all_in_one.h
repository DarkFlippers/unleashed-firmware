#pragma once

#include "nfc_supported_card.h"

bool all_in_one_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool all_in_one_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool all_in_one_parser_parse(NfcDeviceData* dev_data);