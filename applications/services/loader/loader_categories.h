#pragma once
#include <string.h>
#include <stdbool.h>

// Only a single visible category below /ext/apps. Never navigate arbitrary args.
static inline bool loader_category_name_valid(const char* name) {
    return name && name[0] && name[0] != '.' && strcmp(name, "assets") != 0 &&
           !strchr(name, '/') && !strchr(name, '\\');
}
static inline bool loader_category_path_valid(const char* path) {
    const char* prefix = "/ext/apps/";
    return path && strncmp(path, prefix, strlen(prefix)) == 0 &&
           loader_category_name_valid(path + strlen(prefix));
}
