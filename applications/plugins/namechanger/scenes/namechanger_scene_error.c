#include "../namechanger.h"

static void namechanger_scene_error_popup_callback(void* context) {
    NameChanger* namechanger = context;
    view_dispatcher_send_custom_event(namechanger->view_dispatcher, NameChangerCustomEventBack);
}

void namechanger_scene_error_on_enter(void* context) {
    NameChanger* namechanger = context;
    Popup* popup = namechanger->popup;

    popup_set_icon(popup, 60, 12, &I_Cry_dolph_55x52);
    popup_set_header(popup, "Error", 5, 7, AlignLeft, AlignTop);

    popup_set_text(popup, furi_string_get_cstr(namechanger->error), 5, 20, AlignLeft, AlignTop);

    popup_set_callback(popup, namechanger_scene_error_popup_callback);
    popup_set_context(popup, namechanger);
    popup_set_timeout(popup, 10000);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(namechanger->view_dispatcher, NameChangerViewPopup);
}

bool namechanger_scene_error_on_event(void* context, SceneManagerEvent event) {
    NameChanger* namechanger = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == NameChangerCustomEventBack) {
            view_dispatcher_stop(namechanger->view_dispatcher);
        }
    }

    return consumed;
}

void namechanger_scene_error_on_exit(void* context) {
    NameChanger* namechanger = context;
    Popup* popup = namechanger->popup;

    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    popup_disable_timeout(popup);
    popup_set_context(popup, NULL);
    popup_set_callback(popup, NULL);
}
