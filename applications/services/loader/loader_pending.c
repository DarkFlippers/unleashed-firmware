#include "loader_pending.h"

#include <string.h>

#include <saved_struct.h>
#include <storage/storage.h>

#define LOADER_PENDING_PATH    INT_PATH(".loader.pending")
#define LOADER_PENDING_MAGIC   (0x4C)
#define LOADER_PENDING_VERSION (1)

bool loader_pending_launch_save(const char* name_or_path, const char* args) {
    furi_check(name_or_path);

    LoaderPendingLaunch* pending = malloc(sizeof(LoaderPendingLaunch));
    bool success = false;

    do {
        if(strlcpy(pending->name_or_path, name_or_path, sizeof(pending->name_or_path)) >=
           sizeof(pending->name_or_path)) {
            break;
        }

        if(args && strlcpy(pending->args, args, sizeof(pending->args)) >= sizeof(pending->args)) {
            break;
        }

        success = saved_struct_save(
            LOADER_PENDING_PATH,
            pending,
            sizeof(LoaderPendingLaunch),
            LOADER_PENDING_MAGIC,
            LOADER_PENDING_VERSION);
    } while(false);

    free(pending);
    return success;
}

LoaderPendingLaunch* loader_pending_launch_take(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    LoaderPendingLaunch* pending = NULL;

    if(storage_common_stat(storage, LOADER_PENDING_PATH, NULL) == FSE_OK) {
        pending = malloc(sizeof(LoaderPendingLaunch));

        bool loaded = saved_struct_load(
            LOADER_PENDING_PATH,
            pending,
            sizeof(LoaderPendingLaunch),
            LOADER_PENDING_MAGIC,
            LOADER_PENDING_VERSION);

        storage_simply_remove(storage, LOADER_PENDING_PATH);

        pending->name_or_path[sizeof(pending->name_or_path) - 1] = '\0';
        pending->args[sizeof(pending->args) - 1] = '\0';

        if(!loaded || (pending->name_or_path[0] == '\0')) {
            free(pending);
            pending = NULL;
        }
    }

    furi_record_close(RECORD_STORAGE);
    return pending;
}
