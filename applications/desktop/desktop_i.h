#pragma once

#include "cmsis_os2.h"
#include "desktop.h"

#include "animations/animation_manager.h"
#include "gui/view_composed.h"
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

    DesktopFirstStartView* first_start_view;
    Popup* hw_mismatch_popup;
    DesktopLockMenuView* lock_menu;
    DesktopDebugView* debug_view;
    CodeInput* code_input;

    View* dolphin_view;
    DesktopMainView* main_view;
    DesktopLockedView* locked_view;

    ViewComposed* main_view_composed;
    ViewComposed* locked_view_composed;

    DesktopSettings settings;
    PinCode pincode_buffer;

    ViewPort* lock_viewport;

    AnimationManager* animation_manager;
    osSemaphoreId_t unload_animation_semaphore;
    FuriPubSubSubscription* app_start_stop_subscription;

    char* text_buffer;
};

Desktop* desktop_alloc();

void desktop_free(Desktop* desktop);
