#pragma once

#include "ibutton.h"

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

#include <ibutton/ibutton_worker.h>
#include <ibutton/ibutton_protocols.h>

#include <rpc/rpc_app.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/widget.h>
#include <gui/modules/loading.h>

#include <assets_icons.h>

#include "ibutton_custom_event.h"
#include "scenes/ibutton_scene.h"

#define IBUTTON_APP_FOLDER             EXT_PATH("ibutton")
#define IBUTTON_APP_FILENAME_PREFIX    "iBtn"
#define IBUTTON_APP_FILENAME_EXTENSION ".ibtn"

#define IBUTTON_KEY_NAME_SIZE 23

typedef enum {
    iButtonWriteModeInvalid,
    iButtonWriteModeId,
    iButtonWriteModeCopy,
} iButtonWriteMode;

struct iButton {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;
    RpcAppSystem* rpc;

    iButtonKey* key;
    iButtonWorker* worker;
    iButtonProtocols* protocols;
    iButtonWriteMode write_mode;

    FuriString* file_path;
    char key_name[IBUTTON_KEY_NAME_SIZE];

    Submenu* submenu;
    ByteInput* byte_input;
    TextInput* text_input;
    Popup* popup;
    Widget* widget;
    Loading* loading;
};

typedef enum {
    iButtonViewSubmenu,
    iButtonViewByteInput,
    iButtonViewTextInput,
    iButtonViewPopup,
    iButtonViewWidget,
    iButtonViewLoading,
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

bool ibutton_select_and_load_key(iButton* ibutton);
bool ibutton_load_key(iButton* ibutton, bool show_error);
bool ibutton_save_key(iButton* ibutton);
bool ibutton_delete_key(iButton* ibutton);
void ibutton_reset_key(iButton* ibutton);
void ibutton_notification_message(iButton* ibutton, uint32_t message);

void ibutton_submenu_callback(void* context, uint32_t index);
void ibutton_widget_callback(GuiButtonType result, InputType type, void* context);
