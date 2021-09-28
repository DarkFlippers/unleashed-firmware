#pragma once

#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <assets_icons.h>

#include "desktop.h"

#include "views/desktop_main.h"
#include "views/desktop_first_start.h"
#include "views/desktop_hw_mismatch.h"
#include "views/desktop_lock_menu.h"
#include "views/desktop_locked.h"
#include "views/desktop_debug.h"
#include "scenes/desktop_scene.h"

#include "desktop/desktop_settings/desktop_settings.h"

#define HINT_TIMEOUT_L 2
#define HINT_TIMEOUT_H 11

typedef enum {
    DesktopViewMain,
    DesktopViewLockMenu,
    DesktopViewLocked,
    DesktopViewDebug,
    DesktopViewFirstStart,
    DesktopViewHwMismatch,
    DesktopViewTotal,
} DesktopViewEnum;

struct Desktop {
    // Scene
    FuriThread* scene_thread;
    // GUI
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    DesktopFirstStartView* first_start_view;
    DesktopHwMismatchView* hw_mismatch_view;
    DesktopMainView* main_view;
    DesktopLockMenuView* lock_menu;
    DesktopLockedView* locked_view;
    DesktopDebugView* debug_view;
    DesktopSettings settings;

    ViewPort* lock_viewport;
};

Desktop* desktop_alloc();

void desktop_free(Desktop* desktop);
