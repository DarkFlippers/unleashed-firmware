#include "archive_files.h"
#include "archive_apps.h"
#include "archive_browser.h"

#define TAG "Archive"

#define ASSETS_DIR "assets"

void archive_set_file_type(ArchiveFile_t* file, const char* path, bool is_folder, bool is_app) {
    furi_assert(file);

    file->is_app = is_app;
    if(is_app) {
        file->type = archive_get_app_filetype(archive_get_app_type(path));
    } else {
        for(size_t i = 0; i < COUNT_OF(known_ext); i++) {
            if((known_ext[i][0] == '?') || (known_ext[i][0] == '*')) continue;
            if(string_search_str(file->path, known_ext[i], 0) != STRING_FAILURE) {
                if(i == ArchiveFileTypeBadUsb) {
                    if(string_search_str(file->path, archive_get_default_path(ArchiveTabBadUsb)) ==
                       0) {
                        file->type = i;
                        return; // *.txt file is a BadUSB script only if it is in BadUSB folder
                    }
                } else {
                    file->type = i;
                    return;
                }
            }
        }

        if(is_folder) {
            file->type = ArchiveFileTypeFolder;
        } else {
            file->type = ArchiveFileTypeUnknown;
        }
    }
}

bool archive_get_items(void* context, const char* path) {
    furi_assert(context);

    bool res = false;
    ArchiveBrowserView* browser = context;

    if(archive_get_tab(browser) == ArchiveTabFavorites) {
        res = archive_favorites_read(browser);
    } else if(strncmp(path, "/app:", 5) == 0) {
        res = archive_app_read_dir(browser, path);
    }
    return res;
}

void archive_file_append(const char* path, const char* format, ...) {
    furi_assert(path);

    string_t string;
    va_list args;
    va_start(args, format);
    string_init_vprintf(string, format, args);
    va_end(args);

    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    bool res = storage_file_open(file, path, FSAM_WRITE, FSOM_OPEN_APPEND);

    if(res) {
        storage_file_write(file, string_get_cstr(string), string_size(string));
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void archive_delete_file(void* context, const char* format, ...) {
    furi_assert(context);

    string_t filename;
    va_list args;
    va_start(args, format);
    string_init_vprintf(filename, format, args);
    va_end(args);

    ArchiveBrowserView* browser = context;
    Storage* fs_api = furi_record_open(RECORD_STORAGE);

    FileInfo fileinfo;
    storage_common_stat(fs_api, string_get_cstr(filename), &fileinfo);

    bool res = false;

    if(fileinfo.flags & FSF_DIRECTORY) {
        res = storage_simply_remove_recursive(fs_api, string_get_cstr(filename));
    } else {
        res = (storage_common_remove(fs_api, string_get_cstr(filename)) == FSE_OK);
    }

    furi_record_close(RECORD_STORAGE);

    if(archive_is_favorite("%s", string_get_cstr(filename))) {
        archive_favorites_delete("%s", string_get_cstr(filename));
    }

    if(res) {
        archive_file_array_rm_selected(browser);
    }

    string_clear(filename);
}
