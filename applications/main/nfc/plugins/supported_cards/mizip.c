#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "MiZIP"

#define KEY_LENGTH       6
#define MIZIP_KEY_TO_GEN 5
#define UID_LENGTH       4

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    MfClassicKeyPair* keys;
    uint32_t verify_sector;
} MizipCardConfig;

static MfClassicKeyPair mizip_1k_keys[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb4c132439eef}, // 000
    {.a = 0x000000000000, .b = 0x000000000000}, // 001
    {.a = 0x000000000000, .b = 0x000000000000}, // 002
    {.a = 0x000000000000, .b = 0x000000000000}, // 003
    {.a = 0x000000000000, .b = 0x000000000000}, // 004
    {.a = 0x0222179AB995, .b = 0x13321774F9B5}, // 005
    {.a = 0xB25CBD76A7B4, .b = 0x7571359B4274}, // 006
    {.a = 0xDA857B4907CC, .b = 0xD26B856175F7}, // 007
    {.a = 0x16D85830C443, .b = 0x8F790871A21E}, // 008
    {.a = 0x88BD5098FC82, .b = 0xFCD0D77745E4}, // 009
    {.a = 0x983349449D78, .b = 0xEA2631FBDEDD}, // 010
    {.a = 0xC599F962F3D9, .b = 0x949B70C14845}, // 011
    {.a = 0x72E668846BE8, .b = 0x45490B5AD707}, // 012
    {.a = 0xBCA105E5685E, .b = 0x248DAF9D674D}, // 013
    {.a = 0x4F6FE072D1FD, .b = 0x4250A05575FA}, // 014
    {.a = 0x56438ABE8152, .b = 0x59A45912B311}, // 015
};

static MfClassicKeyPair mizip_mini_keys[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb4c132439eef}, // 000
    {.a = 0x000000000000, .b = 0x000000000000}, // 001
    {.a = 0x000000000000, .b = 0x000000000000}, // 002
    {.a = 0x000000000000, .b = 0x000000000000}, // 003
    {.a = 0x000000000000, .b = 0x000000000000}, // 004
};

//KDF
void mizip_generate_key(uint8_t* uid, uint8_t keyA[5][KEY_LENGTH], uint8_t keyB[5][KEY_LENGTH]) {
    // Static XOR table for key generation
    static const uint8_t xor_table_keyA[4][6] = {
        {0x09, 0x12, 0x5A, 0x25, 0x89, 0xE5},
        {0xAB, 0x75, 0xC9, 0x37, 0x92, 0x2F},
        {0xE2, 0x72, 0x41, 0xAF, 0x2C, 0x09},
        {0x31, 0x7A, 0xB7, 0x2F, 0x44, 0x90}};

    static const uint8_t xor_table_keyB[4][6] = {
        {0xF1, 0x2C, 0x84, 0x53, 0xD8, 0x21},
        {0x73, 0xE7, 0x99, 0xFE, 0x32, 0x41},
        {0xAA, 0x4D, 0x13, 0x76, 0x56, 0xAE},
        {0xB0, 0x13, 0x27, 0x27, 0x2D, 0xFD}};

    // Permutation table for rearranging elements in uid
    static const uint8_t xorOrderA[6] = {0, 1, 2, 3, 0, 1};
    static const uint8_t xorOrderB[6] = {2, 3, 0, 1, 2, 3};

    // Generate key based on uid and XOR table
    for(uint8_t j = 1; j < 5; j++) {
        for(uint8_t i = 0; i < 6; i++) {
            keyA[j][i] = uid[xorOrderA[i]] ^ xor_table_keyA[j - 1][i];
            keyB[j][i] = uid[xorOrderB[i]] ^ xor_table_keyB[j - 1][i];
        }
    }
}

static bool mizip_get_card_config(MizipCardConfig* config, MfClassicType type) {
    bool success = true;

    if(type == MfClassicType1k) {
        config->verify_sector = 0;
        config->keys = mizip_1k_keys;
    } else if(type == MfClassicTypeMini) {
        config->verify_sector = 0;
        config->keys = mizip_mini_keys;
    } else {
        success = false;
    }

    return success;
}

static bool mizip_verify_type(Nfc* nfc, MfClassicType type) {
    bool verified = false;

    do {
        MizipCardConfig cfg = {};
        if(!mizip_get_card_config(&cfg, type)) break;

        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(cfg.verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %lu", cfg.verify_sector);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(cfg.keys[cfg.verify_sector].b, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeB, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(
                TAG, "Failed to read block %u: %d, this is not a MiZIP card", block_num, error);
            break;
        }
        FURI_LOG_D(TAG, "Found a MiZIP Card");
        verified = true;
    } while(false);

    return verified;
}

static bool mizip_verify(Nfc* nfc) {
    return mizip_verify_type(nfc, MfClassicType1k) || mizip_verify_type(nfc, MfClassicTypeMini);
}

static bool mizip_read(Nfc* nfc, NfcDevice* device) {
    FURI_LOG_D(TAG, "Entering MiZIP KDF");
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicTypeMini;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        //temp fix but fix mf_classic_poller_sync_detect_type because view type mfclassic1k and not verify mfmini
        data->type = MfClassicTypeMini;
        MizipCardConfig cfg = {};
        if(!mizip_get_card_config(&cfg, data->type)) break;

        uint8_t uid[UID_LENGTH];
        memcpy(uid, data->iso14443_3a_data->uid, UID_LENGTH);

        uint8_t keyA[MIZIP_KEY_TO_GEN][KEY_LENGTH];
        uint8_t keyB[MIZIP_KEY_TO_GEN][KEY_LENGTH];
        mizip_generate_key(uid, keyA, keyB);

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

static bool mizip_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify card type
        MizipCardConfig cfg = {};
        if(!mizip_get_card_config(&cfg, data->type)) break;

        // Verify key
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, cfg.verify_sector);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_b.data, 6);
        if(key != cfg.keys[cfg.verify_sector].b) return false;

        //Get UID
        uint8_t uid[UID_LENGTH];
        memcpy(uid, data->iso14443_3a_data->uid, UID_LENGTH);

        //Get credit
        uint8_t credit_pointer = 0x08;
        uint8_t previus_credit_pointer = 0x09;
        if(data->block[10].data[0] == 0x55) {
            credit_pointer = 0x09;
            previus_credit_pointer = 0x08;
        }
        uint16_t balance = (data->block[credit_pointer].data[2] << 8) |
                           (data->block[credit_pointer].data[1]);
        uint16_t previus_balance = (data->block[previus_credit_pointer].data[2] << 8) |
                                   (data->block[previus_credit_pointer].data[1]);

        //parse data
        furi_string_cat_printf(parsed_data, "\e#MiZIP Card\n");
        furi_string_cat_printf(parsed_data, "UID:");
        for(size_t i = 0; i < UID_LENGTH; i++) {
            furi_string_cat_printf(parsed_data, " %02X", uid[i]);
        }
        furi_string_cat_printf(
            parsed_data, "\nCurrent Credit: %d.%02d E \n", balance / 100, balance % 100);
        furi_string_cat_printf(
            parsed_data,
            "Previus Credit: %d.%02d E \n",
            previus_balance / 100,
            previus_balance % 100);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin mizip_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = mizip_verify,
    .read = mizip_read,
    .parse = mizip_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor mizip_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &mizip_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* mizip_plugin_ep(void) {
    return &mizip_plugin_descriptor;
}
