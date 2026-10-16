#include "../t5577_app.h"
#include <bit_lib/bit_lib.h>

static void t5577_app_scene_password_callback(void* context) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventConfirm);
}

void t5577_app_scene_password_on_enter(void* context) {
    T5577App* app = context;

    bit_lib_num_to_bytes_be(app->password, sizeof(app->password_bytes), app->password_bytes);

    byte_input_set_header_text(app->byte_input, "Password of the tag");
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

    app->password = bit_lib_bytes_to_num_be(app->password_bytes, sizeof(app->password_bytes));
    app->password_entered = true;

    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneRead);

    return true;
}

void t5577_app_scene_password_on_exit(void* context) {
    T5577App* app = context;
    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
