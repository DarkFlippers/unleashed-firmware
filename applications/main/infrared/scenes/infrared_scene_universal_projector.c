#include "../infrared_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_projector_on_enter(void* context) {
    infrared_scene_universal_common_on_enter(context);

    Infrared* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    InfraredBruteForce* brute_force = infrared->brute_force;

    infrared_brute_force_set_db_filename(brute_force, EXT_PATH("infrared/assets/projectors.ir"));
    //TODO Improve Projectors universal remote
    button_panel_reserve(button_panel, 2, 3);
    uint32_t i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        6,
        24,
        &I_power_19x20,
        &I_power_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 4, 46, &I_power_text_24x5);
    infrared_brute_force_add_record(brute_force, i++, "Power");
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        39,
        24,
        &I_mute_19x20,
        &I_mute_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 39, 46, &I_mute_text_19x5);
    infrared_brute_force_add_record(brute_force, i++, "Mute");
    button_panel_add_item(
        button_panel,
        i,
        1,
        1,
        37,
        55,
        &I_volup_24x21,
        &I_volup_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Vol_up");

    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        37,
        89,
        &I_voldown_24x21,
        &I_voldown_hover_24x21,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Vol_dn");
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        6,
        58,
        &I_play_19x20,
        &I_play_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Play");
    button_panel_add_icon(button_panel, 6, 80, &I_play_text_19x5);
    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        6,
        87,
        &I_pause_19x20,
        &I_pause_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Pause");
    button_panel_add_icon(button_panel, 4, 109, &I_pause_text_23x5);

    button_panel_add_label(button_panel, 3, 11, FontPrimary, "Proj. remote");
    button_panel_add_icon(button_panel, 34, 68, &I_vol_ac_text_30x30);

    view_set_orientation(view_stack_get_view(infrared->view_stack), ViewOrientationVertical);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewStack);

    infrared_show_loading_popup(infrared, true);
    bool success = infrared_brute_force_calculate_messages(brute_force);
    infrared_show_loading_popup(infrared, false);

    if(!success) {
        scene_manager_next_scene(infrared->scene_manager, InfraredSceneErrorDatabases);
    }
}

bool infrared_scene_universal_projector_on_event(void* context, SceneManagerEvent event) {
    return infrared_scene_universal_common_on_event(context, event);
}

void infrared_scene_universal_projector_on_exit(void* context) {
    infrared_scene_universal_common_on_exit(context);
}
