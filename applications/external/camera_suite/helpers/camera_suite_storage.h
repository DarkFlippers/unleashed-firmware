#include <stdlib.h>
#include <string.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format_i.h>
#include "../camera_suite.h"

#define BOILERPLATE_SETTINGS_FILE_VERSION 1
#define CONFIG_FILE_DIRECTORY_PATH EXT_PATH("apps_data/camera_suite")
#define BOILERPLATE_SETTINGS_SAVE_PATH CONFIG_FILE_DIRECTORY_PATH "/camera_suite.conf"
#define BOILERPLATE_SETTINGS_SAVE_PATH_TMP BOILERPLATE_SETTINGS_SAVE_PATH ".tmp"
#define BOILERPLATE_SETTINGS_HEADER "Camera Suite Config File"
#define BOILERPLATE_SETTINGS_KEY_ORIENTATION "Orientation"
#define BOILERPLATE_SETTINGS_KEY_HAPTIC "Haptic"
#define BOILERPLATE_SETTINGS_KEY_LED "Led"
#define BOILERPLATE_SETTINGS_KEY_SPEAKER "Speaker"
#define BOILERPLATE_SETTINGS_KEY_SAVE_SETTINGS "SaveSettings"

void camera_suite_save_settings(void* context);

void camera_suite_read_settings(void* context);
