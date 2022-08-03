#include "storage.h"
#include "storage_i.h"
#include "storage_message.h"
#include "storage_processing.h"
#include "storage/storage_glue.h"
#include "storages/storage_int.h"
#include "storages/storage_ext.h"

#define STORAGE_TICK 1000

#define ICON_SD_MOUNTED &I_SDcardMounted_11x8
#define ICON_SD_ERROR &I_SDcardFail_11x8

#define TAG RECORD_STORAGE

static void storage_app_sd_icon_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    furi_assert(context);
    Storage* app = context;

    // here we don't care about thread race when reading / writing status
    switch(app->storage[ST_EXT].status) {
    case StorageStatusNotReady:
        break;
    case StorageStatusOK:
        canvas_draw_icon(canvas, 0, 0, ICON_SD_MOUNTED);
        break;
    default:
        canvas_draw_icon(canvas, 0, 0, ICON_SD_ERROR);
        break;
    }
}

Storage* storage_app_alloc() {
    Storage* app = malloc(sizeof(Storage));
    app->message_queue = furi_message_queue_alloc(8, sizeof(StorageMessage));
    app->pubsub = furi_pubsub_alloc();

    for(uint8_t i = 0; i < STORAGE_COUNT; i++) {
        storage_data_init(&app->storage[i]);
    }

#ifndef FURI_RAM_EXEC
    storage_int_init(&app->storage[ST_INT]);
#endif
    storage_ext_init(&app->storage[ST_EXT]);

    // sd icon gui
    app->sd_gui.enabled = false;
    app->sd_gui.view_port = view_port_alloc();
    view_port_set_width(app->sd_gui.view_port, icon_get_width(ICON_SD_MOUNTED));
    view_port_draw_callback_set(app->sd_gui.view_port, storage_app_sd_icon_draw_callback, app);
    view_port_enabled_set(app->sd_gui.view_port, false);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, app->sd_gui.view_port, GuiLayerStatusBarLeft);
    furi_record_close(RECORD_GUI);

    return app;
}

void storage_tick(Storage* app) {
    for(uint8_t i = 0; i < STORAGE_COUNT; i++) {
        StorageApi api = app->storage[i].api;
        if(api.tick != NULL) {
            api.tick(&app->storage[i]);
        }
    }

    // storage not enabled but was enabled (sd card unmount)
    if(app->storage[ST_EXT].status == StorageStatusNotReady && app->sd_gui.enabled == true) {
        app->sd_gui.enabled = false;
        view_port_enabled_set(app->sd_gui.view_port, false);

        FURI_LOG_I(TAG, "SD card unmount");
        StorageEvent event = {.type = StorageEventTypeCardUnmount};
        furi_pubsub_publish(app->pubsub, &event);
    }

    // storage enabled (or in error state) but was not enabled (sd card mount)
    if((app->storage[ST_EXT].status == StorageStatusOK ||
        app->storage[ST_EXT].status == StorageStatusNotMounted ||
        app->storage[ST_EXT].status == StorageStatusNoFS ||
        app->storage[ST_EXT].status == StorageStatusNotAccessible ||
        app->storage[ST_EXT].status == StorageStatusErrorInternal) &&
       app->sd_gui.enabled == false) {
        app->sd_gui.enabled = true;
        view_port_enabled_set(app->sd_gui.view_port, true);

        if(app->storage[ST_EXT].status == StorageStatusOK) {
            FURI_LOG_I(TAG, "SD card mount");
            StorageEvent event = {.type = StorageEventTypeCardMount};
            furi_pubsub_publish(app->pubsub, &event);
        } else {
            FURI_LOG_I(TAG, "SD card mount error");
            StorageEvent event = {.type = StorageEventTypeCardMountError};
            furi_pubsub_publish(app->pubsub, &event);
        }
    }
}

int32_t storage_srv(void* p) {
    UNUSED(p);
    Storage* app = storage_app_alloc();
    furi_record_create(RECORD_STORAGE, app);

    StorageMessage message;
    while(1) {
        if(furi_message_queue_get(app->message_queue, &message, STORAGE_TICK) == FuriStatusOk) {
            storage_process_message(app, &message);
        } else {
            storage_tick(app);
        }
    }

    return 0;
}
