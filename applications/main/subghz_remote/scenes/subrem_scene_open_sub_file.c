#include "../subghz_remote_app_i.h"

void subrem_scene_open_sub_file_error_popup_callback(void* context) {
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(
        app->view_dispatcher, SubRemCustomEventSceneEditOpenSubErrorPopup);
}

SubRemLoadSubState subrem_scene_open_sub_file_dialog(SubGhzRemoteApp* app) {
    furi_assert(app);

    SubRemSubFilePreset* sub = app->map_preset->subs_preset[app->chusen_sub];

    FuriString* temp_file_path = furi_string_alloc();

    if(furi_string_empty(sub->file_path)) {
        furi_string_set(temp_file_path, SUBGHZ_RAW_FOLDER);
    } else {
        furi_string_set(temp_file_path, sub->file_path);
    }

    SubRemLoadSubState ret = SubRemLoadSubStateNotSet;

    DialogsFileBrowserOptions browser_options;

    dialog_file_browser_set_basic_options(
        &browser_options, SUBGHZ_APP_FILENAME_EXTENSION, &I_sub1_10px);
    browser_options.base_path = SUBGHZ_RAW_FOLDER;

    // Input events and views are managed by file_select
    if(!dialog_file_browser_show(app->dialogs, temp_file_path, temp_file_path, &browser_options)) {
    } else {
        // Check sub file
        SubRemSubFilePreset* sub_candidate = subrem_sub_file_preset_alloc();
        furi_string_set(sub_candidate->label, sub->label);
        furi_string_set(sub_candidate->file_path, temp_file_path);

        Storage* storage = furi_record_open(RECORD_STORAGE);
        FlipperFormat* fff_file = flipper_format_file_alloc(storage);

        if(flipper_format_file_open_existing(
               fff_file, furi_string_get_cstr(sub_candidate->file_path))) {
            ret = subrem_sub_preset_load(sub_candidate, app->txrx, fff_file);
        }

        flipper_format_file_close(fff_file);
        flipper_format_free(fff_file);
        furi_record_close(RECORD_STORAGE);

        if(ret == SubRemLoadSubStateOK) {
            subrem_sub_file_preset_free(app->map_preset->subs_preset[app->chusen_sub]);
            app->map_preset->subs_preset[app->chusen_sub] = sub_candidate;
            app->map_not_saved = true;
        } else {
            subrem_sub_file_preset_free(sub_candidate);
        }
    }

    furi_string_free(temp_file_path);

    return ret;
}

void subrem_scene_open_sub_file_on_enter(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;

    SubRemLoadSubState load_state = subrem_scene_open_sub_file_dialog(app);

    Popup* popup = app->popup;
    // popup_set_icon();
    popup_set_header(popup, "ERROR", 63, 16, AlignCenter, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, app);
    popup_set_callback(popup, subrem_scene_open_sub_file_error_popup_callback);
    popup_enable_timeout(popup);

    if(load_state == SubRemLoadSubStateOK) {
        scene_manager_previous_scene(app->scene_manager);
    } else if(load_state == SubRemLoadSubStateNotSet) {
        scene_manager_previous_scene(app->scene_manager);
    } else {
        switch(load_state) {
        case SubRemLoadSubStateErrorFreq:

            popup_set_text(popup, "Bad frequency", 63, 30, AlignCenter, AlignBottom);
            break;
        case SubRemLoadSubStateErrorMod:

            popup_set_text(popup, "Bad modulation", 63, 30, AlignCenter, AlignBottom);
            break;
        case SubRemLoadSubStateErrorProtocol:

            popup_set_text(popup, "Unsupported protocol", 63, 30, AlignCenter, AlignBottom);
            break;

        default:
            break;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDPopup);
    }
}

bool subrem_scene_open_sub_file_on_event(void* context, SceneManagerEvent event) {
    SubGhzRemoteApp* app = context;

    if(event.type == SceneManagerEventTypeCustom &&
       event.event == SubRemCustomEventSceneEditOpenSubErrorPopup) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return false;
}

void subrem_scene_open_sub_file_on_exit(void* context) {
    SubGhzRemoteApp* app = context;

    popup_reset(app->popup);
}
