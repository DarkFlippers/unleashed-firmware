#include "../avr_isp_app_i.h"

void avr_isp_scene_success_popup_callback(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, AvrIspCustomEventSceneSuccess);
}

void avr_isp_scene_success_on_enter(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    Popup* popup = app->popup;
    popup_set_icon(popup, 32, 5, &I_dolphin_nice_96x59);
    popup_set_header(popup, "Success!", 8, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, app);
    popup_set_callback(popup, avr_isp_scene_success_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(app->view_dispatcher, AvrIspViewPopup);
}

bool avr_isp_scene_success_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    AvrIspApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == AvrIspCustomEventSceneSuccess) {
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, AvrIspSceneStart);
            return true;
        }
    }
    return false;
}

void avr_isp_scene_success_on_exit(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    Popup* popup = app->popup;
    popup_reset(popup);
}
