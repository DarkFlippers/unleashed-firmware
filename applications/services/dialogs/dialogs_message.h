#pragma once
#include <furi.h>
#include "dialogs_i.h"
#include <toolbox/api_lock.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* extension;
    bool skip_assets;
    bool hide_ext;
    bool hide_dot_files;
    const Icon* file_icon;
    FuriString* result_path;
    FuriString* preselected_filename;
    FileBrowserLoadItemCallback item_callback;
    void* item_callback_context;
    const char* base_path;
} DialogsAppMessageDataFileBrowser;

typedef struct {
    const DialogMessage* message;
} DialogsAppMessageDataDialog;

typedef union {
    DialogsAppMessageDataFileBrowser file_browser;
    DialogsAppMessageDataDialog dialog;
} DialogsAppData;

typedef union {
    bool bool_value;
    DialogMessageButton dialog_value;
} DialogsAppReturn;

typedef enum {
    DialogsAppCommandFileBrowser,
    DialogsAppCommandDialog,
} DialogsAppCommand;

typedef struct {
    FuriApiLock lock;
    DialogsAppCommand command;
    DialogsAppData* data;
    DialogsAppReturn* return_data;
} DialogsAppMessage;

#ifdef __cplusplus
}
#endif
