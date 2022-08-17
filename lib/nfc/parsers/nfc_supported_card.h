#pragma once

#include <furi_hal_nfc.h>
#include "../nfc_worker.h"
#include "../nfc_device.h"

#include <m-string.h>

typedef enum {
    NfcSupportedCardTypeTroyka,

    NfcSupportedCardTypeEnd,
} NfcSupportedCardType;

typedef bool (*NfcSupportedCardVerify)(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

typedef bool (*NfcSupportedCardRead)(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

typedef bool (*NfcSupportedCardParse)(NfcDeviceData* dev_data);

typedef struct {
    NfcProtocol protocol;
    NfcSupportedCardVerify verify;
    NfcSupportedCardRead read;
    NfcSupportedCardParse parse;
} NfcSupportedCard;

extern NfcSupportedCard nfc_supported_card[NfcSupportedCardTypeEnd];

bool nfc_supported_card_verify_and_parse(NfcDeviceData* dev_data);
