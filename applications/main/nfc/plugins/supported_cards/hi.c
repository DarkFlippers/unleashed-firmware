#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "HI!"

#define KEY_LENGTH    6
#define HI_KEY_TO_GEN 5
#define UID_LENGTH    7

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    MfClassicKeyPair* keys;
    uint32_t verify_sector;
} HiCardConfig;

static MfClassicKeyPair hi_1k_keys[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0x30871CF60CF1}, // 000
    {.a = 0x000000000000, .b = 0x000000000000}, // 001
    {.a = 0x000000000000, .b = 0x000000000000}, // 002
    {.a = 0x000000000000, .b = 0x000000000000}, // 003
    {.a = 0x000000000000, .b = 0x000000000000}, // 004
    {.a = 0x42FFE4C76209, .b = 0x7B30CFD04CBD}, // 005
    {.a = 0x01ED8145BDF8, .b = 0x92257F472FCE}, // 006
    {.a = 0x7583A07D21A6, .b = 0x51CA6EA8EE26}, // 007
    {.a = 0x1E10BF5D6A1D, .b = 0x87B9B9BFABA6}, // 008
    {.a = 0xF9DB1B2B21BA, .b = 0x80A781F4134C}, // 009
    {.a = 0x7F5283FACB72, .b = 0x73250009D75A}, // 010
    {.a = 0xE48E86A03078, .b = 0xCFFBBF08A254}, // 011
    {.a = 0x39AB26301F60, .b = 0xC71A6E532C83}, // 012
    {.a = 0xAD656C6C639F, .b = 0xFD9819CBD20A}, // 013
    {.a = 0xF0E15160DB3E, .b = 0x3F622D515ADD}, // 014
    {.a = 0x03F44E033C42, .b = 0x61E897875F46}, // 015
};

//KDF
void hi_generate_key(uint8_t* uid, uint8_t keyA[5][KEY_LENGTH], uint8_t keyB[5][KEY_LENGTH]) {
    // Static XOR table for key generation
    static const uint8_t xor_table_keyB[4][6] = {
        {0x1F, 0xC4, 0x4D, 0x94, 0x6A, 0x31},
        {0x12, 0xC1, 0x5C, 0x70, 0xDF, 0x31},
        {0x56, 0xF0, 0x13, 0x1B, 0x63, 0xF2},
        {0x4E, 0xFA, 0xC2, 0xF8, 0xC9, 0xCC}};

    static const uint8_t xor_table_keyA[4][6] = {
        {0xB6, 0xE6, 0xAE, 0x72, 0x91, 0x0D},
        {0x6D, 0x38, 0x50, 0xFB, 0x42, 0x89},
        {0x1E, 0x5F, 0xC7, 0xED, 0xAA, 0x02},
        {0x7E, 0xB9, 0xCA, 0xF1, 0x9C, 0x59}};

    // Permutation table for rearranging elements in uid
    static const uint8_t xorOrderA[6] = {0, 1, 2, 3, 0, 2};
    static const uint8_t xorOrderB[6] = {1, 3, 3, 2, 1, 0};

    // Generate key based on uid and XOR table
    for(uint8_t j = 1; j < 5; j++) {
        for(uint8_t i = 0; i < 6; i++) {
            keyA[j][i] = uid[xorOrderA[i]] ^ xor_table_keyA[j - 1][i];
            keyB[j][i] = uid[xorOrderB[i]] ^ xor_table_keyB[j - 1][i];
        }
    }
}

static bool hi_get_card_config(HiCardConfig* config, MfClassicType type) {
    bool success = true;

    if(type == MfClassicType1k) {
        config->verify_sector = 0;
        config->keys = hi_1k_keys;
    } else {
        success = false;
    }

    return success;
}

static bool hi_verify_type(Nfc* nfc, MfClassicType type) {
    bool verified = false;

    do {
        HiCardConfig cfg = {};
        if(!hi_get_card_config(&cfg, type)) break;

        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(cfg.verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %lu", cfg.verify_sector);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(cfg.keys[cfg.verify_sector].b, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeB, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(
                TAG, "Failed to read block %u: %d, this is not a HI card", block_num, error);
            break;
        }
        FURI_LOG_D(TAG, "Found a HI Card");
        verified = true;
    } while(false);

    return verified;
}

static bool hi_verify(Nfc* nfc) {
    return hi_verify_type(nfc, MfClassicType1k);
}

static bool hi_read(Nfc* nfc, NfcDevice* device) {
    FURI_LOG_D(TAG, "Entering HI KDF");
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        HiCardConfig cfg = {};
        if(!hi_get_card_config(&cfg, data->type)) break;

        uint8_t uid[UID_LENGTH];
        memcpy(uid, data->iso14443_3a_data->uid, UID_LENGTH);

        uint8_t keyA[HI_KEY_TO_GEN][KEY_LENGTH];
        uint8_t keyB[HI_KEY_TO_GEN][KEY_LENGTH];
        hi_generate_key(uid, keyA, keyB);

        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            if(cfg.keys[i].a == 0x000000000000 && cfg.keys[i].b == 0x000000000000) {
                cfg.keys[i].a = bit_lib_bytes_to_num_be(keyA[i], KEY_LENGTH);
                cfg.keys[i].b = bit_lib_bytes_to_num_be(keyB[i], KEY_LENGTH);
            }
        }

        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(cfg.keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(cfg.keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error != MfClassicErrorNone) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = mf_classic_is_card_read(data);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool hi_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify card type
        HiCardConfig cfg = {};
        if(!hi_get_card_config(&cfg, data->type)) break;

        // Verify key
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, cfg.verify_sector);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_b.data, 6);
        if(key != cfg.keys[cfg.verify_sector].b) return false;

        //Get UID
        uint8_t uid[UID_LENGTH];
        memcpy(uid, data->iso14443_3a_data->uid, UID_LENGTH);

        //parse data
        furi_string_cat_printf(parsed_data, "\e#HI! Card\n");
        furi_string_cat_printf(parsed_data, "UID:");
        for(size_t i = 0; i < UID_LENGTH; i++) {
            furi_string_cat_printf(parsed_data, " %02X", uid[i]);
        }
        furi_string_cat_printf(parsed_data, "\n");

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin hi_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = hi_verify,
    .read = hi_read,
    .parse = hi_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor hi_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &hi_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* hi_plugin_ep(void) {
    return &hi_plugin_descriptor;
}
