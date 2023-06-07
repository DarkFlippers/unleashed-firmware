#include "../fuzzer_i.h"
#include "../helpers/fuzzer_custom_event.h"

void fuzzer_scene_field_editor_callback(FuzzerCustomEvent event, void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void fuzzer_scene_field_editor_on_enter(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    fuzzer_view_field_editor_set_callback(
        app->field_editor_view, fuzzer_scene_field_editor_callback, app);

    fuzzer_worker_get_current_key(app->worker, app->payload);

    switch(scene_manager_get_scene_state(app->scene_manager, FuzzerSceneFieldEditor)) {
    case FuzzerFieldEditorStateEditingOn:
        fuzzer_view_field_editor_reset_data(app->field_editor_view, app->payload, true);
        break;

    case FuzzerFieldEditorStateEditingOff:
        fuzzer_view_field_editor_reset_data(app->field_editor_view, app->payload, false);
        break;

    default:
        break;
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FuzzerViewIDFieldEditor);
}

bool fuzzer_scene_field_editor_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FuzzerCustomEventViewFieldEditorBack) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
        } else if(event.event == FuzzerCustomEventViewFieldEditorOk) {
            fuzzer_view_field_editor_get_uid(app->field_editor_view, app->payload);
            if(fuzzer_worker_init_attack_bf_byte(
                   app->worker,
                   app->fuzzer_state.proto_index,
                   app->payload,
                   fuzzer_view_field_editor_get_index(app->field_editor_view))) {
                scene_manager_next_scene(app->scene_manager, FuzzerSceneAttack);
            }
        }
    }

    return consumed;
}

void fuzzer_scene_field_editor_on_exit(void* context) {
    // furi_assert(context);
    // PacsFuzzerApp* app = context;
    UNUSED(context);
}
