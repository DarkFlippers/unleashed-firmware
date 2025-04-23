#include "../findmy_i.h"

void findmy_scene_main_callback(FindMyMainEvent event, void* context) {
    furi_assert(context);
    FindMy* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void findmy_scene_main_on_enter(void* context) {
    FindMy* app = context;

    findmy_main_set_callback(app->findmy_main, findmy_scene_main_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FindMyViewMain);
}

bool findmy_scene_main_on_event(void* context, SceneManagerEvent event) {
    FindMy* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case FindMyMainEventToggle:
            findmy_toggle_beacon(app);
            break;
        case FindMyMainEventBackground:
            app->state.beacon_active = true;
            findmy_state_save_and_apply(&app->state);
            view_dispatcher_stop(app->view_dispatcher);
            break;
        case FindMyMainEventConfig:
            scene_manager_next_scene(app->scene_manager, FindMySceneConfig);
            break;
        case FindMyMainEventIntervalUp:
            findmy_change_broadcast_interval(app, app->state.broadcast_interval + 1);
            break;
        case FindMyMainEventIntervalDown:
            findmy_change_broadcast_interval(app, app->state.broadcast_interval - 1);
            break;
        case FindMyMainEventQuit:
            app->state.beacon_active = false;
            findmy_state_save_and_apply(&app->state);
            break;
        default:
            consumed = false;
            break;
        }
    }

    return consumed;
}

void findmy_scene_main_on_exit(void* context) {
    FindMy* app = context;
    UNUSED(app);
}
