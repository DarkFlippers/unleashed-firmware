#include "../subghz_test_app_i.h"
#include <subghz_test_icons.h>

void subghz_test_scene_show_only_rx_popup_callback(void* context) {
    SubGhzTestApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SubGhzTestCustomEventSceneShowOnlyRX);
}

void subghz_test_scene_show_only_rx_on_enter(void* context) {
    SubGhzTestApp* app = context;

    // Setup view
    Popup* popup = app->popup;

    const char* header_text = "Transmission is blocked";
    const char* message_text = "Transmission on\nthis frequency is\nrestricted in\nyour region";
    if(!furi_hal_region_is_provisioned()) {
        header_text = "Firmware update needed";
        message_text = "Please update\nfirmware before\nusing this feature\nflipp.dev/upd";
    }

    popup_set_header(popup, header_text, 63, 3, AlignCenter, AlignTop);
    popup_set_text(popup, message_text, 0, 17, AlignLeft, AlignTop);
    popup_set_icon(popup, 72, 17, &I_DolphinCommon_56x48);

    popup_set_timeout(popup, 1500);
    popup_set_context(popup, app);
    popup_set_callback(popup, subghz_test_scene_show_only_rx_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzTestViewPopup);
}

bool subghz_test_scene_show_only_rx_on_event(void* context, SceneManagerEvent event) {
    SubGhzTestApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzTestCustomEventSceneShowOnlyRX) {
            scene_manager_previous_scene(app->scene_manager);
            return true;
        }
    }
    return false;
}

void subghz_test_scene_show_only_rx_on_exit(void* context) {
    SubGhzTestApp* app = context;
    Popup* popup = app->popup;

    popup_reset(popup);
}
