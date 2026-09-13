#include "lfrfid_settings.h"

#include <furi.h>
#include <toolbox/saved_struct.h>

#define TAG "LfRfidSettings"

void lfrfid_settings_load(LFRFIDSettings* settings) {
    furi_check(settings);

    if(!saved_struct_load(
           LFRFID_SETTINGS_PATH,
           settings,
           sizeof(LFRFIDSettings),
           LFRFID_SETTINGS_MAGIC,
           LFRFID_SETTINGS_VERSION)) {
        FURI_LOG_D(TAG, "Failed to load settings, using defaults");
        settings->write_target_mask = LFRFID_WRITE_TARGET_MASK_ALL;
    }
}

bool lfrfid_settings_save(const LFRFIDSettings* settings) {
    furi_check(settings);

    // The folder only exists once a key has been saved, so a user who opens settings first
    // would otherwise never get a file written.
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, LFRFID_SETTINGS_FOLDER);
    furi_record_close(RECORD_STORAGE);

    bool result = saved_struct_save(
        LFRFID_SETTINGS_PATH,
        settings,
        sizeof(LFRFIDSettings),
        LFRFID_SETTINGS_MAGIC,
        LFRFID_SETTINGS_VERSION);

    if(!result) {
        FURI_LOG_E(TAG, "Failed to save settings");
    }

    return result;
}
