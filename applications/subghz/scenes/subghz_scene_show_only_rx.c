#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_show_only_rx_popup_callback(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneShowOnlyRX);
}

void subghz_scene_show_only_rx_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    Popup* popup = subghz->popup;
    popup_set_icon(popup, 67, 12, &I_DolphinFirstStart7_61x51);
    popup_set_text(
        popup,
        "This frequency can\nonly be used for RX\nin your region",
        38,
        40,
        AlignCenter,
        AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, subghz);
    popup_set_callback(popup, subghz_scene_show_only_rx_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdPopup);
}

bool subghz_scene_show_only_rx_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneShowOnlyRX) {
            scene_manager_previous_scene(subghz->scene_manager);
            return true;
        }
    }
    return false;
}

void subghz_scene_show_only_rx_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    Popup* popup = subghz->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
    popup_set_callback(popup, NULL);
    popup_set_context(popup, NULL);
    popup_set_timeout(popup, 0);
    popup_disable_timeout(popup);
}
