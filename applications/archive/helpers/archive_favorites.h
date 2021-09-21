#pragma once
#include "file-worker.h"

#define ARCHIVE_FAV_PATH "/any/favorites.txt"
#define ARCHIVE_FAV_TEMP_PATH "/any/favorites.tmp"

uint16_t archive_favorites_count(void* context);
bool archive_favorites_read(void* context);
bool archive_favorites_delete(const char* format, ...);
bool archive_is_favorite(const char* format, ...);
bool archive_favorites_rename(const char* file_path, const char* src, const char* dst);
void archive_add_to_favorites(const char* file_path);