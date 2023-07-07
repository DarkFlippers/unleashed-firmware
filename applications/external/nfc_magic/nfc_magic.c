#include "nfc_magic_i.h"

bool nfc_magic_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    NfcMagic* nfc_magic = context;
    return scene_manager_handle_custom_event(nfc_magic->scene_manager, event);
}

bool nfc_magic_back_event_callback(void* context) {
    furi_assert(context);
    NfcMagic* nfc_magic = context;
    return scene_manager_handle_back_event(nfc_magic->scene_manager);
}

void nfc_magic_tick_event_callback(void* context) {
    furi_assert(context);
    NfcMagic* nfc_magic = context;
    scene_manager_handle_tick_event(nfc_magic->scene_manager);
}

void nfc_magic_show_loading_popup(void* context, bool show) {
    NfcMagic* nfc_magic = context;
    TaskHandle_t timer_task = xTaskGetHandle(configTIMER_SERVICE_TASK_NAME);

    if(show) {
        // Raise timer priority so that animations can play
        vTaskPrioritySet(timer_task, configMAX_PRIORITIES - 1);
        view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewLoading);
    } else {
        // Restore default timer priority
        vTaskPrioritySet(timer_task, configTIMER_TASK_PRIORITY);
    }
}

NfcMagic* nfc_magic_alloc() {
    NfcMagic* nfc_magic = malloc(sizeof(NfcMagic));

    nfc_magic->worker = nfc_magic_worker_alloc();
    nfc_magic->view_dispatcher = view_dispatcher_alloc();
    nfc_magic->scene_manager = scene_manager_alloc(&nfc_magic_scene_handlers, nfc_magic);
    view_dispatcher_enable_queue(nfc_magic->view_dispatcher);
    view_dispatcher_set_event_callback_context(nfc_magic->view_dispatcher, nfc_magic);
    view_dispatcher_set_custom_event_callback(
        nfc_magic->view_dispatcher, nfc_magic_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        nfc_magic->view_dispatcher, nfc_magic_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        nfc_magic->view_dispatcher, nfc_magic_tick_event_callback, 100);

    // Nfc device
    nfc_magic->dev = malloc(sizeof(NfcMagicDevice));
    nfc_magic->source_dev = nfc_device_alloc();
    furi_string_set(nfc_magic->source_dev->folder, NFC_APP_FOLDER);

    // Open GUI record
    nfc_magic->gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(
        nfc_magic->view_dispatcher, nfc_magic->gui, ViewDispatcherTypeFullscreen);

    // Open Notification record
    nfc_magic->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Submenu
    nfc_magic->submenu = submenu_alloc();
    view_dispatcher_add_view(
        nfc_magic->view_dispatcher, NfcMagicViewMenu, submenu_get_view(nfc_magic->submenu));

    // Popup
    nfc_magic->popup = popup_alloc();
    view_dispatcher_add_view(
        nfc_magic->view_dispatcher, NfcMagicViewPopup, popup_get_view(nfc_magic->popup));

    // Loading
    nfc_magic->loading = loading_alloc();
    view_dispatcher_add_view(
        nfc_magic->view_dispatcher, NfcMagicViewLoading, loading_get_view(nfc_magic->loading));

    // Text Input
    nfc_magic->text_input = text_input_alloc();
    view_dispatcher_add_view(
        nfc_magic->view_dispatcher,
        NfcMagicViewTextInput,
        text_input_get_view(nfc_magic->text_input));

    // Byte Input
    nfc_magic->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        nfc_magic->view_dispatcher,
        NfcMagicViewByteInput,
        byte_input_get_view(nfc_magic->byte_input));

    // Custom Widget
    nfc_magic->widget = widget_alloc();
    view_dispatcher_add_view(
        nfc_magic->view_dispatcher, NfcMagicViewWidget, widget_get_view(nfc_magic->widget));

    return nfc_magic;
}

void nfc_magic_free(NfcMagic* nfc_magic) {
    furi_assert(nfc_magic);

    // Nfc device
    free(nfc_magic->dev);
    nfc_device_free(nfc_magic->source_dev);

    // Submenu
    view_dispatcher_remove_view(nfc_magic->view_dispatcher, NfcMagicViewMenu);
    submenu_free(nfc_magic->submenu);

    // Popup
    view_dispatcher_remove_view(nfc_magic->view_dispatcher, NfcMagicViewPopup);
    popup_free(nfc_magic->popup);

    // Loading
    view_dispatcher_remove_view(nfc_magic->view_dispatcher, NfcMagicViewLoading);
    loading_free(nfc_magic->loading);

    // Text Input
    view_dispatcher_remove_view(nfc_magic->view_dispatcher, NfcMagicViewTextInput);
    text_input_free(nfc_magic->text_input);

    // Byte Input
    view_dispatcher_remove_view(nfc_magic->view_dispatcher, NfcMagicViewByteInput);
    byte_input_free(nfc_magic->byte_input);

    // Custom Widget
    view_dispatcher_remove_view(nfc_magic->view_dispatcher, NfcMagicViewWidget);
    widget_free(nfc_magic->widget);

    // Worker
    nfc_magic_worker_stop(nfc_magic->worker);
    nfc_magic_worker_free(nfc_magic->worker);

    // View Dispatcher
    view_dispatcher_free(nfc_magic->view_dispatcher);

    // Scene Manager
    scene_manager_free(nfc_magic->scene_manager);

    // GUI
    furi_record_close(RECORD_GUI);
    nfc_magic->gui = NULL;

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    nfc_magic->notifications = NULL;

    free(nfc_magic);
}

static const NotificationSequence nfc_magic_sequence_blink_start_cyan = {
    &message_blink_start_10,
    &message_blink_set_color_cyan,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence nfc_magic_sequence_blink_stop = {
    &message_blink_stop,
    NULL,
};

void nfc_magic_blink_start(NfcMagic* nfc_magic) {
    notification_message(nfc_magic->notifications, &nfc_magic_sequence_blink_start_cyan);
}

void nfc_magic_blink_stop(NfcMagic* nfc_magic) {
    notification_message(nfc_magic->notifications, &nfc_magic_sequence_blink_stop);
}

int32_t nfc_magic_app(void* p) {
    UNUSED(p);
    NfcMagic* nfc_magic = nfc_magic_alloc();

    scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneStart);

    view_dispatcher_run(nfc_magic->view_dispatcher);

    magic_deactivate();
    nfc_magic_free(nfc_magic);

    return 0;
}
