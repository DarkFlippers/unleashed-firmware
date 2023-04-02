#include "../avr_isp_app_i.h"

void avr_isp_scene_chip_detect_callback(AvrIspCustomEvent event, void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void avr_isp_scene_chip_detect_on_enter(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    switch(app->error) {
    case AvrIspErrorReading:
    case AvrIspErrorWriting:
    case AvrIspErrorWritingFuse:
        avr_isp_chip_detect_set_state(
            app->avr_isp_chip_detect_view, AvrIspChipDetectViewStateErrorOccured);
        break;
    case AvrIspErrorVerification:
        avr_isp_chip_detect_set_state(
            app->avr_isp_chip_detect_view, AvrIspChipDetectViewStateErrorVerification);
        break;

    default:
        avr_isp_chip_detect_set_state(
            app->avr_isp_chip_detect_view, AvrIspChipDetectViewStateNoDetect);
        break;
    }
    app->error = AvrIspErrorNoError;
    avr_isp_chip_detect_view_set_callback(
        app->avr_isp_chip_detect_view, avr_isp_scene_chip_detect_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, AvrIspViewChipDetect);
}

bool avr_isp_scene_chip_detect_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    AvrIspApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case AvrIspCustomEventSceneChipDetectOk:

            if(scene_manager_get_scene_state(app->scene_manager, AvrIspSceneChipDetect) ==
               AvrIspViewProgrammer) {
                scene_manager_next_scene(app->scene_manager, AvrIspSceneProgrammer);
            } else if(
                scene_manager_get_scene_state(app->scene_manager, AvrIspSceneChipDetect) ==
                AvrIspViewReader) {
                scene_manager_next_scene(app->scene_manager, AvrIspSceneInputName);
            } else if(
                scene_manager_get_scene_state(app->scene_manager, AvrIspSceneChipDetect) ==
                AvrIspViewWriter) {
                scene_manager_next_scene(app->scene_manager, AvrIspSceneLoad);
            }

            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
    }
    return consumed;
}

void avr_isp_scene_chip_detect_on_exit(void* context) {
    UNUSED(context);
}
