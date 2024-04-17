#include "../ibutton_i.h"

void ibutton_scene_info_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    Widget* widget = ibutton->widget;

    const iButtonProtocolId protocol_id = ibutton_key_get_protocol_id(key);

    FuriString* tmp = furi_string_alloc();
    FuriString* brief_data = furi_string_alloc();

    furi_string_printf(
        tmp,
        "Name:%s\n\e#%s %s\e#\n",
        ibutton->key_name,
        ibutton_protocols_get_manufacturer(ibutton->protocols, protocol_id),
        ibutton_protocols_get_name(ibutton->protocols, protocol_id));

    ibutton_protocols_render_brief_data(ibutton->protocols, key, brief_data);

    furi_string_cat(tmp, brief_data);

    widget_add_text_box_element(
        widget, 0, 0, 128, 64, AlignLeft, AlignTop, furi_string_get_cstr(tmp), false);

    furi_string_reset(tmp);
    furi_string_reset(brief_data);

    if(ibutton_protocols_get_features(ibutton->protocols, protocol_id) &
       iButtonProtocolFeatureExtData) {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "More", ibutton_widget_callback, context);
    }

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
    furi_string_free(tmp);
    furi_string_free(brief_data);
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
