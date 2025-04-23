#pragma once

#include <storage/storage.h>

#define ARCHIVE_FAV_PATH      EXT_PATH("favorites.txt")
#define ARCHIVE_FAV_TEMP_PATH EXT_PATH("favorites.tmp")

uint16_t archive_favorites_count(void* context);
bool archive_favorites_read(void* context);
bool archive_favorites_delete(const char* format, ...) _ATTRIBUTE((__format__(__printf__, 1, 2)));
bool archive_is_favorite(const char* format, ...) _ATTRIBUTE((__format__(__printf__, 1, 2)));
bool archive_favorites_rename(const char* src, const char* dst);
void archive_add_to_favorites(const char* file_path);
void archive_favorites_save(void* context);

/**
 * Intended to be called by settings apps to handle long presses, as well as
 * internally from within the archive
 * 
 * @param app_name name of the referring application
 * @param setting  name of the setting, which will be both displayed to the user
 *                 and passed to the application as an argument upon recall
 */
void archive_favorites_handle_setting_pin_unpin(const char* app_name, const char* setting);
