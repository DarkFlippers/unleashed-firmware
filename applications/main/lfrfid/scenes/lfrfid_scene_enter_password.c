#include "../lfrfid_i.h"
#include "bit_lib.h"
#include "tools/t5577.h"
#include <toolbox/keys_dict.h>
#include "gui/scene_manager.h"

int next_scene;

void lfrfid_scene_enter_password_on_enter(void* context) {
    LfRfid* app = context;
    ByteInput* byte_input = app->byte_input;

    next_scene = scene_manager_get_scene_state(app->scene_manager, LfRfidSceneEnterPassword);

    bool password_set = app->password[0] | app->password[1] | app->password[2] | app->password[3];

    if(next_scene == LfRfidSceneWriteAndSetPass && !password_set) {
        // A word out of the dictionary beats an empty field, and which one hardly matters - so
        // it is whichever the tick lands on, reached by walking to it.
        KeysDict* dict =
            keys_dict_alloc(T5577_DICT_PATH, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);
        const size_t total = keys_dict_get_total_keys(dict);

        size_t skip = total ? furi_get_tick() % total : 0;
        uint8_t key[T5577_DICT_KEY_SIZE];
        bool read = false;

        // The count was taken when the file opened and the walk happens after it, so a word is
        // only in hand once a read says so: a card pulled in between would otherwise leave the
        // stack's contents here, and this is the one write with no way back.
        while((read = keys_dict_get_next_key(dict, key, sizeof(key))) && skip > 0) {
            skip--;
        }

        if(read) {
            memcpy(app->password, key, sizeof(key));
        } else {
            FURI_LOG_W("LfRfid", "No password to suggest from %s", T5577_DICT_PATH);
        }

        keys_dict_free(dict);
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
