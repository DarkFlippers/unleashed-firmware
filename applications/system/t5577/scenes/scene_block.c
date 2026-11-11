#include "../t5577_app.h"
#include <bit_lib/bit_lib.h>

static void t5577_app_scene_block_callback(void* context) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventConfirm);
}

void t5577_app_scene_block_on_enter(void* context) {
    T5577App* app = context;

    const T5577Target target = t5577_app_edit_block(app);

    bit_lib_num_to_bytes_be(
        app->dump.block[target.page][target.block], sizeof(app->edit_bytes), app->edit_bytes);

    // Which block is being typed into, since the keyboard covers the list that said so.
    snprintf(app->name, sizeof(app->name), "Page %u block %u", target.page, target.block);
    byte_input_set_header_text(app->byte_input, app->name);
    byte_input_set_result_callback(
        app->byte_input,
        t5577_app_scene_block_callback,
        NULL,
        app,
        app->edit_bytes,
        sizeof(app->edit_bytes));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewByteInput);
}

bool t5577_app_scene_block_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventConfirm) return false;

    const T5577Target target = t5577_app_edit_block(app);

    // Setting a block also marks it as one the dump holds, so a block that was never read
    // becomes one a write puts back - which is the point of typing it in.
    t5577_dump_set(
        &app->dump,
        target.page,
        target.block,
        bit_lib_bytes_to_num_be(app->edit_bytes, sizeof(app->edit_bytes)));

    scene_manager_previous_scene(app->scene_manager);

    return true;
}

void t5577_app_scene_block_on_exit(void* context) {
    T5577App* app = context;
    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
