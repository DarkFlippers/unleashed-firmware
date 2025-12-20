#include "nfc_supported_card_plugin.h"
#include <core/check.h>

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include "../../api/mosgortrans/mosgortrans_util.h"
#include "furi_hal_rtc.h"

#define TAG "Social_Moscow"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    const MfClassicKeyPair* keys;
    uint32_t data_sector;
} SocialMoscowCardConfig;

static const MfClassicKeyPair social_moscow_1k_keys[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0x7de02a7f6025},
    {.a = 0x2735fc181807, .b = 0xbf23a53c1f63},
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368},
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f},
    {.a = 0x73068f118c13, .b = 0x2b7f3253fac5},
    {.a = 0x186d8c4b93f9, .b = 0x9f131d8c2057},
    {.a = 0x3a4bba8adaf0, .b = 0x67362d90f973},
    {.a = 0x8765b17968a2, .b = 0x6202a38f69e2},
    {.a = 0x40ead80721ce, .b = 0x100533b89331},
    {.a = 0x0db5e6523f7c, .b = 0x653a87594079},
    {.a = 0x51119dae5216, .b = 0xd8a274b2e026},
    {.a = 0x51119dae5216, .b = 0xd8a274b2e026},
    {.a = 0x51119dae5216, .b = 0xd8a274b2e026},
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368},
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f},
    {.a = 0xa0a1a2a3a4a5, .b = 0x7de02a7f6025}};

static const MfClassicKeyPair social_moscow_4k_keys[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0x7de02a7f6025}, //1
    {.a = 0x2735fc181807, .b = 0xbf23a53c1f63}, //2
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368}, //3
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f}, //4
    {.a = 0x73068f118c13, .b = 0x2b7f3253fac5}, //5
    {.a = 0x186d8c4b93f9, .b = 0x9f131d8c2057}, //6
    {.a = 0x3a4bba8adaf0, .b = 0x67362d90f973}, //7
    {.a = 0x8765b17968a2, .b = 0x6202a38f69e2}, //8
    {.a = 0x40ead80721ce, .b = 0x100533b89331}, //9
    {.a = 0x0db5e6523f7c, .b = 0x653a87594079}, //10
    {.a = 0x51119dae5216, .b = 0xd8a274b2e026}, //11
    {.a = 0x51119dae5216, .b = 0xd8a274b2e026}, //12
    {.a = 0x51119dae5216, .b = 0xd8a274b2e026}, //13
    {.a = 0xa0a1a2a3a4a5, .b = 0x7de02a7f6025}, //14
    {.a = 0xa0a1a2a3a4a5, .b = 0x7de02a7f6025}, //15
    {.a = 0xa0a1a2a3a4a5, .b = 0x7de02a7f6025}, //16
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //17
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368}, //18
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f}, //19
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368}, //20
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f}, //21
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //22
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //23
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //24
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //25
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //26
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //27
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //28
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //29
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //30
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //31
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //32
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //33
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //34
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //35
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //36
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //37
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //38
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //39
    {.a = 0xa229e68ad9e5, .b = 0x49c2b5296ef4}, //40
};

static bool social_moscow_get_card_config(SocialMoscowCardConfig* config, MfClassicType type) {
    bool success = true;
    if(type == MfClassicType1k) {
        config->data_sector = 15;
        config->keys = social_moscow_1k_keys;
    } else if(type == MfClassicType4k) {
        config->data_sector = 15;
        config->keys = social_moscow_4k_keys;
    } else {
        success = false;
    }

    return success;
}

static bool social_moscow_verify_type(Nfc* nfc, MfClassicType type) {
    bool verified = false;

    do {
        SocialMoscowCardConfig cfg = {};
        if(!social_moscow_get_card_config(&cfg, type)) break;

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

static bool social_moscow_verify(Nfc* nfc) {
    return social_moscow_verify_type(nfc, MfClassicType1k) ||
           social_moscow_verify_type(nfc, MfClassicType4k);
}

static bool social_moscow_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType4k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        SocialMoscowCardConfig cfg = {};
        if(!social_moscow_get_card_config(&cfg, data->type)) break;

        MfClassicDeviceKeys keys = {};
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

static uint8_t calculate_luhn(uint64_t number) {
    // https://en.wikipedia.org/wiki/Luhn_algorithm
    // Drop existing check digit to form payload
    uint64_t payload = number / 10;
    int sum = 0;
    int position = 0;

    while(payload > 0) {
        int digit = payload % 10;
        if(position % 2 == 0) {
            digit *= 2;
        }
        if(digit > 9) {
            digit = (digit / 10) + (digit % 10);
        }
        sum += digit;
        payload /= 10;
        position++;
    }

    return (10 - (sum % 10)) % 10;
}

static uint64_t hex_num(uint64_t hex) {
    uint64_t result = 0;
    for(uint8_t i = 0; i < 8; ++i) {
        uint8_t half_byte = hex & 0x0F;
        uint64_t num = 0;
        for(uint8_t j = 0; j < 4; ++j) {
            num += (half_byte & 0x1) * (1 << j);
            half_byte = half_byte >> 1;
        }
        result += num * pow(10, i);
        hex = hex >> 4;
    }
    return result;
}

static bool social_moscow_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify card type
        SocialMoscowCardConfig cfg = {};
        if(!social_moscow_get_card_config(&cfg, data->type)) break;

        // Verify key
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, cfg.data_sector);

        const uint64_t key_a =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        const uint64_t key_b =
            bit_lib_bytes_to_num_be(sec_tr->key_b.data, COUNT_OF(sec_tr->key_b.data));
        if((key_a != cfg.keys[cfg.data_sector].a) || (key_b != cfg.keys[cfg.data_sector].b)) break;

        uint32_t card_code = bit_lib_get_bits_32(data->block[60].data, 8, 24);
        uint8_t card_region = bit_lib_get_bits(data->block[60].data, 32, 8);
        uint64_t card_number = bit_lib_get_bits_64(data->block[60].data, 40, 40);
        uint8_t card_control = bit_lib_get_bits(data->block[60].data, 80, 4);
        uint64_t omc_number = bit_lib_get_bits_64(data->block[21].data, 8, 64);
        uint8_t year = data->block[60].data[11];
        uint8_t month = data->block[60].data[12];

        uint64_t number = hex_num(card_control) + hex_num(card_number) * 10 +
                          hex_num(card_region) * 10 * 10000000000 +
                          hex_num(card_code) * 10 * 10000000000 * 100;

        uint8_t luhn = calculate_luhn(number);
        if(luhn != card_control) break;

        FuriString* metro_result = furi_string_alloc();
        FuriString* ground_result = furi_string_alloc();
        bool is_metro_data_present =
            mosgortrans_parse_transport_block(&data->block[4], metro_result);
        bool is_ground_data_present =
            mosgortrans_parse_transport_block(&data->block[16], ground_result);
        furi_string_cat_printf(
            parsed_data,
            "\e#Social \ecard\nNumber: %lx %x %llx %x\nOMC: %llx\nValid for: %02x/%02x %02x%02x\n",
            card_code,
            card_region,
            card_number,
            card_control,
            omc_number,
            month,
            year,
            data->block[60].data[13],
            data->block[60].data[14]);
        if(is_metro_data_present && !furi_string_empty(metro_result)) {
            render_section_header(parsed_data, "Metro", 22, 21);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(metro_result));
        }
        if(is_ground_data_present && !furi_string_empty(ground_result)) {
            render_section_header(parsed_data, "Ground", 21, 20);
            furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(ground_result));
        }
        furi_string_free(ground_result);
        furi_string_free(metro_result);
        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin social_moscow_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = social_moscow_verify,
    .read = social_moscow_read,
    .parse = social_moscow_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor social_moscow_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &social_moscow_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* social_moscow_plugin_ep() {
    return &social_moscow_plugin_descriptor;
}
