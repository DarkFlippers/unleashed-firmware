#include "../avr_isp_app_i.h"

void avr_isp_scene_writer_callback(AvrIspCustomEvent event, void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void avr_isp_scene_writer_on_enter(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    avr_isp_writer_set_file_path(
        app->avr_isp_writer_view, furi_string_get_cstr(app->file_path), app->file_name_tmp);
    avr_isp_writer_view_set_callback(app->avr_isp_writer_view, avr_isp_scene_writer_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AvrIspViewWriter);
}

bool avr_isp_scene_writer_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    AvrIspApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeBack) {
        //do not handle exit on "Back"
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case AvrIspCustomEventSceneExitStartMenu:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, AvrIspSceneStart);
            consumed = true;
            break;
        case AvrIspCustomEventSceneExit:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, AvrIspSceneChipDetect);
            consumed = true;
            break;
        case AvrIspCustomEventSceneErrorVerification:
            app->error = AvrIspErrorVerification;
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, AvrIspSceneChipDetect);
            consumed = true;
            break;
        case AvrIspCustomEventSceneErrorWriting:
            app->error = AvrIspErrorWriting;
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, AvrIspSceneChipDetect);
            consumed = true;
            break;
        case AvrIspCustomEventSceneErrorWritingFuse:
            app->error = AvrIspErrorWritingFuse;
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, AvrIspSceneChipDetect);
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        avr_isp_writer_update_progress(app->avr_isp_writer_view);
    }
    return consumed;
}

void avr_isp_scene_writer_on_exit(void* context) {
    UNUSED(context);
}
