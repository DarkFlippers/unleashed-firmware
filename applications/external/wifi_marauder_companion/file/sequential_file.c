#include "sequential_file.h"

char* sequential_file_resolve_path(
    Storage* storage,
    const char* dir,
    const char* prefix,
    const char* extension) {
    if(storage == NULL || dir == NULL || prefix == NULL || extension == NULL) {
        return NULL;
    }

    char file_path[256];
    int file_index = 0;

    do {
        if(snprintf(
               file_path, sizeof(file_path), "%s/%s_%d.%s", dir, prefix, file_index, extension) <
           0) {
            return NULL;
        }
        file_index++;
    } while(storage_file_exists(storage, file_path));

    return strdup(file_path);
}

bool sequential_file_open(
    Storage* storage,
    File* file,
    const char* dir,
    const char* prefix,
    const char* extension) {
    if(storage == NULL || file == NULL || dir == NULL || prefix == NULL || extension == NULL) {
        return false;
    }

    char* file_path = sequential_file_resolve_path(storage, dir, prefix, extension);
    if(file_path == NULL) {
        return false;
    }

    bool success = storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    free(file_path);

    return success;
}