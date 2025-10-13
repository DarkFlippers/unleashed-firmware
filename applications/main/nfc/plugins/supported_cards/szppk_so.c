#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "SZPPK_SO"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    const MfClassicKeyPair* keys;
    uint32_t data_sector;
} SOCardConfig;

typedef struct {
    uint16_t station_id;
    const char* station_name;
} StationMap;

const StationMap station_map[] = {
    {0x099C, "DEVYATKINO"},
    {0x2494, "FINBAN"},
    {0x3e99, "KAVGOLOVO"},
    {0x2194, "MOSBAN"},
    {0x1295, "TOKSOVO"},
    {0x4998, "UDEL'NAYA"},
    {0x1D97, "ST.DEREVNYA"},

    // Here'll be other stations someday

};

const size_t num_station_map_entries = sizeof(station_map) / sizeof(station_map[0]);

static const MfClassicKeyPair so_card_2k[] = {
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B5B8B4B5}, //0
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B6B2B9B8B7}, //1
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B2B4B4B4B0}, //2
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B3B5B2B2B2}, //3
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B8B4B8B0B9}, //4
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B3B7B9B8B6}, //5
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B6B5B5B5B6}, //6
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B0B0B2B0}, //7
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B1B2B8B0}, //8
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B1B8B3B2}, //9
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B0B3B1B8}, //10
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B2B6B7B5B2}, //11
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B9B2B0B2B0}, //12
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B8B8B7B3}, //13
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B3B4B9B2B0}, //14
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B2B0B0B9}, //15
    {.a = 0xA94CAB611187, .b = 0x389109BD1D82}, //16
    {.a = 0xBF4280329F11, .b = 0x28D9EDD2096D}, //17
    {.a = 0xDE6BD90BD6B0, .b = 0x94866C16E9A4}, //18
    {.a = 0x2EA9493CAA7C, .b = 0x5068BCE2BC1C}, //19
    {.a = 0x15A41BA53F6C, .b = 0x3BD3CF43571C}, //20
    {.a = 0x1290FFD80DB5, .b = 0xD821B7916B7E}, //21
    {.a = 0x68C1A07E96A9, .b = 0x2B3323E75750}, //22
    {.a = 0xC699831AB307, .b = 0xCD7F7E9111F1}, //23
    {.a = 0x4E5884BF23E9, .b = 0x2287812A6AEE}, //24
    {.a = 0xC55212F716DC, .b = 0x594E368CCEFF}, //25
    {.a = 0x6EF127E674B1, .b = 0xDD21C8D3E0B9}, //26
    {.a = 0xFB79FAF4B55C, .b = 0xFE52B3B2A93B}, //27
    {.a = 0x6CF85CDFF647, .b = 0xCCAD7C41FC8A}, //28
    {.a = 0x591F6C130F91, .b = 0x2D2B734ECF91}, //29
    {.a = 0xEEB83529B79B, .b = 0xCB14E70EBA38}, //30
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //31

};

bool szppk_so_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 19;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(so_card_2k[verify_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        } else {
            FURI_LOG_D(TAG, "Auth success, this is so card");
        }

        verified = true;
    } while(false);

    return verified;
}

static bool szppk_so_read(Nfc* nfc, NfcDevice* device) {
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
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < /*mf_classic_get_total_sectors_num(data->type)*/ 32; i++) {
            bit_lib_num_to_bytes_be(so_card_2k[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(so_card_2k[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

static bool szppk_so_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;
    do {
        // Verify key
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 19);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != so_card_2k[19].a) break;

        uint16_t origin = (data->block[76].data[5] << 8) | (data->block[76].data[6]);
        bool passes_remain = data->block[77].data[0];
        bool station_is_known = 0;
        furi_string_cat_printf(parsed_data, "\e#SZPPK Accompany Card\n");
        for(size_t i = 0; i < num_station_map_entries; ++i) {
            if(origin == station_map[i].station_id) {
                // Found a match, print the corresponding word
                furi_string_cat_printf(
                    parsed_data, "Station: > %s\n", station_map[i].station_name);
                station_is_known = 1;
            } // Exit the function once found
        }

        if(station_is_known == 0)
            furi_string_cat_printf(parsed_data, "Station of origin: %04x\n", origin);

        if(passes_remain == 1) {
            furi_string_cat_printf(parsed_data, "Was it used: No\n");
        } else {
            furi_string_cat_printf(parsed_data, "Was it used: Yes\n");
        };
        /*Test s16b0
            const uint8_t* temp_ptr = data->block[74].data;
            uint8_t s16b0_arr[4] = {0};
            for(size_t i = 0; i < 5; i++) {
            s16b0_arr[i] = temp_ptr[15 - i];
        }
        uint64_t s16b0 = 0;
        for(size_t i = 0; i <5; i++) {
            s16b0 = (s16b0 << 8) | s16b0_arr[i];
        }

            furi_string_cat_printf(parsed_data, "SYS N:> %lld\n", s16b0);*/
        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin szppk_so_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = szppk_so_verify,
    .read = szppk_so_read,
    .parse = szppk_so_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor szppk_so_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &szppk_so_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* szppk_so_plugin_ep(void) {
    return &szppk_so_plugin_descriptor;
}
