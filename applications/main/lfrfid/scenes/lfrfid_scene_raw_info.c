#include "../lfrfid_i.h"

void lfrfid_scene_raw_info_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    // string_t tmp_string;
    // string_init(tmp_string);

    bool sd_exist = storage_sd_status(app->storage) == FSE_OK;
    if(!sd_exist) {
        widget_add_icon_element(widget, 0, 0, &I_SDQuestion_35x43);
        widget_add_string_multiline_element(
            widget,
            81,
            4,
            AlignCenter,
            AlignTop,
            FontSecondary,
            "No SD card found.\nThis function will not\nwork without\nSD card.");

        widget_add_button_element(widget, GuiButtonTypeLeft, "Back", lfrfid_widget_callback, app);
    } else {
        widget_add_string_multiline_element(
            widget,
            0,
            1,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "RAW RFID data reader\n1) Put the Flipper on your card\n2) Press OK\n3) Wait until data is read");

        widget_add_button_element(widget, GuiButtonTypeCenter, "OK", lfrfid_widget_callback, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    //string_clear(tmp_string);
}

bool lfrfid_scene_raw_info_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
        scene_manager_search_and_switch_to_previous_scene(scene_manager, LfRfidSceneExtraActions);
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(scene_manager, LfRfidSceneRawRead);
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(
                scene_manager, LfRfidSceneExtraActions);
        }
    }

    return consumed;
}

void lfrfid_scene_raw_info_on_exit(void* context) {
    LfRfid* app = context;
    widget_reset(app->widget);
}
