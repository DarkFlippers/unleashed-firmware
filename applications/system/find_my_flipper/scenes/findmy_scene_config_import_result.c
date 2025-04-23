#include "../findmy_i.h"

enum PopupEvent {
    PopupEventExit,
};

static void findmy_scene_config_import_result_callback(void* context) {
    FindMy* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, PopupEventExit);
}

void findmy_scene_config_import_result_on_enter(void* context) {
    FindMy* app = context;
    Popup* popup = app->popup;

    const char* error = (const char*)scene_manager_get_scene_state(
        app->scene_manager, FindMySceneConfigImportResult);
    if(error) {
        popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
        popup_set_header(popup, "Error!", 13, 22, AlignLeft, AlignBottom);
        popup_set_text(popup, error, 6, 26, AlignLeft, AlignTop);
        popup_disable_timeout(popup);
    } else {
        popup_set_icon(popup, 36, 5, &I_DolphinDone_80x58);
        popup_set_header(popup, "Imported!", 7, 14, AlignLeft, AlignBottom);
        popup_enable_timeout(popup);
    }
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, app);
    popup_set_callback(popup, findmy_scene_config_import_result_callback);
    findmy_main_update_active(app->findmy_main, app->state.beacon_active);
    findmy_main_update_mac(app->findmy_main, app->state.mac);
    findmy_main_update_type(app->findmy_main, app->state.tag_type);
    view_dispatcher_switch_to_view(app->view_dispatcher, FindMyViewPopup);
}

bool findmy_scene_config_import_result_on_event(void* context, SceneManagerEvent event) {
    FindMy* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case PopupEventExit:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, FindMySceneConfig);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void findmy_scene_config_import_result_on_exit(void* context) {
    FindMy* app = context;
    popup_reset(app->popup);
}
