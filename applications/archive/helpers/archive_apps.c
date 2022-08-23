#include "archive_files.h"
#include "archive_apps.h"
#include "archive_browser.h"

static const char* known_apps[] = {
    [ArchiveAppTypeU2f] = "u2f",
};

ArchiveAppTypeEnum archive_get_app_type(const char* path) {
    const char* app_name = strchr(path, ':');
    if(app_name == NULL) {
        return ArchiveAppTypeUnknown;
    }
    app_name++;

    for(size_t i = 0; i < COUNT_OF(known_apps); i++) {
        if(strncmp(app_name, known_apps[i], strlen(known_apps[i])) == 0) {
            return i;
        }
    }
    return ArchiveAppTypeUnknown;
}

bool archive_app_is_available(void* context, const char* path) {
    UNUSED(context);
    furi_assert(path);

    ArchiveAppTypeEnum app = archive_get_app_type(path);

    if(app == ArchiveAppTypeU2f) {
        bool file_exists = false;
        Storage* storage = furi_record_open(RECORD_STORAGE);

        if(storage_file_exists(storage, ANY_PATH("u2f/key.u2f"))) {
            file_exists = storage_file_exists(storage, ANY_PATH("u2f/cnt.u2f"));
        }

        furi_record_close(RECORD_STORAGE);
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
        Storage* fs_api = furi_record_open(RECORD_STORAGE);
        res = (storage_common_remove(fs_api, ANY_PATH("u2f/key.u2f")) == FSE_OK);
        res |= (storage_common_remove(fs_api, ANY_PATH("u2f/cnt.u2f")) == FSE_OK);
        furi_record_close(RECORD_STORAGE);

        if(archive_is_favorite("/app:u2f/U2F Token")) {
            archive_favorites_delete("/app:u2f/U2F Token");
        }
    }

    if(res) {
        archive_file_array_rm_selected(browser);
    }
}
