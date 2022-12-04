#include <toolbox/saved_struct.h>
#include <storage/storage.h>
#include "power_settings_filename.h"

#define POWER_SETTINGS_VER (1)

#define POWER_SETTINGS_PATH INT_PATH(POWER_SETTINGS_FILE_NAME)
#define POWER_SETTINGS_MAGIC (0x21)

#define SAVE_POWER_SETTINGS(x) \
    saved_struct_save(         \
        POWER_SETTINGS_PATH, (x), sizeof(uint32_t), POWER_SETTINGS_MAGIC, POWER_SETTINGS_VER)

#define LOAD_POWER_SETTINGS(x) \
    saved_struct_load(         \
        POWER_SETTINGS_PATH, (x), sizeof(uint32_t), POWER_SETTINGS_MAGIC, POWER_SETTINGS_VER)
