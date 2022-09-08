#include "../subbrute.h"
#include "m-string.h"
#include "subghz/types.h"
#include <lib/toolbox/random_name.h>
#include <gui/modules/validators.h>
#include <lib/toolbox/path.h>

#define MAX_TEXT_INPUT_LEN 22

bool backpressed = false;

bool subbrute_path_is_file(string_t path) {
    return string_end_with_str_p(path, ".sub");
}
// method modified from subghz_i.c
// https://github.com/flipperdevices/flipperzero-firmware/blob/b0daa601ad5b87427a45f9089c8b403a01f72c2a/applications/subghz/subghz_i.c#L417-L456
bool subbrute_save_protocol_to_file(Stream* flipper_format_stream, const char* dev_file_name) {
    furi_assert(dev_file_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool saved = false;
    string_t file_dir;
    string_init(file_dir);

    path_extract_dirname(dev_file_name, file_dir);
    do {
        if(!storage_simply_mkdir(storage, string_get_cstr(file_dir))) {
            FURI_LOG_E(TAG, "(save) Cannot mkdir");
            break;
        }

        if(!storage_simply_remove(storage, dev_file_name)) {
            FURI_LOG_E(TAG, "(save) Cannot remove");
            break;
        }

        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, dev_file_name, FSOM_CREATE_ALWAYS);

        saved = true;
        FURI_LOG_D(TAG, "(save) OK Save");
    } while(0);
    string_clear(file_dir);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

void custom_callback(SubBruteState* context) {
    if(strcmp(context->file_name_tmp, "")) {
        string_cat_printf(context->file_path, "/%s%s", context->file_name_tmp, ".sub");
        if(subbrute_path_is_file(context->file_path_tmp)) {
            context->current_scene = SceneAttack;
            return; //false;

        } else {
            subbrute_save_protocol_to_file(context->stream, string_get_cstr(context->file_path));
        }

        string_set_str(context->file_path, EXT_PATH("subghz"));
        string_reset(context->file_path_tmp);

        //scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveSuccess);
        context->current_scene = SceneAttack;
        return; //true;
    } else {
        //error no file name
        context->current_scene = SceneAttack;
        return; //true;
    }
}

void subbrute_scene_save_name_text_input_callback(void* context) {
    furi_assert(context);
    SubBruteState* statee = context;
    custom_callback(statee);
}

void subbrute_scene_save_name_on_tick(SubBruteState* context) {
    if(backpressed) {
        void* validator_context = text_input_get_validator_callback_context(context->text_input);
        text_input_set_validator(context->text_input, NULL, NULL);
        validator_is_file_free(validator_context);

        // Clear view
        text_input_reset(context->text_input);

        // TextInput
        view_dispatcher_remove_view(context->view_dispatcher, 0);
        text_input_free(context->text_input);

        // Popup
        view_dispatcher_remove_view(context->view_dispatcher, 1);
        popup_free(context->popup);

        context->current_scene = SceneAttack;
    }
}

bool subbrute_back_event_callback(void* context) {
    UNUSED(context);
    backpressed = true;
    return true;
}

void subbrute_scene_save_name_on_enter(SubBruteState* context) {
    // Text Input
    context->text_input = text_input_alloc();
    view_dispatcher_add_view(
        context->view_dispatcher, 0, text_input_get_view(context->text_input));

    // Popup
    context->popup = popup_alloc();
    view_dispatcher_add_view(context->view_dispatcher, 1, popup_get_view(context->popup));

    // Setup view
    TextInput* text_input = context->text_input;
    bool dev_name_empty = false;

    string_t file_name;
    string_t dir_name;
    string_init(file_name);
    string_init(dir_name);

    if(!subbrute_path_is_file(context->file_path)) {
        char file_name_buf[64] = {0};
        set_random_name(file_name_buf, 64);
        string_set_str(file_name, file_name_buf);
        string_set_str(context->file_path, EXT_PATH("subghz"));
        //highlighting the entire filename by default
        dev_name_empty = true;
    } else {
        string_set(context->file_path_tmp, context->file_path);
        path_extract_dirname(string_get_cstr(context->file_path), dir_name);
        path_extract_filename(context->file_path, file_name, true);
        string_set(context->file_path, dir_name);
    }

    strncpy(context->file_name_tmp, string_get_cstr(file_name), 64);
    text_input_set_header_text(text_input, "Name signal");
    text_input_set_result_callback(
        text_input,
        subbrute_scene_save_name_text_input_callback,
        context,
        context->file_name_tmp,
        MAX_TEXT_INPUT_LEN, // buffer size
        dev_name_empty);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(string_get_cstr(context->file_path), ".sub", "");
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    string_clear(file_name);
    string_clear(dir_name);

    view_dispatcher_set_navigation_event_callback(
        context->view_dispatcher, subbrute_back_event_callback);

    view_dispatcher_switch_to_view(context->view_dispatcher, 0);
}

void subbrute_scene_save_name_on_event(SubBruteEvent event, SubBruteState* context) {
    UNUSED(context);
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
            case InputKeyUp:
            case InputKeyLeft:
            case InputKeyRight:
            case InputKeyOk:
                break;
            case InputKeyBack:
                //context->current_scene = SceneAttack;
                break;
            }
        }
    }
}

void subbrute_scene_save_name_on_exit(SubBruteState* context) {
    if(!backpressed) {
        // Clear validator
        void* validator_context = text_input_get_validator_callback_context(context->text_input);
        text_input_set_validator(context->text_input, NULL, NULL);
        validator_is_file_free(validator_context);

        // Clear view
        text_input_reset(context->text_input);

        // Setup view
        Popup* popup = context->popup;
        popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
        popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
        popup_set_timeout(popup, 1500);
        popup_set_context(popup, context);
        popup_set_callback(popup, NULL);
        popup_enable_timeout(popup);
        view_dispatcher_switch_to_view(context->view_dispatcher, 1);

        furi_delay_ms(1050);
        // Clear view
        //Popup* popup = subghz->popup;
        popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
        popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
        popup_set_icon(popup, 0, 0, NULL);
        popup_set_callback(popup, NULL);
        popup_set_context(popup, NULL);
        popup_set_timeout(popup, 0);
        popup_disable_timeout(popup);

        // TextInput
        view_dispatcher_remove_view(context->view_dispatcher, 0);
        text_input_free(context->text_input);

        // Popup
        view_dispatcher_remove_view(context->view_dispatcher, 1);
        popup_free(context->popup);
    } else {
        backpressed = false;
    }
}