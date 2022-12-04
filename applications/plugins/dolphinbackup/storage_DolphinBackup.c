#include "storage_DolphinBackup.h"
#include <core/common_defines.h>
#include <core/log.h>
#include "loader/loader.h"
#include "m-string.h"
#include <stdint.h>

#define TAG "DolphinBackup"

#define MOVE_SRC "/int"
#define MOVE_DST "/ext"

static const char* app_dirsDolphinBackup[] = {
    "subghz",
    "lfrfid",
    "nfc",
    "infrared",
    "ibutton",
    "badusb",
    ".bt.settings",
    ".desktop.settings",
    ".dolphin.state",
    ".notification.settings",
    ".bt.keys",
    ".power.settings",
};

bool storage_DolphinBackup_perform(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* path_src;
    FuriString* path_dst;
    FuriString* new_path;
    path_src = furi_string_alloc();
    path_dst = furi_string_alloc();
    new_path = furi_string_alloc();

    furi_string_printf(new_path, "%s/dolphin_restorer", MOVE_DST);
    storage_common_mkdir(storage, furi_string_get_cstr(new_path));
    furi_string_free(new_path);
    for(uint32_t i = 0; i < COUNT_OF(app_dirsDolphinBackup); i++) {
        if(i > 5) {
            furi_string_printf(path_src, "%s/%s", MOVE_SRC, app_dirsDolphinBackup[i]);
            furi_string_printf(
                path_dst, "%s/dolphin_restorer/%s", MOVE_DST, app_dirsDolphinBackup[i]);
            storage_simply_remove_recursive(storage, furi_string_get_cstr(path_dst));
            storage_common_copy(
                storage, furi_string_get_cstr(path_src), furi_string_get_cstr(path_dst));
        } else {
            furi_string_printf(path_src, "%s/%s", MOVE_SRC, app_dirsDolphinBackup[i]);
            furi_string_printf(path_dst, "%s/%s", MOVE_DST, app_dirsDolphinBackup[i]);
            storage_common_merge(
                storage, furi_string_get_cstr(path_src), furi_string_get_cstr(path_dst));
            storage_simply_remove_recursive(storage, furi_string_get_cstr(path_src));
        }
    }

    furi_string_free(path_src);
    furi_string_free(path_dst);

    furi_record_close(RECORD_STORAGE);

    return false;
}

static bool storage_DolphinBackup_check(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FileInfo file_info;
    bool state = false;
    FuriString* path;
    path = furi_string_alloc();

    for(uint32_t i = 0; i < COUNT_OF(app_dirsDolphinBackup); i++) {
        furi_string_printf(path, "%s/%s", MOVE_SRC, app_dirsDolphinBackup[i]);
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

static bool storage_DolphinBackup_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    StorageDolphinBackup* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool storage_DolphinBackup_back_event_callback(void* context) {
    furi_assert(context);
    StorageDolphinBackup* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void storage_DolphinBackup_unmount_callback(const void* message, void* context) {
    StorageDolphinBackup* app = context;
    furi_assert(app);
    const StorageEvent* storage_event = message;

    if((storage_event->type == StorageEventTypeCardUnmount) ||
       (storage_event->type == StorageEventTypeCardMountError)) {
        view_dispatcher_send_custom_event(app->view_dispatcher, DolphinBackupCustomEventExit);
    }
}

static StorageDolphinBackup* storage_DolphinBackup_alloc() {
    StorageDolphinBackup* app = malloc(sizeof(StorageDolphinBackup));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&storage_DolphinBackup_scene_handlers, app);

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, storage_DolphinBackup_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, storage_DolphinBackup_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, StorageDolphinBackupViewWidget, widget_get_view(app->widget));

    scene_manager_next_scene(app->scene_manager, StorageDolphinBackupConfirm);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    app->sub = furi_pubsub_subscribe(
        storage_get_pubsub(storage), storage_DolphinBackup_unmount_callback, app);
    furi_record_close(RECORD_STORAGE);

    return app;
}

static void storage_DolphinBackup_free(StorageDolphinBackup* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_unsubscribe(storage_get_pubsub(storage), app->sub);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);

    view_dispatcher_remove_view(app->view_dispatcher, StorageDolphinBackupViewWidget);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t storage_DolphinBackup_app(void* p) {
    UNUSED(p);

    if(storage_DolphinBackup_check()) {
        StorageDolphinBackup* app = storage_DolphinBackup_alloc();
        notification_message(app->notifications, &sequence_display_backlight_on);
        view_dispatcher_run(app->view_dispatcher);
        storage_DolphinBackup_free(app);
    } else {
        FURI_LOG_I(TAG, "Nothing to move");
    }

    return 0;
}

static void storage_DolphinBackup_mount_callback(const void* message, void* context) {
    UNUSED(context);

    const StorageEvent* storage_event = message;

    if(storage_event->type == StorageEventTypeCardMount) {
        Loader* loader = furi_record_open("loader");
        loader_start(loader, "StorageDolphinBackup", NULL);
        furi_record_close("loader");
    }
}

int32_t storage_DolphinBackup_start(void* p) {
    UNUSED(p);
    Storage* storage = furi_record_open(RECORD_STORAGE);

    furi_pubsub_subscribe(storage_get_pubsub(storage), storage_DolphinBackup_mount_callback, NULL);

    furi_record_close(RECORD_STORAGE);
    return 0;
}
