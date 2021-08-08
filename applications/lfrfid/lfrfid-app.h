#pragma once
#include <furi.h>
#include <furi-hal.h>

#include <generic-scene.hpp>
#include <scene-controller.hpp>
#include <view-controller.hpp>
#include <record-controller.hpp>
#include <text-store.h>

#include <view-modules/submenu-vm.h>
#include <view-modules/popup-vm.h>
#include <view-modules/dialog-ex-vm.h>
#include <view-modules/text-input-vm.h>
#include <view-modules/byte-input-vm.h>
#include "view/container-vm.h"

#include <notification/notification-messages.h>

#include "helpers/rfid-worker.h"

class LfRfidApp {
public:
    enum class EventType : uint8_t {
        GENERIC_EVENT_ENUM_VALUES,
        Next,
        MenuSelected,
    };

    enum class SceneType : uint8_t {
        GENERIC_SCENE_ENUM_VALUES,
        Read,
        ReadSuccess,
        ReadedMenu,
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

    RfidWorker worker;

    TextStore text_store;

    void run(void* args);

    static const char* app_folder;
    static const char* app_extension;

    bool save_key(RfidKey* key);
    bool load_key_from_file_select(bool need_restore);
    bool delete_key(RfidKey* key);

    bool load_key_data(const char* path, RfidKey* key);
    bool save_key_data(const char* path, RfidKey* key);

    void make_app_folder();
};