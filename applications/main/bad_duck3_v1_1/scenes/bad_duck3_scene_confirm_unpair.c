#include "../bad_duck3_app_i.h"

void bad_duck3_scene_confirm_unpair_on_enter(void* context) {
    BadDuck3App* app = context;

    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 64, 10, AlignCenter, AlignCenter, FontPrimary, "Remove BLE Pairing?");
    widget_add_string_multiline_element(
        app->widget,
        64,
        32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "This will remove all\nBluetooth bonded devices.");
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Cancel", NULL, app);
    widget_add_button_element(app->widget, GuiButtonTypeRight, "Remove", NULL, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewWidget);
}

bool bad_duck3_scene_confirm_unpair_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            duck3_hid_ble_remove_pairing();
            scene_manager_next_scene(app->scene_manager, BadDuck3SceneDone);
            consumed = true;
        }
    }

    return consumed;
}

void bad_duck3_scene_confirm_unpair_on_exit(void* context) {
    BadDuck3App* app = context;
    widget_reset(app->widget);
}
