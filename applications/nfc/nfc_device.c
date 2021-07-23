#include "nfc_device_i.h"

#include <file-worker.h>
#include <path.h>
#include <hex.h>

#define NFC_DEVICE_MAX_DATA_LEN 14

static const char* nfc_app_folder = "/any/nfc";
static const char* nfc_app_extension = ".nfc";

static bool nfc_device_read_hex(string_t str, uint8_t* buff, uint16_t len) {
    string_strim(str);
    uint8_t nibble_high = 0;
    uint8_t nibble_low = 0;
    bool parsed = true;

    for(uint16_t i = 0; i < len; i++) {
        if(hex_char_to_hex_nibble(string_get_char(str, 0), &nibble_high) &&
           hex_char_to_hex_nibble(string_get_char(str, 1), &nibble_low)) {
            buff[i] = (nibble_high << 4) | nibble_low;
            string_right(str, 3);
        } else {
            parsed = false;
            break;
        }
    }
    return parsed;
}

uint16_t nfc_device_prepare_format_string(NfcDevice* dev, string_t format_string) {
    if(dev->format == NfcDeviceSaveFormatUid) {
        string_set_str(format_string, "UID\n");
    } else if(dev->format == NfcDeviceSaveFormatBankCard) {
        string_set_str(format_string, "Bank card\n");
    } else if(dev->format == NfcDeviceSaveFormatMifareUl) {
        string_set_str(format_string, "Mifare Ultralight\n");
    } else {
        string_set_str(format_string, "Unknown\n");
    }
    return string_size(format_string);
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

uint16_t nfc_device_prepare_uid_string(NfcDevice* dev, string_t uid_string) {
    NfcDeviceCommomData* uid_data = &dev->dev_data.nfc_data;
    string_printf(uid_string, "UID len: %02X UID: ", dev->dev_data.nfc_data.uid_len);
    for(uint8_t i = 0; i < uid_data->uid_len; i++) {
        string_cat_printf(uid_string, "%02X ", uid_data->uid[i]);
    }
    string_cat_printf(
        uid_string,
        "ATQA: %02X %02X SAK: %02X\n",
        uid_data->atqa[0],
        uid_data->atqa[1],
        uid_data->sak);
    return string_size(uid_string);
}

bool nfc_device_parse_uid_string(NfcDevice* dev, string_t uid_string) {
    NfcDeviceCommomData* uid_data = &dev->dev_data.nfc_data;
    bool parsed = false;

    do {
        // strlen("UID len: ") = 9
        string_right(uid_string, 9);
        if(!nfc_device_read_hex(uid_string, &uid_data->uid_len, 1)) {
            break;
        }
        // strlen("UID: ") = 5
        string_right(uid_string, 5);
        if(!nfc_device_read_hex(uid_string, uid_data->uid, uid_data->uid_len)) {
            break;
        }
        // strlen("ATQA: ") = 6
        string_right(uid_string, 6);
        if(!nfc_device_read_hex(uid_string, uid_data->atqa, 2)) {
            break;
        }
        // strlen("SAK: ") = 5
        string_right(uid_string, 5);
        if(!nfc_device_read_hex(uid_string, &uid_data->sak, 1)) {
            break;
        }
        parsed = true;
    } while(0);

    return parsed;
}

uint16_t nfc_device_prepare_mifare_ul_string(NfcDevice* dev, string_t mifare_ul_string) {
    MifareUlData* data = &dev->dev_data.mf_ul_data;
    string_printf(mifare_ul_string, "Signature:");
    for(uint8_t i = 0; i < sizeof(data->signature); i++) {
        string_cat_printf(mifare_ul_string, " %02X", data->signature[i]);
    }
    string_cat_printf(mifare_ul_string, "\nVersion:");
    uint8_t* version = (uint8_t*)&data->version;
    for(uint8_t i = 0; i < sizeof(data->version); i++) {
        string_cat_printf(mifare_ul_string, " %02X", version[i]);
    }
    for(uint8_t i = 0; i < 3; i++) {
        string_cat_printf(
            mifare_ul_string,
            "\nCounter %d: %lu Tearing flag %d: %02X",
            i,
            data->counter[i],
            i,
            data->tearing[i]);
    }
    string_cat_printf(mifare_ul_string, "\nData size: %d\n", data->data_size);
    for(uint16_t i = 0; i < data->data_size; i += 4) {
        string_cat_printf(
            mifare_ul_string,
            "%02X %02X %02X %02X\n",
            data->data[i],
            data->data[i + 1],
            data->data[i + 2],
            data->data[i + 3]);
    }
    return string_size(mifare_ul_string);
}

bool nfc_device_parse_mifare_ul_string(NfcDevice* dev, string_t mifare_ul_string) {
    MifareUlData* data = &dev->dev_data.mf_ul_data;
    uint16_t tearing_tmp = 0;
    uint16_t cnt_num = 0;
    size_t ws = 0;
    int res = 0;
    bool parsed = false;

    do {
        // strlen("Signature: ") = 11
        string_right(mifare_ul_string, 11);
        if(!nfc_device_read_hex(mifare_ul_string, data->signature, sizeof(data->signature))) {
            break;
        }
        // strlen("Version: ") = 9
        string_right(mifare_ul_string, 9);
        if(!nfc_device_read_hex(
               mifare_ul_string, (uint8_t*)&data->version, sizeof(data->version))) {
            break;
        }
        string_strim(mifare_ul_string);
        // Read counters and tearing flags
        for(uint8_t i = 0; i < 3; i++) {
            res = sscanf(
                string_get_cstr(mifare_ul_string),
                "Counter %hX: %lu Tearing flag %hX: %02hX",
                &cnt_num,
                &data->counter[i],
                &cnt_num,
                &tearing_tmp);
            if(res != 4) {
                break;
            }
            data->tearing[i] = tearing_tmp;
            ws = string_search_char(mifare_ul_string, '\n');
            string_right(mifare_ul_string, ws + 1);
        }
        // Read data size
        res = sscanf(string_get_cstr(mifare_ul_string), "Data size: %hu", &data->data_size);
        if(res != 1) {
            break;
        }
        ws = string_search_char(mifare_ul_string, '\n');
        string_right(mifare_ul_string, ws + 1);
        // Read data
        for(uint16_t i = 0; i < data->data_size; i += 4) {
            if(!nfc_device_read_hex(mifare_ul_string, &data->data[i], 4)) {
                break;
            }
        }
        parsed = true;
    } while(0);

    return parsed;
}

uint16_t nfc_device_prepare_bank_card_string(NfcDevice* dev, string_t bank_card_string) {
    NfcEmvData* data = &dev->dev_data.emv_data;
    string_printf(bank_card_string, "AID len: %d, AID:", data->aid_len);
    for(uint8_t i = 0; i < data->aid_len; i++) {
        string_cat_printf(bank_card_string, " %02X", data->aid[i]);
    }
    string_cat_printf(bank_card_string, "\nName: %s\nNumber:", data->name);
    for(uint8_t i = 0; i < sizeof(data->number); i++) {
        string_cat_printf(bank_card_string, " %02X", data->number[i]);
    }
    return string_size(bank_card_string);
}

bool nfc_device_parse_bank_card_string(NfcDevice* dev, string_t bank_card_string) {
    NfcEmvData* data = &dev->dev_data.emv_data;
    bool parsed = false;
    int res = 0;
    memset(data, 0, sizeof(NfcEmvData));

    do {
        res = sscanf(string_get_cstr(bank_card_string), "AID len: %hu", &data->aid_len);
        if(res != 1) {
            break;
        }
        // strlen("AID len: ") = 9
        string_right(bank_card_string, 9);
        size_t ws = string_search_char(bank_card_string, ':');
        string_right(bank_card_string, ws + 1);
        if(!nfc_device_read_hex(bank_card_string, data->aid, data->aid_len)) {
            break;
        }
        res = sscanf(string_get_cstr(bank_card_string), "Name: %s\n", data->name);
        if(res != 1) {
            break;
        }
        ws = string_search_char(bank_card_string, '\n');
        string_right(bank_card_string, ws + 1);
        // strlen("Number: ") = 8
        string_right(bank_card_string, 8);
        if(!nfc_device_read_hex(bank_card_string, data->number, sizeof(data->number))) {
            break;
        }
        parsed = true;
    } while(0);

    return parsed;
}

void nfc_device_set_name(NfcDevice* dev, const char* name) {
    furi_assert(dev);

    strlcpy(dev->dev_name, name, NFC_DEV_NAME_MAX_LEN);
}

bool nfc_device_save(NfcDevice* dev, const char* dev_name) {
    furi_assert(dev);

    FileWorker* file_worker = file_worker_alloc(false);
    string_t dev_file_name;
    string_init(dev_file_name);
    string_t temp_str;
    string_init(temp_str);
    uint16_t string_len = 0;

    do {
        // Create nfc directory if necessary
        if(!file_worker_mkdir(file_worker, nfc_app_folder)) {
            break;
        };
        // First remove nfc device file if it was saved
        string_printf(dev_file_name, "%s/%s%s", nfc_app_folder, dev_name, nfc_app_extension);
        if(!file_worker_remove(file_worker, string_get_cstr(dev_file_name))) {
            break;
        };
        // Open file
        if(!file_worker_open(
               file_worker, string_get_cstr(dev_file_name), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }
        // Prepare and write format name on 1st line
        string_len = nfc_device_prepare_format_string(dev, temp_str);
        if(!file_worker_write(file_worker, string_get_cstr(temp_str), string_len)) {
            break;
        }
        // Prepare and write UID data on 2nd line
        string_len = nfc_device_prepare_uid_string(dev, temp_str);
        if(!file_worker_write(file_worker, string_get_cstr(temp_str), string_len)) {
            break;
        }
        // Save more data if necessary
        if(dev->format == NfcDeviceSaveFormatMifareUl) {
            string_len = nfc_device_prepare_mifare_ul_string(dev, temp_str);
            if(!file_worker_write(file_worker, string_get_cstr(temp_str), string_len)) {
                break;
            }
        } else if(dev->format == NfcDeviceSaveFormatBankCard) {
            string_len = nfc_device_prepare_bank_card_string(dev, temp_str);
            if(!file_worker_write(file_worker, string_get_cstr(temp_str), string_len)) {
                break;
            }
        }
    } while(0);

    string_clear(temp_str);
    string_clear(dev_file_name);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return true;
}

static bool nfc_device_load_data(FileWorker* file_worker, string_t path, NfcDevice* dev) {
    string_t temp_string;
    string_init(temp_string);
    bool parsed = false;

    do {
        // Open key file
        if(!file_worker_open(file_worker, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }
        // Read and parse format from 1st line
        if(!file_worker_read_until(file_worker, temp_string, '\n')) {
            break;
        }
        if(!nfc_device_parse_format_string(dev, temp_string)) {
            break;
        }
        // Read and parse UID data from 2nd line
        if(!file_worker_read_until(file_worker, temp_string, '\n')) {
            break;
        }
        if(!nfc_device_parse_uid_string(dev, temp_string)) {
            break;
        }
        // Parse other data
        if(dev->format == NfcDeviceSaveFormatMifareUl) {
            // Read until EOF
            if(!file_worker_read_until(file_worker, temp_string, 0x05)) {
                break;
            }
            if(!nfc_device_parse_mifare_ul_string(dev, temp_string)) {
                break;
            }
        } else if(dev->format == NfcDeviceSaveFormatBankCard) {
            // Read until EOF
            if(!file_worker_read_until(file_worker, temp_string, 0x05)) {
                break;
            }
            if(!nfc_device_parse_bank_card_string(dev, temp_string)) {
                break;
            }
        }
        parsed = true;
    } while(0);

    string_clear(temp_string);
    return parsed;
}

bool nfc_device_load(NfcDevice* dev, const char* file_path) {
    furi_assert(dev);
    furi_assert(file_path);

    FileWorker* file_worker = file_worker_alloc(false);
    // Load device data
    string_t path;
    string_init_set_str(path, file_path);
    bool dev_load = nfc_device_load_data(file_worker, path, dev);
    if(dev_load) {
        // Set device name
        path_extract_filename_no_ext(file_path, path);
        nfc_device_set_name(dev, string_get_cstr(path));
    }
    string_clear(path);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return dev_load;
}

bool nfc_file_select(NfcDevice* dev) {
    furi_assert(dev);

    FileWorker* file_worker = file_worker_alloc(false);
    // Input events and views are managed by file_select
    bool res = file_worker_file_select(
        file_worker,
        nfc_app_folder,
        nfc_app_extension,
        dev->file_name,
        sizeof(dev->file_name),
        NULL);
    if(res) {
        string_t dev_str;

        // Get key file path
        string_init_printf(dev_str, "%s/%s%s", nfc_app_folder, dev->file_name, nfc_app_extension);

        res = nfc_device_load_data(file_worker, dev_str, dev);
        if(res) {
            nfc_device_set_name(dev, dev->file_name);
        }
        string_clear(dev_str);
    }
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return res;
}

void nfc_device_clear(NfcDevice* dev) {
    furi_assert(dev);

    memset(&dev->dev_data, 0, sizeof(dev->dev_data));
    nfc_device_set_name(dev, "");
    dev->format = NfcDeviceSaveFormatUid;
}

bool nfc_device_delete(NfcDevice* dev) {
    furi_assert(dev);

    bool result = false;
    FileWorker* file_worker = file_worker_alloc(false);
    string_t file_path;
    string_init_printf(file_path, "%s/%s%s", nfc_app_folder, dev->dev_name, nfc_app_extension);
    result = file_worker_remove(file_worker, string_get_cstr(file_path));
    string_clear(file_path);
    file_worker_close(file_worker);
    file_worker_free(file_worker);
    return result;
}
