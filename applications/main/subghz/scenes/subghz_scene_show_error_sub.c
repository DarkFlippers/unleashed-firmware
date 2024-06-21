#include "../subghz_i.h" // IWYU pragma: keep
#include "../helpers/subghz_custom_event.h"

void subghz_scene_show_error_sub_popup_callback(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneShowErrorSub);
}

void subghz_scene_show_error_sub_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    Popup* popup = subghz->popup;
    popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
    popup_set_header(popup, furi_string_get_cstr(subghz->error_str), 14, 15, AlignLeft, AlignTop);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, subghz);
    popup_set_callback(popup, subghz_scene_show_error_sub_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdPopup);

    notification_message(subghz->notifications, &sequence_set_red_255);
}

bool subghz_scene_show_error_sub_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneShowErrorSub) {
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
            return true;
        }
    }
    return false;
}

void subghz_scene_show_error_sub_on_exit(void* context) {
    SubGhz* subghz = context;
    Popup* popup = subghz->popup;

    popup_reset(popup);

    furi_string_reset(subghz->error_str);

    notification_message(subghz->notifications, &sequence_reset_rgb);
}
