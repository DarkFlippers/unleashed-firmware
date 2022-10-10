#include "subbrute_i.h"
#include "subbrute_custom_event.h"
#include "scenes/subbrute_scene.h"

#define TAG "SubBruteApp"

static bool subbrute_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SubBruteState* instance = context;
    return scene_manager_handle_custom_event(instance->scene_manager, event);
}

static bool subbrute_back_event_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    return scene_manager_handle_back_event(instance->scene_manager);
}

static void subbrute_tick_event_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    scene_manager_handle_tick_event(instance->scene_manager);
}

SubBruteState* subbrute_alloc() {
    SubBruteState* instance = malloc(sizeof(SubBruteState));

    memset(instance->text_store, 0, sizeof(instance->text_store));
    instance->file_path = furi_string_alloc();

    instance->scene_manager = scene_manager_alloc(&subbrute_scene_handlers, instance);
    instance->view_dispatcher = view_dispatcher_alloc();

    instance->gui = furi_record_open(RECORD_GUI);

    view_dispatcher_enable_queue(instance->view_dispatcher);
    view_dispatcher_set_event_callback_context(instance->view_dispatcher, instance);
    view_dispatcher_set_custom_event_callback(
        instance->view_dispatcher, subbrute_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        instance->view_dispatcher, subbrute_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        instance->view_dispatcher, subbrute_tick_event_callback, 100);

    //Dialog
    instance->dialogs = furi_record_open(RECORD_DIALOGS);

    // Notifications
    instance->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Devices
    instance->device = subbrute_device_alloc();

    // SubBruteWorker
    instance->worker = subbrute_worker_alloc();

    // TextInput
    instance->text_input = text_input_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        SubBruteViewTextInput,
        text_input_get_view(instance->text_input));

    // Custom Widget
    instance->widget = widget_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, SubBruteViewWidget, widget_get_view(instance->widget));

    // Popup
    instance->popup = popup_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, SubBruteViewPopup, popup_get_view(instance->popup));

    // ViewStack
    instance->view_stack = view_stack_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, SubBruteViewStack, view_stack_get_view(instance->view_stack));

    // SubBruteMainView
    instance->view_main = subbrute_main_view_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        SubBruteViewMain,
        subbrute_main_view_get_view(instance->view_main));

    // SubBruteAttackView
    instance->view_attack = subbrute_attack_view_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        SubBruteViewAttack,
        subbrute_attack_view_get_view(instance->view_attack));

    //instance->flipper_format = flipper_format_string_alloc();
    //instance->environment = subghz_environment_alloc();

    return instance;
}

void subbrute_free(SubBruteState* instance) {
    furi_assert(instance);

    // SubBruteWorker
    subbrute_worker_stop(instance->worker);
    subbrute_worker_free(instance->worker);

    // SubBruteDevice
    subbrute_device_free(instance->device);

    // Notifications
    notification_message(instance->notifications, &sequence_blink_stop);
    furi_record_close(RECORD_NOTIFICATION);
    instance->notifications = NULL;

    // View Main
    view_dispatcher_remove_view(instance->view_dispatcher, SubBruteViewMain);
    subbrute_main_view_free(instance->view_main);

    // View Attack
    view_dispatcher_remove_view(instance->view_dispatcher, SubBruteViewAttack);
    subbrute_attack_view_free(instance->view_attack);

    // TextInput
    view_dispatcher_remove_view(instance->view_dispatcher, SubBruteViewTextInput);
    text_input_free(instance->text_input);

    // Custom Widget
    view_dispatcher_remove_view(instance->view_dispatcher, SubBruteViewWidget);
    widget_free(instance->widget);

    // Popup
    view_dispatcher_remove_view(instance->view_dispatcher, SubBruteViewPopup);
    popup_free(instance->popup);

    // ViewStack
    view_dispatcher_remove_view(instance->view_dispatcher, SubBruteViewStack);
    view_stack_free(instance->view_stack);

    //Dialog
    furi_record_close(RECORD_DIALOGS);
    instance->dialogs = NULL;

    // Scene manager
    scene_manager_free(instance->scene_manager);

    // View Dispatcher
    view_dispatcher_free(instance->view_dispatcher);

    // GUI
    furi_record_close(RECORD_GUI);
    instance->gui = NULL;

    furi_string_free(instance->file_path);

    // The rest
    free(instance);
}

void subbrute_text_input_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    view_dispatcher_send_custom_event(
        instance->view_dispatcher, SubBruteCustomEventTypeTextEditDone);
}

void subbrute_popup_closed_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    view_dispatcher_send_custom_event(
        instance->view_dispatcher, SubBruteCustomEventTypePopupClosed);
}

// ENTRYPOINT
int32_t subbrute_app(void* p) {
    UNUSED(p);

    SubBruteState* instance = subbrute_alloc();
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);

    furi_hal_power_suppress_charge_enter();
    notification_message(instance->notifications, &sequence_display_backlight_on);
    view_dispatcher_run(instance->view_dispatcher);
    furi_hal_power_suppress_charge_exit();
    subbrute_free(instance);

    return 0;
}