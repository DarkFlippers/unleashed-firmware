#pragma once
#include "gui/modules/button_menu.h"
#include "gui/modules/text_input.h"
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include "irda-app.hpp"
#include "view/irda-app-brut-view.h"
#include "gui/modules/button_panel.h"

class IrdaAppViewManager {
public:
    enum class ViewType : uint8_t {
        DialogEx,
        TextInput,
        Submenu,
        ButtonMenu,
        ButtonPanel,
        Popup,
    };

    IrdaAppViewManager();
    ~IrdaAppViewManager();

    void switch_to(ViewType type);

    void receive_event(IrdaAppEvent* event);
    void send_event(IrdaAppEvent* event);

    DialogEx* get_dialog_ex();
    Submenu* get_submenu();
    Popup* get_popup();
    TextInput* get_text_input();
    ButtonMenu* get_button_menu();
    ButtonPanel* get_button_panel();
    IrdaAppPopupBrut* get_popup_brut();

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
    IrdaAppPopupBrut* popup_brut;

    osMessageQueueId_t event_queue;

    void add_view(ViewType view_type, View* view);
};

