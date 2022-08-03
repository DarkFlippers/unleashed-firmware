#include "nfc_supported_card.h"

#include "troyka_parser.h"

NfcSupportedCard nfc_supported_card[NfcSupportedCardTypeEnd] = {
    [NfcSupportedCardTypeTroyka] = {
        .protocol = NfcDeviceProtocolMifareClassic,
        .verify = troyka_parser_verify,
        .read = troyka_parser_read,
        .parse = troyka_parser_parse,
    },
};
