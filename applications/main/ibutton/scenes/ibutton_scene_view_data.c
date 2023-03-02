#include "../ibutton_i.h"

void ibutton_scene_view_data_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    Widget* widget = ibutton->widget;

    FuriString* tmp = furi_string_alloc();
    ibutton_protocols_render_data(ibutton->protocols, key, tmp);

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(tmp));

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
    furi_string_free(tmp);
}

bool ibutton_scene_view_data_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void ibutton_scene_view_data_on_exit(void* context) {
    iButton* ibutton = context;
    widget_reset(ibutton->widget);
}
