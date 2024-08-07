#pragma once
#include <furi.h>
#include <gui/view_holder.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include "accessor_event.h"

class AccessorAppViewManager {
public:
    enum class ViewType : uint8_t {
        Submenu,
        Popup,
    };

    FuriMessageQueue* event_queue;

    AccessorAppViewManager(void);
    ~AccessorAppViewManager(void);

    void switch_to(ViewType type);

    void receive_event(AccessorEvent* event);
    void send_event(AccessorEvent* event);

    Submenu* get_submenu(void);
    Popup* get_popup(void);

private:
    Gui* gui;
    ViewHolder* view_holder;

    void view_holder_back_callback(void* context);

    // view elements
    Submenu* submenu;
    Popup* popup;
};
