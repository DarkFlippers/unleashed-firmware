#include "nfc_supported_card.h"

#include "plantain_parser.h"
#include "troika_parser.h"
#include "plantain_4k_parser.h"
#include "troika_4k_parser.h"
#include "two_cities.h"
#include "all_in_one.h"

NfcSupportedCard nfc_supported_card[NfcSupportedCardTypeEnd] = {
    [NfcSupportedCardTypePlantain] =
        {
            .protocol = NfcDeviceProtocolMifareClassic,
            .verify = plantain_parser_verify,
            .read = plantain_parser_read,
            .parse = plantain_parser_parse,
        },
    [NfcSupportedCardTypeTroika] =
        {
            .protocol = NfcDeviceProtocolMifareClassic,
            .verify = troika_parser_verify,
            .read = troika_parser_read,
            .parse = troika_parser_parse,
        },
    [NfcSupportedCardTypePlantain4K] =
        {
            .protocol = NfcDeviceProtocolMifareClassic,
            .verify = plantain_4k_parser_verify,
            .read = plantain_4k_parser_read,
            .parse = plantain_4k_parser_parse,
        },
    [NfcSupportedCardTypeTroika4K] =
        {
            .protocol = NfcDeviceProtocolMifareClassic,
            .verify = troika_4k_parser_verify,
            .read = troika_4k_parser_read,
            .parse = troika_4k_parser_parse,
        },
    [NfcSupportedCardTypeTwoCities] =
        {
            .protocol = NfcDeviceProtocolMifareClassic,
            .verify = two_cities_parser_verify,
            .read = two_cities_parser_read,
            .parse = two_cities_parser_parse,
        },
    [NfcSupportedCardTypeAllInOne] =
        {
            .protocol = NfcDeviceProtocolMifareUl,
            .verify = all_in_one_parser_verify,
            .read = all_in_one_parser_read,
            .parse = all_in_one_parser_parse,
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
