/** @file lfrfid_settings.h
 *
 * Persistent LFRFID settings, shared by the app, the CLI and any app that writes keys
 * through LFRFIDWorker. Stored next to the saved keys so it survives an app reinstall.
 */

#pragma once
#include <storage/storage.h>
#include "lfrfid_write_targets.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LFRFID_SETTINGS_FOLDER  EXT_PATH("lfrfid")
#define LFRFID_SETTINGS_PATH    LFRFID_SETTINGS_FOLDER "/.lfrfid.settings"
#define LFRFID_SETTINGS_VERSION (1)
#define LFRFID_SETTINGS_MAGIC   (0x4C)

typedef struct {
    /** Write targets the user allows, as LFRFIDWriteTarget bits. */
    uint32_t write_target_mask;
} LFRFIDSettings;

/** Load settings, falling back to defaults when the file is missing or was written by
 * another version. Never fails: settings are always usable afterwards.
 *
 * @param[out] settings  Where to store the loaded settings
 */
void lfrfid_settings_load(LFRFIDSettings* settings);

/** Save settings, creating the app folder if this is the first write.
 *
 * @param[in]  settings  The settings to store
 * @return     true on success
 */
bool lfrfid_settings_save(const LFRFIDSettings* settings);

#ifdef __cplusplus
}
#endif
