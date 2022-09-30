#include "../ibutton_i.h"
#include <dolphin/dolphin.h>

static void ibutton_scene_save_success_popup_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventBack);
}

void ibutton_scene_save_success_on_enter(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;
    DOLPHIN_DEED(DolphinDeedIbuttonSave);

    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 5, 7, AlignLeft, AlignTop);

    popup_set_callback(popup, ibutton_scene_save_success_popup_callback);
    popup_set_context(popup, ibutton);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);
}

bool ibutton_scene_save_success_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventBack) {
            scene_manager_search_and_switch_to_another_scene(
                ibutton->scene_manager, iButtonSceneSelectKey);
        }
    }

    return consumed;
}

void ibutton_scene_save_success_on_exit(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;

    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    popup_disable_timeout(popup);
    popup_set_context(popup, NULL);
    popup_set_callback(popup, NULL);
}
