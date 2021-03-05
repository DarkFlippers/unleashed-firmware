#pragma once
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include "subghz-event.h"
#include "view/subghz-view-spectrum-settings.h"

class SubghzAppViewManager {
public:
    enum class ViewType : uint8_t {
        Submenu,
        SpectrumSettings,
    };

    osMessageQueueId_t event_queue;

    SubghzAppViewManager();
    ~SubghzAppViewManager();

    void switch_to(ViewType type);

    void receive_event(SubghzEvent* event);
    void send_event(SubghzEvent* event);

    Submenu* get_submenu();
    SubghzViewSpectrumSettings* get_spectrum_settings();

private:
    ViewDispatcher* view_dispatcher;
    Gui* gui;

    uint32_t previous_view_callback(void* context);

    // view elements
    Submenu* submenu;
    SubghzViewSpectrumSettings* spectrum_settings;
};