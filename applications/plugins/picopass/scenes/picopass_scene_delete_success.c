#include "../picopass_i.h"

void picopass_scene_delete_success_popup_callback(void* context) {
    Picopass* picopass = context;
    view_dispatcher_send_custom_event(picopass->view_dispatcher, PicopassCustomEventViewExit);
}

void picopass_scene_delete_success_on_enter(void* context) {
    Picopass* picopass = context;

    // Setup view
    Popup* popup = picopass->popup;
    popup_set_icon(popup, 0, 2, &I_DolphinMafia_115x62);
    popup_set_header(popup, "Deleted", 83, 19, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, picopass);
    popup_set_callback(popup, picopass_scene_delete_success_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewPopup);
}

bool picopass_scene_delete_success_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PicopassCustomEventViewExit) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                picopass->scene_manager, PicopassSceneStart);
        }
    }
    return consumed;
}

void picopass_scene_delete_success_on_exit(void* context) {
    Picopass* picopass = context;

    // Clear view
    popup_reset(picopass->popup);
}
