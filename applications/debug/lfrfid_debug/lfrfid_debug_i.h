#pragma once
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include <gui/modules/submenu.h>

#include "views/lfrfid_debug_view_tune.h"
#include "scenes/lfrfid_debug_scene.h"

typedef struct LfRfidDebug LfRfidDebug;

struct LfRfidDebug {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    // Common Views
    Submenu* submenu;
    LfRfidTuneView* tune_view;
};

typedef enum {
    LfRfidDebugViewSubmenu,
    LfRfidDebugViewTune,
} LfRfidDebugView;
