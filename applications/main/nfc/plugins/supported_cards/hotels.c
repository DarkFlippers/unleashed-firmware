#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>

#define TAG "Hotels"

static const uint64_t saflok_sector1_keya = 0x2A2C13CC242A;
static const uint64_t vingcard_sector2_keyb = 0x0000014B5C31;
static const uint64_t onity_sector1_keya = 0x8A19D40CF2B5;

bool hotels_verify(Nfc* nfc) {
    furi_assert(nfc);
    return false;
}

static bool hotels_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);
    return false;
}

static bool hotels_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;
    FuriString* name = furi_string_alloc_set_str("Unknown");

    do {
        const uint8_t verify_sector = 1;
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, verify_sector);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key == saflok_sector1_keya) {
            furi_string_set_str(name, "Saflok");
            parsed = true;
        }
        if(key == onity_sector1_keya) {
            furi_string_set_str(name, "Onity");
            parsed = true;
        }
        sec_tr = mf_classic_get_sector_trailer_by_sector(data, 2);
        key = bit_lib_bytes_to_num_be(sec_tr->key_b.data, 6);
        if(key == vingcard_sector2_keyb) {
            furi_string_set_str(name, "VingCard");
            parsed = true;
        }

        if(!parsed) break;
        furi_string_printf(parsed_data, "\e#Hotel\n%s", furi_string_get_cstr(name));
    } while(false);

    furi_string_free(name);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin hotels_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = hotels_verify,
    .read = hotels_read,
    .parse = hotels_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor hotels_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &hotels_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* hotels_plugin_ep(void) {
    return &hotels_plugin_descriptor;
}
