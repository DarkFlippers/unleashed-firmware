#include "picopass_device.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

#define TAG "PicopassDevice"

static const char* picopass_file_header = "Flipper Picopass device";
static const uint32_t picopass_file_version = 1;

const uint8_t picopass_iclass_decryptionkey[] =
    {0xb4, 0x21, 0x2c, 0xca, 0xb7, 0xed, 0x21, 0x0f, 0x7b, 0x93, 0xd4, 0x59, 0x39, 0xc7, 0xdd, 0x36};

PicopassDevice* picopass_device_alloc() {
    PicopassDevice* picopass_dev = malloc(sizeof(PicopassDevice));
    picopass_dev->dev_data.pacs.legacy = false;
    picopass_dev->dev_data.pacs.se_enabled = false;
    picopass_dev->dev_data.pacs.pin_length = 0;
    picopass_dev->storage = furi_record_open(RECORD_STORAGE);
    picopass_dev->dialogs = furi_record_open(RECORD_DIALOGS);
    string_init(picopass_dev->load_path);
    return picopass_dev;
}

void picopass_device_set_name(PicopassDevice* dev, const char* name) {
    furi_assert(dev);

    strlcpy(dev->dev_name, name, PICOPASS_DEV_NAME_MAX_LEN);
}

static bool picopass_device_save_file(
    PicopassDevice* dev,
    const char* dev_name,
    const char* folder,
    const char* extension,
    bool use_load_path) {
    furi_assert(dev);

    bool saved = false;
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);
    PicopassPacs* pacs = &dev->dev_data.pacs;
    PicopassBlock* AA1 = dev->dev_data.AA1;
    string_t temp_str;
    string_init(temp_str);

    do {
        if(use_load_path && !string_empty_p(dev->load_path)) {
            // Get directory name
            path_extract_dirname(string_get_cstr(dev->load_path), temp_str);
            // Create picopass directory if necessary
            if(!storage_simply_mkdir(dev->storage, string_get_cstr(temp_str))) break;
            // Make path to file to save
            string_cat_printf(temp_str, "/%s%s", dev_name, extension);
        } else {
            // Create picopass directory if necessary
            if(!storage_simply_mkdir(dev->storage, PICOPASS_APP_FOLDER)) break;
            // First remove picopass device file if it was saved
            string_printf(temp_str, "%s/%s%s", folder, dev_name, extension);
        }
        // Open file
        if(!flipper_format_file_open_always(file, string_get_cstr(temp_str))) break;

        if(dev->format == PicopassDeviceSaveFormatHF) {
            uint32_t fc = pacs->record.FacilityCode;
            uint32_t cn = pacs->record.CardNumber;
            // Write header
            if(!flipper_format_write_header_cstr(file, picopass_file_header, picopass_file_version))
                break;
            if(pacs->record.valid) {
                if(!flipper_format_write_uint32(file, "Facility Code", &fc, 1)) break;
                if(!flipper_format_write_uint32(file, "Card Number", &cn, 1)) break;
                if(!flipper_format_write_hex(
                       file, "Credential", pacs->credential, PICOPASS_BLOCK_LEN))
                    break;
                if(pacs->pin_length > 0) {
                    if(!flipper_format_write_hex(file, "PIN\t\t", pacs->pin0, PICOPASS_BLOCK_LEN))
                        break;
                    if(!flipper_format_write_hex(
                           file, "PIN(cont.)\t", pacs->pin1, PICOPASS_BLOCK_LEN))
                        break;
                }
            }
            if(!flipper_format_write_comment_cstr(file, "Picopass blocks")) break;
            bool block_saved = true;

            size_t app_limit = AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[0] < PICOPASS_MAX_APP_LIMIT ?
                                   AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[0] :
                                   PICOPASS_MAX_APP_LIMIT;
            for(size_t i = 0; i < app_limit; i++) {
                string_printf(temp_str, "Block %d", i);
                if(!flipper_format_write_hex(
                       file, string_get_cstr(temp_str), AA1[i].data, PICOPASS_BLOCK_LEN)) {
                    block_saved = false;
                    break;
                }
            }
            if(!block_saved) break;
        } else if(dev->format == PicopassDeviceSaveFormatLF) {
            const char* lf_header = "Flipper RFID key";
            // Write header
            if(!flipper_format_write_header_cstr(file, lf_header, 1)) break;
            if(!flipper_format_write_comment_cstr(
                   file,
                   "This was generated from the Picopass plugin and may not match current lfrfid"))
                break;
            // When lfrfid supports more formats, update this
            if(!flipper_format_write_string_cstr(file, "Key type", "H10301")) break;
            uint8_t H10301[3] = {0};
            H10301[0] = pacs->record.FacilityCode;
            H10301[1] = pacs->record.CardNumber >> 8;
            H10301[2] = pacs->record.CardNumber & 0x00FF;
            if(!flipper_format_write_hex(file, "Data", H10301, 3)) break;
        }
        saved = true;
    } while(0);

    if(!saved) {
        dialog_message_show_storage_error(dev->dialogs, "Can not save\nfile");
    }
    string_clear(temp_str);
    flipper_format_free(file);
    return saved;
}

bool picopass_device_save(PicopassDevice* dev, const char* dev_name) {
    if(dev->format == PicopassDeviceSaveFormatHF) {
        return picopass_device_save_file(
            dev, dev_name, PICOPASS_APP_FOLDER, PICOPASS_APP_EXTENSION, true);
    } else if(dev->format == PicopassDeviceSaveFormatLF) {
        return picopass_device_save_file(dev, dev_name, ANY_PATH("lfrfid"), ".rfid", true);
    }
    return false;
}

static bool picopass_device_load_data(PicopassDevice* dev, string_t path, bool show_dialog) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(dev->storage);
    PicopassBlock* AA1 = dev->dev_data.AA1;
    PicopassPacs* pacs = &dev->dev_data.pacs;
    string_t temp_str;
    string_init(temp_str);
    bool deprecated_version = false;

    if(dev->loading_cb) {
        dev->loading_cb(dev->loading_cb_ctx, true);
    }

    do {
        if(!flipper_format_file_open_existing(file, string_get_cstr(path))) break;

        // Read and verify file header
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(string_cmp_str(temp_str, picopass_file_header) || (version != picopass_file_version)) {
            deprecated_version = true;
            break;
        }

        // Parse header blocks
        bool block_read = true;
        for(size_t i = 0; i < 6; i++) {
            string_printf(temp_str, "Block %d", i);
            if(!flipper_format_read_hex(
                   file, string_get_cstr(temp_str), AA1[i].data, PICOPASS_BLOCK_LEN)) {
                block_read = false;
                break;
            }
        }

        size_t app_limit = AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[0];
        for(size_t i = 6; i < app_limit; i++) {
            string_printf(temp_str, "Block %d", i);
            if(!flipper_format_read_hex(
                   file, string_get_cstr(temp_str), AA1[i].data, PICOPASS_BLOCK_LEN)) {
                block_read = false;
                break;
            }
        }
        if(!block_read) break;

        if(picopass_device_parse_credential(AA1, pacs) != ERR_NONE) break;
        if(picopass_device_parse_wiegand(pacs->credential, &pacs->record) != ERR_NONE) break;

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

void picopass_device_clear(PicopassDevice* dev) {
    furi_assert(dev);

    picopass_device_data_clear(&dev->dev_data);
    memset(&dev->dev_data, 0, sizeof(dev->dev_data));
    dev->format = PicopassDeviceSaveFormatHF;
    string_reset(dev->load_path);
}

void picopass_device_free(PicopassDevice* picopass_dev) {
    furi_assert(picopass_dev);
    picopass_device_clear(picopass_dev);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);
    string_clear(picopass_dev->load_path);
    free(picopass_dev);
}

bool picopass_file_select(PicopassDevice* dev) {
    furi_assert(dev);

    // Input events and views are managed by file_browser
    string_t picopass_app_folder;
    string_init_set_str(picopass_app_folder, PICOPASS_APP_FOLDER);
    bool res = dialog_file_browser_show(
        dev->dialogs,
        dev->load_path,
        picopass_app_folder,
        PICOPASS_APP_EXTENSION,
        true,
        &I_Nfc_10px,
        true);
    string_clear(picopass_app_folder);
    if(res) {
        string_t filename;
        string_init(filename);
        path_extract_filename(dev->load_path, filename, true);
        strncpy(dev->dev_name, string_get_cstr(filename), PICOPASS_DEV_NAME_MAX_LEN);
        res = picopass_device_load_data(dev, dev->load_path, true);
        if(res) {
            picopass_device_set_name(dev, dev->dev_name);
        }
        string_clear(filename);
    }

    return res;
}

void picopass_device_data_clear(PicopassDeviceData* dev_data) {
    for(size_t i = 0; i < PICOPASS_MAX_APP_LIMIT; i++) {
        memset(dev_data->AA1[i].data, 0, sizeof(dev_data->AA1[i].data));
    }
    dev_data->pacs.legacy = false;
    dev_data->pacs.se_enabled = false;
    dev_data->pacs.pin_length = 0;
}

bool picopass_device_delete(PicopassDevice* dev, bool use_load_path) {
    furi_assert(dev);

    bool deleted = false;
    string_t file_path;
    string_init(file_path);

    do {
        // Delete original file
        if(use_load_path && !string_empty_p(dev->load_path)) {
            string_set(file_path, dev->load_path);
        } else {
            string_printf(
                file_path, "%s/%s%s", PICOPASS_APP_FOLDER, dev->dev_name, PICOPASS_APP_EXTENSION);
        }
        if(!storage_simply_remove(dev->storage, string_get_cstr(file_path))) break;
        deleted = true;
    } while(0);

    if(!deleted) {
        dialog_message_show_storage_error(dev->dialogs, "Can not remove file");
    }

    string_clear(file_path);
    return deleted;
}

void picopass_device_set_loading_callback(
    PicopassDevice* dev,
    PicopassLoadingCallback callback,
    void* context) {
    furi_assert(dev);

    dev->loading_cb = callback;
    dev->loading_cb_ctx = context;
}

ReturnCode picopass_device_decrypt(uint8_t* enc_data, uint8_t* dec_data) {
    uint8_t key[32] = {0};
    memcpy(key, picopass_iclass_decryptionkey, sizeof(picopass_iclass_decryptionkey));
    mbedtls_des3_context ctx;
    mbedtls_des3_init(&ctx);
    mbedtls_des3_set2key_dec(&ctx, key);
    mbedtls_des3_crypt_ecb(&ctx, enc_data, dec_data);
    mbedtls_des3_free(&ctx);
    return ERR_NONE;
}

ReturnCode picopass_device_parse_credential(PicopassBlock* AA1, PicopassPacs* pacs) {
    ReturnCode err;

    // Thank you proxmark!
    pacs->legacy = (memcmp(AA1[5].data, "\xff\xff\xff\xff\xff\xff\xff\xff", 8) == 0);
    pacs->se_enabled = (memcmp(AA1[5].data, "\xff\xff\xff\x00\x06\xff\xff\xff", 8) == 0);

    pacs->biometrics = AA1[6].data[4];
    pacs->pin_length = AA1[6].data[6] & 0x0F;
    pacs->encryption = AA1[6].data[7];

    if(pacs->encryption == PicopassDeviceEncryption3DES) {
        FURI_LOG_D(TAG, "3DES Encrypted");
        err = picopass_device_decrypt(AA1[7].data, pacs->credential);
        if(err != ERR_NONE) {
            FURI_LOG_E(TAG, "decrypt error %d", err);
            return err;
        }

        err = picopass_device_decrypt(AA1[8].data, pacs->pin0);
        if(err != ERR_NONE) {
            FURI_LOG_E(TAG, "decrypt error %d", err);
            return err;
        }

        err = picopass_device_decrypt(AA1[9].data, pacs->pin1);
        if(err != ERR_NONE) {
            FURI_LOG_E(TAG, "decrypt error %d", err);
            return err;
        }
    } else if(pacs->encryption == PicopassDeviceEncryptionNone) {
        FURI_LOG_D(TAG, "No Encryption");
        memcpy(pacs->credential, AA1[7].data, PICOPASS_BLOCK_LEN);
        memcpy(pacs->pin0, AA1[8].data, PICOPASS_BLOCK_LEN);
        memcpy(pacs->pin1, AA1[9].data, PICOPASS_BLOCK_LEN);
    } else if(pacs->encryption == PicopassDeviceEncryptionDES) {
        FURI_LOG_D(TAG, "DES Encrypted");
    } else {
        FURI_LOG_D(TAG, "Unknown encryption");
    }

    return ERR_NONE;
}

ReturnCode picopass_device_parse_wiegand(uint8_t* data, PicopassWiegandRecord* record) {
    uint32_t* halves = (uint32_t*)data;
    if(halves[0] == 0) {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[1]));
        record->bitLength = 31 - leading0s;
    } else {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[0]));
        record->bitLength = 63 - leading0s;
    }
    FURI_LOG_D(TAG, "bitLength: %d", record->bitLength);

    if(record->bitLength == 26) {
        uint8_t* v4 = data + 4;
        uint32_t bot = v4[3] | (v4[2] << 8) | (v4[1] << 16) | (v4[0] << 24);

        record->CardNumber = (bot >> 1) & 0xFFFF;
        record->FacilityCode = (bot >> 17) & 0xFF;
        FURI_LOG_D(TAG, "FC:%u CN: %u\n", record->FacilityCode, record->CardNumber);
        record->valid = true;
    } else {
        record->CardNumber = 0;
        record->FacilityCode = 0;
        record->valid = false;
    }
    return ERR_NONE;
}
