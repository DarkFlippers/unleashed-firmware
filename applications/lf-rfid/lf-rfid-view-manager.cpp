#include "lf-rfid-view-manager.h"
#include "lf-rfid-event.h"
#include <callback-connector.h>

LfrfidAppViewManager::LfrfidAppViewManager() {
    event_queue = osMessageQueueNew(10, sizeof(LfrfidEvent), NULL);

    view_dispatcher = view_dispatcher_alloc();
    auto callback = cbc::obtain_connector(this, &LfrfidAppViewManager::previous_view_callback);

    // allocate views
    submenu = submenu_alloc();
    add_view(ViewType::Submenu, submenu_get_view(submenu));

    popup = popup_alloc();
    add_view(ViewType::Popup, popup_get_view(popup));

    tune = new LfRfidViewTune();
    add_view(ViewType::Tune, tune->get_view());

    gui = static_cast<Gui*>(furi_record_open("gui"));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    // set previous view callback for all views
    view_set_previous_callback(submenu_get_view(submenu), callback);
    view_set_previous_callback(popup_get_view(popup), callback);
    view_set_previous_callback(tune->get_view(), callback);
}

LfrfidAppViewManager::~LfrfidAppViewManager() {
    // remove views
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(LfrfidAppViewManager::ViewType::Submenu));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(LfrfidAppViewManager::ViewType::Popup));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(LfrfidAppViewManager::ViewType::Tune));

    // free view modules
    submenu_free(submenu);
    popup_free(popup);
    delete tune;

    // free dispatcher
    view_dispatcher_free(view_dispatcher);

    // free event queue
    osMessageQueueDelete(event_queue);
}

void LfrfidAppViewManager::switch_to(ViewType type) {
    view_dispatcher_switch_to_view(view_dispatcher, static_cast<uint32_t>(type));
}

Submenu* LfrfidAppViewManager::get_submenu() {
    return submenu;
}

Popup* LfrfidAppViewManager::get_popup() {
    return popup;
}

LfRfidViewTune* LfrfidAppViewManager::get_tune() {
    return tune;
}

void LfrfidAppViewManager::receive_event(LfrfidEvent* event) {
    if(osMessageQueueGet(event_queue, event, NULL, 100) != osOK) {
        event->type = LfrfidEvent::Type::Tick;
    }
}

void LfrfidAppViewManager::send_event(LfrfidEvent* event) {
    osStatus_t result = osMessageQueuePut(event_queue, event, 0, 0);
    furi_check(result == osOK);
}

uint32_t LfrfidAppViewManager::previous_view_callback(void* context) {
    if(event_queue != NULL) {
        LfrfidEvent event;
        event.type = LfrfidEvent::Type::Back;
        send_event(&event);
    }

    return VIEW_IGNORE;
}

void LfrfidAppViewManager::add_view(ViewType view_type, View* view) {
    view_dispatcher_add_view(view_dispatcher, static_cast<uint32_t>(view_type), view);
}