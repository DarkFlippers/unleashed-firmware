#include "../t5577_app.h"
#include <t5577_icons.h>

// Counting walks the file, so the handle is opened and closed for the count alone. Both are
// opened OpenExisting - merely looking at this screen must not create a file, which is what
// T5577_DICT_USER_PATH's own documentation promises and is where NFC's screen differs.
static uint32_t t5577_app_scene_dict_count(const char* path) {
    KeysDict* dict = keys_dict_alloc(path, KeysDictModeOpenExisting, T5577_DICT_KEY_SIZE);
    const uint32_t total = (uint32_t)keys_dict_get_total_keys(dict);

    keys_dict_free(dict);

    return total;
}

static void t5577_app_scene_dict_button(GuiButtonType type, InputType input, void* context) {
    T5577App* app = context;

    if(input != InputTypeShort) return;

    if(type == GuiButtonTypeCenter) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventDictAdd);
    } else if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventDictList);
    }
}

void t5577_app_scene_dict_on_enter(void* context) {
    T5577App* app = context;

    widget_reset(app->widget);

    const uint32_t system_words = t5577_app_scene_dict_count(T5577_DICT_PATH);
    const uint32_t user_words = t5577_app_scene_dict_count(T5577_DICT_USER_PATH);

    // Laid out as the NFC key dictionaries are, down to the wording: the two files are the same
    // two ideas there, and somebody who has met that screen should not have to learn a second
    // vocabulary for this one.
    widget_add_string_element(
        app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "T5577 Passwords");

    FuriString* line = furi_string_alloc();

    // Both counts, because the sweep uses both and the difference is the whole point of the
    // split: the system words come back with every firmware update and the user's do not.
    furi_string_printf(line, "System dict: %lu", (unsigned long)system_words);
    widget_add_string_element(
        app->widget, 0, 20, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(line));

    furi_string_printf(line, "User dict: %lu", (unsigned long)user_words);
    widget_add_string_element(
        app->widget, 0, 32, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(line));

    furi_string_free(line);

    widget_add_icon_element(app->widget, 87, 13, &I_Key_39x36);

    widget_add_button_element(
        app->widget, GuiButtonTypeCenter, "Add", t5577_app_scene_dict_button, app);

    // Only where there is something to list. The system words are not listed at all - they are
    // replaced wholesale by the next update, so editing them achieves nothing.
    if(user_words > 0) {
        widget_add_button_element(
            app->widget, GuiButtonTypeRight, "List", t5577_app_scene_dict_button, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewWidget);
}

bool t5577_app_scene_dict_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == T5577AppCustomEventDictAdd) {
        // Zeroed here rather than in the Add scene, so coming back from a duplicate warning
        // still shows what was typed. The buffer is this feature's own: the password flow's
        // would otherwise show its last word through as the start of this one.
        memset(app->dict_bytes, 0, sizeof(app->dict_bytes));
        scene_manager_next_scene(app->scene_manager, T5577AppSceneDictAdd);
        return true;
    }

    if(event.event == T5577AppCustomEventDictList) {
        scene_manager_set_scene_state(app->scene_manager, T5577AppSceneDictList, 0);
        scene_manager_next_scene(app->scene_manager, T5577AppSceneDictList);
        return true;
    }

    return false;
}

void t5577_app_scene_dict_on_exit(void* context) {
    T5577App* app = context;
    widget_reset(app->widget);
}
