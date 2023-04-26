#pragma once

#include <storage/storage.h>

char* sequential_file_resolve_path(
    Storage* storage,
    const char* dir,
    const char* prefix,
    const char* extension);
bool sequential_file_open(
    Storage* storage,
    File* file,
    const char* dir,
    const char* prefix,
    const char* extension);