#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view_stack.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>
#include <gui/modules/loading.h>

#include <dialogs/dialogs.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "subbrute.h"
#include "subbrute_device.h"
#include "helpers/subbrute_worker.h"
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

    // Text store
    char text_store[SUBBRUTE_MAX_LEN_NAME];
    FuriString* file_path;

    // Views
    SubBruteMainView* view_main;
    SubBruteAttackView* view_attack;
    SubBruteView current_view;

    // Scene
    SceneManager* scene_manager;

    // SubBruteDevice
    SubBruteDevice* device;
    // SubBruteWorker
    SubBruteWorker* worker;
};

void subbrute_show_loading_popup(void* context, bool show);
void subbrute_text_input_callback(void* context);
void subbrute_popup_closed_callback(void* context);