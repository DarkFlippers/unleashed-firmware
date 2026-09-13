#include "lfrfid_settings.h"

#include <furi.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#define TAG "LfRfidSettings"

// Private to this file: the app, the CLI and third-party apps go through the accessors, so the
// stored layout is free to change. Bump the version whenever it does - including when a write
// target is appended - but note that a bump makes an existing file be rejected whole, so every
// other choice the user made goes back to default with it.
#define LFRFID_SETTINGS_FOLDER  EXT_PATH("lfrfid")
#define LFRFID_SETTINGS_PATH    LFRFID_SETTINGS_FOLDER "/.lfrfid.settings"
#define LFRFID_SETTINGS_VERSION (1)
#define LFRFID_SETTINGS_MAGIC   (0x4C)

typedef struct {
    LFRFIDWriteTargetMask write_target_mask;
} LFRFIDSettings;

LFRFIDWriteTargetMask lfrfid_settings_get_write_targets(void) {
    // Checked before loading, not after: no file is the normal state until the user changes
    // something, and saved_struct_load() logs a missing file at E. This runs on every write.
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool exists = storage_file_exists(storage, LFRFID_SETTINGS_PATH);
    furi_record_close(RECORD_STORAGE);

    LFRFIDSettings settings;

    if(exists && saved_struct_load(
                     LFRFID_SETTINGS_PATH,
                     &settings,
                     sizeof(LFRFIDSettings),
                     LFRFID_SETTINGS_MAGIC,
                     LFRFID_SETTINGS_VERSION)) {
        return settings.write_target_mask;
    }

    // saved_struct logs the cause; this is the consequence - a choice the user made is gone.
    if(exists) {
        FURI_LOG_W(TAG, "%s unreadable, re-enabling every write target", LFRFID_SETTINGS_PATH);
    }

    return LFRFID_WRITE_TARGET_MASK_ALL;
}

bool lfrfid_settings_set_write_targets(LFRFIDWriteTargetMask mask) {
    LFRFIDSettings settings = {.write_target_mask = mask & LFRFID_WRITE_TARGET_MASK_ALL};

    // Defensive: the app's resources normally create this folder, but the user can delete it.
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, LFRFID_SETTINGS_FOLDER);
    furi_record_close(RECORD_STORAGE);

    bool result = saved_struct_save(
        LFRFID_SETTINGS_PATH,
        &settings,
        sizeof(LFRFIDSettings),
        LFRFID_SETTINGS_MAGIC,
        LFRFID_SETTINGS_VERSION);

    if(!result) {
        FURI_LOG_E(TAG, "Failed to save %s", LFRFID_SETTINGS_PATH);
    }

    return result;
}
