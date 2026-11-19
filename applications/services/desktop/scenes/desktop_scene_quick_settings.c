#include <furi.h>
#include <gui/scene_manager.h>

#include "../desktop_i.h"
#include "../views/desktop_view_quick_settings.h"
#include "desktop_scene.h"
#include <toolbox/saved_struct.h>
#include <bt/bt_service/bt_settings_api_i.h>

// The same short beep LCD & Notifications plays, so the new volume can be judged by ear.
static const NotificationSequence sequence_volume_probe = {
    &message_note_c5,
    &message_delay_100,
    &message_sound_off,
    NULL,
};

// Private remembered nonzero volume only; upstream notification format is untouched.
#define HYBRIDUS_MUTE_PATH INT_PATH(".hybridus-mute")
#define HYBRIDUS_MUTE_MAGIC 0xC8
#define HYBRIDUS_MUTE_VERSION 1

// Scene state holds whether a value was touched but not written to storage yet.
#define DESKTOP_QUICK_SETTINGS_CLEAN 0
#define DESKTOP_QUICK_SETTINGS_DIRTY 1

// This file belongs only to this scene. Verify persisted bytes, not a RAM cache:
// a failed/missing/corrupt read must fall back to the original immediate save.
// No deferred writes, no change to the format or failure behavior of Mute.
static void desktop_scene_quick_settings_save_restore_volume(uint8_t restore) {
    uint8_t persisted = 0;
    if(!saved_struct_load(HYBRIDUS_MUTE_PATH, &persisted, sizeof(persisted),
                          HYBRIDUS_MUTE_MAGIC, HYBRIDUS_MUTE_VERSION) || persisted != restore) {
        saved_struct_save(HYBRIDUS_MUTE_PATH, &restore, sizeof(restore),
                          HYBRIDUS_MUTE_MAGIC, HYBRIDUS_MUTE_VERSION);
    }
}

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
        desktop->notification->settings.vibro_on,
        desktop->settings.dummy_mode);

    uint8_t restore_volume = 10;
    if(!saved_struct_load(HYBRIDUS_MUTE_PATH, &restore_volume, sizeof(restore_volume),
                          HYBRIDUS_MUTE_MAGIC, HYBRIDUS_MUTE_VERSION) ||
       restore_volume == 0 || restore_volume > 20) restore_volume = 10;
    Bt* bt = furi_record_open(RECORD_BT);
    BtSettings settings;
    bt_get_settings(bt, &settings);
    desktop_quick_settings_set_system_state(desktop->quick_settings, settings.enabled,
        furi_hal_rtc_is_flag_set(FuriHalRtcFlagHandOrient), restore_volume);
    furi_record_close(RECORD_BT);

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
        case DesktopQuickSettingsEventDummyChanged:
            desktop_set_dummy_mode_state(
                desktop, desktop_quick_settings_get_dummy(desktop->quick_settings));
            consumed = true;
            break;
        case DesktopQuickSettingsEventBluetoothChanged: {
            Bt* bt = furi_record_open(RECORD_BT);
            BtSettings settings;
            bt_get_settings(bt, &settings);
            settings.enabled = desktop_quick_settings_get_bluetooth(desktop->quick_settings);
            bt_set_settings(bt, &settings);
            furi_record_close(RECORD_BT);
            consumed = true;
            break;
        }
        case DesktopQuickSettingsEventMuteChanged: {
            const uint8_t restore = desktop_quick_settings_get_restore_volume(desktop->quick_settings);
            desktop_scene_quick_settings_save_restore_volume(restore);
            desktop->notification->settings.speaker_volume =
                desktop_quick_settings_get_volume(desktop->quick_settings);
            // Mute is SOUND ONLY. Do not use StealthMode, which also suppresses vibration.
            // Stop a current notification tone; direct-speaker applications retain upstream semantics.
            notification_message(desktop->notification, &sequence_reset_sound);
            scene_manager_set_scene_state(
                desktop->scene_manager, DesktopSceneQuickSettings, DESKTOP_QUICK_SETTINGS_DIRTY);
            desktop_scene_quick_settings_save(desktop);
            consumed = true;
            break;
        }
        case DesktopQuickSettingsEventHandChanged:
            if(desktop_quick_settings_get_left_hand(desktop->quick_settings)) {
                furi_hal_rtc_set_flag(FuriHalRtcFlagHandOrient);
            } else {
                furi_hal_rtc_reset_flag(FuriHalRtcFlagHandOrient);
            }
            consumed = true;
            break;
        case DesktopQuickSettingsEventSave:
            desktop_scene_quick_settings_save(desktop);
            consumed = true;
            break;
        case DesktopQuickSettingsEventClose:
            // Legacy page is an explicit long-Back shortcut, never a sideways trap.
            scene_manager_search_and_switch_to_previous_scene(
                desktop->scene_manager, DesktopSceneMain);
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneLockMenu);
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        // One Back always exits; there is no nested edit/page mode.
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
