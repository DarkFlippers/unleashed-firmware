#include "../infrared_app_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_tv_on_enter(void* context) {
    InfraredApp* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    InfraredBruteForce* brute_force = infrared->brute_force;

    infrared_brute_force_set_db_filename(brute_force, EXT_PATH("infrared/assets/tv.ir"));

    button_panel_reserve(button_panel, 2, 3);
    uint32_t i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        6,
        16,
        &I_power_19x20,
        &I_power_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 4, 38, &I_power_text_24x5);
    infrared_brute_force_add_record(brute_force, i++, "Power");
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        39,
        16,
        &I_mute_19x20,
        &I_mute_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 39, 38, &I_mute_text_19x5);

    button_panel_add_icon(button_panel, 0, 66, &I_ch_text_31x34);
    button_panel_add_icon(button_panel, 35, 66, &I_vol_tv_text_29x34);

    infrared_brute_force_add_record(brute_force, i++, "Mute");
    button_panel_add_item(
        button_panel,
        i,
        1,
        1,
        38,
        53,
        &I_volup_24x21,
        &I_volup_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);

    infrared_brute_force_add_record(brute_force, i++, "Vol_up");
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        3,
        53,
        &I_ch_up_24x21,
        &I_ch_up_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Ch_next");
    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        38,
        91,
        &I_voldown_24x21,
        &I_voldown_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Vol_dn");
    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        3,
        91,
        &I_ch_down_24x21,
        &I_ch_down_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Ch_prev");

    button_panel_add_label(button_panel, 5, 10, FontPrimary, "TV remote");

    infrared_scene_universal_common_on_enter(context);
}

bool infrared_scene_universal_tv_on_event(void* context, SceneManagerEvent event) {
    return infrared_scene_universal_common_on_event(context, event);
}

void infrared_scene_universal_tv_on_exit(void* context) {
    infrared_scene_universal_common_on_exit(context);
}
