#include <furi.h>
#include <gui/scene_manager.h>

#include "../desktop_i.h"
#include "../views/desktop_view_quick_settings.h"
#include "desktop_scene.h"

// The same short beep LCD & Notifications plays, so the new volume can be judged by ear.
static const NotificationSequence sequence_volume_probe = {
    &message_note_c5,
    &message_delay_100,
    &message_sound_off,
    NULL,
};

// Scene state holds whether a value was touched but not written to storage yet.
#define DESKTOP_QUICK_SETTINGS_CLEAN 0
#define DESKTOP_QUICK_SETTINGS_DIRTY 1

static void desktop_scene_quick_settings_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

static void desktop_scene_quick_settings_save(Desktop* desktop) {
    if(scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneQuickSettings) ==
       DESKTOP_QUICK_SETTINGS_CLEAN) {
        return;
    }
    scene_manager_set_scene_state(
        desktop->scene_manager, DesktopSceneQuickSettings, DESKTOP_QUICK_SETTINGS_CLEAN);
    notification_message_save_settings(desktop->notification);
}

void desktop_scene_quick_settings_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    scene_manager_set_scene_state(
        desktop->scene_manager, DesktopSceneQuickSettings, DESKTOP_QUICK_SETTINGS_CLEAN);
    desktop_quick_settings_set_callback(
        desktop->quick_settings, desktop_scene_quick_settings_callback, desktop);
    desktop_quick_settings_reset(
        desktop->quick_settings,
        desktop->notification->settings.display_brightness,
        desktop->notification->settings.speaker_volume,
        desktop->notification->settings.vibro_on);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdQuickSettings);
}

bool desktop_scene_quick_settings_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        // Every change goes into the live notification settings and is played back at once,
        // exactly like the matching item in LCD & Notifications does. The two bars only write
        // to storage once the item is left, so holding a direction does not hammer the flash;
        // the checkbox is a single press, so it writes straight away.
        case DesktopQuickSettingsEventBrightnessChanged:
            desktop->notification->settings.display_brightness =
                desktop_quick_settings_get_brightness(desktop->quick_settings);
            notification_message(desktop->notification, &sequence_display_backlight_force_on);
            scene_manager_set_scene_state(
                desktop->scene_manager, DesktopSceneQuickSettings, DESKTOP_QUICK_SETTINGS_DIRTY);
            consumed = true;
            break;
        case DesktopQuickSettingsEventVolumeChanged:
            desktop->notification->settings.speaker_volume =
                desktop_quick_settings_get_volume(desktop->quick_settings);
            notification_message(desktop->notification, &sequence_volume_probe);
            scene_manager_set_scene_state(
                desktop->scene_manager, DesktopSceneQuickSettings, DESKTOP_QUICK_SETTINGS_DIRTY);
            consumed = true;
            break;
        case DesktopQuickSettingsEventVibroChanged:
            desktop->notification->settings.vibro_on =
                desktop_quick_settings_get_vibro(desktop->quick_settings);
            notification_message(desktop->notification, &sequence_single_vibro);
            scene_manager_set_scene_state(
                desktop->scene_manager, DesktopSceneQuickSettings, DESKTOP_QUICK_SETTINGS_DIRTY);
            desktop_scene_quick_settings_save(desktop);
            consumed = true;
            break;
        case DesktopQuickSettingsEventSave:
            desktop_scene_quick_settings_save(desktop);
            consumed = true;
            break;
        case DesktopQuickSettingsEventClose:
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        // Back is only unconsumed by the view when no item is being edited, and then it
        // means "done with the menu", not "one page left" - sideways does that.
        scene_manager_search_and_switch_to_previous_scene(
            desktop->scene_manager, DesktopSceneMain);
        consumed = true;
    }

    return consumed;
}

void desktop_scene_quick_settings_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    desktop_scene_quick_settings_save(desktop);
}
