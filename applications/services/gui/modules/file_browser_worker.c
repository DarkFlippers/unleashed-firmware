#include "file_browser_worker.h"
#include <core/check.h>
#include <core/common_defines.h>
#include "m-string.h"
#include "storage/filesystem_api_defines.h"
#include <m-array.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <furi.h>
#include <stddef.h>
#include "toolbox/path.h"

#define TAG "BrowserWorker"

#define ASSETS_DIR "assets"
#define BROWSER_ROOT STORAGE_ANY_PATH_PREFIX
#define FILE_NAME_LEN_MAX 256
#define LONG_LOAD_THRESHOLD 100

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtLoad = (1 << 1),
    WorkerEvtFolderEnter = (1 << 2),
    WorkerEvtFolderExit = (1 << 3),
    WorkerEvtFolderRefresh = (1 << 4),
    WorkerEvtConfigChange = (1 << 5),
} WorkerEvtFlags;

#define WORKER_FLAGS_ALL                                                          \
    (WorkerEvtStop | WorkerEvtLoad | WorkerEvtFolderEnter | WorkerEvtFolderExit | \
     WorkerEvtFolderRefresh | WorkerEvtConfigChange)

ARRAY_DEF(idx_last_array, int32_t)

struct BrowserWorker {
    FuriThread* thread;

    string_t filter_extension;
    string_t path_next;
    int32_t item_sel_idx;
    uint32_t load_offset;
    uint32_t load_count;
    bool skip_assets;
    idx_last_array_t idx_last;

    void* cb_ctx;
    BrowserWorkerFolderOpenCallback folder_cb;
    BrowserWorkerListLoadCallback list_load_cb;
    BrowserWorkerListItemCallback list_item_cb;
    BrowserWorkerLongLoadCallback long_load_cb;
};

static bool browser_path_is_file(string_t path) {
    bool state = false;
    FileInfo file_info;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(storage_common_stat(storage, string_get_cstr(path), &file_info) == FSE_OK) {
        if((file_info.flags & FSF_DIRECTORY) == 0) {
            state = true;
        }
    }
    furi_record_close(RECORD_STORAGE);
    return state;
}

static bool browser_path_trim(string_t path) {
    bool is_root = false;
    size_t filename_start = string_search_rchar(path, '/');
    string_left(path, filename_start);
    if((string_empty_p(path)) || (filename_start == STRING_FAILURE)) {
        string_set_str(path, BROWSER_ROOT);
        is_root = true;
    }
    return is_root;
}

static bool browser_filter_by_name(BrowserWorker* browser, string_t name, bool is_folder) {
    if(is_folder) {
        // Skip assets folders (if enabled)
        if(browser->skip_assets) {
            return ((string_cmp_str(name, ASSETS_DIR) == 0) ? (false) : (true));
        } else {
            return true;
        }
    } else {
        // Filter files by extension
        if((string_empty_p(browser->filter_extension)) ||
           (string_cmp_str(browser->filter_extension, "*") == 0)) {
            return true;
        }
        if(string_end_with_string_p(name, browser->filter_extension)) {
            return true;
        }
    }
    return false;
}

static bool browser_folder_check_and_switch(string_t path) {
    FileInfo file_info;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool is_root = false;

    if(string_search_rchar(path, '/') == 0) {
        is_root = true;
    }

    while(1) {
        // Check if folder is existing and navigate back if not
        if(storage_common_stat(storage, string_get_cstr(path), &file_info) == FSE_OK) {
            if(file_info.flags & FSF_DIRECTORY) {
                break;
            }
        }
        if(is_root) {
            break;
        }
        is_root = browser_path_trim(path);
    }
    furi_record_close(RECORD_STORAGE);
    return is_root;
}

static bool browser_folder_init(
    BrowserWorker* browser,
    string_t path,
    string_t filename,
    uint32_t* item_cnt,
    int32_t* file_idx) {
    bool state = false;
    FileInfo file_info;
    uint32_t total_files_cnt = 0;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* directory = storage_file_alloc(storage);

    char name_temp[FILE_NAME_LEN_MAX];
    string_t name_str;
    string_init(name_str);

    *item_cnt = 0;
    *file_idx = -1;

    if(storage_dir_open(directory, string_get_cstr(path))) {
        state = true;
        while(1) {
            if(!storage_dir_read(directory, &file_info, name_temp, FILE_NAME_LEN_MAX)) {
                break;
            }
            if((storage_file_get_error(directory) == FSE_OK) && (name_temp[0] != '\0')) {
                total_files_cnt++;
                string_set_str(name_str, name_temp);
                if(browser_filter_by_name(browser, name_str, (file_info.flags & FSF_DIRECTORY))) {
                    if(!string_empty_p(filename)) {
                        if(string_cmp(name_str, filename) == 0) {
                            *file_idx = *item_cnt;
                        }
                    }
                    (*item_cnt)++;
                }
                if(total_files_cnt == LONG_LOAD_THRESHOLD) {
                    // There are too many files in folder and counting them will take some time - send callback to app
                    if(browser->long_load_cb) {
                        browser->long_load_cb(browser->cb_ctx);
                    }
                }
            }
        }
    }

    string_clear(name_str);

    storage_dir_close(directory);
    storage_file_free(directory);

    furi_record_close(RECORD_STORAGE);

    return state;
}

static bool
    browser_folder_load(BrowserWorker* browser, string_t path, uint32_t offset, uint32_t count) {
    FileInfo file_info;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* directory = storage_file_alloc(storage);

    char name_temp[FILE_NAME_LEN_MAX];
    string_t name_str;
    string_init(name_str);

    uint32_t items_cnt = 0;

    do {
        if(!storage_dir_open(directory, string_get_cstr(path))) {
            break;
        }

        items_cnt = 0;
        while(items_cnt < offset) {
            if(!storage_dir_read(directory, &file_info, name_temp, FILE_NAME_LEN_MAX)) {
                break;
            }
            if(storage_file_get_error(directory) == FSE_OK) {
                string_set_str(name_str, name_temp);
                if(browser_filter_by_name(browser, name_str, (file_info.flags & FSF_DIRECTORY))) {
                    items_cnt++;
                }
            } else {
                break;
            }
        }
        if(items_cnt != offset) {
            break;
        }

        if(browser->list_load_cb) {
            browser->list_load_cb(browser->cb_ctx, offset);
        }

        items_cnt = 0;
        while(items_cnt < count) {
            if(!storage_dir_read(directory, &file_info, name_temp, FILE_NAME_LEN_MAX)) {
                break;
            }
            if(storage_file_get_error(directory) == FSE_OK) {
                string_set_str(name_str, name_temp);
                if(browser_filter_by_name(browser, name_str, (file_info.flags & FSF_DIRECTORY))) {
                    string_printf(name_str, "%s/%s", string_get_cstr(path), name_temp);
                    if(browser->list_item_cb) {
                        browser->list_item_cb(
                            browser->cb_ctx, name_str, (file_info.flags & FSF_DIRECTORY), false);
                    }
                    items_cnt++;
                }
            } else {
                break;
            }
        }
        if(browser->list_item_cb) {
            browser->list_item_cb(browser->cb_ctx, NULL, false, true);
        }
    } while(0);

    string_clear(name_str);

    storage_dir_close(directory);
    storage_file_free(directory);

    furi_record_close(RECORD_STORAGE);

    return (items_cnt == count);
}

static int32_t browser_worker(void* context) {
    BrowserWorker* browser = (BrowserWorker*)context;
    furi_assert(browser);
    FURI_LOG_D(TAG, "Start");

    uint32_t items_cnt = 0;
    string_t path;
    string_init_set_str(path, BROWSER_ROOT);
    browser->item_sel_idx = -1;

    string_t filename;
    string_init(filename);

    furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtConfigChange);

    while(1) {
        uint32_t flags =
            furi_thread_flags_wait(WORKER_FLAGS_ALL, FuriFlagWaitAny, FuriWaitForever);
        furi_assert((flags & FuriFlagError) == 0);

        if(flags & WorkerEvtConfigChange) {
            // If start path is a path to the file - try finding index of this file in a folder
            if(browser_path_is_file(browser->path_next)) {
                path_extract_filename(browser->path_next, filename, false);
            }
            idx_last_array_reset(browser->idx_last);

            furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtFolderEnter);
        }

        if(flags & WorkerEvtFolderEnter) {
            string_set(path, browser->path_next);
            bool is_root = browser_folder_check_and_switch(path);

            // Push previous selected item index to history array
            idx_last_array_push_back(browser->idx_last, browser->item_sel_idx);

            int32_t file_idx = 0;
            browser_folder_init(browser, path, filename, &items_cnt, &file_idx);
            FURI_LOG_D(
                TAG,
                "Enter folder: %s items: %u idx: %d",
                string_get_cstr(path),
                items_cnt,
                file_idx);
            if(browser->folder_cb) {
                browser->folder_cb(browser->cb_ctx, items_cnt, file_idx, is_root);
            }
            string_reset(filename);
        }

        if(flags & WorkerEvtFolderExit) {
            browser_path_trim(path);
            bool is_root = browser_folder_check_and_switch(path);

            int32_t file_idx = 0;
            browser_folder_init(browser, path, filename, &items_cnt, &file_idx);
            if(idx_last_array_size(browser->idx_last) > 0) {
                // Pop previous selected item index from history array
                idx_last_array_pop_back(&file_idx, browser->idx_last);
            }
            FURI_LOG_D(
                TAG, "Exit to: %s items: %u idx: %d", string_get_cstr(path), items_cnt, file_idx);
            if(browser->folder_cb) {
                browser->folder_cb(browser->cb_ctx, items_cnt, file_idx, is_root);
            }
        }

        if(flags & WorkerEvtFolderRefresh) {
            bool is_root = browser_folder_check_and_switch(path);

            int32_t file_idx = 0;
            string_reset(filename);
            browser_folder_init(browser, path, filename, &items_cnt, &file_idx);
            FURI_LOG_D(
                TAG,
                "Refresh folder: %s items: %u idx: %d",
                string_get_cstr(path),
                items_cnt,
                browser->item_sel_idx);
            if(browser->folder_cb) {
                browser->folder_cb(browser->cb_ctx, items_cnt, browser->item_sel_idx, is_root);
            }
        }

        if(flags & WorkerEvtLoad) {
            FURI_LOG_D(TAG, "Load offset: %u cnt: %u", browser->load_offset, browser->load_count);
            browser_folder_load(browser, path, browser->load_offset, browser->load_count);
        }

        if(flags & WorkerEvtStop) {
            break;
        }
    }

    string_clear(filename);
    string_clear(path);

    FURI_LOG_D(TAG, "End");
    return 0;
}

BrowserWorker* file_browser_worker_alloc(string_t path, const char* filter_ext, bool skip_assets) {
    BrowserWorker* browser = malloc(sizeof(BrowserWorker));

    idx_last_array_init(browser->idx_last);

    string_init_set_str(browser->filter_extension, filter_ext);
    browser->skip_assets = skip_assets;
    string_init_set(browser->path_next, path);

    browser->thread = furi_thread_alloc();
    furi_thread_set_name(browser->thread, "BrowserWorker");
    furi_thread_set_stack_size(browser->thread, 2048);
    furi_thread_set_context(browser->thread, browser);
    furi_thread_set_callback(browser->thread, browser_worker);
    furi_thread_start(browser->thread);

    return browser;
}

void file_browser_worker_free(BrowserWorker* browser) {
    furi_assert(browser);

    furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtStop);
    furi_thread_join(browser->thread);
    furi_thread_free(browser->thread);

    string_clear(browser->filter_extension);
    string_clear(browser->path_next);

    idx_last_array_clear(browser->idx_last);

    free(browser);
}

void file_browser_worker_set_callback_context(BrowserWorker* browser, void* context) {
    furi_assert(browser);
    browser->cb_ctx = context;
}

void file_browser_worker_set_folder_callback(
    BrowserWorker* browser,
    BrowserWorkerFolderOpenCallback cb) {
    furi_assert(browser);
    browser->folder_cb = cb;
}

void file_browser_worker_set_list_callback(
    BrowserWorker* browser,
    BrowserWorkerListLoadCallback cb) {
    furi_assert(browser);
    browser->list_load_cb = cb;
}

void file_browser_worker_set_item_callback(
    BrowserWorker* browser,
    BrowserWorkerListItemCallback cb) {
    furi_assert(browser);
    browser->list_item_cb = cb;
}

void file_browser_worker_set_long_load_callback(
    BrowserWorker* browser,
    BrowserWorkerLongLoadCallback cb) {
    furi_assert(browser);
    browser->long_load_cb = cb;
}

void file_browser_worker_set_config(
    BrowserWorker* browser,
    string_t path,
    const char* filter_ext,
    bool skip_assets) {
    furi_assert(browser);
    string_set(browser->path_next, path);
    string_set_str(browser->filter_extension, filter_ext);
    browser->skip_assets = skip_assets;
    furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtConfigChange);
}

void file_browser_worker_folder_enter(BrowserWorker* browser, string_t path, int32_t item_idx) {
    furi_assert(browser);
    string_set(browser->path_next, path);
    browser->item_sel_idx = item_idx;
    furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtFolderEnter);
}

void file_browser_worker_folder_exit(BrowserWorker* browser) {
    furi_assert(browser);
    furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtFolderExit);
}

void file_browser_worker_folder_refresh(BrowserWorker* browser, int32_t item_idx) {
    furi_assert(browser);
    browser->item_sel_idx = item_idx;
    furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtFolderRefresh);
}

void file_browser_worker_load(BrowserWorker* browser, uint32_t offset, uint32_t count) {
    furi_assert(browser);
    browser->load_offset = offset;
    browser->load_count = count;
    furi_thread_flags_set(furi_thread_get_id(browser->thread), WorkerEvtLoad);
}
