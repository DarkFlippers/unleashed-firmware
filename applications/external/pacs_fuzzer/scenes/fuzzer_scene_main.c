#include "../fuzzer_i.h"
#include "../helpers/fuzzer_custom_event.h"

#include "../lib/worker/protocol.h"

void fuzzer_scene_main_callback(FuzzerCustomEvent event, void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

static bool fuzzer_scene_main_load_custom_dict(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    FuzzerConsts* consts = app->fuzzer_const;

    furi_string_set_str(app->file_path, consts->custom_dict_folder);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, consts->custom_dict_extension, &I_rfid_10px);
    browser_options.base_path = consts->custom_dict_folder;
    browser_options.hide_ext = false;

    bool res =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options);

    return res;
}

static bool fuzzer_scene_main_load_key(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    FuzzerConsts* consts = app->fuzzer_const;

    furi_string_set_str(app->file_path, consts->path_key_folder);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, consts->key_extension, consts->key_icon);
    browser_options.base_path = consts->path_key_folder;
    browser_options.hide_ext = true;

    bool res =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options);

    return res;
}

void fuzzer_scene_main_on_enter(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    fuzzer_view_main_set_callback(app->main_view, fuzzer_scene_main_callback, app);

    fuzzer_view_main_update_data(app->main_view, app->fuzzer_state);

    view_dispatcher_switch_to_view(app->view_dispatcher, FuzzerViewIDMain);
}

bool fuzzer_scene_main_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FuzzerCustomEventViewMainBack) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
        } else if(event.event == FuzzerCustomEventViewMainOk) {
            fuzzer_view_main_get_state(app->main_view, &app->fuzzer_state);

            // TODO error logic
            bool loading_ok = false;

            switch(app->fuzzer_state.menu_index) {
            case FuzzerMainMenuIndexDefaultValues:

                loading_ok =
                    fuzzer_worker_init_attack_dict(app->worker, app->fuzzer_state.proto_index);

                if(!loading_ok) {
                    // error
                }
                break;

            case FuzzerMainMenuIndexLoadFile:
                if(!fuzzer_scene_main_load_key(app)) {
                    break;
                } else {
                    if(fuzzer_worker_load_key_from_file(
                           app->worker,
                           app->fuzzer_state.proto_index,
                           furi_string_get_cstr(app->file_path))) {
                        scene_manager_next_scene(app->scene_manager, FuzzerSceneFieldEditor);
                        FURI_LOG_I("Scene", "Load ok");
                    } else {
                        FURI_LOG_W("Scene", "Load err");
                    }
                }
                break;

            case FuzzerMainMenuIndexLoadFileCustomUids:
                if(!fuzzer_scene_main_load_custom_dict(app)) {
                    break;
                } else {
                    loading_ok = fuzzer_worker_init_attack_file_dict(
                        app->worker, app->fuzzer_state.proto_index, app->file_path);
                }
                break;

            default:
                break;
            }

            if(loading_ok) {
                scene_manager_next_scene(app->scene_manager, FuzzerSceneAttack);
            }
            consumed = true;
        }
    }

    return consumed;
}

void fuzzer_scene_main_on_exit(void* context) {
    // furi_assert(context);
    // PacsFuzzerApp* app = context;
    UNUSED(context);
}