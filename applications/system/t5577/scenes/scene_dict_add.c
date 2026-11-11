#include "../t5577_app.h"
#include <bit_lib/bit_lib.h>

static void t5577_app_scene_dict_add_callback(void* context) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventConfirm);
}

void t5577_app_scene_dict_add_on_enter(void* context) {
    T5577App* app = context;

    byte_input_set_header_text(app->byte_input, "Password in hex");
    byte_input_set_result_callback(
        app->byte_input,
        t5577_app_scene_dict_add_callback,
        NULL,
        app,
        app->dict_bytes,
        sizeof(app->dict_bytes));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewByteInput);
}

bool t5577_app_scene_dict_add_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventConfirm) return false;

    // The same helper the sniff screen's Save uses, so a word typed in and a word recovered off
    // the air are checked against the same two lists and get the same four answers.
    t5577_app_dict_save(app, bit_lib_bytes_to_num_be(app->dict_bytes, sizeof(app->dict_bytes)));

    // Back to the menu either way, so the counts above are redrawn with whatever just happened.
    scene_manager_previous_scene(app->scene_manager);

    return true;
}

void t5577_app_scene_dict_add_on_exit(void* context) {
    T5577App* app = context;

    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
