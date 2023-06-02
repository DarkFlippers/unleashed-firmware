#include "../fuzzer_i.h"
#include "../helpers/fuzzer_custom_event.h"

void fuzzer_scene_main_callback(FuzzerCustomEvent event, void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void fuzzer_scene_main_on_enter(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    fuzzer_view_main_set_callback(app->main_view, fuzzer_scene_main_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, FuzzerViewIDMain);
}

bool fuzzer_scene_main_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FuzzerCustomEventViewMainBack) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
        }
    }

    return consumed;
}

void fuzzer_scene_main_on_exit(void* context) {
    // furi_assert(context);
    // PacsFuzzerApp* app = context;
    UNUSED(context);
}
