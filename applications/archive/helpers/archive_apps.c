#include "archive_files.h"
#include "archive_apps.h"
#include "archive_browser.h"

static const char* known_apps[] = {
    [ArchiveAppTypeU2f] = "u2f",
};

ArchiveAppTypeEnum archive_get_app_type(const char* path) {
    for(size_t i = 0; i < SIZEOF_ARRAY(known_apps); i++) {
        if(strncmp(path, known_apps[i], strlen(known_apps[i])) != STRING_FAILURE) {
            return i;
        }
    }
    return ArchiveAppTypeUnknown;
}

bool archive_app_is_available(void* context, const char* path) {
    furi_assert(path);

    ArchiveAppTypeEnum app = archive_get_app_type(path);

    if(app == ArchiveAppTypeU2f) {
        FileWorker* file_worker = file_worker_alloc(true);
        bool file_exists = false;
        file_worker_is_file_exist(file_worker, "/any/u2f/key.u2f", &file_exists);
        if(file_exists) {
            file_worker_is_file_exist(file_worker, "/any/u2f/cnt.u2f", &file_exists);
        }
        file_worker_free(file_worker);
        return file_exists;
    } else {
        return false;
    }
}

bool archive_app_read_dir(void* context, const char* path) {
    furi_assert(context);
    furi_assert(path);
    ArchiveBrowserView* browser = context;

    archive_file_array_rm_all(browser);

    ArchiveAppTypeEnum app = archive_get_app_type(path);

    if(app == ArchiveAppTypeU2f) {
        archive_add_app_item(browser, "/app:u2f/U2F Token");
        return true;
    } else {
        return false;
    }
}

void archive_app_delete_file(void* context, const char* path) {
    furi_assert(context);
    furi_assert(path);
    ArchiveBrowserView* browser = context;

    ArchiveAppTypeEnum app = archive_get_app_type(path);
    bool res = false;

    if(app == ArchiveAppTypeU2f) {
        FileWorker* file_worker = file_worker_alloc(true);
        res = file_worker_remove(file_worker, "/any/u2f/key.u2f");
        res |= file_worker_remove(file_worker, "/any/u2f/cnt.u2f");
        file_worker_free(file_worker);

        if(archive_is_favorite("/app:u2f/U2F Token")) {
            archive_favorites_delete("/app:u2f/U2F Token");
        }
    }

    if(res) {
        archive_file_array_rm_selected(browser);
    }
}
