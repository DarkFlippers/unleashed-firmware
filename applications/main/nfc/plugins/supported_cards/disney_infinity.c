#include <mbedtls/sha1.h>
#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>

#define TAG     "DisneyInfinity"
#define UID_LEN 7

// Derived from https://nfc.toys/#new-interoperability-for-infinity
static uint8_t seed[38] = {0x0A, 0x14, 0xFD, 0x05, 0x07, 0xFF, 0x4B, 0xCD, 0x02, 0x6B,
                           0xA8, 0x3F, 0x0A, 0x3B, 0x89, 0xA9, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x28, 0x63, 0x29, 0x20, 0x44, 0x69, 0x73,
                           0x6E, 0x65, 0x79, 0x20, 0x32, 0x30, 0x31, 0x33};

void di_key(const uint8_t* uid, MfClassicKey* key) {
    uint8_t hash[20];
    memcpy(seed + 16, uid, UID_LEN);
    mbedtls_sha1(seed, sizeof(seed), hash);
    key->data[0] = hash[3];
    key->data[1] = hash[2];
    key->data[2] = hash[1];
    key->data[3] = hash[0];
    key->data[4] = hash[7];
    key->data[5] = hash[6];
}

static bool disney_infinity_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);
    size_t* uid_len = 0;
    bool is_read = false;
    MfClassicData* data = mf_classic_alloc();

    nfc_device_copy_data(device, NfcProtocolMfClassic, data);
    const uint8_t* uid_bytes = mf_classic_get_uid(data, uid_len);
    MfClassicDeviceKeys keys = {};

    do {
        MfClassicType type = MfClassicTypeMini;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            di_key(uid_bytes, &keys.key_a[i]);
            di_key(uid_bytes, &keys.key_b[i]);
            FURI_BIT_SET(keys.key_a_mask, i);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error != MfClassicErrorNone) {
            FURI_LOG_W(TAG, "Failed to read data: %d", error);
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = mf_classic_is_card_read(data);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool disney_infinity_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    size_t* uid_len = 0;
    bool parsed = false;
    FuriString* name = furi_string_alloc();
    const uint8_t verify_sector = 0;
    MfClassicKey key = {};

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    const uint8_t* uid_bytes = mf_classic_get_uid(data, uid_len);

    do {
        // verify key
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, verify_sector);

        di_key(uid_bytes, &key);
        if(memcmp(key.data, sec_tr->key_a.data, 6) != 0) break;

        // At some point I'd like to add name lookup like Skylanders
        furi_string_printf(parsed_data, "\e#Disney Infinity\n");

        parsed = true;

    } while(false);

    furi_string_free(name);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin disney_infinity_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL, // Need UID to verify key(s)
    .read = disney_infinity_read,
    .parse = disney_infinity_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor disney_infinity_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &disney_infinity_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* disney_infinity_plugin_ep(void) {
    return &disney_infinity_plugin_descriptor;
}
