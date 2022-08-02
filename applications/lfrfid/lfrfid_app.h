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

#include "helpers/rfid_worker.h"
#include "rpc/rpc_app.h"

class LfRfidApp {
public:
    enum class EventType : uint8_t {
        GENERIC_EVENT_ENUM_VALUES,
        Next,
        MenuSelected,
        Stay,
        Retry,
        Exit,
        EmulateStart,
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
    };

    class Event {
    public:
        union {
            int32_t menu_index;
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

    RfidWorker worker;

    TextStore text_store;

    string_t file_path;

    RpcAppSystem* rpc_ctx;

    void run(void* args);

    static const char* app_folder;
    static const char* app_extension;
    static const char* app_filetype;

    bool save_key(RfidKey* key);
    bool load_key_from_file_select(bool need_restore);
    bool delete_key(RfidKey* key);

    bool load_key_data(string_t path, RfidKey* key, bool show_dialog);
    bool save_key_data(string_t path, RfidKey* key);

    void make_app_folder();
    //bool rpc_command_callback(RpcAppSystemEvent event, const char* arg, void* context);
};
