#include "../subghz_remote_app_i.h"
#include "../views/remote.h"

#define TAG "SubRemScenRemote"

void subghz_scene_edit_preview_save_popup_callback(void* context) {
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(
        app->view_dispatcher, SubRemCustomEventSceneEditPreviewSaved);
}

void subrem_scene_edit_preview_callback(SubRemCustomEvent event, void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void subrem_scene_edit_preview_on_enter(void* context) {
    SubGhzRemoteApp* app = context;

    // Setup view
    Popup* popup = app->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, app);
    popup_set_callback(popup, subghz_scene_edit_preview_save_popup_callback);
    popup_enable_timeout(popup);

    subrem_view_remote_update_data_labels(app->subrem_remote_view, app->map_preset->subs_preset);
    subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateOFF, 0);

    subrem_view_remote_set_callback(
        app->subrem_remote_view, subrem_scene_edit_preview_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDRemote);
}

bool subrem_scene_edit_preview_on_event(void* context, SceneManagerEvent event) {
    SubGhzRemoteApp* app = context;

    if(event.type == SceneManagerEventTypeBack ||
       (event.type == SceneManagerEventTypeCustom &&
        (event.event == SubRemCustomEventViewRemoteStartLEFT ||
         event.event == SubRemCustomEventViewRemoteForcedStop))) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    } else if(
        event.type == SceneManagerEventTypeCustom &&
        (event.event == SubRemCustomEventViewRemoteStartRIGHT ||
         event.event == SubRemCustomEventViewRemoteStartOK)) {
        if(subrem_save_map_to_file(app)) {
            view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDPopup);
            app->map_not_saved = false;
            return true;
        }
        // TODO error screen
        return true;
    } else if(
        event.type == SceneManagerEventTypeCustom &&
        event.event == SubRemCustomEventSceneEditPreviewSaved) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SubRemSceneEditMenu);
    }
    // } else if(event.type == SceneManagerEventTypeTick) {
    // }
    return false;
}

void subrem_scene_edit_preview_on_exit(void* context) {
    SubGhzRemoteApp* app = context;

    subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateIdle, 0);
    popup_reset(app->popup);
}
