#include "../ibutton_i.h"

void ibutton_scene_info_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    Widget* widget = ibutton->widget;

    const iButtonProtocolId protocol_id = ibutton_key_get_protocol_id(key);

    FuriString* tmp = furi_string_alloc();

    furi_string_printf(
        tmp,
        "\e#%s [%s]\e#",
        ibutton->key_name,
        ibutton_protocols_get_name(ibutton->protocols, protocol_id));

    widget_add_text_box_element(
        widget, 0, 2, 128, 12, AlignLeft, AlignTop, furi_string_get_cstr(tmp), true);

    furi_string_reset(tmp);
    ibutton_protocols_render_brief_data(ibutton->protocols, key, tmp);

    widget_add_string_multiline_element(
        widget, 0, 16, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(tmp));

    if(ibutton_protocols_get_features(ibutton->protocols, protocol_id) &
       iButtonProtocolFeatureExtData) {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "More", ibutton_widget_callback, context);
    }

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
    furi_string_free(tmp);
}

bool ibutton_scene_info_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneViewData);
        }
    }

    return consumed;
}

void ibutton_scene_info_on_exit(void* context) {
    iButton* ibutton = context;
    widget_reset(ibutton->widget);
}
