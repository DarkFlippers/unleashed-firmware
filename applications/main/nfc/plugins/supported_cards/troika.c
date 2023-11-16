#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_util.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "Troika"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    const MfClassicKeyPair* keys;
    uint32_t data_sector;
} TroikaCardConfig;

static const MfClassicKeyPair troika_1k_keys[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xfbf225dc5d58},
    {.a = 0xa82607b01c0d, .b = 0x2910989b6880},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0x73068f118c13, .b = 0x2b7f3253fac5},
    {.a = 0xfbc2793d540b, .b = 0xd3a297dc2698},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0xae3d65a3dad4, .b = 0x0f1c63013dba},
    {.a = 0xa73f5dc1d333, .b = 0xe35173494a81},
    {.a = 0x69a32f1c2f19, .b = 0x6b8bd9860763},
    {.a = 0x9becdf3d9273, .b = 0xf8493407799d},
    {.a = 0x08b386463229, .b = 0x5efbaecef46b},
    {.a = 0xcd4c61c26e3d, .b = 0x31c7610de3b0},
    {.a = 0xa82607b01c0d, .b = 0x2910989b6880},
    {.a = 0x0e8f64340ba4, .b = 0x4acec1205d75},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
};

static const MfClassicKeyPair troika_4k_keys[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xfbf225dc5d58}, {.a = 0xa82607b01c0d, .b = 0x2910989b6880},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99}, {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0x73068f118c13, .b = 0x2b7f3253fac5}, {.a = 0xfbc2793d540b, .b = 0xd3a297dc2698},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99}, {.a = 0xae3d65a3dad4, .b = 0x0f1c63013dbb},
    {.a = 0xa73f5dc1d333, .b = 0xe35173494a81}, {.a = 0x69a32f1c2f19, .b = 0x6b8bd9860763},
    {.a = 0x9becdf3d9273, .b = 0xf8493407799d}, {.a = 0x08b386463229, .b = 0x5efbaecef46b},
    {.a = 0xcd4c61c26e3d, .b = 0x31c7610de3b0}, {.a = 0xa82607b01c0d, .b = 0x2910989b6880},
    {.a = 0x0e8f64340ba4, .b = 0x4acec1205d75}, {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0x6b02733bb6ec, .b = 0x7038cd25c408}, {.a = 0x403d706ba880, .b = 0xb39d19a280df},
    {.a = 0xc11f4597efb5, .b = 0x70d901648cb9}, {.a = 0x0db520c78c1c, .b = 0x73e5b9d9d3a4},
    {.a = 0x3ebce0925b2f, .b = 0x372cc880f216}, {.a = 0x16a27af45407, .b = 0x9868925175ba},
    {.a = 0xaba208516740, .b = 0xce26ecb95252}, {.a = 0xcd64e567abcd, .b = 0x8f79c4fd8a01},
    {.a = 0x764cd061f1e6, .b = 0xa74332f74994}, {.a = 0x1cc219e9fec1, .b = 0xb90de525ceb6},
    {.a = 0x2fe3cb83ea43, .b = 0xfba88f109b32}, {.a = 0x07894ffec1d6, .b = 0xefcb0e689db3},
    {.a = 0x04c297b91308, .b = 0xc8454c154cb5}, {.a = 0x7a38e3511a38, .b = 0xab16584c972a},
    {.a = 0x7545df809202, .b = 0xecf751084a80}, {.a = 0x5125974cd391, .b = 0xd3eafb5df46d},
    {.a = 0x7a86aa203788, .b = 0xe41242278ca2}, {.a = 0xafcef64c9913, .b = 0x9db96dca4324},
    {.a = 0x04eaa462f70b, .b = 0xac17b93e2fae}, {.a = 0xe734c210f27e, .b = 0x29ba8c3e9fda},
    {.a = 0xd5524f591eed, .b = 0x5daf42861b4d}, {.a = 0xe4821a377b75, .b = 0xe8709e486465},
    {.a = 0x518dc6eea089, .b = 0x97c64ac98ca4}, {.a = 0xbb52f8cce07f, .b = 0x6b6119752c70},
};

static bool troika_get_card_config(TroikaCardConfig* config, MfClassicType type) {
    bool success = true;

    if(type == MfClassicType1k) {
        config->data_sector = 8;
        config->keys = troika_1k_keys;
    } else if(type == MfClassicType4k) {
        config->data_sector = 4;
        config->keys = troika_4k_keys;
    } else {
        success = false;
    }

    return success;
}

static bool troika_verify_type(Nfc* nfc, MfClassicType type) {
    bool verified = false;

    do {
        TroikaCardConfig cfg = {};
        if(!troika_get_card_config(&cfg, type)) break;

        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(cfg.data_sector);
        FURI_LOG_D(TAG, "Verifying sector %lu", cfg.data_sector);

        MfClassicKey key = {0};
        nfc_util_num2bytes(cfg.keys[cfg.data_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool troika_verify(Nfc* nfc) {
    return troika_verify_type(nfc, MfClassicType1k) || troika_verify_type(nfc, MfClassicType4k);
}

static bool troika_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicTypeMini;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        TroikaCardConfig cfg = {};
        if(!troika_get_card_config(&cfg, data->type)) break;

        MfClassicDeviceKeys keys = {
            .key_a_mask = 0,
            .key_b_mask = 0,
        };
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            nfc_util_num2bytes(cfg.keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            nfc_util_num2bytes(cfg.keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error != MfClassicErrorNone) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = true;
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool troika_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify card type
        TroikaCardConfig cfg = {};
        if(!troika_get_card_config(&cfg, data->type)) break;

        // Verify key
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, cfg.data_sector);

        const uint64_t key = nfc_util_bytes2num(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != cfg.keys[cfg.data_sector].a) break;

        // Parse data
        const uint8_t start_block_num = mf_classic_get_first_block_num_of_sector(cfg.data_sector);

        const uint8_t* temp_ptr = &data->block[start_block_num + 1].data[5];
        uint16_t balance = ((temp_ptr[0] << 8) | temp_ptr[1]) / 25;
        temp_ptr = &data->block[start_block_num].data[2];

        uint32_t number = 0;
        for(size_t i = 1; i < 5; i++) {
            number <<= 8;
            number |= temp_ptr[i];
        }
        number >>= 4;
        number |= (temp_ptr[0] & 0xf) << 28;

        furi_string_printf(parsed_data, "\e#Troika\nNum: %lu\nBalance: %u RUR", number, balance);
        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin troika_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = troika_verify,
    .read = troika_read,
    .parse = troika_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor troika_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &troika_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* troika_plugin_ep() {
    return &troika_plugin_descriptor;
}
