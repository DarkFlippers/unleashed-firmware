#include "../lfrfid_i.h"

void lfrfid_scene_saved_info_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    string_t tmp_string;
    string_init(tmp_string);

    string_printf(
        tmp_string,
        "%s [%s]\r\n",
        string_get_cstr(app->file_name),
        protocol_dict_get_name(app->dict, app->protocol_id));

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = (uint8_t*)malloc(size);
    protocol_dict_get_data(app->dict, app->protocol_id, data, size);
    for(uint8_t i = 0; i < size; i++) {
        if(i != 0) {
            string_cat_printf(tmp_string, " ");
        }

        string_cat_printf(tmp_string, "%02X", data[i]);
    }
    free(data);

    string_t render_data;
    string_init(render_data);
    protocol_dict_render_data(app->dict, render_data, app->protocol_id);
    string_cat_printf(tmp_string, "\r\n%s", string_get_cstr(render_data));
    string_clear(render_data);

    widget_add_string_multiline_element(
        widget, 0, 1, AlignLeft, AlignTop, FontSecondary, string_get_cstr(tmp_string));

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    string_clear(tmp_string);
}

bool lfrfid_scene_saved_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void lfrfid_scene_saved_info_on_exit(void* context) {
    LfRfid* app = context;
    widget_reset(app->widget);
}
