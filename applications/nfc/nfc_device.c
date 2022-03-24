#include "nfc_device.h"
#include "nfc_types.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

static const char* nfc_file_header = "Flipper NFC device";
static const uint32_t nfc_file_version = 2;

NfcDevice* nfc_device_alloc() {
    NfcDevice* nfc_dev = malloc(sizeof(NfcDevice));
    nfc_dev->storage = furi_record_open("storage");
    nfc_dev->dialogs = furi_record_open("dialogs");
    return nfc_dev;
}

void nfc_device_free(NfcDevice* nfc_dev) {
    furi_assert(nfc_dev);
    nfc_device_clear(nfc_dev);
    furi_record_close("storage");
    furi_record_close("dialogs");
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
        dev->dev_data.nfc_data.protocol = NfcDeviceProtocolUnknown;
        return true;
    }
    if(string_start_with_str_p(format_string, "Bank card")) {
        dev->format = NfcDeviceSaveFormatBankCard;
        dev->dev_data.nfc_data.protocol = NfcDeviceProtocolEMV;
        return true;
    }
    // Check Mifare Ultralight types
    for(MfUltralightType type = MfUltralightTypeUnknown; type < MfUltralightTypeNum; type++) {
        if(string_start_with_str_p(format_string, nfc_mf_ul_type(type, true))) {
            dev->format = NfcDeviceSaveFormatMifareUl;
            dev->dev_data.nfc_data.protocol = NfcDeviceProtocolMifareUl;
            dev->dev_data.mf_ul_data.type = type;
            return true;
        }
    }
    if(string_start_with_str_p(format_string, "Mifare Classic")) {
        dev->format = NfcDeviceSaveFormatMifareClassic;
        dev->dev_data.nfc_data.protocol = NfcDeviceProtocolMifareClassic;
        return true;
    }
    if(string_start_with_str_p(format_string, "Mifare DESFire")) {
        dev->format = NfcDeviceSaveFormatMifareDesfire;
        dev->dev_data.nfc_data.protocol = NfcDeviceProtocolMifareDesfire;
        return true;
    }
    return false;
}

static bool nfc_device_save_mifare_ul_data(FlipperFormat* file, NfcDevice* dev) {
    bool saved = false;
    MifareUlData* data = &dev->dev_data.mf_ul_data;
    string_t temp_str;
    string_init(temp_str);

    // Save Mifare Ultralight specific data
    do {
        if(!flipper_format_write_comment_cstr(file, "Mifare Ultralight specific data")) break;
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
        bool pages_saved = true;
        for(uint16_t i = 0; i < data->data_size; i += 4) {
            string_printf(temp_str, "Page %d", i / 4);
            if(!flipper_format_write_hex(file, string_get_cstr(temp_str), &data->data[i], 4)) {
                pages_saved = false;
                break;
            }
        }
        if(!pages_saved) break;
        saved = true;
    } while(false);

    string_clear(temp_str);
    return saved;
}

bool nfc_device_load_mifare_ul_data(FlipperFormat* file, NfcDevice* dev) {
    bool parsed = false;
    MifareUlData* data = &dev->dev_data.mf_ul_data;
    string_t temp_str;
    string_init(temp_str);

    do {
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
        uint32_t pages = 0;
        if(!flipper_format_read_uint32(file, "Pages total", &pages, 1)) break;
        data->data_size = pages * 4;
        bool pages_parsed = true;
        for(uint16_t i = 0; i < pages; i++) {
            string_printf(temp_str, "Page %d", i);
            if(!flipper_format_read_hex(file, string_get_cstr(temp_str), &data->data[i * 4], 4)) {
                pages_parsed = false;
                break;
            }
        }
        if(!pages_parsed) break;
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
        string_printf(key, "%s Max Keys", prefix);
        if(!flipper_format_read_hex(file, string_get_cstr(key), &ks->max_keys, 1)) break;
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
        for(int i = 0; i < n_files; i++) {
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
        data->master_key_settings = malloc(sizeof(MifareDesfireKeySettings));
        memset(data->master_key_settings, 0, sizeof(MifareDesfireKeySettings));
        if(!nfc_device_load_mifare_df_key_settings(file, data->master_key_settings, "PICC")) {
            free(data->master_key_settings);
            data->master_key_settings = NULL;
            break;
        }
        uint32_t n_apps;
        if(!flipper_format_read_uint32(file, "Application Count", &n_apps, 1)) break;
        tmp = malloc(n_apps * 3);
        if(!flipper_format_read_hex(file, "Application IDs", tmp, n_apps * 3)) break;
        bool parsed_apps = true;
        MifareDesfireApplication** app_head = &data->app_head;
        for(int i = 0; i < n_apps; i++) {
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
        parsed = true;
    } while(false);

    free(tmp);
    return parsed;
}

static bool nfc_device_save_bank_card_data(FlipperFormat* file, NfcDevice* dev) {
    bool saved = false;
    NfcEmvData* data = &dev->dev_data.emv_data;
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
    NfcEmvData* data = &dev->dev_data.emv_data;
    memset(data, 0, sizeof(NfcEmvData));
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
        if(!flipper_format_write_comment_cstr(file, "Mifare Classic blocks")) break;

        bool block_saved = true;
        for(size_t i = 0; i < blocks; i++) {
            string_printf(temp_str, "Block %d", i);
            if(!flipper_format_write_hex(
                   file, string_get_cstr(temp_str), data->block[i].value, 16)) {
                block_saved = false;
                break;
            }
        }
        if(!block_saved) break;
        saved = true;
    } while(false);

    string_clear(temp_str);
    return saved;
}

static bool nfc_device_load_mifare_classic_data(FlipperFormat* file, NfcDevice* dev) {
    bool parsed = false;
    MfClassicData* data = &dev->dev_data.mf_classic_data;
    string_t temp_str;
    string_init(temp_str);
    uint16_t data_blocks = 0;

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
        // Read Mifare Classic blocks
        bool block_read = true;
        for(size_t i = 0; i < data_blocks; i++) {
            string_printf(temp_str, "Block %d", i);
            if(!flipper_format_read_hex(
                   file, string_get_cstr(temp_str), data->block[i].value, 16)) {
                block_read = false;
                break;
            }
        }
        if(!block_read) break;
        parsed = true;
    } while(false);

    string_clear(temp_str);
    return parsed;
}

void nfc_device_set_name(NfcDevice* dev, const char* name) {
    furi_assert(dev);

    strlcpy(dev->dev_name, name, NFC_DEV_NAME_MAX_LEN);
}

static bool nfc_device_save_file(
    NfcDevice* dev,
    const char* dev_name,
    const char* folder,
    const char* extension) {
    furi_assert(dev);

    bool saved = false;
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);
    NfcDeviceCommonData* data = &dev->dev_data.nfc_data;
    string_t temp_str;
    string_init(temp_str);

    do {
        // Create nfc directory if necessary
        if(!storage_simply_mkdir(dev->storage, NFC_APP_FOLDER)) break;
        // First remove nfc device file if it was saved
        string_printf(temp_str, "%s/%s%s", folder, dev_name, extension);
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
            if(!nfc_device_save_mifare_classic_data(file, dev)) break;
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
    return nfc_device_save_file(dev, dev_name, NFC_APP_FOLDER, NFC_APP_EXTENSION);
}

bool nfc_device_save_shadow(NfcDevice* dev, const char* dev_name) {
    dev->shadow_file_exist = true;
    return nfc_device_save_file(dev, dev_name, NFC_APP_FOLDER, NFC_APP_SHADOW_EXTENSION);
}

static bool nfc_device_load_data(NfcDevice* dev, string_t path) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);
    NfcDeviceCommonData* data = &dev->dev_data.nfc_data;
    uint32_t data_cnt = 0;
    string_t temp_str;
    string_init(temp_str);
    bool depricated_version = false;

    do {
        // Check existance of shadow file
        size_t ext_start = string_search_str(path, NFC_APP_EXTENSION);
        string_set_n(temp_str, path, 0, ext_start);
        string_cat_printf(temp_str, "%s", NFC_APP_SHADOW_EXTENSION);
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
            depricated_version = true;
            break;
        }
        // Read Nfc device type
        if(!flipper_format_read_string(file, "Device type", temp_str)) break;
        if(!nfc_device_parse_format_string(dev, temp_str)) break;
        // Read and parse UID, ATQA and SAK
        if(!flipper_format_get_value_count(file, "UID", &data_cnt)) break;
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

    if(!parsed) {
        if(depricated_version) {
            dialog_message_show_storage_error(dev->dialogs, "File format depricated");
        } else {
            dialog_message_show_storage_error(dev->dialogs, "Can not parse\nfile");
        }
    }

    string_clear(temp_str);
    flipper_format_free(file);
    return parsed;
}

bool nfc_device_load(NfcDevice* dev, const char* file_path) {
    furi_assert(dev);
    furi_assert(file_path);

    // Load device data
    string_t path;
    string_init_set_str(path, file_path);
    bool dev_load = nfc_device_load_data(dev, path);
    if(dev_load) {
        // Set device name
        path_extract_filename_no_ext(file_path, path);
        nfc_device_set_name(dev, string_get_cstr(path));
    }
    string_clear(path);

    return dev_load;
}

bool nfc_file_select(NfcDevice* dev) {
    furi_assert(dev);

    // Input events and views are managed by file_select
    bool res = dialog_file_select_show(
        dev->dialogs,
        NFC_APP_FOLDER,
        NFC_APP_EXTENSION,
        dev->file_name,
        sizeof(dev->file_name),
        dev->dev_name);
    if(res) {
        string_t dev_str;
        // Get key file path
        string_init_printf(dev_str, "%s/%s%s", NFC_APP_FOLDER, dev->file_name, NFC_APP_EXTENSION);
        res = nfc_device_load_data(dev, dev_str);
        if(res) {
            nfc_device_set_name(dev, dev->file_name);
        }
        string_clear(dev_str);
    }

    return res;
}

void nfc_device_data_clear(NfcDeviceData* dev_data) {
    if(dev_data->nfc_data.protocol == NfcDeviceProtocolMifareDesfire) {
        mf_df_clear(&dev_data->mf_df_data);
    }
}

void nfc_device_clear(NfcDevice* dev) {
    furi_assert(dev);

    nfc_device_data_clear(&dev->dev_data);
    memset(&dev->dev_data, 0, sizeof(dev->dev_data));
    dev->format = NfcDeviceSaveFormatUid;
}

bool nfc_device_delete(NfcDevice* dev) {
    furi_assert(dev);

    bool deleted = false;
    string_t file_path;
    string_init(file_path);

    do {
        // Delete original file
        string_init_printf(file_path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_EXTENSION);
        if(!storage_simply_remove(dev->storage, string_get_cstr(file_path))) break;
        // Delete shadow file if it exists
        if(dev->shadow_file_exist) {
            string_printf(
                file_path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_SHADOW_EXTENSION);
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

bool nfc_device_restore(NfcDevice* dev) {
    furi_assert(dev);
    furi_assert(dev->shadow_file_exist);

    bool restored = false;
    string_t path;

    do {
        string_init_printf(
            path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_SHADOW_EXTENSION);
        if(!storage_simply_remove(dev->storage, string_get_cstr(path))) break;
        dev->shadow_file_exist = false;
        string_printf(path, "%s/%s%s", NFC_APP_FOLDER, dev->dev_name, NFC_APP_EXTENSION);
        if(!nfc_device_load_data(dev, path)) break;
        restored = true;
    } while(0);

    string_clear(path);
    return restored;
}
