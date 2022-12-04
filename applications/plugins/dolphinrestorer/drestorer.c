#include "drestorer.h"
#include <core/common_defines.h>
#include <core/log.h>
#include "loader/loader.h"
#include "m-string.h"
#include <stdint.h>

#define TAG "MoveToInt"

#define MOVE_SRC "/ext/dolphin_restorer"
#define MOVE_DST "/int"

static const char* app_dirs[] = {
    ".bt.settings",
    ".desktop.settings",
    ".dolphin.state",
    ".notification.settings",
    ".bt.keys",
    ".power.settings",
};

bool drestorer_perform(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* path_src;
    FuriString* path_dst;
    path_src = furi_string_alloc();
    path_dst = furi_string_alloc();

    for(uint32_t i = 0; i < COUNT_OF(app_dirs); i++) {
        furi_string_printf(path_src, "%s/%s", MOVE_SRC, app_dirs[i]);
        furi_string_printf(path_dst, "%s/%s", MOVE_DST, app_dirs[i]);
        storage_simply_remove_recursive(storage, furi_string_get_cstr(path_dst));
        storage_common_copy(
            storage, furi_string_get_cstr(path_src), furi_string_get_cstr(path_dst));
    }

    furi_string_free(path_src);
    furi_string_free(path_dst);

    furi_record_close(RECORD_STORAGE);

    return false;
}

static bool drestorer_check(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FileInfo file_info;
    bool state = false;
    FuriString* path;
    path = furi_string_alloc();

    for(uint32_t i = 0; i < COUNT_OF(app_dirs); i++) {
        furi_string_printf(path, "%s/%s", MOVE_SRC, app_dirs[i]);
        if(storage_common_stat(storage, furi_string_get_cstr(path), &file_info) == FSE_OK) {
            // if((file_info.flags & FSF_DIRECTORY) != 0) {
            state = true;
            break;
            // }
        }
    }

    furi_string_free(path);

    furi_record_close(RECORD_STORAGE);

    return state;
}

static bool drestorer_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    StorageMoveToSd* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool drestorer_back_event_callback(void* context) {
    furi_assert(context);
    StorageMoveToSd* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void drestorer_unmount_callback(const void* message, void* context) {
    StorageMoveToSd* app = context;
    furi_assert(app);
    const StorageEvent* storage_event = message;

    if((storage_event->type == StorageEventTypeCardUnmount) ||
       (storage_event->type == StorageEventTypeCardMountError)) {
        view_dispatcher_send_custom_event(app->view_dispatcher, MoveToSdCustomEventExit);
    }
}

static StorageMoveToSd* drestorer_alloc() {
    StorageMoveToSd* app = malloc(sizeof(StorageMoveToSd));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&drestorer_scene_handlers, app);

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, drestorer_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, drestorer_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, StorageMoveToSdViewWidget, widget_get_view(app->widget));

    scene_manager_next_scene(app->scene_manager, StorageMoveToSdConfirm);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    app->sub = furi_pubsub_subscribe(storage_get_pubsub(storage), drestorer_unmount_callback, app);
    furi_record_close(RECORD_STORAGE);

    return app;
}

static void drestorer_free(StorageMoveToSd* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_unsubscribe(storage_get_pubsub(storage), app->sub);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);

    view_dispatcher_remove_view(app->view_dispatcher, StorageMoveToSdViewWidget);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t drestorer_app(void* p) {
    UNUSED(p);

    if(drestorer_check()) {
        StorageMoveToSd* app = drestorer_alloc();
        notification_message(app->notifications, &sequence_display_backlight_on);
        view_dispatcher_run(app->view_dispatcher);
        drestorer_free(app);
    } else {
        FURI_LOG_I(TAG, "Nothing to move");
    }

    return 0;
}

static void drestorer_mount_callback(const void* message, void* context) {
    UNUSED(context);

    const StorageEvent* storage_event = message;

    if(storage_event->type == StorageEventTypeCardMount) {
        Loader* loader = furi_record_open("loader");
        loader_start(loader, "StorageMoveToSd", NULL);
        furi_record_close("loader");
    }
}

int32_t drestorer_start(void* p) {
    UNUSED(p);
    Storage* storage = furi_record_open(RECORD_STORAGE);

    furi_pubsub_subscribe(storage_get_pubsub(storage), drestorer_mount_callback, NULL);

    furi_record_close(RECORD_STORAGE);
    return 0;
}
