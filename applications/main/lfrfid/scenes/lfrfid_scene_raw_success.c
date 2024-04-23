#include "../lfrfid_i.h"

void lfrfid_scene_raw_success_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    widget_add_text_box_element(
        widget,
        0,
        0,
        128,
        64,
        AlignLeft,
        AlignTop,
        "\e#RAW RFID Read Success\e#\n"
        "Now you can analyze files or\n"
        "send them to developers",
        false);

    widget_add_button_element(widget, GuiButtonTypeCenter, "OK", lfrfid_widget_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
}

bool lfrfid_scene_raw_success_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_search_and_switch_to_previous_scene(
                scene_manager, LfRfidSceneExtraActions);
        }
        consumed = true;

    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_raw_success_on_exit(void* context) {
    LfRfid* app = context;
    widget_reset(app->widget);
}
