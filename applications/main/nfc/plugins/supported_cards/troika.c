#include "nfc_supported_card_plugin.h"
#include <core/check.h>

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include "../../api/mosgortrans/mosgortrans_util.h"

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
    {.a = 0xEC29806D9738, .b = 0xFBF225DC5D58}, //1
    {.a = 0xA0A1A2A3A4A5, .b = 0x7DE02A7F6025}, //2
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //3
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //4
    {.a = 0x73068F118C13, .b = 0x2B7F3253FAC5}, //5
    {.a = 0xFBC2793D540B, .b = 0xD3A297DC2698}, //6
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //7
    {.a = 0xAE3D65A3DAD4, .b = 0x0F1C63013DBA}, //8
    {.a = 0xA73F5DC1D333, .b = 0xE35173494A81}, //9
    {.a = 0x69A32F1C2F19, .b = 0x6B8BD9860763}, //10
    {.a = 0x9BECDF3D9273, .b = 0xF8493407799D}, //11
    {.a = 0x08B386463229, .b = 0x5EFBAECEF46B}, //12
    {.a = 0xCD4C61C26E3D, .b = 0x31C7610DE3B0}, //13
    {.a = 0xA82607B01C0D, .b = 0x2910989B6880}, //14
    {.a = 0x0E8F64340BA4, .b = 0x4ACEC1205D75}, //15
    {.a = 0x2AA05ED1856F, .b = 0xEAAC88E5DC99}, //16
    {.a = 0x6B02733BB6EC, .b = 0x7038CD25C408}, //17
    {.a = 0x403D706BA880, .b = 0xB39D19A280DF}, //18
    {.a = 0xC11F4597EFB5, .b = 0x70D901648CB9}, //19
    {.a = 0x0DB520C78C1C, .b = 0x73E5B9D9D3A4}, //20
    {.a = 0x3EBCE0925B2F, .b = 0x372CC880F216}, //21
    {.a = 0x16A27AF45407, .b = 0x9868925175BA}, //22
    {.a = 0xABA208516740, .b = 0xCE26ECB95252}, //23
    {.a = 0xCD64E567ABCD, .b = 0x8F79C4FD8A01}, //24
    {.a = 0x764CD061F1E6, .b = 0xA74332F74994}, //25
    {.a = 0x1CC219E9FEC1, .b = 0xB90DE525CEB6}, //26
    {.a = 0x2FE3CB83EA43, .b = 0xFBA88F109B32}, //27
    {.a = 0x07894FFEC1D6, .b = 0xEFCB0E689DB3}, //28
    {.a = 0x04C297B91308, .b = 0xC8454C154CB5}, //29
    {.a = 0x7A38E3511A38, .b = 0xAB16584C972A}, //30
    {.a = 0x7545DF809202, .b = 0xECF751084A80}, //31
    {.a = 0x5125974CD391, .b = 0xD3EAFB5DF46D}, //32
    {.a = 0x7A86AA203788, .b = 0xE41242278CA2}, //33
    {.a = 0xAFCEF64C9913, .b = 0x9DB96DCA4324}, //34
    {.a = 0x04EAA462F70B, .b = 0xAC17B93E2FAE}, //35
    {.a = 0xE734C210F27E, .b = 0x29BA8C3E9FDA}, //36
    {.a = 0xD5524F591EED, .b = 0x5DAF42861B4D}, //37
    {.a = 0xE4821A377B75, .b = 0xE8709E486465}, //38
    {.a = 0x518DC6EEA089, .b = 0x97C64AC98CA4}, //39
    {.a = 0xBB52F8CCE07F, .b = 0x6B6119752C70}, //40
};

static bool troika_get_card_config(TroikaCardConfig* config, MfClassicType type) {
    bool success = true;

    if(type == MfClassicType1k) {
        config->data_sector = 11;
        config->keys = troika_1k_keys;
    } else if(type == MfClassicType4k) {
        config->data_sector = 8; // Further testing needed
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
        bit_lib_num_to_bytes_be(cfg.keys[cfg.data_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }
        FURI_LOG_D(TAG, "Verify success!");
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
            bit_lib_num_to_bytes_be(cfg.keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(cfg.keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

        const uint64_t key =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != cfg.keys[cfg.data_sector].a) break;

        FuriString* metro_result = furi_string_alloc();
        FuriString* ground_result = furi_string_alloc();
        FuriString* tat_result = furi_string_alloc();

        bool is_metro_data_present =
            mosgortrans_parse_transport_block(&data->block[32], metro_result);
        bool is_ground_data_present =
            mosgortrans_parse_transport_block(&data->block[28], ground_result);
        bool is_tat_data_present = mosgortrans_parse_transport_block(&data->block[16], tat_result);

        furi_string_cat_printf(parsed_data, "\e#Troyka card\n");
        if(is_metro_data_present && !furi_string_empty(metro_result)) {
            render_section_header(parsed_data, "Metro", 22, 21);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(metro_result));
        }

        if(is_ground_data_present && !furi_string_empty(ground_result)) {
            render_section_header(parsed_data, "Ediny", 22, 22);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(ground_result));
        }

        if(is_tat_data_present && !furi_string_empty(tat_result)) {
            render_section_header(parsed_data, "TAT", 24, 23);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(tat_result));
        }

        furi_string_free(tat_result);
        furi_string_free(ground_result);
        furi_string_free(metro_result);

        parsed = is_metro_data_present || is_ground_data_present || is_tat_data_present;
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
const FlipperAppPluginDescriptor* troika_plugin_ep(void) {
    return &troika_plugin_descriptor;
}
