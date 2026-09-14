#include "lfrfid_settings.h"

#include <furi.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#define TAG "LfRfidSettings"

// Private to this file: the app, the CLI and third-party apps go through the accessors, so the
// stored layout is free to change. Bump the version whenever it does - but a bump rejects an
// existing file whole, taking every other choice the user made with it. Appending a write target
// is not a layout change: an older file reads the new bit as 0, so bump for that only if the new
// target should default to on.
#define LFRFID_SETTINGS_FOLDER  EXT_PATH("lfrfid")
#define LFRFID_SETTINGS_PATH    LFRFID_SETTINGS_FOLDER "/.lfrfid.settings"
#define LFRFID_SETTINGS_VERSION (1)
#define LFRFID_SETTINGS_MAGIC   (0x4C)

typedef struct {
    LFRFIDWriteTargetMask write_target_mask;
} LFRFIDSettings;

LFRFIDWriteTargetMask lfrfid_settings_get_write_targets(void) {
    // Stat before loading, not after: no file is the normal state until the user changes
    // something, and saved_struct_load() logs a missing file at E. This runs on every write.
    // storage_file_exists() would do, but it folds "no file" together with "the SD is not
    // there". The fallback is the same either way, but the default is a subset of what a user
    // can enable, so the second case is worth a log line rather than silence.
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FS_Error stat = storage_common_stat(storage, LFRFID_SETTINGS_PATH, NULL);
    furi_record_close(RECORD_STORAGE);

    LFRFIDSettings settings;

    if(stat == FSE_OK && saved_struct_load(
                             LFRFID_SETTINGS_PATH,
                             &settings,
                             sizeof(LFRFIDSettings),
                             LFRFID_SETTINGS_MAGIC,
                             LFRFID_SETTINGS_VERSION)) {
        return settings.write_target_mask;
    }

    // saved_struct logs the cause of a bad file; this is the consequence either way - a choice
    // the user made is gone.
    if(stat != FSE_NOT_EXIST) {
        FURI_LOG_W(
            TAG,
            "%s unreadable (%s), restoring the default write targets",
            LFRFID_SETTINGS_PATH,
            storage_error_get_desc(stat));
    }

    return lfrfid_write_targets_default();
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
