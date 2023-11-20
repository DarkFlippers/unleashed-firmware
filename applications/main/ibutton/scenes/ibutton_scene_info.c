#include "../ibutton_i.h"

void ibutton_scene_info_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    Widget* widget = ibutton->widget;

    const iButtonProtocolId protocol_id = ibutton_key_get_protocol_id(key);

    FuriString* tmp = furi_string_alloc();
    FuriString* keynumber = furi_string_alloc();

    ibutton_protocols_render_brief_data(ibutton->protocols, key, keynumber);

    furi_string_printf(
        tmp,
        "\e#%s\n[%s]\e#\n%s",
        ibutton->key_name,
        ibutton_protocols_get_name(ibutton->protocols, protocol_id),
        furi_string_get_cstr(keynumber));

    widget_add_text_box_element(
        widget, 0, 2, 128, 64, AlignLeft, AlignTop, furi_string_get_cstr(tmp), true);

    if(ibutton_protocols_get_features(ibutton->protocols, protocol_id) &
       iButtonProtocolFeatureExtData) {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "More", ibutton_widget_callback, context);
    }

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
    furi_string_free(tmp);
    furi_string_free(keynumber);
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
