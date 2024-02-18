#pragma once

#include "archive_files.h"

typedef enum {
    ArchiveAppTypeU2f,
    ArchiveAppTypeUnknown,
    ArchiveAppsTotal,
} ArchiveAppTypeEnum;

static const ArchiveFileTypeEnum app_file_types[] = {
    [ArchiveAppTypeU2f] = ArchiveFileTypeU2f,
    [ArchiveAppTypeUnknown] = ArchiveFileTypeUnknown,
};

static inline ArchiveFileTypeEnum archive_get_app_filetype(ArchiveAppTypeEnum app) {
    return app_file_types[app];
}

ArchiveAppTypeEnum archive_get_app_type(const char* path);
bool archive_app_is_available(void* context, const char* path);
bool archive_app_read_dir(void* context, const char* path);
void archive_app_delete_file(void* context, const char* path);
