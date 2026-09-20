#pragma once

#include <furi.h>

#define LOADER_PENDING_MAX_PATH_LEN 255u

typedef struct {
    char name_or_path[LOADER_PENDING_MAX_PATH_LEN];
    char args[LOADER_PENDING_MAX_PATH_LEN];
} LoaderPendingLaunch;

bool loader_pending_launch_save(const char* name_or_path, const char* args);

LoaderPendingLaunch* loader_pending_launch_take(void);
