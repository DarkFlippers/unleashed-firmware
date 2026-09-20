#include "loader_pending.h"

#include <string.h>

#include <saved_struct.h>
#include <storage/storage.h>

#define LOADER_PENDING_PATH    INT_PATH(".loader.pending")
#define LOADER_PENDING_MAGIC   (0x4C)
#define LOADER_PENDING_VERSION (1)

bool loader_pending_launch_save(const char* name_or_path, const char* args) {
    furi_check(name_or_path);

    LoaderPendingLaunch pending = {0};

    if(strlcpy(pending.name_or_path, name_or_path, sizeof(pending.name_or_path)) >=
       sizeof(pending.name_or_path)) {
        return false;
    }

    if(args && strlcpy(pending.args, args, sizeof(pending.args)) >= sizeof(pending.args)) {
        return false;
    }

    return saved_struct_save(
        LOADER_PENDING_PATH,
        &pending,
        sizeof(pending),
        LOADER_PENDING_MAGIC,
        LOADER_PENDING_VERSION);
}

bool loader_pending_launch_take(LoaderPendingLaunch* pending) {
    furi_check(pending);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool taken = false;

    if(storage_file_exists(storage, LOADER_PENDING_PATH)) {
        taken = saved_struct_load(
            LOADER_PENDING_PATH,
            pending,
            sizeof(*pending),
            LOADER_PENDING_MAGIC,
            LOADER_PENDING_VERSION);

        storage_simply_remove(storage, LOADER_PENDING_PATH);
    }

    furi_record_close(RECORD_STORAGE);

    if(!taken) return false;

    pending->name_or_path[sizeof(pending->name_or_path) - 1] = '\0';
    pending->args[sizeof(pending->args) - 1] = '\0';

    return pending->name_or_path[0] != '\0';
}
