#include "../lfrfid_i.h"

#define LFRFID_SCENE_DELETE_MAX_HEX_WIDTH (7UL)

void lfrfid_scene_delete_confirm_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    FuriString* display_text = furi_string_alloc_printf(
        "\e#Delete %s?\e#\n"
        "Hex: ",
        furi_string_get_cstr(app->file_name));

    const size_t data_size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = malloc(data_size);

    protocol_dict_get_data(app->dict, app->protocol_id, data, data_size);

    for(size_t i = 0; i < data_size; i++) {
        if(i == LFRFID_SCENE_DELETE_MAX_HEX_WIDTH) {
            furi_string_cat(display_text, " ...");
            break;
        }

        furi_string_cat_printf(display_text, "%s%02X", i != 0 ? " " : "", data[i]);
    }

    furi_string_push_back(display_text, '\n');

    free(data);

    const char* protocol = protocol_dict_get_name(app->dict, app->protocol_id);
    const char* manufacturer = protocol_dict_get_manufacturer(app->dict, app->protocol_id);

    if(strcasecmp(protocol, manufacturer) != 0 && strcasecmp(manufacturer, "N/A") != 0) {
        furi_string_cat_printf(display_text, "%s ", manufacturer);
    }

    furi_string_cat(display_text, protocol);

    widget_add_text_box_element(
        widget, 0, 0, 128, 64, AlignCenter, AlignTop, furi_string_get_cstr(display_text), true);
    widget_add_button_element(widget, GuiButtonTypeLeft, "Cancel", lfrfid_widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Delete", lfrfid_widget_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    furi_string_free(display_text);
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
