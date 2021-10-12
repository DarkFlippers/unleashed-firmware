#include "archive_files.h"
#include "archive_browser.h"

bool filter_by_extension(FileInfo* file_info, const char* tab_ext, const char* name) {
    furi_assert(file_info);
    furi_assert(tab_ext);
    furi_assert(name);

    bool result = false;

    if(strcmp(tab_ext, "*") == 0) {
        result = true;
    } else if(strstr(name, tab_ext) != NULL) {
        result = true;
    } else if(file_info->flags & FSF_DIRECTORY) {
        result = true;
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

void set_file_type(ArchiveFile_t* file, FileInfo* file_info) {
    furi_assert(file);
    furi_assert(file_info);

    for(size_t i = 0; i < SIZEOF_ARRAY(known_ext); i++) {
        if(string_search_str(file->name, known_ext[i], 0) != STRING_FAILURE) {
            file->type = i;
            return;
        }
    }

    if(file_info->flags & FSF_DIRECTORY) {
        file->type = ArchiveFileTypeFolder;
    } else {
        file->type = ArchiveFileTypeUnknown;
    }
}

bool archive_get_filenames(void* context, const char* path) {
    furi_assert(context);

    bool res;
    ArchiveBrowserView* browser = context;
    archive_file_array_rm_all(browser);

    if(archive_get_tab(browser) != ArchiveTabFavorites) {
        res = archive_read_dir(browser, path);
    } else {
        res = archive_favorites_read(browser);
    }
    return res;
}

bool archive_dir_empty(void* context, const char* path) { // can be simpler?
    furi_assert(context);

    FileInfo file_info;
    Storage* fs_api = furi_record_open("storage");
    File* directory = storage_file_alloc(fs_api);
    char name[MAX_NAME_LEN];

    if(!storage_dir_open(directory, path)) {
        storage_dir_close(directory);
        storage_file_free(directory);
        return false;
    }

    bool files_found = false;
    while(1) {
        if(!storage_dir_read(directory, &file_info, name, MAX_NAME_LEN)) {
            break;
        }
        if(files_found) {
            break;
        } else if(storage_file_get_error(directory) == FSE_OK) {
            files_found = name[0];
        } else {
            return false;
        }
    }
    storage_dir_close(directory);
    storage_file_free(directory);

    furi_record_close("storage");

    return files_found;
}

bool archive_read_dir(void* context, const char* path) {
    furi_assert(context);

    ArchiveBrowserView* browser = context;
    FileInfo file_info;
    Storage* fs_api = furi_record_open("storage");
    File* directory = storage_file_alloc(fs_api);
    char name[MAX_NAME_LEN];
    size_t files_cnt = 0;

    if(!storage_dir_open(directory, path)) {
        storage_dir_close(directory);
        storage_file_free(directory);
        return false;
    }

    while(1) {
        if(!storage_dir_read(directory, &file_info, name, MAX_NAME_LEN)) {
            break;
        }
        if(files_cnt > MAX_FILES) {
            break;
        } else if(storage_file_get_error(directory) == FSE_OK) {
            archive_add_item(browser, &file_info, name);
            ++files_cnt;
        } else {
            storage_dir_close(directory);
            storage_file_free(directory);
            return false;
        }
    }
    storage_dir_close(directory);
    storage_file_free(directory);

    furi_record_close("storage");

    return true;
}

void archive_file_append(const char* path, const char* format, ...) {
    furi_assert(path);

    string_t string;
    va_list args;
    va_start(args, format);
    string_init_vprintf(string, format, args);
    va_end(args);

    FileWorker* file_worker = file_worker_alloc(false);

    if(!file_worker_open(file_worker, path, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        FURI_LOG_E("Archive", "Append open error");
    }

    if(!file_worker_write(file_worker, string_get_cstr(string), string_size(string))) {
        FURI_LOG_E("Archive", "Append write error");
    }

    file_worker_close(file_worker);
    file_worker_free(file_worker);
}

void archive_delete_file(void* context, const char* format, ...) {
    furi_assert(context);

    string_t filename;
    va_list args;
    va_start(args, format);
    string_init_vprintf(filename, format, args);
    va_end(args);

    ArchiveBrowserView* browser = context;
    FileWorker* file_worker = file_worker_alloc(true);

    bool res = file_worker_remove(file_worker, string_get_cstr(filename));
    file_worker_free(file_worker);

    if(archive_is_favorite("%s", string_get_cstr(filename))) {
        archive_favorites_delete("%s", string_get_cstr(filename));
    }

    if(res) {
        archive_file_array_rm_selected(browser);
    }

    string_clear(filename);
}