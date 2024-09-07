#include <core/check.h>
#include <gui/scene_manager.h>
#include <gui/modules/popup.h>

#include "../desktop_settings_app.h"
#include "../desktop_settings_custom_event.h"
#include <desktop/desktop_settings.h>
#include "desktop_settings_scene.h"

static void pin_disable_back_callback(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, DesktopSettingsCustomEventExit);
}

void desktop_settings_scene_pin_disable_on_enter(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;

    desktop_pin_code_reset();

    popup_set_context(app->popup, app);
    popup_set_callback(app->popup, pin_disable_back_callback);
    popup_set_icon(app->popup, 0, 2, &I_DolphinMafia_119x62);
    popup_set_header(app->popup, "Removed", 100, 10, AlignCenter, AlignTop);
    popup_set_timeout(app->popup, 1500);
    popup_enable_timeout(app->popup);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPopup);
}

bool desktop_settings_scene_pin_disable_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsCustomEventExit:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, DesktopSettingsAppScenePinMenu);
            consumed = true;
            break;

        default:
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_pin_disable_on_exit(void* context) {
    UNUSED(context);
}
