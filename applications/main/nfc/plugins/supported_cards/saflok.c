// From: https://gitee.com/jadenwu/Saflok_KDF/blob/master/saflok.c
// KDF published and reverse engineered by Jaden Wu
// FZ plugin by @noproto

#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#include <bit_lib.h>

#define TAG "Saflok"

#define MAGIC_TABLE_SIZE 192
#define KEY_LENGTH       6
#define UID_LENGTH       4
#define CHECK_SECTOR     1

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static MfClassicKeyPair saflok_1k_keys[] = {
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 000
    {.a = 0x2a2c13cc242a, .b = 0xffffffffffff}, // 001
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, // 002
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, // 003
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 004
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 005
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 006
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 007
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 008
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 009
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 010
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 011
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 012
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 013
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 014
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 015
};

void generate_saflok_key(const uint8_t* uid, uint8_t* key) {
    static const uint8_t magic_table[MAGIC_TABLE_SIZE] = {
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xF0, 0x57, 0xB3, 0x9E, 0xE3, 0xD8, 0x00, 0x00, 0xAA,
        0x00, 0x00, 0x00, 0x96, 0x9D, 0x95, 0x4A, 0xC1, 0x57, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00,
        0x8F, 0x43, 0x58, 0x0D, 0x2C, 0x9D, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xFF, 0xCC, 0xE0,
        0x05, 0x0C, 0x43, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x34, 0x1B, 0x15, 0xA6, 0x90, 0xCC,
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x89, 0x58, 0x56, 0x12, 0xE7, 0x1B, 0x00, 0x00, 0xAA,
        0x00, 0x00, 0x00, 0xBB, 0x74, 0xB0, 0x95, 0x36, 0x58, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00,
        0xFB, 0x97, 0xF8, 0x4B, 0x5B, 0x74, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xC9, 0xD1, 0x88,
        0x35, 0x9F, 0x92, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x8F, 0x92, 0xE9, 0x7F, 0x58, 0x97,
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x16, 0x6C, 0xA2, 0xB0, 0x9F, 0xD1, 0x00, 0x00, 0xAA,
        0x00, 0x00, 0x00, 0x27, 0xDD, 0x93, 0x10, 0x1C, 0x6C, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00,
        0xDA, 0x3E, 0x3F, 0xD6, 0x49, 0xDD, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x58, 0xDD, 0xED,
        0x07, 0x8E, 0x3E, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x5C, 0xD0, 0x05, 0xCF, 0xD9, 0x07,
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x11, 0x8D, 0xD0, 0x01, 0x87, 0xD0};

    uint8_t magic_byte = (uid[3] >> 4) + (uid[2] >> 4) + (uid[0] & 0x0F);
    uint8_t magickal_index = (magic_byte & 0x0F) * 12 + 11;

    uint8_t temp_key[KEY_LENGTH] = {magic_byte, uid[0], uid[1], uid[2], uid[3], magic_byte};
    uint8_t carry_sum = 0;

    for(int i = KEY_LENGTH - 1; i >= 0; i--, magickal_index--) {
        uint16_t keysum = temp_key[i] + magic_table[magickal_index] + carry_sum;
        temp_key[i] = (keysum & 0xFF);
        carry_sum = keysum >> 8;
    }

    memcpy(key, temp_key, KEY_LENGTH);
}

static bool saflok_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(CHECK_SECTOR);
        FURI_LOG_D(TAG, "Saflok: Verifying sector %i", CHECK_SECTOR);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(saflok_1k_keys[CHECK_SECTOR].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Saflok: Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool saflok_read(Nfc* nfc, NfcDevice* device) {
    FURI_LOG_D(TAG, "Entering Saflok KDF");

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

        size_t uid_len;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
        FURI_LOG_D(
            TAG, "Saflok: UID identified: %02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
        if(uid_len != UID_LENGTH) break;

        uint8_t key[KEY_LENGTH];
        generate_saflok_key(uid, key);
        uint64_t num_key = bit_lib_bytes_to_num_be(key, KEY_LENGTH);
        FURI_LOG_D(TAG, "Saflok: Key generated for UID: %012llX", num_key);

        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            if(saflok_1k_keys[i].a == 0x000000000000) {
                saflok_1k_keys[i].a = num_key;
            }
        }

        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(saflok_1k_keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(saflok_1k_keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error == MfClassicErrorNotPresent) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin saflok_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = saflok_verify,
    .read = saflok_read,
    // KDF mode
    .parse = NULL,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor saflok_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &saflok_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* saflok_plugin_ep() {
    return &saflok_plugin_descriptor;
}
