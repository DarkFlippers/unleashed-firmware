#pragma once
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>
#include "ibutton_event.h"

class iButtonAppViewManager {
public:
    enum class Type : uint8_t {
        iButtonAppViewTextInput,
        iButtonAppViewByteInput,
        iButtonAppViewSubmenu,
        iButtonAppViewDialogEx,
        iButtonAppViewPopup,
        iButtonAppViewWidget,
    };

    osMessageQueueId_t event_queue;

    iButtonAppViewManager();
    ~iButtonAppViewManager();

    void switch_to(Type type);

    Submenu* get_submenu();
    Popup* get_popup();
    DialogEx* get_dialog_ex();
    TextInput* get_text_input();
    ByteInput* get_byte_input();
    Widget* get_widget();

    void receive_event(iButtonEvent* event);
    void send_event(iButtonEvent* event);

private:
    ViewDispatcher* view_dispatcher;
    DialogEx* dialog_ex;
    Submenu* submenu;
    TextInput* text_input;
    ByteInput* byte_input;
    Popup* popup;
    Widget* widget;
    Gui* gui;

    uint32_t previous_view_callback(void* context);
};
