#pragma once

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

#include "helpers/hid_worker.h"

class HIDApp {
public:
    enum class EventType : uint8_t {
        GENERIC_EVENT_ENUM_VALUES,
        Next,
        MenuSelected,
        Stay,
        Retry,
    };

    enum class SceneType : uint8_t {
        GENERIC_SCENE_ENUM_VALUES,
        Read,
        ReadSuccess,
    };

    class Event {
    public:
        union {
            int32_t menu_index;
        } payload;

        EventType type;
    };

    HIDApp();
    ~HIDApp();

    void run(void* args);

    // private:
    SceneController<GenericScene<HIDApp>, HIDApp> scene_controller;
    ViewController<HIDApp, SubmenuVM, PopupVM, DialogExVM, TextInputVM, ByteInputVM, ContainerVM>
        view_controller;
    RecordController<NotificationApp> notification;
    RecordController<Storage> storage;
    RecordController<DialogsApp> dialogs;
    TextStore text_store;

    HIDWorker worker;
};