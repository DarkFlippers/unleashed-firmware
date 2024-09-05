/* itso.c - Parser for ITSO cards (United Kingdom). */
#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <lib/nfc/protocols/mf_desfire/mf_desfire.h>
#include <lib/toolbox/strint.h>

#include <applications/services/locale/locale.h>
#include <datetime/datetime.h>

static const MfDesfireApplicationId itso_app_id = {.data = {0x16, 0x02, 0xa0}};
static const MfDesfireFileId itso_file_id = 0x0f;

int64_t swap_int64(int64_t val) {
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

uint64_t swap_uint64(uint64_t val) {
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | (val >> 32);
}

static bool itso_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    bool parsed = false;

    do {
        const MfDesfireData* data = nfc_device_get_data(device, NfcProtocolMfDesfire);

        const MfDesfireApplication* app = mf_desfire_get_application(data, &itso_app_id);
        if(app == NULL) break;

        typedef struct {
            uint64_t part1;
            uint64_t part2;
            uint64_t part3;
            uint64_t part4;
        } ItsoFile;

        const MfDesfireFileSettings* file_settings =
            mf_desfire_get_file_settings(app, &itso_file_id);

        if(file_settings == NULL || file_settings->type != MfDesfireFileTypeStandard ||
           file_settings->data.size < sizeof(ItsoFile))
            break;

        const MfDesfireFileData* file_data = mf_desfire_get_file_data(app, &itso_file_id);
        if(file_data == NULL) break;

        const ItsoFile* itso_file = simple_array_cget_data(file_data->data);

        uint64_t x1 = swap_uint64(itso_file->part1);
        uint64_t x2 = swap_uint64(itso_file->part2);

        char cardBuff[32];
        char dateBuff[18];

        snprintf(cardBuff, sizeof(cardBuff), "%llx%llx", x1, x2);
        snprintf(dateBuff, sizeof(dateBuff), "%llx", x2);

        char* cardp = cardBuff + 4;
        cardp[18] = '\0';

        // All itso card numbers are prefixed with "633597"
        if(strncmp(cardp, "633597", 6) != 0) break;

        char* datep = dateBuff + 12;
        dateBuff[17] = '\0';

        // DateStamp is defined in BS EN 1545 - Days passed since 01/01/1997
        uint32_t dateStamp;
        if(strint_to_uint32(datep, NULL, &dateStamp, 16) != StrintParseNoError) {
            return false;
        }
        uint32_t unixTimestamp = dateStamp * 24 * 60 * 60 + 852076800U;

        furi_string_set(parsed_data, "\e#ITSO Card\n");

        // Digit count in each space-separated group
        static const uint8_t digit_count[] = {6, 4, 4, 4};

        for(uint32_t i = 0, k = 0; i < COUNT_OF(digit_count); k += digit_count[i++]) {
            for(uint32_t j = 0; j < digit_count[i]; ++j) {
                furi_string_push_back(parsed_data, cardp[j + k]);
            }
            furi_string_push_back(parsed_data, ' ');
        }

        DateTime timestamp = {0};
        datetime_timestamp_to_datetime(unixTimestamp, &timestamp);
        FuriString* timestamp_str = furi_string_alloc();
        locale_format_date(timestamp_str, &timestamp, locale_get_date_format(), "-");

        furi_string_cat(parsed_data, "\nExpiry: ");
        furi_string_cat(parsed_data, timestamp_str);

        furi_string_free(timestamp_str);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin itso_plugin = {
    .protocol = NfcProtocolMfDesfire,
    .verify = NULL,
    .read = NULL,
    .parse = itso_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor itso_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &itso_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* itso_plugin_ep(void) {
    return &itso_plugin_descriptor;
}
