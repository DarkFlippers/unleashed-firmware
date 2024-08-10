#include "desktop_settings.h"
#include "desktop_settings_filename.h"

#include <saved_struct.h>
#include <storage/storage.h>

#define TAG "DesktopSettings"

#define DESKTOP_SETTINGS_VER_13 (13)
#define DESKTOP_SETTINGS_VER    (14)

#define DESKTOP_SETTINGS_PATH  INT_PATH(DESKTOP_SETTINGS_FILE_NAME)
#define DESKTOP_SETTINGS_MAGIC (0x17)

typedef struct {
    uint8_t reserved[11];
    DesktopSettings settings;
} DesktopSettingsV13;

// Actual size of DesktopSettings v13
//static_assert(sizeof(DesktopSettingsV13) == 1234);

void desktop_settings_load(DesktopSettings* settings) {
    furi_assert(settings);

    bool success = false;

    do {
        uint8_t version;
        if(!saved_struct_get_metadata(DESKTOP_SETTINGS_PATH, NULL, &version, NULL)) break;

        if(version == DESKTOP_SETTINGS_VER) {
            success = saved_struct_load(
                DESKTOP_SETTINGS_PATH,
                settings,
                sizeof(DesktopSettings),
                DESKTOP_SETTINGS_MAGIC,
                DESKTOP_SETTINGS_VER);

        } else if(version == DESKTOP_SETTINGS_VER_13) {
            DesktopSettingsV13* settings_v13 = malloc(sizeof(DesktopSettingsV13));

            success = saved_struct_load(
                DESKTOP_SETTINGS_PATH,
                settings_v13,
                sizeof(DesktopSettingsV13),
                DESKTOP_SETTINGS_MAGIC,
                DESKTOP_SETTINGS_VER_13);

            if(success) {
                *settings = settings_v13->settings;
            }

            free(settings_v13);
        }

    } while(false);

    if(!success) {
        FURI_LOG_W(TAG, "Failed to load file, using defaults");
        memset(settings, 0, sizeof(DesktopSettings));
        desktop_settings_save(settings);
    }
}

void desktop_settings_save(const DesktopSettings* settings) {
    furi_assert(settings);

    const bool success = saved_struct_save(
        DESKTOP_SETTINGS_PATH,
        settings,
        sizeof(DesktopSettings),
        DESKTOP_SETTINGS_MAGIC,
        DESKTOP_SETTINGS_VER);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to save file");
    }
}
