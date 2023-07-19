#include "../camera_suite.h"
#include "../helpers/camera_suite_custom_event.h"
#include "../helpers/camera_suite_haptic.h"
#include "../helpers/camera_suite_led.h"
#include "../views/camera_suite_view_style_2.h"

void camera_suite_view_style_2_callback(CameraSuiteCustomEvent event, void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void camera_suite_scene_style_2_on_enter(void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    camera_suite_view_style_2_set_callback(
        app->camera_suite_view_style_2, camera_suite_view_style_2_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, CameraSuiteViewIdScene2);
}

bool camera_suite_scene_style_2_on_event(void* context, SceneManagerEvent event) {
    CameraSuite* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case CameraSuiteCustomEventSceneStyle2Left:
        case CameraSuiteCustomEventSceneStyle2Right:
        case CameraSuiteCustomEventSceneStyle2Up:
        case CameraSuiteCustomEventSceneStyle2Down:
        case CameraSuiteCustomEventSceneStyle2Ok:
            // Do nothing.
            break;
        case CameraSuiteCustomEventSceneStyle2Back:
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

void camera_suite_scene_style_2_on_exit(void* context) {
    CameraSuite* app = context;
    UNUSED(app);
}
