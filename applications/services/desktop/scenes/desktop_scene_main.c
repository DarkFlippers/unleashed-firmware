#include <furi.h>
#include <furi_hal.h>
#include <applications.h>
#include <assets_icons.h>
#include <loader/loader.h>

#include "../desktop_i.h"
#include "../views/desktop_events.h"
#include "../views/desktop_view_main.h"
#include "desktop_scene.h"
#include "desktop_scene_i.h"

#define TAG "DesktopSrv"

#define FAP_LOADER_APP_NAME "Applications"

static void desktop_scene_main_new_idle_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(
        desktop->view_dispatcher, DesktopAnimationEventNewIdleAnimation);
}

static void desktop_scene_main_check_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(
        desktop->view_dispatcher, DesktopAnimationEventCheckAnimation);
}

static void desktop_scene_main_interact_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(
        desktop->view_dispatcher, DesktopAnimationEventInteractAnimation);
}

#ifdef APP_ARCHIVE
static void desktop_switch_to_app(Desktop* desktop, const FlipperApplication* flipper_app) {
    furi_assert(desktop);
    furi_assert(flipper_app);
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(furi_thread_get_state(desktop->scene_thread) != FuriThreadStateStopped) {
        FURI_LOG_E("Desktop", "Thread is already running");
        return;
    }

    FuriHalRtcHeapTrackMode mode = furi_hal_rtc_get_heap_track_mode();
    if(mode > FuriHalRtcHeapTrackModeNone) {
        furi_thread_enable_heap_trace(desktop->scene_thread);
    } else {
        furi_thread_disable_heap_trace(desktop->scene_thread);
    }

    furi_thread_set_name(desktop->scene_thread, flipper_app->name);
    furi_thread_set_stack_size(desktop->scene_thread, flipper_app->stack_size);
    furi_thread_set_callback(desktop->scene_thread, flipper_app->app);

    furi_thread_start(desktop->scene_thread);
}
#endif

static void desktop_scene_main_open_app_or_profile(Desktop* desktop, const char* path) {
    do {
        LoaderStatus status = loader_start(desktop->loader, FAP_LOADER_APP_NAME, path);
        if(status == LoaderStatusOk) break;
        FURI_LOG_E(TAG, "loader_start failed: %d", status);

        status = loader_start(desktop->loader, "Passport", NULL);
        if(status != LoaderStatusOk) {
            FURI_LOG_E(TAG, "loader_start failed: %d", status);
        }
    } while(false);
}

static void desktop_scene_main_start_favorite(Desktop* desktop, FavoriteApp* application) {
    LoaderStatus status = LoaderStatusErrorInternal;
    if(application->is_external) {
        status = loader_start(desktop->loader, FAP_LOADER_APP_NAME, application->name_or_path);
    } else if(strlen(application->name_or_path) > 0) {
        status = loader_start(desktop->loader, application->name_or_path, NULL);
    } else {
        // No favourite app is set! So we skipping this part
        return;
        //status = loader_start(desktop->loader, FAP_LOADER_APP_NAME, NULL);
    }

    if(status != LoaderStatusOk) {
        FURI_LOG_E(TAG, "loader_start failed: %d", status);
    }
}

void desktop_scene_main_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    if(desktop->in_transition) return;
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

    desktop_main_set_callback(main_view, desktop_scene_main_callback, desktop);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdMain);
}

bool desktop_scene_main_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopMainEventOpenMenu: {
            Loader* loader = furi_record_open(RECORD_LOADER);
            loader_show_menu(loader);
            furi_record_close(RECORD_LOADER);
            consumed = true;
        } break;

        case DesktopMainEventOpenLockMenu:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneLockMenu);
            consumed = true;
            break;

        case DesktopMainEventOpenDebug:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneDebug);
            consumed = true;
            break;

        case DesktopMainEventLock:
            scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
            desktop_lock(desktop);
            consumed = true;
            break;

        case DesktopMainEventOpenArchive:
#ifdef APP_ARCHIVE
            desktop_switch_to_app(desktop, &FLIPPER_ARCHIVE);
#endif
            consumed = true;
            break;

        case DesktopMainEventOpenPowerOff: {
            LoaderStatus status = loader_start(desktop->loader, "Power", "off");
            if(status != LoaderStatusOk) {
                FURI_LOG_E(TAG, "loader_start failed: %d", status);
            }
            consumed = true;
            break;
        }

        case DesktopMainEventOpenFavoritePrimary:
            DESKTOP_SETTINGS_LOAD(&desktop->settings);
            desktop_scene_main_start_favorite(desktop, &desktop->settings.favorite_primary);
            consumed = true;
            break;
        case DesktopMainEventOpenFavoriteSecondary:
            DESKTOP_SETTINGS_LOAD(&desktop->settings);
            desktop_scene_main_start_favorite(desktop, &desktop->settings.favorite_secondary);
            consumed = true;
            break;
        case DesktopMainEventOpenFavoriteTertiary:
            DESKTOP_SETTINGS_LOAD(&desktop->settings);
            desktop_scene_main_start_favorite(desktop, &desktop->settings.favorite_tertiary);
            consumed = true;
            break;
        case DesktopAnimationEventCheckAnimation:
            animation_manager_check_blocking_process(desktop->animation_manager);
            consumed = true;
            break;
        case DesktopAnimationEventNewIdleAnimation:
            animation_manager_new_idle_process(desktop->animation_manager);
            consumed = true;
            break;
        case DesktopAnimationEventInteractAnimation:
            if(!animation_manager_interact_process(desktop->animation_manager)) {
                LoaderStatus status = loader_start(desktop->loader, "Passport", NULL);
                if(status != LoaderStatusOk) {
                    FURI_LOG_E(TAG, "loader_start failed: %d", status);
                }
            }
            consumed = true;
            break;
        case DesktopMainEventOpenPassport: {
            LoaderStatus status = loader_start(desktop->loader, "Passport", NULL);
            if(status != LoaderStatusOk) {
                FURI_LOG_E(TAG, "loader_start failed: %d", status);
            }
            break;
        }
        case DesktopMainEventOpenGameMenu: {
            desktop_scene_main_open_app_or_profile(desktop, EXT_PATH("/apps/Games/snake.fap"));
            break;
        }
        case DesktopMainEventOpenTetris: {
            desktop_scene_main_open_app_or_profile(desktop, EXT_PATH("/apps/Games/tetris.fap"));
            break;
        }
        case DesktopMainEventOpenArkanoid: {
            desktop_scene_main_open_app_or_profile(desktop, EXT_PATH("/apps/Games/arkanoid.fap"));
            break;
        }
        case DesktopMainEventOpenDOOM: {
            desktop_scene_main_open_app_or_profile(desktop, EXT_PATH("/apps/Games/doom.fap"));
            break;
        }
        case DesktopMainEventOpenZombiez: {
            desktop_scene_main_open_app_or_profile(desktop, EXT_PATH("/apps/Games/zombiez.fap"));
            break;
        }
        case DesktopMainEventOpenHeap: {
            desktop_scene_main_open_app_or_profile(
                desktop, EXT_PATH("/apps/Games/heap_defence.fap"));
            break;
        }
        case DesktopLockedEventUpdate:
            desktop_view_locked_update(desktop->locked_view);
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

    animation_manager_set_new_idle_callback(desktop->animation_manager, NULL);
    animation_manager_set_check_callback(desktop->animation_manager, NULL);
    animation_manager_set_interact_callback(desktop->animation_manager, NULL);
    animation_manager_set_context(desktop->animation_manager, desktop);
}
