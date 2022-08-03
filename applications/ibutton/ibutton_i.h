#pragma once

#include "ibutton.h"

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>

#include <one_wire/ibutton/ibutton_worker.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/widget.h>

#include "ibutton_custom_event.h"
#include "scenes/ibutton_scene.h"

#define IBUTTON_FILE_NAME_SIZE 100
#define IBUTTON_TEXT_STORE_SIZE 128

#define IBUTTON_APP_FOLDER ANY_PATH("ibutton")
#define IBUTTON_APP_EXTENSION ".ibtn"
#define IBUTTON_APP_FILE_TYPE "Flipper iButton key"

struct iButton {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;

    iButtonWorker* key_worker;
    iButtonKey* key;

    string_t file_path;
    char text_store[IBUTTON_TEXT_STORE_SIZE + 1];

    Submenu* submenu;
    ByteInput* byte_input;
    TextInput* text_input;
    Popup* popup;
    Widget* widget;
    DialogEx* dialog_ex;

    void* rpc_ctx;
};

typedef enum {
    iButtonViewSubmenu,
    iButtonViewByteInput,
    iButtonViewTextInput,
    iButtonViewPopup,
    iButtonViewWidget,
    iButtonViewDialogEx,
} iButtonView;

typedef enum {
    iButtonNotificationMessageError,
    iButtonNotificationMessageSuccess,
    iButtonNotificationMessageReadStart,
    iButtonNotificationMessageEmulateStart,
    iButtonNotificationMessageYellowBlink,
    iButtonNotificationMessageEmulateBlink,
    iButtonNotificationMessageRedOn,
    iButtonNotificationMessageRedOff,
    iButtonNotificationMessageGreenOn,
    iButtonNotificationMessageGreenOff,
    iButtonNotificationMessageBlinkStop,
} iButtonNotificationMessage;

bool ibutton_file_select(iButton* ibutton);
bool ibutton_load_key_data(iButton* ibutton, string_t key_path, bool show_dialog);
bool ibutton_save_key(iButton* ibutton, const char* key_name);
bool ibutton_delete_key(iButton* ibutton);
void ibutton_text_store_set(iButton* ibutton, const char* text, ...);
void ibutton_text_store_clear(iButton* ibutton);
void ibutton_switch_to_previous_scene_one_of(
    iButton* ibutton,
    const uint32_t* scene_ids,
    size_t scene_ids_size);
void ibutton_notification_message(iButton* ibutton, uint32_t message);
