#pragma once
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include "lf-rfid-event.h"
#include "view/lf-rfid-view-tune.h"

class LfrfidAppViewManager {
public:
    enum class ViewType : uint8_t {
        Submenu,
        Popup,
        Tune,
    };

    osMessageQueueId_t event_queue;

    LfrfidAppViewManager();
    ~LfrfidAppViewManager();

    void switch_to(ViewType type);

    void receive_event(LfrfidEvent* event);
    void send_event(LfrfidEvent* event);

    Submenu* get_submenu();
    Popup* get_popup();
    LfRfidViewTune* get_tune();

private:
    ViewDispatcher* view_dispatcher;
    Gui* gui;

    uint32_t previous_view_callback(void* context);
    void add_view(ViewType view_type, View* view);

    // view elements
    Submenu* submenu;
    Popup* popup;
    LfRfidViewTune* tune;
};