#include "nfc_device.h"

#include <lib/toolbox/path.h>
#include <lib/flipper_file/flipper_file.h>

static const char* nfc_app_folder = "/any/nfc";
static const char* nfc_app_extension = ".nfc";
static const char* nfc_app_shadow_extension = ".shd";
static const char* nfc_file_header = "Flipper NFC device";
static const uint32_t nfc_file_version = 2;

NfcDevice* nfc_device_alloc() {
    NfcDevice* nfc_dev = furi_alloc(sizeof(NfcDevice));
    nfc_dev->storage = furi_record_open("storage");
    nfc_dev->dialogs = furi_record_open("dialogs");
    return nfc_dev;
}

void nfc_device_free(NfcDevice* nfc_dev) {
    furi_assert(nfc_dev);
    furi_record_close("storage");
    furi_record_close("dialogs");
    free(nfc_dev);
}

void nfc_device_prepare_format_string(NfcDevice* dev, string_t format_string) {
    if(dev->format == NfcDeviceSaveFormatUid) {
        string_set_str(format_string, "UID");
    } else if(dev->format == NfcDeviceSaveFormatBankCard) {
        string_set_str(format_string, "Bank card");
    } else if(dev->format == NfcDeviceSaveFormatMifareUl) {
        string_set_str(format_string, "Mifare Ultralight");
    } else {
        string_set_str(format_string, "Unknown");
    }
}

bool nfc_device_parse_format_string(NfcDevice* dev, string_t format_string) {
    if(string_start_with_str_p(format_string, "UID")) {
        dev->format = NfcDeviceSaveFormatUid;
        dev->dev_data.nfc_data.protocol = NfcDeviceProtocolUnknown;
        return true;
    } else if(string_start_with_str_p(format_string, "Bank card")) {
        dev->format = NfcDeviceSaveFormatBankCard;
        dev->dev_data.nfc_data.protocol = NfcDeviceProtocolEMV;
        return true;
    } else if(string_start_with_str_p(format_string, "Mifare Ultralight")) {
        dev->format = NfcDeviceSaveFormatMifareUl;
        dev->dev_data.nfc_data.protocol = NfcDeviceProtocolMifareUl;
        return true;
    }
    return false;
}

static bool nfc_device_save_mifare_ul_data(FlipperFile* file, NfcDevice* dev) {
    bool saved = false;
    MifareUlData* data = &dev->dev_data.mf_ul_data;
    string_t temp_str;
    string_init(temp_str);

    // Save Mifare Ultralight specific data
    do {
        if(!flipper_file_write_comment_cstr(file, "Mifare Ultralight specific data")) break;
        if(!flipper_file_write_hex(file, "Signature", data->signature, sizeof(data->signature)))
            break;
        if(!flipper_file_write_hex(
               file, "Mifare version", (uint8_t*)&data->version, sizeof(data->version)))
            break;
        // Write conters and tearing flags data
        bool counters_saved = true;
        for(uint8_t i = 0; i < 3; i++) {
            string_printf(temp_str, "Counter %d", i);
            if(!flipper_file_write_uint32(file, string_get_cstr(temp_str), &data->counter[i], 1)) {
                counters_saved = false;
                break;
            }
            string_printf(temp_str, "Tearing %d", i);
            if(!flipper_file_write_hex(file, string_get_cstr(temp_str), &data->tearing[i], 1)) {
                counters_saved = false;
                break;
            }
        }
        if(!counters_saved) break;
        // Write pages data
        uint32_t pages_total = data->data_size / 4;
        if(!flipper_file_write_uint32(file, "Pages total", &pages_total, 1)) break;
        bool pages_saved = true;
        for(uint16_t i = 0; i < data->data_size; i += 4) {
            string_printf(temp_str, "Page %d", i / 4);
            if(!flipper_file_write_hex(file, string_get_cstr(temp_str), &data->data[i], 4)) {
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

bool nfc_device_load_mifare_ul_data(FlipperFile* file, NfcDevice* dev) {
    bool parsed = false;
    MifareUlData* data = &dev->dev_data.mf_ul_data;
    string_t temp_str;
    string_init(temp_str);

    do {
        // Read signature
        if(!flipper_file_read_hex(file, "Signature", data->signature, sizeof(data->signature)))
            break;
        // Read Mifare version
        if(!flipper_file_read_hex(
               file, "Mifare version", (uint8_t*)&data->version, sizeof(data->version)))
            break;
        // Read counters and tearing flags
        bool counters_parsed = true;
        for(uint8_t i = 0; i < 3; i++) {
            string_printf(temp_str, "Counter %d", i);
            if(!flipper_file_read_uint32(file, string_get_cstr(temp_str), &data->counter[i], 1)) {
                counters_parsed = false;
                break;
            }
            string_printf(temp_str, "Tearing %d", i);
            if(!flipper_file_read_hex(file, string_get_cstr(temp_str), &data->tearing[i], 1)) {
                counters_parsed = false;
                break;
            }
        }
        if(!counters_parsed) break;
        // Read pages
        uint32_t pages = 0;
        if(!flipper_file_read_uint32(file, "Pages total", &pages, 1)) break;
        data->data_size = pages * 4;
        bool pages_parsed = true;
        for(uint16_t i = 0; i < pages; i++) {
            string_printf(temp_str, "Page %d", i);
            if(!flipper_file_read_hex(file, string_get_cstr(temp_str), &data->data[i * 4], 4)) {
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

static bool nfc_device_save_bank_card_data(FlipperFile* file, NfcDevice* dev) {
    bool saved = false;
    NfcEmvData* data = &dev->dev_data.emv_data;
    uint32_t data_temp = 0;

    do {
        // Write Bank card specific data
        if(!flipper_file_write_comment_cstr(file, "Bank card specific data")) break;
        if(!flipper_file_write_hex(file, "AID", data->aid, data->aid_len)) break;
        if(!flipper_file_write_string_cstr(file, "Name", data->name)) break;
        if(!flipper_file_write_hex(file, "Number", data->number, data->number_len)) break;
        if(data->exp_mon) {
            uint8_t exp_data[2] = {data->exp_mon, data->exp_year};
            if(!flipper_file_write_hex(file, "Exp data", exp_data, sizeof(exp_data))) break;
        }
        if(data->country_code) {
            data_temp = data->country_code;
            if(!flipper_file_write_uint32(file, "Country code", &data_temp, 1)) break;
        }
        if(data->currency_code) {
            data_temp = data->currency_code;
            if(!flipper_file_write_uint32(file, "Currency code", &data_temp, 1)) break;
        }
        saved = true;
    } while(false);

    return saved;
}

bool nfc_device_load_bank_card_data(FlipperFile* file, NfcDevice* dev) {
    bool parsed = false;
    NfcEmvData* data = &dev->dev_data.emv_data;
    memset(data, 0, sizeof(NfcEmvData));
    uint32_t data_cnt = 0;
    string_t temp_str;
    string_init(temp_str);

    do {
        // Load essential data
        if(!flipper_file_get_value_count(file, "AID", &data_cnt)) break;
        data->aid_len = data_cnt;
        if(!flipper_file_read_hex(file, "AID", data->aid, data->aid_len)) break;
        if(!flipper_file_read_string(file, "Name", temp_str)) break;
        strlcpy(data->name, string_get_cstr(temp_str), sizeof(data->name));
        if(!flipper_file_get_value_count(file, "Number", &data_cnt)) break;
        data->number_len = data_cnt;
        if(!flipper_file_read_hex(file, "Number", data->number, data->number_len)) break;
        parsed = true;
        // Load optional data
        uint8_t exp_data[2] = {};
        if(flipper_file_read_hex(file, "Exp data", exp_data, 2)) {
            data->exp_mon = exp_data[0];
            data->exp_year = exp_data[1];
        }
        if(flipper_file_read_uint32(file, "Country code", &data_cnt, 1)) {
            data->country_code = data_cnt;
        }
        if(flipper_file_read_uint32(file, "Currency code", &data_cnt, 1)) {
            data->currency_code = data_cnt;
        }
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
    FlipperFile* file = flipper_file_alloc(dev->storage);
    NfcDeviceCommonData* data = &dev->dev_data.nfc_data;
    string_t temp_str;
    string_init(temp_str);

    do {
        // Create nfc directory if necessary
        if(!storage_simply_mkdir(dev->storage, nfc_app_folder)) break;
        // First remove nfc device file if it was saved
        string_printf(temp_str, "%s/%s%s", folder, dev_name, extension);
        // Open file
        if(!flipper_file_open_always(file, string_get_cstr(temp_str))) break;
        // Write header
        if(!flipper_file_write_header_cstr(file, nfc_file_header, nfc_file_version)) break;
        // Write nfc device type
        if(!flipper_file_write_comment_cstr(
               file, "Nfc device type can be UID, Mifare Ultralight, Bank card"))
            break;
        nfc_device_prepare_format_string(dev, temp_str);
        if(!flipper_file_write_string(file, "Device type", temp_str)) break;
        // Write UID, ATQA, SAK
        if(!flipper_file_write_comment_cstr(file, "UID, ATQA and SAK are common for all formats"))
            break;
        if(!flipper_file_write_hex(file, "UID", data->uid, data->uid_len)) break;
        if(!flipper_file_write_hex(file, "ATQA", data->atqa, 2)) break;
        if(!flipper_file_write_hex(file, "SAK", &data->sak, 1)) break;
        // Save more data if necessary
        if(dev->format == NfcDeviceSaveFormatMifareUl) {
            if(!nfc_device_save_mifare_ul_data(file, dev)) break;
        } else if(dev->format == NfcDeviceSaveFormatBankCard) {
            if(!nfc_device_save_bank_card_data(file, dev)) break;
        }
        saved = true;
    } while(0);

    if(!saved) {
        dialog_message_show_storage_error(dev->dialogs, "Can not save\nkey file");
    }
    string_clear(temp_str);
    flipper_file_close(file);
    flipper_file_free(file);
    return saved;
}

bool nfc_device_save(NfcDevice* dev, const char* dev_name) {
    return nfc_device_save_file(dev, dev_name, nfc_app_folder, nfc_app_extension);
}

bool nfc_device_save_shadow(NfcDevice* dev, const char* dev_name) {
    dev->shadow_file_exist = true;
    return nfc_device_save_file(dev, dev_name, nfc_app_folder, nfc_app_shadow_extension);
}

static bool nfc_device_load_data(NfcDevice* dev, string_t path) {
    bool parsed = false;
    FlipperFile* file = flipper_file_alloc(dev->storage);
    NfcDeviceCommonData* data = &dev->dev_data.nfc_data;
    uint32_t data_cnt = 0;
    string_t temp_str;
    string_init(temp_str);
    bool depricated_version = false;

    do {
        // Check existance of shadow file
        size_t ext_start = string_search_str(path, nfc_app_extension);
        string_set_n(temp_str, path, 0, ext_start);
        string_cat_printf(temp_str, "%s", nfc_app_shadow_extension);
        dev->shadow_file_exist =
            storage_common_stat(dev->storage, string_get_cstr(temp_str), NULL) == FSE_OK;
        // Open shadow file if it exists. If not - open original
        if(dev->shadow_file_exist) {
            if(!flipper_file_open_existing(file, string_get_cstr(temp_str))) break;
        } else {
            if(!flipper_file_open_existing(file, string_get_cstr(path))) break;
        }
        // Read and verify file header
        uint32_t version = 0;
        if(!flipper_file_read_header(file, temp_str, &version)) break;
        if(string_cmp_str(temp_str, nfc_file_header) || (version != nfc_file_version)) {
            depricated_version = true;
            break;
        }
        // Read Nfc device type
        if(!flipper_file_read_string(file, "Device type", temp_str)) break;
        if(!nfc_device_parse_format_string(dev, temp_str)) break;
        // Read and parse UID, ATQA and SAK
        if(!flipper_file_get_value_count(file, "UID", &data_cnt)) break;
        data->uid_len = data_cnt;
        if(!flipper_file_read_hex(file, "UID", data->uid, data->uid_len)) break;
        if(!flipper_file_read_hex(file, "ATQA", data->atqa, 2)) break;
        if(!flipper_file_read_hex(file, "SAK", &data->sak, 1)) break;
        // Parse other data
        if(dev->format == NfcDeviceSaveFormatMifareUl) {
            if(!nfc_device_load_mifare_ul_data(file, dev)) break;
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
    flipper_file_close(file);
    flipper_file_free(file);
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
        nfc_app_folder,
        nfc_app_extension,
        dev->file_name,
        sizeof(dev->file_name),
        dev->dev_name);
    if(res) {
        string_t dev_str;
        // Get key file path
        string_init_printf(dev_str, "%s/%s%s", nfc_app_folder, dev->file_name, nfc_app_extension);
        res = nfc_device_load_data(dev, dev_str);
        if(res) {
            nfc_device_set_name(dev, dev->file_name);
        }
        string_clear(dev_str);
    }

    return res;
}

void nfc_device_clear(NfcDevice* dev) {
    furi_assert(dev);

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
        string_init_printf(file_path, "%s/%s%s", nfc_app_folder, dev->dev_name, nfc_app_extension);
        if(!storage_simply_remove(dev->storage, string_get_cstr(file_path))) break;
        // Delete shadow file if it exists
        if(dev->shadow_file_exist) {
            string_printf(
                file_path, "%s/%s%s", nfc_app_folder, dev->dev_name, nfc_app_shadow_extension);
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
            path, "%s/%s%s", nfc_app_folder, dev->dev_name, nfc_app_shadow_extension);
        if(!storage_simply_remove(dev->storage, string_get_cstr(path))) break;
        dev->shadow_file_exist = false;
        string_printf(path, "%s/%s%s", nfc_app_folder, dev->dev_name, nfc_app_extension);
        if(!nfc_device_load_data(dev, path)) break;
        restored = true;
    } while(0);

    string_clear(path);
    return restored;
}
