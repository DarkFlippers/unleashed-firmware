#include "bt_settings.h"

#include <furi.h>
#include <lib/toolbox/saved_struct.h>
#include <storage/storage.h>

#define BT_SETTINGS_PATH INT_PATH(BT_SETTINGS_FILE_NAME)
#define BT_SETTINGS_VERSION (0)
#define BT_SETTINGS_MAGIC (0x19)

bool bt_settings_load(BtSettings* bt_settings) {
    furi_assert(bt_settings);

    return saved_struct_load(
        BT_SETTINGS_PATH, bt_settings, sizeof(BtSettings), BT_SETTINGS_MAGIC, BT_SETTINGS_VERSION);
}

bool bt_settings_save(BtSettings* bt_settings) {
    furi_assert(bt_settings);

    return saved_struct_save(
        BT_SETTINGS_PATH, bt_settings, sizeof(BtSettings), BT_SETTINGS_MAGIC, BT_SETTINGS_VERSION);
}
