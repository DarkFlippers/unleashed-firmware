#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <assets_icons.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <cli/cli.h>
#include <notification/notification_messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/widget.h>

#include <lfrfid/views/lfrfid_view_read.h>

#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

#include <rpc/rpc_app.h>

#include <toolbox/protocols/protocol_dict.h>
#include <toolbox/path.h>
#include <lfrfid/lfrfid_dict_file.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include <lfrfid/lfrfid_worker.h>

#include <lfrfid/scenes/lfrfid_scene.h>

#define LFRFID_KEY_NAME_SIZE   22
#define LFRFID_TEXT_STORE_SIZE 40

#define LFRFID_APP_FOLDER                    EXT_PATH("lfrfid")
#define LFRFID_SD_FOLDER                     EXT_PATH("lfrfid")
#define LFRFID_APP_FILENAME_PREFIX           "RFID"
#define LFRFID_APP_FILENAME_EXTENSION        ".rfid"
#define LFRFID_APP_SHADOW_FILENAME_EXTENSION ".shd"

#define LFRFID_APP_RAW_ASK_EXTENSION ".ask.raw"
#define LFRFID_APP_RAW_PSK_EXTENSION ".psk.raw"

enum LfRfidCustomEvent {
    LfRfidEventNext = 100,
    LfRfidEventExit,
    LfRfidEventPopupClosed,
    LfRfidEventReadSenseStart,
    LfRfidEventReadSenseEnd,
    LfRfidEventReadSenseCardStart,
    LfRfidEventReadSenseCardEnd,
    LfRfidEventReadStartASK,
    LfRfidEventReadStartPSK,
    LfRfidEventReadDone,
    LfRfidEventReadOverrun,
    LfRfidEventReadError,
    LfRfidEventWriteOK,
    LfRfidEventWriteProtocolCannotBeWritten,
    LfRfidEventWriteFobCannotBeWritten,
    LfRfidEventWriteTooLongToWrite,
    LfRfidEventRpcLoadFile,
    LfRfidEventRpcSessionClose,
};

typedef enum {
    LfRfidRpcStateIdle,
    LfRfidRpcStateEmulating,
} LfRfidRpcState;

typedef struct LfRfid LfRfid;

struct LfRfid {
    LFRFIDWorker* lfworker;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    NotificationApp* notifications;
    SceneManager* scene_manager;
    Storage* storage;
    DialogsApp* dialogs;
    Widget* widget;

    char text_store[LFRFID_TEXT_STORE_SIZE + 1];
    FuriString* file_path;
    FuriString* file_name;
    FuriString* raw_file_name;

    ProtocolDict* dict;
    ProtocolId protocol_id;
    ProtocolId protocol_id_next;
    LFRFIDWorkerReadType read_type;

    uint8_t* old_key_data;
    uint8_t* new_key_data;

    RpcAppSystem* rpc_ctx;
    LfRfidRpcState rpc_state;

    // Common Views
    Submenu* submenu;
    DialogEx* dialog_ex;
    Popup* popup;
    TextInput* text_input;
    ByteInput* byte_input;

    // Custom views
    LfRfidReadView* read_view;
};

typedef enum {
    LfRfidViewSubmenu,
    LfRfidViewDialogEx,
    LfRfidViewPopup,
    LfRfidViewWidget,
    LfRfidViewTextInput,
    LfRfidViewByteInput,
    LfRfidViewRead,
} LfRfidView;

typedef enum {
    LfRfidMenuIndexRead,
    LfRfidMenuIndexSaved,
    LfRfidMenuIndexAddManually,
    LfRfidMenuIndexExtraActions,
} LfRfidMenuIndex;

bool lfrfid_save_key(LfRfid* app);

bool lfrfid_load_key_from_file_select(LfRfid* app);

bool lfrfid_delete_key(LfRfid* app);

bool lfrfid_load_key_data(LfRfid* app, FuriString* path, bool show_dialog);

bool lfrfid_save_key_data(LfRfid* app, FuriString* path);

void lfrfid_make_app_folder(LfRfid* app);

void lfrfid_text_store_set(LfRfid* app, const char* text, ...);

void lfrfid_text_store_clear(LfRfid* app);

void lfrfid_popup_timeout_callback(void* context);

void lfrfid_widget_callback(GuiButtonType result, InputType type, void* context);

void lfrfid_text_input_callback(void* context);
