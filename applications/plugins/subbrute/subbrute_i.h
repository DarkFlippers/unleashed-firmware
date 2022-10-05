#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>

#include "lib/toolbox/path.h"
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <lib/toolbox/stream/stream.h>
#include <stream_buffer.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view_stack.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>
#include <gui/modules/loading.h>

#include <dialogs/dialogs.h>

#include <lib/subghz/protocols/base.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/environment.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "subbrute_device.h"
#include "helpers/subbrute_worker.h"
#include "subbrute.h"
#include "scenes/subbrute_scene.h"
#include "views/subbrute_attack_view.h"
#include "views/subbrute_main_view.h"

typedef enum {
    SubBruteViewNone,
    SubBruteViewMain,
    SubBruteViewAttack,
    SubBruteViewTextInput,
    SubBruteViewDialogEx,
    SubBruteViewPopup,
    SubBruteViewWidget,
    SubBruteViewStack,
} SubBruteView;

struct SubBruteState {
    // GUI elements
    NotificationApp* notifications;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    ViewStack* view_stack;
    TextInput* text_input;
    Popup* popup;
    Widget* widget;
    DialogsApp* dialogs;
    Loading* loading;

    // Views
    SubBruteMainView* view_main;
    SubBruteAttackView* view_attack;
    SubBruteView current_view;

    // Scene
    SceneManager* scene_manager;

    SubBruteDevice* device;
    SubBruteWorker* worker;

    //Menu stuff
    // TODO: Do we need it?
    uint8_t menu_index;
};

void subbrute_show_loading_popup(void* context, bool show);
void subbrute_text_input_callback(void* context);
void subbrute_popup_closed_callback(void* context);
const char* subbrute_get_menu_name(uint8_t index);
const char* subbrute_get_small_menu_name(uint8_t index);