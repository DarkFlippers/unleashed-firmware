/* myki.c - Parser for myki cards (Melbourne, Australia).
 *
 * Based on the code by Emily Trau (https://github.com/emilytrau)
 * Original pull request URL: https://github.com/flipperdevices/flipperzero-firmware/pull/2326
 * Reference: https://github.com/metrodroid/metrodroid/wiki/Myki
 */
#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <lib/nfc/protocols/mf_desfire/mf_desfire.h>

static const MfDesfireApplicationId myki_app_id = {.data = {0x00, 0x11, 0xf2}};
static const MfDesfireFileId myki_file_id = 0x0f;

static uint8_t myki_calculate_luhn(uint64_t number) {
    // https://en.wikipedia.org/wiki/Luhn_algorithm
    // Drop existing check digit to form payload
    uint64_t payload = number / 10;
    int sum = 0;
    int position = 0;

    while(payload > 0) {
        int digit = payload % 10;
        if(position % 2 == 0) {
            digit *= 2;
        }
        if(digit > 9) {
            digit = (digit / 10) + (digit % 10);
        }
        sum += digit;
        payload /= 10;
        position++;
    }

    return (10 - (sum % 10)) % 10;
}

static bool myki_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    bool parsed = false;

    do {
        const MfDesfireData* data = nfc_device_get_data(device, NfcProtocolMfDesfire);

        const MfDesfireApplication* app = mf_desfire_get_application(data, &myki_app_id);
        if(app == NULL) break;

        typedef struct {
            uint32_t top;
            uint32_t bottom;
        } MykiFile;

        const MfDesfireFileSettings* file_settings =
            mf_desfire_get_file_settings(app, &myki_file_id);

        if(file_settings == NULL || file_settings->type != MfDesfireFileTypeStandard ||
           file_settings->data.size < sizeof(MykiFile))
            break;

        const MfDesfireFileData* file_data = mf_desfire_get_file_data(app, &myki_file_id);
        if(file_data == NULL) break;

        const MykiFile* myki_file = simple_array_cget_data(file_data->data);

        // All myki card numbers are prefixed with "308425"
        if(myki_file->top != 308425UL) break;
        // Card numbers are always 15 digits in length
        if(myki_file->bottom < 10000000UL || myki_file->bottom >= 100000000UL) break;

        uint64_t card_number = myki_file->top * 1000000000ULL + myki_file->bottom * 10UL;
        // Stored card number doesn't include check digit
        card_number += myki_calculate_luhn(card_number);

        furi_string_set(parsed_data, "\e#myki\nNo.: ");

        // Stylise card number according to the physical card
        char card_string[20];
        snprintf(card_string, sizeof(card_string), "%llu", card_number);

        // Digit count in each space-separated group
        static const uint8_t digit_count[] = {1, 5, 4, 4, 1};

        for(uint32_t i = 0, k = 0; i < COUNT_OF(digit_count); k += digit_count[i++]) {
            for(uint32_t j = 0; j < digit_count[i]; ++j) {
                furi_string_push_back(parsed_data, card_string[j + k]);
            }
            furi_string_push_back(parsed_data, ' ');
        }

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin myki_plugin = {
    .protocol = NfcProtocolMfDesfire,
    .verify = NULL,
    .read = NULL,
    .parse = myki_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor myki_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &myki_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* myki_plugin_ep(void) {
    return &myki_plugin_descriptor;
}
