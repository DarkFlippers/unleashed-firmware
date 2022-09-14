#include "../picopass_i.h"
#include <dolphin/dolphin.h>

void picopass_scene_save_success_popup_callback(void* context) {
    Picopass* picopass = context;
    view_dispatcher_send_custom_event(picopass->view_dispatcher, PicopassCustomEventViewExit);
}

void picopass_scene_save_success_on_enter(void* context) {
    Picopass* picopass = context;
    DOLPHIN_DEED(DolphinDeedNfcSave);

    // Setup view
    Popup* popup = picopass->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, picopass);
    popup_set_callback(popup, picopass_scene_save_success_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewPopup);
}

bool picopass_scene_save_success_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PicopassCustomEventViewExit) {
            if(scene_manager_has_previous_scene(picopass->scene_manager, PicopassSceneCardMenu)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    picopass->scene_manager, PicopassSceneCardMenu);
            } else {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    picopass->scene_manager, PicopassSceneStart);
            }
        }
    }
    return consumed;
}

void picopass_scene_save_success_on_exit(void* context) {
    Picopass* picopass = context;

    // Clear view
    popup_reset(picopass->popup);
}
