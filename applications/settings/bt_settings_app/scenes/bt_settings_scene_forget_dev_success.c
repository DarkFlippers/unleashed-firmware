#include "../bt_settings_app.h"
#include <furi_hal_bt.h>

void bt_settings_app_scene_forget_dev_success_popup_callback(void* context) {
    BtSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, BtSettingsCustomEventExitView);
}

void bt_settings_scene_forget_dev_success_on_enter(void* context) {
    BtSettingsApp* app = context;
    Popup* popup = app->popup;

    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Done", 14, 15, AlignLeft, AlignTop);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, app);
    popup_set_callback(popup, bt_settings_app_scene_forget_dev_success_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(app->view_dispatcher, BtSettingsAppViewPopup);
}

bool bt_settings_scene_forget_dev_success_on_event(void* context, SceneManagerEvent event) {
    BtSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BtSettingsCustomEventExitView) {
            if(scene_manager_has_previous_scene(app->scene_manager, BtSettingsAppSceneStart)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    app->scene_manager, BtSettingsAppSceneStart);
            }
        }
    }

    return consumed;
}

void bt_settings_scene_forget_dev_success_on_exit(void* context) {
    BtSettingsApp* app = context;
    popup_reset(app->popup);
}
