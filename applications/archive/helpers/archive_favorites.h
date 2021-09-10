#pragma once
#include "file-worker.h"

#define ARCHIVE_FAV_PATH "/any/favorites.txt"
#define ARCHIVE_FAV_TEMP_PATH "/any/favorites.tmp"

bool archive_favorites_read(void* context);
bool archive_favorites_delete(const char* file_path, const char* name);
bool archive_is_favorite(const char* file_path, const char* name);
bool archive_favorites_rename(const char* file_path, const char* src, const char* dst);
void archive_add_to_favorites(const char* file_path, const char* name);
