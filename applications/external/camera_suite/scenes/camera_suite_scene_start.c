#include "../camera_suite.h"
#include "../helpers/camera_suite_custom_event.h"
#include "../views/camera_suite_view_start.h"

void camera_suite_scene_start_callback(CameraSuiteCustomEvent event, void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void camera_suite_scene_start_on_enter(void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    camera_suite_view_start_set_callback(
        app->camera_suite_view_start, camera_suite_scene_start_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, CameraSuiteViewIdStartscreen);
}

bool camera_suite_scene_start_on_event(void* context, SceneManagerEvent event) {
    CameraSuite* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case CameraSuiteCustomEventStartLeft:
        case CameraSuiteCustomEventStartRight:
            break;
        case CameraSuiteCustomEventStartUp:
        case CameraSuiteCustomEventStartDown:
            break;
        case CameraSuiteCustomEventStartOk:
            scene_manager_next_scene(app->scene_manager, CameraSuiteSceneMenu);
            consumed = true;
            break;
        case CameraSuiteCustomEventStartBack:
            notification_message(app->notification, &sequence_reset_red);
            notification_message(app->notification, &sequence_reset_green);
            notification_message(app->notification, &sequence_reset_blue);
            if(!scene_manager_search_and_switch_to_previous_scene(
                   app->scene_manager, CameraSuiteSceneStart)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
            break;
        }
    }

    return consumed;
}

void camera_suite_scene_start_on_exit(void* context) {
    CameraSuite* app = context;
    UNUSED(app);
}