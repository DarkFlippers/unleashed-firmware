#include "../infrared_app_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_audio_on_enter(void* context) {
    InfraredApp* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    InfraredBruteForce* brute_force = infrared->brute_force;

    infrared_brute_force_set_db_filename(brute_force, EXT_PATH("infrared/assets/audio.ir"));

    button_panel_reserve(button_panel, 2, 4);
    uint32_t i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        6,
        13,
        &I_power_19x20,
        &I_power_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 4, 35, &I_power_text_24x5);
    infrared_brute_force_add_record(brute_force, i++, "Power");
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        39,
        13,
        &I_mute_19x20,
        &I_mute_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 39, 35, &I_mute_text_19x5);
    infrared_brute_force_add_record(brute_force, i++, "Mute");
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        6,
        42,
        &I_play_19x20,
        &I_play_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 6, 64, &I_play_text_19x5);
    infrared_brute_force_add_record(brute_force, i++, "Play");
    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        6,
        71,
        &I_pause_19x20,
        &I_pause_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 4, 93, &I_pause_text_23x5);
    infrared_brute_force_add_record(brute_force, i++, "Pause");
    button_panel_add_item(
        button_panel,
        i,
        0,
        3,
        6,
        101,
        &I_prev_19x20,
        &I_prev_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 6, 123, &I_prev_text_19x5);
    infrared_brute_force_add_record(brute_force, i++, "Prev");
    button_panel_add_item(
        button_panel,
        i,
        1,
        3,
        39,
        101,
        &I_next_19x20,
        &I_next_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 39, 123, &I_next_text_19x6);
    infrared_brute_force_add_record(brute_force, i++, "Next");
    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        37,
        77,
        &I_voldown_24x21,
        &I_voldown_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Vol_dn");
    button_panel_add_item(
        button_panel,
        i,
        1,
        1,
        37,
        43,
        &I_volup_24x21,
        &I_volup_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Vol_up");

    button_panel_add_label(button_panel, 1, 10, FontPrimary, "Mus. remote");
    button_panel_add_icon(button_panel, 34, 56, &I_vol_ac_text_30x30);

    infrared_scene_universal_common_on_enter(context);
}

bool infrared_scene_universal_audio_on_event(void* context, SceneManagerEvent event) {
    return infrared_scene_universal_common_on_event(context, event);
}

void infrared_scene_universal_audio_on_exit(void* context) {
    infrared_scene_universal_common_on_exit(context);
}
