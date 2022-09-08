#include "../lfrfid_i.h"

void lfrfid_scene_raw_success_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    widget_add_button_element(widget, GuiButtonTypeCenter, "OK", lfrfid_widget_callback, app);

    widget_add_string_multiline_element(
        widget,
        0,
        1,
        AlignLeft,
        AlignTop,
        FontSecondary,
        "RAW RFID read success!\nNow you can analyze files\nOr send them to developers");

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
}

bool lfrfid_scene_raw_success_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_search_and_switch_to_previous_scene(
                scene_manager, LfRfidSceneExtraActions);
        }
    }
    return consumed;
}

void lfrfid_scene_raw_success_on_exit(void* context) {
    LfRfid* app = context;
    widget_reset(app->widget);
}
