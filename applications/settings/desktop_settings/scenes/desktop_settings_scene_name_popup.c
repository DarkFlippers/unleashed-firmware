#include <stdint.h>
#include <core/check.h>
#include <gui/scene_manager.h>
#include <gui/modules/popup.h>

#include "../desktop_settings_app.h"
#include <desktop/desktop_settings.h>
#include "desktop_settings_scene.h"

#define SCENE_EVENT_EXIT (0U)

static void name_popup_back_callback(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_EXIT);
}

void desktop_settings_scene_name_popup_on_enter(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;

    popup_set_context(app->popup, app);
    popup_set_callback(app->popup, name_popup_back_callback);
    popup_set_header(app->popup, "Name is set!", 64, 20, AlignCenter, AlignCenter);
    popup_set_text(
        app->popup, "Your Flipper will be\nrestarted on exit.", 64, 40, AlignCenter, AlignCenter);
    popup_set_timeout(app->popup, 2100);
    popup_enable_timeout(app->popup);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPopup);
}

bool desktop_settings_scene_name_popup_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EVENT_EXIT:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, DesktopSettingsAppSceneStart);
            consumed = true;
            break;

        default:
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_name_popup_on_exit(void* context) {
    UNUSED(context);
}
