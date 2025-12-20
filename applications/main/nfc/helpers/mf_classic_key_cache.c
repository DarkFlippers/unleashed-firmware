#include "mf_classic_key_cache.h"

#include <furi/furi.h>
#include <storage/storage.h>

#define NFC_APP_KEYS_EXTENSION   ".keys"
#define NFC_APP_KEY_CACHE_FOLDER "/ext/nfc/.cache"

static const char* mf_classic_key_cache_file_header = "Flipper NFC keys";
static const uint32_t mf_classic_key_cache_file_version = 1;

struct MfClassicKeyCache {
    MfClassicDeviceKeys keys;
    MfClassicKeyType current_key_type;
    uint8_t current_sector;
};

static void nfc_get_key_cache_file_path(const uint8_t* uid, size_t uid_len, FuriString* path) {
    furi_string_printf(path, "%s/", NFC_APP_KEY_CACHE_FOLDER);
    for(size_t i = 0; i < uid_len; i++) {
        furi_string_cat_printf(path, "%02X", uid[i]);
    }
    furi_string_cat_printf(path, "%s", NFC_APP_KEYS_EXTENSION);
}

MfClassicKeyCache* mf_classic_key_cache_alloc(void) {
    MfClassicKeyCache* instance = malloc(sizeof(MfClassicKeyCache));

    return instance;
}

void mf_classic_key_cache_free(MfClassicKeyCache* instance) {
    furi_assert(instance);

    free(instance);
}

bool mf_classic_key_cache_save(MfClassicKeyCache* instance, const MfClassicData* data) {
    UNUSED(instance);
    furi_assert(data);

    size_t uid_len = 0;
    const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
    FuriString* file_path = furi_string_alloc();
    nfc_get_key_cache_file_path(uid, uid_len, file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    FuriString* temp_str = furi_string_alloc();
    bool save_success = false;
    do {
        if(!storage_simply_mkdir(storage, NFC_APP_KEY_CACHE_FOLDER)) break;
        if(!storage_simply_remove(storage, furi_string_get_cstr(file_path))) break;
        if(!flipper_format_buffered_file_open_always(ff, furi_string_get_cstr(file_path))) break;

        if(!flipper_format_write_header_cstr(
               ff, mf_classic_key_cache_file_header, mf_classic_key_cache_file_version))
            break;
        if(!flipper_format_write_string_cstr(
               ff, "Mifare Classic type", mf_classic_get_device_name(data, NfcDeviceNameTypeShort)))
            break;
        if(!flipper_format_write_hex_uint64(ff, "Key A map", &data->key_a_mask, 1)) break;
        if(!flipper_format_write_hex_uint64(ff, "Key B map", &data->key_b_mask, 1)) break;

        uint8_t sector_num = mf_classic_get_total_sectors_num(data->type);
        bool key_save_success = true;
        for(size_t i = 0; (i < sector_num) && (key_save_success); i++) {
            MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, i);
            if(FURI_BIT(data->key_a_mask, i)) {
                furi_string_printf(temp_str, "Key A sector %d", i);
                key_save_success = flipper_format_write_hex(
                    ff, furi_string_get_cstr(temp_str), sec_tr->key_a.data, sizeof(MfClassicKey));
            }
            if(!key_save_success) break;
            if(FURI_BIT(data->key_b_mask, i)) {
                furi_string_printf(temp_str, "Key B sector %d", i);
                key_save_success = flipper_format_write_hex(
                    ff, furi_string_get_cstr(temp_str), sec_tr->key_b.data, sizeof(MfClassicKey));
            }
        }
        save_success = key_save_success;
    } while(false);

    flipper_format_free(ff);
    furi_string_free(temp_str);
    furi_string_free(file_path);
    furi_record_close(RECORD_STORAGE);

    return save_success;
}

bool mf_classic_key_cache_load(MfClassicKeyCache* instance, const uint8_t* uid, size_t uid_len) {
    furi_assert(instance);
    furi_assert(uid);

    mf_classic_key_cache_reset(instance);

    FuriString* file_path = furi_string_alloc();
    nfc_get_key_cache_file_path(uid, uid_len, file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    FuriString* temp_str = furi_string_alloc();
    bool load_success = false;
    do {
        if(!flipper_format_buffered_file_open_existing(ff, furi_string_get_cstr(file_path))) break;

        uint32_t version = 0;
        if(!flipper_format_read_header(ff, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, mf_classic_key_cache_file_header)) break;
        if(version != mf_classic_key_cache_file_version) break;

        if(!flipper_format_read_hex_uint64(ff, "Key A map", &instance->keys.key_a_mask, 1)) break;
        if(!flipper_format_read_hex_uint64(ff, "Key B map", &instance->keys.key_b_mask, 1)) break;

        bool key_read_success = true;
        for(size_t i = 0; (i < MF_CLASSIC_TOTAL_SECTORS_MAX) && (key_read_success); i++) {
            if(FURI_BIT(instance->keys.key_a_mask, i)) {
                furi_string_printf(temp_str, "Key A sector %d", i);
                key_read_success = flipper_format_read_hex(
                    ff,
                    furi_string_get_cstr(temp_str),
                    instance->keys.key_a[i].data,
                    sizeof(MfClassicKey));
            }
            if(!key_read_success) break;
            if(FURI_BIT(instance->keys.key_b_mask, i)) {
                furi_string_printf(temp_str, "Key B sector %d", i);
                key_read_success = flipper_format_read_hex(
                    ff,
                    furi_string_get_cstr(temp_str),
                    instance->keys.key_b[i].data,
                    sizeof(MfClassicKey));
            }
        }
        load_success = key_read_success;
    } while(false);

    flipper_format_buffered_file_close(ff);
    flipper_format_free(ff);
    furi_string_free(temp_str);
    furi_string_free(file_path);
    furi_record_close(RECORD_STORAGE);

    return load_success;
}

void mf_classic_key_cache_load_from_data(MfClassicKeyCache* instance, const MfClassicData* data) {
    furi_assert(instance);
    furi_assert(data);

    mf_classic_key_cache_reset(instance);
    instance->keys.key_a_mask = data->key_a_mask;
    instance->keys.key_b_mask = data->key_b_mask;
    for(size_t i = 0; i < MF_CLASSIC_TOTAL_SECTORS_MAX; i++) {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, i);

        if(FURI_BIT(data->key_a_mask, i)) {
            instance->keys.key_a[i] = sec_tr->key_a;
        }
        if(FURI_BIT(data->key_b_mask, i)) {
            instance->keys.key_b[i] = sec_tr->key_b;
        }
    }
}

bool mf_classic_key_cache_get_next_key(
    MfClassicKeyCache* instance,
    uint8_t* sector_num,
    MfClassicKey* key,
    MfClassicKeyType* key_type) {
    furi_assert(instance);
    furi_assert(sector_num);
    furi_assert(key);
    furi_assert(key_type);

    bool next_key_found = false;
    for(uint8_t i = instance->current_sector; i < MF_CLASSIC_TOTAL_SECTORS_MAX; i++) {
        if(FURI_BIT(instance->keys.key_a_mask, i)) {
            FURI_BIT_CLEAR(instance->keys.key_a_mask, i);
            *key = instance->keys.key_a[i];
            *key_type = MfClassicKeyTypeA;
            *sector_num = i;

            next_key_found = true;
            break;
        }
        if(FURI_BIT(instance->keys.key_b_mask, i)) {
            FURI_BIT_CLEAR(instance->keys.key_b_mask, i);
            *key = instance->keys.key_b[i];
            *key_type = MfClassicKeyTypeB;
            *sector_num = i;

            next_key_found = true;
            instance->current_sector = i;
            break;
        }
    }

    return next_key_found;
}

void mf_classic_key_cache_reset(MfClassicKeyCache* instance) {
    furi_assert(instance);

    instance->current_key_type = MfClassicKeyTypeA;
    instance->current_sector = 0;
    instance->keys.key_a_mask = 0;
    instance->keys.key_b_mask = 0;
}
