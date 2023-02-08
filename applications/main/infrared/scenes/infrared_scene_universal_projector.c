#include "../infrared_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_projector_on_enter(void* context) {
    infrared_scene_universal_common_on_enter(context);

    Infrared* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    InfraredBruteForce* brute_force = infrared->brute_force;

    infrared_brute_force_set_db_filename(brute_force, EXT_PATH("infrared/assets/projector.ir"));

    button_panel_reserve(button_panel, 2, 2);
    uint32_t i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        3,
        19,
        &I_Power_25x27,
        &I_Power_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Power");
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        36,
        19,
        &I_Mute_25x27,
        &I_Mute_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Mute");
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        3,
        66,
        &I_Vol_up_25x27,
        &I_Vol_up_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Vol_up");
    button_panel_add_item(
        button_panel,
        i,
        1,
        1,
        36,
        66,
        &I_Vol_down_25x27,
        &I_Vol_down_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Vol_dn");

    button_panel_add_label(button_panel, 2, 11, FontPrimary, "Proj. remote");
    button_panel_add_label(button_panel, 17, 62, FontSecondary, "Volume");

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
