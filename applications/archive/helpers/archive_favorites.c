#include "archive_favorites.h"
#include "archive_files.h"
#include "../views/archive_main_view.h"

bool archive_favorites_read(void* context) {
    furi_assert(context);

    ArchiveMainView* archive_view = context;
    FileWorker* file_worker = file_worker_alloc(true);

    string_t buffer;
    FileInfo file_info;
    string_init(buffer);

    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_ALWAYS);

    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }

            archive_view_add_item(archive_view, &file_info, string_get_cstr(buffer));
            string_clean(buffer);
        }
    }
    string_clear(buffer);
    file_worker_close(file_worker);
    file_worker_free(file_worker);
    return result;
}

bool archive_favorites_delete(const char* file_path, const char* name) {
    furi_assert(file_path);
    furi_assert(name);

    FileWorker* file_worker = file_worker_alloc(true);

    string_t path;
    string_t buffer;
    string_init(buffer);

    string_init_printf(path, "%s/%s", file_path, name);

    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }

            if(string_search(buffer, path)) {
                string_t temp;
                string_init_printf(temp, "%s\r\n", string_get_cstr(buffer));
                archive_file_append(ARCHIVE_FAV_TEMP_PATH, temp);
                string_clear(temp);
            }
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

bool archive_is_favorite(const char* file_path, const char* name) {
    furi_assert(file_path);
    furi_assert(name);

    FileWorker* file_worker = file_worker_alloc(true);

    string_t path;
    string_t buffer;
    string_init(buffer);
    bool found = false;

    string_init_printf(path, "%s/%s", file_path, name);
    bool result = file_worker_open(file_worker, ARCHIVE_FAV_PATH, FSAM_READ, FSOM_OPEN_ALWAYS);

    if(result) {
        while(1) {
            if(!file_worker_read_until(file_worker, buffer, '\n')) {
                break;
            }
            if(!string_size(buffer)) {
                break;
            }
            if(!string_search(buffer, path)) {
                found = true;
                break;
            }
        }
    }

    string_clear(buffer);
    string_clear(path);
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
    string_t temp;

    string_init(buffer);
    string_init(temp);
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
            string_printf(
                temp, "%s\r\n", string_search(buffer, path) ? string_get_cstr(buffer) : dst);
            archive_file_append(ARCHIVE_FAV_TEMP_PATH, temp);
            string_clean(temp);
        }
    }

    string_clear(temp);
    string_clear(buffer);
    string_clear(path);

    file_worker_close(file_worker);
    file_worker_remove(file_worker, ARCHIVE_FAV_PATH);
    file_worker_rename(file_worker, ARCHIVE_FAV_TEMP_PATH, ARCHIVE_FAV_PATH);

    file_worker_free(file_worker);

    return result;
}

void archive_add_to_favorites(const char* file_path, const char* name) {
    furi_assert(file_path);
    furi_assert(name);

    string_t buffer_src;

    string_init_printf(buffer_src, "%s/%s\r\n", file_path, name);
    archive_file_append(ARCHIVE_FAV_PATH, buffer_src);
    string_clear(buffer_src);
}
