#pragma once

#include "m-string.h"
#include <gui/view.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BrowserWorker BrowserWorker;
typedef void (*BrowserWorkerFolderOpenCallback)(
    void* context,
    uint32_t item_cnt,
    int32_t file_idx,
    bool is_root);
typedef void (*BrowserWorkerListLoadCallback)(void* context, uint32_t list_load_offset);
typedef void (*BrowserWorkerListItemCallback)(
    void* context,
    string_t item_path,
    bool is_folder,
    bool is_last);
typedef void (*BrowserWorkerLongLoadCallback)(void* context);

void file_browser_worker_get_filename(string_t path, string_t name, bool trim_ext);

BrowserWorker* file_browser_worker_alloc(string_t path, char* filter_ext, bool skip_assets);

void file_browser_worker_free(BrowserWorker* browser);

void file_browser_worker_set_callback_context(BrowserWorker* browser, void* context);

void file_browser_worker_set_folder_callback(
    BrowserWorker* browser,
    BrowserWorkerFolderOpenCallback cb);

void file_browser_worker_set_list_callback(
    BrowserWorker* browser,
    BrowserWorkerListLoadCallback cb);

void file_browser_worker_set_item_callback(
    BrowserWorker* browser,
    BrowserWorkerListItemCallback cb);

void file_browser_worker_set_long_load_callback(
    BrowserWorker* browser,
    BrowserWorkerLongLoadCallback cb);

void file_browser_worker_folder_enter(BrowserWorker* browser, string_t path, int32_t item_idx);

void file_browser_worker_folder_exit(BrowserWorker* browser);

void file_browser_worker_load(BrowserWorker* browser, uint32_t offset, uint32_t count);

#ifdef __cplusplus
}
#endif
