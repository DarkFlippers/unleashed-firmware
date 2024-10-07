#include "../infrared_app_i.h"

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_leds_on_enter(void* context) {
    InfraredApp* infrared = context;
    ButtonPanel* button_panel = infrared->button_panel;
    InfraredBruteForce* brute_force = infrared->brute_force;

    // Button codes
    // Power_off, Power_on, Brightness_up, Brightness_dn, Red, Blue, Green, White

    infrared_brute_force_set_db_filename(brute_force, EXT_PATH("infrared/assets/leds.ir"));

    button_panel_reserve(button_panel, 2, 4);
    uint32_t i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        10,
        12,
        &I_power_19x20,
        &I_power_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 15, 34, &I_on_text_9x5);
    infrared_brute_force_add_record(brute_force, i++, "Power_on");

    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        35,
        12,
        &I_off_19x20,
        &I_off_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 38, 34, &I_off_text_12x5);
    infrared_brute_force_add_record(brute_force, i++, "Power_off");

    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        10,
        42,
        &I_plus_19x20,
        &I_plus_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Brightness_up");

    button_panel_add_item(
        button_panel,
        i,
        1,
        1,
        35,
        42,
        &I_minus_19x20,
        &I_minus_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 12, 64, &I_brightness_text_40x5);
    infrared_brute_force_add_record(brute_force, i++, "Brightness_dn");

    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        10,
        74,
        &I_red_19x20,
        &I_red_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Red");
    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        35,
        74,
        &I_green_19x20,
        &I_green_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Green");
    button_panel_add_item(
        button_panel,
        i,
        0,
        3,
        10,
        99,
        &I_blue_19x20,
        &I_blue_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Blue");
    button_panel_add_item(
        button_panel,
        i,
        1,
        3,
        35,
        99,
        &I_white_19x20,
        &I_white_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 19, 121, &I_color_text_24x5);
    infrared_brute_force_add_record(brute_force, i++, "White");

    button_panel_add_label(button_panel, 20, 9, FontPrimary, "LEDs");

    infrared_scene_universal_common_on_enter(context);
}

bool infrared_scene_universal_leds_on_event(void* context, SceneManagerEvent event) {
    return infrared_scene_universal_common_on_event(context, event);
}

void infrared_scene_universal_leds_on_exit(void* context) {
    infrared_scene_universal_common_on_exit(context);
}
