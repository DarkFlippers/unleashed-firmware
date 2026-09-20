#pragma once

#include <furi.h>

bool loader_pending_launch_save(const char* name_or_path, const char* args);

bool loader_pending_launch_take(FuriString* name_or_path, FuriString* args);
