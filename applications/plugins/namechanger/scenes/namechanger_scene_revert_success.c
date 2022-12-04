#include "../namechanger.h"

static void namechanger_scene_revert_success_popup_callback(void* context) {
    NameChanger* namechanger = context;
    view_dispatcher_send_custom_event(namechanger->view_dispatcher, NameChangerCustomEventBack);
}

void namechanger_scene_revert_success_on_enter(void* context) {
    NameChanger* namechanger = context;
    Popup* popup = namechanger->popup;

    popup_set_icon(popup, 0, 2, &I_DolphinMafia_115x62);
    popup_set_header(popup, "Reverted!", 70, 5, AlignLeft, AlignTop);
    popup_set_text(popup, "Rebooting...", 70, 16, AlignLeft, AlignTop);

    popup_set_callback(popup, namechanger_scene_revert_success_popup_callback);
    popup_set_context(popup, namechanger);
    popup_set_timeout(popup, 5000);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(namechanger->view_dispatcher, NameChangerViewPopup);
}

bool namechanger_scene_revert_success_on_event(void* context, SceneManagerEvent event) {
    NameChanger* namechanger = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
        scene_manager_search_and_switch_to_previous_scene(
            namechanger->scene_manager, NameChangerSceneStart);
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == NameChangerCustomEventBack) {
            scene_manager_search_and_switch_to_previous_scene(
                namechanger->scene_manager, NameChangerSceneStart);
        }
    }

    return consumed;
}

void namechanger_scene_revert_success_on_exit(void* context) {
    NameChanger* namechanger = context;
    Popup* popup = namechanger->popup;

    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    popup_disable_timeout(popup);
    popup_set_context(popup, NULL);
    popup_set_callback(popup, NULL);

    furi_hal_power_reset();
}