#include "../lfrfid_i.h"

#define LFRFID_SCENE_READ_SUCCESS_MAX_HEX_WIDTH (7UL)

void lfrfid_scene_read_success_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;
    FuriString* display_text = furi_string_alloc();

    const char* protocol = protocol_dict_get_name(app->dict, app->protocol_id);
    const char* manufacturer = protocol_dict_get_manufacturer(app->dict, app->protocol_id);

    if(strcasecmp(protocol, manufacturer) != 0 && strcasecmp(manufacturer, "N/A") != 0) {
        furi_string_printf(display_text, "\e#%s %s\e#", manufacturer, protocol);
    } else {
        furi_string_printf(display_text, "\e#%s\e#", protocol);
    }

    furi_string_cat(display_text, "\nHex: ");

    const size_t data_size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = malloc(data_size);

    protocol_dict_get_data(app->dict, app->protocol_id, data, data_size);

    for(size_t i = 0; i < data_size; i++) {
        if(i == LFRFID_SCENE_READ_SUCCESS_MAX_HEX_WIDTH) {
            furi_string_cat(display_text, " ...");
            break;
        }

        furi_string_cat_printf(display_text, "%s%02X", i != 0 ? " " : "", data[i]);
    }

    free(data);

    FuriString* rendered_data = furi_string_alloc();
    protocol_dict_render_brief_data(app->dict, rendered_data, app->protocol_id);
    furi_string_cat_printf(display_text, "\n%s", furi_string_get_cstr(rendered_data));
    furi_string_free(rendered_data);

    widget_add_text_box_element(
        widget, 0, 0, 128, 52, AlignLeft, AlignTop, furi_string_get_cstr(display_text), true);
    widget_add_button_element(widget, GuiButtonTypeLeft, "Retry", lfrfid_widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "More", lfrfid_widget_callback, app);

    notification_message_block(app->notifications, &sequence_set_green_255);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    furi_string_free(display_text);
}

bool lfrfid_scene_read_success_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(scene_manager, LfRfidSceneExitConfirm);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(scene_manager, LfRfidSceneRetryConfirm);
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(scene_manager, LfRfidSceneReadKeyMenu);
        }
    }

    return consumed;
}

void lfrfid_scene_read_success_on_exit(void* context) {
    LfRfid* app = context;
    notification_message_block(app->notifications, &sequence_reset_green);
    widget_reset(app->widget);
}
