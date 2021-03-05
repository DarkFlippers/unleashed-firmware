#include "subghz-view-manager.h"
#include "subghz-event.h"
#include <callback-connector.h>

SubghzAppViewManager::SubghzAppViewManager() {
    event_queue = osMessageQueueNew(10, sizeof(SubghzEvent), NULL);

    view_dispatcher = view_dispatcher_alloc();
    auto callback = cbc::obtain_connector(this, &SubghzAppViewManager::previous_view_callback);

    // allocate views
    submenu = submenu_alloc();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(SubghzAppViewManager::ViewType::Submenu),
        submenu_get_view(submenu));

    spectrum_settings = new SubghzViewSpectrumSettings();
    view_dispatcher_add_view(
        view_dispatcher,
        static_cast<uint32_t>(SubghzAppViewManager::ViewType::SpectrumSettings),
        spectrum_settings->get_view());

    gui = static_cast<Gui*>(furi_record_open("gui"));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    // set previous view callback for all views
    view_set_previous_callback(submenu_get_view(submenu), callback);
    view_set_previous_callback(spectrum_settings->get_view(), callback);
}

SubghzAppViewManager::~SubghzAppViewManager() {
    // remove views
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(SubghzAppViewManager::ViewType::Submenu));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(SubghzAppViewManager::ViewType::SpectrumSettings));

    // free view modules
    submenu_free(submenu);
    free(spectrum_settings);

    // free dispatcher
    view_dispatcher_free(view_dispatcher);

    // free event queue
    osMessageQueueDelete(event_queue);
}

void SubghzAppViewManager::switch_to(ViewType type) {
    view_dispatcher_switch_to_view(view_dispatcher, static_cast<uint32_t>(type));
}

Submenu* SubghzAppViewManager::get_submenu() {
    return submenu;
}

SubghzViewSpectrumSettings* SubghzAppViewManager::get_spectrum_settings() {
    return spectrum_settings;
}

void SubghzAppViewManager::receive_event(SubghzEvent* event) {
    if(osMessageQueueGet(event_queue, event, NULL, 100) != osOK) {
        event->type = SubghzEvent::Type::Tick;
    }
}

void SubghzAppViewManager::send_event(SubghzEvent* event) {
    osStatus_t result = osMessageQueuePut(event_queue, event, 0, 0);
    furi_check(result == osOK);
}

uint32_t SubghzAppViewManager::previous_view_callback(void* context) {
    if(event_queue != NULL) {
        SubghzEvent event;
        event.type = SubghzEvent::Type::Back;
        send_event(&event);
    }

    return VIEW_IGNORE;
}