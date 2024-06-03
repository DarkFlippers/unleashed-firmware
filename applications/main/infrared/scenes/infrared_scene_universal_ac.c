#include "../infrared_app_i.h" // IWYU pragma: keep

#include "common/infrared_scene_universal_common.h"

void infrared_scene_universal_ac_on_enter(void* context) {
    InfraredApp* infrared = context;
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
        6,
        15,
        &I_off_19x20,
        &I_off_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 10, 37, &I_off_text_12x5);
    infrared_brute_force_add_record(brute_force, i++, "Off");
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        39,
        15,
        &I_dry_19x20,
        &I_dry_hover_19x20,
        infrared_scene_universal_common_item_callback,
        context);
    button_panel_add_icon(button_panel, 41, 37, &I_dry_text_15x5);
    infrared_brute_force_add_record(brute_force, i++, "Dh");
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        3,
        49,
        &I_max_24x23,
        &I_max_hover_24x23,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Cool_hi");
    button_panel_add_item(
        button_panel,
        i,
        1,
        1,
        37,
        49,
        &I_max_24x23,
        &I_max_hover_24x23,
        infrared_scene_universal_common_item_callback,
        context);
    infrared_brute_force_add_record(brute_force, i++, "Heat_hi");
    if(furi_hal_rtc_get_locale_units() == FuriHalRtcLocaleUnitsMetric) {
        button_panel_add_item(
            button_panel,
            i,
            0,
            2,
            3,
            100,
            &I_celsius_24x23,
            &I_celsius_hover_24x23,
            infrared_scene_universal_common_item_callback,
            context);
    } else {
        button_panel_add_item(
            button_panel,
            i,
            0,
            2,
            3,
            100,
            &I_fahren_24x23,
            &I_fahren_hover_24x23,
            infrared_scene_universal_common_item_callback,
            context);
    }
    infrared_brute_force_add_record(brute_force, i++, "Cool_lo");

    if(furi_hal_rtc_get_locale_units() == FuriHalRtcLocaleUnitsMetric) {
        button_panel_add_item(
            button_panel,
            i,
            1,
            2,
            37,
            100,
            &I_celsius_24x23,
            &I_celsius_hover_24x23,
            infrared_scene_universal_common_item_callback,
            context);
    } else {
        button_panel_add_item(
            button_panel,
            i,
            1,
            2,
            37,
            100,
            &I_fahren_24x23,
            &I_fahren_hover_24x23,
            infrared_scene_universal_common_item_callback,
            context);
    }
    infrared_brute_force_add_record(brute_force, i++, "Heat_lo");

    button_panel_add_icon(button_panel, 0, 60, &I_cool_30x51);
    button_panel_add_icon(button_panel, 34, 60, &I_heat_30x51);

    button_panel_add_label(button_panel, 4, 10, FontPrimary, "AC remote");

    infrared_scene_universal_common_on_enter(context);
}

bool infrared_scene_universal_ac_on_event(void* context, SceneManagerEvent event) {
    return infrared_scene_universal_common_on_event(context, event);
}

void infrared_scene_universal_ac_on_exit(void* context) {
    infrared_scene_universal_common_on_exit(context);
}
