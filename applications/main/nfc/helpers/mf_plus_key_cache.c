#include "mf_plus_key_cache.h"

#include <furi/furi.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

// Shares MIFARE Classic's /ext/nfc/.cache folder but with a distinct extension: the same UID read
// as SL1 (MIFARE Classic {UID}.keys) and as SL3 must not collide. The header is a second guard.
#define MF_PLUS_KEY_CACHE_FOLDER    "/ext/nfc/.cache"
#define MF_PLUS_KEY_CACHE_EXTENSION ".mfpkeys"

static const char* mf_plus_key_cache_file_header = "Flipper NFC Plus keys";
static const uint32_t mf_plus_key_cache_file_version = 1;

struct MfPlusKeyCache {
    MfPlusKey key_a[MF_PLUS_MAX_SECTORS];
    MfPlusKey key_b[MF_PLUS_MAX_SECTORS];
    uint64_t key_a_mask; // per-sector "key A cached" bitmap
    uint64_t key_b_mask;
    MfPlusKey admin_key[MfPlusAdminKeyNum];
    uint8_t admin_key_mask; // one bit per MfPlusAdminKeyType
};

static void mf_plus_key_cache_file_path(const uint8_t* uid, size_t uid_len, FuriString* path) {
    furi_string_printf(path, "%s/", MF_PLUS_KEY_CACHE_FOLDER);
    for(size_t i = 0; i < uid_len; i++) {
        furi_string_cat_printf(path, "%02X", uid[i]);
    }
    furi_string_cat_printf(path, "%s", MF_PLUS_KEY_CACHE_EXTENSION);
}

MfPlusKeyCache* mf_plus_key_cache_alloc(void) {
    return malloc(sizeof(MfPlusKeyCache));
}

void mf_plus_key_cache_free(MfPlusKeyCache* instance) {
    furi_assert(instance);

    free(instance);
}

void mf_plus_key_cache_reset(MfPlusKeyCache* instance) {
    furi_assert(instance);

    // Only the masks gate the getters, so clearing them is enough to invalidate stale key bytes.
    instance->key_a_mask = 0;
    instance->key_b_mask = 0;
    instance->admin_key_mask = 0;
}

bool mf_plus_key_cache_save(const MfPlusData* data) {
    furi_assert(data);

    // Nothing recovered -> nothing worth caching (avoids leaving empty cache files behind).
    if((data->key_a_mask == 0) && (data->key_b_mask == 0) && (data->admin_key_mask == 0)) {
        return true;
    }

    size_t uid_len = 0;
    const uint8_t* uid = mf_plus_get_uid(data, &uid_len);
    FuriString* file_path = furi_string_alloc();
    mf_plus_key_cache_file_path(uid, uid_len, file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    FuriString* temp_str = furi_string_alloc();
    uint64_t admin_mask = data->admin_key_mask;
    bool save_success = false;
    do {
        if(!storage_simply_mkdir(storage, MF_PLUS_KEY_CACHE_FOLDER)) break;
        if(!storage_simply_remove(storage, furi_string_get_cstr(file_path))) break;
        if(!flipper_format_buffered_file_open_always(ff, furi_string_get_cstr(file_path))) break;

        if(!flipper_format_write_header_cstr(
               ff, mf_plus_key_cache_file_header, mf_plus_key_cache_file_version))
            break;
        if(!flipper_format_write_string_cstr(
               ff, "Mifare Plus type", mf_plus_get_device_name(data, NfcDeviceNameTypeShort)))
            break;
        if(!flipper_format_write_hex_uint64(ff, "Key A map", &data->key_a_mask, 1)) break;
        if(!flipper_format_write_hex_uint64(ff, "Key B map", &data->key_b_mask, 1)) break;
        if(!flipper_format_write_hex_uint64(ff, "Admin map", &admin_mask, 1)) break;

        bool key_save_success = true;
        for(size_t i = 0; (i < MF_PLUS_MAX_SECTORS) && key_save_success; i++) {
            if(FURI_BIT(data->key_a_mask, i)) {
                furi_string_printf(temp_str, "Key A sector %d", i);
                key_save_success = flipper_format_write_hex(
                    ff, furi_string_get_cstr(temp_str), data->key_a[i].data, MF_PLUS_KEY_SIZE);
            }
            if(!key_save_success) break;
            if(FURI_BIT(data->key_b_mask, i)) {
                furi_string_printf(temp_str, "Key B sector %d", i);
                key_save_success = flipper_format_write_hex(
                    ff, furi_string_get_cstr(temp_str), data->key_b[i].data, MF_PLUS_KEY_SIZE);
            }
        }
        for(size_t i = 0; (i < MfPlusAdminKeyNum) && key_save_success; i++) {
            if(FURI_BIT(data->admin_key_mask, i)) {
                furi_string_printf(temp_str, "Admin key %d", i);
                key_save_success = flipper_format_write_hex(
                    ff, furi_string_get_cstr(temp_str), data->admin_key[i].data, MF_PLUS_KEY_SIZE);
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

bool mf_plus_key_cache_load(MfPlusKeyCache* instance, const uint8_t* uid, size_t uid_len) {
    furi_assert(instance);
    furi_assert(uid);

    mf_plus_key_cache_reset(instance);

    FuriString* file_path = furi_string_alloc();
    mf_plus_key_cache_file_path(uid, uid_len, file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    FuriString* temp_str = furi_string_alloc();
    uint64_t admin_mask = 0;
    bool load_success = false;
    do {
        if(!flipper_format_buffered_file_open_existing(ff, furi_string_get_cstr(file_path))) break;

        uint32_t version = 0;
        if(!flipper_format_read_header(ff, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, mf_plus_key_cache_file_header)) break;
        if(version != mf_plus_key_cache_file_version) break;

        if(!flipper_format_read_hex_uint64(ff, "Key A map", &instance->key_a_mask, 1)) break;
        if(!flipper_format_read_hex_uint64(ff, "Key B map", &instance->key_b_mask, 1)) break;
        if(!flipper_format_read_hex_uint64(ff, "Admin map", &admin_mask, 1)) break;
        instance->admin_key_mask = (uint8_t)admin_mask;

        bool key_read_success = true;
        for(size_t i = 0; (i < MF_PLUS_MAX_SECTORS) && key_read_success; i++) {
            if(FURI_BIT(instance->key_a_mask, i)) {
                furi_string_printf(temp_str, "Key A sector %d", i);
                key_read_success = flipper_format_read_hex(
                    ff, furi_string_get_cstr(temp_str), instance->key_a[i].data, MF_PLUS_KEY_SIZE);
            }
            if(!key_read_success) break;
            if(FURI_BIT(instance->key_b_mask, i)) {
                furi_string_printf(temp_str, "Key B sector %d", i);
                key_read_success = flipper_format_read_hex(
                    ff, furi_string_get_cstr(temp_str), instance->key_b[i].data, MF_PLUS_KEY_SIZE);
            }
        }
        for(size_t i = 0; (i < MfPlusAdminKeyNum) && key_read_success; i++) {
            if(FURI_BIT(instance->admin_key_mask, i)) {
                furi_string_printf(temp_str, "Admin key %d", i);
                key_read_success = flipper_format_read_hex(
                    ff,
                    furi_string_get_cstr(temp_str),
                    instance->admin_key[i].data,
                    MF_PLUS_KEY_SIZE);
            }
        }
        load_success = key_read_success;
    } while(false);

    // A partially-parsed file must not surface half a keyset; drop everything on any error.
    if(!load_success) mf_plus_key_cache_reset(instance);

    flipper_format_buffered_file_close(ff);
    flipper_format_free(ff);
    furi_string_free(temp_str);
    furi_string_free(file_path);
    furi_record_close(RECORD_STORAGE);

    return load_success;
}

bool mf_plus_key_cache_get_sector_key(
    const MfPlusKeyCache* instance,
    uint8_t sector,
    MfPlusKeyType key_type,
    MfPlusKey* out_key) {
    furi_assert(instance);
    furi_assert(out_key);

    if(sector >= MF_PLUS_MAX_SECTORS) return false;

    if(key_type == MfPlusKeyTypeB) {
        if(!FURI_BIT(instance->key_b_mask, sector)) return false;
        *out_key = instance->key_b[sector];
    } else {
        if(!FURI_BIT(instance->key_a_mask, sector)) return false;
        *out_key = instance->key_a[sector];
    }

    return true;
}

bool mf_plus_key_cache_get_admin_key(
    const MfPlusKeyCache* instance,
    MfPlusAdminKeyType admin_type,
    MfPlusKey* out_key) {
    furi_assert(instance);
    furi_assert(out_key);

    if(admin_type >= MfPlusAdminKeyNum) return false;
    if(!FURI_BIT(instance->admin_key_mask, admin_type)) return false;
    *out_key = instance->admin_key[admin_type];

    return true;
}
