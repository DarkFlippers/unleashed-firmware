#include "../lfrfid_i.h"

void lfrfid_scene_saved_info_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    FuriString* display_text = furi_string_alloc();

    furi_string_printf(display_text, "Name: %s\n", furi_string_get_cstr(app->file_name));

    const char* protocol = protocol_dict_get_name(app->dict, app->protocol_id);
    const char* manufacturer = protocol_dict_get_manufacturer(app->dict, app->protocol_id);

    if(strcasecmp(protocol, manufacturer) != 0 && strcasecmp(manufacturer, "N/A") != 0) {
        furi_string_cat_printf(display_text, "\e#%s %s", manufacturer, protocol);
    } else {
        furi_string_cat_printf(display_text, "\e#%s", protocol);
    }

    furi_string_cat(display_text, "\nHex: ");

    const size_t data_size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = malloc(data_size);

    protocol_dict_get_data(app->dict, app->protocol_id, data, data_size);

    for(size_t i = 0; i < data_size; i++) {
        furi_string_cat_printf(display_text, "%s%02X", i != 0 ? " " : "", data[i]);
    }

    free(data);

    FuriString* rendered_data;
    rendered_data = furi_string_alloc();
    protocol_dict_render_data(app->dict, rendered_data, app->protocol_id);
    furi_string_cat_printf(display_text, "\n%s", furi_string_get_cstr(rendered_data));
    furi_string_free(rendered_data);

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(display_text));

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    furi_string_free(display_text);
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
