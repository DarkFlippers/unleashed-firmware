#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>

#define TAG      "Skylanders"
#define POLY     UINT64_C(0x42f0e1eba9ea3693)
#define TOP      UINT64_C(0x800000000000)
#define UID_LEN  4
#define KEY_MASK 0xFFFFFFFFFFFF

static const uint64_t skylanders_key = 0x4b0b20107ccb;

static const char* nfc_resources_header = "Flipper NFC resources";
static const uint32_t nfc_resources_file_version = 1;

uint64_t crc64_like(uint64_t result, uint8_t sector) {
    result ^= (uint64_t)sector << 40;
    for(int i = 0; i < 8; i++) {
        result = (result & TOP) ? (result << 1) ^ POLY : result << 1;
    }
    return result;
}

uint64_t taghash(uint32_t uid) {
    uint64_t result = 0x9AE903260CC4;
    uint8_t uidBytes[UID_LEN] = {0};
    memcpy(uidBytes, &uid, UID_LEN);

    for(int i = 0; i < UID_LEN; i++) {
        result = crc64_like(result, uidBytes[i]);
    }
    return result;
}

static bool skylanders_search_data(
    Storage* storage,
    const char* file_name,
    FuriString* key,
    FuriString* data) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str;
    temp_str = furi_string_alloc();

    do {
        // Open file
        if(!flipper_format_file_open_existing(file, file_name)) break;
        // Read file header and version
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, nfc_resources_header) ||
           (version != nfc_resources_file_version))
            break;
        if(!flipper_format_read_string(file, furi_string_get_cstr(key), data)) break;
        parsed = true;
    } while(false);

    furi_string_free(temp_str);
    flipper_format_free(file);
    return parsed;
}

bool skylanders_get_name(Storage* storage, uint16_t id, FuriString* name) {
    bool parsed = false;
    FuriString* key;
    key = furi_string_alloc_printf("%04X", id);
    if(skylanders_search_data(storage, EXT_PATH("nfc/assets/skylanders.nfc"), key, name)) {
        parsed = true;
    }
    furi_string_free(key);
    return parsed;
}

bool skylanders_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 0;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(skylanders_key, COUNT_OF(key.data), key.data);

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

static bool skylanders_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    size_t* uid_len = 0;
    const uint8_t* uid_bytes = mf_classic_get_uid(data, uid_len);
    uint32_t uid = 0;
    memcpy(&uid, uid_bytes, sizeof(uid));
    uint64_t hash = taghash(uid);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            if(i == 0) {
                bit_lib_num_to_bytes_be(skylanders_key, sizeof(MfClassicKey), keys.key_a[i].data);
                FURI_BIT_SET(keys.key_a_mask, i);
            } else {
                uint64_t sectorhash = crc64_like(hash, i);
                uint64_t key = sectorhash & KEY_MASK;
                uint8_t* keyBytes = (uint8_t*)&key;
                memcpy(keys.key_a[i].data, keyBytes, sizeof(MfClassicKey));
                FURI_BIT_SET(keys.key_a_mask, i);
                memset(keys.key_b[i].data, 0, sizeof(MfClassicKey));
                FURI_BIT_SET(keys.key_b_mask, i);
            }
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

static bool skylanders_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;
    FuriString* name = furi_string_alloc();

    do {
        // verify key
        const uint8_t verify_sector = 0;
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, verify_sector);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != skylanders_key) break;

        const uint16_t id = data->block[1].data[1] << 8 | data->block[1].data[0];
        if(id == 0) break;

        Storage* storage = furi_record_open(RECORD_STORAGE);

        bool success = skylanders_get_name(storage, id, name);

        furi_record_close(RECORD_STORAGE);
        if(!success) break;

        furi_string_printf(parsed_data, "\e#Skylanders\n%s", furi_string_get_cstr(name));

        parsed = true;

    } while(false);

    furi_string_free(name);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin skylanders_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = skylanders_verify,
    .read = skylanders_read,
    .parse = skylanders_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor skylanders_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &skylanders_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* skylanders_plugin_ep(void) {
    return &skylanders_plugin_descriptor;
}
