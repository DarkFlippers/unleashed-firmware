#include "../desktop_i.h"
#include "../views/desktop_main.h"
#include "applications.h"
#include "assets_icons.h"
#include "cmsis_os2.h"
#include "desktop/desktop.h"
#include "desktop/views/desktop_events.h"
#include "dolphin/dolphin.h"
#include "furi/pubsub.h"
#include "furi/record.h"
#include "furi/thread.h"
#include "storage/storage-glue.h"
#include <loader/loader.h>
#include <m-list.h>
#define MAIN_VIEW_DEFAULT (0UL)

static void desktop_scene_main_app_started_callback(const void* message, void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    const LoaderEvent* event = message;

    if(event->type == LoaderEventTypeApplicationStarted) {
        view_dispatcher_send_custom_event(
            desktop->view_dispatcher, DesktopMainEventBeforeAppStarted);
        osSemaphoreAcquire(desktop->unload_animation_semaphore, osWaitForever);
    } else if(event->type == LoaderEventTypeApplicationStopped) {
        view_dispatcher_send_custom_event(
            desktop->view_dispatcher, DesktopMainEventAfterAppFinished);
    }
}

static void desktop_scene_main_new_idle_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopMainEventNewIdleAnimation);
}

static void desktop_scene_main_check_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopMainEventCheckAnimation);
}

static void desktop_scene_main_interact_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopMainEventInteractAnimation);
}

static void desktop_switch_to_app(Desktop* desktop, const FlipperApplication* flipper_app) {
    furi_assert(desktop);
    furi_assert(flipper_app);
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(furi_thread_get_state(desktop->scene_thread) != FuriThreadStateStopped) {
        FURI_LOG_E("Desktop", "Thread is already running");
        return;
    }

    furi_thread_set_name(desktop->scene_thread, flipper_app->name);
    furi_thread_set_stack_size(desktop->scene_thread, flipper_app->stack_size);
    furi_thread_set_callback(desktop->scene_thread, flipper_app->app);

    furi_thread_start(desktop->scene_thread);

    furi_thread_join(desktop->scene_thread);
}

void desktop_scene_main_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_main_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopMainView* main_view = desktop->main_view;

    animation_manager_set_context(desktop->animation_manager, desktop);
    animation_manager_set_new_idle_callback(
        desktop->animation_manager, desktop_scene_main_new_idle_animation_callback);
    animation_manager_set_check_callback(
        desktop->animation_manager, desktop_scene_main_check_animation_callback);
    animation_manager_set_interact_callback(
        desktop->animation_manager, desktop_scene_main_interact_animation_callback);

    furi_assert(osSemaphoreGetCount(desktop->unload_animation_semaphore) == 0);
    desktop->app_start_stop_subscription = furi_pubsub_subscribe(
        loader_get_pubsub(), desktop_scene_main_app_started_callback, desktop);

    desktop_main_set_callback(main_view, desktop_scene_main_callback, desktop);
    view_port_enabled_set(desktop->lock_viewport, false);

    if(scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneMain) ==
       DesktopMainEventUnlocked) {
        desktop_main_unlocked(desktop->main_view);
    }

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewMain);
}

bool desktop_scene_main_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopMainEventOpenMenu:
            loader_show_menu();
            consumed = true;
            break;

        case DesktopMainEventOpenLockMenu:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneLockMenu);
            consumed = true;
            break;

        case DesktopMainEventOpenDebug:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneDebug);
            consumed = true;
            break;

        case DesktopMainEventOpenArchive:
#ifdef APP_ARCHIVE
            animation_manager_unload_and_stall_animation(desktop->animation_manager);
            desktop_switch_to_app(desktop, &FLIPPER_ARCHIVE);
            animation_manager_load_and_continue_animation(desktop->animation_manager);
#endif
            consumed = true;
            break;

        case DesktopMainEventOpenFavorite:
            LOAD_DESKTOP_SETTINGS(&desktop->settings);
            animation_manager_unload_and_stall_animation(desktop->animation_manager);
            if(desktop->settings.favorite < FLIPPER_APPS_COUNT) {
                desktop_switch_to_app(desktop, &FLIPPER_APPS[desktop->settings.favorite]);
            } else {
                FURI_LOG_E("DesktopSrv", "Can't find favorite application");
            }
            animation_manager_load_and_continue_animation(desktop->animation_manager);
            consumed = true;
            break;

        case DesktopMainEventCheckAnimation:
            animation_manager_check_blocking_process(desktop->animation_manager);
            consumed = true;
            break;
        case DesktopMainEventNewIdleAnimation:
            animation_manager_new_idle_process(desktop->animation_manager);
            consumed = true;
            break;
        case DesktopMainEventInteractAnimation:
            animation_manager_interact_process(desktop->animation_manager);
            consumed = true;
            break;
        case DesktopMainEventBeforeAppStarted:
            animation_manager_unload_and_stall_animation(desktop->animation_manager);
            osSemaphoreRelease(desktop->unload_animation_semaphore);
            consumed = true;
            break;
        case DesktopMainEventAfterAppFinished:
            animation_manager_load_and_continue_animation(desktop->animation_manager);
            consumed = true;
            break;

        default:
            break;
        }
    }

    return consumed;
}

void desktop_scene_main_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;

    /**
     * We're allowed to leave this scene only when any other app & loader
     * is finished, that's why we can be sure there is no task waiting
     * for start/stop semaphore
     */
    furi_pubsub_unsubscribe(loader_get_pubsub(), desktop->app_start_stop_subscription);
    furi_assert(osSemaphoreGetCount(desktop->unload_animation_semaphore) == 0);

    animation_manager_set_new_idle_callback(desktop->animation_manager, NULL);
    animation_manager_set_check_callback(desktop->animation_manager, NULL);
    animation_manager_set_interact_callback(desktop->animation_manager, NULL);
    animation_manager_set_context(desktop->animation_manager, desktop);
    scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneMain, MAIN_VIEW_DEFAULT);
    desktop_main_reset_hint(desktop->main_view);
}
