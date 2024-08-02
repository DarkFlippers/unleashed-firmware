#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "TwoCities"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static const MfClassicKeyPair two_cities_4k_keys[] = {
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99}, {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0xe56ac127dd45, .b = 0x19fc84a3784b}, {.a = 0x77dabc9825e1, .b = 0x9764fec3154a},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xa73f5dc1d333, .b = 0xe35173494a81}, {.a = 0x69a32f1c2f19, .b = 0x6b8bd9860763},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0xacffffffffff, .b = 0x71f3a315ad26}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0x72f96bdd3714, .b = 0x462225cd34cf}, {.a = 0x044ce1872bc3, .b = 0x8c90c70cff4a},
    {.a = 0xbc2d1791dec1, .b = 0xca96a487de0b}, {.a = 0x8791b2ccb5c4, .b = 0xc956c3b80da3},
    {.a = 0x8e26e45e7d65, .b = 0x8e65b3af7d22}, {.a = 0x0f318130ed18, .b = 0x0c420a20e056},
    {.a = 0x045ceca15535, .b = 0x31bec3d9e510}, {.a = 0x9d993c5d4ef4, .b = 0x86120e488abf},
    {.a = 0xc65d4eaa645b, .b = 0xb69d40d1a439}, {.a = 0x3a8a139c20b4, .b = 0x8818a9c5d406},
    {.a = 0xbaff3053b496, .b = 0x4b7cb25354d3}, {.a = 0x7413b599c4ea, .b = 0xb0a2AAF3A1BA},
    {.a = 0x0ce7cd2cc72b, .b = 0xfa1fbb3f0f1f}, {.a = 0x0be5fac8b06a, .b = 0x6f95887a4fd3},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0x7a396f0d633d, .b = 0xad2bdc097023}, {.a = 0xa3faa6daff67, .b = 0x7600e889adf9},
    {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99}, {.a = 0x2aa05ed1856f, .b = 0xeaac88e5dc99},
    {.a = 0xa7141147d430, .b = 0xff16014fefc7}, {.a = 0x8a8d88151a00, .b = 0x038b5f9b5a2a},
    {.a = 0xb27addfb64b0, .b = 0x152fd0c420a7}, {.a = 0x7259fa0197c6, .b = 0x5583698df085},
};

bool two_cities_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 4;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(two_cities_4k_keys[verify_sector].a, COUNT_OF(key.data), key.data);

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

static bool two_cities_read(Nfc* nfc, NfcDevice* device) {
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
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(
                two_cities_4k_keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(
                two_cities_4k_keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

static bool two_cities_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify key
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 4);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != two_cities_4k_keys[4].a) return false;

        // =====
        // PLANTAIN
        // =====

        // Point to block 0 of sector 4, value 0
        const uint8_t* temp_ptr = data->block[16].data;
        // Read first 4 bytes of block 0 of sector 4 from last to first and convert them to uint32_t
        // 38 18 00 00 becomes 00 00 18 38, and equals to 6200 decimal
        uint32_t balance =
            ((temp_ptr[3] << 24) | (temp_ptr[2] << 16) | (temp_ptr[1] << 8) | temp_ptr[0]) / 100;
        // Read card number
        // Point to block 0 of sector 0, value 0
        temp_ptr = data->block[0].data;
        // Read first 7 bytes of block 0 of sector 0 from last to first and convert them to uint64_t
        // 04 31 16 8A 23 5C 80 becomes 80 5C 23 8A 16 31 04, and equals to 36130104729284868 decimal
        uint8_t card_number_arr[7];
        for(size_t i = 0; i < 7; i++) {
            card_number_arr[i] = temp_ptr[6 - i];
        }
        // Copy card number to uint64_t
        uint64_t card_number = 0;
        for(size_t i = 0; i < 7; i++) {
            card_number = (card_number << 8) | card_number_arr[i];
        }

        // =====
        // --PLANTAIN--
        // =====
        // TROIKA
        // =====

        const uint8_t* troika_temp_ptr = &data->block[33].data[5];
        uint16_t troika_balance = ((troika_temp_ptr[0] << 8) | troika_temp_ptr[1]) / 25;
        troika_temp_ptr = &data->block[32].data[2];
        uint32_t troika_number = 0;
        for(size_t i = 0; i < 4; i++) {
            troika_number <<= 8;
            troika_number |= troika_temp_ptr[i];
        }
        troika_number >>= 4;

        furi_string_printf(
            parsed_data,
            "\e#Troika+Plantain\nPN: %lluX\nPB: %lu rur.\nTN: %lu\nTB: %u rur.\n",
            card_number,
            balance,
            troika_number,
            troika_balance);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin two_cities_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = two_cities_verify,
    .read = two_cities_read,
    .parse = two_cities_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor two_cities_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &two_cities_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* two_cities_plugin_ep(void) {
    return &two_cities_plugin_descriptor;
}
