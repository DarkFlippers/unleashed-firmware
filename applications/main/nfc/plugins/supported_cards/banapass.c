#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "Banapass"

static const uint64_t banapass_key_b_value_block = 0x019761AA8082;
static const uint64_t banapass_key_b_access_code = 0x574343467632;
typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static MfClassicKeyPair banapass_keys_if_value_block[] = {
    {.a = 0x6090D00632F5, .b = 0x019761AA8082},
    {.a = 0xA99164400748, .b = 0x62742819AD7C},
    {.a = 0xCC5075E42BA1, .b = 0xB9DF35A0814C},
    {.a = 0x8AF9C718F23D, .b = 0x58CD5C3673CB},
    {.a = 0xFC80E88EB88C, .b = 0x7A3CDAD7C023},
    {.a = 0x30424C029001, .b = 0x024E4E44001F},
    {.a = 0xECBBFA57C6AD, .b = 0x4757698143BD},
    {.a = 0x1D30972E6485, .b = 0xF8526D1A8D6D},
    {.a = 0x1300EC8C7E80, .b = 0xF80A65A87FFA},
    {.a = 0xDEB06ED4AF8E, .b = 0x4AD96BF28190},
    {.a = 0x000390014D41, .b = 0x0800F9917CB0},
    {.a = 0x730050555253, .b = 0x4146D4A956C4},
    {.a = 0x131157FBB126, .b = 0xE69DD9015A43},
    {.a = 0x337237F254D5, .b = 0x9A8389F32FBF},
    {.a = 0x7B8FB4A7100B, .b = 0xC8382A233993},
    {.a = 0x7B304F2A12A6, .b = 0xFC9418BF788B},
};

static MfClassicKeyPair banapass_keys_if_access_code[] = {
    {.a = 0x6090D00632F5, .b = 0x574343467632},
    {.a = 0xA99164400748, .b = 0x62742819AD7C},
    {.a = 0xCC5075E42BA1, .b = 0xB9DF35A0814C},
    {.a = 0x8AF9C718F23D, .b = 0x58CD5C3673CB},
    {.a = 0xFC80E88EB88C, .b = 0x7A3CDAD7C023},
    {.a = 0x30424C029001, .b = 0x024E4E44001F},
    {.a = 0xECBBFA57C6AD, .b = 0x4757698143BD},
    {.a = 0x1D30972E6485, .b = 0xF8526D1A8D6D},
    {.a = 0x1300EC8C7E80, .b = 0xF80A65A87FFA},
    {.a = 0xDEB06ED4AF8E, .b = 0x4AD96BF28190},
    {.a = 0x000390014D41, .b = 0x0800F9917CB0},
    {.a = 0x730050555253, .b = 0x4146D4A956C4},
    {.a = 0x131157FBB126, .b = 0xE69DD9015A43},
    {.a = 0x337237F254D5, .b = 0x9A8389F32FBF},
    {.a = 0x7B8FB4A7100B, .b = 0xC8382A233993},
    {.a = 0x7B304F2A12A6, .b = 0xFC9418BF788B},
};

static bool banapass_verify(Nfc* nfc) {
    bool verified = true;

    const uint8_t verify_sector = 0;
    uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
    FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

    MfClassicKey key_a_0 = {};
    bit_lib_num_to_bytes_be(
        banapass_keys_if_value_block[0].a, COUNT_OF(key_a_0.data), key_a_0.data);

    MfClassicAuthContext auth_ctx = {};
    MfClassicError error =
        mf_classic_poller_sync_auth(nfc, block_num, &key_a_0, MfClassicKeyTypeA, &auth_ctx);

    if(error != MfClassicErrorNone) {
        FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
        verified = false;
    }

    return verified;
}

static bool banapass_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;
        if(type != MfClassicType1k) {
            FURI_LOG_E(TAG, "Card not MIFARE Classic 1k");
            break;
        }

        data->type = type;
        MfClassicDeviceKeys keys = {};

        // Access Code Read Attempt
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(
                banapass_keys_if_access_code[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(
                banapass_keys_if_access_code[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        MfClassicError error_access_code = mf_classic_poller_sync_read(nfc, &keys, data);

        if(error_access_code == MfClassicErrorNone) {
            nfc_device_set_data(device, NfcProtocolMfClassic, data);
            is_read = true;
            break;
        }

        // Value Block Read Attempt
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(
                banapass_keys_if_value_block[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(
                banapass_keys_if_value_block[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        MfClassicError error_value_block = mf_classic_poller_sync_read(nfc, &keys, data);

        if(error_value_block == MfClassicErrorNone) {
            nfc_device_set_data(device, NfcProtocolMfClassic, data);
            is_read = true;
            break;
        }
        FURI_LOG_E(TAG, "Failed to read data. Bad keys?");
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool banapass_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // verify key
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 0);
        uint64_t key_a = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        uint64_t key_b = bit_lib_bytes_to_num_be(sec_tr->key_b.data, 6);
        if(key_a != banapass_keys_if_value_block[0].a) break;

        furi_string_set_str(parsed_data, "\e#Banapass\n");
        furi_string_cat_str(
            parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        furi_string_cat_str(parsed_data, "\nBandai Namco Passport\n");

        // banapass Magic is stored at block 1, byte 2-7
        uint8_t magic_bytes[6];
        for(int i = 0; i < 6; i++) {
            magic_bytes[i] = data->block[1].data[2 + i];
        }

        // verify banapass magic
        if(magic_bytes[0] != 'N' || magic_bytes[1] != 'B' || magic_bytes[2] != 'G' ||
           magic_bytes[3] != 'I' || magic_bytes[4] != 'C')
            break;

        // banapass checksum is stored at block 1, starts from byte 8-15
        uint8_t check_sum[8];
        for(int i = 0; i < 8; i++) {
            check_sum[i] = data->block[1].data[8 + i];
        }

        furi_string_cat_str(
            parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");

        bool is_block_2_null = true;
        for(int i = 0; i < 16; i++) {
            if(data->block[2].data[i] != 0) {
                is_block_2_null = false;
                break;
            }
        }
        if(is_block_2_null) {
            furi_string_cat_str(
                parsed_data,
                "\nPlease scan the clone at the\nnearest CHUNITHM or\nmaimai Cabinet for the\nAccess Code.\n");
            furi_string_cat_str(
                parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        } else {
            switch(key_b) {
            case banapass_key_b_value_block:
                int32_t value = 0;
                uint8_t addr = 0;
                bool value_found = mf_classic_block_to_value(
                    &data->block[2], &value, &addr); // block 2 is value block
                if(value_found) {
                    furi_string_cat_printf(parsed_data, "\nValue: %08lX", value);
                } else {
                    furi_string_cat_str(parsed_data, "\nPotential clone:\nInvalid value block.");
                }
                furi_string_cat_str(
                    parsed_data,
                    "\nPlease check the back of\nyour Bandai Namco Passport\nfor the Access Code.\n");
                furi_string_cat_str(
                    parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
                break;

            case banapass_key_b_access_code:
                // banapass access code is stored as decimal hex representation in block 2, starts from byte 6, len 10 bytes
                uint8_t access_code[10];

                furi_string_cat_printf(parsed_data, "\nAccess Code:\n");
                bool access_code_is_bcd = true;

                for(int i = 0; i < 10; i++) {
                    access_code[i] = data->block[2].data[6 + i];
                    furi_string_cat_printf(parsed_data, "%02X", access_code[i]);
                    if(i % 2 == 1) {
                        furi_string_cat_str(parsed_data, " ");
                    }
                    if((access_code[i] >> 4) > 9) access_code_is_bcd = false;
                    if((access_code[i] & 0x0F) > 9) access_code_is_bcd = false;
                }
                furi_string_cat_printf(
                    parsed_data, "\nBCD valid: %s\n", access_code_is_bcd ? "Yes" : "No");
                if((access_code[0] >> 4) != 3) {
                    furi_string_cat_printf(
                        parsed_data,
                        "Potential clone:\nAccess Code preamble\nexpected 3, got %d\n",
                        (access_code[0] >> 4));
                }
                furi_string_cat_str(
                    parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");

                break;
            default:
                break;
            }
        }
        furi_string_cat_str(parsed_data, "\nMagic:\n");
        for(int i = 0; i < 6; i++) {
            furi_string_cat_printf(parsed_data, "%c", magic_bytes[i]);
        }
        furi_string_cat_str(parsed_data, "\nChecksum:\n");
        for(int i = 0; i < 8; i++) {
            furi_string_cat_printf(parsed_data, "%02X ", check_sum[i]);
        }

        furi_string_cat_str(parsed_data, "\n");
        furi_string_cat_str(
            parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");

        parsed = true;

    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin banapass_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = banapass_verify,
    .read = banapass_read,
    .parse = banapass_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor banapass_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &banapass_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* banapass_plugin_ep(void) {
    return &banapass_plugin_descriptor;
}
