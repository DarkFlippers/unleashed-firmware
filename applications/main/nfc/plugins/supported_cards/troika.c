#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_util.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include "furi_hal_rtc.h"

#define TAG "Troika"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    const MfClassicKeyPair* keys;
    uint32_t data_sector;
} TroikaCardConfig;

typedef enum {
    TroikaLayoutUnknown = 0x0,
    TroikaLayout2 = 0x2,
    TroikaLayoutE = 0xE,
} TroikaLayout;

typedef enum {
    TroikaSublayoutUnknown = 0x0,
    TroikaSublayout3 = 0x3,
    TroikaSublayout5 = 0x5,
    TroikaSublayout6 = 0x6,
} TroikaSubLayout;

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
        config->data_sector = 8; // Further testing needed
        config->keys = troika_4k_keys;
    } else {
        success = false;
    }

    return success;
}

static TroikaLayout troika_get_layout(const MfClassicData* data, uint8_t start_block_num) {
    furi_assert(data);

    // Layout is stored in byte 6 of block, length 4 bits (bits 52 - 55), second nibble.
    const uint8_t* layout_ptr = &data->block[start_block_num].data[6];
    const uint8_t layout = (*layout_ptr & 0x0F);

    TroikaLayout result = TroikaLayoutUnknown;
    switch(layout) {
    case TroikaLayout2:
    case TroikaLayoutE:
        result = layout;
        break;
    default:
        // If debug is enabled - pass the actual layout value for the debug text
        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            return layout;
        } else {
            return TroikaLayoutUnknown;
        }
    }

    return result;
}

static TroikaSubLayout troika_get_sub_layout(const MfClassicData* data, uint8_t start_block_num) {
    furi_assert(data);

    // Sublayout is stored in byte 7 (bits 56 - 60) of block, length 5 bits (first nibble and one bit from second nibble)
    const uint8_t* sub_layout_ptr = &data->block[start_block_num].data[7];
    const uint8_t sub_layout = (*sub_layout_ptr & 0x3F) >> 3;

    TroikaSubLayout result = TroikaSublayoutUnknown;
    switch(sub_layout) {
    case TroikaSublayout3:
    case TroikaSublayout5:
    case TroikaSublayout6:
        result = sub_layout;
        break;
    default:
        // If debug is enabled - pass the actual sublayout value for the debug text
        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            return sub_layout;
        } else {
            return TroikaSublayoutUnknown;
        }
    }

    return result;
}

static bool troika_has_balance(TroikaLayout layout, TroikaSubLayout sub_layout) {
    UNUSED(sub_layout);
    // Layout 0x2 has no balance

    if(layout == TroikaLayout2) {
        return false;
    }

    return true;
}

static uint16_t troika_get_balance(
    const MfClassicData* data,
    uint8_t start_block_num,
    TroikaLayout layout,
    TroikaSubLayout sub_layout) {
    furi_assert(data);

    // In layout 0x3 balance in bits 188:209 ( from sector start, length 22).
    // In layout 0x5 balance in bits 165:185 ( from sector start, length 20).

    uint32_t balance = 0;
    uint8_t balance_data_offset = 0;
    bool supported_layout = false;

    if(layout == TroikaLayoutE && sub_layout == TroikaSublayout3) {
        balance_data_offset = 7;
        supported_layout = true;
    } else if(layout == TroikaLayoutE && sub_layout == TroikaSublayout5) {
        balance_data_offset = 4;
        supported_layout = true;
    }

    if(supported_layout) {
        const uint8_t* temp_ptr = &data->block[start_block_num + 1].data[balance_data_offset];
        balance |= (temp_ptr[0] & 0x3) << 18;
        balance |= temp_ptr[1] << 10;
        balance |= temp_ptr[2] << 2;
        balance |= (temp_ptr[3] & 0xC0) >> 6;
    }

    return balance / 100;
}

static uint32_t troika_get_number(
    const MfClassicData* data,
    uint8_t start_block_num,
    TroikaLayout layout,
    TroikaSubLayout sub_layout) {
    furi_assert(data);
    UNUSED(sub_layout);

    if(layout == TroikaLayoutE || layout == TroikaLayout2) {
        const uint8_t* temp_ptr = &data->block[start_block_num].data[2];

        uint32_t number = 0;
        for(size_t i = 1; i < 5; i++) {
            number <<= 8;
            number |= temp_ptr[i];
        }
        number >>= 4;
        number |= (temp_ptr[0] & 0xf) << 28;

        return number;
    } else {
        return 0;
    }
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

        const uint64_t key = nfc_util_bytes2num(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != cfg.keys[cfg.data_sector].a) break;

        // Get the block number of the block that contains the data
        const uint8_t start_block_num = mf_classic_get_first_block_num_of_sector(cfg.data_sector);

        // Get layout, sublayout, balance and number
        TroikaLayout layout = troika_get_layout(data, start_block_num);
        TroikaSubLayout sub_layout = troika_get_sub_layout(data, start_block_num);

        if(!furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            // If debug is enabled - proceed even if layout or sublayout is unknown, that will make collecting data easier
            if(layout == TroikaLayoutUnknown || sub_layout == TroikaSublayoutUnknown) break;
        }

        uint32_t number = troika_get_number(data, start_block_num, layout, sub_layout);

        furi_string_printf(parsed_data, "\e#Troika\nNum: %lu", number);

        if(troika_has_balance(layout, sub_layout) ||
           furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            uint16_t balance = troika_get_balance(data, start_block_num, layout, sub_layout);
            furi_string_cat_printf(parsed_data, "\nBalance: %u RUR", balance);
        } else {
            furi_string_cat_printf(parsed_data, "\nBalance: Not available");
        }

        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            furi_string_cat_printf(
                parsed_data,
                "\nLayout: %02x\nSublayout: %02x\nData Block: %u",
                layout,
                sub_layout,
                start_block_num);
        }

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
