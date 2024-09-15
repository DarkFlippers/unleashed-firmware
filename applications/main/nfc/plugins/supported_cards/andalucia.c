#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include "protocols/mf_classic/mf_classic.h"
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#include <bit_lib.h>

#define TAG "Andalucia"

static const uint64_t andalucia_key = 0x99100225D83B;
static const uint8_t andalucia_sector = 9;

bool andalucia_verify(Nfc* nfc) {
    bool verified = false;

    do {
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(andalucia_sector);
        FURI_LOG_D(TAG, "Andalucia: Verifying sector %u", andalucia_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(andalucia_key, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);

        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Andalucia: Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool andalucia_read(Nfc* nfc, NfcDevice* device) {
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
        if(type != MfClassicType1k) break;

        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(andalucia_key, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(andalucia_key, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error == MfClassicErrorNotPresent) {
            FURI_LOG_W(TAG, "Andalucia: Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool andalucia_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify key
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, andalucia_sector);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != andalucia_key) return false;

        // Verify card type
        if(data->type != MfClassicType1k) return false;

        // Point to sector 9, block 1, value 0
        const uint8_t* temp_ptr = &data->block[andalucia_sector * 4 + 1].data[0];
        // Read first 4 bytes of block 0 of sector 4 from last to first and convert them to uint32_t
        // 38 18 00 00 becomes 00 00 18 38, and equals to 6200 decimal
        uint32_t balance =
            ((temp_ptr[3] << 24) | (temp_ptr[2] << 16) | (temp_ptr[1] << 8) | temp_ptr[0]);

        double eur = (balance / 2) / 100.0f;

        furi_string_printf(
            parsed_data, "\e#Andalucia\nBalance: %.2f euros.", eur);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin andalucia_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = andalucia_verify,
    .read = andalucia_read,
    .parse = andalucia_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor andalucia_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &andalucia_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* andalucia_plugin_ep(void) {
    return &andalucia_plugin_descriptor;
}