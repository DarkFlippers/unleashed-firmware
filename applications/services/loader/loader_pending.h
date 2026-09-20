#pragma once

#include <furi.h>

typedef struct {
    char name_or_path[96];
    char args[96];
} LoaderPendingLaunch;

bool loader_pending_launch_save(const char* name_or_path, const char* args);

bool loader_pending_launch_take(LoaderPendingLaunch* pending);
