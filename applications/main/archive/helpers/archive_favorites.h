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
