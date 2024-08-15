#include "scenes/updater_scene.h"
#include "updater_i.h"

#include <storage/storage.h>
#include <gui/view_dispatcher.h>
#include <furi.h>
#include <furi_hal.h>
#include <stdint.h>

static bool updater_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Updater* updater = (Updater*)context;
    return scene_manager_handle_custom_event(updater->scene_manager, event);
}

static void updater_tick_event_callback(void* context) {
    furi_assert(context);
    Updater* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static bool updater_back_event_callback(void* context) {
    furi_assert(context);
    Updater* updater = (Updater*)context;
    return scene_manager_handle_back_event(updater->scene_manager);
}

static void
    status_update_cb(const char* message, const uint8_t progress, bool failed, void* context) {
    UpdaterMainView* main_view = context;
    updater_main_model_set_state(main_view, message, progress, failed);
}

Updater* updater_alloc(const char* arg) {
    Updater* updater = malloc(sizeof(Updater));
    if(arg && strlen(arg)) {
        updater->startup_arg = furi_string_alloc_set(arg);
        furi_string_replace(updater->startup_arg, ANY_PATH(""), EXT_PATH(""));
    } else {
        updater->startup_arg = furi_string_alloc();
    }

    updater->storage = furi_record_open(RECORD_STORAGE);
    updater->notification = furi_record_open(RECORD_NOTIFICATION);

    updater->gui = furi_record_open(RECORD_GUI);
    updater->view_dispatcher = view_dispatcher_alloc();
    updater->scene_manager = scene_manager_alloc(&updater_scene_handlers, updater);

    view_dispatcher_set_event_callback_context(updater->view_dispatcher, updater);
    view_dispatcher_set_custom_event_callback(
        updater->view_dispatcher, updater_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        updater->view_dispatcher, updater_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        updater->view_dispatcher, updater_tick_event_callback, UPDATER_APP_TICK);

    view_dispatcher_attach_to_gui(
        updater->view_dispatcher, updater->gui, ViewDispatcherTypeFullscreen);

    updater->main_view = updater_main_alloc();
    view_dispatcher_add_view(
        updater->view_dispatcher, UpdaterViewMain, updater_main_get_view(updater->main_view));

#ifndef FURI_RAM_EXEC
    updater->widget = widget_alloc();
    view_dispatcher_add_view(
        updater->view_dispatcher, UpdaterViewWidget, widget_get_view(updater->widget));
#endif

#ifdef FURI_RAM_EXEC
    if(true) {
#else
    FuriHalRtcBootMode boot_mode = furi_hal_rtc_get_boot_mode();
    if(!arg && ((boot_mode == FuriHalRtcBootModePreUpdate) ||
                (boot_mode == FuriHalRtcBootModePostUpdate))) {
#endif
        updater->update_task = update_task_alloc();
        update_task_set_progress_cb(updater->update_task, status_update_cb, updater->main_view);

        scene_manager_next_scene(updater->scene_manager, UpdaterSceneMain);
    } else {
#ifndef FURI_RAM_EXEC
        scene_manager_next_scene(updater->scene_manager, UpdaterSceneLoadCfg);
#endif
    }

    return updater;
}

void updater_free(Updater* updater) {
    furi_assert(updater);

    furi_string_free(updater->startup_arg);
    if(updater->update_task) {
        update_task_set_progress_cb(updater->update_task, NULL, NULL);
        update_task_free(updater->update_task);
    }

    view_dispatcher_remove_view(updater->view_dispatcher, UpdaterViewMain);
    updater_main_free(updater->main_view);

#ifndef FURI_RAM_EXEC
    view_dispatcher_remove_view(updater->view_dispatcher, UpdaterViewWidget);
    widget_free(updater->widget);
#endif

    view_dispatcher_free(updater->view_dispatcher);
    scene_manager_free(updater->scene_manager);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);

    free(updater);
}

int32_t updater_srv(void* p) {
    const char* cfgpath = p;

    Updater* updater = updater_alloc(cfgpath);
    view_dispatcher_run(updater->view_dispatcher);
    updater_free(updater);

    return 0;
}
