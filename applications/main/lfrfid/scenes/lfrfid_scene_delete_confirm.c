#include "../lfrfid_i.h"

void lfrfid_scene_delete_confirm_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    string_t tmp_string;
    string_init(tmp_string);

    widget_add_button_element(widget, GuiButtonTypeLeft, "Back", lfrfid_widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Delete", lfrfid_widget_callback, app);

    string_printf(tmp_string, "Delete %s?", string_get_cstr(app->file_name));
    widget_add_string_element(
        widget, 64, 0, AlignCenter, AlignTop, FontPrimary, string_get_cstr(tmp_string));

    string_reset(tmp_string);
    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = (uint8_t*)malloc(size);
    protocol_dict_get_data(app->dict, app->protocol_id, data, size);
    for(uint8_t i = 0; i < MIN(size, (size_t)8); i++) {
        if(i != 0) {
            string_cat_printf(tmp_string, " ");
        }

        string_cat_printf(tmp_string, "%02X", data[i]);
    }
    free(data);

    widget_add_string_element(
        widget, 64, 19, AlignCenter, AlignTop, FontSecondary, string_get_cstr(tmp_string));
    widget_add_string_element(
        widget,
        64,
        49,
        AlignCenter,
        AlignBottom,
        FontSecondary,
        protocol_dict_get_name(app->dict, app->protocol_id));

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    string_clear(tmp_string);
}

bool lfrfid_scene_delete_confirm_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true; // Ignore Back button presses
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            lfrfid_delete_key(app);
            scene_manager_next_scene(scene_manager, LfRfidSceneDeleteSuccess);
        }
    }

    return consumed;
}

void lfrfid_scene_delete_confirm_on_exit(void* context) {
    LfRfid* app = context;
    widget_reset(app->widget);
}
