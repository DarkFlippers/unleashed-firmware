#include "desktop_settings.h"
#include "desktop_settings_filename.h"

#include <saved_struct.h>
#include <storage/storage.h>

#define TAG "DesktopSettings"

#define DESKTOP_SETTINGS_VER_14 (14)
#define DESKTOP_SETTINGS_VER    (17)

#define DESKTOP_SETTINGS_PATH  INT_PATH(DESKTOP_SETTINGS_FILE_NAME)
#define DESKTOP_SETTINGS_MAGIC (0x17)

typedef struct {
    uint32_t auto_lock_delay_ms;
    uint8_t displayBatteryPercentage;
    uint8_t dummy_mode;
    uint8_t display_clock;
    FavoriteApp favorite_apps[FavoriteAppNumber];
    FavoriteApp dummy_apps[DummyAppNumber];
} DesktopSettingsV14;

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

        } else if(version == DESKTOP_SETTINGS_VER_14) {
            DesktopSettingsV14* settings_v14 = malloc(sizeof(DesktopSettingsV14));

            success = saved_struct_load(
                DESKTOP_SETTINGS_PATH,
                settings_v14,
                sizeof(DesktopSettingsV14),
                DESKTOP_SETTINGS_MAGIC,
                DESKTOP_SETTINGS_VER_14);

            if(success) {
                settings->auto_lock_delay_ms = settings_v14->auto_lock_delay_ms;
                settings->usb_inhibit_auto_lock = 0;
                settings->displayBatteryPercentage = settings_v14->displayBatteryPercentage;
                settings->dummy_mode = settings_v14->dummy_mode;
                settings->display_clock = settings_v14->display_clock;
                memcpy(
                    settings->favorite_apps,
                    settings_v14->favorite_apps,
                    sizeof(settings->favorite_apps));
                memcpy(
                    settings->dummy_apps, settings_v14->dummy_apps, sizeof(settings->dummy_apps));
            }

            free(settings_v14);
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
