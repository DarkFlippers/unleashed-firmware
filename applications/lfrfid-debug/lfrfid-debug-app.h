#pragma once
#include <furi.h>
#include <furi-hal.h>

#include <generic-scene.hpp>
#include <scene-controller.hpp>
#include <view-controller.hpp>

#include <view-modules/submenu-vm.h>
#include "view-modules/lfrfid-view-tune-vm.h"

class LfRfidDebugApp {
public:
    enum class EventType : uint8_t {
        GENERIC_EVENT_ENUM_VALUES,
        MenuSelected,
    };

    enum class SceneType : uint8_t {
        GENERIC_SCENE_ENUM_VALUES,
        TuneScene,
    };

    class Event {
    public:
        union {
            int32_t menu_index;
        } payload;

        EventType type;
    };

    SceneController<GenericScene<LfRfidDebugApp>, LfRfidDebugApp> scene_controller;
    ViewController<LfRfidDebugApp, SubmenuVM, LfRfidViewTuneVM> view_controller;

    ~LfRfidDebugApp();
    LfRfidDebugApp();

    void run();
};