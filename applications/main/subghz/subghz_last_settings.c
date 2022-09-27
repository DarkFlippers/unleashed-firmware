#include "subghz_last_settings.h"

#define TAG "SubGhzLastSettings"

// 1 = "AM650"
// "AM270", "AM650", "FM238", "FM476",
#define SUBGHZ_LAST_SETTING_DEFAULT_PRESET 1
#define SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY 433920000

void subghz_last_settings_check_struct(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_file_exists(storage, SUBGHZ_LAST_SETTINGS_PATH)) {
        SubGhzLastSettings* instance = malloc(sizeof(SubGhzLastSettings));
        instance->frequency = SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY;
        instance->preset = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
        SAVE_SUBGHZ_LAST_SETTINGS(&instance);
    }
    furi_record_close(RECORD_STORAGE);
}