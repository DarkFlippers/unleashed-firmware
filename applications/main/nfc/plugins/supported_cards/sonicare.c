// Parser for Philips Sonicare toothbrush heads.
// Made by @Sil333033
// Thanks to Cyrill Künzi for this research! https://kuenzi.dev/toothbrush/

#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

#define TAG "Sonicare"

typedef enum {
    SonicareHeadWhite,
    SonicareHeadBlack,
    SonicareHeadUnkown,
} SonicareHead;

static SonicareHead sonicare_get_head_type(const MfUltralightData* data) {
    // data.page[34].data got 4 bytes
    // 31:32:31:34 for black (not sure)
    // 31:31:31:31 for white (not sure)
    // the data should be in here based on the research, but i cant find it.
    // page 34 byte 0 is always 0x30 for the white brushes i have, so i guess thats white
    // TODO: Get a black brush and test this

    if(data->page[34].data[0] == 0x30) {
        return SonicareHeadWhite;
    } else {
        return SonicareHeadUnkown;
    }
}

static uint32_t sonicare_get_seconds_brushed(const MfUltralightData* data) {
    uint32_t seconds_brushed = 0;

    seconds_brushed += data->page[36].data[0];
    seconds_brushed += data->page[36].data[1] << 8;

    return seconds_brushed;
}

static bool sonicare_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfUltralightData* data = nfc_device_get_data(device, NfcProtocolMfUltralight);

    bool parsed = false;

    do {
        // Check for NDEF link match
        const char* test = "philips.com/nfcbrushheadtap";
        // Data is a array of arrays, cast to char array and compare
        if(strncmp(test, (const char*)&data->page[5].data[3], strlen(test)) != 0) {
            FURI_LOG_D(TAG, "Not a Philips Sonicare head");
            break;
        }

        const SonicareHead head_type = sonicare_get_head_type(data);
        const uint32_t seconds_brushed = sonicare_get_seconds_brushed(data);

        FuriString* head_type_str = furi_string_alloc();

        switch(head_type) {
        case SonicareHeadWhite:
            furi_string_printf(head_type_str, "White");
            break;
        case SonicareHeadBlack:
            furi_string_printf(head_type_str, "Black");
            break;
        case SonicareHeadUnkown:
        default:
            furi_string_printf(head_type_str, "Unknown");
            break;
        }

        furi_string_printf(
            parsed_data,
            "\e#Philips Sonicare head\nColor: %s\nTime brushed: %02.0f:%02.0f:%02ld\n",
            furi_string_get_cstr(head_type_str),
            floor(seconds_brushed / 3600),
            floor((seconds_brushed / 60) % 60),
            seconds_brushed % 60);

        furi_string_free(head_type_str);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin sonicare_plugin = {
    .protocol = NfcProtocolMfUltralight,
    .verify = NULL,
    .read = NULL,
    .parse = sonicare_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor sonicare_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &sonicare_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* sonicare_plugin_ep(void) {
    return &sonicare_plugin_descriptor;
}
