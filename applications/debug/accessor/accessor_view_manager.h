#pragma once
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include "accessor_event.h"

class AccessorAppViewManager {
public:
    enum class ViewType : uint8_t {
        Submenu,
        Popup,
        Tune,
    };

    FuriMessageQueue* event_queue;

    AccessorAppViewManager();
    ~AccessorAppViewManager();

    void switch_to(ViewType type);

    void receive_event(AccessorEvent* event);
    void send_event(AccessorEvent* event);

    Submenu* get_submenu();
    Popup* get_popup();

private:
    ViewDispatcher* view_dispatcher;
    Gui* gui;

    uint32_t previous_view_callback(void* context);
    void add_view(ViewType view_type, View* view);

    // view elements
    Submenu* submenu;
    Popup* popup;
};
