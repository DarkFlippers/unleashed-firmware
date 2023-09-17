#pragma once

#include "nfc_supported_card.h"

bool social_card_moscow_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool social_card_moscow_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

bool social_card_moscow_parser_parse(NfcDeviceData* dev_data);