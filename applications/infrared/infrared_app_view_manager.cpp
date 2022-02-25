#include <gui/modules/button_menu.h>
#include <gui/view_stack.h>
#include <gui/modules/loading.h>
#include <gui/modules/button_panel.h>
#include <gui/modules/dialog_ex.h>
#include <furi.h>
#include <callback-connector.h>

#include "infrared/infrared_app_view_manager.h"
#include "infrared/view/infrared_progress_view.h"
#include "infrared_app.h"
#include "infrared/infrared_app_event.h"

InfraredAppViewManager::InfraredAppViewManager() {
    event_queue = osMessageQueueNew(10, sizeof(InfraredAppEvent), NULL);

    view_dispatcher = view_dispatcher_alloc();
    auto callback = cbc::obtain_connector(this, &InfraredAppViewManager::previous_view_callback);

    gui = static_cast<Gui*>(furi_record_open("gui"));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    button_menu = button_menu_alloc();
    submenu = submenu_alloc();
    popup = popup_alloc();
    dialog_ex = dialog_ex_alloc();
    text_input = text_input_alloc();
    button_panel = button_panel_alloc();
    progress_view = infrared_progress_view_alloc();
    loading_view = loading_alloc();
    universal_view_stack = view_stack_alloc();
    view_stack_add_view(universal_view_stack, button_panel_get_view(button_panel));
    view_set_orientation(view_stack_get_view(universal_view_stack), ViewOrientationVertical);

    add_view(ViewId::UniversalRemote, view_stack_get_view(universal_view_stack));
    add_view(ViewId::ButtonMenu, button_menu_get_view(button_menu));
    add_view(ViewId::Submenu, submenu_get_view(submenu));
    add_view(ViewId::Popup, popup_get_view(popup));
    add_view(ViewId::DialogEx, dialog_ex_get_view(dialog_ex));
    add_view(ViewId::TextInput, text_input_get_view(text_input));

    view_set_previous_callback(view_stack_get_view(universal_view_stack), callback);
    view_set_previous_callback(button_menu_get_view(button_menu), callback);
    view_set_previous_callback(submenu_get_view(submenu), callback);
    view_set_previous_callback(popup_get_view(popup), callback);
    view_set_previous_callback(dialog_ex_get_view(dialog_ex), callback);
    view_set_previous_callback(text_input_get_view(text_input), callback);
}

InfraredAppViewManager::~InfraredAppViewManager() {
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(InfraredAppViewManager::ViewId::UniversalRemote));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(InfraredAppViewManager::ViewId::ButtonMenu));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(InfraredAppViewManager::ViewId::TextInput));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(InfraredAppViewManager::ViewId::DialogEx));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(InfraredAppViewManager::ViewId::Submenu));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(InfraredAppViewManager::ViewId::Popup));

    view_stack_remove_view(universal_view_stack, button_panel_get_view(button_panel));
    view_stack_free(universal_view_stack);
    button_panel_free(button_panel);
    submenu_free(submenu);
    popup_free(popup);
    button_menu_free(button_menu);
    dialog_ex_free(dialog_ex);
    text_input_free(text_input);
    infrared_progress_view_free(progress_view);
    loading_free(loading_view);

    view_dispatcher_free(view_dispatcher);
    furi_record_close("gui");
    osMessageQueueDelete(event_queue);
}

void InfraredAppViewManager::switch_to(ViewId type) {
    view_dispatcher_switch_to_view(view_dispatcher, static_cast<uint32_t>(type));
}

TextInput* InfraredAppViewManager::get_text_input() {
    return text_input;
}

DialogEx* InfraredAppViewManager::get_dialog_ex() {
    return dialog_ex;
}

Submenu* InfraredAppViewManager::get_submenu() {
    return submenu;
}

Popup* InfraredAppViewManager::get_popup() {
    return popup;
}

ButtonMenu* InfraredAppViewManager::get_button_menu() {
    return button_menu;
}

ButtonPanel* InfraredAppViewManager::get_button_panel() {
    return button_panel;
}

InfraredProgressView* InfraredAppViewManager::get_progress() {
    return progress_view;
}

Loading* InfraredAppViewManager::get_loading() {
    return loading_view;
}

ViewStack* InfraredAppViewManager::get_universal_view_stack() {
    return universal_view_stack;
}

osMessageQueueId_t InfraredAppViewManager::get_event_queue() {
    return event_queue;
}

void InfraredAppViewManager::clear_events() {
    InfraredAppEvent event;
    while(osMessageQueueGet(event_queue, &event, NULL, 0) == osOK)
        ;
}

void InfraredAppViewManager::receive_event(InfraredAppEvent* event) {
    if(osMessageQueueGet(event_queue, event, NULL, 100) != osOK) {
        event->type = InfraredAppEvent::Type::Tick;
    }
}

void InfraredAppViewManager::send_event(InfraredAppEvent* event) {
    uint32_t timeout = 0;
    /* Rapid button hammering on signal send scenes causes queue overflow - ignore it,
     * but try to keep button release event - it switches off INFRARED DMA sending. */
    if(event->type == InfraredAppEvent::Type::MenuSelectedRelease) {
        timeout = 200;
    }
    if((event->type == InfraredAppEvent::Type::DialogExSelected) &&
       (event->payload.dialog_ex_result == DialogExReleaseCenter)) {
        timeout = 200;
    }

    osMessageQueuePut(event_queue, event, 0, timeout);
}

uint32_t InfraredAppViewManager::previous_view_callback(void* context) {
    if(event_queue != NULL) {
        InfraredAppEvent event;
        event.type = InfraredAppEvent::Type::Back;
        send_event(&event);
    }

    return VIEW_IGNORE;
}

void InfraredAppViewManager::add_view(ViewId view_type, View* view) {
    view_dispatcher_add_view(view_dispatcher, static_cast<uint32_t>(view_type), view);
}
