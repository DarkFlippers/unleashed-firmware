#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <m-string.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view_stack.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>
#include <gui/modules/loading.h>

#include <dialogs/dialogs.h>

#include "subbrute.h"
#include "subbrute_i.h"
#include "subbrute_custom_event.h"

#define TAG "SubBruteApp"

static const char* subbrute_menu_names[] = {
    [SubBruteAttackNone] = "None",
    [SubBruteAttackCAME12bit307] = "CAME 12bit 307mhz",
    [SubBruteAttackCAME12bit433] = "CAME 12bit 433mhz",
    [SubBruteAttackCAME12bit868] = "CAME 12bit 868mhz",
    [SubBruteAttackChamberlain9bit315] = "Chamberlain 9bit 315mhz",
    [SubBruteAttackChamberlain9bit390] = "Chamberlain 9bit 390mhz",
    [SubBruteAttackLinear10bit300] = "Linear 10bit 300mhz",
    [SubBruteAttackLinear10bit310] = "Linear 10bit 310mhz",
    [SubBruteAttackNICE12bit433] = "NICE 12bit 433mhz",
    [SubBruteAttackNICE12bit868] = "NICE 12bit 868mhz",
    [SubBruteAttackLoadFile] = "BF existing dump",
    [SubBruteAttackTotalCount] = "Total Count",
};

bool subbrute_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SubBruteState* instance = context;
    return scene_manager_handle_custom_event(instance->scene_manager, event);
}

bool subbrute_back_event_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    return scene_manager_handle_back_event(instance->scene_manager);
}

void subbrute_tick_event_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    scene_manager_handle_tick_event(instance->scene_manager);
}

SubBruteState* subbrute_alloc() {
    SubBruteState* instance = malloc(sizeof(SubBruteState));

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

    // TextInput
    instance->text_input = text_input_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, SubBruteViewTextInput, text_input_get_view(instance->text_input));
    
    // Custom Widget
    instance->widget = widget_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, SubBruteViewWidget, widget_get_view(instance->widget));

    // Popup
    instance->popup = popup_alloc();
    view_dispatcher_add_view(instance->view_dispatcher, SubBruteViewPopup, popup_get_view(instance->popup));

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

    // Loading
    instance->loading = loading_alloc();
    //instance->flipper_format = flipper_format_string_alloc();
    //instance->environment = subghz_environment_alloc();

    return instance;
}

void subbrute_free(SubBruteState* instance) {
    furi_assert(instance);

    // Notifications
    notification_message(instance->notifications, &sequence_blink_stop);
    furi_record_close(RECORD_NOTIFICATION);
    instance->notifications = NULL;

    // Loading
    loading_free(instance->loading);

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

    // SubBruteDevice
    subbrute_device_free(instance->device);

    // The rest
    free(instance);
}

void subbrute_show_loading_popup(void* context, bool show) {
    TaskHandle_t timer_task = xTaskGetHandle(configTIMER_SERVICE_TASK_NAME);
    SubBruteState* instance = context;
    ViewStack* view_stack = instance->view_stack;
    Loading* loading = instance->loading;

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

void subbrute_text_input_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    view_dispatcher_send_custom_event(
        instance->view_dispatcher, SubBruteCustomEventTypeTextEditResult);
}

void subbrute_popup_closed_callback(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;
    view_dispatcher_send_custom_event(
        instance->view_dispatcher, SubBruteCustomEventTypePopupClosed);
}

const char* subbrute_get_menu_name(SubBruteAttacks index) {
    furi_assert(index < SubBruteAttackTotalCount - 1);

    return subbrute_menu_names[index];
}

// ENTRYPOINT
int32_t subbrute_app(void* p) {
    UNUSED(p);

    SubBruteState* instance = subbrute_alloc();
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Starting subbrute_alloc done");
#endif
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "scene_manager_next_scene set");
#endif
    furi_hal_power_suppress_charge_enter();
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "view_dispatcher_run");
#endif
    view_dispatcher_run(instance->view_dispatcher);
    furi_hal_power_suppress_charge_exit();

    subbrute_free(instance);

    return 0;
}