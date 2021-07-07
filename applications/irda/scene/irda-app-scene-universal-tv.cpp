#include "irda/scene/irda-app-scene.hpp"
#include "irda/irda-app.hpp"

void IrdaAppSceneUniversalTV::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    ButtonPanel* button_panel = view_manager->get_button_panel();
    button_panel_reserve(button_panel, 2, 3);

    int i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        3,
        19,
        &I_Power_25x27,
        &I_Power_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "POWER");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        36,
        19,
        &I_Mute_25x27,
        &I_Mute_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "MUTE");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        3,
        66,
        &I_Vol_up_25x27,
        &I_Vol_up_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "VOL+");
    ++i;
    button_panel_add_item(
        button_panel, i, 1, 1, 36, 66, &I_Up_25x27, &I_Up_hvr_25x27, irda_app_item_callback, app);
    brute_force.add_record(i, "CH+");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        3,
        98,
        &I_Vol_down_25x27,
        &I_Vol_down_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "VOL-");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        36,
        98,
        &I_Down_25x27,
        &I_Down_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "CH-");

    button_panel_add_label(button_panel, 6, 11, FontPrimary, "TV remote");
    button_panel_add_label(button_panel, 9, 64, FontSecondary, "Vol");
    button_panel_add_label(button_panel, 43, 64, FontSecondary, "Ch");

    view_manager->switch_to(IrdaAppViewManager::ViewType::ButtonPanel);

    if(!brute_force.calculate_messages()) {
        app->switch_to_previous_scene();
    }
}
