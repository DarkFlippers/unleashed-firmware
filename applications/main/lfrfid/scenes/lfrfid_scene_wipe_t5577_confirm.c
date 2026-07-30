#include "../lfrfid_i.h"

void lfrfid_scene_wipe_t5577_confirm_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    widget_add_button_element(widget, GuiButtonTypeLeft, "Exit", lfrfid_widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Wipe", lfrfid_widget_callback, app);
    widget_add_string_element(widget, 64, 3, AlignCenter, AlignTop, FontPrimary, "Wipe T5577");
    widget_add_string_multiline_element(
        widget,
        64,
        19,
        AlignCenter,
        AlignTop,
        FontSecondary,
        "Erases all data. Tag won't\nread until re-written.\nHold still while running");

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
}

bool lfrfid_scene_wipe_t5577_confirm_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true; // Ignore Back button presses
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(
                scene_manager, LfRfidSceneExtraActions);
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(scene_manager, LfRfidSceneWipeT5577);
        }
    }

    return consumed;
}

void lfrfid_scene_wipe_t5577_confirm_on_exit(void* context) {
    LfRfid* app = context;
    widget_reset(app->widget);
}
