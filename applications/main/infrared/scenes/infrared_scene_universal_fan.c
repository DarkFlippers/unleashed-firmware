#include "../infrared_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_fan_on_enter(void* context) {
    infrared_scene_universal_common_on_enter(context);

    Infrared* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    InfraredBruteForce* brute_force = infrared->brute_force;

    infrared_brute_force_set_db_filename(brute_force, EXT_PATH("infrared/assets/fans.ir"));

    //TODO Improve Fan universal remote
    button_panel_reserve(button_panel, 2, 3);
    uint32_t i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        3,
        24,
        &I_Power_25x27,
        &I_Power_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "POWER");
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        36,
        24,
        &I_Mode_25x27,
        &I_Mode_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "MODE");
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
    infrared_brute_force_add_record(brute_force, i++, "SPEED+");
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
    infrared_brute_force_add_record(brute_force, i++, "SPEED-");
    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        3,
        98,
        &I_Rotate_25x27,
        &I_Rotate_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "ROTATE");
    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        36,
        98,
        &I_Timer_25x27,
        &I_Timer_hvr_25x27,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "TIMER");

    button_panel_add_label(button_panel, 5, 11, FontPrimary, "Fan remote");
    button_panel_add_label(button_panel, 20, 63, FontSecondary, "Speed");
    button_panel_add_label(button_panel, 8, 23, FontSecondary, "Pwr");
    button_panel_add_label(button_panel, 40, 23, FontSecondary, "Mod");

    view_set_orientation(view_stack_get_view(infrared->view_stack), ViewOrientationVertical);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewStack);

    infrared_show_loading_popup(infrared, true);
    bool success = infrared_brute_force_calculate_messages(brute_force);
    infrared_show_loading_popup(infrared, false);

    if(!success) {
        scene_manager_next_scene(infrared->scene_manager, InfraredSceneErrorDatabases);
    }
}

bool infrared_scene_universal_fan_on_event(void* context, SceneManagerEvent event) {
    return infrared_scene_universal_common_on_event(context, event);
}

void infrared_scene_universal_fan_on_exit(void* context) {
    infrared_scene_universal_common_on_exit(context);
}
