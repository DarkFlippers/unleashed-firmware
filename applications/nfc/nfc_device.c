#include "nfc_device.h"

#include <file-worker.h>

#define NFC_DEVICE_MAX_DATA_LEN 14

static const char* nfc_app_folder = "nfc";
static const char* nfc_app_extension = ".nfc";

void nfc_device_set_name(NfcDevice* dev, const char* name) {
    furi_assert(dev);

    strlcpy(dev->dev_name, name, NFC_DEV_NAME_MAX_LEN);
}

bool nfc_device_save(NfcDevice* dev, const char* dev_name) {
    furi_assert(dev);

    FileWorker* file_worker = file_worker_alloc(false);
    string_t dev_file_name;

    // Create nfc directory if necessary
    if(!file_worker_mkdir(file_worker, nfc_app_folder)) {
        return false;
    };

    // First remove nfc device file if it was saved
    string_init_printf(dev_file_name, "%s/%s%s", nfc_app_folder, dev_name, nfc_app_extension);
    if(!file_worker_remove(file_worker, string_get_cstr(dev_file_name))) {
        string_clear(dev_file_name);
        return false;
    };

    // Prepare buffer to write
    uint8_t buff[NFC_DEVICE_MAX_DATA_LEN];
    buff[0] = dev->data.uid_len;
    memcpy(&buff[1], dev->data.uid, dev->data.uid_len);
    memcpy(&buff[dev->data.uid_len + 1], dev->data.atqa, 2);
    buff[dev->data.uid_len + 3] = dev->data.sak;

    // Save nfc device
    bool res = file_worker_open(
        file_worker, string_get_cstr(dev_file_name), FSAM_WRITE, FSOM_CREATE_ALWAYS);
    string_clear(dev_file_name);
    if(res) {
        // Write UID length
        if(!file_worker_write_hex(file_worker, buff, dev->data.uid_len + 4)) {
            file_worker_close(file_worker);
            return false;
        }
    }
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return true;
}

static bool nfc_device_load_data(FileWorker* file_worker, string_t path, NfcDevice* dev) {
    // Open key file
    if(!file_worker_open(file_worker, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        return false;
    }

    uint8_t buff[NFC_DEVICE_MAX_DATA_LEN] = {};

    // Load first byte - UID length
    if(!file_worker_read_hex(file_worker, buff, 1)) {
        return false;
    }
    // Read space
    uint8_t space = 0;
    if(!file_worker_read(file_worker, &space, 1)) {
        return false;
    }

    // Load other data
    if(!file_worker_read_hex(file_worker, &buff[1], buff[0] + 3)) {
        return false;
    }

    // Set loaded data
    dev->data.uid_len = buff[0];
    memcpy(dev->data.uid, &buff[1], dev->data.uid_len);
    memcpy(dev->data.atqa, &buff[dev->data.uid_len + 1], 2);
    dev->data.sak = buff[dev->data.uid_len + 3];
    return true;
}

bool nfc_device_load(NfcDevice* dev, const char* dev_name) {
    furi_assert(dev);

    return true;
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
