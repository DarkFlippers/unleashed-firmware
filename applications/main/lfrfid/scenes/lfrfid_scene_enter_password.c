#include "../lfrfid_i.h"
#include "gui/scene_manager.h"

int next_scene;

void lfrfid_scene_enter_password_on_enter(void* context) {
    LfRfid* app = context;
    ByteInput* byte_input = app->byte_input;

    next_scene = scene_manager_get_scene_state(app->scene_manager, LfRfidSceneEnterPassword);

    bool password_set = app->password[0] | app->password[1] | app->password[2] | app->password[3];

    if(next_scene == LfRfidSceneWriteAndSetPass && !password_set) {
        uint8_t password_list_size;
        const uint32_t* password_list = lfrfid_get_t5577_default_passwords(&password_list_size);
        uint32_t pass = password_list[furi_get_tick() % password_list_size];

        for(uint8_t i = 0; i < 4; i++) app->password[4 - (i + 1)] = (pass >> (8 * i)) & 0xFF;
    }

    byte_input_set_header_text(byte_input, "Enter the password in hex");

    byte_input_set_result_callback(
        byte_input, lfrfid_text_input_callback, NULL, app, app->password, 4);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewByteInput);
}

bool lfrfid_scene_enter_password_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventNext) {
            consumed = true;

            scene_manager_next_scene(scene_manager, next_scene);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        uint32_t prev_scenes[] = {LfRfidSceneExtraActions, LfRfidSceneSavedKeyMenu};
        scene_manager_search_and_switch_to_previous_scene_one_of(
            scene_manager, prev_scenes, sizeof(prev_scenes[0]));
    }

    return consumed;
}

void lfrfid_scene_enter_password_on_exit(void* context) {
    UNUSED(context);
}
