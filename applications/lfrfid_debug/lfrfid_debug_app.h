#pragma once
#include <furi.h>
#include <furi_hal.h>

#include <generic_scene.hpp>
#include <scene_controller.hpp>
#include <view_controller.hpp>

#include <view_modules/submenu_vm.h>
#include "view_modules/lfrfid_view_tune_vm.h"

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
