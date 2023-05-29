#include "../infrared_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_ac_on_enter(void* context) {
    infrared_scene_universal_common_on_enter(context);

    Infrared* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    InfraredBruteForce* brute_force = infrared->brute_force;

    infrared_brute_force_set_db_filename(brute_force, EXT_PATH("infrared/assets/ac.ir"));

    button_panel_reserve(button_panel, 2, 3);
    uint32_t i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        3,
        22,
        &I_Off_25x27,
        &I_Off_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Off");
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        36,
        22,
        &I_Dehumidify_25x27,
        &I_Dehumidify_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Dh");
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        3,
        59,
        &I_CoolHi_25x27,
        &I_CoolHi_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Cool_hi");
    button_panel_add_item(
        button_panel,
        i,
        1,
        1,
        36,
        59,
        &I_HeatHi_25x27,
        &I_HeatHi_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Heat_hi");
    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        3,
        91,
        &I_CoolLo_25x27,
        &I_CoolLo_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Cool_lo");
    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        36,
        91,
        &I_HeatLo_25x27,
        &I_HeatLo_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Heat_lo");

    button_panel_add_label(button_panel, 6, 10, FontPrimary, "AC remote");

    view_set_orientation(view_stack_get_view(infrared->view_stack), ViewOrientationVertical);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewStack);

    infrared_show_loading_popup(infrared, true);
    bool success = infrared_brute_force_calculate_messages(brute_force);
    infrared_show_loading_popup(infrared, false);

    if(!success) {
        scene_manager_next_scene(infrared->scene_manager, InfraredSceneErrorDatabases);
    }
}

bool infrared_scene_universal_ac_on_event(void* context, SceneManagerEvent event) {
    return infrared_scene_universal_common_on_event(context, event);
}

void infrared_scene_universal_ac_on_exit(void* context) {
    infrared_scene_universal_common_on_exit(context);
}
