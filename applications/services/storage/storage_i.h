#pragma once
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include "storage_glue.h"
#include "storage_sd_api.h"
#include "filesystem_api_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_COUNT (ST_INT + 1)

#define APPS_DATA_PATH EXT_PATH("apps_data")
#define APPS_ASSETS_PATH EXT_PATH("apps_assets")

typedef struct {
    ViewPort* view_port;
    bool enabled;
} StorageSDGui;

struct Storage {
    FuriMessageQueue* message_queue;
    StorageData storage[STORAGE_COUNT];
    StorageSDGui sd_gui;
    FuriPubSub* pubsub;
};

#ifdef __cplusplus
}
#endif
