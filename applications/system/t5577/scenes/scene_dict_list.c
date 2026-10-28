#include "../t5577_app.h"
#include <bit_lib/bit_lib.h>

#define TAG "T5577Dict"

/** Words the list shows.
 *
 * Every one is a Submenu item holding its own string, and this list is the user's own words
 * rather than the shipped hundred-odd, so it is short in practice. The cap is here so a
 * hand-edited file cannot turn a screen into an allocation.
 */
#define T5577_DICT_LIST_MAX 100

static void t5577_app_scene_dict_list_callback(void* context, uint32_t index) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void t5577_app_scene_dict_list_on_enter(void* context) {
    T5577App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Select to delete:");

    // OpenExisting: the list is a read, and nothing should create the file just by looking at it.
    KeysDict* dict =
        keys_dict_alloc(T5577_DICT_USER_PATH, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);
    const size_t total = MIN((size_t)T5577_DICT_LIST_MAX, keys_dict_get_total_keys(dict));

    for(size_t i = 0; i < total; i++) {
        uint8_t key[T5577_DICT_KEY_SIZE];

        if(!keys_dict_get_next_key(dict, key, sizeof(key))) {
            // The count said there were more. A short list rather than no list, and a line
            // saying which it was.
            FURI_LOG_W(TAG, "user list ended early at %u of %u", (unsigned)i, (unsigned)total);
            break;
        }

        char label[16];

        snprintf(
            label,
            sizeof(label),
            "%08lX",
            (unsigned long)bit_lib_bytes_to_num_be(key, sizeof(key)));
        submenu_add_item(app->submenu, label, i, t5577_app_scene_dict_list_callback, app);
    }

    keys_dict_free(dict);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, T5577AppSceneDictList));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewSubmenu);
}

bool t5577_app_scene_dict_list_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneDictList, event.event);
    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneDictDelete, event.event);
    scene_manager_next_scene(app->scene_manager, T5577AppSceneDictDelete);

    return true;
}

void t5577_app_scene_dict_list_on_exit(void* context) {
    T5577App* app = context;
    submenu_reset(app->submenu);
}
