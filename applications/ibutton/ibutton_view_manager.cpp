#include "ibutton_view_manager.h"
#include "ibutton_event.h"
#include <callback-connector.h>

iButtonAppViewManager::iButtonAppViewManager() {
    event_queue = osMessageQueueNew(10, sizeof(iButtonEvent), NULL);

    view_dispatcher = view_dispatcher_alloc();
    auto callback = cbc::obtain_connector(this, &iButtonAppViewManager::previous_view_callback);

    dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewDialogEx),
        dialog_ex_get_view(dialog_ex));

    submenu = submenu_alloc();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewSubmenu),
        submenu_get_view(submenu));

    text_input = text_input_alloc();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewTextInput),
        text_input_get_view(text_input));

    byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewByteInput),
        byte_input_get_view(byte_input));

    popup = popup_alloc();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewPopup),
        popup_get_view(popup));

    widget = widget_alloc();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewWidget),
        widget_get_view(widget));

    gui = static_cast<Gui*>(furi_record_open("gui"));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    //TODO think about that method, seems unsafe and over-engineered
    view_set_previous_callback(dialog_ex_get_view(dialog_ex), callback);
    view_set_previous_callback(submenu_get_view(submenu), callback);
    view_set_previous_callback(text_input_get_view(text_input), callback);
    view_set_previous_callback(byte_input_get_view(byte_input), callback);
    view_set_previous_callback(popup_get_view(popup), callback);
    view_set_previous_callback(widget_get_view(widget), callback);
}

iButtonAppViewManager::~iButtonAppViewManager() {
    // remove views
    view_dispatcher_remove_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewDialogEx));
    view_dispatcher_remove_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewSubmenu));
    view_dispatcher_remove_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewTextInput));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewPopup));
    view_dispatcher_remove_view(
        view_dispatcher,
        static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewByteInput));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(iButtonAppViewManager::Type::iButtonAppViewWidget));

    // free view modules
    popup_free(popup);
    text_input_free(text_input);
    byte_input_free(byte_input);
    submenu_free(submenu);
    dialog_ex_free(dialog_ex);
    widget_free(widget);

    // free dispatcher
    view_dispatcher_free(view_dispatcher);

    // free event queue
    osMessageQueueDelete(event_queue);
}

void iButtonAppViewManager::switch_to(Type type) {
    view_dispatcher_switch_to_view(view_dispatcher, static_cast<uint32_t>(type));
}

Submenu* iButtonAppViewManager::get_submenu() {
    return submenu;
}

Popup* iButtonAppViewManager::get_popup() {
    return popup;
}

DialogEx* iButtonAppViewManager::get_dialog_ex() {
    return dialog_ex;
}

TextInput* iButtonAppViewManager::get_text_input() {
    return text_input;
}

ByteInput* iButtonAppViewManager::get_byte_input() {
    return byte_input;
}

Widget* iButtonAppViewManager::get_widget() {
    return widget;
}

void iButtonAppViewManager::receive_event(iButtonEvent* event) {
    if(osMessageQueueGet(event_queue, event, NULL, 100) != osOK) {
        event->type = iButtonEvent::Type::EventTypeTick;
    }
}

void iButtonAppViewManager::send_event(iButtonEvent* event) {
    osStatus_t result = osMessageQueuePut(event_queue, event, 0, 0);
    furi_check(result == osOK);
}

uint32_t iButtonAppViewManager::previous_view_callback(void* context) {
    if(event_queue != NULL) {
        iButtonEvent event;
        event.type = iButtonEvent::Type::EventTypeBack;
        send_event(&event);
    }

    return VIEW_IGNORE;
}