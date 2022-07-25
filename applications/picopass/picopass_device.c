#include "picopass_device.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

#define TAG "PicopassDevice"

static const char* picopass_file_header = "Flipper Picopass device";
static const uint32_t picopass_file_version = 1;

PicopassDevice* picopass_device_alloc() {
    PicopassDevice* picopass_dev = malloc(sizeof(PicopassDevice));
    picopass_dev->dev_data.pacs.legacy = false;
    picopass_dev->dev_data.pacs.se_enabled = false;
    picopass_dev->dev_data.pacs.pin_length = 0;
    picopass_dev->storage = furi_record_open("storage");
    picopass_dev->dialogs = furi_record_open("dialogs");
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
        dialog_message_show_storage_error(dev->dialogs, "Can not save\nkey file");
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
        return picopass_device_save_file(dev, dev_name, "/any/lfrfid", ".rfid", true);
    }
    return false;
}

void picopass_device_clear(PicopassDevice* dev) {
    furi_assert(dev);

    picopass_device_data_clear(&dev->dev_data);
    memset(&dev->dev_data, 0, sizeof(dev->dev_data));
}

void picopass_device_free(PicopassDevice* picopass_dev) {
    furi_assert(picopass_dev);
    picopass_device_clear(picopass_dev);
    furi_record_close("storage");
    furi_record_close("dialogs");
    string_clear(picopass_dev->load_path);
    free(picopass_dev);
}

void picopass_device_data_clear(PicopassDeviceData* dev_data) {
    for(size_t i = 0; i < PICOPASS_MAX_APP_LIMIT; i++) {
        memset(dev_data->AA1[i].data, 0, sizeof(dev_data->AA1[i].data));
    }
    dev_data->pacs.legacy = false;
    dev_data->pacs.se_enabled = false;
    dev_data->pacs.pin_length = 0;
}
