/**
 * @file file_browser.h
 * GUI: FileBrowser view module API
 */

#pragma once

#include "m-string.h"
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FileBrowser FileBrowser;
typedef void (*FileBrowserCallback)(void* context);

typedef bool (
    *FileBrowserLoadItemCallback)(string_t path, void* context, uint8_t** icon, string_t item_name);

FileBrowser* file_browser_alloc(string_ptr result_path);

void file_browser_free(FileBrowser* browser);

View* file_browser_get_view(FileBrowser* browser);

void file_browser_configure(
    FileBrowser* browser,
    const char* extension,
    bool skip_assets,
    const Icon* file_icon,
    bool hide_ext);

void file_browser_start(FileBrowser* browser, string_t path);

void file_browser_stop(FileBrowser* browser);

void file_browser_set_callback(FileBrowser* browser, FileBrowserCallback callback, void* context);

void file_browser_set_item_callback(
    FileBrowser* browser,
    FileBrowserLoadItemCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
