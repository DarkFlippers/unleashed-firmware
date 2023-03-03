#include "infrared_i.h"

#include <string.h>
#include <dolphin/dolphin.h>

#define INFRARED_TX_MIN_INTERVAL_MS 50U

static const NotificationSequence* infrared_notification_sequences[] = {
    &sequence_success,
    &sequence_set_only_green_255,
    &sequence_reset_green,
    &sequence_solid_yellow,
    &sequence_reset_rgb,
    &sequence_blink_start_cyan,
    &sequence_blink_start_magenta,
    &sequence_blink_stop,
};

static void infrared_make_app_folder(Infrared* infrared) {
    if(!storage_simply_mkdir(infrared->storage, INFRARED_APP_FOLDER)) {
        dialog_message_show_storage_error(infrared->dialogs, "Cannot create\napp folder");
    }
}

static bool infrared_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Infrared* infrared = context;
    return scene_manager_handle_custom_event(infrared->scene_manager, event);
}

static bool infrared_back_event_callback(void* context) {
    furi_assert(context);
    Infrared* infrared = context;
    return scene_manager_handle_back_event(infrared->scene_manager);
}

static void infrared_tick_event_callback(void* context) {
    furi_assert(context);
    Infrared* infrared = context;
    scene_manager_handle_tick_event(infrared->scene_manager);
}

static void infrared_rpc_command_callback(RpcAppSystemEvent event, void* context) {
    furi_assert(context);
    Infrared* infrared = context;
    furi_assert(infrared->rpc_ctx);

    if(event == RpcAppEventSessionClose) {
        view_dispatcher_send_custom_event(
            infrared->view_dispatcher, InfraredCustomEventTypeRpcSessionClose);
        rpc_system_app_set_callback(infrared->rpc_ctx, NULL, NULL);
        infrared->rpc_ctx = NULL;
    } else if(event == RpcAppEventAppExit) {
        view_dispatcher_send_custom_event(
            infrared->view_dispatcher, InfraredCustomEventTypeRpcExit);
    } else if(event == RpcAppEventLoadFile) {
        view_dispatcher_send_custom_event(
            infrared->view_dispatcher, InfraredCustomEventTypeRpcLoad);
    } else if(event == RpcAppEventButtonPress) {
        view_dispatcher_send_custom_event(
            infrared->view_dispatcher, InfraredCustomEventTypeRpcButtonPress);
    } else if(event == RpcAppEventButtonRelease) {
        view_dispatcher_send_custom_event(
            infrared->view_dispatcher, InfraredCustomEventTypeRpcButtonRelease);
    } else {
        rpc_system_app_confirm(infrared->rpc_ctx, event, false);
    }
}

static void infrared_find_vacant_remote_name(FuriString* name, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FuriString* base_path;
    base_path = furi_string_alloc_set(path);

    if(furi_string_end_with(base_path, INFRARED_APP_EXTENSION)) {
        size_t filename_start = furi_string_search_rchar(base_path, '/');
        furi_string_left(base_path, filename_start);
    }

    furi_string_printf(
        base_path, "%s/%s%s", path, furi_string_get_cstr(name), INFRARED_APP_EXTENSION);

    FS_Error status = storage_common_stat(storage, furi_string_get_cstr(base_path), NULL);

    if(status == FSE_OK) {
        /* If the suggested name is occupied, try another one (name2, name3, etc) */
        size_t dot = furi_string_search_rchar(base_path, '.');
        furi_string_left(base_path, dot);

        FuriString* path_temp;
        path_temp = furi_string_alloc();

        uint32_t i = 1;
        do {
            furi_string_printf(
                path_temp, "%s%lu%s", furi_string_get_cstr(base_path), ++i, INFRARED_APP_EXTENSION);
            status = storage_common_stat(storage, furi_string_get_cstr(path_temp), NULL);
        } while(status == FSE_OK);

        furi_string_free(path_temp);

        if(status == FSE_NOT_EXIST) {
            furi_string_cat_printf(name, "%lu", i);
        }
    }

    furi_string_free(base_path);
    furi_record_close(RECORD_STORAGE);
}

static Infrared* infrared_alloc() {
    Infrared* infrared = malloc(sizeof(Infrared));

    infrared->file_path = furi_string_alloc();

    InfraredAppState* app_state = &infrared->app_state;
    app_state->is_learning_new_remote = false;
    app_state->is_debug_enabled = furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug);
    app_state->edit_target = InfraredEditTargetNone;
    app_state->edit_mode = InfraredEditModeNone;
    app_state->current_button_index = InfraredButtonIndexNone;

    infrared->scene_manager = scene_manager_alloc(&infrared_scene_handlers, infrared);
    infrared->view_dispatcher = view_dispatcher_alloc();

    infrared->gui = furi_record_open(RECORD_GUI);

    ViewDispatcher* view_dispatcher = infrared->view_dispatcher;
    view_dispatcher_enable_queue(view_dispatcher);
    view_dispatcher_set_event_callback_context(view_dispatcher, infrared);
    view_dispatcher_set_custom_event_callback(view_dispatcher, infrared_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(view_dispatcher, infrared_back_event_callback);
    view_dispatcher_set_tick_event_callback(view_dispatcher, infrared_tick_event_callback, 100);

    infrared->storage = furi_record_open(RECORD_STORAGE);
    infrared->dialogs = furi_record_open(RECORD_DIALOGS);
    infrared->notifications = furi_record_open(RECORD_NOTIFICATION);

    infrared->worker = infrared_worker_alloc();
    infrared->remote = infrared_remote_alloc();
    infrared->received_signal = infrared_signal_alloc();
    infrared->brute_force = infrared_brute_force_alloc();

    infrared->submenu = submenu_alloc();
    view_dispatcher_add_view(
        view_dispatcher, InfraredViewSubmenu, submenu_get_view(infrared->submenu));

    infrared->text_input = text_input_alloc();
    view_dispatcher_add_view(
        view_dispatcher, InfraredViewTextInput, text_input_get_view(infrared->text_input));

    infrared->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        view_dispatcher, InfraredViewDialogEx, dialog_ex_get_view(infrared->dialog_ex));

    infrared->button_menu = button_menu_alloc();
    view_dispatcher_add_view(
        view_dispatcher, InfraredViewButtonMenu, button_menu_get_view(infrared->button_menu));

    infrared->popup = popup_alloc();
    view_dispatcher_add_view(view_dispatcher, InfraredViewPopup, popup_get_view(infrared->popup));

    infrared->view_stack = view_stack_alloc();
    view_dispatcher_add_view(
        view_dispatcher, InfraredViewStack, view_stack_get_view(infrared->view_stack));

    if(app_state->is_debug_enabled) {
        infrared->debug_view = infrared_debug_view_alloc();
        view_dispatcher_add_view(
            view_dispatcher,
            InfraredViewDebugView,
            infrared_debug_view_get_view(infrared->debug_view));
    }

    infrared->button_panel = button_panel_alloc();
    infrared->loading = loading_alloc();
    infrared->progress = infrared_progress_view_alloc();

    return infrared;
}

static void infrared_free(Infrared* infrared) {
    furi_assert(infrared);
    ViewDispatcher* view_dispatcher = infrared->view_dispatcher;
    InfraredAppState* app_state = &infrared->app_state;

    if(infrared->rpc_ctx) {
        rpc_system_app_set_callback(infrared->rpc_ctx, NULL, NULL);
        rpc_system_app_send_exited(infrared->rpc_ctx);
        infrared->rpc_ctx = NULL;
    }

    view_dispatcher_remove_view(view_dispatcher, InfraredViewSubmenu);
    submenu_free(infrared->submenu);

    view_dispatcher_remove_view(view_dispatcher, InfraredViewTextInput);
    text_input_free(infrared->text_input);

    view_dispatcher_remove_view(view_dispatcher, InfraredViewDialogEx);
    dialog_ex_free(infrared->dialog_ex);

    view_dispatcher_remove_view(view_dispatcher, InfraredViewButtonMenu);
    button_menu_free(infrared->button_menu);

    view_dispatcher_remove_view(view_dispatcher, InfraredViewPopup);
    popup_free(infrared->popup);

    view_dispatcher_remove_view(view_dispatcher, InfraredViewStack);
    view_stack_free(infrared->view_stack);

    if(app_state->is_debug_enabled) {
        view_dispatcher_remove_view(view_dispatcher, InfraredViewDebugView);
        infrared_debug_view_free(infrared->debug_view);
    }

    button_panel_free(infrared->button_panel);
    loading_free(infrared->loading);
    infrared_progress_view_free(infrared->progress);

    view_dispatcher_free(view_dispatcher);
    scene_manager_free(infrared->scene_manager);

    infrared_brute_force_free(infrared->brute_force);
    infrared_signal_free(infrared->received_signal);
    infrared_remote_free(infrared->remote);
    infrared_worker_free(infrared->worker);

    furi_record_close(RECORD_NOTIFICATION);
    infrared->notifications = NULL;

    furi_record_close(RECORD_DIALOGS);
    infrared->dialogs = NULL;

    furi_record_close(RECORD_GUI);
    infrared->gui = NULL;

    furi_string_free(infrared->file_path);

    free(infrared);
}

bool infrared_add_remote_with_button(
    Infrared* infrared,
    const char* button_name,
    InfraredSignal* signal) {
    InfraredRemote* remote = infrared->remote;

    FuriString *new_name, *new_path;
    new_name = furi_string_alloc_set(INFRARED_DEFAULT_REMOTE_NAME);
    new_path = furi_string_alloc_set(INFRARED_APP_FOLDER);

    infrared_find_vacant_remote_name(new_name, furi_string_get_cstr(new_path));
    furi_string_cat_printf(
        new_path, "/%s%s", furi_string_get_cstr(new_name), INFRARED_APP_EXTENSION);

    infrared_remote_reset(remote);
    infrared_remote_set_name(remote, furi_string_get_cstr(new_name));
    infrared_remote_set_path(remote, furi_string_get_cstr(new_path));

    furi_string_free(new_name);
    furi_string_free(new_path);
    return infrared_remote_add_button(remote, button_name, signal);
}

bool infrared_rename_current_remote(Infrared* infrared, const char* name) {
    InfraredRemote* remote = infrared->remote;
    const char* remote_path = infrared_remote_get_path(remote);

    if(!strcmp(infrared_remote_get_name(remote), name)) {
        return true;
    }

    FuriString* new_name;
    new_name = furi_string_alloc_set(name);

    infrared_find_vacant_remote_name(new_name, remote_path);

    FuriString* new_path;
    new_path = furi_string_alloc_set(infrared_remote_get_path(remote));
    if(furi_string_end_with(new_path, INFRARED_APP_EXTENSION)) {
        size_t filename_start = furi_string_search_rchar(new_path, '/');
        furi_string_left(new_path, filename_start);
    }
    furi_string_cat_printf(
        new_path, "/%s%s", furi_string_get_cstr(new_name), INFRARED_APP_EXTENSION);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    FS_Error status = storage_common_rename(
        storage, infrared_remote_get_path(remote), furi_string_get_cstr(new_path));
    infrared_remote_set_name(remote, furi_string_get_cstr(new_name));
    infrared_remote_set_path(remote, furi_string_get_cstr(new_path));

    furi_string_free(new_name);
    furi_string_free(new_path);

    furi_record_close(RECORD_STORAGE);
    return (status == FSE_OK || status == FSE_EXIST);
}

void infrared_tx_start_signal(Infrared* infrared, InfraredSignal* signal) {
    if(infrared->app_state.is_transmitting) {
        return;
    }

    const uint32_t time_elapsed = furi_get_tick() - infrared->app_state.last_transmit_time;

    if(time_elapsed < INFRARED_TX_MIN_INTERVAL_MS) {
        return;
    }

    if(infrared_signal_is_raw(signal)) {
        InfraredRawSignal* raw = infrared_signal_get_raw_signal(signal);
        infrared_worker_set_raw_signal(infrared->worker, raw->timings, raw->timings_size);
    } else {
        InfraredMessage* message = infrared_signal_get_message(signal);
        infrared_worker_set_decoded_signal(infrared->worker, message);
    }

    DOLPHIN_DEED(DolphinDeedIrSend);
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStartSend);

    infrared_worker_tx_set_get_signal_callback(
        infrared->worker, infrared_worker_tx_get_signal_steady_callback, infrared);
    infrared_worker_tx_start(infrared->worker);

    infrared->app_state.is_transmitting = true;
}

void infrared_tx_start_button_index(Infrared* infrared, size_t button_index) {
    furi_assert(button_index < infrared_remote_get_button_count(infrared->remote));

    InfraredRemoteButton* button = infrared_remote_get_button(infrared->remote, button_index);
    InfraredSignal* signal = infrared_remote_button_get_signal(button);

    infrared_tx_start_signal(infrared, signal);
}

void infrared_tx_start_received(Infrared* infrared) {
    infrared_tx_start_signal(infrared, infrared->received_signal);
}

void infrared_tx_stop(Infrared* infrared) {
    if(!infrared->app_state.is_transmitting) {
        return;
    }

    infrared_worker_tx_stop(infrared->worker);
    infrared_worker_tx_set_get_signal_callback(infrared->worker, NULL, NULL);

    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStop);

    infrared->app_state.is_transmitting = false;
    infrared->app_state.last_transmit_time = furi_get_tick();
}

void infrared_text_store_set(Infrared* infrared, uint32_t bank, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(infrared->text_store[bank], INFRARED_TEXT_STORE_SIZE, text, args);

    va_end(args);
}

void infrared_text_store_clear(Infrared* infrared, uint32_t bank) {
    memset(infrared->text_store[bank], 0, INFRARED_TEXT_STORE_SIZE + 1);
}

void infrared_play_notification_message(Infrared* infrared, uint32_t message) {
    furi_assert(message < sizeof(infrared_notification_sequences) / sizeof(NotificationSequence*));
    notification_message(infrared->notifications, infrared_notification_sequences[message]);
}

void infrared_show_loading_popup(Infrared* infrared, bool show) {
    TaskHandle_t timer_task = xTaskGetHandle(configTIMER_SERVICE_TASK_NAME);
    ViewStack* view_stack = infrared->view_stack;
    Loading* loading = infrared->loading;

    if(show) {
        // Raise timer priority so that animations can play
        vTaskPrioritySet(timer_task, configMAX_PRIORITIES - 1);
        view_stack_add_view(view_stack, loading_get_view(loading));
    } else {
        view_stack_remove_view(view_stack, loading_get_view(loading));
        // Restore default timer priority
        vTaskPrioritySet(timer_task, configTIMER_TASK_PRIORITY);
    }
}

void infrared_signal_received_callback(void* context, InfraredWorkerSignal* received_signal) {
    furi_assert(context);
    Infrared* infrared = context;

    if(infrared_worker_signal_is_decoded(received_signal)) {
        infrared_signal_set_message(
            infrared->received_signal, infrared_worker_get_decoded_signal(received_signal));
    } else {
        const uint32_t* timings;
        size_t timings_size;
        infrared_worker_get_raw_signal(received_signal, &timings, &timings_size);
        infrared_signal_set_raw_signal(
            infrared->received_signal,
            timings,
            timings_size,
            INFRARED_COMMON_CARRIER_FREQUENCY,
            INFRARED_COMMON_DUTY_CYCLE);
    }

    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeSignalReceived);
}

void infrared_text_input_callback(void* context) {
    furi_assert(context);
    Infrared* infrared = context;
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTextEditDone);
}

void infrared_popup_closed_callback(void* context) {
    furi_assert(context);
    Infrared* infrared = context;
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypePopupClosed);
}

int32_t infrared_app(void* p) {
    Infrared* infrared = infrared_alloc();

    infrared_make_app_folder(infrared);

    bool is_remote_loaded = false;
    bool is_rpc_mode = false;

    if(p && strlen(p)) {
        uint32_t rpc_ctx = 0;
        if(sscanf(p, "RPC %lX", &rpc_ctx) == 1) {
            infrared->rpc_ctx = (void*)rpc_ctx;
            rpc_system_app_set_callback(
                infrared->rpc_ctx, infrared_rpc_command_callback, infrared);
            rpc_system_app_send_started(infrared->rpc_ctx);
            is_rpc_mode = true;
        } else {
            furi_string_set(infrared->file_path, (const char*)p);
            is_remote_loaded = infrared_remote_load(infrared->remote, infrared->file_path);
            if(!is_remote_loaded) {
                dialog_message_show_storage_error(
                    infrared->dialogs, "Failed to load\nselected remote");
                return -1;
            }
        }
    }

    if(is_rpc_mode) {
        view_dispatcher_attach_to_gui(
            infrared->view_dispatcher, infrared->gui, ViewDispatcherTypeDesktop);
        scene_manager_next_scene(infrared->scene_manager, InfraredSceneRpc);
    } else {
        view_dispatcher_attach_to_gui(
            infrared->view_dispatcher, infrared->gui, ViewDispatcherTypeFullscreen);
        if(is_remote_loaded) { //-V547
            scene_manager_next_scene(infrared->scene_manager, InfraredSceneRemote);
        } else {
            scene_manager_next_scene(infrared->scene_manager, InfraredSceneStart);
        }
    }

    view_dispatcher_run(infrared->view_dispatcher);

    infrared_free(infrared);
    return 0;
}
