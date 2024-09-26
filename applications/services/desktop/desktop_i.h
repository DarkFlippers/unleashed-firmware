#pragma once

#include "desktop.h"
#include "desktop_settings.h"

#include "animations/animation_manager.h"
#include "views/desktop_view_pin_timeout.h"
#include "views/desktop_view_pin_input.h"
#include "views/desktop_view_locked.h"
#include "views/desktop_view_main.h"
#include "views/desktop_view_lock_menu.h"
#include "views/desktop_view_debug.h"
#include "views/desktop_view_slideshow.h"

#include <gui/gui.h>
#include <gui/view_stack.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/popup.h>
#include <gui/scene_manager.h>

#include <loader/loader.h>
#include <notification/notification_app.h>

#define STATUS_BAR_Y_SHIFT 13

typedef enum {
    DesktopViewIdMain,
    DesktopViewIdLockMenu,
    DesktopViewIdLocked,
    DesktopViewIdDebug,
    DesktopViewIdPopup,
    DesktopViewIdPinInput,
    DesktopViewIdPinTimeout,
    DesktopViewIdSlideshow,
    DesktopViewIdTotal,
} DesktopViewId;

typedef struct {
    uint8_t hour;
    uint8_t minute;
    bool format_12; // 1 - 12 hour, 0 - 24H
} DesktopClock;

struct Desktop {
    FuriThread* scene_thread;

    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    Popup* popup;
    DesktopLockMenuView* lock_menu;
    DesktopDebugView* debug_view;
    DesktopViewLocked* locked_view;
    DesktopMainView* main_view;
    DesktopViewPinTimeout* pin_timeout_view;
    DesktopSlideshowView* slideshow_view;
    DesktopViewPinInput* pin_input_view;

    ViewStack* main_view_stack;
    ViewStack* locked_view_stack;

    ViewPort* lock_icon_viewport;
    ViewPort* dummy_mode_icon_viewport;
    ViewPort* clock_viewport;
    ViewPort* stealth_mode_icon_viewport;

    Loader* loader;
    Storage* storage;
    NotificationApp* notification;

    FuriPubSub* status_pubsub;
    FuriPubSub* input_events_pubsub;
    FuriPubSubSubscription* input_events_subscription;

    FuriTimer* auto_lock_timer;
    FuriTimer* update_clock_timer;

    AnimationManager* animation_manager;
    FuriSemaphore* animation_semaphore;

    DesktopClock clock;
    DesktopSettings settings;

    bool in_transition;
    bool app_running;
    bool locked;
};

void desktop_lock(Desktop* desktop);
void desktop_unlock(Desktop* desktop);
void desktop_set_dummy_mode_state(Desktop* desktop, bool enabled);
void desktop_set_stealth_mode_state(Desktop* desktop, bool enabled);
