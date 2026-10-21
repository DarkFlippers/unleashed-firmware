#include "../t5577_app.h"
#include <bit_lib/bit_lib.h>

static void t5577_app_scene_password_callback(void* context) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventConfirm);
}

void t5577_app_scene_password_on_enter(void* context) {
    T5577App* app = context;

    bit_lib_num_to_bytes_be(app->password, sizeof(app->password_bytes), app->password_bytes);

    // Two callers, two questions: a read wants the password the tag has, a sweep wants the one
    // worth trying before the dictionary. Which one is asking is the scene state.
    const bool sweeping =
        scene_manager_get_scene_state(app->scene_manager, T5577AppScenePassword) != 0;

    byte_input_set_header_text(
        app->byte_input, sweeping ? "Password to try first" : "Password of the tag");
    byte_input_set_result_callback(
        app->byte_input,
        t5577_app_scene_password_callback,
        NULL,
        app,
        app->password_bytes,
        sizeof(app->password_bytes));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewByteInput);
}

bool t5577_app_scene_password_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventConfirm) return false;

    const uint32_t word =
        bit_lib_bytes_to_num_be(app->password_bytes, sizeof(app->password_bytes));
    const uint32_t next = scene_manager_get_scene_state(app->scene_manager, T5577AppScenePassword);

    if(next == 0) {
        app->password = word;
        app->password_entered = true;
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneRead);
        return true;
    }

    // The warning goes here rather than in the menu, so that it is the last thing between the
    // user and a tag being written, with the password already settled.
    if(t5577_app_ask(
           app,
           "Clear Password",
           "Apply tag to Flipper's\nback and don't move it\nwhile it runs",
           "Start")) {
        // Kept only now: a guess the user walked away from is not the password this tag has, and
        // that is what these two fields mean everywhere else.
        app->password = word;
        app->password_entered = true;
        scene_manager_next_scene(app->scene_manager, next);
    } else {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
    }

    return true;
}

void t5577_app_scene_password_on_exit(void* context) {
    T5577App* app = context;

    // Spent either way: left by Back or by a sweep starting, the next read to ask for a password
    // must not be sent off to that sweep.
    scene_manager_set_scene_state(app->scene_manager, T5577AppScenePassword, 0);

    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
