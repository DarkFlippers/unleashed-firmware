#pragma once
#include <gui/modules/button_menu.h>
#include <gui/modules/text_input.h>
#include <gui/view_stack.h>
#include <gui/modules/button_panel.h>
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>

#include "irda_app_event.h"
#include "view/irda_progress_view.h"

class IrdaAppViewManager {
public:
    enum class ViewType : uint8_t {
        DialogEx,
        TextInput,
        Submenu,
        ButtonMenu,
        UniversalRemote,
        Popup,
    };

    IrdaAppViewManager();
    ~IrdaAppViewManager();

    void switch_to(ViewType type);

    void receive_event(IrdaAppEvent* event);
    void send_event(IrdaAppEvent* event);
    void clear_events();

    DialogEx* get_dialog_ex();
    Submenu* get_submenu();
    Popup* get_popup();
    TextInput* get_text_input();
    ButtonMenu* get_button_menu();
    ButtonPanel* get_button_panel();
    ViewStack* get_universal_view_stack();
    IrdaProgressView* get_progress();
    Loading* get_loading();

    osMessageQueueId_t get_event_queue();

    uint32_t previous_view_callback(void* context);

private:
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    TextInput* text_input;
    DialogEx* dialog_ex;
    Submenu* submenu;
    Popup* popup;
    ButtonMenu* button_menu;
    ButtonPanel* button_panel;
    ViewStack* universal_view_stack;
    IrdaProgressView* progress_view;
    Loading* loading_view;

    osMessageQueueId_t event_queue;

    void add_view(ViewType view_type, View* view);
};
