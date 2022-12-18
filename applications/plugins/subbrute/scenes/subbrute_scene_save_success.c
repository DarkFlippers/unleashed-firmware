#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"

void subbrute_scene_save_success_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = context;

    // Setup view
    Popup* popup = instance->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, instance);
    popup_set_callback(popup, subbrute_popup_closed_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(instance->view_dispatcher, SubBruteViewPopup);
}

bool subbrute_scene_save_success_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    //SubBruteMainView* view = instance->view_main;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubBruteCustomEventTypePopupClosed) {
            if(!scene_manager_search_and_switch_to_previous_scene(
                   instance->scene_manager, SubBruteSceneSetupAttack)) {
                scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);
            }
            return true;
        }
    }
    return false;
}

void subbrute_scene_save_success_on_exit(void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;

    // Clear view
    Popup* popup = instance->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
    popup_set_callback(popup, NULL);
    popup_set_context(popup, NULL);
    popup_set_timeout(popup, 0);
    popup_disable_timeout(popup);
}
