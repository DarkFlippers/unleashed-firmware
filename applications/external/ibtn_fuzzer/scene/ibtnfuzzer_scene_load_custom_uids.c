#include "ibtnfuzzer_scene_load_custom_uids.h"
#include "ibtnfuzzer_scene_run_attack.h"
#include "ibtnfuzzer_scene_entrypoint.h"

#define IBTNFUZZER_UIDS_EXTENSION ".txt"
#define IBTNFUZZER_APP_PATH_FOLDER "/ext/ibtnfuzzer"

bool ibtnfuzzer_load_uids(iBtnFuzzerState* context, const char* file_path) {
    bool result = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    context->uids_stream = buffered_file_stream_alloc(storage);
    result =
        buffered_file_stream_open(context->uids_stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING);
    // Close if loading fails
    if(!result) {
        buffered_file_stream_close(context->uids_stream);
        return false;
    }
    return result;
}

bool ibtnfuzzer_load_custom_uids_from_file(iBtnFuzzerState* context) {
    // Input events and views are managed by file_select
    FuriString* uid_path;
    uid_path = furi_string_alloc();
    furi_string_set(uid_path, IBTNFUZZER_APP_PATH_FOLDER);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, IBTNFUZZER_UIDS_EXTENSION, &I_ibutt_10px);
    browser_options.base_path = IBTNFUZZER_APP_PATH_FOLDER;
    browser_options.hide_ext = false;

    bool res = dialog_file_browser_show(context->dialogs, uid_path, uid_path, &browser_options);

    if(res) {
        res = ibtnfuzzer_load_uids(context, furi_string_get_cstr(uid_path));
    }

    furi_string_free(uid_path);

    return res;
}

void ibtnfuzzer_scene_load_custom_uids_on_enter(iBtnFuzzerState* context) {
    if(ibtnfuzzer_load_custom_uids_from_file(context)) {
        // Force context loading
        ibtnfuzzer_scene_run_attack_on_enter(context);
        context->current_scene = SceneAttack;
    } else {
        ibtnfuzzer_scene_entrypoint_on_enter(context);
        context->current_scene = SceneEntryPoint;
    }
}

void ibtnfuzzer_scene_load_custom_uids_on_exit(iBtnFuzzerState* context) {
    UNUSED(context);
}

void ibtnfuzzer_scene_load_custom_uids_on_tick(iBtnFuzzerState* context) {
    UNUSED(context);
}

void ibtnfuzzer_scene_load_custom_uids_on_event(iBtnFuzzerEvent event, iBtnFuzzerState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
            case InputKeyUp:
            case InputKeyLeft:
            case InputKeyRight:
            case InputKeyOk:
            case InputKeyBack:
                context->current_scene = SceneEntryPoint;
                break;
            default:
                break;
            }
        }
    }
}

void ibtnfuzzer_scene_load_custom_uids_on_draw(Canvas* canvas, iBtnFuzzerState* context) {
    UNUSED(context);
    UNUSED(canvas);
}
