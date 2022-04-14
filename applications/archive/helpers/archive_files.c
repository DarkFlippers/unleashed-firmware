#include "archive_files.h"
#include "archive_apps.h"
#include "archive_browser.h"

#define TAG "Archive"

#define ASSETS_DIR "assets"

bool archive_filter_by_extension(FileInfo* file_info, const char* tab_ext, const char* name) {
    furi_assert(file_info);
    furi_assert(tab_ext);
    furi_assert(name);

    bool result = false;

    if(strcmp(tab_ext, "*") == 0) {
        result = true;
    } else if(strstr(name, tab_ext) != NULL) {
        result = true;
    } else if(file_info->flags & FSF_DIRECTORY) {
        if(strstr(name, ASSETS_DIR) != NULL) {
            result = false; // Skip assets folder in all tabs except browser
        } else {
            result = true;
        }
    }

    return result;
}

void archive_trim_file_path(char* name, bool ext) {
    char* slash = strrchr(name, '/') + 1;
    if(strlen(slash)) strlcpy(name, slash, strlen(slash) + 1);
    if(ext) {
        char* dot = strrchr(name, '.');
        if(strlen(dot)) *dot = '\0';
    }
}

void archive_get_file_extension(char* name, char* ext) {
    char* dot = strrchr(name, '.');
    if(dot == NULL)
        *ext = '\0';
    else
        strncpy(ext, dot, MAX_EXT_LEN);
}

void archive_set_file_type(ArchiveFile_t* file, FileInfo* file_info, const char* path, bool is_app) {
    furi_assert(file);

    file->is_app = is_app;
    if(is_app) {
        file->type = archive_get_app_filetype(archive_get_app_type(path));
    } else {
        furi_assert(file_info);

        for(size_t i = 0; i < SIZEOF_ARRAY(known_ext); i++) {
            if((known_ext[i][0] == '?') || (known_ext[i][0] == '*')) continue;
            if(string_search_str(file->name, known_ext[i], 0) != STRING_FAILURE) {
                if(i == ArchiveFileTypeBadUsb) {
                    if(string_search_str(file->name, archive_get_default_path(ArchiveTabBadUsb)) ==
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

        if(file_info->flags & FSF_DIRECTORY) {
            file->type = ArchiveFileTypeFolder;
        } else {
            file->type = ArchiveFileTypeUnknown;
        }
    }
}

bool archive_get_filenames(void* context, const char* path) {
    furi_assert(context);

    bool res;
    ArchiveBrowserView* browser = context;

    if(archive_get_tab(browser) == ArchiveTabFavorites) {
        res = archive_favorites_read(browser);
    } else if(strncmp(path, "/app:", 5) == 0) {
        res = archive_app_read_dir(browser, path);
    } else {
        res = archive_file_array_load(browser, 0);
    }
    return res;
}

uint32_t archive_dir_count_items(void* context, const char* path) {
    furi_assert(context);
    ArchiveBrowserView* browser = context;

    FileInfo file_info;
    Storage* fs_api = furi_record_open("storage");
    File* directory = storage_file_alloc(fs_api);
    char name[MAX_NAME_LEN];

    if(!storage_dir_open(directory, path)) {
        storage_dir_close(directory);
        storage_file_free(directory);
        return 0;
    }

    uint32_t files_found = 0;
    while(1) {
        if(!storage_dir_read(directory, &file_info, name, MAX_NAME_LEN)) {
            break;
        }
        if((storage_file_get_error(directory) == FSE_OK) && (name[0])) {
            if(archive_filter_by_extension(
                   &file_info, archive_get_tab_ext(archive_get_tab(browser)), name)) {
                files_found++;
            }
        }
    }
    storage_dir_close(directory);
    storage_file_free(directory);

    furi_record_close("storage");

    archive_set_item_count(browser, files_found);

    return files_found;
}

uint32_t archive_dir_read_items(void* context, const char* path, uint32_t offset, uint32_t count) {
    furi_assert(context);

    ArchiveBrowserView* browser = context;
    FileInfo file_info;
    Storage* fs_api = furi_record_open("storage");
    File* directory = storage_file_alloc(fs_api);
    char name[MAX_NAME_LEN];
    snprintf(name, MAX_NAME_LEN, "%s/", path);
    size_t path_len = strlen(name);

    if(!storage_dir_open(directory, path)) {
        storage_dir_close(directory);
        storage_file_free(directory);
        return false;
    }

    // Skip items before offset
    uint32_t items_cnt = 0;
    while(items_cnt < offset) {
        if(!storage_dir_read(directory, &file_info, &name[path_len], MAX_NAME_LEN)) {
            break;
        }
        if(storage_file_get_error(directory) == FSE_OK) {
            if(archive_filter_by_extension(
                   &file_info, archive_get_tab_ext(archive_get_tab(browser)), name)) {
                items_cnt++;
            }
        } else {
            break;
        }
    }
    if(items_cnt != offset) {
        storage_dir_close(directory);
        storage_file_free(directory);
        furi_record_close("storage");

        return false;
    }

    items_cnt = 0;
    archive_file_array_rm_all(browser);
    while(items_cnt < count) {
        if(!storage_dir_read(directory, &file_info, &name[path_len], MAX_NAME_LEN - path_len)) {
            break;
        }

        if(storage_file_get_error(directory) == FSE_OK) {
            archive_add_file_item(browser, &file_info, name);
            items_cnt++;
        } else {
            break;
        }
    }
    storage_dir_close(directory);
    storage_file_free(directory);
    furi_record_close("storage");

    return (items_cnt == count);
}

void archive_file_append(const char* path, const char* format, ...) {
    furi_assert(path);

    string_t string;
    va_list args;
    va_start(args, format);
    string_init_vprintf(string, format, args);
    va_end(args);

    Storage* fs_api = furi_record_open("storage");
    File* file = storage_file_alloc(fs_api);

    bool res = storage_file_open(file, path, FSAM_WRITE, FSOM_OPEN_APPEND);

    if(res) {
        storage_file_write(file, string_get_cstr(string), string_size(string));
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close("storage");
}

void archive_delete_file(void* context, const char* format, ...) {
    furi_assert(context);

    string_t filename;
    va_list args;
    va_start(args, format);
    string_init_vprintf(filename, format, args);
    va_end(args);

    ArchiveBrowserView* browser = context;
    Storage* fs_api = furi_record_open("storage");

    FileInfo fileinfo;
    storage_common_stat(fs_api, string_get_cstr(filename), &fileinfo);

    bool res = false;

    if(fileinfo.flags & FSF_DIRECTORY) {
        res = storage_simply_remove_recursive(fs_api, string_get_cstr(filename));
    } else {
        res = (storage_common_remove(fs_api, string_get_cstr(filename)) == FSE_OK);
    }

    furi_record_close("storage");

    if(archive_is_favorite("%s", string_get_cstr(filename))) {
        archive_favorites_delete("%s", string_get_cstr(filename));
    }

    if(res) {
        archive_file_array_rm_selected(browser);
    }

    string_clear(filename);
}
