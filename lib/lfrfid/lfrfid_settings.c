#include "lfrfid_settings.h"

#include <furi.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#define TAG "LfRfidSettings"

// Private to this file: the app, the CLI and third-party apps go through the accessors, so the
// stored layout is free to change. Bump the version whenever it does - including when a write
// target is appended, so an existing file does not leave the new chip switched off.
#define LFRFID_SETTINGS_FOLDER  EXT_PATH("lfrfid")
#define LFRFID_SETTINGS_PATH    LFRFID_SETTINGS_FOLDER "/.lfrfid.settings"
#define LFRFID_SETTINGS_VERSION (1)
#define LFRFID_SETTINGS_MAGIC   (0x4C)

typedef struct {
    LFRFIDWriteTargetMask write_target_mask;
} LFRFIDSettings;

static void lfrfid_settings_load(LFRFIDSettings* settings) {
    if(!saved_struct_load(
           LFRFID_SETTINGS_PATH,
           settings,
           sizeof(LFRFIDSettings),
           LFRFID_SETTINGS_MAGIC,
           LFRFID_SETTINGS_VERSION)) {
        // W, not D: a file that exists but cannot be read silently re-enables every chip, which
        // is the opposite of what the user asked for, and this log line is the only trace of it.
        FURI_LOG_W(TAG, "Failed to load %s, enabling every write target", LFRFID_SETTINGS_PATH);

        // Whole struct, so a setting added later cannot be left as stack garbage here.
        *settings = (LFRFIDSettings){.write_target_mask = LFRFID_WRITE_TARGET_MASK_ALL};
    }
}

LFRFIDWriteTargetMask lfrfid_settings_get_write_targets(void) {
    LFRFIDSettings settings;
    lfrfid_settings_load(&settings);

    return settings.write_target_mask;
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
