#include "bt_settings.h"
#include <furi.h>
#include <file_worker.h>

#define TAG "BtSettings"
#define BT_SETTINGS_PATH "/int/bt.settings"

bool bt_settings_load(BtSettings* bt_settings) {
    furi_assert(bt_settings);
    bool file_loaded = false;
    BtSettings settings = {};

    FURI_LOG_I(TAG, "Loading settings from \"%s\"", BT_SETTINGS_PATH);
    FileWorker* file_worker = file_worker_alloc(true);
    if(file_worker_open(file_worker, BT_SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(file_worker_read(file_worker, &settings, sizeof(settings))) {
            file_loaded = true;
        }
    }
    file_worker_free(file_worker);

    if(file_loaded) {
        FURI_LOG_I(TAG, "Settings load success");
        if(settings.version != BT_SETTINGS_VERSION) {
            FURI_LOG_E(TAG, "Settings version mismatch");
        } else {
            osKernelLock();
            *bt_settings = settings;
            osKernelUnlock();
        }
    } else {
        FURI_LOG_E(TAG, "Settings load failed");
    }
    return file_loaded;
}

bool bt_settings_save(BtSettings* bt_settings) {
    furi_assert(bt_settings);
    bool result = false;

    FileWorker* file_worker = file_worker_alloc(true);
    if(file_worker_open(file_worker, BT_SETTINGS_PATH, FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        if(file_worker_write(file_worker, bt_settings, sizeof(BtSettings))) {
            FURI_LOG_I(TAG, "Settings saved to \"%s\"", BT_SETTINGS_PATH);
            result = true;
        }
    }
    file_worker_free(file_worker);
    return result;
}
