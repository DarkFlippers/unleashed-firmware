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

    const char* header_text = "Transmission is blocked";
    const char* message_text = "This frequency is\noutside of default\nrange";

    popup_set_header(popup, header_text, 63, 3, AlignCenter, AlignTop);
    popup_set_text(popup, message_text, 0, 17, AlignLeft, AlignTop);
    popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);

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
    Popup* popup = subghz->popup;

    popup_reset(popup);
}
