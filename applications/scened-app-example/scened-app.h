#pragma once
#include <furi.h>
#include <api-hal.h>

#include <generic-scene.hpp>
#include <scene-controller.hpp>
#include <view-controller.hpp>
#include <record-controller.hpp>
#include <text-store.h>

#include <view-modules/submenu-vm.h>
#include <view-modules/byte-input-vm.h>

#include <notification/notification-messages.h>

class ScenedApp {
public:
    enum class EventType : uint8_t {
        GENERIC_EVENT_ENUM_VALUES,
        MenuSelected,
        ByteEditResult,
    };

    enum class SceneType : uint8_t {
        GENERIC_SCENE_ENUM_VALUES,
        ByteInputScene,
    };

    class Event {
    public:
        union {
            int32_t menu_index;
        } payload;

        EventType type;
    };

    SceneController<GenericScene<ScenedApp>, ScenedApp> scene_controller;
    TextStore text_store;
    ViewController<ScenedApp, SubmenuVM, ByteInputVM> view_controller;
    RecordController<NotificationApp> notification;

    ~ScenedApp();
    ScenedApp();

    void run();
};