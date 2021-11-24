#include "../desktop_i.h"
#include "../views/desktop_main.h"
#include "applications.h"
#include "assets_icons.h"
#include "desktop/desktop.h"
#include "desktop/helpers/desktop_animation.h"
#include "dolphin/dolphin.h"
#include "furi/pubsub.h"
#include "furi/record.h"
#include "storage/storage-glue.h"
#include <loader/loader.h>
#include <m-list.h>

#define LEVELUP_SCENE_PLAYING 0
#define LEVELUP_SCENE_STOPPED 1

static void desktop_scene_levelup_callback(DesktopMainEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

static void desktop_scene_levelup_animation_changed_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(
        desktop->view_dispatcher, DesktopMainEventUpdateOneShotAnimation);
}

void desktop_scene_levelup_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopMainView* main_view = desktop->main_view;

    desktop_main_set_callback(main_view, desktop_scene_levelup_callback, desktop);
    desktop_animation_set_animation_changed_callback(
        desktop->animation, desktop_scene_levelup_animation_changed_callback, desktop);

    desktop_animation_start_oneshot_levelup(desktop->animation);
    const Icon* icon = desktop_animation_get_oneshot_frame(desktop->animation);
    desktop_main_switch_dolphin_icon(desktop->main_view, icon);
    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewMain);
    scene_manager_set_scene_state(
        desktop->scene_manager, DesktopSceneLevelUp, LEVELUP_SCENE_PLAYING);
}

bool desktop_scene_levelup_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;
    DesktopMainEvent main_event = event.event;

    if(event.type == SceneManagerEventTypeCustom) {
        if(main_event == DesktopMainEventUpdateOneShotAnimation) {
            const Icon* icon = desktop_animation_get_oneshot_frame(desktop->animation);
            if(icon) {
                desktop_main_switch_dolphin_icon(desktop->main_view, icon);
            } else {
                scene_manager_set_scene_state(
                    desktop->scene_manager, DesktopSceneLevelUp, LEVELUP_SCENE_STOPPED);
            }
            consumed = true;
        } else {
            if(scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneLevelUp) ==
               LEVELUP_SCENE_STOPPED) {
                scene_manager_previous_scene(desktop->scene_manager);
            }
        }
    }

    return consumed;
}

void desktop_scene_levelup_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;

    Dolphin* dolphin = furi_record_open("dolphin");
    dolphin_upgrade_level(dolphin);
    furi_record_close("dolphin");
    desktop_animation_set_animation_changed_callback(desktop->animation, NULL, NULL);
    desktop_start_new_idle_animation(desktop->animation);
}
