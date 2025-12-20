// Flipper Zero parser for H World Hotel Key Cards
// H World operates around 10,000 hotels, most of which in mainland China
// Reverse engineering and parser written by @Torron (Github: @zinongli)
#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <bit_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG                    "H World"
#define ROOM_SECTOR            1
#define VIP_SECTOR             5
#define ROOM_SECTOR_KEY_BLOCK  7
#define VIP_SECTOR_KEY_BLOCK   23
#define ACCESS_INFO_BLOCK      5
#define ROOM_NUM_DECIMAL_BLOCK 6
#define H_WORLD_YEAR_OFFSET    2000

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static MfClassicKeyPair hworld_standard_keys[] = {
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 000
    {.a = 0x543071543071, .b = 0x5F01015F0101}, // 001
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 002
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 003
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 004
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 005
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 006
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 007
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 008
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 009
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 010
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 011
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 012
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 013
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 014
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 015
};

static MfClassicKeyPair hworld_vip_keys[] = {
    {.a = 0x000000000000, .b = 0xFFFFFFFFFFFF}, // 000
    {.a = 0x543071543071, .b = 0x5F01015F0101}, // 001
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 002
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 003
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 004
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 005
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 006
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 007
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 008
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 009
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 010
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 011
    {.a = 0xFFFFFFFFFFFF, .b = 0x200510241234}, // 012
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 013
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 014
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, // 015
};

static bool hworld_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(ROOM_SECTOR);

        MfClassicKey standard_key = {0};
        bit_lib_num_to_bytes_be(
            hworld_standard_keys[ROOM_SECTOR].a, COUNT_OF(standard_key.data), standard_key.data);

        MfClassicAuthContext auth_context;
        MfClassicError standard_error = mf_classic_poller_sync_auth(
            nfc, block_num, &standard_key, MfClassicKeyTypeA, &auth_context);

        if(standard_error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed static key check for block %u", block_num);
            break;
        }

        MfClassicKey vip_key = {0};
        bit_lib_num_to_bytes_be(
            hworld_vip_keys[VIP_SECTOR].b, COUNT_OF(vip_key.data), vip_key.data);

        MfClassicError vip_error = mf_classic_poller_sync_auth(
            nfc, block_num, &vip_key, MfClassicKeyTypeB, &auth_context);

        if(vip_error == MfClassicErrorNone) {
            FURI_LOG_D(TAG, "VIP card detected");
        } else {
            FURI_LOG_D(TAG, "Standard card detected");
        }

        verified = true;
    } while(false);

    return verified;
}

static bool hworld_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError standard_error = mf_classic_poller_sync_detect_type(nfc, &type);
        MfClassicError vip_error = MfClassicErrorNotPresent;
        if(standard_error != MfClassicErrorNone) break;
        data->type = type;

        MfClassicDeviceKeys standard_keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(
                hworld_standard_keys[i].a, sizeof(MfClassicKey), standard_keys.key_a[i].data);
            FURI_BIT_SET(standard_keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(
                hworld_standard_keys[i].b, sizeof(MfClassicKey), standard_keys.key_b[i].data);
            FURI_BIT_SET(standard_keys.key_b_mask, i);
        }

        standard_error = mf_classic_poller_sync_read(nfc, &standard_keys, data);
        if(standard_error == MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Standard card successfully read");
        } else {
            MfClassicDeviceKeys vip_keys = {};
            for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
                bit_lib_num_to_bytes_be(
                    hworld_vip_keys[i].a, sizeof(MfClassicKey), vip_keys.key_a[i].data);
                FURI_BIT_SET(vip_keys.key_a_mask, i);
                bit_lib_num_to_bytes_be(
                    hworld_vip_keys[i].b, sizeof(MfClassicKey), vip_keys.key_b[i].data);
                FURI_BIT_SET(vip_keys.key_b_mask, i);
            }

            vip_error = mf_classic_poller_sync_read(nfc, &vip_keys, data);

            if(vip_error == MfClassicErrorNone) {
                FURI_LOG_I(TAG, "VIP card successfully read");
            } else {
                break;
            }
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (standard_error == MfClassicErrorNone) | (vip_error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

bool hworld_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;

    do {
        // Check card type
        if(data->type != MfClassicType1k) break;

        // Check static key for verificaiton
        const uint8_t* data_room_sec_key_a_ptr = &data->block[ROOM_SECTOR_KEY_BLOCK].data[0];
        const uint8_t* data_room_sec_key_b_ptr = &data->block[ROOM_SECTOR_KEY_BLOCK].data[10];
        uint64_t data_room_sec_key_a = bit_lib_get_bits_64(data_room_sec_key_a_ptr, 0, 48);
        uint64_t data_room_sec_key_b = bit_lib_get_bits_64(data_room_sec_key_b_ptr, 0, 48);
        if((data_room_sec_key_a != hworld_standard_keys[ROOM_SECTOR].a) |
           (data_room_sec_key_b != hworld_standard_keys[ROOM_SECTOR].b))
            break;

        // Check whether this card is VIP
        const uint8_t* data_vip_sec_key_b_ptr = &data->block[VIP_SECTOR_KEY_BLOCK].data[10];
        uint64_t data_vip_sec_key_b = bit_lib_get_bits_64(data_vip_sec_key_b_ptr, 0, 48);
        bool is_hworld_vip = (data_vip_sec_key_b == hworld_vip_keys[VIP_SECTOR].b);
        uint8_t room_floor = data->block[ACCESS_INFO_BLOCK].data[13];
        uint8_t room_num = data->block[ACCESS_INFO_BLOCK].data[14];

        // Check in date & time
        uint16_t check_in_year = data->block[ACCESS_INFO_BLOCK].data[2] + H_WORLD_YEAR_OFFSET;
        uint8_t check_in_month = data->block[ACCESS_INFO_BLOCK].data[3];
        uint8_t check_in_day = data->block[ACCESS_INFO_BLOCK].data[4];
        uint8_t check_in_hour = data->block[ACCESS_INFO_BLOCK].data[5];
        uint8_t check_in_minute = data->block[ACCESS_INFO_BLOCK].data[6];

        // Expire date & time
        uint16_t expire_year = data->block[ACCESS_INFO_BLOCK].data[7] + H_WORLD_YEAR_OFFSET;
        uint8_t expire_month = data->block[ACCESS_INFO_BLOCK].data[8];
        uint8_t expire_day = data->block[ACCESS_INFO_BLOCK].data[9];
        uint8_t expire_hour = data->block[ACCESS_INFO_BLOCK].data[10];
        uint8_t expire_minute = data->block[ACCESS_INFO_BLOCK].data[11];

        furi_string_cat_printf(parsed_data, "\e#H World Card\n");
        furi_string_cat_printf(
            parsed_data, "%s\n", is_hworld_vip ? "VIP card" : "Standard room key");
        furi_string_cat_printf(parsed_data, "Room Num: %u%02u\n", room_floor, room_num);
        furi_string_cat_printf(
            parsed_data,
            "Check-in Date: \n%04u-%02d-%02d\n%02d:%02d:00\n",
            check_in_year,
            check_in_month,
            check_in_day,
            check_in_hour,
            check_in_minute);
        furi_string_cat_printf(
            parsed_data,
            "Expiration Date: \n%04u-%02d-%02d\n%02d:%02d:00",
            expire_year,
            expire_month,
            expire_day,
            expire_hour,
            expire_minute);
        parsed = true;
    } while(false);
    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin hworld_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = hworld_verify,
    .read = hworld_read,
    .parse = hworld_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor hworld_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &hworld_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* hworld_plugin_ep(void) {
    return &hworld_plugin_descriptor;
}
