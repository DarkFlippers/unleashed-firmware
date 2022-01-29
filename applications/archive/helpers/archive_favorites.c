
#include "archive_favorites.h"
#include "archive_browser.h"

uint16_t archive_favorites_count(void* context) {
    furi_assert(context);

    FileWorker* file_worker = file_worker_alloc(true);

    string_t buffer;
    string_init(buffer);

    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    uint16_t lines = 0;

    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }
            ++lines;
        }
    }

    string_clear(buffer);
    file_worker_close(file_worker);
    file_worker_free(file_worker);
    return lines;
}

static bool archive_favourites_rescan() {
    string_t buffer;
    string_init(buffer);
    FileWorker* file_worker = file_worker_alloc(true);

    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }

            bool file_exists = false;
            file_worker_is_file_exist(file_worker, string_get_cstr(buffer), &file_exists);
            if(file_exists) {
                archive_file_append(ARCHIVE_FAV_TEMP_PATH, "%s\n", string_get_cstr(buffer));
            }
        }
    }

    string_clear(buffer);

    file_worker_close(file_worker);
    file_worker_remove(file_worker, ARCHIVE_FAV_PATH);
    file_worker_rename(file_worker, ARCHIVE_FAV_TEMP_PATH, ARCHIVE_FAV_PATH);

    file_worker_free(file_worker);

    return result;
}

bool archive_favorites_read(void* context) {
    furi_assert(context);

    ArchiveBrowserView* browser = context;
    FileWorker* file_worker = file_worker_alloc(true);

    string_t buffer;
    FileInfo file_info;
    string_init(buffer);

    bool need_refresh = false;

    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_EXISTING);

    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }

            bool file_exists = false;
            file_worker_is_file_exist(file_worker, string_get_cstr(buffer), &file_exists);

            if(file_exists)
                archive_add_item(browser, &file_info, string_get_cstr(buffer));
            else
                need_refresh = true;
            string_reset(buffer);
        }
    }
    string_clear(buffer);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    if(need_refresh) {
        archive_favourites_rescan();
    }

    return result;
}

bool archive_favorites_delete(const char* format, ...) {
    string_t buffer;
    string_t filename;
    va_list args;
    va_start(args, format);
    string_init_vprintf(filename, format, args);
    va_end(args);

    string_init(buffer);
    FileWorker* file_worker = file_worker_alloc(true);

    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }

            if(string_search(buffer, filename)) {
                archive_file_append(ARCHIVE_FAV_TEMP_PATH, "%s\n", string_get_cstr(buffer));
            }
        }
    }

    string_clear(buffer);
    string_clear(filename);

    file_worker_close(file_worker);
    file_worker_remove(file_worker, ARCHIVE_FAV_PATH);
    file_worker_rename(file_worker, ARCHIVE_FAV_TEMP_PATH, ARCHIVE_FAV_PATH);

    file_worker_free(file_worker);

    return result;
}

bool archive_is_favorite(const char* format, ...) {
    string_t buffer;
    string_t filename;
    va_list args;
    va_start(args, format);
    string_init_vprintf(filename, format, args);
    va_end(args);

    string_init(buffer);
    FileWorker* file_worker = file_worker_alloc(true);

    bool found = false;
    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_EXISTING);

    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }
            if(!string_search(buffer, filename)) {
                found = true;
                break;
            }
        }
    }

    string_clear(buffer);
    string_clear(filename);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return found;
}

bool archive_favorites_rename(const char* file_path, const char* src, const char* dst) {
    furi_assert(file_path);
    furi_assert(src);
    furi_assert(dst);

    FileWorker* file_worker = file_worker_alloc(true);

    string_t path;
    string_t buffer;

    string_init(buffer);
    string_init(path);

    string_printf(path, "%s/%s", file_path, src);
    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_EXISTING);

    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }

            archive_file_append(
                ARCHIVE_FAV_TEMP_PATH,
                "%s\n",
                string_search(buffer, path) ? string_get_cstr(buffer) : dst);
        }
    }

    string_clear(buffer);
    string_clear(path);

    file_worker_close(file_worker);
    file_worker_remove(file_worker, ARCHIVE_FAV_PATH);
    file_worker_rename(file_worker, ARCHIVE_FAV_TEMP_PATH, ARCHIVE_FAV_PATH);

    file_worker_free(file_worker);

    return result;
}

void archive_add_to_favorites(const char* file_path) {
    furi_assert(file_path);

    archive_file_append(ARCHIVE_FAV_PATH, "%s\n", file_path);
}

void archive_favorites_save(void* context) {
    furi_assert(context);

    ArchiveBrowserView* browser = context;
    FileWorker* file_worker = file_worker_alloc(true);

    for(size_t i = 0; i < archive_file_array_size(browser); i++) {
        ArchiveFile_t* item = archive_get_file_at(browser, i);
        archive_file_append(ARCHIVE_FAV_TEMP_PATH, "%s\n", string_get_cstr(item->name));
    }

    file_worker_remove(file_worker, ARCHIVE_FAV_PATH);
    file_worker_rename(file_worker, ARCHIVE_FAV_TEMP_PATH, ARCHIVE_FAV_PATH);

    file_worker_free(file_worker);
}