#include "ibutton_settings.h"

#include <furi.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#define TAG "IButtonSettings"

// Private to this file: the app, the CLI and third-party apps go through the accessors, so the
// stored layout is free to change. Bump the version whenever it does - but a bump rejects an
// existing file whole, so any setting added beside this one goes with it. Appending a write target
// is not a layout change: an older file reads the new bit as 0, so bump for that only if the new
// target should default to on.
#define IBUTTON_SETTINGS_FOLDER  EXT_PATH("ibutton")
#define IBUTTON_SETTINGS_PATH    IBUTTON_SETTINGS_FOLDER "/.ibutton.settings"
#define IBUTTON_SETTINGS_VERSION (1)
#define IBUTTON_SETTINGS_MAGIC   (0x1B)

typedef struct {
    iButtonWriteTargetMask write_target_mask;
} iButtonSettings;

iButtonWriteTargetMask ibutton_settings_get_write_targets(void) {
    // Stat before loading, not after: no file is the normal state until the user changes
    // something, and saved_struct_load() logs a missing file at E.
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FS_Error stat = storage_common_stat(storage, IBUTTON_SETTINGS_PATH, NULL);
    furi_record_close(RECORD_STORAGE);

    iButtonSettings settings;

    if(stat == FSE_OK) {
        if(saved_struct_load(
               IBUTTON_SETTINGS_PATH,
               &settings,
               sizeof(iButtonSettings),
               IBUTTON_SETTINGS_MAGIC,
               IBUTTON_SETTINGS_VERSION)) {
            // Masked on the way out as well as in: a file from a newer firmware passes the
            // version check and can carry bits this build knows nothing about.
            return settings.write_target_mask & IBUTTON_WRITE_TARGET_MASK_ALL;
        }

        // saved_struct logs which check failed; this is the consequence either way. Remove it
        // rather than leave it: it cannot be read, so every later read would fail the same
        // way, and the settings page would keep presenting defaults as a saved choice and
        // skip the save that would replace it.
        FURI_LOG_W(TAG, "%s unusable, restoring the default write targets", IBUTTON_SETTINGS_PATH);

        storage = furi_record_open(RECORD_STORAGE);
        storage_simply_remove(storage, IBUTTON_SETTINGS_PATH);
        furi_record_close(RECORD_STORAGE);

    } else if(stat != FSE_NOT_EXIST) {
        FURI_LOG_W(
            TAG,
            "%s unreachable (%s), using the default write targets",
            IBUTTON_SETTINGS_PATH,
            storage_error_get_desc(stat));
    }

    return ibutton_write_targets_default();
}

bool ibutton_settings_set_write_targets(iButtonWriteTargetMask mask) {
    // Read-modify-write, not a fresh struct: a designated initialiser would zero any setting
    // added beside this one, which is exactly what the header promises is safe to do.
    iButtonSettings settings = {.write_target_mask = ibutton_settings_get_write_targets()};
    settings.write_target_mask = mask & IBUTTON_WRITE_TARGET_MASK_ALL;

    // Defensive: the app normally creates this folder on startup, but the user can delete it.
    // Return deliberately unchecked - the save below fails and reports if this did not work.
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, IBUTTON_SETTINGS_FOLDER);
    furi_record_close(RECORD_STORAGE);

    bool result = saved_struct_save(
        IBUTTON_SETTINGS_PATH,
        &settings,
        sizeof(iButtonSettings),
        IBUTTON_SETTINGS_MAGIC,
        IBUTTON_SETTINGS_VERSION);

    if(!result) {
        FURI_LOG_E(TAG, "Failed to save %s", IBUTTON_SETTINGS_PATH);
    }

    return result;
}
