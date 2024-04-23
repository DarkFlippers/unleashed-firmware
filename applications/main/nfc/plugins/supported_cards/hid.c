#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "HID"

static const uint64_t hid_key = 0x484944204953;

bool hid_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 1;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(hid_key, COUNT_OF(key.data), key.data);

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

static bool hid_read(Nfc* nfc, NfcDevice* device) {
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
            bit_lib_num_to_bytes_be(hid_key, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(hid_key, sizeof(MfClassicKey), keys.key_b[i].data);
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

static uint8_t get_bit_length(const uint8_t* half_block) {
    uint8_t bitLength = 0;
    uint32_t* halves = (uint32_t*)half_block;
    if(halves[0] == 0) {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[1]));
        bitLength = 31 - leading0s;
    } else {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[0]));
        bitLength = 63 - leading0s;
    }

    return bitLength;
}

static uint64_t get_pacs_bits(const uint8_t* block, uint8_t bitLength) {
    // Remove sentinel bit from credential.  Byteswapping to handle array of bytes vs 64bit value
    uint64_t sentinel = __builtin_bswap64(1ULL << bitLength);
    uint64_t swapped = 0;
    memcpy(&swapped, block, sizeof(uint64_t));
    swapped = __builtin_bswap64(swapped ^ sentinel);
    FURI_LOG_D(TAG, "PACS: (%d) %016llx", bitLength, swapped);
    return swapped;
}

static bool hid_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // verify key
        const uint8_t verify_sector = 1;
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, verify_sector);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != hid_key) break;

        // Currently doesn't support bit length > 63
        const uint8_t* credential_block = data->block[5].data + 8;

        uint8_t bitLength = get_bit_length(credential_block);
        if(bitLength == 0) break;

        uint64_t credential = get_pacs_bits(credential_block, bitLength);
        if(credential == 0) break;

        furi_string_printf(parsed_data, "\e#HID Card\n%dbit\n%llx", bitLength, credential);

        parsed = true;

    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin hid_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = hid_verify,
    .read = hid_read,
    .parse = hid_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor hid_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &hid_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* hid_plugin_ep(void) {
    return &hid_plugin_descriptor;
}
