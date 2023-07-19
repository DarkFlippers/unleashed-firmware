#include "../camera_suite.h"
#include "../helpers/camera_suite_custom_event.h"
#include "../views/camera_suite_view_guide.h"

void camera_suite_view_guide_callback(CameraSuiteCustomEvent event, void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void camera_suite_scene_guide_on_enter(void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    camera_suite_view_guide_set_callback(
        app->camera_suite_view_guide, camera_suite_view_guide_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, CameraSuiteViewIdGuide);
}

bool camera_suite_scene_guide_on_event(void* context, SceneManagerEvent event) {
    CameraSuite* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case CameraSuiteCustomEventSceneGuideLeft:
        case CameraSuiteCustomEventSceneGuideRight:
        case CameraSuiteCustomEventSceneGuideUp:
        case CameraSuiteCustomEventSceneGuideDown:
            // Do nothing.
            break;
        case CameraSuiteCustomEventSceneGuideBack:
            notification_message(app->notification, &sequence_reset_red);
            notification_message(app->notification, &sequence_reset_green);
            notification_message(app->notification, &sequence_reset_blue);
            if(!scene_manager_search_and_switch_to_previous_scene(
                   app->scene_manager, CameraSuiteSceneMenu)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
            break;
        }
    }

    return consumed;
}

void camera_suite_scene_guide_on_exit(void* context) {
    CameraSuite* app = context;
    UNUSED(app);
}