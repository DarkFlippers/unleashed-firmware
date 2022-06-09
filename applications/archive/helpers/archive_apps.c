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
        Storage* fs_api = furi_record_open("storage");
        File* file = storage_file_alloc(fs_api);

        file_exists = storage_file_open(file, "/any/u2f/key.u2f", FSAM_READ, FSOM_OPEN_EXISTING);
        if(file_exists) {
            storage_file_close(file);
            file_exists =
                storage_file_open(file, "/any/u2f/cnt.u2f", FSAM_READ, FSOM_OPEN_EXISTING);
            if(file_exists) {
                storage_file_close(file);
            }
        }

        storage_file_free(file);
        furi_record_close("storage");

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
        Storage* fs_api = furi_record_open("storage");
        res = (storage_common_remove(fs_api, "/any/u2f/key.u2f") == FSE_OK);
        res |= (storage_common_remove(fs_api, "/any/u2f/cnt.u2f") == FSE_OK);
        furi_record_close("storage");

        if(archive_is_favorite("/app:u2f/U2F Token")) {
            archive_favorites_delete("/app:u2f/U2F Token");
        }
    }

    if(res) {
        archive_file_array_rm_selected(browser);
    }
}
