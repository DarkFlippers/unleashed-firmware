#pragma once

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
    FuriString* item_path,
    bool is_folder,
    bool is_last);
typedef void (*BrowserWorkerLongLoadCallback)(void* context);

BrowserWorker* file_browser_worker_alloc(
    FuriString* path,
    const char* base_path,
    const char* ext_filter,
    bool skip_assets,
    bool hide_dot_files);

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

void file_browser_worker_set_config(
    BrowserWorker* browser,
    FuriString* path,
    const char* ext_filter,
    bool skip_assets,
    bool hide_dot_files);

void file_browser_worker_folder_enter(BrowserWorker* browser, FuriString* path, int32_t item_idx);

bool file_browser_worker_is_in_start_folder(BrowserWorker* browser);

void file_browser_worker_folder_exit(BrowserWorker* browser);

void file_browser_worker_folder_refresh(BrowserWorker* browser, int32_t item_idx);

void file_browser_worker_load(BrowserWorker* browser, uint32_t offset, uint32_t count);

#ifdef __cplusplus
}
#endif
