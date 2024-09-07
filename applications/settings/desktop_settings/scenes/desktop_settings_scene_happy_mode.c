#include <furi.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/dialog_ex.h>
#include <dolphin/dolphin.h>

#include "desktop_settings_scene.h"
#include "../desktop_settings_app.h"
#include "../desktop_settings_custom_event.h"

static void desktop_settings_scene_happy_mode_done_callback(DialogExResult result, void* context) {
    DesktopSettingsApp* app = context;
    DolphinSettings settings;
    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    dolphin_get_settings(dolphin, &settings);
    settings.happy_mode = (result == DialogExResultRight);
    dolphin_set_settings(dolphin, &settings);
    furi_record_close(RECORD_DOLPHIN);
    view_dispatcher_send_custom_event(app->view_dispatcher, DesktopSettingsCustomEventExit);
}

void desktop_settings_scene_happy_mode_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    DolphinSettings settings;
    dolphin_get_settings(dolphin, &settings);
    furi_record_close(RECORD_DOLPHIN);

    dialog_ex_set_header(app->dialog_ex, "Happy Mode", 64, 0, AlignCenter, AlignTop);
    dialog_ex_set_text(
        app->dialog_ex,
        "I will never get angry at you\nfor not spending time with me\nas long as this mode is enabled",
        64,
        30,
        AlignCenter,
        AlignCenter);
    dialog_ex_set_left_button_text(app->dialog_ex, settings.happy_mode ? "Disable" : "Go back");
    dialog_ex_set_right_button_text(
        app->dialog_ex, settings.happy_mode ? "Keep enabled" : "Enable");
    dialog_ex_set_result_callback(app->dialog_ex, desktop_settings_scene_happy_mode_done_callback);
    dialog_ex_set_context(app->dialog_ex, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewDialogEx);
}

bool desktop_settings_scene_happy_mode_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsCustomEventExit:
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
            break;
        default:
            furi_crash();
        }
    }
    return consumed;
}

void desktop_settings_scene_happy_mode_on_exit(void* context) {
    UNUSED(context);
}
