#include <furi.h>
#include <file-worker.h>
#include "desktop_settings.h"

#define DESKTOP_SETTINGS_TAG "Desktop settings"
#define DESKTOP_SETTINGS_PATH "/int/desktop.settings"

bool desktop_settings_load(DesktopSettings* desktop_settings) {
    furi_assert(desktop_settings);
    bool file_loaded = false;
    DesktopSettings settings = {};

    FURI_LOG_I(DESKTOP_SETTINGS_TAG, "Loading settings from \"%s\"", DESKTOP_SETTINGS_PATH);
    FileWorker* file_worker = file_worker_alloc(true);
    if(file_worker_open(file_worker, DESKTOP_SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(file_worker_read(file_worker, &settings, sizeof(settings))) {
            file_loaded = true;
        }
    }
    file_worker_free(file_worker);

    if(file_loaded) {
        if(settings.version != DESKTOP_SETTINGS_VER) {
            FURI_LOG_E(DESKTOP_SETTINGS_TAG, "Settings version mismatch");
        } else {
            osKernelLock();
            *desktop_settings = settings;
            osKernelUnlock();
        }
    } else {
        FURI_LOG_E(DESKTOP_SETTINGS_TAG, "Settings load failed");
    }
    return file_loaded;
}

bool desktop_settings_save(DesktopSettings* desktop_settings) {
    furi_assert(desktop_settings);
    bool result = false;

    FileWorker* file_worker = file_worker_alloc(true);
    if(file_worker_open(file_worker, DESKTOP_SETTINGS_PATH, FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        if(file_worker_write(file_worker, desktop_settings, sizeof(DesktopSettings))) {
            FURI_LOG_I(DESKTOP_SETTINGS_TAG, "Settings saved to \"%s\"", DESKTOP_SETTINGS_PATH);
            result = true;
        }
    }
    file_worker_free(file_worker);
    return result;
}
