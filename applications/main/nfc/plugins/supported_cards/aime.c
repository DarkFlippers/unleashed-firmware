#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "Aime"

static const uint64_t aime_key = 0x574343467632;

bool aime_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 0;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(aime_key, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);

        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool aime_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(aime_key, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(aime_key, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error == MfClassicErrorNotPresent) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = true;
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool aime_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // verify key
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 0);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != aime_key) break;

        // Aime Magic is stored at block 1, starts from byte 0, len 4 bytes
        const uint8_t* aime_magic = &data->block[1].data[0];

        // verify aime magic
        if(aime_magic[0] != 'S' || aime_magic[1] != 'B' || aime_magic[2] != 'S' ||
           aime_magic[3] != 'D')
            break;

        // Aime checksum is stored at block 1, starts from byte 13, len 3 bytes
        // seems like only old games checks this? e.g., old versions of Chunithm
        const uint8_t* aime_checksum = &data->block[1].data[13];

        // Aime access code is stored as decimal hex representation in block 2, starts from byte 6, len 10 bytes
        const uint8_t* aime_accesscode = &data->block[2].data[6];

        char aime_accesscode_str[24 + 1];
        snprintf(
            aime_accesscode_str,
            sizeof(aime_accesscode_str),
            "%02x%02x %02x%02x %02x%02x %02x%02x %02x%02x",
            aime_accesscode[0],
            aime_accesscode[1],
            aime_accesscode[2],
            aime_accesscode[3],
            aime_accesscode[4],
            aime_accesscode[5],
            aime_accesscode[6],
            aime_accesscode[7],
            aime_accesscode[8],
            aime_accesscode[9]);

        // validate decimal hex representation
        bool code_is_hex = true;
        for(int i = 0; i < 24; i++) {
            if(aime_accesscode_str[i] == ' ') continue;
            if(aime_accesscode_str[i] < '0' || aime_accesscode_str[i] > '9') {
                code_is_hex = false;
                break;
            }
        }
        if(!code_is_hex) break;

        // Note: Aime access code has some other self-check algorithms that are not public.
        // This parser does not try to verify the number.

        furi_string_printf(
            parsed_data,
            "\e#Aime Card\nAccess Code: \n%s\nChecksum: %02X%02X%02X\n",
            aime_accesscode_str,
            aime_checksum[0],
            aime_checksum[1],
            aime_checksum[2]);

        parsed = true;

    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin aime_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = aime_verify,
    .read = aime_read,
    .parse = aime_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor aime_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &aime_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* aime_plugin_ep(void) {
    return &aime_plugin_descriptor;
}
