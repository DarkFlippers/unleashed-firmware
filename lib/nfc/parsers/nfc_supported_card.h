#pragma once

#include <furi_hal_nfc.h>
#include "../nfc_worker.h"
#include "../nfc_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NfcSupportedCardTypePlantain,
    NfcSupportedCardTypeTroika,
    NfcSupportedCardTypePlantain4K,
    NfcSupportedCardTypeTroika4K,
    NfcSupportedCardTypeTwoCities,
    NfcSupportedCardTypeAllInOne,
    NfcSupportedCardTypeOpal,

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

// stub_parser_verify_read does nothing, and always reports that it does not
// support the card. This is needed for DESFire card parsers which can't
// provide keys, and only use NfcSupportedCard->parse.
bool stub_parser_verify_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx);

#ifdef __cplusplus
}
#endif