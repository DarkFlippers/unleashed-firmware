#include "loader_pending.h"

#include <string.h>

#include <saved_struct.h>
#include <storage/storage.h>

#define TAG "LoaderPending"

#define LOADER_PENDING_PATH    INT_PATH(".loader.pending")
#define LOADER_PENDING_MAGIC   (0x4C)
#define LOADER_PENDING_VERSION (1)

typedef struct {
    char name_or_path[128];
    char args[128];
} LoaderPendingLaunch;

bool loader_pending_launch_save(const char* name_or_path, const char* args) {
    furi_check(name_or_path);

    LoaderPendingLaunch pending = {0};

    if(strlcpy(pending.name_or_path, name_or_path, sizeof(pending.name_or_path)) >=
       sizeof(pending.name_or_path)) {
        FURI_LOG_E(TAG, "Name does not fit: %s", name_or_path);
        return false;
    }

    if(args && strlcpy(pending.args, args, sizeof(pending.args)) >= sizeof(pending.args)) {
        FURI_LOG_E(TAG, "Args do not fit: %s", args);
        return false;
    }

    return saved_struct_save(
        LOADER_PENDING_PATH,
        &pending,
        sizeof(pending),
        LOADER_PENDING_MAGIC,
        LOADER_PENDING_VERSION);
}

bool loader_pending_launch_take(FuriString* name_or_path, FuriString* args) {
    furi_check(name_or_path);
    furi_check(args);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool taken = false;

    if(storage_file_exists(storage, LOADER_PENDING_PATH)) {
        LoaderPendingLaunch pending;
        bool loaded = saved_struct_load(
            LOADER_PENDING_PATH,
            &pending,
            sizeof(pending),
            LOADER_PENDING_MAGIC,
            LOADER_PENDING_VERSION);

        storage_simply_remove(storage, LOADER_PENDING_PATH);

        if(loaded) {
            pending.name_or_path[sizeof(pending.name_or_path) - 1] = '\0';
            pending.args[sizeof(pending.args) - 1] = '\0';

            if(pending.name_or_path[0] != '\0') {
                furi_string_set_str(name_or_path, pending.name_or_path);
                furi_string_set_str(args, pending.args);
                taken = true;
            }
        }
    }

    furi_record_close(RECORD_STORAGE);
    return taken;
}
