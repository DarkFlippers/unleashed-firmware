#pragma once
#include "m-string.h"
#include <furi.h>
#include <furi_hal.h>

#include <generic_scene.hpp>
#include <scene_controller.hpp>
#include <view_controller.hpp>
#include <record_controller.hpp>
#include <text_store.h>

#include <view_modules/submenu_vm.h>
#include <view_modules/popup_vm.h>
#include <view_modules/dialog_ex_vm.h>
#include <view_modules/text_input_vm.h>
#include <view_modules/byte_input_vm.h>
#include "view/container_vm.h"

#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include "rpc/rpc_app.h"

#include <toolbox/protocols/protocol_dict.h>
#include <lfrfid/lfrfid_dict_file.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include <lfrfid/lfrfid_worker.h>

#define LFRFID_KEY_NAME_SIZE 22

class LfRfidApp {
public:
    enum class EventType : uint8_t {
        GENERIC_EVENT_ENUM_VALUES,
        Next,
        MenuSelected,
        Stay,
        Retry,
        Exit,
        ReadEventSenseStart,
        ReadEventSenseEnd,
        ReadEventSenseCardStart,
        ReadEventSenseCardEnd,
        ReadEventStartASK,
        ReadEventStartPSK,
        ReadEventDone,
        ReadEventOverrun,
        ReadEventError,
        WriteEventOK,
        WriteEventProtocolCannotBeWritten,
        WriteEventFobCannotBeWritten,
        WriteEventTooLongToWrite,
        RpcLoadFile,
        RpcSessionClose,
    };

    enum class SceneType : uint8_t {
        GENERIC_SCENE_ENUM_VALUES,
        Read,
        ReadSuccess,
        RetryConfirm,
        ExitConfirm,
        ReadKeyMenu,
        Write,
        WriteSuccess,
        Emulate,
        SaveName,
        SaveSuccess,
        SelectKey,
        SavedKeyMenu,
        SaveData,
        SaveType,
        SavedInfo,
        DeleteConfirm,
        DeleteSuccess,
        Rpc,
        ExtraActions,
        RawInfo,
        RawName,
        RawRead,
        RawSuccess,
    };

    class Event {
    public:
        union {
            int32_t signed_int;
        } payload;

        EventType type;
    };

    SceneController<GenericScene<LfRfidApp>, LfRfidApp> scene_controller;
    ViewController<LfRfidApp, SubmenuVM, PopupVM, DialogExVM, TextInputVM, ByteInputVM, ContainerVM>
        view_controller;

    ~LfRfidApp();
    LfRfidApp();

    RecordController<NotificationApp> notification;
    RecordController<Storage> storage;
    RecordController<DialogsApp> dialogs;

    TextStore text_store;

    string_t file_path;

    RpcAppSystem* rpc_ctx;

    void run(void* args);

    static const char* app_folder;
    static const char* app_sd_folder;
    static const char* app_extension;
    static const char* app_filetype;

    bool save_key();
    bool load_key_from_file_select(bool need_restore);
    bool delete_key();

    bool load_key_data(string_t path, bool show_dialog);
    bool save_key_data(string_t path);

    void make_app_folder();

    ProtocolDict* dict;
    LFRFIDWorker* lfworker;
    string_t file_name;
    ProtocolId protocol_id;
    LFRFIDWorkerReadType read_type;

    uint8_t* old_key_data;
    uint8_t* new_key_data;

    string_t raw_file_name;
};
