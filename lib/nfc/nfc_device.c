#include "nfc_device.h"
#include "assets_icons.h"
#include "m-string.h"
#include "nfc_types.h"

#include <lib/toolbox/path.h>
#include <lib/toolbox/hex.h>
#include <lib/nfc/protocols/nfc_util.h>
#include <flipper_format/flipper_format.h>

#define NFC_DEVICE_KEYS_FOLDER EXT_PATH("nfc/cache")
#define NFC_DEVICE_KEYS_EXTENSION ".keys"

static const char* nfc_file_header = "Flipper NFC device";
static const uint32_t nfc_file_version = 2;

static const char* nfc_keys_file_header = "Flipper NFC keys";
static const uint32_t nfc_keys_file_version = 1;

// Protocols format versions
static const uint32_t nfc_mifare_classic_data_format_version = 2;
static const uint32_t nfc_mifare_ultralight_data_format_version = 1;

NfcDevice* nfc_device_alloc() {
    NfcDevice* nfc_dev = malloc(sizeof(NfcDevice));
    nfc_dev->storage = furi_record_open(RECORD_STORAGE);
    nfc_dev->dialogs = furi_record_open(RECORD_DIALOGS);
    string_init(nfc_dev->load_path);
    string_init(nfc_dev->dev_data.parsed_data);
    return nfc_dev;
}

void nfc_device_free(NfcDevice* nfc_dev) {
    furi_assert(nfc_dev);
    nfc_device_clear(nfc_dev);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);
    string_clear(nfc_dev->load_path);
    string_clear(nfc_dev->dev_data.parsed_data);
    free(nfc_dev);
}

static void nfc_device_prepare_format_string(NfcDevice* dev, string_t format_string) {
    if(dev->format == NfcDeviceSaveFormatUid) {
        string_set_str(format_string, "UID");
    } else if(dev->format == NfcDeviceSaveFormatBankCard) {
        string_set_str(format_string, "Bank card");
    } else if(dev->format == NfcDeviceSaveFormatMifareUl) {
        string_set_str(format_string, nfc_mf_ul_type(dev->dev_data.mf_ul_data.type, true));
    } else if(dev->format == NfcDeviceSaveFormatMifareClassic) {
        string_set_str(format_string, "Mifare Classic");
    } else if(dev->format == NfcDeviceSaveFormatMifareDesfire) {
        string_set_str(format_string, "Mifare DESFire");
    } else {
        string_set_str(format_string, "Unknown");
    }
}

static bool nfc_device_parse_format_string(NfcDevice* dev, string_t format_string) {
    if(string_start_with_str_p(format_string, "UID")) {
        dev->format = NfcDeviceSaveFormatUid;
        dev->dev_data.protocol = NfcDeviceProtocolUnknown;
        return true;
    }
    if(string_start_with_str_p(format_string, "Bank card")) {
        dev->format = NfcDeviceSaveFormatBankCard;
        dev->dev_data.protocol = NfcDeviceProtocolEMV;
        return true;
    }
    // Check Mifare Ultralight types
    for(MfUltralightType type = MfUltralightTypeUnknown; type < MfUltralightTypeNum; type++) {
        if(string_equal_str_p(format_string, nfc_mf_ul_type(type, true))) {
            dev->format = NfcDeviceSaveFormatMifareUl;
            dev->dev_data.protocol = NfcDeviceProtocolMifareUl;
            dev->dev_data.mf_ul_data.type = type;
            return true;
        }
    }
    if(string_start_with_str_p(format_string, "Mifare Classic")) {
        dev->format = NfcDeviceSaveFormatMifareClassic;
        dev->dev_data.protocol = NfcDeviceProtocolMifareClassic;
        return true;
    }
    if(string_start_with_str_p(format_string, "Mifare DESFire")) {
        dev->format = NfcDeviceSaveFormatMifareDesfire;
        dev->dev_data.protocol = NfcDeviceProtocolMifareDesfire;
        return true;
    }
    return false;
}

static bool nfc_device_save_mifare_ul_data(FlipperFormat* file, NfcDevice* dev) {
    bool saved = false;
    MfUltralightData* data = &dev->dev_data.mf_ul_data;
    string_t temp_str;
    string_init(temp_str);

    // Save Mifare Ultralight specific data
    do {
        if(!flipper_format_write_comment_cstr(file, "Mifare Ultralight specific data")) break;
        if(!flipper_format_write_uint32(
               file, "Data format version", &nfc_mifare_ultralight_data_format_version, 1))
            break;
        if(!flipper_format_write_hex(file, "Signature", data->signature, sizeof(data->signature)))
            break;
        if(!flipper_format_write_hex(
               file, "Mifare version", (uint8_t*)&data->version, sizeof(data->version)))
            break;
        // Write conters and tearing flags data
        bool counters_saved = true;
        for(uint8_t i = 0; i < 3; i++) {
            string_printf(temp_str, "Counter %d", i);
            if(!flipper_format_write_uint32(
                   file, string_get_cstr(temp_str), &data->counter[i], 1)) {
                counters_saved = false;
                break;
            }
            string_printf(temp_str, "Tearing %d", i);
            if(!flipper_format_write_hex(file, string_get_cstr(temp_str), &data->tearing[i], 1)) {
                counters_saved = false;
                break;
            }
        }
        if(!counters_saved) break;
        // Write pages data
        uint32_t pages_total = data->data_size / 4;
        if(!flipper_format_write_uint32(file, "Pages total", &pages_total, 1)) break;
        uint32_t pages_read = data->data_read / 4;
        if(!flipper_format_write_uint32(file, "Pages read", &pages_read, 1)) break;
        bool pages_saved = true;
        for(uint16_t i = 0; i < data->data_size; i += 4) {
            string_printf(temp_str, "Page %d", i / 4);
            if(!flipper_format_write_hex(file, string_get_cstr(temp_str), &data->data[i], 4)) {
                pages_saved = false;
                break;
            }
        }
        if(!pages_saved) break;

        // Write authentication counter
        uint32_t auth_counter = data->curr_authlim;
        if(!flipper_format_write_uint32(file, "Failed authentication attempts", &auth_counter, 1))
            break;

        saved = true;
    } while(false);

    string_clear(temp_str);
    return saved;
}

bool nfc_device_load_mifare_ul_data(FlipperFormat* file, NfcDevice* dev) {
    bool parsed = false;
    MfUltralightData* data = &dev->dev_data.mf_ul_data;
    string_t temp_str;
    string_init(temp_str);
    uint32_t data_format_version = 0;

    do {
        // Read Mifare Ultralight format version
        if(!flipper_format_read_uint32(file, "Data format version", &data_format_version, 1)) {
            if(!flipper_format_rewind(file)) break;
        }

        // Read signature
        if(!flipper_format_read_hex(file, "Signature", data->signature, sizeof(data->signature)))
            break;
        // Read Mifare version
        if(!flipper_format_read_hex(
               file, "Mifare version", (uint8_t*)&data->version, sizeof(data->version)))
            break;
        // Read counters and tearing flags
        bool counters_parsed = true;
        for(uint8_t i = 0; i < 3; i++) {
            string_printf(temp_str, "Counter %d", i);
            if(!flipper_format_read_uint32(file, string_get_cstr(temp_str), &data->counter[i], 1)) {
                counters_parsed = false;
                break;
            }
            string_printf(temp_str, "Tearing %d", i);
            if(!flipper_format_read_hex(file, string_get_cstr(temp_str), &data->tearing[i], 1)) {
                counters_parsed = false;
                break;
            }
        }
        if(!counters_parsed) break;
        // Read pages
        uint32_t pages_total = 0;
        if(!flipper_format_read_uint32(file, "Pages total", &pages_total, 1)) break;
        uint32_t pages_read = 0;
        if(data_format_version < nfc_mifare_ultralight_data_format_version) {
            pages_read = pages_total;
        } else {
            if(!flipper_format_read_uint32(file, "Pages read", &pages_read, 1)) break;
        }
        data->data_size = pages_total * 4;
        data->data_read = pages_read * 4;
        if(data->data_size > MF_UL_MAX_DUMP_SIZE || data->data_read > MF_UL_MAX_DUMP_SIZE) break;
        bool pages_parsed = true;
        for(uint16_t i = 0; i < pages_total; i++) {
            string_printf(temp_str, "Page %d", i);
            if(!flipper_format_read_hex(file, string_get_cstr(temp_str), &data->data[i * 4], 4)) {
                pages_parsed = false;
                break;
            }
        }
        if(!pages_parsed) break;

        // Read authentication counter
        uint32_t auth_counter;
        if(!flipper_format_read_uint32(file, "Failed authentication attempts", &auth_counter, 1))
            auth_counter = 0;

        parsed = true;
    } while(false);

    string_clear(temp_str);
    return parsed;
}

static bool nfc_device_save_mifare_df_key_settings(
    FlipperFormat* file,
    MifareDesfireKeySettings* ks,
    const char* prefix) {
    bool saved = false;
    string_t key;
    string_init(key);

    do {
        string_printf(key, "%s Change Key ID", prefix);
        if(!flipper_format_write_hex(file, string_get_cstr(key), &ks->change_key_id, 1)) break;
        string_printf(key, "%s Config Changeable", prefix);
        if(!flipper_format_write_bool(file, string_get_cstr(key), &ks->config_changeable, 1))
            break;
        string_printf(key, "%s Free Create Delete", prefix);
        if(!flipper_format_write_bool(file, string_get_cstr(key), &ks->free_create_delete, 1))
            break;
        string_printf(key, "%s Free Directory List", prefix);
        if(!flipper_format_write_bool(file, string_get_cstr(key), &ks->free_directory_list, 1))
            break;
        string_printf(key, "%s Key Changeable", prefix);
        if(!flipper_format_write_bool(file, string_get_cstr(key), &ks->master_key_changeable, 1))
            break;
        if(ks->flags) {
            string_printf(key, "%s Flags", prefix);
            if(!flipper_format_write_hex(file, string_get_cstr(key), &ks->flags, 1)) break;
        }
        string_printf(key, "%s Max Keys", prefix);
        if(!flipper_format_write_hex(file, string_get_cstr(key), &ks->max_keys, 1)) break;
        for(MifareDesfireKeyVersion* kv = ks->key_version_head; kv; kv = kv->next) {
            string_printf(key, "%s Key %d Version", prefix, kv->id);
            if(!flipper_format_write_hex(file, string_get_cstr(key), &kv->version, 1)) break;
        }
        saved = true;
    } while(false);

    string_clear(key);
    return saved;
}

bool nfc_device_load_mifare_df_key_settings(
    FlipperFormat* file,
    MifareDesfireKeySettings* ks,
    const char* prefix) {
    bool parsed = false;
    string_t key;
    string_init(key);

    do {
        string_printf(key, "%s Change Key ID", prefix);
        if(!flipper_format_read_hex(file, string_get_cstr(key), &ks->change_key_id, 1)) break;
        string_printf(key, "%s Config Changeable", prefix);
        if(!flipper_format_read_bool(file, string_get_cstr(key), &ks->config_changeable, 1)) break;
        string_printf(key, "%s Free Create Delete", prefix);
        if(!flipper_format_read_bool(file, string_get_cstr(key), &ks->free_create_delete, 1))
            break;
        string_printf(key, "%s Free Directory List", prefix);
        if(!flipper_format_read_bool(file, string_get_cstr(key), &ks->free_directory_list, 1))
            break;
        string_printf(key, "%s Key Changeable", prefix);
        if(!flipper_format_read_bool(file, string_get_cstr(key), &ks->master_key_changeable, 1))
            break;
        string_printf(key, "%s Flags", prefix);
        if(flipper_format_key_exist(file, string_get_cstr(key))) {
            if(!flipper_format_read_hex(file, string_get_cstr(key), &ks->flags, 1)) break;
        }
        string_printf(key, "%s Max Keys", prefix);
        if(!flipper_format_read_hex(file, string_get_cstr(key), &ks->max_keys, 1)) break;
        ks->flags |= ks->max_keys >> 4;
        ks->max_keys &= 0xF;
        MifareDesfireKeyVersion** kv_head = &ks->key_version_head;
        for(int key_id = 0; key_id < ks->max_keys; key_id++) {
            string_printf(key, "%s Key %d Version", prefix, key_id);
            uint8_t version;
            if(flipper_format_read_hex(file, string_get_cstr(key), &version, 1)) {
                MifareDesfireKeyVersion* kv = malloc(sizeof(MifareDesfireKeyVersion));
                memset(kv, 0, sizeof(MifareDesfireKeyVersion));
                kv->id = key_id;
                kv->version = version;
                *kv_head = kv;
                kv_head = &kv->next;
            }
        }
        parsed = true;
    } while(false);

    string_clear(key);
    return parsed;
}

static bool nfc_device_save_mifare_df_app(FlipperFormat* file, MifareDesfireApplication* app) {
    bool saved = false;
    string_t prefix, key;
    string_init_printf(prefix, "Application %02x%02x%02x", app->id[0], app->id[1], app->id[2]);
    string_init(key);
    uint8_t* tmp = NULL;

    do {
        if(app->key_settings) {
            if(!nfc_device_save_mifare_df_key_settings(
                   file, app->key_settings, string_get_cstr(prefix)))
                break;
        }
        if(!app->file_head) break;
        uint32_t n_files = 0;
        for(MifareDesfireFile* f = app->file_head; f; f = f->next) {
            n_files++;
        }
        tmp = malloc(n_files);
        int i = 0;
        for(MifareDesfireFile* f = app->file_head; f; f = f->next) {
            tmp[i++] = f->id;
        }
        string_printf(key, "%s File IDs", string_get_cstr(prefix));
        if(!flipper_format_write_hex(file, string_get_cstr(key), tmp, n_files)) break;
        bool saved_files = true;
        for(MifareDesfireFile* f = app->file_head; f; f = f->next) {
            saved_files = false;
            string_printf(key, "%s File %d Type", string_get_cstr(prefix), f->id);
            if(!flipper_format_write_hex(file, string_get_cstr(key), &f->type, 1)) break;
            string_printf(
                key, "%s File %d Communication Settings", string_get_cstr(prefix), f->id);
            if(!flipper_format_write_hex(file, string_get_cstr(key), &f->comm, 1)) break;
            string_printf(key, "%s File %d Access Rights", string_get_cstr(prefix), f->id);
            if(!flipper_format_write_hex(
                   file, string_get_cstr(key), (uint8_t*)&f->access_rights, 2))
                break;
            uint16_t size = 0;
            if(f->type == MifareDesfireFileTypeStandard ||
               f->type == MifareDesfireFileTypeBackup) {
                size = f->settings.data.size;
                string_printf(key, "%s File %d Size", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_uint32(
                       file, string_get_cstr(key), &f->settings.data.size, 1))
                    break;
            } else if(f->type == MifareDesfireFileTypeValue) {
                string_printf(key, "%s File %d Hi Limit", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_uint32(
                       file, string_get_cstr(key), &f->settings.value.hi_limit, 1))
                    break;
                string_printf(key, "%s File %d Lo Limit", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_uint32(
                       file, string_get_cstr(key), &f->settings.value.lo_limit, 1))
                    break;
                string_printf(
                    key, "%s File %d Limited Credit Value", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_uint32(
                       file, string_get_cstr(key), &f->settings.value.limited_credit_value, 1))
                    break;
                string_printf(
                    key, "%s File %d Limited Credit Enabled", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_bool(
                       file, string_get_cstr(key), &f->settings.value.limited_credit_enabled, 1))
                    break;
                size = 4;
            } else if(
                f->type == MifareDesfireFileTypeLinearRecord ||
                f->type == MifareDesfireFileTypeCyclicRecord) {
                string_printf(key, "%s File %d Size", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_uint32(
                       file, string_get_cstr(key), &f->settings.record.size, 1))
                    break;
                string_printf(key, "%s File %d Max", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_uint32(
                       file, string_get_cstr(key), &f->settings.record.max, 1))
                    break;
                string_printf(key, "%s File %d Cur", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_uint32(
                       file, string_get_cstr(key), &f->settings.record.cur, 1))
                    break;
                size = f->settings.record.size * f->settings.record.cur;
            }
            if(f->contents) {
                string_printf(key, "%s File %d", string_get_cstr(prefix), f->id);
                if(!flipper_format_write_hex(file, string_get_cstr(key), f->contents, size)) break;
            }
            saved_files = true;
        }
        if(!saved_files) {
            break;
        }
        saved = true;
    } while(false);

    free(tmp);
    string_clear(prefix);
    string_clear(key);
    return saved;
}

bool nfc_device_load_mifare_df_app(FlipperFormat* file, MifareDesfireApplication* app) {
    bool parsed = false;
    string_t prefix, key;
    string_init_printf(prefix, "Application %02x%02x%02x", app->id[0], app->id[1], app->id[2]);
    string_init(key);
    uint8_t* tmp = NULL;
    MifareDesfireFile* f = NULL;

    do {
        app->key_settings = malloc(sizeof(MifareDesfireKeySettings));
        memset(app->key_settings, 0, sizeof(MifareDesfireKeySettings));
        if(!nfc_device_load_mifare_df_key_settings(
               file, app->key_settings, string_get_cstr(prefix))) {
            free(app->key_settings);
            app->key_settings = NULL;
            break;
        }
        string_printf(key, "%s File IDs", string_get_cstr(prefix));
        uint32_t n_files;
        if(!flipper_format_get_value_count(file, string_get_cstr(key), &n_files)) break;
        tmp = malloc(n_files);
        if(!flipper_format_read_hex(file, string_get_cstr(key), tmp, n_files)) break;
        MifareDesfireFile** file_head = &app->file_head;
        bool parsed_files = true;
        for(uint32_t i = 0; i < n_files; i++) {
            parsed_files = false;
            f = malloc(sizeof(MifareDesfireFile));
            memset(f, 0, sizeof(MifareDesfireFile));
            f->id = tmp[i];
            string_printf(key, "%s File %d Type", string_get_cstr(prefix), f->id);
            if(!flipper_format_read_hex(file, string_get_cstr(key), &f->type, 1)) break;
            string_printf(
                key, "%s File %d Communication Settings", string_get_cstr(prefix), f->id);
            if(!flipper_format_read_hex(file, string_get_cstr(key), &f->comm, 1)) break;
            string_printf(key, "%s File %d Access Rights", string_get_cstr(prefix), f->id);
            if(!flipper_format_read_hex(file, string_get_cstr(key), (uint8_t*)&f->access_rights, 2))
                break;
            if(f->type == MifareDesfireFileTypeStandard ||
               f->type == MifareDesfireFileTypeBackup) {
                string_printf(key, "%s File %d Size", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_uint32(
                       file, string_get_cstr(key), &f->settings.data.size, 1))
                    break;
            } else if(f->type == MifareDesfireFileTypeValue) {
                string_printf(key, "%s File %d Hi Limit", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_uint32(
                       file, string_get_cstr(key), &f->settings.value.hi_limit, 1))
                    break;
                string_printf(key, "%s File %d Lo Limit", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_uint32(
                       file, string_get_cstr(key), &f->settings.value.lo_limit, 1))
                    break;
                string_printf(
                    key, "%s File %d Limited Credit Value", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_uint32(
                       file, string_get_cstr(key), &f->settings.value.limited_credit_value, 1))
                    break;
                string_printf(
                    key, "%s File %d Limited Credit Enabled", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_bool(
                       file, string_get_cstr(key), &f->settings.value.limited_credit_enabled, 1))
                    break;
            } else if(
                f->type == MifareDesfireFileTypeLinearRecord ||
                f->type == MifareDesfireFileTypeCyclicRecord) {
                string_printf(key, "%s File %d Size", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_uint32(
                       file, string_get_cstr(key), &f->settings.record.size, 1))
                    break;
                string_printf(key, "%s File %d Max", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_uint32(
                       file, string_get_cstr(key), &f->settings.record.max, 1))
                    break;
                string_printf(key, "%s File %d Cur", string_get_cstr(prefix), f->id);
                if(!flipper_format_read_uint32(
                       file, string_get_cstr(key), &f->settings.record.cur, 1))
                    break;
            }
            string_printf(key, "%s File %d", string_get_cstr(prefix), f->id);
            if(flipper_format_key_exist(file, string_get_cstr(key))) {
                uint32_t size;
                if(!flipper_format_get_value_count(file, string_get_cstr(key), &size)) break;
                f->contents = malloc(size);
                if(!flipper_format_read_hex(file, string_get_cstr(key), f->contents, size)) break;
            }
            *file_head = f;
            file_head = &f->next;
            f = NULL;
            parsed_files = true;
        }
        if(!parsed_files) {
            break;
        }
        parsed = true;
    } while(false);

    if(f) {
        free(f->contents);
        free(f);
    }
    free(tmp);
    string_clear(prefix);
    string_clear(key);
    return parsed;
}

static bool nfc_device_save_mifare_df_data(FlipperFormat* file, NfcDevice* dev) {
    bool saved = false;
    MifareDesfireData* data = &dev->dev_data.mf_df_data;
    uint8_t* tmp = NULL;

    do {
        if(!flipper_format_write_comment_cstr(file, "Mifare DESFire specific data")) break;
        if(!flipper_format_write_hex(
               file, "PICC Version", (uint8_t*)&data->version, sizeof(data->version)))
            break;
        if(data->free_memory) {
            if(!flipper_format_write_uint32(file, "PICC Free Memory", &data->free_memory->bytes, 1))
                break;
        }
        if(data->master_key_settings) {
            if(!nfc_device_save_mifare_df_key_settings(file, data->master_key_settings, "PICC"))
                break;
        }
        uint32_t n_apps = 0;
        for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
            n_apps++;
        }
        if(!flipper_format_write_uint32(file, "Application Count", &n_apps, 1)) break;
        if(n_apps) {
            tmp = malloc(n_apps * 3);
            int i = 0;
            for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
                memcpy(tmp + i, app->id, 3);
                i += 3;
            }
            if(!flipper_format_write_hex(file, "Application IDs", tmp, n_apps * 3)) break;
            for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
                if(!nfc_device_save_mifare_df_app(file, app)) break;
            }
        }
        saved = true;
    } while(false);

    free(tmp);
    return saved;
}

bool nfc_device_load_mifare_df_data(FlipperFormat* file, NfcDevice* dev) {
    bool parsed = false;
    MifareDesfireData* data = &dev->dev_data.mf_df_data;
    memset(data, 0, sizeof(MifareDesfireData));
    uint8_t* tmp = NULL;

    do {
        if(!flipper_format_read_hex(
               file, "PICC Version", (uint8_t*)&data->version, sizeof(data->version)))
            break;
        if(flipper_format_key_exist(file, "PICC Free Memory")) {
            data->free_memory = malloc(sizeof(MifareDesfireFreeMemory));
            memset(data->free_memory, 0, sizeof(MifareDesfireFreeMemory));
            if(!flipper_format_read_uint32(
                   file, "PICC Free Memory", &data->free_memory->bytes, 1)) {
                free(data->free_memory);
                break;
            }
        }
        if(flipper_format_key_exist(file, "PICC Change Key ID")) {
            data->master_key_settings = malloc(sizeof(MifareDesfireKeySettings));
            memset(data->master_key_settings, 0, sizeof(MifareDesfireKeySettings));
            if(!nfc_device_load_mifare_df_key_settings(file, data->master_key_settings, "PICC")) {
                free(data->master_key_settings);
                data->master_key_settings = NULL;
                break;
            }
        }
        uint32_t n_apps;
        if(!flipper_format_read_uint32(file, "Application Count", &n_apps, 1)) break;
        if(n_apps) {
            tmp = malloc(n_apps * 3);
            if(!flipper_format_read_hex(file, "Application IDs", tmp, n_apps * 3)) break;
            bool parsed_apps = true;
            MifareDesfireApplication** app_head = &data->app_head;
            for(uint32_t i = 0; i < n_apps; i++) {
                MifareDesfireApplication* app = malloc(sizeof(MifareDesfireApplication));
                memset(app, 0, sizeof(MifareDesfireApplication));
                memcpy(app->id, &tmp[i * 3], 3);
                if(!nfc_device_load_mifare_df_app(file, app)) {
                    free(app);
                    parsed_apps = false;
                    break;
                }
                *app_head = app;
                app_head = &app->next;
            }
            if(!parsed_apps) break;
        }
        parsed = true;
    } while(false);

    free(tmp);
    return parsed;
}

static bool nfc_device_save_bank_card_data(FlipperFormat* file, NfcDevice* dev) {
    bool saved = false;
    EmvData* data = &dev->dev_data.emv_data;
    uint32_t data_temp = 0;

    do {
        // Write Bank card specific data
        if(!flipper_format_write_comment_cstr(file, "Bank card specific data")) break;
        if(!flipper_format_write_hex(file, "AID", data->aid, data->aid_len)) break;
        if(!flipper_format_write_string_cstr(file, "Name", data->name)) break;
        if(!flipper_format_write_hex(file, "Number", data->number, data->number_len)) break;
        if(data->exp_mon) {
            uint8_t exp_data[2] = {data->exp_mon, data->exp_year};
            if(!flipper_format_write_hex(file, "Exp data", exp_data, sizeof(exp_data))) break;
        }
        if(data->country_code) {
            data_temp = data->country_code;
            if(!flipper_format_write_uint32(file, "Country code", &data_temp, 1)) break;
        }
        if(data->currency_code) {
            data_temp = data->currency_code;
            if(!flipper_format_write_uint32(file, "Currency code", &data_temp, 1)) break;
        }
        saved = true;
    } while(false);

    return saved;
}

bool nfc_device_load_bank_card_data(FlipperFormat* file, NfcDevice* dev) {
    bool parsed = false;
    EmvData* data = &dev->dev_data.emv_data;
    memset(data, 0, sizeof(EmvData));
    uint32_t data_cnt = 0;
    string_t temp_str;
    string_init(temp_str);

    do {
        // Load essential data
        if(!flipper_format_get_value_count(file, "AID", &data_cnt)) break;
        data->aid_len = data_cnt;
        if(!flipper_format_read_hex(file, "AID", data->aid, data->aid_len)) break;
        if(!flipper_format_read_string(file, "Name", temp_str)) break;
        strlcpy(data->name, string_get_cstr(temp_str), sizeof(data->name));
        if(!flipper_format_get_value_count(file, "Number", &data_cnt)) break;
        data->number_len = data_cnt;
        if(!flipper_format_read_hex(file, "Number", data->number, data->number_len)) break;
        parsed = true;
        // Load optional data
        uint8_t exp_data[2] = {};
        if(flipper_format_read_hex(file, "Exp data", exp_data, 2)) {
            data->exp_mon = exp_data[0];
            data->exp_year = exp_data[1];
        }
        if(flipper_format_read_uint32(file, "Country code", &data_cnt, 1)) {
            data->country_code = data_cnt;
        }
        if(flipper_format_read_uint32(file, "Currency code", &data_cnt, 1)) {
            data->currency_code = data_cnt;
        }
    } while(false);

    string_clear(temp_str);
    return parsed;
}

static void nfc_device_write_mifare_classic_block(
    string_t block_str,
    MfClassicData* data,
    uint8_t block_num) {
    string_reset(block_str);
    bool is_sec_trailer = mf_classic_is_sector_trailer(block_num);
    if(is_sec_trailer) {
        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, sector_num);
        // Write key A
        for(size_t i = 0; i < sizeof(sec_tr->key_a); i++) {
            if(mf_classic_is_key_found(data, sector_num, MfClassicKeyA)) {
                string_cat_printf(block_str, "%02X ", sec_tr->key_a[i]);
            } else {
                string_cat_printf(block_str, "?? ");
            }
        }
        // Write Access bytes
        for(size_t i = 0; i < MF_CLASSIC_ACCESS_BYTES_SIZE; i++) {
            if(mf_classic_is_block_read(data, block_num)) {
                string_cat_printf(block_str, "%02X ", sec_tr->access_bits[i]);
            } else {
                string_cat_printf(block_str, "?? ");
            }
        }
        // Write key B
        for(size_t i = 0; i < sizeof(sec_tr->key_b); i++) {
            if(mf_classic_is_key_found(data, sector_num, MfClassicKeyB)) {
                string_cat_printf(block_str, "%02X ", sec_tr->key_b[i]);
            } else {
                string_cat_printf(block_str, "?? ");
            }
        }
    } else {
        // Write data block
        for(size_t i = 0; i < MF_CLASSIC_BLOCK_SIZE; i++) {
            if(mf_classic_is_block_read(data, block_num)) {
                string_cat_printf(block_str, "%02X ", data->block[block_num].value[i]);
            } else {
                string_cat_printf(block_str, "?? ");
            }
        }
    }
    string_strim(block_str);
}

static bool nfc_device_save_mifare_classic_data(FlipperFormat* file, NfcDevice* dev) {
    bool saved = false;
    MfClassicData* data = &dev->dev_data.mf_classic_data;
    string_t temp_str;
    string_init(temp_str);
    uint16_t blocks = 0;

    // Save Mifare Classic specific data
    do {
        if(!flipper_format_write_comment_cstr(file, "Mifare Classic specific data")) break;

        if(data->type == MfClassicType1k) {
            if(!flipper_format_write_string_cstr(file, "Mifare Classic type", "1K")) break;
            blocks = 64;
        } else if(data->type == MfClassicType4k) {
            if(!flipper_format_write_string_cstr(file, "Mifare Classic type", "4K")) break;
            blocks = 256;
        }
        if(!flipper_format_write_uint32(
               file, "Data format version", &nfc_mifare_classic_data_format_version, 1))
            break;
        if(!flipper_format_write_comment_cstr(
               file, "Mifare Classic blocks, \'??\' means unknown data"))
            break;
        bool block_saved = true;
        string_t block_str;
        string_init(block_str);
        for(size_t i = 0; i < blocks; i++) {
            string_printf(temp_str, "Block %d", i);
            nfc_device_write_mifare_classic_block(block_str, data, i);
            if(!flipper_format_write_string(file, string_get_cstr(temp_str), block_str)) {
                block_saved = false;
                break;
            }
        }
        string_clear(block_str);
        if(!block_saved) break;
        saved = true;
    } while(false);

    string_clear(temp_str);
    return saved;
}

static void nfc_device_load_mifare_classic_block(
    string_t block_str,
    MfClassicData* data,
    uint8_t block_num) {
    string_strim(block_str);
    MfClassicBlock block_tmp = {};
    bool is_sector_trailer = mf_classic_is_sector_trailer(block_num);
    uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
    uint16_t block_unknown_bytes_mask = 0;

    string_strim(block_str);
    for(size_t i = 0; i < MF_CLASSIC_BLOCK_SIZE; i++) {
        char hi = string_get_char(block_str, 3 * i);
        char low = string_get_char(block_str, 3 * i + 1);
        uint8_t byte = 0;
        if(hex_char_to_uint8(hi, low, &byte)) {
            block_tmp.value[i] = byte;
        } else {
            FURI_BIT_SET(block_unknown_bytes_mask, i);
        }
    }

    if(block_unknown_bytes_mask == 0xffff) {
        // All data is unknown, exit
        return;
    }

    if(is_sector_trailer) {
        MfClassicSectorTrailer* sec_tr_tmp = (MfClassicSectorTrailer*)&block_tmp;
        // Load Key A
        // Key A mask 0b0000000000111111 = 0x003f
        if((block_unknown_bytes_mask & 0x003f) == 0) {
            uint64_t key = nfc_util_bytes2num(sec_tr_tmp->key_a, sizeof(sec_tr_tmp->key_a));
            mf_classic_set_key_found(data, sector_num, MfClassicKeyA, key);
        }
        // Load Access Bits
        // Access bits mask 0b0000001111000000 = 0x03c0
        if((block_unknown_bytes_mask & 0x03c0) == 0) {
            mf_classic_set_block_read(data, block_num, &block_tmp);
        }
        // Load Key B
        // Key B mask 0b1111110000000000 = 0xfc00
        if((block_unknown_bytes_mask & 0xfc00) == 0) {
            uint64_t key = nfc_util_bytes2num(sec_tr_tmp->key_b, sizeof(sec_tr_tmp->key_b));
            mf_classic_set_key_found(data, sector_num, MfClassicKeyB, key);
        }
    } else {
        if(block_unknown_bytes_mask == 0) {
            mf_classic_set_block_read(data, block_num, &block_tmp);
        }
    }
}

static bool nfc_device_load_mifare_classic_data(FlipperFormat* file, NfcDevice* dev) {
    bool parsed = false;
    MfClassicData* data = &dev->dev_data.mf_classic_data;
    string_t temp_str;
    uint32_t data_format_version = 0;
    string_init(temp_str);
    uint16_t data_blocks = 0;
    memset(data, 0, sizeof(MfClassicData));

    do {
        // Read Mifare Classic type
        if(!flipper_format_read_string(file, "Mifare Classic type", temp_str)) break;
        if(!string_cmp_str(temp_str, "1K")) {
            data->type = MfClassicType1k;
            data_blocks = 64;
        } else if(!string_cmp_str(temp_str, "4K")) {
            data->type = MfClassicType4k;
            data_blocks = 256;
        } else {
            break;
        }

        bool old_format = false;
        // Read Mifare Classic format version
        if(!flipper_format_read_uint32(file, "Data format version", &data_format_version, 1)) {
            // Load unread sectors with zero keys access for backward compatability
            if(!flipper_format_rewind(file)) break;
            old_format = true;
        } else {
            if(data_format_version < nfc_mifare_classic_data_format_version) {
                old_format = true;
            }
        }

        // Read Mifare Classic blocks
        bool block_read = true;
        string_t block_str;
        string_init(block_str);
        for(size_t i = 0; i < data_blocks; i++) {
            string_printf(temp_str, "Block %d", i);
            if(!flipper_format_read_string(file, string_get_cstr(temp_str), block_str)) {
                block_read = false;
                break;
            }
            nfc_device_load_mifare_classic_block(block_str, data, i);
        }
        string_clear(block_str);
        if(!block_read) break;

        // Set keys and blocks as unknown for backward compatibility
        if(old_format) {
            data->key_a_mask = 0ULL;
            data->key_b_mask = 0ULL;
            memset(data->block_read_mask, 0, sizeof(data->block_read_mask));
        }

        parsed = true;
    } while(false);

    string_clear(temp_str);
    return parsed;
}

static void nfc_device_get_key_cache_file_path(NfcDevice* dev, string_t file_path) {
    uint8_t* uid = dev->dev_data.nfc_data.uid;
    uint8_t uid_len = dev->dev_data.nfc_data.uid_len;
    string_set_str(file_path, NFC_DEVICE_KEYS_FOLDER "/");
    for(size_t i = 0; i < uid_len; i++) {
        string_cat_printf(file_path, "%02X", uid[i]);
    }
    string_cat_printf(file_path, NFC_DEVICE_KEYS_EXTENSION);
}

static bool nfc_device_save_mifare_classic_keys(NfcDevice* dev) {
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);
    MfClassicData* data = &dev->dev_data.mf_classic_data;
    string_t temp_str;
    string_init(temp_str);

    nfc_device_get_key_cache_file_path(dev, temp_str);
    bool save_success = false;
    do {
        if(!storage_simply_mkdir(dev->storage, NFC_DEVICE_KEYS_FOLDER)) break;
        if(!storage_simply_remove(dev->storage, string_get_cstr(temp_str))) break;
        if(!flipper_format_file_open_always(file, string_get_cstr(temp_str))) break;
        if(!flipper_format_write_header_cstr(file, nfc_keys_file_header, nfc_keys_file_version))
            break;
        if(data->type == MfClassicType1k) {
            if(!flipper_format_write_string_cstr(file, "Mifare Classic type", "1K")) break;
        } else if(data->type == MfClassicType4k) {
            if(!flipper_format_write_string_cstr(file, "Mifare Classic type", "4K")) break;
        }
        if(!flipper_format_write_hex_uint64(file, "Key A map", &data->key_a_mask, 1)) break;
        if(!flipper_format_write_hex_uint64(file, "Key B map", &data->key_b_mask, 1)) break;
        uint8_t sector_num = mf_classic_get_total_sectors_num(data->type);
        bool key_save_success = true;
        for(size_t i = 0; (i < sector_num) && (key_save_success); i++) {
            MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, i);
            if(FURI_BIT(data->key_a_mask, i)) {
                string_printf(temp_str, "Key A sector %d", i);
                key_save_success =
                    flipper_format_write_hex(file, string_get_cstr(temp_str), sec_tr->key_a, 6);
            }
            if(!key_save_success) break;
            if(FURI_BIT(data->key_a_mask, i)) {
                string_printf(temp_str, "Key B sector %d", i);
                key_save_success =
                    flipper_format_write_hex(file, string_get_cstr(temp_str), sec_tr->key_b, 6);
            }
        }
        save_success = key_save_success;
    } while(false);

    flipper_format_free(file);
    string_clear(temp_str);
    return save_success;
}

bool nfc_device_load_key_cache(NfcDevice* dev) {
    furi_assert(dev);
    string_t temp_str;
    string_init(temp_str);

    MfClassicData* data = &dev->dev_data.mf_classic_data;
    nfc_device_get_key_cache_file_path(dev, temp_str);
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);

    bool load_success = false;
    do {
        if(storage_common_stat(dev->storage, string_get_cstr(temp_str), NULL) != FSE_OK) break;
        if(!flipper_format_file_open_existing(file, string_get_cstr(temp_str))) break;
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(string_cmp_str(temp_str, nfc_keys_file_header)) break;
        if(version != nfc_keys_file_version) break;
        if(!flipper_format_read_string(file, "Mifare Classic type", temp_str)) break;
        if(!string_cmp_str(temp_str, "1K")) {
            data->type = MfClassicType1k;
        } else if(!string_cmp_str(temp_str, "4K")) {
            data->type = MfClassicType4k;
        } else {
            break;
        }
        if(!flipper_format_read_hex_uint64(file, "Key A map", &data->key_a_mask, 1)) break;
        if(!flipper_format_read_hex_uint64(file, "Key B map", &data->key_b_mask, 1)) break;
        uint8_t sectors = mf_classic_get_total_sectors_num(data->type);
        bool key_read_success = true;
        for(size_t i = 0; (i < sectors) && (key_read_success); i++) {
            MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, i);
            if(FURI_BIT(data->key_a_mask, i)) {
                string_printf(temp_str, "Key A sector %d", i);
                key_read_success =
                    flipper_format_read_hex(file, string_get_cstr(temp_str), sec_tr->key_a, 6);
            }
            if(!key_read_success) break;
            if(FURI_BIT(data->key_b_mask, i)) {
                string_printf(temp_str, "Key B sector %d", i);
                key_read_success =
                    flipper_format_read_hex(file, string_get_cstr(temp_str), sec_tr->key_b, 6);
            }
        }
        load_success = key_read_success;
    } while(false);

    string_clear(temp_str);
    flipper_format_free(file);

    return load_success;
}

void nfc_device_set_name(NfcDevice* dev, const char* name) {
    furi_assert(dev);

    strlcpy(dev->dev_name, name, NFC_DEV_NAME_MAX_LEN);
}

static void nfc_device_get_path_without_ext(string_t orig_path, string_t shadow_path) {
    // TODO: this won't work if there is ".nfc" anywhere in the path other than
    // at the end
    size_t ext_start = string_search_str(orig_path, NFC_APP_EXTENSION);
    string_set_n(shadow_path, orig_path, 0, ext_start);
}

static void nfc_device_get_shadow_path(string_t orig_path, string_t shadow_path) {
    nfc_device_get_path_without_ext(orig_path, shadow_path);
    string_cat_printf(shadow_path, "%s", NFC_APP_SHADOW_EXTENSION);
}

static bool nfc_device_save_file(
    NfcDevice* dev,
    const char* dev_name,
    const char* folder,
    const char* extension,
    bool use_load_path) {
    furi_assert(dev);

    bool saved = false;
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);
    FuriHalNfcDevData* data = &dev->dev_data.nfc_data;
    string_t temp_str;
    string_init(temp_str);

    do {
        if(use_load_path && !string_empty_p(dev->load_path)) {
            // Get directory name
            path_extract_dirname(string_get_cstr(dev->load_path), temp_str);
            // Create nfc directory if necessary
            if(!storage_simply_mkdir(dev->storage, string_get_cstr(temp_str))) break;
            // Make path to file to save
            string_cat_printf(temp_str, "/%s%s", dev_name, extension);
        } else {
            // Create nfc directory if necessary
            if(!storage_simply_mkdir(dev->storage, NFC_APP_FOLDER)) break;
            // First remove nfc device file if it was saved
            string_printf(temp_str, "%s/%s%s", folder, dev_name, extension);
        }
        // Open file
        if(!flipper_format_file_open_always(file, string_get_cstr(temp_str))) break;
        // Write header
        if(!flipper_format_write_header_cstr(file, nfc_file_header, nfc_file_version)) break;
        // Write nfc device type
        if(!flipper_format_write_comment_cstr(
               file, "Nfc device type can be UID, Mifare Ultralight, Mifare Classic, Bank card"))
            break;
        nfc_device_prepare_format_string(dev, temp_str);
        if(!flipper_format_write_string(file, "Device type", temp_str)) break;
        // Write UID, ATQA, SAK
        if(!flipper_format_write_comment_cstr(file, "UID, ATQA and SAK are common for all formats"))
            break;
        if(!flipper_format_write_hex(file, "UID", data->uid, data->uid_len)) break;
        if(!flipper_format_write_hex(file, "ATQA", data->atqa, 2)) break;
        if(!flipper_format_write_hex(file, "SAK", &data->sak, 1)) break;
        // Save more data if necessary
        if(dev->format == NfcDeviceSaveFormatMifareUl) {
            if(!nfc_device_save_mifare_ul_data(file, dev)) break;
        } else if(dev->format == NfcDeviceSaveFormatMifareDesfire) {
            if(!nfc_device_save_mifare_df_data(file, dev)) break;
        } else if(dev->format == NfcDeviceSaveFormatBankCard) {
            if(!nfc_device_save_bank_card_data(file, dev)) break;
        } else if(dev->format == NfcDeviceSaveFormatMifareClassic) {
            // Save data
            if(!nfc_device_save_mifare_classic_data(file, dev)) break;
            // Save keys cache
            if(!nfc_device_save_mifare_classic_keys(dev)) break;
        }
        saved = true;
    } while(0);

    if(!saved) {
        dialog_message_show_storage_error(dev->dialogs, "Can not save\nkey file");
    }
    string_clear(temp_str);
    flipper_format_free(file);
    return saved;
}

bool nfc_device_save(NfcDevice* dev, const char* dev_name) {
    return nfc_device_save_file(dev, dev_name, NFC_APP_FOLDER, NFC_APP_EXTENSION, true);
}

bool nfc_device_save_shadow(NfcDevice* dev, const char* dev_name) {
    dev->shadow_file_exist = true;
    return nfc_device_save_file(dev, dev_name, NFC_APP_FOLDER, NFC_APP_SHADOW_EXTENSION, true);
}

static bool nfc_device_load_data(NfcDevice* dev, string_t path, bool show_dialog) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);
    FuriHalNfcDevData* data = &dev->dev_data.nfc_data;
    uint32_t data_cnt = 0;
    string_t temp_str;
    string_init(temp_str);
    bool deprecated_version = false;

    if(dev->loading_cb) {
        dev->loading_cb(dev->loading_cb_ctx, true);
    }

    do {
        // Check existance of shadow file
        nfc_device_get_shadow_path(path, temp_str);
        dev->shadow_file_exist =
            storage_common_stat(dev->storage, string_get_cstr(temp_str), NULL) == FSE_OK;
        // Open shadow file if it exists. If not - open original
        if(dev->shadow_file_exist) {
            if(!flipper_format_file_open_existing(file, string_get_cstr(temp_str))) break;
        } else {
            if(!flipper_format_file_open_existing(file, string_get_cstr(path))) break;
        }
        // Read and verify file header
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(string_cmp_str(temp_str, nfc_file_header) || (version != nfc_file_version)) {
            deprecated_version = true;
            break;
        }
        // Read Nfc device type
        if(!flipper_format_read_string(file, "Device type", temp_str)) break;
        if(!nfc_device_parse_format_string(dev, temp_str)) break;
        // Read and parse UID, ATQA and SAK
        if(!flipper_format_get_value_count(file, "UID", &data_cnt)) break;
        if(!(data_cnt == 4 || data_cnt == 7)) break;
        data->uid_len = data_cnt;
        if(!flipper_format_read_hex(file, "UID", data->uid, data->uid_len)) break;
        if(!flipper_format_read_hex(file, "ATQA", data->atqa, 2)) break;
        if(!flipper_format_read_hex(file, "SAK", &data->sak, 1)) break;
        // Parse other data
        if(dev->format == NfcDeviceSaveFormatMifareUl) {
            if(!nfc_device_load_mifare_ul_data(file, dev)) break;
        } else if(dev->format == NfcDeviceSaveFormatMifareClassic) {
            if(!nfc_device_load_mifare_classic_data(file, dev)) break;
        } else if(dev->format == NfcDeviceSaveFormatMifareDesfire) {
            if(!nfc_device_load_mifare_df_data(file, dev)) break;
        } else if(dev->format == NfcDeviceSaveFormatBankCard) {
            if(!nfc_device_load_bank_card_data(file, dev)) break;
        }
        parsed = true;
    } while(false);

    if(dev->loading_cb) {
        dev->loading_cb(dev->loading_cb_ctx, false);
    }

    if((!parsed) && (show_dialog)) {
        if(deprecated_version) {
            dialog_message_show_storage_error(dev->dialogs, "File format deprecated");
        } else {
            dialog_message_show_storage_error(dev->dialogs, "Can not parse\nfile");
        }
    }

    string_clear(temp_str);
    flipper_format_free(file);
    return parsed;
}

bool nfc_device_load(NfcDevice* dev, const char* file_path, bool show_dialog) {
    furi_assert(dev);
    furi_assert(file_path);

    // Load device data
    string_set_str(dev->load_path, file_path);
    bool dev_load = nfc_device_load_data(dev, dev->load_path, show_dialog);
    if(dev_load) {
        // Set device name
        string_t filename;
        string_init(filename);
        path_extract_filename_no_ext(file_path, filename);
        nfc_device_set_name(dev, string_get_cstr(filename));
        string_clear(filename);
    }

    return dev_load;
}

bool nfc_file_select(NfcDevice* dev) {
    furi_assert(dev);

    // Input events and views are managed by file_browser
    string_t nfc_app_folder;
    string_init_set_str(nfc_app_folder, NFC_APP_FOLDER);
    bool res = dialog_file_browser_show(
        dev->dialogs, dev->load_path, nfc_app_folder, NFC_APP_EXTENSION, true, &I_Nfc_10px, true);
    string_clear(nfc_app_folder);
    if(res) {
        string_t filename;
        string_init(filename);
        path_extract_filename(dev->load_path, filename, true);
        strncpy(dev->dev_name, string_get_cstr(filename), NFC_DEV_NAME_MAX_LEN);
        res = nfc_device_load_data(dev, dev->load_path, true);
        if(res) {
            nfc_device_set_name(dev, dev->dev_name);
        }
        string_clear(filename);
    }

    return res;
}

void nfc_device_data_clear(NfcDeviceData* dev_data) {
    if(dev_data->protocol == NfcDeviceProtocolMifareDesfire) {
        mf_df_clear(&dev_data->mf_df_data);
    } else if(dev_data->protocol == NfcDeviceProtocolMifareClassic) {
        memset(&dev_data->mf_classic_data, 0, sizeof(MfClassicData));
    } else if(dev_data->protocol == NfcDeviceProtocolMifareUl) {
        mf_ul_reset(&dev_data->mf_ul_data);
    } else if(dev_data->protocol == NfcDeviceProtocolEMV) {
        memset(&dev_data->emv_data, 0, sizeof(EmvData));
    }
    memset(&dev_data->nfc_data, 0, sizeof(FuriHalNfcDevData));
    dev_data->protocol = NfcDeviceProtocolUnknown;
    string_reset(dev_data->parsed_data);
}

void nfc_device_clear(NfcDevice* dev) {
    furi_assert(dev);

    nfc_device_set_name(dev, "");
    nfc_device_data_clear(&dev->dev_data);
    dev->format = NfcDeviceSaveFormatUid;
    string_reset(dev->load_path);
}

bool nfc_device_delete(NfcDevice* dev, bool use_load_path) {
    furi_assert(dev);

    bool deleted = false;
    string_t file_path;
    string_init(file_path);

    do {
        // Delete original file
        if(use_load_path && !string_empty_p(dev->load_path)) {
            string_set(file_path, dev->load_path);
        } else {
            string_printf(file_path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_EXTENSION);
        }
        if(!storage_simply_remove(dev->storage, string_get_cstr(file_path))) break;
        // Delete shadow file if it exists
        if(dev->shadow_file_exist) {
            if(use_load_path && !string_empty_p(dev->load_path)) {
                nfc_device_get_shadow_path(dev->load_path, file_path);
            } else {
                string_printf(
                    file_path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_SHADOW_EXTENSION);
            }
            if(!storage_simply_remove(dev->storage, string_get_cstr(file_path))) break;
        }
        deleted = true;
    } while(0);

    if(!deleted) {
        dialog_message_show_storage_error(dev->dialogs, "Can not remove file");
    }

    string_clear(file_path);
    return deleted;
}

bool nfc_device_restore(NfcDevice* dev, bool use_load_path) {
    furi_assert(dev);
    furi_assert(dev->shadow_file_exist);

    bool restored = false;
    string_t path;

    string_init(path);

    do {
        if(use_load_path && !string_empty_p(dev->load_path)) {
            nfc_device_get_shadow_path(dev->load_path, path);
        } else {
            string_printf(
                path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_SHADOW_EXTENSION);
        }
        if(!storage_simply_remove(dev->storage, string_get_cstr(path))) break;
        dev->shadow_file_exist = false;
        if(use_load_path && !string_empty_p(dev->load_path)) {
            string_set(path, dev->load_path);
        } else {
            string_printf(path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_EXTENSION);
        }
        if(!nfc_device_load_data(dev, path, true)) break;
        restored = true;
    } while(0);

    string_clear(path);
    return restored;
}

void nfc_device_set_loading_callback(NfcDevice* dev, NfcLoadingCallback callback, void* context) {
    furi_assert(dev);

    dev->loading_cb = callback;
    dev->loading_cb_ctx = context;
}
