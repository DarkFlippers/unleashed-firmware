#pragma once

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_stack.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/text_input.h>
#include <gui/modules/button_menu.h>
#include <gui/modules/button_panel.h>

#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include <notification/notification_messages.h>

#include <infrared_worker.h>

#include "infrared.h"
#include "infrared_remote.h"
#include "infrared_brute_force.h"
#include "infrared_custom_event.h"

#include "scenes/infrared_scene.h"
#include "views/infrared_progress_view.h"
#include "views/infrared_debug_view.h"

#include "rpc/rpc_app.h"

#define INFRARED_FILE_NAME_SIZE 100
#define INFRARED_TEXT_STORE_NUM 2
#define INFRARED_TEXT_STORE_SIZE 128

#define INFRARED_MAX_BUTTON_NAME_LENGTH 22
#define INFRARED_MAX_REMOTE_NAME_LENGTH 22

#define INFRARED_APP_FOLDER ANY_PATH("infrared")
#define INFRARED_APP_EXTENSION ".ir"

#define INFRARED_DEFAULT_REMOTE_NAME "Remote"
#define INFRARED_LOG_TAG "InfraredApp"

typedef enum {
    InfraredButtonIndexNone = -1,
} InfraredButtonIndex;

typedef enum {
    InfraredEditTargetNone,
    InfraredEditTargetRemote,
    InfraredEditTargetButton,
} InfraredEditTarget;

typedef enum {
    InfraredEditModeNone,
    InfraredEditModeRename,
    InfraredEditModeDelete,
} InfraredEditMode;

typedef struct {
    bool is_learning_new_remote;
    bool is_debug_enabled;
    bool is_transmitting;
    InfraredEditTarget edit_target : 8;
    InfraredEditMode edit_mode : 8;
    int32_t current_button_index;
} InfraredAppState;

struct Infrared {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;
    InfraredWorker* worker;
    InfraredRemote* remote;
    InfraredSignal* received_signal;
    InfraredBruteForce* brute_force;

    Submenu* submenu;
    TextInput* text_input;
    DialogEx* dialog_ex;
    ButtonMenu* button_menu;
    Popup* popup;

    ViewStack* view_stack;
    InfraredDebugView* debug_view;

    ButtonPanel* button_panel;
    Loading* loading;
    InfraredProgressView* progress;

    string_t file_path;
    char text_store[INFRARED_TEXT_STORE_NUM][INFRARED_TEXT_STORE_SIZE + 1];
    InfraredAppState app_state;

    void* rpc_ctx;
};

typedef enum {
    InfraredViewSubmenu,
    InfraredViewTextInput,
    InfraredViewDialogEx,
    InfraredViewButtonMenu,
    InfraredViewPopup,
    InfraredViewStack,
    InfraredViewDebugView,
} InfraredView;

typedef enum {
    InfraredNotificationMessageSuccess,
    InfraredNotificationMessageGreenOn,
    InfraredNotificationMessageGreenOff,
    InfraredNotificationMessageYellowOn,
    InfraredNotificationMessageYellowOff,
    InfraredNotificationMessageBlinkStartRead,
    InfraredNotificationMessageBlinkStartSend,
    InfraredNotificationMessageBlinkStop,
} InfraredNotificationMessage;

bool infrared_add_remote_with_button(Infrared* infrared, const char* name, InfraredSignal* signal);
bool infrared_rename_current_remote(Infrared* infrared, const char* name);
void infrared_tx_start_signal(Infrared* infrared, InfraredSignal* signal);
void infrared_tx_start_button_index(Infrared* infrared, size_t button_index);
void infrared_tx_start_received(Infrared* infrared);
void infrared_tx_stop(Infrared* infrared);
void infrared_text_store_set(Infrared* infrared, uint32_t bank, const char* text, ...);
void infrared_text_store_clear(Infrared* infrared, uint32_t bank);
void infrared_play_notification_message(Infrared* infrared, uint32_t message);
void infrared_show_loading_popup(Infrared* infrared, bool show);

void infrared_signal_received_callback(void* context, InfraredWorkerSignal* received_signal);
void infrared_text_input_callback(void* context);
void infrared_popup_closed_callback(void* context);
