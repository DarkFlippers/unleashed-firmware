#include "../lfrfid_i.h"

void lfrfid_scene_read_success_on_enter(void* context) {
    LfRfid* app = context;
    Widget* widget = app->widget;

    FuriString* tmp_string;
    tmp_string = furi_string_alloc();

    widget_add_button_element(widget, GuiButtonTypeLeft, "Retry", lfrfid_widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "More", lfrfid_widget_callback, app);

    furi_string_printf(
        tmp_string,
        "%s[%s]",
        protocol_dict_get_name(app->dict, app->protocol_id),
        protocol_dict_get_manufacturer(app->dict, app->protocol_id));

    widget_add_string_element(
        widget, 16, 3, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(tmp_string));

    furi_string_reset(tmp_string);
    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = (uint8_t*)malloc(size);
    protocol_dict_get_data(app->dict, app->protocol_id, data, size);
    for(uint8_t i = 0; i < size; i++) {
        if(i >= 9) {
            furi_string_cat_printf(tmp_string, "..");
            break;
        } else {
            if(i != 0) {
                furi_string_cat_printf(tmp_string, ":");
            }
            furi_string_cat_printf(tmp_string, "%02X", data[i]);
        }
    }
    free(data);

    FuriString* render_data;
    render_data = furi_string_alloc();
    protocol_dict_render_brief_data(app->dict, render_data, app->protocol_id);
    furi_string_cat_printf(tmp_string, "\r\n%s", furi_string_get_cstr(render_data));
    furi_string_free(render_data);

    widget_add_string_multiline_element(
        widget, 0, 16, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(tmp_string));

    widget_add_icon_element(app->widget, 0, 0, &I_RFIDSmallChip_14x14);

    notification_message_block(app->notifications, &sequence_set_green_255);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
    furi_string_free(tmp_string);
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
