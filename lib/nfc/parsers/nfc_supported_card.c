#include "nfc_supported_card.h"

#include "troyka_parser.h"

NfcSupportedCard nfc_supported_card[NfcSupportedCardTypeEnd] = {
    [NfcSupportedCardTypeTroyka] =
        {
            .protocol = NfcDeviceProtocolMifareClassic,
            .verify = troyka_parser_verify,
            .read = troyka_parser_read,
            .parse = troyka_parser_parse,
        },
};

bool nfc_supported_card_verify_and_parse(NfcDeviceData* dev_data) {
    furi_assert(dev_data);

    bool card_parsed = false;
    for(size_t i = 0; i < COUNT_OF(nfc_supported_card); i++) {
        if(nfc_supported_card[i].parse(dev_data)) {
            card_parsed = true;
            break;
        }
    }

    return card_parsed;
}
