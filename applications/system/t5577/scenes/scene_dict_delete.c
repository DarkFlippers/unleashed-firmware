#include "../t5577_app.h"
#include <bit_lib/bit_lib.h>

#define TAG "T5577Dict"

// The word at that place in the list, read back rather than carried over from the list scene:
// the file is the truth, and it could have changed between the two screens.
static bool t5577_app_scene_dict_delete_word(uint32_t index, uint32_t* word) {
    KeysDict* dict =
        keys_dict_alloc(T5577_DICT_USER_PATH, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);
    uint8_t key[T5577_DICT_KEY_SIZE];
    bool found = false;

    for(uint32_t i = 0; i <= index; i++) {
        found = keys_dict_get_next_key(dict, key, sizeof(key));
        if(!found) break;
    }

    keys_dict_free(dict);

    if(found) *word = bit_lib_bytes_to_num_be(key, sizeof(key));

    return found;
}

static void
    t5577_app_scene_dict_delete_button(GuiButtonType type, InputType input, void* context) {
    T5577App* app = context;

    if(input != InputTypeShort) return;

    if(type == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventDictCancel);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventDictDelete);
    }
}

void t5577_app_scene_dict_delete_on_enter(void* context) {
    T5577App* app = context;

    const uint32_t index =
        scene_manager_get_scene_state(app->scene_manager, T5577AppSceneDictDelete);
    uint32_t word = 0;
    const bool found = t5577_app_scene_dict_delete_word(index, &word);

    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 0, 2, AlignLeft, AlignTop, FontPrimary, found ? "Delete?" : "Gone");

    FuriString* line = furi_string_alloc();

    if(found) {
        furi_string_printf(line, "%08lX", (unsigned long)word);
        widget_add_string_element(
            app->widget, 0, 20, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(line));
        // A text box rather than a string element: that one draws a newline as nothing, so the
        // second line of a two-line caption is simply lost.
        widget_add_text_box_element(
            app->widget,
            0,
            34,
            128,
            18,
            AlignLeft,
            AlignTop,
            "Clear Password stops trying it",
            false);
        widget_add_button_element(
            app->widget, GuiButtonTypeRight, "Delete", t5577_app_scene_dict_delete_button, app);
    } else {
        // The list said there was a word here and the file no longer has one. Offering Delete
        // would ask the user to confirm removing a blank line.
        FURI_LOG_W(TAG, "word %lu is gone from the user list", (unsigned long)index);
        widget_add_text_box_element(
            app->widget,
            0,
            20,
            128,
            22,
            AlignLeft,
            AlignTop,
            "That word is no longer in the list",
            false);
    }

    furi_string_free(line);

    widget_add_button_element(
        app->widget,
        GuiButtonTypeLeft,
        found ? "Cancel" : "Back",
        t5577_app_scene_dict_delete_button,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWidget);
}

bool t5577_app_scene_dict_delete_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == T5577AppCustomEventDictCancel) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }

    if(event.event != T5577AppCustomEventDictDelete) return false;

    const uint32_t index =
        scene_manager_get_scene_state(app->scene_manager, T5577AppSceneDictDelete);
    uint32_t word = 0;
    bool deleted = false;

    if(t5577_app_scene_dict_delete_word(index, &word)) {
        uint8_t key[T5577_DICT_KEY_SIZE];

        bit_lib_num_to_bytes_be(word, sizeof(key), key);

        KeysDict* dict =
            keys_dict_alloc(T5577_DICT_USER_PATH, KeysDictModeOpenAlways, T5577_DICT_KEY_SIZE);

        deleted = keys_dict_delete_key(dict, key, sizeof(key));
        keys_dict_free(dict);
    }

    if(deleted) {
        t5577_app_say(app, "Deleted", "Clear Password will\nstop trying it");
    } else {
        // Full SD, a read-only card, a file that changed under us: the user would otherwise
        // land back on a list that still shows the word.
        FURI_LOG_E(TAG, "failed to delete word %lu", (unsigned long)index);
        t5577_app_say(app, "Not deleted", "Could not write to\nthe SD card");
    }

    // The list is rebuilt from the file on the way back, so a deleted word leaves with it - but
    // the last word leaving would land the user on a list with nothing in it and no way on, so
    // that case goes to the menu instead, where the count is now zero and List has gone.
    KeysDict* left =
        keys_dict_alloc(T5577_DICT_USER_PATH, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);
    const bool empty = keys_dict_get_total_keys(left) == 0;

    keys_dict_free(left);

    if(empty) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneDict);
    } else {
        scene_manager_previous_scene(app->scene_manager);
    }

    return true;
}

void t5577_app_scene_dict_delete_on_exit(void* context) {
    T5577App* app = context;
    widget_reset(app->widget);
}
