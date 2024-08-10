#include "accessor_view_manager.h"
#include "accessor_event.h"
#include "callback_connector.h"

AccessorAppViewManager::AccessorAppViewManager() {
    event_queue = furi_message_queue_alloc(10, sizeof(AccessorEvent));

    view_holder = view_holder_alloc();
    auto callback =
        cbc::obtain_connector(this, &AccessorAppViewManager::view_holder_back_callback);

    // allocate views
    submenu = submenu_alloc();
    popup = popup_alloc();

    // set back callback
    view_holder_set_back_callback(view_holder, callback, NULL);

    gui = static_cast<Gui*>(furi_record_open(RECORD_GUI));
    view_holder_attach_to_gui(view_holder, gui);
}

AccessorAppViewManager::~AccessorAppViewManager() {
    // remove current view
    view_holder_set_view(view_holder, NULL);
    // free view modules
    furi_record_close(RECORD_GUI);
    submenu_free(submenu);
    popup_free(popup);
    // free view holder
    view_holder_free(view_holder);
    // free event queue
    furi_message_queue_free(event_queue);
}

void AccessorAppViewManager::switch_to(ViewType type) {
    View* view;

    switch(type) {
    case ViewType::Submenu:
        view = submenu_get_view(submenu);
        break;
    case ViewType::Popup:
        view = popup_get_view(popup);
        break;
    default:
        furi_crash();
    }

    view_holder_set_view(view_holder, view);
}

Submenu* AccessorAppViewManager::get_submenu() {
    return submenu;
}

Popup* AccessorAppViewManager::get_popup() {
    return popup;
}

void AccessorAppViewManager::receive_event(AccessorEvent* event) {
    if(furi_message_queue_get(event_queue, event, 100) != FuriStatusOk) {
        event->type = AccessorEvent::Type::Tick;
    }
}

void AccessorAppViewManager::send_event(AccessorEvent* event) {
    FuriStatus result = furi_message_queue_put(event_queue, event, 0);
    furi_check(result == FuriStatusOk);
}

void AccessorAppViewManager::view_holder_back_callback(void*) {
    if(event_queue != NULL) {
        AccessorEvent event;
        event.type = AccessorEvent::Type::Back;
        send_event(&event);
    }
}
