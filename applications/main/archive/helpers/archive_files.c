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
            if(furi_string_search(file->path, known_ext[i], 0) != FURI_STRING_FAILURE) {
                if(i == ArchiveFileTypeBadUsb) {
                    if(furi_string_search(
                           file->path, archive_get_default_path(ArchiveTabBadUsb)) == 0) {
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
            char tmp_extension[MAX_EXT_LEN];
            path_extract_extension(file->path, tmp_extension, MAX_EXT_LEN);
            if((strcmp(tmp_extension, ".txt") == 0) || (strcmp(tmp_extension, ".md") == 0)) {
                file->is_text_file = true;
            }
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

    FuriString* string;
    va_list args;
    va_start(args, format);
    string = furi_string_alloc_vprintf(format, args);
    va_end(args);

    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    bool res = storage_file_open(file, path, FSAM_WRITE, FSOM_OPEN_APPEND);

    if(res) {
        storage_file_write(file, furi_string_get_cstr(string), furi_string_size(string));
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void archive_delete_file(void* context, const char* format, ...) {
    furi_assert(context);

    FuriString* filename;
    va_list args;
    va_start(args, format);
    filename = furi_string_alloc_vprintf(format, args);
    va_end(args);

    ArchiveBrowserView* browser = context;
    Storage* fs_api = furi_record_open(RECORD_STORAGE);

    FileInfo fileinfo;
    storage_common_stat(fs_api, furi_string_get_cstr(filename), &fileinfo);

    bool res = false;

    if(file_info_is_dir(&fileinfo)) {
        res = storage_simply_remove_recursive(fs_api, furi_string_get_cstr(filename));
    } else {
        res = (storage_common_remove(fs_api, furi_string_get_cstr(filename)) == FSE_OK);
    }

    furi_record_close(RECORD_STORAGE);

    if(archive_is_favorite("%s", furi_string_get_cstr(filename))) {
        archive_favorites_delete("%s", furi_string_get_cstr(filename));
    }

    if(res) {
        archive_file_array_rm_selected(browser);
    }

    furi_string_free(filename);
}

FS_Error archive_rename_file_or_dir(void* context, const char* src_path, const char* dst_path) {
    furi_assert(context);

    FURI_LOG_I(TAG, "Rename from %s to %s", src_path, dst_path);

    ArchiveBrowserView* browser = context;
    Storage* fs_api = furi_record_open(RECORD_STORAGE);

    FileInfo fileinfo;
    storage_common_stat(fs_api, src_path, &fileinfo);

    FS_Error error = FSE_OK;

    if(!path_contains_only_ascii(dst_path)) {
        error = FSE_INVALID_NAME;
    } else {
        error = storage_common_rename(fs_api, src_path, dst_path);
    }
    furi_record_close(RECORD_STORAGE);

    if(archive_is_favorite("%s", src_path)) {
        archive_favorites_rename(src_path, dst_path);
    }

    if(error == FSE_OK || error == FSE_EXIST) {
        FURI_LOG_I(TAG, "Rename from %s to %s is DONE", src_path, dst_path);
        archive_refresh_dir(browser);
    } else {
        FURI_LOG_E(
            TAG, "Rename failed: %s, Code: %d", filesystem_api_error_get_desc(error), error);
    }

    return error;
}
