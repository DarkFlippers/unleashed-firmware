#pragma once

#include "desktop.h"

#include <furi.h>
#include <furi-hal.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/popup.h>
#include <gui/modules/code_input.h>
#include <gui/scene_manager.h>
#include <assets_icons.h>
#include <storage/storage.h>
#include <power/power_service/power.h>

#include "views/desktop_main.h"
#include "views/desktop_first_start.h"
#include "views/desktop_lock_menu.h"
#include "views/desktop_locked.h"
#include "views/desktop_debug.h"

#include "scenes/desktop_scene.h"
#include "helpers/desktop_animation.h"
#include "desktop/desktop_settings/desktop_settings.h"
#include <gui/icon.h>

#define STATUS_BAR_Y_SHIFT 13

typedef enum {
    DesktopViewMain,
    DesktopViewLockMenu,
    DesktopViewLocked,
    DesktopViewDebug,
    DesktopViewFirstStart,
    DesktopViewHwMismatch,
    DesktopViewPinSetup,
    DesktopViewTotal,
} DesktopViewEnum;

struct Desktop {
    // Scene
    FuriThread* scene_thread;
    // GUI
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    DesktopAnimation* animation;
    DesktopFirstStartView* first_start_view;
    Popup* hw_mismatch_popup;
    DesktopMainView* main_view;
    DesktopLockMenuView* lock_menu;
    DesktopLockedView* locked_view;
    DesktopDebugView* debug_view;
    CodeInput* code_input;

    DesktopSettings settings;
    PinCode pincode_buffer;

    ViewPort* lock_viewport;
};

Desktop* desktop_alloc();

void desktop_free(Desktop* desktop);
