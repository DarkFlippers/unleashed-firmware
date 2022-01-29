#pragma once

#include "desktop.h"
#include "animations/animation_manager.h"
#include "views/desktop_main.h"
#include "views/desktop_first_start.h"
#include "views/desktop_lock_menu.h"
#include "views/desktop_locked.h"
#include "views/desktop_debug.h"
#include "desktop/desktop_settings/desktop_settings.h"

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_stack.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/popup.h>
#include <gui/modules/code_input.h>
#include <gui/scene_manager.h>

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

    DesktopMainView* main_view;
    DesktopLockedView* locked_view;

    ViewStack* main_view_stack;
    ViewStack* locked_view_stack;

    DesktopSettings settings;
    PinCode pincode_buffer;

    ViewPort* lock_viewport;

    AnimationManager* animation_manager;
    osSemaphoreId_t unload_animation_semaphore;
    FuriPubSubSubscription* app_start_stop_subscription;
};

Desktop* desktop_alloc();

void desktop_free(Desktop* desktop);
