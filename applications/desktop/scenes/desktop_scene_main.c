#include "../desktop_i.h"
#include "../views/desktop_main.h"
#include "applications.h"
#include <loader/loader.h>
#define MAIN_VIEW_DEFAULT (0UL)

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
}

void desktop_scene_main_callback(DesktopMainEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_main_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopMainView* main_view = desktop->main_view;

    desktop_main_set_callback(main_view, desktop_scene_main_callback, desktop);
    view_port_enabled_set(desktop->lock_viewport, false);

    desktop_settings_load(&desktop->settings);

    if(scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneMain) ==
       DesktopMainEventUnlocked) {
        desktop_main_unlocked(desktop->main_view);
    }

    desktop_main_switch_dolphin_animation(desktop->main_view);
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
            desktop_switch_to_app(desktop, &FLIPPER_ARCHIVE);
            consumed = true;
            break;

        case DesktopMainEventOpenFavorite:
            desktop_settings_load(&desktop->settings);
            desktop_switch_to_app(desktop, &FLIPPER_APPS[desktop->settings.favorite]);
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
    scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneMain, MAIN_VIEW_DEFAULT);
    desktop_main_reset_hint(desktop->main_view);
}
